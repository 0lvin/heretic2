//
// p_anims.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "../../header/local.h"
#include "p_animactor.h"
#include "p_anim_branch.h"
#include "p_anim_data.h"
#include "p_anims.h"
#include "p_main.h"
#include "../../header/g_skeletons.h"
#include "../../common/fx.h"
#include "../../common/h2rand.h"
#include "../../common/effectflags.h"

void PlayerAnimSetUpperSeq(playerinfo_t *playerinfo, int seq)
{
	assert(playerinfo);

	if (playerinfo->upperseq != seq)
	{
		// We don't set all the data up right because it's up to AnimUpdateFrame to do this.

		playerinfo->upperseq = seq;
		playerinfo->upperframe = -1;
		playerinfo->upperidle = false;
	}

	playerinfo->uppermove = PlayerSeqData[seq].move;

	playerinfo->uppermove_index=seq;

	assert(playerinfo->uppermove);

	if (playerinfo->upperseq == ASEQ_NONE)
		playerinfo->upperidle = true;
}

void PlayerAnimSetLowerSeq(playerinfo_t *playerinfo, int seq)
{
	paceldata_t *seqdata;

	assert(playerinfo);

	if (playerinfo->lowerseq != seq)
	{
		// We don't set all the data up right because it's up to AnimUpdateFrame to do this.

		playerinfo->lowerseq = seq;
		playerinfo->lowerframe = -1;
		playerinfo->loweridle = false;
	}

	if (playerinfo->edictflags & FL_CHICKEN)
		playerinfo->lowermove = PlayerChickenData[seq].move;
	else
		playerinfo->lowermove = PlayerSeqData[seq].move;

	assert(playerinfo->lowermove);

	playerinfo->lowermove_index=seq;

	// The lower two bytes of the player flags are stomped by the sequences' flags.

	if (playerinfo->edictflags & FL_CHICKEN)
	{
		seqdata = &PlayerChickenData[seq];
	}
	else
	{
		seqdata = &PlayerSeqData[seq];
		playerinfo->viewheight = PlayerSeqData2[seq].viewheight;
	}

	playerinfo->flags = seqdata->playerflags | (playerinfo->flags & PLAYER_FLAG_PERSMASK);

	// Set / reset flag that says I am flying..

	if (seqdata->fly)
		playerinfo->edictflags |= FL_FLY;
	else
		playerinfo->edictflags &= ~FL_FLY;

	// Set / reset flag that says I am standing still.

	if (playerinfo->flags & PLAYER_FLAG_STAND)
		playerinfo->pm_flags |= PMF_STANDSTILL;
	else
		playerinfo->pm_flags &= ~PMF_STANDSTILL;

	// Set / reset flag that says I am movelocked.

	if (seqdata->lockmove)
		playerinfo->pm_flags |= PMF_LOCKMOVE;
	else
		playerinfo->pm_flags &= ~PMF_LOCKMOVE;
}

void PlayerBasicAnimReset(playerinfo_t *playerinfo)
{
	PlayerAnimSetLowerSeq(playerinfo, ASEQ_STAND);
	playerinfo->lowerframeptr = playerinfo->lowermove->frame;

	PlayerAnimSetUpperSeq(playerinfo, ASEQ_NONE);
	playerinfo->upperframeptr = playerinfo->uppermove->frame;

	playerinfo->effects|=EF_SWAPFRAME;
	playerinfo->effects &= ~(EF_DISABLE_EXTRA_FX | EF_ON_FIRE | EF_TRAILS_ENABLED);

	PlayerSetHandFX(playerinfo, HANDFX_NONE, -1);

	if (playerinfo->pers.weaponready == WEAPON_READY_NONE)		// Just in case we die with WEAPON_READY_NONE
		playerinfo->pers.weaponready = WEAPON_READY_HANDS;

	playerinfo->switchtoweapon = playerinfo->pers.weaponready;
	playerinfo->self->client->newweapon = NULL;

	// Straighten out joints, i.e. reset torso twisting.

	if (!(playerinfo->edictflags&FL_CHICKEN))
		pi.ResetJointAngles(playerinfo->self);

	memset(playerinfo->seqcmd,0,ACMD_MAX*sizeof(int));
}

void PlayerAnimReset(playerinfo_t *playerinfo)
{
	PlayerAnimSetLowerSeq(playerinfo, ASEQ_STAND);
	playerinfo->lowerframeptr = playerinfo->lowermove->frame;

	PlayerAnimSetUpperSeq(playerinfo, ASEQ_NONE);
	playerinfo->upperframeptr = playerinfo->uppermove->frame;

	playerinfo->pers.armortype = ARMOR_TYPE_NONE;
	playerinfo->pers.bowtype = BOW_TYPE_NONE;
	playerinfo->pers.stafflevel = STAFF_LEVEL_BASIC;
	playerinfo->pers.helltype = HELL_TYPE_BASIC;
	playerinfo->pers.altparts = 0;
	playerinfo->pers.weaponready = WEAPON_READY_HANDS;
	playerinfo->switchtoweapon = WEAPON_READY_HANDS;
	playerinfo->self->client->newweapon = NULL;
	PlayerUpdateModelAttributes(playerinfo->self->client);
	playerinfo->pers.handfxtype = HANDFX_NONE;

	PlayerSetHandFX(playerinfo, HANDFX_NONE, -1);

	playerinfo->effects|=EF_SWAPFRAME;
	playerinfo->effects &= ~(EF_DISABLE_EXTRA_FX | EF_ON_FIRE | EF_TRAILS_ENABLED);

	// Straighten out joints, i.e. no torso aiming.

	if (!(playerinfo->edictflags & FL_CHICKEN))
		pi.ResetJointAngles(playerinfo->self);

	memset(playerinfo->seqcmd,0,ACMD_MAX * sizeof(int));
}

int PlayerAnimWeaponSwitch(playerinfo_t *playerinfo)
{
	qboolean BranchCheckDismemberAction(playerinfo_t *playerinfo, int weapon);

	int newseq;

	assert(playerinfo);

	// See if we have the arm to do that magic.

	if (playerinfo->switchtoweapon != playerinfo->pers.weaponready)
	{
		if (!BranchCheckDismemberAction(playerinfo, playerinfo->switchtoweapon))
			return ASEQ_NONE;

		newseq = PlayerAnimWeaponSwitchSeq[playerinfo->pers.weaponready][playerinfo->switchtoweapon];
		if (newseq)
		{
			PlayerAnimSetUpperSeq(playerinfo, newseq);
			return newseq;
		}
	}
	else if (playerinfo->self->client->newweapon)
	{
		if (!BranchCheckDismemberAction(playerinfo, playerinfo->self->client->newweapon->tag))
			return ASEQ_NONE;

		newseq = PlayerAnimWeaponSwitchSeq[playerinfo->pers.weaponready][playerinfo->pers.weaponready];
		if (newseq)
		{
			PlayerAnimSetUpperSeq(playerinfo, newseq);
			return newseq;
		}
	}
	return ASEQ_NONE;
}

void PlayerAnimUpperIdle(playerinfo_t *playerinfo)
{
	int ret;

	if ((ret = BranchUprReady(playerinfo)))
	{
		PlayerAnimSetUpperSeq(playerinfo, ret);

		assert(playerinfo->uppermove);
	}

	assert(playerinfo->uppermove);
}

void PlayerAnimLowerIdle(playerinfo_t *playerinfo)
{
	int ret;

	if (playerinfo->flags & PLAYER_FLAG_SURFSWIM)
	{
		if ((ret = BranchLwrSurfaceSwim(playerinfo)))
		{
			PlayerAnimSetLowerSeq(playerinfo, ret);
		}
	}
	else if (playerinfo->flags & PLAYER_FLAG_UNDERWATER)
	{
		if ((ret = BranchLwrUnderwaterSwim(playerinfo)))
		{
			PlayerAnimSetLowerSeq(playerinfo, ret);
		}
	}
	else if (playerinfo->flags & PLAYER_FLAG_ONROPE)
	{
		if ((ret = BranchLwrClimbing(playerinfo)))
		{
			PlayerAnimSetLowerSeq(playerinfo, ret);
		}
	}
	else if ((ret = BranchLwrStanding(playerinfo)))
	{
		PlayerAnimSetLowerSeq(playerinfo, ret);
	}
	else
	{
		if (playerinfo->leveltime - playerinfo->idletime > 15.0)
		{
			if (playerinfo->lowerseq >= ASEQ_IDLE_READY_GO && playerinfo->lowerseq <= ASEQ_IDLE_LOOKR && playerinfo->lowerseq != ASEQ_IDLE_READY_END)
			{
				// Only certain idle should be called out of here.

				switch(pi.irand(playerinfo, 0, 3))
				{
					case 0:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_LOOKL);
						break;
					case 1:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_LOOKR);
						break;
					case 2:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_READY_END);
						break;
					default:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_READY);
						break;
				}
			}
			// if we are in a cinematic, always do this idle, since its silent
			else
			if (playerinfo->sv_cinematicfreeze)
				PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_LOOKBACK);
			else if ((playerinfo->pers.weaponready == WEAPON_READY_BOW))
			{
				// Because the bow doesn't look right in some idles.

				switch(pi.irand(playerinfo, 0, 2))
				{
					case 0:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_SCRATCH_ASS);
						break;
					case 1:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_LOOKBACK);
						break;
					default:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_READY_GO);
						break;
				}
			}
			else if ((playerinfo->pers.weaponready == WEAPON_READY_SWORDSTAFF))
			{
				// Because the staff doesn't look right in some idles.

				switch(pi.irand(playerinfo, 0, 3))
				{
					case 0:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_FLY1);
						break;
					case 1:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_FLY2);
						break;
					case 2:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_WIPE_BROW);
						break;
					default:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_READY_GO);
						break;
				}
			}
			else
			{
				switch(pi.irand(playerinfo, 0, 6))
				{
					case 0:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_FLY1);
						break;
					case 1:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_FLY2);
						break;
					case 2:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_SCRATCH_ASS);
						break;
					case 3:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_LOOKBACK);
						break;
					case 4:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_WIPE_BROW);
						break;
					default:
						PlayerAnimSetLowerSeq(playerinfo, ASEQ_IDLE_READY_GO);
						break;
				}
			}
		}
	}
}

void PlayerAnimUpperUpdate(playerinfo_t *playerinfo)
{
	seqctrl_t *seqctrl;

	int newseq=ASEQ_NONE;

	/*
	// first check if the lower anim forces the lower anim to lock in sync with it.
	if (PlayerSeqData2[playerinfo->lowerseq].nosplit)
	{
		// A seq value of NONE indicates that it is not asserting a move, copy the companion half.
		playerinfo->upperseq = ASEQ_NONE;
		playerinfo->upperidle = true;
		return;
	}
	*/

	// Init some values.

	playerinfo->upperidle = false;

	// Grab the sequence ctrl struct.

	seqctrl = &SeqCtrl[playerinfo->upperseq];

	// First check the branch function. This evaluates "extra" command flags for a potential
	// modification of the "simple" procedure.

	if (seqctrl->branchfunc)
	{
		newseq = seqctrl->branchfunc(playerinfo);
	}

	if (newseq == ASEQ_NONE)
	{
		if (seqctrl->command != ACMD_NONE)
		{
			if (playerinfo->seqcmd[seqctrl->command])
			{
				newseq = seqctrl->continueseq;
			}
			else
			{
				newseq = seqctrl->ceaseseq;
			}
		}
		else
		{
			newseq = seqctrl->ceaseseq;
		}
	}

	// Now check for idles.  If the upper half has an idle, then the upper half is copied.

	if (newseq == ASEQ_NONE)
	{
		if (playerinfo->lowerseq == ASEQ_NONE)
		{
			newseq=BranchIdle(playerinfo);
			playerinfo->loweridle = true;
		}
		playerinfo->upperidle = true;
	}

	PlayerAnimSetUpperSeq(playerinfo, newseq);
}

void PlayerAnimLowerUpdate(playerinfo_t *playerinfo)
{
	seqctrl_t	*seqctrl;
	int newseq = ASEQ_NONE;

	// Init some values.
	playerinfo->loweridle = false;

	// Grab the sequence ctrl struct.
	if (playerinfo->edictflags & FL_CHICKEN)
		seqctrl = &ChickenCtrl[playerinfo->lowerseq];
	else
		seqctrl = &SeqCtrl[playerinfo->lowerseq];

	// Check for noclip, just to make things more robust.
	if (playerinfo->movetype == MOVETYPE_NOCLIP)
	{
		if (playerinfo->lowerseq != ASEQ_STAND)
		{
			PlayerAnimSetLowerSeq(playerinfo, ASEQ_STAND);
		}
	}

	if (!newseq)	// That is if that waterseq transition wasn't necessary...
	{
		// First check the branch function. This evaluates "extra" command flags for a potential
		// modification of the "simple" procedure.

		if (seqctrl->branchfunc)
		{
			newseq = seqctrl->branchfunc(playerinfo);
		}
	}

	// If even after the special-case BranchFunc didn't indicate a new sequence...

	if (!newseq)
	{
		// The seqctrl indicates the control flag that this sequence is dependent on.
		// We've defined a continue and terminate sequence depending on it.

		if (seqctrl->command != ACMD_NONE)
		{
			if (playerinfo->seqcmd[seqctrl->command])
			{
				newseq = seqctrl->continueseq;
			}
			else
			{
				newseq = seqctrl->ceaseseq;
			}
		}
		else
		{
			newseq = seqctrl->ceaseseq;
		}
	}

	// Now check for idles.  If the lower half has an idle, then the upper half is copied.
	PlayerAnimSetLowerSeq(playerinfo, newseq);
}

void PlayerAnimSetVault(playerinfo_t *playerinfo, int seq)
{
	assert(playerinfo);

	PlayerAnimSetLowerSeq(playerinfo, seq);
	playerinfo->fwdvel	= 0.0;
	playerinfo->sidevel = 0.0;
	playerinfo->upvel	= 0.0;
	playerinfo->edictflags |= FL_FLY | FL_LOCKMOVE;
	playerinfo->flags = PlayerSeqData[ASEQ_VAULT_LOW].playerflags | (playerinfo->flags & PLAYER_FLAG_PERSMASK);
	playerinfo->pm_flags |= PMF_LOCKMOVE;
	VectorClear(playerinfo->velocity);

	if (playerinfo->waterlevel>1)
		playerinfo->waterlevel = 1;
}

void PlayerPlayPain(playerinfo_t *playerinfo, int type)
{
	int chance = irand(0,100);

	if (!(playerinfo->edictflags & FL_CHICKEN))
	{
		// Chicken plays no pain sound.
		switch (type)
		{
			// Normal.

			case 0:
				if (chance < 50)
					pi.G_Sound(SND_PRED_ID40,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, pi.G_SoundIndex("*pain1.wav"), 1.0, ATTN_NORM, 0);
				else
					pi.G_Sound(SND_PRED_ID41,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, pi.G_SoundIndex("*pain2.wav"), 1.0, ATTN_NORM, 0);

				break;

			// Gas.

			case 1:
				if (chance < 33)
					pi.G_Sound(SND_PRED_ID42,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, pi.G_SoundIndex("*cough1.wav"), 1.0, ATTN_NORM, 0);
				else if (chance < 66)
					pi.G_Sound(SND_PRED_ID43,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, pi.G_SoundIndex("*cough2.wav"), 1.0, ATTN_NORM, 0);
				else
					pi.G_Sound(SND_PRED_ID44,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, pi.G_SoundIndex("*cough3.wav"), 1.0, ATTN_NORM, 0);

				break;

			// Small.

			case 2:
				pi.G_Sound(SND_PRED_ID45,playerinfo->leveltime,playerinfo->self, CHAN_VOICE, pi.G_SoundIndex("*ow.wav"), 1.0, ATTN_NORM, 0);
				break;
		}
	}
}
