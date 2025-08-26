/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (c) ZeniMax Media Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * The "camera" through which the player looks into the game.
 *
 * =======================================================================
 */

#include "../header/local.h"
#include "../monster/misc/player.h"
#include "../header/g_skeletons.h"
#include "../header/g_teleport.h"
#include "../player/library/p_animactor.h"
#include "../player/library/p_anims.h"
#include "../player/library/p_ctrl.h"
#include "../player/library/p_main.h"
#include "../common/h2rand.h"

static edict_t *current_player;
static gclient_t *current_client;

static vec3_t forward, right, up;
static float xyspeed;

static float bobmove;
static int bobcycle; /* odd cycles are right foot going forward */
static float bobfracsin; /* sin(bobfrac*M_PI) */

static float
SV_CalcRoll(vec3_t angles, vec3_t velocity)
{
	float sign;
	float side;
	float value;

	side = DotProduct(velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);

	value = sv_rollangle->value;

	if (side < sv_rollspeed->value)
	{
		side = side * value / sv_rollspeed->value;
	}
	else
	{
		side = value;
	}

	return side * sign;
}

/*
 * Handles color blends and view kicks
 */
static void
P_DamageFeedback(edict_t *player)
{
	gclient_t *client;
	int count;

	if (!player)
	{
		return;
	}

	/* death/gib sound is now aggregated and played here */
	if (player->sounds)
	{
		gi.sound(player, CHAN_VOICE, player->sounds, 1, ATTN_NORM, 0);
		player->sounds = 0;
	}

	client = player->client;

	/* flash the backgrounds behind the status numbers */
	client->ps.stats[STAT_FLASHES] = 0;

	if (client->damage_blood)
	{
		client->ps.stats[STAT_FLASHES] |= 1;
	}

	/* total points of damage shot at the player this frame */
	count = client->damage_blood;
	if (count == 0)
	{
		return; /* didn't take any damage */
	}

	//Check for gasssss damage
	if (player->pain_debounce_time < level.time && client->damage_gas)
	{
		if ( client->playerinfo.loweridle && client->playerinfo.upperidle )
			playerExport->PlayerAnimSetLowerSeq(&client->playerinfo, ASEQ_PAIN_A);

		playerExport->PlayerPlayPain(&client->playerinfo, 1);
	}
	else if (((!irand(0, 4)) || count > 8) && (player->pain_debounce_time < level.time)) // Play pain animation.
	{
		if ( client->playerinfo.loweridle && client->playerinfo.upperidle )
			playerExport->PlayerAnimSetLowerSeq(&client->playerinfo, ASEQ_PAIN_A);

		if (count <= 4)
			playerExport->PlayerPlayPain(&client->playerinfo, 2);
		else
			playerExport->PlayerPlayPain(&client->playerinfo, 0);

		player->pain_debounce_time = level.time + 0.5;
	}

	/* play an apropriate pain sound */
	if ((level.time > player->pain_debounce_time) &&
		!(player->flags & FL_GODMODE) &&
		(client->invincible_framenum <= level.framenum) &&
		player->health > 0)
	{
		player->pain_debounce_time = level.time + 0.7;
	}

	/* clear totals */
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_parmor = 0;
	client->damage_knockback = 0;
}

/*
 * Auto pitching on slopes?
 *
 * fall from 128: 400 = 160000
 * fall from 256: 580 = 336400
 * fall from 384: 720 = 518400
 * fall from 512: 800 = 640000
 * fall from 640: 960 =
 *
 * damage = deltavelocity*deltavelocity  * 0.0001
 */
static void
SV_CalcViewOffset(edict_t *ent)
{
	float *angles;
	float bob;
	float ratio;
	float delta;
	vec3_t v;

	if (!ent)
	{
		return;
	}

	/* base angles */
	angles = ent->client->ps.kick_angles;

	/* if dead, fix the angle and don't add any kick */
	if (ent->deadflag)
	{
		VectorClear(angles);

		if (ent->flags & FL_SAM_RAIMI)
		{
			ent->client->ps.viewangles[ROLL] = 0;
			ent->client->ps.viewangles[PITCH] = 0;
		}
		else
		{
			ent->client->ps.viewangles[ROLL] = 40;
			ent->client->ps.viewangles[PITCH] = -15;
		}

		ent->client->ps.viewangles[YAW] = ent->client->killer_yaw;
	}
	else
	{
		/* add angles based on weapon kick */
		VectorCopy(ent->client->kick_angles, angles);

		/* add angles based on damage kick */
		ratio = (ent->client->v_dmg_time - level.time) / DAMAGE_TIME;

		if (ratio < 0)
		{
			ratio = 0;
			ent->client->v_dmg_pitch = 0;
			ent->client->v_dmg_roll = 0;
		}

		angles[PITCH] += ratio * ent->client->v_dmg_pitch;
		angles[ROLL] += ratio * ent->client->v_dmg_roll;

		/* add pitch based on fall kick */
		ratio = (ent->client->fall_time - level.time) / FALL_TIME;

		if (ratio < 0)
		{
			ratio = 0;
		}

		angles[PITCH] += ratio * ent->client->fall_value;

		/* add angles based on velocity */
		delta = DotProduct(ent->velocity, forward);
		angles[PITCH] += delta * run_pitch->value;

		delta = DotProduct(ent->velocity, right);
		angles[ROLL] += delta * run_roll->value;

		/* add angles based on bob */
		delta = bobfracsin * bob_pitch->value * xyspeed;

		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			delta *= 6; /* crouching */
		}

		angles[PITCH] += delta;
		delta = bobfracsin * bob_roll->value * xyspeed;

		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			delta *= 6; /* crouching */
		}

		if (bobcycle & 1)
		{
			delta = -delta;
		}

		angles[ROLL] += delta;
	}

	/* =================================== */

	/* base origin */
	VectorClear(v);

	/* add view height */
	v[2] += ent->viewheight;

	/* add fall height */
	ratio = (ent->client->fall_time - level.time) / FALL_TIME;

	if (ratio < 0)
	{
		ratio = 0;
	}

	v[2] -= ratio * ent->client->fall_value * 0.4;

	/* add bob height */
	bob = bobfracsin * xyspeed * bob_up->value;

	if (bob > 6)
	{
		bob = 6;
	}

	v[2] += bob;

	/* add kick offset */
	VectorAdd(v, ent->client->kick_origin, v);

	/* absolutely bound offsets
	   so the view can never be
	   outside the player box */
	if (!ent->client->chasetoggle)
	{
		if (v[0] < -14)
		{
			v[0] = -14;
		}
		else if (v[0] > 14)
		{
			v[0] = 14;
		}

		if (v[1] < -14)
		{
			v[1] = -14;
		}
		else if (v[1] > 14)
		{
			v[1] = 14;
		}

		if (v[2] < -22)
		{
			v[2] = -22;
		}
		else if (v[2] > 30)
		{
			v[2] = 30;
		}
	}
	else
	{
		VectorSet(v, 0, 0, 0);
		if (ent->client->chasecam)
		{
			int i;

			/*
			 * code had used ent->client->ps.pmove.origin,
			 * that can't be unused with 4k+ coordinates,
			 * so use viewoffset with clamp
			 */
			VectorSubtract(ent->client->chasecam->s.origin, ent->s.origin, v);

			/* Clamp coordinates to -30..30 */
			for (i = 0; i < 3; i++)
			{
				if (v[i] > 30)
				{
					v[i] = 30;
				}
				else if (v[i] < -30)
				{
					v[i] = -30;
				}
			}
		}
	}

	VectorCopy(v, ent->client->ps.viewoffset);
}

void
SetupPlayerinfo(edict_t *ent)
{
	int	i;

	// ********************************************************************************************
	// Inputs only.
	// ********************************************************************************************

	// Pointer to the associated player's edict_t.

	ent->client->playerinfo.self = ent;

	// Game .dll variables.
	ent->client->playerinfo.leveltime = level.time;

	// Server variables.

	ent->client->playerinfo.sv_gravity = sv_gravity->value;
	ent->client->playerinfo.sv_cinematicfreeze = sv_cinematicfreeze->value;
	ent->client->playerinfo.sv_jumpcinematic = sv_jumpcinematic->value;

	// From edict_t.
	ent->client->playerinfo.groundentity = ent->groundentity;

	// Deathmatch flags - only set this if we are in death match.
	if (deathmatch->value)
	{
		// Send the high bit if deathmatch.
		ent->client->playerinfo.dmflags = DF_DEATHMATCH_SET | (int)dmflags->value;
	}
	else
	{
		ent->client->playerinfo.dmflags = 0;
	}

	ent->client->playerinfo.advancedstaff = (int)(advancedstaff->value);

	// ********************************************************************************************
	// Inputs & outputs.
	// ********************************************************************************************

	// If we are in a cinematic, remove certain commands from the ucmd_t the server received from
	// the client. NOTE: THIS IS HIGHLY SUBJECTIVE. REQUIRES VIGOUROUS TESTING.
	// Basically, just killing all buttons pressed while a cinematic is running - Probably not the best way to do this
	// Jake 9/28/98
	// need to reget this constantly, since it changes on the fly.
	if (sv_cinematicfreeze->value)
	{
		ent->client->pcmd.buttons = 0;
		ent->client->pcmd.sidemove = 0;
		ent->client->pcmd.forwardmove = 0;
		ent->client->buttons = 0;
	}

	// From edict_t.
	ent->client->playerinfo.target = ent->target;
	ent->client->playerinfo.teamchain = ent->teamchain;
	ent->client->playerinfo.target_ent = ent->target_ent;
	ent->client->playerinfo.nextthink = ent->nextthink;
	ent->client->playerinfo.viewheight = ent->viewheight;
	ent->client->playerinfo.watertype = ent->watertype;
	ent->client->playerinfo.waterlevel = ent->waterlevel;
	ent->client->playerinfo.deadflag = ent->deadflag;
	ent->client->playerinfo.movetype = ent->movetype;

	// From entity_state_t.
	ent->client->playerinfo.frame = ent->s.frame,
	ent->client->playerinfo.swapFrame = ent->s.swapFrame;
	ent->client->playerinfo.effects = ent->s.effects;
	ent->client->playerinfo.renderfx = ent->s.renderfx;
	ent->client->playerinfo.skinnum = ent->s.skinnum;
	ent->client->playerinfo.clientnum = ent->s.clientnum;

	for(i = 0; i < MAX_FM_MESH_NODES; i++)
	{
		ent->client->playerinfo.fmnodeinfo[i] = ent->s.fmnodeinfo[i];
	}

	// From pmove_state_t.
	ent->client->playerinfo.pm_flags = ent->client->ps.pmove.pm_flags;
}

void
WritePlayerinfo(edict_t *ent)
{
	int	i;

	// ********************************************************************************************
	// Inputs & outputs.
	// ********************************************************************************************
	// From edict_t.
	ent->teamchain = ent->client->playerinfo.teamchain;
	ent->target_ent = ent->client->playerinfo.target_ent;
	ent->target = ent->client->playerinfo.target;
	ent->nextthink = ent->client->playerinfo.nextthink;
	ent->viewheight = ent->client->playerinfo.viewheight;
	ent->watertype = ent->client->playerinfo.watertype;
	ent->waterlevel = ent->client->playerinfo.waterlevel;
	ent->deadflag = ent->client->playerinfo.deadflag;
	ent->movetype = ent->client->playerinfo.movetype;

	// From entity_state_t.
	ent->s.frame = ent->client->playerinfo.frame,
	ent->s.swapFrame = ent->client->playerinfo.swapFrame;
	ent->s.effects = ent->client->playerinfo.effects;
	ent->s.renderfx = ent->client->playerinfo.renderfx;
	ent->s.skinnum = ent->client->playerinfo.skinnum;
	ent->s.clientnum = ent->client->playerinfo.clientnum;
	ent->rrs.mesh = 0;

	for(i = 0; i < MAX_FM_MESH_NODES;i++)
	{
		ent->s.fmnodeinfo[i] = ent->client->playerinfo.fmnodeinfo[i];
		if (ent->client->playerinfo.fmnodeinfo[i].flags & FMNI_NO_DRAW)
		{
			ent->rrs.mesh |= (1 << i);
		}
	}

	// From pmove_state_t.
	ent->client->ps.pmove.pm_flags = ent->client->playerinfo.pm_flags;

	// ********************************************************************************************
	// Outputs only.
	// ********************************************************************************************

	// From playerstate_t.
	VectorCopy(ent->client->playerinfo.offsetangles,ent->client->ps.offsetangles);
}

// ************************************************************************************************
// SetupPlayerinfo_effects
// -----------------------
// ************************************************************************************************

void
SetupPlayerinfo_effects(edict_t *ent)
{
	int i;

	ent->client->playerinfo.effects = ent->s.effects;
	ent->client->playerinfo.renderfx = ent->s.renderfx;
	ent->client->playerinfo.skinnum = ent->s.skinnum;
	ent->client->playerinfo.clientnum = ent->s.clientnum;

	for(i = 0; i < MAX_FM_MESH_NODES; i++)
	{
		ent->client->playerinfo.fmnodeinfo[i] = ent->s.fmnodeinfo[i];
	}
}

// ************************************************************************************************
// WritePlayerinfo_effects
// -----------------------
// ************************************************************************************************

void WritePlayerinfo_effects(edict_t *ent)
{
	int i;

	ent->s.effects = ent->client->playerinfo.effects;
	ent->s.renderfx = ent->client->playerinfo.renderfx;
	ent->s.skinnum = ent->client->playerinfo.skinnum;
	ent->s.clientnum = ent->client->playerinfo.clientnum;
	ent->rrs.mesh = 0;

	for(i = 0; i < MAX_FM_MESH_NODES; i++)
	{
		ent->s.fmnodeinfo[i] = ent->client->playerinfo.fmnodeinfo[i];
		if (ent->s.fmnodeinfo[i].flags & FMNI_NO_DRAW)
		{
			ent->rrs.mesh |= (1 << i);
		}
	}
}

// ************************************************************************************************
// PlayerTimerUpdate
// -----------------
// Deal with incidental player stuff, like setting the personal light to OFF if its should be.
// ************************************************************************************************

void PlayerTimerUpdate(edict_t *ent)
{
	playerinfo_t *playerinfo;

	playerinfo = &ent->client->playerinfo;

	// Disable light when we should.

	if (playerinfo->light_timer < level.time)
		playerinfo->effects &= ~EF_LIGHT_ENABLED;

	// Disable speed when we should.

	if (playerinfo->speed_timer < level.time)
		playerinfo->effects &= ~EF_SPEED_ACTIVE;

	// Disable max speed when we should.

	if (playerinfo->knockbacktime < level.time)
		playerinfo->effects &= ~EF_HIGH_MAX;

	// Disable powerup when we should.

	if (playerinfo->powerup_timer < level.time)
		playerinfo->effects &= ~EF_POWERUP_ENABLED;

	// Disable relection when we should.

	if (playerinfo->reflect_timer < level.time)
	{
		// Were we relfective last frame?

		if (playerinfo->renderfx &RF_REFLECTION)
		{
			playerinfo->renderfx &= ~RF_REFLECTION;

			// Unset the skin.

			playerExport->PlayerUpdateModelAttributes(ent);
		}
	}

	// Disable ghosting when we should.

	if (playerinfo->ghost_timer < level.time)
		playerinfo->renderfx &= ~RF_TRANS_GHOST;
}

static void
P_WorldEffects(void)
{
	int waterlevel, old_waterlevel;
	vec3_t Origin, Dir;

	if (current_player->client->playerinfo.deadflag > DEAD_NO)
		return;

	// If we are in no clip, or we have special lungs, we don't need air.

	if (current_player->movetype == MOVETYPE_NOCLIP)
	{
		current_player->air_finished = level.time + HOLD_BREATH_TIME;
		return;
	}

	waterlevel = current_player->waterlevel;
	old_waterlevel = current_client->old_waterlevel;
	current_client->old_waterlevel = waterlevel;

	/* if just entered a water volume, play a sound */
	if (!old_waterlevel && waterlevel)
	{
		// Clear damage_debounce, so the pain sound will play immediately.

		current_player->damage_debounce_time = level.time-1;

		if (current_player->watertype & CONTENTS_LAVA)
		{
			gi.sound(current_player, CHAN_BODY,
					gi.soundindex("player/inlava.wav"), 1, ATTN_NORM, 0);
			current_player->flags |= FL_INLAVA;
		}
		else if (current_player->watertype & CONTENTS_SLIME)
		{
			gi.sound(current_player, CHAN_BODY,
					gi.soundindex("player/muckin.wav"), 1, ATTN_NORM, 0);
			current_player->flags |=  FL_INSLIME;
		}
		else
		{
			gi.sound(current_player, CHAN_BODY,
					gi.soundindex("player/Water Enter.wav"), 1, ATTN_NORM, 0);
		}

		current_player->flags |= FL_INWATER;

		VectorCopy(current_player->s.origin,Origin);

		Origin[2] += current_player->client->playerinfo.waterheight;

		// Fixme: Need to determine the actual water surface normal - if we have any sloping water??

		Dir[0] = 0.0;
		Dir[1] = 0.0;
		Dir[2] = 1.0;

		VectorCopy(Origin,current_player->client->playerinfo.LastWatersplashPos);

		gi.CreateEffect(NULL,
						FX_WATER_ENTRYSPLASH,
						CEF_FLAG7,
						Origin,
						"bd",
						128|96,			// FIXME: Size propn. to entry velocity?
						Dir);
	}
	else if (old_waterlevel&&!waterlevel)
	{

		//
		// If the current player just completely exited a water volume, play a sound.
		//

		// INWATER is set whether in lava, slime or water.
		if (current_player->flags & FL_INLAVA)
		{
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/inlava.wav"), 1, ATTN_NORM, 0);
			current_player->flags &= ~FL_INLAVA;
		}
		else if (current_player->flags & FL_INSLIME)
		{
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/muckexit.wav"), 1, ATTN_NORM, 0);
			current_player->flags &= ~FL_INSLIME;
		}
		else
		{
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/Water Exit.wav"), 1, ATTN_NORM, 0);
		}
		current_player->flags &= ~FL_INWATER;

		VectorCopy(current_player->s.origin,Origin);

		Origin[2] = current_player->client->playerinfo.LastWatersplashPos[2];

		// Fixme: Need to determine the actual water surface normal - if we have any sloping water??

		Dir[0] = 0.0;
		Dir[1] = 0.0;
		Dir[2] = 1.0;

		VectorCopy(Origin,current_player->client->playerinfo.LastWatersplashPos);

		gi.CreateEffect(NULL,
						FX_WATER_ENTRYSPLASH,
						0,
						Origin,
						"bd",
						96,				// FIXME: Size propn. to exit velocity.
						Dir);
	}

	//
	// Start a waterwake effect if the current player has been in water and still is in water.
	//

	if (waterlevel && (old_waterlevel&&waterlevel < 3) && (VectorLength(current_player->velocity) != 0.0))
	{
		// no ripples while in cinematics
		if (sv_cinematicfreeze->value)
		{
			return;
		}

		if ((((int)(current_client->bobtime+bobmove)) != bobcycle) && (!sv_cinematicfreeze->value))
		{
			// FIXME: Doing more work then we need to here???? How about re-writing this so that it
			// is always active on the client and does water tests itself? We'll see - currently not
			// enough info. is available on the client to try this.
			vec3_t	Angles, Temp;
			byte	angle_byte;

			VectorCopy(current_player->velocity, Temp);

			VectorNormalize(Temp);

			VectoAngles(Temp,Angles);

			VectorCopy(current_player->s.origin,Origin);

			Origin[2] += current_player->client->playerinfo.waterheight;

			angle_byte = Q_ftol(((Angles[YAW] + 180.0F)/360.0) * 255.0);

			gi.CreateEffect(NULL,
							FX_WATER_WAKE,
							0,
							Origin,
							"sbv",
							(short)current_player->s.number,
							angle_byte,
							current_player->velocity);
		}
	}

	/* check for head just coming out of water */
	if ((old_waterlevel == 3) && (waterlevel != 3))
	{
		if (current_player->air_finished < level.time)
		{
			/* gasp for air */
			if (irand(0,1))
				gi.sound(current_player, CHAN_BODY, gi.soundindex("*gasp1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(current_player, CHAN_BODY, gi.soundindex("*gasp2.wav"), 1, ATTN_NORM, 0);

		}
		else if (current_player->air_finished < level.time + 11)
		{
			/* just break surface */
			gi.sound(current_player, CHAN_BODY, gi.soundindex("*waterresurface.wav"), 1, ATTN_NORM, 0);
		}
	}

	/* check for drowning */
	if (waterlevel == 3)
	{
		if (current_player->watertype & CONTENTS_SLIME)
		{
			// Slime should kill really quick.

			current_player->dmg = 25;

			// Play a gurp sound instead of a normal pain sound.

			if (irand(0, 1))
				gi.sound(current_player, CHAN_VOICE, gi.soundindex("*drowning1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(current_player, CHAN_VOICE, gi.soundindex("*drowning2.wav"), 1, ATTN_NORM, 0);

			current_player->pain_debounce_time = level.time;

			T_Damage(current_player,
					 world,
					 world,
					 vec3_origin,
					 current_player->s.origin,
					 vec3_origin,
					 current_player->dmg,
					 0,
					 DAMAGE_SUFFOCATION,
					 MOD_SLIME);
		}
		/* if out of air, start drowning */
		else if ((current_player->air_finished + current_player->client->playerinfo.lungs_timer) < level.time)
		{
			/* drown! */
			if ((current_player->client->next_drown_time < level.time) &&
				(current_player->health > 0))
			{
				current_player->client->next_drown_time = level.time + 1;

				/* take more damage the longer underwater */
				current_player->dmg += 2;

				if (current_player->dmg > 15)
				{
					current_player->dmg = 15;
				}

				/* play a gurp sound instead of a normal pain sound */
				if (current_player->health <= current_player->dmg)
				{
					gi.sound(current_player, CHAN_VOICE,
							gi.soundindex("player/drown1.wav"), 1, ATTN_NORM, 0);
				}
				else if (randk() & 1)
				{
					gi.sound(current_player, CHAN_VOICE,
							gi.soundindex("*drowning1.wav"), 1, ATTN_NORM, 0);
				}
				else
				{
					gi.sound(current_player, CHAN_VOICE,
							gi.soundindex("*drowning2.wav"), 1, ATTN_NORM, 0);
				}

				current_player->pain_debounce_time = level.time;

				T_Damage(current_player, world, world, vec3_origin,
						current_player->s.origin, vec3_origin,
						current_player->dmg, 0, DAMAGE_SUFFOCATION,
						MOD_WATER);
			}

		}
		// else, we aren't drowning yet, but we may well need to decrement the amount of extra lungs we have from shrines
		// since we still have lungs, reset air finished till we don't anymore.
		else
		if (current_player->client->playerinfo.lungs_timer)
		{
			// floatinf point inaccuracy never lets us equal zero by ourselves
			if (current_player->client->playerinfo.lungs_timer < 0.1)
				current_player->client->playerinfo.lungs_timer = 0.0;
			else
			{
				current_player->client->playerinfo.lungs_timer -= 0.1;
				current_player->air_finished = level.time + HOLD_BREATH_TIME;
			}
		}

		if (old_waterlevel !=  3)
		{	// We weren't underwater before this, so play a submerged sound
			gi.sound(current_player, CHAN_VOICE, gi.soundindex("player/underwater.wav"), 1, ATTN_IDLE, 0);
		}

		// Play an underwater sound every 4 seconds!
		if (((int)(level.time/4.0))*4.0 == level.time)
		{	// Underwater ambient
			// Play local only?
			gi.sound(current_player, CHAN_BODY, gi.soundindex("player/underwater.wav"), 1, ATTN_IDLE, 0);
		}
	}
	else
	{
		current_player->air_finished = level.time + HOLD_BREATH_TIME;
		current_player->dmg = 2;
	}

	// ********************************************************************************************
	// Handle lava sizzle damage.
	// ********************************************************************************************

	if (waterlevel && (current_player->watertype&(CONTENTS_LAVA)) )
	{
		if (current_player->health > 0 && current_player->pain_debounce_time <= level.time)
		{
			gi.sound(current_player, CHAN_VOICE, gi.soundindex("player/lavadamage.wav"), 1, ATTN_NORM, 0);

			current_player->pain_debounce_time = level.time + 1;
		}

		T_Damage(current_player,
				 world,
				 world,
				 vec3_origin,
				 current_player->s.origin,
				 vec3_origin,
				 (waterlevel>2)?25:(3*waterlevel),
				 0,
				 DAMAGE_LAVA,
				 MOD_LAVA);
	}
}

void
G_SetClientSound(edict_t *ent)
{
}

void
G_SetClientFrame(edict_t *ent, float speed)
{
	if (speed)
	{
		xyspeed = speed;
	}
}

/*
 * Called for each player at the end of
 * the server frame and right after spawning
 */
void
ClientEndServerFrame(edict_t *ent)
{
	float bobtime;
	int i, index;

	if (!ent)
	{
		return;
	}

	current_player = ent;
	current_client = ent->client;

	/* check fog changes */
	ForceFogTransition(ent, false);

	/* If the origin or velocity have changed since ClientThink(),
	   update the pmove values. This will happen when the client
	   is pushed by a bmodel or kicked by an explosion.
	   If it wasn't updated here, the view position would lag a frame
	   behind the body position when pushed -- "sinking into plats" */
	for (i = 0; i < 3; i++)
	{
		/*
		 * set ps.pmove.origin is not required as server uses ent.origin instead
		 */
		current_client->ps.pmove.velocity[i] = ent->velocity[i] * 8.0;
	}

	/* If the end of unit layout is displayed, don't give
	   the player any normal movement attributes */
	if (level.intermissiontime)
	{
		current_client->ps.fov = 75;
		G_SetStats(ent);
		return;
	}

	AngleVectors(ent->client->v_angle, forward, right, up);

	/* burn from lava, etc */
	P_WorldEffects();

	// ********************************************************************************************
	// Set the player entity's model angles.
	// ********************************************************************************************

	if (ent->deadflag == DEAD_NO)
	{
		// PITCH.

		if ((ent->client->playerinfo.pm_w_flags & (WF_DIVING | WF_SWIMFREE)))
		{
			if (ent->client->v_angle[PITCH] > 180.0)
				ent->s.angles[PITCH] = -(-360.0 + ent->client->v_angle[PITCH]);
			else
				ent->s.angles[PITCH] = -ent->client->v_angle[PITCH];
		}
		else
		{
			ent->s.angles[PITCH] = 0.0;
		}

		// YAW and ROLL.

		ent->s.angles[YAW] = ent->client->v_angle[YAW];
		ent->s.angles[ROLL] = 0.0;
	}

	// ********************************************************************************************
	// Handle calcs for cyclic effects like walking / swimming.
	// ********************************************************************************************

	xyspeed = sqrt(ent->velocity[0] * ent->velocity[0] + ent->velocity[1] * ent->velocity[1]);

	if (xyspeed < 5)
	{
		bobmove = 0;
		current_client->bobtime = 0; /* start at beginning of cycle again */
	}
	else if (ent->groundentity && !current_player->waterlevel)
	{
		/* so bobbing only cycles when on ground */
		if (xyspeed > 210)
		{
			bobmove = 0.25;
		}
		else if (xyspeed > 100)
		{
			bobmove = 0.125;
		}
		else
		{
			bobmove = 0.0625;
		}
	}
	else if (current_player->waterlevel)
	{
		// So bobbing only cycles when in water.

		if (xyspeed > 100)
			bobmove = 1.0;
		else if (xyspeed > 50)
			bobmove = 0.5;
		else
			bobmove = 0.25;
	}

	bobtime = (current_client->bobtime += bobmove);

	bobcycle = (int)bobtime;
	bobfracsin = fabs(sin(bobtime * M_PI));

	// ********************************************************************************************
	// Calculate damage (if any) from hitting the floor and apply the damage taken this frame from
	// ALL sources.
	// ********************************************************************************************

	SetupPlayerinfo(ent);

	playerExport->PlayerFallingDamage(&ent->client->playerinfo);

	/* apply all the damage taken this frame */
	P_DamageFeedback(ent);

	/* determine the view offsets */
	SV_CalcViewOffset(ent);

	WritePlayerinfo(ent);

	// ********************************************************************************************
	// Generate client-side status display data
	// ********************************************************************************************
	G_SetStats(ent);
	G_SetClientSound(ent);
	G_SetClientFrame(ent, 0);

	// ********************************************************************************************
	// Handle player animation.
	// ********************************************************************************************
	SetupPlayerinfo(ent); /* Animation check */

	playerExport->PlayerUpdateCmdFlags(ent->client);

	if (showbuoys->value) // Note this is a bit of a hack
	{
		if (ent->client->playerinfo.seqcmd[ACMDL_ACTION])
		{
			PrintLocalBuoyInfo(ent->s.origin);
		}
	}

	playerExport->PlayerUpdate(&ent->client->playerinfo);

	// Validate
	playerExport->AnimUpdateFrame(&ent->client->playerinfo);

	PlayerTimerUpdate(ent);

	WritePlayerinfo(ent);

	// ********************************************************************************************
	// Save velocity and viewangles away for use next game frame.
	// ********************************************************************************************

	VectorCopy(ent->velocity,ent->client->playerinfo.oldvelocity);
	VectorCopy(ent->client->ps.viewangles,ent->client->oldviewangles);

	// ********************************************************************************************
	// If the deathmatch scoreboard is up then update it.
	// ********************************************************************************************

	if (ent->client->playerinfo.showscores && deathmatch->value && (!(level.framenum&31)))
	{
		DeathmatchScoreboardMessage(ent, ent->enemy);

		gi.unicast(ent,false);
	}

	// ********************************************************************************************
	// Reflect inventory changes in the client's playetstate.
	// ********************************************************************************************

	current_client->ps.NoOfItems = i = 0;
	current_client->ps.NoOfItems = i;

	// ********************************************************************************************
	// Reflect changes to the client's origin and velocity due to the current player animation, in
	// the client's playerstate.
	// ********************************************************************************************

	current_client->ps.pmove.origin[0] = ent->s.origin[0] * 8.0;
	current_client->ps.pmove.origin[1] = ent->s.origin[1] * 8.0;
	current_client->ps.pmove.origin[2] = ent->s.origin[2] * 8.0;

	current_client->ps.pmove.velocity[0] = ent->velocity[0] * 8.0;
	current_client->ps.pmove.velocity[1] = ent->velocity[1] * 8.0;
	current_client->ps.pmove.velocity[2] = ent->velocity[2] * 8.0;

	// ********************************************************************************************
	// Write all the shit that animation system modifies out to the playerstate (for prediction).
	// ********************************************************************************************

	current_client->ps.watertype = current_client->playerinfo.watertype;
	current_client->ps.waterlevel = current_client->playerinfo.waterlevel;
	current_client->ps.waterheight = current_client->playerinfo.waterheight;

	VectorCopy(current_client->playerinfo.grabloc,current_client->ps.grabloc);
	current_client->ps.grabangle = current_client->playerinfo.grabangle;

	current_client->ps.fwdvel = current_client->playerinfo.fwdvel;
	current_client->ps.sidevel = current_client->playerinfo.sidevel;
	current_client->ps.upvel = current_client->playerinfo.upvel;

	current_client->ps.flags = current_client->playerinfo.flags;

	current_client->ps.edictflags = ent->flags;

	current_client->ps.oldvelocity_z = current_client->playerinfo.oldvelocity[2];

	current_client->ps.upperseq = current_client->playerinfo.upperseq;
	current_client->ps.lowerseq = current_client->playerinfo.lowerseq;

	current_client->ps.upperframe = current_client->playerinfo.upperframe;
	current_client->ps.lowerframe = current_client->playerinfo.lowerframe;

	current_client->ps.upperidle = (byte)((current_client->playerinfo.upperidle)?1:0);
	current_client->ps.loweridle = (byte)((current_client->playerinfo.loweridle)?1:0);

	current_client->ps.uppermove_index = current_client->playerinfo.uppermove_index;
	current_client->ps.lowermove_index = current_client->playerinfo.lowermove_index;

	current_client->ps.weapon = (byte)ITEM_INDEX(current_client->playerinfo.pers.weapon);
	current_client->ps.defense = (byte)ITEM_INDEX(current_client->pers.defence);
	current_client->ps.lastweapon = (byte)ITEM_INDEX(current_client->pers.lastweapon);
	current_client->ps.lastdefense = (byte)ITEM_INDEX(current_client->pers.lastdefence);
	current_client->ps.weaponready = (byte)current_client->pers.weaponready;
	current_client->ps.switchtoweapon = (byte)current_client->playerinfo.switchtoweapon;
	current_client->ps.newweapon = (byte)ITEM_INDEX(current_client->newweapon);
	current_client->ps.weap_ammo_index = (byte)current_client->playerinfo.weap_ammo_index;
	current_client->ps.def_ammo_index = (byte)current_client->playerinfo.def_ammo_index;
	current_client->ps.weaponcharge = (byte)current_client->playerinfo.weaponcharge;
	current_client->ps.armortype = (byte)current_client->pers.armortype;
	current_client->ps.bowtype = (byte)current_client->pers.bowtype;
	current_client->ps.stafflevel = (byte)current_client->pers.stafflevel;
	current_client->ps.helltype = (byte)current_client->pers.helltype;
	current_client->ps.meteor_count = (byte)current_client->playerinfo.meteor_count;
	current_client->ps.handfxtype = (byte)current_client->pers.handfxtype;
	current_client->ps.plaguelevel = (byte)current_client->playerinfo.plaguelevel;
	current_client->ps.altparts = (byte)current_client->pers.altparts;
	current_client->ps.deadflag = current_client->playerinfo.deadflag;
	current_client->ps.ideal_yaw = ent->ideal_yaw;
	current_client->ps.leveltime = level.time;
	current_client->ps.idletime = current_client->playerinfo.idletime;
	current_client->ps.quickturnEndTime = current_client->playerinfo.quickturnEndTime;
	current_client->ps.powerup_timer = current_client->playerinfo.powerup_timer;
	current_client->ps.quickturn_rate = current_client->playerinfo.quickturn_rate;

	current_client->ps.dmflags = current_client->playerinfo.dmflags;
	current_client->ps.advancedstaff = current_client->playerinfo.advancedstaff;

	current_client->ps.cinematicfreeze = current_client->playerinfo.sv_cinematicfreeze;

	/* if the inventory is up, update it */
	if (ent->client->showinventory)
	{
		InventoryMessage(ent);
		gi.unicast(ent, false);
	}

	if (ent->client->chasetoggle == 1)
	{
		CheckChasecam_Viewent(ent);
	}
}
