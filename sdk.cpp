#include "stdafx.h"
#include "sdk.h"
#include "xor.hpp"
#include "lazyimporter.h"
#include "memory.h"
#include <map>
#include "defs.h"
#include "globals.h"
#include "xorstr.hpp"
#include "weapon.h"
#pragma comment(lib, "user32.lib")
#define DEBASE(a) ((size_t)a - (size_t)(unsigned long long)GetModuleHandleA(NULL))

uintptr_t dwProcessBase;
uint64_t backup = 0, Online_Loot__GetItemQuantity = 0, stackFix = 0;
NTSTATUS(*NtContinue)(PCONTEXT threadContext, BOOLEAN raiseAlert) = nullptr;

DWORD64 resolveRelativeAddress(DWORD64 instr, DWORD offset, DWORD instrSize) {
	return instr == 0ui64 ? 0ui64 : (instr + instrSize + *(int*)(instr + offset));
}

bool compareByte(const char* pData, const char* bMask, const char* szMask) {
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)
			return false;
	return (*szMask) == NULL;
}

DWORD64 findPattern(DWORD64 dwAddress, DWORD64 dwLen, const char* bMask, const char* szMask) {
	DWORD length = (DWORD)strlen(szMask);
	for (DWORD i = 0; i < dwLen - length; i++)
		if (compareByte((const char*)(dwAddress + i), bMask, szMask))
			return (DWORD64)(dwAddress + i);
	return 0ui64;
}

LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
	if (pExceptionInfo && pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION)
	{
		if (pExceptionInfo->ContextRecord->R11 == 0xDEEDBEEF89898989)
		{
			pExceptionInfo->ContextRecord->R11 = backup;

			if (pExceptionInfo->ContextRecord->Rip > Online_Loot__GetItemQuantity && pExceptionInfo->ContextRecord->Rip < (Online_Loot__GetItemQuantity + 0x1000))
			{
				pExceptionInfo->ContextRecord->Rip = stackFix;
				pExceptionInfo->ContextRecord->Rax = 1;
			}
			NtContinue(pExceptionInfo->ContextRecord, 0);
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}


namespace process
{
	HWND hwnd;

	BOOL CALLBACK EnumWindowCallBack(HWND hWnd, LPARAM lParam)
	{
		DWORD dwPid = 0;
		GetWindowThreadProcessId(hWnd, &dwPid);
		if (dwPid == lParam)
		{
			hwnd = hWnd;
			return FALSE;
		}
		return TRUE;
	}

	HWND get_process_window()
	{
		if (hwnd)
			return hwnd;

		EnumWindows(EnumWindowCallBack, GetCurrentProcessId());

		if (hwnd == NULL)
			Exit();

		return hwnd;
	}
}
namespace radarVars
{
	bool transparent;
	bool bEnable2DRadar;
	bool bSetRadarSize = false;
	int iScreenWidth = 0;
	int iScreenHeight = 0;
	ImVec2 v2RadarNormalLocation;
	ImVec2 v2RadarNormalSize;
	ImVec2 v2SetRadarSize;
	ImDrawList* Draw;
}
namespace g_data
{
	uintptr_t base;
	uintptr_t Peb;
	HWND hWind;
	uintptr_t unlocker;
	uintptr_t ddl_loadasset;
	uintptr_t ddl_getrootstate;
	uintptr_t ddl_getdllbuffer;
	uintptr_t ddl_movetoname;
	uintptr_t ddl_setint;
	uintptr_t Dvar_FindVarByName;
	uintptr_t Dvar_SetBoolInternal;
	uintptr_t Dvar_SetInt_Internal;
	uintptr_t Dvar_SetFloat_Internal;
	uintptr_t Camo_Offset_Auto_Test;
	

	uintptr_t Clantag_auto;

	uintptr_t a_parse;
	uintptr_t ddl_setstring;
	uintptr_t ddl_movetopath;
	uintptr_t ddlgetInth;
	uintptr_t visible_base;
	QWORD current_visible_offset;
	QWORD cached_visible_base;
	QWORD last_visible_offset;
	uintptr_t cached_client_t = 0;

	bool MemCompare(const BYTE* bData, const BYTE* bMask, const char* szMask) {
		for (; *szMask; ++szMask, ++bData, ++bMask) {
			if (*szMask == 'x' && *bData != *bMask) {
				return false;
			}
		}
		return (*szMask == NULL);
	}
	uintptr_t PatternScanEx(uintptr_t start, uintptr_t size, const char* sig, const char* mask)
	{
		BYTE* data = new BYTE[size];
		SIZE_T bytesRead;

		(iat(memcpy).get()(data, (LPVOID)start, size));
		//ReadProcessMemory(hProcess, (LPVOID)start, data, size, &bytesRead);

		for (uintptr_t i = 0; i < size; i++)
		{
			if (MemCompare((const BYTE*)(data + i), (const BYTE*)sig, mask)) {
				return start + i;
			}
		}
		delete[] data;
		return NULL;
	}

	uintptr_t FindOffset(uintptr_t start, uintptr_t size, const char* sig, const char* mask, uintptr_t base_offset, uintptr_t pre_base_offset, uintptr_t rindex, bool addRip =true)
	{
		auto address = PatternScanEx(start, size, sig, mask) + rindex;
		if (!address)
			return 0;
		auto ret = pre_base_offset + *reinterpret_cast<int32_t*>(address + base_offset);

		if (addRip)
		{
			ret = ret + address;
			if (ret)
				return (ret - base);
		}

		return ret;
	}

	void init()
	{
		base = (uintptr_t)(iat(GetModuleHandleA).get()(xorstr_("cod.exe")));
	
		
		hWind = process::get_process_window();
		
		Peb = __readgsqword(0x60);
		/*globals::unlockallbase = PatternScanEx(base + 0x5000000, 0x4F00000, xorstr_("\x48\x8D\x0D\x00\x00\xD3\x00\xE8\x00\x00\x00\x00\x8B\xC8\xC5\xF0\x57\xC9\xC4\xE1\xF3\x2A\xC9\x48\x8B\xCF\xE8\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\xB8\x00\x00\x00\x00\x48\x83\xC4\x20\x5F\xC3"), xorstr_("xxx??x?x????xxxxxxxxxxxxxxx????xxxx?x????xxxxxx")) - base;
		globals::unlockallbase = globals::unlockallbase + 0x7;
		memcpy((BYTE*)(globals::UnlockBytes), (BYTE*)g_data::base + globals::unlockallbase, 5);*/
     /*   globals::checkbase = FindOffset(base + 0x2000000, 0x1F00000, xorstr_("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x84\xC0\x75\x10\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x84\xC0\x74\x0F"), xorstr_("xxx????x????xxxxxxx????x????xxxx"),3,7,0);
		globals::checkbase = globals::checkbase +0x4;*/
	}
}
template<typename T> inline auto readMemory(uintptr_t ptr) noexcept -> T {
	if (is_bad_ptr(ptr)) {
		//DEBUG_INFO("Attempted to read invalid memory at {:#x}", ptr);
		return {};
	}
	return *reinterpret_cast<T*>(ptr);
}
template<typename T> inline auto writeMemory(uintptr_t ptr, T value) noexcept -> T {
	if (is_bad_ptr(ptr)) {
		//DEBUG_INFO("Attempted to read invalid memory at {:#x}", ptr);
		return {};
	}
	return *reinterpret_cast<T*>(ptr) = value;
}
dvar_s* Dvar_FindVarByName(const char* dvarName)
{
	//[48 83 EC 48 49 8B C8 E8 ?? ?? ?? ?? + 0x7] resolve call.
	return reinterpret_cast<dvar_s * (__fastcall*)(const char* dvarName)>(g_data::base + globals::Dvar_FindVarByName)(dvarName);
}

uintptr_t Dvar_SetBool_Internal(dvar_s* a1, bool a2)
{
	//E8 ? ? ? ? 80 3D ? ? ? ? ? 4C 8D 35 ? ? ? ? 74 43 33 D2 F7 05 ? ? ? ? ? ? ? ? 76 2E
	return reinterpret_cast<std::ptrdiff_t(__fastcall*)(dvar_s * a1, bool a2)>(g_data::base + globals::Dvar_SetBoolInternal)(a1, a2);
}

uintptr_t Dvar_SetInt_Internal(dvar_s* a1, unsigned int a2)
{
	//40 53 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 0F B6 41 09 48 8B D9
	return reinterpret_cast<std::ptrdiff_t(__fastcall*)(dvar_s * a1, unsigned int a2)>(g_data::base + globals::Dvar_SetInt_Internal)(a1, a2);
}

uintptr_t Dvar_SetBoolByName(const char* dvarName, bool value)
{
	//"48 89 ? ? ? 57 48 81 EC ? ? ? ? 0F B6 ? 48 8B"
	int64(__fastcall * Dvar_SetBoolByName_t)(const char* dvarName, bool value); //48 89 5C 24 ? 57 48 81 EC ? ? ? ? 0F B6 DA
	return reinterpret_cast<decltype(Dvar_SetBoolByName_t)>(globals::Dvar_SetBoolByName)(dvarName, value);
}

uintptr_t Dvar_SetFloat_Internal(dvar_s* a1, float a2)
{
	//E8 ? ? ? ? 45 0F 2E C8 RESOLVE CALL
	return reinterpret_cast<std::ptrdiff_t(__fastcall*)(dvar_s * a1, float a2)>(g_data::base + globals::Dvar_SetFloat_Internal)(a1, a2);
}

uintptr_t Dvar_SetIntByName(const char* dvarname, int value)
{
	uintptr_t(__fastcall * Dvar_SetIntByName_t)(const char* dvarname, int value); //48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 8B DA 48 8B F9
	return reinterpret_cast<decltype(Dvar_SetIntByName_t)>(globals::Dvar_SetIntByName)(dvarname, value);
}

__int64 Com_DDL_LoadAsset(__int64 file) {
	uintptr_t address = g_data::base + globals::ddl_loadasset;
	return ((__int64 (*)(__int64))address)(file);
}

__int64 DDL_GetRootState(__int64 state, __int64 file) {
	uintptr_t address = g_data::base + globals::ddl_getrootstate;
	return ((__int64 (*)(__int64, __int64))address)(state, file);
}

bool CL_PlayerData_GetDDLBuffer(__int64 context, int controllerindex, int stats_source, unsigned int statsgroup) {
	uintptr_t address = g_data::base + globals::ddl_getdllbuffer;
	return ((bool (*)(__int64, int, int, unsigned int))address)(context, controllerindex, stats_source, statsgroup);
}
bool ParseShit(const char* a, const char** b, const int c, int* d)
{
	uintptr_t address = g_data::base + globals::a_parse;
	return ((bool (*)(const char* a, const char** b, const int c, int* d))address)(a, b, c, d);

}
char DDL_MoveToPath(__int64* fromState, __int64* toState, int depth, const char** path) {
	uintptr_t address = g_data::base + globals::ddl_movetopath;
	return ((char (*)(__int64* fromState, __int64* toState, int depth, const char** path))address)(fromState, toState, depth, path);

}
__int64 DDL_MoveToName(__int64 fstate, __int64 tstate, __int64 path) {
	uintptr_t address = g_data::base + globals::ddl_movetoname;
	return ((__int64 (*)(__int64, __int64, __int64))address)(fstate, tstate, path);
}

char DDL_SetInt(__int64 fstate, __int64 context, unsigned int value) {
	uintptr_t address = g_data::base + globals::ddl_setint;
	return ((char (*)(__int64, __int64, unsigned int))address)(fstate, context, value);
}
int DDL_GetInt(__int64* fstate, __int64* context) {
	uintptr_t address = g_data::base + globals::ddl_getint;
	return ((int (*)(__int64*, __int64*))address)(fstate, context);
}
char DDL_SetString(__int64 fstate, __int64 context, const char* value) {
	uintptr_t address = g_data::base + globals::ddl_setstring;
	return ((char (*)(__int64, __int64, const char*))address)(fstate, context, value);
}
char DDL_SetInt2(__int64* fstate, __int64* context, int value) {
	uintptr_t address = g_data::base + globals::ddl_setint;
	return ((char (*)(__int64*, __int64*, unsigned int))address)(fstate, context, value);
}
namespace sdk
{
	const DWORD nTickTime = 64;//64 ms
	bool bUpdateTick = false;
	std::map<DWORD, velocityInfo_t> velocityMap;




	void enable_uav()
	{

		
		auto uavptr = *(uint64_t*)(g_data::base + globals::uavbase);
		if (uavptr != 0)
		{
			
			*(BYTE*)(uavptr + 0x430) = 2;
			
		}

	
	}
	float valuesRecoilBackup[962][60];
	float valuesSpreadBackup[962][22];
	void no_spread()
	{
		WeaponCompleteDefArr* weapons = (WeaponCompleteDefArr*)(g_data::base + bones::weapon_definitions);
		if (globals::b_spread && in_game)
		{

			for (int count = 0; count < 962; count++)
			{
				if (weapons->weaponCompleteDefArr[count]->weapDef)
				{

					valuesSpreadBackup[count][0] = weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadDuckedDecay;
					valuesSpreadBackup[count][1] = weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadProneDecay;
					valuesSpreadBackup[count][2] = weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadSprintDecay;
					valuesSpreadBackup[count][3] = weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadInAirDecay;
					valuesSpreadBackup[count][4] = weapons->weaponCompleteDefArr[count]->weapDef->fHipReticleSidePos;
					valuesSpreadBackup[count][5] = weapons->weaponCompleteDefArr[count]->weapDef->fAdsIdleAmount;
					valuesSpreadBackup[count][6] = weapons->weaponCompleteDefArr[count]->weapDef->fHipIdleAmount;
					valuesSpreadBackup[count][7] = weapons->weaponCompleteDefArr[count]->weapDef->adsIdleSpeed;
					valuesSpreadBackup[count][8] = weapons->weaponCompleteDefArr[count]->weapDef->hipIdleSpeed;
					valuesSpreadBackup[count][9] = weapons->weaponCompleteDefArr[count]->weapDef->fIdleCrouchFactor;
					valuesSpreadBackup[count][10] = weapons->weaponCompleteDefArr[count]->weapDef->fIdleProneFactor;
					valuesSpreadBackup[count][11] = weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxPitch;
					valuesSpreadBackup[count][12] = weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxYaw;
					valuesSpreadBackup[count][13] = weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxPitch;
					valuesSpreadBackup[count][14] = weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxYaw;
					valuesSpreadBackup[count][15] = weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpStartTime;
					valuesSpreadBackup[count][16] = weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpTime;
					valuesSpreadBackup[count][17] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMin;
					valuesSpreadBackup[count][18] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMax;
					valuesSpreadBackup[count][19] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadDecayRate;
					valuesSpreadBackup[count][20] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadFireAdd;
					valuesSpreadBackup[count][21] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadTurnAdd;
					weapons->weaponCompleteDefArr[22]->weapDef->ballisticInfo.muzzleVelocity;
					// WRITE

					weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadDuckedDecay = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadProneDecay = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadSprintDecay = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadInAirDecay = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fHipReticleSidePos = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fAdsIdleAmount = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fHipIdleAmount = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleSpeed = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->hipIdleSpeed = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fIdleCrouchFactor = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fIdleProneFactor = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxPitch = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxYaw = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxPitch = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxYaw = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpStartTime = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpTime = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMin = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMax = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadDecayRate = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadFireAdd = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadTurnAdd = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->ballisticInfo.muzzleVelocity = 0.0f;
				}
			}
		}
		else
		{
			for (int count = 0; count < 962; count++)
			{
				if (weapons->weaponCompleteDefArr[count]->weapDef)
				{
					weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadDuckedDecay = valuesSpreadBackup[count][0];
					weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadProneDecay = valuesSpreadBackup[count][1];
					weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadSprintDecay = valuesSpreadBackup[count][2];
					weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadInAirDecay = valuesSpreadBackup[count][3];
					weapons->weaponCompleteDefArr[count]->weapDef->fHipReticleSidePos = valuesSpreadBackup[count][4];
					weapons->weaponCompleteDefArr[count]->weapDef->fAdsIdleAmount = valuesSpreadBackup[count][5];
					weapons->weaponCompleteDefArr[count]->weapDef->fHipIdleAmount = valuesSpreadBackup[count][6];
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleSpeed = valuesSpreadBackup[count][7];
					weapons->weaponCompleteDefArr[count]->weapDef->hipIdleSpeed = valuesSpreadBackup[count][8];
					weapons->weaponCompleteDefArr[count]->weapDef->fIdleCrouchFactor = valuesSpreadBackup[count][9];
					weapons->weaponCompleteDefArr[count]->weapDef->fIdleProneFactor = valuesSpreadBackup[count][10];
					weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxPitch = valuesSpreadBackup[count][11];
					weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxYaw = valuesSpreadBackup[count][12];
					weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxPitch = valuesSpreadBackup[count][13];
					weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxYaw = valuesSpreadBackup[count][14];
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpStartTime = valuesSpreadBackup[count][15];
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpTime = valuesSpreadBackup[count][16];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMin = valuesSpreadBackup[count][17];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMax = valuesSpreadBackup[count][18];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadDecayRate = valuesSpreadBackup[count][19];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadFireAdd = valuesSpreadBackup[count][20];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadTurnAdd = valuesSpreadBackup[count][21];

				}
			}
		}
	}
	void no_recoil()
	{
		uint64_t characterInfo_ptr = get_client_info();

		if (characterInfo_ptr)
		{
			
			uint64_t r12 = characterInfo_ptr;
			r12 += player_info::recoil_offset;
			uint64_t rsi = r12 + 0x4;
			DWORD edx = *(uint64_t*)(r12 + 0xC);
			DWORD ecx = (DWORD)r12;
			ecx ^= edx;
			DWORD eax = (DWORD)((uint64_t)ecx + 0x2);
			eax *= ecx;
			ecx = (DWORD)rsi;
			ecx ^= edx;
			DWORD udZero = eax;
			
			eax = (DWORD)((uint64_t)ecx + 0x2);
			eax *= ecx;
			DWORD lrZero = eax;
			*(DWORD*)(r12) = udZero;
			*(DWORD*)(rsi) = lrZero;

		}
	}
	void unlockall()
	{
		HMODULE ntdll = GetModuleHandleA(xorstr_("ntdll"));
		NtContinue = (decltype(NtContinue))GetProcAddress(ntdll, xorstr_("NtContinue"));

		void(*RtlAddVectoredExceptionHandler)(LONG First, PVECTORED_EXCEPTION_HANDLER Handler) = (decltype(RtlAddVectoredExceptionHandler))GetProcAddress(ntdll, xorstr_("RtlAddVectoredExceptionHandler"));
		RtlAddVectoredExceptionHandler(0, TopLevelExceptionHandler);

		uint64_t FindOnline_Loot__GetItemQuantity = findPattern(g_data::base + 0x1000000, 0xF000000, xorstr_("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\xC8\xC5\xF0\x57\xC9\xC4\xE1\xF3\x2A\xC9"), xorstr_("xxx????x????xxxxxxxxxxx"));

		if (FindOnline_Loot__GetItemQuantity)
		{
			Online_Loot__GetItemQuantity = resolveRelativeAddress(FindOnline_Loot__GetItemQuantity + 7, 1, 5);

			uint64_t FindDvar = findPattern(Online_Loot__GetItemQuantity, 0x1000, xorstr_("\x4C\x8B\x1D"), xorstr_("xxx"));
			uint64_t FindStackFix = findPattern(Online_Loot__GetItemQuantity, 0x2000, xorstr_("\xE8\x00\x00\x00\x00\x00\x8B\x00\x00\x00\x00\x8B"), xorstr_("x?????x????x"));

			if (FindStackFix)
			{
				stackFix = (FindStackFix + 5);

				backup = *(uint64_t*)resolveRelativeAddress(FindDvar, 3, 7);
				*(uint64_t*)resolveRelativeAddress(FindDvar, 3, 7) = 0xDEEDBEEF89898989;
			}
		}
	}
	uintptr_t _get_player(int i)
	{
		auto cl_info_base = get_client_info_base();

		if (is_bad_ptr(cl_info_base))return 0;
		
		
			auto base_address = *(uintptr_t*)(cl_info_base);
			if (is_bad_ptr(base_address))return 0;

				return sdk::get_client_info_base() + (i * player_info::size);

	}
	bool in_game()
	{
		auto gameMode = *(int*)(g_data::base + game_mode);
		return  gameMode > 1;
	}

	int get_game_mode()
	{
		return *(int*)(g_data::base + game_mode + 0x4);
	}

	int get_max_player_count()
	{
		return *(int*)(g_data::base + game_mode);
	}

	Vector3 _get_pos(uintptr_t address)
	{
		auto local_pos_ptr = *(uintptr_t*)((uintptr_t)address + player_info::position_ptr);

		if (local_pos_ptr)
		{
			return *(Vector3*)(local_pos_ptr + 0x40);
		}
		return Vector3{};
	}

	uint32_t _get_index(uintptr_t address)
	{
		auto cl_info_base = get_client_info_base();

		if (is_bad_ptr(cl_info_base))return 0;

		return ((uintptr_t)address - cl_info_base) / player_info::size;
	
		
	}

	BYTE _team_id(uintptr_t address)    {

		return *(BYTE*)((uintptr_t)address + player_info::team_id);
	}




	bool _is_visible(uintptr_t address)
	{
		if (IsValidPtr<uintptr_t>(&g_data::visible_base))
		{
			uint64_t VisibleList = *(uint64_t*)(g_data::visible_base + 0x108);
			if (is_bad_ptr( VisibleList))
				return false;

			uint64_t rdx = VisibleList + (_get_index(address) * 9 + 0x14E) * 8;
			if (is_bad_ptr(rdx))
				return false;

			DWORD VisibleFlags = (rdx + 0x10) ^ (*(DWORD*)(rdx + 0x14));
			if (is_bad_ptr(VisibleFlags))
				return false;

			DWORD v511 = VisibleFlags * (VisibleFlags + 2);
			if (!v511)
				return false;

			BYTE VisibleFlags1 = *(DWORD*)(rdx + 0x10) ^ v511 ^ BYTE1(v511);
			if (VisibleFlags1 == 3) {
				return true;
			}
		}
		return false;
	}

	Vector3 RotatePoint(Vector3 EntityPos, Vector3 LocalPlayerPos, int posX, int posY, int sizeX, int sizeY, float angle, float zoom, bool* viewCheck)
	{
		float r_1, r_2;
		float x_1, y_1;

		r_1 = -(EntityPos.y - LocalPlayerPos.y);
		r_2 = EntityPos.x - LocalPlayerPos.x;
		float Yaw = angle - 90.0f;

		float yawToRadian = Yaw * (float)(M_PI / 180.0F);
		x_1 = (float)(r_2 * (float)cos((double)(yawToRadian)) - r_1 * sin((double)(yawToRadian))) / 20;
		y_1 = (float)(r_2 * (float)sin((double)(yawToRadian)) + r_1 * cos((double)(yawToRadian))) / 20;

		*viewCheck = y_1 < 0;

		x_1 *= zoom;
		y_1 *= zoom;

		int sizX = sizeX / 2;
		int sizY = sizeY / 2;

		x_1 += sizX;
		y_1 += sizY;

		if (x_1 < 5)
			x_1 = 5;

		if (x_1 > sizeX - 5)
			x_1 = sizeX - 5;

		if (y_1 < 5)
			y_1 = 5;

		if (y_1 > sizeY - 5)
			y_1 = sizeY - 5;


		x_1 += posX;
		y_1 += posY;


		return Vector3(x_1, y_1, 0);
	}

	// For 2D Radar.
	ImVec2 Rotate(const ImVec2& center, const ImVec2& pos, float angle)
	{
		ImVec2 Return;
		angle *= -(M_PI / 180.0f);
		float cos_theta = cos(angle);
		float sin_theta = sin(angle);
		Return.x = (cos_theta * (pos[0] - center[0]) - sin_theta * (pos[1] - center[1])) + center[0];
		Return.y = (sin_theta * (pos[0] - center[0]) + cos_theta * (pos[1] - center[1])) + center[1];
		return Return;
	}

	// For 2D Radar.
	void DrawEntity(const ImVec2& pos, float angle, DWORD color)
	{
		constexpr long up_offset = 7;
		constexpr long lr_offset = 5;

		for (int FillIndex = 0; FillIndex < 5; ++FillIndex)
		{
			ImVec2 up_pos(pos.x, pos.y - up_offset + FillIndex);
			ImVec2 left_pos(pos.x - lr_offset + FillIndex, pos.y + up_offset - FillIndex);
			ImVec2 right_pos(pos.x + lr_offset - FillIndex, pos.y + up_offset - FillIndex);

			ImVec2 p0 = Rotate(pos, up_pos, angle);
			ImVec2 p1 = Rotate(pos, left_pos, angle);
			ImVec2 p2 = Rotate(pos, right_pos, angle);

			ImGui::GetOverlayDrawList()->AddLine(p0, p1, FillIndex == 0 ? 0xFF010000 : color, 1.0f);
			ImGui::GetOverlayDrawList()->AddLine(p1, p2, FillIndex == 0 ? 0xFF010000 : color, 1.0f);
			ImGui::GetOverlayDrawList()->AddLine(p2, p0, FillIndex == 0 ? 0xFF010000 : color, 1.0f);
		}
	}

	// For 2D Radar.
	void DrawEntity(sdk::player_t* local_entity, sdk::player_t* entity, ImVec2 window_pos, int distance)
	{
		if (distance <= 105)
		{
			const float local_rotation = local_entity->get_rotation();
			float rotation = entity->get_rotation();

			rotation = rotation - local_rotation;

			if (rotation < 0)
				rotation = 360.0f - std::fabs(rotation);
			DrawEntity(ImVec2(window_pos.x, window_pos.y), rotation, ImColor(255, 0, 0));
		}
	}

	// For 2D Radar.
	void DrawRadarPoint(sdk::player_t* Player, sdk::player_t* Local, int xAxis, int yAxis, int width, int height, ImDrawList* draw, int distatce)
	{
		bool out = false;
		Vector3 siz;
		siz.x = width;
		siz.y = height;
		Vector3 pos;
		pos.x = xAxis;
		pos.y = yAxis;
		bool ck = false;

		Vector3 single = RotatePoint(Player->get_pos(), Local->get_pos(), pos.x, pos.y, siz.x, siz.y, Local->get_rotation(), 1.f, &ck);

		DrawEntity(Local, Player, ImVec2(single.x, single.y), distatce);
	}
	uint64_t get_client_info()
	{
		auto baseModuleAddr = g_data::base;
		auto Peb = __readgsqword(0x60);
		uint64_t rax = baseModuleAddr, rbx = baseModuleAddr, rcx = baseModuleAddr, rdx = baseModuleAddr, rdi = baseModuleAddr, rsi = baseModuleAddr, r8 = baseModuleAddr, r9 = baseModuleAddr, r10 = baseModuleAddr, r11 = baseModuleAddr, r12 = baseModuleAddr, r13 = baseModuleAddr, r14 = baseModuleAddr, r15 = baseModuleAddr;
		rbx = *(uintptr_t*)(baseModuleAddr + 0x12E0BB58);
		if (!rbx)
			return rbx;
		rdx = ~Peb;              //mov rdx, gs:[rax]
		rax = rbx;              //mov rax, rbx
		rax >>= 0xB;            //shr rax, 0x0B
		rbx ^= rax;             //xor rbx, rax
		rax = rbx;              //mov rax, rbx
		rax >>= 0x16;           //shr rax, 0x16
		rbx ^= rax;             //xor rbx, rax
		rax = rbx;              //mov rax, rbx
		rax >>= 0x2C;           //shr rax, 0x2C
		rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
		rbx ^= rax;             //xor rbx, rax
		rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
		rcx ^= *(uintptr_t*)(baseModuleAddr + 0xA97010E);               //xor rcx, [0x00000000085A6000]
		rbx -= rdx;             //sub rbx, rdx
		rax = 0xACE6CAF426D17B99;               //mov rax, 0xACE6CAF426D17B99
		rcx = ~rcx;             //not rcx
		rbx *= *(uintptr_t*)(rcx + 0x13);               //imul rbx, [rcx+0x13]
		rbx *= rax;             //imul rbx, rax
		rbx += rdx;             //add rbx, rdx
		rax = rbx;              //mov rax, rbx
		rax >>= 0x9;            //shr rax, 0x09
		rbx ^= rax;             //xor rbx, rax
		rax = rbx;              //mov rax, rbx
		rax >>= 0x12;           //shr rax, 0x12
		rbx ^= rax;             //xor rbx, rax
		rax = rbx;              //mov rax, rbx
		rax >>= 0x24;           //shr rax, 0x24
		rbx ^= rax;             //xor rbx, rax
		g_data::cached_client_t = rbx;
		return g_data::cached_client_t;
	}
	uint64_t get_client_info_base()
	{
		auto baseModuleAddr = g_data::base;
		auto Peb = __readgsqword(0x60);
		
		uint64_t rax = baseModuleAddr, rbx = baseModuleAddr, rcx = baseModuleAddr, rdx = baseModuleAddr, rdi = baseModuleAddr, rsi = baseModuleAddr, r8 = baseModuleAddr, r9 = baseModuleAddr, r10 = baseModuleAddr, r11 = baseModuleAddr, r12 = baseModuleAddr, r13 = baseModuleAddr, r14 = baseModuleAddr, r15 = baseModuleAddr;
		r8 = *(uintptr_t*)(get_client_info() + 0x10ec90);
		if (!r8)
			return r8;
		rbx = Peb;              //mov rbx, gs:[rax]
		rax = rbx;              //mov rax, rbx
		rax >>= 0x1D;           //shr rax, 0x1D
		rax &= 0xF;
		switch (rax) {
		case 0:
		{
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                //mov r10, [0x00000000081B73CD]
			r8 += rbx;              //add r8, rbx
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = ~rax;             //not rax
			r8 *= *(uintptr_t*)(rax + 0x19);                //imul r8, [rax+0x19]
			rax = r8;               //mov rax, r8
			rax >>= 0x4;            //shr rax, 0x04
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x8;            //shr rax, 0x08
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x10;           //shr rax, 0x10
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x20;           //shr rax, 0x20
			r8 ^= rax;              //xor r8, rax
			rax = 0x93873E8211D79EB1;               //mov rax, 0x93873E8211D79EB1
			r8 *= rax;              //imul r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0xA;            //shr rax, 0x0A
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x14;           //shr rax, 0x14
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x28;           //shr rax, 0x28
			r8 ^= rax;              //xor r8, rax
			rax = baseModuleAddr;           //lea rax, [0xFFFFFFFFFD846CA3]
			r8 += rax;              //add r8, rax
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			rcx = rax;              //mov rcx, rax
			uintptr_t RSP_0xFFFFFFFFFFFFFF90;
			RSP_0xFFFFFFFFFFFFFF90 = baseModuleAddr + 0x40B0E6EF;           //lea rax, [0x000000003E355946] : RBP+0xFFFFFFFFFFFFFF90
			rcx ^= RSP_0xFFFFFFFFFFFFFF90;          //xor rcx, [rbp-0x70]
			rcx += rax;             //add rcx, rax
			rax = baseModuleAddr + 0xAE74;          //lea rax, [0xFFFFFFFFFD851A50]
			r8 += rax;              //add r8, rax
			r8 += rcx;              //add r8, rcx
			return r8;
		}
		case 1:
		{
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                //mov r10, [0x00000000081B6D15]
			rax = 0xCA11C716243EF320;               //mov rax, 0xCA11C716243EF320
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0xD;            //shr rax, 0x0D
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x1A;           //shr rax, 0x1A
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x34;           //shr rax, 0x34
			r8 ^= rax;              //xor r8, rax
			rax = 0x9701358D55016045;               //mov rax, 0x9701358D55016045
			r8 *= rax;              //imul r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = ~rax;             //not rax
			r8 *= *(uintptr_t*)(rax + 0x19);                //imul r8, [rax+0x19]
			rax = r8;               //mov rax, r8
			rax >>= 0x7;            //shr rax, 0x07
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0xE;            //shr rax, 0x0E
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x1C;           //shr rax, 0x1C
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x38;           //shr rax, 0x38
			rax ^= r8;              //xor rax, r8
			r8 = rax + rbx * 2;             //lea r8, [rax+rbx*2]
			return r8;
		}
		case 2:
		{
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                //mov r10, [0x00000000081B68D9]
			rax = r8;               //mov rax, r8
			rax >>= 0x1B;           //shr rax, 0x1B
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x36;           //shr rax, 0x36
			r8 ^= rax;              //xor r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = ~rax;             //not rax
			r8 *= *(uintptr_t*)(rax + 0x19);                //imul r8, [rax+0x19]
			rax = r8;               //mov rax, r8
			rax >>= 0x25;           //shr rax, 0x25
			r8 ^= rax;              //xor r8, rax
			rax = 0x6318266AECA5809D;               //mov rax, 0x6318266AECA5809D
			r8 *= rax;              //imul r8, rax
			r8 ^= rbx;              //xor r8, rbx
			rax = baseModuleAddr;           //lea rax, [0xFFFFFFFFFD8464FD]
			r8 ^= rax;              //xor r8, rax
			rcx = rbx;              //mov rcx, rbx
			rcx = ~rcx;             //not rcx
			rax = baseModuleAddr + 0x382DF655;              //lea rax, [0x0000000035B25BB9]
			rcx ^= rax;             //xor rcx, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x22;           //shr rax, 0x22
			r8 ^= rax;              //xor r8, rax
			r8 += rcx;              //add r8, rcx
			return r8;
		}
		case 3:
		{
			r11 = baseModuleAddr + 0xDFF5;          //lea r11, [0xFFFFFFFFFD8542A4]
			r9 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                 //mov r9, [0x00000000081B63BB]
			rax = 0xFA656DEB4E31CC2D;               //mov rax, 0xFA656DEB4E31CC2D
			r8 *= rax;              //imul r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x24;           //shr rax, 0x24
			r8 ^= rax;              //xor r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = ~rax;             //not rax
			r8 *= *(uintptr_t*)(rax + 0x19);                //imul r8, [rax+0x19]
			rax = r11;              //mov rax, r11
			rax = ~rax;             //not rax
			rax ^= rbx;             //xor rax, rbx
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x8;            //shr rax, 0x08
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x10;           //shr rax, 0x10
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x20;           //shr rax, 0x20
			r8 ^= rax;              //xor r8, rax
			rax = 0xD802B16C1E0C4F0D;               //mov rax, 0xD802B16C1E0C4F0D
			r8 += rax;              //add r8, rax
			rax = 0x270E95F379D182D5;               //mov rax, 0x270E95F379D182D5
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x1F;           //shr rax, 0x1F
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x3E;           //shr rax, 0x3E
			r8 ^= rax;              //xor r8, rax
			return r8;
		}
		case 4:
		{
			r11 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                //mov r11, [0x00000000081B5F25]
			rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
			rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
			rcx = rbx;              //mov rcx, rbx
			rdx ^= r11;             //xor rdx, r11
			rdx = ~rdx;             //not rdx
			rdx = *(uintptr_t*)(rdx + 0x19);                //mov rdx, [rdx+0x19]
			rax = baseModuleAddr + 0x2C7F;          //lea rax, [0xFFFFFFFFFD848722]
			rcx *= rax;             //imul rcx, rax
			r8 *= rdx;              //imul r8, rdx
			rax = baseModuleAddr + 0x3142F877;              //lea rax, [0x000000002EC7530B]
			r8 += rcx;              //add r8, rcx
			r8 += rax;              //add r8, rax
			rax = 0x178FC4957BE7717;                //mov rax, 0x178FC4957BE7717
			r8 *= rax;              //imul r8, rax
			rax = 0x1262623EA5881C03;               //mov rax, 0x1262623EA5881C03
			r8 += rax;              //add r8, rax
			rcx = rbx + 0x1;                //lea rcx, [rbx+0x01]
			rax = baseModuleAddr + 0xC589;          //lea rax, [0xFFFFFFFFFD85222A]
			rcx *= rax;             //imul rcx, rax
			rax = 0x30E9E4362A70CE1B;               //mov rax, 0x30E9E4362A70CE1B
			r8 += rax;              //add r8, rax
			r8 += rcx;              //add r8, rcx
			rax = r8;               //mov rax, r8
			rax >>= 0x28;           //shr rax, 0x28
			r8 ^= rax;              //xor r8, rax
			return r8;
		}
		case 5:
		{
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                //mov r10, [0x00000000081B5A85]
			r11 = baseModuleAddr;           //lea r11, [0xFFFFFFFFFD8457E5]
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rax = rbx;              //mov rax, rbx
			rax -= r11;             //sub rax, r11
			rcx ^= r10;             //xor rcx, r10
			rax -= 0x4A33C193;              //sub rax, 0x4A33C193
			rcx = ~rcx;             //not rcx
			rax ^= r8;              //xor rax, r8
			r8 = baseModuleAddr;            //lea r8, [0xFFFFFFFFFD8457A8]
			rax -= r8;              //sub rax, r8
			r8 = rax + 0xffffffff9fa11e31;          //lea r8, [rax-0x605EE1CF]
			r8 += rbx;              //add r8, rbx
			r8 *= *(uintptr_t*)(rcx + 0x19);                //imul r8, [rcx+0x19]
			rax = 0x344845A5E1811F80;               //mov rax, 0x344845A5E1811F80
			rax -= rbx;             //sub rax, rbx
			r8 += rax;              //add r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x13;           //shr rax, 0x13
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x26;           //shr rax, 0x26
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x7;            //shr rax, 0x07
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0xE;            //shr rax, 0x0E
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x1C;           //shr rax, 0x1C
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x38;           //shr rax, 0x38
			r8 ^= rax;              //xor r8, rax
			rax = 0x37FCC244760FDB3;                //mov rax, 0x37FCC244760FDB3
			r8 *= rax;              //imul r8, rax
			return r8;
		}
		case 6:
		{
			r9 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                 //mov r9, [0x00000000081B5605]
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = ~rax;             //not rax
			r8 *= *(uintptr_t*)(rax + 0x19);                //imul r8, [rax+0x19]
			rax = baseModuleAddr;           //lea rax, [0xFFFFFFFFFD845357]
			r8 += rax;              //add r8, rax
			rax = 0xBEC89CF840406765;               //mov rax, 0xBEC89CF840406765
			r8 *= rax;              //imul r8, rax
			rax = 0x391CFCBD3EE83409;               //mov rax, 0x391CFCBD3EE83409
			r8 -= rax;              //sub r8, rax
			r8 ^= rbx;              //xor r8, rbx
			rax = 0xA832E2D8E8C5C197;               //mov rax, 0xA832E2D8E8C5C197
			r8 ^= rax;              //xor r8, rax
			r8 += rbx;              //add r8, rbx
			rax = r8;               //mov rax, r8
			rax >>= 0x10;           //shr rax, 0x10
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x20;           //shr rax, 0x20
			r8 ^= rax;              //xor r8, rax
			return r8;
		}
		case 7:
		{
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                //mov r10, [0x00000000081B50BB]
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = ~rax;             //not rax
			r8 *= *(uintptr_t*)(rax + 0x19);                //imul r8, [rax+0x19]
			rax = baseModuleAddr + 0x1049D007;              //lea rax, [0x000000000DCE1AAF]
			rax -= rbx;             //sub rax, rbx
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x1A;           //shr rax, 0x1A
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x34;           //shr rax, 0x34
			r8 ^= rax;              //xor r8, rax
			rax = baseModuleAddr;           //lea rax, [0xFFFFFFFFFD844E10]
			rax += 0xE7FF;          //add rax, 0xE7FF
			rax += rbx;             //add rax, rbx
			r8 ^= rax;              //xor r8, rax
			rax = 0xF13779F42172E8F5;               //mov rax, 0xF13779F42172E8F5
			r8 *= rax;              //imul r8, rax
			rax = rbx;              //mov rax, rbx
			uintptr_t RSP_0xFFFFFFFFFFFFFF80;
			RSP_0xFFFFFFFFFFFFFF80 = baseModuleAddr + 0x18F6D119;           //lea rax, [0x00000000167B2082] : RBP+0xFFFFFFFFFFFFFF80
			rax ^= RSP_0xFFFFFFFFFFFFFF80;          //xor rax, [rbp-0x80]
			r8 -= rax;              //sub r8, rax
			rax = 0x32DBCE65F2BBC207;               //mov rax, 0x32DBCE65F2BBC207
			r8 ^= rax;              //xor r8, rax
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			uintptr_t RSP_0x78;
			RSP_0x78 = baseModuleAddr + 0x61CE;             //lea rax, [0xFFFFFFFFFD84B125] : RSP+0x78
			rax += RSP_0x78;                //add rax, [rsp+0x78]
			r8 ^= rax;              //xor r8, rax
			return r8;
		}
		case 8:
		{
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                //mov r10, [0x00000000081B4AC0]
			rcx = baseModuleAddr + 0x5E64B02C;              //lea rcx, [0x000000005BE8F93D]
			rax = rbx;              //mov rax, rbx
			rax ^= rcx;             //xor rax, rcx
			r8 -= rax;              //sub r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x9;            //shr rax, 0x09
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x12;           //shr rax, 0x12
			r8 ^= rax;              //xor r8, rax
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rax = r8;               //mov rax, r8
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rax >>= 0x24;           //shr rax, 0x24
			rcx ^= r10;             //xor rcx, r10
			r8 ^= rax;              //xor r8, rax
			rcx = ~rcx;             //not rcx
			rax = baseModuleAddr;           //lea rax, [0xFFFFFFFFFD84484B]
			r8 += rax;              //add r8, rax
			rax = 0x4076808BAB8E08D5;               //mov rax, 0x4076808BAB8E08D5
			r8 *= *(uintptr_t*)(rcx + 0x19);                //imul r8, [rcx+0x19]
			r8 *= rax;              //imul r8, rax
			rax = 0x25B9C191F2971614;               //mov rax, 0x25B9C191F2971614
			r8 ^= rax;              //xor r8, rax
			r8 += rbx;              //add r8, rbx
			return r8;
		}
		case 9:
		{
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                //mov r10, [0x00000000081B46ED]
			rax = rbx;              //mov rax, rbx
			uintptr_t RSP_0xFFFFFFFFFFFFFF80;
			RSP_0xFFFFFFFFFFFFFF80 = baseModuleAddr + 0x1027E517;           //lea rax, [0x000000000DAC2AB2] : RBP+0xFFFFFFFFFFFFFF80
			rax ^= RSP_0xFFFFFFFFFFFFFF80;          //xor rax, [rbp-0x80]
			r8 -= rax;              //sub r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = ~rax;             //not rax
			r8 *= *(uintptr_t*)(rax + 0x19);                //imul r8, [rax+0x19]
			rax = 0xCCD296279A2A0C35;               //mov rax, 0xCCD296279A2A0C35
			r8 *= rax;              //imul r8, rax
			rax = 0x68CF128039F2D136;               //mov rax, 0x68CF128039F2D136
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x9;            //shr rax, 0x09
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x12;           //shr rax, 0x12
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rcx = baseModuleAddr + 0x1B74E16C;              //lea rcx, [0x0000000018F92445]
			rax >>= 0x24;           //shr rax, 0x24
			rcx = ~rcx;             //not rcx
			r8 ^= rax;              //xor r8, rax
			rcx ^= rbx;             //xor rcx, rbx
			r8 += rcx;              //add r8, rcx
			rax = baseModuleAddr;           //lea rax, [0xFFFFFFFFFD8442C2]
			r8 += rax;              //add r8, rax
			rax = baseModuleAddr;           //lea rax, [0xFFFFFFFFFD84435A]
			r8 -= rax;              //sub r8, rax
			return r8;
		}
		case 10:
		{
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                //mov r10, [0x00000000081B411D]
			rcx = rbx;              //mov rcx, rbx
			rax = baseModuleAddr;           //lea rax, [0xFFFFFFFFFD843B51]
			rcx -= rax;             //sub rcx, rax
			rax = r8;               //mov rax, r8
			r8 = baseModuleAddr;            //lea r8, [0xFFFFFFFFFD843B44]
			rax -= r8;              //sub rax, r8
			r8 = rcx + 0xffffffffc2bbca64;          //lea r8, [rcx-0x3D44359C]
			r8 ^= rax;              //xor r8, rax
			rax = 0xDF76BA8F4165C7E3;               //mov rax, 0xDF76BA8F4165C7E3
			r8 *= rax;              //imul r8, rax
			rax = 0x2DD2F4C38B2336ED;               //mov rax, 0x2DD2F4C38B2336ED
			r8 -= rax;              //sub r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x14;           //shr rax, 0x14
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x28;           //shr rax, 0x28
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x4;            //shr rax, 0x04
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x8;            //shr rax, 0x08
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x10;           //shr rax, 0x10
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x20;           //shr rax, 0x20
			r8 ^= rax;              //xor r8, rax
			rax = 0x8840989061DA058B;               //mov rax, 0x8840989061DA058B
			r8 *= rax;              //imul r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = ~rax;             //not rax
			r8 *= *(uintptr_t*)(rax + 0x19);                //imul r8, [rax+0x19]
			return r8;
		}
		case 11:
		{
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                //mov r10, [0x00000000081B3BE2]
			rax = 0xD4DBAFED29842F53;               //mov rax, 0xD4DBAFED29842F53
			r8 *= rax;              //imul r8, rax
			rax = 0xA6403642F692DBDE;               //mov rax, 0xA6403642F692DBDE
			r8 ^= rax;              //xor r8, rax
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r10;             //xor rcx, r10
			rax = baseModuleAddr;           //lea rax, [0xFFFFFFFFFD843637]
			rax += 0x770A;          //add rax, 0x770A
			rcx = ~rcx;             //not rcx
			rax += rbx;             //add rax, rbx
			r8 ^= rax;              //xor r8, rax
			r8 *= *(uintptr_t*)(rcx + 0x19);                //imul r8, [rcx+0x19]
			rax = r8;               //mov rax, r8
			rax >>= 0xB;            //shr rax, 0x0B
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x16;           //shr rax, 0x16
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x2C;           //shr rax, 0x2C
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x1B;           //shr rax, 0x1B
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x36;           //shr rax, 0x36
			r8 ^= rax;              //xor r8, rax
			r8 ^= rbx;              //xor r8, rbx
			rax = 0x54600854BBFD341B;               //mov rax, 0x54600854BBFD341B
			r8 *= rax;              //imul r8, rax
			return r8;
		}
		case 12:
		{
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                //mov r10, [0x00000000081B36F9]
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			uintptr_t RSP_0xFFFFFFFFFFFFFF90;
			RSP_0xFFFFFFFFFFFFFF90 = baseModuleAddr + 0x3CAE5B88;           //lea rax, [0x000000003A3290D3] : RBP+0xFFFFFFFFFFFFFF90
			rax *= RSP_0xFFFFFFFFFFFFFF90;          //imul rax, [rbp-0x70]
			r8 ^= rax;              //xor r8, rax
			rcx = baseModuleAddr + 0x1C3B;          //lea rcx, [0xFFFFFFFFFD8450DE]
			rcx = ~rcx;             //not rcx
			rcx ^= rbx;             //xor rcx, rbx
			rax = r8;               //mov rax, r8
			rax >>= 0x23;           //shr rax, 0x23
			r8 ^= rax;              //xor r8, rax
			rax = 0x60067CBF5B99DB2D;               //mov rax, 0x60067CBF5B99DB2D
			r8 -= rcx;              //sub r8, rcx
			r8 ^= rax;              //xor r8, rax
			rax = baseModuleAddr;           //lea rax, [0xFFFFFFFFFD84335E]
			rax += 0x501736C6;              //add rax, 0x501736C6
			rax += rbx;             //add rax, rbx
			r8 += rax;              //add r8, rax
			rax = 0xDAF1BB5756765A17;               //mov rax, 0xDAF1BB5756765A17
			r8 *= rax;              //imul r8, rax
			rax = 0x9D32BE3C6298C00C;               //mov rax, 0x9D32BE3C6298C00C
			r8 ^= rax;              //xor r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = ~rax;             //not rax
			r8 *= *(uintptr_t*)(rax + 0x19);                //imul r8, [rax+0x19]
			return r8;
		}
		case 13:
		{
			r9 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                 //mov r9, [0x00000000081B321E]
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			uintptr_t RSP_0xFFFFFFFFFFFFFF80;
			RSP_0xFFFFFFFFFFFFFF80 = baseModuleAddr + 0x50F2FC40;           //lea rax, [0x000000004E772D94] : RBP+0xFFFFFFFFFFFFFF80
			rax *= RSP_0xFFFFFFFFFFFFFF80;          //imul rax, [rbp-0x80]
			r8 ^= rax;              //xor r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = ~rax;             //not rax
			r8 *= *(uintptr_t*)(rax + 0x19);                //imul r8, [rax+0x19]
			rax = 0x429EA005A5262E35;               //mov rax, 0x429EA005A5262E35
			r8 *= rax;              //imul r8, rax
			rax = 0xF29F41A489A8EF78;               //mov rax, 0xF29F41A489A8EF78
			r8 ^= rax;              //xor r8, rax
			rax = baseModuleAddr + 0x2E1DAC7E;              //lea rax, [0x000000002BA1D994]
			rax -= rbx;             //sub rax, rbx
			r8 += rax;              //add r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x20;           //shr rax, 0x20
			r8 ^= rax;              //xor r8, rax
			rax = 0x96602C352CB8C395;               //mov rax, 0x96602C352CB8C395
			r8 *= rax;              //imul r8, rax
			r8 ^= rbx;              //xor r8, rbx
			return r8;
		}
		case 14:
		{
			r9 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                 //mov r9, [0x00000000081B2CBA]
			r8 += rbx;              //add r8, rbx
			rax = baseModuleAddr + 0x3124;          //lea rax, [0xFFFFFFFFFD84582D]
			rax = ~rax;             //not rax
			rax += rbx;             //add rax, rbx
			r8 ^= rax;              //xor r8, rax
			rax = baseModuleAddr + 0x8F4C;          //lea rax, [0xFFFFFFFFFD84B7EA]
			rax = ~rax;             //not rax
			rax -= rbx;             //sub rax, rbx
			r8 += rax;              //add r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = ~rax;             //not rax
			r8 *= *(uintptr_t*)(rax + 0x19);                //imul r8, [rax+0x19]
			rax = r8;               //mov rax, r8
			rax >>= 0x25;           //shr rax, 0x25
			r8 ^= rax;              //xor r8, rax
			rax = 0x97EC55AC5BB54B67;               //mov rax, 0x97EC55AC5BB54B67
			r8 ^= rax;              //xor r8, rax
			rax = 0x313534C0905939A7;               //mov rax, 0x313534C0905939A7
			r8 += rax;              //add r8, rax
			rax = 0x48AE73CF4425EEDF;               //mov rax, 0x48AE73CF4425EEDF
			r8 *= rax;              //imul r8, rax
			return r8;
		}
		case 15:
		{
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97014B);                //mov r10, [0x00000000081B2776]
			rax = r8;               //mov rax, r8
			rax >>= 0x6;            //shr rax, 0x06
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0xC;            //shr rax, 0x0C
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x18;           //shr rax, 0x18
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x30;           //shr rax, 0x30
			r8 ^= rax;              //xor r8, rax
			r12 = baseModuleAddr;           //lea r12, [0xFFFFFFFFFD842232]
			rax = 0x62982A4003AD2E71;               //mov rax, 0x62982A4003AD2E71
			r8 *= rax;              //imul r8, rax
			rax = 0xA43D1FE8C1BECCEA;               //mov rax, 0xA43D1FE8C1BECCEA
			r8 += rbx;              //add r8, rbx
			r8 += rax;              //add r8, rax
			r8 += r12;              //add r8, r12
			r8 ^= rbx;              //xor r8, rbx
			rax = 0x20FAA945A888AC30;               //mov rax, 0x20FAA945A888AC30
			r8 += rax;              //add r8, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = ~rax;             //not rax
			r8 *= *(uintptr_t*)(rax + 0x19);                //imul r8, [rax+0x19]
			rax = r8;               //mov rax, r8
			rax >>= 0x15;           //shr rax, 0x15
			r8 ^= rax;              //xor r8, rax
			rax = r8;               //mov rax, r8
			rax >>= 0x2A;           //shr rax, 0x2A
			r8 ^= rax;              //xor r8, rax
			return r8;
		}
		}
	}
	uint64_t get_bone_ptr()
	{
		auto baseModuleAddr = g_data::base;
		auto Peb = __readgsqword(0x60);
	
	
		uint64_t rax = baseModuleAddr, rbx = baseModuleAddr, rcx = baseModuleAddr, rdx = baseModuleAddr, rdi = baseModuleAddr, rsi = baseModuleAddr, r8 = baseModuleAddr, r9 = baseModuleAddr, r10 = baseModuleAddr, r11 = baseModuleAddr, r12 = baseModuleAddr, r13 = baseModuleAddr, r14 = baseModuleAddr, r15 = baseModuleAddr;
		rdx = *(uintptr_t*)(baseModuleAddr + 0xDF18C88);
		if (!rdx)
			return rdx;
		r11 = ~Peb;              //mov r11, gs:[rax]
		rax = r11;              //mov rax, r11
		rax = _rotl64(rax, 0x2A);               //rol rax, 0x2A
		rax &= 0xF;
		switch (rax) {
		case 0:
		{
			r14 = baseModuleAddr + 0x2B426A6A;              //lea r14, [0x0000000028AD0477]
			r12 = baseModuleAddr + 0xA2AE;          //lea r12, [0xFFFFFFFFFD6B3CAC]
			r9 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                 //mov r9, [0x0000000008019BA6]
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			rax = r11;              //mov rax, r11
			rax ^= r14;             //xor rax, r14
			rdx += rax;             //add rdx, rax
			rax = 0xE6A00EE721FE924D;               //mov rax, 0xE6A00EE721FE924D
			rdx ^= rax;             //xor rdx, rax
			rax = 0xF85FC6FE6CA3AFC1;               //mov rax, 0xF85FC6FE6CA3AFC1
			rdx ^= rax;             //xor rdx, rax
			rax = r11;              //mov rax, r11
			rax *= r12;             //imul rax, r12
			rdx += rax;             //add rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0xC;            //shr rax, 0x0C
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x18;           //shr rax, 0x18
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x30;           //shr rax, 0x30
			rdx ^= rax;             //xor rdx, rax
			rax = 0x6C481D59AA6ED33B;               //mov rax, 0x6C481D59AA6ED33B
			rdx *= rax;             //imul rdx, rax
			rax = baseModuleAddr + 0x825E;          //lea rax, [0xFFFFFFFFFD6B189E]
			rax -= r11;             //sub rax, r11
			rdx ^= rax;             //xor rdx, rax
			return rdx;
		}
		case 1:
		{
			rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6A94F4]
			r12 = baseModuleAddr + 0x5A8CB88B;              //lea r12, [0x0000000057F74D70]
			r13 = baseModuleAddr + 0xD274;          //lea r13, [0xFFFFFFFFFD6B674A]
			r9 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                 //mov r9, [0x0000000008019673]
			rax = 0x14DEAB663A2F858C;               //mov rax, 0x14DEAB663A2F858C
			rdx ^= rax;             //xor rdx, rax
			rax = r11;              //mov rax, r11
			rax = ~rax;             //not rax
			rax ^= r13;             //xor rax, r13
			rdx -= rax;             //sub rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			rdx ^= r11;             //xor rdx, r11
			rdx ^= r12;             //xor rdx, r12
			rdx += rbx;             //add rdx, rbx
			rax = rdx;              //mov rax, rdx
			rax >>= 0x12;           //shr rax, 0x12
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x24;           //shr rax, 0x24
			rdx ^= rax;             //xor rdx, rax
			rax = 0x9817F566E7F8A9A7;               //mov rax, 0x9817F566E7F8A9A7
			rdx *= rax;             //imul rdx, rax
			rax = 0x5DE28D862F51DDE7;               //mov rax, 0x5DE28D862F51DDE7
			rdx ^= rax;             //xor rdx, rax
			return rdx;
		}
		case 2:
		{
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                //mov r10, [0x00000000080190F8]
			rax = baseModuleAddr + 0x9D06;          //lea rax, [0xFFFFFFFFFD6B28C6]
			rax = ~rax;             //not rax
			rax *= r11;             //imul rax, r11
			rdx += rax;             //add rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			rax = 0x143E7D3E66AE510E;               //mov rax, 0x143E7D3E66AE510E
			rdx -= rax;             //sub rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0xB;            //shr rax, 0x0B
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x16;           //shr rax, 0x16
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x2C;           //shr rax, 0x2C
			rdx ^= rax;             //xor rdx, rax
			rdx -= r11;             //sub rdx, r11
			rax = rdx;              //mov rax, rdx
			rax >>= 0xF;            //shr rax, 0x0F
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1E;           //shr rax, 0x1E
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x3C;           //shr rax, 0x3C
			rdx ^= rax;             //xor rdx, rax
			rax = 0xD8F1F2C2B7DC93C5;               //mov rax, 0xD8F1F2C2B7DC93C5
			rdx *= rax;             //imul rdx, rax
			rax = 0xDBD25BE7153998F1;               //mov rax, 0xDBD25BE7153998F1
			rdx ^= rax;             //xor rdx, rax
			return rdx;
		}
		case 3:
		{
			r9 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                 //mov r9, [0x0000000008018BD0]
			rax = baseModuleAddr + 0x1C7135F7;              //lea rax, [0x0000000019DBBE4F]
			rax = ~rax;             //not rax
			rax += r11;             //add rax, r11
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1B;           //shr rax, 0x1B
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x36;           //shr rax, 0x36
			rdx ^= rax;             //xor rdx, rax
			rax = 0x4A9DBE2EF039C329;               //mov rax, 0x4A9DBE2EF039C329
			rdx += rax;             //add rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x21;           //shr rax, 0x21
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x26;           //shr rax, 0x26
			rdx ^= rax;             //xor rdx, rax
			rax = 0xD6FDCAE8D382135;                //mov rax, 0xD6FDCAE8D382135
			rdx *= rax;             //imul rdx, rax
			rax = 0x232C7F4AC999301;                //mov rax, 0x232C7F4AC999301
			rdx -= rax;             //sub rdx, rax
			return rdx;
		}
		case 4:
		{
			rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6A8567]
			r12 = baseModuleAddr + 0x2F51754D;              //lea r12, [0x000000002CBBFAA5]
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                //mov r10, [0x0000000008018723]
			rcx = r12;              //mov rcx, r12
			rcx = ~rcx;             //not rcx
			rax = r11;              //mov rax, r11
			rax = ~rax;             //not rax
			rcx *= rax;             //imul rcx, rax
			rcx -= r11;             //sub rcx, r11
			rcx -= rbx;             //sub rcx, rbx
			rdx += rcx;             //add rdx, rcx
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			rax = 0x3858421A0A895A00;               //mov rax, 0x3858421A0A895A00
			rdx += rax;             //add rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x7;            //shr rax, 0x07
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0xE;            //shr rax, 0x0E
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1C;           //shr rax, 0x1C
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x38;           //shr rax, 0x38
			rdx ^= rax;             //xor rdx, rax
			rax = 0xC8F45F536ED35183;               //mov rax, 0xC8F45F536ED35183
			rdx *= rax;             //imul rdx, rax
			rax = 0x6ED03529B216505A;               //mov rax, 0x6ED03529B216505A
			rdx ^= rax;             //xor rdx, rax
			return rdx;
		}
		case 5:
		{
			r14 = baseModuleAddr + 0x1816;          //lea r14, [0xFFFFFFFFFD6A98F8]
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                //mov r10, [0x00000000080182A6]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x8;            //shr rax, 0x08
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x10;           //shr rax, 0x10
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x20;           //shr rax, 0x20
			rdx ^= rax;             //xor rdx, rax
			rax = 0xBCDDEB3B16150D77;               //mov rax, 0xBCDDEB3B16150D77
			rdx *= rax;             //imul rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x17;           //shr rax, 0x17
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x2E;           //shr rax, 0x2E
			rdx ^= rax;             //xor rdx, rax
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rax = r14;              //mov rax, r14
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rax = ~rax;             //not rax
			rax -= r11;             //sub rax, r11
			rcx ^= r10;             //xor rcx, r10
			rdx += rax;             //add rdx, rax
			rcx = _byteswap_uint64(rcx);            //bswap rcx
			rdx *= *(uintptr_t*)(rcx + 0x7);                //imul rdx, [rcx+0x07]
			rdx += r11;             //add rdx, r11
			rax = 0x9DD94CC9005941A8;               //mov rax, 0x9DD94CC9005941A8
			rdx ^= rax;             //xor rdx, rax
			rdx += r11;             //add rdx, r11
			return rdx;
		}
		case 6:
		{
			rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6A7C79]
			r12 = baseModuleAddr + 0x5413DB21;              //lea r12, [0x00000000517E578B]
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                //mov r10, [0x0000000008017E13]
			rcx = r12;              //mov rcx, r12
			rcx = ~rcx;             //not rcx
			rax = r11;              //mov rax, r11
			rax = ~rax;             //not rax
			rcx *= rax;             //imul rcx, rax
			rdx ^= rcx;             //xor rdx, rcx
			uintptr_t RSP_0x48;
			RSP_0x48 = 0x3F992D238EA9462D;          //mov rax, 0x3F992D238EA9462D : RSP+0x48
			rdx *= RSP_0x48;                //imul rdx, [rsp+0x48]
			rdx -= rbx;             //sub rdx, rbx
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rdx += 0xFFFFFFFFC079DBC0;              //add rdx, 0xFFFFFFFFC079DBC0
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rdx += r11;             //add rdx, r11
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			rax = baseModuleAddr + 0x870E;          //lea rax, [0xFFFFFFFFFD6B01A9]
			rax = ~rax;             //not rax
			rax *= r11;             //imul rax, r11
			rdx += rax;             //add rdx, rax
			rax = 0x337053B1D46053DA;               //mov rax, 0x337053B1D46053DA
			rdx ^= rax;             //xor rdx, rax
			rax = 0xC564E7ABB7147159;               //mov rax, 0xC564E7ABB7147159
			rdx *= rax;             //imul rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0xB;            //shr rax, 0x0B
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x16;           //shr rax, 0x16
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x2C;           //shr rax, 0x2C
			rdx ^= rax;             //xor rdx, rax
			return rdx;
		}
		case 7:
		{
			rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6A7789]
			r13 = baseModuleAddr + 0x4084A708;              //lea r13, [0x000000003DEF1E82]
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                //mov r10, [0x000000000801796D]
			rax = 0x1B29CE5A1FEAC2FF;               //mov rax, 0x1B29CE5A1FEAC2FF
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x13;           //shr rax, 0x13
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x26;           //shr rax, 0x26
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x8;            //shr rax, 0x08
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x10;           //shr rax, 0x10
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x20;           //shr rax, 0x20
			rdx ^= rax;             //xor rdx, rax
			rdx ^= rbx;             //xor rdx, rbx
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			rax = baseModuleAddr + 0x45C7161F;              //lea rax, [0x000000004331885F]
			rax += r11;             //add rax, r11
			rdx += rax;             //add rdx, rax
			rcx = r11;              //mov rcx, r11
			rcx = ~rcx;             //not rcx
			rax = rdx;              //mov rax, rdx
			rcx ^= r13;             //xor rcx, r13
			rdx = 0x9A5987173CBCE857;               //mov rdx, 0x9A5987173CBCE857
			rdx *= rax;             //imul rdx, rax
			rdx += rcx;             //add rdx, rcx
			return rdx;
		}
		case 8:
		{
			r12 = baseModuleAddr + 0x46558CD1;              //lea r12, [0x0000000043BFFE4C]
			r13 = baseModuleAddr + 0x7708260E;              //lea r13, [0x000000007472977A]
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                //mov r10, [0x000000000801735F]
			rcx = baseModuleAddr + 0x4DCA1145;              //lea rcx, [0x000000004B3481F7]
			rax = 0xF32FC9A683965DA1;               //mov rax, 0xF32FC9A683965DA1
			rdx *= rax;             //imul rdx, rax
			rax = r11 + r12 * 1;            //lea rax, [r11+r12*1]
			rdx += rax;             //add rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1A;           //shr rax, 0x1A
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x34;           //shr rax, 0x34
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			rdx += r11;             //add rdx, r11
			rdx ^= r11;             //xor rdx, r11
			rax = r13;              //mov rax, r13
			rax *= r11;             //imul rax, r11
			rdx -= rax;             //sub rdx, rax
			rax = rcx;              //mov rax, rcx
			rax ^= r11;             //xor rax, r11
			rdx += rax;             //add rdx, rax
			return rdx;
		}
		case 9:
		{
			r14 = baseModuleAddr + 0x7831C4AC;              //lea r14, [0x00000000759C3048]
			r9 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                 //mov r9, [0x0000000008016D0C]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1A;           //shr rax, 0x1A
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x34;           //shr rax, 0x34
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			rdx ^= r11;             //xor rdx, r11
			rax = r14;              //mov rax, r14
			rax ^= r11;             //xor rax, r11
			rdx ^= rax;             //xor rdx, rax
			rax = 0x374067541BFC9B6C;               //mov rax, 0x374067541BFC9B6C
			rdx -= rax;             //sub rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1E;           //shr rax, 0x1E
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x3C;           //shr rax, 0x3C
			rdx ^= rax;             //xor rdx, rax
			rax = 0x1B42D959E6DC4DC6;               //mov rax, 0x1B42D959E6DC4DC6
			rdx ^= rax;             //xor rdx, rax
			rax = 0x39E1AB220B9303F;                //mov rax, 0x39E1AB220B9303F
			rdx *= rax;             //imul rdx, rax
			return rdx;
		}
		case 10:
		{
			rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6A65FB]
			r12 = baseModuleAddr + 0xD396;          //lea r12, [0xFFFFFFFFFD6B3982]
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                //mov r10, [0x00000000080167DB]
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			rax = rdx;              //mov rax, rdx
			rax >>= 0xB;            //shr rax, 0x0B
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x16;           //shr rax, 0x16
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x2C;           //shr rax, 0x2C
			rdx ^= rax;             //xor rdx, rax
			rax = 0xDDA62443628C88AF;               //mov rax, 0xDDA62443628C88AF
			rdx *= rax;             //imul rdx, rax
			rax = 0x6FF473D3D186BCB4;               //mov rax, 0x6FF473D3D186BCB4
			rdx += rax;             //add rdx, rax
			rax = 0x76970B4298DE684A;               //mov rax, 0x76970B4298DE684A
			rdx ^= rax;             //xor rdx, rax
			rcx = r11;              //mov rcx, r11
			rcx -= rbx;             //sub rcx, rbx
			rcx -= 0x70E6128A;              //sub rcx, 0x70E6128A
			rax = r11 + r12 * 1;            //lea rax, [r11+r12*1]
			rcx ^= rax;             //xor rcx, rax
			rdx ^= rcx;             //xor rdx, rcx
			rax = rdx;              //mov rax, rdx
			rax >>= 0x7;            //shr rax, 0x07
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0xE;            //shr rax, 0x0E
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1C;           //shr rax, 0x1C
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x38;           //shr rax, 0x38
			rdx ^= rax;             //xor rdx, rax
			return rdx;
		}
		case 11:
		{
			r9 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                 //mov r9, [0x00000000080161A4]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x16;           //shr rax, 0x16
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x2C;           //shr rax, 0x2C
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x7;            //shr rax, 0x07
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0xE;            //shr rax, 0x0E
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1C;           //shr rax, 0x1C
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x38;           //shr rax, 0x38
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x12;           //shr rax, 0x12
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x24;           //shr rax, 0x24
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x13;           //shr rax, 0x13
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x26;           //shr rax, 0x26
			rdx ^= rax;             //xor rdx, rax
			rax = 0x1B59B442FC549DB7;               //mov rax, 0x1B59B442FC549DB7
			rdx *= rax;             //imul rdx, rax
			rax = 0x93D962D3892C4B07;               //mov rax, 0x93D962D3892C4B07
			rdx ^= rax;             //xor rdx, rax
			rax = 0x2B4472275B36B070;               //mov rax, 0x2B4472275B36B070
			rdx -= rax;             //sub rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			return rdx;
		}
		case 12:
		{
			rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6A5AC0]
			r13 = baseModuleAddr + 0xF7;            //lea r13, [0xFFFFFFFFFD6A5BA8]
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                //mov r10, [0x0000000008015CA4]
			rax = r13;              //mov rax, r13
			rax ^= r11;             //xor rax, r11
			rdx -= rax;             //sub rdx, rax
			rax = rdx;              //mov rax, rdx
			rcx = baseModuleAddr + 0x2C90A915;              //lea rcx, [0x0000000029FB0294]
			rcx *= r11;             //imul rcx, r11
			rdx = 0x5068AC5825C6AEB7;               //mov rdx, 0x5068AC5825C6AEB7
			rdx *= rax;             //imul rdx, rax
			rdx += rcx;             //add rdx, rcx
			rax = rdx;              //mov rax, rdx
			rax >>= 0x24;           //shr rax, 0x24
			rdx ^= rax;             //xor rdx, rax
			rax = 0x7A7192246BD306CF;               //mov rax, 0x7A7192246BD306CF
			rdx ^= rax;             //xor rdx, rax
			rax = r11;              //mov rax, r11
			rax = ~rax;             //not rax
			rax -= rbx;             //sub rax, rbx
			rax += 0xFFFFFFFFB6023E3B;              //add rax, 0xFFFFFFFFB6023E3B
			rdx += rax;             //add rdx, rax
			rax = 0xDF1F224B893CA8A3;               //mov rax, 0xDF1F224B893CA8A3
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			return rdx;
		}
		case 13:
		{
			rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6A55A4]
			r9 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                 //mov r9, [0x0000000008015726]
			rax = 0xBE006C3D9512D333;               //mov rax, 0xBE006C3D9512D333
			rdx *= rax;             //imul rdx, rax
			rdx -= r11;             //sub rdx, r11
			uintptr_t RSP_0x28;
			RSP_0x28 = 0x4EE8A6411C0FCC55;          //mov rax, 0x4EE8A6411C0FCC55 : RSP+0x28
			rdx *= RSP_0x28;                //imul rdx, [rsp+0x28]
			uintptr_t RSP_0x48;
			RSP_0x48 = 0x23A574AC4C8AEE44;          //mov rax, 0x23A574AC4C8AEE44 : RSP+0x48
			rdx -= RSP_0x48;                //sub rdx, [rsp+0x48]
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x8;            //shr rax, 0x08
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x10;           //shr rax, 0x10
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x20;           //shr rax, 0x20
			rdx ^= rax;             //xor rdx, rax
			rax = baseModuleAddr + 0x2E35A551;              //lea rax, [0x000000002B9FF609]
			rax = ~rax;             //not rax
			rax += r11;             //add rax, r11
			rdx += rax;             //add rdx, rax
			rdx += rbx;             //add rdx, rbx
			return rdx;
		}
		case 14:
		{
			r14 = baseModuleAddr + 0xE6B8;          //lea r14, [0xFFFFFFFFFD6B374B]
			r10 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                //mov r10, [0x0000000008015270]
			rcx = r11;              //mov rcx, r11
			rax = r14;              //mov rax, r14
			rcx = ~rcx;             //not rcx
			rax = ~rax;             //not rax
			rdx += rax;             //add rdx, rax
			rdx += rcx;             //add rdx, rcx
			rdx ^= r11;             //xor rdx, r11
			rax = 0x2C96FDD1F7FC4605;               //mov rax, 0x2C96FDD1F7FC4605
			rdx *= rax;             //imul rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x12;           //shr rax, 0x12
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x24;           //shr rax, 0x24
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r10;             //xor rax, r10
			rax = _byteswap_uint64(rax);            //bswap rax
			rdx *= *(uintptr_t*)(rax + 0x7);                //imul rdx, [rax+0x07]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x23;           //shr rax, 0x23
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x12;           //shr rax, 0x12
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x24;           //shr rax, 0x24
			rdx ^= rax;             //xor rdx, rax
			rdx += r11;             //add rdx, r11
			return rdx;
		}
		case 15:
		{
			rbx = baseModuleAddr;           //lea rbx, [0xFFFFFFFFFD6A4BCD]
			r9 = *(uintptr_t*)(baseModuleAddr + 0xA97023F);                 //mov r9, [0x0000000008014D58]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x14;           //shr rax, 0x14
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x28;           //shr rax, 0x28
			rdx ^= rax;             //xor rdx, rax
			rdx -= rbx;             //sub rdx, rbx
			rax = rdx;              //mov rax, rdx
			rax >>= 0x1A;           //shr rax, 0x1A
			rdx ^= rax;             //xor rdx, rax
			rax = rdx;              //mov rax, rdx
			rax >>= 0x34;           //shr rax, 0x34
			rdx ^= rax;             //xor rdx, rax
			rax = 0;                //and rax, 0xFFFFFFFFC0000000
			rax = _rotl64(rax, 0x10);               //rol rax, 0x10
			rax ^= r9;              //xor rax, r9
			rax = _byteswap_uint64(rax);            //bswap rax
			rax = *(uintptr_t*)(rax + 0x7);                 //mov rax, [rax+0x07]
			uintptr_t RSP_0x28;
			RSP_0x28 = 0x4F0933C03F0A6A07;          //mov rax, 0x4F0933C03F0A6A07 : RSP+0x28
			rax *= RSP_0x28;                //imul rax, [rsp+0x28]
			rdx *= rax;             //imul rdx, rax
			rax = 0x4E1E6416338C51B4;               //mov rax, 0x4E1E6416338C51B4
			rdx ^= rax;             //xor rdx, rax
			rdx -= r11;             //sub rdx, r11
			return rdx;
		}
		}
	}
	uint16_t get_bone_index(uint32_t bone_index)
	{
		auto baseModuleAddr = g_data::base;
		auto Peb = __readgsqword(0x60);
		uint64_t rax = baseModuleAddr, rbx = baseModuleAddr, rcx = baseModuleAddr, rdx = baseModuleAddr, rdi = baseModuleAddr, rsi = baseModuleAddr, r8 = baseModuleAddr, r9 = baseModuleAddr, r10 = baseModuleAddr, r11 = baseModuleAddr, r12 = baseModuleAddr, r13 = baseModuleAddr, r14 = baseModuleAddr, r15 = baseModuleAddr;

		rdi = bone_index;
		rcx = rdi * 0x13C8;
		rax = 0xA936923D3DFD979;                //mov rax, 0xA936923D3DFD979
		rax = _umul128(rax, rcx, (uintptr_t*)&rdx);             //mul rcx
		rax = rcx;              //mov rax, rcx
		r11 = baseModuleAddr;           //lea r11, [0xFFFFFFFFFD38BD15]
		rax -= rdx;             //sub rax, rdx
		r10 = 0xAFBC1EAC2D6DDB7B;               //mov r10, 0xAFBC1EAC2D6DDB7B
		rax >>= 0x1;            //shr rax, 0x01
		rax += rdx;             //add rax, rdx
		rax >>= 0xC;            //shr rax, 0x0C
		rax = rax * 0x1EBB;             //imul rax, rax, 0x1EBB
		rcx -= rax;             //sub rcx, rax
		rax = 0x5F4EC9815388ADDD;               //mov rax, 0x5F4EC9815388ADDD
		r8 = rcx * 0x1EBB;              //imul r8, rcx, 0x1EBB
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rdx >>= 0xC;            //shr rdx, 0x0C
		rax = rdx * 0x2AFA;             //imul rax, rdx, 0x2AFA
		r8 -= rax;              //sub r8, rax
		rax = 0x17196A8082D3E9ED;               //mov rax, 0x17196A8082D3E9ED
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rax = r8;               //mov rax, r8
		rax -= rdx;             //sub rax, rdx
		rax >>= 0x1;            //shr rax, 0x01
		rax += rdx;             //add rax, rdx
		rax >>= 0xB;            //shr rax, 0x0B
		rcx = rax * 0xEAD;              //imul rcx, rax, 0xEAD
		rax = 0xCCCCCCCCCCCCCCCD;               //mov rax, 0xCCCCCCCCCCCCCCCD
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rdx >>= 0x2;            //shr rdx, 0x02
		rcx += rdx;             //add rcx, rdx
		rax = rcx + rcx * 4;            //lea rax, [rcx+rcx*4]
		rax += rax;             //add rax, rax
		rcx = r8 + r8 * 2;              //lea rcx, [r8+r8*2]
		rcx <<= 0x2;            //shl rcx, 0x02
		rcx -= rax;             //sub rcx, rax
		rax = *(uint16_t*)(rcx + r11 * 1 + 0xAA2C9B0);          //movzx eax, word ptr [rcx+r11*1+0xAA2C9B0]
		r8 = rax * 0x13C8;              //imul r8, rax, 0x13C8
		rax = r10;              //mov rax, r10
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rcx = r8;               //mov rcx, r8
		rax = r10;              //mov rax, r10
		rcx -= rdx;             //sub rcx, rdx
		rcx >>= 0x1;            //shr rcx, 0x01
		rcx += rdx;             //add rcx, rdx
		rcx >>= 0xD;            //shr rcx, 0x0D
		rcx = rcx * 0x25F3;             //imul rcx, rcx, 0x25F3
		r8 -= rcx;              //sub r8, rcx
		r9 = r8 * 0x2A22;               //imul r9, r8, 0x2A22
		rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		rax = r9;               //mov rax, r9
		rax -= rdx;             //sub rax, rdx
		rax >>= 0x1;            //shr rax, 0x01
		rax += rdx;             //add rax, rdx
		rax >>= 0xD;            //shr rax, 0x0D
		rax = rax * 0x25F3;             //imul rax, rax, 0x25F3
		r9 -= rax;              //sub r9, rax
		rax = 0xAAAAAAAAAAAAAAAB;               //mov rax, 0xAAAAAAAAAAAAAAAB
		rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		rax = 0xADA9AD688DC78693;               //mov rax, 0xADA9AD688DC78693
		rdx >>= 0x2;            //shr rdx, 0x02
		rcx = rdx + rdx * 2;            //lea rcx, [rdx+rdx*2]
		rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		rdx >>= 0xB;            //shr rdx, 0x0B
		rax = rdx + rcx * 2;            //lea rax, [rdx+rcx*2]
		rcx = rax * 0x1796;             //imul rcx, rax, 0x1796
		rax = r9 * 0x1798;              //imul rax, r9, 0x1798
		rax -= rcx;             //sub rax, rcx
		r15 = *(uint16_t*)(rax + r11 * 1 + 0xAA35C80);          //movsx r15d, word ptr [rax+r11*1+0xAA35C80]
		return r15;
	}

	player_t get_player(int i)
	{
		uint64_t decryptedPtr = get_client_info();

		if (is_valid_ptr (decryptedPtr))
		{
			uint64_t client_info = get_client_info_base();

			if (is_valid_ptr(client_info))
			{
				return player_t(client_info + (i * player_info::size));
			}
		}
		return player_t(NULL);
	}

	//player_t player_t
	
	//player_t get_local_player()
	//{
	//	auto addr = sdk::get_client_info_base() + (get_local_index() * player_info::size);
	//	if (is_bad_ptr(addr)) return 0;
	//	return addr;


	//}

	player_t get_local_player()
	{
		uint64_t decryptedPtr = get_client_info();

		if (is_bad_ptr(decryptedPtr))return player_t(NULL);

			auto local_index = *(uintptr_t*)(decryptedPtr + player_info::local_index);
			if (is_bad_ptr(local_index))return player_t(NULL);
			auto index = *(int*)(local_index + player_info::local_index_pos);
			return get_player(index);
		
		
	}

	/*name_t* get_name_ptr(int i)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);

		if (bgs)
		{
			name_t* pClientInfo = (name_t*)(bgs + client::name_array_padding + ((i + i * 8) << 4));

			if (is_bad_ptr(pClientInfo))return 0;
			else
			return pClientInfo;
			
		}
		return 0;
	}*/

	refdef_t* get_refdef()
	{
		uint32_t crypt_0 = *(uint32_t*)(g_data::base + view_port::refdef_ptr);
		uint32_t crypt_1 = *(uint32_t*)(g_data::base + view_port::refdef_ptr + 0x4);
		uint32_t crypt_2 = *(uint32_t*)(g_data::base + view_port::refdef_ptr + 0x8);
		// lower 32 bits
		uint32_t entry_1 = (uint32_t)(g_data::base + view_port::refdef_ptr);
		uint32_t entry_2 = (uint32_t)(g_data::base + view_port::refdef_ptr + 0x4);
		// decryption
		uint32_t _low = entry_1 ^ crypt_2;
		uint32_t _high = entry_2 ^ crypt_2;
		uint32_t low_bit = crypt_0 ^ _low * (_low + 2);
		uint32_t high_bit = crypt_1 ^ _high * (_high + 2);
		auto ret = (refdef_t*)(((QWORD)high_bit << 32) + low_bit);
		if (is_bad_ptr(ret)) return 0;
		else
			return ret;
	}

	Vector3 get_camera_pos()
	{
		Vector3 pos = Vector3{};

		auto camera_ptr = *(uint64_t*)(g_data::base + view_port::camera_ptr);

		if (is_bad_ptr(camera_ptr))return pos;
		
		
		pos = *(Vector3*)(camera_ptr + view_port::camera_pos);
		if (pos.IsZero())return {};
		else
		return pos;
	}

	/*std::string get_player_name(int i)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);

		if (is_bad_ptr(bgs))return NULL;


		if (bgs)
		{
			name_t* clientInfo_ptr = (name_t*)(bgs + client::name_array_padding + (i * 0xD0));
			if (is_bad_ptr(clientInfo_ptr))return NULL;

			int length = strlen(clientInfo_ptr->name);
			for (int j = 0; j < length; ++j)
			{
				char ch = clientInfo_ptr->name[j];
				bool is_english = ch >= 0 && ch <= 127;
				if (!is_english)
					return xorstr_("Player");
			}
			return clientInfo_ptr->name;
		}
		return xorstr_("Player");
	}*/

	
    bool bones_to_screen(Vector3* BonePosArray, Vector2* ScreenPosArray, const long Count)
    {
        for (long i = 0; i < Count; ++i)
        {
            if (!world(BonePosArray[i], &ScreenPosArray[i]))
                return false;
        }
        return true;
    }



	bool get_bone_by_player_index(int i, int bone_id, Vector3* Out_bone_pos)
	{
		uint64_t decrypted_ptr = get_bone_ptr();

		if (is_bad_ptr(decrypted_ptr))return false;
		
			unsigned short index = get_bone_index(i);
			if (index != 0)
			{
				uint64_t bone_ptr = *(uint64_t*)(decrypted_ptr + (index * bones::size) + 0xD8);

				if (is_bad_ptr(bone_ptr))return false;
				
					Vector3 bone_pos = *(Vector3*)(bone_ptr + (bone_id * 0x20) + 0x10);

					if (bone_pos.IsZero())return false;

					uint64_t client_info = get_client_info();

					if (is_bad_ptr(client_info))return false;

					
					
						Vector3 BasePos = *(Vector3*)(client_info + bones::bone_base_pos);

						if (BasePos.IsZero())return false;

						bone_pos.x += BasePos.x;
						bone_pos.y += BasePos.y;
						bone_pos.z += BasePos.z;

						*Out_bone_pos = bone_pos;
						return true;
					
				
			}
		
		return false;
	}
	
	int get_player_health(int i)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);

		if (bgs)
		{
			name_t* pClientInfo = (name_t*)(bgs + client::name_array_padding  +(i * 0xD8));

			if (pClientInfo)
			{
				return pClientInfo->get_health();
			}
		}
		return 0;
	}

	std::string get_player_name(int entityNum)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);
		if (is_bad_ptr(bgs)) return "";

		if (bgs)
		{
			name_t* clientInfo_ptr = (name_t*)(bgs + client::name_array_padding + (entityNum * 0xD8));
			if (is_bad_ptr(clientInfo_ptr)) return "";

			int length = strlen(clientInfo_ptr->name);
			for (int j = 0; j < length; ++j)
			{
				char ch = clientInfo_ptr->name[j];
				bool is_english = ch >= 0 && ch <= 127;
				if (!is_english)
					return xorstr_("Player");
			}
			return clientInfo_ptr->name;
		}
		return xorstr_("Player");
	}

	void start_tick()
	{
		static DWORD lastTick = 0;
		DWORD t = GetTickCount();
		bUpdateTick = lastTick < t;

		if (bUpdateTick)
			lastTick = t + nTickTime;
	}

	void update_vel_map(int index, Vector3 vPos)
	{
		if (!bUpdateTick)
			return;

		velocityMap[index].delta = vPos - velocityMap[index].lastPos;
		velocityMap[index].lastPos = vPos;
	}

	void clear_map()
	{
		if (!velocityMap.empty()) { velocityMap.clear(); }
	}

	Vector3 get_speed(int index)
	{
		return velocityMap[index].delta;
	}

	Vector3 get_prediction(int index, Vector3 source, Vector3 destination)
	{
		auto local_velocity = get_speed(local_index());
		auto target_velocity = get_speed(index);

		const auto distance = source.distance_to(destination);
		const auto travel_time = distance / globals::bullet_speed;
		auto pred_destination = destination + (target_velocity - local_velocity) * travel_time;
		/*position.x += travel_time * final_speed.x;
		position.y += travel_time * final_speed.y;
		position.z += 0.5 * globals::bullet_gravity * travel_time * travel_time;
		return position;*/

		pred_destination.z += 0.5f * std::fabsf(globals::bullet_gravity) * travel_time;

		return pred_destination;
	}


	Result MidnightSolver(float a, float b, float c)
	{
		Result res;

		double subsquare = b * b - 4 * a * c;

		if (subsquare < 0)
		{
			res.hasResult = false;
			return res;
		}
		else
		{
			res.hasResult = true,
			res.a = (float)((-b + sqrt(subsquare)) / (2 * a));
			res.b = (float)((-b - sqrt(subsquare)) / (2 * a));
		}
		return res;
	}

	Vector3 prediction_solver(Vector3 local_pos, Vector3 position, int index, float bullet_speed)
	{
		Vector3 aimPosition = Vector3().Zero();
		auto target_speed = get_speed(index);

		local_pos -= position; 

		float a = (target_speed.x * target_speed.x) + (target_speed.y * target_speed.y) + (target_speed.z * target_speed.z) - ((bullet_speed * bullet_speed) * 100);
		float b = (-2 * local_pos.x * target_speed.x) + (-2 * local_pos.y * target_speed.y) + (-2 * local_pos.z * target_speed.z);
		float c = (local_pos.x * local_pos.x) + (local_pos.y * local_pos.y) + (local_pos.z * local_pos.z);

		local_pos += position; 

		Result r = MidnightSolver(a, b, c);

		if (r.a >= 0 && !(r.b >= 0 && r.b < r.a))
		{
			aimPosition = position + target_speed * r.a;
		}
		else if (r.b >= 0)
		{
			aimPosition = position + target_speed * r.b;
		}

		return aimPosition;
	
	}
	int decrypt_visible_flag(int i, QWORD valid_list)
	{
		auto ptr = valid_list + ((static_cast<unsigned long long>(i) * 9 + 0x152) * 8) + 0x10;
		DWORD dw1 = (*(DWORD*)(ptr + 4) ^ (DWORD)ptr);
		DWORD dw2 = ((dw1 + 2) * dw1);
		BYTE dec_visible_flag = *(BYTE*)(ptr) ^ BYTE1(dw2) ^ (BYTE)dw2;

		return (int)dec_visible_flag;
	}

	uint64_t get_visible_base()
	{
		
		for (int32_t j{}; j <= 0x1770; ++j)
		{
			
			uint64_t vis_base_ptr = *(uint64_t*)(g_data::base + bones::distribute) + (j * 0x190);
			uint64_t cmp_function = *(uint64_t*)(vis_base_ptr + 0x38);

			if (!cmp_function)
				continue;

			//LOGS_ADDR(cmp_function);

			uint64_t about_visible = g_data::base + bones::visible;

			if (cmp_function == about_visible)
			{

				g_data::current_visible_offset = vis_base_ptr;
				return g_data::current_visible_offset;
			}

		}
		return NULL;
	}
	
	bool is_visible(int entityNum) {

		if (!g_data::last_visible_offset)
			return false;

		uint64_t VisibleList = *(uint64_t*)(g_data::last_visible_offset + 0x80);
		if (!VisibleList)
			return false;
		uint64_t v421 = VisibleList + (entityNum * 9 + 0x152) * 8;
		if (!v421)
			return false;
		DWORD VisibleFlags = (v421 + 0x10) ^ *(DWORD*)(v421 + 0x14);
		if (!VisibleFlags)
			return false;
		DWORD v1630 = VisibleFlags * (VisibleFlags + 2);
		if (!v1630)
			return false;
		BYTE VisibleFlags1 = *(DWORD*)(v421 + 0x10) ^ v1630 ^ BYTE1(v1630);
		if (VisibleFlags1 == 3) {
			return true;
		}
		return false;
	}

	void update_last_visible()
	{
		g_data::last_visible_offset = g_data::current_visible_offset;
	}

	// player class methods
	bool player_t::is_valid() {
		if (is_bad_ptr(address))return 0;

		return *(bool*)((uintptr_t)address + player_info::valid);
	}

	//bool player_t::is_dead() {
	//	if (is_bad_ptr(address))return 0;

	//	auto dead1 = *(bool*)((uintptr_t)address + player_info::dead_1);
	//	auto dead2 = *(bool*)((uintptr_t)address + player_info::dead_2);
	//	return !(!dead1 && !dead2 && is_valid());
	//}
	bool player_t::is_dead() {
		if (is_bad_ptr(address))return 0;

		auto dead1 = *(int*)((uintptr_t)address + player_info::dead_1);
		//auto dead2 = *(bool*)((uintptr_t)address + player_info::dead_2);
		return !(!dead1 && is_valid());
	}

	BYTE player_t::team_id() {

		if (is_bad_ptr(address))return 0;
		return *(BYTE*)((uintptr_t)address + player_info::team_id);
	}

	int player_t::get_stance() {
		
		if (is_bad_ptr(address))return 4;
		auto ret = *(int*)((uintptr_t)address + player_info::stance);
	

		return ret;
	}


	float player_t::get_rotation()
	{
		if (is_bad_ptr(address))return 0;
		auto local_pos_ptr = *(uintptr_t*)((uintptr_t)address + player_info::position_ptr);

		if (is_bad_ptr(local_pos_ptr))return 0;

		auto rotation = *(float*)(local_pos_ptr + 0x58);

		if (rotation < 0)
			rotation = 360.0f - (rotation * -1);

		rotation += 90.0f;

		if (rotation >= 360.0f)
			rotation = rotation - 360.0f;

		return rotation;
	}

	Vector3 player_t::get_pos() 
	{
		if (is_bad_ptr(address))return {};
		auto local_pos_ptr = *(uintptr_t*)((uintptr_t)address + player_info::position_ptr);

		if (is_bad_ptr(local_pos_ptr))return{};
		else
			return *(Vector3*)(local_pos_ptr + 0x48);
		return Vector3{}; 


	}

	uint32_t player_t::get_index()
	{
		if (is_bad_ptr(this->address))return 0;

		auto cl_info_base = get_client_info_base();
		if (is_bad_ptr(cl_info_base))return 0;
		
		
	return ((uintptr_t)this->address - cl_info_base) / player_info::size;
		
	
	}

	bool player_t::is_visible()
	{
		if (is_bad_ptr(g_data::visible_base))return false;

		if (is_bad_ptr(this->address))return false;
		
			uint64_t VisibleList =*(uint64_t*)(g_data::visible_base + 0x108);
			if (is_bad_ptr(VisibleList))
				return false;

			uint64_t rdx = VisibleList + (player_t::get_index() * 9 + 0x14E) * 8;
			if (is_bad_ptr(rdx))
				return false;

			DWORD VisibleFlags = (rdx + 0x10) ^ *(DWORD*)(rdx + 0x14);
			if (!VisibleFlags)
				return false;

			DWORD v511 = VisibleFlags * (VisibleFlags + 2);
			if (!v511)
				return false;

			BYTE VisibleFlags1 = *(DWORD*)(rdx + 0x10) ^ v511 ^ BYTE1(v511);
			if (VisibleFlags1 == 3) {
				return true;
			}
		
		return false;
	}
	



}

