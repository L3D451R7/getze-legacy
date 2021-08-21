#pragma once

#include "sdk.hpp"

namespace Engine
{

class Movement : public Core::Singleton<Movement>
{
public:
	float get_move_angle(float speed);
	bool get_closest_plane(Vector * plane);
	bool will_hit_obstacle_in_future(float predict_time, float step);
	void circle_strafe(CUserCmd * cmd, float * circle_yaw);
	void predict_velocity(Vector * velocity);
	void RotateMovement(CUserCmd * cmd, float yaw);
	void quick_stop(CUserCmd * cmd);
	void Begin( CUserCmd* cmd, bool& send_packet);
	void Fix_Movement(CUserCmd* cmd, QAngle original_angles);
	void End(CUserCmd* cmd);

	QAngle m_qRealAngles = {};
	QAngle m_qAngles = { };
	QAngle m_qAnglesView = { };
	QAngle m_qAnglesLast = { };

private:
	CUserCmd* m_pCmd = nullptr;
	C_CSPlayer* m_pPlayer = nullptr;
};

}