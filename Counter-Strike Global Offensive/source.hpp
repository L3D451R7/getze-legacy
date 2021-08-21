#pragma once

#include "sdk.hpp"

namespace Source
{
	extern IBaseClientDLL* m_pClient;
	extern ISurface* m_pSurface;
	extern InputSystem* m_pInputSystem;
	extern ICvar* m_pCvar;
	extern IClientEntityList* m_pEntList;
	extern IGameMovement* m_pGameMovement;
	extern IPrediction* m_pPrediction;
	extern IMoveHelper* m_pMoveHelper;
	extern IInput* m_pInput;
	extern CClientState* m_pClientState;
	extern IVModelInfo* m_pModelInfo;
	extern CGlobalVars* m_pGlobalVars;
	extern IVEngineClient* m_pEngine;
	extern IPhysicsSurfaceProps* m_pPhysProps;
	extern IMaterialSystem* m_pMaterialSystem;
	extern IPanel* m_pPanel;
	extern IVRenderView* m_pRenderView;
	extern IEngineTrace* m_pEngineTrace;
	extern IVDebugOverlay* m_pDebugOverlay;
	extern IMemAlloc* m_pMemAlloc;
	extern IGameEventManager* m_pGameEvents;
	extern IVModelRender* m_pModelRender;
	extern IEngineVGui* m_pEngineVGUI;
	extern ILocalize* m_pLocalize;
	extern IEngineSound* m_pEngineSound;

	extern Memory::VmtSwap::Shared m_pClientSwap;
	extern Memory::VmtSwap::Shared m_pClientStateSwap;
	extern Memory::VmtSwap::Shared m_pClientModeSwap;
	extern Memory::VmtSwap::Shared m_pSurfaceSwap;
	extern Memory::VmtSwap::Shared m_pPredictionSwap;
	extern Memory::VmtSwap::Shared m_pPanelSwap;
	extern Memory::VmtSwap::Shared m_pRenderViewSwap;
	extern Memory::VmtSwap::Shared m_pEngineSwap;
	extern Memory::VmtSwap::Shared m_pFireBulletsSwap;
	extern Memory::VmtSwap::Shared m_pEngineVGUISwap;
	extern Memory::VmtSwap::Shared m_pModelRenderSwap;
	extern Memory::VmtSwap::Shared m_pNetChannelSwap;
	extern Memory::VmtSwap::Shared m_pShowImpactsSwap;

	bool Create();
	void Destroy();

	void* CreateInterface(const std::string& image_name, const std::string& name, bool force = false);

}

class c_variables
{
public:
	/*bool save_cfg(std::string file_name);
	bool load_cfg(std::string file_name);
	bool reset_settings();
	std::string cfg_folder();*/

	/* ragebot shit */
	//bool	ragebot_enabled = true;
	//bool	ragebot_hitscan[5] = { true, false, true, true, true };
	//bool	ragebot_pointscan[5] = { true, false, true, true, true };
	//bool	ragebot_lagcomp = true;
	//bool	ragebot_lagfix = false;
	//bool	ragebot_removals = true;
	//bool	ragebot_lagshoot = true;
	//bool	ragebot_autor8 = true;
	//bool	ragebot_resolver = true;
	//bool	ragebot_select_by_hp = true;
	//bool	ragebot_scale_damage_on_hp = true;
	//bool	ragebot_autoscope = true;
	//int	ragebot_delay_shot = 1;
	//bool	ragebot_backshot = false;
	//bool	ragebot_lag = false;

	/* anti aim */

	//bool	ragebot_anti_aim_enabled = true;
	//int		ragebot_anti_aim_pitch = 2;

	//struct c_different_modes
	//{
	//	int		anti_aim_yaw = 0;
	//	int		desync_mode = 2;
	//	int		desync_range = -1;
	//	bool	desync_randomize = false;
	//} ragebot_anti_aim_settings[3];

	//bool	ragebot_anti_aim_break_lby = true;
	//int		ragebot_anti_aim_direction = 1;
	//int		ragebot_anti_aim_lby_delta = 114;
	//int		ragebot_anti_aim_desync_walk = 1;
	//int		ragebot_anti_aim_desync_walk_speed = 7;

	/* resolver preferences */
	//bool	resolver_autoresolve = true;
	/*bool	resolver_freestand = false;
	bool	resolver_lastmovelby = false;*/

	/* hitscan overriders */
	//int		ragebot_hs_baim_hp = 0;
	//bool	ragebot_hs_headaim_lby = true;
	//bool	ragebot_hs_headaim_shotbt = true;
	//bool	ragebot_hs_headaim_move = true;
	//bool	ragebot_hs_baim_awp = true;
	//int		ragebot_hs_baim_preference = 0;
	//bool	ragebot_hs_headaim_preferences[3] = { false, false, false };
	//bool	ragebot_hs_baim_preferences[5] = { false, false, false, false, false };

	/* accuracy preferences */
	//int		ragebot_hitchance = 50;
	//int		ragebot_hitchance_fakeduck = 80;
	//int		ragebot_min_damage = 19;
	//int		ragebot_pointscale_head = 65;
	//int		ragebot_pointscale_body = 95;

	/* visual shit */
	//bool	esp_enabled = true;
	//bool	esp_teammates = false;
	//int		esp_box = 0;
	//bool	esp_name = true;
	//bool	esp_weapon = true;
	//bool	esp_health = true;
	//bool	esp_split_health = true;
	//int		esp_skeleton = 0;
	//bool	glow_enabled = true;
	//bool	esp_snaplines = false;
	//bool	esp_pov_arrows = false;

	//bool	visuals_effects_no_post_process = false;
	//bool	visuals_effects_nightmode = false;
	//float	visuals_effects_misc_props_modifier = 100.f;

	//bool	visuals_removals_smoke = false;
	//bool	visuals_removals_flash = false;
	//bool	visuals_removals_scope = false;
	//bool	visuals_removals_punch = false;

	//int		visuals_misc_crosshair = 0;
	//int		visuals_misc_spread_visualize = 0;
	//bool	visuals_misc_preserve_killfeed = false;
	//bool	visuals_misc_thirdperson = false;
	//bool	visuals_misc_thirdperson_grenade = false;
	//float	visuals_misc_thirdperson_dist = 30.f;
	//bool	visuals_misc_grenade_trajectory = false;

	/*bool	visuals_misc2_manual_indicator = false;
	bool	visuals_misc2_desync_indicator = false;
	bool	visuals_misc2_fakelag_indicator = false;
	bool	visuals_misc2_event_log = false;*/

	//int		chams_enabled = false;
	//bool	chams_hidden = false;
	//bool	chams_teammates = false;

	//float	esp_color[3] = { Color(0, 129, 255, 255).Hue() / 360.f, 1.f, 1.f };
	//float	skeleton_color[3] = { Color(255, 255, 255, 255).Hue() / 360.f, 1.f, 1.f };
	//float	glow_color[3] = { Color(0, 129, 255, 255).Hue() / 360.f, 204 / 255.f, 1.f };
	//float	snap_lines_color[3] = { Color(128, 0, 128, 255).Hue() / 360.f, 1.f, 1.f };
	//float	map_color[3] = { 1.f, 1.f, 0.f };
	//float	sky_color[3] = { 1.f, 1.f, 0.f };
	//float	chams_color[3] = { Color(0, 129, 255, 255).Hue() / 360.f, 1.f, 0.f };
	//float	hidden_chams_color[3] = { Color(0, 255, 129, 255).Hue() / 360.f, 1.f, 0.f };

	//float	visuals_misc_viewmodel_fov = 30.f;
	//float	visuals_misc_viewmodel[3] = { 1.f, 1.f, 1.f };

	/* misc shit */
	//int	misc_fwalk_speed = 10;
	//int		misc_fakelag_trigger = 0;
	//int		misc_fakelag_base_factor = 0;
	//float   misc_fakelag_variance = 0.f;
	//float	misc_fakelag_value = 15.f;
	
	//bool	misc_strafer = true;
	//bool	misc_bhop = true;
	//bool	misc_cstrafer = false; 
	//bool	misc_ctag = false;

	int		music_curtrack = 0;
	std::unordered_map<int, int> paint;
	Vector2D radar_pos = { 100, 200 };
};

#include "parser.hpp"