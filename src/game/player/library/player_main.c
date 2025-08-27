//
// Heretic II
// Copyright 1998 Raven Software
//

#include "../../header/local.h"
#include "p_anim_data.h"
#include "p_main.h"

player_import_t pi;

static player_export_t pe;

Q2_DLL_EXPORTED player_export_t *
GetPlayerAPI(player_import_t *import)
{
	pi = *import;

	pe.PlayerReleaseRope = PlayerReleaseRope;
	pe.KnockDownPlayer = KnockDownPlayer;
	pe.PlayFly = PlayFly;
	pe.PlaySlap = PlaySlap;
	pe.PlayScratch = PlayScratch;
	pe.PlaySigh = PlaySigh;
	pe.SpawnDustPuff = SpawnDustPuff;
	pe.PlayerInterruptAction = PlayerInterruptAction;

	pe.BranchCheckDismemberAction = BranchCheckDismemberAction;

	pe.TurnOffPlayerEffects = TurnOffPlayerEffects;
	pe.AnimUpdateFrame = AnimUpdateFrame;
	pe.PlayerFallingDamage = PlayerFallingDamage;

	pe.PlayerBasicAnimReset = PlayerBasicAnimReset;
	pe.PlayerAnimReset = PlayerAnimReset;
	pe.PlayerAnimSetLowerSeq = PlayerAnimSetLowerSeq;
	pe.PlayerAnimSetUpperSeq = PlayerAnimSetUpperSeq;
	pe.PlayerAnimUpperIdle = PlayerAnimUpperIdle;
	pe.PlayerAnimLowerIdle = PlayerAnimLowerIdle;
	pe.PlayerAnimUpperUpdate = PlayerAnimUpperUpdate;
	pe.PlayerAnimLowerUpdate = PlayerAnimLowerUpdate;
	pe.PlayerAnimSetVault = PlayerAnimSetVault;
	pe.PlayerPlayPain = PlayerPlayPain;

	pe.PlayerIntLand = PlayerIntLand;

	pe.PlayerInit = PlayerInit;
	pe.PlayerUpdate = PlayerUpdate;
	pe.PlayerUpdateCmdFlags = PlayerUpdateCmdFlags;
	pe.PlayerUpdateModelAttributes = PlayerUpdateModelAttributes;

	return &pe;
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
