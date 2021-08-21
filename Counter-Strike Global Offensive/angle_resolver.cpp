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


bool c_resolver::has_fake(C_BasePlayer* entity)
{
	auto index = entity->entindex() - 1;
	auto player_lag_record = &cheat::features::lagcomp.records[index];

	if (player_lag_record->tick_records.size() < 2)
		return true;

	if (fabs(player_lag_record->tick_records.front().simulation_time - player_lag_record->tick_records.at(1).simulation_time) == Source::m_pGlobalVars->interval_per_tick)
		return false;

	return true;
}

//void c_base_player::calc_anim_velocity(bool reset) {
//	int idx = ce()->GetIndex();
//
//	/*secret antipaste movement code*/
//
//	if (reset) {
//		vec3_t velocity = m_vecVelocity();
//		sm_animdata[idx].m_lastOrigin = m_vecOrigin();
//		sm_animdata[idx].m_lastVelocity = velocity;
//		sm_animdata[idx].m_animVelocity = velocity;
//	}
//	else {
//		static auto sv_accelerate = g_csgo.m_cvar()->FindVar(xors("sv_accelerate"));
//
//		float delta = m_flSimulationTime() - m_flOldSimulationTime();
//		delta = std::max(delta, TICK_INTERVAL());
//
//		bool on_ground = m_fFlags() & FL_ONGROUND;
//
//		vec3_t origin = m_vecOrigin();
//		vec3_t origin_delta = origin - sm_animdata[idx].m_lastOrigin;
//
//		vec3_t velocity = origin_delta / delta;
//		vec3_t last_velocity = sm_animdata[idx].m_lastVelocity;
//
//		vec3_t anim_vel;
//
//		if (on_ground) {
//			vec3_t ang = math::vector_angles(vec3_t(), velocity);
//
//			float move_yaw = math::vector_angles(velocity).y;
//			float move_delta = math::vector_angles(last_velocity).y;
//			move_delta -= move_yaw;
//			move_delta = std::remainderf(move_delta, 360.f);
//
//			vec3_t move_dir = math::angle_vectors(vec3_t(0.f, move_delta, 0.f)) * 450.f;
//
//			vec3_t forward, right, up;
//			math::angle_vectors(ang, &forward, &right, &up);
//
//			vec3_t wishdir;
//			/*super secret antipaste math code*/
//
//			anim_vel = friction(last_velocity);
//			if (anim_vel.length2d() < 1.f)
//				anim_vel = vec3_t();
//
//
//			int ticks = TIME_TO_TICKS(delta);
//			vec3_t est_tick_vel;
//			/*super secret fakewalk detection code*/
//
//			if (velocity.length2d() > last_velocity.length2d())
//				anim_vel = accelerate(anim_vel, wishdir, 250.f, sv_accelerate->get_float());
//
//			//assume fakewalk
//			float last_speed = sm_animdata[idx].m_animVelocity.length2d();
//			if (anim_vel.length2d() >= last_speed && est_tick_vel.length2d() < 1.f)
//				anim_vel = vec3_t();
//		}
//		else {
//			//doesn't matter much anyway
//			anim_vel = math::lerp(
//				last_velocity,
//				velocity,
//				TICK_INTERVAL() / delta
//			);
//		}
//
//		sm_animdata[idx].m_animVelocity = anim_vel;
//		sm_animdata[idx].m_lastVelocity = velocity;
//		sm_animdata[idx].m_lastOrigin = origin;
//	}
//}

void c_resolver::CLBYRECORD::store(C_BasePlayer* entity)
{
	lby = entity->m_flLowerBodyYawTarget();
	speed = entity->m_vecVelocity().Length();
}

//float c_resolver::get_average_moving_lby(const int& entindex, const float& accuracy)
//{
//	auto logs = move_logs[entindex];
//
//	if (logs.empty())
//		return 1337.f;
//
//	auto ilogs = logs.size() - 1;
//
//	if (ilogs > 12)
//		logs.resize(12);
//
//	float piska = 0.f;
//
//	for (auto siski : logs)
//		piska += siski.lby;
//
//	return Math::normalize_angle(piska / ilogs);
//}

bool c_resolver::compare_delta(float v1, float v2, float Tolerance)
{
	v1 = Math::normalize_angle(v1);
	v2 = Math::normalize_angle(v2);

	if (fabs(Math::AngleDiff(v1, v2)) <= Tolerance)
		return true;

	return false;
}

float brutelist[] =
{
	119.f,
	180.f,
	81.f,
	180.f,
	-82.f,
};

float brutelist_shots[] =
{
	0.f,
	119.f,
	180.f,
	-119.f,
	180.f,
	-82.f,
};

std::unordered_map <int, float> old_static_yaw = {};
std::unordered_map <int, int> prev_rotation = {};

void c_resolver::on_real_angle_arrive(C_BasePlayer* m_player, resolver_records* resolve_data, float real_yaw)
{
	if (fabs(resolve_data->last_shot_time - Source::m_pGlobalVars->realtime) > 0.1f)
		resolve_data->latest_delta_used		= Math::AngleDiff(resolve_data->latest_fake.y, real_yaw);

	//resolve_data->is_velocity_angle_fine	= (resolve_data->last_speed > 0.1f && fabs(Math::AngleDiff(real_yaw, resolve_data->last_velocity_angle)) < 45.f);
	resolve_data->had_fake					= 
	resolve_data->fakeangles = (fabs(Math::AngleDiff(resolve_data->latest_fake.y, real_yaw)) > 5.f || fabs(Math::AngleDiff(resolve_data->latest_angles.y, real_yaw)) > 5.f);
}

void c_resolver::on_new_data_arrive(C_BasePlayer* m_player, resolver_records* resolve_data, CCSGOPlayerAnimState* state)
{
	auto history = cheat::main::shots_fired[m_player->entindex() - 1];

	auto current_velocity_angle			 = RAD2DEG(atan2(m_player->m_vecVelocity().y, m_player->m_vecVelocity().x));
	resolve_data->balance_adjust		 = (resolve_data->server_anim_layers[3].m_flWeight > 0.01f && m_player->get_sec_activity(resolve_data->server_anim_layers[3].m_nSequence) == 979 && resolve_data->server_anim_layers[3].m_flCycle < 1.f);
	auto activity						 = m_player->get_sec_activity(resolve_data->server_anim_layers[1].m_nSequence);

	if (resolve_data->balance_adjust)
		resolve_data->last_time_balanced = Source::m_pGlobalVars->realtime;

	resolve_data->did_shot_this_tick	 = (m_player->get_weapon() != nullptr && m_player->get_weapon()->m_flLastShotTime() != resolve_data->last_shot_time && (m_player->m_angEyeAngles().x != resolve_data->previous_angles.x || fabs(m_player->m_angEyeAngles().x) < 65));//((activity >= ACT_CSGO_FIRE_PRIMARY && activity <= ACT_CSGO_FIRE_SECONDARY_OPT_2) && resolve_data->server_anim_layers[1].m_flWeight >= 0.01f && m_player->get_animation_layer(1).m_flCycle < 0.05f) || (m_player->get_weapon() && m_player->get_weapon()->m_Activity() == 208);
	resolve_data->tick_delta			 = Math::normalize_angle(m_player->m_angEyeAngles().y - resolve_data->latest_angles.y);
	resolve_data->lby_delta				 = Math::normalize_angle(m_player->m_flLowerBodyYawTarget() - m_player->m_angEyeAngles().y);
	resolve_data->last_velocity_angle	 = current_velocity_angle;
	resolve_data->last_speed			 = state->speed_2d;
	resolve_data->last_lby				 = m_player->m_flLowerBodyYawTarget();
	resolve_data->origin				 = m_player->m_vecOrigin();

	if (state->speed_2d > 0.1f)
		resolve_data->last_time_moved = Source::m_pGlobalVars->realtime;
	if (TIME_TO_TICKS(m_player->m_flSimulationTime() - m_player->m_flOldSimulationTime()) > 1)
		resolve_data->last_time_choked = Source::m_pGlobalVars->realtime;

	resolve_data->is_balancing = (fabs(Source::m_pGlobalVars->realtime - resolve_data->last_time_balanced) < 0.6f && fabs(resolve_data->lby_delta) > 0.0001f);
	resolve_data->did_lby_update_fail = (/*resolve_data->abs_yaw_delta) > 0.0001f &&*/ fabs(resolve_data->lby_delta) > 0.0001f);

	resolve_data->fakeangles = (resolve_data->is_balancing || TIME_TO_TICKS(m_player->m_flSimulationTime() - resolve_data->last_simtime) > 1);

	if (resolve_data->fakeangles)
		resolve_data->latest_angles_when_faked = resolve_data->latest_angles;
}

bool c_resolver::resolve_freestand(C_BasePlayer* m_player, resolver_records* resolve, CCSGOPlayerAnimState* state, float &step)
{
	const auto freestanding_record = resolve->freestanding_record;

	//const float at_target_yaw = Math::CalcAngle(cheat::main::local()->m_vecOrigin(), m_player->m_vecOrigin()).y;

	if (freestanding_record.left_damage <= 0 && freestanding_record.right_damage <= 0)
	{
		if (freestanding_record.right_fraction == freestanding_record.left_fraction)
			return false;

		if (freestanding_record.right_fraction < freestanding_record.left_fraction /*|| (cheat::main::shots_fired[m_player->entindex() - 1] % 2 == 0 && cheat::main::shots_fired[m_player->entindex() - 1] > 0)*/)
			step = -1.f;
		else
			step = 1.f;
	}
	else
	{
		if (freestanding_record.left_damage > freestanding_record.right_damage /*|| (cheat::main::shots_fired[m_player->entindex()-1] % 2 == 0 && cheat::main::shots_fired[m_player->entindex() - 1] > 0)*/)
			step = -1.f;
		else
			step = 1.f;
	}

	return true;
}

void c_resolver::add_new_lby_delta_record(resolver_records* resolve, float record)
{
	//if (piska.empty())
	resolve->lby_deltas.emplace_front(record);
	/*else
	{
		if (Math::AngleDiff(record, piska.front()) > 35.f) {
			if (fabs(record) < 35.f)
				return;
			else
				piska.emplace_front(record);
		}
		else
			piska.emplace_front(record);
	}*/
}

bool c_resolver::get_lby_delta_record(C_BasePlayer* m_player, resolver_records* resolve, float& returnval)
{
	auto &piska = resolve->lby_deltas;

	if (piska.size() <= 1)
		return false;

	auto ilogs = (piska.size() - 1);

	if (ilogs > 12)
		piska.resize(12);

	float total = 0.f;

	for (auto siski : piska)
		total += abs(siski);

	auto final_delta = (total / ilogs);

	//final_delta = copysign(final_delta, piska.front());

	if (resolve->missed_shots[3] % 2 == 1)
		final_delta *= -1.f;

	returnval = Math::normalize_angle(final_delta);

	return true;
}

bool c_resolver::get_yaw_by_walls(C_BasePlayer* entity, resolver_records* resolve, float& yaw)
{
	const auto freestanding_record = resolve->freestanding_record;

	const float at_target_yaw = Math::CalcAngle(cheat::main::local()->m_vecOrigin(), entity->m_vecOrigin()).y;

	if (freestanding_record.left_damage >= 20 && freestanding_record.right_damage >= 20)
		return false;

	if (freestanding_record.left_damage <= 0 && freestanding_record.right_damage <= 0)
	{
		if (freestanding_record.right_fraction < freestanding_record.left_fraction)
			yaw = at_target_yaw + 125.f;
		else
			yaw = at_target_yaw - 73.f;
	}
	else
	{
		if (freestanding_record.left_damage > freestanding_record.right_damage)
			yaw = at_target_yaw + 130.f;
		else
			yaw = at_target_yaw - 49.f;
	}

	return true;
}

void c_resolver::resolve(C_BasePlayer* m_player, int &history, int &chocked_ticks)
{
	auto state = m_player->get_animation_state();

	auto idx = m_player->entindex() - 1;

	auto lag_data = &player_resolver_records[idx];
	auto history_data = cheat::features::lagcomp.records[idx].tick_records;
	
	lag_data->resolved_yaw = state->abs_yaw;
	lag_data->did_lby_flick = false;

	if ((lag_data->last_simtime - m_player->m_flSimulationTime()) > 0.5f) {
		lag_data->lby_deltas.clear();
		
		if (lag_data->origin.Distance(m_player->m_vecOrigin()) > 16.f/* && fabs(Math::AngleDiff(lag_data->last_lby, m_player->m_flLowerBodyYawTarget())) > 20.f*/) {
			lag_data->last_move_lby_valid/*saved_off_delta = lag_data->move_lby*/ = false;
		}
	}

	lag_data->did_shot_this_tick = (m_player->get_weapon() != nullptr && m_player->get_weapon()->m_flLastShotTime() != lag_data->last_shot_time && (m_player->m_angEyeAngles().x != lag_data->previous_angles.x || fabs(m_player->m_angEyeAngles().x) < 65.f));//((activity >= ACT_CSGO_FIRE_PRIMARY && activity <= ACT_CSGO_FIRE_SECONDARY_OPT_2) && resolve_data->server_anim_layers[1].m_flWeight >= 0.01f && m_player->get_animation_layer(1).m_flCycle < 0.05f) || (m_player->get_weapon() && m_player->get_weapon()->m_Activity() == 208);
	//lag_data->fakeangles = (/*lag_data->is_balancing || */TIME_TO_TICKS(m_player->m_flSimulationTime() - lag_data->last_simtime) >= 1);

	lag_data->last_lby = m_player->m_flLowerBodyYawTarget();
	//lag_data->origin = m_player->m_vecOrigin();

	if (!cheat::Cvars.Misc_AntiUT.GetValue() && history_data.size() > 1)
	{
		auto record1 = history_data.at(0), record2 = history_data.at(1);
	
		if (!(m_player->m_fFlags() & FL_ONGROUND) && record2.entity_flags & FL_ONGROUND && m_player->m_angEyeAngles().x >= 178.36304f)
			m_player->m_angEyeAngles().x = -89.f;
		else
		{
			if (abs(m_player->m_angEyeAngles().x) > 89.f)
				m_player->m_angEyeAngles().x = cheat::main::shots_fired[idx] % 4 != 3 ? 89.f : -89.f;
			else
				m_player->m_angEyeAngles().x = cheat::main::shots_fired[idx] % 4 != 3 ? 89.f : m_player->m_angEyeAngles().x;
		}
	}

	auto current_velocity_angle = RAD2DEG(atan2(m_player->m_vecVelocity().y, m_player->m_vecVelocity().x));

	float lby = m_player->m_flLowerBodyYawTarget();

	float step = 0.0f;

	lag_data->resolving_method = 0;
	auto at_targ = Math::CalcAngle(cheat::main::local()->m_vecOrigin(), m_player->m_vecOrigin()).y;
	auto delta_from_bw = Math::AngleDiff(at_targ + 180.f, state->abs_yaw);
	auto vdelta_from_bw = Math::AngleDiff(at_targ + 180.f, current_velocity_angle);
	auto is_backwards = fabs(delta_from_bw) < 70.f;
	float start_angle = m_player->m_angEyeAngles().y;

	lag_data->latest_angles = m_player->m_angEyeAngles();

	lag_data->latest_fake.y = state->abs_yaw;
	lag_data->previous_rotation = m_player->m_angEyeAngles().y;
	lag_data->previous_angles = m_player->m_angEyeAngles();

	if (!(m_player->m_fFlags() & FL_ONGROUND) || m_player->m_vecVelocity().Length2D() > 0.15f)
		lag_data->next_lby_time = FLT_MAX;

	if(fabs(Math::AngleDiff(lby, lag_data->prev_lby)) > 30.f && lag_data->prev_lby != FLT_MAX) {
		lag_data->did_lby_flick = true;
		lag_data->next_lby_time = m_player->m_anim_time() + 1.1f;

		lag_data->prev_lby = lby;
	}
	else
		lag_data->prev_lby = lby;

	if ((lag_data->next_lby_time - m_player->m_anim_time()) < 0.0f && lag_data->next_lby_time != FLT_MAX)
	{
		lag_data->did_lby_flick = true;
		lag_data->next_lby_time = m_player->m_anim_time() + 1.1f;
	}

	if (lag_data->did_shot_this_tick && lag_data->pre_shot_pitch != FLT_MAX)
		m_player->m_angEyeAngles().x = lag_data->pre_shot_pitch;

	/*if (lag_data->last_move_lby_valid)
	{
		auto v41 = Math::normalize_angle(lby - lag_data->old_lby);

		if (fabs(v41) <= 35.0)
		{

		}
	}*/

	if (cheat::Cvars.RageBot_Resolver.GetValue())
	{
		const auto on_ground = m_player->m_fFlags() & FL_ONGROUND/* && lag_data->prev_flags & FL_ONGROUND*/;

		if (!on_ground)
			m_player->m_angEyeAngles().y = lby + Math::normalize_angle(brutelist_shots[cheat::main::shots_fired[idx] % 6]);
		else
		{
			if (m_player->m_vecVelocity().Length2D() > 0.1f)
			{
				lag_data->did_lby_flick = true;
				m_player->m_angEyeAngles().y = lby;
				lag_data->move_lby = lby;
				lag_data->never_saw_movement = false;
				lag_data->last_move_lby_valid = true;
				lag_data->was_moving = true;
				lag_data->origin = m_player->m_vecOrigin();
				lag_data->resolving_method = 1;
				lag_data->last_time_moved = Source::m_pGlobalVars->realtime;
			}
			else
			{
				if (lag_data->was_moving && lag_data->last_move_lby_valid)
				{
					auto v41 = Math::normalize_angle(lag_data->move_lby - lby);

					if (fabs(v41) >= 100.f)
					{
						lag_data->old_lby = lby;
						lag_data->saved_off_delta = v41;
						lag_data->was_moving = false;
					}
				}

				/*if ((Source::m_pGlobalVars->realtime - lag_data->last_time_moved) < 0.75f)
				{
					m_player->m_angEyeAngles().y = lby;
					lag_data->resolving_method = 5;
				}*/
				if ((Source::m_pGlobalVars->realtime - lag_data->last_time_moved) > 0.75f && lag_data->was_moving)
				{
					lag_data->old_lby = lby;
					lag_data->was_moving = false;
				}

				/*else */if (lag_data->did_lby_flick && lag_data->missed_shots[2] < 2)
				{
					m_player->m_flSimulationTime() = m_player->m_flOldSimulationTime();
					m_player->m_angEyeAngles().y = lby;
					lag_data->resolving_method = 2;
				}
				else
				{
					if (lag_data->never_saw_movement || !lag_data->last_move_lby_valid || lag_data->saved_off_delta > 1000.0f)
					{
						lag_data->resolving_method = 3;
						if (float yaw_resolved = 0.0f; get_yaw_by_walls(m_player, lag_data, yaw_resolved)) {
							m_player->m_angEyeAngles().y = Math::normalize_angle(yaw_resolved);
							lag_data->resolving_method = 4;
						}
						else
							m_player->m_angEyeAngles().y = m_player->m_flLowerBodyYawTarget();
					}
					else
					{
						lag_data->resolving_method = 4;

						if (lag_data->last_move_lby_valid)
							m_player->m_angEyeAngles().y = lag_data->move_lby;
						else
						{
							m_player->m_angEyeAngles().y = lby + lag_data->saved_off_delta;

							if (lag_data->saved_off_delta == 0.0f)
								m_player->m_angEyeAngles().y += RandomFloat(-20.f, 20.f);// -20 ; 20
						}
					}

					m_player->m_angEyeAngles().y += Math::normalize_angle(brutelist_shots[cheat::main::shots_fired[idx] % 6]);

					/*if (lag_data->avg_delta != FLT_MAX) {

						static auto plol = lag_data->saved_off_delta;

						if ((cheat::main::shots_fired[idx] % 2) == 1 && cheat::main::shots_fired[idx] < 2)
							plol *= -1.f;
						else
							plol = lag_data->saved_off_delta;

						m_player->m_angEyeAngles().y = Math::normalize_angle(lby + copysign(lag_data->avg_delta, lag_data->saved_off_delta));

						lag_data->resolving_method = 3;
					}
					else if (float yaw_resolved = 0.0f; get_yaw_by_walls(m_player, lag_data, yaw_resolved)) {
						m_player->m_angEyeAngles().y = Math::normalize_angle(yaw_resolved);
						lag_data->resolving_method = 4;
					}

					auto lol = cheat::main::shots_fired[idx] % 5;

					if (cheat::main::shots_fired[idx] > 1)
						m_player->m_angEyeAngles().y += Math::normalize_angle(brutelist[lol]);*/
				}
			}
		}
	}

	m_player->m_angEyeAngles().y = Math::normalize_angle(m_player->m_angEyeAngles().y);

	if (lag_data->did_shot_this_tick && m_player->get_weapon())
		lag_data->last_shot_time = m_player->get_weapon()->m_flLastShotTime();

	lag_data->last_simtime = m_player->m_flSimulationTime();

	if (!lag_data->did_shot_this_tick)
		lag_data->pre_shot_pitch = m_player->m_angEyeAngles().x;

	lag_data->prev_flags = m_player->m_fFlags();
}

//void c_resolver::resolve(C_BasePlayer* m_player, int history)
//{
//	auto state = m_player->get_animation_state();
//
//	auto idx = m_player->entindex() - 1;
//
//	auto lag_data = &player_resolver_records[idx];
//
//	if (!state || m_player == cheat::main::local()) {
//		cheat::main::shots_fired[idx] = 0;
//		//old_static_yaw[idx] = 0.f;
//		//prev_rotation[idx] = 0;
//		lag_data->previous_angles.y = FLT_MAX;
//		return;
//	}
//
//	//auto eye_angles = &entity->m_angEyeAngles();
//	//auto speed = entity->m_vecVelocity().Length();
//	//auto on_ground = (entity->m_fFlags() & FL_ONGROUND);
//
//	//const auto prev_resolve_info = player_resolver_records[idx];
//	//auto resolve_info = &player_resolver_records[idx];
//
//	//resolve_info->is_dormant = entity->IsDormant();
//
//	//if (resolve_info->is_dormant) {
//	//	if (!move_logs[idx].empty())
//	//		move_logs[idx].clear();
//	//	return;
//	//}
//
//	//auto player_lag_record = &cheat::features::lagcomp.records[idx];
//
//	////resolve_info->did_lby_flick = false;
//
//	//resolve_info->is_dormant = entity->IsDormant();
//
//	///*auto player_lag_record = &cheat::features::lagcomp.records[idx-1];
//
//	//if (player_lag_record->m_Tickrecords.size() < 2)
//	//continue;
//
//	//auto record1 = player_lag_record->m_Tickrecords.at(0), record2 = player_lag_record->m_Tickrecords.at(1);*/
//
//	////auto animlayer_7 = &entity->get_animation_layer(7);
//	////auto animlayer_12 = &entity->get_animation_layer(12);
//	////auto animlayer_6 = &entity->get_animation_layer(6);
//	auto animlayer_11 = &m_player->get_animation_layer(11);
//	auto animlayer_3 = &m_player->get_animation_layer(3);
//	////auto pose_2 = &entity->GetPoseParameter(2);
//
//	////if (!animlayer_7 || !animlayer_12 || !animlayer_6 || !pose_2 || !animlayer_11) return;
//
//	////if (prev_resolve_info.is_dormant && entity->m_vecVelocity().Length() > 0.f && animlayer_6->m_flWeight < 1.f)
//	////	animlayer_6->m_flWeight = 1.f;
//
//	//resolve_info->is_breaking_lby = (speed < 0.1f && /*abs((*pose_11 / 360.f) * 35.f) > 0.3f*/animlayer_3->m_flWeight > 0 && animlayer_3->m_nSequence == 4 && animlayer_3->m_flCycle < 0.9);
//
//	////if (animlayer_3->m_nSequence != 4)
//	////	printf("%d\n", animlayer_3->m_nSequence);
//
//	///*
//	//• Check for weight of 981 being 1.
//	//• Check for sequence 2 weight being 0.
//	//• Check for weight of layer 12 being bigger than 0.
//	//• Check for layer 6's playbackrate being smaller than (about) 0.0001.
//	//*/
//
//	////if ((speed > 0.1f && (animlayer_7->m_flWeight <= 0 && (animlayer_12->m_flWeight > 0 && animlayer_6->m_flPlaybackRate < 0.1 && animlayer_11->m_flWeight > 0 /*&& animlayer_11->m_nSequence == 9*/ || resolve_info->is_breaking_lby))))
//	////	resolve_info->is_fakewalking = true;
//	////else
//	////	resolve_info->is_fakewalking = false;
//
//	////resolve_info->has_fake = has_fake(entity);
//
//	////if (Math::NormalizePitch(entity->m_angEyeAngles().x) > 40.f)
//	////	resolve_info->last_time_down_pitch = Source::m_pGlobalVars->curtime;
//
//	//resolve_info->lower_body_yaw = entity->m_flLowerBodyYawTarget();
//
//	//if (cheat::settings.ragebot_hitchance <= 0 && player_lag_record->m_Tickrecords.size() > 1)
//	//{
//	//	auto record1 = player_lag_record->m_Tickrecords.at(0), record2 = player_lag_record->m_Tickrecords.at(1);
//
//	//	eye_angles->x = std::clamp(Math::normalize_angle(eye_angles->x), -89.f, 89.f);
//
//	//	if (!(entity->m_fFlags() & FL_ONGROUND) && record2.entity_flags & FL_ONGROUND && eye_angles->x >= 178.36304f)
//	//		eye_angles->x = -89.f;
//	//	else
//	//	{
//	//		if (abs(eye_angles->x) > 89.f)
//	//			eye_angles->x = cheat::main::shots_fired[idx] % 4 != 3 ? 89.f : -89.f;
//	//		else
//	//			eye_angles->x = cheat::main::shots_fired[idx] % 4 != 3 ? 89.f : eye_angles->x;
//	//	}
//	//}
//
//	//resolve_info->previous_angles = *eye_angles;
//
//	////auto v23 = sub_59B13C30(animstate);
//
//	////float v45 = 0.f;
//
//	////if (cheat::settings.ragebot_resolver) {
//
//	//	//if (*(float*)(uintptr_t(animstate) + 0xEC) > 70.f
//	//	//	|| *(float*)(uintptr_t(animstate) + 0xF0) > 0.f)
//	//	//{
//
//	//	//	/*float v40 = *(float *)(v18 - 1424671547);
//	//	//	float v41 = v33 - 1;
//	//	//	if (v41)
//	//	//	{
//	//	//		if (v41 == 1)
//	//	//			v40 = v40 - v23;
//	//	//	}
//	//	//	else
//	//	//	{
//	//	//		v40 = v40 + v23;
//	//	//	}
//
//	//	//	float v47 = (float)(unsigned __int16)(signed int)(float)(v40 * *(float *)&dword_673A97F8) * *(float *)&dword_673A9564;
//	//	//	float v48 = (float)((float)(unsigned __int16)(signed int)(float)(*(float *)(v18 - 0x54EAC333) * *(float *)&dword_673A97F8)
//	//	//		* *(float *)&dword_673A9564)
//	//	//		- v47;
//
//	//	//	if (-180 < v48)
//	//	//	{
//	//	//		if (v48 > 180.f)
//	//	//			v48 = v48 - 360;
//	//	//	}
//	//	//	else
//	//	//	{
//	//	//		v48 = v48 + 360;
//	//	//	}
//
//	//	//	if (v48 <= unk)
//	//	//	{
//	//	//		if (fabs(unk) <= v48)
//	//	//			v45 = (float)(unsigned __int16)(signed int)(float)(*(float *)(v18 - 0x54EAC333) * *(float *)&dword_673A97F8)
//	//	//			* *(float *)&dword_673A9564;
//	//	//		else
//	//	//			v45 = v47 - unk;
//	//	//	}
//	//	//	else
//	//	//	{
//	//	//		v45 = unk + v47;
//	//	//	}*/
//
//	//	//	v45 = entity->m_flLowerBodyYawTarget();
//	//	//}
//	//	//else
//	//	//	v45 = entity->m_angEyeAngles().y + (cheat::main::shots_fired[idx] % 3 != 0 && cheat::main::shots_fired[idx] % 3 == 1 ? -v23 : v23);
//
//	//	//if (-180 < v45)
//	//	//{
//	//	//	if (v45 > 180.f)
//	//	//		v45 = v45 - 360.f;
//	//	//}
//	//	//else
//	//	//{
//	//	//	v45 = v45 + 360.f;
//	//	//}
//
//	////	auto absang = entity->m_angEyeAngles();
//	////	animstate->m_flGoalFeetYaw = cheat::features::antiaimbot.rebuild_setupvelocity(entity, false);
//	////	/*entity->m_angEyeAngles().y = */absang.y = animstate->m_flGoalFeetYaw;
//	////	entity->UpdateAnimationState(animstate, absang);
//	////}
//	////eye_angles->y = animstate->m_flCurrentFeetYaw; 
//	////entity->m_flLowerBodyYawTarget() = Math::normalize_angle(entity->m_flLowerBodyYawTarget());
//
//	///*auto max_delta = get_max_desync_delta(entity);
//
//	//if (cheat::main::shots_fired[idx] > 1) {
//	//	switch (cheat::main::shots_fired[idx] % 3)
//	//	{
//	//	case 1:
//	//		eye_angles->y += max_delta;
//	//		break;
//	//	case 2:
//	//		eye_angles->y -= max_delta;
//	//		break;
//	//	default:
//	//		break;
//	//	}
//	//}*/
//
//	////if (resolve_info->has_fake) {
//	////	//else {
//	////		//// super fucking ghetto fix to stop their pitch from going to 0 when they're shooting (only neccesary when they're using psilent)
//	////		//if (fabs(Source::m_pGlobalVars->curtime - resolve_info->last_time_down_pitch) < 0.5f)
//	////		//	eye_angles->x = 80.f;
//	////	//}
//
//	////	if (on_ground)
//	////	{
//	////		entity->m_flLowerBodyYawTarget() = Math::normalize_angle(entity->m_flLowerBodyYawTarget());
//
//	////		if (speed > 0.1f && !resolve_info->is_fakewalking && !prev_resolve_info.is_fakewalking && animstate->m_flTimeSinceStartedMoving > 0.f)
//	////		{
//	////			resolve_info->last_moving_lby = entity->m_flLowerBodyYawTarget();
//	////			eye_angles->y = entity->m_flLowerBodyYawTarget();
//	////			cheat::main::shots_fired[idx] = 0;
//	////			resolve_info->origin = entity->m_vecOrigin();
//	////			resolve_info->last_time_moving = Source::m_pGlobalVars->curtime;
//
//	////			if (move_logs[idx].empty() || move_logs[idx].front().lby != entity->m_flLowerBodyYawTarget())
//	////			move_logs[idx].push_back(CLBYRECORD(entity));
//	////		}
//	////		else
//	////		{
//	////			auto movelby = get_average_moving_lby(idx);
//
//	////			if (movelby < 361.f)
//	////				resolve_info->last_moving_lby = movelby;
//
//	////			if (resolve_info->is_last_moving_lby_valid && (Source::m_pGlobalVars->curtime - resolve_info->last_time_moving) < 1.f/* && !resolve_record.is_fakewalking */&& resolve_info->lower_body_yaw != resolve_info->last_moving_lby && animstate->m_flTimeSinceStoppedMoving < 1.f)
//	////			{
//	////				resolve_info->is_getting_right_delta = true;
//
//	////				if (resolve_info->lower_body_yaw < 0.0f)
//	////					resolve_info->lower_body_yaw += 360.f;
//	////				else if (resolve_info->lower_body_yaw > 360.f)
//	////					resolve_info->lower_body_yaw -= 360.f;
//
//	////				if (resolve_info->last_moving_lby < 0.0f)
//	////					resolve_info->last_moving_lby += 360.f;
//	////				else if(resolve_info->last_moving_lby > 360.f)
//	////					resolve_info->last_moving_lby -= 360.f;
//
//	////				resolve_info->just_stopped_delta = Math::normalize_angle(/*abs(*/resolve_info->lower_body_yaw - resolve_info->last_moving_lby);
//	////			}
//	////			else
//	////				resolve_info->is_getting_right_delta = false;
//
//	////			resolve_info->is_last_moving_lby_valid = (fabs((resolve_info->origin - entity->m_vecOrigin()).Length()) < 16.f);
//
//	////			//if (!resolve_info->is_breaking_lby)
//	////			//	eye_angles->y = entity->m_flLowerBodyYawTarget();
//	////			//else {
//	////			if (resolve_info->last_moving_lby != 0.0000001337f && resolve_info->is_last_moving_lby_valid && cheat::settings.resolver_lastmovelby)
//	////				eye_angles->y = resolve_info->last_moving_lby;
//	////			else if (static auto lol = 0.f; freestanding_resolver(entity, lol) && cheat::settings.resolver_freestand) 
//	////				eye_angles->y = lol;
//	////			else
//	////				eye_angles->y = resolve_info->lower_body_yaw + ((resolve_info->just_stopped_delta > 0) ? -resolve_info->just_stopped_delta : resolve_info->just_stopped_delta);
//
//	////			/*if (cheat::main::shots_fired[idx] > 1) {
//	////				switch (cheat::main::shots_fired[idx] % 3)
//	////				{
//	////				case 1:
//	////					eye_angles->y += 180.0;
//	////					break;
//	////				case 2:
//	////					eye_angles->y -= 180.0;
//	////					break;
//	////				default:
//	////					break;
//	////				}
//	////			}*/
//	////		}
//	////	}
//	////	else
//	////	{
//	////		float v31 = entity->m_flLowerBodyYawTarget();
//	////		auto v32 = entity->m_angEyeAngles();
//	////		auto v43 = Math::normalize_angle(v32.y + 180.f);
//
//	////		switch (/*(G::player_shots[idx - 1] - G::player_hits[index])*/cheat::main::shots_fired[idx] % 6)
//	////		{
//	////		case 1:
//	////			v31 = v43;
//	////			//player_resolve_record->yaw_resolver = 2/*+ 180.f*/;
//	////			break;
//	////		case 2:
//	////			v31 = v43 + 15.0;
//	////			//player_resolve_record->yaw_resolver = 3/*+ 180.f + 15*/;
//	////			break;
//	////		case 3:
//	////			v31 = v43 - 15.0;
//	////			//player_resolve_record->yaw_resolver = 4/*+ 180.f - 15*/;
//	////			break;
//	////		case 4:
//	////			v31 = v43 + 60.0;
//	////			//player_resolve_record->yaw_resolver = 5/*+ 180.f + 60*/;
//	////			break;
//	////		case 5:
//	////			v31 = v43 - 60.0;
//	////			//player_resolve_record->yaw_resolver = 6/*+ 180.f - 60*/;
//	////			break;
//	////		default:
//	////			v31 = entity->m_flLowerBodyYawTarget();
//	////			//player_resolve_record->yaw_resolver = 1/*lby*/;
//	////			break;
//	////		}
//
//	////		eye_angles->y = Math::normalize_angle(v31/*v43*/);
//	////	}
//	////}
//	//
//	////resolve_info->delta_between_ticks = abs(Math::normalize_angle(resolve_info->previous_angles.y) - Math::normalize_angle(eye_angles->y));
//
//	////resolve_info->fakeangles = /*resolve_info->is_fakewalking || */resolve_info->is_breaking_lby /*|| resolve_info->delta_between_ticks > 50|| (resolve_info->has_fake && speed < 0.1f)*/;
//
//	////if (cheat::settings.resolver_autoresolve && cheat::main::shots_fired[idx] > 0)
//	////	eye_angles->y += (cheat::main::shots_fired[idx] % 2 == 1 ? -58.f : 58.f);
//
//	////if (cheat::settings.resolver_autoresolve)
//	////	eye_angles->y = entity->m_flLowerBodyYawTarget();
//
//	////eye_angles->y = Math::normalize_angle(eye_angles->y);
//
//	////entity->set_abs_angles(QAngle(0, eye_angles->y, 0));
//
//	////if (resolve_info->is_breaking_lby) {
//	//	//animlayer_3->m_flCycle = animlayer_3->m_flCycle = animlayer_3->m_flWeight = 0.f;
//	//	//animlayer_3->m_nSequence = 5;
//	////}
//
//	//static auto CalculatePlaybackRate = [](C_BasePlayer* player, const float& velocity_delta, float move_dist = -1.0f) {
//	//	auto state = player->get_animation_state();
//	//	auto anim_layer = &player->get_animation_layer(6);
//
//	//	auto move_yaw = player->m_flPoseParameter()[7];
//	//	player->m_flPoseParameter()[7] = velocity_delta;
//
//	//	auto seq_dur = player->SequenceDuration(*(CStudioHdr**)((uintptr_t)player + 0x294C), anim_layer->m_nSequence);
//
//	//	float v56;
//	//	if (seq_dur <= 0.0f)
//	//		v56 = 10.0f;
//	//	else
//	//		v56 = 1.0f / seq_dur;
//
//	//	float v237 = 1.0f / (1.0f / v56);
//
//	//	auto dist = move_dist;
//	//	if (move_dist == -1.0f) {
//	//		dist = player->GetSequenceMoveDist(*(CStudioHdr**)((uintptr_t)player + 0x294C), anim_layer->m_nSequence);
//	//	}
//
//	//	if (dist * v237 <= 0.001f) {
//	//		dist = 0.001f;
//	//	}
//	//	else {
//	//		dist *= v237;
//	//	}
//
//	//	player->m_flPoseParameter()[7] = move_yaw;
//
//	//	float speed = state->m_flSpeed2D;
//	//	float v50 = (1.0f - (state->m_flStopToFullRunningFraction * 0.15f)) * ((speed / dist) * v56); // 0x11C
//	//	float new_playback_rate = Source::m_pGlobalVars->interval_per_tick * v50;
//	//	return new_playback_rate;
//	//};
//
//	auto v58 = m_player->m_angEyeAngles().y - m_player->get_abs_eye_angles().y;
//
//	auto v20 = v58;
//
//	if (v58 >= -180.0)
//	{
//		if (v58 > 180.0)
//			v20 = v58 - 360.0;
//	}
//	else
//	{
//		v20 = v58 + 360.0;
//	}
//
//	auto v59 = cheat::features::aaa.sub_59B13C30(state);
//	auto v60 = fabs(v20) > v58;
//	//auto v61 = fabs(v20) > 0.f;
//
//	auto pitch = m_player->m_angEyeAngles().x;
//
//	cheat::features::aaa.player_resolver_records[idx].fakeangles = (fabs(v20) > 0.f || v60) && cheat::Cvars.RageBot_Resolver.GetValue();
//
//	if (!cheat::Cvars.RageBot_Resolver.GetValue() || v59 < 20.f)
//		return;
//
//	//if (pitch == 88.9947510f) // fix netvar compression
//	m_player->m_angEyeAngles().x = fminf(fmaxf(pitch, -90.0), 90.0);//Math::clamp(pitch,-90.f,90.f);
//
//	float lby = m_player->m_flLowerBodyYawTarget();
//
//	//auto fucked_up = (history % 3) > 1;
//	//if ((state->m_flSpeed2D > 0.1f && v59 > 30.f) || fabs(state->m_flUpSpeed) > 100.0f)
//	//{
//	//	float step = 0.0f;
//	//	switch (history % 3) {
//	//	case 0:
//	//		step = v59;
//	//		break;
//	//	case 1:
//	//		step = -v59;
//	//		break;
//	//	}
//
//	//	state->m_flGoalFeetYaw = fmod(Math::NormalizeYaw(m_player->m_angEyeAngles().y - step), 360.0f); // from [-180;180] to [0, 360]
//	//}
//	//else
//	//{
//	//	float delta = Math::NormalizeYaw(lby - m_player->m_angEyeAngles().y);
//	//	if (std::fabsf(delta) > v59) {
//	//		auto step = ((delta > 0.f ? -v59 : v59) * (fucked_up ? 0.5f : 1.f));
//
//	//		if (history % 2 == 0)
//	//			state->m_flGoalFeetYaw = fmod(Math::NormalizeYaw(m_player->m_angEyeAngles().y - step), 360.0f); // from [-180;180] to [0, 360]
//	//	}
//	//}
//
//	if (cheat::Cvars.RageBot_Resolver.GetValue() == 1) {
//
//		static float step = 0.0f;
//
//		if (state->m_flSpeed2D > 0.1f /*|| std::fabsf(state->m_flUpSpeed) > 100.0f*/) {
//			resolve_moving_entity(m_player);
//
//			//auto cycle_updated = (lag_data->last_mcycle_update != lag_data->anim_layers[6].m_flCycle);
//
//			//if (cycle_updated)
//			//{
//			//	step = 0;// Math::normalize_angle(m_player->m_flLowerBodyYawTarget() - m_player->m_angEyeAngles().y);
//			//	lag_data->last_mcycle_update = lag_data->anim_layers[6].m_flCycle;
//			//}
//			//else {
//			//	/*if ((Source::m_pGlobalVars->realtime - lag_data->last_time_balanced) < 0.5f)
//			//	{
//			//		auto pDelta = Math::normalize_angle(m_player->m_flLowerBodyYawTarget() - m_player->m_angEyeAngles().y);
//
//			//		if (fabs(pDelta) > 5.f) {
//			//			auto side = pDelta < 1.f;
//
//			//			if (side)
//			//				step = v59;
//			//			else
//			//				step = -v59;
//
//			//			cheat::features::aaa.player_resolver_records[idx].fakeangles = true;
//			//		}
//			//		else
//			//			step = 0.f;
//			//	}
//			//	else*/
//			//		step = Math::normalize_angle(state->m_flGoalFeetYaw - m_player->m_angEyeAngles().y);//Math::normalize_angle(m_player->m_angEyeAngles().y - state->m_flCurrentTorsoYaw) * 0.6f;
//			//}
//
//			//state->m_flGoalFeetYaw = fmod(Math::NormalizeYaw(m_player->m_angEyeAngles().y + step), 360.0f); // from [-180;180] to [0, 360]
//
//			//if (state->m_flGoalFeetYaw < 0.0f)
//			//	state->m_flGoalFeetYaw += 360.0f;
//
//			////QAngle angles = m_player->get_abs_eye_angles();
//			//auto angles_y = Math::NormalizeYaw(state->m_flGoalFeetYaw);
//			//m_player->set_abs_angles(QAngle(0, angles_y, 0));
//		}
//		else
//		{
//			//if (lby == m_player->m_angEyeAngles().y && lby == state->m_flGoalFeetYaw)
//			//	return;
//
//			//*auto is_using_static = fabs(v20) < 10.f && fabs(Math::normalize_angle(m_player->m_angEyeAngles().y - m_player->m_flLowerBodyYawTarget())) < 10.f && !v60;
//
//			//if (is_using_static)
//			//{
//			//	auto v41 = (history % 3);
//			//	auto low_values = Math::normalize_angle(m_player->m_angEyeAngles().y - state->m_flCurrentTorsoYaw);
//
//			//	auto benis = history % 2 == 0 ? 0.f : 0.5f;
//
//			//	if (old_static_yaw[idx] != low_values) {
//
//			//		if (old_static_yaw[idx] < low_values) {
//			//			step = v59 * benis;
//			//			prev_rotation[idx] = 1;
//			//		}
//			//		else {
//			//			step = v59 * -benis;
//			//			prev_rotation[idx] = 2;
//			//		}
//
//			//		old_static_yaw[idx] = low_values;
//			//	}
//			//	else if (old_static_yaw[idx] != 0 && v41 < 2)
//			//	{
//			//		switch (prev_rotation[idx])
//			//		{
//			//		case 1:
//			//			step = v59 * benis;
//			//			break;
//			//		case 2:
//			//			step = v59 * -benis;
//			//			break;
//			//		}
//			//	}
//			//	else {
//			//		if (v41)
//			//		{
//			//			if (v41 == 1)
//			//				step = v59 * -benis;
//			//		}
//			//		else
//			//			step = v59 * benis;
//			//	}
//			//}
//			//else */
//			//if (v60)
//			//{
//			//	auto v33 = 0;
//
//			//	if ((history % 3) != 2)
//			//		v33 = (v58 <= 0.0f) + 1;
//			//	else
//			//		v33 = (v58 > 0.0f) + 1;
//
//			//	auto v41 = v33 - 1;
//			//	if (v41)
//			//	{
//			//		if (v41 == 1)
//			//			step = -v59;
//			//	}
//			//	else
//			//	{
//			//		step = v59;
//			//	}
//			//}
//			//else
//			//{
//			//	/*switch (history % 3) {
//			//	case 0:
//			//		step = v59;
//			//		break;
//			//	case 1:
//			//		step = -v59;
//			//		break;
//			//	}*/
//			//	auto v41 = history - 1;
//			//	if (v41)
//			//	{
//			//		if (v41 == 1)
//			//			step = -v59;
//			//	}
//			//	else
//			//	{
//			//		step = v59;
//			//	}
//			//}
//
//			auto balance_adjust = (animlayer_3->m_flWeight > 0 && animlayer_3->m_nSequence == 4 && animlayer_3->m_flCycle < 0.9);
//
//			if (balance_adjust)
//				lag_data->last_time_balanced = Source::m_pGlobalVars->realtime;
//
//			auto cycle_updated = false;//(lag_data->last_cycle_update == lag_data->anim_layers[11].m_flCycle) || (lag_data->last_cycle_update == 0.f);
//
//			if (cycle_updated && ((cheat::main::shots_fired[idx] % 5) < 4))
//			{
//				step = 0.f;
//
//				lag_data->last_cycle_update = (lag_data->anim_layers[11].m_flCycle + fabs(lag_data->last_cycle_update - lag_data->anim_layers[11].m_flCycle) * 2);
//			}
//			else {
//				if ((Source::m_pGlobalVars->realtime - lag_data->last_time_balanced) < 0.5f)
//				{
//					auto pDelta = Math::normalize_angle(m_player->m_flLowerBodyYawTarget() - m_player->m_angEyeAngles().y);
//
//					if (fabs(pDelta) > 5.f) {
//						auto side = pDelta < 1.f;
//
//						if (side)
//							step = v59;
//						else
//							step = -v59;
//
//						cheat::features::aaa.player_resolver_records[idx].fakeangles = true;
//					}
//					else
//						step = 0.f;
//				}
//				else {
//					auto delta = (state->m_flCurrentTorsoYaw - m_player->m_angEyeAngles().y);
//
//					step = Math::clamp(delta, -34.f, 34.f);
//
//					if ((cheat::main::shots_fired[idx] % 3 != 0) || !lag_data->use_min_delta) {
//
//						if (lag_data->max_delta)
//							step = copysign(v59, delta);
//
//						lag_data->max_delta = (cheat::main::shots_fired[idx] % 3 != 0);
//
//						lag_data->use_min_delta = false;
//					}
//				}
//			}
//
//			//if (lag_data->previous_angles.y < FLT_MAX && history % 2 == 0)
//			//	step = Math::normalize_angle(m_player->m_angEyeAngles().y - lag_data->previous_angles.y);
//
//			state->m_flGoalFeetYaw = fmod(Math::NormalizeYaw(m_player->m_angEyeAngles().y + step), 360.0f); // from [-180;180] to [0, 360]
//
//			if (state->m_flGoalFeetYaw < 0.0f)
//				state->m_flGoalFeetYaw += 360.0f;
//
//			//QAngle angles = m_player->get_abs_eye_angles();
//			auto angles_y = Math::normalize_angle(state->m_flGoalFeetYaw);
//			m_player->set_abs_angles(QAngle(0, angles_y, 0));
//
//			lag_data->last_time_standed = Source::m_pGlobalVars->realtime;
//
//			//float absEyeDelta = Math::NormalizeYaw(state->m_flGoalFeetYaw - m_player->m_angEyeAngles().y);
//			//float pose = absEyeDelta;
//
//			// body_yaw
//			//m_player->m_flPoseParameter()[11] = (pose / 120.0f) + 0.5f;
//			//float pose = 0.0f; //not initialized?
//			/*if (absEyeDelta < 0.0f || *(float*)((uintptr_t)state + 0x334) == 0.0f) {
//			if (*(float*)((uintptr_t)state + 0x330) != 0.0f)
//			pose = (absEyeDelta / *(float*)((uintptr_t)state + 0x330)) * -58.0f;
//			}
//			else {
//			pose = (absEyeDelta / *(float*)((uintptr_t)state + 0x334)) * 58.0f;
//			}*/
//
//			//using poseParametorFN = mstudioposeparamdesc_t * (__thiscall*)(CStudioHdr*, int);
//			//static poseParametorFN pPoseParametor = (poseParametorFN)Memory::Scan("client.dll", "55 8B EC 8B 45 08 57 8B F9 8B 4F 04 85 C9 75 15 8B");
//
//			//CStudioHdr* pStudioHdr = *(CStudioHdr**)((uintptr_t)m_player + 0x294C);//Source::m_pModelInfo->GetStudioModel(model);
//
//			///auto absRotation = angles_y;
//
//			//if (state->m_flSpeed2D > 0.1f || std::fabsf(state->m_flUpSpeed) > 100.0f) {
//			//	absRotation = cheat::features::antiaimbot.some_func(state->m_flEyeYaw, state->m_flGoalFeetYaw,
//			//		(((*(float *)(uintptr_t(state) + 0x11C)) * 20.0f) + 30.0f) * Source::m_pGlobalVars->interval_per_tick);
//			//}
//			//else {
//			//	absRotation = cheat::features::antiaimbot.some_func(lby, state->m_flGoalFeetYaw, Source::m_pGlobalVars->interval_per_tick * 100.0f);
//			//}
//
//			////float absEyeDelta = Math::normalize_angle(eyeAngles.y - absRotation);
//			//float pose = 0.0f;
//			//if (absEyeDelta <= step) {
//			//	pose = -step;
//			//	if (absEyeDelta >= -step)
//			//		pose = absEyeDelta;
//			//}
//			//else {
//			//	pose = step;
//			//}
//
//
//			//// body_yaw
//			///m_player->m_flPoseParameter()[11] = (pose / 120.0f) + 0.5f;
//
//			//float move_yaw = (m_player->m_flPoseParameter()[7] * 360.0f - 180.0f) - (absRotation - state->m_flGoalFeetYaw);
//			//if (move_yaw <= 180.0f) {
//			//	if (move_yaw < -180.0f)
//			//		move_yaw = move_yaw + 360.0f;
//			//}
//			//else {
//			//	move_yaw = move_yaw - 360.0f;
//			//}
//			//m_player->m_flPoseParameter()[7] = (move_yaw / 360.0f) + 0.5f;
//		}
//	}
//
//	//(*(QAngle*)(uintptr_t(m_player) + 0x128)).y = m_player->get_abs_eye_angles().y;
//}

void c_resolver::work()
{
	for (auto idx = 1; idx < Source::m_pGlobalVars->maxClients; idx++)
	{
		auto entity = (C_BasePlayer*)Source::m_pEntList->GetClientEntity(idx);

		if (entity == nullptr ||
			!entity->IsPlayer())
			continue;

		if (entity->m_iHealth() <= 0 ||
			entity->m_iTeamNum() == cheat::main::local()->m_iTeamNum()) {

			cheat::main::shots_fired[idx-1] = 0;

			//if (!move_logs[idx-1].empty())
			//	move_logs[idx-1].clear();

			continue;
		}

		//resolve(entity, );
	}
}