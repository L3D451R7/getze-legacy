#include "visuals.hpp"
#include "source.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "hooked.hpp"
#include "aimbot.hpp"
#include "displacement.hpp"
#include "lag_compensation.hpp"
#include "anti_aimbot.hpp"
#include "angle_resolver.hpp"
#include "prop_manager.hpp"
#include "game_movement.h"
#include "autowall.hpp"
#include <algorithm>
#include "rmenu.hpp"
#include "movement.hpp"
#include "sound_parser.hpp"
#include "menu.h"

std::vector<C_BasePlayer*> m_entities;
std::vector<C_BasePlayer*> m_worldentities;
std::vector<_event> _events;
std::vector<c_bullet_tracer> bullet_tracers;
//std::vector<std::string> m_esp_info;

bool c_visuals::get_espbox(C_BasePlayer *entity, int& x, int& y, int& w, int& h)
{
	if (entity->GetClientClass() && entity->GetClientClass()->m_ClassID == class_ids::CCSPlayer) {
		auto min = entity->GetCollideable()->OBBMins();
		auto max = entity->GetCollideable()->OBBMaxs();

		if (cheat::Cvars.Visuals_Box.GetValue() == 2) {
			entity->ComputeHitboxSurroundingBox(entity->m_CachedBoneData().Base(), &min, &max);

			max.z += 5.f;
		}

		Vector vF, vR, vU;

		auto dir = Engine::Movement::Instance()->m_qRealAngles;

		dir.x = 0;
		dir.z = 0;
		//dir.Normalize();
		//printf("%.1f\n", dir.y);
		Math::AngleVectors(dir, &vF, &vR, &vU);

		auto zh = vU * max.z + vF * max.y + vR * min.x; // = Front left front
		auto e = vU * max.z + vF * max.y + vR * max.x; //  = Front right front
		auto d = vU * max.z + vF * min.y + vR * min.x; //  = Front left back
		auto c = vU * max.z + vF * min.y + vR * max.x; //  = Front right back

		auto g = vU * min.z + vF * max.y + vR * min.x; //  = Bottom left front
		auto f = vU * min.z + vF * max.y + vR * max.x; //  = Bottom right front
		auto a = vU * min.z + vF * min.y + vR * min.x; //  = Bottom left back
		auto b = vU * min.z + vF * min.y + vR * max.x; //  = Bottom right back*-

		Vector pointList[] = {
			a,
			b,
			c,
			d,
			e,
			f,
			g,
			zh,
		};

		Vector transformed[ARRAYSIZE(pointList)];

		if (cheat::Cvars.Visuals_Box.GetValue() != 2) {
			for (int i = 0; i < ARRAYSIZE(pointList); i++)
			{
				if (cheat::Cvars.Visuals_Box.GetValue() != 2)
					pointList[i] += entity->get_abs_origin();

				if (!Drawing::WorldToScreen(pointList[i], transformed[i]))
					return false;
			}
		}
		else
		{
			Vector PpointList[] = {
				Vector(min.x, min.y, min.z),
				Vector(min.x, max.y, min.z),
				Vector(max.x, max.y, min.z),
				Vector(max.x, min.y, min.z),
				Vector(max.x, max.y, max.z),
				Vector(min.x, max.y, max.z),
				Vector(min.x, min.y, max.z),
				Vector(max.x, min.y, max.z)
			};

			for (int i = 0; i < ARRAYSIZE(PpointList); i++)
			{
				if (!Drawing::WorldToScreen(PpointList[i], transformed[i]))
					return false;
			}
		}


		float left = FLT_MAX;
		float top = -FLT_MAX;
		float right = -FLT_MAX;
		float bottom = FLT_MAX;
		for (int i = 0; i < ARRAYSIZE(pointList); i++) {
			if (left > transformed[i].x)
				left = transformed[i].x;
			if (top < transformed[i].y)
				top = transformed[i].y;
			if (right < transformed[i].x)
				right = transformed[i].x;
			if (bottom > transformed[i].y)
				bottom = transformed[i].y;
		}

		x = left;
		y = bottom;
		w = right - left;
		h = top - bottom;

		return true;
	}
	else
	{
		Vector vOrigin, min, max, flb, brt, blb, frt, frb, brb, blt, flt;
		//float left, top, right, bottom;

		auto collideable = entity->GetCollideable();

		if (!collideable)
			return false;

		min = collideable->OBBMins();
		max = collideable->OBBMaxs();

		matrix3x4_t &trans = entity->GetCollisionBoundTrans();

		Vector points[] =
		{
			Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z)
		};

		Vector pointsTransformed[8];
		for (int i = 0; i < 8; i++) {
			Math::VectorTransform(points[i], trans, pointsTransformed[i]);
		}

		Vector pos = entity->m_vecOrigin();

		if (!Drawing::WorldToScreen(pointsTransformed[3], flb) || !Drawing::WorldToScreen(pointsTransformed[5], brt)
			|| !Drawing::WorldToScreen(pointsTransformed[0], blb) || !Drawing::WorldToScreen(pointsTransformed[4], frt)
			|| !Drawing::WorldToScreen(pointsTransformed[2], frb) || !Drawing::WorldToScreen(pointsTransformed[1], brb)
			|| !Drawing::WorldToScreen(pointsTransformed[6], blt) || !Drawing::WorldToScreen(pointsTransformed[7], flt))
			return false;

		Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };
		//+1 for each cuz of borders at the original box
		float left = flb.x;        // left
		float top = flb.y;        // top
		float right = flb.x;    // right
		float bottom = flb.y;    // bottom

		for (int i = 1; i < 8; i++)
		{
			if (left > arr[i].x)
				left = arr[i].x;
			if (bottom < arr[i].y)
				bottom = arr[i].y;
			if (right < arr[i].x)
				right = arr[i].x;
			if (top > arr[i].y)
				top = arr[i].y;
		}

		x = (int)left;
		y = (int)top;
		w = (int)(right - left);
		h = (int)(bottom - top);

		return true;
	}
	return false;
}

void c_visuals::draw_beam(Vector Start, Vector End, Color color, float Width)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS;
	beamInfo.m_pszModelName = "sprites/purplelaser1.vmt";
	beamInfo.m_nModelIndex = -1;
	beamInfo.m_flHaloScale = 0.0f;
	beamInfo.m_flLife = 4.f;

	beamInfo.m_flWidth = Width;
	beamInfo.m_flEndWidth = Width;

	beamInfo.m_flFadeLength = 0.0f;
	beamInfo.m_flAmplitude = 2.0f;
	beamInfo.m_flBrightness = 255.f;
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0;
	beamInfo.m_flFrameRate = 0.f;
	beamInfo.m_nSegments = 2;
	beamInfo.m_bRenderable = true;
	beamInfo.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

	beamInfo.m_flRed = color.r();
	beamInfo.m_flGreen = color.g();
	beamInfo.m_flBlue = color.b();

	beamInfo.m_vecStart = Start;
	beamInfo.m_vecEnd = End;

	auto myBeam = Engine::Displacement::Data::m_uClientBeams->CreateBeamPoints(beamInfo);

	if (myBeam) 
		Engine::Displacement::Data::m_uClientBeams->DrawBeam(myBeam);
}

void c_visuals::render_tracers()
{
	if (!cheat::main::local() || cheat::main::local()->IsDead()) return;

	if (bullet_tracers.empty()) return;

	for (size_t i = 0; i < bullet_tracers.size(); i++)
	{
		//get the current item
		auto current = bullet_tracers.at(i);
		draw_beam(current.src, current.dst, current.color1, 1);

		bullet_tracers.erase(bullet_tracers.begin() + i);
	}
}

void c_visuals::logs()
{
	int bottom = 0;

	if (!cheat::main::fired_shot.empty()) {
		auto record = &cheat::main::fired_shot.back();

		if (fabs(record->_avg_impact_time - Source::m_pGlobalVars->curtime) > 5.f)
			cheat::main::fired_shot.clear();

		//if (record->_target && !record->_target->IsDormant())
		//	skeleton(record->_target, Color::White(), record->_matrix);

		if (record->_impact_called && !record->_printed) {
			//if (record->_hurt_called) {
				//auto precord = record->_record;
				//auto rtype = precord->type;
				//auto bticks = precord->backtrack_ticks;

				//_events.push_back(_event(Source::m_pGlobalVars->curtime + 3.f, std::string(bticks > 0 ? "hit resolved backtrack shot." : "hit estimated angle.")));//"hit | priority: [" + std::string(rtype == RECORD_SHOT ? "true" : "false") + "] | backtrack: [" + (bticks > 0 ? "true" : "false") + "] ")));//(record->_backtracked_ticks ? "hit resolved backtrack shot." : "hit estimated angle.")));
			//}
			//else {
			if (!record->_hurt_called) {
				if (cheat::Cvars.Misc_AntiUT.GetValue() > 0)
				{
					auto start = record->_eyepos;
					auto entity = record->_target;
					auto endpos = record->_impact_pos;
					auto hitbox = record->_hitbox;
					auto precord = record->_record;
					auto rtype = precord->type;
					auto bticks = precord->backtrack_ticks;

					if (entity != nullptr && entity->GetModelPtr())
					{
						const auto angle = Math::CalcAngle(start, endpos);
						Vector forward;
						Math::AngleVectors(angle, &forward);
						auto end = start + forward * 8092.f;

						int type = 0; // spread

						auto didhit = false;

						C_Hitbox box; cheat::features::aimbot.get_hitbox_data(&box, entity, record->_hitboxid, record->_matrix);

						didhit = box.isOBB ? Math::IntersectBB(start, end, box.mins, box.maxs) : Math::Intersect(start, end, box.mins, box.maxs, box.radius);

						if (didhit/* || record->_spread < 0.005f*/)
							type = 1; // resolve issue

						auto resolve_info = &cheat::features::aaa.player_resolver_records[entity->entindex() - 1];

						if (type)
							_events.push_back(_event(Source::m_pGlobalVars->curtime + 5.f, "missed shot due to resolver.", std::string("R: " + std::to_string(resolve_info->resolving_method) + " | last delta: " + std::to_string(resolve_info->latest_delta_used))));
						else {
							if (cheat::main::shots_fired[entity->entindex() - 1] > 0)
								cheat::main::shots_fired[entity->entindex() - 1]--;

							_events.push_back(_event(Source::m_pGlobalVars->curtime + 5.f, "missed shot due to spread.", std::string("R: " + std::to_string(resolve_info->resolving_method) + " | last delta: " + std::to_string(resolve_info->latest_delta_used))));
						}
					}
				}
			}

			record->_printed = true;
		}
	}

	if (_events.empty())
		return;

	//static auto name_siz = Drawing::GetTextSize(F::ESP, "[gangster]");

	for (auto &event : _events)
	{
		if (_events.back()._time < Source::m_pGlobalVars->curtime)
			_events.pop_back();

		if (event._time < Source::m_pGlobalVars->curtime && event._displayticks > 0.f)
			continue;

		if (bottom > 10)
			break;

		if (event._msg.size() > 0) {

			//auto good = (event._msg.find("hurt") != std::string::npos);

			if (!event._displayed)
			{
				Source::m_pCvar->ConsoleColorPrintf(cheat::Cvars.MenuTheme.GetColor(), "[EAGLE]");
				Source::m_pCvar->ConsoleColorPrintf((/*good ? Color::Green() : */Color::White()), " %s\n", event._msg.c_str());

				if ((event._msg.find("missed") != std::string::npos || event._msg.find("hurt") != std::string::npos) && event._secretmsg.size() > 0)
					Source::m_pCvar->ConsoleColorPrintf((Color::Green()), " %s\n", event._secretmsg.c_str());
				
				event._displayed = true;
			}

			auto disp_ticks = (event._displayticks > 1.f ? 1.f : event._displayticks);
			float alpha = std::clamp(event._time - Source::m_pGlobalVars->curtime, 0.f, disp_ticks) * (255.f / disp_ticks);

			static auto name_siz = Drawing::GetTextSize(F::OldESP, "[EAGLE]");
			Drawing::DrawString(F::OldESP, 5, 2 + 14 * bottom, cheat::Cvars.MenuTheme.GetColor().alpha(alpha), FONT_LEFT, "[EAGLE]");
			Drawing::DrawString(F::OldESP, 5 + name_siz.right + 3, 2 + 14 * bottom++, (/*good ? Color::Green(alpha) : */Color::White(alpha)), FONT_LEFT, "%s", event._msg.c_str());
		}
	}
}

//void c_visuals::logs()
//{
//	int bottom = 0;
//
//	if (!cheat::main::fired_shot.empty()) {
//		auto record = &cheat::main::fired_shot.back();
//
//		if (fabs(record->_avg_impact_time - Source::m_pGlobalVars->curtime) > 0.5f)
//			cheat::main::fired_shot.clear();
//
//		if (record->_impact_called && !record->_printed) {
//			if (record->_hurt_called) {
//				auto rtype = record->_type;
//				auto bticks = record->_backtracked_ticks;
//				_events.push_back(_event(Source::m_pGlobalVars->curtime + 3.f, std::string(bticks ? "hit resolved backtrack shot." : "hit estimated angle.")/*std::string("hit | priority: [" + std::string(rtype == RECORD_SHOT ? "true" : "false") + "] | backtrack: [" + std::to_string(bticks) + "t] ")*/, Color::Green()));//(record->_backtracked_ticks ? "hit resolved backtrack shot." : "hit estimated angle.")));
//			}
//			else {
//				if (cheat::settings.ragebot_hitchance > 0)
//				{
//					/*auto start = record->_eyepos;
//					auto end = record->_impact_pos;
//					auto mx = record->_matrix;
//					auto entity = record->_target;
//
//					CGameTrace tr;
//					Ray_t ray;
//
//					ray.Init(start, end);
//
//					Source::m_pEngineTrace->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, entity, &tr);
//
//					if (tr.m_pEnt == entity)
//					_events.push_back(_event(Source::m_pGlobalVars->curtime + 3.f, "missed shot due to bad resolve."));
//					else
//					_events.push_back(_event(Source::m_pGlobalVars->curtime + 3.f, "missed shot due to spread."));*/
//
//					auto start = record->_eyepos;
//					auto entity = record->_target;
//					auto end = record->_impact_pos;
//					auto hitbox = record->_hitbox;
//					auto rtype = record->_type;
//					auto bticks = record->_backtracked_ticks;
//
//					QAngle anglesReal = Math::CalcAngle(start, end);
//
//					float distance = (hitbox - start).Length();
//
//					Vector simulatedPoint;
//					Math::AngleVectors(anglesReal, &simulatedPoint);
//					simulatedPoint.NormalizeInPlace();
//					simulatedPoint = start + simulatedPoint * distance;
//
//					simulatedPoint.z -= 1.f;
//
//					float delta = (simulatedPoint - hitbox).Length();
//
//					//float damage = cheat::features::autowall.CanHit(start, simulatedPoint);
//
//					int type = 0;
//
//					if (/*damage <= 0.0f || */delta >= 15.0f) {
//						type = 0;
//
//						if (cheat::main::shots_fired[entity->entindex() - 1] > 0)
//							cheat::main::shots_fired[entity->entindex() - 1] -= 1;
//					}
//					else
//						type = 1;
//
//					//if (rtype != RECORD_SHOT)
//					//	cheat::main::shots_fired[entity->entindex() - 1] -= 1;
//
//					/*if (damage <= 0.0f || delta >= 11.0f) {
//					_events.push_back(_event(Source::m_pGlobalVars->curtime + 3.f, "missed shot due to spread."));
//					cheat::main::shots_fired[entity->entindex() - 1] -= 1;
//					}
//					else
//					_events.push_back(_event(Source::m_pGlobalVars->curtime + 3.f, "missed shot due to desync."));*/
//
//					_events.push_back(_event(Source::m_pGlobalVars->curtime + 1.f, std::string("missed shot due to " + std::string(type == 0 ? "spread" : "desync"))/*std::string("miss | priority: [" + std::string(rtype != RECORD_NORMAL ? "true" : "false") + "] | reason: [" + (type == 0 ? "spread" : "incorrect angles") + "] | backtrack: [" + std::to_string(bticks) + "t] ")*/, Color::Red()));
//				}
//				else
//				{
//					_events.push_back(_event(Source::m_pGlobalVars->curtime + 1.f, "missed shot due to lag compensation."));
//				}
//			}
//
//			record->_printed = true;
//		}
//	}
//
//	if (_events.empty())
//		return;
//
//	//static auto name_siz = Drawing::GetTextSize(F::ESP, "[gangster]");
//
//	for (auto &event : _events)
//	{
//		if (_events.back()._time < Source::m_pGlobalVars->curtime)
//			_events.pop_back();
//
//		if (event._time < Source::m_pGlobalVars->curtime && event._displayticks > 0.f)
//			continue;
//
//		if (bottom > 10)
//			break;
//
//		if (event._msg.size() > 0) {
//
//			//auto good = (event._msg.find("hit") != std::string::npos);
//
//			if (!event._displayed)
//			{
//				Source::m_pCvar->ConsoleColorPrintf(Color(50, 122, 239), "[EAGLE]");
//				Source::m_pCvar->ConsoleColorPrintf(event._colr, " %s\n", event._msg.c_str());
//				event._displayed = true;
//			}
//
//			auto disp_ticks = event._displayticks;
//			auto disp_ticks_clamped = (disp_ticks > 1.f ? 1.f : disp_ticks);
//			float time_to_end = std::clamp(event._time - Source::m_pGlobalVars->curtime, 0.f, disp_ticks);
//			float time_to_end_clamped = std::clamp(event._time - Source::m_pGlobalVars->curtime, 0.f, disp_ticks_clamped);
//			float alpha = time_to_end * (255.f / disp_ticks_clamped);
//
//			if (event._x == 0.f)
//				event._x = (screenW / 3);
//
//			event._x += (time_to_end > (disp_ticks / 2) ? -1 : 1);
//			auto ymin = screenH / 3 - 200 + (disp_ticks - time_to_end) * 150;
//
//			Drawing::DrawString(F::LBY, event._x, ymin/*2 + 14 * bottom++*/, event._colr.alpha(alpha), FONT_LEFT, "%s", event._msg.c_str());
//		}
//	}
//}

void c_visuals::skeleton(C_BasePlayer *Entity, Color color, matrix3x4_t *pBoneToWorldOut)
{
	auto model = Entity->GetModel();
	if (!model) return;

	auto studio_model = Source::m_pModelInfo->GetStudioModel(Entity->GetModel());
	if (!studio_model) return;

	for (int i = 0; i < studio_model->numbones; i++)
	{
		auto pBone = studio_model->pBone(i);

		if (!pBone || !(pBone->flags & 256) || pBone->parent == -1)
			continue;

		if (Vector(pBoneToWorldOut[i][0][3], pBoneToWorldOut[i][1][3], pBoneToWorldOut[i][2][3]).IsZero() || Vector(pBoneToWorldOut[pBone->parent][0][3], pBoneToWorldOut[pBone->parent][1][3], pBoneToWorldOut[pBone->parent][2][3]).IsZero())
			continue;

		Vector vBonePos1;
		if (!Drawing::WorldToScreen(Vector(pBoneToWorldOut[i][0][3], pBoneToWorldOut[i][1][3], pBoneToWorldOut[i][2][3]), vBonePos1))
			continue;

		Vector vBonePos2;
		if (!Drawing::WorldToScreen(Vector(pBoneToWorldOut[pBone->parent][0][3], pBoneToWorldOut[pBone->parent][1][3], pBoneToWorldOut[pBone->parent][2][3]), vBonePos2))
			continue;

		Drawing::DrawLine((int)vBonePos1.x, (int)vBonePos1.y, (int)vBonePos2.x, (int)vBonePos2.y, color);
	}
}

void c_visuals::draw_pov_arrows(C_BasePlayer *entity, float alpha)
{
	auto idx = entity->entindex() - 1;
	Vector poopvec;
	int screen_w, screen_h;
	Source::m_pEngine->GetScreenSize(screen_w, screen_h);

	Vector vEnemyOrigin = entity->m_vecOrigin();
	Vector vLocalOrigin = cheat::main::local()->m_vecOrigin();

	//if (Drawing::WorldToScreen(vEnemyOrigin, poopvec))
	//	return;

	auto isOnScreen = [](Vector origin, Vector& screen) -> bool
	{
		if (!Drawing::WorldToScreen(origin, screen))
			return false;

		int iScreenWidth, iScreenHeight;
		Source::m_pEngine->GetScreenSize(iScreenWidth, iScreenHeight);

		bool xOk = iScreenWidth > screen.x > 0, yOk = iScreenHeight > screen.y > 0;
		return xOk && yOk;
	};

	Vector screenPos;

	if (!entity->IsDormant()) 
	{
		if (isOnScreen(cheat::features::aimbot.get_hitbox(entity, 2), screenPos)) //TODO (?): maybe a combo/checkbox to turn this on/off
			return;
	}
	else
	{
		if (isOnScreen(vEnemyOrigin, screenPos))
			return;
	}

	float view_angle = Engine::Movement::Instance()->m_qRealAngles.y;
	if (view_angle < 0.f)
	{
		view_angle += 360.f;
	}
	view_angle = DEG2RAD(view_angle);

	auto entity_angle = Math::CalcAngle(vLocalOrigin, vEnemyOrigin);
	entity_angle.Normalized();

	if (entity_angle.y < 0.f) {
		entity_angle.y += 360.f;
	}
	entity_angle.y = DEG2RAD(entity_angle.y);
	entity_angle.y -= view_angle;

	auto position = Vector2D(screen_w / 2, screen_h / 2);
	position.x -= 180;

	Drawing::rotate_point(position, Vector2D(screen_w / 2, screen_h / 2), false, entity_angle.y);

	auto size = std::clamp(100 - int(vEnemyOrigin.Distance(vLocalOrigin) / 6), 10, 25);

	Drawing::filled_tilted_triangle(position, Vector2D(size-1, size), position, true, -entity_angle.y, (entity->IsDormant() ? Color(150, 0, 0, alpha) : Color(255,0,0, alpha)));
}

class CPredTraceFilter : public ITraceFilter {
public:
	CPredTraceFilter() = default;

	bool ShouldHitEntity(IHandleEntity* pEntityHandle, int /*contentsMask*/) {
		auto it = std::find(entities.begin(), entities.end(), pEntityHandle);
		if (it != entities.end())
			return false;

		ClientClass* pEntCC = ((C_BasePlayer*)pEntityHandle)->GetClientClass();
		if (pEntCC && strcmp(ccIgnore, "")) {
			if (pEntCC->m_pNetworkName == ccIgnore)
				return false;
		}

		return true;
	}

	virtual int GetTraceType() const { return TRACE_EVERYTHING; }

	inline void SetIgnoreClass(const char* Class) { ccIgnore = Class; }

	std::vector< void* > entities;
	const char* ccIgnore = "";
};

void GrenadePrediction(C_WeaponCSBaseGun* weapon)
{
	static Vector origin = Vector::Zero;
	static Vector vel = Vector::Zero;
	static std::vector< void* > ignore_entities;
	// Needed stuff
	auto molotov_throw_detonate_time = Source::m_pCvar->FindVar("molotov_throw_detonate_time");
	auto weapon_molotov_maxdetonateslope = Source::m_pCvar->FindVar("weapon_molotov_maxdetonateslope");
	auto sv_gravity = Source::m_pCvar->FindVar("sv_gravity");

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

	auto GetGrenadeDetonateTime = [molotov_throw_detonate_time](int item)
	{
		switch (item)
		{
		case weapon_flashbang:
		case weapon_hegrenade:
			return 1.5f;
			break;
		case weapon_incgrenade:
		case weapon_molotov:
			return molotov_throw_detonate_time->GetFloat();
			break;
		case weapon_tagrenade:
			return 5.f;
			break;
		}

		return 3.f;
	};

	auto DrawLine = [](Vector start, Vector end, int r, int g, int b, int a = 255)
	{
		Vector startw2s, endw2s;

		if (!Drawing::WorldToScreen(start, startw2s)
			|| !Drawing::WorldToScreen(end, endw2s))
			return;

		Source::m_pSurface->DrawSetColor(r, g, b, a);
		Source::m_pSurface->DrawLine(startw2s.x, startw2s.y, endw2s.x, endw2s.y);
	};

	auto PhysicsClipVelocity = [](const Vector& in, const Vector& normal, Vector& out, float overbounce)
	{
		int blocked = 0;
		float angle = normal[2];

		if (angle > 0)
			blocked |= 1; // floor

		if (!angle)
			blocked |= 2; // step

		float backoff = in.Dot(normal) * overbounce;

		for (int i = 0; i < 3; i++)
		{
			out[i] = in[i] - (normal[i] * backoff);

			if (out[i] > -0.1f && out[i] < 0.1f)
				out[i] = 0;
		}

		return blocked;
	};

	auto itemIndex = weapon->m_iItemDefinitionIndex();
	auto wpnData = weapon->GetCSWeaponData();

	if (!wpnData
		|| !IsGrenade(itemIndex)) {
		ignore_entities.clear();
		return;
	}

	Vector forward;

	auto angThrow = Engine::Movement::Instance()->m_qRealAngles;

	angThrow.x = Math::NormalizeFloat(angThrow.x);
	angThrow.x -= (90.f - abs(angThrow.x)) * 0.11111111f;

	Math::AngleVectors(angThrow, &forward);

	float throwStrengh = weapon->m_flThrowStrength();
	float throwVelocity = fmin(fmax(wpnData->throw_velocity * 0.9f, 15.f), 750.f);

	float throwHeight = (throwStrengh * 12.f) - 12.f;
	float v68 = throwVelocity * ((0.7f * throwStrengh) + 0.3f);

	//if (!cheat::main::jittering || origin.IsZero() || vel.IsZero()) {
		origin = cheat::main::local()->GetEyePosition();
		vel = cheat::main::local()->m_vecVelocity();
	//}

	// NOTE: m_vecEyePosition = GetAbsOrigin + m_vecViewOffset
	Vector startPos = origin + Vector(0, 0, throwHeight), endPos = startPos + (forward * 22.f);

	ignore_entities.push_back(cheat::main::local());
	
	CPredTraceFilter filter;
	filter.pSkip = cheat::main::local();
	filter.SetIgnoreClass("CBaseCSGrenadeProjectile");
	filter.entities = ignore_entities;

	trace_t trace;

	Ray_t ray; ray.Init(startPos, endPos, Vector(-2.f, -2.f, -2.f), Vector(2.f, 2.f, 2.f));

	Source::m_pEngineTrace->TraceRay(ray, 0x200400B, &filter, &trace);

	endPos = trace.endpos - (forward * 6.f);
	auto throwPos = (vel * 1.25f) + (forward * v68);

	DrawLine(startPos, endPos, 255, 255, 255);
	float gravity = (sv_gravity->GetFloat() * 0.4f);

	for (int ticks = TIME_TO_TICKS(GetGrenadeDetonateTime(itemIndex)); ticks >= 0; --ticks)
	{
		auto throwDir = Vector(throwPos.x, throwPos.y, (throwPos.z + (throwPos.z - (gravity * Source::m_pGlobalVars->interval_per_tick))) * 0.5f);

		auto temp = throwDir * Source::m_pGlobalVars->interval_per_tick;

		throwPos.z -= gravity * Source::m_pGlobalVars->interval_per_tick;

		auto src = endPos, end = endPos + temp;

		Ray_t ray2; ray2.Init (src, end, Vector(-2.f, -2.f, -2.f), Vector(2.f, 2.f, 2.f));

		Source::m_pEngineTrace->TraceRay(ray2, 0x4200608B, &filter, &trace);

		if (trace.allsolid)
			throwPos = Vector(0, 0, 0);

		endPos = trace.endpos;
		DrawLine(src, endPos, 255, 255, 255);

		if (trace.fraction != 1.f)
		{
			if (Vector screen; Drawing::WorldToScreen(endPos, screen))
				Drawing::DrawRect(screen.x - 2, screen.y - 2, 4, 4, Color::LightBlue());

			float surfaceElasticity = 1.f;

			Vector throwPos2;

			//if (trace.m_pEnt) // if we did hit an entity
			//{
			//	if (trace.m_pEnt->GetClientClass() && trace.m_pEnt->GetClientClass()->m_ClassID == 35) // did hit a player
			//	{
			//		throwVelocity *= 0.2f;
			//		surfaceElasticity = 0.3f;

			//		v68 = throwVelocity * ((0.7f * throwStrengh) + 0.3f);
			//		throwPos = (cheat::main::local()->m_vecVelocity() * 1.25f) + (forward * v68);

			//		endPos = trace.endpos;

			//		ignore_entities.push_back(trace.m_pEnt);
			//	}
			//	else {
			//		if (cheat::features::autowall.IsBreakableEntity((C_BasePlayer*)trace.m_pEnt)) // hit world, check for breakableent
			//		{
			//			if (((C_BasePlayer*)trace.m_pEnt)->m_iHealth() <= 0) // if will break it, slow down the grenade. Should include bounce mechanic but i've never seen nades bounce off breakables, so didn't bother
			//			{
			//				throwVelocity *= 0.4f;
			//				endPos = trace.endpos;
			//				ignore_entities.push_back(trace.m_pEnt);
			//			}
			//		}
			//	}
			//}


			PhysicsClipVelocity(throwPos, trace.plane.normal, throwPos2, 2.f);

			player_info playerInfo;

			if (trace.m_pEnt && Source::m_pEngine->GetPlayerInfo(trace.m_pEnt->entindex(), &playerInfo)) {
				surfaceElasticity = 0.3f;
				throwVelocity *= 0.2f;
			}

			throwPos2 *= std::clamp(surfaceElasticity * 0.45f, 0.f, 0.9f);

			end = endPos + (throwPos2 * ((1.f - trace.fraction) * Source::m_pGlobalVars->interval_per_tick));

			// TODO: do checks for smoke & molotov/incendiary
			// NOTE: the molotov and incendiary grenade only bounce of walls
			if (itemIndex == weapon_molotov
				|| itemIndex == weapon_incgrenade)
			{
				if (trace.plane.normal.z >= cos(DEG2RAD(weapon_molotov_maxdetonateslope->GetFloat())))
				{
					ignore_entities.clear();
					return;
				}
			}

			// NOTE: the tactical awareness grenade sticks to surfaces
			if (itemIndex == weapon_tagrenade)
			{
				ignore_entities.clear();
				return;
			}

			Ray_t ray3; ray3.Init(endPos, end, Vector(-2.f, -2.f, -2.f), Vector(2.f, 2.f, 2.f));

			Source::m_pEngineTrace->TraceRay(ray3, 0x200400B, &filter, &trace);

			DrawLine(endPos, end, 255, 255, 255);

			endPos = trace.endpos;
			throwPos = throwPos2;
		}
	}
}

//void c_visuals::draw_capsule(Vector Min, Vector Max, float Radius, matrix3x4_t& Transform, Color color)
//{
//	//1 ряд образует шапки над плоскостями цилиндра, содержит все нужные поинты
//	//2 ряд образует цилиндр(бесполезно для мультипоинтов)
//	//3 ряд образует контакты верхней плоскости цилиндра с нижней | 0 и 2 верхние боковые грани, 4 задняя верхняя могут быть использованы как альтернатива 1 ряду
//
//
//	Vector Forward = (Max - Min);
//
//	Forward.Normalize();
//
//	Forward *= Radius;
//
//
//	QAngle Ref;
//	Vector Up;
//	Vector Right;
//	Vector Forward2;
//	Vector P1, P2;
//
//	if (!Forward.LengthSquared()) Forward.x += 0.0001f;
//
//	Math::VectorAngles(Forward, Ref);
//
//
//	Math::AngleVectors(Ref, &Forward2, &Right, &Up);
//
//
//	Up *= Radius;
//	Right *= Radius;
//
//	//float ang_step = (2.0f * M_PI) / 16.0f; //original step(rip fps)
//	float ang_step = (2.0f * M_PI) / 6.0f;
//
//	for (float Angle = 0; Angle < M_PI; Angle += ang_step)
//	{
//		Vector Temp[8];
//		Vector Temp2[8];
//		Temp[0] = Max + Right * cosf(Angle) + Forward * sinf(Angle);
//		Temp[1] = Max + Right * cosf(Angle + ang_step) + Forward * sinf(Angle + ang_step);
//		Temp[2] = Max + Up * cosf(Angle) + Forward * sinf(Angle);
//		Temp[3] = Max + Up * cosf(Angle + ang_step) + Forward * sinf(Angle + ang_step);
//		Temp[4] = Min + Right * cosf(Angle) - Forward * sinf(Angle);
//		Temp[5] = Min + Right * cosf(Angle + ang_step) - Forward * sinf(Angle + ang_step);
//		Temp[6] = Min + Up * cosf(Angle) - Forward * sinf(Angle);
//		Temp[7] = Min + Up * cosf(Angle + ang_step) - Forward * sinf(Angle + ang_step);
//
//
//
//		for (int i = 0; i < 8; i++)
//			Math::VectorTransform(Temp[i], Transform, Temp2[i]);
//
//
//		for (int i = 0; i < 8; i += 2)
//		{
//			if (Drawing::WorldToScreen(Temp2[i], P1) && Drawing::WorldToScreen(Temp2[i + 1], P2))
//				Drawing::DrawLine(P1.x,P1.y, P2.x,P2.y, color);
//
//		}
//
//
//
//	}
//
//
//	for (float Angle = 0; Angle < 2.0f * M_PI; Angle += ang_step)
//	{
//		Vector Temp[4];
//		Vector Temp2[4];
//		Temp[0] = Max + Right * cosf(Angle) + Up * sinf(Angle);
//		Temp[1] = Max + Right * cosf(Angle + ang_step) + Up * sinf(Angle + ang_step);
//		Temp[2] = Min + Right * cosf(Angle) + Up * sinf(Angle);
//		Temp[3] = Min + Right * cosf(Angle + ang_step) + Up * sinf(Angle + ang_step);
//
//		for (int i = 0; i < 4; i++)
//			Math::VectorTransform(Temp[i], Transform, Temp2[i]);
//
//		for (int i = 0; i < 4; i += 2)
//		{
//
//			if (Drawing::WorldToScreen(Temp2[i], P1) && Drawing::WorldToScreen(Temp2[i + 1], P2))
//				Drawing::DrawLine(P1.x, P1.y, P2.x, P2.y, color);
//
//		}
//
//
//
//
//	}
//
//	Vector Temp[8];
//	Vector Temp2[8];
//	Temp[0] = Max + Up;
//	Temp[1] = Min + Up;
//	Temp[2] = Max - Up;
//	Temp[3] = Min - Up;
//	Temp[4] = Max + Right;
//	Temp[5] = Min + Right;
//	Temp[6] = Max - Right;
//	Temp[7] = Min - Right;
//
//	for (int i = 0; i < 8; i++)
//		Math::VectorTransform(Temp[i], Transform, Temp2[i]);
//
//	for (int i = 0; i < 8; i += 2)
//	{
//		if (Drawing::WorldToScreen(Temp2[i], P1) && Drawing::WorldToScreen(Temp2[i + 1], P2))
//			Drawing::DrawLine(P1.x, P1.y, P2.x, P2.y, color);
//	}
//
//	/*
//	if (WorldToScreen(Temp2[0], Temp2[0])
//	&& WorldToScreen(Temp2[1], Temp2[1])
//	&& WorldToScreen(Temp2[2], Temp2[2])
//	&& WorldToScreen(Temp2[3], Temp2[3])
//	&& WorldToScreen(Temp2[4], Temp2[4])
//	&& WorldToScreen(Temp2[5], Temp2[5])
//	&& WorldToScreen(Temp2[6], Temp2[6])
//	&& WorldToScreen(Temp2[7], Temp2[7]))
//	{
//
//	DrawText(Temp2[0].x, Temp2[0].y, D3DWHITE, "0");
//	DrawText(Temp2[1].x, Temp2[1].y, D3DWHITE, "1");
//	DrawText(Temp2[2].x, Temp2[2].y, D3DWHITE, "2");
//	DrawText(Temp2[3].x, Temp2[3].y, D3DWHITE, "3");
//	DrawText(Temp2[4].x, Temp2[4].y, D3DWHITE, "4");
//	DrawText(Temp2[5].x, Temp2[5].y, D3DWHITE, "5");
//	DrawText(Temp2[6].x, Temp2[6].y, D3DWHITE, "6");
//	DrawText(Temp2[7].x, Temp2[7].y, D3DWHITE, "7");
//
//
//	}
//	*/
//
//
//
//}

void c_visuals::render(bool reset)
{
	int indicators = 0;

	if (!m_entities.empty())
		m_entities.clear();

	if (!m_worldentities.empty())
		m_worldentities.clear();
	//if (!m_esp_info.empty())
	//	m_esp_info.clear();

	/*if (cheat::Cvars.RageBot_Hitboxes.Has(0))
		Drawing::DrawString(F::OldESP, 10, 10, Color(255, 255, 255, 250), FONT_LEFT, "head");

	if (cheat::Cvars.RageBot_Hitboxes.Has(1))
		Drawing::DrawString(F::OldESP, 10, 25, Color(255, 255, 255, 250), FONT_LEFT, "body");

	if (cheat::Cvars.RageBot_Hitboxes.Has(2))
		Drawing::DrawString(F::OldESP, 10, 40, Color(255, 255, 255, 250), FONT_LEFT, "arms");*/

	if (cheat::Cvars.Visuals_misc_event_log.GetValue())
		logs();

	auto clean_classid_name = [](std::string _name) -> std::string
	{
		std::string name = _name;
		// Tidy up the weapon Name
		if (name[0] == 'C')
			name.erase(name.begin());

		// Remove the word Weapon
		auto start_str = name.find("Weapon");
		if (start_str != std::string::npos)
			name.erase(name.begin() + start_str, name.begin() + start_str + 6);

		std::transform(name.begin(), name.end(), name.begin(), tolower);

		return name;
	};

	auto get_name_by_paint_kit = [](int wpn, int id) -> const char*
	{
		if (!parser::weapons.list[wpn].skins.list.size())
			return "";

		auto weapon = -1;

		for (auto i = 0; i < parser::weapons.list.size(); i++) {
			auto & cwpn = parser::weapons.list[i];
			if (cwpn.id == wpn) {
				weapon = i;
			}
		}

		if (weapon == -1)
			return "";

		for (auto i = 0; i < parser::weapons.list[weapon].skins.list.size(); i++) {
			auto & skin = parser::weapons.list[weapon].skins.list[i];

			if (skin.id == id) {

				char name[128] = " | ";
				sprintf(name, "%s", skin.translated_name.c_str());

				return name;
			}
		}

		return "";
	};

	auto draw_bind = [](int centerX, int centerY, bool reset) -> void
	{
		static bool filled[3] = { false,false,false };

		auto left_pos = Vector2D(centerX - 45, centerY), right_pos = Vector2D(centerX + 45, centerY), down_pos = Vector2D(centerX, centerY + 45), siz = Vector2D(8, 8);

		static std::vector< Vertex_t > vertices_left =
		{
			Vertex_t{ Vector2D(left_pos.x - siz.x, left_pos.y + siz.y), Vector2D() },
			Vertex_t{ Vector2D(left_pos.x, left_pos.y - siz.y), Vector2D() },
			Vertex_t{ left_pos + siz, Vector2D() }
		};

		static std::vector< Vertex_t > vertices_right =
		{
			Vertex_t{ Vector2D(right_pos.x - siz.x, right_pos.y + siz.y), Vector2D() },
			Vertex_t{ Vector2D(right_pos.x, right_pos.y - siz.y), Vector2D() },
			Vertex_t{ right_pos + siz, Vector2D() }
		};

		static std::vector< Vertex_t > vertices_down =
		{
			Vertex_t{ Vector2D(down_pos.x - siz.x, down_pos.y + siz.y), Vector2D() },
			Vertex_t{ Vector2D(down_pos.x, down_pos.y - siz.y), Vector2D() },
			Vertex_t{ down_pos + siz, Vector2D() }
		};

		if (!filled[0]) {
			for (unsigned int p = 0; p < vertices_left.size(); p++) {
				Drawing::rotate_point(vertices_left[p].m_Position, left_pos, false, 3.15f);
			}
			filled[0] = true;
		}

		if (!filled[1]) {
			for (unsigned int p = 0; p < vertices_right.size(); p++) {
				Drawing::rotate_point(vertices_right[p].m_Position, right_pos, false, 0);
			}
			filled[1] = true;
		}

		if (!filled[2]) {
			for (unsigned int p = 0; p < vertices_down.size(); p++) {
				Drawing::rotate_point(vertices_down[p].m_Position, down_pos, false, 61.25f);
			}
			filled[2] = true;
		}

		Drawing::TexturedPolygon(vertices_left.size(), vertices_left, (cheat::main::side == 0 ? Color(50, 122, 239, 200) : Color::White(150)));
		Drawing::TexturedPolygon(vertices_right.size(), vertices_right, (cheat::main::side == 1 ? Color(50, 122, 239, 200) : Color::White(150)));
		Drawing::TexturedPolygon(vertices_down.size(), vertices_down, (cheat::main::side == 2 ? Color(50, 122, 239, 200) : Color::White(150)));
	};

	auto get_spectators = []() -> std::list<int>
	{
		std::list<int> list = {};

		if (!Source::m_pEngine->IsInGame() || !cheat::main::local() || cheat::main::local()->IsDead())
			return list;

		for (int i = 1; i < Source::m_pGlobalVars->maxClients; i++)
		{
			auto ent = Source::m_pEntList->GetClientEntity(i);

			if (!ent)
				continue;

			if (ent->IsDormant() || !ent->GetClientClass())
				continue;

			if (ent->m_hObserverTarget() == INVALID_HANDLE_VALUE)
				continue;

			auto target = Source::m_pEntList->GetClientEntityFromHandle(*ent->m_hObserverTarget());

			if (!target || cheat::main::local() != target)
				continue;

			list.push_back(i);
		}

		return list;
	};

	int screenW, screenH;
	Source::m_pEngine->GetScreenSize(screenW, screenH);

	Drawing::DrawString(Drawing::Eagle, screenW - 20, 5, { 255, 255, 255, 230 }, FONT_LEFT, "r");

	if (cheat::Cvars.Visuals_spectators.GetValue())
	{
		static float prev_alpha = 0.f;

		auto spectators = 1;

		static auto d = Drawing::GetTextSize(F::NewESP, "H");

		auto lol = get_spectators();

		Vector2D spectator_size = { 150.f, lol.size() * d.bottom + 15.f };

		if ((cheat::settings.radar_pos + spectator_size) > cheat::game::screen_size)
		{
			if ((cheat::settings.radar_pos.x + spectator_size.x) >= cheat::game::screen_size.x)
				cheat::settings.radar_pos.x = cheat::game::screen_size.x - 150.f;

			if ((cheat::settings.radar_pos.y + spectator_size.y) >= cheat::game::screen_size.y)
				cheat::settings.radar_pos.y = cheat::game::screen_size.y - (lol.size() * d.bottom + 15.f);
		}
		else if (cheat::settings.radar_pos < Vector2D(0, 0)) {
			if (cheat::settings.radar_pos.x < 0.f)
				cheat::settings.radar_pos.x = 0.f;

			if (cheat::settings.radar_pos.y < 0.f)
				cheat::settings.radar_pos.y = 0.f;
		}

		if (cheat::features::menu.menu_opened)
			prev_alpha = 255.f;

		Drawing::DrawRect(cheat::settings.radar_pos.x, cheat::settings.radar_pos.y, spectator_size.x, 15.f, Color(31.f, 33.f, 35.f, prev_alpha * float(cheat::Cvars.Visuals_spectators_alpha.GetValue() / 100.f))); // 31.f, 33.f, 35.f
		Drawing::DrawString(F::ESPInfo, cheat::settings.radar_pos.x + 18.f, cheat::settings.radar_pos.y + 3.f, Color(255.f, 255.f, 255.f, prev_alpha * float(cheat::Cvars.Visuals_spectators_alpha.GetValue() / 80.f)), FONT_LEFT, "spectators");
		Drawing::DrawString(Drawing::Eagle, cheat::settings.radar_pos.x, cheat::settings.radar_pos.y, Color( 255.f, 255.f, 255.f, prev_alpha * float(cheat::Cvars.Visuals_spectators_alpha.GetValue() / 80.f) ), FONT_LEFT, "r");
		Drawing::DrawRect(cheat::settings.radar_pos.x, cheat::settings.radar_pos.y + 15.f, spectator_size.x, spectator_size.y - 13.f, Color(43.f, 44.f, 46.f, prev_alpha * float(cheat::Cvars.Visuals_spectators_alpha.GetValue() / 100.f)));

		if (lol.size() > 0)
		{
			if (prev_alpha < 255.f)
				prev_alpha += min(5.f, 255.f - prev_alpha);

			for (int spec : lol)
			{
				player_info spec_inf;
				Source::m_pEngine->GetPlayerInfo(spec, &spec_inf);

				auto size = Drawing::GetTextSize(F::NewESP, spec_inf.name);
				//Drawing::DrawRect(cheat::settings.radar_pos.x, cheat::settings.radar_pos.y + d.bottom * spectators, size.right + 2, 13, Color(0.f, 0.f, 0.f, prev_alpha * float(cheat::Cvars.Visuals_spectators_alpha.GetValue() / 100.f)));

				if (spectators> 1)
					Drawing::DrawLine(cheat::settings.radar_pos.x, cheat::settings.radar_pos.y + 1.f + d.bottom * spectators, cheat::settings.radar_pos.x + 149.f, cheat::settings.radar_pos.y + 1.f + d.bottom * spectators, Color(80.f, 80.f, 80.f, prev_alpha * float(cheat::Cvars.Visuals_spectators_alpha.GetValue() / 80.f)));

				Drawing::DrawString(F::NewESP, cheat::settings.radar_pos.x + 2.f, cheat::settings.radar_pos.y + 2.f + d.bottom * spectators, Color(255, 255, 255, prev_alpha), FONT_LEFT, u8"%s", spec_inf.name);

				spectators++;
			}
		}
		else
		{
			if (prev_alpha > 0.f && !cheat::features::menu.menu_opened)
				prev_alpha -= min(5.f, prev_alpha);
		}

		if (cheat::features::menu.menu_opened)
		{
			if (cheat::game::pressed_keys[1] && cheat::features::menu.mouse_in_pos(Vector(cheat::settings.radar_pos.x, cheat::settings.radar_pos.y, 0), Vector(cheat::settings.radar_pos.x + spectator_size.x, cheat::settings.radar_pos.y + 15.f, 0)) || was_moved)
			{
				if (save_pos == false)
				{
					saved_x = cheat::features::menu._cursor_position.x - cheat::settings.radar_pos.x;
					saved_y = cheat::features::menu._cursor_position.y - cheat::settings.radar_pos.y;
					save_pos = true;
				}
				cheat::settings.radar_pos.x = cheat::features::menu._cursor_position.x;
				cheat::settings.radar_pos.y = cheat::features::menu._cursor_position.y;
				cheat::settings.radar_pos.x = cheat::settings.radar_pos.x - saved_x;
				cheat::settings.radar_pos.y = cheat::settings.radar_pos.y - saved_y;
			}
			else
				save_pos = was_moved = false;

			if (!was_moved)
				was_moved = cheat::game::pressed_keys[1] && cheat::features::menu.mouse_in_pos(Vector(cheat::settings.radar_pos.x, cheat::settings.radar_pos.y, 0), Vector(cheat::settings.radar_pos.x + spectator_size.x, cheat::settings.radar_pos.y + 15.f, 0));
			else
				was_moved = cheat::game::pressed_keys[1];
		}
		else
			was_moved = false;
	}

	if (!cheat::main::local() || !Source::m_pEngine->IsInGame()) return;

	static auto linegoesthrusmoke = Memory::Scan(cheat::main::clientdll, "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
	static auto smokecout = *(DWORD*)(linegoesthrusmoke + 0x8);

	if (cheat::Cvars.Visuals_rem_smoke.GetValue())
		*(int*)(smokecout) = 0;

	auto ind_h = screenH / 2 + 10;

	auto local_weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon()));

	if (local_weapon)
	{
		render_tracers();

		if (cheat::Cvars.Visuals_misc_nade_tracer.GetValue())
			GrenadePrediction(local_weapon);

		auto accuracy = 0;

		int centerX = screenW / 2, centerY = screenH / 2;

		if (!(cheat::convars::sv_usercmd_custom_random_seed == 0 || cheat::convars::weapon_accuracy_nospread == 1)) {
			//accuracy = (local_weapon->GetInaccuracy() + local_weapon->GetSpread()) * 500.f;
			float spreadDist = ((local_weapon->GetInaccuracy() + local_weapon->GetSpread()) * 320.f) / std::tan(DEG2RAD(cheat::main::fov) * 0.5f);
			accuracy = spreadDist * (screenH / 480.f);
		}
		else
			accuracy = 0;

		if (cheat::main::local()->m_bIsScoped() && cheat::Cvars.Visuals_rem_scope.GetValue()) {

			//if (cheat::Cvars.Visuals_misc_wpn_spread.GetValue() != 2)
			//	accuracy = 0;

			Drawing::DrawLine(0, centerY, centerX - accuracy + 1, centerY, Color::Black());
			Drawing::DrawLine(centerX + accuracy - 1, centerY, screenW, centerY, Color::Black());

			Drawing::DrawLine(centerX, 0, centerX, centerY - accuracy + 1, Color::Black());
			Drawing::DrawLine(centerX, centerY + accuracy - 1, centerX, screenH, Color::Black());

			Drawing::DrawPixel(centerX, centerY, Color::White());
		}
		if (cheat::Cvars.Visuals_misc_wpn_spread.GetValue() == 1)
			Drawing::DrawFilledCircle(centerX + 0.5f, centerY + 0.5f, accuracy, 50, cheat::Cvars.Visuals_spread_crosshair_color.GetColor().alpha(30));
		else if (cheat::Cvars.Visuals_misc_wpn_spread.GetValue() == 2)
			Drawing::DrawRect(centerX - accuracy, centerY - accuracy, accuracy * 2.f, accuracy * 2.f, cheat::Cvars.Visuals_spread_crosshair_color.GetColor().alpha(30));
	}

	if (local_weapon && !cheat::main::local()->IsDead() && (DWORD)cheat::main::local()->get_animation_state() > 0x5000)
	{
		//matrix3x4_t noob[128];
		//cheat::main::local()->SetupFuckingBones(noob, 128, 256, 0);
		//skeleton(cheat::main::local(), Color::White(200), noob);

		static bool was_breaking = false;

		auto is_breaking_lagcomp = (cheat::main::local()->m_vecOrigin() - cheat::features::antiaimbot.last_sent_origin).Length2D() >= 64;

		if (cheat::Cvars.Visuals_misc_desync_indicator.GetValue())
		{
			static auto lbysize = Drawing::GetTextSize(F::LBY, "LBY");

			Drawing::DrawString(F::LBY, 10, screenH - 88 - 26 * indicators, Color(30, 30, 30, 250), FONT_LEFT, "LBY");

			auto factor = (1.f - ((cheat::features::antiaimbot.m_next_lby_update_time - Source::m_pGlobalVars->curtime) / ((cheat::main::local()->get_animation_state()->speed_2d > 0.1f && !cheat::main::fakewalking) ? 0.22f : 1.1f)));

			factor = Math::clamp(factor, 0.f, 1.f);

			if (cheat::main::local()->get_animation_state()->speed_2d > 0.1f && !cheat::main::fakewalking)
				factor = 1.f;

			*(bool*)((DWORD)Source::m_pSurface + 0x280) = true;
			int x, y, x1, y1;
			Drawing::GetDrawingArea(x, y, x1, y1);
			Drawing::LimitDrawingArea(0, screenH - 88 - 26 * indicators, int((lbysize.right + 15) * factor), (int)lbysize.bottom);

			const auto lby_broken = fabs(Math::normalize_angle(cheat::main::local()->get_animation_state()->abs_yaw - cheat::main::local()->m_flLowerBodyYawTarget())) >= 35.f;

			//if (cheat::main::local()->get_animation_state()->speed_2d < 0.1f || cheat::main::fakewalking)
			Drawing::DrawString(F::LBY, 10, screenH - 88 - 26 * indicators++, lby_broken ? Color(131, 198, 4, 250) : Color::Red(250), FONT_LEFT, "LBY");

			Drawing::LimitDrawingArea(x, y, x1, y1);
			*(bool*)((DWORD)Source::m_pSurface + 0x280) = false;
		}

		if (cheat::Cvars.Visuals_misc_flag_indicator.GetValue()) 
		{
			if (!is_breaking_lagcomp)
				is_breaking_lagcomp = !(cheat::main::local()->m_fFlags() & FL_ONGROUND) && was_breaking && cheat::main::local()->m_vecVelocity().Length2D() > 270;

			if (cheat::main::local()->m_vecVelocity().Length2D() > 250)
				Drawing::DrawString(F::LBY, 10, screenH - 88 - 26 * indicators++, is_breaking_lagcomp ? Color(131, 198, 4, 250) : Color::Red(250), FONT_LEFT, "LC");
		}

		was_breaking = is_breaking_lagcomp;

		//Drawing::DrawString(F::NewESP, 60, ind_h + 12 + 20 * indicators + 20, Color(255, 255, 255, 240), FONT_CENTER, "WST: %.5f", local_weapon->m_flLastShotTime());

		//Drawing::DrawString(F::NewESP, 60, ind_h + 12 + 20 * indicators + 20, Color(255, 255, 255, 240), FONT_CENTER, "WST: %d", cheat::main::local()->m_nTickBase());

		int centerX = screenW / 2, centerY = screenH / 2;

		if (cheat::Cvars.Visuals_misc_manual_indicator.GetValue())
			draw_bind(centerX, centerY, reset);

		static int linesize = 5, linedec = 11;
		static int alpha = 0;

		if (cheat::main::hit_time > Source::m_pGlobalVars->realtime)
			alpha = 255;
		else
			alpha = alpha - (255 / 0.3 * Source::m_pGlobalVars->frametime);

		if (alpha > 0 && cheat::Cvars.Visuals_misc_hitmarker.GetValue()) {
			Drawing::DrawLine(screenW / 2.f - linesize, screenH / 2.f - linesize, screenW / 2.f - linedec, screenH / 2.f - linedec, Color(200, 200, 200, alpha));
			Drawing::DrawLine(screenW / 2.f - linesize, screenH / 2.f + linesize, screenW / 2.f - linedec, screenH / 2.f + linedec, Color(200, 200, 200, alpha));
			Drawing::DrawLine(screenW / 2.f + linesize, screenH / 2.f + linesize, screenW / 2.f + linedec, screenH / 2.f + linedec, Color(200, 200, 200, alpha));
			Drawing::DrawLine(screenW / 2.f + linesize, screenH / 2.f - linesize, screenW / 2.f + linedec, screenH / 2.f - linedec, Color(200, 200, 200, alpha));
		}

		//if (local_weapon && local_weapon->m_iItemDefinitionIndex() == weapon_taser)
		//{
		//	Vector prev_scr_pos, scr_pos;

		//	static float step = M_PI * 2.0 / 2047; //adjust if you need 1-5 extra fps lol
		//	float rad = local_weapon->GetCSWeaponData()->range;
		//	Vector origin = cheat::main::local()->GetEyePosition();

		//	static int rainbow = 0;

		//	Ray_t ray;
		//	trace_t trace;
		//	CTraceFilter filter;

		//	filter.pSkip = cheat::main::local();
		//	static float hue_offset = 0;
		//	for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
		//	{
		//		Vector pos(rad * cos(rotation) + origin.x, rad * sin(rotation) + origin.y, origin.z);

		//		ray.Init(origin, pos);

		//		Source::m_pEngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace);

		//		if (Drawing::WorldToScreen(trace.endpos, scr_pos))
		//		{
		//			if (!prev_scr_pos.IsZero())
		//			{
		//				int hue = RAD2DEG(rotation) + hue_offset;
		//				Color temp = Color().FromHSV(hue / 360.f, 1.f, 1.f);

		//				Source::m_pSurface->DrawSetColor(temp);
		//				Source::m_pSurface->DrawLine(prev_scr_pos.x, prev_scr_pos.y, scr_pos.x, scr_pos.y);
		//			}
		//			prev_scr_pos = scr_pos;
		//		}
		//	}
		//	hue_offset += 0.25;
		//}

		/*if (Vector dummy = { 0,0,0 }; Drawing::WorldToScreen(cheat::main::local()->m_vecOrigin(), dummy)) {
			QAngle ang = QAngle(0, 0, 0);
			Source::m_pPrediction->GetLocalViewAngles(ang);

			Drawing::DrawString(F::ESP, dummy.x, dummy.y, { 255, 0, 0, 240 }, FONT_CENTER, "%.0f", cheat::main::local()->m_angEyeAngles().y);
			Drawing::DrawString(F::ESP, dummy.x, dummy.y + 14, { 0, 255, 0, 240 }, FONT_CENTER, "%.0f", ang.y);
		}*/

		/*for (auto i = 0; i < cheat::main::local()->get_animation_layers_count(); i++)
		{
			auto layer = &cheat::main::local()->get_animation_layer(i);
			Drawing::DrawString(F::OldESP, 10, 20 + 12 * i, Color::White(200), FONT_LEFT, "%d weight: %.2f", i, layer->m_flWeight);
			Drawing::DrawString(F::OldESP, 110, 20 + 12 * i, Color::White(200), FONT_LEFT, "%d playback: %.6f", i, layer->m_flPlaybackRate);
			Drawing::DrawString(F::OldESP, 230, 20 + 12 * i, Color::White(200), FONT_LEFT, "%d cycle: %.6f", i, layer->m_flCycle);
			Drawing::DrawString(F::OldESP, 330, 20 + 12 * i, Color::White(200), FONT_LEFT, "%d sequence: %d", i, layer->m_nSequence);
			Drawing::DrawString(F::OldESP, 430, 20 + 12 * i, Color::White(200), FONT_LEFT, "%d layer animtime: %.6f", i, layer->m_flLayerAnimtime);
		}*/

		//if (g_Options.esp.angle_lines) {
			/*auto drawAngleLine = [&](const Vector& origin, const Vector& w2sOrigin, const float& angle, const char* text, Color clr) {
				Vector forward = QAngle(0.0f, angle, 0.0f).ToVectors();

				float AngleLinesLength = 30.0f;

				Vector w2sReal;
				if (Drawing::WorldToScreen(origin + forward * AngleLinesLength, w2sReal)) {
					Drawing::DrawLine( w2sOrigin.x, w2sOrigin.y, w2sReal.x, w2sReal.y, clr);
					Drawing::DrawRect( w2sReal.x - 5.0f, w2sReal.y - 5.0f , 10.0f, 10.0f, Color(50, 50, 50, 150));
					Drawing::DrawString(F::OldESP, w2sReal.x, w2sReal.y , clr, FONT_CENTER, text);
				}
			};

			Vector w2sOrigin;
			if (Drawing::WorldToScreen(cheat::main::local()->m_vecOrigin(), w2sOrigin)) {
				drawAngleLine(cheat::main::local()->m_vecOrigin(), w2sOrigin, cheat::game::last_cmd->viewangles.y, "viewangles", Color(0.937f * 255.f, 0.713f* 255.f, 0.094f* 255.f, 1.0f* 255.f));
				drawAngleLine(cheat::main::local()->m_vecOrigin(), w2sOrigin, cheat::main::local()->m_flLowerBodyYawTarget(), "lby", Color(0.0f* 255.f* 255.f, 0.0f* 255.f, 1.0f* 255.f, 1.0f* 255.f));
				drawAngleLine(cheat::main::local()->m_vecOrigin(), w2sOrigin, cheat::main::local()->m_angEyeAngles().y, "angles", Color(0.0f* 255.f, 1.0f* 255.f, 0.0f* 255.f, 1.0f* 255.f));
				drawAngleLine(cheat::main::local()->m_vecOrigin(), w2sOrigin, cheat::main::local()->get_animation_state()->abs_yaw, "abs yaw", Color(1.0f* 255.f, 0.0f* 255.f, 0.0f* 255.f, 1.0f* 255.f));
			}*/
			//}

			//auto poses = cheat::main::local()->m_flPoseParameter();

			//for (int i = 0; i < 24; ++i)
			//	Drawing::DrawString(F::ESP, 10, 20 + 12 * i, Color::White(200), FONT_LEFT, " %d pose: %.6f", i, poses[i]);

			//{
			//	C_Simulationdata simulation_data;

			//	simulation_data.entity = cheat::main::local();
			//	/*auto origin = */simulation_data.origin = cheat::main::local()->m_vecOrigin();
			//	//auto abs_origin = origin;
			//	simulation_data.velocity = cheat::main::local()->m_vecVelocity();
			//	simulation_data.on_ground = cheat::main::local()->m_fFlags() & FL_ONGROUND;
			//	simulation_data.data_filled = true;
			//	simulation_data.extrapolation = true;

			//	// Calculate Delta's
			//	//auto simulation_time_delta = 15//current_record->simulation_time - next_record->simulation_time;
			//	auto simulation_tick_delta = 15;//std::clamp(TIME_TO_TICKS(simulation_time_delta), 1, 15);
			//	//auto delta_ticks = (std::clamp(TIME_TO_TICKS(Source::m_pEngine->GetNetChannelInfo()->GetAvgLatency(1) + Source::m_pEngine->GetNetChannelInfo()->GetAvgLatency(0)) + Source::m_pGlobalVars->tickcount - TIME_TO_TICKS(cheat::main::local()->m_flSimulationTime() + cheat::main::lerp_time), 0, 100)) - simulation_tick_delta;

			//	// Calculate movement delta
			//	//auto current_velocity_angle = RAD2DEG(atan2(entity->m_vecVelocity().y, entity->m_vecVelocity().x));
			//	//auto next_velocity_angle = RAD2DEG(atan2(next_record->velocity.y, next_record->velocity.x));
			//	//auto velocity_angle_delta = Math::NormalizeYaw(current_velocity_angle - next_velocity_angle);
			//	//auto velocity_movement_delta = velocity_angle_delta / TICKS_TO_TIME(simulation_tick_delta);

			//	if (cheat::main::local()->m_vecVelocity().Length() > 0 && simulation_data.data_filled)
			//	{
			//		auto ticks_left = simulation_tick_delta;

			//		do
			//		{
			//			cheat::features::lagcomp.simulate_movement(&simulation_data);
			//			//current_record->simulation_time += Source::m_pGlobalVars->interval_per_tick;

			//			Vector dummy;

			//			if (Drawing::WorldToScreen(simulation_data.origin, dummy))
			//				Drawing::DrawString(F::ESP, dummy.x, dummy.y, Color::White(255.f), FONT_LEFT, "O");

			//			--ticks_left;
			//		} while (ticks_left > 0);

			//		//origin = simulation_data.origin;
			//		//abs_origin = simulation_data.origin;
			//	}
			//}

			//C_Tickrecord back_record;
			//cheat::features::lagcomp.store_record_data(cheat::main::local(), &back_record);

			/*C_Tickrecord back_record;
			cheat::features::lagcomp.store_record_data(cheat::main::local(), &back_record);

			for (auto i = 0; i < 40; i++) {
				Source::m_pPrediction->RunCommand(cheat::main::local(), cheat::game::last_cmd, Source::m_pMoveHelper);

				if (Vector dummy; Drawing::WorldToScreen(cheat::main::local()->m_vecOrigin(), dummy))
					Drawing::DrawString(F::ESP, dummy.x, dummy.y, Color::White(255.f), FONT_LEFT, "sim_pos %d", i);
			}

			cheat::features::lagcomp.apply_record_data(cheat::main::local(), &back_record, true);*/

			//if (Vector dummy; !cheat::main::local()->m_vecOrigin().IsZero() && Drawing::WorldToScreen(cheat::main::local()->m_vecOrigin(), dummy))
				//Drawing::DrawString(F::ESP, dummy.x, dummy.y, Color::White(255.f), FONT_LEFT, "abs yaw: %.1f | networked yaw: %.1f", cheat::main::local()->get_abs_eye_angles().y, cheat::main::local()->m_angEyeAngles().y);

			//auto bone_pos = cheat::features::antiaimbot.last_sent_origin;
			//auto velocity = cheat::main::local()->m_vecVelocity();
			//float ipt = Source::m_pGlobalVars->interval_per_tick;
			//auto origin = bone_pos + velocity * ipt;

			//for (int simulationTickDelta = 1; simulationTickDelta < Source::m_pClientState->chokedcommands; simulationTickDelta++) {

			//	if (simulationTickDelta >= Source::m_pClientState->chokedcommands)
			//		break;
			//	//RebuildGameMovement::Get().FullWalkMove(cheat::main::local());

			//	origin += velocity * ipt;

			//	if (Drawing::WorldToScreen(origin, dummy))
			//		Drawing::DrawString(F::ESP, dummy.x, dummy.y, Color::Red(255.f), FONT_LEFT, "simulated_pos %d", simulationTickDelta);
			//}

			//cheat::features::lagcomp.apply_record_data(cheat::main::local(), &back_record);
	}

	if (!cheat::main::local() || cheat::main::local()->IsDead() || !local_weapon) {
		if (cheat::Cvars.Visuals_misc_crosshair.GetValue()) {
			Drawing::DrawLine(screenW / 2.f - 10, screenH / 2.f, screenW / 2.f + 10, screenH / 2.f, Color::Black());
			Drawing::DrawLine(screenW / 2.f, screenH / 2.f - 10, screenW / 2.f, screenH / 2.f + 10, Color::Black());
		}
	}
	else
	{
		auto weapon_info = local_weapon->GetCSWeaponData();
		if (!weapon_info)
			return;

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

		float autowall_dmg = (float)weapon_info->damage;
		static float previous_damage = -1.f;
		static bool disabled_due_nade = false;

		if (IsGrenade(local_weapon->m_iItemDefinitionIndex()))
		{
			if (cheat::main::thirdperson &&  cheat::Cvars.Visuals_lp_forcetpnade.GetValue()) {
				cheat::main::thirdperson = false;
				disabled_due_nade = true;
			}
		}
		else {
			if (disabled_due_nade && !cheat::main::thirdperson)
			{
				cheat::main::thirdperson = true;
				disabled_due_nade = false;
			}
			if (cheat::Cvars.Visuals_misc_crosshair.GetValue() >= 2 && local_weapon->IsGun())
			{
				Vector direction;
				Math::AngleVectors(Engine::Movement::Instance()->m_qRealAngles, &direction);

				Vector start = cheat::main::local()->GetEyePosition();

				auto max_range = weapon_info->range * 2;

				Vector end = start + (direction * max_range);
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
					autowall_dmg = -1.f;

				if (enterTrace.m_pEnt != nullptr)
				{
					//This looks gay as fuck if we put it into 1 long line of code.
					bool canDoDamage = (enterTrace.hitgroup != HITGROUP_GEAR && enterTrace.hitgroup != HITGROUP_GENERIC);
					bool isPlayer = (enterTrace.m_pEnt->GetClientClass() && enterTrace.m_pEnt->GetClientClass()->m_ClassID == 35);
					bool isEnemy = (cheat::main::local()->m_iTeamNum() != ((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum());
					bool onTeam = (((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum() == 2 || ((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum() == 3);

					//TODO: Team check config
					if ((canDoDamage && isPlayer && isEnemy) && onTeam)
						cheat::features::autowall.ScaleDamage(enterTrace, weapon_info, autowall_dmg);

					if (!canDoDamage && isPlayer && isEnemy)
						autowall_dmg = -1.f;
				}

				auto penetrate_count = 4;

				if (!cheat::features::autowall.HandleBulletPenetration(weapon_info, enterTrace, start, direction, penetrate_count, autowall_dmg, weapon_info->penetration, cheat::convars::ff_damage_bullet_penetration))
					autowall_dmg = -1.f;

				if (penetrate_count <= 0)
					autowall_dmg = -1.f;
			}
			else
				autowall_dmg = -1.f;
		}

		if (cheat::Cvars.Visuals_misc_crosshair.GetValue()) {
			if (cheat::Cvars.Visuals_misc_crosshair.GetValue() == 1)
				autowall_dmg = 0.1f;

			auto color = (autowall_dmg > cheat::Cvars.RageBot_MinDamage.GetValue() ? Color(131, 198, 4, 250) : (autowall_dmg > 0.0f ? Color(0, 129, 255, 255) : Color::Red()));

			//Drawing::DrawLine(screenW / 2.f - 10, screenH / 2.f, screenW / 2.f + 10, screenH / 2.f, color);
			//Drawing::DrawLine(screenW / 2.f, screenH / 2.f - 10, screenW / 2.f, screenH / 2.f + 10, color);
			static auto lolk = Drawing::GetTextSize(F::ESPInfo, "+");
			Drawing::DrawString(F::ESPInfo, screenW / 2.f - lolk.left / 2, screenH / 2.f - lolk.right / 2, color, FONT_LEFT, "+");

			if (fabs(previous_damage - autowall_dmg) < 3.f)
				autowall_dmg = previous_damage;

			if (autowall_dmg >= 1.f && cheat::Cvars.Visuals_misc_crosshair.GetValue() == 3) {

				char dmg[10];
				sprintf(dmg, "%.0f", autowall_dmg);

				const auto tsize = Drawing::GetTextSize(F::Menu, dmg);

				Drawing::DrawRect(screenW / 2.f - tsize.right / 2 - 2, screenH / 2.f - 26, tsize.right + 3, tsize.bottom - 2, Color(10,10,10,200));
				Drawing::DrawString(F::Menu, screenW / 2.f, screenH / 2.f - 26, Color::White(), FONT_CENTER, dmg);
			}

			previous_damage = autowall_dmg;
		}
	}

	if (!cheat::Cvars.Visuals_Enable.GetValue())
		return;

	const bool center = (int)cheat::Cvars.Visuals_Box.GetValue() == 0;

	static auto max_bombtime = Source::m_pCvar->FindVar("mp_c4timer");

	for (auto idx = 0; idx < Source::m_pEntList->GetHighestEntityIndex(); idx++)
	{
		auto entity = (C_BasePlayer*)Source::m_pEntList->GetClientEntity(idx);

		if (!entity ||
			!entity->GetClientClass() ||
			((DWORD)entity->GetClientRenderable() < 0x1000))
			continue;

		if (entity->IsPlayer() && idx < 64)
		{
			if (entity->m_iHealth() <= 0 ||
				entity->m_iTeamNum() == cheat::main::local()->m_iTeamNum() && !cheat::Cvars.Visuals_teammates.GetValue())
				continue;

			if (entity->IsDormant() && dormant_alpha[idx - 1] <= 0.f) {
				cheat::features::dormant.draw_sound(entity);
				continue;
			}

			player_info info;

			if (!entity ||
				!entity->IsPlayer() ||
				/*entity->IsDormant() ||*/
				entity->m_iHealth() <= 0 ||
				(entity->m_iTeamNum() == cheat::main::local()->m_iTeamNum() && !cheat::Cvars.Visuals_teammates.GetValue()) ||
				!Source::m_pEngine->GetPlayerInfo(entity->entindex(), &info)
				) continue;

			auto idx = entity->entindex() - 1;

			if (entity->IsDormant()) {
				if (dormant_alpha[idx] <= 0.f)
					continue;
				else
					dormant_alpha[idx] -= min(0.25f, dormant_alpha[idx]);
			}
			else if (dormant_alpha[idx] < 255.f)
				dormant_alpha[idx] += min(10.f, 255 - dormant_alpha[idx]);

			auto player_record = &cheat::features::lagcomp.records[entity->entindex() - 1];

			if (cheat::Cvars.Visuals_fov_arrows.GetValue() && !cheat::main::local()->IsDead())
				draw_pov_arrows(entity, dormant_alpha[idx]);

			int x, y, w, h;
			if (!get_espbox(entity, x, y, w, h)) continue;

			int right = 0;
			int down = 0;
			//static RECT size_info = Drawing::GetTextSize(F::ESPInfo, ".");
			static auto size_info = 9.f;

			Color color = cheat::Cvars.EnemyBoxCol.GetColor();//Color().FromHSB(cheat::settings.esp_color[0], cheat::settings.esp_color[2], 1.f).alpha(cheat::settings.esp_color[1]);
															  //Color snaplinescolor = cheat::Cvars.Visuals_SnaplinesColor.GetColor;/Color().FromHSB(cheat::settings.snap_lines_color[0], cheat::settings.snap_lines_color[2], 1.f).alpha(cheat::settings.snap_lines_color[1] * 255.f);
			Color skeletonscolor = cheat::Cvars.Visuals_skeletonColor.GetColor();//Color().FromHSB(cheat::settings.skeleton_color[0], cheat::settings.skeleton_color[2], 1.f).alpha(cheat::settings.skeleton_color[1] * 255.f);

			if (cheat::Cvars.Visuals_Box.GetValue()) {
				Drawing::DrawOutlinedRect(x, y, w, h, color.alpha(dormant_alpha[idx]));
				Drawing::DrawOutlinedRect(x - 1, y - 1, w + 2, h + 2, Color(10, 10, 10, (dormant_alpha[idx] * 0.8f)));
				Drawing::DrawOutlinedRect(x + 1, y + 1, w - 2, h - 2, Color(10, 10, 10, (dormant_alpha[idx] * 0.8f)));
			}

			if (cheat::Cvars.Visuals_Name.GetValue()) {
				auto text_size = Drawing::GetTextSize(F::NewESP, info.name);

				Drawing::DrawString(F::NewESP, x + w / 2 - text_size.right / 2, y - 13, Color::White(dormant_alpha[idx] - 25.f), FONT_LEFT, u8"%s", info.name);
			}

			if (cheat::Cvars.Visuals_Snaplines.GetValue())
				Drawing::DrawLine(screenW / 2, screenH / 2, x + w / 2, y + h, cheat::Cvars.Visuals_SnaplinesColor.GetColor().alpha(dormant_alpha[idx]));

			if (cheat::Cvars.Visuals_Health.GetValue()) {
				int hp = entity->m_iHealth();

				if (hp > 100)
					hp = 100;

				int hp_percent = h - (int)((h * hp) / 100);

				int width = (w * (hp / 100.f));

				int red = 255 - (hp*2.55);
				int green = hp * 2.55;

				char hps[10] = "";

				sprintf(hps, "%iHP", hp);

				auto text_size = Drawing::GetTextSize(F::ESPInfo, hps);

				Drawing::DrawRect(x - 5, y - 1, 4, h + 2, Color(80, 80, 80, dormant_alpha[idx] * 0.49f));
				Drawing::DrawOutlinedRect(x - 5, y - 1, 4, h + 2, Color(10, 10, 10, (dormant_alpha[idx] * 0.8f)));
				Drawing::DrawRect(x - 4, y + hp_percent, 2, h - hp_percent, Color(red, green, 0, dormant_alpha[idx]));

				if (hp < 90)
					Drawing::DrawString(F::ESPInfo, x - text_size.right - 6, y - 1, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, hps);
				//Drawing::DrawRectGradientHorizontal(x - 3, y + hp_percent, 14, 11, Color(0, 0, 0, 0), Color(7, 39, 17, dormant_alpha[idx] - 35));

				float height = h / 10.f;

				//if (cheat::settings.esp_split_health) {
				//	for (int i = 1; i < 10; i++)
				//		Drawing::DrawLine(x - 5, y + i * height, x - 2, y + i * height, Color::Black(dormant_alpha[idx]));
				//}
			}

			if (entity->m_ArmorValue() > 0.f && cheat::Cvars.Visuals_enemies_flags.Has(0))
				Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, (entity->m_bHasHelmet() ? "HELM" : "KEV"));

			auto resolver_info = cheat::features::aaa.player_resolver_records[entity->entindex() - 1];

			if (resolver_info.fakeangles && cheat::Cvars.Visuals_enemies_flags.Has(2))
				Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(200, 130, 0, dormant_alpha[idx] - 55.f), FONT_LEFT, "FAKE");

			/*if (cheat::Cvars.Visuals_enemies_flags.Has(1) && resolver_info.fakeangles) {
				Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "FAKE");
				Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "R%d", resolver_info.resolving_method);
			}*/

			//if (cheat::Cvars.Visuals_enemies_flags.Has(3) && entity->m_iPing())

			/*Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "1:%s", resolver_info.is_using_static ? "+" : "-");
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "2:%s", resolver_info.is_using_balance ? "+" : "-");
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "3:%.3f", fabs(resolver_info.lby_delta));
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "4:%.3f", fabs(Source::m_pGlobalVars->realtime - resolver_info.last_time_balanced));
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "5:%s", resolver_info.is_jittering ? "+" : "-");
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "6:%.3f", fabs(resolver_info.abs_yaw_delta));
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "7:%.3f", fabs(resolver_info.tick_delta));
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "8:%.3f", fabs(resolver_info.m_flClientRate - resolver_info.m_flRate));
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "C:%.3f", fabs(resolver_info.server_anim_layers[6].m_flCycle - resolver_info.client_anim_layers[6].m_flCycle));
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "W:%.3f", fabs(resolver_info.server_anim_layers[6].m_flWeight - resolver_info.client_anim_layers[6].m_flWeight));
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "PC:%.3f", fabs(resolver_info.server_anim_layers[6].m_flPrevCycle - resolver_info.client_anim_layers[6].m_flPrevCycle));

			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "LCD:%.3f", fabs(resolver_info.last_cycle_desync - Source::m_pGlobalVars->realtime));
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "LT3:%.3f", fabs(resolver_info.last_time_three - Source::m_pGlobalVars->realtime));*/
			//Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "R:%d", resolver_info.freestanding_record.right_damage);
			//Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "L:%d", resolver_info.freestanding_record.left_damage);

			//Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "RF:%.4f", resolver_info.freestanding_record.right_fraction);
			//Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "LF:%.4f", resolver_info.freestanding_record.left_fraction);

			auto acweapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(entity->m_hActiveWeapon()));
			auto weapons = entity->m_hMyWeapons();

			if (acweapon && weapons)
			{
				auto weap_info = acweapon->GetCSWeaponData();

				if (!weap_info)
					continue;

				if (cheat::Cvars.Visuals_ammo_bar.GetValue()) {
					int ammo = acweapon->m_iClip1();

					if (ammo > weap_info->max_clip)
						ammo = weap_info->max_clip;

					int hp_percent = w - (int)((w * ammo) / 100);

					int width = (w * (ammo / float(weap_info->max_clip)));

					char ammostr[10];
					sprintf(ammostr, "%d", ammo);

					const auto text_size = Drawing::GetTextSize(F::ESPInfo, ammostr);

					if (ammo < int(ammo * 0.5f) && ammo > 0)
						Drawing::DrawString(F::ESPInfo, x + 1 - text_size.right, y + h, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, ammostr);

					Drawing::DrawRect(x, y + 2 + h, w, 4, Color(80, 80, 80, dormant_alpha[idx] * 0.49f));
					Drawing::DrawOutlinedRect(x, y + 2 + h, w, 4, Color(10, 10, 10, (dormant_alpha[idx] * 0.8f)));
					Drawing::DrawRect(x + 1, y + 3 + h, width - 2, 2, Color::LightBlue().alpha(dormant_alpha[idx]));

					down++;
				}

				if (cheat::Cvars.Visuals_Weapon.selected > 0) {
					if (cheat::Cvars.Visuals_Weapon.Has(0) && cheat::Cvars.Visuals_Weapon.selected <= 1) {
						char wpn_name[100] = "";

						sprintf(wpn_name, "%s", weap_info->weapon_name + 7);

						if (acweapon->m_iItemDefinitionIndex() == 64)
							strcpy(wpn_name, "revolver");

						if (acweapon->m_nFallbackPaintKit() > 0)
							strcat(wpn_name, get_name_by_paint_kit(acweapon->m_iItemDefinitionIndex(), acweapon->m_nFallbackPaintKit()));

						auto wpn_name_size = Drawing::GetTextSize(F::ESPInfo, wpn_name);

						Drawing::DrawString(F::ESPInfo, x + w / 2 - wpn_name_size.right / 2, y + h + (down++ * 6.f), Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, wpn_name);
					}
					else {
						int weapon_count = 0;
						bool lol = (cheat::Cvars.Visuals_Weapon.selected <= 2 && cheat::Cvars.Visuals_Weapon.Has(4));

						for (int i = 0; weapons[i] != 0xFFFFFFFF; i++)
						{
							if (weapons[i] == 0) continue;

							auto weapon = (C_WeaponCSBaseGun*)Source::m_pEntList->GetClientEntityFromHandle(weapons[i]);

							if (weapon == nullptr) continue;

							auto cweap_info = weapon->GetCSWeaponData();

							auto is_active = (acweapon == weapon && !lol);

							auto is_taser = weapon->m_iItemDefinitionIndex() == weapon_taser;

							if (!is_taser) 
							{
								if (!is_active && weapon->is_knife())
									continue;
								if (cweap_info->type == WEAPONTYPE_PISTOL && !cheat::Cvars.Visuals_Weapon.Has(1))
									continue;
								if (cweap_info->type == WEAPONTYPE_GRENADE && !cheat::Cvars.Visuals_Weapon.Has(2))
									continue;
								if (cweap_info->type == WEAPONTYPE_C4 && !cheat::Cvars.Visuals_Weapon.Has(4))
									continue;
								if (cweap_info->type > WEAPONTYPE_PISTOL && cweap_info->type < WEAPONTYPE_C4 && !cheat::Cvars.Visuals_Weapon.Has(0))
									continue;
							}
							else if (is_taser && !cheat::Cvars.Visuals_Weapon.Has(3))
								continue;

							char wpn_name[100] = "";

							sprintf(wpn_name, "%s", cweap_info->weapon_name + 7);

							if (weapon->m_iItemDefinitionIndex() == 64)
								strcpy(wpn_name, "revolver");

							if (weapon->m_nFallbackPaintKit() > 0)
								strcat(wpn_name, get_name_by_paint_kit(weapon->m_iItemDefinitionIndex(), weapon->m_nFallbackPaintKit()));

							auto wpn_name_size = Drawing::GetTextSize(F::ESPInfo, wpn_name);

							Drawing::DrawString(F::ESPInfo, x + w / 2 - wpn_name_size.right / 2, y + h + 9 * weapon_count++ + (down * 6.f), (is_active ? Color(0, 255, 0, dormant_alpha[idx] - 55.f) : Color(255, 255, 255, dormant_alpha[idx] - 55.f)), FONT_LEFT, wpn_name);
						}
					}
				}

				if ((acweapon->m_zoomLevel() > 0 || entity->m_bIsScoped()) && cheat::Cvars.Visuals_enemies_flags.Has(4))
					Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(244, 215, 66, dormant_alpha[idx] - 55.f), FONT_LEFT, "SCOPED"); // ping color : 77, 137, 234
			}

			//Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "R: %d", resolver_info.resolving_method);
			if (resolver_info.did_lby_flick && resolver_info.fakeangles)
				Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(230, 255, 230, dormant_alpha[idx] - 55.f), FONT_LEFT, "TAP");

			//Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "animt: %.2f", entity->m_anim_time());
			//Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "lbyt: %.2f", resolver_info.next_lby_time);
			//Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "lby upd: %.2f", fabs(entity->m_anim_time() - resolver_info.next_lby_time));
			//Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "flick: %s", resolver_info.did_lby_flick ? "+" : "-");

			//if (entity->m_flDuckAmount() > 0.98f && entity->m_fFlags() & IN_DUCK && Source::m_pGameMovement->GetPlayerMaxs(true).Distance(entity->GetCollideable()->OBBMaxs()) > 1.f)
			//	Drawing::DrawString(F::ESP, x + w + 3, y + right++ * (size_info.bottom + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "FAKEDUCK");

			/*if (fabs(Source::m_pGlobalVars->realtime - resolver_info.last_time_jittered) < 1.f)
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "lt j: %.2f", fabs(Source::m_pGlobalVars->realtime - resolver_info.last_time_jittered));

			if (fabs(Source::m_pGlobalVars->realtime - resolver_info.last_time_balanced) < 1.f)
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "lt b: %.2f", fabs(Source::m_pGlobalVars->realtime - resolver_info.last_time_balanced));

			if (fabs(Source::m_pGlobalVars->realtime - resolver_info.last_time_choked) < 1.f)
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "lt c: %.2f", fabs(Source::m_pGlobalVars->realtime - resolver_info.last_time_choked));

			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "R: %d", resolver_info.resolving_method);

			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "D: %.1f", resolver_info.delta_between_ticks);

			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "n vel: %.2f", entity->m_vecVelocity().Length2D());

			if (entity->get_animation_state())
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "a vel: %.2f", entity->get_animation_state()->m_flSpeed2D);

			if (entity->get_animation_state())
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "fyr: %.2f", entity->get_animation_state()->m_flFeetYawRate);

			if (entity->get_animation_state())
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "fc: %.2f", entity->get_animation_state()->m_flFeetCycle);*/
			//Drawing::DrawString(F::ESP, x + w + 3, y + right++ * (size_info.bottom + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, (cheat::features::aimbot.hitbox[entity->entindex() - 1] == 0 ? "HEAD" : "BODY"));

			//if (entity->m_bStrafing())
			//Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(255, 0, 0, dormant_alpha[idx] - 55.f), FONT_LEFT, "ST");

			//static auto angrot = Engine::PropManager::Instance()->GetOffset("DT_BaseEntity", "m_angRotation");

			//if (auto state = entity->get_animation_state(); state != nullptr) {
			//	//Drawing::DrawString(F::ESP, x + w + 3, y + right++ * (size_info.bottom + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "%.1f", entity->m_flLowerBodyYawTarget());

			//	Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "GFY: %.1f", state->m_flGoalFeetYaw);
			//	Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "CFY: %.1f", state->m_flCurrentFeetYaw);
			//	Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "EY: %.1f", state->m_flEyeYaw);

			//	Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "FYR: %.5f", state->m_flFeetYawRate);
			//	Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "FC: %.5f", state->m_flFeetCycle);
			//	Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "FSFS: %.5f", state->m_flFeetSpeedForwardSideways);
			//	Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "FSUFS: %.5f", state->m_flFeetSpeedUnknownForwardSideways);
			//	Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "TSSM: %.5f", state->m_flTimeSinceStartedMoving);
			//	Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "!TSSM: %.5f", state->m_flTimeSinceStoppedMoving);
			//}

			//Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "LTC: %.5f", (Source::m_pGlobalVars->realtime - resolver_info.last_time_choked));
			//Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "P: %.5f", (fabs(entity->get_pose(7) - 0.5f) * 116.f));
			//for(auto i = 0; i < 24; i++)
			//	Drawing::DrawString(F::OldESP, 10, 20 + 12 + i * 13, Color::White(200), FONT_LEFT, "%d %.6f", i, entity->get_pose(i));

			//for (auto j = 0; j < 19; j++)
			//{

			//	if (cheat::main::points[idx][j].empty())
			//		continue;

			//	for (auto i = 0; i < cheat::main::points[idx][j].size(); i++)
			//	{
			//		auto kek = cheat::main::points[idx][j][i];

			//		if (kek.IsZero()) continue;

			//		if (Vector screen = Vector::Zero; Drawing::WorldToScreen(kek, screen) && !screen.IsZero())
			//			Drawing::DrawString(F::OldESP, screen.x, screen.y, Color::White(200), FONT_LEFT, "%d", i);
			//	}
			//	//auto kek = cheat::features::aimbot.get_hitbox(entity, j);

			//	//if (Vector screen = Vector::Zero; Drawing::WorldToScreen(kek, screen) && !screen.IsZero())
			//	//	Drawing::DrawString(F::OldESP, screen.x, screen.y, Color::White(200), FONT_LEFT, "%d", j);
			//}

			//for (auto i = 0; i < entity->get_animation_layers_count(); i++)
			//{
			//	auto layer = /*&entity->get_animation_layer(i)*/&cheat::features::aaa.player_resolver_records[idx].server_anim_layers[i];
			//	Drawing::DrawString(F::OldESP, 10, 20 + 12 + i * 13, Color::White(200), FONT_LEFT, "[%d] weight: %.2f", i, layer->m_flWeight);
			//	Drawing::DrawString(F::OldESP, 120, 20 + 12 + i * 13, Color::White(200), FONT_LEFT, "[%d] playback: %.6f", i, layer->m_flPlaybackRate);
			//	Drawing::DrawString(F::OldESP, 260, 20 + 12 + i * 13, Color::White(200), FONT_LEFT, "[%d] cycle: %.6f", i, layer->m_flCycle);
			//	Drawing::DrawString(F::OldESP, 380, 20 + 12 + i * 13, Color::White(200), FONT_LEFT, "[%d] sequence: %d", i, layer->m_nSequence);
			//	//Drawing::DrawString(F::OldESP, 460, 20 + 12 + i * 13, Color::White(200), FONT_LEFT, "[%d] layer animtime: %.6f", i, layer->m_flLayerAnimtime);
			//}

			//for (auto i = 0; i < entity->get_animation_layers_count(); i++)
			//{
			//	auto layer = /*&entity->get_animation_layer(i)*/&cheat::features::aaa.player_resolver_records[idx].client_anim_layers[i];
			//	Drawing::DrawString(F::OldESP, 10, 300 + 12 + i * 13, Color::White(200), FONT_LEFT, "[%d] weight: %.2f", i, layer->m_flWeight);
			//	Drawing::DrawString(F::OldESP, 120, 300 + 12 + i * 13, Color::White(200), FONT_LEFT, "[%d] playback: %.6f", i, layer->m_flPlaybackRate);
			//	Drawing::DrawString(F::OldESP, 260, 300 + 12 + i * 13, Color::White(200), FONT_LEFT, "[%d] cycle: %.6f", i, layer->m_flCycle);
			//	Drawing::DrawString(F::OldESP, 380, 300 + 12 + i * 13, Color::White(200), FONT_LEFT, "[%d] sequence: %d", i, layer->m_nSequence);
			//	//Drawing::DrawString(F::OldESP, 460, 20 + 12 + i * 13, Color::White(200), FONT_LEFT, "[%d] layer animtime: %.6f", i, layer->m_flLayerAnimtime);
			//}

			/*auto cycle6 = cheat::features::aaa.player_resolver_records[idx].server_anim_layers[6].m_flCycle;
			float c6cycle = cheat::features::aaa.player_resolver_records[idx].client_anim_layers[6].m_flCycle;

			auto dogshit = TIME_TO_TICKS(cycle6 - c6cycle);

			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(60, 160, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "6:%d", dogshit);

			auto cycle11 = cheat::features::aaa.player_resolver_records[idx].server_anim_layers[11].m_flCycle;
			float c11cycle = cheat::features::aaa.player_resolver_records[idx].client_anim_layers[11].m_flCycle;

			auto dogshit11 = TIME_TO_TICKS(cycle11 - c11cycle);

			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(60, 160, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "11:%d", dogshit11);*/

			/*Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "DA: %.5f", entity->m_flDuckAmount());
			if (auto state = entity->get_animation_state(); state != nullptr) {
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "SDA: %.5f", state->m_flDuckAmount);
			}
			Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * (size_info + 2), Color::White(dormant_alpha[idx] - 55.f), FONT_LEFT, "DS: %.5f", entity->m_flDuckSpeed());*/

			if (!player_record->tick_records.empty() && !cheat::main::local()->IsDead())
			{
				if (cheat::Cvars.Visuals_enemies_flags.Has(1) && player_record->tick_records.front().type == RECORD_PRIORITY)
					Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color(60, 160, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, "vulnerable");


				if (cheat::Cvars.Visuals_Skeleton.GetValue())
				{
					if (cheat::Cvars.Visuals_Skeleton.GetValue() == 2 && player_record->tick_records.size() > 2)
						skeleton(entity, skeletonscolor.malpha(dormant_alpha[idx]), player_record->tick_records.at(player_record->tick_records.size() - 2).matrixes);
					else
						skeleton(entity, skeletonscolor.malpha(dormant_alpha[idx]), entity->m_CachedBoneData().Base());
				}
			}

			/*for (auto i = 0; i < player_record->m_Tickrecords.size(); i++)
			{
			if (player_record->m_Tickrecords.size() < i || player_record->m_Tickrecords.at(i).type != RECORD_SHOT)
			continue;

			skeleton(entity, Color::Red(200), player_record->m_Tickrecords.at(i).matrixes);
			}*/

			/*auto drawAngleLine = [&](const Vector& origin, const Vector& w2sOrigin, const float& angle, const char* text, Color clr) {
			Vector forward = QAngle(0.0f, angle, 0.0f).ToVectors();

			float AngleLinesLength = 30.0f;

			Vector w2sReal;
			if (Drawing::WorldToScreen(origin + forward * AngleLinesLength, w2sReal)) {
				Drawing::DrawLine(w2sOrigin.x, w2sOrigin.y, w2sReal.x, w2sReal.y, clr);
				Drawing::DrawRect(w2sReal.x - 5.0f, w2sReal.y - 5.0f, 10.0f, 10.0f, Color(50, 50, 50, 150));
				Drawing::DrawString(F::OldESP, w2sReal.x, w2sReal.y, clr, FONT_CENTER, text);
			}
			};

			Vector w2sOrigin;
			if (Drawing::WorldToScreen(entity->m_vecOrigin(), w2sOrigin) && entity->get_animation_state()) {
				drawAngleLine(entity->m_vecOrigin(), w2sOrigin, entity->m_flLowerBodyYawTarget(), "lby", Color(0, 0, 255, 255));
				drawAngleLine(entity->m_vecOrigin(), w2sOrigin, entity->get_animation_state()->abs_yaw, "abs", Color(255, 0, 0, 255));
				drawAngleLine(entity->m_vecOrigin(), w2sOrigin, entity->get_animation_state()->eye_yaw, "yaw", Color(255, 0, 0, 255));
				drawAngleLine(entity->m_vecOrigin(), w2sOrigin, entity->m_angEyeAngles().y, "ang", Color(0, 255, 0, 255));
				drawAngleLine(entity->m_vecOrigin(), w2sOrigin, entity->get_eye_angles().y, "aang", Color(255, 255, 0, 255));
			}*/

			//cheat::features::aimbot.draw_capsule(entity, 0);
		}
		else
		{
			if (entity == nullptr ||
				entity->IsDormant())
				continue;

			auto client_class = entity->GetClientClass();

			if (client_class == nullptr)
				continue;

			auto class_id = client_class->m_ClassID;

			switch (class_id)
			{
			case class_ids::CPlantedC4: //BOMB
			{
				if (!cheat::Cvars.Visuals_wrld_entities.Has(1))
					continue;

				auto bomb_blow_timer = entity->get_bomb_blow_timer();
				auto bomb_defuse_timer = entity->get_bomb_defuse_timer();

				auto can_defuse = (bomb_defuse_timer <= bomb_blow_timer && cheat::main::local()->m_iTeamNum() == 3) || (bomb_defuse_timer > bomb_blow_timer && cheat::main::local()->m_iTeamNum() == 2 || bomb_defuse_timer <= bomb_blow_timer);

				if (cheat::main::local()->m_iHealth() > 0)
				{
					float flArmor = cheat::main::local()->m_ArmorValue();
					float flDistance = cheat::main::local()->m_vecOrigin().Distance(entity->m_vecOrigin());
					float a = 450.7f;
					float b = 75.68f;
					float c = 789.2f;
					float d = ((flDistance - b) / c);

					float flDmg = a * exp(-d * d);
					float flDamage = flDmg;

					if (flArmor > 0)
					{
						float flNew = flDmg * 0.5f;
						float flArmor = (flDmg - flNew) * 0.5f;

						if (flArmor > static_cast<float>(flArmor))
						{
							flArmor = static_cast<float>(flArmor) * (1.f / 0.5f);
							flNew = flDmg - flArmor;
						}

						flDamage = flNew;
					}

					float dmg = 100 - (cheat::main::local()->m_iHealth() - flDamage);

					if (dmg > 100)
						dmg = 100;
					if (dmg < 0)
						dmg = 0;

					int Red = dmg;
					int Green = 255 - dmg * 2.55;

					if (flDamage > 1) {
						if (flDamage >= cheat::main::local()->m_iHealth())
							Drawing::DrawString(F::LBY, 10, 150, Color(Red, Green, 0, 255), FONT_LEFT, "FATAL -%.1fHP", flDamage);
						else
							Drawing::DrawString(F::LBY, 10, 150, Color(Red, Green, 0, 255), FONT_LEFT, "-%.1fHP", flDamage);
					}
				}

				if (bomb_blow_timer > 0.f)
				{
					auto max_bombtimer = max_bombtime->GetFloat();

					auto btime = bomb_blow_timer * (100.f / max_bombtimer);

					if (btime > 100.f)
						btime = 100.f;
					else if (btime < 0.f)
						btime = 0.f;

					int blow_percent = screenH - (int)((98 * btime) / 100);
					int b_red = 255 - (btime * 2.55);
					int b_green = btime * 2.55;

					Drawing::DrawRect(10, ind_h + 15 + 20 * indicators, 100, 15, { 0, 0, 0, 130 });
					Drawing::DrawString(F::ESPInfo, 60, ind_h + 15 + 20 * indicators, can_defuse ? Color::Green(240) : Color::Red(240), FONT_CENTER, "BOMB");

					Drawing::DrawRect(11,
						ind_h + 25 + 20 * indicators++,
						(98.f * float(bomb_blow_timer / max_bombtimer)),
						4,
						{ b_red,b_green ,0,130 });

					if (Vector dummy = Vector::Zero; Drawing::WorldToScreen(entity->m_vecOrigin(), dummy) && !dummy.IsZero())
						Drawing::DrawString(F::ESPInfo, dummy.x, dummy.y, can_defuse ? Color::Green(240) : Color::Red(240), FONT_CENTER, "BOMB: %.1f", bomb_blow_timer);

					if (bomb_defuse_timer > 0.f)
					{
						auto defuser = entity->m_hBombDefuser();

						auto max_defuse_timer = (defuser && !defuser->IsDead() && defuser->m_bHasDefuser()) ? 5.f : 10.f;

						auto dtime = bomb_defuse_timer * (100.f / max_defuse_timer);

						if (dtime > 100.f)
							dtime = 100.f;
						else if (dtime < 0.f)
							dtime = 0.f;

						int defuse_percent = screenH - (int)((98 * dtime) / 100);
						int d_red = 255 - (dtime * 2.55);
						int d_green = dtime * 2.55;

						Drawing::DrawRect(10, ind_h + 15 + 20 * indicators, 100, 15, { 0, 0, 0, 130 });
						Drawing::DrawString(F::ESPInfo, 60, ind_h + 15 + 20 * indicators, can_defuse ? Color::Green(240) : Color::Red(240), FONT_CENTER, "DEFUSE");

						Drawing::DrawRect(11,
							ind_h + 25 + 20 * indicators++,
							(98.f * float(bomb_defuse_timer / max_defuse_timer)),
							4,
							{ b_red, b_green, 0,130 });
					}
				}
				break;
			}
			case class_ids::CInferno:
			{
				const auto owner = entity->m_hOwnerEntity();

				auto eowner = Source::m_pEntList->GetClientEntityFromHandle(owner);

				int x, y, w, h;
				if (!get_espbox(entity, x, y, w, h) || !eowner || !cheat::main::local() || (eowner->m_iTeamNum() == cheat::main::local()->m_iTeamNum() && eowner != cheat::main::local())) continue;

				if (cheat::Cvars.Visuals_Box.GetValue()) {
					Drawing::DrawOutlinedRect(x, y, w, h, Color::Red().alpha(200));
					Drawing::DrawOutlinedRect(x - 1, y - 1, w + 2, h + 2, Color(10, 10, 10, 150));
					Drawing::DrawOutlinedRect(x + 1, y + 1, w - 2, h - 2, Color(10, 10, 10, 150));
				}

				const auto spawn_time = *(float*)(uintptr_t(entity) + 0x20);
				const auto factor = ((spawn_time + 7.f) - Source::m_pGlobalVars->curtime) / 7.f;

				int red = max(min(255 * factor, 255), 0);
				int green = max(min(255 * (1.f - factor), 255), 0);

				static auto text_size = Drawing::GetTextSize(F::ESPInfo, "inferno");
				//Drawing::DrawRect(x - 50, y, 100, 15, { 0, 0, 0, 205 });
				Drawing::DrawRect(x - 49, y + 10, 98.f, 4, { 80, 80, 80, 125 });
				Drawing::DrawRect(x - 49, y + 10, 98.f * factor, 4, { red, green, 0, 245 });
				Drawing::DrawString(F::ESPInfo, x, y, { 255, 255, 255, 255 }, FONT_CENTER, "inferno");

				Drawing::DrawString(F::ESPInfo, x - 49 + 98.f * factor, y + 8, { 255, 255, 255, 240 }, FONT_CENTER, "%.0f", (spawn_time + 7.f) - Source::m_pGlobalVars->curtime);

				break;
			}
			case class_ids::CC4:
			{
				const auto owner = entity->m_hOwnerEntity();
				int x, y, w, h;
				if (!cheat::Cvars.Visuals_wrld_entities.Has(1) || !get_espbox(entity, x, y, w, h) || owner != -1) continue;

				static auto text_size = Drawing::GetTextSize(F::ESPInfo, "bomb");
				Drawing::DrawString(F::ESPInfo, x + w / 2 - text_size.right / 2, (center ? y + h / 2.f : y + h), Color::Green(230), FONT_LEFT, "bomb");

				break;
			}
			default:
			{
				const auto owner = entity->m_hOwnerEntity();

				if (cheat::Cvars.Visuals_wrld_entities.Has(0))
				{
					if (owner == -1)
					{
						const char* name = "";

						if (strstr(client_class->m_pNetworkName, "CWeapon"))
						{
							auto weapon = (C_WeaponCSBaseGun*)entity;

							name = "unknown";

							if (weapon->m_iItemDefinitionIndex() != 64 && weapon->GetCSWeaponData())
								name = (weapon->GetCSWeaponData()->weapon_name + 7);
							else
								name = "R8 Revolver";
						}
						else if (client_class->m_ClassID == class_ids::CAK47)
							name = "AK-47";
						else if (client_class->m_ClassID == class_ids::CDEagle)
							name = "Deagle";

						int x, y, w, h;
						if (strlen(name) <= 1 || !get_espbox(entity, x, y, w, h)) continue;

						if (cheat::Cvars.Visuals_Box.GetValue()) {
							Drawing::DrawOutlinedRect(x, y, w, h, cheat::Cvars.Visuals_wrld_entities_color.GetColor().alpha(200));
							Drawing::DrawOutlinedRect(x - 1, y - 1, w + 2, h + 2, Color(10, 10, 10, 150));
							Drawing::DrawOutlinedRect(x + 1, y + 1, w - 2, h - 2, Color(10, 10, 10, 150));
						}

						auto text_size = Drawing::GetTextSize(F::ESPInfo, name);
						Drawing::DrawString(F::ESPInfo, x + w / 2 - text_size.right / 2, (center ? y + h / 2.f : y + h), cheat::Cvars.Visuals_wrld_entities_color.GetColor().alpha(230), FONT_LEFT, name);
					}
				}

				if (cheat::Cvars.Visuals_wrld_entities.Has(2))
				{
					if (strstr(client_class->m_pNetworkName, "Projectile") || strstr(client_class->m_pNetworkName, "Grenade") || strstr(client_class->m_pNetworkName, "Flashbang"))
					{
						//DrawThrowable(Entity, Entity->GetClientClass());
						const model_t* nadeModel = entity->GetModel();

						if (!nadeModel)
							continue;

						studiohdr_t* hdr = Source::m_pModelInfo->GetStudioModel(nadeModel);

						if (!hdr)
							continue;

						if (!strstr(hdr->name, "thrown") && !strstr(hdr->name, "dropped"))
							continue;

						if (entity->m_nExplodeEffectTickBegin() >= 1)
							continue;

						Color nadeColor = Color(255, 255, 255, 255);
						const char* nadeName = "Unknown Grenade";

						IMaterial* mats[32];
						Source::m_pModelInfo->GetModelMaterials(nadeModel, hdr->numtextures, mats);

						for (int i = 0; i < hdr->numtextures; i++)
						{
							IMaterial* mat = mats[i];
							if (!mat || mat->IsErrorMaterial())
								continue;

							if (strstr(mat->GetName(), "flashbang"))
							{
								nadeName = "Flashbang";
								nadeColor = Color::LightBlue();
								break;
							}
							else if (strstr(mat->GetName(), "m67_grenade") || strstr(mat->GetName(), "hegrenade"))
							{
								nadeName = "Frag";
								nadeColor = Color::Red();
								break;
							}
							else if (strstr(mat->GetName(), "smoke"))
							{
								nadeName = "Smoke";
								nadeColor = Color::Green();
								break;
							}
							else if (strstr(mat->GetName(), "incendiary") || strstr(mat->GetName(), "molotov"))
							{
								nadeName = "Molotov";
								nadeColor = Color(200, 130, 0);
								break;
							}
						}

						//if (is_dropped)
						//	nadeColor = Color::Grey();

						int x, y, w, h;
						if (!get_espbox(entity, x, y, w, h)) continue;

						if (cheat::Cvars.Visuals_Box.GetValue()) {
							Drawing::DrawOutlinedRect(x, y, w, h, nadeColor.alpha(200));
							Drawing::DrawOutlinedRect(x - 1, y - 1, w + 2, h + 2, Color(10, 10, 10, 150));
							Drawing::DrawOutlinedRect(x + 1, y + 1, w - 2, h - 2, Color(10, 10, 10, 150));
						}

						auto text_size = Drawing::GetTextSize(F::ESPInfo, nadeName);
						Drawing::DrawString(F::ESPInfo, x + w / 2 - text_size.right / 2, (center ? y + h / 2.f : y + h), nadeColor.alpha(230), FONT_LEFT, nadeName);
					}
				}

				break;
			}
			}
		}
	}
}