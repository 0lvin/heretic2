//
// Heretic II
// Copyright 1998 Raven Software
//
#include "../../../common/header/common.h"
#include "../../header/game.h"
#include "player.h"
#include "p_anim_data.h"

player_import_t pi;

void P_Init(void)
{
}

void P_Shutdown(void)
{
}

player_export_t playerExport;

Q2_DLL_EXPORTED player_export_t *
GetPlayerAPI(player_import_t *import)
{
	pi = *import;

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

	pi.dprintf("%s", text);
}
