#include "animations.hpp"
#include "aimbot.hpp"
#include "source.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "hooked.hpp"
#include "math.hpp"
#include "displacement.hpp"
#include "autowall.hpp"
#include "angle_resolver.hpp"
#include "game_movement.h"
#include "anti_aimbot.hpp"
#include <unordered_map>
#include <algorithm>
#include "rmenu.hpp"
#include "lag_compensation.hpp"

void c_animsystem::run_velocity_calculation(C_BasePlayer* m_player, C_Animrecord* m_record, C_Animrecord* m_previous, c_anim_records* lag)
{
	auto resolver_info = &cheat::features::aaa.player_resolver_records[m_player->entindex()-1];

	m_record->anim_velocity.Set();
	Vector vecAnimVelocity = m_record->velocity;

	if (!m_previous)
	{
		if (m_record->layers[6].m_flPlaybackRate > 0.0f && vecAnimVelocity.Length() > 0.1f)
		{
			if (m_record->layers[6].m_flWeight > 0.0f) {
				auto v30 = m_player->GetMaxSpeed();

				if (m_record->entity_flags & 6)
					v30 = v30 * 0.34f;
				else if (m_player->m_bIsWalking())
					v30 = v30 * 0.52f;

				auto v35 = m_record->layers[6].m_flWeight * v30;
				vecAnimVelocity *= v35 / vecAnimVelocity.Length();
			}
			else
				vecAnimVelocity.Set();
		}
		else
			vecAnimVelocity.Set();

		if (m_record->entity_flags & 1)
			vecAnimVelocity.z = 0.f;

		m_record->velocity = vecAnimVelocity;
		m_record->anim_velocity = vecAnimVelocity;
		return;
	}

	// check if player has been on ground for two consecutive ticks
	bool bIsOnground = m_record->entity_flags & FL_ONGROUND && m_previous->entity_flags & FL_ONGROUND;

	const Vector vecOriginDelta = m_record->origin - m_previous->origin;

	//
	// entity teleported, reset his velocity (thats what server does)
	// - L3D451R7
	//
	if ((m_player->m_fEffects() & 8) != 0
		|| m_player->m_ubEFNoInterpParity() != m_player->m_ubEFNoInterpParityOld())
	{
		m_record->velocity.clear();
		m_record->anim_velocity.clear();
		return;
	}

	if (m_record->lag <= 1) 
	{
		m_record->velocity = vecAnimVelocity;
		m_record->anim_velocity = vecAnimVelocity;
		return;
	}

	// fix up velocity 
	auto origin_delta_lenght = vecOriginDelta.Length();

	if (m_record->lag > 0 && m_record->lag < 20 && origin_delta_lenght >= 1.f && origin_delta_lenght <= 1000000.0f) {
		vecAnimVelocity = vecOriginDelta / TICKS_TO_TIME(m_record->lag);
		vecAnimVelocity.ValidateVector();

		m_record->velocity = vecAnimVelocity;
	}

	float anim_speed = 0.f;

	// determine if we can calculate animation speed modifier using server anim overlays
	if (bIsOnground
		&& m_record->layers[11].m_flWeight > 0.0f
		&& m_record->layers[11].m_flWeight < 1.0f
		&& m_record->layers[11].m_flPlaybackRate == m_record->layers[11].m_flPlaybackRate)
	{
		// calculate animation speed yielded by anim overlays
		auto flAnimModifier = 0.35f * (1.0f - m_record->layers[11].m_flWeight);
		if (flAnimModifier > 0.0f && flAnimModifier < 1.0f)
			anim_speed = m_player->GetMaxSpeed() * (flAnimModifier + 0.55f);
	}

	// this velocity is valid ONLY IN ANIMFIX UPDATE TICK!!!
	// so don't store it to record as m_vecVelocity. i added vecAbsVelocity for that, it acts as a animationVelocity.
	// -L3D451R7
	if (anim_speed > 0.0f) {
		anim_speed /= vecAnimVelocity.Length2D();
		vecAnimVelocity.x *= anim_speed;
		vecAnimVelocity.y *= anim_speed;
	}
	//else
	//{
	//	// simulate player movement
	//	if (g_Vars.rage.fakelag_correction == 1) {
	//		SimulateMovement(player, m_record, m_previous);
	//	}
	//}

	// calculate average player direction when bunny hopping
	if (lag->anim_records.size() > 2 && vecAnimVelocity.Length() >= 400.f)
	{
		auto prepre_rec = &lag->anim_records[2];

		if (prepre_rec && prepre_rec->animated)
		{
			auto vecPreviousVelocity = (m_previous->origin - prepre_rec->origin) / TICKS_TO_TIME(m_previous->lag);

			// make sure to only calculate average player direction whenever they're bhopping
			if (!vecPreviousVelocity.IsZero() && !bIsOnground) {
				auto vecCurrentDirection = Math::normalize_angle(RAD2DEG(atan2(vecAnimVelocity.y, vecAnimVelocity.x)));
				auto vecPreviousDirection = Math::normalize_angle(RAD2DEG(atan2(vecPreviousVelocity.y, vecPreviousVelocity.x)));

				auto vecAvgDirection = vecCurrentDirection - vecPreviousDirection;
				vecAvgDirection = DEG2RAD(Math::normalize_angle(vecCurrentDirection + vecAvgDirection * 0.5f));

				auto vecDirectionCos = cos(vecAvgDirection);
				auto vecDirectionSin = sin(vecAvgDirection);

				// modify velocity accordingly
				vecAnimVelocity.x = vecDirectionCos * vecAnimVelocity.Length2D();
				vecAnimVelocity.y = vecDirectionSin * vecAnimVelocity.Length2D();
			}
		}
	}

	if (bIsOnground)
		vecAnimVelocity.z = 0.0f;

	// detect fakewalking players
	if (m_record->velocity.Length() >= 0.1f)
	{
		if (m_record->layers[4].m_flWeight == 0.0f
			&& m_record->layers[5].m_flWeight == 0.0f
			&& m_record->layers[6].m_flPlaybackRate == 0.0f
			&& m_record->entity_flags & FL_ONGROUND)
		{
			vecAnimVelocity.clear();
			resolver_info->fakewalk = true;
			//m_record->m_AnimationFlags |= ELagRecordFlags::RF_FakeWalking;
		}
	}

	vecAnimVelocity.ValidateVector();

	// assign fixed velocity to record velocity
	m_record->anim_velocity = vecAnimVelocity;
}

bool c_animsystem::update_animations_data(C_BasePlayer* m_player)
{
	CCSGOPlayerAnimState* state = m_player->get_animation_state();

	auto idx = m_player->entindex() - 1;

	if (!state || m_player == cheat::main::local())
		return false;

	auto weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(m_player->m_hActiveWeapon()));

	const auto flags_backup = m_player->m_fFlags();
	const auto i_e_flags_backup = m_player->m_iEFlags();
	const auto base_vel_backup = m_player->m_vecBaseVelocity();
	const auto abs_origin_backup = m_player->get_abs_origin();
	
	m_player->set_abs_origin(m_player->m_vecOrigin());
	m_player->m_angEyeAngles().x = Math::NormalizePitch(m_player->m_angEyeAngles().x);

	auto m_records = &records[idx].anim_records;
	auto resolver_info = &cheat::features::aaa.player_resolver_records[idx];

	auto front = &m_records->front();
	auto previous = m_records->size() > 1 && m_records->at(1).animated && m_records->at(1).data_filled ? &m_records->at(1) : nullptr;

	const auto curtime = Source::m_pGlobalVars->curtime;
	const auto frametime = Source::m_pGlobalVars->frametime;
	auto time_delta = m_player->m_flSimulationTime() - m_player->m_flOldSimulationTime();
	auto v75 = TIME_TO_TICKS(time_delta);

	if (v75 >= 20)
		Source::m_pGlobalVars->curtime = m_player->m_flSimulationTime();
	else
		Source::m_pGlobalVars->curtime = Source::m_pGlobalVars->interval_per_tick + m_player->m_flOldSimulationTime();

	Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;

	memcpy(resolver_info->server_anim_layers, m_player->animation_layers_ptr(), 0x38 * m_player->get_animation_layers_count());
	resolver_info->m_flRate = resolver_info->server_anim_layers[6].m_flPlaybackRate;

	resolver_info->fakewalk = false;
	front->shot_time = 0;

	if (!m_player->IsBot())
	{
		int simulation_ticks = m_player->m_flSimulationTime();
		int old_simulation_ticks = m_player->m_flOldSimulationTime();
		const auto m_lag_time = Math::clamp(m_player->m_flSimulationTime() - m_player->m_flOldSimulationTime(), Source::m_pGlobalVars->interval_per_tick, 1.0f);
		const auto m_lag = simulation_ticks - old_simulation_ticks;

		/*if (m_player->get_animation_layers_count() >= 3)
		{
			if (front->layers[3].m_flCycle == 0.0f && front->server_layers[3].m_flWeightDeltaRate != 0.0f
				|| m_player->get_sec_activity(front->layers[3].m_nSequence) == 979
				&& (previous->layers[3].m_nSequence != front->layers[3].m_nSequence
					|| previous->layers[3].m_flCycle > front->layers[3].m_flCycle))
			{
				player->m_PlayerAnimState()->m_bOnGround = 1;
				player->m_PlayerAnimState()->m_bLanding = 0;
				player->m_PlayerAnimState()->m_flVelocityLengthXY = 0;
				player->m_PlayerAnimState()->m_vecVelocity.Set();
			}
		}*/

		// set previous flags.
		if (m_lag > 1)
		{
			auto v490 = m_player->get_sec_activity(front->layers[5].m_nSequence);

			if (m_player->get_animation_layers_count() <= 5
				|| front->layers[5].m_nSequence == previous->layers[5].m_nSequence && (previous->layers[5].m_flWeight != 0.0f || front->layers[5].m_flWeight == 0.0f)
				|| !(v490 == ACT_CSGO_LAND_LIGHT || v490 == ACT_CSGO_LAND_HEAVY))
			{
				if ((front->entity_flags & 1) != 0 && (previous->entity_flags & FL_ONGROUND) == 0)
					m_player->m_fFlags() &= ~FL_ONGROUND;
			}
			else
				m_player->m_fFlags() |= FL_ONGROUND;
		}

		run_velocity_calculation(m_player, front, previous, &records[idx]);

		if (weapon && weapon->IsGun() && front->lag > 0)
		{
			const auto& shot_tick = TIME_TO_TICKS(weapon->m_flLastShotTime());

			if (shot_tick > old_simulation_ticks && simulation_ticks >= shot_tick)
			{
				front->shot_this_tick = true;

				if (shot_tick == simulation_ticks)
					front->shot_time = old_simulation_ticks + 1;
			}
			else
			{
				if (abs(simulation_ticks - shot_tick) > front->lag + 2)
				{
					resolver_info->m_flLastNonShotPitch = front->eye_angles.x;
				}
			}
		}

		if (front->shot_this_tick)
		{
			//fakelagging_and_other_checks = (ImportantFlags_2 & 16) == 0;
			//if (fakelagging_and_other_checks)
			//{
			if (front->lag > 1)
			{
				if (front->eye_angles.x < 70.0f)            // if(eyeangles.x < 70)
				{
					if (resolver_info->m_flLastNonShotPitch > 80.0f)
						front->eye_angles.x = resolver_info->m_flLastNonShotPitch;
				}
			}
			//}
		}
	}

	// EFL_DIRTY_ABSVELOCITY
	// skip call to C_BaseEntity::CalcAbsoluteVelocity
	m_player->m_iEFlags() &= ~0x1000;
	m_player->m_vecAbsVelocity() = m_player->m_vecVelocity();

	cheat::features::aaa.resolve(m_player, cheat::main::shots_fired[idx], v75);

	state->feet_rate = 0.f;

	if (state) {
		if (state->last_anim_upd_tick == Source::m_pGlobalVars->framecount)
			state->last_anim_upd_tick = Source::m_pGlobalVars->framecount - 1;

		//if (state->last_anim_upd_time == Source::m_pGlobalVars->curtime)
		//	state->last_anim_upd_time = Source::m_pGlobalVars->curtime - state->anim_update_delta;
	}

	/*if (v75 > 1 && !m_records.empty())
	{
		auto old_duck_amount = m_records.front().duck_amt;

		m_player->m_flDuckAmount() = (((m_player->m_flDuckAmount() - old_duck_amount) / time_delta) * Source::m_pGlobalVars->interval_per_tick) + old_duck_amount;

		if (old_duck_amount < m_player->m_flDuckAmount() && m_player->m_flDuckAmount() != 1.f)
			m_player->m_angEyeAngles() = m_records.front().eye_angles;
	}*/

	m_player->update_clientside_animations();

	fix_poses(m_player);

	memcpy(resolver_info->client_anim_layers, m_player->animation_layers_ptr(), 0x38 * m_player->get_animation_layers_count());
	resolver_info->m_flClientRate = resolver_info->client_anim_layers[6].m_flPlaybackRate;

	//*(QAngle*)(uintptr_t(m_player) + 0x128) = ang_rot_backup;

	memcpy(m_player->animation_layers_ptr(), resolver_info->server_anim_layers, 0x38 * m_player->get_animation_layers_count());

	Source::m_pGlobalVars->curtime = curtime; //local_weapon->GetCSWeaponData()->range
	Source::m_pGlobalVars->frametime = frametime;

	//m_player->m_bCanMoveDuringFreezePeriod() = can_move_during_assfuck;

	m_player->force_bone_rebuild();
	m_player->SetupBonesEx();

	//if (resolver_info->did_shot_this_tick) {
		//cheat::features::aimbot.visualise_hitboxes(m_player, m_player->m_CachedBoneData().Base(), Color::White(), Source::m_pGlobalVars->interval_per_tick * 2.0f);
		//m_player->DrawServerHitboxes();
	//}
}

void c_animsystem::reset()
{

}
