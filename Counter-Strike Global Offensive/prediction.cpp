#include "prediction.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "source.hpp"
#include "md5_shit.h"
#include "lag_compensation.hpp"
#include "displacement.hpp"
#include "movement.hpp"

namespace Engine
{

	void Prediction::Begin(CUserCmd* cmd)
	{
		if (C_CSPlayer::GetLocalPlayer())
			m_nTickBase = C_CSPlayer::GetLocalPlayer()->m_nTickBase();

		RestoreNetvars();

		//static auto host_frameticks = (int*)(Memory::Scan("engine.dll", "03 05 ? ? ? ? 83 CF 10") + 2);
		//if (*host_frameticks > 1) {
		//	auto delta_tick = Source::m_pClientState->m_iDeltaTick;
		//	if (delta_tick > 0)
		//		Source::m_pPrediction->Update(delta_tick, delta_tick > 0, Source::m_pClientState->m_iLastCommandAck,
		//			Source::m_pClientState->m_iLastOutgoingCommand + Source::m_pClientState->m_iChockedCommands);
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

		m_nServerCommandsAcknowledged = *(int*)(uintptr_t(Source::m_pPrediction) + 0x20);
		m_bInPrediction = *(bool*)(uintptr_t(Source::m_pPrediction) + 8);

		*(int*)(uintptr_t(Source::m_pPrediction) + 0x20) = 0;
		*(bool*)(uintptr_t(Source::m_pPrediction) + 8) = true;

		const auto randomseed = *(int*)(Engine::Displacement::Data::m_uPredictionRandomSeed);
		const auto ucmd = *(CUserCmd * *)(uintptr_t(m_pPlayer) + Engine::Displacement::C_BasePlayer::m_pCurrentCommand);
		const auto predplayer = *(C_BasePlayer * *)(Engine::Displacement::Data::m_uPredictionPlayer);

		//auto tickbase = m_pPlayer->m_nTickBase();

		cmd->random_seed = MD5_PseudoRandom(cmd->command_number) & 0x7fffffff;

		Source::m_pGlobalVars->curtime = TICKS_TO_TIME(m_pPlayer->m_nTickBase());
		Source::m_pGlobalVars->frametime = *(bool*)(uintptr_t(Source::m_pPrediction) + 0xA)/*m_bEnginePaused*/ ? 0.f : Source::m_pGlobalVars->interval_per_tick;

		CMoveData move = { };
		memset(&move, 0, sizeof(move));

		Source::m_pMoveHelper->SetHost(m_pPlayer);

		m_pPlayer->SetCurrentCommand(cmd);
		C_BaseEntity::SetPredictionRandomSeed(cmd);
		C_BaseEntity::SetPredictionPlayer(m_pPlayer);

		//Source::m_pGameMovement->StartTrackPredictionErrors(m_pPlayer);
		Source::m_pPrediction->SetupMove(m_pPlayer, cmd, Source::m_pMoveHelper, &move);
		Source::m_pGameMovement->ProcessMovement(m_pPlayer, &move);
		Source::m_pPrediction->FinishMove(m_pPlayer, cmd, &move);
		Source::m_pMoveHelper->SetHost(nullptr);

		*(int*)(Engine::Displacement::Data::m_uPredictionRandomSeed) = randomseed;
		C_BaseEntity::SetPredictionPlayer(predplayer);
		m_pPlayer->SetCurrentCommand(ucmd);

		*(int*)(uintptr_t(Source::m_pPrediction) + 0x20) = m_nServerCommandsAcknowledged;
		*(bool*)(uintptr_t(Source::m_pPrediction) + 8) = m_bInPrediction;

		if (m_pWeapon)
			m_pWeapon->UpdateAccuracyPenalty();

		if (m_pPlayer->m_MoveType() == MOVETYPE_WALK && m_pPlayer->get_sec_activity(m_pPlayer->get_animation_layer(3).m_nSequence) == 979)
		{
			m_pPlayer->get_animation_layer(3).m_flWeight = 0.f;
			m_pPlayer->get_animation_layer(3).m_flCycle = 0.f;
		}

		/*m_pPlayer->set_abs_angles(QAngle(0, Engine::Movement::Instance()->m_qRealAngles.y, 0));
		matrix3x4_t mx[128];
		memcpy(mx, m_pPlayer->m_CachedBoneData().Base(), m_pPlayer->GetBoneCount() * sizeof(matrix3x4_t));

		const auto oldposeparam = *(float*)(uintptr_t(m_pPlayer) + (Engine::Displacement::DT_CSPlayer::m_flPoseParameter + 48));
		*(DWORD*)(uintptr_t(m_pPlayer) + (Engine::Displacement::DT_CSPlayer::m_flPoseParameter + 48)) = 0x3F000000;
		m_pPlayer->force_bone_rebuild();
		m_pPlayer->SetupBonesEx();
		*(float*)(uintptr_t(m_pPlayer) + (Engine::Displacement::DT_CSPlayer::m_flPoseParameter + 48)) = oldposeparam;
		memcpy(cheat::main::zero_matrix, m_pPlayer->m_CachedBoneData().Base(), m_pPlayer->GetBoneCount() * sizeof(matrix3x4_t));*/

		m_pPlayer->GetEyePosition(); //call weapon_shootpos

		//memcpy(m_pPlayer->m_CachedBoneData().Base(), mx, m_pPlayer->GetBoneCount() * sizeof(matrix3x4_t));

	}

	void Prediction::End()
	{
		if (!m_pCmd || !m_pPlayer || !cheat::main::local() || !m_pWeapon)
			return;

		Source::m_pGlobalVars->curtime = m_flCurrentTime;
		Source::m_pGlobalVars->frametime = m_flFrameTime;
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

	void Prediction::RestoreNetvars(bool prev)
	{
		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local)
			return;

		auto slot = local->m_nTickBase() - (prev ? 1 : 0);
		auto data = &m_Data[slot % 150];

		if (!data->filled)
			return;

		const auto aim_punch_vel_diff = data->m_aimPunchAngleVel - local->m_aimPunchAngleVel();
		const auto aim_punch_diff = data->m_aimPunchAngle - local->m_aimPunchAngle();
		const auto vel_diff = data->m_vecVelocity - local->m_vecVelocity();

		if (fabs(aim_punch_vel_diff.x) <= 0.03125f && fabs(aim_punch_vel_diff.y) <= 0.03125f && fabs(aim_punch_vel_diff.z) <= 0.03125f)
			local->m_aimPunchAngleVel() = data->m_aimPunchAngleVel;

		if (fabs(aim_punch_diff.x) <= 0.03125f && fabs(aim_punch_diff.y) <= 0.03125f && fabs(aim_punch_diff.z) <= 0.03125f)
			local->m_aimPunchAngle() = data->m_aimPunchAngle;

		//if (fabs(vel_diff.x) <= 0.03125f && fabs(vel_diff.y) <= 0.03125f && fabs(vel_diff.z) <= 0.03125f)
		//	local->m_vecVelocity() = data->m_vecVelocity;

		if (fabs(local->m_vecViewOffset().z - data->m_vecViewOffset.z) <= 0.03125f)
			local->m_vecViewOffset().z = fminf(fmaxf(data->m_vecViewOffset.z, 46.0f), 64.0f);
	}

	void Prediction::OnRunCommand(C_BasePlayer* player)
	{
		//static auto prev_tickbase = 0;
		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local || local != player)
			return;

		auto slot = local->m_nTickBase();
		auto data = &m_Data[slot % 150];

		data->m_aimPunchAngle = local->m_aimPunchAngle();
		data->m_vecViewOffset = local->m_vecViewOffset();
		data->m_vecVelocity = local->m_vecVelocity();
		data->m_aimPunchAngleVel = local->m_aimPunchAngleVel();

		data->filled = true;
		//prev_tickbase = local->m_nTickBase();
	}
}