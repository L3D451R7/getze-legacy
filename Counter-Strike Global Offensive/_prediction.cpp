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

	int old_tickbase = 0;

	void __fastcall RunCommand(void* ecx, void* edx, C_BasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper)
	{
		using Fn = void(__thiscall*)(void*, C_BasePlayer*, CUserCmd*, IMoveHelper*);

		if (!player || player->entindex() != Source::m_pEngine->GetLocalPlayer())
			return Source::m_pPredictionSwap->VCall<Fn>(Index::IPrediction::RunCommand)(ecx, player, ucmd, moveHelper);;

		Engine::Prediction::Instance()->RestoreNetvars(ucmd->command_number - 1);

		FixViewmodel(ucmd, true);
		Source::m_pPredictionSwap->VCall<Fn>(Index::IPrediction::RunCommand)(ecx, player, ucmd, moveHelper);
		FixViewmodel(ucmd, false);

		Engine::Prediction::Instance()->OnRunCommand(player, ucmd->command_number);

		auto weapon = cheat::main::local()->get_weapon();
		static float next_update_time = Source::m_pGlobalVars->interval_per_tick;

		if (weapon != nullptr) 
		{
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

		if (Source::m_pPredictionSwap)
			return Source::m_pPredictionSwap->VCall<Fn>(Index::IPrediction::InPrediction)(Source::m_pPrediction);
	}

}