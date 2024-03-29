#pragma once

#include "sdk.hpp"

namespace Engine
{

struct PlayerData
{
	QAngle m_aimPunchAngle = {};
	QAngle m_aimPunchAngleVel = {};
	Vector m_vecVelocity = {};
	Vector m_vecViewOffset = {};
	int command_number = {};
	bool filled = false;
	//float m_flFallVelocity = 0.0f;
};

class Prediction : public Core::Singleton<Prediction>
{
public:
	void Begin( CUserCmd* cmd );
	void End();

	int GetFlags();
	CUserCmd GetPrevCmd();
	Vector GetVelocity();

	void RestoreNetvars(int cmdnr);
	void DetectPredError();
	void OnRunCommand( C_BasePlayer* player, int cmdnr);

private:
	CUserCmd* m_pCmd = nullptr;
	CUserCmd* m_pPrevCmd = nullptr;

	C_CSPlayer* m_pPlayer = nullptr;
	C_WeaponCSBaseGun* m_pWeapon = nullptr;

	int m_fFlags = 0;
	Vector m_vecVelocity = Vector::Zero;

	float m_flCurrentTime = 0.0f;
	float m_flFrameTime = 0.0f;
	bool m_bFirstTimePredicted;
	bool m_bInPrediction;
	int m_nTickBase = 0;

	PlayerData m_Data[150] = { };
};

}