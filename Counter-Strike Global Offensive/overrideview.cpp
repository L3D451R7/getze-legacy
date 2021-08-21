#include "hooked.hpp"
#include "displacement.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "prediction.hpp"
#include "movement.hpp"
#include "aimbot.hpp"
#include "anti_aimbot.hpp"
#include "menu.h"
#include "game_movement.h"
#include "lag_compensation.hpp"
#include <intrin.h>
#include "rmenu.hpp"

namespace Hooked
{

	void __fastcall OverrideView(void* ecx, void* edx, CViewSetup* vsView)
	{
		//static auto prev_duckamt = 0.f;
		using Fn = void(__thiscall*)(void*, CViewSetup*);

		//if (cheat::game::pressed_keys[88] && cheat::main::local() && !cheat::main::local()->IsDead()) {
		//	prev_duckamt = cheat::main::local()->m_flDuckAmount();
		//	cheat::main::local()->m_flDuckAmount() = 0.f;
		//}
		//if (cheat::main::local()) {
		//	if (auto viewmodel = cheat::main::local()->m_hViewModel(); viewmodel != nullptr)
		//	{
		//		auto pViewModel = Source::m_pEntList->GetClientEntityFromHandle((CBaseHandle)viewmodel);

		//		if (pViewModel)
		//		{
		//			auto penis = pViewModel->get_abs_origin();
		//			penis.x += cheat::settings.visuals_misc_viewmodel[0];
		//			penis.y += cheat::settings.visuals_misc_viewmodel[1];
		//			penis.z += cheat::settings.visuals_misc_viewmodel[2];

		//			//pViewModel->set_abs_origin(vsView->origin);

		//			Vector camForward, camRight, camUp;
		//			Math::AngleVectors(Vector(0,0,0), &camForward, &camRight, &camUp);

		//			Math::VectorMA(penis, 0, camForward, penis);

		//			pViewModel->set_abs_origin(penis);
		//		}
		//	}
		//}

		static ConVar* zoom_sensitivity_ratio_mouse = NULL;
		static float defaultSens = 1.0f;

		if (!zoom_sensitivity_ratio_mouse)
		{
			zoom_sensitivity_ratio_mouse = Source::m_pCvar->FindVar("zoom_sensitivity_ratio_mouse");

			defaultSens = zoom_sensitivity_ratio_mouse->GetFloat();
		}

		if (cheat::main::thirdperson && cheat::Cvars.Visuals_lp_forcetp.GetValue() && cheat::main::local() && !cheat::main::local()->IsDead())
		{
			if (!Source::m_pInput->m_fCameraInThirdPerson)
				Source::m_pInput->m_fCameraInThirdPerson = true;

			if (Source::m_pInput->m_fCameraInThirdPerson)
			{
				trace_t trace;
				auto angles = Engine::Movement::Instance()->m_qRealAngles;

				Vector camForward, camRight, camUp;
				Vector camAngles;

				camAngles[0] = angles[0];
				camAngles[1] = angles[1];
				camAngles[2] = cheat::Cvars.Visuals_lp_tpdist.GetValue();

				Math::AngleVectors(camAngles, &camForward, &camRight, &camUp);

				Vector vecCamOffset = cheat::main::local()->GetEyePosition() + (camForward * -(cheat::Cvars.Visuals_lp_tpdist.GetValue())) + (camRight * 1.f) + (camUp * 1.f);

				Ray_t ray;
				ray.Init(cheat::main::local()->GetEyePosition(), vecCamOffset, Vector(-16, -16, -16), Vector(16, 16, 16));
				CTraceFilterWorldOnly filter;
				filter.pSkip = cheat::main::local();
				Source::m_pEngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

				static float old_frac = 0.f;

				if (trace.fraction < 1.0f)
				{
					if (trace.m_pEnt)
					{
						if (!trace.m_pEnt->GetClientClass() || trace.m_pEnt->GetClientClass() && trace.m_pEnt->GetClientClass()->m_ClassID != class_ids::CCSPlayer) {
							camAngles[2] *= trace.fraction;

							old_frac = trace.fraction;
						}
						else if (old_frac < trace.fraction)
							camAngles[2] *= old_frac;
					}
				}

				Source::m_pInput->m_vecCameraOffset = angles.NiceCode();
				Source::m_pInput->m_vecCameraOffset.z = camAngles[2]; // Thirdperson Distance
			}
		}
		else {
			Source::m_pInput->m_fCameraInThirdPerson = false;
			Source::m_pInput->m_vecCameraOffset.z = 0.f;

			static auto vmodel_x = Source::m_pCvar->FindVar("viewmodel_offset_x");
			if (vmodel_x) {
				vmodel_x->m_fnChangeCallbacks.m_Size = NULL;
				vmodel_x->SetValue(cheat::Cvars.Visuals_misc_off_x.GetValue());
			}

			static auto vmodel_y = Source::m_pCvar->FindVar("viewmodel_offset_y");
			if (vmodel_y) {
				vmodel_y->m_fnChangeCallbacks.m_Size = NULL;
				vmodel_y->SetValue(cheat::Cvars.Visuals_misc_off_y.GetValue());
			}

			static auto vmodel_z = Source::m_pCvar->FindVar("viewmodel_offset_z");
			if (vmodel_z) {
				vmodel_z->m_fnChangeCallbacks.m_Size = NULL;
				vmodel_z->SetValue(cheat::Cvars.Visuals_misc_off_z.GetValue());
			}
		}
		
		if (cheat::main::local()) {
			auto local_weapon = (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon()));

			if (local_weapon) {
				if (!cheat::main::local()->m_bIsScoped()) {
					if (zoom_sensitivity_ratio_mouse->GetFloat() != defaultSens)
						zoom_sensitivity_ratio_mouse->SetValue(defaultSens);

					cheat::main::fov = vsView->fov;
				}
				else
				{
					auto zl = local_weapon->m_zoomLevel();

					if (cheat::Cvars.Visuals_rem_scope.GetValue() && zl != 2) {
						vsView->fov = 90.f;
						if (zoom_sensitivity_ratio_mouse->GetFloat() != 2.0f)
							zoom_sensitivity_ratio_mouse->SetValue(2.0f);
					}
				}
			}
		}

		Source::m_pClientModeSwap->VCall<Fn>(Index::IBaseClientDLL::OverrideView)(ecx, vsView);
		
		if (cheat::game::pressed_keys[(int)cheat::Cvars.Exploits_fakeduck.GetValue()] && (int)cheat::Cvars.Exploits_fakeduck.GetValue() != 0 && cheat::main::local() && !cheat::main::local()->IsDead())
		{
			vsView->origin = cheat::main::local()->get_abs_origin() + Vector(0,0, Source::m_pGameMovement->GetPlayerViewOffset(false).z);

			if (Source::m_pInput->m_fCameraInThirdPerson) {
				Vector cam_ofs = Source::m_pInput->m_vecCameraOffset;
				Vector camAngles;

				camAngles[0] = cam_ofs[0];
				camAngles[1] = cam_ofs[1];
				camAngles[2] = 0;

				Vector camForward, camRight, camUp;
				Math::AngleVectors(camAngles, &camForward, &camRight, &camUp);

				Math::VectorMA(vsView->origin, -cam_ofs[2], camForward, vsView->origin);
			}
		}

		//	cheat::main::local()->m_flDuckAmount() = prev_duckamt;

		//if (cheat::game::pressed_keys[88] && cheat::main::local() && !cheat::main::local()->IsDead() /*&& (*reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(Source::m_pInput) + 0xAD))*/)
		//	vsView->origin = cheat::main::local()->get_abs_origin() + Vector(-110.f, -5.f,-5.f + (Source::m_pGameMovement->GetPlayerMins(false).z + Source::m_pGameMovement->GetPlayerMaxs(false).z) * 0.55f);
	}

	bool __fastcall DoPostScreenEffects(void* clientmode, void*, int a1)
	{
		/*matrix3x4_t mx[128];

		if (cheat::main::local() && !cheat::main::local()->IsDead() && (int)cheat::Cvars.Visuals_lglow.GetValue()) {
			memcpy(mx, cheat::main::local()->m_CachedBoneData().Base(), sizeof(matrix3x4_t) * cheat::main::local()->GetBoneCount());
			memcpy(cheat::main::local()->m_CachedBoneData().Base(), cheat::features::antiaimbot.last_sent_matrix, sizeof(matrix3x4_t) * cheat::main::local()->GetBoneCount());
		}*/

		using Fn = bool(__thiscall*)(void*, int);
		auto penis = Source::m_pClientModeSwap->VCall<Fn>(44)(clientmode, a1);

		/*if (cheat::main::local() && !cheat::main::local()->IsDead() && (int)cheat::Cvars.Visuals_lglow.GetValue()) {
			memcpy(cheat::main::local()->m_CachedBoneData().Base(), mx, sizeof(matrix3x4_t) * cheat::main::local()->GetBoneCount());
		}*/

		return penis;
	}
}