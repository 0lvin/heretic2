//
// Heretic II
// Copyright 1998 Raven Software
//
#include "../../../common/header/common.h"
#include "../../header/game.h"
#include "player.h"
#include "p_anim_data.h"
static game_import_t gi;

void P_Init(void)
{
	InitItems();
}

void P_Shutdown(void)
{
}

gitem_t*
GetPlayerItems(void)
{
	return p_itemlist;
}

int
GetPlayerItemsCount(void)
{
	return p_num_items;
}

player_export_t playerExport;

Q2_DLL_EXPORTED player_export_t *
GetPlayerAPI(game_import_t *import)
{
	gi = *import;

	playerExport.Init = P_Init;
	playerExport.Shutdown = P_Shutdown;

	playerExport.PlayerReleaseRope = PlayerReleaseRope;
	playerExport.KnockDownPlayer = KnockDownPlayer;
	playerExport.PlayFly = PlayFly;
	playerExport.PlaySlap = PlaySlap;
	playerExport.PlayScratch = PlayScratch;
	playerExport.PlaySigh = PlaySigh;
	playerExport.SpawnDustPuff = SpawnDustPuff;
	playerExport.PlayerInterruptAction = PlayerInterruptAction;

	playerExport.BranchCheckDismemberAction = BranchCheckDismemberAction;

	playerExport.TurnOffPlayerEffects = TurnOffPlayerEffects;
	playerExport.AnimUpdateFrame = AnimUpdateFrame;
	playerExport.PlayerFallingDamage = PlayerFallingDamage;

	playerExport.PlayerBasicAnimReset = PlayerBasicAnimReset;
	playerExport.PlayerAnimReset = PlayerAnimReset;
	playerExport.PlayerAnimSetLowerSeq = PlayerAnimSetLowerSeq;
	playerExport.PlayerAnimSetUpperSeq = PlayerAnimSetUpperSeq;
	playerExport.PlayerAnimUpperIdle = PlayerAnimUpperIdle;
	playerExport.PlayerAnimLowerIdle = PlayerAnimLowerIdle;
	playerExport.PlayerAnimUpperUpdate = PlayerAnimUpperUpdate;
	playerExport.PlayerAnimLowerUpdate = PlayerAnimLowerUpdate;
	playerExport.PlayerAnimSetVault = PlayerAnimSetVault;
	playerExport.PlayerPlayPain = PlayerPlayPain;

	playerExport.PlayerIntLand = PlayerIntLand;

	playerExport.PlayerInit = PlayerInit;
	playerExport.PlayerClearEffects = PlayerClearEffects;
	playerExport.PlayerUpdate = PlayerUpdate;
	playerExport.PlayerUpdateCmdFlags = PlayerUpdateCmdFlags;
	playerExport.PlayerUpdateModelAttributes = PlayerUpdateModelAttributes;

	playerExport.Weapon_Ready = Weapon_Ready;
	playerExport.Weapon_EquipSpell = Weapon_EquipSpell;
	playerExport.Weapon_EquipSwordStaff = Weapon_EquipSwordStaff;
	playerExport.Weapon_EquipHellStaff = Weapon_EquipHellStaff;
	playerExport.Weapon_EquipBow = Weapon_EquipBow;
	playerExport.Weapon_EquipArmor = Weapon_EquipArmor;
	playerExport.Weapon_CurrentShotsLeft = Weapon_CurrentShotsLeft;
	playerExport.Defence_CurrentShotsLeft = Defence_CurrentShotsLeft;

	playerExport.GetItemIndex = GetItemIndex;
	playerExport.GetItemByIndex = GetItemByIndex;
	playerExport.FindItemByClassname = FindItemByClassname;
	playerExport.FindItem = FindItem;
	playerExport.InitItems = InitItems;
	playerExport.GetPlayerItems = GetPlayerItems;
	playerExport.GetPlayerItemsCount = GetPlayerItemsCount;

	return &playerExport;
}

void
Com_Printf(const char *msg, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, msg);
	vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

	gi.dprintf("%s", text);
}
