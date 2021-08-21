#pragma once
#include "sdk.hpp"

class c_player_records;

class C_Hitbox
{
public:
	int hitboxID;
	bool isOBB;
	Vector mins;
	Vector maxs;
	float radius;
	int bone;
};

class c_aimbot
{
public:
	virtual Vector get_hitbox(C_BasePlayer * ent, int ihitbox);
	virtual void draw_capsule(C_BasePlayer * ent, int ihitbox);
	virtual Vector get_hitbox(C_BasePlayer * ent, int ihitbox, matrix3x4_t mat[]);
	virtual bool hit_chance(QAngle angle, C_BasePlayer *ent, float chance);
	virtual float can_hit(int hitbox, C_BasePlayer * Entity, Vector position, matrix3x4_t mx[], bool check_center = false);
	virtual void DrawHitchanceLines();
	//virtual bool TraceHitchance(Vector & aechse, C_BasePlayer * ent, float chance);
	virtual void visualise_hitboxes(C_BasePlayer * entity, matrix3x4_t * mx, Color color, float time);
	virtual void autostop(CUserCmd * cmd, bool & send_packet, C_WeaponCSBaseGun * local_weapon/*, C_BasePlayer * best_player, float dmg, bool hitchanced*/);
	virtual float check_wall(C_WeaponCSBaseGun * local_weapon, Vector startpoint, Vector direction, C_BasePlayer* entity);
	virtual void get_hitbox_data(C_Hitbox* rtn, C_BasePlayer* ent, int ihitbox, matrix3x4_t* matrix);
	virtual bool work(CUserCmd* cmd, bool &send_packet);
	//virtual void ClipTraceToPlayers(Vector & absStart, Vector absEnd, unsigned int mask, ITraceFilter * filter, CGameTrace * tr);
	virtual bool knifebot_work(CUserCmd * cmd, bool & send_packet);

	virtual bool knifebot_target();

	float r8cock_time = 0.f;
	bool is_cocking = false;
	bool lastAttacked = false;
	int historytick = -1;
	int m_nBestIndex = -1;
	float m_nBestDist = -1;
	Vector m_nAngle;
	C_BasePlayer* pBestEntity;
};