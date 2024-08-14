
#include "UnlockAllV2.h"

#include"xor.hpp"
#include"spoofcall.h"
#include"offsets.h"
#include "memory.h"
void UnlockAllV2::OnlineLoot_UpdateItemInInventory(__int64 Online_Loot, int controllerIndex, unsigned int itemID, int quantity, char set)//40 53 55 57 41 54 41 55 41 56 41 57 48 81 ec ? ? ? ? 48 8b 05 ? ? ? ? 48 33 c4 48 89 44 24 ? 4c 8b 15
{
    VariadicCall<void>(sig_offsets::OnlineLoot_UpdateItemInInventory, Online_Loot, controllerIndex, itemID, quantity, set);

}

void* UnlockAllV2::OnlineLoot_GetInstance() //E8 ? ? ? ? 44 8B CB C6 44 24 ? ? 44 8B C7 41 8B D6 48
{
    return	VariadicCall<void*>(sig_offsets::OnlineLoot_GetInstance);
}

const char* UnlockAllV2::StringTable_GetColumnValueForRow(StringTable* table, const int row, const int column)//E8 ? ? ? ? 48 8B D0 0F B7 CB E8 ? ? ? ? 48 8B 4C 24 ? 41 B8 ? ? ? ? 8B D7 E8
{
    return VariadicCall<const char*>(sig_offsets::StringTable_GetColumnValueForRow, table, row, column);

}

void UnlockAllV2::StringTable_GetAsset(const char* filename, StringTable** tableptr) //E8 ? ? ? ? 48 8B 4C 24 ? 33 F6 E8 ? ? ? ? 85 C0 7E 58 0F 1F 40 00 0F 1F 84 00
{
    return VariadicCall<void>(sig_offsets::StringTable_GetAsset, filename, tableptr);

}

__int64 UnlockAllV2::OnlineLoot_GetItemQuantity(__int64 Online_Loot, unsigned int controllerIndex, unsigned int id) //e8 ? ? ? ? 89 45 ? 85 c0 74 ? 49 8b 0c 24
{
    return VariadicCall<__int64>(sig_offsets::OnlineLoot_GetItemQuantity, Online_Loot, controllerIndex, id);

}



void UnlockAllV2::unlock_All()
{
    auto online_loot = OnlineLoot_GetInstance();
    v10[0] = (__int64)online_loot + 0x48; // pointer to inventory
    v10[1] = (__int64)online_loot + 0x2D520; //number of items

    StringTable_GetAsset(xorstr("loot/loot_master.csv"), &loot_table);// gets the main loot asset which contains every single item in game

    for (size_t i = 0; i < loot_table->rowCount; i++) // max items in loot_master column
    {
        if (loot_table != nullptr)
        {
            auto LootID = StringTable_GetColumnValueForRow(loot_table, i, 0); //looping trough StringTable to get the item_ids
            auto Item_ID = atoi(LootID);
            if (Item_ID > 0)
            {
                if (!OnlineLoot_GetItemQuantity((uintptr_t)online_loot, 0, Item_ID)) // ckeck if the item is already unlocked
                {
                    OnlineLoot_UpdateItemInInventory((__int64)v10, 0, Item_ID, 1, true); // set the item and temporary unlocks it (soft unlock)
                }
            }
        }
    }
}


void UnlockAllV2::Test()
{
    auto online_loot = OnlineLoot_GetInstance();
    v10[0] = (__int64)online_loot + 0x48; // pointer to inventory
    v10[1] = (__int64)online_loot + 0x2D520; //number of items

    StringTable_GetAsset(xorstr("loot/feature_ids.csv"), &loot_table);// gets the main loot asset which contains every single item in game

    for (size_t i = 0; i < loot_table->rowCount; i++) // max items in loot_master column
    {
        if (loot_table != nullptr)
        {
            auto LootID = StringTable_GetColumnValueForRow(loot_table, i, 0); //looping trough StringTable to get the item_ids
            auto Item_ID = atoi(LootID);
            if (Item_ID > 0)
            {
                if (!OnlineLoot_GetItemQuantity((uintptr_t)online_loot, 0, Item_ID)) // ckeck if the item is already unlocked
                {
                    nlog("id = %d", Item_ID);

                    OnlineLoot_UpdateItemInInventory((__int64)v10, 0, Item_ID, 1, true); // set the item and temporary unlocks it (soft unlock)
                }
            }
        }
    }
}

void UnlockAllV2::unlock(const char* path)
{
    auto online_loot = OnlineLoot_GetInstance();
    v10[0] = (__int64)online_loot + 0x48;
    v10[1] = (__int64)online_loot + 0x2D520;

    StringTable_GetAsset(path, &loot_table);// gets the main loot asset which contains every single item in game


    for (size_t i = 0; i < loot_table->rowCount; i++)
    {
        if (loot_table != nullptr)
        {
            auto LootID = StringTable_GetColumnValueForRow(loot_table, i, 0); //looping trough StringTable to get the item_ids
            auto Item_ID = atoi(LootID);

            if (Item_ID > 0)
            {
                if (!OnlineLoot_GetItemQuantity((uintptr_t)online_loot, 0, Item_ID)) // is the item valid?
                {
                    OnlineLoot_UpdateItemInInventory((__int64)v10, 0, Item_ID, 1, true); // set the item and temporary unlocks it (soft unlock)
                }
            }

        }
    }
}

void UnlockAllV2::invidual_unlock()
{

    if (invidual_select == 0) // weapon
    {
        unlock(xorstr("loot/weapon_ids.csv"));

    }
    else if (invidual_select == 1) // 
    {
        unlock(xorstr("loot/attachment_ids.csv"));
    }
    else if (invidual_select == 2) // 
    {
        unlock(xorstr("loot/camo_ids.csv"));
    }
    else if (invidual_select == 3) // 
    {
        unlock(xorstr("loot/bundle_ids.csv"));

    }
    else if (invidual_select == 4) // 
    {
        unlock(xorstr("loot/equipment_ids.csv"));
    }
    else if (invidual_select == 5) // 
    {
        unlock(xorstr("loot/killstreak_ids.csv"));
    }
    else if (invidual_select == 6) // 
    {
        unlock(xorstr("loot/perk_ids.csv"));
    }
    else if (invidual_select == 7) // 
    {
        unlock(xorstr("loot/operator_ids.csv"));
    }
    else if (invidual_select == 8) // 
    {
        unlock(xorstr("loot/operator_equip_ids.csv"));
    }
    else if (invidual_select == 9) // 
    {
        unlock(xorstr("loot/operator_skin_ids.csv"));
    }
    else if (invidual_select == 10) // 
    {
        unlock(xorstr("loot/vehicle_skin_ids.csv"));
    }

    else if (invidual_select == 11) // 
    {
        unlock(xorstr("loot/vehicle_horn_ids.csv"));
    }
    else if (invidual_select == 12) // 
    {
        unlock(xorstr("loot/vehicle_track_ids.csv"));
    }
    else if (invidual_select == 13) // 
    {
        unlock(xorstr("loot/feature_ids.csv"));
    }



}


void UnlockAllV2::UnlockAllV3()
{
    char buffer[50];
    StringTable_GetAsset((char*)0x168E1F4AA495119C, &loot_table);
    auto online_loot = OnlineLoot_GetInstance();

    for (int row = 0; row < loot_table->rowCount; ++row)
    {
        const char* item = StringTable_GetColumnValueForRow(loot_table, row, 0);
        int itemID = atoi(item);

        memset(buffer, 0, 50);
        sprintf_s(buffer, "0x%x", itemID);
        nlog("id :0x%X ", itemID);

        int16_t result_hash{};

        for (int i = 0; buffer[i]; ++i)
            result_hash = buffer[i] + 31 * result_hash;

        result_hash &= 0x7FFF;

        int16_t start = result_hash;
        __int64* cell{};
        int* itemCount = (int*)online_loot;

        for (int16_t* i = (int16_t*)((uintptr_t)online_loot + 2 * start + 4);; i = (int16_t*)((uintptr_t)online_loot + 2 * result_hash + 4))
        {
            if (*i == -1)
            {
                int idx = *itemCount;
                cell = (__int64*)((uintptr_t)online_loot + 8 * idx + 0x10004);
                *cell = (1LL << 32) | itemID;

                *i = idx;
                ++* itemCount;
                break;
            }
            cell = (__int64*)((uintptr_t)online_loot + 8 * *i + 0x10004);

            if (*(PINT)cell == itemID) // item is already unlocked 
                break;

            result_hash = ++result_hash & 0x7FFF;
            if (result_hash == start)
                break;
        }
    }
}
