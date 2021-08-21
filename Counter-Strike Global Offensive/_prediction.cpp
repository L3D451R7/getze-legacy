#include "hooked.hpp"
#include "prediction.hpp"
#include "player.hpp"
#include "angle_resolver.hpp"
#include "lag_compensation.hpp"
#include "anti_aimbot.hpp"
#include <intrin.h>
#include "weapon.hpp"

namespace Hooked
{

	void FixViewmodel(CUserCmd* cmd, bool restore)
	{
		if (cheat::main::local()->IsDead())
			return;

		static float cycleBackup = 0.0f;
		static bool weaponAnimation = false;

		auto viewmodel = Source::m_pEntList->GetClientEntityFromHandle((CBaseHandle)cheat::main::local()->m_hViewModel());

		if (viewmodel) {
			if (restore) {
				weaponAnimation = cmd->weaponselect > 0 || cmd->buttons & (IN_ATTACK2 | IN_ATTACK);
				cycleBackup = viewmodel->m_flCycle();//*(float*)(uintptr_t(viewModel) + 0xA14);
			}
			else if (weaponAnimation && cheat::main::update_cycle == 0) {
				if (viewmodel->m_flCycle() == 0.0f && cycleBackup > 0.0f)
					cheat::main::update_cycle |= CYCLE_PRE_UPDATE;
			}
		}
	}

	//// runcommand 
	//float VelocityFix(int command_number, bool before) {
	//	float velModifier = cheat::main::local()->m_flVelocityModifier();

	//	if (cheat::main::last_velocity_modifier_tick == -1)
	//		return cheat::main::local()->m_flVelocityModifier();

	//	auto delta = command_number - cheat::main::last_velocity_modifier_tick - 1;

	//	if (!before)
	//		delta = command_number - cheat::main::last_velocity_modifier_tick;

	//	const auto vel_mod_backup = cheat::main::last_velocity_modifier;

	//	if (delta < 0 || vel_mod_backup == 1.0f) {
	//		velModifier = cheat::main::local()->m_flVelocityModifier();
	//	}
	//	else if (delta) {
	//		auto v6 = ((Source::m_pGlobalVars->interval_per_tick * 0.4f) * float(delta)) + vel_mod_backup;
	//		if (v6 <= 1.0f) {
	//			if (v6 >= 0.0f)
	//				velModifier = v6;
	//			else
	//				velModifier = 0.0f;
	//		}
	//		else
	//			velModifier = 1.0f;
	//	}
	//	else
	//		velModifier = cheat::main::last_velocity_modifier;

	//	return velModifier;
	//}

	void __fastcall RunCommand(void* ecx, void* edx, C_BasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper)
	{
		using Fn = void(__thiscall*)(void*, C_BasePlayer*, CUserCmd*, IMoveHelper*);
		static int old_tickbase = 0;

		if (cheat::main::local() != nullptr && player == cheat::main::local()) {
			FixViewmodel(ucmd, true);
			//cheat::main::local()->m_flVelocityModifier() = VelocityFix(ucmd->command_number, true);

			if (cheat::main::can_store_netvars)
			{
				//Engine::Prediction::Instance()->FixNetvarCompression(true);
				cheat::main::run_cmd_got_called = true;
			}

			cheat::main::last_velocity_modifier = cheat::main::local()->m_flVelocityModifier();
		}

		Source::m_pPredictionSwap->VCall<Fn>(Index::IPrediction::RunCommand)(ecx, player, ucmd, moveHelper);

		if (cheat::main::local() != nullptr && player == cheat::main::local())
		{
			cheat::main::local()->m_flVelocityModifier() = cheat::main::last_velocity_modifier;
			//cheat::main::local()->m_flVelocityModifier() = VelocityFix(ucmd->command_number, false);
			FixViewmodel(ucmd, false);

			if (cheat::main::can_store_netvars && cheat::main::last_frame_stage == 4)
				cheat::main::last_netvars_update_tick = ucmd->command_number;

			const auto weapon = cheat::main::local()->get_weapon();

			static float next_update_time = Source::m_pGlobalVars->interval_per_tick;

			if (weapon != nullptr) {
				static int old_activity = weapon->m_Activity();
				const auto tickbase = player->m_nTickBase() - 1;
				auto activity = weapon->m_Activity();

				if (weapon->m_iItemDefinitionIndex() == 64 && !ucmd->hasbeenpredicted) {

					if (old_activity != activity && weapon->m_Activity() == 208)
						old_tickbase = tickbase + 2;

					if (weapon->m_Activity() == 208 && old_tickbase == tickbase)
						weapon->m_flPostponeFireReadyTime() = TICKS_TO_TIME(tickbase) + 0.2f;
				}

				old_activity = activity;
			}
		}

		if (Source::m_pMoveHelper != moveHelper)
			Source::m_pMoveHelper = moveHelper;
	}

	bool __stdcall InPrediction()
	{
		using Fn =  bool(__thiscall*)(void*);

		//static auto CalC_BasePlayerView = (void*)Memory::Scan("client.dll", "84 C0 75 08 57 8B CE E8 ? ? ? ? 8B 06");

		//if (_ReturnAddress() == CalC_BasePlayerView)
		//{
		//	for (int i = 1; i < 65; i++)
		//	{
		//		auto ent = (C_BasePlayer*)Source::m_pEntList->GetClientEntity(i);

		//		if (!ent)
		//			continue;

		//		// if ( ent == ctx.m_local && !csgo.m_input( )->m_fCameraInThirdPerson )
		//		// continue;

		//		//by setting last bone setup time to float NaN here, all setupbones calls (even from game) will break
		//		//only way to setupbones then is to invalidate bone cache before running setupbones
		//		//this boosts performance but also make visuals a bit more polished as u dont see any fighting happening between cheat and game

		//		if (!ent->IsDormant() && !ent->IsDead())
		//			ent->break_setup_bones();
		//		else if (ent->setup_bones_broken())
		//			ent->force_bone_rebuild();
		//	}
		//}
		static auto MST = (void*)Memory::Scan(cheat::main::clientdll, "84 C0 74 17 8B 87");

		if (_ReturnAddress() == MST) {
			return true;
		}

		if (Source::m_pPredictionSwap)
			return Source::m_pPredictionSwap->VCall<Fn>(Index::IPrediction::InPrediction)(Source::m_pPrediction);
	}

}