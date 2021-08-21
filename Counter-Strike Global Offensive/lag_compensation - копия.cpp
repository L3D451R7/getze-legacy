#include "aimbot.hpp"
#include "source.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "hooked.hpp"
#include "math.hpp"
#include "displacement.hpp"
#include "lag_compensation.hpp"
#include "autowall.hpp"
#include "angle_resolver.hpp"
#include "game_movement.h"
#include "anti_aimbot.hpp"
#include <unordered_map>
#include <algorithm>
#include "rmenu.hpp"

#define LAG_COMPENSATION_TICKS 18
#define EFL_DIRTY_ABSVELOCITY (1<<12)

void c_lagcomp::sync_animations(C_BasePlayer* entity)
{
	if (!cheat::Cvars.RageBot_Enable.GetValue())
		return;

	//for (auto idx = 1; idx <= Source::m_pGlobalVars->maxClients; idx++)
	//{
	//	auto entity = (C_BasePlayer*)Source::m_pEntList->GetClientEntity(idx);

	//	if (!entity ||
	//		!entity->IsPlayer() ||
	//		entity->IsDormant() ||
	//		entity->m_iHealth() <= 0 ||
	//		entity->m_iTeamNum() == cheat::main::local->m_iTeamNum()
	//		) continue;

		if (/*!should_lag_compensate(entity) || */entity == nullptr)
			return;

		auto player_index = entity->entindex() - 1;
		auto player_record = &records[player_index];

		if (player_record->m_Tickrecords.size() < 2)
			return;

		auto last_record = player_record->m_Tickrecords.front();
		auto prev_record = player_record->m_Tickrecords.at(1);

		const auto m_lag = TIME_TO_TICKS(last_record.simulation_time - prev_record.simulation_time);

		// fix velocity.
		// https://github.com/VSES/SourceEngine2007/blob/master/se2007/game/client/c_baseplayer.cpp#L659
		if (m_lag != 0) {
			// get pointer to previous record.
			entity->m_vecVelocity() = (last_record.origin - prev_record.origin) * (1.f / m_lag);
		}

		if (last_record.entity_flags & 1
			&& !(prev_record.entity_flags & 1)
			&& (((last_record.simulation_time - prev_record.simulation_time) / Source::m_pGlobalVars->interval_per_tick) + 0.5) > 2)
		{
			entity->m_fFlags() |= FL_ONGROUND;
		}

		auto old_curtime = Source::m_pGlobalVars->curtime;
		auto old_frametime = Source::m_pGlobalVars->frametime;
		C_AnimationLayer backup_layers[15];

		std::memcpy(backup_layers, entity->animation_layers_ptr(), 0x38 * entity->get_animation_layers_count());

		Source::m_pGlobalVars->curtime = *(float*)(uintptr_t(entity) + Engine::Displacement::DT_BasePlayer::m_flSimulationTime + 4) + Source::m_pGlobalVars->interval_per_tick;
		Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;

		auto anim_state = entity->get_animation_state();

		if (anim_state) {
			//*(int*)((uintptr_t)anim_state + 96) = 0;

			if (anim_state->last_anim_upd_tick == Source::m_pGlobalVars->framecount)
				anim_state->last_anim_upd_tick = Source::m_pGlobalVars->framecount - 1;
		}

		//entity->get_ragdoll_pos() = old_ragpos;
		entity->update_clientside_animations();

		std::memcpy(entity->animation_layers_ptr(), backup_layers, 0x38 * entity->get_animation_layers_count());

		Source::m_pGlobalVars->curtime = old_curtime;
		Source::m_pGlobalVars->frametime = old_frametime;

		static auto r_jiggle_bones = Source::m_pCvar->FindVar("r_jiggle_bones");

		const auto jiggleBonesBackup = r_jiggle_bones->GetInt();
		r_jiggle_bones->SetValue(0);

		*(int*)((uintptr_t)entity + 0xA30) = 0;
		*(int*)((uintptr_t)entity + 0x269C) = 0;

		const int backup_effects = *(int*)((uintptr_t)entity + 0xEC);
		*(int*)((uintptr_t)entity + 0xEC) |= 8;

		entity->SetupBones(nullptr, -1, 0x7FF00, entity->m_flSimulationTime());

		*(int*)((uintptr_t)entity + 0xEC) = backup_effects;
		r_jiggle_bones->SetValue(jiggleBonesBackup);
	//}
}

void c_lagcomp::get_interpolation()
{
	static auto cl_interp = Source::m_pCvar->FindVar("cl_interp");
	static auto cl_interp_ratio = Source::m_pCvar->FindVar("cl_interp_ratio");
	static auto sv_client_min_interp_ratio = Source::m_pCvar->FindVar("sv_client_min_interp_ratio");
	static auto sv_client_max_interp_ratio = Source::m_pCvar->FindVar("sv_client_max_interp_ratio");
	static auto cl_updaterate = Source::m_pCvar->FindVar("cl_updaterate");
	static auto sv_minupdaterate = Source::m_pCvar->FindVar("sv_minupdaterate");
	static auto sv_maxupdaterate = Source::m_pCvar->FindVar("sv_maxupdaterate");

	auto updaterate = std::clamp(cl_updaterate->GetFloat(), sv_minupdaterate->GetFloat(), sv_maxupdaterate->GetFloat());
	auto lerp_ratio = std::clamp(cl_interp_ratio->GetFloat(), sv_client_min_interp_ratio->GetFloat(), sv_client_max_interp_ratio->GetFloat());
	cheat::main::lerp_time = std::clamp(lerp_ratio / updaterate, cl_interp->GetFloat(), 1.0f);
}

/*
  CAnimationLayer animlayer[15];
  memcpy(animlayer, player->animlayerptr(), 56 * player->animlayercount());

  if ( animstate)
  {
	if (animstate->LastUpdateTick == Globals->TickCount)
		animstate->LastUpdateTick = Globals->TickCount - 1;

    *(bool*)((DWORD)g_ClientState + 0x4D40) = true;
	player->bClientsideAnimations() = true;
    player->UpdateClientSideAnimation();
	player->bClientsideAnimations() = false;
	*(bool*)((DWORD)g_ClientState + 0x4D40) = false;
  }

  memcpy(player->animlayerptr(), animlayer, 56 * player->animlayercount());
*/

void c_lagcomp::store_record_data(C_BasePlayer* entity, C_Tickrecord* record_data)
{
	if (entity == nullptr || entity->get_animation_state() == nullptr)
		return;

	//entity->set_abs_origin(entity->m_vecOrigin());
	//entity->force_bone_rebuild();
	//entity->SetupBones(record_data->matrixes, 128, 0x100, entity->m_flSimulationTime());
	//entity->SetupFuckingBones(record_data->matrixes, 128, 0x100, record_data->simulation_time, &record_data->eye_angles);

	/*const int backup_effects = *(int*)((uintptr_t)entity + 0xEC);
	*(int*)((uintptr_t)entity + 0xEC) |= 8;
	auto absorg = entity->get_abs_origin();
	entity->set_abs_origin(entity->m_vecOrigin());
	entity->SetupFuckingBones(record_data->matrixes, 128, 0x100, entity->m_flSimulationTime());
	*(int*)((uintptr_t)entity + 0xEC) = backup_effects;
	entity->set_abs_origin(absorg);*/

	auto activity = entity->get_sec_activity(entity->get_animation_layer(1).m_nSequence);
	//auto shot_bt = ((activity >= ACT_CSGO_FIRE_PRIMARY && activity <= ACT_CSGO_FIRE_SECONDARY_OPT_2) && entity->get_animation_layer(1).m_flWeight > 0.01f && entity->get_animation_layer(1).m_flCycle < 0.05f) || (entity->get_weapon() && entity->get_weapon()->m_Activity() == 208);

	auto priority = ((activity == ACT_CSGO_RELOAD) && entity->get_animation_layer(1).m_flWeight > 0.001f && entity->get_animation_layer(1).m_flCycle < 1.f);

	if (cheat::features::aaa.player_resolver_records[entity->entindex() - 1].did_lby_flick) {
		record_data->shot_this_tick = true;
		record_data->type = RECORD_SHOT;
		record_data->shot_time = entity->m_flSimulationTime();
	}//ACT_CSGO_FLASHBANG_REACTION,
	else if (priority)
		record_data->type = RECORD_PRIORITY;
	
	//if (record_data->shot_this_tick) {
	//	float absEyeDelta = Math::NormalizeYaw(entity->get_animation_state()->m_flGoalFeetYaw - entity->m_angEyeAngles().y) / 2.f;
	//	entity->m_flPoseParameter()[11] = (absEyeDelta / 120.0f) + 0.5f;
	//}

	//entity->force_bone_rebuild();
	//entity->SetupBonesEx();
	
	std::memcpy(record_data->matrixes, entity->m_CachedBoneData().Base(), entity->GetBoneCount() * sizeof(matrix3x4_t));

	if (record_data->type == RECORD_NORMAL && records[entity->entindex()-1].m_Tickrecords.size() > 1) {
		record_data->type = ((fabs(entity->m_angEyeAngles().x) < 45.f && fabs(records[entity->entindex() - 1].m_Tickrecords.at(1).eye_angles.x) > 45.f) ? RECORD_PRIORITY : RECORD_NORMAL);
	}

	record_data->origin = entity->m_vecOrigin();
	record_data->abs_origin = entity->get_abs_origin();
	record_data->velocity = entity->m_vecVelocity();
	record_data->object_mins = entity->OBBMins();
	record_data->object_maxs = entity->OBBMaxs();
	record_data->eye_angles = entity->m_angEyeAngles();
	record_data->abs_eye_angles = entity->get_abs_eye_angles();
	record_data->sequence = entity->m_nSequence();
	record_data->entity_flags = entity->m_fFlags();
	record_data->simulation_time = entity->m_flSimulationTime();
	record_data->lower_body_yaw = entity->m_flLowerBodyYawTarget();
	record_data->cycle = entity->m_flCycle();
	record_data->dormant = entity->IsDormant();
	record_data->anim_velocity = entity->m_vecVelocity();
	record_data->ientity_flags = entity->m_iEFlags();
	record_data->duck_amt = entity->m_flDuckAmount();
	record_data->bones_count = entity->GetBoneCount();
	record_data->lby_delta = 0.f;// cheat::features::aaa.sub_59B13C30(entity->get_animation_state());
	record_data->skin = entity->m_nSkin();
	record_data->body = entity->m_nBody();

	record_data->pose_paramaters = entity->m_flPoseParameter();
	std::memcpy(record_data->anim_layers, entity->animation_layers_ptr(), 0x38 * entity->get_animation_layers_count());

	//record_data->shot_this_tick = (record_data->anim_layers[1].m_nSequence == 91 || record_data->anim_layers[1].m_nSequence == 93) && record_data->anim_layers[1].m_flWeight > 0.00f;

	record_data->tick_count = Source::m_pGlobalVars->tickcount;
	//record_data->orig_bonematrix_ptr = *entity->dwBoneMatrix();

	record_data->data_filled = true;
}

void c_lagcomp::create_lby_record(C_BasePlayer* entity, C_Tickrecord* record_data)
{
	if (entity == nullptr || entity->get_animation_state() == nullptr)
		return;

	//entity->force_bone_rebuild();
	//entity->SetupFuckingBones(record_data->matrixes, 128, 0x100, record_data->simulation_time, &record_data->eye_angles);

	/*const int backup_effects = *(int*)((uintptr_t)entity + 0xEC);
	*(int*)((uintptr_t)entity + 0xEC) |= 8;
	auto absorg = entity->get_abs_origin();
	entity->set_abs_origin(entity->m_vecOrigin());
	entity->SetupFuckingBones(record_data->matrixes, 128, 0x100, entity->m_flSimulationTime());
	*(int*)((uintptr_t)entity + 0xEC) = backup_effects;
	entity->set_abs_origin(absorg);*/

	auto activity = entity->get_sec_activity(entity->get_animation_layer(1).m_nSequence);
	auto activity9 = entity->get_sec_activity(entity->get_animation_layer(9).m_nSequence);
	//auto shot_bt = ((activity >= ACT_CSGO_FIRE_PRIMARY && activity <= ACT_CSGO_FIRE_SECONDARY_OPT_2) && entity->get_animation_layer(1).m_flWeight > 0.01f && entity->get_animation_layer(1).m_flCycle < 0.05f) || (entity->get_weapon() && entity->get_weapon()->m_Activity() == 208);

	auto priority = ((activity == ACT_CSGO_RELOAD) && entity->get_animation_layer(1).m_flWeight > 0.001f && entity->get_animation_layer(1).m_flCycle < 1.f) || ((activity9 == ACT_CSGO_FLASHBANG_REACTION) && entity->get_animation_layer(9).m_flWeight > 0.001f && entity->get_animation_layer(9).m_flCycle < 1.f);

	if (cheat::features::aaa.player_resolver_records[entity->entindex()-1].did_lby_flick) {
		record_data->shot_this_tick = true;
		record_data->type = RECORD_SHOT;
		record_data->shot_time = entity->m_flSimulationTime();
	}//ACT_CSGO_FLASHBANG_REACTION,
	else if (priority)
		record_data->type = RECORD_PRIORITY;

	//if (record_data->shot_this_tick) {
	//	float absEyeDelta = Math::NormalizeYaw(entity->get_animation_state()->m_flGoalFeetYaw - entity->m_angEyeAngles().y) * 0.3f;
	//	entity->m_flPoseParameter()[11] = (absEyeDelta / 120.0f) + 0.5f;
	//}

	//entity->force_bone_rebuild();
	//entity->SetupBonesEx();

	std::memcpy(record_data->matrixes, entity->m_CachedBoneData().Base(), entity->GetBoneCount() * sizeof(matrix3x4_t));

	if (record_data->type == RECORD_NORMAL && records[entity->entindex() - 1].m_Tickrecords.size() > 1) {
		record_data->type = ((fabs(entity->m_angEyeAngles().x) < 45.f && fabs(records[entity->entindex() - 1].m_Tickrecords.at(1).eye_angles.x) > 45.f) ? RECORD_PRIORITY : RECORD_NORMAL);
	}

	//record_data->type = RECORD_NORMAL;
	record_data->origin = entity->m_vecOrigin();
	record_data->abs_origin = entity->get_abs_origin();
	record_data->velocity = entity->m_vecVelocity();
	record_data->object_mins = entity->OBBMins();
	record_data->object_maxs = entity->OBBMaxs();
	//record_data->eye_angles = entity->m_angEyeAngles();
	//record_data->abs_eye_angles = entity->get_abs_eye_angles();
	record_data->sequence = entity->m_nSequence();
	record_data->entity_flags = entity->m_fFlags();
	//record_data->simulation_time = entity->m_flSimulationTime();
	record_data->lower_body_yaw = entity->m_flLowerBodyYawTarget();
	record_data->cycle = entity->m_flCycle();
	record_data->dormant = entity->IsDormant();
	record_data->anim_velocity = entity->m_vecVelocity();
	record_data->skin = entity->m_nSkin();
	record_data->body = entity->m_nBody();
	//record_data->ientity_flags = entity->m_iEFlags();

	record_data->duck_amt = entity->m_flDuckAmount();
	record_data->bones_count = entity->GetBoneCount();
	record_data->lby_delta = 0.f;

	record_data->pose_paramaters = entity->m_flPoseParameter();
	std::memcpy(record_data->anim_layers, entity->animation_layers_ptr(), 0x38 * entity->get_animation_layers_count());

	//record_data->shot_this_tick = (record_data->anim_layers[1].m_nSequence == 91 || record_data->anim_layers[1].m_nSequence == 93) && record_data->anim_layers[1].m_flWeight > 0.00f;

	record_data->tick_count = Source::m_pGlobalVars->tickcount;
	record_data->data_filled = true;
	records[entity->entindex() - 1].m_Tickrecords.push_front(*record_data);
}

void c_lagcomp::apply_record_data(C_BasePlayer* entity, C_Tickrecord* record_data, bool backup, bool force_everything)
{
	if (entity == nullptr || !record_data->data_filled || !entity->get_animation_state())
		return;

	//*(matrix3x4_t**)((uintptr_t)entity + 0x2910) = &record_data->matrixes[0];

	std::memcpy(entity->m_CachedBoneData().Base(), record_data->matrixes, record_data->bones_count * sizeof(matrix3x4_t));
	entity->GetBoneCount() = record_data->bones_count;
	entity->m_vecOrigin() = record_data->origin;
	entity->m_vecVelocity() = record_data->velocity;
	entity->OBBMins() = record_data->object_mins;
	entity->OBBMaxs() = record_data->object_maxs;
	entity->m_angEyeAngles() = record_data->eye_angles;
	//entity->get_abs_eye_angles() = record_data->abs_eye_angles;
	//entity->m_nSequence() = record_data->sequence;
	entity->m_fFlags() = record_data->entity_flags;
	//entity->m_flSimulationTime() = record_data->simulation_time;
	//entity->get_animation_state()->m_flDuckAmount = entity->m_flDuckAmount() = record_data->duck_amt;
	//entity->m_iEFlags() = record_data->ientity_flags;
	entity->m_flLowerBodyYawTarget() = record_data->lower_body_yaw;
	//entity->m_flCycle() = record_data->cycle;

	/*if (force_everything) {
		entity->m_flPoseParameter() = record_data->pose_paramaters;
		std::memcpy(entity->animation_layers_ptr(), record_data->anim_layers, 0x38 * entity->get_animation_layers_count());
	}*/

	entity->set_abs_angles(QAngle(0,record_data->abs_eye_angles.y,0));
	entity->set_abs_origin(/*(backup ? record_data->abs_origin : */record_data->origin/*)*/);

	//entity->force_bone_rebuild();
	//entity->SetupBones(0, -1, 0x100, 0);

	/*const int backup_effects = *(int*)((uintptr_t)entity + 0xEC);
	*(int*)((uintptr_t)entity + 0xEC) |= 8;
	auto absorg = entity->get_abs_origin();
	entity->set_abs_origin(entity->m_vecOrigin());
	entity->SetupFuckingBones(record_data->matrixes, 128, 0x100, entity->m_flSimulationTime());
	*(int*)((uintptr_t)entity + 0xEC) = backup_effects;
	entity->set_abs_origin(absorg);*/

	//entity->GetBoneCount() = record_data->bones_count;
	//std::memcpy(entity->m_CachedBoneData().Base(), record_data->matrixes, record_data->bones_count * sizeof(matrix3x4_t));

	//std::memcpy(entity->dwBoneMatrixptr(), record_data->matrixes, sizeof(matrix3x4_t) * 128);

	//if (backup)
	//	*entity->dwBoneMatrix() = (matrix3x4_t*)record_data->orig_bonematrix_ptr;
	//else
	//	*entity->dwBoneMatrix() = (matrix3x4_t*)&record_data->matrixes;
}

void c_lagcomp::simulate_movement(C_Simulationdata* data)
{
	trace_t trace;
	CTraceFilter filter;
	filter.pSkip = cheat::main::local;

	auto sv_gravity = Source::m_pCvar->FindVar("sv_gravity")->GetInt();
	auto sv_jump_impulse = Source::m_pCvar->FindVar("sv_jump_impulse")->GetFloat(); // math.sqrt(91200) = 301.1
	auto gravity_per_tick = sv_gravity * Source::m_pGlobalVars->interval_per_tick;
	auto predicted_origin = data->origin;

	if (data->on_ground && !data->extrapolation)
		data->velocity.z -= gravity_per_tick;

	predicted_origin += data->velocity * Source::m_pGlobalVars->interval_per_tick;

	Ray_t ray;
	ray.Init(data->origin, predicted_origin, data->entity->OBBMins(), data->entity->OBBMaxs());

	Source::m_pEngineTrace->TraceRay(ray, CONTENTS_SOLID, &filter, &trace);

	if (trace.fraction == 1.f)
		data->origin = predicted_origin;

	Ray_t ray2;
	ray2.Init(data->origin, data->origin - Vector(0.f, 0.f, 2.f), data->entity->OBBMins(), data->entity->OBBMaxs());

	Source::m_pEngineTrace->TraceRay(ray2, CONTENTS_SOLID, &filter, &trace);

	data->on_ground = trace.fraction == 0.f;
}

void c_lagcomp::predict_player(C_BasePlayer* entity, C_Tickrecord* current_record, C_Tickrecord* next_record)
{
	// Create Simulation Data
	C_Simulationdata simulation_data;

	simulation_data.entity = entity;
	simulation_data.origin = current_record->origin;
	simulation_data.velocity = current_record->velocity;
	simulation_data.on_ground = current_record->entity_flags & FL_ONGROUND;
	simulation_data.data_filled = true;

	// Calculate Delta's
	auto simulation_time_delta = current_record->simulation_time - next_record->simulation_time;
	auto simulation_time_clamped_delta = Math::clamp(simulation_time_delta, Source::m_pGlobalVars->interval_per_tick, 1.0f);
	auto delta_ticks_clamped = Math::clamp(TIME_TO_TICKS(simulation_time_clamped_delta), 1, 15);
	auto delta_ticks = Math::clamp(TIME_TO_TICKS(simulation_time_delta), 1, 15);

	// Calculate movement delta
	auto current_velocity_angle = RAD2DEG(atan2(current_record->velocity.y, current_record->velocity.x));
	auto next_velocity_angle = RAD2DEG(atan2(next_record->velocity.y, next_record->velocity.x));
	auto velocity_angle_delta = Math::NormalizeYaw(current_velocity_angle - next_velocity_angle);
	auto velocity_movement_delta = velocity_angle_delta / simulation_time_delta;

	if (delta_ticks > 0 && simulation_data.data_filled)
	{
		for (; delta_ticks >= 0; delta_ticks -= delta_ticks_clamped)
		{
			auto ticks_left = delta_ticks_clamped;

			do
			{
				simulate_movement(&simulation_data);
				current_record->simulation_time += Source::m_pGlobalVars->interval_per_tick;

				--ticks_left;
			} while (ticks_left);
		}

		current_record->origin = simulation_data.origin;
		current_record->abs_origin = simulation_data.origin;
	}
}

struct simulation_data {
public:
	simulation_data(C_BasePlayer* ent, C_Tickrecord* current);
	simulation_data(C_BasePlayer* ent);
public:
	C_BasePlayer * m_ent;
	Vector m_origin, m_velocity, m_maxs, m_mins;
	float m_move_angle;
	int m_flags;
};

simulation_data::simulation_data(C_BasePlayer* ent, C_Tickrecord * current)
{
	this->m_ent = ent;
	this->m_origin = current->origin;
	this->m_velocity = current->velocity;
	this->m_move_angle = RAD2DEG(std::atan2(current->velocity.y, current->velocity.x));
	this->m_mins = current->object_mins;
	this->m_maxs = current->object_maxs;
	this->m_flags = current->entity_flags;
}

simulation_data::simulation_data(C_BasePlayer* ent)
{
	this->m_ent = ent;
	this->m_origin = ent->get_abs_origin();
	this->m_velocity = ent->m_vecVelocity();
	this->m_move_angle = RAD2DEG(std::atan2(ent->m_vecVelocity().y, ent->m_vecVelocity().x));
	this->m_mins = ent->OBBMins();
	this->m_maxs = ent->OBBMaxs();
	this->m_flags = ent->m_fFlags();
}

bool c_lagcomp::extrapolate(C_BasePlayer * ent, const int ticks, C_Tickrecord* extrapolated_record)
{
	if (ticks == 0)
		return false;

	if (!extrapolated_record)
		return false;

	auto track = &records[ent->entindex() - 1];

	if (!track)
		return false;

	if (track->m_Tickrecords.size() < 2)
		return false;

	auto current_rec = &track->m_Tickrecords.front();
	auto previous_rec = &track->m_Tickrecords.at(1);

	if (!current_rec || !previous_rec)
		return false;

	simulation_data data(ent, current_rec);

	const auto cur_move_angle = data.m_move_angle;
	const auto prev_move_angle = RAD2DEG(std::atan2(previous_rec->velocity.y, previous_rec->velocity.x));
	auto move_angle_delta = (cur_move_angle - prev_move_angle) / (current_rec->simulation_time - previous_rec->simulation_time);
	move_angle_delta = Math::normalize_angle(move_angle_delta);

	// note: you could improve accuracy slightly by accounting for acceleration,
	// I have the calcs done for my lag fix but it seems unnecessary for this.
	//const auto move_speed = data.m_velocity.Length2D();
	const auto move_speed = ((current_rec->origin - previous_rec->origin) * (1.f / (current_rec->simulation_time - previous_rec->simulation_time))).Length2D();
	const auto should_jump = !(current_rec->entity_flags & FL_ONGROUND) || !(previous_rec->entity_flags & FL_ONGROUND);

	for (auto i = 0; i < ticks; i++)
	{
		// add the turn amount in a single tick to our current angle.
		data.m_move_angle += move_angle_delta * Source::m_pGlobalVars->interval_per_tick;
		data.m_move_angle = Math::normalize_angle(data.m_move_angle);

		// adjust our movement direction to reflect our current angle.
		data.m_velocity.x = std::cos(DEG2RAD(data.m_move_angle)) * move_speed;
		data.m_velocity.y = std::sin(DEG2RAD(data.m_move_angle)) * move_speed;

		if (!(data.m_flags & FL_ONGROUND)) {
			// if we're in the air, subtract gravity.
			static auto sv_gravity = Source::m_pCvar->FindVar("sv_gravity");
			data.m_velocity.z -= (sv_gravity->GetFloat() * Source::m_pGlobalVars->interval_per_tick);
		}
		else if (should_jump) {
			static auto sv_jump_impulse = Source::m_pCvar->FindVar("sv_jump_impulse");
			data.m_velocity.z = sv_jump_impulse->GetFloat();
		}

		auto src = data.m_origin;
		auto end = src + (data.m_velocity * Source::m_pGlobalVars->interval_per_tick);

		Ray_t ray;
		ray.Init(src, end, data.m_mins, data.m_maxs);

		trace_t trace;
		CTraceFilter filter;
		filter.pSkip = ent;
		//TraceLine(src, end, CONTENTS_SOLID, ent, &trace);
		Source::m_pEngineTrace->TraceRay(ray, CONTENTS_SOLID, &filter, &trace);
		// fix for grounded walking entities colliding with ground?
		const auto along_ground = (data.m_flags & FL_ONGROUND) && trace.fraction != 1.f && !should_jump && data.m_velocity.Dot(trace.plane.normal) <= FLT_EPSILON;

		if (along_ground)
		{
			const auto mod = Vector(0, 0, 1);
			ray.Init(src + mod, end + mod, data.m_mins, data.m_maxs);
			Source::m_pEngineTrace->TraceRay(ray, CONTENTS_SOLID, &filter, &trace);
		}
		// if we hit a plane, reflect the vector along the plane in the nearest non-colliding direction.
		if (trace.fraction != 1.f) {
			for (auto c = 0U; c < 2; c++) {
				data.m_velocity -= trace.plane.normal * data.m_velocity.Dot(trace.plane.normal);
				const auto dot = data.m_velocity.Dot(trace.plane.normal);
				if (dot < 0.f)
					data.m_velocity -= (trace.plane.normal * dot);
				end = trace.endpos + (data.m_velocity * (Source::m_pGlobalVars->interval_per_tick * (1.f - trace.fraction)));
				ray.Init(trace.endpos, end, data.m_mins, data.m_maxs);
				Source::m_pEngineTrace->TraceRay(ray, CONTENTS_SOLID, &filter, &trace);
				if (trace.fraction == 1.f)
					break;
			}
		}

		//Vector3 s[2];
		//if (Hack.Misc->WorldToScreen(data.m_origin, s[0]) && Hack.Misc->WorldToScreen(trace.endpos, s[1]))
		//	Hack.Drawings->DrawLine(s[0].x, s[0].y, s[1].x, s[1].y, Color::Red());
		data.m_origin = trace.endpos;
		// we have to check if we hit ground here, in case we need to simulate a jump next simulated tick.
		end = trace.endpos;
		//end.z -= 2.f;

		ray.Init(data.m_origin, end, data.m_mins, data.m_maxs);
		Source::m_pEngineTrace->TraceRay(ray, CONTENTS_SOLID, &filter, &trace);

		data.m_flags &= ~FL_ONGROUND;

		if (trace.DidHit() && trace.plane.normal.z > 0.7f)
			data.m_flags |= FL_ONGROUND;

		current_rec->simulation_time += Source::m_pGlobalVars->interval_per_tick;
	}

	// copy the current record over so all the other data is recent.
	*extrapolated_record = *current_rec;

	// set the extrapolated information.
	extrapolated_record->origin = data.m_origin;
	extrapolated_record->velocity = data.m_velocity;
	extrapolated_record->entity_flags = data.m_flags;
	//extrapolated_record->simulation_time += Source::m_pGlobalVars->interval_per_tick

	/*if ((data.m_origin - current_rec->m_origin).Length() < 20.0f)
	{
	printf("predicted-last length3d: %f\n", (data.m_origin - current_rec->m_origin).Length());
	return false;
	}*/
	return true;
}

void c_lagcomp::accelerate_velocity(C_BasePlayer* player, Vector& velocity, Vector new_velocity_angle, Vector old_velocity_angle)
{
	static auto maxspeed = Source::m_pCvar->FindVar("sv_maxspeed");

	auto fl_maxspeed = maxspeed->GetFloat();
	Vector forward, right, up;
	Math::AngleVectors(old_velocity_angle, &forward, &right, &up);

	forward[2] = 0.f;
	right[2] = 0.f;

	forward.Normalize();
	right.Normalize();

	auto wishvel = right * abs(old_velocity_angle.y - new_velocity_angle.y); // note this only works because we assume cstrafe
	wishvel[2] = 0;

	auto wishdir = wishvel;
	auto wishspd = wishdir.Length();

	if (wishspd != 0.f)
		wishdir /= wishspd;

	if (wishspd != 0.f && wishspd > fl_maxspeed)
		wishspd = fl_maxspeed;

	if (wishspd > 30.f)
		wishspd = 30.f;

	const auto currentspeed = velocity.Dot(wishdir);
	const auto addspeed = wishspd - currentspeed;

	if (addspeed <= 0) {
		const auto speed = velocity.Length2D();
		velocity.x = std::cosf(DEG2RAD(new_velocity_angle.y)) * (speed);
		velocity.y = std::sinf(DEG2RAD(new_velocity_angle.y)) * (speed);
		return;
	}

	static ConVar* sv_airaccelerate = Source::m_pCvar->FindVar("sv_accelerate");

	auto accelspeed = sv_airaccelerate->GetFloat() * wishspd * Source::m_pGlobalVars->interval_per_tick;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	const auto speed = velocity.Length2D();
	velocity.x = std::cosf(DEG2RAD(new_velocity_angle.y)) * (speed + accelspeed);
	velocity.y = std::sinf(DEG2RAD(new_velocity_angle.y)) * (speed + accelspeed);
}

bool c_lagcomp::is_time_delta_too_large(C_Tickrecord* wish_record)
{
	float correct = 0;

	correct += Source::m_pEngine->GetNetChannelInfo()->GetLatency(0);
	correct += Source::m_pEngine->GetNetChannelInfo()->GetLatency(1);
	correct += cheat::main::lerp_time;
	
	if (cheat::game::pressed_keys[(int)cheat::Cvars.Exploits_fakeduck.GetValue()] && (int)cheat::Cvars.Exploits_fakeduck.GetValue() != 0)
		correct += TICKS_TO_TIME(14 - Source::m_pClientState->m_iChockedCommands);

	auto sv_maxunlag = Source::m_pCvar->FindVar("sv_maxunlag");

	correct = std::clamp(correct, 0.f, sv_maxunlag->GetFloat());

	float deltaTime = std::abs(correct - (Source::m_pGlobalVars->curtime - wish_record->simulation_time));

	if (deltaTime < TICKS_TO_TIME(14))
		return false;
	
	/*if (wish_record->type == RECORD_SHOT) {
		Source::m_pCvar->ConsoleColorPrintf(Color(50, 122, 239), "[EAGLE]");
		Source::m_pCvar->ConsoleDPrintf(" ""shot record time is wrong | "" ");
		Source::m_pCvar->ConsoleColorPrintf(Color(255, 255, 255), "ticks: %d\n", TIME_TO_TICKS(deltaTime));
	}*/

	return true;
}

bool c_lagcomp::is_time_delta_too_large(const float &simulation_time)
{
	float correct = 0;

	correct += Source::m_pEngine->GetNetChannelInfo()->GetLatency(0);
	correct += Source::m_pEngine->GetNetChannelInfo()->GetLatency(1);
	correct += cheat::main::lerp_time;

	if (cheat::game::pressed_keys[(int)cheat::Cvars.Exploits_fakeduck.GetValue()] && (int)cheat::Cvars.Exploits_fakeduck.GetValue() != 0)
		correct += TICKS_TO_TIME(14 - Source::m_pClientState->m_iChockedCommands);

	auto sv_maxunlag = Source::m_pCvar->FindVar("sv_maxunlag");

	correct = std::clamp(correct, 0.f, sv_maxunlag->GetFloat());

	float deltaTime = std::abs(correct - (Source::m_pGlobalVars->curtime - simulation_time));

	if (deltaTime < TICKS_TO_TIME(14))
		return false;

	return true;
}

int c_lagcomp::start_lag_compensation(C_BasePlayer* entity, int wish_tick, C_Tickrecord* output_record)
{
	//if (!should_lag_compensate(entity))
	//	return -1;

	auto player_index = entity->entindex() - 1;
	auto player_record = &records[player_index];

	if (player_record->m_Tickrecords.empty() || (wish_tick + 1 > player_record->m_Tickrecords.size() - 1))
		return -1;

	auto current_record = player_record->m_Tickrecords.at(wish_tick);
	auto next_record = player_record->m_Tickrecords.at(wish_tick + 1);

	if (!current_record.data_filled || !next_record.data_filled || wish_tick > 0 && is_time_delta_too_large(&current_record))
		return -1;

	if (wish_tick == 0 && (current_record.origin - next_record.origin).LengthSquared() > 4096.f && !current_record.origin.IsZero() && !next_record.origin.IsZero() && current_record.tick_count != Source::m_pGlobalVars->tickcount) {

		C_Tickrecord extrapolated;
		C_Tickrecord backup;
		
		auto simulation_time_delta = current_record.simulation_time - next_record.simulation_time;
		auto simulation_time_clamped_delta = Math::clamp(simulation_time_delta, Source::m_pGlobalVars->interval_per_tick, 1.0f);
		auto delta_ticks = Math::clamp(TIME_TO_TICKS(simulation_time_clamped_delta), 1, 16);

		store_record_data(entity, &extrapolated);

		if (delta_ticks > TIME_TO_TICKS(Source::m_pEngine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING)) && extrapolate(entity, delta_ticks, &extrapolated)) {

			store_record_data(entity, &backup);

			apply_record_data(entity, &extrapolated);

			entity->force_bone_rebuild();
			entity->SetupBonesEx();

			store_record_data(entity, &extrapolated);

			apply_record_data(entity, &backup);

			*output_record = extrapolated;

			return TIME_TO_TICKS(extrapolated.simulation_time + cheat::main::lerp_time);
		}

		//predict_player(entity, &current_record, &next_record);
	}
	
	//if (wish_tick == 0) {
		//if ((current_record.origin - next_record.origin).LengthSquared() > 4096.f) {
		//	//predict_player(entity, &current_record, &next_record);
		//	float simulationTimeDelta = current_record.simulation_time - next_record.simulation_time;

		//	int simulationTickDelta = std::clamp(TIME_TO_TICKS(simulationTimeDelta), 1, 15);

		//	CUserCmd cmds /*= new CUserCmd*/;

		//	cmds.viewangles = entity->m_angEyeAngles();
		//	Vector Difference = current_record.origin - next_record.origin; Difference.Normalize();
		//	cmds.forwardmove = Difference.x; cmds.sidemove = Difference.y; cmds.upmove = Difference.z;

		//	//static int simulated_choke = 0;
		//	//auto noob_choke = std::clamp(TIME_TO_TICKS(current_record.simulation_time - next_record.simulation_time) - 1, 0, 16);

		//	//if (noob_choke > 0)
		//	//	simulated_choke = noob_choke;
		//	//else
		//	//	simulated_choke++;

		//	for (auto i = 0; i < /*min(simulationTimeDelta, 16)*/simulationTimeDelta; i++) //{
		//		Source::m_pPrediction->RunCommand(entity, &cmds, Source::m_pMoveHelper);
		//	//Source::m_pDebugOverlay->AddBoxOverlay(entity->m_vecOrigin(), Vector(-3, -3, -3), Vector(3, 3, 3), Vector::Zero, 255, i == 0 ? 255 : 0, 0, 255, Source::m_pGlobalVars->interval_per_tick * 2.f);
		////}

		//	//if (simulated_choke == noob_choke)
		//	//	simulated_choke = 0;

		//	C_Tickrecord new_record;
		//	store_record_data(entity, &new_record);

		//	*output_record = new_record;

		//	// Bandage fix so we "restore" to the lagfixed player.
		//	//m_RestoreLagRecord[player->EntIndex()].second.SaveRecord(player);
		//	//*record = m_RestoreLagRecord[player->EntIndex()].second;
		//}
	//}

	if (output_record != nullptr && current_record.data_filled)
		*output_record = current_record;

	return TIME_TO_TICKS(current_record.simulation_time + cheat::main::lerp_time);
}

bool got_update[65];
float flick_time[65];

void c_lagcomp::update_player_record_data(C_BasePlayer* entity)
{
	if (entity == nullptr)
		return;

	auto player_index = entity->entindex() - 1;
	auto player_record = &records[player_index];

	//if (!player_record->m_Tickrecords.empty() && player_record->m_Tickrecords.front().simulation_time >= entity->m_flSimulationTime())
	//{
	//	auto resolve_info = &cheat::features::aaa.player_resolver_records[player_index];
	//	resolve_info->is_shifting = (player_record->m_Tickrecords.front().eye_angles != entity->m_angEyeAngles() || player_record->m_Tickrecords.front().origin != entity->m_vecOrigin());
	//}

	if (entity->m_flSimulationTime() > 0.f && player_record->m_Tickrecords.empty() || player_record->m_Tickrecords.front().simulation_time != entity->m_flSimulationTime())
	{
		auto pasti = player_record->m_Tickrecords;

		update_animations_data(entity, 0);

		//if (entity->m_flSimulationTime() > 0.f && player_record->m_Tickrecords.empty() || player_record->m_Tickrecords.front().simulation_time < entity->m_flSimulationTime()) {

		C_Tickrecord new_record;
		store_record_data(entity, &new_record);

		if (new_record.data_filled)
			player_record->m_Tickrecords.push_front(new_record);
		//}

		if (player_record->m_Tickrecords.size() > 18)
			player_record->m_Tickrecords.resize(18);

		while (!player_record->m_Tickrecords.empty() && is_time_delta_too_large(&player_record->m_Tickrecords.back()))
			player_record->m_Tickrecords.pop_back();
	}
	else
		entity->client_side_animation() = true;
}

void c_lagcomp::start_position_adjustment()
{
	if (!cheat::Cvars.RageBot_AdjustPositions.GetValue())
		return;

	//cheat::main::fast_autostop = false;

	for (auto index = 1; index < Source::m_pGlobalVars->maxClients; index++)
	{
		auto entity = (C_BasePlayer*)Source::m_pEntList->GetClientEntity(index);

		if (!entity || !entity->IsPlayer())
			continue;

		if (entity->IsDead() && entity->m_iTeamNum() != cheat::main::local->m_iTeamNum())
		{
			cheat::main::shots_fired[index - 1] = 0;
			cheat::main::shots_total[index - 1] = 0;
			memset(cheat::features::aaa.player_resolver_records[index - 1].missed_shots, 0, sizeof(int) * 11);
		}

		if (entity->IsDormant() ||
			entity->m_iHealth() <= 0 ||
			entity->IsDead() ||
			entity->m_iTeamNum() == cheat::main::local->m_iTeamNum()
			)
		{
			auto entidx = entity->entindex();

			if (entidx == 0)
				continue;

			if (records[entidx - 1].tick_count != -1)
				records[entidx - 1].reset((entity->IsDormant() && entity->m_iHealth() <= 0));

			got_update[entidx - 1] = false;
			continue;
		}

		//if (!should_lag_compensate(entity))
		//	continue;

		auto player_record = &records[entity->entindex() - 1];

		if (player_record->m_Tickrecords.size() <= 2)
			continue;

		player_record->being_lag_compensated = true;
		start_position_adjustment(entity);
		player_record->being_lag_compensated = false;
	}
}

void c_lagcomp::start_position_adjustment(C_BasePlayer* entity)
{
	auto player_index = entity->entindex() - 1;
	auto player_record = &records[player_index];

	auto local_weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local->m_hActiveWeapon()));

	if (player_record->m_Tickrecords.empty() || !local_weapon)
		return;

	auto has_knife = (local_weapon->m_iItemDefinitionIndex() == weapon_knife_ct || local_weapon->m_iItemDefinitionIndex() == weapon_knife_t);
	auto has_zeus = local_weapon->m_iItemDefinitionIndex() == weapon_taser;

	player_record->backtrack_ticks = 0;
	player_record->tick_count = -1;
	player_record->type = 0;
	player_record->hitbox_position.Set();
	player_record->matrix_valid = false;

	store_record_data(entity, &player_record->restore_record);

	/*if (entity->m_vecVelocity().Length2D() > 20.f)
	{
		auto predicted_data = predict_player_peek(entity, &player_record->m_Tickrecords.front(), &player_record->m_Tickrecords.at(1));

		entity->set_abs_origin(predicted_data.origin);

		matrix3x4_t predicted_bones[128];
		entity->SetupFuckingBones(predicted_bones, 128, 0x100, 0);

		auto chest = cheat::features::aimbot.get_hitbox(entity, HITBOX_CHEST, predicted_bones);

		if (cheat::features::autowall.CanHit(cheat::main::local->GetEyePosition(), chest, cheat::main::local, entity, HITBOX_CHEST) > 40)
			cheat::main::fast_autostop = true;

		apply_record_data(entity, &player_record->restore_record, true);
	}*/

	float max_dmg = 1.f;
	TickrecordType type = RECORD_NORMAL;
	float max_distance = 8196.f;

	if (player_record->m_Tickrecords.front().type > RECORD_NORMAL)
	{
		C_Tickrecord corrected_record;

		auto tick_count = start_lag_compensation(entity, 0, &corrected_record);

		if (tick_count != -1 && corrected_record.data_filled)
		{

			apply_record_data(entity, &corrected_record);

			corrected_record.hitbox_positon = cheat::features::aimbot.get_hitbox(entity, 0, corrected_record.matrixes);
			auto left_leg = cheat::features::aimbot.get_hitbox(entity, 11, corrected_record.matrixes);
			auto chest = cheat::features::aimbot.get_hitbox(entity, HITBOX_PELVIS, corrected_record.matrixes);
			auto right_leg = cheat::features::aimbot.get_hitbox(entity, 12, corrected_record.matrixes);

			//const auto suitable_dmg = (entity->m_iHealth() < cheat::settings.ragebot_min_damage && cheat::settings.ragebot_scale_damage_on_hp ? entity->m_iHealth() : cheat::settings.ragebot_min_damage);

			auto pass = cheat::features::aimbot.can_hit(0, entity, player_record->hitbox_position, corrected_record.matrixes, true);

			if (pass <= 0.f)
				pass = cheat::features::aimbot.can_hit(HITBOX_PELVIS, entity, chest, corrected_record.matrixes);
			if (pass <= 0.f)
				pass = cheat::features::aimbot.can_hit(11, entity, left_leg, corrected_record.matrixes, true);
			if (pass <= 0.f)
				pass = cheat::features::aimbot.can_hit(12, entity, right_leg, corrected_record.matrixes, true);

			//apply_record_data(_entity, restore_record, true);

			if (corrected_record.hitbox_positon.IsZero() || left_leg.IsZero() || chest.IsZero() || right_leg.IsZero())
			{
				player_record->hitbox_position = corrected_record.hitbox_positon;

				if (pass > max_dmg || (corrected_record.type > RECORD_NORMAL && pass > 0)) {
					player_record->tick_count = tick_count;
					player_record->type = corrected_record.type;
					std::memcpy(player_record->matrix, corrected_record.matrixes, sizeof(matrix3x4_t) * 128);
					player_record->matrix_valid = true;

					player_record->backtrack_ticks = 0;
					max_dmg = pass;
					type = (TickrecordType)(corrected_record.type);

					if (type > RECORD_NORMAL && pass > 0.f)
						return;
				}
			}
		}
	}

	// Just normally loop as we would
	for (int index = 0; index < player_record->m_Tickrecords.size(); index++)
	{
		if ((index + 1) > (player_record->m_Tickrecords.size() - 1))
			break;

		auto current_record = &player_record->m_Tickrecords.at(index);

		C_Tickrecord* prev_record = nullptr;

		if (player_record->m_Tickrecords.size() > 2 && index < (player_record->m_Tickrecords.size() - 1)) {
			if (!is_time_delta_too_large(current_record))
				prev_record = &player_record->m_Tickrecords.at(index + 1);
			else
				prev_record = nullptr;
		}

		if (is_time_delta_too_large(current_record))
			continue;

		if (has_knife || has_zeus) {
			C_Tickrecord corrected_record;

			auto tick_count = start_lag_compensation(entity, index, &corrected_record);

			if (tick_count == -1 || !corrected_record.data_filled)
				continue;

			if (index > 0 && prev_record != nullptr && current_record->velocity == prev_record->velocity && current_record->origin == prev_record->origin)
				continue;

			apply_record_data(entity, &corrected_record);

			auto chest = cheat::features::aimbot.get_hitbox(entity, HITBOX_PELVIS, (&corrected_record)->matrixes);
			auto dist = cheat::main::local->m_vecOrigin().Distance(chest);

			if (dist < max_distance)
			{
				max_distance = dist;

				player_record->tick_count = tick_count;
				player_record->type = (&corrected_record)->type;
				std::memcpy(player_record->matrix, (&corrected_record)->matrixes, sizeof(matrix3x4_t) * 128);
				player_record->matrix_valid = true;
				player_record->backtrack_ticks = index;
			}
		}
		else {
			if (current_record->velocity.Length() < .1f) 
			{
				C_Tickrecord corrected_record;

				auto tick_count = start_lag_compensation(entity, index, &corrected_record);

				if (tick_count == -1 || !corrected_record.data_filled)
					continue;

				if (index > 0 && prev_record != nullptr && current_record->type <= RECORD_NORMAL && current_record->velocity == prev_record->velocity && current_record->origin == prev_record->origin && (current_record->eye_angles == prev_record->eye_angles) && (current_record->hitbox_positon == prev_record->hitbox_positon))
					continue;

				apply_record_data(entity, &corrected_record);

				(&corrected_record)->hitbox_positon = cheat::features::aimbot.get_hitbox(entity, 0, (&corrected_record)->matrixes);
				auto left_leg = cheat::features::aimbot.get_hitbox(entity, 11, (&corrected_record)->matrixes);
				auto chest = cheat::features::aimbot.get_hitbox(entity, HITBOX_PELVIS, (&corrected_record)->matrixes);
				auto right_leg = cheat::features::aimbot.get_hitbox(entity, 12, (&corrected_record)->matrixes);

				//const auto suitable_dmg = (entity->m_iHealth() < cheat::settings.ragebot_min_damage && cheat::settings.ragebot_scale_damage_on_hp ? entity->m_iHealth() : cheat::settings.ragebot_min_damage);

				auto pass = cheat::features::aimbot.can_hit(0, entity, player_record->hitbox_position, (&corrected_record)->matrixes, true);

				if (pass <= 0.f)
					pass = cheat::features::aimbot.can_hit(HITBOX_PELVIS, entity, chest, (&corrected_record)->matrixes);
				if (pass <= 0.f)
					pass = cheat::features::aimbot.can_hit(11, entity, left_leg, (&corrected_record)->matrixes, true);
				if (pass <= 0.f)
					pass = cheat::features::aimbot.can_hit(12, entity, right_leg, (&corrected_record)->matrixes, true);

				//apply_record_data(_entity, restore_record, true);

				if ((&corrected_record)->hitbox_positon.IsZero() || left_leg.IsZero() || chest.IsZero() || right_leg.IsZero())
					continue;

				player_record->hitbox_position = (&corrected_record)->hitbox_positon;

				if (pass > max_dmg || ((&corrected_record)->type > RECORD_NORMAL && pass > 0)) {
					player_record->tick_count = tick_count;
					player_record->type = (&corrected_record)->type;
					std::memcpy(player_record->matrix, (&corrected_record)->matrixes, sizeof(matrix3x4_t) * 128);
					player_record->matrix_valid = true;

					player_record->backtrack_ticks = index;
					max_dmg = pass;
					type = (TickrecordType)((&corrected_record)->type);

					if (type > RECORD_NORMAL && pass > 0.f)
						return;
				}
			}
			else {
				C_Tickrecord corrected_record;

				auto tick_count = start_lag_compensation(entity, index, &corrected_record);

				if (tick_count == -1 || !corrected_record.data_filled)
					continue;

				if (index > 0 && prev_record != nullptr && current_record->type <= RECORD_NORMAL && current_record->velocity == prev_record->velocity && current_record->origin == prev_record->origin && (current_record->eye_angles == prev_record->eye_angles) && (current_record->hitbox_positon == prev_record->hitbox_positon))
					continue;

				//if (current_record->type == RECORD_SHOT || current_record->type == RECORD_LBYUPDATE) //if we got some flick/shot tick lets priority it
				//	if (test_and_apply_record(entity, player_record, &player_record->restore_record, &corrected_record, tick_count, index))
				//		return;

				//if (test_and_apply_record(entity, player_record, &player_record->restore_record, &corrected_record, tick_count, index))
				//	return;
				apply_record_data(entity, &corrected_record);

				(&corrected_record)->hitbox_positon = cheat::features::aimbot.get_hitbox(entity, 0, (&corrected_record)->matrixes);
				auto left_leg = cheat::features::aimbot.get_hitbox(entity, 11, (&corrected_record)->matrixes);
				auto chest = cheat::features::aimbot.get_hitbox(entity, HITBOX_PELVIS, (&corrected_record)->matrixes);
				auto right_leg = cheat::features::aimbot.get_hitbox(entity, 12, (&corrected_record)->matrixes);

				//const auto suitable_dmg = (entity->m_iHealth() < cheat::settings.ragebot_min_damage && cheat::settings.ragebot_scale_damage_on_hp ? entity->m_iHealth() : cheat::settings.ragebot_min_damage);

				auto pass = cheat::features::aimbot.can_hit(0, entity, player_record->hitbox_position, (&corrected_record)->matrixes, true);

				if (pass <= 0.f)
					pass = cheat::features::aimbot.can_hit(HITBOX_PELVIS, entity, chest, (&corrected_record)->matrixes);
				if (pass <= 0.f)
					pass = cheat::features::aimbot.can_hit(11, entity, left_leg, (&corrected_record)->matrixes, true);
				if (pass <= 0.f)
					pass = cheat::features::aimbot.can_hit(12, entity, right_leg, (&corrected_record)->matrixes, true);

				//apply_record_data(_entity, restore_record, true);

				if ((&corrected_record)->hitbox_positon.IsZero() || left_leg.IsZero() || chest.IsZero() || right_leg.IsZero())
					continue;

				player_record->hitbox_position = (&corrected_record)->hitbox_positon;

				if (pass > max_dmg || ((&corrected_record)->type > RECORD_NORMAL && pass > 0.f) || (&corrected_record)->type == RECORD_SHOT) {
					player_record->tick_count = tick_count;
					player_record->type = (&corrected_record)->type;
					std::memcpy(player_record->matrix, (&corrected_record)->matrixes, sizeof(matrix3x4_t) * 128);
					player_record->matrix_valid = true;

					player_record->backtrack_ticks = index;
					max_dmg = pass;
					type = (TickrecordType)((&corrected_record)->type);

					if (type > RECORD_NORMAL && pass > 0.f)
						return;
				}

				if (index == 0 && entity->m_vecVelocity().Length() > 100.f && pass > 0.f && (((&corrected_record)->type) > 0 || (&corrected_record)->type == RECORD_SHOT))
					break;
			}
		}
	}
}

void c_lagcomp::finish_position_adjustment()
{
	if (!cheat::Cvars.RageBot_AdjustPositions.GetValue())
		return;

	for (auto index = 1; index < Source::m_pGlobalVars->maxClients; index++)
	{
		auto entity = (C_BasePlayer*)Source::m_pEntList->GetClientEntity(index);

		if (!entity || !entity->IsPlayer())
			continue;

		if (entity->IsDormant() ||
			entity->m_iHealth() <= 0 ||
			entity->IsDead() ||
			entity->m_iTeamNum() == cheat::main::local->m_iTeamNum()
			)
		{
			auto entidx = entity->entindex();

			if (entidx == 0)
				continue;

			records[entidx - 1].reset((entity->IsDormant() && entity->m_iHealth() <= 0));
			got_update[entidx - 1] = false;
			continue;
		}

		finish_position_adjustment(entity);
	}
}

std::unordered_map <int, float> m_simulation;

//void c_lagcomp::update_animations_data(C_BasePlayer* m_player, C_Tickrecord* record) {
//	CCSGOPlayerAnimState* state = m_player->get_animation_state();
//
//	auto idx = m_player->entindex() - 1;
//
//	if (!state || m_player == cheat::main::local)
//		return;
//
//	auto m_records = records[idx].m_Tickrecords;
//
//	//// player respawned.
//	//if (m_player->m_flSpawnTime() != m_spawn[idx]) {
//	//	// reset animation state.
//	//	reset_animstate(state);
//
//	//	// note new spawn time.
//	//	m_spawn[idx] = m_player->m_flSpawnTime();
//	//}
//
//	// backup curtime.
//	const float curtime = Source::m_pGlobalVars->curtime;
//	const float frametime = Source::m_pGlobalVars->frametime;
//	const float lerpamt = Source::m_pGlobalVars->interpolation_amount;
//
//	// set curtime to animtime.
//	// set frametime to ipt just like on the server during simulation.
//	Source::m_pGlobalVars->curtime = m_player->m_anim_time();
//	Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;
//	Source::m_pGlobalVars->interpolation_amount = 0.f;
//
//	// backup stuff that we do not want to fuck with.
//	C_Tickrecord backup_r; auto backup = &backup_r;
//
//	backup->origin = m_player->m_vecOrigin();
//	backup->velocity = m_player->m_vecVelocity();
//	//backup->m_abs_velocity = m_player->m_vecAbsVelocity();
//	backup->entity_flags = m_player->m_fFlags();
//	backup->ientity_flags = m_player->m_iEFlags();
//	backup->duck_amt = m_player->m_flDuckAmount();
//	backup->lower_body_yaw = m_player->m_flLowerBodyYawTarget();
//
//	std::memcpy(backup->anim_layers, m_player->animation_layers_ptr(), 0x38 * m_player->get_animation_layers_count());
//
//	// is player a bot?
//	//bool bot = game::IsFakePlayer(m_player->index());
//
//	// reset fakewalk state.
//	//record->m_fake_walk = false;
//	//record->m_mode = Resolver::Modes::RESOLVE_NONE;
//
//	// fix velocity.
//	// https://github.com/VSES/SourceEngine2007/blob/master/se2007/game/client/c_baseplayer.cpp#L659
//	if (record->m_lag > 0 && record->m_lag < 16 && m_records.size() >= 2) {
//		// get pointer to previous record.
//		auto previous = &m_records[1];
//
//		if (previous && !previous->dormant)
//			record->velocity = (record->origin - previous->origin) * (1.f / TICKS_TO_TIME(record->m_lag));
//	}
//
//	// set this fucker, it will get overriden anyway.
//	record->anim_velocity = record->velocity;
//
//	// fix various issues with the game
//	// these issues can only occur when a player is choking data.
//	if (record->m_lag > 1) {
//		// we need atleast 2 updates/records
//		// to fix these issues.
//		if (m_records.size() >= 2) {
//			// get pointer to previous record.
//			auto previous = &m_records[1];
//			auto cur = &m_records[0];
//
//			if (previous && !previous->dormant) {
//				// estimate flags, duckatm, and animation velocity...
//
//				if (cur->entity_flags & 1
//					&& !(previous->entity_flags & 1)
//					&& record->m_lag > 2)
//				{
//					m_player->m_fFlags() |= FL_ONGROUND;
//				}
//
//
//
//				//TODO: predict one tick of playa movement.
//			}
//		}
//	}
//
//	// if using fake angles, correct angles. // NO RESOLVER YET
//	//if (fake)
//	//	g_resolver.ResolveAngles(m_player, record);
//
//	// set stuff before animating.
//	m_player->m_vecOrigin() = record->origin;
//	m_player->m_vecVelocity() = m_player->m_vecAbsVelocity() = record->anim_velocity;
//	m_player->m_flLowerBodyYawTarget() = record->lower_body_yaw;
//
//	// EFL_DIRTY_ABSVELOCITY
//	// skip call to C_BaseEntity::CalcAbsoluteVelocity
//	m_player->m_iEFlags() &= ~0x1000;
//
//	// write potentially resolved angles. // NO RESOLVER YET BUT OK
//	//m_player->m_angEyeAngles() = record->eye_angles;
//
//	// fix animating in same frame.
//	if (state->m_Last_Animations_Update_Tick == Source::m_pGlobalVars->framecount)
//		state->m_Last_Animations_Update_Tick -= 1;
//
//	//if (cheat::settings.ragebot_resolver)
//
//	m_player->client_side_animation() = true;
//
//	//if (/*m_simulation[idx] != m_player->m_anim_time() &&*/cheat::settings.ragebot_resolver) {
//		//m_player->UpdateAnimationState(state, m_player->m_angEyeAngles());
//	//	//m_simulation[idx] = m_player->m_anim_time();
//
//	//	state->m_flGoalFeetYaw = m_player->m_angEyeAngles().y;
//	//}
//
//	auto absang = m_player->m_angEyeAngles();
//	state->m_flGoalFeetYaw = cheat::features::antiaimbot.rebuild_setupvelocity(m_player, false);
//	/*entity->m_angEyeAngles().y = */absang.y = state->m_flGoalFeetYaw;
//	m_player->UpdateAnimationState(state, absang);
//
//	m_player->update_clientside_animations();
//
//	m_player->client_side_animation() = false; //set this to true after storing not to have problems with zero pitch
//
//	record->pose_paramaters = m_player->m_flPoseParameter();
//	std::memcpy(record->anim_layers, m_player->animation_layers_ptr(), 0x38 * m_player->get_animation_layers_count());
//
//	// correct poses if fake angles. // NO RESOLVER YET
//	//if (fake)
//	//	g_resolver.ResolvePoses(m_player, record);
//
//	// store updated/animated poses and rotation in lagrecord.
//	record->pose_paramaters = m_player->m_flPoseParameter();
//	record->abs_eye_angles = m_player->get_abs_eye_angles();
//
//	// restore backup data.
//	m_player->m_vecOrigin() = backup->origin;
//	m_player->m_vecVelocity() = backup->velocity;
//	m_player->m_fFlags() = backup->entity_flags;
//	m_player->m_iEFlags() = backup->ientity_flags;
//	m_player->m_flDuckAmount() = backup->duck_amt;
//	m_player->m_flLowerBodyYawTarget() = backup->lower_body_yaw;
//	//m_player->m_flPoseParameter() = backup->pose_paramaters;
//	std::memcpy(m_player->animation_layers_ptr(), backup->anim_layers, 0x38 * m_player->get_animation_layers_count());
//
//	// IMPORTANT: do not restore poses here, since we want to preserve them for rendering.
//	// also dont restore the render angles which indicate the model rotation.
//
//	// restore globals.
//	Source::m_pGlobalVars->curtime = curtime;
//	Source::m_pGlobalVars->frametime = frametime;
//	Source::m_pGlobalVars->interpolation_amount = lerpamt;
//}

/*

feet = -360.0;
  animstate->m_flCurrentFeetYaw = animstate->m_flGoalFeetYaw;
  v52 = animstate->m_flGoalFeetYaw;
  v130 = -360.0;
  if ( v52 >= -360.0 )
  {
    feet = fminf(v52, 360.0);
    v130 = feet;
  }
  v53 = animstate->m_flEyeYaw - feet;
  animstate->m_flGoalFeetYaw = feet;
  yaw = animstate->m_flEyeYaw;
  v139 = fmod(v53, 360.0);
  yaw_feet_delta = v139;
  if ( yaw <= v130 )
  {
    if ( v139 <= -180.0 )
      yaw_feet_delta = v139 + 360.0;
  }
  else if ( v139 >= 180.0 )
  {
    yaw_feet_delta = v139 - 360.0;
  }
  speed_fraction = animstate->m_flFeetSpeedForwardSideways;
  if ( speed_fraction >= 0.0 )
    speed_fraction_clamped = fminf(speed_fraction, 1.0);
  else
    speed_fraction_clamped = 0.0;
  duck_amount = animstate->m_flDuckAmount;
  v59 = (float)((float)((float)(*(float *)&animstate[1]._0x0000[4] * -0.30000001) - 0.19999999) * speed_fraction_clamped)
      + 1.0;
  if ( duck_amount > 0.0 )
  {
    v60 = animstate->m_flFeetSpeedUnknownForwardSideways;
    if ( v60 >= 0.0 )
      v61 = fminf(v60, 1.0);
    else
      v61 = 0.0;
    v59 = v59 + (float)((float)(v61 * duck_amount) * (float)(0.5 - v59));
  }
  max_rotation = animstate[2].m_flDuckAmount * v59;
  inverted_max_rotation = *(float *)animstate[2]._0x00A0 * v59;
  if ( yaw_feet_delta <= max_rotation )
  {
    if ( inverted_max_rotation > yaw_feet_delta )
      animstate->m_flGoalFeetYaw = COERCE_FLOAT(LODWORD(inverted_max_rotation) & xmmword_108C97B0) + yaw;
  }
  else
  {
    animstate->m_flGoalFeetYaw = yaw - COERCE_FLOAT(LODWORD(max_rotation) & xmmword_108C97B0);
  }
  goal_feet_yaw = fmod(animstate->m_flGoalFeetYaw, 360.0);
  goal_feet_yaw_clamped = goal_feet_yaw;
  if ( goal_feet_yaw > 180.0 )
    goal_feet_yaw_clamped = goal_feet_yaw - 360.0;
  if ( goal_feet_yaw_clamped < -180.0 )
    goal_feet_yaw_clamped = goal_feet_yaw_clamped + 360.0;
  v65 = animstate->m_flSpeed2D;
  animstate->m_flGoalFeetYaw = goal_feet_yaw_clamped;
  if ( v65 > 0.1 || COERCE_FLOAT(LODWORD(animstate->m_flUpSpeed) & xmmword_108C97B0) > 100.0 )
  {
    v71 = (float)((float)(*(float *)&animstate[1]._0x0000[4] * 20.0) + 30.0) * animstate->m_flEyePitch;
    v72 = (float)(unsigned __int16)(signed int)(float)(goal_feet_yaw_clamped * 182.04445) * 0.0054931641;
    v73 = (float)((float)(unsigned __int16)(signed int)(float)(animstate->m_flEyeYaw * 182.04445) * 0.0054931641) - v72;
    if ( v71 < 0.0 )
      LODWORD(v71) ^= xmmword_108C97D0;
    if ( v73 >= -180.0 )
    {
      if ( v73 > 180.0 )
        v73 = v73 - 360.0;
    }
    else
    {
      v73 = v73 + 360.0;
    }
    if ( v73 <= v71 )
    {
      if ( COERCE_FLOAT(LODWORD(v71) ^ xmmword_108C97D0) <= v73 )
        v74 = (float)(unsigned __int16)(signed int)(float)(animstate->m_flEyeYaw * 182.04445) * 0.0054931641;
      else
        v74 = v72 - v71;
    }
    else
    {
      v74 = v72 + v71;
    }
    v75 = (CGlobalVarsBase *)gpGlobals;
    v76 = (char *)animstate->m_BaseEntity;
    animstate->m_flGoalFeetYaw = v74;
    v77 = v76 + 0x279C;
    v142 = v77;
    *(float *)&animstate->_0x010A[2] = v75->curtime + 0.22000001;
    if ( *v77 != LODWORD(animstate->m_flEyeYaw) )
    {
      v78 = (int)(v77 + 0xFFFFF619);
      if ( *((_BYTE *)v77 + 0xFFFFD8BC) )
      {
        *(_BYTE *)(v78 + 92) |= 1u;
      }
      else if ( *(_DWORD *)(v78 + 28) )
      {
        NetworkStateChanged((int *)v77[0xFFFFF620], 0x279C);
        v77 = v142;
      }
      *v77 = LODWORD(animstate->m_flEyeYaw);
    }
  }
  else
  {
    v131 = (float *)((char *)animstate->m_BaseEntity + 0x279C);
    v66 = ApproachAngle(*v131, goal_feet_yaw_clamped, animstate->m_flEyePitch * 100.0);
    v67 = (CGlobalVarsBase *)gpGlobals;
    LODWORD(animstate->m_flGoalFeetYaw) = v66.m128_i32[0];
    v149 = v66.m128_f32[0];
    if ( v67->curtime > *(float *)&animstate->_0x010A[2] )
    {
      v141 = fmod((float)(v66.m128_f32[0] - animstate->m_flEyeYaw), 360.0);
      v68 = v141;
      if ( v149 <= animstate->m_flEyeYaw )
      {
        if ( v141 <= -180.0 )
          v68 = v141 + 360.0;
      }
      else if ( v141 >= 180.0 )
      {
        v68 = v141 - 360.0;
      }
      if ( COERCE_FLOAT(LODWORD(v68) & xmmword_108C97B0) > 35.0 )
      {
        v69 = v131;
        *(float *)&animstate->_0x010A[2] = v67->curtime + 1.1;
        if ( *(_DWORD *)v131 != LODWORD(animstate->m_flEyeYaw) )
        {
          v70 = (int)(v131 + 0xFFFFF619);
          if ( *((_BYTE *)v131 + 0xFFFFD8BC) )
          {
            *(_BYTE *)(v70 + 92) |= 1u;
            *v131 = animstate->m_flEyeYaw;
          }
          else
          {
            if ( *(_DWORD *)(v70 + 28) )
            {
              NetworkStateChanged(*(int **)(v70 + 28), 0x279C);
              v69 = v131;
            }
            *v69 = animstate->m_flEyeYaw;
          }
        }
      }
    }
  }

*/

void c_lagcomp::fix_netvar_compression(C_BasePlayer* player) {
	auto& history = records[player->entindex()-1].m_Tickrecords;

	if (history.size() < 2 || player == nullptr || player->get_animation_state() == nullptr)
		return;

	auto& record = history.front();
	auto& pre_record = history.at(1);

	auto v15 = (player->m_fFlags() & FL_ONGROUND) == 0;

	float chokedTime = player->m_flSimulationTime() - record.simulation_time;
	//chokedTime = Math::clamp(chokedTime, fmaxf(Source::m_pGlobalVars->interval_per_tick, chokedTime), 1.0f);

	if (chokedTime <= 1.0)
		chokedTime = fmaxf(Source::m_pGlobalVars->interval_per_tick, chokedTime);
	else
		chokedTime = 0.0;

	Vector origin = player->m_vecOrigin();
	Vector deltaOrigin = (origin - record.origin) * (1.0f / chokedTime);

	if (v15 || !(record.entity_flags & FL_ONGROUND)) {

		float penultimateChoke = record.simulation_time - pre_record.simulation_time;
		//penultimateChoke = Math::clamp(penultimateChoke, 0.0f, fmaxf(Source::m_pGlobalVars->interval_per_tick, penultimateChoke));

		if (penultimateChoke <= 1.0)
			penultimateChoke = fmaxf(Source::m_pGlobalVars->interval_per_tick, penultimateChoke);
		else
			penultimateChoke = 0.0;

		float delta = RAD2DEG(atan2((record.origin.y - pre_record.origin.y) * (1.0f / penultimateChoke),
			(record.origin.x - pre_record.origin.x) * (1.0f / penultimateChoke)));

		float direction = RAD2DEG(atan2(deltaOrigin.y, deltaOrigin.x));
		float deltaDirection = Math::normalize_angle(direction - delta);

		deltaDirection = DEG2RAD(deltaDirection * 0.5f + direction);

		float dirCos, dirSin;

		dirSin = sin(deltaDirection);
		dirCos = cos(deltaDirection);

		float move = deltaOrigin.Length2D();
		Vector velocity;
		velocity.x = dirCos * move;
		velocity.y = dirSin * move;
		velocity.z = deltaOrigin.z;

		if (!(player->m_fFlags() & FL_ONGROUND) || !player->get_animation_state()->on_ground)
			velocity.z -= Source::m_pCvar->FindVar("sv_gravity")->GetFloat() * chokedTime * 0.5f;

		player->m_vecVelocity() = velocity;
	}
	else
	{
		player->m_vecVelocity() = deltaOrigin;
	}
	//static auto sv_accelerate = Source::m_pCvar->FindVar("sv_accelerate");

	//float delta = player->m_flSimulationTime() - record.simulation_time;
	//delta = max(delta, Source::m_pGlobalVars->interval_per_tick); // TICK_INTERVAL()

	//bool on_ground = (player->m_fFlags() & FL_ONGROUND || player->get_animation_state()->on_ground);

	//auto origin = player->m_vecOrigin();
	//auto origin_delta = origin - record.origin;

	//auto velocity = origin_delta / delta;
	//auto last_velocity = record.velocity;

	////if (velocity.Length2D() > last_velocity.Length2D())
	////	anim_vel = RebuildGameMovement::Get().Accelerate/*accelerate*/(this, anim_vel, wishdir, 250.f, sv_accelerate->GetFloat())

	//if (on_ground)
	//{

	//}
	//else
	//{
	//	player->m_vecVelocity() = Math::Lerp(Source::m_pGlobalVars->interval_per_tick / delta,
	//		last_velocity,
	//		velocity
	//	);
	//}
}

//void c_lagcomp::update_animations_data(C_BasePlayer* m_player, C_Tickrecord* record) {
//	CCSGOPlayerAnimState* state = m_player->get_animation_state();
//
//	auto idx = m_player->entindex() - 1;
//
//	if (!state || m_player == cheat::main::local)
//		return;
//
//	auto m_records = records[idx].m_Tickrecords;
//
//	//// player respawned.
//	//if (m_player->m_flSpawnTime() != m_spawn[idx]) {
//	//	// reset animation state.
//	//	reset_animstate(state);
//
//	//	// note new spawn time.
//	//	m_spawn[idx] = m_player->m_flSpawnTime();
//	//}
//
//	auto resolver_info = cheat::features::aaa.player_resolver_records[idx];
//
//	// backup curtime.
//	const float curtime = Source::m_pGlobalVars->curtime;
//	const float frametime = Source::m_pGlobalVars->frametime;
//	//const float lerpamt = Source::m_pGlobalVars->interpolation_amount;
//
//	// set curtime to animtime.
//	// set frametime to ipt just like on the server during simulation.
//	Source::m_pGlobalVars->curtime = m_player->m_flSimulationTime();
//	Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;
//	//Source::m_pGlobalVars->interpolation_amount = 0.f;
//
//	// backup stuff that we do not want to fuck with.
//	C_Tickrecord backup_r; auto backup = &backup_r;
//
//	fix_netvar_compression(m_player);
//
//	backup->origin = m_player->m_vecOrigin();
//	backup->velocity = m_player->m_vecVelocity();
//	backup->entity_flags = m_player->m_fFlags();
//	backup->ientity_flags = m_player->m_iEFlags();
//	backup->duck_amt = m_player->m_flDuckAmount();
//	backup->lower_body_yaw = m_player->m_flLowerBodyYawTarget();
//	backup->eye_angles = record->eye_angles = m_player->m_angEyeAngles();
//
//	std::memcpy(backup->anim_layers, m_player->animation_layers_ptr(), 0x38 * m_player->get_animation_layers_count());
//
//	resolver_info.m_flRate = backup->anim_layers[6].m_flPlaybackRate;
//
//	// if using fake angles, correct angles.
//	auto absang = m_player->m_angEyeAngles();
//
//	int history = cheat::main::shots_fired[idx];
//
//	//if (cheat::main::shots_fired[idx] % 2 == 0)
//	//	history = cheat::main::history_hit[idx];
//
//	cheat::features::aaa.resolve(m_player, history);
//	
//	record->eye_angles.y = absang.y;
//	//if (cheat::settings.ragebot_resolver)
//	//	m_player->UpdateAnimationState(state, absang);
//
//	// set stuff before animating.
//	m_player->m_vecOrigin() = record->origin;
//	m_player->m_flLowerBodyYawTarget() = record->lower_body_yaw;
//
//	// EFL_DIRTY_ABSVELOCITY
//	// skip call to C_BaseEntity::CalcAbsoluteVelocity
//	//m_player->m_iEFlags() &= ~0x1000;
//	//m_player->m_vecAbsVelocity() = m_player->m_vecVelocity();
//
//	// EFL_DIRTY_ABSTRANSFORM
//	// skip call to C_BaseEntity::CalcAbsolutePosition( )
//	//m_player->m_iEFlags() &= ~0x800;
//	//*(Vector*)((uintptr_t)m_player + 0xA0) = m_player->m_vecOrigin(); //m_player->AbsOrigin = m_player->VecOrigin 
//
//	// write potentially resolved angles. // NO RESOLVER YET BUT OK
//	m_player->m_angEyeAngles() = record->eye_angles;
//
//	m_player->client_side_animation() = true;
//
//	//*(QAngle*)(uintptr_t(m_player) + 0x128) = m_player->m_angEyeAngles();
//
//	// fix animating in same frame.
//	if (state->m_Last_Animations_Update_Tick >= Source::m_pGlobalVars->framecount)
//		state->m_Last_Animations_Update_Tick = Source::m_pGlobalVars->framecount - 1;
//
//	m_player->update_clientside_animations();
//
//	auto animlayer_6 = &m_player->animation_layer(6);
//	resolver_info.m_flClientRate = animlayer_6->m_flPlaybackRate;
//
//	/*if (resolver_info.b_bRateCheck) {
//		auto animlayer_6 = &m_player->animation_layer(6);
//		resolver_info.m_flClientRate = animlayer_6->m_flPlaybackRate;
//		resolver_info.b_bRateCheck = false;
//	}*/
//
//	m_player->client_side_animation() = false;
//
//	record->pose_paramaters = m_player->m_flPoseParameter();
//	std::memcpy(record->anim_layers, m_player->animation_layers_ptr(), 0x38 * m_player->get_animation_layers_count());
//
//	// correct poses if fake angles.
//	//if (cheat::settings.ragebot_resolver)
//	//	m_player->m_flPoseParameter().at(12) = (record->eye_angles.x + 90) / 180;
//
//	// store updated/animated poses and rotation in lagrecord.
//	record->pose_paramaters = m_player->m_flPoseParameter();
//	record->abs_eye_angles = m_player->get_abs_eye_angles();
//
//	// restore backup data.
//	m_player->m_angEyeAngles() = record->eye_angles;
//	m_player->m_vecOrigin() = backup->origin;
//	m_player->m_vecVelocity() = backup->velocity;
//	m_player->m_fFlags() = backup->entity_flags;
//	m_player->m_iEFlags() = backup->ientity_flags;
//	m_player->m_flDuckAmount() = backup->duck_amt;
//	m_player->m_flLowerBodyYawTarget() = backup->lower_body_yaw;
//
//	std::memcpy(m_player->animation_layers_ptr(), backup->anim_layers, 0x38 * m_player->get_animation_layers_count());
//
//	// IMPORTANT: do not restore poses here, since we want to preserve them for rendering.
//	// also dont restore the render angles which indicate the model rotation.
//
//	// restore globals.
//	Source::m_pGlobalVars->curtime = curtime;
//	Source::m_pGlobalVars->frametime = frametime;
//	//Source::m_pGlobalVars->interpolation_amount = lerpamt;
//
//	if (cheat::Cvars.RageBot_FixLag.GetValue()) {
//		m_player->LastBoneSetupFrame() = 0; // force bone animation update
//		m_player->force_bone_rebuild();
//
//		m_player->SetupBones(nullptr, -1, 0x7FF00, Source::m_pGlobalVars->curtime);
//	}
//	//matrix3x4_t mx[128];
//	//m_player->SetupBones(mx, 128, 0x7FF00, 0);
//
//	//m_player->DrawServerHitboxes();
//	//cheat::features::aimbot.visualise_hitboxes(m_player, mx, Color::White(), Source::m_pGlobalVars->interval_per_tick * 2.0f);
//}

void c_lagcomp::update_animations_data(C_BasePlayer* m_player, C_Tickrecord* record) 
{
	CCSGOPlayerAnimState* state = m_player->get_animation_state();

	auto idx = m_player->entindex() - 1;

	if (!state || m_player == cheat::main::local)
		return;

	auto weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(m_player->m_hActiveWeapon()));

	m_player->set_abs_origin(m_player->m_vecOrigin());

	//const auto flags_backup = m_player->m_fFlags();
	//const auto i_e_flags_backup = m_player->m_iEFlags();
	//const auto base_vel_backup = m_player->m_vecBaseVelocity();
	//const auto ang_rot_backup = *(QAngle*)(uintptr_t(m_player) + 0x128);
	//const auto old_seq_backup = *(int*)(uintptr_t(m_player) + 0x26BC); //0x28BC

	//fix_netvar_compression(m_player);

	//m_player->m_angEyeAngles().x = Math::NormalizePitch(m_player->m_angEyeAngles().x);

	auto m_records = records[idx].m_Tickrecords;
	auto resolver_info = &cheat::features::aaa.player_resolver_records[idx];

	const auto curtime = Source::m_pGlobalVars->curtime;
	//const auto frametime = Source::m_pGlobalVars->frametime;
	auto time_delta = m_player->m_flSimulationTime() - m_player->m_flOldSimulationTime();
	auto v75 = TIME_TO_TICKS(time_delta);

	if (v75 >= 20)
		Source::m_pGlobalVars->curtime = m_player->m_flSimulationTime();
	else
		Source::m_pGlobalVars->curtime = Source::m_pGlobalVars->interval_per_tick + m_player->m_flOldSimulationTime();

	//Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;

	std::memcpy(resolver_info->server_anim_layers, m_player->animation_layers_ptr(), 0x38 * m_player->get_animation_layers_count());
	resolver_info->m_flRate = resolver_info->server_anim_layers[6].m_flPlaybackRate;

	if (m_records.size() > 1) {

		const auto m_lag = TIME_TO_TICKS(m_records.front().simulation_time - m_records.at(1).simulation_time);

		if (!m_player->IsBot() && (m_player->m_fFlags() & FL_ONGROUND)) {
			if (m_lag > 0 && m_lag < 16) {
				/*if (m_player->m_vecVelocity().Length2D() > 0.1f && m_player->m_vecVelocity().Length2D() > m_records.front().velocity.Length2D())
				{
					const auto new_move_angle = Vector(0, RAD2DEG(std::atan2(m_player->m_vecVelocity().y, m_player->m_vecVelocity().x)), 0);
					const auto prev_move_angle = Vector(0, RAD2DEG(std::atan2(m_records.front().velocity.y, m_records.front().velocity.x)), 0);

					accelerate_velocity(m_player, m_player->m_vecVelocity(), new_move_angle, prev_move_angle);
				}
				else*/
					m_player->m_vecVelocity() = (m_records.front().origin - m_records.at(1).origin) * (1.f / TICKS_TO_TIME(m_lag));
			}
		}

		if (m_lag > 1 && !m_player->IsBot()) {
			// we need atleast 2 updates/records
			// to fix these issues.
			if (m_records.size() >= 2) {
				// get pointer to previous record.
				auto previous = &m_records.at(1);

				if (previous && !previous->dormant) {
					// estimate flags, duckamt, and animation velocity...

					if (m_player->m_fFlags() & 1 && previous->entity_flags & 1)
						m_player->m_fFlags() |= 1u;

					//if (resolver_info->server_anim_layers[6].m_flWeight < previous->anim_layers[6].m_flWeight && resolver_info->server_anim_layers[6].m_flCycle == 0.0f)
					//	m_player->m_fFlags() |= 1u;

					if (m_player->m_fFlags() & 1 && !(previous->entity_flags & 1))
						m_player->m_fFlags() &= ~1;

					if (resolver_info->server_anim_layers[5].m_flWeight > 0.f)
						m_player->m_fFlags() |= FL_ONGROUND;
				}
			}

			if (resolver_info->server_anim_layers[6].m_flWeight == 0.0f && m_player->m_vecVelocity().Length2D() > 0.1f && m_player->m_vecVelocity().Length2D() < 100.f && (m_player->m_fFlags() & FL_ONGROUND)) {
				state->velocity = m_player->m_vecVelocity() = { 0.f,0.f,0.f };
			}
		}
	}

	state->on_ground = (m_player->m_fFlags() & FL_ONGROUND);

	if (state->duck_amt > 0.0f)
		m_player->m_fFlags() |= FL_DUCKING;

	// EFL_DIRTY_ABSVELOCITY
	// skip call to C_BaseEntity::CalcAbsoluteVelocity
	m_player->m_iEFlags() &= ~EFL_DIRTY_ABSVELOCITY;
	//m_player->m_vecAbsVelocity() = m_player->m_vecVelocity();

	//state->speed_2d = Math::clamp(m_player->m_vecVelocity().Length2D(), 0.0f, 260.f);

	//*(QAngle*)(uintptr_t(m_player) + 0x128) = m_player->m_angEyeAngles();
	//*(Vector*)(uintptr_t(m_player) + 0x4C) = *(Vector*)(uintptr_t(m_player) + 0xDC);

	//const auto v25 = std::clamp(m_player->m_flDuckAmount() + state->landing_duck, 0.0f, 1.0f);
	//state->duck_amt = std::clamp(v25, 0.0f, 1.0f);

	////// copy back pre-animation data.
	//if (m_records.size() > 1 && state->speed_2d > 100.f)
	//{
	//	const auto& lol = m_records[0];
	//	/* REMOVED BLACK MAGIC HERE TROLOLOL */
	//	state->feet_cycle /*= record.feet_cycle*/ = lol.anim_layers[6].m_flCycle;
	//	state->feet_rate /*= record.feet_yaw_rate*/ = lol.anim_layers[6].m_flWeight;
	//}

	cheat::features::aaa.resolve(m_player, cheat::main::shots_fired[idx], v75);

	state->feet_rate = 0.f;

	if (state) {
		if (state->last_anim_upd_tick == Source::m_pGlobalVars->framecount)
			state->last_anim_upd_tick = Source::m_pGlobalVars->framecount - 1;

		if (state->last_anim_upd_time == Source::m_pGlobalVars->curtime)
			state->last_anim_upd_time = Source::m_pGlobalVars->curtime - state->anim_update_delta;
	}

	auto backup_duck_amount = m_player->m_flDuckAmount();

	if (v75 > 1 && !m_records.empty())
	{
		auto old_duck_amount = m_records.front().duck_amt;

		m_player->m_flDuckAmount() = (((m_player->m_flDuckAmount() - old_duck_amount) / time_delta) * Source::m_pGlobalVars->interval_per_tick) + old_duck_amount;

		if (old_duck_amount < m_player->m_flDuckAmount() && m_player->m_flDuckAmount() != 1.f)
			m_player->m_angEyeAngles() = m_records.front().eye_angles;
	}

	m_player->update_clientside_animations();

	std::memcpy(resolver_info->client_anim_layers, m_player->animation_layers_ptr(), 0x38 * m_player->get_animation_layers_count());
	resolver_info->m_flClientRate = resolver_info->client_anim_layers[6].m_flPlaybackRate;
	
	m_player->m_flDuckAmount() = backup_duck_amount;

	//m_player->m_fFlags() = flags_backup;
	//m_player->m_iEFlags() = i_e_flags_backup;
	//m_player->m_vecBaseVelocity() = base_vel_backup;
	//*(QAngle*)(uintptr_t(m_player) + 0x128) = ang_rot_backup;

	std::memcpy(m_player->animation_layers_ptr(), resolver_info->server_anim_layers, 0x38 * m_player->get_animation_layers_count());

	auto unknownLayer = &m_player->get_animation_layer(12);

	if (unknownLayer && m_player->get_animation_layers_count() > 12)
	{
		unknownLayer->m_flPlaybackRate = 0;
		unknownLayer->m_flCycle = 0;
		unknownLayer->m_flWeight = 0;
	}

	Source::m_pGlobalVars->curtime = curtime;

	//m_player->set_abs_angles(QAngle(0, m_player->m_angEyeAngles().y/*resolver_info->resolved_yaw*/, 0));

	m_player->force_bone_rebuild();
	m_player->SetupBonesEx();
	//m_player->SetupBones(nullptr, -1, 0x7FF00, Source::m_pGlobalVars->curtime);

	//if (resolver_info->did_shot_this_tick) {
		//cheat::features::aimbot.visualise_hitboxes(m_player, m_player->m_CachedBoneData().Base(), Color::White(), Source::m_pGlobalVars->interval_per_tick * 2.0f);
		//m_player->DrawServerHitboxes();
	//}
}


void c_lagcomp::finish_position_adjustment(C_BasePlayer* entity)
{
	if (!(int)cheat::Cvars.RageBot_AdjustPositions.GetValue())
		return;

	auto player_index = entity->entindex() - 1;
	auto player_record = &records[player_index];

	if (!player_record->restore_record.data_filled)
		return;

	apply_record_data(entity, &player_record->restore_record);

	player_record->restore_record.data_filled = false; // Set to false so that we dont apply this record again if its not set next time
}

void c_lagcomp::store_records()
{
	get_interpolation();

	if (cheat::main::local == nullptr)
	{
		reset();
		return;
	}

	for (auto index = 1; index < Source::m_pGlobalVars->maxClients; index++)
	{
		auto entity = (C_BasePlayer*)Source::m_pEntList->GetClientEntity(index);

		if (!entity || !entity->IsPlayer())
			continue;

		auto entidx = entity->entindex();

		if	(entity->IsDormant() ||
			entity->m_iHealth() <= 0 ||
			entity->IsDead() ||
			entity->m_iTeamNum() == cheat::main::local->m_iTeamNum()
			)
		{
			if (entidx == 0)
				continue;

			records[entidx - 1].reset((entity->IsDormant() && entity->m_iHealth() <= 0));
			got_update[entidx - 1] = false;

			continue;
		}

		if (!entity->IsDormant() && entity->m_iTeamNum() != cheat::main::local->m_iTeamNum())
		{
			auto &data = cheat::features::aaa.player_resolver_records[entidx - 1];

			if ((entity->m_flSimulationTime() - data.last_simtime) > 1.f)
				cheat::main::shots_fired[entidx - 1] = 0;
		}

		if (!cheat::main::local->IsDead()) {
			const auto lol = entity->m_flSimulationTime();
			update_player_record_data(entity);
			entity->m_flSimulationTime() = lol;
		}
		else {
			cheat::main::updating_anims = true;
			entity->client_side_animation() = true;
		}

	}
}

void c_lagcomp::reset()
{
	for (auto i = 0; i < 64; i++) {
		records[i].reset(true);
		//cheat::features::aaa.move_logs[i].clear();
	}
}