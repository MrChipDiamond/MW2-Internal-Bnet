#pragma once
#include<Windows.h>

struct StringTable {
	char pad[12];
	int rowCount;
};

class UnlockAllV2
{
private:
	StringTable* loot_table = nullptr;
	__int64 v10[2];
	void  OnlineLoot_UpdateItemInInventory(__int64 Online_Loot, int controllerIndex, unsigned int itemID, int quantity, char set);
	void* OnlineLoot_GetInstance();
	const char* StringTable_GetColumnValueForRow(StringTable* table, const int row, const int column);
	void  StringTable_GetAsset(const char* filename, StringTable** tableptr);
	__int64  OnlineLoot_GetItemQuantity(__int64 Online_Loot, unsigned int controllerIndex, unsigned int id);
public:
	int invidual_select = 0;

	void Test();

	void unlock_All();
	void unlock(const char* path);
	void invidual_unlock();
	void UnlockAllV3();


};

inline UnlockAllV2 v2 = UnlockAllV2();