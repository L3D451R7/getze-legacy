#include "player.hpp"
#include "displacement.hpp"
#include "source.hpp"
#include "prop_manager.hpp"
#include "lag_compensation.hpp"
#include "game_movement.h"
#include <array>
#include "rmenu.hpp"

class CBoneAccessor;

QAngle& C_BasePlayer::m_aimPunchAngle()
{
	return *( QAngle* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_aimPunchAngle );
}

QAngle& C_BasePlayer::m_aimPunchAngleVel()
{
	static auto m_aimPunchAngleVel = Engine::PropManager::Instance()->GetOffset("DT_BasePlayer", "m_aimPunchAngleVel");
	return *(QAngle*)((uintptr_t)this + m_aimPunchAngleVel);
}

QAngle& C_BasePlayer::m_viewPunchAngle()
{
	return *( QAngle* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_viewPunchAngle );
}

Vector& C_BasePlayer::m_vecViewOffset()
{
	return *( Vector* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_vecViewOffset );
}

float& C_BasePlayer::m_flVelocityModifier()
{
	static auto m_flVelocityModifier = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_flVelocityModifier");
	return *(float*)((uintptr_t)this + m_flVelocityModifier);
}

float &C_BasePlayer::m_flDuckAmount()
{
	return *(float*)((DWORD)this + Engine::Displacement::DT_CSPlayer::m_flDuckAmount);
}

float &C_BasePlayer::m_flDuckSpeed()
{
	static auto m_flDuckSpeed = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_flDuckSpeed");
	return *(float*)((DWORD)this + m_flDuckSpeed);
}

datamap_t *C_BasePlayer::GetPredDescMap()
{
	typedef datamap_t*(__thiscall *o_GetPredDescMap)(void*);
	return Memory::VCall<o_GetPredDescMap>(this, 17)(this);
}

float &C_BasePlayer::m_flStepSize()
{
	static auto _m_flStepSize = Engine::PropManager::Instance()->GetOffset("DT_BaseEntity", "m_flStepSize");
	return *(float_t*)((uintptr_t)this + _m_flStepSize);
}

bool &C_BasePlayer::m_bGunGameImmunity()
{
	static auto _m_bGunGameImmunity = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_bGunGameImmunity");
	return *(bool*)((uintptr_t)this + _m_bGunGameImmunity);
}

bool &C_BasePlayer::m_bStrafing()
{
	static auto m_bStrafing = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_bStrafing");
	return *(bool*)((uintptr_t)this + m_bStrafing);
}

Vector &C_BasePlayer::m_datavecBaseVelocity()
{
	static unsigned int _m_vecBaseVelocity = FindInDataMap(GetPredDescMap(), "m_vecBaseVelocity");
	return *(Vector*)((uintptr_t)this + _m_vecBaseVelocity);
}

float &C_BasePlayer::m_flMaxspeed()
{
	static unsigned int _m_flMaxspeed = FindInDataMap(GetPredDescMap(), "m_flMaxspeed");
	return *(float*)((uintptr_t)this + _m_flMaxspeed);
}

unsigned int C_BasePlayer::FindInDataMap(datamap_t *pMap, const char *name)
{
	//FUNCTION_GUARD;

	while (pMap)
	{
		for (int i = 0; i<pMap->dataNumFields; i++)
		{
			if (pMap->dataDesc[i].fieldName == NULL)
				continue;

			if (strcmp(name, pMap->dataDesc[i].fieldName) == 0)
				return pMap->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

			if (pMap->dataDesc[i].fieldType == FIELD_EMBEDDED)
			{
				if (pMap->dataDesc[i].td)
				{
					unsigned int offset;

					if ((offset = FindInDataMap(pMap->dataDesc[i].td, name)) != 0)
						return offset;
				}
			}
		}
		pMap = pMap->baseMap;
	}

	return 0;
}

float &C_BasePlayer::m_surfaceFriction()
{
	static unsigned int _m_surfaceFriction = FindInDataMap(GetPredDescMap(), "m_surfaceFriction");
	return *(float*)((uintptr_t)this + _m_surfaceFriction);
}

C_WeaponCSBaseGun *C_BasePlayer::get_weapon()
{
	return (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(m_hActiveWeapon()));
}

void C_BasePlayer::CreateAnimationState(CCSGOPlayerAnimState *state)
{
	using CreateAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*, C_BasePlayer*);
	static auto CreateAnimState = (CreateAnimState_t)Memory::Scan(cheat::main::clientdll, "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46");
	if (!CreateAnimState)
		return;

	CreateAnimState(state, this);
}

float C_BasePlayer::m_flSpawnTime()
{
	return *(float*)((uintptr_t)this + 0xA290);
}

Vector C_BasePlayer::get_bone_pos(int iBone)
{
	if (iBone < 0 || iBone > 128 || !m_CachedBoneData().Base())
		return Vector::Zero;

	return Vector(m_CachedBoneData().Base()[iBone][0][3], m_CachedBoneData().Base()[iBone][1][3], m_CachedBoneData().Base()[iBone][2][3]);
}

void C_BasePlayer::modify_eye_pos(CCSGOPlayerAnimState* animstate, Vector* pos)
{
	/// Unneeded but whatever
	if (Source::m_pEngine->IsHLTV() || Source::m_pEngine->IsPlayingDemo())
		return;

	using lookup_bone_fnt = int(__thiscall*)(void*, const char*);
	static auto lookup_bone_fn = (lookup_bone_fnt)Memory::Scan(cheat::main::clientdll, "55 8B EC 53 56 8B F1 57 83 BE ? ? ? ? ? 75 14 8B 46 04 8D 4E 04 FF 50 20 85 C0 74 07 8B CE E8 ? ? ? ? 8B 8E ? ? ? ? 85 C9 0F 84");

	if (*reinterpret_cast< bool* >(uintptr_t(animstate) + 0x109) /// in hitground
		&& animstate->duck_amt != 0.f) /// duck amount
	{
		auto base_entity = *reinterpret_cast< void** >(uintptr_t(animstate) + 0x60);

		auto bone_pos = get_bone_pos(/*base_entity, */lookup_bone_fn(base_entity, "head_0"));

		bone_pos.z += 1.7f;

		if ((*pos).z > bone_pos.z)
		{
			float some_factor = 0.f;

			float delta = (*pos).z - bone_pos.z;

			float some_offset = (delta - 4.f) / 6.f;
			if (some_offset >= 0.f)
				some_factor = std::fminf(some_offset, 1.f);

			(*pos).z += ((bone_pos.z - (*pos).z) * (((some_factor * some_factor) * 3.f) - (((some_factor * some_factor) * 2.f) * some_factor)));
		}
	}
}

void C_BasePlayer::eye_pos(Vector* piska)
{
	using fn = void(__thiscall*)(void*, Vector*);
	return Memory::VCall<fn>(this, 163)(this, piska);
}

int& C_BasePlayer::m_iShotsFired()
{
	static auto m_iShotsFired = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_iShotsFired");
	return *(int*)((uintptr_t)this + m_iShotsFired);
}

bool& C_BasePlayer::m_bWaitForNoAttack()
{
	static auto m_bWaitForNoAttack = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_bWaitForNoAttack");
	return *(bool*)((DWORD)this + m_bWaitForNoAttack);
}

float& C_BasePlayer::m_flNextAttack()
{
	static auto m_flNextAttack = Engine::PropManager::Instance()->GetOffset("DT_BaseCombatCharacter", "m_flNextAttack");
	return *(float*)((uintptr_t)this + m_flNextAttack);
}

Vector C_BasePlayer::GetEyePosition()
{
	if (this == cheat::main::local()) 
	{
		Vector eyePos;
		eyePos[2] = 0.0;
		eyePos[1] = 0.0;
		eyePos[0] = 0.0;
		Memory::VCall<void(__thiscall*)(void*, Vector&)>(this, 277)(this, eyePos); //ayyyyy

		// when fakelagging something weird happens
		// viewoffset.z will be ~46.05 or ~64.1f when it should be 46.f and 64.f 
		if (m_vecViewOffset().z < 46.1f)
		{
			eyePos.z -= m_vecViewOffset().z;
			eyePos.z += 46.f;
		}
		else if (m_vecViewOffset().z > 64.f)
		{
			eyePos.z -= m_vecViewOffset().z;
			eyePos.z += 64.f;
		}

		return eyePos;
	}
	else {
		Vector origin = m_vecOrigin();

		Vector vDuckHullMin = Source::m_pGameMovement->GetPlayerMins(true);
		Vector vStandHullMin = Source::m_pGameMovement->GetPlayerMins(false);

		float fMore = (vDuckHullMin.z - vStandHullMin.z);

		Vector vecDuckViewOffset = Source::m_pGameMovement->GetPlayerViewOffset(true);
		Vector vecStandViewOffset = Source::m_pGameMovement->GetPlayerViewOffset(false);
		float duckFraction = m_flDuckAmount();

		float tempz = ((vecDuckViewOffset.z - fMore) * duckFraction) +
			(vecStandViewOffset.z * (1 - duckFraction));

		origin.z += tempz;

		return origin;
	}
}

bool C_BasePlayer::IsBot()
{
	player_info pinfo;
	return (Source::m_pEngine->GetPlayerInfo(entindex(), &pinfo) && pinfo.fakeplayer);
}

Vector C_BasePlayer::predicted_eyepos()
{
	//auto fakeduck = (cheat::game::pressed_keys[(int)cheat::Cvars.Exploits_fakeduck.GetValue()] && (int)cheat::Cvars.Exploits_fakeduck.GetValue() != 0);
	//
	if (this == cheat::main::local() /*&& !fakeduck && Source::m_pClientState->m_iDeltaTick != -1 && !cheat::main::updating_skins && Source::m_pGlobalVars->frametime*/) {
		//	Vector vec{};
		//
		//	const auto bflags = m_iEFlags();
		//	m_iEFlags() &= ~0x800;
		//
		//	auto backup = get_abs_origin();
		//
		//	set_abs_origin(m_vecOrigin());
		//
		//	Memory::VCall<void(__thiscall*)(void*, Vector&)>(this, 281)(this, vec);
		//
		//	set_abs_origin(backup);
		//
		//	m_iEFlags() = bflags;
		//
		//	return vec;

		auto pos = Vector{};

		eye_pos(&pos);

		if (*reinterpret_cast< int32_t* >(uintptr_t(this) + 0x39E1))
		{
			auto anim_state = get_animation_state();
			if (anim_state)
				modify_eye_pos(anim_state, &pos);
		}

		pos += (m_vecVelocity() * (Source::m_pGlobalVars->interval_per_tick));

		return pos;
	}
	else {
		Vector origin = m_vecOrigin() + (m_vecVelocity() * (Source::m_pGlobalVars->interval_per_tick * 2.f));

		Vector vDuckHullMin = Source::m_pGameMovement->GetPlayerMins(true);
		Vector vStandHullMin = Source::m_pGameMovement->GetPlayerMins(false);

		float fMore = (vDuckHullMin.z - vStandHullMin.z);

		Vector vecDuckViewOffset = Source::m_pGameMovement->GetPlayerViewOffset(true);
		Vector vecStandViewOffset = Source::m_pGameMovement->GetPlayerViewOffset(false);
		float duckFraction = m_flDuckAmount();

		float tempz = ((vecDuckViewOffset.z - fMore) * duckFraction) +
			(vecStandViewOffset.z * (1 - duckFraction));

		origin.z += tempz;

		return origin;
	}
}

int& C_BasePlayer::LastBoneSetupFrame()
{
	return *(int*)((uintptr_t)this + 0xA64);
	// TODO: insert return statement here
}

void C_BasePlayer::update_clientside_animations()
{
	//const auto realtime = Source::m_pGlobalVars->realtime;
	////const auto curtime = Source::m_pGlobalVars->curtime;
	//const auto frametime = Source::m_pGlobalVars->frametime;
	//const auto absframetime = Source::m_pGlobalVars->absoluteframetime;
	//const auto interpamt = Source::m_pGlobalVars->interpolation_amount;
	//const auto framecount = Source::m_pGlobalVars->framecount;
	//const auto tickcount = Source::m_pGlobalVars->tickcount;

	//auto v4 = TIME_TO_TICKS(m_flSimulationTime());
	//Source::m_pGlobalVars->realtime = m_flSimulationTime();
	////Source::m_pGlobalVars->curtime = m_flSimulationTime();
	//Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;
	//Source::m_pGlobalVars->absoluteframetime = Source::m_pGlobalVars->interval_per_tick;
	//Source::m_pGlobalVars->framecount = v4;
	//Source::m_pGlobalVars->tickcount = v4;
	//Source::m_pGlobalVars->interpolation_amount = 0.0;

	//if (get_animation_state()) {
	//	if (get_animation_state()->last_anim_upd_tick == Source::m_pGlobalVars->framecount)
	//		get_animation_state()->last_anim_upd_tick = Source::m_pGlobalVars->framecount - 1;

	//	//if (get_animation_state()->last_anim_upd_time == Source::m_pGlobalVars->curtime)
	//	//	get_animation_state()->last_anim_upd_time = Source::m_pGlobalVars->curtime - get_animation_state()->anim_update_delta;
	//}

	//client_side_animation() = true;
	cheat::main::updating_anims = true;
	client_side_animation() = true;
	//*(int*)(uintptr_t(m_player) + 0x28BC) = 0;

	//*(int*)(uintptr_t(Source::m_pClientState) + 0x4D40) = 1; //m_bIsHLTV
	Memory::VCall<void(__thiscall*)(void*)>(this, UPDATE_CLIENTSIDE_ANIMATIONS)(this);
	//*(int*)(uintptr_t(Source::m_pClientState) + 0x4D40) = 0;

	cheat::main::updating_anims = false;
	//client_side_animation() = false;

	//Source::m_pGlobalVars->realtime = realtime;
	////Source::m_pGlobalVars->curtime = curtime;
	//Source::m_pGlobalVars->frametime = frametime;
	//Source::m_pGlobalVars->absoluteframetime = absframetime;
	//Source::m_pGlobalVars->framecount = framecount;
	//Source::m_pGlobalVars->tickcount = tickcount;
	//Source::m_pGlobalVars->interpolation_amount = interpamt;
}

matrix3x4_t** C_BasePlayer::dwBoneMatrix()
{
	/*auto rend = GetClientRenderable();

	if (!rend)
		return 0;*/

	return (matrix3x4_t**)((uintptr_t)this + 0x2910); // m_CachedBoneData.Base()
}

CUtlVector< matrix3x4_t >& C_BasePlayer::m_CachedBoneData() {
	return *(CUtlVector< matrix3x4_t >*)((uintptr_t)this + 0x4 + 0x28FC);
}//DT_BasePlayer::m_CachedBoneData = reinterpret_cast<std::uintptr_t>(core::find_signature(charenc("client.dll"), charenc("8B BF ? ? ? ? 8D 14 49")) + 2); for using base ent

int& C_BasePlayer::GetBoneCount()
{
	return *(int*)((uintptr_t)this + 0x290C);
}

CBoneAccessor *C_BasePlayer::GetBoneAccessor()
{
	return (CBoneAccessor*)((uintptr_t)this + 0x2698); //0x2694
}

CStudioHdr *C_BasePlayer::GetModelPtr()
{
	return *(CStudioHdr**)((uintptr_t)this + 0x293C);
}

void C_BasePlayer::StandardBlendingRules(CStudioHdr *hdr, Vector *pos, Quaternion *q, float curtime, int32_t boneMask)
{
	typedef void(__thiscall *o_StandardBlendingRules)(void*, CStudioHdr*, Vector*, Quaternion*, float_t, int32_t);
	Memory::VCall<o_StandardBlendingRules>(this, 200)(this, hdr, pos, q, curtime, boneMask);
}

void C_BasePlayer::set_model_index(int index)
{
	typedef void(__thiscall* OriginalFn)(PVOID, int);
	return Memory::VCall<OriginalFn>(this, 75)(this, index);
}

void C_BasePlayer::BuildTransformations(CStudioHdr *hdr, Vector *pos, Quaternion *q, const matrix3x4_t &cameraTransform, int32_t boneMask, byte *computed)
{
	typedef void(__thiscall *o_BuildTransformations)(void*, CStudioHdr*, Vector*, Quaternion*, const matrix3x4_t&, int32_t, byte*);
	Memory::VCall<o_BuildTransformations>(this, 185)(this, hdr, pos, q, cameraTransform, boneMask, computed);
}

//bool C_BasePlayer::HandleBoneSetup(int32_t boneMask, matrix3x4_t *boneOut, float_t curtime)
//{
//	CStudioHdr *hdr = this->GetModelPtr();
//	if (!hdr)
//		return false;
//
//	CBoneAccessor *accessor = this->GetBoneAccessor();
//	if (!accessor)
//		return false;
//
//	matrix3x4_t *backup_matrix = accessor->GetBoneArrayForWrite();
//	if (!backup_matrix)
//		return false;
//
//	Vector origin = this->m_vecOrigin();
//	QAngle angles = this->m_angEyeAngles();
//
//	Vector backup_origin = this->GetAbsOrigin();
//	QAngle backup_angles = this->GetAbsAngles();
//
//	std::array<float_t, 24> backup_poses;
//	backup_poses = this->m_flPoseParameter();
//
//	AnimationLayer backup_layers[15];
//	memcpy(backup_layers, this->GetAnimOverlays(), (sizeof(AnimationLayer) * this->GetNumAnimOverlays()));
//
//	alignas(16) matrix3x4_t parentTransform;
//	Math::AngleMatrix(angles, origin, parentTransform);
//
//	auto &anim_data = Animation::Get().GetPlayerAnimationInfo(this->EntIndex());
//
//	this->SetAbsOrigin(origin);
//	this->SetAbsAngles(angles);
//	this->m_flPoseParameter() = anim_data.m_flPoseParameters;
//	memcpy(this->GetAnimOverlays(), anim_data.m_AnimationLayer, (sizeof(AnimationLayer) * this->GetNumAnimOverlays()));
//
//	Vector *pos = (Vector*)(g_pMemAlloc->Alloc(sizeof(Vector[128])));
//	Quaternion *q = (Quaternion*)(g_pMemAlloc->Alloc(sizeof(Quaternion[128])));
//	std::memset(pos, 0xFF, sizeof(pos));
//	std::memset(q, 0xFF, sizeof(q));
//
//	this->StandardBlendingRules(hdr, pos, q, curtime, boneMask);
//
//	accessor->SetBoneArrayForWrite(boneOut);
//
//	byte *computed = (byte*)(g_pMemAlloc->Alloc(sizeof(byte[0x20])));
//	std::memset(computed, 0, sizeof(byte[0x20]));
//
//	this->BuildTransformations(hdr, pos, q, parentTransform, boneMask, &computed[0]);
//
//	accessor->SetBoneArrayForWrite(backup_matrix);
//
//	this->SetAbsOrigin(backup_origin);
//	this->SetAbsAngles(backup_angles);
//	this->m_flPoseParameter() = backup_poses;
//	memcpy(this->GetAnimOverlays(), backup_layers, (sizeof(AnimationLayer) * this->GetNumAnimOverlays()));
//
//	return true;
//}

void C_BasePlayer::force_bone_rebuild()
{
	/*
	string: "current: %d" = C_CSPlayer::ClientThink
	scroll down to a call = C_CSPlayer::ReevauluateAnimLOD

	0xA30 = last_occlusiontime
	0xA28 = occlusion_flags
	*/

	//return; // no need to do this anyway

	*(int*)((DWORD)this + 0xA30) = 0;//Source::m_pGlobalVars->framecount;
	*(int*)((DWORD)this + 0xA28) = 0; // m_bCanUseFastPath

	*(int*)((DWORD)this + 0x2680) = 0;
	*(int*)((DWORD)this + 0x2914) = 0xFF7FFFFF;

	//*(int*)((DWORD)this + Engine::Displacement::DT_BaseAnimating::m_nForceBone + 0x20) = 0; // writable bones
	//*(int*)((DWORD)this + Engine::Displacement::DT_BaseAnimating::m_nForceBone + 0x4) = -1; // m_iMostRecentModelBoneCounter

	//static auto invalidate_blackbones = Memory::Scan(cheat::main::clientdll, "A1 ? ? ? ? 09 B7 ? ? ? ?");

	//auto model_bone_counter = **(unsigned long**)(invalidate_blackbones + 0xA);

	//*(uint32_t*)(uintptr_t(this) + 0x2914) = 0xFF7FFFFF;
	//*(std::uint32_t *)(uintptr_t(this) + 0x2690) = ((*(std::uint32_t *)(uintptr_t(this) + 0x267C))/*model_bone_counter*/ - 1u);

	//LastBoneSetupFrame() = 0;

	//invalidate_anims();
}

bool& C_BasePlayer::client_side_animation()
{
	return *(bool*)((DWORD)this + Engine::Displacement::DT_BaseAnimating::m_bClientSideAnimation);
}

bool& C_BasePlayer::is_animating()
{
	return Memory::VCall<bool&(__thiscall*)(void*)>(this, IS_SELF_ANIMATING)(this); //153
}

Vector& C_BasePlayer::get_abs_origin()
{
	return Memory::VCall<Vector&(__thiscall*)(void*)>(this, GET_ABS_ORIGIN)(this);
}

QAngle& C_BasePlayer::get_abs_eye_angles()
{
	return Memory::VCall<QAngle&(__thiscall*)(void*)>(this, GET_ABS_ANGLES)(this);
}

QAngle& C_BasePlayer::get_eye_angles()
{
	return Memory::VCall<QAngle&(__thiscall*)(void*)>(this, 164)(this);
}

float C_BasePlayer::get_fixed_curtime(CUserCmd * cmd)
{
	static int g_tick = 0;
	static CUserCmd *LastpCmd;

	int tickbase = *reinterpret_cast<int*>((DWORD)this + Engine::Displacement::DT_BasePlayer::m_nTickBase);

	if (!LastpCmd || LastpCmd->hasbeenpredicted) {
		g_tick = tickbase; //m_nTickBase; 
	}
	else {
		// Required because prediction only runs on frames, not ticks 
		// So if your framerate goes below tickrate, m_nTickBase won't update every tick 
		++g_tick;
	}

	LastpCmd = cmd;

	float curtime = (float)g_tick * Source::m_pGlobalVars->interval_per_tick;

	return curtime;
}

void C_BasePlayer::set_abs_origin(Vector origin)
{
	static auto set_abs_origin = (std::uintptr_t*)Memory::Scan(cheat::main::clientdll, "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8");

	if (set_abs_origin)
		reinterpret_cast<void(__thiscall*)(void*, const Vector&)>(set_abs_origin)(this, origin);

	//m_iEFlags() &= ~0x800;
	//*(Vector*)((uintptr_t)this + 0xA0) = origin;
	//Memory::VCall<Vector&(__thiscall*)(void*)>(this, GET_ABS_ORIGIN)(this) = origin; // idx 10 = GetAbsOrigin
}

void C_BasePlayer::UpdateVisibilityAllEntities()
{
	static uintptr_t* update_visibility_all_entities = nullptr;
	if (update_visibility_all_entities == nullptr) {
		//static auto relative_call = (std::uintptr_t*)(Memory::Scan("client.dll", "E8 ? ? ? ? 83 7D D8 00 7C 0F"));

		//static auto offset = *(uintptr_t*)(relative_call + 0x1);
		//auto update_visibility_all_entities = (uintptr_t*)(relative_call + 5 + offset);

		static DWORD callInstruction = Memory::Scan(cheat::main::clientdll, "E8 ? ? ? ? 83 7D D8 00 7C 0F"); // get the instruction address
		static DWORD relativeAddress = *(DWORD*)(callInstruction + 1); // read the rel32
		static DWORD nextInstruction = callInstruction + 5; // get the address of next instruction
		update_visibility_all_entities = (uintptr_t*)(nextInstruction + relativeAddress); // our function address will be nextInstruction + relativeAddress

	}
	else
		reinterpret_cast<void(__thiscall*)(void*)>(update_visibility_all_entities)(this);
}

int C_BasePlayer::draw_model(int flags, uint8_t alpha)
{
	if (!this) return 0;
	auto renderable = this->GetClientRenderable();
	if (!renderable) return 0;

	typedef int(__thiscall* origfn)(void*, int, uint8_t);
	return Memory::VCall<origfn>(renderable, SET_MODEL)(renderable, flags, alpha);
}

int& C_BasePlayer::m_nSequence()
{
	return *(int*)((DWORD)this + Engine::Displacement::DT_BaseAnimating::m_nSequence);
}

float& C_BasePlayer::m_flLowerBodyYawTarget()
{
	return *(float*)((DWORD)this + Engine::Displacement::DT_CSPlayer::m_flLowerBodyYawTarget);
}

std::array<float, 24> &C_BasePlayer::m_flPoseParameter()
{
	return *(std::array<float, 24>*)((DWORD)this + Engine::Displacement::DT_CSPlayer::m_flPoseParameter);
}

float &C_BasePlayer::get_pose(int index)
{
	return *(float*)((DWORD)this + (Engine::Displacement::DT_CSPlayer::m_flPoseParameter + sizeof(float) * index));
}

int& C_BasePlayer::m_nHitboxSet()
{
	return *(int*)((DWORD)this + Engine::Displacement::DT_BaseAnimating::m_nHitboxSet);
}

//m_bCanMoveDuringFreezePeriod

bool& C_BasePlayer::m_bCanMoveDuringFreezePeriod()
{
	static int m_bCanMoveDuringFreezePeriod = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_bCanMoveDuringFreezePeriod");

	return *(bool*)((DWORD)this + m_bCanMoveDuringFreezePeriod);
}

int& C_BasePlayer::m_nBody()
{
	static auto m_nBody = Engine::PropManager::Instance()->GetOffset("DT_BaseAnimating", "m_nBody");

	return *(int*)((DWORD)this + m_nBody);
}

Vector& C_BasePlayer::m_vecRagdollOrigin()
{
	static auto m_vecRagdollOrigin = Engine::PropManager::Instance()->GetOffset("DT_CSRagdoll", "m_vecRagdollOrigin");

	return *(Vector*)((DWORD)this + m_vecRagdollOrigin);
}

float C_BasePlayer::get_bomb_blow_timer()
{
	if (!this)
		return 0;

	static auto m_flC4Blow = Engine::PropManager::Instance()->GetOffset("DT_PlantedC4", "m_flC4Blow");

	float bombTime = *(float*)((DWORD)this + m_flC4Blow);
	float returnValue = bombTime - Source::m_pGlobalVars->curtime;
	return (returnValue < 0) ? 0.f : returnValue;
}

bool C_BasePlayer::m_bHasDefuser()
{
	if (!this || this->m_iTeamNum() != 3) return false;

	static int offset = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_bHasDefuser");

	return *(bool*)((DWORD)this + offset);
}

int &C_BasePlayer::m_nExplodeEffectTickBegin()
{
	static int offset = Engine::PropManager::Instance()->GetOffset("DT_BaseCSGrenadeProjectile", "m_nExplodeEffectTickBegin");

	return *(int*)((DWORD)this + offset);
}

C_BasePlayer* C_BasePlayer::m_hBombDefuser()
{
	if (!this) return false;

	static int offset = Engine::PropManager::Instance()->GetOffset("DT_PlantedC4", "m_hBombDefuser");

	auto defuser = *(CBaseHandle*)((DWORD)this + offset);

	return Source::m_pEntList->GetClientEntityFromHandle(defuser);
}

float C_BasePlayer::get_bomb_defuse_timer()
{
	if (!this)
		return 0.f;

	float returnValue = m_flDefuseCountDown() - Source::m_pGlobalVars->curtime;
	return (returnValue < 0) ? 0.f : returnValue;
}

float& C_BasePlayer::m_flDefuseCountDown()
{
	static int offset = Engine::PropManager::Instance()->GetOffset("DT_PlantedC4", "m_flDefuseCountDown");
	return *(float*)((DWORD)this + offset);
}

int& C_BasePlayer::m_nSkin()
{
	static auto m_nSkin = Engine::PropManager::Instance()->GetOffset("DT_BaseAnimating", "m_nSkin");

	return *(int*)((DWORD)this + m_nSkin);
}

int& C_BasePlayer::m_hOwnerEntity()
{
	static auto m_hOwnerEntity = Engine::PropManager::Instance()->GetOffset("DT_BasePlayer", "m_hOwnerEntity");

	return *(int*)((DWORD)this + m_hOwnerEntity);
}

matrix3x4_t& C_BasePlayer::GetCollisionBoundTrans()
{
	return m_rgflCoordinateFrame();
}

bool& C_BasePlayer::m_bHasHelmet()
{
	return *(bool*)((DWORD)this + Engine::Displacement::DT_CSPlayer::m_bHasHelmet);
}

QAngle& C_BasePlayer::m_angEyeAngles()
{
	return *(QAngle*)((DWORD)this + Engine::Displacement::DT_CSPlayer::m_angEyeAngles);
}

int C_BasePlayer::get_sec_activity(int sequence)
{
	if (!this || !this->GetClientRenderable()) return 0;

	if (!this->GetModel())
		return 0;

	auto hdr = Source::m_pModelInfo->GetStudioModel(this->GetModel());

	if (!hdr)
		return -1;

	// c_csplayer vfunc 242, follow calls to find the function.
	static DWORD fn = NULL;

	if (!fn) {
		fn = (DWORD)Memory::Scan(cheat::main::clientdll, "55 8B EC 83 7D 08 FF 56 8B F1 74 3D");
		return 0;
	}

	static auto GetSequenceActivity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(fn);

	return GetSequenceActivity(this, hdr, sequence);
}

int &C_BasePlayer::TakeDamage()
{
	static auto model_Index = Engine::PropManager::Instance()->GetOffset("DT_BasePlayer", "m_nModelIndex");

	return *(int*)((DWORD)this + model_Index - 0xf);
}

std::string	C_BasePlayer::GetSteamID()
{
	player_info p_info;
	if (Source::m_pEngine->GetPlayerInfo(this->entindex(), &p_info))
		return p_info.guid;

	return "";
}

int &C_BasePlayer::m_ArmorValue()
{
	static auto m_ArmorValue = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_ArmorValue");

	return *(int*)((DWORD)this + m_ArmorValue);
}

float &C_BasePlayer::m_flFlashMaxAlpha()
{
	static auto m_flFlashMaxAlpha = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_flFlashMaxAlpha");

	return *(float*)((DWORD)this + m_flFlashMaxAlpha);
}

float &C_BasePlayer::m_flFlashDuration()
{
	static auto m_flFlashDuration = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_flFlashDuration");

	return *(float*)((DWORD)this + m_flFlashDuration);
}

float &C_BasePlayer::m_flPlaybackRate()
{
	static auto m_flPlaybackRate = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_flPlaybackRate");

	return *(float*)((DWORD)this + m_flPlaybackRate);
}

uint8_t* C_BasePlayer::GetServerEdict() {
	static uintptr_t pServerGlobals = **(uintptr_t**)(Memory::Scan("server.dll", "8B 15 ? ? ? ? 33 C9 83 7A 18 01") + 0x2);

	if (!pServerGlobals)
		return nullptr;

	int iMaxClients = *(int*)((uintptr_t)pServerGlobals + 0x18);
	if (entindex() > 0 && iMaxClients >= 1) {
		if (entindex() <= iMaxClients) {
			int v10 = entindex() * 16;
			uintptr_t v11 = *(uintptr_t*)(pServerGlobals + 96);
			if (v11) {
				if (!((*(uintptr_t*)(v11 + v10) >> 1) & 1)) {
					uintptr_t v12 = *(uintptr_t*)(v10 + v11 + 12);
					if (v12) {
						uint8_t* pReturn = nullptr;

						// abusing asm is not good
						__asm
						{
							pushad
							mov ecx, v12
							mov eax, dword ptr[ecx]
							call dword ptr[eax + 0x14]
							mov pReturn, eax
							popad
						}

						return pReturn;
					}
				}
			}
		}
	}
	return nullptr;
}

bool C_BasePlayer::ComputeHitboxSurroundingBox(matrix3x4_t * mx,Vector *pVecWorldMins, Vector *pVecWorldMaxs)
{
	// Note that this currently should not be called during Relink because of IK.
	// The code below recomputes bones so as to get at the hitboxes,
	// which causes IK to trigger, which causes raycasts against the other entities to occur,
	// which is illegal to do while in the Relink phase.

	if (!GetModelPtr() || !this)
		return false;

	studiohdr_t *pStudioHdr = GetModelPtr()->m_pStudioHdr;
	if (!pStudioHdr)
		return false;

	mstudiohitboxset_t *set = pStudioHdr->pHitboxSet(m_nHitboxSet());
	if (!set || !set->numhitboxes)
		return false;

	//CBoneCache *pCache = GetBoneCache();

	// Compute a box in world space that surrounds this entity
	pVecWorldMins->Set(FLT_MAX, FLT_MAX, FLT_MAX);
	pVecWorldMaxs->Set(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	Vector vecBoxAbsMins, vecBoxAbsMaxs;
	for (int i = 0; i < set->numhitboxes; i++)
	{
		mstudiobbox_t *pbox = set->pHitbox(i);
		
		if (mx)
		{
			Math::TransformAABB(mx[pbox->bone], pbox->bbmin /** GetModelScale()*/, pbox->bbmax /** GetModelScale()*/, vecBoxAbsMins, vecBoxAbsMaxs);
			Math::VectorMin(*pVecWorldMins, vecBoxAbsMins, *pVecWorldMins);
			Math::VectorMax(*pVecWorldMaxs, vecBoxAbsMaxs, *pVecWorldMaxs);
		}
	}
	return true;
}

void C_BasePlayer::DrawServerHitboxes() {
	//auto duration = g_CVar->FindVar( "sv_showlagcompensation_duration" )->GetFloat();
	auto duration = Source::m_pGlobalVars->interval_per_tick * 2.0f;

	auto serverPlayer = GetServerEdict();
	if (serverPlayer) {
		// CBaseAnimating::DrawServerHitboxes(CBaseAnimating *this, float a2, __int64 a3)
		// ref: mass %.1f, first call in found function - draw hitboxes

		static auto pCall = (uintptr_t*)(Memory::Scan("server.dll", "55 8B EC 81 EC ? ? ? ? 53 56 8B 35 ? ? ? ? 8B D9 57 8B CE"));

		if (!pCall)
			return;

		float fDuration = duration;

		__asm
		{
			pushad
			movss xmm1, fDuration
			push 0 //bool monoColor
			mov ecx, serverPlayer
			call pCall
			popad
		}
	}
}

void C_BasePlayer::set_pose_param(int param, float value)
{
	static auto Studio_SetPoseParameter = reinterpret_cast<void*>(Memory::Scan(cheat::main::clientdll, "55 8B EC 83 E4 F8 83 EC 08 F3 0F 11 54 24 ? 85 D2"));

	auto result = 0.0f;
	auto hdr = this->GetModelPtr();

	__asm {
		pushad
		movss xmm2, [value]
		lea eax, [result]
		push eax
		mov edx, param
		mov ecx, hdr
		call Studio_SetPoseParameter
		pop eax
		popad
	}

	this->m_flPoseParameter()[param] = result;
}

void C_BasePlayer::SetupBonesEx() {
	// pvs fix
	//OcclusionMask() = 0;

	auto accessor = GetBoneAccessor();
	accessor->m_ReadableBones = 0;

	// BuildTransformations Fix (credits: polak)
	static auto r_jiggle_bones = Source::m_pCvar->FindVar("r_jiggle_bones")->GetInt();
	Source::m_pCvar->FindVar("r_jiggle_bones")->SetValue(0);

	//// first bone snapshot fix
	//if (*(uintptr_t*)((uintptr_t)this + 0x39F0) != 0)
	//	*(uintptr_t*)((uintptr_t)this + 0x39F0) = 0;

	//// second bone snapshot fix
	//if (*(uintptr_t*)((uintptr_t)this + 0x6E40) != 0)
	//	*(uintptr_t*)((uintptr_t)this + 0x6E40) = 0;

	//// AccumulateInterleavedDispatchedLayers fix
	//if (this != cheat::main::local()) {

	//	if ((*(uint8_t*)((uintptr_t)this + 0x270) & 1) == 0)
	//		*(uint8_t*)((uintptr_t)this + 0x270) |= 1;

	//	if (*(uint16_t*)((uintptr_t)this + 0x268) == -1)
	//		*(uint16_t*)((uintptr_t)this + 0x268) = 0;
	//}

	//OnClientWeaponChange fix
	//auto backup_valid = get_animation_state()->m_invalid;
	//get_animation_state()->m_invalid = 0;

	//float saved_time_backup = *reinterpret_cast< float* >((reinterpret_cast< uintptr_t >(Source::m_pPrediction) + 0x5C));
	//*reinterpret_cast< float* >((reinterpret_cast< uintptr_t >(Source::m_pPrediction) + 0x5C)) = Source::m_pGlobalVars->curtime;

	auto effects = *(int*)((uintptr_t)this + 0xEC);
	*(int*)((uintptr_t)this + 0xEC) |= 8;
	//*(int*)((uintptr_t)this + 0x68) |= 2;
	//const auto v26 = *(DWORD *)((uintptr_t)this + 0x2670);
	//*(DWORD *)((uintptr_t)this + 0x2670) = 0;
	cheat::main::setup_bones = true;
	bool ret = SetupBones(nullptr, -1, 0x7FF00, Source::m_pGlobalVars->curtime);
	cheat::main::setup_bones = false;
	//*(DWORD *)((uintptr_t)this + 0x2670) = v26;
	*(int*)((uintptr_t)this + 0xEC) = effects;

	Source::m_pCvar->FindVar("r_jiggle_bones")->SetValue(r_jiggle_bones);

	//*reinterpret_cast< float* >((reinterpret_cast< uintptr_t >(Source::m_pPrediction) + 0x5C)) = saved_time_backup;

	//get_animation_state()->m_invalid = backup_valid;
}

QAngle* C_BasePlayer::m_angEyeAngles_ptr()
{
	return (QAngle*)((DWORD)this + Engine::Displacement::DT_CSPlayer::m_angEyeAngles);
}

float& C_BasePlayer::m_flCycle()
{
	return *(float*)((DWORD)this + Engine::Displacement::DT_BaseAnimating::m_flCycle);
}

std::string C_BasePlayer::m_szNickName()
{
	player_info info;
	if (!Source::m_pEngine->GetPlayerInfo(entindex(), &info))
		return "";

	return std::string(info.name);
}

void C_BasePlayer::set_abs_angles(QAngle &origin)
{
	static auto setabsangles_ptr = Memory::Scan(cheat::main::clientdll, "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8");

	if (setabsangles_ptr)
		reinterpret_cast<void(__thiscall*)(C_BaseEntity*, const QAngle&)>(setabsangles_ptr)(this, origin);
}

HANDLE C_BasePlayer::m_hViewModel()
{
	static auto m_hViewModel = Engine::PropManager::Instance()->GetOffset("DT_BasePlayer", "m_hViewModel[0]");

	return *(HANDLE*)((DWORD)this + m_hViewModel);
}

CBaseHandle* C_BasePlayer::m_hObserverTarget()
{
	static auto m_hObserverTarget = Engine::PropManager::Instance()->GetOffset("DT_BasePlayer", "m_hObserverTarget");

	return (CBaseHandle*)((DWORD)this + m_hObserverTarget);
}

void C_BasePlayer::invalidate_anims()
{
	static auto p = Memory::Scan(cheat::main::clientdll, "55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56 83 E0 04");

	if (p)
		((void(__thiscall*)(C_BasePlayer*, int))p)(this, 0x8);
}

float& C_BasePlayer::m_flSimulationTime()
{
	return *(float*)((DWORD)this + Engine::Displacement::DT_BasePlayer::m_flSimulationTime);
}

float& C_BasePlayer::m_flAnimTime()
{
	return *(float*)((DWORD)this + Engine::Displacement::DT_BasePlayer::m_flSimulationTime - 8);
}

QAngle& C_BasePlayer::get_render_angles()
{
	static auto deadflag = Engine::PropManager::Instance()->GetOffset("DT_BasePlayer", "deadflag");
	return *(QAngle*)((DWORD)this + (deadflag + 0x4));
}

CCSGOPlayerAnimState* C_BasePlayer::get_animation_state()
{
	if (this == nullptr) return nullptr;

	return *reinterpret_cast<CCSGOPlayerAnimState**>((DWORD)this + (Engine::Displacement::DT_CSPlayer::m_bIsScoped - 0xa));
}

bool C_BasePlayer::get_multipoints(int ihitbox, std::vector<Vector>& points, matrix3x4_t mx[])
{
	points.clear();

	if (!this) return false;

	//"head", "body", "arms", "legs"

	if ((!cheat::Cvars.RageBot_MultipointHBoxes.Has(0) && ihitbox == HITBOX_HEAD ||
		(!cheat::Cvars.RageBot_MultipointHBoxes.Has(1) && ihitbox >= HITBOX_PELVIS && ihitbox <= HITBOX_UPPER_CHEST) ||
		(!cheat::Cvars.RageBot_MultipointHBoxes.Has(2) && ihitbox >= HITBOX_RIGHT_HAND && ihitbox <= HITBOX_LEFT_FOREARM) ||
		(!cheat::Cvars.RageBot_MultipointHBoxes.Has(3) && ihitbox >= HITBOX_RIGHT_THIGH && ihitbox <= HITBOX_LEFT_CALF) ||
		(!cheat::Cvars.RageBot_MultipointHBoxes.Has(4) && ihitbox >= HITBOX_RIGHT_FOOT && ihitbox <= HITBOX_LEFT_FOOT)))
		return false;

	const model_t* model = this->GetModel();

	if (!model)
		return false;

	studiohdr_t *pStudioHdr = Source::m_pModelInfo->GetStudioModel(model);

	if (!pStudioHdr)
		return false;

	mstudiobbox_t *hitbox = pStudioHdr->pHitbox(ihitbox, this->m_nHitboxSet());

	if (!hitbox)
		return false;

	float POINT_SCALE = (ihitbox == 0 ? cheat::Cvars.RageBot_PointScale_Head.GetValue() : cheat::Cvars.RageBot_PointScale_Body.GetValue()) * 0.01f;

	//if (ihitbox == HITBOX_LEFT_FOOT || ihitbox == HITBOX_RIGHT_FOOT)
	//	POINT_SCALE = cheat::Cvars.RageBot_PointScale_Foot.GetValue() * 0.01f;

	const auto mod = hitbox->radius != -1.f ? hitbox->radius : 0.f;

	Vector min, max;
	Math::VectorTransform(hitbox->bbmin - mod, mx[hitbox->bone], min);
	Math::VectorTransform(hitbox->bbmax + mod, mx[hitbox->bone], max);

	Vector mins, maxs;
	Math::VectorTransform(hitbox->bbmin, mx[hitbox->bone], mins);
	Math::VectorTransform(hitbox->bbmax, mx[hitbox->bone], maxs);

	auto center = (min + max) * 0.5f;
	auto centered = (mins + maxs) * 0.5f;

	if (hitbox->radius == -1.f)
	{
		if (ihitbox != 11 && ihitbox != 12)
		{
			// center and all the points of the hitbox, stolen from Kiro / Kamay
			Vector point[] = { ((hitbox->bbmin + hitbox->bbmax) * .5f), // center
				Vector(hitbox->bbmin.x, hitbox->bbmin.y, hitbox->bbmin.z), // left bottom back corner
				Vector(hitbox->bbmin.x, hitbox->bbmax.y, hitbox->bbmin.z), // left bottom front corner
				Vector(hitbox->bbmax.x, hitbox->bbmax.y, hitbox->bbmin.z), // left top front corner
				Vector(hitbox->bbmax.x, hitbox->bbmin.y, hitbox->bbmin.z), // left top back corner
				Vector(hitbox->bbmax.x, hitbox->bbmax.y, hitbox->bbmax.z), // right top front corner // 5
				Vector(hitbox->bbmin.x, hitbox->bbmax.y, hitbox->bbmax.z), // right bottom front corner
				Vector(hitbox->bbmin.x, hitbox->bbmin.y, hitbox->bbmax.z), // right bottom back corner
				Vector(hitbox->bbmax.x, hitbox->bbmin.y, hitbox->bbmax.z)  // right top back corner
			};

			for (auto index = 0; index < (sizeof(point) / sizeof(Vector)); ++index) {
				auto dummy = Vector(0, 0, 0);
				Math::VectorTransform(point[index], mx[hitbox->bone], dummy);
				points.emplace_back(dummy);
			}
		}
		else
		{
			auto v49 = (hitbox->bbmax - hitbox->bbmin).Length();
			Vector v59 = Vector::Zero;
			Vector v76 = Vector::Zero;

			Math::MatrixAngles(mx[hitbox->bone], &v59, &v76);

			auto v70 = center + ((v76 * (v49 * 0.5f)) * 0.60f);
			//Vector dura;
			//Math::VectorTransform(v70, mx[hitbox->bone], dura);

			//if (ihitbox == 11)
			//	v70.z += 3.8f;

			points.emplace_back(v70);
			v70 = center - ((v76 * (v49 * 0.5f)) * 0.60f);
			//Math::VectorTransform(v70, mx[hitbox->bone], v70);

			//if (ihitbox == 12)
			//	v70.z += 3.8f;

			points.emplace_back(v70);
			points.emplace_back(center + Vector(0,0,1.f));
		}
	}
	else
	{
		const auto cur_angles = Math::CalcAngle(center, cheat::main::local()->GetEyePosition());

		Vector forward;
		Math::AngleVectors(cur_angles, &forward);

		auto rs = hitbox->radius * POINT_SCALE;

		const auto right = forward.Cross(Vector(0, 0, 1)) * rs;
		const auto left = Vector(-right.x, -right.y, right.z);
		const auto top = Vector(0, 0, 1) * rs;

		auto srs = hitbox->radius * (POINT_SCALE * 0.5f);

		const auto sright = forward.Cross(Vector(0, 0, 1)) * srs;
		const auto sleft = Vector(-sright.x, -sright.y, sright.z);
		const auto stop = Vector(0, 0, 1) * srs;

		if (ihitbox <= 1) {
			points.emplace_back(center + top);
			points.emplace_back(center + stop);
		}

		//points.emplace_back(center + top * 0.95f);
		points.emplace_back(center + right);
		points.emplace_back(center + left);

		points.emplace_back(center + sright);
		points.emplace_back(center + sleft);

		points.emplace_back(centered);
	}

	return true;
}

void C_BasePlayer::set_collision_bounds(const Vector mins, const Vector maxs)
{
	static auto _SetCollisionBounds = Memory::Scan(cheat::main::clientdll, "53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 10 56 57 8B 7B");

	if (_SetCollisionBounds && GetCollideable())
		reinterpret_cast<void(__thiscall*)(ICollideable*, const Vector&, const Vector&)>(_SetCollisionBounds)(GetCollideable(), mins, maxs);
}

void *C_BasePlayer::animation_layers_ptr()
{
	return *(void**)((DWORD)this + 0x2970);
}

C_AnimationLayer &C_BasePlayer::animation_layer(int i)
{
	return *(C_AnimationLayer*)((DWORD)this + 0x2970 + (56 * i));
}

C_AnimationLayer &C_BasePlayer::get_animation_layer(int index, bool eblan)
{
	// ref: CCBaseEntityAnimState::ComputePoseParam_MoveYaw
	// move_x move_y move_yaw

	//C_AnimationLayer null;

	typedef C_AnimationLayer&(__thiscall* Fn)(void*, int, bool);
	static Fn fn = NULL;

	if (!fn)
		fn = (Fn)Memory::Scan(cheat::main::clientdll, "55 8B EC 57 8B F9 8B 97 ? ? ? ? 85 D2");

	index = Math::clamp(index, 0, get_animation_layers_count());

	if (!eblan && fn)
	{
		//return fn(*(PVOID*)(*(DWORD*)(uintptr_t(this) + 0x3890) + 0x1C), index, 1);
		return fn(this, index, 1);
	}
	else {
		return animation_layer(index);
	}
}

int C_BasePlayer::get_animation_layers_count()
{
	if (this == nullptr)
		return 13;

	return *(int*)((DWORD)this + 0x297C);
}

Vector& C_BasePlayer::m_vecVelocity()
{
	return *( Vector* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_vecVelocity );
}

int& C_BasePlayer::m_iEFlags()
{
	return *(int*)((uintptr_t)this + 0xE4);
}

Vector& C_BasePlayer::m_vecAbsVelocity()
{
	return *(Vector*)((uintptr_t)this + 0x90);
}

float &C_BasePlayer::m_flOldSimulationTime()
{
	return *(float *)((uintptr_t)this + (Engine::Displacement::DT_BasePlayer::m_flSimulationTime + 4));
}

//void C_BasePlayer::calc_anim_velocity(sm_animdata_t* sm_animdata, bool reset) {
//	int idx = this->entindex() - 1;
//
//	auto VectorMultiply = [](const Vector &a, float b, Vector &c)
//	{
//		CHECK_VALID(a);
//		Assert(IsFinite(b));
//		c.x = a.x * b;
//		c.y = a.y * b;
//		c.z = a.z * b;
//	};
//
//	auto friction = [VectorMultiply](Vector out_vel) -> Vector {
//		float speed, newspeed, control;
//		float friction;
//		float drop;
//
//		Vector new_vel;
//
//		speed = out_vel.Length();
//
//		if (speed <= 0.1f)
//			return out_vel;
//
//		drop = 0;
//
//		if (cheat::main::local()->m_fFlags() & FL_ONGROUND) {
//			friction = Source::m_pCvar->FindVar("sv_friction")->GetFloat() * cheat::main::local()->m_surfaceFriction();
//
//			control = (speed < Source::m_pCvar->FindVar("sv_stopspeed")->GetFloat()) ? Source::m_pCvar->FindVar("sv_stopspeed")->GetFloat() : speed;
//
//			drop += control * friction * Source::m_pGlobalVars->frametime;
//		}
//
//		newspeed = speed - drop;
//		if (newspeed < 0)
//			newspeed = 0;
//
//		if (newspeed != speed) {
//			newspeed /= speed;
//
//			VectorMultiply(out_vel, newspeed, new_vel);
//		}
//
//		return new_vel;
//	};
//
//	/*secret antipaste movement code*/
//
//	if (reset) {
//		auto velocity = m_vecVelocity();
//		sm_animdata/*[idx]*/->m_lastOrigin = m_vecOrigin();
//		sm_animdata/*[idx]*/->m_lastVelocity = velocity;
//		sm_animdata/*[idx]*/->m_animVelocity = velocity;
//	}
//	else {
//		static auto sv_accelerate = Source::m_pCvar->FindVar("sv_accelerate");
//
//		float delta = m_flSimulationTime() - m_flOldSimulationTime();
//		delta = max(delta, Source::m_pGlobalVars->interval_per_tick); // TICK_INTERVAL()
//
//		bool on_ground = m_fFlags() & FL_ONGROUND;
//
//		auto origin = m_vecOrigin();
//		auto origin_delta = origin - sm_animdata/*[idx]*/->m_lastOrigin;
//
//		auto velocity = origin_delta / delta;
//		auto last_velocity = sm_animdata/*[idx]*/->m_lastVelocity;
//
//		Vector anim_vel;
//
//		if (on_ground) {
//			QAngle ang;
//			Math::VectorAngles(Vector::Zero, velocity, ang);
//
//			auto move_yaw = 0.f;
//
//			QAngle move_yaw_angle; Math::VectorAngles(velocity, move_yaw_angle);
//
//			move_yaw = move_yaw_angle.y;
//
//			auto move_delta = 0.f;
//			QAngle move_delta_angle; Math::VectorAngles(last_velocity, move_delta_angle); move_delta = move_delta_angle.y;
//			move_delta -= move_yaw;
//			move_delta = std::remainderf(move_delta, 360.f);
//
//			Vector move_dir;
//
//			Vector move_dir_vec;
//			Math::AngleVectors(QAngle(0.f, move_delta, 0.f), &move_dir_vec);
//
//			move_dir = move_dir_vec * 450.f;
//
//			Vector forward, right, up;
//			Math::AngleVectors(ang, &forward, &right, &up);
//
//			Vector wishdir;
//			/*super secret antipaste math code*/
//
//			anim_vel = friction(last_velocity);
//			if (anim_vel.Length2D() < 1.f)
//				anim_vel = Vector::Zero;
//
//
//			int ticks = TIME_TO_TICKS(delta);
//			Vector est_tick_vel;
//			/*super secret fakewalk detection code*/
//
//			if (velocity.Length2D() > last_velocity.Length2D())
//				anim_vel = RebuildGameMovement::Get().Accelerate/*accelerate*/(this, anim_vel, wishdir, 250.f, sv_accelerate->GetFloat());
//
//			//assume fakewalk
//			float last_speed = sm_animdata/*[idx]*/->m_animVelocity.Length2D();
//			if (anim_vel.Length2D() >= last_speed && est_tick_vel.Length2D() < 1.f)
//				anim_vel = Vector::Zero;
//		}
//		else {
//			//doesn't matter much anyway
//			anim_vel = Math::Lerp(Source::m_pGlobalVars->interval_per_tick / delta, 
//				last_velocity,
//				velocity
//			);
//		}
//
//		sm_animdata/*[idx]*/->m_animVelocity = anim_vel;
//		sm_animdata/*[idx]*/->m_lastVelocity = velocity;
//		sm_animdata/*[idx]*/->m_lastOrigin = origin;
//	}
//}

Vector& C_BasePlayer::m_vecBaseVelocity()
{
	//return *( Vector* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_vecBaseVelocity );
	static unsigned int _m_vecBaseVelocity = FindInDataMap(GetPredDescMap(), "m_vecBaseVelocity");
	return *(Vector*)((uintptr_t)this + _m_vecBaseVelocity);
}

float& C_BasePlayer::m_flFallVelocity()
{
	return *( float* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_flFallVelocity );
}

char& C_BasePlayer::m_lifeState()
{
	return *(char*)(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_lifeState);
}

int& C_BasePlayer::m_nTickBase()
{
	return *( int* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_nTickBase );
}

int& C_BasePlayer::m_iHealth()
{
	return *( int* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_iHealth );
}

int& C_BasePlayer::m_fFlags()
{
	return *( int* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_fFlags );
}

bool & C_BasePlayer::m_bIsScoped()
{
	return *(bool*)(uintptr_t(this) + Engine::Displacement::DT_CSPlayer::m_bIsScoped);
}

bool C_BasePlayer::IsDead()
{
	return ( this->m_lifeState() );
}

void C_BasePlayer::SetCurrentCommand( CUserCmd* cmd )
{
	*( CUserCmd** )(uintptr_t(this) + Engine::Displacement::C_BasePlayer::m_pCurrentCommand ) = cmd;
}

C_CSPlayer* C_CSPlayer::GetLocalPlayer()
{
	auto index = Source::m_pEngine->GetLocalPlayer();

	if( !index )
		return nullptr;

	auto client = Source::m_pEntList->GetClientEntity( index );

	if( !client )
		return nullptr;

	return ToCSPlayer( client->GetBaseEntity() );
}

int * C_BasePlayer::m_hMyWeapons()
{
	return (int*)((DWORD)this + Engine::Displacement::DT_BaseCombatCharacter::m_hActiveWeapon - 256);
}

QAngle& C_CSPlayer::m_angEyeAngles()
{
	return *( QAngle* )(uintptr_t(this) + Engine::Displacement::DT_CSPlayer::m_angEyeAngles );
}

float C_BasePlayer::m_anim_time()
{
	return (m_flOldSimulationTime() + Source::m_pGlobalVars->interval_per_tick);
}

void C_BasePlayer::UpdateAnimationState(CCSGOPlayerAnimState *state, QAngle angle)
{
	if (!state)
		return;

	angle.y = Math::normalize_angle(angle.y);

	static auto UpdateAnimState = Memory::Scan(cheat::main::clientdll, "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24");
	if (!UpdateAnimState)
		return;

	__asm
	{
#ifdef _DEBUG
		push 1
#endif // _DEBUG

		mov ecx, state


		movss xmm1, dword ptr[angle + 4]
		movss xmm2, dword ptr[angle]

		call UpdateAnimState
	}
}

void C_BasePlayer::SetupVelocity(CCSGOPlayerAnimState *state)
{
	if (!state)
		return;

	typedef void(__thiscall* Fn)(CCSGOPlayerAnimState*, double);
	static Fn fn = NULL;

	if (!fn)
		fn = (Fn)Memory::Scan(cheat::main::clientdll, "55 8B EC 83 E4 F8 83 EC 30 56 57 8B 3D ? ? ? ?");
	
	if (!fn)
		return;

	return fn(state,1);
}

std::vector<Vector> C_BasePlayer::build_capsules(Vector Min, Vector Max, float Radius, float scale, matrix3x4_t& Transform)
{
	//1 ряд образует шапки над плоскостями цилиндра, содержит все нужные поинты
	//2 ряд образует цилиндр(бесполезно для мультипоинтов)
	//3 ряд образует контакты верхней плоскости цилиндра с нижней | 0 и 2 верхние боковые грани, 4 задняя верхняя могут быть использованы как альтернатива 1 ряду

	std::vector <Vector> points;

	points.clear();

	Vector Forward = (Max - Min);
	Forward.Normalize();
	Forward *= (Radius * scale);
	QAngle Ref;
	Vector Up;
	Vector Right;
	Vector Forward2;
	if (!Forward.LengthSquared()) Forward.x += 0.0001f;
	Math::VectorAngles(Forward, Ref);
	Math::AngleVectors(Ref, &Forward2, &Right, &Up);
	Up *= (Radius * scale);
	Right *= (Radius * scale);
	//float ang_step = (2.0f * M_PI) / 16.0f; //original step(rip fps)
	float ang_step = (2.0f * M_PI) / 6.0f;

	for (float Angle = 0; Angle < M_PI; Angle += ang_step)
	{
		Vector Temp;
		Vector Temp2;
		Temp = Max + Right * cosf(Angle) + Forward * sinf(Angle);

		Math::VectorTransform(Temp, Transform, Temp2);

		points.emplace_back(Temp2);
	}

	Vector Temp[6];
	Vector Temp2[6];
	Temp[0] = Max + Up;
	Temp[1] = Min + Up;
	Temp[2] = Max - Up;
	Temp[3] = Min - Up;
	Temp[4] = Max + Right;
	Temp[5] = Max - Right;

	for (int i = 0; i < 6; i++)
		Math::VectorTransform(Temp[i], Transform, Temp2[i]);

	points.emplace_back(Temp2[2]);
	points.emplace_back(Temp2[0]);
	points.emplace_back(Temp2[3]);
	points.emplace_back(Temp2[1]);
	points.emplace_back(Temp2[5]);
	points.emplace_back(Temp2[4]);

	//0 / 2 , 22, 20, 23, 21, 26, 24

	return points;
}

bool C_BasePlayer::SetupFuckingBones(matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime, QAngle* angles_override)
{
	//auto hdr = this->GetModelPtr();
	//if (!hdr)
	//	return false;

	//auto accessor = this->GetBoneAccessor();

	//if (!accessor)
	//	return false;

	//auto backup_matrix = accessor->GetBoneArrayForWrite();

	//if (!backup_matrix)
	//	return false;

	////entity->force_bone_rebuild();

	//auto origin = this->m_vecOrigin();
	//auto angles = this->m_angEyeAngles();
	//auto backup_origin = this->get_abs_origin();
	//auto backup_angles = this->get_abs_eye_angles();

	//std::array<float, 24> backup_poses;
	//backup_poses = this->m_flPoseParameter();

	//C_AnimationLayer backup_layers[15];
	//memcpy(backup_layers, this->animation_layers_ptr(), (sizeof(C_AnimationLayer) * this->get_animation_layers_count()));

	//const auto _backup = *(byte*)(uintptr_t(this) + 0x270);
	//*(byte*)(uintptr_t(this) + 0x270) = 0;

	//this->set_abs_origin(origin);
	//this->set_abs_angles(QAngle(0, angles.y,0));

	////alignas(16) Vector pos[128]; //auto pos = (Vector*)(Source::m_pMemAlloc->Alloc(sizeof(Vector[128])));
	////alignas(16) Quaternion q[128]; //auto q = (Quaternion*)(Source::m_pMemAlloc->Alloc(sizeof(Quaternion[128])));
	////std::memset(pos, 0xFF, sizeof(pos));
	////std::memset(q, 0xFF, sizeof(q));

	////this->StandardBlendingRules(hdr, &pos[0], &q[0], currentTime, boneMask);

	////accessor->m_pBones = paste;

	////byte computed[0x1000] = { 0 }; //auto computed = (byte*)(Source::m_pMemAlloc->Alloc(sizeof(byte[0x20])));
	//////std::memset(computed, 0, sizeof(byte[0x20]));

	////this->BuildTransformations(hdr, &pos[0], &q[0], parentTransform, boneMask, &computed[0]);
	////
	////accessor->m_pBones = backup_matrix;

	//alignas(16) matrix3x4_t tmpAligned[128];
	//alignas(16) matrix3x4_t parentTransform;
	//alignas(16) Quaternion q[128];
	//alignas(16) Vector pos[128];
	//byte computed[0x1000] = { 0 }; //0x20?

	//accessor->m_pBones = tmpAligned;
	//accessor->m_WritableBones = boneMask;
	//accessor->m_ReadableBones = boneMask;

	//Math::AngleMatrix(angles, origin, parentTransform);
	//StandardBlendingRules(hdr, &pos[0], &q[0], currentTime, boneMask);
	//BuildTransformations(hdr, &pos[0], &q[0], parentTransform, boneMask, &computed[0]);

	//accessor->m_pBones = backup_matrix;

	//this->set_abs_origin(backup_origin);
	//this->set_abs_angles(backup_angles);
	//this->m_flPoseParameter() = backup_poses;
	//memcpy(this->animation_layers_ptr(), backup_layers, (sizeof(C_AnimationLayer) * this->get_animation_layers_count()));

	//*(byte*)(uintptr_t(this) + 0x270) = _backup;

	///*if (tmpAligned)
	//	*(matrix3x4_t**)accessor->m_pBones = (matrix3x4_t*)&tmpAligned;*/

	//int maxBones = min(128, hdr->m_pStudioHdr->numbones);
	//if (accessor->m_pBones)
	//{
	//	for (int i = 0; i < maxBones; i++)
	//		accessor->m_pBones[i] = tmpAligned[i];
	//}

	////memcpy(accessor->m_pBones, paste, sizeof(matrix3x4a_t) * accessor->m_ReadableBones/*sizeof(matrix3x4a_t) * 128*/);

	//if (pBoneToWorldOut)
	//	pBoneToWorldOut = (matrix3x4_t*)&tmpAligned;

	//return true;

	auto hdr = this->GetModelPtr();

	if (!hdr)
		return false;

	auto boneAccessor = GetBoneAccessor();

	if (!boneAccessor)
		return false;

	auto backup_matrix = boneAccessor->m_pBones;

	if (!backup_matrix)
		return false;

	auto rend = GetClientRenderable();

	if (!rend)
		return false;

	//if (!this->get_animation_state())

	//force_bone_rebuild();

	alignas(16) matrix3x4_t tmpAligned[128];
	alignas(16) matrix3x4_t parentTransform;
	alignas(16) Quaternion q[128];
	alignas(16) Vector pos[128];
	byte computed[0x100] = { 0 }; //0x20?

	boneAccessor->m_pBones = tmpAligned;

	memset(&computed[0], 0, 0x100u);
	//boneMask |= 0x0007FF00;

	//boneAccessor->m_WritableBones = boneMask;
	//boneAccessor->m_ReadableBones = boneMask;

	auto origin = this->get_abs_origin();
	//auto angles = this->m_angEyeAngles();

	//auto backup_origin = this->get_abs_origin();
	auto backup_angles = this->get_abs_eye_angles();

	//set_abs_origin(origin);
	//auto new_abs_angles = QAngle(0, this->get_animation_state()->m_flGoalFeetYaw, 0);
	//set_abs_angles(new_abs_angles);

	auto angles = backup_angles;

	if (angles_override)
		angles.y = angles_override->y;

	/*auto backup_poses = m_flPoseParameter();
	C_AnimationLayer backup_layers[15];

	memcpy(backup_layers, animation_layers_ptr(), 0x38 * get_animation_layers_count());

	if (this != cheat::main::local()) {
		auto animrecord = cheat::features::lagcomp.records[entindex() - 1].m_AnimationRecord;

		if (animrecord.simulation_time != 0.f)
		{
			if (angles_override)
				animrecord.pose_paramaters.at(11) = (angles_override->y + 60) / 180;

			m_flPoseParameter() = animrecord.pose_paramaters;

			memcpy(animation_layers_ptr(), animrecord.anim_layers, 0x38 * get_animation_layers_count());
		}
	}
	else
		angles.x = 0.f;*/

	Math::AngleMatrix(angles, origin, parentTransform);
	StandardBlendingRules(hdr, &pos[0], &q[0], currentTime, 0x100);
	BuildTransformations(hdr, &pos[0], &q[0], parentTransform, 0x100, &computed[0]);

	//m_flPoseParameter() = backup_poses;
	//memcpy(animation_layers_ptr(), backup_layers, 0x38 * get_animation_layers_count());
	//set_abs_origin(backup_origin);
	//set_abs_angles(backup_angles);

	boneAccessor->m_pBones = backup_matrix;

	//CUtlVectorSimple* CachedBoneData = (CUtlVectorSimple*)((uintptr_t)rend + 0x28FC);

	if (pBoneToWorldOut)
		memcpy((void*)pBoneToWorldOut, boneAccessor->m_pBones, sizeof(matrix3x4_t) * hdr->m_pStudioHdr->numbones);

	GetBoneCount() = hdr->m_pStudioHdr->numbones;
	memcpy(m_CachedBoneData().Base(), boneAccessor->m_pBones, hdr->m_pStudioHdr->numbones * sizeof(matrix3x4_t));

	return true;
}