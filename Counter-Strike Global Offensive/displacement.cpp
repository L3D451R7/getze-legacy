#include "displacement.hpp"
#include "prop_manager.hpp"

namespace Engine::Displacement
{

	bool Create()
	{
		auto& pPropManager = PropManager::Instance();

		C_BaseEntity::m_MoveType = pPropManager->GetOffset("DT_BaseEntity", "m_nRenderMode") + 1;
		C_BaseEntity::m_rgflCoordinateFrame = pPropManager->GetOffset("DT_BaseEntity", "m_CollisionGroup") - 48;

		DT_BaseEntity::m_iTeamNum = pPropManager->GetOffset("DT_BaseEntity", "m_iTeamNum");
		DT_BaseEntity::m_vecOrigin = pPropManager->GetOffset("DT_BaseEntity", "m_vecOrigin");

		DT_BaseCombatCharacter::m_hActiveWeapon = pPropManager->GetOffset("DT_CSPlayer", "m_hActiveWeapon");

		auto m_hConstraintEntity = pPropManager->GetOffset("DT_BasePlayer", "m_hConstraintEntity");

		C_BasePlayer::m_pCurrentCommand = (m_hConstraintEntity - 0xC);//Memory::Scan( "client.dll", "89 ?? ?? ?? ?? ?? E8 ?? ?? ?? ?? 85 FF 75" ) + 2 );

		DT_BasePlayer::m_aimPunchAngle = pPropManager->GetOffset("DT_BasePlayer", "m_aimPunchAngle");
		DT_BasePlayer::m_viewPunchAngle = pPropManager->GetOffset("DT_BasePlayer", "m_viewPunchAngle");
		DT_BasePlayer::m_vecViewOffset = pPropManager->GetOffset("DT_BasePlayer", "m_vecViewOffset[0]");
		DT_BasePlayer::m_vecVelocity = pPropManager->GetOffset("DT_BasePlayer", "m_vecVelocity[0]");
		DT_BasePlayer::m_vecBaseVelocity = pPropManager->GetOffset("DT_BasePlayer", "m_vecBaseVelocity");
		DT_BasePlayer::m_flFallVelocity = pPropManager->GetOffset("DT_BasePlayer", "m_flFallVelocity");
		DT_BasePlayer::m_lifeState = pPropManager->GetOffset("DT_CSPlayer", "m_lifeState");
		DT_BasePlayer::m_nTickBase = pPropManager->GetOffset("DT_BasePlayer", "m_nTickBase");
		DT_BasePlayer::m_iHealth = pPropManager->GetOffset("DT_BasePlayer", "m_iHealth");
		DT_BasePlayer::m_fFlags = pPropManager->GetOffset("DT_BasePlayer", "m_fFlags");
		DT_BasePlayer::m_flSimulationTime = pPropManager->GetOffset("DT_BasePlayer", "m_flSimulationTime");

		DT_CSPlayer::m_angEyeAngles = pPropManager->GetOffset("DT_CSPlayer", "m_angEyeAngles[0]");
		DT_CSPlayer::m_bIsScoped = pPropManager->GetOffset("DT_CSPlayer", "m_bIsScoped");
		DT_CSPlayer::m_flDuckAmount = pPropManager->GetOffset("DT_CSPlayer", "m_flDuckAmount");
		DT_CSPlayer::m_flPoseParameter = pPropManager->GetOffset("DT_CSPlayer", "m_flPoseParameter");
		DT_CSPlayer::m_bHasHelmet = pPropManager->GetOffset("DT_CSPlayer", "m_bHasHelmet");
		DT_CSPlayer::m_flLowerBodyYawTarget = pPropManager->GetOffset("DT_CSPlayer", "m_flLowerBodyYawTarget");

		DT_BaseAnimating::m_nForceBone = pPropManager->GetOffset("DT_BaseAnimating", "m_nForceBone");
		DT_BaseAnimating::m_bClientSideAnimation = pPropManager->GetOffset("DT_BaseAnimating", "m_bClientSideAnimation");
		DT_BaseAnimating::m_nSequence = pPropManager->GetOffset("DT_BaseAnimating", "m_nSequence");
		DT_BaseAnimating::m_nHitboxSet = pPropManager->GetOffset("DT_BaseAnimating", "m_nHitboxSet");
		DT_BaseAnimating::m_flCycle = pPropManager->GetOffset("DT_BaseAnimating", "m_flCycle");

		DT_BaseCombatWeapon::m_flNextPrimaryAttack = pPropManager->GetOffset("DT_BaseCombatWeapon", "m_flNextPrimaryAttack");
		DT_BaseCombatWeapon::m_flNextSecondaryAttack = pPropManager->GetOffset("DT_BaseCombatWeapon", "m_flNextSecondaryAttack");
		DT_BaseCombatWeapon::m_hOwner = pPropManager->GetOffset("DT_BaseCombatWeapon", "m_hOwner");
		DT_BaseCombatWeapon::m_iClip1 = pPropManager->GetOffset("DT_BaseCombatWeapon", "m_iClip1");
		DT_BaseCombatWeapon::m_iItemDefinitionIndex = pPropManager->GetOffset("DT_BaseCombatWeapon", "m_iItemDefinitionIndex");

		DT_WeaponCSBase::m_flRecoilIndex = pPropManager->GetOffset("DT_WeaponCSBase", "m_flRecoilIndex");

		//Data::m_pGlowManager = *reinterpret_cast<CGlowObjectManager**>(Memory::Scan("client.dll", "0F 11 05 ? ? ? ? 83 C8 01") + 3);
		//Data::m_uTraceLineIgnoreTwoEntities = *(DWORD**)(Memory::Scan("client.dll", "53 8B DC 83 EC ? 83 E4 ? 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC 8C"));
		////Data::m_uClipTracePlayers = (DWORD)(Memory::Scan("client.dll", "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10"));
		//Data::m_uClientState = **(std::uintptr_t**)(Memory::Scan("engine.dll", "A1 ? ? ? ? 8B 80 ? ? ? ? C3") + 1);//8B F1 E8 ? ? ? ? 8B 8E ? ? ? ? 3B 8E ? ? ? ?
		//Data::m_uMoveHelper = **(std::uintptr_t**)(Memory::Scan("client.dll", "8B 0D ?? ?? ?? ?? 8B 45 ?? 51 8B D4 89 02 8B 01") + 2);
		//Data::m_uInput = *(std::uintptr_t*)(Memory::Scan("client.dll", "B9 ?? ?? ?? ?? F3 0F 11 04 24 FF 50 10") + 1);
		////Data::m_uGlobalVars = **( std::uintptr_t** )( Memory::Scan( "client.dll", "A3 ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 6A" ) + 1 );
		//Data::m_uPredictionRandomSeed = *(std::uintptr_t*)(Memory::Scan("client.dll", "8B 0D ?? ?? ?? ?? BA ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 C4 04") + 2); // 8B 0D ? ? ? ? BA ? ? ?? E8 ? ? ? ? 83 C4 04
		//Data::m_uUpdateClientSideAnimation = (DWORD)(Memory::Scan("client.dll", "55 8B EC 51 56 8B F1 80 BE ? ? 00 00 00 74 36"));
		//Data::m_uPredictionPlayer = *(std::uintptr_t*)(Memory::Scan("client.dll", "89 ?? ?? ?? ?? ?? F3 0F 10 48 20") + 2);
		Data::m_uClientBeams = *(IViewRenderBeams**)(Memory::Scan(cheat::main::clientdll, "B9 ? ? ? ? A1 ? ? ? ? FF 10 A1 ? ? ? ? B9") + 1);

		auto image_vstdlib = GetModuleHandleA("vstdlib.dll");

		Function::m_uRandomSeed = (std::uintptr_t)(GetProcAddress(image_vstdlib, "RandomSeed"));
		Function::m_uRandomFloat = (std::uintptr_t)(GetProcAddress(image_vstdlib, "RandomFloat"));
		Function::m_uRandomInt = (std::uintptr_t)(GetProcAddress(image_vstdlib, "RandomInt"));

		return true;
	}

	void Destroy()
	{

	}

	namespace C_BaseEntity
	{
		int m_MoveType = 0;
		int m_rgflCoordinateFrame = 0;
	}
	namespace DT_BaseAnimating
	{
		int m_nForceBone = 0;
		int m_bClientSideAnimation = 0;
		int m_nSequence = 0;
		int m_flCycle = 0;
		int m_nHitboxSet = 0;
	}
	namespace DT_BaseEntity
	{
		int m_iTeamNum = 0;
		int m_vecOrigin = 0;
	}
	namespace DT_BaseCombatCharacter
	{
		int m_hActiveWeapon = 0;
	}
	namespace C_BasePlayer
	{
		int m_pCurrentCommand = 0;
	}
	namespace DT_BasePlayer
	{
		int m_aimPunchAngle = 0;
		int m_viewPunchAngle = 0;
		int m_vecViewOffset = 0;
		int m_vecVelocity = 0;
		int m_vecBaseVelocity = 0;
		int m_flFallVelocity = 0;
		int m_lifeState = 0;
		int m_nTickBase = 0;
		int m_iHealth = 0;
		int m_fFlags = 0;
		int m_flSimulationTime = 0;
	}
	namespace DT_CSPlayer
	{
		int m_flLowerBodyYawTarget = 0;
		int m_angEyeAngles = 0;
		int m_bIsScoped = 0;
		int m_flDuckAmount = 0;
		int m_flPoseParameter = 0;
		int m_bHasHelmet = 0;
	}
	namespace DT_BaseCombatWeapon
	{
		int m_flNextPrimaryAttack = 0;
		int m_flNextSecondaryAttack = 0;
		int m_hOwner = 0;
		int m_iClip1 = 0;
		int m_iItemDefinitionIndex = 0;
	}
	namespace DT_WeaponCSBase
	{
		int m_flRecoilIndex = 0;
	}
	namespace Data
	{
		CGlowObjectManager* m_pGlowManager = 0;
		DWORD *m_uTraceLineIgnoreTwoEntities = 0;
		std::uintptr_t m_uMoveHelper = 0u;
		std::uintptr_t m_uUpdateClientSideAnimation = 0u;
		DWORD m_uClipTracePlayers = 0;
		std::uintptr_t m_uClientState = 0u;
		std::uintptr_t m_uInput = 0u;
		std::uintptr_t m_uGlobalVars = 0u;
		std::uintptr_t m_uPredictionRandomSeed = 0u;
		std::uintptr_t m_uPredictionPlayer = 0u;
		IViewRenderBeams* m_uClientBeams = 0u;
	}
	namespace Function
	{
		std::uintptr_t m_uRandomSeed = 0u;
		std::uintptr_t m_uRandomFloat = 0u;
		std::uintptr_t m_uRandomInt = 0u;
	}

}
