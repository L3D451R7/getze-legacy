#pragma once

#include "sdk.hpp"
#include "matrix.hpp"

#pragma region decl_indices
namespace Index
{
	namespace IHandleEntity
	{
		enum
		{
			SetRefEHandle = 1,
			GetRefEHandle = 2,
		};
	}
	namespace IClientUnknown
	{
		enum
		{
			GetCollideable = 3,
			GetClientNetworkable = 4,
			GetClientRenderable = 5,
			GetIClientEntity = 6,
			GetBaseEntity = 7,
		};
	}
	namespace ICollideable
	{
		enum
		{
			OBBMins = 1,
			OBBMaxs = 2,
			SolidType = 11,
		};
	}
	namespace IClientNetworkable
	{
		enum
		{
			GetClientClass = 2,
			IsDormant = 9,
			entindex = 10,
			GetDataTableBasePtr = 12,
		};
	}
	namespace IClientRenderable
	{
		enum
		{
			GetModel = 8,
			SetupBones = 13,
		};
	}
	namespace C_BaseEntity
	{
		enum
		{
			IsPlayer = 152,
		};
	}
}
#pragma endregion

enum solid_type
{
	solid_none = 0,
	solid_bsp = 1,
	solid_bbox = 2,
	solid_obb = 3,
	solid_obb_yaw = 4,
	solid_custom = 5,
	solid_vphysics = 6,
	solid_last
};

class IHandleEntity
{
public:
	void SetRefEHandle( const CBaseHandle& handle );
	const CBaseHandle& GetRefEHandle();
};

class IClientUnknown : public IHandleEntity
{
public:
	ICollideable* GetCollideable();
	IClientNetworkable* GetClientNetworkable();
	IClientRenderable* GetClientRenderable();
	IClientEntity* GetIClientEntity();
	C_BaseEntity* GetBaseEntity();
};

class ICollideable
{
public:
	Vector& OBBMins();
	Vector& OBBMaxs();
	solid_type GetSolidType();
};

class IClientNetworkable
{
public:
	ClientClass* GetClientClass();
	bool IsDormant();
	int entindex();
	void* GetDataTableBasePtr();
};

class IClientRenderable
{
public:
	const model_t* GetModel();
	bool SetupBones( matrix3x4_t* pBoneToWorld, int nMaxBones, int boneMask, float currentTime );
};

class IClientEntity : public IClientUnknown
{
public:
	Vector& OBBMins();
	Vector& OBBMaxs();

	bool is_breakable();

	ClientClass* GetClientClass();
	bool IsDormant();
	int entindex();

	const model_t* GetModel();
	bool SetupBones( matrix3x4_t* pBoneToWorld, int nMaxBones, int boneMask, float currentTime );
};

class C_BaseEntity : public IClientEntity
{
public:
	bool IsPlayer();

	std::uint8_t& m_MoveType();
	matrix3x4_t& m_rgflCoordinateFrame();

	int& m_iTeamNum();
	Vector& m_vecOrigin();

public:
	static void SetPredictionRandomSeed( const CUserCmd* cmd );
	static void SetPredictionPlayer( C_BasePlayer* player );
};

class C_BaseCombatCharacter : public C_BaseEntity
{
public:
	CBaseHandle& m_hActiveWeapon();
};