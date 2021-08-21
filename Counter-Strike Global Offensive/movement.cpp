#include "movement.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "source.hpp"
#include "prediction.hpp"
#include "lag_compensation.hpp"
#include "rmenu.hpp"
#include "anti_aimbot.hpp"
#include <cmath>

#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)

namespace Engine
{

	/*
	* get_move_angle
	* Returns the movement angle from the current speed
	*/
	float Movement::get_move_angle(float speed)
	{
		auto move_angle = RAD2DEG(std::atan2(15.f, speed));
		//auto move_angle = RAD2DEG(std::atan2(30.f / speed));

		if (!isfinite(move_angle) || move_angle > 90.f)
			move_angle = 90.f;
		else if (move_angle < 0.f)
			move_angle = 0.f;

		return move_angle;
	}

	/*
	* get_closest_plane
	* Returns the closest plane
	*/
	bool Movement::get_closest_plane(Vector* plane)
	{
		trace_t trace;
		CTraceFilterWorldOnly filter; filter.pSkip = cheat::main::local();

		auto start = cheat::main::local()->m_vecOrigin(), mins = cheat::main::local()->OBBMins(), maxs = cheat::main::local()->OBBMaxs();

		Vector planes;
		auto count = 0;

		for (auto step = 0.f; step <= M_PI * 2.f; step += M_PI / 10.f)
		{
			auto end = start;

			end.x += cos(step) * 64.f;
			end.y += sin(step) * 64.f;

			Ray_t ray;
			ray.Init(start, end, mins, maxs);

			Source::m_pEngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

			if (trace.fraction < 1.f)
			{
				planes += trace.plane.normal;
				count++;
			}
		}

		planes /= count;

		if (planes.z < 0.1f)
		{
			*plane = planes;
			return true;
		}

		return false;
	}

	/*
	* will_hit_obstacle_in_future
	* Calculates if we will hit anything in the future
	*/
	bool Movement::will_hit_obstacle_in_future(float predict_time, float step)
	{
		static auto sv_gravity = Source::m_pCvar->FindVar("sv_gravity");
		static auto sv_jump_impulse = Source::m_pCvar->FindVar("sv_jump_impulse");

		bool ground = cheat::main::local()->get_animation_state() && cheat::main::local()->get_animation_state()->on_ground;
		auto gravity_per_tick = sv_gravity->GetFloat() * Source::m_pGlobalVars->interval_per_tick;

		auto start = cheat::main::local()->m_vecOrigin(), end = start;
		auto velocity = cheat::main::local()->m_vecVelocity();

		trace_t trace;
		CTraceFilterWorldOnly filter; filter.pSkip = cheat::main::local();

		auto predicted_ticks_needed = TIME_TO_TICKS(predict_time);
		auto velocity_rotation_angle = RAD2DEG(atan2(velocity.y, velocity.x));
		auto ticks_done = 0;

		if (predicted_ticks_needed <= 0)
			return false;

		while (true)
		{
			auto rotation_angle = velocity_rotation_angle + step;

			velocity.x = cos(DEG2RAD(rotation_angle)) * cheat::main::local()->m_vecVelocity().Length2D();
			velocity.y = sin(DEG2RAD(rotation_angle)) * cheat::main::local()->m_vecVelocity().Length2D();
			velocity.z = ground ? sv_jump_impulse->GetFloat() : velocity.z - gravity_per_tick;

			end += velocity * Source::m_pGlobalVars->interval_per_tick;

			Ray_t ray;
			ray.Init(start, end, cheat::main::local()->OBBMins(), cheat::main::local()->OBBMaxs());

			Source::m_pEngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

			if (trace.fraction != 1.f && trace.plane.normal.z <= 0.9f || trace.startsolid || trace.allsolid)
				break;

			end = trace.endpos;
			end.z -= 2.f;

			Ray_t ray2;
			ray2.Init(trace.endpos, end, cheat::main::local()->OBBMins(), cheat::main::local()->OBBMaxs());

			Source::m_pEngineTrace->TraceRay(ray2, MASK_PLAYERSOLID, &filter, &trace);
			ground = (trace.fraction < 1.f || trace.allsolid || trace.startsolid) && trace.plane.normal.z >= 0.7f;

			if (++ticks_done >= predicted_ticks_needed)
				return false;

			velocity_rotation_angle = rotation_angle;
		}

		return true;
	}

	/*
	* circle_strafe
	* Attempts to automatically strafe around and through objects.
	*/
	void Movement::circle_strafe(CUserCmd* cmd, float* circle_yaw)
	{
		auto velocity_2d = cheat::main::local()->m_vecVelocity().Length2D();

		auto benis = Source::m_pGlobalVars->interval_per_tick * 128.f;

		const auto min_step = benis * 2.25f;
		const auto max_step = benis * 5.f;

		auto ideal_strafe = std::clamp(get_move_angle(velocity_2d) * 2.f, min_step, max_step);
		auto predict_time = std::clamp(320.f / velocity_2d, 0.35f, 1.f);

		auto step = ideal_strafe;

		if (will_hit_obstacle_in_future(predict_time, step)) {
			while (true)
			{
				if (!will_hit_obstacle_in_future(predict_time, step) || step > max_step)
					break;

				step += 0.2f;
			}
		}

		if (step > max_step)
		{
			step = ideal_strafe;
			if (will_hit_obstacle_in_future(predict_time, step)) {
				while (true)
				{
					if (!will_hit_obstacle_in_future(predict_time, step) || step < -min_step)
						break;

					step -= 0.2f;
				}
			}

			if (step < -min_step)
			{
				Vector plane;
				if (get_closest_plane(&plane))
					step = -Math::normalize_angle(*circle_yaw - RAD2DEG(atan2(plane.y, plane.x))) * 0.1f;
			}
			else
				step -= 0.2f;
		}
		else
			step += 0.2f;

		auto yaw = Math::normalize_angle(*circle_yaw + step);

		m_qAnglesView.y = yaw;
		m_pCmd->sidemove = copysign(450.f, -step);
		*circle_yaw = yaw;
	}

	void Movement::predict_velocity(Vector* velocity) {
		static auto sv_friction = Source::m_pCvar->FindVar("sv_friction");
		static auto sv_stopspeed = Source::m_pCvar->FindVar("sv_stopspeed");

		float speed = velocity->Length();
		if (speed >= 0.1f) {
			float friction = sv_friction->GetFloat();
			float stop_speed = std::max< float >(speed, sv_stopspeed->GetFloat());
			float time = std::max< float >(Source::m_pGlobalVars->interval_per_tick, Source::m_pGlobalVars->frametime);
			*velocity *= std::max< float >(0.f, speed - friction * stop_speed * time / speed);
		}
	};

	void Movement::RotateMovement(CUserCmd* cmd, float yaw)
	{
		float rotation = DEG2RAD(m_qRealAngles.y - yaw);

		float cos_rot = cos(rotation);
		float sin_rot = sin(rotation);

		float new_forwardmove = (cos_rot * cmd->forwardmove) - (sin_rot * cmd->sidemove);
		float new_sidemove = (sin_rot * cmd->forwardmove) + (cos_rot * cmd->sidemove);

		cmd->forwardmove = new_forwardmove;
		cmd->sidemove = new_sidemove;
	}

	void Movement::quick_stop(CUserCmd* cmd) {
		/*auto vel = cheat::main::local()->m_vecVelocity();
		float speed = vel.Length2D();
		if (speed > 13.f) {
			QAngle direction = QAngle::Zero;
			Math::VectorAngles({ 0.f, 0.f, 0.f }, vel, direction);
			direction.y = cmd->viewangles.y - direction.y;

			Vector new_move = Vector::Zero;
			Math::AngleVectors(direction, &new_move);
			new_move *= -450.f;

			cmd->forwardmove = new_move.x;
			cmd->sidemove = new_move.y;
		}
		else {
			cmd->forwardmove = 0.f;
			cmd->sidemove = 0.f;
		}*/

		/*cmd->sidemove = 0;
		cmd->forwardmove = 450;

		RotateMovement(cmd, Math::CalcAngle(Vector(0, 0, 0), cheat::main::local()->m_vecVelocity()).y + 180.f);*/

		Vector hvel = cheat::main::local()->m_vecVelocity();
		hvel.z = 0;
		float speed = hvel.Length2D();

		if (speed < 1.f) // Will be clipped to zero anyways
		{
			cmd->forwardmove = 0.f;
			cmd->sidemove = 0.f;
			return;
		}

		// Homework: Get these dynamically
		static float accel = Source::m_pCvar->FindVar("sv_accelerate")->GetFloat();
		static float maxSpeed = Source::m_pCvar->FindVar("sv_maxspeed")->GetFloat();
		float playerSurfaceFriction = cheat::main::local()->m_surfaceFriction(); // I'm a slimy boi
		float max_accelspeed = accel * Source::m_pGlobalVars->interval_per_tick * maxSpeed * playerSurfaceFriction;

		float wishspeed{};

		// Only do custom deceleration if it won't end at zero when applying max_accel
		// Gamemovement truncates speed < 1 to 0
		if (speed - max_accelspeed <= -1.f)
		{
			// We try to solve for speed being zero after acceleration:
			// speed - accelspeed = 0
			// speed - accel*frametime*wishspeed = 0
			// accel*frametime*wishspeed = speed
			// wishspeed = speed / (accel*frametime)
			// ^ Theoretically, that's the right equation, but it doesn't work as nice as 
			//   doing the reciprocal of that times max_accelspeed, so I'm doing that :shrug:
			wishspeed = max_accelspeed / (speed / (accel * Source::m_pGlobalVars->interval_per_tick));
		}
		else // Full deceleration, since it won't overshoot
		{
			// Or use max_accelspeed, doesn't matter
			wishspeed = max_accelspeed;
		}

		// Calculate the negative movement of our velocity, relative to our viewangles
		Vector ndir = (hvel * -1.f); Math::VectorAngles(ndir, ndir);
		ndir.y = cmd->viewangles.y - ndir.y; // Relative to local view
		Math::AngleVectors(ndir, &ndir);

		cmd->forwardmove = ndir.x * wishspeed;
		cmd->sidemove = ndir.y * wishspeed;
	};

	inline void VectorMultiply(const Vector &a, float b, Vector &c)
	{
		CHECK_VALID(a);
		Assert(IsFinite(b));
		c.x = a.x * b;
		c.y = a.y * b;
		c.z = a.z * b;
	}

	inline void VectorAdd(const Vector &a, const Vector &b, Vector &c)
	{
		CHECK_VALID(a);
		CHECK_VALID(b);
		c.x = a.x + b.x;
		c.y = a.y + b.y;
		c.z = a.z + b.z;
	}

	inline void VectorSubtract(const Vector &a, const Vector &b, Vector &c)
	{
		CHECK_VALID(a);
		CHECK_VALID(b);
		c.x = a.x - b.x;
		c.y = a.y - b.y;
		c.z = a.z - b.z;
	}


	void Movement::Begin(CUserCmd* cmd, bool& send_packet)
	{
		m_pCmd = cmd;

		if (!cmd || !cheat::main::local())
			return;

		auto local_weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon()));

		if (!m_pCmd || !local_weapon)
			return;

		m_pPlayer = C_CSPlayer::GetLocalPlayer();

		if (!m_pPlayer || m_pPlayer->IsDead())
			return;

		Source::m_pEngine->GetViewAngles(m_qRealAngles);

		static auto circle_yaw = 0.f;
		m_qAngles = m_pCmd->viewangles;
		m_qAnglesView = m_pCmd->viewangles;

		auto& prediction = Prediction::Instance();

		if (!(cheat::main::local()->m_fFlags() & FL_ONGROUND) && !(cheat::game::pressed_keys[16] && local_weapon->m_iItemDefinitionIndex() == weapon_ssg08))
		{
			if (cheat::Cvars.Misc_AutoJump.GetValue())
				m_pCmd->buttons &= ~IN_JUMP;

			if (cheat::game::pressed_keys[(int)cheat::Cvars.Misc_CircleStrafer.GetValue()] && cheat::Cvars.Misc_CircleStrafer.GetValue() > 0.f) {
				circle_strafe(cmd, &circle_yaw);
			}
			else if (cheat::Cvars.Misc_AutoStrafe.GetValue() && !cheat::game::pressed_keys[(int)cheat::Cvars.Exploits_fakeduck.GetValue()] || !(int)cheat::Cvars.Exploits_fakeduck.GetValue())
			{
				static int old_yaw;

				auto get_velocity_degree = [](float length_2d)
				{
					auto tmp = RAD2DEG(atan(30.f / length_2d));

					if (CheckIfNonValidNumber(tmp) || tmp > 90.f)
						return 90.f;

					else if (tmp < 0.f)
						return 0.f;
					else
						return tmp;
				};

				if (cheat::main::local()->m_MoveType() != MOVETYPE_WALK)
					return;

				auto velocity = cheat::main::local()->m_vecVelocity();
				velocity.z = 0;

				static auto flip = false;
				auto turn_direction_modifier = (flip) ? 1.f : -1.f;
				flip = !flip;

				float forwardmove = 0.f;

				auto sidemove = m_pCmd->sidemove;

				if (((forwardmove = m_pCmd->forwardmove, forwardmove != 0.0) || sidemove != 0.0) && cheat::Cvars.Misc_AutoStrafeWASD.GetValue())
				{
					m_pCmd->forwardmove = 0.0;
					m_pCmd->sidemove = 0.0;
					auto v26 = atan2(sidemove * -1, forwardmove);
					m_qAnglesView.y += (v26 * 57.295776);
				}
				else if (m_pCmd->forwardmove > 0.f)
					m_pCmd->forwardmove = 0.f;

				auto velocity_length_2d = velocity.Length2D();

				auto strafe_angle = RAD2DEG(atan(15.f / velocity_length_2d));

				if (strafe_angle > 90.f)
					strafe_angle = 90.f;

				else if (strafe_angle < 0.f)
					strafe_angle = 0.f;

				Vector Buffer(0, m_qAnglesView.y - old_yaw, 0);
				Buffer.y = Math::NormalizeFloat(Buffer.y);

				int yaw_delta = Buffer.y;
				old_yaw = m_qAnglesView.y;

				if (yaw_delta > 0.f)
					m_pCmd->sidemove = -450.f;

				else if (yaw_delta < 0.f)
					m_pCmd->sidemove = 450.f;

				auto abs_yaw_delta = abs(yaw_delta);

				if (abs_yaw_delta <= strafe_angle || abs_yaw_delta >= 30.f)
				{
					Vector velocity_angles;
					Math::VectorAngles(velocity, velocity_angles);

					Buffer = Vector(0, m_qAnglesView.y - velocity_angles.y, 0);
					Buffer.y = Math::NormalizeFloat(Buffer.y);
					int velocityangle_yawdelta = Buffer.y;

					auto velocity_degree = get_velocity_degree(velocity_length_2d) * 2.f; // retrack value, for teleporters

					if (velocityangle_yawdelta <= velocity_degree || velocity_length_2d <= 15.f)
					{
						if (-(velocity_degree) <= velocityangle_yawdelta || velocity_length_2d <= 15.f)
						{
							m_qAnglesView.y += (strafe_angle * turn_direction_modifier);
							m_pCmd->sidemove = 450.f * turn_direction_modifier;
						}

						else
						{
							m_qAnglesView.y = velocity_angles.y - velocity_degree;
							m_pCmd->sidemove = 450.f;
						}
					}

					else
					{
						m_qAnglesView.y = velocity_angles.y + velocity_degree;
						m_pCmd->sidemove = -450.f;
					}


				}

				circle_yaw = m_qAnglesView.y;
			}
		}
		else
		{
			/*	auto friction = [](Vector & out_vel) -> void {
					float speed, newspeed, control;
					float friction;
					float drop;

					speed = out_vel.Length();

					if (speed <= 0.1f)
						return;

					drop = 0;

					if (cheat::main::local()->m_fFlags() & FL_ONGROUND) {
						friction = Source::m_pCvar->FindVar("sv_friction")->GetFloat() * cheat::main::local()->m_surfaceFriction();

						control = (speed < Source::m_pCvar->FindVar("sv_stopspeed")->GetFloat()) ? Source::m_pCvar->FindVar("sv_stopspeed")->GetFloat() : speed;

						drop += control * friction * Source::m_pGlobalVars->frametime;
					}

					newspeed = speed - drop;
					if (newspeed < 0)
						newspeed = 0;

					if (newspeed != speed) {
						newspeed /= speed;

						VectorMultiply(out_vel, newspeed, out_vel);
					}
				};

				auto walk_move = [](C_BasePlayer * e, Vector & out_vel) -> void
				{
					auto accelerate = [](C_BasePlayer * e, Vector & wishdir, float wishspeed, float accel, Vector & out_vel)
					{
						float currentspeed = out_vel.Dot(wishdir);

						float addspeed = wishspeed - currentspeed;

						if (addspeed <= 0)
							return;

						float accelspeed = accel * Source::m_pGlobalVars->frametime * wishspeed * e->m_surfaceFriction();

						if (accelspeed > addspeed)
							accelspeed = addspeed;

						for (int i = 0; i < 3; i++)
							out_vel[i] += accelspeed * wishdir[i];
					};

					Vector forward, right, up, wishvel, wishdir, dest;
					float fmove, smove, wishspeed;

					Math::AngleVectors(e->m_angEyeAngles(), &forward, &right, &up);

					Source::m_pMoveHelper->SetHost(e);
					fmove = Source::m_pMoveHelper->m_flForwardMove;
					smove = Source::m_pMoveHelper->m_flSideMove;
					Source::m_pMoveHelper->SetHost(nullptr);

					if (forward[2] != 0) {
						forward[2] = 0;
						forward.Normalize();
					}

					if (right[2] != 0) {
						right[2] = 0;
						right.Normalize();
					}

					for (int i = 0; i < 2; i++)
						wishvel[i] = forward[i] * fmove + right[i] * smove;

					wishvel[2] = 0;

					wishdir = wishvel;
					wishspeed = wishdir.Normalize();


					Source::m_pMoveHelper->SetHost(e);
					if ((wishspeed != 0.0f) && (wishspeed > Source::m_pMoveHelper->m_flMaxSpeed)) {
						VectorMultiply(wishvel, e->m_flMaxspeed() / wishspeed, wishvel);
						wishspeed = e->m_flMaxspeed();
					}
					Source::m_pMoveHelper->SetHost(nullptr);

					out_vel[2] = 0;
					accelerate(e, wishdir, wishspeed, Source::m_pCvar->FindVar("sv_accelerate")->GetFloat(), out_vel);
					out_vel[2] = 0;

					VectorAdd(out_vel, e->m_vecBaseVelocity(), out_vel);

					float spd = out_vel.Length();

					if (spd < 1.0f) {
						out_vel.clear();

						VectorSubtract(out_vel, e->m_vecBaseVelocity(), out_vel);
						return;
					}

					Source::m_pMoveHelper->SetHost(e);
					Source::m_pMoveHelper->m_outWishVel += wishdir * wishspeed;
					Source::m_pMoveHelper->SetHost(nullptr);

					if (!(e->m_fFlags() & FL_ONGROUND)) {
						VectorSubtract(out_vel, e->m_vecBaseVelocity(), out_vel);
						return;
					}

					VectorSubtract(out_vel, e->m_vecBaseVelocity(), out_vel);
				};*/
		}
	}

	void Movement::Fix_Movement(CUserCmd* cmd, QAngle original_angles)
	{
		Vector wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

		auto viewangles = cmd->viewangles;
		auto movedata = Vector(cmd->forwardmove, cmd->sidemove, cmd->upmove);
		viewangles.Normalize();

		if (!(cheat::main::local()->m_fFlags() & FL_ONGROUND) && viewangles.z != 0.f)
			movedata.y = 0.f;

		Math::AngleVectors(original_angles, &wish_forward, &wish_right, &wish_up);
		Math::AngleVectors(viewangles, &cmd_forward, &cmd_right, &cmd_up);

		const auto v8 = sqrtf(wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y), v10 = sqrtf(wish_right.x * wish_right.x + wish_right.y * wish_right.y), v12 = sqrtf(wish_up.z * wish_up.z);

		Vector wish_forward_norm(1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f),
			wish_right_norm(1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f),
			wish_up_norm(0.f, 0.f, 1.0f / v12 * wish_up.z);

		const auto v14 = sqrtf(cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y), v16 = sqrtf(cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y), v18 = sqrtf(cmd_up.z * cmd_up.z);

		Vector cmd_forward_norm(1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f),
			cmd_right_norm(1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f),
			cmd_up_norm(0.f, 0.f, 1.0f / v18 * cmd_up.z);

		const auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 = wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 = wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 = wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 = wish_up_norm.y * movedata.z;

		Vector correct_movement = Vector::Zero;

		correct_movement.x = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25
			+ (cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28)
			+ (cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27);
		correct_movement.y = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25
			+ (cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28)
			+ (cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27);
		correct_movement.z = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25
			+ (cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28)
			+ (cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27);

		correct_movement.x = Math::clamp(correct_movement.x, -450.f, 450.f);
		correct_movement.y = Math::clamp(correct_movement.y, -450.f, 450.f);
		correct_movement.z = Math::clamp(correct_movement.z, -320.f, 320.f);

		cmd->forwardmove = correct_movement.x;
		cmd->sidemove = correct_movement.y;
		cmd->upmove = correct_movement.z;
	}

	void Movement::End(CUserCmd* cmd)
	{
		//aimware
		if (!cmd || !cheat::main::local())
			return;

		Fix_Movement(cmd, m_qAnglesView);
	}

}