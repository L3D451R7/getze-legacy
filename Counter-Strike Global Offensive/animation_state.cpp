#include "aimbot.hpp"
#include "source.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "hooked.hpp"
#include "math.hpp"
#include "displacement.hpp"
#include "angle_resolver.hpp"
#include "lag_compensation.hpp"
#include "anti_aimbot.hpp"
#include <algorithm>
#include <numeric>
#include "rmenu.hpp"
#include "animation_state.hpp"

void c_player_animstate_rebuilt::setup_velocity(CCSGOPlayerAnimState *state, C_BasePlayer* player)
{
	// [COLLAPSED LOCAL DECLARATIONS. PRESS KEYPAD CTRL-"+" TO EXPAND]

	auto entindex = player->entindex() - 1;

	//if (!(player->m_iEFlags() & 0x1000))
		//CBaseEntity::CalcAbsoluteVelocity((float *)player);

	auto v7 = state->anim_update_delta * 2000.0;
	auto velocity = state->velocity;
	state->speed_2d = velocity.z;
	velocity.z = 0.0f;

	/*float leanspd = m_vecLastSetupLeanVelocity.LengthSqr();

	m_bIsAccelerating = velocity.Length2D() > leanspd;*/

	auto weapon = player->get_weapon();

	float flMaxMovementSpeed = 260.0f;
	if (weapon)
		flMaxMovementSpeed = std::fmax(weapon->GetMaxWeaponSpeed(), 0.001f);

	state->speed_2d = std::fmin(state->velocity.Length(), 260.0f);
	state->speed_norm = Math::clamp(state->speed_2d / flMaxMovementSpeed, 0.0f, 1.0f);
	state->feet_shit = state->speed_2d / (flMaxMovementSpeed * 0.520f);
	state->feet_speed = state->speed_2d / (flMaxMovementSpeed * 0.340f);

	auto m_flVelocityUnknown = *(float*)(uintptr_t(state) + 0x2A4);

	if (state->feet_shit < 1.0f)
	{
		if (state->feet_shit < 0.5f)
		{
			float vel = m_flVelocityUnknown;
			float delta = state->anim_update_delta * 60.0f;
			float newvel;
			if ((80.0f - vel) <= delta)
			{
				if (-delta <= (80.0f - vel))
					newvel = 80.0f;
				else
					newvel = vel - delta;
			}
			else
			{
				newvel = vel + delta;
			}
			m_flVelocityUnknown = newvel;
		}
	}
	else
		m_flVelocityUnknown = state->speed_2d;

	bool bWasMovingLastUpdate = false;
	bool bJustStartedMovingLastUpdate = false;
	if (state->speed_2d <= 0.0f)
	{
		state->t_since_started_moving = 0.0f;
		bWasMovingLastUpdate = state->t_since_stopped_moving <= 0.0f;
		state->t_since_stopped_moving += state->anim_update_delta;
	}
	else
	{
		state->t_since_stopped_moving = 0.0f;
		bJustStartedMovingLastUpdate = state->t_since_started_moving <= 0.0f;
		state->t_since_started_moving = state->anim_update_delta + state->t_since_started_moving;
	}

	/*if (!*(_BYTE *)(state + 0x130)
		&& v27
		&& *(_BYTE *)(state + 0xF8)
		&& !*(_BYTE *)(state + 0x120)
		&& !*(_BYTE *)(state + 0x101)
		&& *(float *)(state + 0x2A8) < 50.0)
	{
		v30 = CCSGOPlayerAnimState::SelectSequenceFromActMods((_DWORD *)state, 980);
		CCSGOPlayerAnimState::SetLayerSequence(state, a3, 3, v30);
		*(_BYTE *)(state + 304) = 1;
	}

	if (CCSGOPlayerAnimState::GetLayerActivity((_DWORD *)state, 3) == 980
		|| CCSGOPlayerAnimState::GetLayerActivity((_DWORD *)state, 3) == 979)
	{
		if (*(_BYTE *)(state + 304) && *(float *)(state + 236) <= 0.25)
		{
			v31 = 3;
			v32 = *(_DWORD *)(*(_DWORD *)(state + 80) + 1248);
			if (v32 - 1 < 3)
				v31 = v32 - 1;
			if (v32 && (v33 = *(_DWORD *)(*(_DWORD *)(v4 + 80) + 1236) + 92 * v31) != 0)
				v34 = *(float *)(v33 + 24);
			else
				v34 = 0.0;
			v35 = v34;
			CCSGOPlayerAnimState::GetLayerWeight(v4, 3, 0);
			v36 = CCSGOPlayerAnimState::IsLayerSequenceCompleted((_DWORD *)v4, 3);
			CCSGOPlayerAnimState::SetLayerWeight(v4, *(float *)&v36, 3);
			CCSGOPlayerAnimState::SetLayerWeightRate(v4, v35, 3);
			v37 = 3;
			v38 = *(_DWORD *)(*(_DWORD *)(v4 + 80) + 1248);
			if (v38 - 1 < 3)
				v37 = v38 - 1;
			if (v38 && (v39 = *(_DWORD *)(*(_DWORD *)(v4 + 80) + 1236) + 92 * v37) != 0)
				*(_BYTE *)(v4 + 304) = (float)((float)(*(float *)(v39 + 16) * *(float *)(v4 + 100)) + *(float *)(v39 + 12)) < 1.0;
			else
				*(_BYTE *)(v4 + 304) = 1;
		}
		else
		{
			v40 = *(_DWORD **)(state + 80);
			v41 = 3;
			*(_BYTE *)(state + 304) = 0;
			v42 = v40[312];
			if (v42 - 1 < 3)
				v41 = v42 - 1;
			if (v42 && (v43 = *(_DWORD *)(*(_DWORD *)(v4 + 80) + 1236) + 92 * v41) != 0)
				v44 = *(float *)(v43 + 24);
			else
				v44 = 0.0;
			v45 = *(float *)(v4 + 100) * 5.0;
			if (COERCE_FLOAT(LODWORD(v44) ^ xmmword_108CF530) <= v45)
			{
				if (COERCE_FLOAT(LODWORD(v45) ^ xmmword_108CF530) <= COERCE_FLOAT(LODWORD(v44) ^ xmmword_108CF530))
					v46 = 0.0;
				else
					v46 = v44 - v45;
			}
			else
			{
				v46 = v45 + v44;
			}
			CCSGOPlayerAnimState::SetLayerWeight(state, v46, 3);
			CCSGOPlayerAnimState::SetLayerWeightRate(state, v44, 3);
		}
	}*/

	state->old_abs_yaw = state->abs_yaw;

	auto v47 = Math::clamp(state->abs_yaw, -360.0f, 360.0f);
	auto v49 = Math::normalize_angle(Math::AngleDiff(state->eye_yaw, v47));

	if (state->feet_speed >= 0.0)
		state->feet_speed = fminf(state->feet_speed, 1.0);
	else
		state->feet_speed = 0.0;

	auto v54 = state->duck_amt;
	auto v55 = ((((*(float*)((uintptr_t)state + 0x11C)) * -0.30000001) - 0.19999999) * state->feet_speed) + 1.0f;
	if (v54 > 0.0)
	{
		if (state->feet_shit >= 0.0)
			state->feet_shit = fminf(state->feet_shit, 1.0);
		else
			state->feet_shit = 0.0;

		v55 += ((state->feet_shit * v54) * (0.5f - v55));
	}

	auto v58 = *(float*)((uintptr_t)state + 0x334) * v55;
	auto v59 = *(float*)((uintptr_t)state + 0x330) * v55;

	if (v49 <= v58)
	{
		if (v59 > v49)
			state->abs_yaw = fabs(v59) + state->eye_yaw;
	}
	else
		state->abs_yaw = state->eye_yaw - fabs(v58);
	
	state->abs_yaw = Math::normalize_angle(state->abs_yaw);

	if (state->speed_2d > 0.1 || fabs(state->speed_up) > 100.0)
	{
		state->abs_yaw = cheat::features::antiaimbot.some_func(
			state->eye_yaw,
			state->abs_yaw,
			(((*(float*)((uintptr_t)state + 0x11C)) * 20.0f) + 30.0f)
			* state->anim_update_delta);

		last_time_lby_updated[entindex] = Source::m_pGlobalVars->curtime + 0.22000001;
		/*if (lastlby != state->m_flEyeYaw)
		{
			v67 = (int)(v134 - 2535);
			if (v67->something)
			{
				*(_BYTE *)(v67 + 92) |= 1u;
				*v134 = state->m_flEyeYaw;
			}
			else
			{
				if (v67->send_updates)
				{
					NetworkStateChanged(v67->lby_prop, 10140);
				}
				v66 = state->m_flEyeYaw;
			}
		}*/
		player->m_flLowerBodyYawTarget() = state->eye_yaw;
	}
	else
	{
		state->abs_yaw = cheat::features::antiaimbot.some_func(
			player->m_flLowerBodyYawTarget(),
			state->abs_yaw,
			state->anim_update_delta * 100.0f);

		if (Source::m_pGlobalVars->curtime > last_time_lby_updated[entindex])
		{
			auto v60 = Math::AngleDiff(player->m_flLowerBodyYawTarget(), state->eye_yaw);

			if (abs(v60) > 35.0)
			{
				last_time_lby_updated[entindex] = Source::m_pGlobalVars->curtime + 1.1;
				/*if (lastlby != state->m_flEyeYaw)
				{
					v67 = (int)(v134 - 2535);
					if (v67->something)
					{
						*(_BYTE *)(v67 + 92) |= 1u;
						*v134 = state->m_flEyeYaw;
					}
					else
					{
						if (v67->send_updates)
						{
							NetworkStateChanged(v67->lby_prop, 10140);
						}
						v66 = state->m_flEyeYaw;
					}
				}*/
				player->m_flLowerBodyYawTarget() = state->eye_yaw;
			}
		}
	}

	if (state->speed_2d <= 1.0
		&& state->on_ground
		&& state->anim_update_delta > 0.0
		&& (Math::AngleDiff(state->old_abs_yaw, state->abs_yaw) / state->anim_update_delta) > 120.0)
	{
		/*v76 = CCSGOPlayerAnimState::SelectSequenceFromActMods(v4, 979);
		CCSGOPlayerAnimState::SetLayerSequence((int)v4, v60, 3, v76);
		LOBYTE(v4[1].N01779501) = 1;*/
		balance_adjust[entindex] = true;
	}
	else
		balance_adjust[entindex] = false;

	if (state->speed_2d > 0.0)
	{
		float velAngle = (atan2(-state->velocity.y, -state->velocity.x) * 180.0f) * (1.0f / M_PI);

		if (velAngle < 0.0f)
			velAngle += 360.0f;

		state->vel_lean = Math::normalize_angle(Math::AngleDiff(velAngle, state->abs_yaw));
	}

	state->lean = Math::normalize_angle(Math::AngleDiff(state->vel_lean, state->torso_yaw));

	if (bJustStartedMovingLastUpdate && state->feet_rate <= 0.0)
	{
		state->torso_yaw = state->vel_lean;
		// too many memecode below, i did not even want to paste it there.
	}
	else
	{
		if (state->speed_2d > 0.1f)
		{
			state->torso_yaw = state->vel_lean;
		}
		else
		{
			if (state->feet_shit >= 0.0)
				state->feet_shit = fminf(state->feet_shit, 1.0);
			else
				state->feet_shit = 0.0;

			if (state->feet_speed >= 0.0)
				state->feet_speed = fminf(state->feet_speed, 1.0);
			else
				state->feet_speed = 0.0;

			//dword_10ADA7F8 = 1075729671;

			auto v105 = ((state->feet_shit - state->feet_speed) * state->duck_amt) + state->feet_speed;
			auto v156 = Math::normalize_angle((((v105 + 0.1f) * state->vel_lean) + state->torso_yaw));

			state->torso_yaw = v156;
		}
	}

	//float eye_pitch_normalized = Math::normalize_angle(state->m_flPitch);
	//float new_body_pitch_pose;

	//if (eye_pitch_normalized <= 0.0f)
	//	new_body_pitch_pose = (eye_pitch_normalized / m_flMaximumPitch) * -90.0f;
	//else
	//	new_body_pitch_pose = (eye_pitch_normalized / m_flMinimumPitch) * 90.0f;

	//m_arrPoseParameters[7].SetValue(pBaseEntity, new_body_pitch_pose);

	float eye_goalfeet_delta = Math::AngleDiff(state->eye_yaw - state->abs_yaw, 360.0f);
	float new_body_yaw_pose = 0.0f; //not initialized?

	if (eye_goalfeet_delta < 0.0f || v58 == 0.0f)
	{
		if (v59 != 0.0f)
			new_body_yaw_pose = (eye_goalfeet_delta / v59) * -58.0f;
	}
	else
	{
		new_body_yaw_pose = (eye_goalfeet_delta / v58) * 58.0f;
	}

	player->m_flPoseParameter()[11] = (new_body_yaw_pose * 0.00833333f) + 0.5f; // body yaw

	// move_yaw
	float move_yaw = (player->m_flPoseParameter()[7] * 360.0f - 180.0f) - (state->abs_yaw - state->old_abs_yaw);
	if (move_yaw <= 180.0f) {
		if (move_yaw < -180.0f)
			move_yaw = move_yaw + 360.0f;
	}
	else
		move_yaw = move_yaw - 360.0f;
	
	player->m_flPoseParameter()[7] = (move_yaw / 120.f) + 0.5f;

	/*v107 = state->m_BaseEntity;
	v108 = state->m_flCurrentTorsoYaw;
	v147 = state->m_BaseEntity;
	v157 = (char *)&v4[1].m_flUnknown2;
	if (!LOBYTE(v4[1].m_flUnknown2))
	{
		sub_10466930(&v4[1].m_flUnknown2, v107, LODWORD(v4[1].m_vecUnknown.x));
		if (!LOBYTE(v4[1].m_flUnknown2))
			goto LABEL_195;
		v107 = v147;
	}
	if (v107)
	{
		v109 = mdlcache;
		(*(void(__thiscall **)(int))(*(_DWORD *)mdlcache + 132))(mdlcache);
		animstate_pose_param_cache_t::SetValue(*((_DWORD *)v157 + 1));
		(*(void(__thiscall **)(int))(*(_DWORD *)v109 + 136))(v109);
		v4 = v140;
	}

	v110 = state->m_flEyeYaw;
	v111 = v110 < state->m_flGoalFeetYaw;
	v112 = v110 == state->m_flGoalFeetYaw;
	v148 = fmod((float)(state->m_flEyeYaw - state->m_flGoalFeetYaw), 360.0);
	v113 = v148;
	if (v111 || v112)
	{
		if (v148 <= -180.0)
			v113 = v148 + 360.0;
	}
	else if (v148 >= 180.0)
	{
		v113 = v148 - 360.0;
	}
	if (v113 < 0.0 || v4[2].m_flDuckAmount == 0.0)
		*(float *)v4[2]._0x00A0;
	v114 = state->m_BaseEntity;
	v149 = state->m_BaseEntity;
	v158 = (char *)&v4[1].m_flUpSpeed;
	if (!LOBYTE(v4[1].m_flUpSpeed))
	{
		sub_10466930(&v4[1].m_flUpSpeed, v114, LODWORD(v4[1].m_flFeetSpeedForwardSideways));
		if (!LOBYTE(v4[1].m_flUpSpeed))
			goto LABEL_208;
		v114 = v149;
	}
	if (v114)
	{
		v115 = mdlcache;
		(*(void(__thiscall **)(int))(*(_DWORD *)mdlcache + 132))(mdlcache);
		animstate_pose_param_cache_t::SetValue(*((_DWORD *)v158 + 1));
		(*(void(__thiscall **)(int))(*(_DWORD *)v115 + 136))(v115);
		v4 = v140;
	}

	v116 = state->m_flPitch;
	v150 = fmod(state->m_flPitch, 360.0);
	v111 = v116 < 0.0;
	v112 = v116 == 0.0;
	v117 = v150;
	if (v111 || v112)
	{
		if (v150 <= -180.0)
			v117 = v150 + 360.0;
	}
	else if (v150 >= 180.0)
	{
		v117 = v150 - 360.0;
	}
	if (v117 <= 0.0)
		v119 = (float)(v117 / v4[2].m_flLandingDuckAdditive) * -90.0;
	else
		v118 = (float)(v117 / *(float *)v4[2]._0x00AC) * 90.0;
	v120 = state->m_BaseEntity;
	v139 = state->m_BaseEntity;
	v159 = (char *)&v4[1].m_flFeetSpeedUnknownForwardSideways;
	if (!LOBYTE(v4[1].m_flFeetSpeedUnknownForwardSideways))
	{
		sub_10466930(&v4[1].m_flFeetSpeedUnknownForwardSideways, v120, LODWORD(v4[1].m_flTimeSinceStoppedMoving));
		if (!LOBYTE(v4[1].m_flFeetSpeedUnknownForwardSideways))
			goto LABEL_221;
		v120 = v139;
	}
	if (v120)
	{
		v121 = mdlcache;
		(*(void(__thiscall **)(int))(*(_DWORD *)mdlcache + 132))(mdlcache);
		animstate_pose_param_cache_t::SetValue(*((_DWORD *)v159 + 1));
		(*(void(__thiscall **)(int))(*(_DWORD *)v121 + 136))(v121);
		v4 = v140;
	}

	v122 = state->m_BaseEntity;
	v123 = state->m_flFeetSpeedForwardSideways;
	v151 = state->m_BaseEntity;
	v160 = (char *)&v4[1].m_vecOrigin.y;
	if (!LOBYTE(v4[1].m_vecOrigin.y))
	{
		sub_10466930(&v4[1].m_vecOrigin.y, v122, LODWORD(v4[1].m_vecLastOrigin.x));
		if (!LOBYTE(v4[1].m_vecOrigin.y))
			goto LABEL_226;
		v122 = v151;
	}
	if (v122)
	{
		v124 = mdlcache;
		(*(void(__thiscall **)(int))(*(_DWORD *)mdlcache + 132))(mdlcache);
		animstate_pose_param_cache_t::SetValue(*((_DWORD *)v160 + 1));
		(*(void(__thiscall **)(int))(*(_DWORD *)v124 + 136))(v124);
		v4 = v140;
	}

	v125 = *(float *)v4[1]._0x000C * state->m_flDuckAmount;
	v126 = state->m_BaseEntity;
	v127 = (_DWORD *)&v4[1].m_flLastOriginZ;
	v152 = v126;
	v161 = v127;
	if (!*(_BYTE *)v127)
	{
		sub_10466930(v127, v126, v127[2]);
		if (!*(_BYTE *)v127)
			return (*(int(__thiscall **)(int))(*(_DWORD *)v3 + 136))(v3);
		v126 = v152;
	}
	if (v126)
	{
		v128 = mdlcache;
		(*(void(__thiscall **)(int))(*(_DWORD *)mdlcache + 132))(mdlcache);
		animstate_pose_param_cache_t::SetValue(v161[1]);
		(*(void(__thiscall **)(int))(*(_DWORD *)v128 + 136))(v128);
	}
	return (*(int(__thiscall **)(int))(*(_DWORD *)v3 + 136))(v3);*/
}