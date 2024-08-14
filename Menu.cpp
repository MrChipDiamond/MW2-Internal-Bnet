#include "stdafx.h"
#include "Menu.h"
#include "imgui/imgui.h"
# include "globals.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "obfuscator.hpp"
#include "xor.hpp"
#include"memory.h"
#include "xorstr.hpp"
#include "style.h"
#include "UnlockAllV2.h"

#define INRANGE(x,a,b)    (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))
bool b_menu_open = true;
bool b_debug_open = false;
bool boxcheck;
int Selected_Camo_MW = 0;
int Selected_Camo_CW = 0;
int Selected_Camo_VG = 0;
int gameMode2 = 0;
int i_MenuTab = 0;
uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets)
{
	if (ptr != 0)
	{
		uintptr_t addr = ptr;
		for (unsigned int i = 0; i < offsets.size(); ++i)
		{
			addr = *(uintptr_t*)addr;
			addr += offsets[i];
		}
		return addr;
	}
	else
		return 0;
}


//uint64_t BASEIMAGE2 = reinterpret_cast<uint64_t>(GetModuleHandleA(NULL));

bool b_fov = false;
float f_fov = 1.20f;
float f_map = 1.0f;
bool b_map = false;
bool b_brightmax = false;
bool b_thirdperson = false;
bool b_heartcheat = false;
bool b_norecoil = false;
bool b_no_flashbang = false;

struct unnamed_type_integer
{
	int min;
	int max;
};
struct unnamed_type_integer64
{
	__int64 min;
	__int64 max;
};
struct unnamed_type_enumeration
{
	int stringCount;
	const char* strings;
};
/* 433 */
struct unnamed_type_unsignedInt64
{
	unsigned __int64 min;
	unsigned __int64 max;
};

/* 434 */
struct unnamed_type_value
{
	float min;
	float max;
	float devguiStep;
};

/* 435 */
struct unnamed_type_vector
{
	float min;
	float max;
	float devguiStep;
};




uintptr_t cbuff1;
uintptr_t cbuff2;
char inputtext[50];
int C_TagMOde = 0;


__int64 find_pattern(__int64 range_start, __int64 range_end, const char* pattern) {
	const char* pat = pattern;
	__int64 firstMatch = NULL;
	__int64 pCur = range_start;
	__int64 region_end;
	MEMORY_BASIC_INFORMATION mbi{};
	while (sizeof(mbi) == VirtualQuery((LPCVOID)pCur, &mbi, sizeof(mbi))) {
		if (pCur >= range_end - strlen(pattern))
			break;
		if (!(mbi.Protect & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_READWRITE))) {
			pCur += mbi.RegionSize;
			continue;
		}
		region_end = pCur + mbi.RegionSize;
		while (pCur < region_end)
		{
			if (!*pat)
				return firstMatch;
			if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat)) {
				if (!firstMatch)
					firstMatch = pCur;
				if (!pat[1] || !pat[2])
					return firstMatch;

				if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
					pat += 3;
				else
					pat += 2;
			}
			else {
				if (firstMatch)
					pCur = firstMatch;
				pat = pattern;
				firstMatch = 0;
			}
			pCur++;
		}
	}
	return NULL;
}


bool init_once = true;
char input[30];
bool Unlock_once = true;

void Visual()
{
	ImGui::Dummy(ImVec2(0.0f, 3.0f));
	ImGui::Spacing();
	ImGui::Checkbox(xorstr_("Check Visibility"), &globals::b_visible);
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(xorstr_("Can only be turned on INGAME!"));
	}
	ImGui::Checkbox(xorstr_("Show Box"), &globals::b_box);
	

	//ImGui::Checkbox(xorstr_("Show HealthBar"), &globals::b_health);
	ImGui::Checkbox(xorstr_("Show Line"), &globals::b_line);
	ImGui::Checkbox(xorstr_("Show Bones "), &globals::b_skeleton);
	ImGui::Checkbox(xorstr_("Show Names"), &globals::b_names);
	if (globals::b_names)
	{
		ImGui::SliderFloat(xorstr_("##Namesize"), &globals::b_namesize, 1.0f, 100.f, xorstr_("Name Size: %0.0f"));
	}
	ImGui::Checkbox(xorstr_("Show Distance"), &globals::b_distance);
	if (globals::b_distance)
	{
		ImGui::SliderFloat(xorstr_("##Distancesize"), &globals::b_distancesize, 1.0f, 100.f, xorstr_("Distance Size: %0.0f"));
	}
	ImGui::Checkbox(xorstr_("Show Team"), &globals::b_friendly);
	ImGui::SliderInt(xorstr_("##MAXDISTANCE"), &globals::max_distance, 0, 1000, xorstr_("ESP Distance: %d"));
}
void KeyBindButton(int& key, int width, int height)
{
	static auto b_get = false;
	static std::string sz_text = xorstr_("Click to bind.");

	if (ImGui::Button(sz_text.c_str(), ImVec2(static_cast<float>(width), static_cast<float>(height))))
		b_get = true;

	if (b_get)
	{
		for (auto i = 1; i < 256; i++)
		{
			if (GetAsyncKeyState(i) & 0x8000)
			{
				if (i != 12)
				{
					key = i == VK_ESCAPE ? -1 : i;
					b_get = false;
				}
			}
		}
		sz_text = xorstr_("Press a Key.");
	}
	else if (!b_get && key == -1)
		sz_text = xorstr_("Click to bind.");
	else if (!b_get && key != -1)
	{
		sz_text = xorstr_("Key ~ ") + std::to_string(key);
	}
}
void Aimbot()
{

	ImGui::Dummy(ImVec2(0.0f, 3.0f));
	ImGui::Spacing();
	ImGui::Checkbox(xorstr_("Enable"), &globals::b_lock);
	if (globals::b_lock)
	{
		ImGui::SliderInt(xorstr_("##LOCKSMOOTH"), &globals::aim_smooth, 1, 30, xorstr_("Lock Smooth: %d"));
	}
	ImGui::Checkbox(xorstr_("Crosshair"), &globals::b_crosshair);
	ImGui::Checkbox(xorstr_("Show FOV"), &globals::b_fov);
	if (globals::b_fov)
	{
		ImGui::SliderFloat(xorstr_("##LOCKFOV"), &globals::f_fov_size, 10.f, 800.f, xorstr_("FOV Size: %0.0f"));
	}
	//ImGui::Checkbox(xorstr_("Show Aimpoints"), &globals::b_aim_point);
	ImGui::Checkbox(xorstr_("Skip Knocked"), &globals::b_skip_knocked);

	
	
	/*ImGui::Checkbox(xorstr_("Prediction"), &globals::b_prediction);*/

	
	

	ImGui::Checkbox(xorstr_("Use Bones"), &globals::target_bone);
	if (globals::target_bone)
	{
		static const char* items[] = { "Helmet","Head","Neck","Chest","Mid","Tummy","Pelvis","Right Food","Left Food"};
		static const char* current_item = "Select Bone";
		
	
	if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n < IM_ARRAYSIZE(items); n++)
		{
			bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
			if (ImGui::Selectable(items[n], is_selected))
				current_item = items[n];
			if (is_selected)
				ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
		}
		ImGui::EndCombo();
	}
	if (current_item == "Helmet")
		globals::b_helmet = true;
	else globals::b_helmet = false;
	if (current_item == "Head")
		globals::b_head = true;
	else globals::b_head = false;
	if (current_item == "Neck")
		globals::b_neck = true;
	else globals::b_neck = false;
	if (current_item == "Chest")
		globals::b_chest = true;
	else globals::b_chest = false;
	if (current_item == "Mid")
		globals::b_mid = true;
	else globals::b_mid = false;
	if (current_item == "Tummy")
		globals::b_tummy = true;
	else globals::b_tummy = false;
	if (current_item == "Pelvis")
		globals::b_pelvis = true;
	else globals::b_pelvis = false;
	if (current_item == "Right Food")
		globals::b_rightfood = true;
	else globals::b_rightfood = false;
	if (current_item == "Left Food")
		globals::b_leftfood = true;
	else globals::b_leftfood = false;
}
	KeyBindButton(globals::aim_key, 100, 30);
	ImGui::Dummy(ImVec2(0.0f, 1.0f));
	ImGui::Spacing();

	

}
void ColorPicker()
{
	ImGui::Dummy(ImVec2(0.0f, 1.0f));
	ImGui::Text("Fov");
	ImGui::ColorEdit4("##Fov Color7", (float*)&color::bfov);
	ImGui::Text("Crosshair");
	ImGui::ColorEdit4("##cross hair Color9", (float*)&color::draw_crosshair);
	ImGui::Text("Healthbar");
	ImGui::ColorEdit4("##health", (float*)&color::healthbar);
	ImGui::Text("Visible Team");
	ImGui::ColorEdit4("##esp Color1", (float*)&color::VisibleColorTeam);
	ImGui::Spacing();
	ImGui::Text("Not Visible Team");
	ImGui::ColorEdit4("##esp Color2", (float*)&color::NotVisibleColorTeam);
	ImGui::Spacing();
	ImGui::Text("Visible Enemy");
	ImGui::ColorEdit4("##esp Color3", (float*)&color::VisibleColorEnemy);
	ImGui::Spacing();
	ImGui::Text("Not Visible Enemy");
	ImGui::ColorEdit4("##esp Color4", (float*)&color::NotVisibleColorEnemy);

}
void CL_PlayerData_SetCustomClanTag(int controllerIndex, const char* clanTag) {
	uintptr_t address = g_data::base + globals::clantagbase;
	((void(*)(int, const char*))address)(controllerIndex, clanTag);
}
void ShowToastNotificationAfterUserJoinedParty(const char* message)
{
	uintptr_t address = g_data::base + 0x2512EF0; // CC 48 89 74 24 ? 57 48 83 EC 20 4C 8B 05 ? ? ? ? 33 + 1 
	((void(*)(int, int, int, const char*, int))address)(0, 0, 0, message, 0);
}
void Misc()
{
	ImGui::Dummy(ImVec2(0.0f, 3.0f));
	ImGui::Checkbox(xorstr_("No Recoil"), &globals::b_recoil);
	//ImGui::Checkbox(xorstr_("No Spread"), &globals::b_spread);
	ImGui::Checkbox(xorstr_("UAV"), &globals::b_UAV);
	/*if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(xorstr_("Use at your own Risk"));
	}*/
	ImGui::Checkbox(xorstr_("2D Radar"), &radarVars::bEnable2DRadar);
	if (radarVars::bEnable2DRadar)
	{
		ImGui::Checkbox(xorstr_("Transparent"), &radarVars::transparent);
	}
	ImGui::Spacing();

	ImGui::Dummy(ImVec2(0.0f, 3.0f));
	ImGui::Spacing();
	/*if (ImGui::SliderFloat("FOV SLIDER", &globals::f_fov, 0.1f, 4.f, "%.2f"))
	{
		dvar_set2("NSSLSNKPN", globals::f_fov);
	}
	ImGui::Spacing();*/
	//if (ImGui::Button(xorstr_("Unlock All"), ImVec2(150, 30)))
	//{

	//	sdk::unlockall();

	//}

	ImGui::Dummy(ImVec2(0.0f, 3.0f));

	ImGui::Text(xorstr_("Unlockall:"));
	ImGui::Spacing();
	if (ImGui::Button(xorstr_("Activate"), ImVec2(100, 25)))
	{
		UnlockAllV2 v2 = UnlockAllV2();
		v2.unlock_All();
	}

	


	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(xorstr_("DEACTIVATE BEFORE JOINING GAME!"));
	}
	ImGui::Dummy(ImVec2(0.0f, 3.0f));
	//ImGui::Checkbox(xorstr_("No Recoil"), &globals::b_recoil);
	//ShowToastNotificationAfterUserJoinedParty("^4Sim^2ple^5Too^2lZ ^3Unlock^1ed ^5Everything! ^1<3");
	ImGui::Spacing();
	
	
	static char customtag[20];
	ImGui::Spacing();
	ImGui::InputTextWithHint(xorstr_("##KeyInput"), xorstr_("Custom Clan Tag"), customtag, 20);
	ImGui::SameLine();
	if (ImGui::Button(xorstr_("Set"), ImVec2(50, 20)))
	{
		CL_PlayerData_SetCustomClanTag(0, customtag);
	}
}
void SetCamo(int Class, int Weapon, int Camo)
{
	char context[255];
	char state[255];
	int navStringCount;
	char* navStrings[16]{};
	const char* mode = "";

	if (gameMode2 == 0)
	{
		mode = xorstr_("ddl/mp/rankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 3);
	}
	else if (gameMode2 == 1)
	{
		mode = xorstr_("ddl/mp/wzrankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 5);
	}



	__int64 ddl_file = Com_DDL_LoadAsset((__int64)mode);

	DDL_GetRootState((__int64)state, ddl_file);
	char buffer[200];
	memset(buffer, 0, 200);
	sprintf_s(buffer, xorstr_("squadMembers.loadouts.%i.weaponSetups.%i.camo"), Class, Weapon);
	ParseShit(buffer, (const char**)navStrings, 16, &navStringCount);
	if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, navStringCount, (const char**)navStrings))
	{
		DDL_SetInt((__int64)state, (__int64)context, Camo);
	}

}

void setWeapon(int loadout, int weapon, int weaponId)
{
	char context[255];
	char state[255];
	int string_count;
	char* str[16]{};
	const char* mode = "";
	char buffer[200];

	if (gameMode2 == 0)
	{
		mode = xorstr_("ddl/mp/rankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 3);
	}
	else if (gameMode2 == 1)
	{
		mode = xorstr_("ddl/mp/wzrankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 5);
	}
	__int64 ddl_file = Com_DDL_LoadAsset((__int64)mode);
	DDL_GetRootState((__int64)state, ddl_file);
	memset(buffer, 0, 200);
	sprintf_s(buffer, xorstr_("squadMembers.loadouts.%i.weaponSetups.%i.weapon"), loadout, weapon);
	ParseShit(buffer, (const char**)str, 255, &string_count);
	if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, string_count, (const char**)str))
	{
		DDL_SetInt((__int64)state, (__int64)context, weaponId);
	}
}
void setOperator(const char operators[255], int operatorid)
{
	char context[255];
	char state[255];
	int string_count;
	char* str[16]{};
	const char* mode = "";
	char buffer[200];

	if (gameMode2 == 0)
	{
		mode = xorstr_("ddl/mp/rankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 3);
	}
	else if (gameMode2 == 1)
	{
		mode = xorstr_("ddl/mp/wzrankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 5);
	}
	__int64 ddl_file = Com_DDL_LoadAsset((__int64)mode);
	DDL_GetRootState((__int64)state, ddl_file);
	memset(buffer, 0, 200);
	sprintf_s(buffer, xorstr_("customizationSetup.operatorCustomization.%s.skin"), operators);
	ParseShit(buffer, (const char**)str, 255, &string_count);
	if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, string_count, (const char**)str))
	{
		DDL_SetInt((__int64)state, (__int64)context, operatorid);
	}
}
void CopyWeapon(int Class)
{
	char context[255];
	char state[255];
	char context2[255];
	char state2[255];
	int navStringCount;
	char* navStrings[16]{};
	int navStringCount2;
	char* navStrings2[16]{};
	const char* mode = "";
	int wep = 0;
	if (gameMode2 == 0)
	{
		mode = xorstr_("ddl/mp/rankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 3);
	}
	else if (gameMode2 == 1)
	{
		mode = xorstr_("ddl/mp/wzrankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 5);
	}



	__int64 ddl_file = Com_DDL_LoadAsset((__int64)mode);

	DDL_GetRootState((__int64)state, ddl_file);
	char buffer[200];
	memset(buffer, 0, 200);
	sprintf_s(buffer, xorstr_("squadMembers.loadouts.%i.weaponSetups.0.weapon"), Class);
	ParseShit(buffer, (const char**)navStrings, 16, &navStringCount);
	if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, navStringCount, (const char**)navStrings))
	{
		wep = DDL_GetInt((__int64*)&state, (__int64*)&context);

	}
	if (gameMode2 == 0)
	{
		mode = xorstr_("ddl/mp/rankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context2, 0, 0, 3);
	}
	else if (gameMode2 == 1)
	{
		mode = xorstr_("ddl/mp/wzrankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context2, 0, 0, 5);
	}
	__int64 ddl_file2 = Com_DDL_LoadAsset((__int64)mode);
	DDL_GetRootState((__int64)state2, ddl_file2);
	char buffer2[200];
	memset(buffer2, 0, 200);
	sprintf_s(buffer2, xorstr_("squadMembers.loadouts.%i.weaponSetups.1.weapon"), Class);
	ParseShit(buffer2, (const char**)navStrings2, 16, &navStringCount2);
	if (DDL_MoveToPath((__int64*)&state2, (__int64*)&state2, navStringCount2, (const char**)navStrings2))
	{
		DDL_SetInt2((__int64*)&state2, (__int64*)&context2, wep);
	}


}
void Loadout()
{
	ImGui::BeginGroup(); {
		ImGui::Text(xorstr_("Select Gamemode:"));
		ImGui::RadioButton(xorstr_("Multiplayer"), &gameMode2, 0);

		ImGui::SameLine();

		ImGui::RadioButton(xorstr_("Warzone"), &gameMode2, 1);
		ImGui::Spacing();
		//ImGui::SetCursorPos(ImVec2(45, 135));
		ImGui::Text(xorstr_("Copy Gun:"));
		static int classNameIndex2 = 0;
		//ImGui::SetCursorPos(ImVec2(45, 155));
		ImGui::Combo("##namesss", &classNameIndex2, xorstr_("Loadout 1\0Loadout 2\0Loadout 3\0Loadout 4\0Loadout 5\0Loadout 6\0Loadout 7\0Loadout 8\0Loadout 9\0Loadout 10\0\0"));
		//ImGui::SameLine();
		ImGui::SameLine();

		if (ImGui::Button(xorstr_("Copy Gun"), ImVec2(100, 25)))
		{
			CopyWeapon(classNameIndex2);
		}
		ImGui::Text(xorstr_("Colored Classes:"));
		static int classNameIndex = 0;
		//ImGui::SetCursorPos(ImVec2(45, 62));
		ImGui::Combo("##namess", &classNameIndex, xorstr_("Loadout 1\0Loadout 2\0Loadout 3\0Loadout 4\0Loadout 5\0Loadout 6\0Loadout 7\0Loadout 8\0Loadout 9\0Loadout 10\0\0"));
		static char str1[128] = "";
		//ImGui::SetCursorPos(ImVec2(45, 86));
		ImGui::InputTextWithHint("", xorstr_("Enter Class Name"), str1, IM_ARRAYSIZE(str1));
		static int q = 0;
		//ImGui::SetCursorPos(ImVec2(45, 110));
		ImGui::SameLine();
		if (ImGui::Button(xorstr_("Set Name"), ImVec2(100, 25)))
		{
			char context[255];
			char state[255];
			int navStringCount;
			char* navStrings[16]{};
			const char* mode = "";

			if (gameMode2 == 0)
			{
				mode = xorstr_("ddl/mp/rankedloadouts.ddl");
				CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 3);
			}
			else if (gameMode2 == 1)
			{
				mode = xorstr_("ddl/mp/wzrankedloadouts.ddl");
				CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 5);
			}

			__int64 ddl_file = Com_DDL_LoadAsset((__int64)mode);

			DDL_GetRootState((__int64)state, ddl_file);
			char buffer[200];
			memset(buffer, 0, 200);
			sprintf_s(buffer, xorstr_("squadMembers.loadouts.%i.name"), classNameIndex);
			ParseShit(buffer, (const char**)navStrings, 16, &navStringCount);
			if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, navStringCount, (const char**)navStrings))
			{
				DDL_SetString((__int64)state, (__int64)context, str1);
			}

		}



		//dvar_set2("NRQQOMLOQL", 0);
		//dvar_set2("RRTLRKKTT", 0);
		//dvar_set2("MKQPRPLQKL", 0);

		ImGui::Spacing();
		ImGui::Text("Set Camo:");
		ImGui::Spacing();
		static const char* items16[] = { "(MW) Gold", "(MW) Platinum", "(MW) Damascus", "(MW) Obsidian", "(CW) Gold", "(CW) Diamond", "(CW) DMU", "(CW) Golden Viper", "(CW) Plague Diamond", "(CW) Dark Aether", "(VG) Gold", "(VG) Diamond", "(VG) Atomic", "(VG) Fake Diamond", "(VG) Golden Viper", "(VG) Plague Diamond", "(VG) Dark Aether", "(CW) Cherry Blossom",  "(MW) Acticamo", "(MW) Banded", };
		static const char* current_item16 = "Select Camo";
		//ImGuiPP::CenterText("Camo Editor", 1, TRUE);
		//ImGui::Indent(10);
		static int item_current_2 = 0;
		ImGui::Combo("          ", &item_current_2, xorstr_("Class 1\0Class 2\0Class 3\0Class 4\0Class 5\0Class 6\0Class 7\0Class 8\0Class 9\0Class 10\0\0"));

		static int item_current_3 = 0;
		ImGui::Combo("                   ", &item_current_3, xorstr_("Primary\0Secondary\0\0"));
		if (ImGui::BeginCombo(xorstr_("##combo"), current_item16)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items16); n++)
			{
				bool is_selected = (current_item16 == items16[n]); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(items16[n], is_selected))
					current_item16 = items16[n];
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
			ImGui::EndCombo();
		}


		ImGui::SameLine();
		if (ImGui::Button(xorstr_("Set Camo"), ImVec2(100, 25)))
		{


			if (current_item16 == "(MW) Gold")
				SetCamo(item_current_2, item_current_3, 111);

			if (current_item16 == "(MW) Platinum")
				SetCamo(item_current_2, item_current_3, 112);

			if (current_item16 == "(MW) Damascus")
				SetCamo(item_current_2, item_current_3, 113);

			if (current_item16 == "(MW) Obsidian")
				SetCamo(item_current_2, item_current_3, 114);

			if (current_item16 == "(CW) Gold")
				SetCamo(item_current_2, item_current_3, 133);

			if (current_item16 == "(CW) Diamond")
				SetCamo(item_current_2, item_current_3, 134);

			if (current_item16 == "(CW) DMU")
				SetCamo(item_current_2, item_current_3, 135);

			if (current_item16 == "(CW) Golden Viper")
				SetCamo(item_current_2, item_current_3, 242);

			if (current_item16 == "(CW) Plague Diamond")
				SetCamo(item_current_2, item_current_3, 243);

			if (current_item16 == "(CW) Dark Aether")
				SetCamo(item_current_2, item_current_3, 244);

			if (current_item16 == "(VG) Gold")
				SetCamo(item_current_2, item_current_3, 345);

			if (current_item16 == "(VG) Diamond")
				SetCamo(item_current_2, item_current_3, 346);

			if (current_item16 == "(VG) Atomic")
				SetCamo(item_current_2, item_current_3, 350);

			if (current_item16 == "(VG) Fake Diamond")
				SetCamo(item_current_2, item_current_3, 349);

			if (current_item16 == "(VG) Golden Viper")
				SetCamo(item_current_2, item_current_3, 402);

			if (current_item16 == "(VG) Plague Diamond")
				SetCamo(item_current_2, item_current_3, 403);

			if (current_item16 == "(VG) Dark Aether")
				SetCamo(item_current_2, item_current_3, 404);

			if (current_item16 == "(CW) Cherry Blossom")
				SetCamo(item_current_2, item_current_3, 160);

			if (current_item16 == "(MW) Acticamo")
				SetCamo(item_current_2, item_current_3, 2);

			if (current_item16 == "(MW) Banded")
				SetCamo(item_current_2, item_current_3, 3);
		}
		ImGui::Spacing();
		if (ImGui::Button(xorstr_("Apply to all Classes!"), ImVec2(305, 25)))
		{

			if (current_item16 == "(MW) Gold")
			{
				SetCamo(0, 0, 111);
				SetCamo(0, 1, 111);
				SetCamo(1, 0, 111);
				SetCamo(1, 1, 111);
				SetCamo(2, 0, 111);
				SetCamo(2, 1, 111);
				SetCamo(3, 0, 111);
				SetCamo(3, 1, 111);
				SetCamo(4, 0, 111);
				SetCamo(4, 1, 111);
				SetCamo(5, 0, 111);
				SetCamo(5, 1, 111);
				SetCamo(6, 0, 111);
				SetCamo(6, 1, 111);
				SetCamo(7, 0, 111);
				SetCamo(7, 1, 111);
				SetCamo(8, 0, 111);
				SetCamo(8, 1, 111);
				SetCamo(9, 0, 111);
				SetCamo(9, 1, 111);
			}

			if (current_item16 == "(MW) Platinum")
			{
				SetCamo(0, 0, 112);
				SetCamo(0, 1, 112);
				SetCamo(1, 0, 112);
				SetCamo(1, 1, 112);
				SetCamo(2, 0, 112);
				SetCamo(2, 1, 112);
				SetCamo(3, 0, 112);
				SetCamo(3, 1, 112);
				SetCamo(4, 0, 112);
				SetCamo(4, 1, 112);
				SetCamo(5, 0, 112);
				SetCamo(5, 1, 112);
				SetCamo(6, 0, 112);
				SetCamo(6, 1, 112);
				SetCamo(7, 0, 112);
				SetCamo(7, 1, 112);
				SetCamo(8, 0, 112);
				SetCamo(8, 1, 112);
				SetCamo(9, 0, 112);
				SetCamo(9, 1, 112);
			}

			if (current_item16 == "(MW) Damascus")
			{
				SetCamo(0, 0, 113);
				SetCamo(0, 1, 113);
				SetCamo(1, 0, 113);
				SetCamo(1, 1, 113);
				SetCamo(2, 0, 113);
				SetCamo(2, 1, 113);
				SetCamo(3, 0, 113);
				SetCamo(3, 1, 113);
				SetCamo(4, 0, 113);
				SetCamo(4, 1, 113);
				SetCamo(5, 0, 113);
				SetCamo(5, 1, 113);
				SetCamo(6, 0, 113);
				SetCamo(6, 1, 113);
				SetCamo(7, 0, 113);
				SetCamo(7, 1, 113);
				SetCamo(8, 0, 113);
				SetCamo(8, 1, 113);
				SetCamo(9, 0, 113);
				SetCamo(9, 1, 113);
			}

			if (current_item16 == "(MW) Obsidian")
			{
				SetCamo(0, 0, 114);
				SetCamo(0, 1, 114);
				SetCamo(1, 0, 114);
				SetCamo(1, 1, 114);
				SetCamo(2, 0, 114);
				SetCamo(2, 1, 114);
				SetCamo(3, 0, 114);
				SetCamo(3, 1, 114);
				SetCamo(4, 0, 114);
				SetCamo(4, 1, 114);
				SetCamo(5, 0, 114);
				SetCamo(5, 1, 114);
				SetCamo(6, 0, 114);
				SetCamo(6, 1, 114);
				SetCamo(7, 0, 114);
				SetCamo(7, 1, 114);
				SetCamo(8, 0, 114);
				SetCamo(8, 1, 114);
				SetCamo(9, 0, 114);
				SetCamo(9, 1, 114);
			}

			if (current_item16 == "(CW) Gold")
			{
				SetCamo(0, 0, 133);
				SetCamo(0, 1, 133);
				SetCamo(1, 0, 133);
				SetCamo(1, 1, 133);
				SetCamo(2, 0, 133);
				SetCamo(2, 1, 133);
				SetCamo(3, 0, 133);
				SetCamo(3, 1, 133);
				SetCamo(4, 0, 133);
				SetCamo(4, 1, 133);
				SetCamo(5, 0, 133);
				SetCamo(5, 1, 133);
				SetCamo(6, 0, 133);
				SetCamo(6, 1, 133);
				SetCamo(7, 0, 133);
				SetCamo(7, 1, 133);
				SetCamo(8, 0, 133);
				SetCamo(8, 1, 133);
				SetCamo(9, 0, 133);
				SetCamo(9, 1, 133);
			}

			if (current_item16 == "(CW) Diamond")
			{
				SetCamo(0, 0, 134);
				SetCamo(0, 1, 134);
				SetCamo(1, 0, 134);
				SetCamo(1, 1, 134);
				SetCamo(2, 0, 134);
				SetCamo(2, 1, 134);
				SetCamo(3, 0, 134);
				SetCamo(3, 1, 134);
				SetCamo(4, 0, 134);
				SetCamo(4, 1, 134);
				SetCamo(5, 0, 134);
				SetCamo(5, 1, 134);
				SetCamo(6, 0, 134);
				SetCamo(6, 1, 134);
				SetCamo(7, 0, 134);
				SetCamo(7, 1, 134);
				SetCamo(8, 0, 134);
				SetCamo(8, 1, 134);
				SetCamo(9, 0, 134);
				SetCamo(9, 1, 134);
			}

			if (current_item16 == "(CW) DMU")
			{
				SetCamo(0, 0, 135);
				SetCamo(0, 1, 135);
				SetCamo(1, 0, 135);
				SetCamo(1, 1, 135);
				SetCamo(2, 0, 135);
				SetCamo(2, 1, 135);
				SetCamo(3, 0, 135);
				SetCamo(3, 1, 135);
				SetCamo(4, 0, 135);
				SetCamo(4, 1, 135);
				SetCamo(5, 0, 135);
				SetCamo(5, 1, 135);
				SetCamo(6, 0, 135);
				SetCamo(6, 1, 135);
				SetCamo(7, 0, 135);
				SetCamo(7, 1, 135);
				SetCamo(8, 0, 135);
				SetCamo(8, 1, 135);
				SetCamo(9, 0, 135);
				SetCamo(9, 1, 135);
			}

			if (current_item16 == "(CW) Golden Viper")
			{
				SetCamo(0, 0, 242);
				SetCamo(0, 1, 242);
				SetCamo(1, 0, 242);
				SetCamo(1, 1, 242);
				SetCamo(2, 0, 242);
				SetCamo(2, 1, 242);
				SetCamo(3, 0, 242);
				SetCamo(3, 1, 242);
				SetCamo(4, 0, 242);
				SetCamo(4, 1, 242);
				SetCamo(5, 0, 242);
				SetCamo(5, 1, 242);
				SetCamo(6, 0, 242);
				SetCamo(6, 1, 242);
				SetCamo(7, 0, 242);
				SetCamo(7, 1, 242);
				SetCamo(8, 0, 242);
				SetCamo(8, 1, 242);
				SetCamo(9, 0, 242);
				SetCamo(9, 1, 242);
			}

			if (current_item16 == "(CW) Plague Diamond")
			{
				SetCamo(0, 0, 243);
				SetCamo(0, 1, 243);
				SetCamo(1, 0, 243);
				SetCamo(1, 1, 243);
				SetCamo(2, 0, 243);
				SetCamo(2, 1, 243);
				SetCamo(3, 0, 243);
				SetCamo(3, 1, 243);
				SetCamo(4, 0, 243);
				SetCamo(4, 1, 243);
				SetCamo(5, 0, 243);
				SetCamo(5, 1, 243);
				SetCamo(6, 0, 243);
				SetCamo(6, 1, 243);
				SetCamo(7, 0, 243);
				SetCamo(7, 1, 243);
				SetCamo(8, 0, 243);
				SetCamo(8, 1, 243);
				SetCamo(9, 0, 243);
				SetCamo(9, 1, 243);
			}


			if (current_item16 == "(CW) Dark Aether")
			{
				SetCamo(0, 0, 244);
				SetCamo(0, 1, 244);
				SetCamo(1, 0, 244);
				SetCamo(1, 1, 244);
				SetCamo(2, 0, 244);
				SetCamo(2, 1, 244);
				SetCamo(3, 0, 244);
				SetCamo(3, 1, 244);
				SetCamo(4, 0, 244);
				SetCamo(4, 1, 244);
				SetCamo(5, 0, 244);
				SetCamo(5, 1, 244);
				SetCamo(6, 0, 244);
				SetCamo(6, 1, 244);
				SetCamo(7, 0, 244);
				SetCamo(7, 1, 244);
				SetCamo(8, 0, 244);
				SetCamo(8, 1, 244);
				SetCamo(9, 0, 244);
				SetCamo(9, 1, 244);
			}

			if (current_item16 == "(VG) Gold")
			{
				SetCamo(0, 0, 345);
				SetCamo(0, 1, 345);
				SetCamo(1, 0, 345);
				SetCamo(1, 1, 345);
				SetCamo(2, 0, 345);
				SetCamo(2, 1, 345);
				SetCamo(3, 0, 345);
				SetCamo(3, 1, 345);
				SetCamo(4, 0, 345);
				SetCamo(4, 1, 345);
				SetCamo(5, 0, 345);
				SetCamo(5, 1, 345);
				SetCamo(6, 0, 345);
				SetCamo(6, 1, 345);
				SetCamo(7, 0, 345);
				SetCamo(7, 1, 345);
				SetCamo(8, 0, 345);
				SetCamo(8, 1, 345);
				SetCamo(9, 0, 345);
				SetCamo(9, 1, 345);
			}

			if (current_item16 == "(VG) Diamond")
			{
				SetCamo(0, 0, 346);
				SetCamo(0, 1, 346);
				SetCamo(1, 0, 346);
				SetCamo(1, 1, 346);
				SetCamo(2, 0, 346);
				SetCamo(2, 1, 346);
				SetCamo(3, 0, 346);
				SetCamo(3, 1, 346);
				SetCamo(4, 0, 346);
				SetCamo(4, 1, 346);
				SetCamo(5, 0, 346);
				SetCamo(5, 1, 346);
				SetCamo(6, 0, 346);
				SetCamo(6, 1, 346);
				SetCamo(7, 0, 346);
				SetCamo(7, 1, 346);
				SetCamo(8, 0, 346);
				SetCamo(8, 1, 346);
				SetCamo(9, 0, 346);
				SetCamo(9, 1, 346);
			}

			if (current_item16 == "(VG) Atomic")
			{
				SetCamo(0, 0, 350);
				SetCamo(0, 1, 350);
				SetCamo(1, 0, 350);
				SetCamo(1, 1, 350);
				SetCamo(2, 0, 350);
				SetCamo(2, 1, 350);
				SetCamo(3, 0, 350);
				SetCamo(3, 1, 350);
				SetCamo(4, 0, 350);
				SetCamo(4, 1, 350);
				SetCamo(5, 0, 350);
				SetCamo(5, 1, 350);
				SetCamo(6, 0, 350);
				SetCamo(6, 1, 350);
				SetCamo(7, 0, 350);
				SetCamo(7, 1, 350);
				SetCamo(8, 0, 350);
				SetCamo(8, 1, 350);
				SetCamo(9, 0, 350);
				SetCamo(9, 1, 350);
			}

			if (current_item16 == "(VG) Fake Diamond")
			{
				SetCamo(0, 0, 349);
				SetCamo(0, 1, 349);
				SetCamo(1, 0, 349);
				SetCamo(1, 1, 349);
				SetCamo(2, 0, 349);
				SetCamo(2, 1, 349);
				SetCamo(3, 0, 349);
				SetCamo(3, 1, 349);
				SetCamo(4, 0, 349);
				SetCamo(4, 1, 349);
				SetCamo(5, 0, 349);
				SetCamo(5, 1, 349);
				SetCamo(6, 0, 349);
				SetCamo(6, 1, 349);
				SetCamo(7, 0, 349);
				SetCamo(7, 1, 349);
				SetCamo(8, 0, 349);
				SetCamo(8, 1, 349);
				SetCamo(9, 0, 349);
				SetCamo(9, 1, 349);
			}


			if (current_item16 == "(VG) Golden Viper")
			{
				SetCamo(0, 0, 402);
				SetCamo(0, 1, 402);
				SetCamo(1, 0, 402);
				SetCamo(1, 1, 402);
				SetCamo(2, 0, 402);
				SetCamo(2, 1, 402);
				SetCamo(3, 0, 402);
				SetCamo(3, 1, 402);
				SetCamo(4, 0, 402);
				SetCamo(4, 1, 402);
				SetCamo(5, 0, 402);
				SetCamo(5, 1, 402);
				SetCamo(6, 0, 402);
				SetCamo(6, 1, 402);
				SetCamo(7, 0, 402);
				SetCamo(7, 1, 402);
				SetCamo(8, 0, 402);
				SetCamo(8, 1, 402);
				SetCamo(9, 0, 402);
				SetCamo(9, 1, 402);
			}

			if (current_item16 == "(VG) Plague Diamond")
			{
				SetCamo(0, 0, 403);
				SetCamo(0, 1, 403);
				SetCamo(1, 0, 403);
				SetCamo(1, 1, 403);
				SetCamo(2, 0, 403);
				SetCamo(2, 1, 403);
				SetCamo(3, 0, 403);
				SetCamo(3, 1, 403);
				SetCamo(4, 0, 403);
				SetCamo(4, 1, 403);
				SetCamo(5, 0, 403);
				SetCamo(5, 1, 403);
				SetCamo(6, 0, 403);
				SetCamo(6, 1, 403);
				SetCamo(7, 0, 403);
				SetCamo(7, 1, 403);
				SetCamo(8, 0, 403);
				SetCamo(8, 1, 403);
				SetCamo(9, 0, 403);
				SetCamo(9, 1, 403);
			}

			if (current_item16 == "(VG) Dark Aether")
			{
				SetCamo(0, 0, 404);
				SetCamo(0, 1, 404);
				SetCamo(1, 0, 404);
				SetCamo(1, 1, 404);
				SetCamo(2, 0, 404);
				SetCamo(2, 1, 404);
				SetCamo(3, 0, 404);
				SetCamo(3, 1, 404);
				SetCamo(4, 0, 404);
				SetCamo(4, 1, 404);
				SetCamo(5, 0, 404);
				SetCamo(5, 1, 404);
				SetCamo(6, 0, 404);
				SetCamo(6, 1, 404);
				SetCamo(7, 0, 404);
				SetCamo(7, 1, 404);
				SetCamo(8, 0, 404);
				SetCamo(8, 1, 404);
				SetCamo(9, 0, 404);
				SetCamo(9, 1, 404);
			}

			if (current_item16 == "(CW) Cherry Blossom")
			{
				SetCamo(0, 0, 160);
				SetCamo(0, 1, 160);
				SetCamo(1, 0, 160);
				SetCamo(1, 1, 160);
				SetCamo(2, 0, 160);
				SetCamo(2, 1, 160);
				SetCamo(3, 0, 160);
				SetCamo(3, 1, 160);
				SetCamo(4, 0, 160);
				SetCamo(4, 1, 160);
				SetCamo(5, 0, 160);
				SetCamo(5, 1, 160);
				SetCamo(6, 0, 160);
				SetCamo(6, 1, 160);
				SetCamo(7, 0, 160);
				SetCamo(7, 1, 160);
				SetCamo(8, 0, 160);
				SetCamo(8, 1, 160);
				SetCamo(9, 0, 160);
				SetCamo(9, 1, 160);
			}

			if (current_item16 == "(MW) Acticamo")
			{
				SetCamo(0, 0, 2);
				SetCamo(0, 1, 2);
				SetCamo(1, 0, 2);
				SetCamo(1, 1, 2);
				SetCamo(2, 0, 2);
				SetCamo(2, 1, 2);
				SetCamo(3, 0, 2);
				SetCamo(3, 1, 2);
				SetCamo(4, 0, 2);
				SetCamo(4, 1, 2);
				SetCamo(5, 0, 2);
				SetCamo(5, 1, 2);
				SetCamo(6, 0, 2);
				SetCamo(6, 1, 2);
				SetCamo(7, 0, 2);
				SetCamo(7, 1, 2);
				SetCamo(8, 0, 2);
				SetCamo(8, 1, 2);
				SetCamo(9, 0, 2);
				SetCamo(9, 1, 2);
			}

			if (current_item16 == "(MW) Banded")
			{
				SetCamo(0, 0, 3);
				SetCamo(0, 1, 3);
				SetCamo(1, 0, 3);
				SetCamo(1, 1, 3);
				SetCamo(2, 0, 3);
				SetCamo(2, 1, 3);
				SetCamo(3, 0, 3);
				SetCamo(3, 1, 3);
				SetCamo(4, 0, 3);
				SetCamo(4, 1, 3);
				SetCamo(5, 0, 3);
				SetCamo(5, 1, 3);
				SetCamo(6, 0, 3);
				SetCamo(6, 1, 3);
				SetCamo(7, 0, 3);
				SetCamo(7, 1, 3);
				SetCamo(8, 0, 3);
				SetCamo(8, 1, 3);
				SetCamo(9, 0, 3);
				SetCamo(9, 1, 3);
			}

		}
		ImGui::Spacing();
		ImGui::Text("Set Operator:");
		ImGui::Spacing();
		static const char* items17[] = { "Doomsayer","High Bitrate","Crystallixed","Canabush","White Robot","Model T-1000","Samurai","Warlock","Racer","Gold Leaf","Ember","Bulldozer","Competitor Primary","Naga Ashen Scale","Gold Circurity",
		"CDL Champs Padmavati","Mythical Decay","Martial Arts","Extratestrial","Night Terror Power Ranger","Model T-800","Skull Guy","Battle Damaged","Hellacious","No living Tissue","Deaths Bride","Rambo","Easter Bunny","Ghost",
		"Weaver","Magma","Hyper Sonic","Loyal Samoyed","Dominion","Dishonored","Runeslayer","Paladin","Titan","Frank the Rabbit","Battle Oni","Midnigator","Ascension","Toxicity","Aurora Borealis","Diciple of the Dark Aether",
		"Sadface","Necro Queen","Ghost Face","Lumins","Ghost of War","Mecha Mesozoic","Judge Dredd","Comic Strip","Disciple of Mayhem" };
		static const char* current_item17 = "Select Operator";
		//ImGuiPP::CenterText("Camo Editor", 1, TRUE);
		//ImGui::Indent(10);

		if (ImGui::BeginCombo(xorstr_("##Operator"), current_item17)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items17); n++)
			{
				bool is_selected = (current_item17 == items17[n]); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(items17[n], is_selected))
					current_item17 = items17[n];
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		if (ImGui::Button(xorstr_("Set Operator"), ImVec2(100, 25)))
		{

			if (current_item17 == "Doomsayer")
			{
				setOperator("default_western", 2925);
				setOperator("default_eastern", 2925);
			}
			if (current_item17 == "High Bitrate")
			{
				setOperator("default_western", 2927);
				setOperator("default_eastern", 2927);
			}
			if (current_item17 == "Crystallixed")
			{
				setOperator("default_western", 1530);
				setOperator("default_eastern", 1530);
			}
			if (current_item17 == "Canabush")
			{
				setOperator("default_western", 1533);
				setOperator("default_eastern", 1533);
			}
			if (current_item17 == "White Robot")
			{
				setOperator("default_western", 2921);
				setOperator("default_eastern", 2921);
			}
			if (current_item17 == "Model T-1000")
			{
				setOperator("default_western", 2923);
				setOperator("default_eastern", 2923);
			}
			if (current_item17 == "Samurai")
			{
				setOperator("default_western", 1500);
				setOperator("default_eastern", 1500);
			}
			if (current_item17 == "Warlock")
			{
				setOperator("default_western", 1501);
				setOperator("default_eastern", 1501);
			}
			if (current_item17 == "Racer")
			{
				setOperator("default_western", 1097);
				setOperator("default_eastern", 1097);
			}
			if (current_item17 == "Gold Leaf")
			{
				setOperator("default_western", 1502);
				setOperator("default_eastern", 1502);
			}
			if (current_item17 == "Ember")
			{
				setOperator("default_western", 1504);
				setOperator("default_eastern", 1504);
			}
			if (current_item17 == "Bulldozer")
			{
				setOperator("default_western", 1124);
				setOperator("default_eastern", 1124);
			}
			if (current_item17 == "Competitor Primary")
			{
				setOperator("default_western", 1128);
				setOperator("default_eastern", 1128);
			}
			if (current_item17 == "Naga Ashen Scale")
			{
				setOperator("default_western", 1152);
				setOperator("default_eastern", 1152);
			}
			if (current_item17 == "Gold Circurity")
			{
				setOperator("default_western", 2919);
				setOperator("default_eastern", 2919);
			}
			if (current_item17 == "CDL Champs Padmavati")
			{
				setOperator("default_western", 2900);
				setOperator("default_eastern", 2900);
			}
			if (current_item17 == "Mythical Decay")
			{
				setOperator("default_western", 2902);
				setOperator("default_eastern", 2902);
			}
			if (current_item17 == "Martial Arts")
			{
				setOperator("default_western", 1489);
				setOperator("default_eastern", 1489);
			}
			if (current_item17 == "Extratestrial")
			{
				setOperator("default_western", 2904);
				setOperator("default_eastern", 2904);
			}
			if (current_item17 == "Night Terror Power Ranger")
			{
				setOperator("default_western", 2905);
				setOperator("default_eastern", 2905);
			}
			if (current_item17 == "Model T-800")
			{
				setOperator("default_western", 2908);
				setOperator("default_eastern", 2908);
			}
			if (current_item17 == "Skull Guy")
			{
				setOperator("default_western", 2915);
				setOperator("default_eastern", 2915);
			}
			if (current_item17 == "Battle Damaged")
			{
				setOperator("default_western", 2916);
				setOperator("default_eastern", 2916);
			}
			if (current_item17 == "Hellacious")
			{
				setOperator("default_western", 2804);
				setOperator("default_eastern", 2804);
			}
			if (current_item17 == "No living Tissue")
			{
				setOperator("default_western", 2917);
				setOperator("default_eastern", 2917);
			}
			if (current_item17 == "Deaths Bride")
			{
				setOperator("default_western", 1363);
				setOperator("default_eastern", 1363);
			}
			if (current_item17 == "Rambo")
			{
				setOperator("default_western", 1371);
				setOperator("default_eastern", 1371);
			}
			if (current_item17 == "Easter Bunny")
			{
				setOperator("default_western", 1297);
				setOperator("default_eastern", 1297);
			}
			if (current_item17 == "Ghost")
			{
				setOperator("default_western", 1284);
				setOperator("default_eastern", 1284);
			}
			if (current_item17 == "Weaver")
			{
				setOperator("default_western", 1372);
				setOperator("default_eastern", 1372);
			}
			if (current_item17 == "Magma")
			{
				setOperator("default_western", 1423);
				setOperator("default_eastern", 1423);
			}
			if (current_item17 == "Hyper Sonic")
			{
				setOperator("default_western", 1457);
				setOperator("default_eastern", 1457);
			}
			if (current_item17 == "Loyal Samoyed")
			{
				setOperator("default_western", 2914);
				setOperator("default_eastern", 2914);
			}
			if (current_item17 == "Dominion")
			{
				setOperator("default_western", 1674);
				setOperator("default_eastern", 1674);
			}
			if (current_item17 == "Dishonored")
			{
				setOperator("default_western", 1686);
				setOperator("default_eastern", 1686);
			}
			if (current_item17 == "Runeslayer")
			{
				setOperator("default_western", 2082);
				setOperator("default_eastern", 2082);
			}
			if (current_item17 == "Paladin")
			{
				setOperator("default_western", 1640);
				setOperator("default_eastern", 1640);
			}
			if (current_item17 == "Titan")
			{
				setOperator("default_western", 1803);
				setOperator("default_eastern", 1803);
			}
			if (current_item17 == "Frank the Rabbit")
			{
				setOperator("default_western", 1638);
				setOperator("default_eastern", 1638);
			}
			if (current_item17 == "Battle Oni")
			{
				setOperator("default_western", 1639);
				setOperator("default_eastern", 1639);
			}
			if (current_item17 == "Midnigator")
			{
				setOperator("default_western", 1758);
				setOperator("default_eastern", 1758);
			}
			if (current_item17 == "Ascension")
			{
				setOperator("default_western", 1596);
				setOperator("default_eastern", 1596);
			}
			if (current_item17 == "Toxicity")
			{
				setOperator("default_western", 1597);
				setOperator("default_eastern", 1597);
			}
			if (current_item17 == "Aurora Borealis")
			{
				setOperator("default_western", 1594);
				setOperator("default_eastern", 1594);
			}
			if (current_item17 == "Diciple of the Dark Aether")
			{
				setOperator("default_western", 1629);
				setOperator("default_eastern", 1629);
			}
			if (current_item17 == "Sadface")
			{
				setOperator("default_western", 1634);
				setOperator("default_eastern", 1634);
			}
			if (current_item17 == "Necro Queen")
			{
				setOperator("default_western", 1570);
				setOperator("default_eastern", 1570);
			}
			if (current_item17 == "Ghost Face")
			{
				setOperator("default_western", 1580);
				setOperator("default_eastern", 1580);
			}
			if (current_item17 == "Lumins")
			{
				setOperator("default_western", 1560);
				setOperator("default_eastern", 1560);
			}
			if (current_item17 == "Ghost of War")
			{
				setOperator("default_western", 1562);
				setOperator("default_eastern", 1562);
			}
			if (current_item17 == "Mecha Mesozoic")
			{
				setOperator("default_western", 2932);
				setOperator("default_eastern", 2932);
			}
			if (current_item17 == "Judge Dredd")
			{
				setOperator("default_western", 1547);
				setOperator("default_eastern", 1547);
			}
			if (current_item17 == "Comic Strip")
			{
				setOperator("default_western", 1550);
				setOperator("default_eastern", 1550);
			}
			if (current_item17 == "Disciple of Mayhem")
			{
				setOperator("default_western", 1557);
				setOperator("default_eastern", 1557);
			}
		}
		/*	static char test_string[128] = "";
			ImGui::InputTextWithHint(xorstr_("##TestWeapon"), xorstr_("Weapon Id"), test_string, ARRAYSIZE(test_string));
			ImGui::Dummy(ImVec2(0.0f, 1.0f));
			if (ImGui::Button(xorstr_("Test"), ImVec2(-5, 20)))
			{
				gameMode2 = 1;
				setOperator("default_western", atoi(test_string));

			}*/
		ImGui::EndGroup();
	}
}
void Draw2DRadarNormal(int xAxis, int yAxis, int width, int height, ImColor lineColor)
{
	bool out = false;
	Vector3 siz;
	siz.x = width;
	siz.y = height;
	Vector3 pos;
	pos.x = xAxis;
	pos.y = yAxis;
	float RadarCenterX = pos.x + (siz.x / 2);
	float RadarCenterY = pos.y + (siz.y / 2);
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->AddLine(ImVec2(pos.x, RadarCenterY), ImVec2(pos.x + siz.x, RadarCenterY), lineColor);
	window->DrawList->AddLine(ImVec2(RadarCenterX, pos.y), ImVec2(RadarCenterX, pos.y + siz.y), lineColor);
}
void RadarNormalUITheme()
{
	ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_PickerHueWheel);
	ImGuiStyle* Style = &ImGui::GetStyle();
	Style->FrameBorderSize = 0;
	Style->WindowRounding = 0;
	Style->WindowPadding = ImVec2(0, 0);
	Style->TabRounding = 0;
	Style->ScrollbarRounding = 0;
	Style->FramePadding = ImVec2(0, 0);
	Style->WindowTitleAlign = ImVec2(0.0f, 0.5f);
	Style->Colors[ImGuiCol_Border] = ImColor(ImVec4(0.06f, 0.06f, 0.06f, 0.54f));
	Style->Colors[ImGuiCol_WindowBg] = ImColor(ImVec4(0.06f, 0.06f, 0.06f, 0.54f));
}

// For 2D Radar.
void Do2DRadar()
{
	if (radarVars::bEnable2DRadar)
	{
		if (!radarVars::bSetRadarSize)
		{
			radarVars::iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
			radarVars::iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

			if (radarVars::iScreenWidth == 1920 && radarVars::iScreenHeight == 1080)
			{
				radarVars::v2SetRadarSize = ImVec2(265.f, 265.f);
			}
			else if (radarVars::iScreenWidth == 2560 && radarVars::iScreenHeight == 1440)
			{
				radarVars::v2SetRadarSize = ImVec2(350.f, 350.f);
			}
			else if (radarVars::iScreenWidth == 3840 && radarVars::iScreenHeight == 2160)
			{
				radarVars::v2SetRadarSize = ImVec2(435.f, 435.f);
			}
			radarVars::bSetRadarSize = true;
		}
		RadarNormalUITheme();
		ImGuiWindowFlags windowflags = 0;
		windowflags |= /*ImGuiWindowFlags_NoResize*/  ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar;
		ImGui::SetNextWindowSize(radarVars::v2SetRadarSize, ImGuiCond_Once);
		ImGui::SetNextWindowPos(ImVec2(radarVars::iScreenWidth / 1.35f, radarVars::iScreenHeight / 40.50f), ImGuiCond_Once);
		ImGui::Begin(xorstr_("RADARTEST"), 0, windowflags);
		radarVars::v2RadarNormalLocation = ImGui::GetWindowPos();
		radarVars::v2RadarNormalSize = ImGui::GetWindowSize();
		Draw2DRadarNormal(radarVars::v2RadarNormalLocation.x, radarVars::v2RadarNormalLocation.y, radarVars::v2RadarNormalSize.x, radarVars::v2RadarNormalSize.y, ImColor(255, 255, 255));
		ImGui::End();
	}
}
namespace g_menu
{
	void menu()
	{
		if (GetAsyncKeyState(VK_INSERT) & 0x1)
		{
			b_menu_open = !b_menu_open;

		}
		if (init_once)
		{
			//init_buffer();
			ImGui::SetNextWindowPos(ImVec2(200, 200));
			init_once = false;
		}
		EditorColorScheme::ApplyTheme();
		if (radarVars::bEnable2DRadar && !radarVars::transparent)
		{
			Do2DRadar();
		}

		if (b_menu_open && screenshot::visuals)
		{

			if (radarVars::bEnable2DRadar && radarVars::transparent)
			{
				Do2DRadar();
			}
			if (globals::b_rainbow)
			{
				EditorColorScheme::ApplyTheme2();


			}
			else
			{
				EditorColorScheme::ApplyTheme();
			}

			ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Always);

			ImGui::Begin(xorstr_("Chair"), &b_menu_open, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize);
			//ImGui::Begin(xorstr_("MENU"), &b_menu_open, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);
			ImGui::Checkbox(xorstr_("Enable Rainbow"), &globals::b_rainbow);

			int dwWidth = GetSystemMetrics(SM_CXSCREEN) / 3;
			int dwHeight = GetSystemMetrics(SM_CYSCREEN) / 2;
			ImGui::Dummy(ImVec2(0.0f, 1.0f));
			for (int i = 0; i < 25; i++)
			{
				ImGui::Spacing();
				ImGui::SameLine();
			}


			ImGui::Dummy(ImVec2(0.0f, 3.0f));
			ImGui::SetWindowPos(ImVec2(dwWidth * 2.0f, dwHeight * 0.2f), ImGuiCond_Once);
			{
				ImGui::BeginChild(xorstr_("##TABCHILD"), ImVec2(110, -1), true);
				{

					if (ImGui::Button(xorstr_("Visual"), ImVec2(95, 30))) { i_MenuTab = 0; }
					for (int i = 0; i < 15; i++)
					{
						ImGui::Spacing();
					}
					if (ImGui::Button(xorstr_("Aimbot"), ImVec2(95, 30))) { i_MenuTab = 1; }
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					{
						ImGui::SetTooltip(xorstr_("Use at your own Risk! High Ban Chance!"));
					}				
					for (int i = 0; i < 15; i++)
					{
						ImGui::Spacing();
					}
					if (ImGui::Button(xorstr_("Color Picker"), ImVec2(95, 30))) { i_MenuTab = 2; }
					for (int i = 0; i < 15; i++)
					{
						ImGui::Spacing();
					}

					if (ImGui::Button(xorstr_("MISC"), ImVec2(95, 30))) { i_MenuTab = 3; }
					for (int i = 0; i < 15; i++)
					{
						ImGui::Spacing();
					}
				/*	if (ImGui::Button(xorstr_("Loadout"), ImVec2(95, 30))) { i_MenuTab = 4; }
					for (int i = 0; i < 15; i++)
					{
						ImGui::Spacing();
					}*/
				
				}
				ImGui::EndChild();

				ImGui::SameLine();

				ImGui::BeginChild(xorstr_("##FEATURESCHILD"), ImVec2(-1, -1), false);
				{
					if (i_MenuTab == 0) Visual();
					if (i_MenuTab == 1) Aimbot();
					if (i_MenuTab == 2) ColorPicker();
					if (i_MenuTab == 3) Misc();
					if (i_MenuTab == 4) Loadout();
				}
				ImGui::EndChild();
			}

		}
	}
}