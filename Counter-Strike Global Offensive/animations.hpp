#pragma once
#include "sdk.hpp"
#include <deque>
#include <array>
#include <unordered_map>

class C_Animrecord
{
public:
	explicit C_Animrecord() : entity_flags(0), animated(false), simulation_time(0), lower_body_yaw(0), lag(0)
	{
	}

	~C_Animrecord()
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
		lag = 0;

		simulation_time = 0.f;
		lower_body_yaw = 0.f;
		simulation_time_old = 0.f;
		duck_amt = 0.f;
		lby_delta = 0.f;

		fill(begin(pose_paramaters), end(pose_paramaters), 0.f);
		memset(server_layers, 0, 56 * 14);

		data_filled = shot_this_tick = dormant = animated = false;
	}

	matrix3x4_t matrixes[128];
	C_AnimationLayer server_layers[13];
	C_AnimationLayerShort layers[13];
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
	int lag;

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


class c_anim_records
{
public:
	std::deque<C_Animrecord> anim_records;
	Vector last_render_origin = { 0,0,0 };
	bool matrix_valid = false;

	void reset(bool sdox)
	{
		last_render_origin.clear();
	}
};

class c_animsystem
{
public:
	void run_velocity_calculation(C_BasePlayer* m_player, C_Animrecord* m_record, C_Animrecord* m_previous, c_anim_records* lag);
	bool update_animations_data(C_BasePlayer* m_player);
	void reset();

	c_anim_records records[64];
};