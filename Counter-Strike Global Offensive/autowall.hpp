#pragma once

#define DAMAGE_NO		0
#define DAMAGE_EVENTS_ONLY	1	
#define DAMAGE_YES		2
#define DAMAGE_AIM		3

#define CHAR_TEX_CONCRETE	'C'			// texture types
#define CHAR_TEX_METAL		'M'
#define CHAR_TEX_DIRT		'D'
#define CHAR_TEX_VENT		'V'
#define CHAR_TEX_GRATE		'G'
#define CHAR_TEX_TILE		'T'
#define CHAR_TEX_SLOSH		'S'
#define CHAR_TEX_WOOD		'W'
#define CHAR_TEX_COMPUTER	'P'
#define CHAR_TEX_GLASS		'Y'
#define CHAR_TEX_FLESH		'F'
#define CHAR_TEX_SNOW		'N'
#define CHAR_TEX_PLASTIC	'L'
#define CHAR_TEX_CARDBOARD	'U'

#define CHAR_TEX_STEAM_PIPE		11

class c_autowall
{
public:
	void TraceLine(Vector& absStart, Vector& absEnd, unsigned int mask, C_BasePlayer* ignore, CGameTrace* ptr);
	void ScaleDamage(CGameTrace& enterTrace, weapon_info* weaponData, float& currentDamage);
	bool TraceToExit(trace_t& enter_trace, trace_t& exit_trace, const Vector start_position, const Vector direction, const bool is_local);
	bool HandleBulletPenetration(weapon_info* weaponData, trace_t& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float shit, bool pskip = false);
	void FixTraceRay(Vector end, Vector start, trace_t* oldtrace, C_BasePlayer* ent);
	void ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, uint32_t mask, ITraceFilter* filter, trace_t* tr);
	bool FireBullet(Vector eyepos, C_WeaponCSBaseGun* pWeapon, Vector& direction, float& currentDamage, C_BasePlayer* ignore, C_BasePlayer* to_who = nullptr, int target_hitbox = 0);
	float CanHit(Vector& vecEyePos, Vector& point);
	float CanHit(Vector& vecEyePos, Vector& point, C_BasePlayer* ignore_ent, C_BasePlayer* start_ent, int target_hitbox);
};