#pragma once

#include "entity.hpp"

enum ItemDefinitionIndex
{
	weapon_none,
	weapon_deagle,
	weapon_elite,
	weapon_fiveseven,
	weapon_glock,
	weapon_ak47 = 7,
	weapon_aug,
	weapon_awp,
	weapon_famas,
	weapon_g3sg1,
	weapon_galilar = 13,
	weapon_m249,
	weapon_m4a4 = 16,
	weapon_mac10,
	weapon_p90 = 19,
	weapon_mp5 = 23,
	weapon_ump45 = 24,
	weapon_xm1014,
	weapon_bizon,
	weapon_mag7,
	weapon_negev,
	weapon_sawedoff,
	weapon_tec9,
	weapon_taser,
	weapon_p2000,
	weapon_mp7,
	weapon_mp9,
	weapon_nova,
	weapon_p250,
	weapon_scar20 = 38,
	weapon_sg556,
	weapon_ssg08,
	weapon_knifegg,
	weapon_knife_ct = 42,
	weapon_flashbang,
	weapon_hegrenade,
	weapon_smokegrenade,
	weapon_molotov,
	weapon_decoy,
	weapon_incgrenade,
	weapon_c4,
	weapon_knife_t = 59,
	weapon_m4a1s = 60,
	weapon_usp = 61,
	weapon_cz75 = 63,
	weapon_revolver,
	weapon_tagrenade = 68,
	weapon_bayonet = 500,
	weapon_flip = 505,
	weapon_gut,
	weapon_karambit,
	weapon_m9bayonet,
	weapon_huntsman,
	weapon_falchion = 512,
	weapon_bowie = 514,
	weapon_ursus = 519,
	weapon_navaja = 520,
	weapon_stiletto = 522,
	weapon_talon = 523,
	weapon_butterfly = 515,
	weapon_pushdagger,
	studded_bloodhound_gloves = 5027,
	sporty_gloves = 5030,
	slick_gloves = 5031,
	leather_handwraps = 5032,
	motorcycle_gloves = 5033,
	specialist_gloves = 5034
};

class C_BaseCombatWeapon : public C_BaseEntity
{
public:
	float& m_flNextPrimaryAttack();
	float& m_flNextSecondaryAttack();
	CBaseHandle& m_hOwner();
	int& m_iClip1();
	int& m_iItemDefinitionIndex();
	int & m_iItemIDHigh();
	int & m_nFallbackPaintKit();
	int & m_iAccountID();
	int & m_OriginalOwnerXuidLow();
	int & m_OriginalOwnerXuidHigh();
	int & m_iEntityQuality();
	CBaseHandle & m_hWeaponWorldModel();
	void set_model_index(int index);
	bool IsGun();
	bool is_default_knife();
	bool is_knife();
};

class C_WeaponCSBaseGun : public C_BaseCombatWeapon
{
public:
	float& m_flRecoilIndex();

	float GetSpread();
	float & m_flLastShotTime();
	float GetInaccuracy();
	void UpdateAccuracyPenalty();

	bool & m_bPinPulled();

	float & m_fThrowTime();
	int& m_Activity();

	bool IsBeingThrowed(CUserCmd* cmd);

	float& m_flThrowStrength();

	weapon_info * GetCSWeaponData();

	bool IsFireTime();
	bool IsSniper();
	int & m_weaponMode();
	int & m_zoomLevel();
	float & m_flPostponeFireReadyTime();
	float & m_fAccuracyPenalty();
	float GetMaxWeaponSpeed();
	bool can_shoot();
	bool can_cock();
	bool IsSecondaryFireTime();
};