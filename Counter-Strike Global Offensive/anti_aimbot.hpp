#pragma once
#include "sdk.hpp"

class c_antiaimbot
{
public:
	virtual void freestand(C_BasePlayer * ent, float & new_angle);
	virtual void DoLBY(CUserCmd * cmd, bool * send_packet);
	virtual void change_angles(CUserCmd* cmd, bool &send_packet);
	virtual void do_exloits(CUserCmd * cmd, bool& send_packet);
	virtual void simulate_lags(CUserCmd* cmd, bool *send_packet);
	virtual void work(CUserCmd * cmd, bool * send_packet);
	virtual float some_func(float target, float value, float speed);

	bool skip_lags_this_tick;
	Vector last_sent_origin;
	matrix3x4_t last_sent_matrix[128];
	Vector last_pre_origin;
	float last_sent_simtime;
	QAngle last_real_angle;
	QAngle last_fake_angle;
	QAngle last_sent_angle;
	float enable_delay;

	bool drop = false;

	QAngle last_nonlby_angle;

	int shift_ticks = 0;
	int piska = 0;
	bool do_tickbase = 0;
	bool did_tickbase = 0;

	float m_next_lby_update_time = 0.f, m_last_lby_update = 0.f, m_last_attempted_lby = 0.f;
	bool m_will_lby_update = false;
	int packets_choked = 0;
	bool unchoke = false;
};