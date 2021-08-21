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

		m_bFirstTimePredicted = Source::m_pPrediction->m_bFirstTimePredicted;
		m_bInPrediction = Source::m_pPrediction->m_bInPrediction;

		Source::m_pPrediction->m_bFirstTimePredicted = 0;
		Source::m_pPrediction->m_bInPrediction = true;

		const auto randomseed = *(int*)(Engine::Displacement::Data::m_uPredictionRandomSeed);
		const auto ucmd = *(CUserCmd * *)(uintptr_t(m_pPlayer) + Engine::Displacement::C_BasePlayer::m_pCurrentCommand);
		const auto predplayer = *(C_BasePlayer * *)(Engine::Displacement::Data::m_uPredictionPlayer);

		//auto tickbase = m_pPlayer->m_nTickBase();

		cmd->random_seed = MD5_PseudoRandom(cmd->command_number) & 0x7fffffff;

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

		Source::m_pPrediction->m_bFirstTimePredicted = m_bFirstTimePredicted;
		Source::m_pPrediction->m_bInPrediction = m_bInPrediction;

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

	void Prediction::RestoreNetvars(int cmdnr)
	{
		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local)
			return;

		auto data = &m_Data[cmdnr % 150];

		if (!data->filled || cmdnr != data->command_number)
			return;

		const auto aim_punch_vel_diff = data->m_aimPunchAngleVel - local->m_aimPunchAngleVel();
		const auto aim_punch_diff = data->m_aimPunchAngle - local->m_aimPunchAngle();
		const auto vel_diff = data->m_vecVelocity - local->m_vecVelocity();

		if (fabs(aim_punch_vel_diff.x) <= 0.03125f && fabs(aim_punch_vel_diff.y) <= 0.03125f && fabs(aim_punch_vel_diff.z) <= 0.03125f)
			local->m_aimPunchAngleVel() = data->m_aimPunchAngleVel;

		if (fabs(aim_punch_diff.x) <= 0.03125f && fabs(aim_punch_diff.y) <= 0.03125f && fabs(aim_punch_diff.z) <= 0.03125f)
			local->m_aimPunchAngle() = data->m_aimPunchAngle;

		if (fabs(local->m_vecViewOffset().z - data->m_vecViewOffset.z) <= 0.125f)
			local->m_vecViewOffset().z = fminf(fmaxf(data->m_vecViewOffset.z, 46.0f), 64.0f);
	}

	void Prediction::DetectPredError()
	{
		if (m_data->m_command_number != m_tick || !m_data->is_filled)
			return;

		uintptr_t player = *g_local_ptr;

		auto& m_aimPunchAngle = *(Vector*)(player + m_aimPunchAngleO);
		auto& m_aimPunchAngleVel = *(Vector*)(player + m_aimPunchAngleVelO);
		auto& m_viewPunchAngle = *(Vector*)(player + m_viewPunchAngleO);
		auto& m_vecViewOffset = *(Vector*)(player + m_vecViewOffsetO);
		auto& m_vecOrigin = *(Vector*)(player + m_vecOriginO);
		auto& m_flVelocityModifier = *(float*)(player + m_flVelocityModifierO);
		auto& m_bWaitForNoAttack = *(char*)(player + m_bWaitForNoAttackO);

		m_data->m_bWaitForNoAttack = m_bWaitForNoAttack;

		auto viewPunch_delta = std::fabsf(m_viewPunchAngle.x - m_data->m_viewPunchAngle.x);

		if (viewPunch_delta <= 0.03125f)
			m_viewPunchAngle.x = m_data->m_viewPunchAngle.x;

		int v34 = 1;
		int v35 = 1;
		int repredict = 0;
		int v37 = 1;

		if (std::abs(m_aimPunchAngle.x - m_data->m_aimPunchAngle.x) > 0.03125f
			|| std::abs(m_aimPunchAngle.y - m_data->m_aimPunchAngle.y) > 0.03125f
			|| std::abs(m_aimPunchAngle.z - m_data->m_aimPunchAngle.z) > 0.03125f)
		{
			v34 = 0;
		}

		if (v34)
			m_aimPunchAngle = m_data->m_aimPunchAngle;
		else {
			//csgo.m_engine()->OnPredError();
			m_data->m_aimPunchAngle = m_aimPunchAngle;
			//feature::anti_aim->force_unchoke = 1;
			repredict = 1;
		}

		if (std::abs(m_aimPunchAngleVel.x - m_data->m_aimPunchAngleVel.x) > 0.03125f
			|| std::abs(m_aimPunchAngleVel.y - m_data->m_aimPunchAngleVel.y) > 0.03125f
			|| std::abs(m_aimPunchAngleVel.z - m_data->m_aimPunchAngleVel.z) > 0.03125f)
		{
			v35 = 0;
		}

		if (v35)
			m_aimPunchAngleVel = m_data->m_aimPunchAngleVel;
		else {
			m_data->m_aimPunchAngleVel = m_aimPunchAngleVel;
			repredict = 1;
		}

		auto v28 = std::abs(m_vecViewOffset.z - m_data->m_vecViewOffset.z);
		if (v28 > 0.125f)
		{
			m_data->m_vecViewOffset.z = m_vecViewOffset.z;
			repredict = 1;
		}
		else
			m_vecViewOffset.z = m_data->m_vecViewOffset.z;

		if ((m_vecOrigin - m_data->m_vecOrigin).LengthSquared() >= 1.0f)
		{
			m_data->m_vecOrigin = m_vecOrigin;
			repredict = 1;
		}

		if (repredict)
		{
			prediction->m_nPreviousStartFrame = -1;
			prediction->m_nCommandsPredicted = 0;
			prediction->m_bPreviousAckHadErrors = 1;
		}
	}

	void Prediction::OnRunCommand(C_BasePlayer* player, int cmdnr)
	{
		//static auto prev_tickbase = 0;
		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local || local != player)
			return;

		auto data = &m_Data[cmdnr % 150];

		data->m_aimPunchAngle = local->m_aimPunchAngle();
		data->m_vecViewOffset = local->m_vecViewOffset();
		data->m_vecVelocity = local->m_vecVelocity();
		data->m_aimPunchAngleVel = local->m_aimPunchAngleVel();
		data->command_number = cmdnr;

		data->filled = true;
		//prev_tickbase = local->m_nTickBase();
	}
}