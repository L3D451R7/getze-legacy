#pragma once
#include "sdk.hpp"
#include <deque>
#include <array>
#include <unordered_map>

#define TIME_TO_TICKS( dt ) ( (int)( 0.5f + (float)(dt) / Source::m_pGlobalVars->interval_per_tick ) )
#define TICKS_TO_TIME( t )  ( Source::m_pGlobalVars->interval_per_tick * ( t ) )
#define ROUND_TO_TICKS( t ) ( Source::m_pGlobalVars->interval_per_tick * TIME_TO_TICKS( t ) )

enum tickRecordPriority : int
{
	RECORD_NORMAL = 0,
	RECORD_LBY_PREDICTION = 1,
	RECORD_LBY_UPDATE = 2,
	RECORD_UNCHOKE = 3,
	RECORD_NON_CHOKED_SHOT = 4,
	RECORD_MOVING = 5,
};

class C_Tickrecord
{
public:
	explicit C_Tickrecord() : entity_flags(0), animated(false), simulation_time(0), lower_body_yaw(0), type(0)
	{
	}

	~C_Tickrecord()
	{
		//free(anim_layers);
	}

	/*
	* reset
	* Resets the tick record
	*/
	void reset()
	{
		if (data_filled)
			return;

		origin.clear();
		velocity.clear();
		abs_origin.clear();
		object_mins.clear();
		object_maxs.clear();
		anim_velocity.clear();

		eye_angles.clear();
		abs_eye_angles.clear();

		entity_flags = 0;
		ientity_flags = 0;
		bones_count = 0;
		servertick = 0;
		resolver_method = 0;

		simulation_time = 0.f;
		lower_body_yaw = 0.f;
		simulation_time_old = 0.f;
		type = RECORD_NORMAL;
		duck_amt = 0.f;
		lby_delta = 0.f;

		fill(begin(pose_paramaters), end(pose_paramaters), 0.f);
		memset(anim_layers, 0, 56 * 14);

		data_filled = shot_this_tick = dormant = animated = false;
	}

	matrix3x4_t matrixes[128];
	C_AnimationLayer anim_layers[13];
	std::array<float, 24> pose_paramaters;

	Vector origin;
	Vector abs_origin;
	Vector anim_velocity;
	Vector velocity;
	Vector object_mins;
	Vector object_maxs;

	QAngle eye_angles;
	QAngle abs_eye_angles;

	int entity_flags;
	int	ientity_flags;
	int bones_count;
	int resolver_method;

	int type = RECORD_NORMAL;
	int servertick = 0;

	float lby_delta;
	float duck_amt;
	float simulation_time;
	float simulation_time_old;
	float shot_time;
	float lower_body_yaw;

	bool data_filled = false;
	bool animated = false;
	bool shot_this_tick = false;
	bool dormant = true;
};

/*
* C_Simulation_Data
* Used when simulating ticks for a player (useful for storing temp data)
*/
class C_Simulationdata
{
public:
	C_Simulationdata() : entity(nullptr), on_ground(false)
	{
	}

	~C_Simulationdata()
	{
	}

	Vector origin;
	Vector velocity;

	C_BasePlayer* entity;

	float simtime;

	bool on_ground;
	bool extrapolation = false;

	bool data_filled = false;
};

class c_player_records
{
public:
	std::deque<C_Tickrecord> tick_records;
	C_Tickrecord restore_record;
	C_Tickrecord* best_record = nullptr;
	Vector last_render_origin;
	bool matrix_valid = false;

	void reset(bool sdox)
	{
		matrix_valid = false;
		restore_record.reset();
		tick_records.clear();
	}
};

class c_lagcomp
{
public:
	void get_interpolation();
	void store_record_data(C_BasePlayer * entity, C_Tickrecord * record_data, bool backup = false);
	void apply_record_data(C_BasePlayer * entity, C_Tickrecord * record_data, bool backup = false);
	bool extrapolate(C_BasePlayer * ent, const int ticks, C_Tickrecord * extrapolated_record);
	bool is_time_delta_too_large(C_Tickrecord * wish_record);
	void update_player_record_data(C_BasePlayer * entity);
	C_Tickrecord* c_lagcomp::find_priority_record(c_player_records* log);
	void start_position_adjustment();
	void backup_players(bool restore = false);
	void update_animations_data(C_BasePlayer * m_player);
	void store_records();
	bool can_backtrack_this_tick(C_Tickrecord* record, C_Tickrecord* previous, c_player_records* log);
	void reset();

	c_player_records records[64];
};

struct _shotinfo
{
	_shotinfo(C_BasePlayer* target, matrix3x4_t *mx, const Vector& eyepos, const Vector& hitbox, c_player_records* record, C_Tickrecord trecord, const int& hitboxid, const float& predtime, const float& sprd)
	{
		memcpy(_matrix, mx, sizeof(matrix3x4_t) * 128);
		_eyepos = eyepos;
		_target = target;
		_hitbox = hitbox;
		_hitboxid = hitboxid;

		_record = record;
		_tickrecord = trecord;
		_headshot = false;

		_avg_impact_time = predtime;
		_spread = sprd;
		_hurt_called = _impact_called = _printed = false;
		_impact_pos = { 0,0,0 };
	}

	matrix3x4_t _matrix[128];
	Vector _eyepos = { 0,0,0 };
	Vector _hitbox = { 0,0,0 };
	int _hitboxid = -1;
	C_BasePlayer* _target = nullptr;
	c_player_records* _record = nullptr;
	C_Tickrecord _tickrecord;
	float _avg_impact_time = 0.f;
	bool _hurt_called = false;
	bool _headshot = false;
	bool _impact_called = false;
	float _spread = 0.f;
	Vector _impact_pos = { 0,0,0 };
	bool _printed = false;
};