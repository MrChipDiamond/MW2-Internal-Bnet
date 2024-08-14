#include "globals.h"
#include "sdk.h"

namespace globals
{
	UINT64 uavbase = 0x132AE3D0 +0x130;
	UINT64 clantagbase = 0x4D81590;
	uintptr_t Dvar_FindVarByName = 0x3EB278D;
	uintptr_t Dvar_SetBoolInternal = 0x3E70250;
	uintptr_t Dvar_SetInt_Internal = 0x3E71F50;
	uintptr_t Dvar_SetBoolByName = 0x3BC60A0;
	uintptr_t Dvar_SetFloat_Internal = 0x3E70CE0;
	uintptr_t Dvar_RegisterFloat;
	uintptr_t Dvar_SetIntByName;
	uintptr_t ddl_loadasset = 0x3898D40;
	uintptr_t ddl_getrootstate = 0x61A13A0;
	uintptr_t ddl_getdllbuffer = 0x4DD6240; //
	uintptr_t ddl_movetoname =0x61A2020;
	uintptr_t ddl_movetopath =0x61A2040; //0x61A2100
	uintptr_t ddl_setint = 0x61A2880;
	uintptr_t ddl_setstring = 0x61A2690; //0x5D3C3E0
	uintptr_t ddl_getint =0x61A12C0;
	uintptr_t a_parse = 0x3C4F810;
	UINT64 checkbase;
	UINT64 unlockallbase = 0x54F96B0;
	char UnlockBytes[5] = "";
	bool b_helmet;
	bool b_mid;
	bool b_rightfood;
	bool b_leftfood;
	bool b_head;
	bool b_chest;
	bool b_neck;
	bool b_tummy;
	bool b_pelvis;
	bool b_spread;
	float f_fov;
	float b_namesize = 15.0f;
	float b_distancesize = 15.0f;
	bool b_visible_only;
	bool b_unlockall;
	bool b_box = true;
	bool b_line;
	bool b_skeleton;
	bool b_names = true;
	bool b_distance = true;
	bool b_visible;
	bool b_fov;
	bool b_lock;
	bool b_crosshair;
	bool b_friendly;
	bool b_recoil;
	int aim_key;
	bool b_in_game;
	bool b_health;
	bool local_is_alive;
	bool b_rapid_fire;
	bool b_skip_knocked;
	bool b_prediction;
	bool b_aim_point;
	bool is_aiming;
	bool target_bone;
	bool b_aimbot;
	bool b_UAV;
	bool b_scale_Fov;
	bool third_person;
	bool gamepad;
	bool b_rainbow;
	bool b_unlock;
	bool b_tut;
	bool b_fog;
	//int game_mode_opt = 0;

	int bone_index = 1; //0 ~ 3
	int box_index = 0; //0 ~ 3
	//int lock_key = 1; //0 ~ 3
	int max_distance = 250; //50 ~ 1000
	int aim_smooth = 5; // 1 ~ 30
	int max_player_count = 0;
	int connecte_players = 0;
	int local_team_id;
	int fire_speed = 40;
	int call = 0;

	int player_index;
	float fov = 1.2f;
	Vector3 localpos1;
	Vector3 enemypos1;
	Vector3 enemypos2;
	Vector3 enemypos3;
	float f_fov_size = 90.0f; // 0 ~ 1000
	float aim_speed = 1.f;
	float bullet_speed = 2402.f; // 1 ~ 3000
	float bullet_gravity = 5.f;

	//float gravity = 1.f;
	const char* stance;
	const char* aim_lock_point[] = { "Helmet", "Head", "Neck", "Chest" };
	const char* box_types[] = { "2D Corner Box", "2D Box" };
	const char* theme_choose[] = { "Dark Mode","White Mode"};
	//const char* aim_lock_key[] = { "Left Button", "Right Button", "M4", "M5" };
	uintptr_t local_ptr;
	uintptr_t enemybase;
	sdk::refdef_t* refdefadd;

	int theme = 0;
}

namespace color
{
	ImColor Color{ 255,255,255,255 };
	ImColor VisibleColorTeam{ 0.f, 0.f, 1.f, 1.f };
	ImColor NotVisibleColorTeam{ 0.f, 0.75f, 1.f, 1.f };
	ImColor VisibleColorEnemy{ 0.33f, 0.97f, 0.35f, 1.00f };
	ImColor NotVisibleColorEnemy{ 0.97f, 0.01f, 0.01f, 1.00f };
	ImColor bfov{ 0.97f, 0.01f, 0.01f, 1.00f };
	ImColor draw_crosshair{ 0.f, 0.75f, 1.f, 1.f };
	ImColor nameColor{ 255,255,0,255 };
	ImColor dis_Color{ 255,255,0,255 };
	ImColor healthbar{ 0.f, 0.f, 1.f, 1.f };
}

namespace screenshot
{
	 bool visuals = true;
	 bool* pDrawEnabled = nullptr;
		uint32_t screenshot_counter = 0;
		uint32_t  bit_blt_log = 0;
	 const char* bit_blt_fail;
	 uintptr_t  bit_blt_anotherlog;

		uint32_t	GdiStretchBlt_log = 0;
	 const char* GdiStretchBlt_fail;
	 uintptr_t  GdiStretchBlt_anotherlog;

	uintptr_t	texture_copy_log = 0;




	 uintptr_t virtualqueryaddr = 0;
}


namespace loot
{
	ImColor lootcolor{ 0.f, 0.75f, 1.f, 1.f };
	bool name;
	bool distance;
	bool ar;
	bool smg;
	bool lmg;
	bool sniper;
	bool pistol;
	bool shotgun;

	bool ar_ammo;
	bool smg_ammo;
	bool sniper_ammo;
	bool shotgun_ammo;



}