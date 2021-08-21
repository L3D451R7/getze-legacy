#include "player.hpp"
#include "sound_parser.hpp"
#include "Visuals.hpp"
#include "source.hpp"
#include <algorithm>
#include "rmenu.hpp"
#include "hooked.hpp"
#include "angle_resolver.hpp"

void c_dormant_esp::start()
{
	if (!Source::m_pEngine->IsInGame() || !cheat::main::local())
		return;
		
	m_utlCurSoundList.RemoveAll();
	Source::m_pEngineSound->GetActiveSounds(m_utlCurSoundList);

	// No active sounds.
	if (!m_utlCurSoundList.Count())
		return;

	// Accumulate sounds for esp correction
	for (int iter = 0; iter < m_utlCurSoundList.Count(); iter++)
	{
		SndInfo_t& sound = m_utlCurSoundList[iter];
		if (sound.m_nSoundSource == 0 || // World
			sound.m_nSoundSource > 64)   // Most likely invalid
			continue;

		C_BasePlayer* player = Source::m_pEntList->GetClientEntity(sound.m_nSoundSource);

		if (!player || player == cheat::main::local() || sound.m_pOrigin->IsZero())
			continue;

		if (!valid_sound(sound))
			continue;

		setup_adjust(player, sound);

		m_cSoundPlayers[sound.m_nSoundSource-1].Override(sound);
	}

	m_utlvecSoundBuffer = m_utlCurSoundList;
}

void c_dormant_esp::setup_adjust(C_BasePlayer* player, SndInfo_t& sound)
{
	Vector src3D, dst3D;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = player;
	src3D = (*sound.m_pOrigin) + Vector(0, 0, 1); // So they dont dig into ground incase shit happens /shrug
	dst3D = src3D - Vector(0, 0, 100);
	ray.Init(src3D, dst3D);

	Source::m_pEngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &tr);

	// step = (tr.fraction < 0.20)
	// shot = (tr.fraction > 0.20)
	// stand = (tr.fraction > 0.50)
	// crouch = (tr.fraction < 0.50)

	/* Corrects origin and important flags. */

	// Player stuck, idk how this happened
	if (tr.allsolid)
	{
		m_cSoundPlayers[sound.m_nSoundSource-1].m_iReceiveTime = -1;
	}

	*sound.m_pOrigin = ((tr.fraction < 0.97) ? tr.endpos : *sound.m_pOrigin);
	m_cSoundPlayers[sound.m_nSoundSource - 1].m_nFlags = player->m_fFlags();
	m_cSoundPlayers[sound.m_nSoundSource - 1].m_nFlags |= (tr.fraction < 0.50f ? FL_DUCKING : 0) | (tr.fraction != 1 ? FL_ONGROUND : 0);   // Turn flags on
	m_cSoundPlayers[sound.m_nSoundSource - 1].m_nFlags &= (tr.fraction > 0.50f ? ~FL_DUCKING : 0) | (tr.fraction == 1 ? ~FL_ONGROUND : 0); // Turn flags off
}

void c_dormant_esp::finish()
{
	//owo
}

void c_dormant_esp::draw_sound(C_BasePlayer* entity)
{
	// Adjusts player's origin and other vars so we can show full-ish esp.

	if ((entity->entindex() - 1) < 0 || (entity->entindex() - 1) > 64)
		return;

	auto& sound_player = m_cSoundPlayers[entity->entindex()-1];
	bool sound_expired = fabs(Source::m_pGlobalVars->realtime - sound_player.m_iReceiveTime) > 4.f;

	float meme = ((4.f - (Source::m_pGlobalVars->realtime - sound_player.m_iReceiveTime)) / 4.f) * 255.f;

	if (sound_expired && cheat::features::visuals.dormant_alpha[entity->entindex()-1] == 0.f)
		return;

	entity->m_fFlags() = sound_player.m_nFlags;
	entity->m_vecOrigin() = sound_player.m_vecOrigin;
	entity->set_abs_origin(sound_player.m_vecOrigin);

	player_info info;

	if (!entity ||
		!entity->IsPlayer() ||
		entity->m_iHealth() <= 0 ||
		(entity->m_iTeamNum() == cheat::main::local()->m_iTeamNum() && !cheat::Cvars.Visuals_teammates.GetValue()) ||
		!Source::m_pEngine->GetPlayerInfo(entity->entindex(), &info)
		) return;

	auto idx = entity->entindex() - 1;

	if (cheat::Cvars.Visuals_fov_arrows.GetValue() && !cheat::main::local()->IsDead())
		cheat::features::visuals.draw_pov_arrows(entity, 200.f);

	int x, y, w, h;
	if (!cheat::features::visuals.get_espbox(entity, x, y, w, h)) return;

	int right = 0;
	int down = 0;
	
	static auto size_info = 9.f;

	Color color = cheat::Cvars.EnemyBoxCol.GetColor();
	Color skeletonscolor = cheat::Cvars.Visuals_skeletonColor.GetColor();

	if (cheat::Cvars.Visuals_Box.GetValue()) {
		Drawing::DrawOutlinedRect(x, y, w, h, color.alpha(meme));
		Drawing::DrawOutlinedRect(x - 1, y - 1, w + 2, h + 2, Color(10, 10, 10, (meme * 0.8f)));
		Drawing::DrawOutlinedRect(x + 1, y + 1, w - 2, h - 2, Color(10, 10, 10, (meme * 0.8f)));
	}

	if (cheat::Cvars.Visuals_Name.GetValue()) {
		auto text_size = Drawing::GetTextSize(F::NewESP, info.name);

		Drawing::DrawString(F::NewESP, x + w / 2 - text_size.right / 2, y - 13, Color::Grey(meme - 55.f), FONT_LEFT, u8"%s", info.name);
	}

	if (cheat::Cvars.Visuals_Snaplines.GetValue())
		Drawing::DrawLine(cheat::game::screen_size.x / 2, cheat::game::screen_size.y / 2, x + w / 2, y + h, cheat::Cvars.Visuals_SnaplinesColor.GetColor().alpha(meme));

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

		Drawing::DrawRect(x - 5, y - 1, 4, h + 2, Color(10, 10, 10, meme / 2));
		Drawing::DrawOutlinedRect(x - 5, y - 1, 4, h + 2, Color(10, 10, 10, (meme * 0.8f)));
		Drawing::DrawRect(x - 4, y + hp_percent, 2, h - hp_percent, Color(red, green, 0, meme));

		if (hp < 90)
			Drawing::DrawString(F::ESPInfo, x - text_size.right - 6, y - 1, Color::Grey(meme - 55.f), FONT_LEFT, hps);

		float height = h / 10.f;
	}

	if (entity->m_ArmorValue() > 0.f && cheat::Cvars.Visuals_enemies_flags.Has(0))
		Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::Grey(meme - 55.f), FONT_LEFT, (entity->m_bHasHelmet() ? "HK" : "K"));

	auto resolver_info = cheat::features::aaa.player_resolver_records[entity->entindex() - 1];

	if (cheat::Cvars.Visuals_enemies_flags.Has(1) && resolver_info.fakeangles) {
		Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::Grey(meme - 55.f), FONT_LEFT, "FAKE");
		Drawing::DrawString(F::ESPInfo, x + w + 3, y + right++ * size_info, Color::Grey(meme - 55.f), FONT_LEFT, "R%d", resolver_info.resolving_method);
	}
}

bool c_dormant_esp::valid_sound(SndInfo_t& sound)
{
	// Use only server dispatched sounds.
	//if (!sound.m_bFromServer)
	//	return false;

	// We don't want the sound to keep following client's predicted origin.
	for (int iter = 0; iter < m_utlvecSoundBuffer.Count(); iter++)
	{
		SndInfo_t& cached_sound = m_utlvecSoundBuffer[iter];
		if (cached_sound.m_nGuid == sound.m_nGuid)
		{
			return false;
		}
	}

	return true;
}