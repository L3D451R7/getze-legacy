#include "game_movement.h"
#include "math.h"
#include "player.hpp"
#include "source.hpp"

void RebuildGameMovement::SetAbsOrigin(C_BasePlayer *player, const Vector &vec)
{
	player->set_abs_origin(vec);
	player->m_vecOrigin() = vec;
}

inline void VectorMultiply(const Vector &a, float b, Vector &c)
{
	CHECK_VALID(a);
	Assert(IsFinite(b));
	c.x = a.x * b;
	c.y = a.y * b;
	c.z = a.z * b;
}

inline void VectorMA(const Vector &start, float scale, const Vector &direction, Vector &dest)
{
	CHECK_VALID(start);
	CHECK_VALID(direction);

	dest.x = start.x + scale * direction.x;
	dest.y = start.y + scale * direction.y;
	dest.z = start.z + scale * direction.z;
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

int RebuildGameMovement::ClipVelocity(Vector &in, Vector &normal, Vector &out, float overbounce)
{
	float	backoff;
	float	change;
	float angle;
	int		i, blocked;

	angle = normal[2];

	blocked = 0x00;         // Assume unblocked.
	if (angle > 0)			// If the plane that is blocking us has a positive z component, then assume it's a floor.
		blocked |= 0x01;	// 
	if (!angle)				// If the plane has no Z, it is vertical (wall/step)
		blocked |= 0x02;	// 

							// Determine how far along plane to slide based on incoming direction.
	backoff = in.Dot(normal) * overbounce;

	for (i = 0; i<3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
	}

	// iterate once to make sure we aren't still moving through the plane
	float adjust = out.Dot(normal);
	if (adjust < 0.0f)
	{
		out -= (normal * adjust);
		printf( "Adjustment = %lf\n", adjust );
	}

	// Return blocking flags.
	return blocked;
}

int RebuildGameMovement::TryPlayerMove(C_BasePlayer *player, Vector *pFirstDest, trace_t *pFirstTrace)
{
	Vector  planes[5];
	numbumps[player->entindex()] = 4;           // Bump up to four times

	blocked[player->entindex()] = 0;           // Assume not blocked
	numplanes[player->entindex()] = 0;           //  and not sliding along any planes

	original_velocity[player->entindex()] = player->m_vecVelocity(); // Store original velocity
	primal_velocity[player->entindex()] = player->m_vecVelocity();

	allFraction[player->entindex()] = 0;
	time_left[player->entindex()] = Source::m_pGlobalVars->frametime;   // Total time for this movement operation.

	new_velocity[player->entindex()].clear();

	for (bumpcount[player->entindex()] = 0; bumpcount[player->entindex()] < numbumps[player->entindex()]; bumpcount[player->entindex()]++)
	{
		if (player->m_vecVelocity().Length() == 0.0)
			break;

		// Assume we can move all the way from the current origin to the
		//  end point.
		VectorMA(player->get_abs_origin(), time_left[player->entindex()], player->m_vecVelocity(), end[player->entindex()]);

		// See if we can make it from origin to end point.
		if (true)
		{
			// If their velocity Z is 0, then we can avoid an extra trace here during WalkMove.
			if (pFirstDest && end[player->entindex()] == *pFirstDest)
				pm[player->entindex()] = *pFirstTrace;
			else
			{
				TracePlayerBBox(player->get_abs_origin(), end[player->entindex()], MASK_PLAYERSOLID, 8, pm[player->entindex()], player);
			}
		}
		else
		{
			TracePlayerBBox(player->get_abs_origin(), end[player->entindex()], MASK_PLAYERSOLID, 8, pm[player->entindex()], player);
		}

		allFraction[player->entindex()] += pm[player->entindex()].fraction;

		// If we started in a solid object, or we were in solid space
		//  the whole way, zero out our velocity and return that we
		//  are blocked by floor and wall.
		if (pm[player->entindex()].allsolid)
		{
			// C_BasePlayer is trapped in another solid
			player->m_vecVelocity() = vec3_origin[player->entindex()];
			return 4;
		}

		// If we moved some portion of the total distance, then
		//  copy the end position into the pmove.origin and 
		//  zero the plane counter.
		if (pm[player->entindex()].fraction > 0)
		{
			if (numbumps[player->entindex()] > 0 && pm[player->entindex()].fraction == 1)
			{
				// There's a precision issue with terrain tracing that can cause a swept box to successfully trace
				// when the end position is stuck in the triangle.  Re-run the test with an uswept box to catch that
				// case until the bug is fixed.
				// If we detect getting stuck, don't allow the movement
				trace_t stuck;
				TracePlayerBBox(pm[player->entindex()].endpos, pm[player->entindex()].endpos, MASK_PLAYERSOLID, 8, stuck, player);
				if (stuck.startsolid || stuck.fraction != 1.0f)
				{
					printf( "Player will become stuck!!!\n" );
					player->m_vecVelocity() = vec3_origin[player->entindex()];
					break;
				}
			}

			// actually covered some distance
			SetAbsOrigin(player, pm[player->entindex()].endpos);
			original_velocity[player->entindex()] = player->m_vecVelocity();
			numplanes[player->entindex()] = 0;
		}

		// If we covered the entire distance, we are done
		//  and can return.
		if (pm[player->entindex()].fraction == 1)
		{
			break;		// moved the entire distance
		}

		// If the plane we hit has a high z component in the normal, then
		//  it's probably a floor
		if (pm[player->entindex()].plane.normal[2] > 0.7)
		{
			blocked[player->entindex()] |= 1;		// floor
		}
		// If the plane has a zero z component in the normal, then it's a 
		//  step or wall
		if (!pm[player->entindex()].plane.normal[2])
		{
			blocked[player->entindex()] |= 2;		// step / wall
		}

		// Reduce amount of m_flFrameTime left by total time left * fraction
		//  that we covered.
		time_left[player->entindex()] -= time_left[player->entindex()] * pm[player->entindex()].fraction;

		// Did we run out of planes to clip against?
		if (numplanes[player->entindex()] >= 5)
		{
			// this shouldn't really happen
			//  Stop our movement if so.
			player->m_vecVelocity() = vec3_origin[player->entindex()];
			printf("Too many planes 4\n");

			break;
		}

		// Set up next clipping plane
		planes[numplanes[player->entindex()]] = pm[player->entindex()].plane.normal;
		numplanes[player->entindex()]++;

		// modify original_velocity so it parallels all of the clip planes
		//

		// reflect player velocity 
		// Only give this a try for first impact plane because you can get yourself stuck in an acute corner by jumping in place
		//  and pressing forward and nobody was really using this bounce/reflection feature anyway...
		if (numplanes[player->entindex()] == 1 &&
			player->m_fFlags() & FL_ONGROUND)
		{
			for (i[player->entindex()] = 0; i[player->entindex()] < numplanes[player->entindex()]; i[player->entindex()]++)
			{
				if (planes[i[player->entindex()]][2] > 0.7)
				{
					// floor or slope
					ClipVelocity(original_velocity[player->entindex()], planes[i[player->entindex()]], new_velocity[player->entindex()], 1);
					original_velocity[player->entindex()] = new_velocity[player->entindex()];
				}
				else
				{
					ClipVelocity(original_velocity[player->entindex()], planes[i[player->entindex()]], new_velocity[player->entindex()], 1.0 + Source::m_pCvar->FindVar("sv_bounce")->GetFloat() * (1 - player->m_surfaceFriction()));
				}
			}

			player->m_vecVelocity() = new_velocity[player->entindex()];
			original_velocity[player->entindex()] = new_velocity[player->entindex()];
		}
		else
		{
			for (i[player->entindex()] = 0; i[player->entindex()] < numplanes[player->entindex()]; i[player->entindex()]++)
			{


				for (j[player->entindex()] = 0; j[player->entindex()]<numplanes[player->entindex()]; j[player->entindex()]++)
					if (j[player->entindex()] != i[player->entindex()])
					{
						// Are we now moving against this plane?
						if (player->m_vecVelocity().Dot(planes[j[player->entindex()]]) < 0)
							break;	// not ok
					}
				if (j[player->entindex()] == numplanes[player->entindex()])  // Didn't have to clip, so we're ok
					break;
			}

			// Did we go all the way through plane set
			if (i[player->entindex()] != numplanes[player->entindex()])
			{	// go along this plane
				// pmove.velocity is set in clipping call, no need to set again.
				;
			}
			else
			{	// go along the crease
				if (numplanes[player->entindex()] != 2)
				{
					player->m_vecVelocity() = vec3_origin[player->entindex()];
					break;
				}

				dir[player->entindex()] = planes[0].Cross(planes[1]);
				dir[player->entindex()].NormalizeInPlace();
				d[player->entindex()] = dir[player->entindex()].Dot(player->m_vecVelocity());
				VectorMultiply(dir[player->entindex()], d[player->entindex()], player->m_vecVelocity());
			}

			//
			// if original velocity is against the original velocity, stop dead
			// to avoid tiny occilations in sloping corners
			//
			d[player->entindex()] = player->m_vecVelocity().Dot(primal_velocity[player->entindex()]);
			if (d[player->entindex()] <= 0)
			{
				printf("Back\n");
				player->m_vecVelocity() = vec3_origin[player->entindex()];
				break;
			}
		}
	}

	if (allFraction == 0)
	{
		player->m_vecVelocity() = vec3_origin[player->entindex()];
	}

	// Check if they slammed into a wall
	float fSlamVol = 0.0f;

	float fLateralStoppingAmount = primal_velocity[player->entindex()].Length2D() - player->m_vecVelocity().Length2D();
	if (fLateralStoppingAmount > 580.f * 2.0f)
	{
		fSlamVol = 1.0f;
	}
	else if (fLateralStoppingAmount > 580.f)
	{
		fSlamVol = 0.85f;
	}

	return blocked[player->entindex()];
}

void RebuildGameMovement::Accelerate(C_BasePlayer *player, Vector &wishdir, float wishspeed, float accel)
{
	// See if we are changing direction a bit
	currentspeed[player->entindex()] = player->m_vecVelocity().Dot(wishdir);

	// Reduce wishspeed by the amount of veer.
	addspeed[player->entindex()] = wishspeed - currentspeed[player->entindex()];

	// If not going to add any speed, done.
	if (addspeed[player->entindex()] <= 0)
		return;

	// Determine amount of accleration.
	accelspeed[player->entindex()] = accel * Source::m_pGlobalVars->frametime * wishspeed * player->m_surfaceFriction();

	// Cap at addspeed
	if (accelspeed[player->entindex()] > addspeed[player->entindex()])
		accelspeed[player->entindex()] = addspeed[player->entindex()];

	// Adjust velocity.
	for (i[player->entindex()] = 0; i[player->entindex()]<3; i[player->entindex()]++)
	{
		player->m_vecVelocity()[i[player->entindex()]] += accelspeed[player->entindex()] * wishdir[i[player->entindex()]];
	}
}

Vector RebuildGameMovement::Accelerate(C_BasePlayer *player, Vector &new_vel, Vector &wishdir, float wishspeed, float accel)
{

	// See if we are changing direction a bit
	currentspeed[player->entindex()] = player->m_vecVelocity().Dot(wishdir);

	// Reduce wishspeed by the amount of veer.
	addspeed[player->entindex()] = wishspeed - currentspeed[player->entindex()];

	// If not going to add any speed, done.
	if (addspeed[player->entindex()] <= 0)
		return new_vel;

	// Determine amount of accleration.
	accelspeed[player->entindex()] = accel * Source::m_pGlobalVars->frametime * wishspeed * player->m_surfaceFriction();

	// Cap at addspeed
	if (accelspeed[player->entindex()] > addspeed[player->entindex()])
		accelspeed[player->entindex()] = addspeed[player->entindex()];

	// Adjust velocity.
	for (auto i = 0; i<3; i++)
	{
		new_vel[i] += accelspeed[player->entindex()] * wishdir[i];
	}

	return new_vel;
}

void RebuildGameMovement::AirAccelerate(C_BasePlayer *player, Vector &wishdir, float wishspeed, float accel)
{

	wishspd[player->entindex()] = wishspeed;

	// Cap speed
	if (wishspd[player->entindex()] > 30.f)
		wishspd[player->entindex()] = 30.f;

	// Determine veer amount
	currentspeed[player->entindex()] = player->m_vecVelocity().Dot(wishdir);

	// See how much to add
	addspeed[player->entindex()] = wishspd[player->entindex()] - currentspeed[player->entindex()];

	// If not adding any, done.
	if (addspeed <= 0)
		return;

	// Determine acceleration speed after acceleration
	accelspeed[player->entindex()] = accel * wishspeed *Source::m_pGlobalVars->frametime * player->m_surfaceFriction();

	// Cap it
	if (accelspeed[player->entindex()] > addspeed[player->entindex()])
		accelspeed[player->entindex()] = addspeed[player->entindex()];

	// Adjust pmove vel.
	for (i[player->entindex()] = 0; i[player->entindex()]<3; i[player->entindex()]++)
	{
		player->m_vecVelocity()[i[player->entindex()]] += accelspeed[player->entindex()] * wishdir[i[player->entindex()]];
		Source::m_pMoveHelper->SetHost(player);
		Source::m_pMoveHelper->m_outWishVel[i[player->entindex()]] += accelspeed[player->entindex()] * wishdir[i[player->entindex()]];

	}
}

void RebuildGameMovement::AirMove(C_BasePlayer *player)
{
	Math::AngleVectors(player->m_angEyeAngles(), &forward[player->entindex()], &right[player->entindex()], &up[player->entindex()]);  // Determine movement angles

																																   // Copy movement amounts
	Source::m_pMoveHelper->SetHost(player);
	fmove[player->entindex()] = Source::m_pMoveHelper->m_flForwardMove;
	smove[player->entindex()] = Source::m_pMoveHelper->m_flSideMove;

	// Zero out z components of movement vectors
	forward[player->entindex()][2] = 0;
	right[player->entindex()][2] = 0;
	(forward[player->entindex()]).Normalize();  // Normalize remainder of vectors
	(right[player->entindex()]).Normalize();    // 

	for (i[player->entindex()] = 0; i[player->entindex()]<2; i[player->entindex()]++)       // Determine x and y parts of velocity
		wishvel[player->entindex()][i[player->entindex()]] = forward[player->entindex()][i[player->entindex()]] * fmove[player->entindex()] + right[player->entindex()][i[player->entindex()]] * smove[player->entindex()];

	wishvel[player->entindex()][2] = 0;             // Zero out z part of velocity

	wishdir[player->entindex()] = wishvel[player->entindex()]; // Determine maginitude of speed of move
	wishspeed[player->entindex()] = wishdir[player->entindex()].Normalize();

	//
	// clamp to server defined max speed
	//
	if (wishspeed != 0 && (wishspeed[player->entindex()] > player->m_flMaxspeed()))
	{
		VectorMultiply(wishvel[player->entindex()], player->m_flMaxspeed() / wishspeed[player->entindex()], wishvel[player->entindex()]);
		wishspeed[player->entindex()] = player->m_flMaxspeed();
	}

	AirAccelerate(player, wishdir[player->entindex()], wishspeed[player->entindex()], Source::m_pCvar->FindVar("sv_airaccelerate")->GetFloat());

	// Add in any base velocity to the current velocity.
	VectorAdd(player->m_vecVelocity(), player->m_vecBaseVelocity(), player->m_vecVelocity());
	trace_t trace;
	TryPlayerMove(player, &dest[player->entindex()], &trace);

	// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
	VectorSubtract(player->m_vecVelocity(), player->m_vecBaseVelocity(), player->m_vecVelocity());
}

void RebuildGameMovement::StepMove(C_BasePlayer *player, Vector &vecDestination, trace_t &trace)
{
	Vector vecEndPos;
	vecEndPos = vecDestination;

	// Try sliding forward both on ground and up 16 pixels
	//  take the move that goes farthest
	Vector vecPos, vecVel;
	vecPos = player->get_abs_origin();
	vecVel = player->m_vecVelocity();

	// Slide move down.
	TryPlayerMove(player, &vecEndPos, &trace);

	// Down results.
	Vector vecDownPos, vecDownVel;
	vecDownPos = player->get_abs_origin();
	vecDownVel = player->m_vecVelocity();

	// Reset original values.
	SetAbsOrigin(player, vecPos);
	player->m_vecVelocity() = vecVel;

	// Move up a stair height.
	vecEndPos = player->get_abs_origin();

	vecEndPos.z += player->m_flStepSize() + 0.03125;



	TracePlayerBBox(player->get_abs_origin(), vecEndPos, MASK_PLAYERSOLID, 8, trace, player);
	if (!trace.startsolid && !trace.allsolid)
	{
		SetAbsOrigin(player, trace.endpos);
	}

	TryPlayerMove(player, &dest[player->entindex()], &trace);

	// Move down a stair (attempt to).
	vecEndPos = player->get_abs_origin();

	vecEndPos.z -= player->m_flStepSize() + 0.03125;


	TracePlayerBBox(player->get_abs_origin(), vecEndPos, MASK_PLAYERSOLID, 8, trace, player);

	// If we are not on the ground any more then use the original movement attempt.
	if (trace.plane.normal[2] < 0.7)
	{
		SetAbsOrigin(player, vecDownPos);
		player->m_vecVelocity() = vecDownVel;

		float flStepDist = player->get_abs_origin().z - vecPos.z;
		if (flStepDist > 0.0f)
		{
			Source::m_pMoveHelper->SetHost(player);
			Source::m_pMoveHelper->m_outStepHeight += flStepDist;
			Source::m_pMoveHelper->SetHost(nullptr);
		}
		return;
	}

	// If the trace ended up in empty space, copy the end over to the origin.
	if (!trace.startsolid && !trace.allsolid)
	{
		player->set_abs_origin(trace.endpos);
	}

	// Copy this origin to up.
	Vector vecUpPos;
	vecUpPos = player->get_abs_origin();

	// decide which one went farther
	float flDownDist = (vecDownPos.x - vecPos.x) * (vecDownPos.x - vecPos.x) + (vecDownPos.y - vecPos.y) * (vecDownPos.y - vecPos.y);
	float flUpDist = (vecUpPos.x - vecPos.x) * (vecUpPos.x - vecPos.x) + (vecUpPos.y - vecPos.y) * (vecUpPos.y - vecPos.y);
	if (flDownDist > flUpDist)
	{
		SetAbsOrigin(player, vecDownPos);
		player->m_vecVelocity() = vecDownVel;
	}
	else
	{
		// copy z value from slide move
		player->m_vecVelocity() = vecDownVel;
	}

	float flStepDist = player->get_abs_origin().z - vecPos.z;
	if (flStepDist > 0)
	{
		Source::m_pMoveHelper->SetHost(player);
		Source::m_pMoveHelper->m_outStepHeight += flStepDist;
		Source::m_pMoveHelper->SetHost(nullptr);
	}
}

void RebuildGameMovement::TracePlayerBBox(const Vector &start, const Vector &end, unsigned int fMask, int collisionGroup, trace_t& pm, C_BasePlayer *player)
{
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = reinterpret_cast<void*>(player);

	ray.Init(start, end, player->GetCollideable()->OBBMins(), player->GetCollideable()->OBBMaxs());
	Source::m_pEngineTrace->TraceRay(ray, fMask, &filter, &pm);
}

void RebuildGameMovement::WalkMove(C_BasePlayer *player)
{
	Math::AngleVectors(player->m_angEyeAngles(), &forward[player->entindex()], &right[player->entindex()], &up[player->entindex()]);  // Determine movement angles
																																   // Copy movement amounts
	Source::m_pMoveHelper->SetHost(player);
	fmove[player->entindex()] = Source::m_pMoveHelper->m_flForwardMove;
	smove[player->entindex()] = Source::m_pMoveHelper->m_flSideMove;
	Source::m_pMoveHelper->SetHost(nullptr);


	if (forward[player->entindex()][2] != 0)
	{
		forward[player->entindex()][2] = 0;
		(forward[player->entindex()]).Normalize();
	}

	if (right[player->entindex()][2] != 0)
	{
		right[player->entindex()][2] = 0;
		(right[player->entindex()]).Normalize();
	}


	for (i[player->entindex()] = 0; i[player->entindex()]<2; i[player->entindex()]++)       // Determine x and y parts of velocity
		wishvel[player->entindex()][i[player->entindex()]] = forward[player->entindex()][i[player->entindex()]] * fmove[player->entindex()] + right[player->entindex()][i[player->entindex()]] * smove[player->entindex()];

	wishvel[player->entindex()][2] = 0;             // Zero out z part of velocity

	wishdir[player->entindex()] = wishvel[player->entindex()]; // Determine maginitude of speed of move
	wishspeed[player->entindex()] = wishdir[player->entindex()].Normalize();

	//
	// Clamp to server defined max speed
	//
	Source::m_pMoveHelper->SetHost(player);
	if ((wishspeed[player->entindex()] != 0.0f) && (wishspeed[player->entindex()] > Source::m_pMoveHelper->m_flMaxSpeed))
	{
		VectorMultiply(wishvel[player->entindex()], player->m_flMaxspeed() / wishspeed[player->entindex()], wishvel[player->entindex()]);
		wishspeed[player->entindex()] = player->m_flMaxspeed();
	}
	Source::m_pMoveHelper->SetHost(nullptr);
	// Set pmove velocity
	player->m_vecVelocity()[2] = 0;
	Accelerate(player, wishdir[player->entindex()], wishspeed[player->entindex()], Source::m_pCvar->FindVar("sv_accelerate")->GetFloat());
	player->m_vecVelocity()[2] = 0;

	// Add in any base velocity to the current velocity.
	VectorAdd(player->m_vecVelocity(), player->m_vecBaseVelocity(), player->m_vecVelocity());

	spd[player->entindex()] = player->m_vecVelocity().Length();

	if (spd[player->entindex()] < 1.0f)
	{
		player->m_vecVelocity().clear();
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract(player->m_vecVelocity(), player->m_vecBaseVelocity(), player->m_vecVelocity());
		return;
	}

	// first try just moving to the destination	
	dest[player->entindex()][0] = player->get_abs_origin()[0] + player->m_vecVelocity()[0] * Source::m_pGlobalVars->frametime;
	dest[player->entindex()][1] = player->get_abs_origin()[1] + player->m_vecVelocity()[1] * Source::m_pGlobalVars->frametime;
	dest[player->entindex()][2] = player->get_abs_origin()[2];

	// first try moving directly to the next spot
	TracePlayerBBox(player->get_abs_origin(), dest[player->entindex()], MASK_PLAYERSOLID, 8, pm[player->entindex()], player);

	// If we made it all the way, then copy trace end as new player position.
	Source::m_pMoveHelper->SetHost(player);
	Source::m_pMoveHelper->m_outWishVel += wishdir[player->entindex()] * wishspeed[player->entindex()];
	Source::m_pMoveHelper->SetHost(nullptr);

	if (pm[player->entindex()].fraction == 1)
	{
		player->set_abs_origin(pm[player->entindex()].endpos);
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract(player->m_vecVelocity(), player->m_vecBaseVelocity(), player->m_vecVelocity());

		return;
	}

	// Don't walk up stairs if not on ground.
	if (!(player->m_fFlags() & FL_ONGROUND))
	{
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract(player->m_vecVelocity(), player->m_vecBaseVelocity(), player->m_vecVelocity());
		return;
	}

	StepMove(player, dest[player->entindex()], pm[player->entindex()]);

	// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
	VectorSubtract(player->m_vecVelocity(), player->m_vecBaseVelocity(), player->m_vecVelocity());
}

void RebuildGameMovement::FinishGravity(C_BasePlayer *player)
{
	float ent_gravity;

	ent_gravity = Source::m_pCvar->FindVar("sv_gravity")->GetFloat();

	// Get the correct velocity for the end of the dt 
	player->m_vecVelocity()[2] -= (ent_gravity * Source::m_pCvar->FindVar("sv_gravity")->GetFloat() * Source::m_pGlobalVars->frametime * 0.5);

	CheckVelocity(player);
}

void RebuildGameMovement::FullWalkMove(C_BasePlayer *player)
{
	StartGravity(player);

	// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor, 
	//  we don't slow when standing still, relative to the conveyor.
	if (player->m_fFlags() & FL_ONGROUND)
	{
		player->m_vecVelocity()[2] = 0.0;
		Friction(player);
	}

	// Make sure velocity is valid.
	CheckVelocity(player);

	if (player->m_fFlags() & FL_ONGROUND)
	{
		WalkMove(player);
	}
	else
	{
		AirMove(player);  // Take into account movement when in air.
	}

	// Make sure velocity is valid.
	CheckVelocity(player);

	// Add any remaining gravitational component.
	FinishGravity(player);

	// If we are on ground, no downward velocity.
	if (player->m_fFlags() & FL_ONGROUND)
	{
		player->m_vecVelocity()[2] = 0;
	}

	CheckFalling(player);
}

void RebuildGameMovement::Friction(C_BasePlayer *player)
{
	// Calculate speed
	speed[player->entindex()] = player->m_vecVelocity().Length();

	// If too slow, return
	if (speed[player->entindex()] < 0.1f)
	{
		return;
	}

	drop[player->entindex()] = 0;

	// apply ground friction
	if (player->m_fFlags() & FL_ONGROUND)  // On an C_BasePlayer that is the ground
	{
		friction[player->entindex()] = Source::m_pCvar->FindVar("sv_friction")->GetFloat() * player->m_surfaceFriction();

		//  Bleed off some speed, but if we have less than the bleed
		//  threshold, bleed the threshold amount.


		control[player->entindex()] = (speed[player->entindex()] < Source::m_pCvar->FindVar("sv_stopspeed")->GetFloat()) ? Source::m_pCvar->FindVar("sv_stopspeed")->GetFloat() : speed[player->entindex()];

		// Add the amount to the drop amount.
		drop[player->entindex()] += control[player->entindex()] * friction[player->entindex()] * Source::m_pGlobalVars->frametime;
	}

	// scale the velocity
	newspeed[player->entindex()] = speed[player->entindex()] - drop[player->entindex()];
	if (newspeed[player->entindex()] < 0)
		newspeed[player->entindex()] = 0;

	if (newspeed[player->entindex()] != speed[player->entindex()])
	{
		// Determine proportion of old speed we are using.
		newspeed[player->entindex()] /= speed[player->entindex()];
		// Adjust velocity according to proportion.
		VectorMultiply(player->m_vecVelocity(), newspeed[player->entindex()], player->m_vecVelocity());
	}

	player->m_vecVelocity() -= player->m_vecVelocity() * (1.f - newspeed[player->entindex()]);
}


void RebuildGameMovement::CheckFalling(C_BasePlayer *player)
{
	// this function really deals with landing, not falling, so early out otherwise
	if (player->m_flFallVelocity() <= 0)
		return;

	if (!player->m_iHealth() && player->m_flFallVelocity() >= 303.0f)
	{
		bool bAlive = true;
		float fvol = 0.5;

		//
		// They hit the ground.
		//
		if (player->m_vecVelocity().z < 0.0f)
		{
			// Player landed on a descending object. Subtract the velocity of the ground C_BasePlayer.
			player->m_flFallVelocity() += player->m_vecVelocity().z;
			player->m_flFallVelocity() = max(0.1f, player->m_flFallVelocity());
		}

		if (player->m_flFallVelocity() > 526.5f)
		{
			fvol = 1.0;
		}
		else if (player->m_flFallVelocity() > 526.5f / 2)
		{
			fvol = 0.85;
		}
		else if (player->m_flFallVelocity() < 173)
		{
			fvol = 0;
		}

	}

	// let any subclasses know that the player has landed and how hard

	//
	// Clear the fall velocity so the impact doesn't happen again.
	//
	player->m_flFallVelocity() = 0;
}
const int nanmask = 255 << 23;
#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)
void RebuildGameMovement::CheckVelocity(C_BasePlayer *player)
{
	Vector org = player->get_abs_origin();

	for (i[player->entindex()] = 0; i[player->entindex()] < 3; i[player->entindex()]++)
	{
		// See if it's bogus.
		if (IS_NAN(player->m_vecVelocity()[i[player->entindex()]]))
		{
			player->m_vecVelocity()[i[player->entindex()]] = 0;
		}

		if (IS_NAN(org[i[player->entindex()]]))
		{
			org[i[player->entindex()]] = 0;
			player->set_abs_origin(org);
			player->m_vecOrigin() = org;
		}

		// Bound it.
		if (player->m_vecVelocity()[i[player->entindex()]] > Source::m_pCvar->FindVar("sv_maxvelocity")->GetFloat())
		{
			player->m_vecVelocity()[i[player->entindex()]] = Source::m_pCvar->FindVar("sv_maxvelocity")->GetFloat();
		}
		else if (player->m_vecVelocity()[i[player->entindex()]] < -Source::m_pCvar->FindVar("sv_maxvelocity")->GetFloat())
		{
			player->m_vecVelocity()[i[player->entindex()]] = -Source::m_pCvar->FindVar("sv_maxvelocity")->GetFloat();
		}
	}
}
void RebuildGameMovement::StartGravity(C_BasePlayer *player)
{
	if (!player || !player->m_iHealth())
		return;

	Vector pVel = player->m_vecVelocity();

	pVel[2] -= (Source::m_pCvar->FindVar("sv_gravity")->GetFloat() * 0.5f * Source::m_pGlobalVars->interval_per_tick);
	pVel[2] += (player->m_vecBaseVelocity()[2] * Source::m_pGlobalVars->interval_per_tick);

	player->m_vecVelocity() = pVel;

	Vector tmp = player->m_vecBaseVelocity();
	tmp[2] = 0.f;
	player->m_vecVelocity() = tmp;
}