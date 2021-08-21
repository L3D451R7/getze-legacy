#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <sys\stat.h>
#include <algorithm>
#include "rmenu.hpp" 
#include "menu.h"
#include "source.hpp"
#include "hooked.hpp"
#include "simpleini.hpp"
#include <locale>
#include <shlobj_core.h>

CWindow main;
CWindow Radar;
CWindow SpecWindow;
CWindow FinderWindow;
CWindow EditorWindow;

CTab legitbot;
CTab ragebot;
CTab visuals;
CTab misc;
CTab skins;
CTab Lists;
CTab settings;
CTab rd;
CTab spectab;
CTab badguytab;
CTab ESPedittab;
CButton Compile;

std::string CVars::GetClipBoardText()
{
	std::string text;

	if (OpenClipboard(NULL))
	{
		auto clip = GetClipboardData(CF_TEXT);
		// lock and copy
		text = (LPSTR)GlobalLock(clip);
		// unlock 
		GlobalUnlock(clip);
		CloseClipboard();
	}
	return text;
}


std::string GetGameDirectory()
{
	char filename[MAX_PATH];
	if (GetModuleFileNameA(GetModuleHandleA(cheat::main::clientdll.c_str()), filename, MAX_PATH) == 0) {
		return nullptr;
	}

	std::string tmp_path(filename);
	int pos = tmp_path.find("csgo");
	tmp_path = tmp_path.substr(0, pos);
	tmp_path = tmp_path + "csgo\\";
	return tmp_path;
}

void LoadClipBoardText()
{
	std::string text = cheat::Cvars.GetClipBoardText().c_str();
	std::string path = GetGameDirectory() + "clipb.zeus";
	//auto lines = LinesCount(text);

	text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
	//printf("%i\n", lines);
	//printf("%s\n", text.c_str());
	std::ofstream(path) << text;


	cheat::Cvars.Load("clipb.zeus");
	std::remove(path.c_str());
}


void AppendLineToFile(std::string filepath, std::string line)
{
	std::ofstream file;
	//can't enable exception now because of gcc bug that raises ios_base::failure with useless message
	//file.exceptions(file.exceptions() | std::ios::failbit);
	file.open(filepath, std::ios::out | std::ios::app);
	if (file.fail())
		throw std::ios_base::failure(std::strerror(errno));

	//make sure write fails with exception if something is wrong
	file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

	file << line << std::endl;
}

int lineamount()
{
	auto path = GetGameDirectory() + "cfgs.txt";
	int number_of_lines = 0;

	std::string line;
	std::ifstream myfile(path);

	while (std::getline(myfile, line))
		++number_of_lines;

	return number_of_lines;
}

std::string linename(int readline)
{
	auto path = GetGameDirectory() + "cfgs.txt";
	WIN32_FIND_DATAA FindFileData;

	auto hf = FindFirstFileA(path.c_str(), &FindFileData);

	if (hf == INVALID_HANDLE_VALUE)
		std::ofstream(path) << "";

	int number_of_lines = 0;
	std::string line;
	std::ifstream myfile(path);
	for (int i = 0; i < readline; i++)
		std::getline(myfile, line);

	return line.c_str();

}

void AddToConfigs()
{
	//cheat::Cvars.Configs.ResetItems();
	//for (int i = 1; i <= lineamount(); i++)
	//	cheat::Cvars.Configs.AddItem(linename(i), i);
}

void LoadFromConfigs()
{
	auto cfgname = cheat::Cvars.Configs.GetName() + ".zeus";
	cheat::Cvars.Load(cfgname);
}

void SaveToConfig()
{
	cheat::Cvars.Save();
}

void DeleteConfig()
{
	auto cfgname = GetGameDirectory() + cheat::Cvars.Configs.GetName() + ".zeus";

	std::remove(cfgname.c_str());

}

void GetConfigMassive() 
{
	//get all files on folder


	cheat::Cvars.Configs.ResetItems();

	std::string szPath1 = GetGameDirectory() + "*";

	WIN32_FIND_DATAA FindFileData;
	HANDLE hf;

	auto i = 0;

	hf = FindFirstFileA(szPath1.c_str(), &FindFileData);
	if (hf != INVALID_HANDLE_VALUE) {
		do {
			std::string filename = FindFileData.cFileName;

			if (filename == ".")
				continue;

			if (filename == "..")
				continue;

			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (filename.find(".zeus") != std::string::npos)
				{
					//configg.push_back(std::string(filename));
					auto name = std::string(filename);

					name.resize(filename.size() - 5);

					cheat::Cvars.Configs.AddItem(name, i++, Color::White());
				}
			}
		} while (FindNextFileA(hf, &FindFileData) != 0);
		FindClose(hf);
	}
}

void AddToFile()
{
	//auto path = GetGameDirectory() + "cfgs.txt";
	auto cfgname = cheat::Cvars.CreateConfig.GetText() + ".zeus";

//	for (int i = 1; i <= lineamount(); i++)
//	{
//		if (cheat::Cvars.CreateConfig.GetText() == linename(i))
//			return;
//	}
//	AppendLineToFile(path, cheat::Cvars.CreateConfig.GetText());

	WIN32_FIND_DATAA FindFileData;

	auto hf = FindFirstFileA((GetGameDirectory() + cfgname).c_str(), &FindFileData);

	if (hf == INVALID_HANDLE_VALUE)
		std::ofstream(GetGameDirectory() + cfgname) << "";

	//cheat::Cvars.GradientFrom.SetColor(Color(66, 140, 244));
	//cheat::Cvars.GradientTo.SetColor(Color(75, 180, 242));
	//cheat::Cvars.MenuTheme.SetColor(Color(66, 140, 244));

	//cheat::Cvars.EnemyBoxCol.SetColor(Color(0, 129, 255));
	//cheat::Cvars.Visuals_glow_color.SetColor(Color(0, 129, 255));
	//cheat::Cvars.Visuals_SnaplinesColor.SetColor(Color(128, 0, 128));
	//cheat::Cvars.Visuals_skeletonColor.SetColor(Color(105, 105, 105));
	//cheat::Cvars.Visuals_chams_color.SetColor(Color(0, 129, 255));
	//cheat::Cvars.Visuals_chams_hidden_color.SetColor(Color(0, 255, 129));
	//cheat::Cvars.Visuals_chams_history_color.SetColor(Color(0, 255, 129));
	//cheat::Cvars.Visuals_wrld_entities_color.SetColor(Color(0, 255, 129));
	//cheat::Cvars.Misc_AntiUT.SetValue(1.f);
}

void full_update()
{
	if (Source::m_pEngine->IsInGame() && cheat::main::local())
		Source::m_pClientState->m_iDeltaTick = -1;
}

void CMenu::PlaceControls()
{
	// setups
	//cheat::Cvars.LegitAim.SetupGroupbox(10, 10, 200, 150, &legitbot, "Aimbot");
	cheat::Cvars.Visuals.SetupGroupbox(10, 25, 180, 372, &visuals, "other players");
	cheat::Cvars.Visuals_chams.SetupGroupbox(200, 25, 190, 372, &visuals, "models");
	cheat::Cvars.Visuals_localp.SetupGroupbox(400, 25, 180, 372, &visuals, "local player");

	cheat::Cvars.Visuals_o_world.SetupGroupbox(10, 25, 180, 372, &visuals, "world", 1);
	cheat::Cvars.Visuals_o_removals.SetupGroupbox(200, 25, 190, 372, &visuals, "removals", 1);
	cheat::Cvars.Visuals_o_misc.SetupGroupbox(400, 25, 180, 372, &visuals, "misc", 1);

	cheat::Cvars.Lists.SetupGroupbox(10, 25, 200, 196, &Lists, "players");
	cheat::Cvars.LuaList.SetupGroupbox(10, 236, 620, 135, &Lists, "lua loader");
	cheat::Cvars.EntLists.SetupGroupbox(220, 10, 200, 150, &Lists, "entities");
	cheat::Cvars.WhiteLists.SetupGroupbox(430, 10, 200, 196, &Lists, "white listed");

	cheat::Cvars.RageBot.SetupGroupbox(10, 25, 200, 340, &ragebot, "aimbot");
	cheat::Cvars.Ragebot_Accuracy.SetupGroupbox(220, 25, 170, 200, &ragebot, "accuracy");
	cheat::Cvars.Ragebot_BAim.SetupGroupbox(220, 260, 170, 155, &ragebot, "preferences");
	cheat::Cvars.Ragebot_Hitscan.SetupGroupbox(400, 25, 200, 340, &ragebot, "hitscan");

	cheat::Cvars.AntiAim.SetupGroupbox(10, 25, 200, 340, &ragebot, "main", 1);
	cheat::Cvars.AntiAim_standair.SetupGroupbox(220, 25, 170, 195, &ragebot, "stand/air yaw", 1);
	cheat::Cvars.AntiAim_move.SetupGroupbox(220, 255, 170, 100, &ragebot, "move yaw", 1);//AntiAim_shift
	cheat::Cvars.AntiAim_fake.SetupGroupbox(400, 25, 200, 100, &ragebot, "fake yaw", 1);
	//cheat::Cvars.AntiAim_shift.SetupGroupbox(400, 155, 200, 210, &ragebot, "exploit anti-aim", 1);

	cheat::Cvars.Weapons.SetupGroupbox(10, 25, 290, 338, &skins, "weapons");
	cheat::Cvars.Skins.SetupGroupbox(310, 25, 290, 340, &skins, "skins");

	cheat::Cvars.movement.SetupGroupbox(220, 25, 200, 340, &misc, "movement");
	cheat::Cvars.fakelag.SetupGroupbox(10, 25, 200, 340, &misc, "fake-lag");
	cheat::Cvars.restrictions.SetupGroupbox(430, 25, 170, 90, &misc, "restrictions");
	cheat::Cvars.exploits.SetupGroupbox(430, 148, 170, 85, &misc, "exploits");
	cheat::Cvars.menutheme.SetupGroupbox(430, 268, 170, 155, &misc, "menu theme");
	cheat::Cvars.configs.SetupGroupbox(10, 25, 590, 340, &misc, " ", 1);

	cheat::Cvars.fakelag.PlaceControl("enabled", &cheat::Cvars.Misc_FakeLag);
	cheat::Cvars.fakelag.PlaceControl("base factor", &cheat::Cvars.Misc_fakelag_baseFactor);
	cheat::Cvars.Misc_fakelag_baseFactor.AddItems({ "maximum", "adaptive", "jitter", "fluctuate" });
	cheat::Cvars.Misc_fakelag_triggers.AddItems({ "while moving", "in air", "when stopped", "peek", "acceleration" });
	cheat::Cvars.fakelag.PlaceControl("triggers", &cheat::Cvars.Misc_fakelag_triggers);
	cheat::Cvars.Misc_fakelag_variance.Setup(0, 100, "%.2f");
	cheat::Cvars.Misc_fakelag_variance.SetValue(0);
	cheat::Cvars.fakelag.PlaceControl("variance", &cheat::Cvars.Misc_fakelag_variance);
	cheat::Cvars.Misc_fakelag_value.Setup(0, 16, "%.2f");
	cheat::Cvars.Misc_fakelag_value.SetValue(0);
	cheat::Cvars.fakelag.PlaceControl("value", &cheat::Cvars.Misc_fakelag_value);

	cheat::Cvars.movement.PlaceControl("bhop", &cheat::Cvars.Misc_AutoJump);
	cheat::Cvars.movement.PlaceControl("auto strafe", &cheat::Cvars.Misc_AutoStrafe);
	cheat::Cvars.movement.PlaceControl("WASD strafe", &cheat::Cvars.Misc_AutoStrafeWASD);
	cheat::Cvars.movement.PlaceControl("circle strafe key", &cheat::Cvars.Misc_CircleStrafer);
	cheat::Cvars.movement.PlaceControl("anti aim left", &cheat::Cvars.Misc_aaleft);
	cheat::Cvars.movement.PlaceControl("anti aim right", &cheat::Cvars.Misc_aaright);
	cheat::Cvars.movement.PlaceControl("anti aim back", &cheat::Cvars.Misc_aaback);

	cheat::Cvars.restrictions.PlaceControl("anti untrusted", &cheat::Cvars.Misc_AntiUT);
	cheat::Cvars.restrictions.PlaceControl("anti vac error", &cheat::Cvars.Misc_AntiKICK);

	cheat::Cvars.exploits.PlaceControl("fakeduck key", &cheat::Cvars.Exploits_fakeduck);
	cheat::Cvars.exploits.PlaceControl("air desync", &cheat::Cvars.Exploits_air_desync);
	cheat::Cvars.exploits.PlaceControl("exploits", &cheat::Cvars.RageBot_exploits);
	cheat::Cvars.RageBot_exploits.AddItems({ "none", "hide flick", "lc break", "anti-aim" });
	cheat::Cvars.RageBot_exploits.SetValue(0);

	cheat::Cvars.menutheme.PlaceControl("menu main color", &cheat::Cvars.MenuTheme);
	cheat::Cvars.menutheme.PlaceControl("menu gradient color a", &cheat::Cvars.GradientFrom);
	cheat::Cvars.menutheme.PlaceControl("menu gradient color b", &cheat::Cvars.GradientTo);
	//cheat::Cvars.movement.PlaceControl("auto walk", &cheat::Cvars.Misc_WalkBot);
	//cheat::Cvars.MiscGroup.PlaceControl("fakelag", &cheat::Cvars.Misc_FakeLag);
	//cheat::Cvars.MiscGroup.PlaceControl("circle strafe", &cheat::Cvars.Misc_CircleStrafer);
	//cheat::Cvars.MiscGroup.PlaceControl("lag key", &cheat::Cvars.Misc_Lag);
	//cheat::Cvars.Misc_lagval.Setup(0, 1000, "%.0f");
	//cheat::Cvars.MiscGroup.PlaceControl("lag value", &cheat::Cvars.Misc_lagval);
	//cheat::Cvars.MiscGroup.PlaceControl("clear keypad codes", &cheat::Cvars.ClearKeypadCodesbtn);
	//cheat::Cvars.MiscGroup.PlaceControl("log keypads", &cheat::Cvars.Misc_logkeypads);
	//cheat::Cvars.MiscGroup.PlaceControl("name stealer", &cheat::Cvars.Misc_NameSteal);
	//cheat::Cvars.Misc_NameSteal.AddItems({ "DarkRP", "Steam" });
	//cheat::Cvars.MiscGroup.PlaceControl("kill say", &cheat::Cvars.Misc_KillSay);
	//cheat::Cvars.Misc_KillSay.AddItems({ "off", "OOC", "Normal" });
	//cheat::Cvars.ClearKeypadCodesbtn.SetSize(190, 12);

	//cheat::Cvars.Visuals_Filter.PlaceControl("enemy only", &cheat::Cvars.Visuals_Enemyonly);
	//cheat::Cvars.Visuals_Filter.PlaceControl("weapons", &cheat::Cvars.Visuals_DroppedWeapons);
	//cheat::Cvars.Visuals_Filter.PlaceControl("players", &cheat::Cvars.Visuals_Players);
	//cheat::Cvars.Visuals_Filter.PlaceControl("whitelisted", &cheat::Cvars.Visuals_Whitelistents);
	//cheat::Cvars.Visuals_Filter.PlaceControl("friends", &cheat::Cvars.Visuals_Friends);
	//cheat::Cvars.Visuals_Filter.PlaceControl("local", &cheat::Cvars.Visuals_Local);

	for (int i = 0; i < 3; ++i)
	{
		cheat::Cvars.Legit_Aimbot.SetupGroupbox(220, 50, 200, 150, &legitbot, "aimbot", i);
		cheat::Cvars.Legit_Aimbot.PlaceControl("enable", &cheat::Cvars.Legit_Enable, i);
		cheat::Cvars.Legit_Aimbot.PlaceControl("key", &cheat::Cvars.Legit_Key, i);
		cheat::Cvars.Legit_Aimbot.PlaceControl("friendly fire", &cheat::Cvars.Legit_ff, i);
		cheat::Cvars.Legit_Aimbot.PlaceControl("position adjs", &cheat::Cvars.Legit_PosAjs, i);
		cheat::Cvars.Legit_Accuracy.SetupGroupbox(10, 50, 200, 150, &legitbot, "accuracy", i);
		cheat::Cvars.Legit_Filter.SetupGroupbox(430, 50, 150, 150, &legitbot, "hit boxes", i);
		cheat::Cvars.Legit_fov[i].Setup(0, 35);
		cheat::Cvars.Legit_smooth[i].Setup(0, 15);
		//cheat::Cvars.Legit_hbotpriority[i].AddItems({ "head", "chest", "pelvis" });
		//cheat::Cvars.Legit_hboxselection[i].AddItems({ "dynamic", "priority" });
		cheat::Cvars.Legit_Accuracy.PlaceControl("fov", &cheat::Cvars.Legit_fov[i], i);
		cheat::Cvars.Legit_Accuracy.PlaceControl("smooth", &cheat::Cvars.Legit_smooth[i], i);
		//cheat::Cvars.Legit_Accuracy.PlaceControl("priority hitbox", &cheat::Cvars.Legit_hbotpriority[i], i);
		//cheat::Cvars.Legit_Accuracy.PlaceControl("hitbox selection", &cheat::Cvars.Legit_hboxselection[i], i);
		cheat::Cvars.Legit_Accuracy.PlaceControl("recoil control", &cheat::Cvars.Legit_rcs[i], i);

		cheat::Cvars.Legit_Filter.PlaceControl("head", &cheat::Cvars.Legit_head[i], i);
		cheat::Cvars.Legit_Filter.PlaceControl("chest", &cheat::Cvars.Legit_chest[i], i);
		cheat::Cvars.Legit_Filter.PlaceControl("stomach", &cheat::Cvars.Legit_stomach[i], i);
		cheat::Cvars.Legit_Filter.PlaceControl("legs", &cheat::Cvars.Legit_legs[i], i);
		cheat::Cvars.Legit_Filter.PlaceControl("arms", &cheat::Cvars.Legit_arms[i], i);
	}
	cheat::Cvars.Legit_Accuracy.SetSize(200, 150);


	cheat::Cvars.RageBot_MaximumFov.Setup(0, 180, "%.2f");
	cheat::Cvars.RageBot_MaximumFov.SetValue(180);
	cheat::Cvars.RageBot_MinDamage.Setup(0, 100, "%.2f");
	cheat::Cvars.RageBot_MinDamage.SetValue(15);

	cheat::Cvars.RageBot.PlaceControl("enabled", &cheat::Cvars.RageBot_Enable);
	cheat::Cvars.RageBot.PlaceControl("auto fire", &cheat::Cvars.RageBot_AutoFire);
	cheat::Cvars.RageBot.PlaceControl("aim key", &cheat::Cvars.RageBot_Key);
	cheat::Cvars.RageBot.PlaceControl("silent aim", &cheat::Cvars.RageBot_SilentAim);
	cheat::Cvars.RageBot.PlaceControl("control recoil", &cheat::Cvars.RageBot_NoRecoil);
	cheat::Cvars.RageBot.PlaceControl("control spread", &cheat::Cvars.RageBot_NoSpread);
	cheat::Cvars.RageBot.PlaceControl("auto revolver", &cheat::Cvars.RageBot_autor8);
	//cheat::Cvars.RageBot.PlaceControl("friendly Fire", &cheat::Cvars.RageBot_FriendlyFire);
	//cheat::Cvars.RageBot.PlaceControl("hitscan", &cheat::Cvars.RageBot_HitScan);
	//cheat::Cvars.RageBot_HitScan.AddItems({ "off", "low", "medium", "high" });

	cheat::Cvars.RageBot.PlaceControl("target selection", &cheat::Cvars.RageBot_TargetSelection);
	//cheat::Cvars.RageBot_OnKey.AddItems({ "always", "on key" });
	//cheat::Cvars.RageBot.PlaceControl("shoot", &cheat::Cvars.RageBot_OnKey);
	cheat::Cvars.RageBot.PlaceControl("maximum fov", &cheat::Cvars.RageBot_MaximumFov);
	cheat::Cvars.RageBot.PlaceControl("min damage", &cheat::Cvars.RageBot_MinDamage);
	cheat::Cvars.RageBot.PlaceControl("scale damage on hp", &cheat::Cvars.RageBot_ScaledmgOnHp);
	cheat::Cvars.RageBot_SilentAim.AddItems({ "none", "client side", "anti-aim" });

#pragma region accuracy
	cheat::Cvars.Ragebot_Accuracy.PlaceControl("auto scope", &cheat::Cvars.RageBot_AutoScope);
	//cheat::Cvars.RageBot_Resolver.AddItems({ "none", "default", "experimental" });
	cheat::Cvars.Ragebot_Accuracy.PlaceControl("resolver", &cheat::Cvars.RageBot_Resolver);
	cheat::Cvars.Ragebot_Accuracy.PlaceControl("adjust positions", &cheat::Cvars.RageBot_AdjustPositions);
	cheat::Cvars.Ragebot_Accuracy.PlaceControl("fix lag", &cheat::Cvars.RageBot_FixLag);
	//cheat::Cvars.Ragebot_Accuracy.PlaceControl("optimal shot", &cheat::Cvars.RageBot_OptimalShot);
	cheat::Cvars.RageBot_OptimalShot.AddItems({ "shot", "lethal", "backtrack", "low delta"});

	cheat::Cvars.RageBot_HitChance.Setup(0, 100, "%.2f");
	cheat::Cvars.RageBot_HitChance.SetValue(45);
	cheat::Cvars.Ragebot_Accuracy.PlaceControl("hit chance", &cheat::Cvars.RageBot_HitChance);
	cheat::Cvars.Ragebot_Accuracy.PlaceControl("autostop", &cheat::Cvars.RageBot_AutoStop);
	cheat::Cvars.RageBot_AutoStop.AddItems({ "none", "on lethal", "hitchance fail", "always" });
	cheat::Cvars.Ragebot_Accuracy.PlaceControl("stop method", &cheat::Cvars.RageBot_AutoStopMethod);
	cheat::Cvars.RageBot_AutoStopMethod.AddItems({ "min walk", "walk + crouch", "full stop" });
#pragma endregion

#pragma region baim
	//cheat::Cvars.Ragebot_BAim.PlaceControl("prefer baim", &cheat::Cvars.RageBot_Baim_PreferBaim);
	//cheat::Cvars.RageBot_Baim_PreferBaim.AddItems({ "none", "adaptive", "lethal" });
	//cheat::Cvars.Ragebot_BAim.PlaceControl("when fake", &cheat::Cvars.RageBot_Baim_fake);
	//cheat::Cvars.Ragebot_BAim.PlaceControl("when inair", &cheat::Cvars.RageBot_Baim_air);
	//cheat::Cvars.Ragebot_BAim.PlaceControl("when lethal", &cheat::Cvars.RageBot_Baim_accurate);
	cheat::Cvars.RageBot_bodyaim.AddItems({ "backwards", "lethal", "in air" });
	cheat::Cvars.Ragebot_BAim.PlaceControl("prefer body if", &cheat::Cvars.RageBot_bodyaim);

	cheat::Cvars.RageBot_Baim_AfterXshots.Setup(0, 10, "%.f");
	cheat::Cvars.RageBot_Baim_AfterXshots.SetValue(0);
	cheat::Cvars.Ragebot_BAim.PlaceControl("head shots", &cheat::Cvars.RageBot_Baim_AfterXshots);

	cheat::Cvars.RageBot_allow_head.AddItems({ "sideways", "lby update", "moving" });
	cheat::Cvars.Ragebot_BAim.PlaceControl("force head if", &cheat::Cvars.RageBot_allow_head);

	cheat::Cvars.Ragebot_BAim.PlaceControl("headaim only on lby", &cheat::Cvars.Ragebot_headaim_only_on_shot);
#pragma endregion

#pragma region hitscan

	cheat::Cvars.RageBot_PointScale_Head.Setup(0, 100, "%.2f");
	cheat::Cvars.RageBot_PointScale_Head.SetValue(0);
	cheat::Cvars.Ragebot_Hitscan.PlaceControl("head point scale", &cheat::Cvars.RageBot_PointScale_Head);

	cheat::Cvars.RageBot_PointScale_Body.Setup(0, 100, "%.2f");
	cheat::Cvars.RageBot_PointScale_Body.SetValue(0);
	cheat::Cvars.Ragebot_Hitscan.PlaceControl("body point scale", &cheat::Cvars.RageBot_PointScale_Body);

	//cheat::Cvars.RageBot_PointScale_Foot.Setup(0, 100, "%.2f");
	//cheat::Cvars.RageBot_PointScale_Foot.SetValue(0);
	//cheat::Cvars.Ragebot_Hitscan.PlaceControl("foot point scale", &cheat::Cvars.RageBot_PointScale_Foot);

	cheat::Cvars.RageBot_Hitboxes.AddItems({ "head", "body", "arms", "legs", "feet" });
	cheat::Cvars.Ragebot_Hitscan.PlaceControl("hitboxes", &cheat::Cvars.RageBot_Hitboxes);

	cheat::Cvars.RageBot_MultipointHBoxes.AddItems({ "head", "body", "arms", "legs", "feet" });
	cheat::Cvars.Ragebot_Hitscan.PlaceControl("multipoints", &cheat::Cvars.RageBot_MultipointHBoxes);
#pragma endregion

#pragma region antiaim
	cheat::Cvars.AntiAim.PlaceControl("enabled", &cheat::Cvars.Antiaim_enable,1);
	cheat::Cvars.AntiAim.PlaceControl("at targets", &cheat::Cvars.Antiaim_attargets, 1);
	cheat::Cvars.AntiAim.PlaceControl("pitch", &cheat::Cvars.anti_aim_pitch, 1);
	cheat::Cvars.anti_aim_pitch.AddItems({ "none", "down", "zero" });
	//cheat::Cvars.AntiAim.PlaceControl("freestanding", &cheat::Cvars.anti_aim_freestand, 1);
	cheat::Cvars.AntiAim.PlaceControl("slow motion key", &cheat::Cvars.anti_aim_slowwalk_key, 1);
	//cheat::Cvars.AntiAim.PlaceControl("maintain spread", &cheat::Cvars.anti_aim_slow_walk_accurate, 1);
	cheat::Cvars.anti_aim_slowwalk_speed.Setup(0, 17, "%.0f");
	cheat::Cvars.anti_aim_slowwalk_speed.SetValue(14);
	//cheat::Cvars.AntiAim.PlaceControl("slow motion speed", &cheat::Cvars.anti_aim_slowwalk_speed, 1);
#pragma endregion

#pragma region standyaw
	cheat::Cvars.AntiAim_standair.PlaceControl("yaw", &cheat::Cvars.anti_aim_s_yaw, 1);
	cheat::Cvars.anti_aim_s_yaw.AddItems({ "none", "backward", "zero", "crooked", "spin" });
	cheat::Cvars.anti_aim_s_addyaw.Setup(-180, 180, "%.2f");
	cheat::Cvars.anti_aim_s_addyaw.SetValue(0);
	cheat::Cvars.AntiAim_standair.PlaceControl("add angle", &cheat::Cvars.anti_aim_s_addyaw, 1);

	cheat::Cvars.AntiAim_standair.PlaceControl("switch mode", &cheat::Cvars.anti_aim_s_switchmode, 1);
	cheat::Cvars.anti_aim_s_switchmode.AddItems({ "none", "jitter", "spin" });

	cheat::Cvars.anti_aim_s_switchspeed.Setup(0, 100, "%.1f");
	cheat::Cvars.anti_aim_s_switchspeed.SetValue(0);
	cheat::Cvars.AntiAim_standair.PlaceControl("switch speed", &cheat::Cvars.anti_aim_s_switchspeed, 1);

	cheat::Cvars.anti_aim_s_switchangle.Setup(-180, 180, "%.1f");
	cheat::Cvars.anti_aim_s_switchangle.SetValue(0);
	cheat::Cvars.AntiAim_standair.PlaceControl("switch angle", &cheat::Cvars.anti_aim_s_switchangle, 1);
#pragma endregion

#pragma region moveyaw
	cheat::Cvars.AntiAim_move.PlaceControl("yaw", &cheat::Cvars.anti_aim_m_yaw, 1);
	cheat::Cvars.anti_aim_m_yaw.AddItems({ "none", "backward", "zero", "crooked", "spin" });
	cheat::Cvars.anti_aim_m_addyaw.Setup(-180, 180, "%.2f");
	cheat::Cvars.anti_aim_m_addyaw.SetValue(0);
	cheat::Cvars.AntiAim_move.PlaceControl("add angle", &cheat::Cvars.anti_aim_m_addyaw, 1);

	cheat::Cvars.AntiAim_move.PlaceControl("switch mode", &cheat::Cvars.anti_aim_m_switchmode, 1);
	cheat::Cvars.anti_aim_m_switchmode.AddItems({ "none", "jitter", "spin" });

	cheat::Cvars.anti_aim_m_switchspeed.Setup(0, 100, "%.1f");
	cheat::Cvars.anti_aim_m_switchspeed.SetValue(0);
	cheat::Cvars.AntiAim_move.PlaceControl("switch speed", &cheat::Cvars.anti_aim_m_switchspeed, 1);

	cheat::Cvars.anti_aim_m_switchangle.Setup(-180, 180, "%.1f");
	cheat::Cvars.anti_aim_m_switchangle.SetValue(0);
	cheat::Cvars.AntiAim_move.PlaceControl("switch angle", &cheat::Cvars.anti_aim_m_switchangle, 1);
#pragma endregion

#pragma region fakeyaw
	cheat::Cvars.AntiAim_fake.PlaceControl("body yaw", &cheat::Cvars.anti_aim_desync, 1);
	cheat::Cvars.anti_aim_desync.AddItems({ "none", "static", "twist" });
	//cheat::Cvars.AntiAim_fake.PlaceControl("flick meme", &cheat::Cvars.Antiaim_flick_exploit, 1);
	cheat::Cvars.anti_aim_desync_range.Setup(0, 80, "%.0f");
	cheat::Cvars.anti_aim_desync_range.SetValue(16);
	cheat::Cvars.AntiAim_fake.PlaceControl("body yaw delta", &cheat::Cvars.anti_aim_desync_range, 1);
	//cheat::Cvars.AntiAim_fake.PlaceControl("fake side switch", &cheat::Cvars.anti_aim_desync_side_switch, 1);
#pragma endregion

#pragma region exploityaw
	
	/*cheat::Cvars.AntiAim_shift.PlaceControl("pitch", &cheat::Cvars.anti_aim_ex_pitch, 1);
	cheat::Cvars.anti_aim_ex_pitch.AddItems({ "none", "up", "down", "zero" });

	cheat::Cvars.AntiAim_shift.PlaceControl("yaw", &cheat::Cvars.anti_aim_ex_yaw, 1);
	cheat::Cvars.anti_aim_ex_yaw.AddItems({ "none", "backward", "inverse", "crooked", "spin" });
	cheat::Cvars.anti_aim_ex_addyaw.Setup(-180, 180, "%.2f");
	cheat::Cvars.anti_aim_ex_addyaw.SetValue(0);
	cheat::Cvars.AntiAim_shift.PlaceControl("add angle", &cheat::Cvars.anti_aim_ex_addyaw, 1);*/

#pragma endregion

	cheat::Cvars.RageBot_TargetSelection.AddItems({ "health", "distance" , "crosshair"});

	cheat::Cvars.Visuals.PlaceControl("enable", &cheat::Cvars.Visuals_Enable);
	cheat::Cvars.Visuals.PlaceControl("teammates", &cheat::Cvars.Visuals_teammates);
	cheat::Cvars.Visuals.PlaceControl("bounding box", &cheat::Cvars.Visuals_Box);
	cheat::Cvars.Visuals_Box.AddItems({ "none", "static", "dynamic" });
	cheat::Cvars.Visuals.PlaceControl("bounding box color", &cheat::Cvars.EnemyBoxCol);
	cheat::Cvars.Visuals_Skeleton.AddItems({ "none", "basic", "history" });
	cheat::Cvars.Visuals.PlaceControl("skeletons", &cheat::Cvars.Visuals_Skeleton);
	cheat::Cvars.Visuals.PlaceControl("skeletons color", &cheat::Cvars.Visuals_skeletonColor);
	cheat::Cvars.Visuals.PlaceControl("names", &cheat::Cvars.Visuals_Name);
	cheat::Cvars.Visuals.PlaceControl("health bar", &cheat::Cvars.Visuals_Health);
	//cheat::Cvars.Visuals.PlaceControl("weapons", &cheat::Cvars.Visuals_Weapon);
	cheat::Cvars.Visuals_Weapon.AddItems({ "primary", "secondary", "nades", "taser", "bomb" });
	cheat::Cvars.Visuals.PlaceControl("weapons", &cheat::Cvars.Visuals_Weapon);
	cheat::Cvars.Visuals_enemies_flags.AddItems({ "armor", "vulnerable", "fake", "ping", "scope" });
	cheat::Cvars.Visuals.PlaceControl("flags", &cheat::Cvars.Visuals_enemies_flags);
	cheat::Cvars.Visuals.PlaceControl("ammo count", &cheat::Cvars.Visuals_ammo_bar);
	cheat::Cvars.Visuals.PlaceControl("snaplines", &cheat::Cvars.Visuals_Snaplines);
	cheat::Cvars.Visuals.PlaceControl("snaplines color", &cheat::Cvars.Visuals_SnaplinesColor);
	cheat::Cvars.Visuals.PlaceControl("fov arrows", &cheat::Cvars.Visuals_fov_arrows);
	cheat::Cvars.Visuals_glow.AddItems({ "off", "static", "pulse", "outline", "outline pulse" });
	cheat::Cvars.Visuals.PlaceControl("glow", &cheat::Cvars.Visuals_glow);
	cheat::Cvars.Visuals.PlaceControl("glow color", &cheat::Cvars.Visuals_glow_color); //Ragebot_headaim_only_on_shot
	//cheat::Cvars.Visuals_Skeleton.AddItems({ "skeleton", "history" });
	//cheat::Cvars.Visuals.PlaceControl("skeleton", &cheat::Cvars.Visuals_Skeleton);
	//cheat::Cvars.Visuals.PlaceControl("aim points", &cheat::Cvars.Visuals_AimPoint);
	////cheat::Cvars.Visuals_Boxtype.AddItems({ "full", "edge" });
	//cheat::Cvars.Visuals_Chams.AddItems({ "off", "material", "flat" });
	//cheat::Cvars.Visuals.PlaceControl("chams", &cheat::Cvars.Visuals_Chams);
	//cheat::Cvars.Visuals_Health.AddItems({ "healthbar", "text" });
	//cheat::Cvars.Visuals_HitBoxes.AddItems({ "off", "white", "color" });
	//cheat::Cvars.Visuals_Weapon.AddItems({ "off", "active", "all" });
	//cheat::Cvars.Visuals.PlaceControl("health", &cheat::Cvars.Visuals_Health);
	//cheat::Cvars.Visuals.PlaceControl("armor", &cheat::Cvars.Visuals_Armor);
	//cheat::Cvars.Visuals.PlaceControl("weapon", &cheat::Cvars.Visuals_Weapon);
	//cheat::Cvars.Visuals.PlaceControl("hit boxes", &cheat::Cvars.Visuals_HitBoxes);
	//cheat::Cvars.Visuals_ChamsHistory.AddItems({ "off", "last", "all" });
	//cheat::Cvars.Visuals.PlaceControl("chams history", &cheat::Cvars.Visuals_ChamsHistory);
	//cheat::Cvars.Visuals.PlaceControl("team colors", &cheat::Cvars.Visuals_UseTeamCol);
	//cheat::Cvars.Visuals.PlaceControl("show admins", &cheat::Cvars.Visuals_showAdm);

	cheat::Cvars.Visuals_chams.PlaceControl("teammates", &cheat::Cvars.Visuals_chams_teammates);
	cheat::Cvars.Visuals_chams.PlaceControl("player chams", &cheat::Cvars.Visuals_chams_type);
	cheat::Cvars.Visuals_chams_type.AddItems({ "none", "basic", "basic + history" });
	cheat::Cvars.Visuals_chams.PlaceControl("player color", &cheat::Cvars.Visuals_chams_color);
	cheat::Cvars.Visuals_chams.PlaceControl("while hidden", &cheat::Cvars.Visuals_chams_hidden);
	cheat::Cvars.Visuals_chams.PlaceControl("player (hidden) color", &cheat::Cvars.Visuals_chams_hidden_color);
	cheat::Cvars.Visuals_chams.PlaceControl("player history color", &cheat::Cvars.Visuals_chams_history_color);

	cheat::Cvars.Visuals_localp.PlaceControl("force tp", &cheat::Cvars.Visuals_lp_forcetp);
	cheat::Cvars.Visuals_localp.PlaceControl("force fp (grenade)", &cheat::Cvars.Visuals_lp_forcetpnade);
	cheat::Cvars.Visuals_lp_tpdist.Setup(0, 1000, "%.0f");
	cheat::Cvars.Visuals_lp_tpdist.SetValue(150);
	cheat::Cvars.Visuals_localp.PlaceControl("tp distance", &cheat::Cvars.Visuals_lp_tpdist);
	cheat::Cvars.Visuals_localp.PlaceControl("tp toggle", &cheat::Cvars.Visuals_lp_toggletp);

	cheat::Cvars.Visuals_lglow.AddItems({ "off", "static", "pulse", "outline", "outline pulse" });
	cheat::Cvars.Visuals_localp.PlaceControl("glow fake", &cheat::Cvars.Visuals_lglow);
	cheat::Cvars.Visuals_localp.PlaceControl("glow color", &cheat::Cvars.Visuals_lglow_color);
	cheat::Cvars.Visuals_localp.PlaceControl("local chams", &cheat::Cvars.Visuals_lchams_enabled);
	cheat::Cvars.Visuals_localp.PlaceControl("player color", &cheat::Cvars.Visuals_lchams_color);

	cheat::Cvars.Visuals_o_world.PlaceControl("disable post process", &cheat::Cvars.Visuals_wrld_postprocess, 1);
	cheat::Cvars.Visuals_world_sky.AddItems({ "none", "tibet", "baggage", "monastery" , "italy/oldinferno" , "aztec" , "vertigo" , "daylight" , "daylight (2)" , "clouds" ,
		"clouds (2)" , "gray" , "clear" , "canals" , "cobblestone"  , "cssault"  , "clouds (dark)"  , "night"  , "night (2)"  , "night (flat)" , "dusty"
		, "rainy" , "c: sunrise" , "c: galaxy" , "c: galaxy (2)" , "c: galaxy (3)", "c: clouds (night)", "c: ocean"
		, "c: grimm night" , "c: heaven" , "c: heaven (2)" , "c: clouds" , "c: night (blue)" });
	cheat::Cvars.Visuals_o_world.PlaceControl("skybox", &cheat::Cvars.Visuals_world_sky, 1);
	cheat::Cvars.Visuals_o_world.PlaceControl("night mode", &cheat::Cvars.Visuals_wrld_nightmode, 1);
	cheat::Cvars.Visuals_wrld_prop_alpha.Setup(0, 100, "%.0f");
	cheat::Cvars.Visuals_o_world.PlaceControl("prop alpha", &cheat::Cvars.Visuals_wrld_prop_alpha, 1);
	cheat::Cvars.Visuals_wrld_prop_alpha.SetValue(100.f);
	cheat::Cvars.Visuals_wrld_entities.AddItems({ "weapon", "bomb", "nade" });
	cheat::Cvars.Visuals_o_world.PlaceControl("other entities", &cheat::Cvars.Visuals_wrld_entities,1);

	cheat::Cvars.Visuals_o_world.PlaceControl("other entities color", &cheat::Cvars.Visuals_wrld_entities_color,1);

	cheat::Cvars.Visuals_o_world.PlaceControl("show impacts", &cheat::Cvars.Visuals_wrld_impact, 1);
	cheat::Cvars.Visuals_wrld_impact_duration.Setup(0, 10, "%.2f");
	cheat::Cvars.Visuals_wrld_impact_duration.SetValue(4);
	cheat::Cvars.Visuals_o_world.PlaceControl("impact duration", &cheat::Cvars.Visuals_wrld_impact_duration, 1);

	cheat::Cvars.Visuals_o_world.PlaceControl("bullet tracers", &cheat::Cvars.Visuals_Bullet_Tracers,1);
	cheat::Cvars.Visuals_Bullet_Tracers.AddItems({ "enemies", "local", "teammates" });
	cheat::Cvars.Visuals_o_world.PlaceControl("local tracer color", &cheat::Cvars.Visuals_tracer_local_color, 1);
	cheat::Cvars.Visuals_o_world.PlaceControl("enemy tracer color", &cheat::Cvars.Visuals_tracer_enemy_color, 1);
	cheat::Cvars.Visuals_o_world.PlaceControl("teammate tracer color", &cheat::Cvars.Visuals_tracer_teammate_color, 1);

	//cheat::Cvars.Visuals_rem_smoke.AddItems({ "none", "on", "wired" });
	cheat::Cvars.Visuals_o_removals.PlaceControl("remove smoke", &cheat::Cvars.Visuals_rem_smoke, 1);
	cheat::Cvars.Visuals_o_removals.PlaceControl("remove flash", &cheat::Cvars.Visuals_rem_flash, 1);
	cheat::Cvars.Visuals_o_removals.PlaceControl("remove scope", &cheat::Cvars.Visuals_rem_scope, 1);
	cheat::Cvars.Visuals_o_removals.PlaceControl("remove punch", &cheat::Cvars.Visuals_rem_punch, 1);

	cheat::Cvars.Visuals_misc_crosshair.AddItems({ "none", "basic", "autowall", "wall damage"});
	cheat::Cvars.Visuals_o_misc.PlaceControl("crosshair", &cheat::Cvars.Visuals_misc_crosshair, 1);
	cheat::Cvars.Visuals_o_misc.PlaceControl("hold killfeed", &cheat::Cvars.Visuals_misc_preserve_kills, 1);
	cheat::Cvars.Visuals_o_misc.PlaceControl("show spectator list", &cheat::Cvars.Visuals_spectators, 1);
	cheat::Cvars.Visuals_o_misc.PlaceControl("unlock inventory", &cheat::Cvars.Visuals_unlock_invertory, 1);
	cheat::Cvars.Visuals_spectators_alpha.Setup(0, 100, "%.f%%");
	cheat::Cvars.Visuals_spectators_alpha.SetValue(0);
	cheat::Cvars.Visuals_o_misc.PlaceControl("list alpha", &cheat::Cvars.Visuals_spectators_alpha, 1);
	cheat::Cvars.Visuals_o_misc.PlaceControl("hit marker", &cheat::Cvars.Visuals_misc_hitmarker, 1);
	cheat::Cvars.Visuals_misc_wpn_spread.AddItems({ "none", "circle", "rect" });
	cheat::Cvars.Visuals_o_misc.PlaceControl("show spread", &cheat::Cvars.Visuals_misc_wpn_spread, 1);
	cheat::Cvars.Visuals_o_misc.PlaceControl("spread color", &cheat::Cvars.Visuals_spread_crosshair_color, 1);
	cheat::Cvars.Visuals_o_misc.PlaceControl("nade tracer", &cheat::Cvars.Visuals_misc_nade_tracer, 1);
	cheat::Cvars.Visuals_o_misc.PlaceControl("clantag", &cheat::Cvars.Visuals_misc_tag, 1);
	cheat::Cvars.Visuals_o_misc.PlaceControl("manual indicator", &cheat::Cvars.Visuals_misc_manual_indicator, 1);
	cheat::Cvars.Visuals_o_misc.PlaceControl("fakelag indicator", &cheat::Cvars.Visuals_misc_flag_indicator, 1);
	cheat::Cvars.Visuals_o_misc.PlaceControl("lby indicator", &cheat::Cvars.Visuals_misc_desync_indicator, 1);
	cheat::Cvars.Visuals_o_misc.PlaceControl("event log", &cheat::Cvars.Visuals_misc_event_log, 1);

	cheat::Cvars.Visuals_misc_fov.Setup(-90, 90, "%.2f");
	cheat::Cvars.Visuals_misc_fov.SetValue(0);
	cheat::Cvars.Visuals_o_misc.PlaceControl("viewmodel fov", &cheat::Cvars.Visuals_misc_fov, 1);

	cheat::Cvars.Visuals_misc_screen_aspr.Setup(0, 5, "%.4f");
	cheat::Cvars.Visuals_misc_screen_aspr.SetValue(0);
	cheat::Cvars.Visuals_o_misc.PlaceControl("aspect ratio", &cheat::Cvars.Visuals_misc_screen_aspr, 1);

	cheat::Cvars.Visuals_misc_off_x.Setup(-10, 10, "%.2f");
	cheat::Cvars.Visuals_misc_off_x.SetValue(0);
	cheat::Cvars.Visuals_o_misc.PlaceControl("viewmodel x", &cheat::Cvars.Visuals_misc_off_x, 1);

	cheat::Cvars.Visuals_misc_off_y.Setup(-10, 10, "%.2f");
	cheat::Cvars.Visuals_misc_off_y.SetValue(0);
	cheat::Cvars.Visuals_o_misc.PlaceControl("viewmodel y", &cheat::Cvars.Visuals_misc_off_y, 1);

	cheat::Cvars.Visuals_misc_off_z.Setup(-10, 10, "%.2f");
	cheat::Cvars.Visuals_misc_off_z.SetValue(0);
	cheat::Cvars.Visuals_o_misc.PlaceControl("viewmodel z", &cheat::Cvars.Visuals_misc_off_z, 1);

	//cheat::Cvars.Colors.PlaceControl("teammate visible", &cheat::Cvars.Player_Visible);
	//cheat::Cvars.Colors.PlaceControl("teammate invisible", &cheat::Cvars.Player_Invisible);
	//cheat::Cvars.Colors.PlaceControl("enemy visible", &cheat::Cvars.EnemyBoxCol);
	//cheat::Cvars.Colors.PlaceControl("enemy invisible", &cheat::Cvars.Enemy_Invisible);
	//cheat::Cvars.Colors.PlaceControl("friendly", &cheat::Cvars.Friendly);
	//cheat::Cvars.Colors.PlaceControl("entities", &cheat::Cvars.Weapons_Dropped);
	//cheat::Cvars.Colors.PlaceControl("weapon chams", &cheat::Cvars.Weapons_Chams);
	//cheat::Cvars.Colors.PlaceControl("arms chams", &cheat::Cvars.arms_chams);
	//cheat::Cvars.Chams_HColor.SetColor(Color(255, 255, 255));
	//cheat::Cvars.Colors.PlaceControl("chams history", &cheat::Cvars.Chams_HColor);
	//cheat::Cvars.Colors.PlaceControl("hit markers", &cheat::Cvars.HitMarkerCol);

	cheat::Cvars.Skins.PlaceControl("search", &cheat::Cvars.Skins_Search);
	cheat::Cvars.Skins_Paintkits.SetSize(0, 300);
	cheat::Cvars.Skins.PlaceControl("paint kits", &cheat::Cvars.Skins_Paintkits);
	cheat::Cvars.Skins_Weapons.SetSize(0, 260);
	cheat::Cvars.Weapons.PlaceControl("weapons", &cheat::Cvars.Skins_Weapons);
	cheat::Cvars.Weapons.PlaceControl("knife", &cheat::Cvars.Skins_Knife);

	cheat::Cvars.Weapons.PlaceControl("update weapons", &cheat::Cvars.UpdateButton);

	// auto size window to colrs
	cheat::Cvars.Visuals_FOV.Setup(0, 150);
	cheat::Cvars.Visuals_FOV.SetValue(120);
	cheat::Cvars.Visuals_Radar_Zoom.Setup(0, 150);
	cheat::Cvars.Visuals_Radar_Size.Setup(0, 1000, "%.0f");
	cheat::Cvars.Visuals_AutoSpectate_Distance.Setup(0, 1000);
	cheat::Cvars.Visuals_AutoSpectate_Distance.SetValue(500);
	cheat::Cvars.Visuals_Radar_Size.SetValue(300);
	cheat::Cvars.Visuals_NightMode.Setup(0, 100);
	cheat::Cvars.Visuals_NightMode.SetValue(100);

	/*cheat::Cvars.Visuals_Misc.PlaceControl("fov", &cheat::Cvars.Visuals_FOV);
	cheat::Cvars.Visuals_Misc.PlaceControl("night mode", &cheat::Cvars.Visuals_NightMode);
	cheat::Cvars.Visuals_Misc.PlaceControl("no visual recoil", &cheat::Cvars.Visuals_NoVisRecoil);
	cheat::Cvars.Visuals_Misc.PlaceControl("hand chams", &cheat::Cvars.Visuals_HandChams);
	cheat::Cvars.Visuals_Misc.PlaceControl("weapon chams", &cheat::Cvars.Visuals_WeaponChams);
	cheat::Cvars.Visuals_Misc.PlaceControl("radar", &cheat::Cvars.Visuals_Radar);
	cheat::Cvars.Visuals_Misc.PlaceControl("radar size", &cheat::Cvars.Visuals_Radar_Size);
	cheat::Cvars.Visuals_Misc.PlaceControl("radar distance", &cheat::Cvars.Visuals_Radar_Zoom);
	cheat::Cvars.Visuals_Misc.PlaceControl("auto spectate", &cheat::Cvars.Visuals_AutoSpectate);
	cheat::Cvars.Visuals_Misc.PlaceControl("auto spectate dist", &cheat::Cvars.Visuals_AutoSpectate_Distance);
	cheat::Cvars.Visuals_Misc.PlaceControl("spectator list", &cheat::Cvars.Visuals_SpecList);
	cheat::Cvars.Visuals_Misc.PlaceControl("enemy finder", &cheat::Cvars.Visuals_BadGuyList);
	cheat::Cvars.Visuals_Misc.PlaceControl("hit marker", &cheat::Cvars.Visuals_HitMarker);
	cheat::Cvars.Visuals_Misc.PlaceControl("anti screengrab", &cheat::Cvars.Visuals_Antiscreengrab);*/

	cheat::Cvars.Visuals_Antiscreengrab.SetDesk("Multi-core rendering will be forced off");
	cheat::Cvars.Visuals_Antiscreengrab.SetValue(1.0f);
	cheat::Cvars.Visuals_HitMarker.AddItems({ "off", "on", "on + sound" });
	cheat::Cvars.Visuals_SpecList.AddItems({ "off", "me", "all" });
	cheat::Cvars.Visuals_BadGuyList.AddItems({ "off", "traitor", "murder" });
	cheat::Cvars.Visuals_HandChams.AddItems({ "off", "material", "flat" });
	cheat::Cvars.Visuals_WeaponChams.AddItems({ "off", "material", "flat" });

	cheat::Cvars.PlayerList.Setup(0, 80);
	cheat::Cvars.Lists.PlaceControl("search", &cheat::Cvars.SearchBlack);
	cheat::Cvars.Lists.PlaceControl("players", &cheat::Cvars.PlayerList);
	cheat::Cvars.Lists.PlaceControl("friendly", &cheat::Cvars.Friendly_Player);
	cheat::Cvars.Lists.PlaceControl("spectate", &cheat::Cvars.Spectate_Player);
	//lua
	cheat::Cvars.Lua_List.Setup(0, 100);
	cheat::Cvars.LuaList.PlaceControl("", &cheat::Cvars.Lua_List);
	cheat::Cvars.LuaList.PlaceControl("Load", &cheat::Cvars.LoadLuaFile);
	cheat::Cvars.LuaList.PlaceControl("Refresh", &cheat::Cvars.RefreshLuaList);
	//cheat::Cvars.RefreshLuaList.SetFunc(RefreshLuaFiles);
	//cheat::Cvars.LoadLuaFile.SetFunc(LoadLua);


	cheat::Cvars.EntList.Setup(0, 100);
	cheat::Cvars.EntLists.PlaceControl("show all ents", &cheat::Cvars.SearchAll);
	cheat::Cvars.EntLists.PlaceControl("search", &cheat::Cvars.SearchWhites);
	cheat::Cvars.EntLists.PlaceControl("entities", &cheat::Cvars.EntList);
	//cheat::Cvars.WhiteList.SetFunc(WhiteListEnt);
	//cheat::Cvars.WhiteListAll.SetFunc(WhiteListEntAll);
	cheat::Cvars.EntLists.PlaceControl("add", &cheat::Cvars.WhiteList);
	cheat::Cvars.EntLists.PlaceControl("add all", &cheat::Cvars.WhiteListAll);
	//whitelist
	cheat::Cvars.WhiteEntList.Setup(0, 80);
	cheat::Cvars.WhiteLists.PlaceControl("white list", &cheat::Cvars.WhiteEntList);
	//cheat::Cvars.RemoveWhiteListd.SetFunc(RemoveListedEnt);
	//cheat::Cvars.ClearWhitelist.SetFunc(ClearWhite);
	cheat::Cvars.WhiteLists.PlaceControl("remove", &cheat::Cvars.RemoveWhiteListd);
	cheat::Cvars.WhiteLists.PlaceControl("clear", &cheat::Cvars.ClearWhitelist);


	//// Settings tab
	cheat::Cvars.SaveButton.SetFunc(SaveToConfig);
	cheat::Cvars.LoadButton.SetFunc(LoadFromConfigs);
	cheat::Cvars.Import.SetFunc(LoadClipBoardText);
	cheat::Cvars.Configs.Setup(0, 80);
	cheat::Cvars.CreateNewConfig.SetFunc(AddToFile);
	cheat::Cvars.UpdateButton.SetFunc(full_update);
	cheat::Cvars.deleteConfig.SetFunc(DeleteConfig);
	//cheat::Cvars.RefreshButton.SetFunc(AddToConfigs);
	cheat::Cvars.configs.PlaceControl("config list", &cheat::Cvars.Configs, 1);
	cheat::Cvars.configs.PlaceControl("config name", &cheat::Cvars.CreateConfig, 1);
	cheat::Cvars.configs.PlaceControl("save config", &cheat::Cvars.SaveButton, 1);
	cheat::Cvars.configs.PlaceControl("load config", &cheat::Cvars.LoadButton, 1);
	//cheat::Cvars.configs.PlaceControl("refresh", &cheat::Cvars.RefreshButton, 1);
	cheat::Cvars.configs.PlaceControl("import from clipboard", &cheat::Cvars.Import, 1);
	cheat::Cvars.configs.PlaceControl("delete config", &cheat::Cvars.deleteConfig, 1);
	cheat::Cvars.configs.PlaceControl("create config", &cheat::Cvars.CreateNewConfig, 1);
	cheat::Cvars.Import.SetSize(580, 15);
	cheat::Cvars.SaveButton.SetSize(580, 15);
	cheat::Cvars.LoadButton.SetSize(580, 15);
	cheat::Cvars.RefreshButton.SetSize(580, 15);
	cheat::Cvars.deleteConfig.SetSize(580, 15);
	cheat::Cvars.CreateNewConfig.SetSize(580, 15);

	//cheat::Cvars.exploits.PlaceControl("lc break", &cheat::Cvars.Exploits_lc_break);
}

void CMenu::StartRender()
{
	for (int i = 0; i < 255; i++)
	{
		if (cheat::game::get_key_press(i))
		{
			FinishRender();
			//PlaceControls();
			break;
		}
	}
}

void CMenu::FinishRender()
{
	// clear controls from previous frame
	for (auto window : windows)
	{
		if (!window->ShouldDraw())
			continue;

		if (window->m_DrawState == PureState)
			continue;

		if (window == &EditorWindow)
			continue;
	}
}

//void CMenu::HandleScroll(WPARAM in)
//{
//	for (auto window : Hack.Drawings->windows)
//	{
//		if (!window->ShouldDraw())
//			continue;
//
//		if (window->m_DrawState == PureState)
//			continue;
//
//		if (window->SelectedTab == nullptr)
//			continue;
//
//		auto tabs = window->SelectedTab;
//		for (int k = 0; k < tabs->Controls.size(); ++k)
//		{
//			auto controls = tabs->Controls[k];
//			if (controls->ControlType != UIC_ListBox)
//				continue;
//
//			static_cast<CListBox*>(controls)->scroll = in;
//		}
//	}
//}

void CMenu::UpdateMenu()
{
	if (DrawMains)
	{
		//Hack.Drawings->ClickHandler(WND);
		HandleTopMost();
		UpdateFrame();
	}
	DrawMains = false;
	for (int i = 0; i < windows.size(); ++i)
	{
		auto window = windows[i];
		if (!window->ShouldDraw())
			continue;

		if (window->m_DrawState != PureState)
			DrawMains = true;

		window->Draw();
	}
	if (DrawMains)
	{
		//Hack.Drawings->DrawMouse(Color::White());

		GetConfigMassive();
	}
	//Hack.Drawings->UpdateScroll(0);

	cheat::features::menu.mouse1_pressed = false;
	static float lol = 0.f;

	if (cheat::Cvars.Configs.GetValue() != lol)
	{
		cheat::Cvars.CreateConfig.SetValue(cheat::Cvars.Configs.GetName());
		lol = cheat::Cvars.Configs.GetValue();
	}
}
bool piska = false;

void CMenu::UpdateFrame()
{
	auto get_skin_by_paint_kit = [](int wpn, int id) -> int
	{
		for (auto i = 0; i < parser::weapons.list[wpn].skins.list.size(); i++) {
			auto & skin = parser::weapons.list[wpn].skins.list[i];

			if (skin.id == id) {
				return i;
			}
		}

		return 0;
	};

	//if (main.SelectedTab == &Lists)
	//{
		//AddToPlayerlist();
		//AddToEntList();
		//AddToWhiteEntList();
	//}
	// on refresh
	//Radar.SetDrawBool(cheat::Cvars.Visuals_Radar.GetValue());
	//SpecWindow.SetDrawBool(cheat::Cvars.Visuals_SpecList.GetValue());
	//FinderWindow.SetDrawBool(cheat::Cvars.Visuals_BadGuyList.GetValue());
	//EditorWindow.SetDrawBool(cheat::Cvars.Visuals_BadGuyList.GetValue());

	//if (cheat::Cvars.Visuals_BadGuyList.GetValue() == 1)
	//	FinderWindow.SetTitle("traitors");
	//else
	//	FinderWindow.SetTitle("murderer");

	//if (cheat::Cvars.Visuals_Radar_Size.GetValue() > 5)
	//	Radar.SetSize(cheat::Cvars.Visuals_Radar_Size.GetValue(), cheat::Cvars.Visuals_Radar_Size.GetValue());

	//if (cheat::Cvars.Visuals_Radar_Zoom.GetValue() > 1)
	//	radar.zoom = cheat::Cvars.Visuals_Radar_Zoom.GetValue();
	//if (piska)
	//CompileCode();

	//cheat::features::menu.mouse1_pressed = false;

	if (!parser::weapons.list.empty()) {

		static int pselected_skin = -1;
		static int pselected_pk = -1;
		static int prev_search_size = 0;
		auto selected_skin = cheat::Cvars.Skins_Weapons.GetIdx();

		auto selected_pk = cheat::Cvars.Skins_Paintkits.GetSkinId();

		if (cheat::Cvars.Skins_Search.GetText().size() != prev_search_size)
			pselected_skin = -1;

		if (selected_skin != pselected_skin) {

			if (selected_skin < 0)
				selected_skin = 0;

			cheat::Cvars.Skins_Paintkits.ResetItems();

			if (cheat::Cvars.Skins_Search.GetText().size() != prev_search_size)
			{
				for (auto i = 0; i < parser::weapons.list[selected_skin].skins.list.size(); i++)
				{
					auto skins = parser::weapons.list[selected_skin].skins.list.data()[i];

					//if (skins.translated_name.find(cheat::Cvars.Skins_Search.GetText()) == std::string::npos)
					if (parser::to_lower(skins.translated_name).find(parser::to_lower(cheat::Cvars.Skins_Search.GetText())) == std::string::npos)
						continue;

					auto color = parser::rarities.get_by_id(skins.rarity);

					cheat::Cvars.Skins_Paintkits.AddItem(skins.translated_name.data(), skins.id, color.Color);
				}

				prev_search_size = cheat::Cvars.Skins_Search.GetText().size();
			}
			else
			{
				for (auto i = 0; i < parser::weapons.list[selected_skin].skins.list.size(); i++)
				{
					auto skins = parser::weapons.list[selected_skin].skins.list.data()[i];
					auto color = parser::rarities.get_by_id(skins.rarity);

					cheat::Cvars.Skins_Paintkits.AddItem(skins.translated_name.data(), skins.id, color.Color);
				}
			}

			auto cur_pt = get_skin_by_paint_kit(selected_skin, cheat::settings.paint[parser::weapons.list[selected_skin].id]);

			if (cur_pt != cheat::Cvars.Skins_Paintkits.GetIdx())
				cheat::Cvars.Skins_Paintkits.SetIdx(cur_pt);

			if (fully_update) {
				full_update();
				fully_update = false;
			}
		}

		if (selected_skin != pselected_skin || pselected_pk != selected_pk)
		{
			if (selected_skin != -1) {

				if (selected_pk < 0)
					selected_pk = 0;

				if (parser::weapons.list[selected_skin].skins.list.size())
					cheat::settings.paint[parser::weapons.list[selected_skin].id] = selected_pk;
			}

			pselected_skin = selected_skin;
			pselected_pk = selected_pk;
		}
	}

	//if (cheat::Cvars.RageBot_AutoFire.GetValue())
	//	cheat::Cvars.RageBot_Key.m_Flags |= UI_SkipRender;
	//else
	//	cheat::Cvars.RageBot_Key.m_Flags &= ~UI_SkipRender;
}

void SetBool()
{
	piska = true;
}

void CMenu::Setup()
{
	main.SetupWindow(400, 400, 600, 497, "bye-bye mister");
	main.RegisterTab(&legitbot, "legit bot", "a", 340, 660, { "Pistol", "Rifle", "Custom" });
	main.RegisterTab(&ragebot, "rage bot", "c", 0, 623, {"aimbot", "anti-aim"});
	main.RegisterTab(&visuals, "visuals", "d", 530, 0, { "players","other" });
	main.RegisterTab(&misc, "miscellaneous", "f", 0, 623, {"misc", "config"});
	main.RegisterTab(&skins, "skins", "e", 0, 623);
	main.RegisterTab(&Lists, "players", "k", 530, 660);

	//main.RegisterTab(&settings, "Settings", "b");
	SpecWindow.SetupWindow(20, 20, 150, 40, "spectators", PureState);
	SpecWindow.m_bDrawTitle = 2;
	SpecWindow.RegisterTab(&spectab, "Speclist");
	SpecWindow.grabheight = 25;
	spectab.RegisterControl(&CSpecList, "Spectator list");
	FinderWindow.SetupWindow(50, 50, 150, 40, "Shouldnt", PureState);
	FinderWindow.m_bDrawTitle = 2;
	FinderWindow.RegisterTab(&badguytab, "Speclist");
	FinderWindow.grabheight = 25;
	//RefreshLuaFiles();
	//EditorWindow.SetupWindow(50, 50, 500, 500, "Code Editor", MainWindowExtra);
	//EditorWindow.m_bDrawTitle = 2;
	//EditorWindow.RegisterTab(&ESPedittab, "ESP");
	//EditorWindow.grabheight = 25;
	//ESPedittab.RegisterControl(&ESPEdit, "Edit ESP");
	//ESPedittab.RegisterControl(&Compile, "Compile");
	//Compile.SetSize(490, 20);
	//Compile.SetPos(0, 445);
	//Compile.SetFunc(SetBool);

	//badguytab.RegisterControl(&CTList, "Bad list");
	//Radar.SetupWindow(200, 200, 300, 300, "radar", PureState);
	//Radar.m_bDrawTitle = 0;
	//Radar.RegisterTab(&rd, "radar");
	//rd.RegisterControl(&radar, "radar");
	PlaceControls();
	//AddToConfigs();
	cheat::Cvars.GradientFrom.SetColor(Color(66, 140, 244));
	cheat::Cvars.GradientTo.SetColor(Color(75, 180, 242));
	cheat::Cvars.MenuTheme.SetColor(Color(66, 140, 244));

}

void CMenu::HandleInput()
{
	//cheat::Cvars.bMenu = !cheat::Cvars.bMenu;
	//Source::m_pInputSystem->EnableInput(!cheat::Cvars.bMenu);
	//Source::m_pSurface->SetCursorAlwaysVisible(cheat::Cvars.bMenu);
}

namespace cstd {
	template <typename T>
	void moveItemToBack(std::vector<T>& v, size_t itemIndex)
	{
		auto it = v.begin() + itemIndex;
		std::rotate(it, it + 1, v.end());
	}
	template<typename It>
	class Range
	{
		It b, e;
	public:
		Range(It b, It e) : b(b), e(e) {}
		It begin() const { return b; }
		It end() const { return e; }
	};

	template<typename ORange, typename OIt = decltype(std::begin(std::declval<ORange>())), typename It = std::reverse_iterator<OIt>>
	Range<It> reverse(ORange && originalRange) {
		return Range<It>(It(std::end(originalRange)), It(std::begin(originalRange)));
	}
}

void CMenu::HandleTopMost()
{
	if (!windows.empty()) {
		auto lastWindow = windows.at(windows.size() - 1);
		if (cheat::features::menu.menu_opened && cheat::features::menu.mouse1_pressed) {
			for (auto window : cstd::reverse(windows)) {

				if (cheat::features::menu.Mousein(Vector(window->m_x, window->m_y, 0), Vector(window->m_w, window->m_h, 0))
					&& !cheat::features::menu.Mousein(Vector(lastWindow->m_x, lastWindow->m_y, 0), Vector(lastWindow->m_w, lastWindow->m_h, 0)))
				{
					ptrdiff_t pos = std::find(windows.begin(), windows.end(), window) - windows.begin();
					cstd::moveItemToBack(windows, pos);

					break;
				}
			}
		}
	}
}