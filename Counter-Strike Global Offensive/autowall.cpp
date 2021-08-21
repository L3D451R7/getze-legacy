#include "sdk.hpp"
#include "autowall.hpp"
#include "aimbot.hpp"
#include "source.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "hooked.hpp"
#include "math.hpp"
#include "displacement.hpp"

#include <thread>

void c_autowall::TraceLine(Vector& absStart, Vector& absEnd, unsigned int mask, C_BasePlayer* ignore, CGameTrace* ptr)
{
	Ray_t ray;
	ray.Init(absStart, absEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	Source::m_pEngineTrace->TraceRay(ray, mask, &filter, ptr);
}

void c_autowall::ScaleDamage(CGameTrace& enterTrace, weapon_info* weaponData, float& currentDamage)
{
	if (!enterTrace.m_pEnt || !enterTrace.m_pEnt->GetClientClass() || enterTrace.m_pEnt->GetClientClass()->m_ClassID != class_ids::CCSPlayer)
		return;

	auto target = ((C_BasePlayer*)enterTrace.m_pEnt);

	int hitgroup = enterTrace.hitgroup;
	auto is_zeus = cheat::main::local()->get_weapon()->m_iItemDefinitionIndex() == weapon_taser;

	const auto is_armored = [&]() -> bool
	{
		if (target->m_ArmorValue() > 0.f)
		{
			switch (hitgroup)
			{
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				return true;
			case HITGROUP_HEAD:
				return target->m_bHasHelmet();
			default:
				break;
			}
		}

		return false;
	};

	if (!is_zeus) {
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			/*if (target->m_bHasHeavyArmor())
				currentDamage = (currentDamage * 4.f) * .5f;
			else*/
				currentDamage *= 4.f;
			break;
		case HITGROUP_STOMACH:
			currentDamage *= 1.25f;
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			currentDamage *= .75f;
			break;
		default:
			break;
		}
	}
	else
		currentDamage *= 0.92f;

	if (is_armored())
	{
		auto modifier = 1.f, armor_bonus_ratio = .5f, armor_ratio = weaponData->armor_ratio * .5f;

		/*if (target->m_bHasHeavyArmor())
		{
			armor_bonus_ratio = 0.33f;
			armor_ratio = (weaponData->armor_ratio * 0.5f) * 0.5f;
			modifier = 0.33f;
		}*/

		auto new_damage = currentDamage * armor_ratio;

		//if (target->m_bHasHeavyArmor())
		//	new_damage *= 0.85f;

		if ((currentDamage - currentDamage * armor_ratio) * (modifier * armor_bonus_ratio) > target->m_ArmorValue())
			new_damage = currentDamage - target->m_ArmorValue() / armor_bonus_ratio;

		currentDamage = new_damage;
	}
}

bool c_autowall::TraceToExit(trace_t& enter_trace, trace_t& exit_trace, const Vector start_position,
	const Vector direction, const bool is_local)
{
	const auto max_distance = is_local ? 200.f : 90.f;
	const auto ray_extension = is_local ? 8.f : 4.f;

	float current_distance = 0;
	auto first_contents = 0;

	while (current_distance <= max_distance)
	{
		current_distance += ray_extension;

		auto start = start_position + direction * current_distance;

		if (!first_contents)
			first_contents = Source::m_pEngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX);

		const auto point_contents = Source::m_pEngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX);

		if (!(point_contents & MASK_SHOT_HULL) || (point_contents & CONTENTS_HITBOX && point_contents != first_contents))
		{
			const auto end = start - direction * ray_extension;

			Ray_t r{};
			r.Init(start, end);
			CTraceFilter filter;
			filter.pSkip = nullptr;
			Source::m_pEngineTrace->TraceRay(r, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &exit_trace);

			if (exit_trace.startsolid && exit_trace.surface.flags & SURF_HITBOX)
			{
				r.Init(start, start_position);
				filter.pSkip = exit_trace.m_pEnt;
				Source::m_pEngineTrace->TraceRay(r, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &exit_trace);

				if (exit_trace.DidHit() && !exit_trace.startsolid)
				{
					start = exit_trace.endpos;
					return true;
				}

				continue;
			}

			if (exit_trace.DidHit() && !exit_trace.startsolid)
			{
				if (enter_trace.m_pEnt->is_breakable() && exit_trace.m_pEnt != nullptr && exit_trace.m_pEnt->is_breakable())
					return true;

				if (enter_trace.surface.flags & SURF_NODRAW
					|| (!(exit_trace.surface.flags & SURF_NODRAW)
						&& exit_trace.plane.normal.Dot(direction) <= 1.f))
				{
					const auto mult_amount = exit_trace.fraction * 4.f;
					start -= direction * mult_amount;
					return true;
				}

				continue;
			}

			if (!exit_trace.DidHit() || exit_trace.startsolid)
			{
				if (enter_trace.DidHitNonWorldEntity() && exit_trace.m_pEnt != nullptr && enter_trace.m_pEnt->is_breakable())
				{
					exit_trace = enter_trace;
					exit_trace.endpos = start;
					return true;
				}
			}
		}
	}

	return false;
}

bool c_autowall::HandleBulletPenetration(weapon_info* weaponData, trace_t& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_bullet_penetration, bool pskip)
{
	//Because there's been issues regarding this- putting this here.
	if (&currentDamage == nullptr)
		return 0;

	//SafeLocalPlayer() false;
	CGameTrace exitTrace;
	C_BasePlayer* pEnemy = (C_BasePlayer*)enterTrace.m_pEnt;
	auto enterSurfaceData = Source::m_pPhysProps->GetSurfaceData(enterTrace.surface.surfaceProps);
	int enter_material = enterSurfaceData->game.material;

	if (possibleHitsRemaining <= 0 || !enterSurfaceData || enterSurfaceData->game.penetrationmodifier < 0.1f) {
		//possibleHitsRemaining = 0;
		return false;
	}

	auto enter_penetration_modifier = enterSurfaceData->game.penetrationmodifier;
	float enterDamageModifier = enterSurfaceData->game.damagemodifier;// , modifier, finalDamageModifier, combinedPenetrationModifier;
	bool isSolidSurf = ((enterTrace.contents >> 3) & CONTENTS_SOLID);
	bool isLightSurf = ((enterTrace.surface.flags >> 7) & SURF_LIGHT);

	//Test for "DE_CACHE/DE_CACHE_TELA_03" as the entering surface and "CS_ITALY/CR_MISCWOOD2B" as the exiting surface.
	//Fixes a wall in de_cache which seems to be broken in some way. Although bullet penetration is not recorded to go through this wall
	//Decals can be seen of bullets within the glass behind of the enemy. Hacky method, but works.
	if (enterTrace.surface.name == (const char*)0x2227c261 && exitTrace.surface.name == (const char*)0x2227c868)
		return false;

	if ((!possibleHitsRemaining && !isLightSurf && !isSolidSurf && enter_material != CHAR_TEX_GRATE && enter_material != CHAR_TEX_GLASS)
		|| weaponData->penetration <= 0.f)
		return false;

	if (!TraceToExit(enterTrace, exitTrace, enterTrace.endpos, direction, weaponData->damage > 10000.f)) {
		if (!(Source::m_pEngineTrace->GetPointContents(enterTrace.endpos, 0x600400B, nullptr) & 0x600400B))
			return false;
	}

	auto exitSurfaceData = Source::m_pPhysProps->GetSurfaceData(exitTrace.surface.surfaceProps);
	int exitMaterial = exitSurfaceData->game.material;
	float exitSurfPenetrationModifier = exitSurfaceData->game.penetrationmodifier;
	float exitDamageModifier = exitSurfaceData->game.damagemodifier;

	auto combined_damage_modifier = 0.16f;
	auto combined_penetration_modifier = 0.f;

	//Are we using the newer penetration system?
	if (enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE) {
		combined_damage_modifier = 0.05f;
		combined_penetration_modifier = 3.f;
	}
	else if (isSolidSurf || isLightSurf) {
		combined_penetration_modifier = 1.f;
		combined_damage_modifier = 0.16f;
	}
	else if (enter_material == CHAR_TEX_FLESH && (pEnemy != nullptr && pEnemy->m_iTeamNum() == cheat::main::local()->m_iTeamNum())) { // TODO: Team check config
		combined_penetration_modifier = ff_damage_bullet_penetration;
		combined_damage_modifier = 0.16f;
	}
	else {
		combined_damage_modifier = 0.16f;
		combined_penetration_modifier = ((enter_penetration_modifier + exitSurfPenetrationModifier) * 0.5f);
	}

	if (enter_material == exitMaterial) {
		if (exitMaterial == CHAR_TEX_WOOD || exitMaterial == CHAR_TEX_CARDBOARD)
			combined_penetration_modifier = 3.f;
		else if (exitMaterial == CHAR_TEX_PLASTIC)
			combined_penetration_modifier = 2.f;
	}

	/*auto v22 = fmaxf(1.0f / combined_penetration_modifier, 0.0f);
	auto v23 = fmaxf(3.0f / penetrationPower, 0.0f);

	auto penetration_modifier = fmaxf(0.f, 1.f / combined_penetration_modifier);
	auto penetration_distance = (exitTrace.endpos - enterTrace.endpos).Length();

	auto damage_lost = ((currentDamage * combined_damage_modifier) + ((v23 * v22) * 3.0f)) + (((penetration_distance * penetration_distance) * v22) * 0.041666668f);

	auto new_damage = currentDamage - damage_lost;

	currentDamage = new_damage;

	if (new_damage > 0.0f)
	{
		*eyePosition = exitTrace.endpos;
		--possibleHitsRemaining;
		return true;
	}*/

	auto thickness = (exitTrace.endpos - enterTrace.endpos).Length();
	thickness *= thickness;
	thickness *= fmaxf(0.f, 1.0f / combined_penetration_modifier);
	thickness /= 24.0f;

	const auto lost_damage = fmaxf(0.0f, currentDamage * combined_damage_modifier + fmaxf(0.f, 1.0f / combined_penetration_modifier)
		* 3.0f * fmaxf(0.0f, 3.0f / penetrationPower) * 1.25f + thickness);

	if (lost_damage > currentDamage)
		return false;

	if (lost_damage > 0.f)
		currentDamage -= lost_damage;

	if (currentDamage < 1.f)
		return false;

	eyePosition = exitTrace.endpos;
	--possibleHitsRemaining;

	return true;
}

void c_autowall::FixTraceRay(Vector end, Vector start, trace_t* oldtrace, C_BasePlayer* ent) {
	if (!ent)
		return;

	const auto mins = ent->OBBMins();
	const auto maxs = ent->OBBMaxs();

	auto dir(end - start);
	dir.Normalized();

	const auto center = (maxs + mins) / 2;
	const auto pos(center + ent->m_vecOrigin());

	auto to = pos - start;
	const float range_along = dir.Dot(to);

	float range;
	if (range_along < 0.f) {
		range = -to.Length();
	}
	else if (range_along > dir.Length()) {
		range = -(pos - end).Length();
	}
	else {
		auto ray(pos - (dir * range_along + start));
		range = ray.Length();
	}

	if (range <= 60.f) {

		Ray_t ray;
		ray.Init(start, end);

		trace_t trace;
		Source::m_pEngineTrace->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ent, &trace);

		if (oldtrace->fraction > trace.fraction)
			* oldtrace = trace;
	}
}

void c_autowall::ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, uint32_t mask, ITraceFilter* filter, trace_t* tr) {
	float smallestFraction = tr->fraction;
	constexpr float maxRange = 60.0f;

	Vector delta(vecAbsEnd - vecAbsStart);
	const float delta_length = delta.Normalize();

	Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd);

	for (int i = 1; i <= 64; ++i) {
		auto ent = Source::m_pEntList->GetClientEntity(i);
		if (!ent || ent->IsDormant() || ent->IsDead())
			continue;

		if (filter && !filter->ShouldHitEntity(ent, mask))
			continue;

		matrix3x4_t coordinate_frame;
		Math::AngleMatrix(ent->get_abs_eye_angles(), ent->m_vecOrigin(), coordinate_frame);

		auto collideble = ent->GetCollideable();
		auto mins = collideble->OBBMins();
		auto maxs = collideble->OBBMaxs();

		auto obb_center = (maxs + mins) * 0.5f;
		Math::VectorTransform(obb_center, coordinate_frame, obb_center);

		auto extend = (obb_center - vecAbsStart);
		auto rangeAlong = delta.Dot(extend);

		float range;
		if (rangeAlong >= 0.0f) {
			if (rangeAlong <= delta_length)
				range = Vector(obb_center - ((delta * rangeAlong) + vecAbsStart)).Length();
			else
				range = -(obb_center - vecAbsEnd).Length();
		}
		else {
			range = -extend.Length();
		}

		if (range > 0.0f && range <= maxRange) {
			trace_t playerTrace;
			Source::m_pEngineTrace->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ent, &playerTrace);
			if (playerTrace.fraction < smallestFraction) {
				// we shortened the ray - save off the trace
				*tr = playerTrace;
				smallestFraction = playerTrace.fraction;
			}
		}
	}
}

//int HitboxToHitgroup(int Hitbox)
//{
//	switch (Hitbox)
//	{
//	case HITBOX_HEAD:
//	case HITBOX_NECK:
//		return HITGROUP_HEAD;
//	case HITBOX_UPPER_CHEST:
//	case HITBOX_CHEST:
//	case HITBOX_THORAX:
//	case HITBOX_LEFT_UPPER_ARM:
//	case HITBOX_RIGHT_UPPER_ARM:
//		return HITGROUP_CHEST;
//	case HITBOX_PELVIS:
//	case HITBOX_LEFT_THIGH:
//	case HITBOX_RIGHT_THIGH:
//	case HITBOX_BODY:
//		return HITGROUP_STOMACH;
//	case HITBOX_LEFT_CALF:
//	case HITBOX_LEFT_FOOT:
//		return HITGROUP_LEFTLEG;
//	case HITBOX_RIGHT_CALF:
//	case HITBOX_RIGHT_FOOT:
//		return HITGROUP_RIGHTLEG;
//	case HITBOX_LEFT_FOREARM:
//	case HITBOX_LEFT_HAND:
//		return HITGROUP_LEFTARM;
//	case HITBOX_RIGHT_FOREARM:
//	case HITBOX_RIGHT_HAND:
//		return HITGROUP_RIGHTARM;
//	default:
//		return HITGROUP_STOMACH;
//	}
//}

bool c_autowall::FireBullet(Vector eyepos, C_WeaponCSBaseGun* pWeapon, Vector& direction, float& currentDamage, C_BasePlayer* ignore, C_BasePlayer* to_who, int target_hitbox)
{
	if (!pWeapon || !ignore)
		return false;

	//SafeLocalPlayer() false;
	//bool sv_penetration_type;
	//	  Current bullet travel Power to penetrate Distance to penetrate Range               Player bullet reduction convars			  Amount to extend ray by
	float currentDistance = 0.f, penetrationPower, penetrationDistance, maxRange, ff_damage_bullet_penetration;

	static ConVar* damageBulletPenetration = Source::m_pCvar->FindVar("ff_damage_bullet_penetration");

	ff_damage_bullet_penetration = damageBulletPenetration->GetFloat();

	weapon_info* weaponData = pWeapon->GetCSWeaponData();
	CGameTrace enterTrace;

	//We should be skipping localplayer when casting a ray to players.
	CTraceFilter filter;
	filter.pSkip = ignore;

	if (!weaponData)
		return false;

	maxRange = weaponData->range;
	penetrationDistance = weaponData->range;
	penetrationPower = weaponData->penetration;

	//This gets set in FX_Firebullets to 4 as a pass-through value.
	//CS:GO has a maximum of 4 surfaces a bullet can pass-through before it 100% stops.
	//Excerpt from Valve: https://steamcommunity.com/sharedfiles/filedetails/?id=275573090
	//"The total number of surfaces any bullet can penetrate in a single flight is capped at 4." -CS:GO Official
	cheat::main::last_penetrated_count = 4;

	//Set our current damage to what our gun's initial damage reports it will do
	currentDamage = weaponData->damage;

	//If our damage is greater than (or equal to) 1, and we can shoot, let's shoot.
	while (cheat::main::last_penetrated_count > 0 && currentDamage >= 1.0f)
	{
		//Calculate max bullet range
		maxRange -= currentDistance;

		//Create endpoint of bullet
		Vector end = eyepos + direction * maxRange;

		TraceLine(eyepos, end, MASK_SHOT_HULL | CONTENTS_HITBOX, ignore, &enterTrace);

		if (to_who/* && target_hitbox == HITBOX_HEAD*/) {
			//Pycache/aimware traceray fix for head while players are jumping
			FixTraceRay(eyepos + direction * 40.f, eyepos, &enterTrace, to_who);
		}
		else
			ClipTraceToPlayers(eyepos, end + direction * 40.f, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace);

		if (enterTrace.fraction == 1.0f)
			break;

		//We have to do this *after* tracing to the player.
		auto enterSurfaceData = Source::m_pPhysProps->GetSurfaceData(enterTrace.surface.surfaceProps);
		float enterSurfPenetrationModifier = enterSurfaceData->game.penetrationmodifier;
		int enterMaterial = enterSurfaceData->game.material;

		//calculate the damage based on the distance the bullet traveled.
		currentDistance += enterTrace.fraction * maxRange;

		//Let's make our damage drops off the further away the bullet is.
		currentDamage *= powf(weaponData->range_modifier, (currentDistance / 500.f));

		//Sanity checking / Can we actually shoot through?
		if (currentDistance > maxRange || currentDistance > 3000.0 && weaponData->penetration > 0.f || enterSurfPenetrationModifier < 0.1f)
			cheat::main::last_penetrated_count = 0;

		//This looks gay as fuck if we put it into 1 long line of code.
		bool canDoDamage = (enterTrace.hitgroup > 0 && enterTrace.hitgroup <= 8);
		bool isPlayer = (enterTrace.m_pEnt->GetClientClass() && enterTrace.m_pEnt->GetClientClass()->m_ClassID == class_ids::CCSPlayer);
		bool isEnemy = (ignore->m_iTeamNum() != ((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum());
		bool onTeam = (((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum() == 2 || ((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum() == 3);

		//TODO: Team check config
		if ((canDoDamage && isPlayer && isEnemy) && onTeam || (to_who == enterTrace.m_pEnt))
		{
			ScaleDamage(enterTrace, weaponData, currentDamage);
			return true;
		}

		//Calling HandleBulletPenetration here reduces our penetrationCounter, and if it returns true, we can't shoot through it.
		if (!HandleBulletPenetration(weaponData, enterTrace, eyepos, direction, cheat::main::last_penetrated_count, currentDamage, penetrationPower, ff_damage_bullet_penetration))
			break;
	}

	return false;
}

////////////////////////////////////// Usage Calls //////////////////////////////////////
float c_autowall::CanHit(Vector& vecEyePos, Vector& point)
{
	Vector angles, direction;
	Vector tmp = point - cheat::main::local()->GetEyePosition();
	float currentDamage = 0;

	Math::VectorAngles(tmp, angles);
	Math::AngleVectors(angles, &direction);
	direction.Normalize();

	auto local_weapon = cheat::main::local()->get_weapon();

	if (local_weapon != nullptr && FireBullet(vecEyePos, local_weapon, direction, currentDamage, cheat::main::local()))
		return currentDamage;

	return -1; //That wall is just a bit too thick buddy
}

float c_autowall::CanHit(Vector& vecEyePos, Vector& point, C_BasePlayer* ignore_ent, C_BasePlayer* to_who, int target_hitbox)
{
	if (ignore_ent == nullptr || to_who == nullptr)
		return 0;

	Vector angles, direction;
	Vector tmp = point - vecEyePos;
	float currentDamage = 0;

	Math::VectorAngles(tmp, angles);
	Math::AngleVectors(angles, &direction);
	direction.Normalize();

	auto local_weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(ignore_ent->m_hActiveWeapon()));

	if (local_weapon != nullptr)
	{
		if (FireBullet(vecEyePos, local_weapon, direction, currentDamage, ignore_ent, to_who, target_hitbox))
			return currentDamage;
		else
			return -1;
	}

	return -1; //That wall is just a bit too thick buddy
}