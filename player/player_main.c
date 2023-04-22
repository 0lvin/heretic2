#include "player.h"
#include "p_anim_data.h"

PLAYER_API void P_Init(void)
{
	InitItems();
}

PLAYER_API void P_Shutdown(void)
{
}

PLAYER_API gitem_t* GetPlayerItems(int* num)
{
	return p_itemlist;
}

PLAYER_API player_export_t GetPlayerAPI(void)
{
	player_export_t playerExport;

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

// jmarshall
	playerExport.PlayerSeqData = PlayerSeqData;
	playerExport.PlayerChickenData = PlayerChickenData;
	playerExport.p_num_items = p_num_items;
	playerExport.p_itemlist = p_itemlist;
// jmarshall end

	return playerExport;
}

