#pragma once

class c_player_animstate_rebuilt
{
	void setup_velocity(CCSGOPlayerAnimState * state, C_BasePlayer * player);
	float last_time_lby_updated[128];
	float last_lby[128];
	bool  balance_adjust[128];
};