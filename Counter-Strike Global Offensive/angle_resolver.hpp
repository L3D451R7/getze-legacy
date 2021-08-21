#pragma once
#include "sdk.hpp"
#include <deque>
#include <array>

class resolver_records
{
public:
	resolver_records() { Reset(); }

	//~PlayerResolverRecord() { Reset(); }

	//PlayerResolverRecord(CBaseEntity* entity) { Record(entity); }

	void Reset()
	{
		/*last_balance_adjust_trigger_time = 0.f;
		just_stopped_delta = 0.f;
		last_time_down_pitch = 0.f;
		last_time_moving = 0.f;
		last_moving_lby = 0.0000001337f;*/

		is_dormant = false;
		/*is_last_moving_lby_valid = false, is_fakewalking = false, is_just_stopped = false, is_breaking_lby = false;*/
		//is_balance_adjust_triggered = false, is_balance_adjust_playing = false, last_moving_lby_can_be_invalid = false;
		//did_lby_flick = false;
		resolved = false;
		fakeangles = false;
		resolving_method = 0;
		//last_time_lby_updated = 0;
		//lby_update_tick = 0;
		previous_angles = QAngle::Zero;
		last_real_angles = QAngle::Zero;
		latest_angles = QAngle::Zero;
		latest_angles_when_faked = QAngle::Zero;
		latest_fake = QAngle::Zero;

		m_flRate = 0.0f;
		m_flServerTorso = 0.0f;

		m_flClientRate = 0.0f;
		m_flLastFlickTime = FLT_MAX;
		m_iSide = -1;
		m_iCurrentSide = -1;
		last_time_balanced = 0.0f;
		b_bRateCheck = false;
		had_fake = false;
		last_time_standed = 0.f;
		last_time_choked = 0.f;
		last_time_moved = 0.f;
		last_shot_time = 0.f;
		previous_rotation = 0.f;
		//for (auto i = 0; i < 10; i++)
		//	missed_shots[i] = 0;

		tick_delta = 0.0f;
		lby_delta = 0.0f;
		latest_delta_used = 0.0f;
		last_speed = 0.0f;
		last_lby = 0.0f;

		is_balancing = false;
		did_shot_this_tick = false;
		did_lby_update_fail = false;
		force_resolving = false;
		prev_pose = 0.f;
		prev_delta = FLT_MAX;
		/*did_hit_low_delta = false;
		did_hit_max_delta = false;
		did_hit_no_delta = false;*/
		balance_adjust = false;
		inverse_lby = false;
		did_hit_inversed_lby = false;
		did_lby_flick = false;
		next_lby_time = FLT_MAX;
		prev_lby_time = FLT_MAX;
		prev_lby = FLT_MAX;
		move_lby = FLT_MAX;
		saved_off_delta = FLT_MAX;
		something = 0.f;
		pre_shot_pitch = FLT_MAX;
		lby_deltas.clear();
		origin.clear();
		prev_flags = 0;
		fakewalk = false;
		last_move_lby_valid = false;
		never_saw_movement = true;
		was_moving = false;
		old_lby = 0.f;

		deltas_count = FLT_MAX;
		total_deltas = FLT_MAX;

		memset(missed_shots, 0, sizeof(int) * 10);
	}

	struct AntiFreestandingRecord
	{
		int right_damage = 0, left_damage = 0;
		float right_fraction = 0.f, left_fraction = 0.f;

		void reset()
		{
			right_damage = 0;
			left_damage = 0;
			right_fraction = 0.f;
			left_fraction = 0.f;
		}
	};

	QAngle previous_angles;
	Vector origin;
	float pre_shot_pitch;
	float prev_pose;
	QAngle last_real_angles;
	QAngle latest_angles;
	QAngle latest_fake;
	QAngle latest_angles_when_faked;
	float previous_rotation;
	float last_time_balanced;
	bool  balance_adjust;
	float last_time_choked;
	bool  had_fake;
	float last_time_standed;
	float last_time_moved;
	float last_simtime;
	float resolved_yaw;

	float latest_delta_used;

	C_AnimationLayer client_anim_layers[15];

	C_AnimationLayer server_anim_layers[15];
	AntiFreestandingRecord freestanding_record;
	float last_shot_time = 0.0f;
	float m_flServerTorso = 0.0f;

	float tick_delta = 0.0f;
	float lby_delta = 0.0f;
	float last_velocity_angle = 0.0f;
	float last_speed = 0.0f;
	float last_lby = 0.0f;
	float old_lby = 0.f;

	bool inverse_lby = false;
	bool did_hit_inversed_lby = false;
	bool fakewalk = false;

	//float next_predicted_lby_update;
	//float last_lby_update;
	//bool did_predicted_lby_flick;

	/*float last_time_down_pitch;
	float last_time_lby_updated;
	int lby_update_tick;*/
	bool did_shot_this_tick = false;
	bool is_balancing = false;
	bool did_lby_update_fail = false;

	bool force_resolving = false;

	float m_flRate = 0.0f;

	float m_flClientRate = 0.0f;
	float prev_delta = FLT_MAX;
	float m_flLastFlickTime = FLT_MAX;
	int m_iSide = -1;
	int m_iCurrentSide = -1;
	bool b_bRateCheck = false;

	int resolving_method;
	int missed_shots[11];

	/*bool lby_update_locked;*/
	bool is_dormant, resolved;
	//bool /*is_last_moving_lby_valid, last_moving_lby_can_be_invalid, is_just_stopped, is_getting_right_delta,*/is_breaking_lby;
	/*bool is_fakewalking;
	bool is_balance_adjust_triggered, is_balance_adjust_playing;*/
	bool did_lby_flick;
	float next_lby_time;
	float something;
	float prev_lby_time;
	float prev_lby;
	float move_lby;
	bool last_move_lby_valid;
	bool was_moving;
	bool never_saw_movement;
	float saved_off_delta;
	float avg_delta;
	float total_deltas;
	int deltas_count;
	int prev_flags;
	bool fakeangles;
	std::deque<float> lby_deltas;
	//bool did_hit_low_delta;
	//bool did_hit_max_delta;
	//bool did_hit_no_delta;
};

class c_resolver
{
public:
	class CLBYRECORD;
	//virtual bool freestanding_resolver(C_BasePlayer * entity, float & yaw);
	bool has_fake(C_BasePlayer * entity);
	bool compare_delta(float v1, float v2, float Tolerance);
	void on_real_angle_arrive(C_BasePlayer * m_player, resolver_records * resolve_data, float real_yaw);
	void on_new_data_arrive(C_BasePlayer * m_player, resolver_records * resolve_data, CCSGOPlayerAnimState * state);
	bool resolve_freestand(C_BasePlayer * m_player, resolver_records * resolve, CCSGOPlayerAnimState * state, float &step);
	void add_new_lby_delta_record(resolver_records * resolve, float record);
	bool get_lby_delta_record(C_BasePlayer * m_player, resolver_records * resolve, float & returnval);
	bool get_yaw_by_walls(C_BasePlayer * entity, resolver_records * resolve, float & yaw);
	void resolve(C_BasePlayer * entity, int &history, int &chocked_ticks);
	void work();

	resolver_records player_resolver_records[128];

	class CLBYRECORD {
	public:
		CLBYRECORD(C_BasePlayer* entity) { store(entity); }
		CLBYRECORD() { speed = lby = -1.f; }
		float speed;
		float lby;
		void store(C_BasePlayer* entity);
	};

	std::deque<CLBYRECORD> move_logs[128];
};