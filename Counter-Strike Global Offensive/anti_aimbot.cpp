#include "aimbot.hpp"
#include "source.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "hooked.hpp"
#include "math.hpp"
#include "displacement.hpp"
#include "anti_aimbot.hpp"
#include "lag_compensation.hpp"
#include "prediction.hpp"
#include "autowall.hpp"
#include <algorithm>
#include "angle_resolver.hpp"
#include <iostream>
#include "rmenu.hpp"
#include "movement.hpp"

float GetRotatedYaw(float lby, float yaw)
{
	float delta = Math::NormalizeYaw(yaw - lby);
	if (fabs(delta) < 25.f)
		return lby;

	if (delta > 0.f)
		return yaw + 25.f;

	return yaw;
}

bool IsPressingMovementKeys(CUserCmd* cmd)
{
	if (!cmd)
		return false;

	if (cmd->buttons & IN_FORWARD ||
		cmd->buttons & IN_BACK || cmd->buttons & IN_LEFT || cmd->buttons & IN_RIGHT ||
		cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT)
		return true;

	return false;
}

Vector TraceEnd(Vector start, Vector end)
{
	trace_t trace;
	CTraceFilterWorldOnly filter;
	Ray_t ray;

	ray.Init(start, end);
	Source::m_pEngineTrace->TraceRay(ray, MASK_ALL, &filter, &trace);

	return trace.endpos;
}

struct angle_data {
	float angle;
	float thickness;
	angle_data(const float angle, const float thickness) : angle(angle), thickness(thickness) {}
};

float quick_normalize(float degree, const float min, const float max) {
	while (degree < min)
		degree += max - min;
	while (degree > max)
		degree -= max - min;

	return degree;
}

bool trace_to_exit_short(Vector &point, Vector &dir, const float step_size, float max_distance)
{
	float flDistance = 0;

	while (flDistance <= max_distance)
	{
		flDistance += step_size;

		point += dir * flDistance;

		auto contents = Source::m_pEngineTrace->GetPointContents(point);

		if ((contents & MASK_SHOT_HULL && contents & CONTENTS_HITBOX) == 0)
		{
			// found first free point
			return true;
		}
	}

	return false;
}

float get_thickness(Vector& start, Vector& end, C_BaseEntity* efilter) {
	Vector dir = end - start;
	Vector step = start;
	dir /= dir.Length();
	CTraceFilter filter;
	filter.pSkip = efilter;
	trace_t trace;
	Ray_t ray;
	float thickness = 0;
	while (true) {
		ray.Init(step, end);
		Source::m_pEngineTrace->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &trace);

		if (!trace.DidHit())
			break;

		const Vector lastStep = trace.endpos;
		step = trace.endpos;

		if ((end - start).Length() <= (step - start).Length())
			break;

		if (!trace_to_exit_short(step, dir, 5, 90))
			return FLT_MAX;

		thickness += (step - lastStep).Length();
	}
	return thickness;
}

void c_antiaimbot::freestand(C_BasePlayer *ent, float& new_angle) {
	std::vector<angle_data> points;

	if (!ent)
		return;

	if (ent->IsDead())
		return;

	const auto local_position = ent->GetEyePosition();
	std::vector<float> scanned = {};

	for (auto i = 0; i <= 64; i++)
	{
		auto p_entity = (C_BasePlayer*)(Source::m_pEntList->GetClientEntity(i));
		if (p_entity == nullptr) continue;
		if (p_entity == ent) continue;
		if (p_entity->IsDead()) continue;
		if (p_entity->m_iTeamNum() == ent->m_iTeamNum()) continue;
		if (p_entity->IsDormant()) continue;
		if (!p_entity->IsPlayer()) continue;

		const auto view = Math::CalcAngle(local_position, p_entity->GetEyePosition());

		std::vector<angle_data> angs;

		for (auto y = 1; y < 5; y++)
		{
			auto ang = quick_normalize((y * 90) + view.y, -180.f, 180.f);
			auto found = false; // check if we already have a similar angle

			for (auto i2 : scanned)
				if (abs(quick_normalize(i2 - ang, -180.f, 180.f)) < 20.f)
					found = true;

			if (found)
				continue;

			points.emplace_back(ang, -1.f);
			scanned.push_back(ang);
		}

		//points.push_back(base_angle_data(view.y, angs)); // base yaws and angle data (base yaw needed for lby breaking etc)
	}

	for (auto i = 0; i <= 64; i++)
	{
		auto p_entity = (C_BasePlayer*)(Source::m_pEntList->GetClientEntity(i));
		if (p_entity == nullptr) continue;
		if (p_entity == ent) continue;
		if (p_entity->IsDead()) continue;
		if (p_entity->m_iTeamNum() == ent->m_iTeamNum()) continue;
		if (p_entity->IsDormant()) continue;
		if (!p_entity->IsPlayer()) continue;

		auto found = false;
		auto points_copy = points; // copy data so that we compair it to the original later to find the lowest thickness
		auto enemy_eyes = p_entity->GetEyePosition();

		for (auto &z : points_copy) // now we get the thickness for all of the data
		{
			const Vector tmp(10, z.angle, 0.0f);
			Vector head;
			Math::AngleVectors(tmp, &head);
			head *= ((16.0f + 3.0f) + ((16.0f + 3.0f) * sin(DEG2RAD(10.0f)))) + 7.0f;
			head += local_position;
			head.z = local_position.z;
			const auto local_thickness = get_thickness(head, enemy_eyes, ent);//i really need my source for this bit, i forgot how it works entirely Autowall::GetThickness1(head, hacks.m_local_player, p_entity);
			z.thickness = local_thickness;

			if (local_thickness != 0) // if theres a thickness of 0 dont use this data
			{
				found = true;
			}
		}

		if (!found) // dont use
			continue;

		for (auto z = 0; points_copy.size() > z; z++)
			if (points_copy[z].thickness < points[z].thickness || points[z].thickness == -1) // find the lowest thickness so that we can hide our head best for all entities
				points[z].thickness = points_copy[z].thickness;

	}
	float best = 0;
	for (auto &i : points)
		if ((i.thickness > best || i.thickness == -1) && i.thickness != 0) // find the best hiding spot (highest thickness)
		{
			best = i.thickness;
			new_angle = i.angle;
		}

	if (best == 0)
		new_angle = FLT_MAX;
}

void c_antiaimbot::DoLBY(CUserCmd* cmd, bool *send_packet)
{
	if (!cheat::main::local())
		return;

	auto curtime = Source::m_pGlobalVars->curtime;
	auto animstate = cheat::main::local()->get_animation_state();

	if (!animstate || Source::m_pClientState->m_iChockedCommands)
		return;

	if (animstate->on_ground) {
		if (animstate->speed_2d > 0.1f)
			cheat::features::antiaimbot.m_next_lby_update_time = curtime + 0.22f;
		else {
			/*if (fabs(Math::AngleDiff(cheat::main::local()->m_flLowerBodyYawTarget(), cheat::main::real_angle)) < 35.f) {
				cheat::features::antiaimbot.m_next_lby_update_time = curtime + 1.1f + Source::m_pGlobalVars->interval_per_tick;
			}*/

			//auto parasha = fabs(Math::AngleDiff(animstate->eye_yaw, cheat::main::local()->m_flLowerBodyYawTarget()));

			if ((cheat::features::antiaimbot.m_next_lby_update_time - curtime) < 0.0f) {
				//if (parasha < 35.f) {
					cheat::features::antiaimbot.m_next_lby_update_time = curtime + 1.1f;
					cheat::features::antiaimbot.m_last_lby_update = curtime;
					cheat::features::antiaimbot.m_will_lby_update = true;
				//}
			}
		}
	}
}

void target_aim(CUserCmd* cmd,bool& pass) {
	auto angle = cmd->viewangles;
	float lowest = 99999999.f;
	Vector EyePos = cheat::main::local()->m_vecOrigin();

	for (int i = 1; i < 65; i++) {
		if (i == Source::m_pEngine->GetLocalPlayer()) continue;
		C_BasePlayer* pEnt = Source::m_pEntList->GetClientEntity(i);
		if (!pEnt) continue;
		if (pEnt->IsDormant()) continue;
		if (pEnt->IsDead()) continue;
		if (pEnt->m_iTeamNum() == cheat::main::local()->m_iTeamNum()) continue;

		//if (Vars.Ragebot.Antiaim.no_enemy)
		pass = true;

		Vector CurPos = pEnt->m_vecOrigin();

		auto dist = CurPos.Distance(EyePos);

		if (dist < lowest) {
			lowest = dist;
			angle = Math::CalcAngle(EyePos, CurPos);
		}
	}

	cmd->viewangles = angle;
}

void c_antiaimbot::change_angles(CUserCmd * cmd, bool &send_packet)
{
	auto IsGrenade = [](int item)
	{
		if (item == weapon_flashbang
			|| item == weapon_hegrenade
			|| item == weapon_smokegrenade
			|| item == weapon_molotov
			|| item == weapon_decoy
			|| item == weapon_incgrenade
			|| item == weapon_tagrenade)
			return true;
		else
			return false;
	};

	auto curtime = Source::m_pGlobalVars->curtime;

	float last_freestand_angle = 0.f, last_freestand_change_time = 0.f;

	auto animstate = cheat::main::local()->get_animation_state();

	if (!animstate || !cheat::Cvars.Antiaim_enable.GetValue() || cheat::features::antiaimbot.enable_delay > Source::m_pGlobalVars->curtime/* || fabs(last_drop_time - Source::m_pGlobalVars->realtime) < 0.2f*/) {
		last_fake_angle = last_real_angle = last_sent_angle = cmd->viewangles;
		return;
	}

	if ((int)cheat::Cvars.RageBot_exploits.GetValue() == 3 && !(cmd->command_number & 1) && !(cheat::game::pressed_keys[(int)cheat::Cvars.Exploits_fakeduck.GetValue()] && (int)cheat::Cvars.Exploits_fakeduck.GetValue() != 0 && cheat::main::local()->m_fFlags() & FL_ONGROUND))
	{
		switch ((int)cheat::Cvars.anti_aim_ex_pitch.GetValue())
		{
		case 1:
			cmd->viewangles.x = -89.f;
			break;
		case 2:
			cmd->viewangles.x = 89.f;
			break;
		case 3:
			cmd->viewangles.x = 0.f;
			break;
		default:
			break;
		}

		switch ((int)cheat::Cvars.anti_aim_ex_yaw.GetValue())
		{
		case 1:
			cmd->viewangles.y += 180.f;
			break;
		case 2:
			cmd->viewangles.y = last_real_angle.y + 180.f;
			break;
		case 3:
			cmd->viewangles.y = cheat::main::local()->m_flLowerBodyYawTarget() + 90.f;
			break;
		case 4:
			cmd->viewangles.y += Math::normalize_angle(Source::m_pGlobalVars->curtime * 250.f);
			break;
		default:
			break;
		}

		if (fabs(cheat::Cvars.anti_aim_ex_addyaw.GetValue()) > 0.0f)
			cmd->viewangles.y += cheat::Cvars.anti_aim_ex_addyaw.GetValue();

		cheat::features::antiaimbot.shift_ticks = 6;
		cheat::features::antiaimbot.do_tickbase = true;

		last_sent_origin = cheat::main::local()->m_vecOrigin();

		return;
	}

	auto is_moving = ((fabs(cheat::main::local()->m_vecVelocity().Length2D()) > 9.f && animstate->t_since_started_moving > 0.5f && cheat::main::local()->m_fFlags() & FL_ONGROUND && !cheat::main::fakewalking));

	int aa_mode = int(is_moving ? cheat::Cvars.anti_aim_m_yaw.GetValue() : cheat::Cvars.anti_aim_s_yaw.GetValue());
	float aa_add = float(is_moving ? cheat::Cvars.anti_aim_m_addyaw.GetValue() : cheat::Cvars.anti_aim_s_addyaw.GetValue());

	int aa_switch = int(is_moving ? cheat::Cvars.anti_aim_m_switchmode.GetValue() : cheat::Cvars.anti_aim_s_switchmode.GetValue());
	int aa_switchspeed = float(is_moving ? cheat::Cvars.anti_aim_m_switchspeed.GetValue() : cheat::Cvars.anti_aim_s_switchspeed.GetValue());
	int aa_switchang = int(is_moving ? cheat::Cvars.anti_aim_m_switchangle.GetValue() : cheat::Cvars.anti_aim_s_switchangle.GetValue());

	bool nreturn = false;
	if (cheat::Cvars.Antiaim_attargets.GetValue() && cheat::main::side == -1)
	{
		target_aim(cmd, nreturn);

		/*if (!nreturn) {
			last_real_angle = last_sent_angle = cmd->viewangles;
			return;
		}*/
	}

		switch ((int)cheat::Cvars.anti_aim_pitch.GetValue())
		{
		case 1:
			cmd->viewangles.x = 89.f;
			break;
		case 2:
			cmd->viewangles.x = 0.f;
			break;
		default:
			break;
		}

	static float lby = 0;

	if (cheat::main::side != -1)
	{
		switch (cheat::main::side)
		{
		case 0:
			cmd->viewangles.y += 90.f;
			break;
		case 1:
			cmd->viewangles.y -= 90.f;
			break;
		default: {
			cmd->viewangles.y += 180.f + (cheat::Cvars.Misc_AntiUT.GetValue() <= 0 ? RandInt(-40, 40) : 0);
			break;
		}
		}
	}
	else {
		switch (aa_mode)
		{
		case 1:
			cmd->viewangles.y += 180.f;
			break;
		case 2:
			break;
		case 3:
			cmd->viewangles.y = cheat::main::local()->m_flLowerBodyYawTarget() + 90.f;
			break;
		case 4:
			cmd->viewangles.y += Math::normalize_angle(Source::m_pGlobalVars->curtime * 250.f);
			break;
		default:
			break;
		}
	}

	//static auto allow_change = false;

	cmd->viewangles.y = Math::normalize_angle(cmd->viewangles.y);

	float base_angle = Math::normalize_angle(last_nonlby_angle.y + Math::normalize_angle(100.f + cheat::Cvars.anti_aim_desync_range.GetValue()) * (cheat::main::side == 0 ? 1.f : -1.f));

	if (cheat::main::side == -1 && cheat::Cvars.anti_aim_freestand.GetValue())
	{
		/*new_*/freestand(cheat::main::local(), base_angle);
	
		if (base_angle != FLT_MAX && cheat::main::side == -1)
			cmd->viewangles.y = Math::normalize_angle(base_angle);
	}

	if (fabs(aa_add) > 0.0f)
		cmd->viewangles.y += aa_add;

	if (((cheat::features::antiaimbot.m_next_lby_update_time - curtime) <= 0.02f || m_will_lby_update) && (int)cheat::Cvars.anti_aim_desync.GetValue() && !send_packet)
	{
		switch ((int)cheat::Cvars.anti_aim_desync.GetValue())
		{
		case 1:
			if (m_will_lby_update) 
				cmd->viewangles.y = Math::normalize_angle(last_nonlby_angle.y + Math::normalize_angle(100.f + cheat::Cvars.anti_aim_desync_range.GetValue()) * (cheat::main::side == 0 ? 1.f : -1.f));
			break;
		case 2:
			if (!m_will_lby_update)
				cmd->viewangles.y = Math::normalize_angle(last_nonlby_angle.y + Math::normalize_angle(10.f + cheat::Cvars.anti_aim_desync_range.GetValue()) * (cheat::main::side == 0 ? 1.f : -1.f));
			else 
				cmd->viewangles.y = Math::normalize_angle(last_nonlby_angle.y + Math::normalize_angle(10.f + cheat::Cvars.anti_aim_desync_range.GetValue()) * (cheat::main::side == 0 ? -1.f : 1.f));
			break;
		}

		//if (!m_will_lby_update && cheat::Cvars.Antiaim_flick_exploit.GetValue() && cmd->command_number % 3 <= 1 && cheat::main::local()->m_vecVelocity().Length2D() < 0.1f)
		//	cmd->tick_count = INT_MAX;

		m_will_lby_update = false;
	}
	else {
		last_nonlby_angle = cmd->viewangles;

		if (aa_switch > 0)
		{
			static float last_switch_ang = 0.f;
			static float last_switch_time = 0.f;

			switch (aa_switch)
			{
			case 1:
			{
				if (fabs(last_switch_time - Source::m_pGlobalVars->realtime) > (0.5f - (aa_switchspeed * 0.05f))) {
					auto lol = abs(aa_switchang);
					RandomSeed(cmd->command_number & 255);
					last_switch_ang = RandomFloat(-lol, lol);
					last_switch_time = Source::m_pGlobalVars->realtime;
				}

				break;
			}
			case 2:
			{
				cmd->viewangles.y -= (fabs(aa_switchang) / 2.f);

				if (fabs(last_switch_time - Source::m_pGlobalVars->realtime) > (0.5f - (aa_switchspeed * 0.05f)))
				{
					if (fabs(last_switch_ang) < fabs(aa_switchang))
						last_switch_ang += copysign(aa_switchspeed * 0.1f, aa_switchang);
					else
						last_switch_ang = 0.f;

					last_switch_time = Source::m_pGlobalVars->realtime;
				}

				break;
			}
			}

			cmd->viewangles.y += last_switch_ang;
		}
	}

	if (send_packet) {

		auto new_fake = cheat::main::local()->m_flLowerBodyYawTarget();

		if (fabs(Math::AngleDiff(cheat::main::local()->m_angEyeAngles().y, new_fake)) < 30.f)
			new_fake = Math::normalize_angle(100.f + cheat::Cvars.anti_aim_desync_range.GetValue()) * (cmd->command_number % 2 == 0 ? (cheat::main::side == 0 ? 1.f : -1.f) : (cheat::main::side == 0 ? -1.f : 1.f));

		cmd->viewangles.y = Math::normalize_angle(last_nonlby_angle.y + new_fake);
		last_fake_angle = cmd->viewangles;
	}
	else
	{
		cmd->viewangles.y = Math::normalize_angle(cmd->viewangles.y);
		last_real_angle = cmd->viewangles;
	}

	last_nonlby_angle.y = Math::normalize_angle(last_nonlby_angle.y);

	if (cheat::Cvars.Misc_AntiUT.GetValue())
		cmd->viewangles.Clamp();

	//prev_desync_state = desync_state;
}

int max_lag_time_after_started_accelerating = 0;

void c_antiaimbot::do_exloits(CUserCmd* cmd, bool& send_packet)
{
	if ((int)cheat::Cvars.RageBot_exploits.GetValue() == 0 || !cheat::main::local() || cheat::main::local()->IsDead() || (cheat::game::pressed_keys[(int)cheat::Cvars.Exploits_fakeduck.GetValue()] && (int)cheat::Cvars.Exploits_fakeduck.GetValue() != 0 && cheat::main::local()->m_fFlags() & FL_ONGROUND)) {
		cheat::features::antiaimbot.shift_ticks = 0;
		cheat::features::antiaimbot.do_tickbase = false;
		return;
	}

	if ((int)cheat::Cvars.RageBot_exploits.GetValue() == 3/* && cheat::features::antiaimbot.do_tickbase*/) {
		send_packet = true;
		return;
	}

	auto wpn = cheat::main::local()->get_weapon();

	if ((int)cheat::Cvars.RageBot_exploits.GetValue() == 1)
	{
		if (cmd->buttons & 1 && wpn && wpn->can_shoot())// && weapon->ready_to_shift())
			cheat::features::antiaimbot.shift_ticks = 6;
	}
	else if ((int)cheat::Cvars.RageBot_exploits.GetValue() == 2)
		cheat::features::antiaimbot.shift_ticks = 12;

	auto v13 = Source::m_pClientState->m_iChockedCommands - 1;

	if (v13 < 5) {
		if ((int)cheat::Cvars.RageBot_exploits.GetValue() == 2)
			cheat::features::antiaimbot.shift_ticks = v13;
	}
	else
		if (Source::m_pClientState->m_iChockedCommands >= 5)
		send_packet = true;
}

float accel_time;
bool was_moving = false;

void c_antiaimbot::simulate_lags(CUserCmd * cmd, bool * send_packet)
{
	if (cheat::main::fakewalking)
		return;

	auto exploit = (int)cheat::Cvars.RageBot_exploits.GetValue();

	auto is_fakeducking = (cheat::game::pressed_keys[(int)cheat::Cvars.Exploits_fakeduck.GetValue()] && (int)cheat::Cvars.Exploits_fakeduck.GetValue() != 0 && cheat::main::local()->m_fFlags() & FL_ONGROUND);;

	if (!cheat::Cvars.Misc_FakeLag.GetValue()) {
		if (cheat::Cvars.anti_aim_desync.GetValue() && !exploit)
			*send_packet = (cmd->command_number % 2 == 0);

		return;
	}

	if (exploit) {
		*send_packet = (Source::m_pClientState->m_iChockedCommands >= (is_fakeducking ? 16 : max((int)cheat::Cvars.Misc_fakelag_value.GetValue(), 6)));

		return;
	}

	auto net = Source::m_pEngine->GetNetChannelInfo();

	if (!net)
		return;

	static int choke_value = 0;
	auto a3 = (int)((cheat::Cvars.Misc_fakelag_variance.GetValue() / 100.f) * cheat::Cvars.Misc_fakelag_value.GetValue());
	/*cheat::game::pressed_keys[88]*/
	//if (v3->Fakelag.b_should_unchoke)
	//	v4 = 0;
	choke_value = (int)cheat::Cvars.Misc_fakelag_value.GetValue();

	auto pressed_move_key = (cmd->buttons & IN_MOVELEFT
		|| cmd->buttons & IN_MOVERIGHT
		|| cmd->buttons & IN_BACK
		|| cmd->buttons & IN_FORWARD);

	if (pressed_move_key && !was_moving && cheat::Cvars.Misc_fakelag_triggers.Has(4) && !cheat::main::fakewalking)
		accel_time = Source::m_pGlobalVars->realtime + 1.f;

	static bool m_bFakeDuck = false;
	auto new_buttons = 0;
	int v10 = cmd->buttons;

	if (a3 > 0 && !is_fakeducking)
		choke_value = (rand() % a3 * (2 * (Source::m_pGlobalVars->tickcount & 1) - 1));
	else if (is_fakeducking)
		choke_value = 16;

	//auto should_choke = false; 
	if (!skip_lags_this_tick) {

		auto is_moving = cheat::main::local()->m_vecVelocity().Length2D() > 0.1f && !cheat::main::fakewalking && cheat::Cvars.Misc_fakelag_triggers.Has(0);
		auto on_ground = !(cheat::main::local()->m_fFlags() & FL_ONGROUND) && cheat::Cvars.Misc_fakelag_triggers.Has(1);
		auto origin_delta = (last_sent_origin - cheat::main::local()->get_abs_origin()).LengthSquared();
		auto did_stop = Engine::Prediction::Instance()->GetVelocity().Length2D() < cheat::main::local()->m_vecVelocity().Length2D() && cheat::Cvars.Misc_fakelag_triggers.Has(2);

		auto should_fakelag = false;

		if (is_moving || on_ground || did_stop)
			should_fakelag = true;

		if (is_fakeducking)
			should_fakelag = true;

		if (accel_time > Source::m_pGlobalVars->realtime && cheat::Cvars.Misc_fakelag_triggers.Has(4) && !cheat::main::fakewalking)
			should_fakelag = true;

		if (cheat::main::fakewalking)
			should_fakelag = true;

		if (should_fakelag || (cheat::main::local()->m_vecVelocity().Length2D() > 0.1f && !cheat::main::fakewalking && cheat::Cvars.Misc_fakelag_triggers.Has(3)))
		{
			switch ((int)cheat::Cvars.Misc_fakelag_baseFactor.GetValue())
			{
			case 0:
				cheat::features::antiaimbot.unchoke = *send_packet = (Source::m_pClientState->m_iChockedCommands >= choke_value);
				break;
			case 1:
				cheat::features::antiaimbot.unchoke = *send_packet = (origin_delta > 4096.0 || Source::m_pClientState->m_iChockedCommands >= choke_value);
				break;
			case 2:
				if (Source::m_pClientState->m_iChockedCommands > (choke_value - 2))
					cheat::features::antiaimbot.unchoke = *send_packet = (Source::m_pClientState->m_iChockedCommands >= choke_value);
				else
				{
					cheat::features::antiaimbot.unchoke = *send_packet = (origin_delta > 4096.0 || Source::m_pClientState->m_iChockedCommands >= choke_value);
				}
				break;
			case 3:
				if (Source::m_pClientState->m_iChockedCommands > 7)
					cheat::features::antiaimbot.unchoke = *send_packet = (Source::m_pClientState->m_iChockedCommands >= 5);
				else
					cheat::features::antiaimbot.unchoke = *send_packet = (Source::m_pClientState->m_iChockedCommands >= 14);
				break;
			default:
				break;
			}

			if (cheat::main::local()->m_vecVelocity().Length2D() > 0.1f && !cheat::main::fakewalking && cheat::Cvars.Misc_fakelag_triggers.Has(3) && accel_time <= Source::m_pGlobalVars->realtime)
			{
				trace_t tr;
				Ray_t ray;
				CTraceFilter traceFilter;
				traceFilter.pSkip = cheat::main::local();
				auto origin = cheat::main::local()->predicted_eyepos();

				Vector forward = Vector::Zero;
				auto start_angle = Engine::Movement::Instance()->m_qRealAngles;

				start_angle.x = 0;

				auto sidemove = fabs(cmd->sidemove) > 20.f;

				if (sidemove)
				{
					if (cmd->sidemove > 0)
						start_angle.y -= 40.f;
					else
						start_angle.y += 40.f;
				}
				else
				{
					if (cmd->forwardmove > 0)
						start_angle.y -= 120.f;
					else
						start_angle.y += 120.f;
				}

				//Source::m_pDebugOverlay->AddBoxOverlay(origin, Vector(-3, -3, -3), Vector(3, 3, 3), Vector(0, 0, 0), 255, 0, 0, 255, 2.f * Source::m_pGlobalVars->interval_per_tick);

				Math::AngleVectors(start_angle, &forward);
				forward.z = 0;
				auto lol = origin + forward * 200.f;

				ray.Init(origin, lol);
				Source::m_pEngineTrace->TraceRay(ray, 0x201400B, &traceFilter, &tr);

				if (!tr.DidHit()) {
					cheat::features::antiaimbot.unchoke = *send_packet = (Source::m_pClientState->m_iChockedCommands >= (int)cheat::Cvars.Misc_fakelag_value.GetValue());
				}
				else
					if (!is_fakeducking && !*send_packet)
						*send_packet = (Source::m_pClientState->m_iChockedCommands >= ((int)cheat::Cvars.Misc_fakelag_value.GetValue() / 3));
			}
		}
		else
			cheat::features::antiaimbot.unchoke = *send_packet = Source::m_pClientState->m_iChockedCommands > 0;

		//if (!(Engine::Prediction::Instance()->GetFlags() & FL_ONGROUND) && cheat::main::local()->m_fFlags() & FL_ONGROUND /*&& (fabs(Math::NormalizeFloat(cheat::features::antiaimbot.last_real_angle.y - cheat::main::local()->m_flLowerBodyYawTarget())) / 180.f) > 0.0*/)
		//	*send_packet = false;

		if ((Source::m_pClientState->m_iChockedCommands + (TIME_TO_TICKS(net->GetLatency(FLOW_OUTGOING)) / 4)) >= (int)cheat::Cvars.Misc_fakelag_value.GetValue() && (!cheat::game::pressed_keys[(int)cheat::Cvars.Exploits_fakeduck.GetValue()] || (int)cheat::Cvars.Exploits_fakeduck.GetValue() == 0))
			cheat::features::antiaimbot.unchoke = *send_packet = true;

		//if ((Source::m_pEngine->IsVoiceRecording() || cheat::features::music.m_playing) && Source::m_pClientState->m_iChockedCommands >= 3)
		//	cheat::features::antiaimbot.unchoke = *send_packet = true;

		if (Source::m_pClientState->m_iChockedCommands > 0 && cheat::main::local()->m_vecVelocity().Length2D() <= 9.f && (!cheat::game::pressed_keys[(int)cheat::Cvars.Exploits_fakeduck.GetValue()] || (int)cheat::Cvars.Exploits_fakeduck.GetValue() == 0) && (int)cheat::Cvars.RageBot_exploits.GetValue() == 0)
			cheat::features::antiaimbot.unchoke = *send_packet = true;

		if (!*send_packet)
			cheat::main::prev_fakelag_value = Source::m_pClientState->m_iChockedCommands;
		//if (cheat::main::local()->m_fFlags() & FL_ONGROUND && cheat::main::local()->m_vecVelocity().Length() > 0.1f) {
		//	//for (int i = 0; i < 64; i++)
		//	//{
		//	//	auto entity = Source::m_pEntList->GetClientEntity(i);

		//	//	if (entity == nullptr || entity->IsDormant() || entity->m_iHealth() <= 0 || entity->m_iTeamNum() == cheat::main::local()->m_iTeamNum() || entity->IsDead())
		//	//		continue;

		//	//	const auto damage_1 = cheat::features::autowall.CanHit(entity->m_vecOrigin() + Vector(0, 0, 64.f), end_position_1, entity, cheat::main::local()),
		//	//		damage_2 = cheat::features::autowall.CanHit(entity->m_vecOrigin() + Vector(0, 0, 64.f), end_position_2, entity, cheat::main::local());



		//	//	if (max(damage_1, damage_2) > 0)
		//	//	{
		//	//		//printf("peeklag!\n");
		//	//		should_choke = true;
		//	//		break;
		//	//	}
		//	//} 

		//	if ((cmd->buttons & IN_MOVELEFT
		//		|| cmd->buttons & IN_MOVERIGHT
		//		|| cmd->buttons & IN_BACK
		//		|| cmd->buttons & IN_FORWARD) &&
		//		!(Engine::Prediction::Instance()->GetPrevCmd().buttons & IN_MOVELEFT
		//			|| Engine::Prediction::Instance()->GetPrevCmd().buttons & IN_MOVERIGHT
		//			|| Engine::Prediction::Instance()->GetPrevCmd().buttons & IN_BACK
		//			|| Engine::Prediction::Instance()->GetPrevCmd().buttons & IN_FORWARD) &&
		//		max_lag_time_after_started_accelerating <= Source::m_pGlobalVars->curtime)
		//		max_lag_time_after_started_accelerating = Source::m_pGlobalVars->curtime + 1.f;

		//	should_choke = (max_lag_time_after_started_accelerating >= Source::m_pGlobalVars->curtime) && !cheat::game::pressed_keys[88];
		//}

		was_moving = pressed_move_key;

		//if (!should_choke)
		//{
		//	if (cheat::game::pressed_keys[88] && cheat::main::local()->m_fFlags() & FL_ONGROUND)
		//		*send_packet = (packets_choked >= 14);
		//	else if (cheat::main::local()->m_fFlags() & FL_ONGROUND)
		//		*send_packet = packets_choked > (cheat::settings.misc_fakelag_value / 3);
		//	else
		//		*send_packet = int(ceil(64.f / (cheat::main::local()->m_vecVelocity().Length2D() * Source::m_pGlobalVars->interval_per_tick)) + 1) <= 0 && packets_choked >= 16;
		//}
		//else {
		//	*send_packet = (packets_choked >= cheat::settings.misc_fakelag_value || !should_choke);
		//	//printf("peeklag!\n");
		//}

		//bool is_doing_fakelag_activity = (cheat::game::pressed_keys[88] || (cheat::main::local()->m_vecVelocity().Length() > 0.1f && !cheat::main::fakewalking) || !(cheat::main::local()->m_fFlags() & FL_ONGROUND));

		//if (!is_doing_fakelag_activity && packets_choked > 0)
		//	*send_packet = true;

		//if ((Source::m_pEngine->IsVoiceRecording() || cheat::features::music.m_playing) && Source::m_pClientState->m_iChockedCommands >= 4)
		//	*send_packet = true;

		// Lag Jump
		if (!(Engine::Prediction::Instance()->GetFlags() & FL_ONGROUND) && cheat::main::local()->m_fFlags() & FL_ONGROUND /*&& (fabs(Math::NormalizeFloat(cheat::features::antiaimbot.last_real_angle.y - cheat::main::local()->m_flLowerBodyYawTarget())) / 180.f) > 0.0*/)
			cheat::features::antiaimbot.unchoke = *send_packet = false;

		//if ((packets_choked >= cheat::settings.misc_fakelag_value && !*send_packet) || packets_choked >= (cheat::settings.misc_fakelag_value + 1) || (packets_choked > 0 && skip_lags_this_tick && !cheat::main::fakewalking))
		//	*send_packet = true;

		!(*send_packet) ? ++packets_choked : packets_choked = 0;
	}
}

void c_antiaimbot::work(CUserCmd * cmd, bool * send_packet)
{
	auto state = cheat::main::local()->get_animation_state();

	if (cmd == nullptr || cheat::main::local() == nullptr || !state)
		return;

	auto local_weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon()));

	if (!cheat::main::local() || !local_weapon) return;

	if (cheat::game::get_key_press(cheat::Cvars.Misc_aaleft.GetValue()) && cheat::Cvars.Misc_aaleft.GetValue())
		cheat::main::side = (cheat::main::side == 0 ? -1 : 0);

	if (cheat::game::get_key_press(cheat::Cvars.Misc_aaright.GetValue()) && cheat::Cvars.Misc_aaright.GetValue())
		cheat::main::side = (cheat::main::side == 1 ? -1 : 1);

	if (cheat::game::get_key_press(cheat::Cvars.Misc_aaback.GetValue()) && cheat::Cvars.Misc_aaback.GetValue())
		cheat::main::side = (cheat::main::side == 2 ? -1 : 2);

	auto IsGrenade = [](int item)
	{
		if (item == weapon_flashbang
			|| item == weapon_hegrenade
			|| item == weapon_smokegrenade
			|| item == weapon_molotov
			|| item == weapon_decoy
			|| item == weapon_incgrenade
			|| item == weapon_tagrenade)
			return true;
		else
			return false;
	};

	if (!cheat::Cvars.RageBot_Enable.GetValue() || (local_weapon && IsGrenade(local_weapon->m_iItemDefinitionIndex()) && local_weapon->IsBeingThrowed(cmd))) {
		last_fake_angle = last_real_angle = last_sent_angle = cmd->viewangles;
		max_lag_time_after_started_accelerating = 0;
		return;
	}

	if (cheat::main::local()->m_MoveType() == 9 || cheat::main::local()->m_MoveType() == 8) {
		last_fake_angle = last_real_angle = last_sent_angle = cmd->viewangles;
		max_lag_time_after_started_accelerating = 0;
		return;
	}

	if (drop)
	{
		Source::m_pEngine->ClientCmd_Unrestricted("drop");
		last_fake_angle = last_real_angle = last_sent_angle = cmd->viewangles;
		*send_packet = true;
		drop = false;

		return;
	}

	/*DoLBY(cmd, send_packet);

	static bool did_fakewalk = false;

	if (cheat::game::pressed_keys[int(cheat::Cvars.anti_aim_slowwalk_key.GetValue())] && cheat::Cvars.anti_aim_slowwalk_key.GetValue() > 0.f) {

		if (!did_fakewalk) {
			Engine::Movement::Instance()->quick_stop(cmd);

			if (Engine::Prediction::Instance()->GetVelocity().Length2D() <= 0.1f)
				did_fakewalk = true;
		}

		cheat::main::fakewalking = true;

		static int choked = 0;
		choked = choked > 7 ? 0 : choked + 1;
		cmd->forwardmove = choked < 2 || choked > 5 ? 0 : cmd->forwardmove;
		cmd->sidemove = choked < 2 || choked > 5 ? 0 : cmd->sidemove;
		*send_packet = choked < 1;
	}
	else
		did_fakewalk = false;*/

	if (cmd->buttons & IN_USE)
	{
		last_fake_angle = last_real_angle = cmd->viewangles;

		/*auto delta = cheat::features::aaa.sub_59B13C30(state) * (cheat::Cvars.anti_aim_desync_range.GetValue() / 100.f);

		if (!(*send_packet)) {
			cmd->viewangles.y = last_real_angle.y - delta * cheat::main::fside;
			visual_real_angle = cmd->viewangles;

			if (cheat::main::fside < 0)
				visual_real_angle.y -= 20.f;
			else
				visual_real_angle.y += 20.f;
		}
		else
			last_sent_angle = visual_fake_angle = cmd->viewangles;*/

		if (cheat::Cvars.Misc_AntiUT.GetValue())
			cmd->viewangles.Clamp();

		return;
	}

	change_angles(cmd, *send_packet);

	//const auto old_delta = cheat::settings.ragebot_anti_aim_lby_delta;
	//cheat::settings.ragebot_anti_aim_lby_delta = cheat::features::aaa.sub_59B13C30(cheat::main::local()->get_animation_state());

	//cheat::settings.ragebot_anti_aim_lby_delta = old_delta;
}

inline float anglemod(float a)
{
	a = (360.f / 65536) * ((int)(a*(65536.f / 360.0f)) & 65535);
	return a;
}

float c_antiaimbot::some_func(float target, float value, float speed)
{
	//target = (target * 182.04445f) * 0.0054931641f;
	//value = (value * 182.04445f) * 0.0054931641f;

	//float delta = target - value;

	//// Speed is assumed to be positive
	//if (speed < 0)
	//	speed = -speed;

	//if (delta < -180.0f)
	//	delta += 360.0f;
	//else if (delta > 180.0f)
	//	delta -= 360.0f;

	//if (delta > speed)
	//	value += speed;
	//else if (delta < -speed)
	//	value -= speed;
	//else
	//	value = target;

	//return value;
	target = anglemod(target);
	value = anglemod(value);

	float delta = target - value;

	// Speed is assumed to be positive
	if (speed < 0)
		speed = -speed;

	if (delta < -180)
		delta += 360;
	else if (delta > 180)
		delta -= 360;

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;
}