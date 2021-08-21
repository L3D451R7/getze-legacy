#include "prediction.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "source.hpp"
#include "md5_shit.h"
#include "lag_compensation.hpp"

namespace Engine
{

	void Prediction::Begin(CUserCmd* cmd)
	{
		if (C_CSPlayer::GetLocalPlayer())
			m_nTickBase = C_CSPlayer::GetLocalPlayer()->m_nTickBase();

		//static auto host_frameticks = (int*)(Memory::Scan("engine.dll", "03 05 ? ? ? ? 83 CF 10") + 2);
		//if (*host_frameticks > 1) {
			auto delta_tick = Source::m_pClientState->m_iDeltaTick;
			if (delta_tick > 0)
				Source::m_pPrediction->Update(delta_tick, delta_tick > 0, Source::m_pClientState->m_iLastCommandAck,
					Source::m_pClientState->m_iLastOutgoingCommand + Source::m_pClientState->m_iChockedCommands);
		//}

		m_pCmd = cmd;

		if (!m_pCmd)
			return;

		m_pPlayer = C_CSPlayer::GetLocalPlayer();

		if (!m_pPlayer || m_pPlayer->IsDead())
			return;

		m_pWeapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(m_pPlayer->m_hActiveWeapon()));

		if (!m_pWeapon)
			return;

		m_fFlags = m_pPlayer->m_fFlags();
		m_vecVelocity = m_pPlayer->m_vecVelocity();

		m_flCurrentTime = Source::m_pGlobalVars->curtime;
		m_flFrameTime = Source::m_pGlobalVars->frametime;

		//auto tickbase = m_pPlayer->m_nTickBase();

		cmd->random_seed = MD5_PseudoRandom(cmd->command_number) & 0x7fffffff;

		m_pPlayer->SetCurrentCommand(cmd);

		C_BaseEntity::SetPredictionRandomSeed(cmd);
		C_BaseEntity::SetPredictionPlayer(m_pPlayer);

		Source::m_pGlobalVars->curtime = float(m_pPlayer->m_nTickBase()) * Source::m_pGlobalVars->interval_per_tick;
		Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;

		CMoveData move = { };
		memset(&move, 0, sizeof(move));

		Source::m_pMoveHelper->SetHost(m_pPlayer);
		Source::m_pGameMovement->StartTrackPredictionErrors(m_pPlayer);
		Source::m_pPrediction->SetupMove(m_pPlayer, cmd, Source::m_pMoveHelper, &move);
		Source::m_pGameMovement->ProcessMovement(m_pPlayer, &move);
		Source::m_pPrediction->FinishMove(m_pPlayer, cmd, &move);

		//m_pPlayer->m_nTickBase() = tickbase;
	}

	void Prediction::End()
	{
		if (!m_pCmd || !m_pPlayer || !cheat::main::local || !m_pWeapon)
			return;

		Source::m_pGameMovement->StartTrackPredictionErrors(m_pPlayer);

		Source::m_pGlobalVars->curtime = m_flCurrentTime;
		Source::m_pGlobalVars->frametime = m_flFrameTime;

		C_BaseEntity::SetPredictionRandomSeed(nullptr);

		Source::m_pMoveHelper->SetHost(nullptr);
		C_BaseEntity::SetPredictionPlayer(nullptr);

		m_pPlayer->SetCurrentCommand(nullptr);
		m_pPrevCmd = m_pCmd;
	}

	int Prediction::GetFlags()
	{
		return m_fFlags;
	}

	CUserCmd Prediction::GetPrevCmd()
	{
		return *m_pPrevCmd;
	}

	Vector Prediction::GetVelocity()
	{
		return m_vecVelocity;
	}

	void Prediction::OnFrameStageNotify(ClientFrameStage_t stage)
	{
		if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
			return;

		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local)
			return;

		auto slot = local->m_nTickBase();
		auto data = &m_Data[slot % 128];

		local->m_aimPunchAngle() = data->m_aimPunchAngle;
		local->m_vecBaseVelocity() = data->m_vecBaseVelocity;
		local->m_flFallVelocity() = data->m_flFallVelocity;
	}

	void Prediction::OnRunCommand(C_BasePlayer* player)
	{
		static auto prev_tickbase = 0;
		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local || local != player)
			return;

		auto slot = local->m_nTickBase();
		auto data = &m_Data[slot % 128];

		/*auto tickbase = local->m_nTickBase() - 1;
		auto time = TICKS_TO_TIME(tickbase);
		auto weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local->m_hActiveWeapon()));

		if (weapon != nullptr && Source::m_pEngine->GetNetChannelInfo()) {
			static int old_activity = weapon->m_Activity();
			auto activity = weapon->m_Activity();

			if (weapon->m_iItemDefinitionIndex() == 64)
			{
				if (old_activity != activity && weapon->m_Activity() == 208 && prev_tickbase == tickbase) {
					weapon->m_flPostponeFireReadyTime() = time + 0.2f - Source::m_pEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
				}
			}

			old_activity = activity;
		}*/

		data->m_aimPunchAngle = local->m_aimPunchAngle();
		data->m_vecBaseVelocity = local->m_vecBaseVelocity();
		data->m_flFallVelocity = local->m_flFallVelocity();
		prev_tickbase = local->m_nTickBase();
	}
}