#include "hooked.hpp"
#include "menu.h"
#include "visuals.hpp"
#include "displacement.hpp"
#include "player.hpp"
#include "anti_aimbot.hpp"
#include "lag_compensation.hpp"
#include "rmenu.hpp"
#include "sound_parser.hpp"
#include "angle_resolver.hpp"

using FnPT = void(__thiscall*)(void*, unsigned int, bool, bool);
using FnFOV = float(__stdcall *)();

float shit = 0.f;
bool ss = false;
bool glow_call = false;

namespace Hooked
{
	int ScreenSize2W, ScreenSize2H;

	void __fastcall PaintTraverse(void* ecx, void* edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
	{
		if (strcmp("HudZoom", Source::m_pPanel->GetName(vguiPanel)) || !cheat::Cvars.Visuals_rem_scope.GetValue())//(std::string(Source::m_pPanel->GetName(vguiPanel)).find("HudZoom") == std::string::npos || !cheat::Cvars.Visuals_rem_scope.GetValue() || !cheat::main::local() || cheat::main::local()->IsDead()) && Source::m_pPanelSwap)
			Source::m_pPanelSwap->VCall<FnPT>(Index::IPanel::PaintTraverse)(ecx, vguiPanel, forceRepaint, allowForce);

		static unsigned int drawPanel = 0;
		if (!drawPanel)
		{
			const char* panelname = Source::m_pPanel->GetName(vguiPanel);

			if (panelname[0] == 'M' && panelname[2] == 't')
				drawPanel = vguiPanel;
		}

		if (vguiPanel != drawPanel)
			return;

		static bool bResChange = false;

		Source::m_pEngine->GetScreenSize(ScreenSize2W, ScreenSize2H);

		if (!bResChange && (ScreenSize2W != cheat::game::screen_size.x
			|| ScreenSize2H != cheat::game::screen_size.y))
		{
			cheat::game::screen_size.x = ScreenSize2W;
			cheat::game::screen_size.y = ScreenSize2H;
			bResChange = true;
		}

		if (bResChange)
		{
			Drawing::CreateFonts();
			bResChange = false;
		}

		if (Source::m_pEngine->IsInGame() && cheat::main::local() && !cheat::main::local()->IsDead() && cheat::Cvars.Visuals_misc_preserve_kills.GetValue()) {
			//if (cheat::game::hud_death_notice) {
			//	float* localDeathNotice = &cheat::game::hud_death_notice->localplayer_lifetime_mod;
			//	//static auto penis = (Memory::Scan("client.dll", "E8 ? ? ? ? 66 83 3E"));
			//	if (localDeathNotice) *localDeathNotice = /*Vars.Visuals.PreserveKillfeed ? */FLT_MAX/*: 1.5f*/;
			//}
		}

		//Drawing::DrawPixel(1, 1, Color::Black());
		//Drawing::DrawString(F::ESP, 5, 2, Color::White(), FONT_LEFT, "csgo hvh hack");
		//cheat::features::menu.render();
		//cheat::features::visuals.render();

		static bool* s_bOverridePostProcessingDisable = *(bool**)(Memory::Scan(cheat::main::clientdll, "80 3D ? ? ? ? ? 53 56 57 0F 85") + 0x2);
		*s_bOverridePostProcessingDisable = false;//(Source::m_pEngine->IsInGame() && cheat::main::local() && cheat::Cvars.Visuals_wrld_postprocess.GetValue());

		if (Source::m_pEngine->IsInGame() && cheat::main::local())
		{
			static auto lol = Source::m_pCvar->FindVar("mat_postprocess_enable");

			if (lol) {
				if ((int)cheat::Cvars.Visuals_wrld_postprocess.GetValue() == 1)
					lol->SetValue(0);
				else
					lol->SetValue(1);
			}
		}
	}

	float __fastcall GetScreenAspectRatio(void *pEcx, void *pEdx, int32_t iWidth, int32_t iHeight)
	{
		static bool lel = true;
		using FnAR = float(__thiscall*)(void*, void*, int32_t, int32_t);
		auto original = Source::m_pEngineSwap->VCall<FnAR>(101)(pEcx, pEdx, iWidth, iHeight);

		if (lel)
		{
			cheat::Cvars.Visuals_misc_screen_aspr.SetValue(((float)cheat::game::screen_size.x / (float)cheat::game::screen_size.y));
			lel = !(((float)cheat::game::screen_size.x / (float)cheat::game::screen_size.y) > 0.f);
		}

		auto mm = cheat::Cvars.Visuals_misc_screen_aspr.GetValue();

		if (mm != 0.f)
			return mm;
		else
			return ((float)cheat::game::screen_size.x / (float)cheat::game::screen_size.y);
	}

	float __stdcall GetViewModelFOV() {
		float fov = Source::m_pClientModeSwap->VCall<FnFOV>(Index::IBaseClientDLL::GetViewModelFOV)();

		if (Source::m_pEngine->IsConnected() && Source::m_pEngine->IsInGame() && cheat::main::local()) {
			fov = fov + cheat::Cvars.Visuals_misc_fov.GetValue();
		}

		return fov;
	}

	void __stdcall EngineVGUI_Paint(int mode)
	{
		typedef void(__thiscall* Paint_t)(IEngineVGui*, int);
		Source::m_pEngineVGUISwap->VCall<Paint_t>(14)(Source::m_pEngineVGUI, mode);

		typedef void(__thiscall *start_drawing)(void *);
		typedef void(__thiscall *finish_drawing)(void *);

		static start_drawing start_draw = (start_drawing)Memory::Scan("vguimatsurface.dll", "55 8B EC 83 E4 C0 83 EC 38");
		static finish_drawing end_draw = (finish_drawing)Memory::Scan("vguimatsurface.dll", "8B 0D ? ? ? ? 56 C6 05");

		if (mode & 1)
		{
			start_draw(Source::m_pSurface);

			static bool bResChange = false;

			Source::m_pEngine->GetScreenSize(ScreenSize2W, ScreenSize2H);

			if (!bResChange && (ScreenSize2W != cheat::game::screen_size.x
				|| ScreenSize2H != cheat::game::screen_size.y))
			{
				cheat::game::screen_size.x = ScreenSize2W;
				cheat::game::screen_size.y = ScreenSize2H;
				bResChange = true;
			}

			if (bResChange)
				Drawing::CreateFonts();
			
			cheat::features::dormant.start();
			cheat::features::menu.render();
			cheat::features::visuals.render(bResChange);
			cheat::menu.UpdateMenu();
			cheat::menu.HandleTopMost();
			cheat::features::dormant.finish();

			if (bResChange)
				bResChange = false;

			end_draw(Source::m_pSurface);
		}
	}

	void __fastcall SceneEnd(void* ecx, void* edx)
	{
		////--- Wireframe Smoke ---//
		//std::vector<const char*> vistasmoke_wireframe =
		//{
		//	"particle/vistasmokev1/vistasmokev1_smokegrenade",
		//};

		//std::vector<const char*> vistasmoke_nodraw =
		//{
		//	"particle/vistasmokev1/vistasmokev1_fire",
		//	"particle/vistasmokev1/vistasmokev1_emods",
		//	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		//};

		//for (auto mat_s : vistasmoke_wireframe)
		//{
		//	IMaterial* mat = Source::m_pMaterialSystem->FindMaterial(mat_s, "Other textures");
		//	mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true); //wireframe
		//}

		//for (auto mat_n : vistasmoke_nodraw)
		//{
		//	IMaterial* mat = Source::m_pMaterialSystem->FindMaterial(mat_n, "Other textures");
		//	mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		//}

		Source::m_pRenderViewSwap->VCall<void(__thiscall*)(void*)>(9)(ecx);

		if ((cheat::Cvars.Visuals_glow.GetValue() || cheat::Cvars.Visuals_lglow.GetValue()) && Source::m_pEngine->IsConnected() && Source::m_pEngine->IsInGame() && cheat::main::local() && Engine::Displacement::Data::m_pGlowManager)
		{
			for (auto i = 0; i < Engine::Displacement::Data::m_pGlowManager->GetSize(); i++)
			{
				auto &glowObject = Engine::Displacement::Data::m_pGlowManager->m_GlowObjectDefinitions[i];
				auto entity = reinterpret_cast<C_BasePlayer*>(glowObject.m_pEntity);

				if (!entity)
					continue;

				if (glowObject.IsUnused())
					continue;

				if (!entity->GetClientClass() || entity->GetClientClass()->m_ClassID != class_ids::CCSPlayer)
					continue;

				bool is_local_player = entity == cheat::main::local();
				//bool is_teammate = cheat::main::local()->m_iTeamNum() == entity->m_iTeamNum() && !is_local_player;
				bool is_enemy = cheat::main::local()->m_iTeamNum() != entity->m_iTeamNum() || is_local_player;

				if (!is_local_player && !(int)cheat::Cvars.Visuals_glow.GetValue())
					continue;
				else
					if (is_local_player && !(int)cheat::Cvars.Visuals_lglow.GetValue())
						continue;

				if (!is_enemy)
					continue;

				Color color = /*Color().FromHSB(cheat::settings.glow_color[0], cheat::settings.glow_color[2], 1.f)*/cheat::Cvars.Visuals_glow_color.GetColor();

				//if (is_local_player && SETTINGS::main_configs.glow_local_enabled)
				//{
				//	should_draw_glow = true;
				//	color = SETTINGS::main_configs.glow_local_color;
				//	style = SETTINGS::main_configs.glow_local_style;
				//	full_bloom = SETTINGS::main_configs.glow_local_fullbloom_enabled;
				//}

				//if (is_teammate && SETTINGS::main_configs.glow_team_enabled)
				//{
				//	should_draw_glow = true;
				//	color = SETTINGS::main_configs.glow_team_color;
				//	style = SETTINGS::main_configs.glow_team_style;
				//	full_bloom = SETTINGS::main_configs.glow_team_fullbloom_enabled;
				//}

				if (is_local_player) 
				{
					if (!(int)cheat::Cvars.Visuals_lglow.GetValue())
						continue;

					glowObject.m_nGlowStyle = (int)cheat::Cvars.Visuals_lglow.GetValue() - 1;
					color = cheat::Cvars.Visuals_lglow_color.GetColor();
				}
				else
					glowObject.m_nGlowStyle = (int)cheat::Cvars.Visuals_glow.GetValue() - 1;

				glowObject.m_bFullBloomRender = false;
				glowObject.m_flRed = color.r() / 255.0f;
				glowObject.m_flGreen = color.g() / 255.0f;
				glowObject.m_flBlue = color.b() / 255.0f;
				glowObject.m_flAlpha = color.a() / 255.0f;
				glowObject.m_bRenderWhenOccluded = true;
				glowObject.m_bRenderWhenUnoccluded = false;
			}
		}

		//--- Wireframe Smoke ---//
		std::vector<const char*> vistasmoke_wireframe =
		{
			"particle/vistasmokev1/vistasmokev1_smokegrenade",
		};

		std::vector<const char*> vistasmoke_nodraw =
		{
			"particle/vistasmokev1/vistasmokev1_fire",
			"particle/vistasmokev1/vistasmokev1_emods",
			"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		};

		for (auto mat_s : vistasmoke_wireframe)
		{
			IMaterial* mat = Source::m_pMaterialSystem->FindMaterial(mat_s, "Other Textures");
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true); //wireframe
		}

		for (auto mat_n : vistasmoke_nodraw)
		{
			IMaterial* mat = Source::m_pMaterialSystem->FindMaterial(mat_n, "Other Textures");
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		}

		if (!cheat::main::local() || !Source::m_pEngine->IsInGame() || !cheat::Cvars.Visuals_chams_type.GetValue())
			return;

		static auto mat = Source::m_pMaterialSystem->CreateMaterial(true, true, false); //Source::m_pMaterialSystem->FindMaterial("chams", "Model textures");
		//static auto fake_mat = Source::m_pMaterialSystem->FindMaterial("dev/glow_armsrace.vmt", nullptr, false, 0);// "Other textures");

		if (!mat || mat->IsErrorMaterial())
			return;

		Color hcolor = cheat::Cvars.Visuals_chams_hidden_color.GetColor();//Color().FromHSB(cheat::settings.hidden_chams_color[0], cheat::settings.hidden_chams_color[2], 1.f);
		float hclr[] = { hcolor.r() / 255.f, hcolor.g() / 255.f, hcolor.b() / 255.f, 1.f };

		Color vcolor = cheat::Cvars.Visuals_chams_color.GetColor();//Color().FromHSB(cheat::settings.chams_color[0], cheat::settings.chams_color[2], 1.f);
		float vclr[] = { vcolor.r() / 255.f, vcolor.g() / 255.f, vcolor.b() / 255.f, 1.f };

		Color lcolor = cheat::Cvars.Visuals_lchams_color.GetColor();//Color().FromHSB(cheat::settings.chams_color[0], cheat::settings.chams_color[2], 1.f);
		float lclr[] = { lcolor.r() / 255.f, lcolor.g() / 255.f, lcolor.b() / 255.f, 1.f };

		for (auto i = 0; i < Source::m_pGlobalVars->maxClients; i++)
		{
			auto entity = Source::m_pEntList->GetClientEntity(i);

			if (!entity)
				continue;

			if (!entity->GetClientClass() || entity->GetClientClass()->m_ClassID != class_ids::CCSPlayer)
				continue;

			bool is_local_player = entity == cheat::main::local() && entity->entindex() == cheat::main::local()->entindex();
			bool is_enemy = cheat::main::local()->m_iTeamNum() != entity->m_iTeamNum() || is_local_player;

			if (!is_enemy && !cheat::Cvars.Visuals_chams_teammates.GetValue())
				continue;

			auto player_record = &cheat::features::lagcomp.records[entity->entindex() - 1];
			auto resolver_record = &cheat::features::aaa.player_resolver_records[entity->entindex() - 1];

			if (TIME_TO_TICKS(entity->m_flSimulationTime() - resolver_record->last_simtime) > 31)
				continue;

			cheat::main::called_chams_render = false;

			if (!entity->IsDead() && !entity->IsDormant())
			{
				auto animstate = entity->get_animation_state();

				if (!animstate)
					continue;

				QAngle ang = { 0,0,0 };

				if (is_local_player)
				{
					cheat::main::called_chams_render = true;

					//const auto blend = Source::m_pRenderView->GetBlend();



					if (cheat::Cvars.Visuals_lchams_enabled.GetValue())
					{
						//const auto origin = entity->m_vecOrigin();

						//entity->set_abs_origin(cheat::features::antiaimbot.last_sent_origin);
						//mat->IncrementReferenceCount();
						mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						Source::m_pRenderView->SetColorModulation(lclr);
						//mat->ColorModulate(lclr[0], lclr[1], lclr[2]);
						Source::m_pModelRender->ForcedMaterialOverride(mat);

						entity->draw_model(0x1, 255);
						Source::m_pModelRender->ForcedMaterialOverride(nullptr);
						//entity->set_abs_origin(origin);
					}

					Source::m_pModelRender->ForcedMaterialOverride(nullptr);

					continue;
				}
				else if (!cheat::Cvars.Visuals_chams_type.GetValue())
					continue;

				if (cheat::Cvars.Visuals_chams_hidden.GetValue()) 
				{
					//mat->IncrementReferenceCount();
					mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
					Source::m_pRenderView->SetColorModulation(hclr);
					//mat->ColorModulate(hclr[0], hclr[1], hclr[2]);
					Source::m_pModelRender->ForcedMaterialOverride(mat);

					entity->draw_model(0x1, 255);
					Source::m_pModelRender->ForcedMaterialOverride(nullptr);
				}

				//mat->IncrementReferenceCount();
				mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
				Source::m_pRenderView->SetColorModulation(vclr);

				//mat->ColorModulate(vclr[0], vclr[1], vclr[2]);
				Source::m_pModelRender->ForcedMaterialOverride(mat);
				entity->draw_model(0x1, 255);
				Source::m_pModelRender->ForcedMaterialOverride(nullptr);

				if (cheat::Cvars.Visuals_chams_type.GetValue() == 2 && player_record->tick_records.size() > 3 && !cheat::main::local()->IsDead() && !is_local_player)
				{
					C_Tickrecord backup;
					cheat::features::lagcomp.store_record_data(entity, &backup);

					auto target_record = &player_record->tick_records.at(player_record->tick_records.size() - 2);

					if (backup.simulation_time > 0.f && target_record->origin.Distance(backup.origin) > 16) {

						cheat::features::lagcomp.apply_record_data(entity, target_record);

						//mat->IncrementReferenceCount();
						mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
						Source::m_pRenderView->SetColorModulation(hclr);
						//mat->ColorModulate(hclr[0], hclr[1], hclr[2]);

						Source::m_pModelRender->ForcedMaterialOverride(mat);
						entity->draw_model(0x1, 255);
						Source::m_pModelRender->ForcedMaterialOverride(nullptr);
						cheat::features::lagcomp.apply_record_data(entity, &backup, true);
					}
				}
			}

			Source::m_pModelRender->ForcedMaterialOverride(nullptr);
		}
	}

	int __stdcall DrawModelExecute(void* context, void* state, ModelRenderInfo_t& info, matrix3x4_t* pCustomBoneToWorld)
	{
		Source::m_pModelRenderSwap->Restore();

		//Source::m_pModelRenderSwap->VCall<int(__stdcall*)(void*, void*, ModelRenderInfo_t&, matrix3x4_t*)>(21)(context, state, info, pCustomBoneToWorld);
		//Source::m_pModelRender->DrawModelExecute(context, state, info, pCustomBoneToWorld);

		if (!Source::m_pModelRender->IsForcedMaterialOverride() || cheat::Cvars.Visuals_lchams_enabled.GetValue()) 
		{

			//static auto mat = Source::m_pMaterialSystem->CreateMaterial(true, true, false); //Source::m_pMaterialSystem->FindMaterial("chams", "Model textures");

			//Color lcolor = cheat::Cvars.Visuals_lchams_color.GetColor();
			//float lclr[] = { lcolor.r() / 255.f, lcolor.g() / 255.f, lcolor.b() / 255.f, 1.f };

			//auto modelName = Source::m_pModelInfo->GetModelName(info.pModel);

			//static auto fake_mat = Source::m_pMaterialSystem->FindMaterial("dev/glow_armsrace.vmt", nullptr, false, 0);// "Other textures");

			//if (Source::m_pEngine->IsInGame() && cheat::main::local() && Source::m_pClientState->m_iDeltaTick != -1 && info.pModel)
			//{
			//	auto model_ent = Source::m_pEntList->GetClientEntity(info.entity_index);

			//	if (model_ent && model_ent->GetClientClass() && model_ent->GetClientClass()->m_ClassID == class_ids::CCSPlayer && model_ent == cheat::main::local())
			//	{
			//		Source::m_pRenderView->SetColorModulation(lclr);
			//		Source::m_pModelRender->ForcedMaterialOverride(mat);
			//		if (cheat::Cvars.Visuals_lchams_type.GetValue() == 2)
			//			Source::m_pModelRender->DrawModelExecute(context, state, info, cheat::features::antiaimbot.last_sent_matrix);
			//		else
			//			Source::m_pModelRender->DrawModelExecute(context, state, info, pCustomBoneToWorld);
			//		Source::m_pModelRender->ForcedMaterialOverride(nullptr);
			//	}
			//}

			//int bRet = Source::m_pModelRender->DrawModelExecute(context, state, info, pCustomBoneToWorld);
			//Source::m_pModelRender->ForcedMaterialOverride(NULL);
			//Source::m_pModelRenderSwap->Replace();

			//return bRet;
		}
		else
		{
			int bRet = Source::m_pModelRender->DrawModelExecute(context, state, info, pCustomBoneToWorld);
			Source::m_pModelRenderSwap->Replace();

			return bRet;
		}
	}
}

