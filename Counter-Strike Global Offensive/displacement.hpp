#pragma once

#include "sdk.hpp"

namespace Engine::Displacement
{

	bool Create();
	void Destroy();

	namespace C_BaseEntity
	{
		extern int m_MoveType;
		extern int m_rgflCoordinateFrame;
	}
	namespace DT_BaseAnimating
	{
		extern int m_nForceBone;
		extern int m_bClientSideAnimation;
		extern int m_nSequence;
		extern int m_flCycle;
		extern int m_nHitboxSet;
	}
	namespace DT_BaseEntity
	{
		extern int m_iTeamNum;
		extern int m_vecOrigin;
	}
	namespace DT_BaseCombatCharacter
	{
		extern int m_hActiveWeapon;
	}
	namespace C_BasePlayer
	{
		extern int m_pCurrentCommand;
	}
	namespace DT_BasePlayer
	{
		extern int m_aimPunchAngle;
		extern int m_viewPunchAngle;
		extern int m_vecViewOffset;
		extern int m_vecVelocity;
		extern int m_vecBaseVelocity;
		extern int m_flFallVelocity;
		extern int m_lifeState;
		extern int m_nTickBase;
		extern int m_iHealth;
		extern int m_fFlags;
		extern int m_flSimulationTime;
	}
	namespace DT_CSPlayer
	{
		extern int m_flLowerBodyYawTarget;
		extern int m_angEyeAngles;
		extern int m_bIsScoped;
		extern int m_flDuckAmount;
		extern int m_flPoseParameter;
		extern int m_bHasHelmet;
	}
	namespace DT_BaseCombatWeapon
	{
		extern int m_flNextPrimaryAttack;
		extern int m_flNextSecondaryAttack;
		extern int m_hOwner;
		extern int m_iClip1;
		extern int m_iItemDefinitionIndex;
	}
	namespace DT_WeaponCSBase
	{
		extern int m_flRecoilIndex;
	}
	namespace Data
	{
		extern CGlowObjectManager* m_pGlowManager;
		extern std::uintptr_t m_uMoveHelper;
		extern DWORD* m_uTraceLineIgnoreTwoEntities;
		extern std::uintptr_t m_uUpdateClientSideAnimation;
		extern DWORD m_uClipTracePlayers;
		extern std::uintptr_t m_uClientState;
		extern std::uintptr_t m_uInput;
		extern std::uintptr_t m_uGlobalVars;
		extern std::uintptr_t m_uPredictionRandomSeed;
		extern std::uintptr_t m_uPredictionPlayer;
		extern IViewRenderBeams* m_uClientBeams;
	}
	namespace Function
	{
		extern std::uintptr_t m_uRandomSeed;
		extern std::uintptr_t m_uRandomFloat;
		extern std::uintptr_t m_uRandomInt;
	}

}
