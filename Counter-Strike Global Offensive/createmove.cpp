#include "hooked.hpp"
#include "displacement.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "prediction.hpp"
#include "movement.hpp"
#include "aimbot.hpp"
#include "anti_aimbot.hpp"
#include "menu.h"
#include "game_movement.h"
#include "lag_compensation.hpp"
#include <intrin.h>
#include "rmenu.hpp"

using SendNetMsg_t = bool(__thiscall*)(INetChannel*, INetMessage&, bool, bool);

namespace Hooked
{

	//void __fastcall CreateMove(void* ecx, void* edx, int sequence_number, float input_sample_frametime, bool active)
	bool __stdcall CreateMove(float flInputSampleTime, CUserCmd* cmd)
	{
		//Source::m_pClientState->m_ptrNetChannel->m_nOutSequenceNr = prev_seq;
		//using Fn = void(__thiscall*)(void*, float, CUserCmd*);
		//Source::m_pClientSwap->VCall<Fn>(Index::IBaseClientDLL::CreateMove)(ecx, flInputSampleTime, cmd);

		cheat::features::antiaimbot.did_tickbase = cheat::features::antiaimbot.do_tickbase;
		cheat::features::antiaimbot.do_tickbase = false;

		if (!cmd || cmd->command_number == 0 || !Source::m_pEngine->IsInGame())
			return false;

		auto& prediction = Engine::Prediction::Instance();
		auto& movement = Engine::Movement::Instance();

		bool send_packet = true;
		cheat::main::jittering = false;
		cheat::main::fakewalking = cheat::game::pressed_keys[int(cheat::Cvars.anti_aim_slowwalk_key.GetValue())] && cheat::Cvars.anti_aim_slowwalk_key.GetValue() > 0.f;
		cheat::features::antiaimbot.skip_lags_this_tick = false;
		cheat::features::antiaimbot.unchoke = false;

		if (cheat::Cvars.Exploits_air_desync.GetValue() && !((cmd->buttons & IN_DUCK)) && cheat::main::local())
		{
			if (!(cmd->buttons & IN_JUMP) || (cheat::main::local()->m_fFlags() & FL_ONGROUND) || cheat::main::local()->m_vecVelocity().z >= -140.0)
				cmd->buttons &= ~IN_DUCK;
			else
				cmd->buttons |= IN_DUCK;
		}

		if (auto net = Source::m_pEngine->GetNetChannelInfo(); net != nullptr)
		{
			cheat::main::latency = Source::m_pEngine->GetNetChannelInfo()->GetLatency(0);
			cheat::main::latency += Source::m_pEngine->GetNetChannelInfo()->GetLatency(1);
		}
		else
			cheat::main::latency = 0;

		Source::m_pGlobalVars->curtime = TICKS_TO_TIME(cheat::main::local()->m_nTickBase());
		Source::m_pGlobalVars->frametime = Source::m_pPrediction->m_bEnginePaused ? 0.f : Source::m_pGlobalVars->interval_per_tick;

		movement->Begin(cmd, send_packet);

		cmd->buttons |= IN_BULLRUSH;

		if (cheat::main::fast_autostop)
			cheat::features::aimbot.autostop(cmd, send_packet, (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon())));

		const auto valid = cheat::main::local() && cheat::main::local()->GetClientClass() && !cheat::main::local()->IsDead() && cheat::main::local()->m_MoveType() != 10;

		if (valid) {

			cheat::features::antiaimbot.DoLBY(cmd, &send_packet);

			static bool did_fakewalk = false;

			if (cheat::main::fakewalking) {

				if (!did_fakewalk) {
					Engine::Movement::Instance()->quick_stop(cmd);

					if (cheat::main::local()->m_vecVelocity().Length2D() <= 0.1f)
						did_fakewalk = true;
				}

				static int choked = 0;
				choked = choked > 7 ? 0 : choked + 1;
				cmd->forwardmove = choked < 2 || choked > 5 ? 0 : cmd->forwardmove;
				cmd->sidemove = choked < 2 || choked > 5 ? 0 : cmd->sidemove;
				send_packet = choked < 1;
			}
			else
				did_fakewalk = false;
		}

		prediction->Begin(cmd);

		if (valid)
		{
			cheat::game::last_cmd = cmd;

			auto weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon()));

			if (weapon)
			{
				cheat::features::antiaimbot.simulate_lags(cmd, &send_packet);

				auto aimbot_worked = cheat::features::aimbot.work(cmd, send_packet);// || cheat::features::aimbot.knifebot_work(cmd, send_packet);

				auto can_shoot = (aimbot_worked || ((cheat::game::pressed_keys[1] || cheat::game::pressed_keys[2]) && !cheat::features::menu.menu_opened)) && weapon->m_iItemDefinitionIndex() == 64 && weapon->can_shoot() || weapon->m_iItemDefinitionIndex() != 64 && cmd->buttons & IN_ATTACK && weapon->can_shoot();

				if (can_shoot)
				{
					weapon->UpdateAccuracyPenalty();

					//auto sv_usercmd_custom_random_seed = Source::m_pCvar->FindVar("sv_usercmd_custom_random_seed");

					if (cheat::Cvars.RageBot_NoSpread.GetValue() && cheat::convars::sv_usercmd_custom_random_seed == 0 && (int)cheat::Cvars.Misc_AntiUT.GetValue() == 0)
					{
						//if (weapon->IsFireTime() || weapon->IsSecondaryFireTime())
						//{
						auto weapon_inaccuracy = weapon->GetInaccuracy();
						auto weapon_spread = weapon->GetSpread();

						RandomSeed((cmd->random_seed & 255) + 1);

						auto rand1 = RandomFloat(0.0f, 1.0f);
						auto rand_pi1 = RandomFloat(0.0f, 6.2831855f/*2.0f * RadPi*/);
						auto rand2 = RandomFloat(0.0f, 1.0f);
						auto rand_pi2 = RandomFloat(0.0f, 6.2831855f/*2.0f * RadPi*/);

						int id = weapon->m_iItemDefinitionIndex();
						auto recoil_index = weapon->m_flRecoilIndex();

						if (id == 64 && cmd->buttons & IN_ATTACK2)
						{
							rand1 = 1.0f - rand1 * rand1;
							rand2 = 1.0f - rand2 * rand2;
						}
						else if (id == 28 && recoil_index < 3.0f)
						{
							for (int i = 3; i > recoil_index; i--)
							{
								rand1 *= rand1;
								rand2 *= rand2;
							}

							rand1 = 1.0f - rand1;
							rand2 = 1.0f - rand2;
						}

						auto rand_inaccuracy = rand1 * weapon_inaccuracy;
						auto rand_spread = rand2 * weapon_spread;

						Vector2D spread =
						{
							std::cos(rand_pi1) * rand_inaccuracy + std::cos(rand_pi2) * rand_spread,
							std::sin(rand_pi1) * rand_inaccuracy + std::sin(rand_pi2) * rand_spread,
						};

						spread *= -1;

						// 
						// pitch/yaw/roll
						// 
						Vector side, up;
						Vector forward = QAngle::Zero.ToVectors(&side, &up);

						Vector direction = forward + (side * spread.x) + (up * spread.y);
						direction.Normalize();

						QAngle angles_spread = direction.ToEulerAngles();

						angles_spread.x -= cmd->viewangles.x;
						angles_spread.Normalize();

						forward = angles_spread.ToVectorsTranspose(&side, &up);

						angles_spread = forward.ToEulerAngles(&up);
						angles_spread.Normalize();

						angles_spread.y += cmd->viewangles.y;
						angles_spread.Normalize();

						cmd->viewangles = angles_spread;

						// 
						// pitch/roll
						// 
						// cmd->viewangles.x += ToDegrees( std::atan( spread.Length() ) );
						// cmd->viewangles.z = -ToDegrees( std::atan2( spread.x, spread.y ) );

						// 
						// yaw/roll
						// 
						// cmd->viewangles.y += ToDegrees( std::atan( spread.Length() ) );
						// cmd->viewangles.z = -( ToDegrees( std::atan2( spread.x, spread.y ) ) - 90.0f );
					}

					auto weapon_recoil_scale = Source::m_pCvar->FindVar("weapon_recoil_scale")->GetFloat();

					if (weapon_recoil_scale > 0.f && cheat::Cvars.RageBot_NoRecoil.GetValue()) {
						cmd->viewangles -= cheat::main::local()->m_aimPunchAngle() * weapon_recoil_scale;

						if (!cheat::Cvars.Misc_AntiUT.GetValue()) {
							cmd->viewangles.x = 180.f - cmd->viewangles.x;
							cmd->viewangles.y = cmd->viewangles.y + 180.f;
						}
					}

					cheat::main::last_shot_time_clientside = Source::m_pGlobalVars->realtime;

					cmd->viewangles.Normalize();
				}
				else
					cheat::features::antiaimbot.work(cmd, &send_packet);
			}

			cheat::features::antiaimbot.do_exloits(cmd, send_packet);

			movement->End(cmd);

			if (cheat::Cvars.Misc_AntiUT.GetValue())
				cmd->viewangles.Clamp();
		}

		prediction->End();

		//if (csgo.m_client_state() != nullptr && csgo.m_prediction() != nullptr && csgo.m_engine()->IsInGame() && ctx.m_local() && !ctx.m_local()->IsDead()/* && ctx.last_frame_stage == FRAME_NET_UPDATE_END*/) {
		//	const auto ticks = ctx.host_frameticks() + csgo.m_client_state()->m_iChockedCommands;
		//
		//	if (ticks > 16) // detected rubberbanding
		//		send_packet = true;
		//}

		if (send_packet && cheat::main::update_cycle & CYCLE_PRE_UPDATE) {
			cheat::main::update_cycle &= ~CYCLE_PRE_UPDATE;
			cheat::main::update_cycle |= CYCLE_UPDATE;
		}

		static auto wasonground = true;

		const auto player_flags = cheat::main::local()->m_fFlags();

		if (!wasonground || !(cheat::main::local()->m_fFlags() & FL_ONGROUND))
			cheat::main::local()->m_fFlags() &= ~FL_ONGROUND;

		if (!Source::m_pClientState->m_iChockedCommands)
			wasonground = player_flags & FL_ONGROUND;

		const auto inlandanim = cheat::main::local()->get_animation_state() && cheat::main::local()->get_animation_state()->hitgr_anim;
		const bool onground = cheat::main::local()->m_fFlags() & FL_ONGROUND;

		if (inlandanim && onground && wasonground)
		{
			cheat::features::antiaimbot.last_sent_angle.x = -10.f;
			cheat::main::fix_modify_eye_pos = true;
		}
		else
			cheat::main::fix_modify_eye_pos = false;

		cheat::main::local()->m_fFlags() = player_flags;

		if (send_packet && cheat::main::local()) {
			cheat::features::antiaimbot.last_sent_origin = cheat::main::local()->m_vecOrigin();
			cheat::features::antiaimbot.last_sent_simtime = cheat::main::local()->m_flSimulationTime();
			cheat::features::antiaimbot.last_real_angle = cmd->viewangles;
			//memcpy(cheat::features::antiaimbot.last_sent_matrix, cheat::main::local()->m_CachedBoneData().Base(), sizeof(matrix3x4_t) * cheat::main::local()->GetBoneCount());
		}

		//cheat::features::lagcomp.update_fake_animations();

		if ((Source::m_pClientState->m_iChockedCommands >= cheat::Cvars.Misc_fakelag_value.GetValue() && !send_packet) || Source::m_pClientState->m_iChockedCommands >= (cheat::Cvars.Misc_fakelag_value.GetValue() + 1) /*|| cheat::features::antiaimbot.unchoke*/)
			send_packet = true;

		/*if (Source::m_pClientState && Source::m_pClientState->m_ptrNetChannel) {
			if (Source::m_pNetChannelSwap && Source::m_pNetChannelSwap->hooked) {
				uintptr_t* vtable = *(uintptr_t**)Source::m_pClientState->m_ptrNetChannel;

				if (vtable != Source::m_pNetChannelSwap->m_pRestore) {
					Source::m_pNetChannelSwap.reset();
				}
			}
		}*/

		//if (!Source::m_pNetChannelSwap && Source::m_pClientState && Source::m_pClientState->m_ptrNetChannel && cheat::main::local() && !cheat::main::local()->IsDead())
		//{
		//	Source::m_pNetChannelSwap = std::make_shared<Memory::VmtSwap>((DWORD**)Source::m_pClientState->m_ptrNetChannel);
		//	Source::m_pNetChannelSwap->Hook(&Hooked::SendNetMsg, 48);
		//}

		cmd->buttons &= ~IN_MOVERIGHT;
		cmd->buttons &= ~IN_MOVELEFT;
		cmd->buttons &= ~IN_FORWARD;
		cmd->buttons &= ~IN_BACK;

		if (cmd->forwardmove > 0.f)
			cmd->buttons |= IN_FORWARD;
		else if (cmd->forwardmove < 0.f)
			cmd->buttons |= IN_BACK;

		if (cmd->sidemove > 0.f)
			cmd->buttons |= IN_MOVERIGHT;
		else if (cmd->sidemove < 0.f)
			cmd->buttons |= IN_MOVELEFT;

		if (!Source::m_pClientStateSwap && Source::m_pClientState && (CClientState*)(uint32_t(Source::m_pClientState) + 8))
		{
			Source::m_pClientStateSwap = std::make_shared<Memory::VmtSwap>((CClientState*)(uint32_t(Source::m_pClientState) + 8));
			Source::m_pClientStateSwap->Hook(&Hooked::PacketStart, 5);
			//Source::m_pClientStateSwap->Hook(&Hooked::PacketEnd, 6);
		}

		if (send_packet)
			cheat::main::command_numbers.emplace_front(cmd->command_number);
		else
		{
			if (Source::m_pClientState != nullptr) {
				auto net_channel = *reinterpret_cast<INetChannel**>(reinterpret_cast<uintptr_t>(Source::m_pClientState) + 0x9C);
				if (net_channel != nullptr && net_channel->m_nChokedPackets > 0 && !(net_channel->m_nChokedPackets % 4)) {
					const auto current_choke = net_channel->m_nChokedPackets;
					net_channel->m_nChokedPackets = 0;
					Memory::VCall<int(__thiscall*)(INetChannel*, void*)>(net_channel, 48)(net_channel, 0);
					--net_channel->m_nOutSequenceNr;
					net_channel->m_nChokedPackets = current_choke;
				}
			}
		}

		while (cheat::main::command_numbers.size() > int(1.0f / Source::m_pGlobalVars->interval_per_tick)) {
			cheat::main::command_numbers.pop_back();
		}

		/*cheat::main::m_corrections_data.push_front(cmd->command_number);

		while (cheat::main::m_corrections_data.size() > int(1.0f / Source::m_pGlobalVars->interval_per_tick)) {
			cheat::main::m_corrections_data.pop_back();
		}*/

		if (!send_packet)
			cheat::features::antiaimbot.last_sent_angle = cmd->viewangles;

		*reinterpret_cast<bool*>(*reinterpret_cast<uintptr_t*>(static_cast<char*>(_AddressOfReturnAddress()) - 0x4) - 0x1C) = send_packet;

		return false;
	}
}