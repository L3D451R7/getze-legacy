#include "entity.hpp"
#include "displacement.hpp"
#include "player.hpp"
#include "source.hpp"

void IHandleEntity::SetRefEHandle( const CBaseHandle& handle )
{
	using Fn = void ( __thiscall* )( void*, const CBaseHandle& );
	return Memory::VCall<Fn>( this, Index::IHandleEntity::SetRefEHandle )( this, handle );
}

const CBaseHandle& IHandleEntity::GetRefEHandle()
{
	using Fn = const CBaseHandle& ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::IHandleEntity::GetRefEHandle )( this );
}

ICollideable* IClientUnknown::GetCollideable()
{
	using Fn = ICollideable* ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::IClientUnknown::GetCollideable )( this );
}

IClientNetworkable* IClientUnknown::GetClientNetworkable()
{
	//using Fn = IClientNetworkable* ( __thiscall* )( void* );
	//return Memory::VCall<Fn>( this, Index::IClientUnknown::GetClientNetworkable )( this );
	return (IClientNetworkable*)(uintptr_t(this) + 0x8);
}

IClientRenderable* IClientUnknown::GetClientRenderable()
{
	/*if (Source::m_pClientState->m_iDeltaTick == -1 || Source::m_pCvar->FindVar("mat_norendering")->GetInt())
		return nullptr;*/
	//using Fn = IClientRenderable* ( __thiscall* )( void* );
	//return Memory::VCall<Fn>( this, Index::IClientUnknown::GetClientRenderable )( this );
	return (IClientRenderable*)(uintptr_t(this) + 0x4);
}

IClientEntity* IClientUnknown::GetIClientEntity()
{
	using Fn = IClientEntity* ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::IClientUnknown::GetIClientEntity )( this );
}

C_BaseEntity* IClientUnknown::GetBaseEntity()
{
	using Fn = C_BaseEntity* ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::IClientUnknown::GetBaseEntity )( this );
}

Vector& ICollideable::OBBMins()
{
	using Fn = Vector& ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::ICollideable::OBBMins )( this );
}

Vector& ICollideable::OBBMaxs()
{
	using Fn = Vector& ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::ICollideable::OBBMaxs )( this );
}

ClientClass* IClientNetworkable::GetClientClass()
{
	using Fn = ClientClass* ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::IClientNetworkable::GetClientClass )( this );
}

bool IClientNetworkable::IsDormant()
{
	using Fn = bool ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::IClientNetworkable::IsDormant )( this );
}

int IClientNetworkable::entindex()
{
	using Fn = int ( __thiscall* )( void* );
	if (this)
		return Memory::VCall<Fn>(this, Index::IClientNetworkable::entindex)(this);
	else
		return 0;
}

void* IClientNetworkable::GetDataTableBasePtr()
{
	using Fn = void*(__thiscall*)(void*);
	return Memory::VCall<Fn>(this, Index::IClientNetworkable::GetDataTableBasePtr)(this);
}

const model_t* IClientRenderable::GetModel()
{
	using Fn = const model_t* ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::IClientRenderable::GetModel )( this );
}

bool IClientRenderable::SetupBones( matrix3x4_t* pBoneToWorld, int nMaxBones, int boneMask, float currentTime )
{
	using Fn = bool ( __thiscall* )( void*, matrix3x4_t*, int, int, float );
	return Memory::VCall<Fn>( this, Index::IClientRenderable::SetupBones )( this, pBoneToWorld, nMaxBones, boneMask, currentTime );
}

Vector& IClientEntity::OBBMins()
{
	return GetCollideable()->OBBMins();
}

Vector& IClientEntity::OBBMaxs()
{
	return GetCollideable()->OBBMaxs();
}

solid_type ICollideable::GetSolidType()
{
	using Fn = solid_type(__thiscall*)(void*);
	return Memory::VCall<Fn>(this, Index::ICollideable::SolidType)(this);
}

bool IClientEntity::is_breakable()
{
	if (!this)
		return false;

	static auto is_breakable_fn = reinterpret_cast<bool(__thiscall*)(IClientEntity*)>(
		Memory::Scan(cheat::main::clientdll, "55 8B EC 51 56 8B F1 85 F6 74 68"));

	const auto result = is_breakable_fn(this);

	if (!result && GetClientClass() != nullptr &&
		(GetClientClass()->m_ClassID == class_ids::CBaseDoor ||
			GetClientClass()->m_ClassID == class_ids::CBreakableSurface ||
			(GetClientClass()->m_ClassID == class_ids::CBaseEntity && GetCollideable() != nullptr && GetCollideable()->GetSolidType() == solid_bsp/*solid_bsp*/)))
		return true;

	return result;
}

ClientClass* IClientEntity::GetClientClass()
{
	if (Source::m_pClientState->m_iDeltaTick == -1)
		return 0;

	auto networkable = GetClientNetworkable();
	return networkable->GetClientClass();
}

bool IClientEntity::IsDormant()
{
	if (Source::m_pClientState->m_iDeltaTick == -1)
		return true;

	auto networkable = GetClientNetworkable();
	return networkable->IsDormant();
}

int IClientEntity::entindex()
{
	if (Source::m_pClientState->m_iDeltaTick == -1)
		return 0;

	auto networkable = GetClientNetworkable();
	return networkable->entindex();
}

const model_t* IClientEntity::GetModel()
{
	auto renderable = GetClientRenderable();

	if (!renderable)
		return nullptr;

	return renderable->GetModel();
}

bool IClientEntity::SetupBones( matrix3x4_t* pBoneToWorld, int nMaxBones, int boneMask, float currentTime )
{
	if (Source::m_pClientState->m_iDeltaTick == -1 || Source::m_pCvar->FindVar("mat_norendering")->GetInt())
		return false;

	auto renderable = GetClientRenderable();

	if (!renderable)
		return false;

	auto setupbones = renderable->SetupBones(pBoneToWorld, nMaxBones, boneMask, currentTime);

	return setupbones;
} 

bool C_BaseEntity::IsPlayer()
{
	using Fn = bool ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::C_BaseEntity::IsPlayer )( this );
}

std::uint8_t& C_BaseEntity::m_MoveType()
{
	return *( std::uint8_t* )( this + Engine::Displacement::C_BaseEntity::m_MoveType );
}

matrix3x4_t& C_BaseEntity::m_rgflCoordinateFrame()
{
	return *( matrix3x4_t* )( this + Engine::Displacement::C_BaseEntity::m_rgflCoordinateFrame );
}

int& C_BaseEntity::m_iTeamNum()
{
	return *( int* )( this + Engine::Displacement::DT_BaseEntity::m_iTeamNum );
}

Vector& C_BaseEntity::m_vecOrigin()
{
	return *( Vector* )( this + Engine::Displacement::DT_BaseEntity::m_vecOrigin );
}

void C_BaseEntity::SetPredictionRandomSeed( const CUserCmd* cmd )
{
	*( int* )( Engine::Displacement::Data::m_uPredictionRandomSeed ) = cmd ? cmd->random_seed : -1;
}

void C_BaseEntity::SetPredictionPlayer( C_BasePlayer* player )
{
	*( C_BasePlayer** )( Engine::Displacement::Data::m_uPredictionPlayer ) = player;
}

CBaseHandle& C_BaseCombatCharacter::m_hActiveWeapon()
{
	return *( CBaseHandle* )( this + Engine::Displacement::DT_BaseCombatCharacter::m_hActiveWeapon );
}