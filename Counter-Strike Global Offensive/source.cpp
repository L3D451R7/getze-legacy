#include "source.hpp"
#include "hooked.hpp"

#include "prop_manager.hpp"
#include "displacement.hpp"
#include <fstream>

#include "player.hpp"
#include "menu.h"
#include "visuals.hpp"
#include "aimbot.hpp"
#include "lag_compensation.hpp"
#include "anti_aimbot.hpp"
#include "angle_resolver.hpp"
#include "autowall.hpp"
#include "detours.h"
#include <intrin.h>
#include <time.h>
#include "game_events.h"
#include <iostream>
#include "sdk.hpp"
#include "rmenu.hpp"
#include "sound_parser.hpp"
#include "prediction.hpp"

constexpr auto s_enginesnd = LIT(("IEngineSoundClient"));
constexpr auto s_sig_clientsideanims = LIT(("A1 ?? ?? ?? ?? F6 44 F0 04 01 74 0B"));
constexpr auto s_sig_effectshead = LIT(("8B 35 ? ? ? ? 85 F6 0F 84 ? ? ? ? BB FF FF ? ? 8B 0E"));
constexpr auto s_prop_smoketick = LIT(("m_nSmokeEffectTickBegin"));
constexpr auto s_prop_simtime = LIT(("m_flSimulationTime"));
constexpr auto s_wndproc_valve = LIT(("Valve001"));
constexpr auto s_hook_impact = LIT(("Impact"));

constexpr auto enginedll = LIT(("engine.dll"));
constexpr auto clientdll = LIT(("client.dll"));

typedef int(__thiscall* StandardBlendingRules_t)(C_BasePlayer*, CStudioHdr*, Vector*, Quaternion*, float_t, int32_t);
typedef const char* (__thiscall* GetForeignFallbackFontName_t)(void*);
typedef void(__thiscall* PhysicsSimulate_t)(void*);
typedef void(__thiscall *LockCursor_t)(void*);
typedef bool(__thiscall *isHLTV_t)(IVEngineClient*);
typedef void(__thiscall *FireEvents_t)(IVEngineClient*);
typedef void(__thiscall *UpdateClientSideAnimations_t)(C_BasePlayer*);
typedef void(__thiscall* ModifyEyePosition_t)(CCSGOPlayerAnimState*, Vector*);
typedef bool(__thiscall *SendNetMsg_t)(INetChannel* pNetChan, INetMessage& msg, bool bForceReliable, bool bVoice);
typedef int(__stdcall* IsBoxVisible_t)(const Vector&, const Vector&);
typedef void(__thiscall* DoExtraBonesProcessing_t)(uintptr_t* ecx, CStudioHdr* hdr, Vector* pos, Quaternion* rotations, const matrix3x4_t&, uint8_t*, void*);

DWORD OriginalSetupBones;
DWORD OriginalUpdateClientSideAnimations;
DWORD OriginalSetupVelocity;
DWORD OriginalCalcAbsoluteVelocity;
DWORD OriginalStandardBlendingRules;
DWORD OriginalDoExtraBonesProcessing;
DWORD OriginalGetForeignFallbackFontName;
DWORD OriginalModifyEyePosition;
DWORD OriginalPhysicsSimulate;

typedef bool(__thiscall *TempEntities_t)(void*, void*/*SVC_TempEntities*/);
TempEntities_t oTempEntities;

typedef bool(__thiscall *SetupBones_t)(void*, matrix3x4_t*, int, int, float);
SetupBones_t o_SetupBones;

typedef bool(__thiscall* sv_impacts_int)(void*);

//typedef void(__thiscall* FireBullets_t)(C_TEFireBullets*, int);
//FireBullets_t oFireBullets;

//void __stdcall createmove_thread()
//{
//	auto m_pClientMode = **(void***)((*(DWORD**)Source::m_pClient)[10] + 5);
//
//	if (!m_pClientMode)
//	{
//		Win32::Error("ClientMode is nullptr (Source::%s)", __FUNCTION__);
//		Source::Destroy();
//		return;
//	}
//
//	Source::m_pClientModeSwap = std::make_shared<Memory::VmtSwap>(m_pClientMode);
//	Source::m_pClientModeSwap->Hook(&Hooked::CreateMove, Index::IBaseClientDLL::CreateMove);
//	Source::m_pClientModeSwap->Hook(&Hooked::GetViewModelFOV, Index::IBaseClientDLL::GetViewModelFOV);
//	Source::m_pClientModeSwap->Hook(&Hooked::OverrideView, Index::IBaseClientDLL::OverrideView);
//}

RecvVarProxyFn m_nSmokeEffectTickBegin;
RecvVarProxyFn m_flSimulationTime;
RecvVarProxyFn m_flAbsYaw;
RecvVarProxyFn m_bClientSideAnimation;
ClientEffectCallback oImpact;

namespace Hooked
{
	void __stdcall LockCursor()
	{
		if (cheat::features::menu.menu_opened) {
			Source::m_pSurface->UnlockCursor();
			return;
		}

		Source::m_pSurfaceSwap->VCall<LockCursor_t>(67)(Source::m_pSurface);
	}

	_declspec(noinline)void PhysicsSimulate_Detour(uintptr_t* ecx)
	{
		//auto v27 = *(bool*)((DWORD)ecx + 0x34D0 + 0x4C);

		if (ecx && cheat::main::local() && !cheat::main::local()->IsDead() && (DWORD)cheat::main::local() == (DWORD)ecx) {
			const auto m_nSimulationTick = *(int*)((DWORD)ecx + 0x2A8);
			const auto needsprocessing = *(bool*)((DWORD)ecx + 0x35F8);
			cheat::main::can_store_netvars = Source::m_pGlobalVars->tickcount != m_nSimulationTick/* && needsprocessing*/; //m_nSimulationTick && needsprocessing

			if (cheat::main::can_store_netvars)
				Engine::Prediction::Instance()->RestoreNetvars(true);
		}
		else
			cheat::main::can_store_netvars = false;

		((PhysicsSimulate_t)OriginalPhysicsSimulate)(ecx);

		if (cheat::main::can_store_netvars && cheat::main::run_cmd_got_called) {
			Engine::Prediction::Instance()->OnRunCommand(cheat::main::local());
			cheat::main::can_store_netvars = cheat::main::run_cmd_got_called = false;
		}

		cheat::main::can_store_netvars = false;
	}

	void __fastcall PhysicsSimulate(uintptr_t* ecx, uint32_t)
	{
		PhysicsSimulate_Detour(ecx);
	}

	_declspec(noinline)void DoExtraBonesProcessing_Detour(uintptr_t* ecx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_computed, void* context)
	{
		if (cheat::main::local() && !cheat::main::local()->IsDead() && ecx != nullptr && *(int*)((DWORD)ecx + 0x64) <= 64)
			return;

		((DoExtraBonesProcessing_t)OriginalDoExtraBonesProcessing)(ecx, hdr, pos, q, matrix, bone_computed, context);
	}

	void __fastcall DoExtraBonesProcessing(uintptr_t* ecx, uint32_t, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_computed, void* context)
	{
		DoExtraBonesProcessing_Detour(ecx, hdr, pos, q, matrix, bone_computed, context);
	}

	_declspec(noinline)void ModifyEyePosition_Detour(CCSGOPlayerAnimState* ecx, Vector* pos)
	{
		auto force = ecx != nullptr && ecx->ent == cheat::main::local() && !cheat::main::local()->IsDead() && cheat::main::fix_modify_eye_pos;

		if (force)
		{
			/* LMAO */
			static matrix3x4_t backup[128];
			memcpy(backup, cheat::main::local()->m_CachedBoneData().Base(), cheat::main::local()->GetBoneCount() * sizeof(matrix3x4_t));
			memcpy(cheat::main::local()->m_CachedBoneData().Base(), cheat::main::zero_matrix, cheat::main::local()->GetBoneCount() * sizeof(matrix3x4_t)); //force matrix with zero pitch XDDDDDDDD
			((ModifyEyePosition_t)OriginalModifyEyePosition)(ecx, pos);
			memcpy(cheat::main::local()->m_CachedBoneData().Base(), backup, cheat::main::local()->GetBoneCount() * sizeof(matrix3x4_t));
		}
		else
			((ModifyEyePosition_t)OriginalModifyEyePosition)(ecx, pos);
	}

	void __fastcall ModifyEyePosition(CCSGOPlayerAnimState* ecx, uint32_t, Vector* pos)
	{
		ModifyEyePosition_Detour(ecx, pos);
	}

	void __fastcall FireEvents(IVEngineClient *_this, void* EDX)
	{
		if (cheat::Cvars.RageBot_AdjustPositions.GetValue() && cheat::main::local() && !cheat::main::local()->IsDead() && Source::m_pClientState->m_iDeltaTick > 0) {
			for (auto i = Source::m_pClientState->m_ptrEvents; i; i = i->next)
				i->fire_delay = 0.0f;
		}

		Source::m_pEngineSwap->VCall<FireEvents_t>(59)(_this);
	}

	_declspec(noinline)bool SetupBones_Detour(void* ECX, matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
	{
		if (!ECX)
			return ((SetupBones_t)OriginalSetupBones)(ECX, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

		auto v9 = (C_BasePlayer*)(uintptr_t(ECX) - 4);

		if ((DWORD)ECX == 0x4 || (*(int*)(uintptr_t(v9) + 0x64)) > 64 || !v9->IsPlayer())
			return ((SetupBones_t)OriginalSetupBones)(ECX, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

		const auto is_local = v9 == cheat::main::local() && v9 != nullptr;

		if (cheat::main::setup_bones || !cheat::main::local() || !is_local && v9->m_iTeamNum() == cheat::main::local()->m_iTeamNum() || cheat::main::local()->IsDead())
		{
			return ((SetupBones_t)OriginalSetupBones)(ECX, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
		}
		else {

			auto v25 = 128;

			if (nMaxBones <= 128)
				v25 = nMaxBones;

			if (pBoneToWorldOut)
			{
				if (v25 > 0)
					memcpy(pBoneToWorldOut, v9->m_CachedBoneData().Base(), sizeof(matrix3x4_t) * v25);

				return true;
			}
			return true;
		}
	}

	bool __fastcall SetupBones(void* ECX, void* EDX, matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
	{
		return SetupBones_Detour(ECX, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
	}

	_declspec(noinline)void StandardBlendingRules_Detour(C_BasePlayer* ent, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curtime, int32_t bonemask)
	{
		if (ent != nullptr && *(int*)(uintptr_t(ent) + 0x64) <= 64)
		{
			auto boneMask = bonemask;

			if (cheat::Cvars.RageBot_AdjustPositions.GetValue() && ent != cheat::main::local())
				boneMask = (0x100 | 0x200);

			((StandardBlendingRules_t)OriginalStandardBlendingRules)(ent, hdr, pos, q, curtime, boneMask);

			if (*(int*)(uintptr_t(ent) + 0xF0) & 8)
				* (int*)(uintptr_t(ent) + 0xF0) &= ~8;
		}
		else
		{
			((StandardBlendingRules_t)OriginalStandardBlendingRules)(ent, hdr, pos, q, curtime, bonemask);
		}
	}

	void __fastcall StandardBlendingRules(C_BasePlayer* a1, int a2, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curtime, int32_t boneMask)
	{
		StandardBlendingRules_Detour(a1, hdr, pos, q, curtime, boneMask);
	}

	bool __fastcall IsHLTV(IVEngineClient *_this, void* EDX)
	{
		/*
		C_CSPlayer::AccumulateLayers
		sub_1037A9B0+6    8B 0D B4 13 15 15                 mov     ecx, dword_151513B4
		sub_1037A9B0+C    8B 01                             mov     eax, [ecx]
		sub_1037A9B0+E    8B 80 74 01 00 00                 mov     eax, [eax+174h]
		sub_1037A9B0+14   FF D0                             call    eax
		sub_1037A9B0+16   84 C0                             test    al, al
		sub_1037A9B0+18   75 0D                             jnz     short loc_1037A9D7
		sub_1037A9B0+1A   F6 87 28 0A 00 00+                test    byte ptr [edi+0A28h], 0Ah ; ent check here, whatever
		sub_1037A9B0+21   0F 85 F1 00 00 00                 jnz     loc_1037AAC8
		*/
		static const auto accumulate_layers_call = (void*)Memory::Scan(cheat::main::clientdll, "84 C0 75 0D F6 87");
		static const auto setup_velocity = (void*)Memory::Scan(cheat::main::clientdll, "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80");

		if ((_ReturnAddress() == accumulate_layers_call || _ReturnAddress() == setup_velocity) && Source::m_pEngine->IsInGame() && cheat::main::local() && Source::m_pClientState->m_iDeltaTick > 0)
			return true;

		if (Source::m_pEngineSwap)
		return Source::m_pEngineSwap->VCall<isHLTV_t>(93)(_this);
	}

	static float spawntime = 0.f;

	_declspec(noinline)void crash_my_csgo(C_BasePlayer* ecx)
	{
		auto is_player = (Source::m_pEngine != nullptr && Source::m_pEngine->IsInGame() && (DWORD)ecx > 0x00001000 && ecx != nullptr && (*(int*)((DWORD)ecx + 0x64) - 1) <= 63) && !ecx->IsDead();
		auto is_local = is_player && cheat::main::local() == ecx;

		C_AnimationLayer backup_layers[15];

		if (/*(*/!is_local/* || cheat::main::updating_local)*/ && is_player)
		{ 
			if (cheat::main::updating_anims == true /*|| cheat::main::updating_local == true*/ || cheat::main::local() == nullptr || ecx->m_iTeamNum() == cheat::main::local()->m_iTeamNum() || cheat::main::local()->IsDead())
				((UpdateClientSideAnimations_t)OriginalUpdateClientSideAnimations)(ecx);

			return;
		}
		
		if (is_player && is_local && cheat::main::local() != nullptr && !cheat::main::local()->IsDead() && Source::m_pClientState->m_iDeltaTick > 0)
		{
			auto animstate = ecx->get_animation_state();

			if (animstate)
			{
				auto in_tp = cheat::main::thirdperson && cheat::Cvars.Visuals_lp_forcetp.GetValue();

				if (!Source::m_pClientState->m_iChockedCommands) 
				{
					if (in_tp)
						Source::m_pPrediction->SetLocalViewAngles(cheat::features::antiaimbot.last_sent_angle);

					memcpy(backup_layers, ecx->animation_layers_ptr(), 0x38 * ecx->get_animation_layers_count());

					const auto i_e_flags_backup = ecx->m_iEFlags();

					const auto curtime = Source::m_pGlobalVars->curtime;
					const auto frametime = Source::m_pGlobalVars->frametime;
					const auto interpamt = Source::m_pGlobalVars->interpolation_amount;

					Source::m_pGlobalVars->curtime = ecx->m_flOldSimulationTime() + Source::m_pGlobalVars->interval_per_tick;
					Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;
					Source::m_pGlobalVars->interpolation_amount = 0.f;

					if (animstate->last_anim_upd_tick == Source::m_pGlobalVars->framecount)
						animstate->last_anim_upd_tick = Source::m_pGlobalVars->framecount - 1;

					animstate->unk_frac = 0.f;

					// EFL_DIRTY_ABSVELOCITY
					// skip call to C_BaseEntity::CalcAbsoluteVelocity
					//ecx->m_iEFlags() &= ~0x1000;
					//ecx->m_vecAbsVelocity() = ecx->m_vecVelocity();

					animstate->feet_rate = 0.f;
					animstate->eye_yaw = cheat::features::antiaimbot.last_sent_angle.y;

					if (ecx->m_fFlags() & FL_ONGROUND)
						animstate->time_to_land = 0.f;

					((UpdateClientSideAnimations_t)OriginalUpdateClientSideAnimations)(ecx);

					cheat::main::real_angle = animstate->abs_yaw;

					memcpy(ecx->animation_layers_ptr(), backup_layers, 0x38 * ecx->get_animation_layers_count());

					auto sixth_overlay = &ecx->get_animation_layer(6);

					if (sixth_overlay && cheat::main::fakewalking) {
						sixth_overlay->m_flWeight = sixth_overlay->m_flPlaybackRate = 0.0f;
						ecx->m_flPoseParameter()[3] = 0.f;
						ecx->m_flPoseParameter()[4] = 0.f;
					}

					ecx->m_iEFlags() = i_e_flags_backup;

					Source::m_pGlobalVars->curtime = curtime;
					Source::m_pGlobalVars->frametime = frametime;
					Source::m_pGlobalVars->interpolation_amount = interpamt;
				}

				ecx->set_abs_angles(QAngle(0, cheat::main::real_angle, 0));

				ecx->force_bone_rebuild();
				ecx->SetupBonesEx();

				if (in_tp)
					Source::m_pPrediction->SetLocalViewAngles(cheat::features::antiaimbot.last_fake_angle);
				//ecx->DrawServerHitboxes();
			}
			else 
				((UpdateClientSideAnimations_t)OriginalUpdateClientSideAnimations)(ecx);
		}
		else
			((UpdateClientSideAnimations_t)OriginalUpdateClientSideAnimations)(ecx);
	}

	bool __fastcall SendNetMsg(INetChannel* pNetChan, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice)
	{
		using Fn = bool(__thiscall*)(INetChannel* pNetChan, INetMessage& msg, bool bForceReliable, bool bVoice);
		auto ofc = Source::m_pNetChannelSwap->VCall<Fn>(42);

		//if (msg.GetGroup() == 11) {
			//if (cheat::game::pressed_keys[78])
			//	cheat::features::antiaimbot.shift_ticks = 6;

		//	uintptr_t uiMsg = (uintptr_t)(&msg);

		//	TickbaseManipulation((CCLCMsg_Move_t*)&msg);
		//}

		if (msg.GetType() == 14) // Return and don't send messsage if its FileCRCCheck
			return false;

		if (msg.GetGroup() == 9) // Fix lag when transmitting voice and fakelagging
			bVoice = true;

		//static auto pasta = (void*)((DWORD)GetModuleHandleA("engine.dll") +0xCCE19);

		return ofc(pNetChan, msg, bForceReliable, bVoice);
	}

	void __fastcall UpdateClientSideAnimation(C_BasePlayer* ecx, void* edx)
	{
		crash_my_csgo(ecx);
	}

	_declspec(noinline)void detour_my_csgo(void* ecx, float accumulated_extra_samples, bool bFinalTick)
	{
	
	}

	void __fastcall CL_Move(void* ecx, void* edx, float accumulated_extra_samples, bool bFinalTick)
	{
		detour_my_csgo(ecx, accumulated_extra_samples, bFinalTick);
	}

	void m_nSmokeEffectTickBeginHook(const CRecvProxyData* pData, void* pStruct, void* pOut) 
	{

		auto begin = (BYTE*)((DWORD)pStruct + pData->m_pRecvProp->m_Offset + 5);

		if (begin &&  cheat::Cvars.Visuals_rem_smoke.GetValue())
			*begin = 1;

		m_nSmokeEffectTickBegin(pData, pStruct, pOut);
	}
	
	bool __fastcall get_int(void* convar, void* edx)
	{
		//static const auto C_CSPlayer_FireBullet = (void*)Memory::Scan("client.dll", "FF D0 8B 0D 94 3E D0 10");
	
		//if (_AddressOfReturnAddress() == C_CSPlayer_FireBullet)
		//	return true;

		return Source::m_pShowImpactsSwap->VCall<sv_impacts_int>(13)(convar);
	}

	float last_time_got_impact = 0;

	void impact_callback(const CEffectData& data)
	{
		auto org = data.origin;

		if (cheat::Cvars.Visuals_wrld_impact.GetValue() && !org.IsZero() && cheat::main::local() && data.entity == cheat::main::local()->entindex()/*fabs(cheat::main::last_shot_time_clientside - Source::m_pGlobalVars->realtime) <= Source::m_pGlobalVars->interval_per_tick*/)
			Source::m_pDebugOverlay->AddBoxOverlay(org, Vector(-2.f, -2.f, -2.f), Vector(2.f, 2.f, 2.f), Vector(0.f, 0.f, 0.f), 255, 0, 0, 127, cheat::Cvars.Visuals_wrld_impact_duration.GetValue());

		oImpact(data);
	}

	typedef int(__thiscall* oKeyEvent)(void*,int, ButtonCode_t, const char*);
	int __fastcall key_event(void* thisptr, void*, int eventcode, ButtonCode_t KeyNum, const char* pszCurrentBinding)
	{
		auto ofunc = Source::m_pClientModeSwap->VCall< oKeyEvent >(20);

		if (pszCurrentBinding) {
			if (strstr(pszCurrentBinding, "drop") && eventcode) {
				cheat::features::antiaimbot.drop = true;
				return 0;
			}
		}

		return ofunc(thisptr, eventcode, KeyNum, pszCurrentBinding);
	}

	void m_flSimulationTimeHook(const CRecvProxyData* pData, void* pStruct, void* pOut) 
	{
		//C_BasePlayer *pEntity = (C_BasePlayer *)pStruct;

		if (pData->m_Value.m_Int)
			m_flSimulationTime(pData, pStruct, pOut);
	}

	int	__stdcall IsBoxVisible(const Vector& mins, const Vector& maxs)
	{
		if (!memcmp(_ReturnAddress(), "\x85\xC0\x74\x2D\x83\x7D\x10\x00\x75\x1C", 10))
			return 1;

		return Source::m_pEngineSwap->VCall< IsBoxVisible_t >(32)(mins, maxs);
	}

	std::unordered_map <int, float> last_time_appeared;

	void m_flAbsYawHook(const CRecvProxyData* pData, void* pStruct, void* pOut)
	{
		static auto m_hPlayer = Engine::PropManager::Instance()->GetOffset("DT_CSRagdoll", "m_hPlayer");
		auto player_handle = (CBaseHandle*)((DWORD)pStruct + m_hPlayer);

		if (*player_handle != 0xFFFFFFFF)
		{
			auto hplayer = (C_BasePlayer*)Source::m_pEntList->GetClientEntityFromHandle(*player_handle);

			if (hplayer && hplayer->GetIClientEntity())
			{
				auto player = (C_BasePlayer*)hplayer->GetIClientEntity();
				if (player && player->entindex() > 0)
				{
					auto paste = &cheat::features::aaa.player_resolver_records[player->entindex() - 1];

					auto value = Math::normalize_angle(pData->m_Value.m_Float);

					if (fabs(last_time_appeared[player->entindex() - 1] - Source::m_pGlobalVars->realtime) >= 0.2f) {
						cheat::features::aaa.on_real_angle_arrive(player, paste, value);
						last_time_appeared[player->entindex() - 1] = Source::m_pGlobalVars->realtime;
					}
				}
			}
		}

		m_flAbsYaw(pData, pStruct, pOut);
	}

	void m_bClientSideAnimationHook(const CRecvProxyData* pData, void* pStruct, void* pOut) 
	{
		m_bClientSideAnimation(pData, pStruct, pOut);

		auto ent = (C_BasePlayer*)pStruct;

		if (ent != nullptr && cheat::main::local() && ent == cheat::main::local())
			*(int*)pOut = 1;
	}

	bool __fastcall IsConnected(void* ecx, void* edx)
	{
		/*
		.text:103A2110 57                          push    edi
		.text:103A2111 8B F9                       mov     edi, ecx
		.text:103A2113 8B 0D AC E5+                mov     ecx, dword_14F8E5AC
		.text:103A2119 8B 01                       mov     eax, [ecx]
		.text:103A211B 8B 40 6C                    mov     eax, [eax+6Ch]
		.text:103A211E FF D0                       call    eax             ; Indirect Call Near Procedure
		.text:103A2120 84 C0                       test    al, al          ; Logical Compare
		.text:103A2122 75 04                       jnz     short loc_103A2128 ; Jump if Not Zero (ZF=0) <-
		.text:103A2124 B0 01                       mov     al, 1
		.text:103A2126 5F                          pop     edi
		*/

		using Fn = bool(__thiscall*)(void* ecx);
		static auto ofc = Source::m_pEngineSwap->VCall<Fn>(27);

		static void* unk = (void*)(Memory::Scan("client.dll", "75 04 B0 01 5F") - 2);

		if (_ReturnAddress() == unk && (int)cheat::Cvars.Visuals_unlock_invertory.GetValue() != 0)
			return false;

		return ofc(ecx);
	}
}

struct InterfaceLinkedList {
	void*(*func)();
	const char *name;
	InterfaceLinkedList *next;
};

static DWORD a1 = 0xDEADBABE;
static DWORD a2 = 0xDEADBABE;
static DWORD a3 = 0xDEADBABE;
static DWORD a4 = 0xDEADBABE;
static DWORD a5 = 0xDEADBABE;
static DWORD a6 = 0xDEADBABE;
static DWORD a7 = 0xDEADBABE;
static DWORD a8 = 0xDEADBABE;
static DWORD a9 = 0xDEADBABE;
static DWORD a10 = 0xDEADBABE;
static DWORD a11 = 0xDEADBABE;
static DWORD a12 = 0xDEADBABE;
static DWORD a13 = 0xDEADBABE;
static DWORD a14 = 0xDEADBABE;
static DWORD a15 = 0xDEADBABE;
static DWORD a16 = 0xDEADBABE;
static DWORD a17 = 0xDEADBABE;
static DWORD a18 = 0xDEADBABE;
static DWORD a19 = 0xDEADBABE;

static DWORD s1 = 0xDEADBABE;
static DWORD s2 = 0xDEADBABE;
static DWORD s3 = 0xDEADBABE;
static DWORD s4 = 0xDEADBABE;
static DWORD s5 = 0xDEADBABE;
static DWORD s6 = 0xDEADBABE;
static DWORD s7 = 0xDEADBABE;

//#define NOAUTH

namespace Source
{
	IBaseClientDLL* m_pClient = nullptr;
	ISurface* m_pSurface = nullptr;
	InputSystem* m_pInputSystem = nullptr;
	ICvar* m_pCvar = nullptr;
	IClientEntityList* m_pEntList = nullptr;
	IGameMovement* m_pGameMovement = nullptr;
	IPrediction* m_pPrediction = nullptr;
	IMoveHelper* m_pMoveHelper = nullptr;
	IInput* m_pInput = nullptr;
	CClientState* m_pClientState = nullptr;
	IVModelInfo* m_pModelInfo = nullptr;
	CGlobalVars* m_pGlobalVars = nullptr;
	IVEngineClient* m_pEngine = nullptr;
	IPanel* m_pPanel = nullptr;
	IVRenderView* m_pRenderView = nullptr;
	IEngineTrace* m_pEngineTrace = nullptr;
	IVDebugOverlay* m_pDebugOverlay = nullptr;
	IMemAlloc* m_pMemAlloc = nullptr;
	IPhysicsSurfaceProps* m_pPhysProps = nullptr;
	IMaterialSystem* m_pMaterialSystem = nullptr;
	IGameEventManager* m_pGameEvents = nullptr;
	IVModelRender* m_pModelRender = nullptr;
	IEngineVGui* m_pEngineVGUI = nullptr;
	ILocalize* m_pLocalize = nullptr;
	IEngineSound* m_pEngineSound = nullptr;

	Memory::VmtSwap::Shared m_pClientSwap = nullptr;
	Memory::VmtSwap::Shared m_pClientStateSwap = nullptr;
	Memory::VmtSwap::Shared m_pClientModeSwap = nullptr;
	Memory::VmtSwap::Shared m_pSurfaceSwap = nullptr;
	Memory::VmtSwap::Shared m_pPredictionSwap = nullptr;
	Memory::VmtSwap::Shared m_pPanelSwap = nullptr;
	Memory::VmtSwap::Shared m_pRenderViewSwap = nullptr;
	Memory::VmtSwap::Shared m_pEngineSwap = nullptr;

	Memory::VmtSwap::Shared m_pFireBulletsSwap = nullptr;
	Memory::VmtSwap::Shared m_pEngineVGUISwap = nullptr;
	Memory::VmtSwap::Shared m_pModelRenderSwap = nullptr;
	Memory::VmtSwap::Shared m_pNetChannelSwap = nullptr;
	Memory::VmtSwap::Shared m_pShowImpactsSwap = nullptr;

	HWND Window = nullptr;
	HANDLE m_pCreateMoveThread = nullptr;
	static DWORD dwCCSPlayerRenderablevftable = NULL;

	//void DynAppSysFactory(const char *module) {
	//	void *create_interface = GetProcAddress(GetModuleHandleA(module), "CreateInterface");
	//
	//	size_t jmp_instruction = (size_t)(create_interface)+4;
	//	//rva from end of instruction
	//	size_t jmp_target = jmp_instruction + *(size_t*)(jmp_instruction + 1) + 5;
	//
	//	auto interface_list = **(InterfaceLinkedList***)(jmp_target + 6);
	//
	//	auto list_ptr = interface_list->next;
	//
	//	//std::vector<std::string> ifaces;
	//
	//	std::ofstream myfile("C:\\MorphEngine\\ifaces.log", std::ios::app);
	//	if (myfile.is_open())
	//	{
	//		while (list_ptr) {
	//
	//			myfile << module << " : " << list_ptr->name << std::endl;
	//
	//			list_ptr = list_ptr->next;
	//		}
	//
	//		myfile.close();
	//	}
	//
	//	/*while (list_ptr) {
	//		
	//		ifaces.emplace_back(list_ptr->name);
	//
	//		list_ptr = list_ptr->next;
	//	}*/
	//
	//	printf("%s iface list ptr: 0x%X\n", module, interface_list);
	//}

	bool Create()
	{
		auto& pPropManager = Engine::PropManager::Instance();

		cheat::main::enginedll = enginedll;
		cheat::main::clientdll = clientdll;

#ifndef NOAUTH

		//auto split = [](const std::wstring& s, char seperator)
		//{
		//	std::vector<std::wstring> output;

		//	std::wstring::size_type prev_pos = 0, pos = 0;

		//	while ((pos = s.find(seperator, pos)) != std::wstring::npos)
		//	{
		//		std::wstring substring(s.substr(prev_pos, pos - prev_pos));

		//		output.push_back(substring);

		//		prev_pos = ++pos;
		//	}

		//	output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word

		//	return output;
		//};
		//std::string proc_g;
		//std::ifstream proc_read("C:\\vr.set");
		//if (proc_read.good()) {
		//	getline(proc_read, proc_g);
		//}
		//else {
		//	return;
		//}

		//WCHAR buffer[512];

		//if (!GlobalGetAtomNameW(std::atoi(proc_g.c_str()), buffer, 512))
		//	return false;
		//
		//std::vector<std::wstring> user_pass = split(std::wstring(buffer), '|');
		//std::string s(user_pass[0].begin(), user_pass[0].end());
		//user = s;
		//if (!CheckIngameLogin(user_pass[0].c_str(), user_pass[1].c_str(), L"3"))
		//	return false;
		

		/*
		{
		"a1" : {"VClient018":"client.dll"},
		"a2" : {"VGUI_Surface031":"vguimatsurface.dll"},
		"a3" : {"InputSystemVersion001":"inputsystem.dll"},
		"a4" : {"VEngineCvar007":"vstdlib.dll"},
		"a5" : {"VClientEntityList003":"client.dll"},
		"a6" : {"GameMovement001":"client.dll"},
		"a7" : {"VClientPrediction001":"client.dll"},
		"a8" : {"VMaterialSystem080":"materialsystem.dll"},
		"a9" : {"EngineTraceClient004":"engine.dll"},
		"a10" : {"VModelInfoClient004":"engine.dll"},
		"a11" : {"VEngineVGui001":"engine.dll"},
		"a12" : {"VPhysicsSurfaceProps001":"vphysics.dll"},
		"a13" : {"VEngineClient014":"engine.dll"},
		"a14" : {"VGUI_Panel009":"vgui2.dll"},
		"a15" : {"VEngineRenderView014":"engine.dll"},
		"a16" : {"VDebugOverlay004":"engine.dll"},
		"a17" : {"GAMEEVENTSMANAGER002":"engine.dll"},
		"a18" : {"VEngineModel016":"engine.dll"},
		"a19" : {"Localize_001":"localize.dll"}
		}
		*/

		m_pClient = (IBaseClientDLL*)a1;//CreateInterface("client.dll", "VClient");

		if (!m_pClient)
			return false;

		if (!pPropManager->Create(m_pClient))
			return false;

		if (!Engine::Displacement::Create())
			return false;

		/*
		{
		"s20" : {"0F 11 05 ? ? ? ? 83 C8 01":"client.dll"},
		"s21" : {"A1 ? ? ? ? 8B 80 ? ? ? ? C3":"engine.dll"},
		"s22" : {"8B 0D ?? ?? ?? ?? 8B 45 ?? 51 8B D4 89 02 8B 01":"client.dll"},
		"s23" : {"B9 ?? ?? ?? ?? F3 0F 11 04 24 FF 50 10":"client.dll"},
		"s24" : {"8B 0D ?? ?? ?? ?? BA ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 C4 04":"client.dll"},
		"s25" : {"55 8B EC 51 56 8B F1 80 BE ? ? 00 00 00 74 36":"client.dll"},
		"s26" : {"89 ?? ?? ?? ?? ?? F3 0F 10 48 20":"client.dll"}
		}
		*/

		Engine::Displacement::Data::m_pGlowManager = *reinterpret_cast<CGlowObjectManager**>(s1 + 3);	//*reinterpret_cast<CGlowObjectManager**>(Memory::Scan("client.dll", "0F 11 05 ? ? ? ? 83 C8 01") + 3);
		Engine::Displacement::Data::m_uClientState = **(std::uintptr_t**)(s2 + 1);						// (Memory::Scan("engine.dll", "A1 ? ? ? ? 8B 80 ? ? ? ? C3") + 1);//8B F1 E8 ? ? ? ? 8B 8E ? ? ? ? 3B 8E ? ? ? ?
		Engine::Displacement::Data::m_uMoveHelper = **(std::uintptr_t**)(s3 + 2);						// (Memory::Scan("client.dll", "8B 0D ?? ?? ?? ?? 8B 45 ?? 51 8B D4 89 02 8B 01") + 2);
		Engine::Displacement::Data::m_uInput = *(std::uintptr_t*)(s4 + 1);								// (Memory::Scan("client.dll", "B9 ?? ?? ?? ?? F3 0F 11 04 24 FF 50 10") + 1);
		Engine::Displacement::Data::m_uPredictionRandomSeed = *(std::uintptr_t*)(s5 + 2);				// (Memory::Scan("client.dll", "8B 0D ?? ?? ?? ?? BA ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 C4 04") + 2); // 8B 0D ? ? ? ? BA ? ? ?? E8 ? ? ? ? 83 C4 04
		Engine::Displacement::Data::m_uUpdateClientSideAnimation = (DWORD)s6;							// (Memory::Scan("client.dll", "55 8B EC 51 56 8B F1 80 BE ? ? 00 00 00 74 36"));
		Engine::Displacement::Data::m_uPredictionPlayer = *(std::uintptr_t*)(s7 + 2);					// (Memory::Scan("client.dll", "89 ?? ?? ?? ?? ?? F3 0F 10 48 20") + 2);

		m_pSurface = (ISurface*)a2;//CreateInterface("vguimatsurface.dll", "VGUI_Surface");//"vguimatsurface.dll", "VGUI_Surface031", true);

		if (!m_pSurface)
			return false;

		m_pInputSystem = (InputSystem*)a3;//CreateInterface("inputsystem.dll", "InputSystemVersion001", true);

		if (!m_pInputSystem)
			return false;

		m_pCvar = (ICvar*)a4;//CreateInterface("vstdlib.dll", "VEngineCvar");

		if (!m_pCvar)
			return false;

		m_pEntList = (IClientEntityList*)a5;//CreateInterface("client.dll", "VClientEntityList");

		if (!m_pEntList)
			return false;

		m_pGameMovement = (IGameMovement*)a6;// CreateInterface("client.dll", "GameMovement");

		if (!m_pGameMovement)
			return false;

		m_pPrediction = (IPrediction*)a7;// CreateInterface("client.dll", "VClientPrediction");

		if (!m_pPrediction)
			return false;

		m_pMaterialSystem = (IMaterialSystem*)a8;// CreateInterface("materialsystem.dll", "VMaterialSystem080", true);

		if (!m_pMaterialSystem)
			return false;

		m_pMoveHelper = (IMoveHelper*)(Engine::Displacement::Data::m_uMoveHelper);

		if (!m_pMoveHelper)
			return false;

		m_pInput = (IInput*)(Engine::Displacement::Data::m_uInput);

		if (!m_pInput)
			return false;
		
		m_pEngineTrace = (IEngineTrace*)a9;//CreateInterface("engine.dll", "EngineTraceClient004", true);

		if (!m_pEngineTrace)
			return false;

		m_pClientState = (CClientState*)(Engine::Displacement::Data::m_uClientState);

		if (!m_pClientState)
			return false;

		m_pGlobalVars = **(CGlobalVars***)((*(DWORD**)m_pClient)[0] + 0x1B);

		if (!m_pGlobalVars)
			return false;

		m_pModelInfo = (IVModelInfo*)a10;// CreateInterface("engine.dll", "VModelInfoClient004", true);

		if (!m_pModelInfo)
			return false;
		
		m_pEngineVGUI = (IEngineVGui*)a11;// CreateInterface("engine.dll", "VEngineVGui001", true);

		if (!m_pEngineVGUI)
			return false;
		
		m_pPhysProps = (IPhysicsSurfaceProps*)a12;// CreateInterface("vphysics.dll", "VPhysicsSurfaceProps001", true);

		if (!m_pPhysProps)
			return false;

		m_pEngine = (IVEngineClient*)a13;// CreateInterface("engine.dll", "VEngineClient");

		if (!m_pEngine)
			return false;

		m_pPanel = (IPanel*)a14;// CreateInterface("vgui2.dll", "VGUI_Panel");

		if (!m_pPanel)
			return false;
		
		m_pRenderView = (IVRenderView*)a15;// CreateInterface("engine.dll", "VEngineRenderView014", true);

		if (!m_pRenderView)
			return false;

		m_pDebugOverlay = (IVDebugOverlay*)a16;// CreateInterface("engine.dll", "VDebugOverlay004", true);

		if (!m_pDebugOverlay)
			return false;

		m_pMemAlloc = *(IMemAlloc**)(GetProcAddress(GetModuleHandleA("tier0.dll"), "g_pMemAlloc"));

		if (!m_pMemAlloc)
			return false;
		
		m_pGameEvents = (IGameEventManager*)a17;// (CreateInterface("engine.dll", "GAMEEVENTSMANAGER002", true));

		if (!m_pGameEvents)
			return false;

		m_pModelRender = (IVModelRender*)a18;// (CreateInterface("engine.dll", "VEngineModel016", true));

		if (!m_pModelRender)
			return false;
		
		m_pLocalize = (ILocalize*)a19;// (CreateInterface("localize.dll", "Localize_001", true));

		if (!m_pLocalize)
			return false;

		a1 = 0;
		a2 = 0;
		a3 = 0;
		a4 = 0;
		a5 = 0;
		a6 = 0;
		a7 = 0;
		a8 = 0;
		a9 = 0;
		a10 = 0;
		a11 = 0;
		a12 = 0;
		a13 = 0;
		a14 = 0;
		a15 = 0;
		a16 = 0;
		a17 = 0;
		a18 = 0;
		a19 = 0;
		s1 = 0;
		s2 = 0;
		s3 = 0;
		s4 = 0;
		s5 = 0;
		s6 = 0;
		s7 = 0;

#else // DEBUG =====================================================================================================

		m_pClient = (IBaseClientDLL*)CreateInterface("client.dll", "VClient");

		if (!m_pClient)
		{
#ifdef DEBUG
			Win32::Error("IBaseClientDLL is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
	}

		if (!pPropManager->Create(m_pClient))
		{
#ifdef DEBUG
			Win32::Error("Engine::PropManager::Create failed (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		if (!Engine::Displacement::Create())
		{
#ifdef DEBUG
			Win32::Error("Engine::Displacement::Create failed (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		Engine::Displacement::Data::m_pGlowManager = *reinterpret_cast<CGlowObjectManager**>(Memory::Scan("client.dll", "0F 11 05 ? ? ? ? 83 C8 01") + 3);
		Engine::Displacement::Data::m_uTraceLineIgnoreTwoEntities = *(DWORD**)(Memory::Scan("client.dll", "53 8B DC 83 EC ? 83 E4 ? 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC 8C"));
		//Data::m_uClipTracePlayers = (DWORD)(Memory::Scan("client.dll", "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10"));
		Engine::Displacement::Data::m_uClientState = **(std::uintptr_t**)(Memory::Scan("engine.dll", "A1 ? ? ? ? 8B 80 ? ? ? ? C3") + 1);//8B F1 E8 ? ? ? ? 8B 8E ? ? ? ? 3B 8E ? ? ? ?
		Engine::Displacement::Data::m_uMoveHelper = **(std::uintptr_t**)(Memory::Scan("client.dll", "8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01") + 2);
		Engine::Displacement::Data::m_uInput = *(std::uintptr_t*)(Memory::Scan("client.dll", "B9 ?? ?? ?? ?? F3 0F 11 04 24 FF 50 10") + 1);
		//Data::m_uGlobalVars = **( std::uintptr_t** )( Memory::Scan( "client.dll", "A3 ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 6A" ) + 1 );
		Engine::Displacement::Data::m_uPredictionRandomSeed = *(std::uintptr_t*)(Memory::Scan("client.dll", "8B 0D ?? ?? ?? ?? BA ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 C4 04") + 2); // 8B 0D ? ? ? ? BA ? ? ?? E8 ? ? ? ? 83 C4 04
		Engine::Displacement::Data::m_uUpdateClientSideAnimation = (DWORD)(Memory::Scan("client.dll", "55 8B EC 51 56 8B F1 80 BE ? ? 00 00 00 74 36"));
		Engine::Displacement::Data::m_uPredictionPlayer = *(std::uintptr_t*)(Memory::Scan("client.dll", "89 ?? ?? ?? ?? ?? F3 0F 10 48 20") + 2);

		m_pSurface = (ISurface*)CreateInterface("vguimatsurface.dll", "VGUI_Surface");//"vguimatsurface.dll", "VGUI_Surface031", true);

		if (!m_pSurface)
		{
#ifdef DEBUG
			Win32::Error("ISurface is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pInputSystem = (InputSystem*)CreateInterface("inputsystem.dll", "InputSystemVersion001", true);

		if (!m_pInputSystem)
		{
#ifdef DEBUG
			Win32::Error("IInputSystem is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pCvar = (ICvar*)CreateInterface("vstdlib.dll", "VEngineCvar");

		if (!m_pCvar)
		{
#ifdef DEBUG
			Win32::Error("IInputSystem is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pEntList = (IClientEntityList*)CreateInterface("client.dll", "VClientEntityList");

		if (!m_pEntList)
		{
#ifdef DEBUG
			Win32::Error("IClientEntityList is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pGameMovement = (IGameMovement*)CreateInterface("client.dll", "GameMovement");

		if (!m_pGameMovement)
		{
#ifdef DEBUG
			Win32::Error("IGameMovement is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pPrediction = (IPrediction*)CreateInterface("client.dll", "VClientPrediction");

		if (!m_pPrediction)
		{
#ifdef DEBUG
			Win32::Error("IPrediction is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pMaterialSystem = (IMaterialSystem*)CreateInterface("materialsystem.dll", "VMaterialSystem080", true);

		if (!m_pMaterialSystem)
		{
#ifdef DEBUG
			Win32::Error("IMaterialSystem is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pMoveHelper = (IMoveHelper*)(Engine::Displacement::Data::m_uMoveHelper);

		if (!m_pMoveHelper)
		{
#ifdef DEBUG
			Win32::Error("IMoveHelper is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pInput = (IInput*)(Engine::Displacement::Data::m_uInput);

		if (!m_pInput)
		{
#ifdef DEBUG
			Win32::Error("IInput is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pEngineTrace = (IEngineTrace*)CreateInterface("engine.dll", "EngineTraceClient004", true);

		if (!m_pEngineTrace)
		{
#ifdef DEBUG
			Win32::Error("IInput is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pClientState = **(CClientState***)(Memory::Scan("engine.dll", "A1 ? ? ? ? 8B 80 ? ? ? ? C3") + 1);

		if (!m_pClientState)
		{
#ifdef DEBUG
			Win32::Error("CClientState is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pGlobalVars = **(CGlobalVars***)((*(DWORD**)m_pClient)[0] + 0x1B);

		if (!m_pGlobalVars)
		{
#ifdef DEBUG
			Win32::Error("CGlobalVars is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pModelInfo = (IVModelInfo*)CreateInterface("engine.dll", "VModelInfoClient004", true);

		if (!m_pModelInfo)
		{
#ifdef DEBUG
			Win32::Error("IVModelInfo is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pEngineVGUI = (IEngineVGui*)CreateInterface("engine.dll", "VEngineVGui001", true);

		if (!m_pEngineVGUI)
		{
#ifdef DEBUG
			Win32::Error("IEngineVGui is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pPhysProps = (IPhysicsSurfaceProps*)CreateInterface("vphysics.dll", "VPhysicsSurfaceProps001", true);

		if (!m_pPhysProps)
		{
#ifdef DEBUG
			Win32::Error("IPhysicsSurfaceProps is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pEngine = (IVEngineClient*)CreateInterface("engine.dll", "VEngineClient");

		if (!m_pEngine)
		{
#ifdef DEBUG
			Win32::Error("IVEngineClient is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pPanel = (IPanel*)CreateInterface("vgui2.dll", "VGUI_Panel");

		if (!m_pPanel)
		{
#ifdef DEBUG
			Win32::Error("IPanel is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pRenderView = (IVRenderView*)CreateInterface("engine.dll", "VEngineRenderView014", true);

		if (!m_pRenderView)
		{
#ifdef DEBUG
			Win32::Error("IVRenderView is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pDebugOverlay = (IVDebugOverlay*)CreateInterface("engine.dll", "VDebugOverlay004", true);

		if (!m_pDebugOverlay)
		{
#ifdef DEBUG
			Win32::Error("IVDebugOverlay is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pMemAlloc = *(IMemAlloc**)(GetProcAddress(GetModuleHandleA("tier0.dll"), "g_pMemAlloc"));

		if (!m_pMemAlloc)
		{
#ifdef DEBUG
			Win32::Error("IMemAlloc is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pGameEvents = (IGameEventManager*)(CreateInterface("engine.dll", "GAMEEVENTSMANAGER002", true));

		if (!m_pGameEvents)
		{
#ifdef DEBUG
			Win32::Error("IGameEventManager is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pModelRender = (IVModelRender*)(CreateInterface("engine.dll", "VEngineModel016", true));

		if (!m_pModelRender)
		{
#ifdef DEBUG
			Win32::Error("IVModelRender is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		m_pLocalize = (ILocalize*)(CreateInterface("localize.dll", "Localize_001", true));

		if (!m_pLocalize)
		{
#ifdef DEBUG
			Win32::Error("ILocalize is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}


		std::ofstream output;
		output.open("C:\\zeus\\CSGOclassids.txt", std::ofstream::out | std::ofstream::app);
		output << "enum class_ids" << std::endl << "{" << std::endl;

		static auto entry = Source::m_pClient->GetAllClasses();

		while (entry)
		{
			output << "\t" << entry->m_pNetworkName << " = " << entry->m_ClassID << "," << std::endl;
			entry = entry->m_pNext;
		}

		output << "};";
		output.close();

#endif // DEBUG

		m_pEngineSound = (IEngineSound*)(CreateInterface(cheat::main::enginedll, s_enginesnd));

		//Engine::PropManager::Instance()->DumpProps();

		/*DynAppSysFactory("engine.dll");
		DynAppSysFactory("vgui2.dll");
		DynAppSysFactory("vphysics.dll");
		DynAppSysFactory("materialsystem.dll");
		DynAppSysFactory("client.dll");
		DynAppSysFactory("vstdlib.dll");
		DynAppSysFactory("inputsystem.dll");
		DynAppSysFactory("matchmaking.dll");
		DynAppSysFactory("serverbrowser.dll");
		DynAppSysFactory("server.dll");

		DynAppSysFactory("datacache.dll");
		DynAppSysFactory("vguimatsurface.dll");

		DynAppSysFactory("filesystem_stdio.dll");*/

		cheat::features::clientside_animlist = *(CUtlVector<clientanimating_t>**)(Memory::Scan(cheat::main::clientdll, s_sig_clientsideanims) + 1);
		cheat::features::effects_head = **reinterpret_cast<CClientEffectRegistration***>(Memory::Scan(cheat::main::clientdll, s_sig_effectshead) + 2);

		m_pClientSwap = std::make_shared<Memory::VmtSwap>(m_pClient);
		m_pSurfaceSwap = std::make_shared<Memory::VmtSwap>(m_pSurface);
		m_pPredictionSwap = std::make_shared<Memory::VmtSwap>(m_pPrediction);
		m_pPanelSwap = std::make_shared<Memory::VmtSwap>(m_pPanel);
		m_pRenderViewSwap = std::make_shared<Memory::VmtSwap>(m_pRenderView);
		m_pEngineSwap = std::make_shared<Memory::VmtSwap>(m_pEngine);
		m_pEngineVGUISwap = std::make_shared<Memory::VmtSwap>(m_pEngineVGUI);
		m_pModelRenderSwap = std::make_shared<Memory::VmtSwap>(m_pModelRender);
		//m_pClientStateSwap = std::make_shared<Memory::VmtSwap>((uint32_t*)(uint32_t(m_pClientState) + 0x8));

		m_pEngineSwap->Hook(&Hooked::GetScreenAspectRatio, 101);
		m_pEngineSwap->Hook(&Hooked::FireEvents, 59);
		m_pEngineSwap->Hook(&Hooked::IsBoxVisible, 32);
		m_pEngineSwap->Hook(&Hooked::IsConnected, 27);

		m_pEngineVGUISwap->Hook(&Hooked::EngineVGUI_Paint, 14);

		auto m_pClientMode = **(void***)((*(DWORD**)Source::m_pClient)[10] + 5);

		if (m_pClientMode)
		{
			m_pClientModeSwap = std::make_shared<Memory::VmtSwap>(m_pClientMode);
			m_pClientModeSwap->Hook(&Hooked::CreateMove, Index::IBaseClientDLL::CreateMove);
			m_pClientModeSwap->Hook(&Hooked::GetViewModelFOV, Index::IBaseClientDLL::GetViewModelFOV);
			m_pClientModeSwap->Hook(&Hooked::OverrideView, Index::IBaseClientDLL::OverrideView);
			m_pClientModeSwap->Hook(&Hooked::DoPostScreenEffects, 44);
			m_pClientModeSwap->Hook(&Hooked::key_event, 20);
		}

		const std::string loli = s_hook_impact;

		for (auto head = cheat::features::effects_head; head; head = head->next)
		{
			if (strstr(head->effectName, loli.c_str()) && strlen(head->effectName) <= 8) {
				oImpact = head->function;
				head->function = &Hooked::impact_callback;
			}
		}

		cheat::main::localstate.last_anim_upd_tick = 0;
		cheat::main::command_numbers.clear();

		m_pClientSwap->Hook(&Hooked::FrameStageNotify, Index::IBaseClientDLL::FrameStageNotify);

		OriginalUpdateClientSideAnimations = (DWORD)DetourFunction((byte*)Engine::Displacement::Data::m_uUpdateClientSideAnimation, (byte*)Hooked::UpdateClientSideAnimation);

		static auto StandardBlendingRules = (DWORD)(Memory::Scan(cheat::main::clientdll, "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6"));
		static auto SetupBones = (DWORD)(Memory::Scan(cheat::main::clientdll, "55 8B EC 83 E4 F0 B8 D8"));
		static auto PhysicsSimulate = (DWORD)(Memory::Scan(cheat::main::clientdll, "56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 21"));
		//static auto DoExtraBonesProcessing = (DWORD)(Memory::Scan(cheat::main::clientdll, "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 8B F1 57 89 74 24 1C"));
		static auto ModifyEyePosition = (DWORD)(Memory::Scan(cheat::main::clientdll, "55 8B EC 83 E4 F8 83 EC 58 56 57 8B F9 83 7F 60"));


		OriginalStandardBlendingRules = (DWORD)DetourFunction((byte*)StandardBlendingRules, (byte*)Hooked::StandardBlendingRules);
		//OriginalDoExtraBonesProcessing = (DWORD)DetourFunction((byte*)DoExtraBonesProcessing, (byte*)Hooked::DoExtraBonesProcessing);
		//OriginalModifyEyePosition = (DWORD)DetourFunction((byte*)ModifyEyePosition, (byte*)Hooked::ModifyEyePosition);
		OriginalSetupBones = (DWORD)DetourFunction((byte*)SetupBones, (byte*)Hooked::SetupBones);
		OriginalPhysicsSimulate = (DWORD)DetourFunction((byte*)PhysicsSimulate, (byte*)Hooked::PhysicsSimulate);

		//m_nSmokeEffectTickBegin = pPropManager->Hook(Hooked::m_nSmokeEffectTickBeginHook, "DT_SmokeGrenadeProjectile", "m_nSmokeEffectTickBegin");
		m_flSimulationTime = pPropManager->Hook(Hooked::m_flSimulationTimeHook, "DT_BaseEntity", "m_flSimulationTime");
		//m_flAbsYaw = pPropManager->Hook(Hooked::m_flAbsYawHook, "DT_CSRagdoll", "m_flAbsYaw");
		m_bClientSideAnimation = pPropManager->Hook(Hooked::m_bClientSideAnimationHook, "DT_BaseAnimating", "m_bClientSideAnimation");

		m_pPredictionSwap->Hook(&Hooked::InPrediction, Index::IPrediction::InPrediction);
		m_pSurfaceSwap->Hook(&Hooked::LockCursor, Index::ISurface::LockCursor);
		m_pPredictionSwap->Hook(&Hooked::RunCommand, Index::IPrediction::RunCommand);
		m_pPanelSwap->Hook(&Hooked::PaintTraverse, Index::IPanel::PaintTraverse);
		m_pRenderViewSwap->Hook(&Hooked::SceneEnd, 9);
		//m_pModelRenderSwap->Hook(&Hooked::DrawModelExecute, 21);

		const std::string lol = s_wndproc_valve;

		Window = FindWindowA(lol.c_str(), NULL);

		Hooked::oldWindowProc = (WNDPROC)SetWindowLongPtr(Window, GWL_WNDPROC, (LONG_PTR)Hooked::WndProc);

		game_events::init();

		//cheat::features::music.init();
		
		_events.clear();

		//_events.push_back(_event(Source::m_pGlobalVars->curtime + 2.f, "BETA Injected"));

		cheat::menu.Setup();

		cheat::Cvars.GradientFrom.SetColor(Color(66, 140, 244));
		cheat::Cvars.GradientTo.SetColor(Color(75, 180, 242));
		cheat::Cvars.MenuTheme.SetColor(Color(66, 140, 244));

		cheat::Cvars.EnemyBoxCol.SetColor(Color(0, 129, 255));
		cheat::Cvars.Visuals_glow_color.SetColor(Color(0, 129, 255));
		cheat::Cvars.Visuals_lglow_color.SetColor(Color(0, 129, 255));
		cheat::Cvars.Visuals_SnaplinesColor.SetColor(Color(128, 0, 128));
		cheat::Cvars.Visuals_skeletonColor.SetColor(Color(105, 105, 105));
		cheat::Cvars.Visuals_chams_color.SetColor(Color(0, 129, 255));
		cheat::Cvars.Visuals_chams_hidden_color.SetColor(Color(0, 255, 129));
		cheat::Cvars.Visuals_chams_history_color.SetColor(Color(0, 255, 129));
		cheat::Cvars.Visuals_wrld_entities_color.SetColor(Color(0, 255, 129));
		cheat::Cvars.Misc_AntiUT.SetValue(1.f);

		cheat::features::antiaimbot.enable_delay = 0.f;

		cheat::main::updating_anims = false;

		if (parser::parse()) {
			//cheat::menu.HandleInput();

			if (parser::weapons.list.size() >= 42) {


				for (auto i = 0; i < parser::weapons.list.size(); i++)
				{
					auto weapon = parser::weapons.list.data()[i];
					cheat::Cvars.Skins_Weapons.AddItem(weapon.translated_name.data(), weapon.id, Color::White());
				}

				for (auto i = 0; i < parser::weapons.list[0].skins.list.size(); i++)
				{
					auto skins = parser::weapons.list[0].skins.list.data()[i];
					auto color = parser::rarities.get_by_id(skins.rarity);

					cheat::Cvars.Skins_Paintkits.AddItem(skins.translated_name.data(), skins.id, color.Color);
				}
			}

			if (!parser::knifes.list.empty()) {
				static auto selected_knife = 0;
				static std::vector<knife_t> knife_list;
				if (knife_list.empty())
				{
					knife_list = parser::knifes.list;

					knife_t default_knife;
					default_knife.id = 0;
					default_knife.translated_name = "Default";
					knife_list.insert(knife_list.begin(), default_knife);
					cheat::Cvars.Skins_Knife.AddKnifes(knife_list);
				}
			}
		}

		return true;
	}

	void Destroy()
	{
		if (OriginalUpdateClientSideAnimations != 0)
			DetourRemove((byte*)OriginalUpdateClientSideAnimations, (byte*)Hooked::UpdateClientSideAnimation);

		DetourRemove((byte*)OriginalPhysicsSimulate, (byte*)Hooked::PhysicsSimulate);
		DetourRemove((byte*)OriginalSetupBones, (byte*)Hooked::SetupBones);
		//DetourRemove((byte*)OriginalModifyEyePosition, (byte*)Hooked::ModifyEyePosition);
		DetourRemove((byte*)OriginalStandardBlendingRules, (byte*)Hooked::StandardBlendingRules);
		//DetourRemove((byte*)OriginalDoExtraBonesProcessing, (byte*)Hooked::DoExtraBonesProcessing);
		//DetourRemove((byte*)OriginalLol, (byte*)Hooked::DangerZoneSmth);
		//CloseHandle(m_pCreateMoveThread);

		const std::string lol = s_hook_impact;

		for (auto head = cheat::features::effects_head; head; head = head->next)
		{
			if (strstr(head->effectName, lol.c_str()) && strlen(head->effectName) <= 8) {
				head->function = oImpact;
			}
		}

		//if (cheat::main::local()state) {
		//	Source::m_pMemAlloc->Free(cheat::main::local()state);
		//	cheat::main::local()state = nullptr;
		//}

		//Engine::PropManager::Instance()->Hook(m_nSmokeEffectTickBegin, "DT_SmokeGrenadeProjectile", "m_nSmokeEffectTickBegin");
		Engine::PropManager::Instance()->Hook(m_flSimulationTime, "DT_BaseEntity", "m_flSimulationTime");
		//Engine::PropManager::Instance()->Hook(m_flAbsYaw, "DT_CSRagdoll", "m_flAbsYaw");
		Engine::PropManager::Instance()->Hook(m_bClientSideAnimation, "DT_BaseAnimating", "m_bClientSideAnimation");

		m_pClientSwap.reset();
		m_pPredictionSwap.reset();
		m_pPanelSwap.reset();
		m_pClientModeSwap.reset();
		m_pSurfaceSwap.reset();
		m_pRenderViewSwap.reset();
		m_pEngineSwap.reset();
		m_pClientStateSwap.reset();
		m_pEngineVGUISwap.reset();
		m_pModelRenderSwap.reset();
		m_pNetChannelSwap.reset();
		//m_pShowImpactsSwap.reset();

		//cheat::convars::spoofed::viewmodel_offset_x->Restore();
		//cheat::convars::spoofed::viewmodel_offset_y->Restore();
		//cheat::convars::spoofed::viewmodel_offset_z->Restore();
		//cheat::convars::spoofed::sv_cheats->Restore();

		//DetourRemove((byte*)OriginalLol, (byte*)Hooked::CL_Move);
		//Memory::VFTableHook::HookManual<TempEntities_t>(*(uintptr_t**)((int)Source::m_pClientState + 0x8), 36, oTempEntities);
		//Memory::VFTableHook::HookManual<SetupBones_t>((uintptr_t*)dwCCSPlayerRenderablevftable, 13, o_SetupBones);
		//DetourRemove((byte*)OriginalLol, (byte*)Hooked::CL_SendMove);
		SetWindowLongPtr(Window, GWL_WNDPROC, (LONG_PTR)Hooked::oldWindowProc);
	}

	void* CreateInterface(const std::string& image_name, const std::string& name, bool force /*= false */)
	{
		auto image = GetModuleHandleA(image_name.c_str());

		if (!image)
			return nullptr;

		auto fn = (CreateInterfaceFn)(GetProcAddress(image, "CreateInterface"));

		if (!fn)
			return nullptr;

		if (force)
			return fn(name.c_str(), nullptr);

		char format[1024] = { };

		for (auto i = 0u; i < 1000u; i++)
		{
			sprintf_s(format, "%s%03u", name.c_str(), i);

			auto factory = fn(format, nullptr);

			if (factory)
				return factory;
		}

		return nullptr;
	}
}

namespace cheat
{
	namespace game
	{
		bool pressed_keys[256] = {};
		int last_frame = -1;
		inline bool get_key_press(int key, int zticks)
		{
			static int ticks[256];
			bool returnvalue = false;

			if (pressed_keys[key])
			{
				ticks[key]++;

				if (ticks[key] <= zticks)
				{
					returnvalue = true;
				}
			}
			else
				ticks[key] = 0;

			return returnvalue;
		}
		CUserCmd* last_cmd = nullptr;
		Vector2D screen_size = Vector2D(0,0);
	}
	namespace features {
		c_drawhack menu;
		c_visuals visuals;
		c_aimbot aimbot;
		c_lagcomp lagcomp;
		c_antiaimbot antiaimbot;
		c_resolver aaa;
		c_autowall autowall;
		c_music_player music;
		CUtlVector<clientanimating_t> *clientside_animlist;
		CClientEffectRegistration *effects_head;
		c_dormant_esp dormant;
	}
	namespace main {
		FORCEINLINE C_BasePlayer* local()
		{
			auto client = Source::m_pEntList->GetClientEntity(Source::m_pEngine->GetLocalPlayer());

			if (!client || !client->IsPlayer())
				return nullptr;

			return client;
		}
		CCSGOPlayerAnimState localstate;
		bool reset_local_animstate = false;
		float lerp_time;
		bool fakewalking;
		bool updating_skins;
		int prev_fakelag_value;
		int skip_shot=0;
		int side = 2;
		int shots_fired[128];
		int shots_total[128];
		int history_hit[128];
		bool thirdperson;
		bool updating_anims;
		bool fast_autostop;
		bool called_chams_render;
		std::vector<Vector> points[64][19];
		std::vector<_shotinfo> fired_shot;
		std::deque<int> command_numbers;
		std::deque<int> m_corrections_data;
		matrix3x4_t localplaya_matrix[128];
		float hit_time;
		bool jittering;
		std::string enginedll;
		std::string clientdll;
		bool setup_bones;
		float fov;
		matrix3x4_t zero_matrix[128];
		int last_frame_stage = 0;
		float latency = 0;
		bool fix_modify_eye_pos = false;
		bool can_store_netvars = false;
		bool run_cmd_got_called = false;
		int last_netvars_update_tick = -1;
		float last_shot_time_clientside = 0.f;
		float last_velocity_modifier = 0.f;
		int last_velocity_modifier_tick = -1;
		int update_cycle = CYCLE_NONE;
		float real_angle = 0.f;
		int last_penetrated_count = 4;
		float lby_angle = 0.f;
		bool updating_local = false;
	}
	namespace convars {
		int sv_usercmd_custom_random_seed;
		float weapon_recoil_scale;
		int weapon_accuracy_nospread;
		int sv_clip_penetration_traces_to_players;
		float ff_damage_reduction_bullets;
		float ff_damage_bullet_penetration;
		int sv_penetration_type;

		//namespace spoofed {
			/*SpoofedConvar *viewmodel_offset_x = nullptr;
			SpoofedConvar *viewmodel_offset_y = nullptr;
			SpoofedConvar *viewmodel_offset_z = nullptr;
			SpoofedConvar *sv_cheats = nullptr;*/
		//}
	}
	c_variables settings;
	CVars Cvars;
	CMenu menu;
}