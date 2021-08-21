#include "aimbot.hpp"
#include "source.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "hooked.hpp"
#include "autowall.hpp"
#include "math.hpp"
#include "displacement.hpp"
#include "lag_compensation.hpp"
#include "anti_aimbot.hpp"
#include "angle_resolver.hpp"
#include "visuals.hpp"
#include "rmenu.hpp"
#include "movement.hpp"

struct TargetListing_t
{
	int index = 0;
	float distance = 0.f;
	float screen_distance = 0.f;
	float hp = 0;

	TargetListing_t(C_BasePlayer* entity)
	{
		index = entity->entindex();
		distance = entity->m_vecOrigin().Distance(cheat::main::local()->m_vecOrigin());
		if (auto dummy = Vector::Zero; Drawing::WorldToScreen(entity->m_vecOrigin(), dummy) && !dummy.IsZero()) screen_distance = dummy.Distance(Vector(cheat::game::screen_size.x, cheat::game::screen_size.y, 0));

		hp = entity->m_iHealth();
	}
};

//std::vector<TargetListing_t> m_entities;

Vector c_aimbot::get_hitbox(C_BasePlayer* ent, int ihitbox)
{
	if (ihitbox < 0 || ihitbox > 19) return Vector::Zero;

	if (!ent) return Vector::Zero;

	const auto model = ent->GetModel();

	if (!model)
		return Vector::Zero;

	auto pStudioHdr = Source::m_pModelInfo->GetStudioModel(model);

	if (!pStudioHdr)
		return Vector::Zero;

	auto hitbox = pStudioHdr->pHitbox(ihitbox, 0);

	if (!hitbox)
		return Vector::Zero;

	Vector min, max;
	Math::VectorTransform(hitbox->bbmin, ent->m_CachedBoneData().Base()[hitbox->bone], min);
	Math::VectorTransform(hitbox->bbmax, ent->m_CachedBoneData().Base()[hitbox->bone], max);

	auto center = (min + max) / 2.f;

	return center;
}

void c_aimbot::draw_capsule(C_BasePlayer* ent, int ihitbox)
{
	if (ihitbox < 0 || ihitbox > 19) return;

	if (!ent) return;

	const auto model = ent->GetModel();

	if (!model)
		return;

	auto pStudioHdr = Source::m_pModelInfo->GetStudioModel(model);

	if (!pStudioHdr)
		return;

	auto hitbox = pStudioHdr->pHitbox(ihitbox, 0);

	if (!hitbox)
		return;

	//Vector min, max;
	//Math::VectorTransform(hitbox->bbmin, mat[hitbox->bone], min);
	//Math::VectorTransform(hitbox->bbmax, mat[hitbox->bone], max);

	//cheat::features::visuals.draw_capsule(hitbox->bbmin, hitbox->bbmax, hitbox->radius, mat[hitbox->bone], Color::Red(200));
}

Vector c_aimbot::get_hitbox(C_BasePlayer* ent, int ihitbox, matrix3x4_t mat[])
{
	if (ihitbox < 0 || ihitbox > 19) return Vector::Zero;

	if (!ent) return Vector::Zero;

	/*matrix3x4_t mat[128];

	ent->force_bone_rebuild();

	const auto _backup = ent->get_animation_state()->m_bOnGround;
	ent->get_animation_state()->m_bOnGround = 0;
	if (!ent->SetupBones(mat, 128, 0x00000100, ent->m_flSimulationTime())) {
	ent->get_animation_state()->m_bOnGround = _backup;
	return Vector::Zero;
	}
	ent->get_animation_state()->m_bOnGround = _backup;*/

	if (!ent->GetClientRenderable())
		return Vector::Zero;

	const auto model = ent->GetModel();

	if (!model)
		return Vector::Zero;

	auto pStudioHdr = Source::m_pModelInfo->GetStudioModel(model);

	if (!pStudioHdr)
		return Vector::Zero;

	auto hitbox = pStudioHdr->pHitbox(ihitbox, ent->m_nHitboxSet());

	if (!hitbox)
		return Vector::Zero;

	if (hitbox->bone > 128 || hitbox->bone < 0)
		return Vector::Zero;

	Vector min, max;
	Math::VectorTransform(hitbox->bbmin, mat[hitbox->bone], min);
	Math::VectorTransform(hitbox->bbmax, mat[hitbox->bone], max);

	auto center = (min + max) / 2.f;

	return center;
}

//void  c_aimbot::ClipTraceToPlayers(Vector& absStart, Vector absEnd, unsigned int mask, ITraceFilter* filter, CGameTrace* tr)
//{
//	//TODO: Un-ASM this
//#ifdef _WIN32
//	_asm
//	{
//		mov eax, filter
//		lea ecx, tr
//		push ecx
//		push eax
//		push mask
//		lea edx, absEnd
//		lea ecx, absStart
//		call Engine::Displacement::Data::m_uClipTracePlayers
//		add esp, 0xC
//	}
//#else
//	UTIL_ClipTraceToPlayers(absStart, absEnd, mask, filter, tr, 60.f, 0.f);
//#endif
//}

bool c_aimbot::knifebot_work(CUserCmd* cmd, bool&send_packet)
{
	auto return_value = false;

	m_nBestIndex = -1;

	auto local_weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon()));

	if (!local_weapon || !local_weapon->is_knife())
		return return_value;

	if (!knifebot_target() || pBestEntity == nullptr) {
		cheat::features::lagcomp.finish_position_adjustment();
		return return_value;
	}

	Vector tempBestAngle = get_hitbox(pBestEntity, HITBOX_PELVIS);
	auto entAng = Math::CalcAngle(cheat::main::local()->GetEyePosition(), tempBestAngle);

	if (!lastAttacked)
	{
		bool stab = false;
		int health = pBestEntity->m_iHealth();
		if (pBestEntity->m_ArmorValue())
		{
			if (health <= 55 &&
				health > 34)
				stab = true;
		}
		else
		{
			if (health <= 65 &&
				health > 40)
				stab = true;
		}

		stab = stab && m_nBestDist <= 32.0f;

		if (cheat::features::aaa.compare_delta(pBestEntity->m_angEyeAngles().y, Math::CalcAngle(cheat::main::local()->m_vecOrigin(), pBestEntity->m_vecOrigin()).y, 75) + 180.f) {
			if (cheat::features::lagcomp.records[pBestEntity->entindex() - 1].backtrack_ticks > 0) {
				if (m_nBestDist < 32.0f)
					stab = true;
				else
				{
					cheat::features::lagcomp.finish_position_adjustment();
					return return_value;
				}
			}
			else
			{
				if (m_nBestDist < 32.0f)
					stab = true;
				else
					stab = false;
			}
		}

		if (m_nBestDist < 32.0f && health >= 90)
			stab = true;

		if (stab)
			cmd->buttons |= IN_ATTACK2;
		else
			cmd->buttons |= IN_ATTACK;

		if (m_nBestDist > 48.0f)
		{
			cheat::features::lagcomp.finish_position_adjustment();
			return return_value;
		}

		float next_shot = (stab ? local_weapon->m_flNextPrimaryAttack() : local_weapon->m_flNextPrimaryAttack()) - Source::m_pGlobalVars->curtime;

		if (!(next_shot > 0))
		{
			cmd->viewangles = entAng;
			cmd->tick_count = cheat::features::lagcomp.records[pBestEntity->entindex() - 1].tick_count;
			send_packet = false;
			return_value = true;
		}
	}

	cheat::features::lagcomp.finish_position_adjustment();

	lastAttacked = !lastAttacked;

	return return_value;
}

bool c_aimbot::knifebot_target()
{
	static auto is_visible = [](C_BasePlayer* thisptr, Vector& Start, Vector& End) -> bool
	{
		if (!thisptr) return NULL;

		CGameTrace tr;
		Ray_t ray;
		static CTraceFilter traceFilter;
		traceFilter.pSkip = cheat::main::local();

		ray.Init(Start, End);

		Source::m_pEngineTrace->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &traceFilter, &tr);
		cheat::features::autowall.FixTraceRay(Start, End, &tr, thisptr);

		return (tr.m_pEnt == thisptr || tr.fraction > 0.99f);
	};

	cheat::features::lagcomp.start_position_adjustment();

	float bestDist = 75;
	for (int i = 0; i < 64; i++)
	{
		auto pBaseEntity = Source::m_pEntList->GetClientEntity(i);
		if (!pBaseEntity)
			continue;
		if (!pBaseEntity->GetClientClass())
			continue;
		if (pBaseEntity->GetClientClass()->m_ClassID != class_ids::CCSPlayer)
			continue;
		if (pBaseEntity == cheat::main::local())
			continue;
		if (pBaseEntity->m_bGunGameImmunity())
			continue;
		if (pBaseEntity->IsDead())
			continue;
		if (pBaseEntity->IsDormant())
			continue;
		if (pBaseEntity->m_iTeamNum() == cheat::main::local()->m_iTeamNum())
			continue;

		Vector localPos = cheat::main::local()->m_vecOrigin() + Vector(0, 0, 60);
		Vector basePos = pBaseEntity->m_vecOrigin() + Vector(0, 0, 60);

		if (!is_visible(pBaseEntity, localPos, basePos))
			continue;

		float curDist = localPos.Distance(basePos);
		if (curDist < bestDist)
		{
			bestDist = curDist;
			m_nBestIndex = i;
			pBestEntity = pBaseEntity;
		}
	}

	m_nBestDist = bestDist;
	return m_nBestIndex != -1;
}

//bool c_aimbot::hit_chance(C_WeaponCSBaseGun* weapon, float chance)
//{
//	if (cheat::Cvars.RageBot_HitChance.GetValue() <= 0)
//		return true;
//
//	if (chance >= 100.f)
//		chance = 100.f;
//
//	if (chance < 1.f)
//		chance = 1.f;
//
//	float flSpread = weapon->GetInaccuracy() * 5;
//	return ((((100.f - chance) * 0.65f) * 0.01125) >= flSpread);
//}

float c_aimbot::can_hit(int hitbox, C_BasePlayer* Entity, Vector position, matrix3x4_t mx[], bool check_center)
{
	static auto is_visible = [](C_BasePlayer* thisptr, Vector& Start, Vector& End) -> bool
	{
		if (!thisptr) return NULL;

		CGameTrace tr;
		Ray_t ray;
		static CTraceFilter traceFilter;
		traceFilter.pSkip = cheat::main::local();

		ray.Init(Start, End);

		Source::m_pEngineTrace->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &traceFilter, &tr);
		cheat::features::autowall.FixTraceRay(Start, End, &tr, thisptr);

		return (tr.m_pEnt == thisptr || tr.fraction > 0.99f);
	};

	auto eyepos = cheat::main::local()->GetEyePosition();

	auto local_weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon()));

	if (!local_weapon->GetCSWeaponData())
		return 0;

	/*static auto is_visible = [](C_BasePlayer* thisptr, Vector& Start, Vector& End) -> bool
	{
	if (!thisptr) return NULL;

	CGameTrace tr;
	Ray_t ray;
	static CTraceFilter traceFilter;
	traceFilter.pSkip = cheat::main::local();

	ray.Init(Start, End);

	Source::m_pEngineTrace->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &traceFilter, &tr);
	cheat::features::aimbot.ClipTraceToPlayers(Start, End, MASK_SHOT_HULL | CONTENTS_HITBOX, &traceFilter, &tr);

	return (tr.m_pEnt == thisptr || tr.fraction > 0.99f);
	};*/

	auto cdmg = cheat::features::autowall.CanHit(eyepos, position, cheat::main::local(), Entity, hitbox);

	const auto scaled = Entity->m_iHealth() < cheat::Cvars.RageBot_MinDamage.GetValue() && cheat::Cvars.RageBot_ScaledmgOnHp.GetValue();

	auto nob = (scaled ? Entity->m_iHealth() : cheat::Cvars.RageBot_MinDamage.GetValue());

	if (scaled)
		nob = min(100, nob + 1);

	if (cdmg >= nob)
		return cdmg;

	static std::vector<Vector> points;

	if (Entity->get_multipoints(hitbox, points, mx) && !points.empty())
	{
		cheat::main::points[Entity->entindex() - 1][hitbox] = points;

		if (hitbox != 11 && hitbox != 12) 
		{
			if (hitbox <= 1) {
				auto dmg = cheat::features::autowall.CanHit(eyepos, points.at(0), cheat::main::local(), Entity, hitbox);
				auto dmg2 = cheat::features::autowall.CanHit(eyepos, points.at(2), cheat::main::local(), Entity, hitbox);
				auto dmg3 = cheat::features::autowall.CanHit(eyepos, points.at(3), cheat::main::local(), Entity, hitbox);

				if (max(dmg, max(dmg2, dmg3)) >= nob)
					return dmg;
			}
			else
			{
				auto dmg = cheat::features::autowall.CanHit(eyepos, points.at(1), cheat::main::local(), Entity, hitbox);
				auto dmg2 = cheat::features::autowall.CanHit(eyepos, points.at(2), cheat::main::local(), Entity, hitbox);
				//auto dmg3 = feature::autowall.CanHit(eyepos, points.at(4), ctx.m_local(), entity, hitbox);

				if (max(dmg, /*max(*/dmg2/*, dmg3)*/) >= nob)
					return dmg;
			}
		}
		else
		{
			auto dmg1 = cheat::features::autowall.CanHit(eyepos, points.at(0), cheat::main::local(), Entity, hitbox);
			auto dmg2 = cheat::features::autowall.CanHit(eyepos, points.at(1), cheat::main::local(), Entity, hitbox);

			if (max(dmg1, dmg2) >= nob)
				return max(dmg1, dmg2);
		}
	}

	if (cdmg >= nob)
		return cdmg;

	return 0;
}

void c_aimbot::DrawHitchanceLines()
{
	auto aechseVektor = [](Vector aechsen, float *forward, float *right, float *up) -> void
	{
		float aechse;
		float sp, sy, cp, cy;

		aechse = aechsen[0] * (3.1415f / 180);
		sp = sin(aechse);
		cp = cos(aechse);

		aechse = aechsen[1] * (3.1415f / 180);
		sy = sin(aechse);
		cy = cos(aechse);

		if (forward)
		{
			forward[0] = cp * cy;
			forward[1] = cp * sy;
			forward[2] = -sp;
		}

		if (right || up)
		{
			float sr, cr;

			aechse = aechsen[2] * (3.1415f / 180);
			sr = sin(aechse);
			cr = cos(aechse);

			if (right)
			{
				right[0] = -1 * sr * sp * cy + -1 * cr * -sy;
				right[1] = -1 * sr * sp * sy + -1 * cr *cy;
				right[2] = -1 * sr * cp;
			}

			if (up)
			{
				up[0] = cr * sp *cy + -sr * -sy;
				up[1] = cr * sp *sy + -sr * cy;
				up[2] = cr * cp;
			}
		}
	};

	auto praviVektor = [](Vector src, Vector &dst) -> void
	{
		float sp, sy, cp, cy;

		Math::SinCos(DEG2RAD(src[1]), &sy, &cy);
		Math::SinCos(DEG2RAD(src[0]), &sp, &cp);

		dst.x = cp * cy;
		dst.y = cp * sy;
		dst.z = -sp;
	};

	auto vekAechse = [this](Vector &forward, Vector &up, Vector &aechse) -> void
	{
		auto CrossProduct = [this](const Vector& a, const Vector& b) -> Vector
		{
			return Vector(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
		};

		Vector left = CrossProduct(up, forward);
		left.NormalizeInPlace();

		float forwardDist = forward.Length2D();

		if (forwardDist > 0.001f)
		{
			aechse.x = atan2f(-forward.z, forwardDist) * 180 / M_PI_F;
			aechse.y = atan2f(forward.y, forward.x) * 180 / M_PI_F;

			float upZ = (left.y * forward.x) - (left.x * forward.y);
			aechse.z = atan2f(left.z, upZ) * 180 / M_PI_F;
		}
		else
		{
			aechse.x = atan2f(-forward.z, forwardDist) * 180 / M_PI_F;
			aechse.y = atan2f(-left.x, left.y) * 180 / M_PI_F;
			aechse.z = 0;
		}
	};

	auto weap = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon()));

	if (!weap)
		return;

	auto weap_data = weap->GetCSWeaponData();

	if (!weap_data)
		return;

	Vector neu = cheat::main::local()->GetEyePosition(), traceEnd;;
	QAngle viewAngles = Engine::Movement::Instance()->m_qRealAngles;

	//Math::AngleVectors(viewAngles, &traceEnd);
	//traceEnd = neu + (traceEnd * 8192.0f);

	Vector forward, right, up;
	Math::AngleVectors(viewAngles, &forward, &right, &up);//aechseVektor(aechse, (float*)&forward, (float*)&right, (float*)&up);

	weap->UpdateAccuracyPenalty();
	float weap_sir = weap->GetSpread();
	float weap_inac = weap->GetInaccuracy() + weap->m_fAccuracyPenalty();
	auto recoil_index = weap->m_flRecoilIndex();

	if (weap_sir <= 0.f)
		return;

	for (int i = 0; i < 150; i++)
	{
		float a = RandomFloat(0.f, 1.f);
		float b = RandomFloat(0.f, 6.2831855f);
		float c = RandomFloat(0.f, 1.f);
		float d = RandomFloat(0.f, 6.2831855f);

		float inac = a * weap_inac;
		float sir = c * weap_sir;

		if (weap->m_iItemDefinitionIndex() == 64)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}
		else if (weap->m_iItemDefinitionIndex() == 28 && recoil_index < 3.0f)
		{
			for (int i = 3; i > recoil_index; i--)
			{
				a *= a;
				c *= c;
			}

			a = 1.0f - a;
			c = 1.0f - c;
		}

		Vector sirVec((cos(b) * inac) + (cos(d) * sir), (sin(b) * inac) + (sin(d) * sir), 0), direction;

		direction.x = forward.x + (sirVec.x * right.x) + (sirVec.y * up.x);
		direction.y = forward.y + (sirVec.x * right.y) + (sirVec.y * up.y);
		direction.z = forward.z + (sirVec.x * right.z) + (sirVec.y * up.z);
		direction.Normalize();

		QAngle viewAnglesSpread;
		Math::VectorAngles(direction, up, viewAnglesSpread);
		viewAnglesSpread.Normalize();

		Vector viewForward;
		Math::AngleVectors(viewAnglesSpread, &viewForward);
		viewForward.NormalizeInPlace();

		viewForward = neu + (viewForward * weap_data->range);

		trace_t tr;
		Ray_t ray;

		ray.Init(neu, viewForward);

		CTraceFilter filter;
		filter.pSkip = cheat::main::local();

		Source::m_pEngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE | CONTENTS_WINDOW, &filter, &tr);

		Source::m_pDebugOverlay->AddBoxOverlay(tr.endpos, Vector(-0.5f, -0.5f, -0.5f), Vector(0.5f, 0.5f, 0.5f), Vector(0, 0, 0), 255, 0, 0, 0, Source::m_pGlobalVars->interval_per_tick * 2);
	}
}

bool c_aimbot::hit_chance(QAngle angle, C_BasePlayer *ent, float chance)
{
	//auto aechseVektor = [](Vector aechsen, float *forward, float *right, float *up) -> void
	//{
	//	float aechse;
	//	float sp, sy, cp, cy;
	//
	//	aechse = aechsen[0] * (3.1415f / 180);
	//	sp = sin(aechse);
	//	cp = cos(aechse);
	//
	//	aechse = aechsen[1] * (3.1415f / 180);
	//	sy = sin(aechse);
	//	cy = cos(aechse);
	//
	//	if (forward)
	//	{
	//		forward[0] = cp * cy;
	//		forward[1] = cp * sy;
	//		forward[2] = -sp;
	//	}
	//
	//	if (right || up)
	//	{
	//		float sr, cr;
	//
	//		aechse = aechsen[2] * (3.1415f / 180);
	//		sr = sin(aechse);
	//		cr = cos(aechse);
	//
	//		if (right)
	//		{
	//			right[0] = -1 * sr * sp * cy + -1 * cr * -sy;
	//			right[1] = -1 * sr * sp * sy + -1 * cr *cy;
	//			right[2] = -1 * sr * cp;
	//		}
	//
	//		if (up)
	//		{
	//			up[0] = cr * sp *cy + -sr * -sy;
	//			up[1] = cr * sp *sy + -sr * cy;
	//			up[2] = cr * cp;
	//		}
	//	}
	//};
	//
	//auto praviVektor = [](Vector src, Vector &dst) -> void
	//{
	//	float sp, sy, cp, cy;
	//
	//	Math::SinCos(DEG2RAD(src[1]), &sy, &cy);
	//	Math::SinCos(DEG2RAD(src[0]), &sp, &cp);
	//
	//	dst.x = cp * cy;
	//	dst.y = cp * sy;
	//	dst.z = -sp;
	//};
	//
	//auto vekAechse = [this](Vector &forward, Vector &up, Vector &aechse) -> void
	//{
	//	auto CrossProduct = [this](const Vector& a, const Vector& b) -> Vector
	//	{
	//		return Vector(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
	//	};
	//
	//	Vector left = CrossProduct(up, forward);
	//	left.NormalizeInPlace();
	//
	//	float forwardDist = forward.Length2D();
	//
	//	if (forwardDist > 0.001f)
	//	{
	//		aechse.x = atan2f(-forward.z, forwardDist) * 180 / M_PI_F;
	//		aechse.y = atan2f(forward.y, forward.x) * 180 / M_PI_F;
	//
	//		float upZ = (left.y * forward.x) - (left.x * forward.y);
	//		aechse.z = atan2f(left.z, upZ) * 180 / M_PI_F;
	//	}
	//	else
	//	{
	//		aechse.x = atan2f(-forward.z, forwardDist) * 180 / M_PI_F;
	//		aechse.y = atan2f(-left.x, left.y) * 180 / M_PI_F;
	//		aechse.z = 0;
	//	}
	//};
	//
	//auto weap = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon()));
	//
	//if (!weap)
	//	return false;
	//
	//if (chance < 2.f)
	//	return true;
	//
	//auto weap_data = weap->GetCSWeaponData();
	//
	//if (!weap_data)
	//	return false;
	//
	//Vector forward, right, up;
	//Vector neu = cheat::main::local()->GetEyePosition();
	//Math::AngleVectors(aechse, &forward, &right, &up);//aechseVektor(aechse, (float*)&forward, (float*)&right, (float*)&up);
	//
	//int cHits = 0;
	//int cNeededHits = static_cast<int>(150.f * (chance / 100.f));
	//
	//weap->UpdateAccuracyPenalty();
	//float weap_sir = weap->GetSpread();
	//float weap_inac = weap->GetInaccuracy() + weap->m_fAccuracyPenalty();
	//auto recoil_index = weap->m_flRecoilIndex();
	//
	//if (weap_sir <= 0.f)
	//	return true;
	//
	//for (int i = 0; i < 150; i++)
	//{
	//	float a = RandomFloat(0.f, 1.f);
	//	float b = RandomFloat(0.f, 6.2831855f);
	//	float c = RandomFloat(0.f, 1.f);
	//	float d = RandomFloat(0.f, 6.2831855f);
	//
	//	float inac = a * weap_inac;
	//	float sir = c * weap_sir;
	//
	//	if (weap->m_iItemDefinitionIndex() == 64)
	//	{
	//		a = 1.f - a * a;
	//		a = 1.f - c * c;
	//	}
	//	else if (weap->m_iItemDefinitionIndex() == 28 && recoil_index < 3.0f)
	//	{
	//		for (int i = 3; i > recoil_index; i--)
	//		{
	//			a *= a;
	//			c *= c;
	//		}
	//
	//		a = 1.0f - a;
	//		c = 1.0f - c;
	//	}
	//
	//	Vector sirVec((cos(b) * inac) + (cos(d) * sir), (sin(b) * inac) + (sin(d) * sir), 0), direction;
	//
	//	direction.x = forward.x + (sirVec.x * right.x) + (sirVec.y * up.x);
	//	direction.y = forward.y + (sirVec.x * right.y) + (sirVec.y * up.y);
	//	direction.z = forward.z + (sirVec.x * right.z) + (sirVec.y * up.z);
	//	direction.Normalize();
	//
	//	QAngle viewAnglesSpread;
	//	Math::VectorAngles(direction, up, viewAnglesSpread);
	//	viewAnglesSpread.Normalize();
	//
	//	Vector viewForward;
	//	Math::AngleVectors(viewAnglesSpread, &viewForward);
	//	viewForward.NormalizeInPlace();
	//
	//	viewForward = neu + (viewForward * weap_data->range);
	//
	//	trace_t tr;
	//	Ray_t ray;
	//
	//	ray.Init(neu, viewForward);
	//
	//	Source::m_pEngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE | CONTENTS_WINDOW, ent, &tr);
	//
	//	//Source::m_pDebugOverlay->AddLineOverlay(neu, tr.endpos, 255, 0, 0, false, Source::m_pGlobalVars->interval_per_tick * 2);
	//
	//	if (tr.m_pEnt == ent)
	//		++cHits;
	//
	//	if (static_cast<int>((static_cast<float>(cHits) / 150.f) * 100.f) >= chance)
	//		return true;
	//
	//	if ((150 - i + cHits) < cNeededHits)
	//		return false;
	//}
	//return false;

	int traces_hit = 0;

	Vector forward, right, up;
	auto eye_position = cheat::main::local()->GetEyePosition();
	Math::AngleVectors(angle/* + cheat::main::local()->m_aimPunchAngle() * 2.f*/, &forward, &right, &up); // maybe add an option to not account for punch.

	auto weapon = cheat::main::local()->get_weapon();

	if (!weapon || !weapon->GetCSWeaponData())
		return false;

	weapon->UpdateAccuracyPenalty();
	const float weap_spread = weapon->GetSpread();
	const float weap_inaccuracy = weapon->GetInaccuracy();// + min(cheat::main::local()->m_vecVelocity().Length2D() / 1500.f, 0.0015f);

	const auto needed = static_cast<int>(128 * (chance / 100.f));
	const auto allowed_misses = 128 - needed;

	for (int i = 0; i < 128; i++)
	{
		//if (cheat::game::last_cmd)
		RandomSeed(i + 1);

		float a = RandomFloat(0.f, 1.f);
		float b = RandomFloat(0.f, 6.2831855f);
		float c = RandomFloat(0.f, 1.f);
		float d = RandomFloat(0.f, 6.2831855f);

		float inaccuracy = a * weap_inaccuracy;
		float spread = c * weap_spread;

		if (weapon->m_iItemDefinitionIndex() == 64)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		Vector spreadView((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

		direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
		direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
		direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);

		QAngle viewAnglesSpread;
		Math::VectorAngles(direction, up, viewAnglesSpread);

		Vector viewForward;
		Math::AngleVectors(viewAnglesSpread, &viewForward);

		viewForward = eye_position + (viewForward * 8092.f);

		trace_t tr;
		Ray_t ray;

		ray.Init(eye_position, viewForward);

		Source::m_pEngineTrace->ClipRayToEntity(ray, 0x4600400B, ent, &tr);

		//Source::m_pDebugOverlay->AddLineOverlay(eye_position, viewForward, 255, 0, 0, false, 2.f * Source::m_pGlobalVars->interval_per_tick);

		if (tr.m_pEnt == ent && cheat::features::autowall.CanHit(eye_position, viewForward, cheat::main::local(), ent, 0) > 0.f)
			++traces_hit;

		const auto hitchance = (static_cast<float>(traces_hit) / 128.f) * 100.f;

		if (hitchance >= chance)
			return true;

		if (i - traces_hit > allowed_misses)
			return false;
	}

	return false;
}

void c_aimbot::visualise_hitboxes(C_BasePlayer* entity, matrix3x4_t *mx, Color color, float time)
{
	auto model = entity->GetModel();

	if (!model)
		return;

	auto studioHdr = Source::m_pModelInfo->GetStudioModel(model);

	if (!studioHdr
		/*|| !entity->SetupBones(matrix, 128, 0x100, 0)*/)
		return;

	auto set = studioHdr->pHitboxSet(entity->m_nHitboxSet());

	if (!set)
		return;

	for (int i = 0; i < set->numhitboxes; i++)
	{
		auto hitbox = set->pHitbox(i);

		if (!hitbox)
			continue;

		Vector min, max, center;
		Math::VectorTransform(hitbox->bbmin, mx[hitbox->bone], min);
		Math::VectorTransform(hitbox->bbmax, mx[hitbox->bone], max);



		if (hitbox->radius != -1) {
			Source::m_pDebugOverlay->AddCapsuleOverlay(min, max, hitbox->radius, color.r(), color.g(), color.b(), 255, time, 0, 1);
		}
		/*else
		{
		Math::VectorTransform((hitbox->bbmin + hitbox->bbmax) * 0.5f, mx[hitbox->bone], center);

		Vector v59 = Vector::Zero;
		Vector v76 = Vector::Zero;

		Math::MatrixAngles(mx[hitbox->bone], &v59, &v76);


		v59.x = 0;

		Source::m_pDebugOverlay->AddBoxOverlay(center, hitbox->bbmin, hitbox->bbmax, v59, color.r(), color.g(), color.b(), 200, time);
		}*/
	}
};

void c_aimbot::autostop(CUserCmd* cmd, bool &send_packet, C_WeaponCSBaseGun* local_weapon/*, C_BasePlayer* best_player, float dmg, bool hitchanced*/)
{
	static auto sv_accelerate = Source::m_pCvar->FindVar("sv_accelerate");
	static auto sv_stopspeed = Source::m_pCvar->FindVar("sv_stopspeed");
	static auto sv_friction = Source::m_pCvar->FindVar("sv_friction");

	if ((int)cheat::Cvars.RageBot_AutoStop.GetValue() == 0)
		return;

	static bool was_onground = cheat::main::local()->m_fFlags() & FL_ONGROUND;

	if (cheat::main::fast_autostop && local_weapon && local_weapon->GetCSWeaponData() && was_onground && cheat::main::local()->m_fFlags() & FL_ONGROUND)
	{
		auto speed = ((cmd->sidemove * cmd->sidemove) + (cmd->forwardmove * cmd->forwardmove));
		auto lol = sqrt(speed);

		auto velocity = cheat::main::local()->m_vecVelocity();
		float maxspeed = 30.f;

		if (!cheat::main::local()->m_bIsScoped())
			maxspeed = *(float*)(uintptr_t(local_weapon->GetCSWeaponData()) + 0x012C);
		else
			maxspeed = *(float*)(uintptr_t(local_weapon->GetCSWeaponData()) + 0x0130);//local_weapon->GetCSWeaponData()->max_speed;

		maxspeed *= 0.33f;
		maxspeed -= 1.f;

		auto v58 = sv_stopspeed->GetFloat();
		v58 = fmaxf(v58, velocity.Length2D());
		v58 = sv_friction->GetFloat() * v58;
		auto slow_walked_speed = fmaxf(velocity.Length2D() - (v58 * Source::m_pGlobalVars->interval_per_tick), 0.0f);

		switch ((int)cheat::Cvars.RageBot_AutoStopMethod.GetValue())
		{
		case 0:
		{
			if (velocity.Length2D() <= slow_walked_speed)
			{
				cmd->buttons &= ~IN_SPEED;

				cmd->sidemove = (maxspeed * (cmd->sidemove / lol));
				cmd->forwardmove = (maxspeed * (cmd->forwardmove / lol));
			}
			else
			{
				Engine::Movement::Instance()->quick_stop(cmd);
			}
		}
		break;
		case 1:
		{
			cheat::main::fakewalking = true;
			cheat::main::skip_shot = cmd->command_number;
		}
		break;
		case 2:
		{
			Engine::Movement::Instance()->quick_stop(cmd);
		}
		break;
		}
	}

	was_onground = (cheat::main::local()->m_fFlags() & FL_ONGROUND);

	cheat::main::fast_autostop = false;
}

float c_aimbot::check_wall(C_WeaponCSBaseGun* local_weapon, Vector startpoint, Vector direction, C_BasePlayer* entity)
{
	static float prev_dmg = -1.f;

	if (!local_weapon)
		return -1.f;

	auto weapon_info = local_weapon->GetCSWeaponData();

	if (!weapon_info)
		return -1.f;

	float autowall_dmg = 0;

	Vector start = startpoint;

	float maxRange = weapon_info->range;

	autowall_dmg = weapon_info->damage;

	auto max_range = weapon_info->range * 2;

	Vector end = start + (direction * max_range);

	//Source::m_pDebugOverlay->AddLineOverlay(start, end, 255, 0, 0, false, Source::m_pGlobalVars->interval_per_tick * 2.f);

	auto currentDistance = 0.f;

	CGameTrace enterTrace;

	CTraceFilter filter;
	filter.pSkip = cheat::main::local();

	cheat::features::autowall.TraceLine(start, end, MASK_SHOT | CONTENTS_GRATE, cheat::main::local(), &enterTrace);

	if (enterTrace.fraction == 1.0f)
		autowall_dmg = 0.f;
	else
		//calculate the damage based on the distance the bullet traveled.
		currentDistance += enterTrace.fraction * max_range;

	//Let's make our damage drops off the further away the bullet is.
	autowall_dmg *= pow(weapon_info->range_modifier, (currentDistance / 500.f));

	auto enterSurfaceData = Source::m_pPhysProps->GetSurfaceData(enterTrace.surface.surfaceProps);
	float enterSurfPenetrationModifier = enterSurfaceData->game.penetrationmodifier;

	if (currentDistance > 3000.0 && weapon_info->penetration > 0.f || enterSurfPenetrationModifier < 0.1f)
		return -1.f;

	//if (enterTrace.m_pEnt != nullptr)
	//{
	//	//This looks gay as fuck if we put it into 1 long line of code.
	//	bool canDoDamage = (enterTrace.hitgroup != HITGROUP_GEAR && enterTrace.hitgroup != HITGROUP_GENERIC);
	//	bool isPlayer = (enterTrace.m_pEnt->GetClientClass() && enterTrace.m_pEnt->GetClientClass()->m_ClassID == class_ids::CCSPlayer);
	//	bool isEnemy = (cheat::main::local()->m_iTeamNum() != ((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum());
	//	bool onTeam = (((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum() == 2 || ((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum() == 3);

	//	//TODO: Team check config
	//	if ((canDoDamage && isPlayer && isEnemy) && onTeam)
	if (entity)
	{
		int armorValue = entity->m_ArmorValue();
		int hitGroup = enterTrace.hitgroup;

		//Fuck making a new function, lambda beste. ~ Does the person have armor on for the hitbox checked?
		auto IsArmored = [&enterTrace, entity]()->bool
		{
			if ((DWORD)entity < 50977)
				return false;

			switch (enterTrace.hitgroup)
			{
			case HITGROUP_HEAD:
				return entity->m_bHasHelmet(); //Fuck compiler errors - force-convert it to a bool via (!!)
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				return true;
			default:
				return false;
			}
		};

		switch (hitGroup)
		{
		case HITGROUP_HEAD:
			autowall_dmg *= /*hasHeavyArmor ? 2.f :*/ 4.f; //Heavy Armor does 1/2 damage
			break;
		case HITGROUP_STOMACH:
			autowall_dmg *= 1.25f;
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			autowall_dmg *= 0.75f;
			break;
		default:
			break;
		}

		if (armorValue > 0 && IsArmored())
		{
			float bonusValue = 1.f, armorBonusRatio = 0.5f, armorRatio = weapon_info->armor_ratio / 2.f;

			////Damage gets modified for heavy armor users
			//if (hasHeavyArmor)
			//{
			//	armorBonusRatio = 0.33f;
			//	armorRatio *= 0.5f;
			//	bonusValue = 0.33f;
			//}

			auto NewDamage = autowall_dmg * armorRatio;

			//if (hasHeavyArmor)
			//	NewDamage *= 0.85f;

			if (((autowall_dmg - (autowall_dmg * armorRatio)) * (bonusValue * armorBonusRatio)) > armorValue)
				NewDamage = autowall_dmg - (armorValue / armorBonusRatio);

			autowall_dmg = NewDamage;
		}
	}

	//	if (!canDoDamage && isPlayer && isEnemy)
	//		return -1.f;

	//	return autowall_dmg;
	//}



	auto penetrate_count = 4;

	if (!cheat::features::autowall.HandleBulletPenetration(weapon_info, enterTrace, start, direction, penetrate_count, autowall_dmg, weapon_info->penetration, cheat::convars::ff_damage_bullet_penetration, true))
		return -1.f;

	if (penetrate_count <= 0)
		return -1.f;

	if (fabs(prev_dmg - autowall_dmg) < 3.f)
		autowall_dmg = prev_dmg;

	prev_dmg = autowall_dmg;

	return autowall_dmg;
}

//struct
//{
//	bool did_hitchance = false;
//	int prev_hitbox = 0;
//} prevtickdata;

void c_aimbot::get_hitbox_data(C_Hitbox* rtn, C_BasePlayer* ent, int ihitbox, matrix3x4_t* matrix)
{
	if (ihitbox < 0 || ihitbox > 19) return;

	if (!ent) return;

	const auto model = ent->GetModel();

	if (!model)
		return;

	auto pStudioHdr = Source::m_pModelInfo->GetStudioModel(model);

	if (!pStudioHdr)
		return;

	auto hitbox = pStudioHdr->pHitbox(ihitbox, 0);

	if (!hitbox)
		return;

	const auto is_capsule = hitbox->radius != -1.f;

	Vector min, max;
	if (is_capsule) {
		Math::VectorTransform(hitbox->bbmin, matrix[hitbox->bone], min);
		Math::VectorTransform(hitbox->bbmax, matrix[hitbox->bone], max);
	}
	else
	{
		Math::VectorTransform(Math::VectorRotate(hitbox->bbmin, hitbox->rotation), matrix[hitbox->bone], min);
		Math::VectorTransform(Math::VectorRotate(hitbox->bbmax, hitbox->rotation), matrix[hitbox->bone], max);
	}

	rtn->hitboxID = ihitbox;
	rtn->isOBB = !is_capsule;
	rtn->radius = hitbox->radius;
	rtn->mins = min;
	rtn->maxs = max;
	rtn->bone = hitbox->bone;
}

bool c_aimbot::work(CUserCmd* cmd, bool &send_packet)
{
	//m_entities.clear();
	C_BasePlayer* best_player = nullptr;
	Vector best_hitbox = Vector::Zero;
	int	best_hitboxid = -1;
	static int last_shoot_tick = 0;

	auto local_weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon()));

	if (!cheat::main::local() || !local_weapon)
		return false;

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

	if (!cheat::Cvars.RageBot_Enable.GetValue() || IsGrenade(local_weapon->m_iItemDefinitionIndex()) || local_weapon->is_knife())
		return false;

	if (cheat::Cvars.RageBot_autor8.GetValue())
	{
		/*if (local_weapon->m_iItemDefinitionIndex() == 64)
		{
			auto v7 = Source::m_pGlobalVars->curtime;
			if (r8cock_time <= (Source::m_pGlobalVars->frametime + v7))
				r8cock_time = v7 + 0.249f;
			else
				cmd->buttons |= IN_ATTACK;
		}
		else
		{
			r8cock_time = 0.0;
		}

		local_weapon->m_flPostponeFireReadyTime() = r8cock_time;*/

		if (local_weapon->m_iItemDefinitionIndex() == 64)
		{
			auto v33 = TICKS_TO_TIME(cheat::main::local()->m_nTickBase());
			is_cocking = 1;
			
			cmd->buttons &= ~IN_ATTACK;
			cmd->buttons &= ~IN_ATTACK2;

			if (local_weapon->can_cock())
			{
				if (r8cock_time <= v33)
				{
					if (local_weapon->m_flNextSecondaryAttack() <= v33)
					{
						r8cock_time = v33 + 0.234375;
					}
					else
					{
						cmd->buttons |= IN_ATTACK2;
						cmd->buttons |= IN_ATTACK2;
					}
				}
				else
				{
					cmd->buttons |= IN_ATTACK;
					cmd->buttons |= IN_ATTACK;
				}
				is_cocking = v33 > r8cock_time;
			}
			else
			{
				is_cocking = false;
				r8cock_time = v33 + 0.234375;
				cmd->buttons &= ~IN_ATTACK;
			}
		}
	}

	auto is_zeus = (local_weapon->m_iItemDefinitionIndex() == weapon_taser);

	static int hitboxesLoop[] =
	{
		HITBOX_STOMACH,
		HITBOX_LOWER_CHEST,
		HITBOX_UPPER_CHEST,
		HITBOX_HEAD,
		HITBOX_CHEST,
		HITBOX_RIGHT_THIGH,
		HITBOX_LEFT_THIGH,
		HITBOX_RIGHT_CALF,
		HITBOX_LEFT_CALF,
		HITBOX_RIGHT_FOOT,
		HITBOX_LEFT_FOOT,
		HITBOX_RIGHT_UPPER_ARM,
		HITBOX_LEFT_UPPER_ARM,
		HITBOX_RIGHT_FOREARM,
		HITBOX_LEFT_FOREARM
	};

	auto skip_hitscan = (cheat::Cvars.RageBot_Hitboxes.GetValue() == 0);

	for (auto k = 1; k <= 64; k++)
	{
		auto entity = (C_BasePlayer*)Source::m_pEntList->GetClientEntity(k);

		if (!entity 
			|| !entity->IsPlayer() 
			|| !entity->GetClientClass() 
			|| entity->IsDormant()
			|| entity->m_iHealth() <= 0
			|| entity->m_iTeamNum() == cheat::main::local()->m_iTeamNum() 
			|| entity->m_bGunGameImmunity()) 
			continue;

		auto idx = entity->entindex();

		float max_damage = cheat::Cvars.RageBot_MinDamage.GetValue();
		auto animstate = entity->get_animation_state();

		if (!animstate)
			continue;

		auto resolve_info = &cheat::features::aaa.player_resolver_records[idx - 1];

		int loopsize = ARRAYSIZE(hitboxesLoop) - 1;
		float maxRange = local_weapon->GetCSWeaponData()->range;
		auto hp = entity->m_iHealth();

		auto player_record = &cheat::features::lagcomp.records[idx - 1];
		auto distance = entity->m_vecOrigin().Distance(cheat::main::local()->m_vecOrigin());

		if (distance >= maxRange)
			continue;

		for (auto i = 0; i <= loopsize; i++)
		{
			const auto& ihitbox = hitboxesLoop[i];

			if (!skip_hitscan && (!cheat::Cvars.RageBot_Hitboxes.Has(0) && ihitbox == HITBOX_HEAD ||
				(!cheat::Cvars.RageBot_Hitboxes.Has(1) && ihitbox >= HITBOX_PELVIS && ihitbox <= HITBOX_UPPER_CHEST) ||
				(!cheat::Cvars.RageBot_Hitboxes.Has(2) && ihitbox >= HITBOX_RIGHT_HAND && ihitbox <= HITBOX_LEFT_FOREARM) ||
				(!cheat::Cvars.RageBot_Hitboxes.Has(3) && ihitbox >= HITBOX_RIGHT_THIGH && ihitbox <= HITBOX_LEFT_CALF) ||
				(!cheat::Cvars.RageBot_Hitboxes.Has(4) && ihitbox >= HITBOX_RIGHT_FOOT && ihitbox <= HITBOX_LEFT_FOOT)))
				continue;

			auto hitbox = get_hitbox(entity, hitboxesLoop[i], entity->m_CachedBoneData().Base());

			if (!hitbox.IsZero())
			{
				auto dmg = can_hit(hitboxesLoop[i], entity, hitbox, entity->m_CachedBoneData().Base());

				if (dmg > max_damage || (dmg > hp && cheat::Cvars.RageBot_ScaledmgOnHp.GetValue()))
				{
					max_damage = dmg;
					best_player = entity;
					best_hitbox = hitbox;
					best_hitboxid = hitboxesLoop[i];
				}
			}
		}
	}

	auto hitchance = false;
	auto aim_angles = QAngle(0, 0, 0);
	matrix3x4_t dolboeb[128];

	if (best_player != nullptr && !best_hitbox.IsZero())
	{
		memcpy(dolboeb, best_player->m_CachedBoneData().Base(), sizeof(matrix3x4_t) * best_player->GetBoneCount());
		aim_angles = Math::CalcAngle(cheat::main::local()->GetEyePosition(), best_hitbox);
		aim_angles.Clamp();

		hitchance = hit_chance(aim_angles, best_player, cheat::Cvars.RageBot_HitChance.GetValue());
	}
	
	if (!best_player || best_hitbox.IsZero() || aim_angles.IsZero())
		return false;

	cheat::features::lagcomp.finish_position_adjustment(best_player);

	auto tick_record = -1;

	auto entity_index = best_player->entindex() - 1;

	auto is_zoomable_weapon = (local_weapon->m_iItemDefinitionIndex() == weapon_ssg08 || local_weapon->m_iItemDefinitionIndex() == weapon_awp || local_weapon->m_iItemDefinitionIndex() == weapon_scar20 || local_weapon->m_iItemDefinitionIndex() == weapon_g3sg1);
	auto sniper = (local_weapon->m_iItemDefinitionIndex() == weapon_ssg08 || local_weapon->m_iItemDefinitionIndex() == weapon_awp);
	auto fakeduck = (cheat::game::pressed_keys[(int)cheat::Cvars.Exploits_fakeduck.GetValue()] && (int)cheat::Cvars.Exploits_fakeduck.GetValue() != 0);

	const bool can_shoot = Source::m_pClientState->m_iChockedCommands > 0 && Source::m_pClientState->m_iChockedCommands <= 14 || cheat::Cvars.RageBot_SilentAim.GetValue() != 2;

	if (local_weapon->can_shoot() && cheat::main::skip_shot && (cheat::Cvars.RageBot_AutoFire.GetValue() || (cheat::game::pressed_keys[(int)cheat::Cvars.RageBot_Key.GetValue()] && (int)cheat::Cvars.RageBot_Key.GetValue() != 0)))
	{
		cheat::main::fast_autostop = (int)cheat::Cvars.RageBot_AutoStop.GetValue()>0;
		
		tick_record = cheat::features::lagcomp.records[entity_index].tick_count;

		if (cheat::Cvars.RageBot_AutoScope.GetValue() && local_weapon->m_zoomLevel() == 0 && is_zoomable_weapon) 
		{
			cmd->buttons |= 0x800u;
			return false;
		}

		if (hitchance && can_shoot)
		{
			cmd->viewangles = aim_angles;
			cmd->buttons |= IN_ATTACK;

			if (tick_record > 0)
				cmd->tick_count = tick_record;
			else {
				if (!cheat::features::lagcomp.is_time_delta_too_large(best_player->m_flSimulationTime() + cheat::main::lerp_time))
					cmd->tick_count = TIME_TO_TICKS(best_player->m_flSimulationTime() + cheat::main::lerp_time);
			}

			cheat::main::shots_fired[entity_index]++;
			cheat::main::shots_total[entity_index]++;
			cheat::features::aaa.player_resolver_records[entity_index].missed_shots[cheat::features::aaa.player_resolver_records[entity_index].resolving_method]++;

			if (cheat::Cvars.RageBot_SilentAim.GetValue() == 2)
				send_packet = false;

			visualise_hitboxes(best_player, dolboeb, Color::Red(), 4);

			if (auto net = Source::m_pEngine->GetNetChannelInfo(); net != nullptr && (cheat::features::lagcomp.records[entity_index].tick_records.size() > cheat::features::lagcomp.records[entity_index].backtrack_ticks && cheat::features::lagcomp.records[entity_index].tick_records.size() > 1)) {
				auto impact_time = Source::m_pGlobalVars->curtime /*- net->GetLatency(FLOW_INCOMING) - net->GetLatency(FLOW_OUTGOING)*/ + /*Source::m_pGlobalVars->interval_per_tick*/net->GetLatency(MAX_FLOWS);
				cheat::main::fired_shot.push_back(_shotinfo(best_player, dolboeb, cheat::main::local()->GetEyePosition(), best_hitbox, &cheat::features::lagcomp.records[entity_index], cheat::features::lagcomp.records[entity_index].tick_records.at(cheat::features::lagcomp.records[entity_index].backtrack_ticks), best_hitboxid, impact_time, local_weapon->GetSpread()));
			}

			if (!cheat::Cvars.RageBot_SilentAim.GetValue())
				Source::m_pEngine->SetViewAngles(aim_angles);

			last_shoot_tick = Source::m_pGlobalVars->tickcount;

			return true;
		}
	}

	return false;
}