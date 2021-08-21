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

void c_lagcomp::store_record_data(C_BasePlayer* entity, C_Tickrecord* record_data, bool backup = false)
{
	if (entity == nullptr || entity->get_animation_state() == nullptr)
		return;

	record_data->abs_origin = entity->get_abs_origin();
	record_data->object_mins = entity->OBBMins();
	record_data->object_maxs = entity->OBBMaxs();

	if (!backup)
	{
		record_data->origin = entity->m_vecOrigin();
		record_data->velocity = entity->m_vecVelocity();
		record_data->eye_angles = entity->m_angEyeAngles();
		record_data->abs_eye_angles = entity->get_abs_eye_angles();
		record_data->entity_flags = entity->m_fFlags();
		record_data->simulation_time = entity->m_flSimulationTime();
		record_data->lower_body_yaw = entity->m_flLowerBodyYawTarget();
		record_data->dormant = entity->IsDormant();
		record_data->anim_velocity = entity->m_vecVelocity();
		record_data->ientity_flags = entity->m_iEFlags();
		record_data->duck_amt = entity->m_flDuckAmount();
		record_data->bones_count = entity->GetBoneCount();
		record_data->lby_delta = 0.f;// cheat::features::aaa.sub_59B13C30(entity->get_animation_state());
		record_data->resolver_method = cheat::features::aaa.player_resolver_records[entity->entindex() - 1].resolving_method;
		record_data->pose_paramaters = entity->m_flPoseParameter();
		record_data->servertick = Source::m_pGlobalVars->tickcount;

		memcpy(record_data->anim_layers, entity->animation_layers_ptr(), 0x38 * entity->get_animation_layers_count());
	}
	else
	{
		memcpy(record_data->matrixes, entity->m_CachedBoneData().Base(), sizeof(matrix3x4_t) * entity->m_CachedBoneData().Count());
		record_data->bones_count = entity->m_CachedBoneData().Count();
	}

	record_data->data_filled = true;
}

void c_lagcomp::apply_record_data(C_BasePlayer* entity, C_Tickrecord* record_data, bool backup)
{
	if (entity == nullptr || !record_data->data_filled || !entity->get_animation_state())
		return;

	record_data->bones_count = Math::clamp(record_data->bones_count, 0, 128);

	entity->GetBoneCount() = record_data->bones_count;
	entity->set_collision_bounds(record_data->object_mins, record_data->object_maxs);
	entity->set_abs_origin(backup ? record_data->abs_origin : record_data->origin);
	std::memcpy(entity->m_CachedBoneData().Base(), record_data->matrixes, record_data->bones_count * sizeof(matrix3x4_t));
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

	if (track->tick_records.size() < 2)
		return false;

	auto current_rec = &track->tick_records.front();
	auto previous_rec = &track->tick_records.at(1);

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

bool c_lagcomp::is_time_delta_too_large(C_Tickrecord* wish_record)
{
	float correct = 0;

	correct += cheat::main::latency;
	correct += cheat::main::lerp_time;
	
	//if (cheat::game::pressed_keys[(int)cheat::Cvars.Exploits_fakeduck.GetValue()] && (int)cheat::Cvars.Exploits_fakeduck.GetValue() != 0)
	//	correct += TICKS_TO_TIME(14 - Source::m_pClientState->m_iChockedCommands);

	static auto sv_maxunlag = Source::m_pCvar->FindVar("sv_maxunlag");

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

bool got_update[65];
float flick_time[65];

void c_lagcomp::update_player_record_data(C_BasePlayer* entity)
{
	if (entity == nullptr)
		return;

	auto player_index = entity->entindex() - 1;
	auto player_record = &records[player_index];

	C_Tickrecord new_record;
	store_record_data(entity, &new_record);

	if (new_record.data_filled)
		player_record->tick_records.push_front(new_record);
	
	update_animations_data(entity);

	while (player_record->tick_records.size() > 64)
		player_record->tick_records.pop_back();
}

bool c_lagcomp::can_backtrack_this_tick(C_Tickrecord* record, C_Tickrecord* previous, c_player_records* log)
{
	if (!record || !log)
		return;

	auto is_sim_valid = !is_time_delta_too_large(record);

	if (!previous)
	{
		//pizdets...
		return is_sim_valid;
	}

	auto origin_delta = record->origin.Distance(previous->origin);

	if (origin_delta <= 4096.f || (Source::m_pGlobalVars->tickcount - record->servertick) <= 3)
		return is_sim_valid;

	if (record->velocity.Length2D() * (Source::m_pGlobalVars->interval_per_tick * TICKS_TO_TIME(cheat::main::latency)) <= 64.0f)
		return is_sim_valid;

	return false;
}

bool IsEqual(C_Tickrecord* a1, C_Tickrecord* a2)
{
	if (a1->type != a2->type)
		return 0;
	else if (a1->lower_body_yaw != a2->lower_body_yaw)
		return 0;

	return a1->origin.x == a2->origin.x
		&& a1->origin.y == a2->origin.y
		&& a1->origin.z == a2->origin.z
		&& a1->eye_angles.x == a2->eye_angles.x
		&& a1->eye_angles.y == a2->eye_angles.y
		&& a1->eye_angles.z == a2->eye_angles.z
		&& a1->entity_flags == a2->entity_flags
		&& a1->duck_amt == a2->duck_amt
		&& a1->shot_time == a2->shot_time
		&& a1->object_maxs.z == a2->object_maxs.z;
}

C_Tickrecord* c_lagcomp::find_priority_record(c_player_records* log)
{
	if (log->tick_records.size() <= 1)
		return nullptr;

	C_Tickrecord* best_rec = nullptr;
	C_Tickrecord* prev_rec = nullptr;

	for (int i = 0; i < log->tick_records.size(); i++)
	{
		auto rec = &log->tick_records[i];

		if (!rec || !rec->data_filled)
			continue;
		
		const auto pre_rec = prev_rec;
		prev_rec = rec;

		if (IsEqual(rec, pre_rec) || is_time_delta_too_large(rec))
			continue;

		if (rec->type < best_rec->type)
			continue;
	}
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

		if (entity->IsDormant()
			|| entity->m_iHealth() <= 0
			|| entity->IsDead()
			|| entity->m_iTeamNum() == cheat::main::local()->m_iTeamNum())
			continue;

		auto lc_log = &records[entity->entindex() - 1];

		if (!lc_log)
			continue;

		lc_log->best_record = nullptr;

		if (lc_log->tick_records.size() < 1)
			continue;

		if (lc_log->tick_records.size() < 2)
		{
			if (can_backtrack_this_tick(&lc_log->tick_records.front(), nullptr, lc_log))
				lc_log->best_record = &lc_log->tick_records.front();

			continue;
		}

		//can_backtrack_this_tick(&lc_log->tick_records.front(), nullptr, lc_log)
	}
}

void c_lagcomp::backup_players(bool restore)
{
	if (!cheat::Cvars.RageBot_AdjustPositions.GetValue())
		return;

	for (auto index = 1; index <= 64; index++)
	{
		auto entity = (C_BasePlayer*)Source::m_pEntList->GetClientEntity(index);

		if (!entity || !entity->IsPlayer())
			continue;

		if (entity->IsDormant() 
			|| entity->m_iHealth() <= 0 
			|| entity->IsDead() 
			|| entity->m_iTeamNum() == cheat::main::local()->m_iTeamNum())
			continue;

		auto entidx = entity->entindex();

		auto lol = &records[entidx - 1];

		if (restore)
		{
			if (!lol->restore_record.data_filled)
				continue;

			apply_record_data(entity, &lol->restore_record, true);
			lol->restore_record.data_filled = false;
		}
		else
		{
			store_record_data(entity, &lol->restore_record);
		}
	}
}

void c_lagcomp::store_records()
{
	get_interpolation();

	if (cheat::main::local() == nullptr)
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
			entity->m_iTeamNum() == cheat::main::local()->m_iTeamNum()
			)
		{
			records[index - 1].reset((entity->IsDormant() && entity->m_iHealth() <= 0));
			got_update[index - 1] = false;

			continue;
		}
			
		update_player_record_data(entity);
	}
}

void c_lagcomp::reset()
{
	for (auto i = 0; i < 64; i++) {
		records[i].reset(true);
		//cheat::features::aaa.move_logs[i].clear();
	}
}