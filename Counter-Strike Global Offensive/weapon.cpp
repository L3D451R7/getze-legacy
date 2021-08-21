#include "weapon.hpp"
#include "displacement.hpp"
#include "source.hpp"
#include "prop_manager.hpp"
#include "lag_compensation.hpp"
#include "player.hpp"
#include "aimbot.hpp"

float& C_BaseCombatWeapon::m_flNextPrimaryAttack()
{
	return *( float* )( this + Engine::Displacement::DT_BaseCombatWeapon::m_flNextPrimaryAttack );
}

float& C_BaseCombatWeapon::m_flNextSecondaryAttack()
{
	return *( float* )( this + Engine::Displacement::DT_BaseCombatWeapon::m_flNextSecondaryAttack );
}

CBaseHandle& C_BaseCombatWeapon::m_hOwner()
{
	return *( CBaseHandle* )( this + Engine::Displacement::DT_BaseCombatWeapon::m_hOwner );
}

int& C_BaseCombatWeapon::m_iClip1()
{
	return *( int* )( this + Engine::Displacement::DT_BaseCombatWeapon::m_iClip1 );
}

int& C_BaseCombatWeapon::m_iItemDefinitionIndex()
{
	return *(int* )( this + Engine::Displacement::DT_BaseCombatWeapon::m_iItemDefinitionIndex );
}

int & C_BaseCombatWeapon::m_iItemIDHigh()
{
	static auto m_iItemIDHigh = Engine::PropManager::Instance()->GetOffset("DT_BaseCombatWeapon", "m_iItemIDHigh");

	return *(int*)((DWORD)this + m_iItemIDHigh);
}

int & C_BaseCombatWeapon::m_nFallbackPaintKit()
{
	static auto m_nFallbackPaintKit = Engine::PropManager::Instance()->GetOffset("DT_BaseCombatWeapon", "m_nFallbackPaintKit");

	return *(int*)((DWORD)this + m_nFallbackPaintKit);
}

int & C_BaseCombatWeapon::m_iAccountID()
{
	static auto m_iAccountID = Engine::PropManager::Instance()->GetOffset("DT_BaseCombatWeapon", "m_iAccountID");

	return *(int*)((DWORD)this + m_iAccountID);
}

int & C_BaseCombatWeapon::m_OriginalOwnerXuidLow()
{
	static auto m_OriginalOwnerXuidLow = Engine::PropManager::Instance()->GetOffset("DT_BaseCombatWeapon", "m_OriginalOwnerXuidLow");

	return *(int*)((DWORD)this + m_OriginalOwnerXuidLow);
}

int & C_BaseCombatWeapon::m_OriginalOwnerXuidHigh()
{
	static auto m_OriginalOwnerXuidHigh = Engine::PropManager::Instance()->GetOffset("DT_BaseCombatWeapon", "m_OriginalOwnerXuidHigh");

	return *(int*)((DWORD)this + m_OriginalOwnerXuidHigh);
}

int & C_BaseCombatWeapon::m_iEntityQuality()
{
	static auto m_iEntityQuality = Engine::PropManager::Instance()->GetOffset("DT_BaseCombatWeapon", "m_iEntityQuality");

	return *(int*)((DWORD)this + m_iEntityQuality);
}

CBaseHandle & C_BaseCombatWeapon::m_hWeaponWorldModel()
{
	static auto m_hWeaponWorldModel = Engine::PropManager::Instance()->GetOffset("DT_BaseCombatWeapon", "m_hWeaponWorldModel");

	return *(CBaseHandle*)((DWORD)this + m_hWeaponWorldModel);
}

void C_BaseCombatWeapon::set_model_index(int index)
{
	typedef void(__thiscall* OriginalFn)(PVOID, int);
	return Memory::VCall<OriginalFn>(this, 75)(this, index);
}

bool C_BaseCombatWeapon::IsGun()
{
	if (!this)
		return false;

	int id = this->m_iItemDefinitionIndex();

	switch (id)
	{
	case weapon_deagle:
	case weapon_elite:
	case weapon_fiveseven:
	case weapon_glock:
	case weapon_ak47:
	case weapon_aug:
	case weapon_awp:
	case weapon_famas:
	case weapon_g3sg1:
	case weapon_galilar:
	case weapon_m249:
	case weapon_m4a4:
	case weapon_mac10:
	case weapon_p90:
	case weapon_mp5:
	case weapon_ump45:
	case weapon_xm1014:
	case weapon_bizon:
	case weapon_mag7:
	case weapon_negev:
	case weapon_sawedoff:
	case weapon_tec9:
	case weapon_taser:
	case weapon_p2000:
	case weapon_mp7:
	case weapon_mp9:
	case weapon_nova:
	case weapon_p250:
	case weapon_scar20:
	case weapon_sg556:
	case weapon_ssg08:
		return true;
	case weapon_knife_ct:
	case weapon_flashbang:
	case weapon_hegrenade:
	case weapon_smokegrenade:
	case weapon_molotov:
	case weapon_decoy:
	case weapon_incgrenade:
	case weapon_c4:
	case weapon_knife_t:
		return false;
	case weapon_m4a1s:
	case weapon_usp:
	case weapon_cz75:
	case weapon_revolver:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::is_default_knife()
{
	return (m_iItemDefinitionIndex() == weapon_knife_ct || m_iItemDefinitionIndex() == weapon_knife_t);
}

bool C_BaseCombatWeapon::is_knife()
{
	return (is_default_knife() || m_iItemDefinitionIndex() == weapon_knifegg || m_iItemDefinitionIndex() == weapon_bayonet || m_iItemDefinitionIndex() == weapon_butterfly
		|| m_iItemDefinitionIndex() == weapon_falchion || m_iItemDefinitionIndex() == weapon_flip || m_iItemDefinitionIndex() == weapon_gut
		|| m_iItemDefinitionIndex() == weapon_karambit || m_iItemDefinitionIndex() == weapon_m9bayonet || m_iItemDefinitionIndex() == weapon_pushdagger
		|| m_iItemDefinitionIndex() == weapon_bowie || m_iItemDefinitionIndex() == weapon_huntsman || m_iItemDefinitionIndex() == weapon_ursus
		|| m_iItemDefinitionIndex() == weapon_navaja || m_iItemDefinitionIndex() == weapon_stiletto || m_iItemDefinitionIndex() == weapon_talon);
}
//
//bool is_grenade()
//{
//	return m_iValue == WEAPON_FLASHBANG || m_iValue == WEAPON_HEGRENADE || m_iValue == WEAPON_SMOKEGRENADE
//		|| m_iValue == WEAPON_MOLOTOV || m_iValue == WEAPON_INCGRENADE || m_iValue == WEAPON_DECOY;
//}
//
//bool is_pistol()
//{
//	return m_iValue == WEAPON_DEAGLE || m_iValue == WEAPON_ELITE || m_iValue == WEAPON_FIVESEVEN
//		|| m_iValue == WEAPON_GLOCK || m_iValue == WEAPON_HKP2000 || m_iValue == WEAPON_P250
//		|| m_iValue == WEAPON_TEC9 || m_iValue == WEAPON_USP_SILENCER;
//}

float& C_WeaponCSBaseGun::m_flRecoilIndex()
{
	return *( float* )( this + Engine::Displacement::DT_WeaponCSBase::m_flRecoilIndex );
}

float C_WeaponCSBaseGun::GetSpread()
{
	using Fn = float ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, 439)( this );
}

float & C_WeaponCSBaseGun::m_flLastShotTime()
{
	static auto m_fLastShotTime = Engine::PropManager::Instance()->GetOffset("DT_WeaponCSBase", "m_fLastShotTime");

	return *(float*)((DWORD)this + m_fLastShotTime);
}

float C_WeaponCSBaseGun::GetInaccuracy()
{
	using Fn = float ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, 469)( this );
}

void C_WeaponCSBaseGun::UpdateAccuracyPenalty()
{
	using Fn = void ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, 470)( this );
}

bool &C_WeaponCSBaseGun::m_bPinPulled()
{
	static auto m_bPinPulled = Engine::PropManager::Instance()->GetOffset("DT_BaseCSGrenade", "m_bPinPulled");
	return *(bool*)((DWORD)this + m_bPinPulled);
}

float &C_WeaponCSBaseGun::m_fThrowTime()
{
	static auto m_fThrowTime = Engine::PropManager::Instance()->GetOffset("DT_BaseCSGrenade", "m_fThrowTime");
	return *(float *)((DWORD)this + m_fThrowTime);
}

int & C_WeaponCSBaseGun::m_Activity()
{
	static auto nigger = ((C_BasePlayer*)this)->FindInDataMap(((C_BasePlayer*)this)->GetPredDescMap(), "m_Activity");
	return *(int*)((DWORD)this + nigger);
}

bool C_WeaponCSBaseGun::IsBeingThrowed(CUserCmd* cmd)
{
	if (!m_bPinPulled()) {
		float throwTime = m_fThrowTime();

		if (throwTime > 0.f)
			return true;
	}

	if ((cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_ATTACK2)) {
		if (m_fThrowTime() > 0.f)
			return true;
	}

	return false;
}

float& C_WeaponCSBaseGun::m_flThrowStrength()
{
	static auto m_flThrowStrength = Engine::PropManager::Instance()->GetOffset("DT_BaseCSGrenade", "m_flThrowStrength");
	return *(float*)((DWORD)this + m_flThrowStrength);
}

weapon_info* C_WeaponCSBaseGun::GetCSWeaponData()
{
	using Fn = weapon_info*(__thiscall*)(void*);
	return Memory::VCall<Fn>(this, 446)(this);
}

bool C_WeaponCSBaseGun::IsFireTime()
{
	auto revolver_timer = Source::m_pGlobalVars->curtime - m_flPostponeFireReadyTime();

	return ( Source::m_pGlobalVars->curtime >= m_flNextPrimaryAttack() || IsSecondaryFireTime() || (revolver_timer <= -0.027f && m_flPostponeFireReadyTime() > 0.f && m_iItemDefinitionIndex() == weapon_revolver));
}

bool C_WeaponCSBaseGun::IsSniper()
{
	if (!this)
		return false;

	auto id = m_iItemDefinitionIndex();

	if (id == weapon_awp || id == weapon_ssg08 || id == weapon_scar20 || id == weapon_g3sg1)
		return true;
	else
		return false;
}

int& C_WeaponCSBaseGun::m_weaponMode()
{
	static auto m_weaponMode = Engine::PropManager::Instance()->GetOffset("DT_WeaponCSBase", "m_weaponMode");
	return *(int*)((DWORD)this + m_weaponMode);
}

int& C_WeaponCSBaseGun::m_zoomLevel()
{
	static auto m_zoomLevel = Engine::PropManager::Instance()->GetOffset("DT_WeaponCSBaseGun", "m_zoomLevel");
	return *(int*)((DWORD)this + m_zoomLevel);
}

float& C_WeaponCSBaseGun::m_flPostponeFireReadyTime()
{
	static auto m_flPostponeFireReadyTime = Engine::PropManager::Instance()->GetOffset("DT_WeaponCSBase", "m_flPostponeFireReadyTime");
	return *(float*)((DWORD)this + m_flPostponeFireReadyTime);
}

float& C_WeaponCSBaseGun::m_fAccuracyPenalty()
{
	static auto m_fAccuracyPenalty = Engine::PropManager::Instance()->GetOffset("DT_WeaponCSBase", "m_fAccuracyPenalty");
	return *(float*)((DWORD)this + m_fAccuracyPenalty);
}

float C_WeaponCSBaseGun::GetMaxWeaponSpeed() {
	if (!this || !GetCSWeaponData())
		return 250.0f;

	if (m_weaponMode() == 0)
		return GetCSWeaponData()->max_speed;

	return GetCSWeaponData()->max_speed_alt;
}

bool C_WeaponCSBaseGun::can_shoot()
{
	auto is_local_weapon = cheat::main::local() != nullptr && this == cheat::main::local()->get_weapon();

	if (is_local_weapon && cheat::main::local()->m_bWaitForNoAttack())
		return false;

	if (m_iItemDefinitionIndex() == 64)
	{
		auto can_attack = IsFireTime();

		if ((can_attack || is_local_weapon && IsSecondaryFireTime() && cheat::main::local()->m_iShotsFired() <= 0)
			&& (m_weaponMode() == 1
				|| m_Activity() != 208
				|| m_flPostponeFireReadyTime() >= TICKS_TO_TIME(cheat::main::local()->m_nTickBase())))
		{
			can_attack = false;
		}

		return can_attack;
	}

	return (IsFireTime() || IsSecondaryFireTime());
}

bool C_WeaponCSBaseGun::can_cock()
{
	auto v15 = TICKS_TO_TIME(cheat::main::local()->m_nTickBase());

	if (v15 < cheat::main::local()->m_flNextAttack())
		return 0;
	if (v15 < m_flNextPrimaryAttack())
		return 0;

	return cheat::features::aimbot.is_cocking;
}

bool C_WeaponCSBaseGun::IsSecondaryFireTime()
{
	return ( Source::m_pGlobalVars->curtime >= m_flNextSecondaryAttack() );
}