#include "hooked.hpp"
#include "displacement.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "prediction.hpp"
#include "movement.hpp"
#include "aimbot.hpp"
#include "anti_aimbot.hpp"
#include "game_events.h"
#include <playsoundapi.h>
#include "visuals.hpp"
#include <intrin.h>
#include "angle_resolver.hpp"
#include "lag_compensation.hpp"
#include "rmenu.hpp"
#include "autowall.hpp"
#include "visuals.hpp"

#pragma comment(lib, "Winmm.lib")

game_events::PlayerHurtListener player_hurt_listener;
game_events::BulletImpactListener bullet_impact_listener;
game_events::PlayerDeathListener player_death_listener;
game_events::RoundEndListener round_end_listener;
game_events::RoundStartListener round_start_listener;

void game_events::init()
{
	//INTERFACES::GameEventManager->AddListener(&item_purchase_listener, "item_purchase", false);
	Source::m_pGameEvents->AddListener(&player_hurt_listener, "player_hurt", false);
	Source::m_pGameEvents->AddListener(&bullet_impact_listener, "bullet_impact", false);
	Source::m_pGameEvents->AddListener(&player_death_listener, "player_death", false);
	Source::m_pGameEvents->AddListener(&round_end_listener, "round_end", false);
	Source::m_pGameEvents->AddListener(&round_start_listener, "round_start", false);
}

char *hitgroup_to_name(const int hitgroup) {
	/*switch (hitgroup) {
	case HITGROUP_HEAD:
		return "head";
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		return "leg";
	case HITGROUP_STOMACH:
		return "stomach";
	default:
		return "body";
	}*/
	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		return "head";
	case HITGROUP_CHEST:
		return "chest";
	case HITGROUP_STOMACH:
		return "stomach";
	case HITGROUP_LEFTARM:
		return "left arm";
	case HITGROUP_RIGHTARM:
		return "right arm";
	case HITGROUP_LEFTLEG:
		return "left leg";
	case HITGROUP_RIGHTLEG:
		return "right leg";
	default:
		return "body";
	}
}

void game_events::PlayerHurtListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event)
		return;

	if (!cheat::main::local())
		return;

	auto entity = Source::m_pEntList->GetClientEntity(Source::m_pEngine->GetPlayerForUserID(game_event->GetInt("userid")));
	if (!entity)
		return;

	auto entity_attacker = Source::m_pEntList->GetClientEntity(Source::m_pEngine->GetPlayerForUserID(game_event->GetInt("attacker")));
	if (!entity_attacker)
		return;

	if (entity->m_iTeamNum() == cheat::main::local()->m_iTeamNum())
		return;

	player_info player_info;
	if (!Source::m_pEngine->GetPlayerInfo(entity->entindex(), &player_info))
		return;

	if (entity_attacker == cheat::main::local())
	{
		if (!cheat::main::fired_shot.empty()) {
			if ((cheat::main::fired_shot.back()._avg_impact_time - Source::m_pGlobalVars->curtime) <= (Source::m_pGlobalVars->interval_per_tick + TICKS_TO_TIME(2))) {
				//_events.push_back(_event(Source::m_pGlobalVars->curtime + 3.f, "Hurt predicted right"));
				cheat::main::fired_shot.back()._hurt_called = true;
				cheat::main::fired_shot.back()._headshot = game_event->GetBool("headshot");
			}
			else {
				//_events.push_back(_event(Source::m_pGlobalVars->curtime + 3.f, std::string("Hurt predict failed! Time:" + std::to_string(cheat::main::fired_shot.back()._avg_impact_time - Source::m_pGlobalVars->curtime))));
				cheat::main::fired_shot.clear();
			}
		}

		_events.push_back(_event(Source::m_pGlobalVars->curtime + 5.f, std::string("Hurt " + std::string(player_info.name) + " in the " + hitgroup_to_name(game_event->GetInt("hitgroup")) + " for " + std::to_string(game_event->GetInt("dmg_health")) + " " + std::string(player_info.name) + " has " + std::to_string(game_event->GetInt("health")) + " health remaining.")));

		if (cheat::main::shots_fired[entity->entindex() - 1] > 0)
			cheat::main::shots_fired[entity->entindex() - 1]--;

		auto& lag_data = cheat::features::aaa.player_resolver_records[entity->entindex() - 1];

		if (lag_data.missed_shots[lag_data.resolving_method] > 0)
			lag_data.missed_shots[lag_data.resolving_method] -= 1;

		Source::m_pSurface->PlaySound_("buttons\\arena_switch_press_02.wav");
		//PlaySoundA(hitmarkersnd, NULL, SND_ASYNC | SND_MEMORY);
		cheat::main::hit_time = Source::m_pGlobalVars->realtime + 0.3f;

		if (game_event->GetBool("headshot") && entity) {
			cheat::main::history_hit[entity->entindex() - 1] = cheat::main::shots_fired[entity->entindex() - 1];

			if (entity && entity->get_animation_state()) {
				lag_data.previous_angles.y = entity->get_animation_state()->abs_yaw;
				//lag_data.did_hit_low_delta = (lag_data.current_resolve_delta < lag_data.current_tick_max_delta);
				//lag_data.did_hit_max_delta = (lag_data.current_resolve_delta == 0.0f);
				//lag_data.did_hit_inversed_lby = lag_data.inverse_lby;

			}
		}
	}
}

float last_bullet_impact_back = 0;

void game_events::BulletImpactListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event)
		return;

	auto entity = Source::m_pEntList->GetClientEntity(Source::m_pEngine->GetPlayerForUserID(game_event->GetInt("userid")));

	if (!entity)
		return;

	if (!cheat::main::local() || !Source::m_pEngine->IsConnected())
		return;

	if (!entity)
		return;

	if (entity->IsDormant())
		return;

	Vector position(game_event->GetInt("x"), game_event->GetInt("y"), game_event->GetInt("z"));

	auto islocal = entity == cheat::main::local();

	if (!islocal || cheat::Cvars.Visuals_Bullet_Tracers.Has(1)) {
		auto eyepos = entity->GetEyePosition();

		//QAngle anglesReal = Math::CalcAngle(eyepos, position);

		//float distance = (position - eyepos).Length();

		//Vector simulatedPoint;
		//Math::AngleVectors(anglesReal, &simulatedPoint);
		////simulatedPoint.NormalizeInPlace();
		//simulatedPoint = eyepos + simulatedPoint * (distance * 3);

		//Source::m_pDebugOverlay->AddLineOverlay(eyepos, simulatedPoint, 255, 125, 0, true, 4.f);

		auto can_trace = (cheat::Cvars.Visuals_Bullet_Tracers.Has(0) && entity->m_iTeamNum() != cheat::main::local()->m_iTeamNum());

		auto trace_color = cheat::Cvars.Visuals_tracer_enemy_color.GetColor();// Color(255, 50, 0);

		if (cheat::Cvars.Visuals_Bullet_Tracers.Has(1) && entity == cheat::main::local()) {
			can_trace = true;
			trace_color = cheat::Cvars.Visuals_tracer_local_color.GetColor();
		}

		if (cheat::Cvars.Visuals_Bullet_Tracers.Has(2) && entity->m_iTeamNum() == cheat::main::local()->m_iTeamNum()) {
			can_trace = true;
			trace_color = cheat::Cvars.Visuals_tracer_teammate_color.GetColor();
		}

		if (can_trace)
			bullet_tracers.push_back(c_bullet_tracer(eyepos, position, Source::m_pGlobalVars->curtime, trace_color));
	}

	if (islocal) {

		if (cheat::Cvars.Visuals_wrld_impact.GetValue())
			Source::m_pDebugOverlay->AddBoxOverlay(position, Vector(-2.f, -2.f, -2.f), Vector(2.f, 2.f, 2.f), Vector(0.f, 0.f, 0.f), 0, 0, 255, 127, cheat::Cvars.Visuals_wrld_impact_duration.GetValue());

		if (!cheat::main::fired_shot.empty()) {

			auto rec = cheat::main::fired_shot.back();

			//QAngle anglesReal = Math::CalcAngle(rec._eyepos, rec._hitbox);

			/*float distance = (rec._hitbox - rec._eyepos).Length();

			Vector simulatedPoint;
			Math::AngleVectors(anglesReal, &simulatedPoint);
			simulatedPoint.NormalizeInPlace();*/
			//simulatedPoint = rec._eyepos + simulatedPoint * distance;
			/*if (auto wp = cheat::main::local()->get_weapon(); wp != nullptr) {
				Vector vecDirShooting, vecRight, vecUp;
				Math::AngleVectors(anglesReal, &vecDirShooting, &vecRight, &vecUp);

				Vector vecDir = vecDirShooting +
					vecRight * (rec._hitbox.x * cheat::main::fired_shot.back()._spread) +
					vecUp * (rec._hitbox.y * cheat::main::fired_shot.back()._spread);

				CTraceFilter filter; filter.pSkip = cheat::main::local();
				trace_t trace;

				Vector vecEnd = rec._eyepos + vecDir * 8000.f;
				Ray_t ray; ray.Init(rec._eyepos, rec._hitbox, Vector(-2.f, -2.f, -2.f), Vector(2.f, 2.f, 2.f));

				Source::m_pEngineTrace->TraceRay(ray, MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_HITBOX, &filter, &trace);

				if (trace.fraction < 1.f) 
					Source::m_pDebugOverlay->AddBoxOverlay(position, Vector(-2.f, -2.f, -2.f), Vector(2.f, 2.f, 2.f), Vector(0.f, 0.f, 0.f), 255, 0, 0, 127, 4);
			}*/

			if ((cheat::main::fired_shot.back()._avg_impact_time - Source::m_pGlobalVars->curtime) <= (Source::m_pGlobalVars->interval_per_tick + TICKS_TO_TIME(2))) {
				//_events.push_back(_event(Source::m_pGlobalVars->curtime + 3.f, "Impact predicted right"));
				cheat::main::fired_shot.back()._impact_called = true;
				cheat::main::fired_shot.back()._impact_pos = position;
			}
			else {
				//_events.push_back(_event(Source::m_pGlobalVars->curtime + 3.f, std::string("Impact predict failed! Time:" + std::to_string(cheat::main::fired_shot.back()._avg_impact_time - Source::m_pGlobalVars->curtime))));
				cheat::main::fired_shot.clear();
			}
		}
	}

	//if (last_bullet_impact_back == Source::m_pGlobalVars->curtime)
	//	return;

	last_bullet_impact_back = Source::m_pGlobalVars->curtime;
}

void game_events::RoundEndListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event)
		return;

	//lc->reset();

	memset(cheat::main::shots_fired, 0, sizeof(int) * 128);
	memset(cheat::main::shots_total, 0, sizeof(int) * 128);

	for (auto i = 0; i++; i < 65) {
		memset(cheat::features::aaa.player_resolver_records[i].missed_shots, 0, sizeof(int) * 11);
	}

	memset(cheat::features::visuals.dormant_alpha, 0.f, sizeof(int) * 128);



	/*auto winner = game_event->GetInt("winner");

	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	if (!local_player) return;

	if (winner == local_player->GetTeam())
		should_disable_aa = true;*/
}

void game_events::RoundStartListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event)
		return;

	//auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
	//if (!local_player)
	//	return;

	memset(cheat::main::shots_fired, 0, sizeof(int) * 128);
	memset(cheat::main::shots_total, 0, sizeof(int) * 128);

	bullet_tracers.clear();

	memset(cheat::features::visuals.dormant_alpha, 0, sizeof(int) * 128);

	static void(__thiscall *ClearDeathNotices)(DWORD);

	if (cheat::Cvars.Visuals_misc_preserve_kills.GetValue()) {
		/*cheat::game::hud_death_notice = cheat::game::find_hud_element<CCSGO_HudDeathNotice*>("CCSGO_HudDeathNotice");
		cheat::game::update_hud_weapons = (void*)Memory::Scan(cheat::main::clientdll, "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C");

		if (cheat::game::hud_death_notice) {

			auto death_notices = ((int*)((uintptr_t)cheat::game::hud_death_notice - 20));

			if (death_notices) {
				auto ClearDeathNotices = (void(__thiscall*)(DWORD))Memory::Scan(cheat::main::clientdll, "55 8B EC 83 EC 0C 53 56 8B 71 58");

				if (ClearDeathNotices)
					ClearDeathNotices((DWORD)cheat::game::hud_death_notice - 20);
			}
		}*/
	}

	cheat::features::lagcomp.reset();

	const auto sv_skyname = Source::m_pCvar->FindVar("sv_skyname");

	static const char* skyboxes[] = {
		"cs_tibet",
		"cs_baggage_skybox_",
		"embassy",
		"italy",
		"jungle",
		"office",
		"sky_cs15_daylight01_hdr",
		"vertigoblue_hdr",
		"sky_cs15_daylight02_hdr",
		"vertigo",
		"sky_day02_05_hdr",
		"nukeblank",
		"sky_venice",
		"sky_cs15_daylight03_hdr",
		"sky_cs15_daylight04_hdr",
		"sky_csgo_cloudy01",
		"sky_csgo_night02",
		"sky_csgo_night02b",
		"sky_csgo_night_flat",
		"sky_dust",
		"vietnam",
		"amethyst",
		"sky_descent",
		"clear_night_sky",
		"otherworld",
		"cloudynight",
		"dreamyocean",
		"grimmnight",
		"sky051",
		"sky081",
		"sky091",
		"sky561",
	};

	sv_skyname->m_nFlags &= ~FCVAR_CHEAT;

	if ((int)cheat::Cvars.Visuals_world_sky.GetValue())
		sv_skyname->SetValue(skyboxes[(int)cheat::Cvars.Visuals_world_sky.GetValue() - 1]);

	////lc->reset();

	static auto shit = Source::m_pCvar->FindVar("mp_freezetime");

	if (shit)
		cheat::features::antiaimbot.enable_delay = Source::m_pGlobalVars->curtime + shit->GetFloat();

	//should_disable_aa = false;

	//static std::string buy = "";

	//if (SETTINGS::settings.ab_primary) //TODO: add check for isweapon already inhand
	//{
	//	buy += ((SETTINGS::settings.ab_primary == 1) ? "buy g3sg1;" : ((SETTINGS::settings.ab_primary == 2) ? "buy ssg08;" : "buy awp;"));
	//}
	//if (SETTINGS::settings.ab_secondary)
	//{
	//	buy += ((SETTINGS::settings.ab_secondary == 1) ? " buy deagle;" : ((SETTINGS::settings.ab_secondary == 2) ? " buy tec9;" : ((SETTINGS::settings.ab_secondary == 3) ? " buy p250;" : "buy elite;")));
	//}
	//if (SETTINGS::settings.armor)
	//	buy += " buy vesthelm;";
	//if (SETTINGS::settings.nades)
	//{
	//	buy += " buy molotov; buy hegrenade; buy smokegrenade; buy defuser; buy taser;";
	//}

	//INTERFACES::Engine->ClientCmd(buy.c_str()); //TODO: maybe find another method how to autobuy? since this one is ghetto asf

	//buy = "";
}

void game_events::PlayerDeathListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event)
		return;

	if (!Source::m_pEngine->IsInGame())
		return;

	auto entity = Source::m_pEntList->GetClientEntity(Source::m_pEngine->GetPlayerForUserID(game_event->GetInt("userid")));
	if (!entity)
		return;

	auto attacker = Source::m_pEntList->GetClientEntity(Source::m_pEngine->GetPlayerForUserID(game_event->GetInt("attacker")));
	if (!attacker)
		return;

	cheat::main::shots_fired[entity->entindex() - 1] = 0;
	cheat::main::shots_total[entity->entindex() - 1] = 0;

	cheat::features::lagcomp.reset();

	//SDK::player_info_t player_info;
	//if (!INTERFACES::Engine->GetPlayerInfo(entity->GetIndex(), &player_info))
	//	return;

	//auto event_weapon = game_event->GetString("weapon");

	//if (!event_weapon)
	//	return;

	//if (strstr(event_weapon, "weapon_unknown"))
	//	return;

	//auto weapon = reinterpret_cast<SDK::CBaseWeapon*>(INTERFACES::ClientEntityList->GetClientEntity(local_player->GetActiveWeaponIndex()));

	//if (!weapon)
	//	return;

	////if (SETTINGS::settings.force_kills)
	////	*(float*)((BYTE*)this + 0x54) = (attacker == local_player) ? 35 /* 1 minutes */ : 5 /* 5 seconds */;
	//if (attacker == local_player)
	//{
	//	//VisualizeBacktrackingData(entity, RED, 3.f);

	//	if (SETTINGS::settings.fake_media > 0)
	//	{
	//		if (GoodWeapon(event_weapon))
	//		{
	//			if (weapon->GetItemDefenitionIndex() == SDK::ItemDefinitionIndex::WEAPON_KNIFE_CT
	//				|| weapon->GetItemDefenitionIndex() == SDK::ItemDefinitionIndex::WEAPON_KNIFE_T
	//				|| weapon->GetItemDefenitionIndex() == SDK::ItemDefinitionIndex::WEAPON_HEGRENADE
	//				|| weapon->GetItemDefenitionIndex() == SDK::ItemDefinitionIndex::WEAPON_INCGRENADE
	//				|| weapon->GetItemDefenitionIndex() == SDK::ItemDefinitionIndex::WEAPON_SMOKEGRENADE)
	//				return;

	//			if (SETTINGS::settings.fake_media == 1) //perefect fake media
	//			{
	//				if (!game_event->GetBool("headshot"))
	//					game_event->SetInt("headshot", 1);
	//			}
	//			else //chance
	//			{
	//				if (randnum(0, 101) < (int)SETTINGS::settings.hs_percent)
	//				{
	//					if (!game_event->GetBool("headshot"))
	//						game_event->SetInt("headshot", 1);
	//				}
	//			}
	//		}
	//	}
	//}
}