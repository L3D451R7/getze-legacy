#include "hooked.hpp"
#include "prediction.hpp"
#include "player.hpp"
#include "lag_compensation.hpp"
#include "anti_aimbot.hpp"
#include "angle_resolver.hpp"
#include <chrono>
#include "visuals.hpp"
#include "prop_manager.hpp"
#include "rmenu.hpp"
#include "menu.h"
#include "weapon.hpp"
#include "movement.hpp"
#include <cctype>

const unsigned int FCLIENTANIM_SEQUENCE_CYCLE = 0x00000001;

class CFlashLightEffect
{
public:
	bool m_bIsOn; //0x0000 
	char pad_0x0001[0x3]; //0x0001
	int m_nEntIndex; //0x0004 
	WORD m_FlashLightHandle; //0x0008 
	char pad_0x000A[0x2]; //0x000A
	float m_flMuzzleFlashBrightness; //0x000C 
	float m_flFov; //0x0010 
	float m_flFarZ; //0x0014 
	float m_flLinearAtten; //0x0018 
	bool m_bCastsShadows; //0x001C 
	char pad_0x001D[0x3]; //0x001D
	float m_flCurrentPullBackDist; //0x0020 
	DWORD m_MuzzleFlashTexture; //0x0024 
	DWORD m_FlashLightTexture; //0x0028 
	char m_szTextureName[64]; //0x1559888 
}; //Size=0x006C

void UpdateFlashLight(CFlashLightEffect *pFlashLight, const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp)
{
	// here we tell to the compiler wich calling convention we will use to call the function and the paramaters needed (note the __thiscall*)
	typedef void(__thiscall* UpdateLight_t)(void*, int, const Vector&, const Vector&, const Vector&, const Vector&, float, float, float, bool, const char*);

	static UpdateLight_t oUpdateLight = NULL;

	if (!oUpdateLight)
	{
		// here we have to deal with a call instruction (E8 rel32)
		DWORD callInstruction = Memory::Scan(cheat::main::clientdll, "E8 ? ? ? ? 8B 06 F3 0F 10 46"); // get the instruction address
		DWORD relativeAddress = *(DWORD*)(callInstruction + 1); // read the rel32
		DWORD nextInstruction = callInstruction + 5; // get the address of next instruction
		oUpdateLight = (UpdateLight_t)(nextInstruction + relativeAddress); // our function address will be nextInstruction + relativeAddress
	}

	oUpdateLight(pFlashLight, pFlashLight->m_nEntIndex, vecPos, vecForward, vecRight, vecUp, pFlashLight->m_flFov, pFlashLight->m_flFarZ, pFlashLight->m_flLinearAtten, pFlashLight->m_bCastsShadows, pFlashLight->m_szTextureName);
}

CFlashLightEffect *CreateFlashLight(int nEntIndex, const char *pszTextureName, float flFov, float flFarZ, float flLinearAtten)
{
	static DWORD oConstructor = Memory::Scan(cheat::main::clientdll, "55 8B EC F3 0F 10 45 ? B8");

	// we need to use the engine memory management if we are calling the destructor later
	CFlashLightEffect *pFlashLight = (CFlashLightEffect*)Source::m_pMemAlloc->Alloc(sizeof(CFlashLightEffect));

	if (!pFlashLight) {
		//std::cout << "Mem Alloc for FL failed" << std::endl;
		return NULL;
	}

	//std::cout << "pFlashLight: " << reinterpret_cast<DWORD*>(pFlashLight) << std::endl;

	// we need to call this function on a non standard way
	__asm
	{
		movss xmm3, flFov
		mov ecx, pFlashLight
		push flLinearAtten
		push flFarZ
		push pszTextureName
		push nEntIndex
		call oConstructor
	}

	return pFlashLight;
}

void DestroyFlashLight(CFlashLightEffect *pFlashLight)
{
	static auto oDestroyFlashLight = reinterpret_cast<void(__thiscall*)(void*, void*)>(Memory::Scan(cheat::main::clientdll, "56 8B F1 E8 ? ? ? ? 8B 4E 28"));

	//std::cout << "oDestroyFlashLight: " << reinterpret_cast<DWORD*>(oDestroyFlashLight) << std::endl;

	// the flash light destructor handles the memory management, so we dont need to free the allocated memory
	// call the destructor of the flashlight
	oDestroyFlashLight(pFlashLight, pFlashLight);
}

void RunFrame()
{
	static CFlashLightEffect *pFlashLight = NULL;

	if (GetAsyncKeyState(70) & 1)
	{
		//std::cout << "pressed F" << std::endl;

		if (!pFlashLight)
		{
			pFlashLight = CreateFlashLight(cheat::main::local()->entindex(), "effects/flashlight001", 40, 1000, 2000);
		}
		else
		{
			DestroyFlashLight(pFlashLight);
			pFlashLight = NULL;
		}
		Source::m_pSurface->PlaySound_("items\\flashlight1.wav");
		//items\\flashlight1
	}

	if (pFlashLight && !cheat::main::local()->IsDead() && Source::m_pEngine->IsConnected())
	{
		Vector f, r, u;
		auto viewAngles = Engine::Movement::Instance()->m_qRealAngles;
		Math::AngleVectors(viewAngles, &f, &r, &u);

		pFlashLight->m_bIsOn = true;
		pFlashLight->m_bCastsShadows = false;
		pFlashLight->m_flFov = 40;/*90 + SETTINGS::settings.override_FoV*//*fabsf(13 + 37 * cosf(x += 0.002f))*/;
		pFlashLight->m_flLinearAtten = 3000;
		UpdateFlashLight(pFlashLight, cheat::main::local()->GetEyePosition(), f, r, u);

		//std::cout << "UpdateFlashLight pFlashLight: " << reinterpret_cast<DWORD*>(pFlashLight) << std::endl;
	}
	else if (pFlashLight && (!Source::m_pEngine->IsConnected() || cheat::main::local()->IsDead()))
	{
		DestroyFlashLight(pFlashLight);
		pFlashLight = NULL;
	}
}

using CNETMsg_File_constructor_fn = void(__thiscall *)(void*);
using CNETMsg_File_destructor_fn = void(__thiscall *)(void*);
using CNETMsg_File_proto_fn = void(__thiscall *)(void*, void*);

template<typename t> t follow_rel32(uintptr_t address, size_t offset) {

	if (!address)
		return t{};

	auto offsetAddr = address + offset;
	auto relative = *(uint32_t *)offsetAddr;
	if (!relative)
		return t{};

	return (t)(offsetAddr + relative + sizeof(uint32_t));
}

bool strstric(const std::string& strHaystack, const std::string& strNeedle)
{
	auto it = std::search(
		strHaystack.begin(), strHaystack.end(),
		strNeedle.begin(), strNeedle.end(),
		[](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
	);
	return (it != strHaystack.end());
}

void night_mode()
{
	static auto sv_skyname = Source::m_pCvar->FindVar("sv_skyname");
	static auto original = sv_skyname->GetString();


	const auto reset = [&]()
	{
		Source::m_pCvar->FindVar("r_DrawSpecificStaticProp")->SetValue(1);
		sv_skyname->SetValue(original);

		for (auto i = Source::m_pMaterialSystem->FirstMaterial(); i != Source::m_pMaterialSystem->InvalidMaterial(); i = Source::m_pMaterialSystem->NextMaterial(i))
		{
			auto mat = Source::m_pMaterialSystem->GetMaterial(i);
			if (!mat)
				continue;

			if (mat->IsErrorMaterial())
				continue;

			std::string name = mat->GetName();
			auto tex_name = mat->GetTextureGroupName();


			if (strstr(tex_name, "World") || strstr(tex_name, "StaticProp") || strstr(tex_name, "SkyBox"))
			{
				mat->ColorModulate(1.f, 1.f, 1.f);
				mat->AlphaModulate(1.f);
			}
		}
	};

	//_(sky_csgo_night02, "sky_csgo_night02");
	const auto set = [&]()
	{
		//static auto load_named_sky = reinterpret_cast<void(__fastcall*)(const char*)>(sig("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45"));
		//load_named_sky(sky_csgo_night02);

		static const char* skyboxes[] = {
					"cs_tibet",
					"cs_baggage_skybox_",
					"embassy",
					"italy",
					"jungle",
					"office",
					"sky_cs15_daylight01_hdr",
					"vertigoblue_hdr",
					"sky_cs15_daylight02_hdr",
					"vertigo",
					"sky_day02_05_hdr",
					"nukeblank",
					"sky_venice",
					"sky_cs15_daylight03_hdr",
					"sky_cs15_daylight04_hdr",
					"sky_csgo_cloudy01",
					"sky_csgo_night02",
					"sky_csgo_night02b",
					"sky_csgo_night_flat",
					"sky_dust",
					"vietnam",
					"amethyst",
					"sky_descent",
					"clear_night_sky",
					"otherworld",
					"cloudynight",
					"dreamyocean",
					"grimmnight",
					"sky051",
					"sky081",
					"sky091",
					"sky561",
		};

		if ((int)cheat::Cvars.Visuals_world_sky.GetValue())
			sv_skyname->SetValue(skyboxes[(int)cheat::Cvars.Visuals_world_sky.GetValue() - 1]);
		else
			sv_skyname->SetValue(original);

		Source::m_pCvar->FindVar("r_DrawSpecificStaticProp")->SetValue(0);

		for (auto i = Source::m_pMaterialSystem->FirstMaterial(); i != Source::m_pMaterialSystem->InvalidMaterial(); i = Source::m_pMaterialSystem->NextMaterial(i))
		{
			auto mat = Source::m_pMaterialSystem->GetMaterial(i);
			if (!mat)
				continue;

			if (mat->IsErrorMaterial())
				continue;

			std::string name = mat->GetName();
			auto tex_name = mat->GetTextureGroupName();

			if (cheat::Cvars.Visuals_wrld_nightmode.GetValue() && strstr(tex_name, "World"))
			{
				mat->ColorModulate(0.15f, 0.13f, 0.14f);
			}
			if (strstr(tex_name, "StaticProp"))
			{
				/*if ( !strstric( name, "box" ) && !strstric( name, "crate" ) && !strstric( name, "door" ) && !
					strstric( name, "stoneblock" ) && !strstric( name, "tree" ) && !strstric( name, "flower" ) && !
					strstric( name, "light" ) && !strstric( name, "lamp" ) && !strstric( name, "props_junk" ) && !
					strstric( name, "props_pipe" ) && !strstric( name, "chair" ) && !strstric( name, "furniture" ) && !
					strstric( name, "debris" ) && !strstric( name, "tire" ) && !strstric( name, "refrigerator" ) && !
					strstric( name, "fence" ) && !strstric( name, "pallet" ) && !strstric( name, "barrel" ) && !strstric(
						 name, "wagon" ) && !strstric( name, "wood" ) && !strstric( name, "wall" ) && !strstric( name, "pole" ) && !strstric( name, "props_urban" ) && !strstric( name, "bench" ) && !strstric( name, "trashcan" ) && !strstric( name, "infwll" ) && !strstric( name, "cash_register" ) && !strstric( name, "prop_vehicles" ) && !strstric( name, "rocks" ) && !strstric( name, "artillery" ) && !strstric( name, "plaster_brick" ) && !strstric( name, "props_interiors" ) && !strstric( name, "props_farm" ) && !strstric( name, "props_highway" ) && !strstric( name, "orange" ) && !strstric( name, "wheel" ) )
					continue;*/

				if (cheat::Cvars.Visuals_wrld_nightmode.GetValue())
					mat->ColorModulate(0.4f, 0.4f, 0.4f);
				if ( /*!strstric( name, "wood" ) &&*/ !strstric(name, "wall"))
					mat->AlphaModulate(1.f - (cheat::Cvars.Visuals_wrld_prop_alpha.GetValue() * 0.01f));
			}

			/*if (ctx.m_settings.misc_visuals_nightmode && strstr(tex_name, "SkyBox"))
			{
				mat->ColorModulate(228.f / 255.f, 35.f / 255.f, 157.f / 255.f);
			}*/

		}
	};

	static auto done = true;
	static auto last_setting = false;
	static auto last_transparency = 0.f;
	static auto was_ingame = false;

	if (!done)
	{
		if (last_setting || last_transparency)
		{
			reset();
			set();
			done = true;
		}
		else
		{
			reset();
			done = true;
		}
	}

	if (was_ingame != Source::m_pEngine->IsInGame() || last_setting != cheat::Cvars.Visuals_wrld_nightmode.GetValue() || last_transparency != cheat::Cvars.Visuals_wrld_prop_alpha.GetValue())
	{
		last_setting = cheat::Cvars.Visuals_wrld_nightmode.GetValue();
		last_transparency = cheat::Cvars.Visuals_wrld_prop_alpha.GetValue();
		was_ingame = Source::m_pEngine->IsInGame();

		done = false;
	}
}

namespace Hooked
{
	static char mapname[128] = "";
	static bool old_map_light = false;
	bool svchts = false;
	static bool checked_fakelag = false;

	tl::optional<knife_t&> get_knife_by_id(uint16_t id)
	{
		for (auto& knife : parser::knifes.list) {
			if (knife.id == id) {
				return knife;
			}
		}

		return tl::nullopt;
	}

	void __fastcall FrameStageNotify(void* ecx, void* edx, ClientFrameStage_t stage)
	{
		auto clantag_changer = []() -> void
		{
			static std::string cur_clantag = " getze.us ";
			static float oldTime = -1.f;

			auto setclantag = [](const char* tag, const char* lol = "Piska") -> void
			{
				typedef void(__fastcall * SetClanTagFn)(const char*, const char*);
				static auto set_clan_tag = reinterpret_cast<SetClanTagFn>(Memory::Scan(cheat::main::enginedll, "53 56 57 8B DA 8B F9 FF 15"));

				if (!set_clan_tag)
					return;

				set_clan_tag(tag, lol);
			};

			auto Marquee = [](std::string& clantag) -> void
			{
				std::string temp = clantag;
				clantag.erase(0, 1);
				clantag += temp[0];
			};

			static float v22 = 0;
			float v14 = 0, v17 = 0;
			auto v15 = (1.0f / Source::m_pGlobalVars->interval_per_tick);
			auto v16 = Source::m_pEngine->GetNetChannelInfo();

			if (v16)
			{
				v17 = v16->GetAvgLatency(0);
				v14 = TIME_TO_TICKS(v17);
			}

			auto v21 = int((v14 + Source::m_pGlobalVars->tickcount) / (v15 / 2)) % 16;

			if (!cheat::Cvars.Visuals_misc_tag.GetValue()) {
				if (oldTime > -1.f) {
					setclantag("");
					oldTime = -1.f;
				}

				return;
			}

			if (Source::m_pEngine->IsInGame() && Source::m_pClientState && Source::m_pClientState->m_iChockedCommands <= 0) {
				/*switch (int(Source::m_pGlobalVars->realtime) % 5)
				{
				case 0: setclantag(u8"\u2708__\u258C_\u258C"); break;
				case 1: setclantag(u8"_\u2708_\u258C_\u258C"); break;
				case 2: setclantag(u8"__\u2708\u258C_\u258C"); break;
				case 3: setclantag(u8"___\u2620\u2708\u258C"); break;
				case 4: setclantag(u8"___\u2620_\u2620"); break;
				}*/

				if (v21 != v22)
				{
					oldTime = Source::m_pGlobalVars->realtime;
					v22 = v21;

					if (v21 == 0)
						cur_clantag = " getze.us ";

					Marquee(cur_clantag);

					setclantag(cur_clantag.c_str());
				}
			}
		};

		auto crash_server = [](int stage)->void
		{
			//auto sendnetmsg_rebuild = [](uint32_t netchan, void* msg, bool force_reliable = false, bool voice = false) -> bool
			//{
			//	bool is_reliable = force_reliable;
			//	if (!is_reliable) is_reliable = Memory::VCall<bool(__thiscall*)(void*)>(msg, 6)(msg);

			//	void* stream = nullptr;
			//	if (is_reliable)
			//		stream = (void*)((DWORD)netchan + 0x30);
			//	else
			//		stream = (void*)((DWORD)netchan + 0x54);

			//	if (voice)
			//		stream = (void*)((DWORD)netchan + 0x78);

			//	if (stream == nullptr)
			//		return false;

			//	return Memory::VCall<bool(__thiscall*)(void*, void*)>(msg, 5)(msg, stream);
			//};

			if (stage != FRAME_RENDER_START || !Source::m_pEngine->IsInGame())
				return;

			//static auto cnetmsg_file_ctor_offset = follow_rel32<uintptr_t>(Memory::Scan(cheat::main::enginedll, "E8 ? ? ? ? FF 75 08 83 4D E0 01"), 1);
			//static auto cnetmsg_file_dtor_offset = follow_rel32<uintptr_t>(Memory::Scan(cheat::main::enginedll, "E8 ? ? ? ? 8B 4D E0 8A 45 0C 83 C9 04 83 C9 08 88 45 D8 8D 46 30 89 4D E0") + 0x30, 1);
			//static auto cnetmsg_file_proto_offset = follow_rel32<uintptr_t>(Memory::Scan(cheat::main::enginedll, "E8 ? ? ? ? 8B 4D E0 8A 45 0C 83 C9 04 83 C9 08 88 45 D8 8D 46 30 89 4D E0"), 1);

			//if (cnetmsg_file_ctor_offset == 0 ||
			//	cnetmsg_file_dtor_offset == 0 ||
			//	cnetmsg_file_proto_offset == 0)
			//	return;

			//static auto constructor = CNETMsg_File_constructor_fn(cnetmsg_file_ctor_offset);
			//static auto destructor = CNETMsg_File_destructor_fn(cnetmsg_file_dtor_offset);
			//static auto protobuf = CNETMsg_File_proto_fn(cnetmsg_file_proto_offset);

			//byte msg[80] = { 0 };
			//constructor(msg);
			//protobuf(&msg[4], "null.wav");

			//auto tStart = std::chrono::high_resolution_clock::now();

			//int commands_count = 10000;

			//for (auto i = 0; i < commands_count; ++i) {
			//	sendnetmsg_rebuild((uint32_t)Source::m_pClientState->m_ptrNetChannel, msg, false, true);
			//}

			//auto time_delta = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds, long long>(std::chrono::high_resolution_clock::now() - tStart).count()) / 1000000000.f;
			//char buf[512]; sprintf(buf, "hit server with %d commands. it took %.5f seconds.", commands_count, time_delta);
			//_events.push_back(_event(Source::m_pGlobalVars->curtime + 2.f, std::string(buf)));

			bool hasProblem = Source::m_pEngine->GetNetChannelInfo()->IsTimingOut();

			// Request non delta compression if high packet loss, show warning message
			if (hasProblem)
				_events.push_back(_event(Source::m_pGlobalVars->curtime + 3.f, "server hit succeed! timeout triggered."));
			else
				_events.push_back(_event(Source::m_pGlobalVars->curtime + 1.f, "waiting server response."));

			//destructor(msg);
		};

		auto change_skins = [](int stage)->void
		{
			if (!Source::m_pEngine->IsInGame())
				return;
			if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
				return;
			if (!cheat::main::local())
				return;
			if (cheat::main::local()->m_iHealth() <= 0)
				return;

			cheat::main::updating_skins = false;

			player_info local_info;
			if (!Source::m_pEngine->GetPlayerInfo(Source::m_pEngine->GetLocalPlayer(), &local_info))
				return;

			auto weapons = cheat::main::local()->m_hMyWeapons();
			if (!weapons) return;

			auto i_weapon = (C_WeaponCSBaseGun*)Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon());

			if (!i_weapon) return;

			auto viewmodel = Source::m_pEntList->GetClientEntityFromHandle((CBaseHandle)cheat::main::local()->m_hViewModel());
			if (!viewmodel) return;

			auto worldmodel = Source::m_pEntList->GetClientEntityFromHandle(i_weapon->m_hWeaponWorldModel());
			if (!worldmodel) return;

			if (!parser::knifes.list.empty() && i_weapon->is_knife() && (int)cheat::Cvars.Skins_Knife.GetValue() > 0/* && parser::default_knifes.get_by_id(m_iItemDefinitionIndex).has_value() && parser::knifes.get_by_id(m_iItemDefinitionIndex).has_value()*/)
			{
				const auto new_knife_id = parser::knifes.list[((int)cheat::Cvars.Skins_Knife.GetValue() - 1)].id;

				auto knife = get_knife_by_id(new_knife_id);

				if (knife.has_value())
				{
					viewmodel->set_model_index(knife->model_player);
					worldmodel->set_model_index(knife->model_world);
				}
			}

			for (int i = 0; weapons[i] != 0xFFFFFFFF; i++)
			{
				if (!weapons[i])
					continue;

				auto weapon = (C_WeaponCSBaseGun*)Source::m_pEntList->GetClientEntity(weapons[i] & 0xFFF);

				if (!weapon)
					continue;

				auto &m_iItemIDHigh = weapon->m_iItemIDHigh();
				auto &m_iItemDefinitionIndex = weapon->m_iItemDefinitionIndex();
				auto &m_iEntityQuality = weapon->m_iEntityQuality();
				auto &m_iAccountID = weapon->m_iAccountID();
				auto &m_OriginalOwnerXuidLow = weapon->m_OriginalOwnerXuidLow();
				auto &m_OriginalOwnerXuidHigh = weapon->m_OriginalOwnerXuidHigh();
				//auto& m_szCustomName = weapon->m_szCustomName();

				weapon->m_iItemIDHigh() = -1;

				weapon->m_iAccountID() = local_info.xuidlow;

				if (local_info.xuidhigh != m_OriginalOwnerXuidHigh ||
					local_info.xuidlow != m_OriginalOwnerXuidLow)
					continue; // not OUR weapon

				auto type = weapon->GetCSWeaponData()->type;

				if (!parser::knifes.list.empty() && type == 0 && weapon->GetCSWeaponData()->max_clip == -1 && (int)cheat::Cvars.Skins_Knife.GetValue() > 0/* && parser::default_knifes.get_by_id(m_iItemDefinitionIndex).has_value() && parser::knifes.get_by_id(m_iItemDefinitionIndex).has_value()*/)
				{
					const auto new_knife_id = parser::knifes.list[((int)cheat::Cvars.Skins_Knife.GetValue() - 1)].id;

					auto knife = get_knife_by_id(new_knife_id);

					if (knife.has_value())
					{

						if ((int)weapon->m_iItemDefinitionIndex() != (int)new_knife_id)
						{
							weapon->m_iItemDefinitionIndex() = (int)new_knife_id;
							weapon->m_iEntityQuality() = 3;
						}

						if (knife->model_player == -1 || knife->model_world == -1) {
							knife->model_world = Source::m_pModelInfo->GetModelIndex(knife->model_world_path.c_str());
							knife->model_player = Source::m_pModelInfo->GetModelIndex(knife->model_player_path.c_str());
						}

						weapon->set_model_index(knife->model_player);
					}
				}

				auto skin = cheat::settings.paint[m_iItemDefinitionIndex];
				if (skin != weapon->m_nFallbackPaintKit())
				{
					weapon->m_nFallbackPaintKit() = skin;
					//m_nFallbackSeed = item.Seed;
					//m_nFallbackStatTrak = item.StatTrak;
					//if (item.StatTrak >= 0) m_iEntityQuality = 9;
					//if (item.Name) strcpy(m_szCustomName, item.Name);
				}
			}
		};

		static auto extend_fakelag_packets = []() -> void
		{
			static bool noob = false;

			if (noob)
				return;

			if (!noob) {
				//DWORD wowfast = 0x100d10bc + 1, old;
				static DWORD lol = Memory::Scan(cheat::main::enginedll, "55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? B9 ? ? ? ? 53 8B 98") + 0xBC + 1;
				DWORD old;

				//printf("%d\n", *(int*)lol);
				VirtualProtect((LPVOID)lol, 1, PAGE_READWRITE, &old);
				*(int*)lol = 62;
				VirtualProtect((LPVOID)lol, 1, old, &old);
				//printf("%d\n", *(int*)lol);

				//BYTE rarefast = 0x3E; // 62
				//VirtualProtect((LPVOID)wowfast, 1, PAGE_READWRITE, &old);
				//WriteProcessMemory(GetCurrentProcess(), (LPVOID)wowfast, &rarefast, sizeof(rarefast), 0);
				//VirtualProtect((LPVOID)wowfast, 1, old, &old);
				noob = true;
			}
		};

		static auto get_convars = []() -> void
		{
			static auto tick = -1;
			static bool was_connected = false;

			if (Source::m_pEngine->IsInGame() != was_connected) {
				tick = -1;
				was_connected = Source::m_pEngine->IsInGame();
			}

			if ((int)GetTickCount() >= tick && was_connected) {
				cheat::convars::weapon_recoil_scale = Source::m_pCvar->FindVar("weapon_recoil_scale")->GetFloat();
				cheat::convars::sv_usercmd_custom_random_seed = Source::m_pCvar->FindVar("sv_usercmd_custom_random_seed")->GetInt();
				cheat::convars::weapon_accuracy_nospread = Source::m_pCvar->FindVar("weapon_accuracy_nospread")->GetInt();

				cheat::convars::sv_clip_penetration_traces_to_players = Source::m_pCvar->FindVar("sv_clip_penetration_traces_to_players")->GetInt();
				cheat::convars::ff_damage_bullet_penetration = Source::m_pCvar->FindVar("ff_damage_bullet_penetration")->GetFloat();
				cheat::convars::ff_damage_reduction_bullets = Source::m_pCvar->FindVar("ff_damage_reduction_bullets")->GetFloat();
				cheat::convars::sv_penetration_type = Source::m_pCvar->FindVar("sv_penetration_type")->GetInt();

				tick = (int)GetTickCount() + 60000;
			}
		};

		QAngle aim, view;

		if (strcmp(mapname, "") && !Source::m_pEngine->IsInGame())
		{
			_events.clear();
			std::memset(cheat::main::history_hit, 0, sizeof(int) * 128);
			old_map_light = !cheat::Cvars.Visuals_wrld_nightmode.GetValue();
			checked_fakelag = false;
			cheat::features::antiaimbot.enable_delay = 0.f;

			cheat::main::command_numbers.clear();

			strcpy(mapname, "");
		}

		if (Source::m_pEngine->IsInGame() && strcmp(mapname, Source::m_pEngine->GetLevelName()))
		{
			_events.clear();
			std::memset(cheat::main::history_hit, 0, sizeof(int) * 128);
			old_map_light = !cheat::Cvars.Visuals_wrld_nightmode.GetValue();
			checked_fakelag = false;
			cheat::features::antiaimbot.enable_delay = 0.f;

			cheat::main::command_numbers.clear();

			cheat::main::reset_local_animstate = true;

			for (auto i = 0; i < parser::knifes.list.size(); i++)
			{
				auto knife = &parser::knifes.list[i];

				knife->model_world = Source::m_pModelInfo->GetModelIndex(knife->model_world_path.c_str());
				knife->model_player = Source::m_pModelInfo->GetModelIndex(knife->model_player_path.c_str());
			}

			strcpy(mapname, Source::m_pEngine->GetLevelName());
		}

		auto is_valid = Source::m_pClientState->m_iDeltaTick != -1 && cheat::main::local() != nullptr && Source::m_pEngine->IsInGame() && cheat::main::local()->GetClientClass() && !cheat::main::local()->IsDead() && (C_WeaponCSBaseGun*)(Source::m_pEntList->GetClientEntityFromHandle(cheat::main::local()->m_hActiveWeapon())) != nullptr && cheat::main::local()->m_iHealth() > 0;

		cheat::game::last_frame = stage;

		//cheat::features::music.run();

		if (stage == FRAME_RENDER_START && Source::m_pClientState->m_iDeltaTick > 0)
		{
			if (cheat::main::local() != nullptr && Source::m_pEngine->IsInGame())
			{
				//if (cheat::game::hud_death_notice == nullptr)
				//	cheat::game::hud_death_notice = cheat::game::find_hud_element<CCSGO_HudDeathNotice*>("CCSGO_HudDeathNotice");
				//if (cheat::game::update_hud_weapons == nullptr)
				//	cheat::game::update_hud_weapons = (void*)Memory::Scan(cheat::main::clientdll, "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C");

				extend_fakelag_packets();
				get_convars();
			}

			clantag_changer();
		}

		/*if (Source::m_pClientState && Source::m_pClientState->m_ptrNetChannel && (CClientState*)(uint32_t(Source::m_pClientState) + 8) && cheat::main::local() && !cheat::main::local()->IsDead()) {
			if (Source::m_pClientStateSwap && Source::m_pClientStateSwap->hooked) {
				uintptr_t* vtable = *(uintptr_t * *)(uint32_t(Source::m_pClientState) + 8);

				if (vtable != Source::m_pClientStateSwap->m_pRestore) {
					Source::m_pClientStateSwap.reset();
				}
			}
		}*/

		night_mode();

		//if (Source::m_pClientState->m_iDeltaTick > 0) {
			//crash_server(stage);
			change_skins(stage);
		//}

		if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		{
			if (is_valid && cheat::main::local())
			{
				if (cheat::Cvars.Visuals_rem_flash.GetValue())
					cheat::main::local()->m_flFlashDuration() = 0;
			}
		}

		if (stage == FRAME_RENDER_START && is_valid)
		{
			if (Source::m_pInput->m_fCameraInThirdPerson)
				cheat::main::local()->UpdateVisibilityAllEntities();

			Source::m_pInput->m_fCameraInThirdPerson = false;

			if (cheat::Cvars.Visuals_rem_punch.GetValue()) {
				aim = cheat::main::local()->m_aimPunchAngle();
				view = cheat::main::local()->m_viewPunchAngle();

				cheat::main::local()->m_aimPunchAngle().Set();
				cheat::main::local()->m_viewPunchAngle().Set();
			}

			if (cheat::game::get_key_press((int)cheat::Cvars.Visuals_lp_toggletp.GetValue()) && (int)cheat::Cvars.Visuals_lp_toggletp.GetValue() > 0 && !cheat::features::menu.menu_opened)
				cheat::main::thirdperson = !cheat::main::thirdperson;

			if (Source::m_pClientSwap)
				Source::m_pClientSwap->VCall<void(__thiscall*)(void*, ClientFrameStage_t)>(Index::IBaseClientDLL::FrameStageNotify)(ecx, stage);

			if (cheat::Cvars.Visuals_rem_punch.GetValue()) {
				cheat::main::local()->m_aimPunchAngle() = aim;
				cheat::main::local()->m_viewPunchAngle() = view;
			}
		}
		else if (Source::m_pClientSwap)
			Source::m_pClientSwap->VCall<void(__thiscall*)(void*, ClientFrameStage_t)>(Index::IBaseClientDLL::FrameStageNotify)(ecx, stage);

		if (cheat::main::local() != nullptr && Source::m_pEngine->IsInGame() && Source::m_pClientState->m_iDeltaTick > 0)
		{
			if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
			{
				static auto old_velocity_modifier = cheat::main::local()->m_flVelocityModifier();
				static float old_flNextCmdTime = 0.f;
				static int old_iLastCommandAck = 0;

				if (Source::m_pClientState)
				{
					auto v146 = Source::m_pClientState->m_iLastCommandAck;
					auto v161 = Source::m_pClientState->m_flNextCmdTime;
					if (old_flNextCmdTime != v161 || old_iLastCommandAck != v146)
					{
						auto v154 = cheat::main::local()->m_flVelocityModifier();
						if (old_velocity_modifier != v154)
						{
							*(bool*)(uintptr_t(Source::m_pPrediction) + 0x24) = true;
							old_velocity_modifier = v154;
						}
						old_flNextCmdTime = v161;
						old_iLastCommandAck = v146;
					}
				}
			}
			if (stage == FRAME_NET_UPDATE_END)
				cheat::features::lagcomp.store_records();
		}
	}

}