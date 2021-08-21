#pragma once
#include "sdk.hpp"
#include <deque>
#include <array>
#include <unordered_map>

#define TIME_TO_TICKS( dt ) ( (int)( 0.5f + (float)(dt) / Source::m_pGlobalVars->interval_per_tick ) )
#define TICKS_TO_TIME( t )  ( Source::m_pGlobalVars->interval_per_tick * ( t ) )
#define ROUND_TO_TICKS( t ) ( Source::m_pGlobalVars->interval_per_tick * TIME_TO_TICKS( t ) )

enum TickrecordType
{
	RECORD_NONE = -1,
	RECORD_NORMAL = 0,
	RECORD_SHOT = 1,
	RECORD_PRIORITY = 2
};

class C_Tickrecord
{
public:
	explicit C_Tickrecord() : sequence(0), entity_flags(0), simulation_time(0), lower_body_yaw(0), cycle(0), type(0)
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
		hitbox_positon.clear();
		anim_velocity.clear();
		force.clear();

		eye_angles.clear();
		abs_eye_angles.clear();

		sequence = 0;
		entity_flags = 0;
		tick_count = 0;
		ientity_flags = 0;
		bones_count = 0;
		skin = 0;
		body = 0;
		resolver_method = 0;

		simulation_time = 0.f;
		lower_body_yaw = 0.f;
		simulation_time_old = 0.f;
		type = RECORD_NONE;
		cycle = 0.f;
		duck_amt = 0.f;
		lby_delta = 0.f;

		fill(begin(pose_paramaters), end(pose_paramaters), 0.f);
		memset(anim_layers, 0, 56 * 14);

		data_filled = shot_this_tick = dormant = false;
	}

	Vector origin;
	Vector abs_origin;
	Vector anim_velocity;
	Vector velocity;
	Vector object_mins;
	Vector object_maxs;
	Vector hitbox_positon;
	Vector force;

	QAngle eye_angles;
	QAngle abs_eye_angles;

	int sequence;
	int entity_flags;
	int	ientity_flags;
	int tick_count;
	int bones_count;
	int skin;
	int body;
	int resolver_method;

	float lby_delta;
	float duck_amt;
	float simulation_time;
	float simulation_time_old;
	float shot_time;
	float lower_body_yaw;
	float cycle;

	std::array<float, 24> pose_paramaters;
	C_AnimationLayer anim_layers[15];
	matrix3x4_t matrixes[128];

	int type = RECORD_NORMAL;

	bool data_filled = false;
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

	C_BasePlayer* entity;

	Vector origin;
	Vector velocity;
	float simtime;

	bool on_ground;
	bool extrapolation = false;

	bool data_filled = false;
};

class c_player_records
{
public:
	std::deque<C_Tickrecord> m_Tickrecords;
	//C_Tickrecord m_AnimationRecord;
	bool being_lag_compensated;
	int backtrack_ticks;
	C_Tickrecord restore_record;
	int tick_count;
	int type;
	matrix3x4_t matrix[128];
	Vector hitbox_position;
	bool matrix_valid = false;

	void reset(bool sdox)
	{
		tick_count = -1;
		hitbox_position.clear();
		type = 0;

		memset(matrix, 0, sizeof(matrix3x4_t) * 128);
		//matrix[8].Invalidate();
		matrix_valid = false;

		being_lag_compensated = false;
		backtrack_ticks = 0;

		restore_record.reset();

		//if (!m_Tickrecords.empty()) {
			//auto front = m_Tickrecords.front();
			m_Tickrecords.clear();
			//if (!sdox)
			//	m_Tickrecords.emplace_front(front);
		//}
	}
};

class c_lagcomp
{
public:
	void get_interpolation();
	void store_record_data(C_BasePlayer * entity, C_Tickrecord * record_data);
	void apply_record_data(C_BasePlayer * entity, C_Tickrecord * record_data, bool backup = false);
	bool extrapolate(C_BasePlayer * ent, const int ticks, C_Tickrecord * extrapolated_record);
	void accelerate_velocity(C_BasePlayer * player, Vector & velocity, Vector new_velocity_angle, Vector old_velocity_angle);
	bool is_time_delta_too_large(C_Tickrecord * wish_record);
	bool is_time_delta_too_large(const float & simulation_time);
	int start_lag_compensation(C_BasePlayer * entity, int wish_tick, C_Tickrecord * output_record);
	void update_player_record_data(C_BasePlayer * entity);
	void start_position_adjustment();
	//bool test_and_apply_record(C_BasePlayer * _entity, c_player_records * player_record, C_Tickrecord * restore_record, C_Tickrecord * corrected_record, int tick_count, int recordidx);
	void start_position_adjustment(C_BasePlayer * entity);
	void finish_position_adjustment();
	void fix_netvar_compression(C_BasePlayer * player);
	void fix_poses(C_BasePlayer* m_player);
	void update_animations_data(C_BasePlayer * m_player);
	void finish_position_adjustment(C_BasePlayer * entity);
	void update_fake_animations();
	void store_records();
	void reset();

	void set_interpolation_flags(C_BasePlayer* entity, int flag)
	{
		auto var_map = reinterpret_cast<uintptr_t>(entity) + 36; // tf2 = 20
		auto var_map_list_count = *reinterpret_cast<int*>(var_map + 20);

		for (auto index = 0; index < var_map_list_count; index++)
			*reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(var_map) + index * 12) = flag;
	}

	c_player_records records[128];
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