#pragma once
#include "stdafx.h"
#include "sdk.h"
#define M_PI   3.141592654f
namespace globals
{
	extern bool b_helmet;
	extern bool b_mid;
	extern bool b_rightfood;
	extern bool b_leftfood;
	extern bool b_head;
	extern bool b_chest;
	extern bool b_neck;
	extern bool b_tummy;
	extern bool b_pelvis;
	extern UINT64 clantagbase;
	extern uintptr_t Dvar_FindVarByName;
	extern uintptr_t Dvar_SetBoolInternal;
	extern uintptr_t Dvar_SetBoolByName;
	extern uintptr_t Dvar_SetFloat_Internal;
	extern uintptr_t Dvar_RegisterFloat;
	extern uintptr_t Dvar_SetInt_Internal;
	extern uintptr_t Dvar_SetIntByName;
	extern uintptr_t ddl_loadasset;
	extern uintptr_t ddl_getrootstate;
	extern uintptr_t ddl_getdllbuffer;
	extern uintptr_t ddl_movetoname;
	extern uintptr_t ddl_movetopath;
	extern uintptr_t ddl_setint;
	extern uintptr_t ddl_setstring;
	extern uintptr_t ddl_getint;
	extern uintptr_t a_parse;
	extern UINT64 unlockallbase;
	extern UINT64 checkbase;
	extern char UnlockBytes[5];
	extern bool b_unlockall;
	extern UINT64 uavbase;
	extern bool b_spread;
	extern float b_namesize;
	extern float b_distancesize;
	extern float f_fov;
	extern bool b_visible_only;
	extern bool b_box;
	extern bool b_line;
	extern bool b_skeleton;
	extern bool b_names;
	extern bool b_distance;
	extern bool b_visible;
	extern bool b_lock;
	extern bool b_crosshair;
	extern bool b_friendly;
	extern bool b_fov;
	extern bool b_recoil;
	extern bool b_unlock;
	extern bool b_tut;
	extern bool b_fog;
	extern bool b_in_game;
	extern bool b_health;
	extern bool local_is_alive;
	extern bool b_rapid_fire;
	extern bool b_skip_knocked;
	extern bool b_prediction;
	extern bool b_aim_point;
	extern bool is_aiming;
	extern bool target_bone;
	extern bool b_aimbot;
	extern bool b_UAV;
	extern bool b_scale_Fov;
	extern bool third_person;
	extern bool gamepad;
	extern bool b_rainbow;
	//extern int game_mode_opt;
	extern int aim_key;
	extern int bone_index;
	extern int lock_key;
	extern int max_distance;
	extern int aim_smooth;
	extern int max_player_count;
	extern int connecte_players;
	extern int local_team_id;
	extern int fire_speed;
	extern int call;
	extern int box_index;

	extern int player_index;
	extern float fov;
	extern Vector3 enemypos1;
	extern Vector3 enemypos2;
	extern Vector3 enemypos3;
	extern Vector3 localpos1;
	extern float f_fov_size;
	extern float aim_speed;
	extern float bullet_speed;
	//extern float gravity;
	extern const char* stance;
	extern const char* aim_lock_point[];
	extern const char* box_types[];
	const char* theme_choose[];
	//extern const char* aim_lock_key[];
	extern sdk::refdef_t* refdefadd;
	extern uintptr_t local_ptr;
	extern uintptr_t enemybase;
	extern float bullet_gravity;
	extern int theme;
}


namespace color
{
	extern ImColor Color;
	extern ImColor VisibleColorTeam;
	extern ImColor NotVisibleColorTeam;
	extern ImColor VisibleColorEnemy;
	extern ImColor NotVisibleColorEnemy;
	extern ImColor bfov;
	extern ImColor draw_crosshair;
	extern ImColor nameColor;
	extern ImColor dis_Color;
	extern ImColor healthbar;
}

namespace screenshot
{
	extern bool visuals ;
	extern bool* pDrawEnabled;
	extern	uint32_t screenshot_counter;
	extern	uint32_t  bit_blt_log;
	extern const char* bit_blt_fail;
	extern uintptr_t  bit_blt_anotherlog;

	extern	uint32_t	GdiStretchBlt_log;
	extern const char* GdiStretchBlt_fail;
	extern uintptr_t  GdiStretchBlt_anotherlog;

	extern	uintptr_t	texture_copy_log;




	extern uintptr_t virtualqueryaddr;
}


namespace loot
{
	extern ImColor lootcolor;
	extern bool name;
	extern bool distance;
	extern bool ar;
	extern bool smg;
	extern bool lmg;
	extern bool sniper;
	extern bool pistol;
	extern bool shotgun;
	extern bool ar_ammo;
	extern bool smg_ammo;
	extern bool sniper_ammo;
	extern bool shotgun_ammo;


	 
}