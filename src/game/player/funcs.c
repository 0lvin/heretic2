//
// Heretic II
// Copyright 1998 Raven Software
//
// p_funcs.c
//
// Heretic II - Raven software
//

#include "../header/local.h"
#include "../player/library/p_animactor.h"
#include "../player/library/p_anims.h"
#include "../player/library/p_ctrl.h"
#include "../player/library/p_main.h"
#include "../header/g_skeletons.h"
#include "../header/g_teleport.h"
#include "../common/angles.h"
#include "../common/h2rand.h"

// ************************************************************************************************
// G_GetEntityStatePtr
// -------------------
// ************************************************************************************************

entity_state_t *G_GetEntityStatePtr(edict_t *entity)
{
	return(&entity->s);
}

void PlayerClimbSound(playerinfo_t *playerinfo, char *name)
{
	gi.sound(playerinfo->self,
				CHAN_VOICE,
				gi.soundindex(name),
				0.75,
				ATTN_NORM,
				0);
}

// ************************************************************************************************
// G_PlayerActionCheckRopeMove
// -------------------
// ************************************************************************************************

void G_PlayerActionCheckRopeMove(playerinfo_t *playerinfo)
{
	vec3_t		vr,vf;
	float		threshold;

	if ( (playerinfo->seqcmd[ACMDL_JUMP]) )
	{
		playerinfo->flags &= ~PLAYER_FLAG_ONROPE;
		VectorCopy((playerinfo->self)->teamchain->teamchain->velocity,playerinfo->velocity);
		threshold = VectorLengthSquared(playerinfo->velocity);

		if (threshold < 300*300)
		{
			AngleVectors(playerinfo->aimangles, vf, NULL, NULL);
			VectorMA(playerinfo->velocity, 200, vf, playerinfo->velocity);
		}
		else
		{
			VectorScale(playerinfo->velocity,0.75,playerinfo->velocity);
		}

		playerinfo->velocity[2]=250.0;
		playerinfo->flags |= PLAYER_FLAG_USE_ENT_POS;

		(playerinfo->self)->monsterinfo.jump_time = playerinfo->leveltime + 2;

		(playerinfo->self)->teamchain->teamchain->s.effects &= ~EF_ALTCLIENTFX;
		(playerinfo->self)->teamchain->enemy = NULL;
		(playerinfo->self)->teamchain = NULL;

		playerExport->PlayerAnimSetUpperSeq(playerinfo, ASEQ_NONE);
		playerExport->PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPFWD);

		return;
	}

	if (playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		AngleVectors(playerinfo->angles, NULL, vr, NULL);
		VectorScale(vr, -32, vr);
		VectorAdd((playerinfo->self)->teamchain->teamchain->velocity,vr,(playerinfo->self)->teamchain->teamchain->velocity);

		switch (playerinfo->lowerseq)
		{
		case ASEQ_CLIMB_HOLD_R:
		case ASEQ_CLIMB_SETTLE_R:
			if (irand(0,1))
				PlayerClimbSound(playerinfo, "player/ropeto.wav");
			else
				PlayerClimbSound(playerinfo, "player/ropefro.wav");
			break;

		case ASEQ_CLIMB_ON:
		case ASEQ_CLIMB_HOLD_L:
		case ASEQ_CLIMB_SETTLE_L:
			if (irand(0,1))
				PlayerClimbSound(playerinfo, "player/ropeto.wav");
			else
				PlayerClimbSound(playerinfo, "player/ropefro.wav");
			break;
		}
	}
	else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		AngleVectors(playerinfo->angles, NULL, vr, NULL);
		VectorScale(vr, 32, vr);
		VectorAdd((playerinfo->self)->teamchain->teamchain->velocity,vr,(playerinfo->self)->teamchain->teamchain->velocity);

		switch (playerinfo->lowerseq)
		{
		case ASEQ_CLIMB_HOLD_R:
		case ASEQ_CLIMB_SETTLE_R:
			if (irand(0,1))
				PlayerClimbSound(playerinfo, "player/ropeto.wav");
			else
				PlayerClimbSound(playerinfo, "player/ropefro.wav");
			break;

		case ASEQ_CLIMB_ON:
		case ASEQ_CLIMB_HOLD_L:
		case ASEQ_CLIMB_SETTLE_L:
			if (irand(0,1))
				PlayerClimbSound(playerinfo, "player/ropeto.wav");
			else
				PlayerClimbSound(playerinfo, "player/ropefro.wav");
			break;
		}
	}
}

// ************************************************************************************************
// G_BranchLwrClimbing
// -------------------
// ************************************************************************************************

int G_BranchLwrClimbing(playerinfo_t *playerinfo)
{
	trace_t		trace;
	vec3_t		vr,	endpoint, playermin, playermax, vf;
	int			chance = irand(0,3);

	assert(playerinfo);

	if (playerinfo->seqcmd[ACMDU_ATTACK])
	{
		if ((playerinfo->self)->teamchain->teamchain->monsterinfo.jump_time < level.time)
		{
			(playerinfo->self)->teamchain->teamchain->monsterinfo.jump_time = level.time + 2;
			AngleVectors(playerinfo->angles, vf, NULL, NULL);
			VectorMA(vf, 400, vf, vf);
			VectorAdd((playerinfo->self)->teamchain->teamchain->velocity,vf,(playerinfo->self)->teamchain->teamchain->velocity);
		}
	}

	if (playerinfo->seqcmd[ACMDL_STRAFE_L])
	{
		if ((playerinfo->self)->teamchain->teamchain->monsterinfo.search_time < level.time)
		{
			(playerinfo->self)->teamchain->teamchain->monsterinfo.search_time = level.time + 2;

			AngleVectors(playerinfo->angles, NULL, vr, NULL);
			VectorScale(vr, -64, vr);
			VectorAdd((playerinfo->self)->teamchain->teamchain->velocity,vr,(playerinfo->self)->teamchain->teamchain->velocity);

			switch (playerinfo->lowerseq)
			{
			case ASEQ_CLIMB_HOLD_R:
			case ASEQ_CLIMB_SETTLE_R:
				if (irand(0,1))
					PlayerClimbSound(playerinfo, "player/ropeto.wav");
				else
					PlayerClimbSound(playerinfo, "player/ropefro.wav");

				return ASEQ_CLIMB_HOLD_R;
				break;

			case ASEQ_CLIMB_ON:
			case ASEQ_CLIMB_HOLD_L:
			case ASEQ_CLIMB_SETTLE_L:
				if (irand(0,1))
					PlayerClimbSound(playerinfo, "player/ropeto.wav");
				else
					PlayerClimbSound(playerinfo, "player/ropefro.wav");

				return ASEQ_CLIMB_HOLD_L;
				break;
			}
		}
	}
	else if (playerinfo->seqcmd[ACMDL_STRAFE_R])
	{
		if ((playerinfo->self)->teamchain->teamchain->monsterinfo.flee_finished < level.time)
		{
			(playerinfo->self)->teamchain->teamchain->monsterinfo.flee_finished = level.time + 2;

			AngleVectors(playerinfo->angles, NULL, vr, NULL);
			VectorScale(vr, 64, vr);
			VectorAdd((playerinfo->self)->teamchain->teamchain->velocity,vr,(playerinfo->self)->teamchain->teamchain->velocity);

			switch (playerinfo->lowerseq)
			{
			case ASEQ_CLIMB_HOLD_R:
			case ASEQ_CLIMB_SETTLE_R:
				if (irand(0,1))
					PlayerClimbSound(playerinfo, "player/ropeto.wav");
				else
					PlayerClimbSound(playerinfo, "player/ropefro.wav");

				return ASEQ_CLIMB_HOLD_R;
				break;

			case ASEQ_CLIMB_ON:
			case ASEQ_CLIMB_HOLD_L:
			case ASEQ_CLIMB_SETTLE_L:
				if (irand(0,1))
					PlayerClimbSound(playerinfo, "player/ropeto.wav");
				else
					PlayerClimbSound(playerinfo, "player/ropefro.wav");

				return ASEQ_CLIMB_HOLD_L;
				break;
			}
		}
	}

	if (playerinfo->seqcmd[ACMDL_FWD])
	{
		VectorCopy(playerinfo->origin, endpoint);
		endpoint[2] += 32;

		VectorCopy(playerinfo->mins, playermin);
		VectorCopy(playerinfo->maxs, playermax);

		trace = gi.trace(playerinfo->origin, playermin, playermax, endpoint, (edict_t*)playerinfo->self, MASK_PLAYERSOLID);

		if (trace.fraction < 1.0)
		{
			// We bumped into something.

			(playerinfo->self)->teamchain->teamchain->viewheight = (playerinfo->self)->teamchain->teamchain->accel;

			switch (playerinfo->lowerseq)
			{
			case ASEQ_CLIMB_HOLD_R:
			case ASEQ_CLIMB_SETTLE_R:
				return ASEQ_CLIMB_HOLD_R;
				break;

			case ASEQ_CLIMB_ON:
			case ASEQ_CLIMB_HOLD_L:
			case ASEQ_CLIMB_SETTLE_L:
				return ASEQ_CLIMB_HOLD_L;
				break;

			case ASEQ_CLIMB_UP_L:
			case ASEQ_CLIMB_DOWN_R:
			case ASEQ_CLIMB_UP_START_L:
			case ASEQ_CLIMB_DOWN_START_L:
				if (irand(0,1))
					PlayerClimbSound(playerinfo, "player/ropeto.wav");
				else
					PlayerClimbSound(playerinfo, "player/ropefro.wav");

				return ASEQ_CLIMB_SETTLE_R;
				break;

			case ASEQ_CLIMB_UP_R:
			case ASEQ_CLIMB_DOWN_L:
			case ASEQ_CLIMB_UP_START_R:
			case ASEQ_CLIMB_DOWN_START_R:
				if (irand(0,1))
					PlayerClimbSound(playerinfo, "player/ropeto.wav");
				else
					PlayerClimbSound(playerinfo, "player/ropefro.wav");

				return ASEQ_CLIMB_SETTLE_L;
				break;
			}
		}

		switch ( playerinfo->lowerseq )
		{
		case ASEQ_CLIMB_UP_R:
		case ASEQ_CLIMB_UP_START_R:
			if (chance == 0)
				PlayerClimbSound(playerinfo, "player/ropeclimb1.wav");
			else if (chance == 1)
				PlayerClimbSound(playerinfo, "player/ropeclimb2.wav");

			return ASEQ_CLIMB_UP_L;
			break;

		case ASEQ_CLIMB_UP_L:
		case ASEQ_CLIMB_UP_START_L:
			if (chance == 0)
				PlayerClimbSound(playerinfo, "player/ropeclimb1.wav");
			else if (chance == 1)
				PlayerClimbSound(playerinfo, "player/ropeclimb2.wav");

			return ASEQ_CLIMB_UP_R;
			break;

		case ASEQ_CLIMB_ON:
		case ASEQ_CLIMB_DOWN_L:
		case ASEQ_CLIMB_HOLD_L:
		case ASEQ_CLIMB_SETTLE_L:
		case ASEQ_CLIMB_DOWN_START_L:
			if (chance == 0)
				PlayerClimbSound(playerinfo, "player/ropeclimb1.wav");
			else if (chance == 1)
				PlayerClimbSound(playerinfo, "player/ropeclimb2.wav");

			return ASEQ_CLIMB_UP_START_L;
			break;

		case ASEQ_CLIMB_DOWN_R:
		case ASEQ_CLIMB_HOLD_R:
		case ASEQ_CLIMB_SETTLE_R:
		case ASEQ_CLIMB_DOWN_START_R:
			if (chance == 0)
				PlayerClimbSound(playerinfo, "player/ropeclimb1.wav");
			else if (chance == 1)
				PlayerClimbSound(playerinfo, "player/ropeclimb2.wav");

			return ASEQ_CLIMB_UP_START_R;
			break;
		}
	}
	else if (playerinfo->seqcmd[ACMDL_BACK])
	{
		VectorCopy(playerinfo->origin, endpoint);
		endpoint[2] -= 32;

		VectorCopy(playerinfo->mins, playermin);
		VectorCopy(playerinfo->maxs, playermax);

		trace = gi.trace(playerinfo->origin, playermin, playermax, endpoint, (edict_t*)playerinfo->self, MASK_PLAYERSOLID);

		if (trace.fraction < 1.0 || trace.endpos[2] < (playerinfo->self)->teamchain->target_ent->s.origin[2])
		{
			// We bumped into something or have come to the end of the rope

			(playerinfo->self)->teamchain->teamchain->viewheight = (playerinfo->self)->teamchain->teamchain->accel;

			switch (playerinfo->lowerseq)
			{

			case ASEQ_CLIMB_HOLD_R:
			case ASEQ_CLIMB_SETTLE_R:
				return ASEQ_CLIMB_HOLD_R;
				break;

			case ASEQ_CLIMB_ON:
			case ASEQ_CLIMB_HOLD_L:
			case ASEQ_CLIMB_SETTLE_L:
				return ASEQ_CLIMB_HOLD_L;
				break;

			case ASEQ_CLIMB_UP_L:
			case ASEQ_CLIMB_DOWN_R:
			case ASEQ_CLIMB_UP_START_L:
			case ASEQ_CLIMB_DOWN_START_L:
				if (irand(0,1))
					PlayerClimbSound(playerinfo, "player/ropeto.wav");
				else
					PlayerClimbSound(playerinfo, "player/ropefro.wav");

				return ASEQ_CLIMB_SETTLE_R;
				break;

			case ASEQ_CLIMB_UP_R:
			case ASEQ_CLIMB_DOWN_L:
			case ASEQ_CLIMB_UP_START_R:
			case ASEQ_CLIMB_DOWN_START_R:
				if (irand(0,1))
					PlayerClimbSound(playerinfo, "player/ropeto.wav");
				else
					PlayerClimbSound(playerinfo, "player/ropefro.wav");

				return ASEQ_CLIMB_SETTLE_L;
				break;
			}
		}

		switch ( playerinfo->lowerseq )
		{
		case ASEQ_CLIMB_DOWN_R:
		case ASEQ_CLIMB_DOWN_START_R:
			if (chance == 0)
				PlayerClimbSound(playerinfo, "player/ropeclimb1.wav");
			else if (chance == 1)
				PlayerClimbSound(playerinfo, "player/ropeclimb2.wav");

			return ASEQ_CLIMB_DOWN_L;
			break;

		case ASEQ_CLIMB_DOWN_L:
		case ASEQ_CLIMB_DOWN_START_L:
			if (chance == 0)
				PlayerClimbSound(playerinfo, "player/ropeclimb1.wav");
			else if (chance == 1)
				PlayerClimbSound(playerinfo, "player/ropeclimb2.wav");

			return ASEQ_CLIMB_DOWN_R;
			break;

		case ASEQ_CLIMB_ON:
		case ASEQ_CLIMB_UP_L:
		case ASEQ_CLIMB_HOLD_R:
		case ASEQ_CLIMB_SETTLE_L:
		case ASEQ_CLIMB_UP_START_L:
			if (chance == 0)
				PlayerClimbSound(playerinfo, "player/ropeclimb1.wav");
			else if (chance == 1)
				PlayerClimbSound(playerinfo, "player/ropeclimb2.wav");

			return ASEQ_CLIMB_DOWN_START_L;
			break;

		case ASEQ_CLIMB_HOLD_L:
		case ASEQ_CLIMB_UP_R:
		case ASEQ_CLIMB_SETTLE_R:
		case ASEQ_CLIMB_UP_START_R:
			if (chance == 0)
				PlayerClimbSound(playerinfo, "player/ropeclimb1.wav");
			else if (chance == 1)
				PlayerClimbSound(playerinfo, "player/ropeclimb2.wav");

			return ASEQ_CLIMB_DOWN_START_R;
			break;
		}
	}
	else if ( (playerinfo->seqcmd[ACMDL_JUMP]) )
	{
		playerinfo->flags &= ~PLAYER_FLAG_ONROPE;
		VectorCopy((playerinfo->self)->teamchain->teamchain->velocity,playerinfo->velocity);
		playerinfo->velocity[2]=150.0;
		playerinfo->flags |= PLAYER_FLAG_USE_ENT_POS;

		(playerinfo->self)->monsterinfo.jump_time = playerinfo->leveltime + 2;

		(playerinfo->self)->teamchain->teamchain->s.effects &= ~EF_ALTCLIENTFX;
		(playerinfo->self)->teamchain->enemy = NULL;
		(playerinfo->self)->teamchain = NULL;

		playerExport->PlayerAnimSetUpperSeq(playerinfo, ASEQ_NONE);

		return ASEQ_JUMPFWD;
	}
	else
	{
		switch (playerinfo->lowerseq)
		{
		case ASEQ_CLIMB_HOLD_R:
		case ASEQ_CLIMB_SETTLE_R:
			return ASEQ_CLIMB_HOLD_R;
			break;

		case ASEQ_CLIMB_ON:
		case ASEQ_CLIMB_HOLD_L:
		case ASEQ_CLIMB_SETTLE_L:
			return ASEQ_CLIMB_HOLD_L;
			break;

		case ASEQ_CLIMB_UP_L:
		case ASEQ_CLIMB_DOWN_R:
		case ASEQ_CLIMB_UP_START_L:
		case ASEQ_CLIMB_DOWN_START_L:
			if (irand(0,1))
				PlayerClimbSound(playerinfo, "player/ropeto.wav");
			else
				PlayerClimbSound(playerinfo, "player/ropefro.wav");

			return ASEQ_CLIMB_SETTLE_R;
			break;

		case ASEQ_CLIMB_UP_R:
		case ASEQ_CLIMB_DOWN_L:
		case ASEQ_CLIMB_UP_START_R:
		case ASEQ_CLIMB_DOWN_START_R:
			if (irand(0,1))
				PlayerClimbSound(playerinfo, "player/ropeto.wav");
			else
				PlayerClimbSound(playerinfo, "player/ropefro.wav");

			return ASEQ_CLIMB_SETTLE_L;
			break;
		}
	}

	return(ASEQ_NONE);
}

// ************************************************************************************************
// G_PlayerActionCheckRopeGrab
// ---------------------------
// ************************************************************************************************

qboolean G_PlayerActionCheckRopeGrab(playerinfo_t *playerinfo, float stomp_org)
{
	edict_t		*rope;
	trace_t		trace;
	vec3_t		rope_end, rope_top, rope_check, vec;
	float		len, dist;
	int			check_dist = 48;

	assert(playerinfo);

	if (playerinfo->groundentity == NULL)
		check_dist = 64;

	rope = (edict_t *)playerinfo->targetEnt;

	//Get the position of the rope's end
	VectorCopy(rope->target_ent->s.origin, rope_end);

	VectorCopy(rope->s.origin, rope_top);
	rope_top[2] += rope->maxs[2];

	//If we're above the rope then we can't grab it
	if (playerinfo->origin[2] > rope_top[2])
	{
		//(playerinfo->self)->teamchain = NULL;
		return false;
	}

	VectorSubtract(playerinfo->origin, rope_top, vec);
	len = VectorLength(vec);

	VectorSubtract(rope_end, rope_top, vec);
	dist = VectorNormalize(vec);

	//Player is below the rope's length
	if (len > dist)
	{
		//(playerinfo->self)->teamchain = NULL;
		return false;
	}

	VectorMA(rope_top, len, vec, rope_check);

	dist = Vector2Length(playerinfo->origin, rope_check);

	if (dist < check_dist)
	{
		// Player is getting on the rope for the first time.

		if (!(playerinfo->flags & PLAYER_FLAG_ONROPE))
		{
			VectorCopy(playerinfo->velocity,((edict_t *)playerinfo->targetEnt)->teamchain->velocity);
			VectorScale(((edict_t *)playerinfo->targetEnt)->teamchain->velocity,2,((edict_t *)playerinfo->targetEnt)->teamchain->velocity);
			VectorClear(playerinfo->velocity);
			VectorCopy(playerinfo->origin,((edict_t *)playerinfo->targetEnt)->teamchain->s.origin);
			VectorSubtract(playerinfo->origin,rope_top,vec);
			rope->teamchain->viewheight=VectorLength(vec);
		}
		else
		{
			trace = gi.trace(playerinfo->origin,
									  playerinfo->mins,
									  playerinfo->maxs,
									  ((edict_t *)playerinfo->targetEnt)->teamchain->s.origin,
										(edict_t*)playerinfo->self,
									  MASK_PLAYERSOLID);

			if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
				return false;

			VectorCopy(((edict_t *)playerinfo->targetEnt)->teamchain->s.origin, playerinfo->origin);
		}

		return true;
	}

	//(playerinfo->self)->teamchain = NULL;
	return false;
}

// ************************************************************************************************
// G_PlayerClimbingMoveFunc
// ------------------------
// ************************************************************************************************

void G_PlayerClimbingMoveFunc(playerinfo_t *playerinfo, float height, float var2, float var3)
{
	// Pull Corvus into the rope.
	G_PlayerActionCheckRopeGrab(playerinfo,1);

	if (playerinfo->targetEnt)
	{
		//Update the rope's information about the player's position
		((edict_t *)playerinfo->targetEnt)->teamchain->accel=((edict_t *)playerinfo->targetEnt)->teamchain->viewheight;
		((edict_t *)playerinfo->targetEnt)->teamchain->viewheight-=height;
	}
}

// ************************************************************************************************
// G_PlayerActionCheckPuzzleGrab
// -----------------------------
// ************************************************************************************************

qboolean G_PlayerActionCheckPuzzleGrab(playerinfo_t *playerinfo)
{
	vec3_t	player_facing,
			forward,
			endpoint;
	trace_t grabtrace;

	VectorCopy(playerinfo->angles,player_facing);
	player_facing[PITCH]=player_facing[ROLL]=0;
	AngleVectors(player_facing,forward,NULL,NULL);
	VectorMA(playerinfo->origin,32,forward,endpoint);

	grabtrace = gi.trace(playerinfo->origin,
					   playerinfo->mins,
					   playerinfo->maxs,
					   endpoint,
					   (edict_t *)playerinfo->self,
					   MASK_PLAYERSOLID);

	if ((grabtrace.fraction==1)||(!grabtrace.ent))
		return false;

	if (!grabtrace.ent->item)
		return false;

	if (grabtrace.ent->item->flags!=IT_PUZZLE)
		return false;

	playerinfo->targetEnt=grabtrace.ent;

	return true;
}

// ************************************************************************************************
// G_PlayerActionTakePuzzle
// ------------------------
// ************************************************************************************************

void G_PlayerActionTakePuzzle(playerinfo_t *playerinfo)
{
	if ((playerinfo->self)->teamchain->use)
		(playerinfo->self)->teamchain->use((playerinfo->self)->teamchain,(playerinfo->self),(playerinfo->self));
}

// ************************************************************************************************
// G_PlayerActionUsePuzzle
// -----------------------
// ************************************************************************************************

qboolean G_PlayerActionUsePuzzle(playerinfo_t *playerinfo)
{
	if (!(playerinfo->self)->target_ent)
		return false;

	if (strcmp((playerinfo->self)->target_ent->classname,"trigger_playerusepuzzle"))
		return false;

	G_UseTargets((playerinfo->self)->target_ent,(playerinfo->self));

	return true;
}

// ************************************************************************************************
// G_PlayerActionCheckPushPull_Ent
// -------------------------------
// ************************************************************************************************

qboolean G_PlayerActionCheckPushPull_Ent(edict_t *ent)
{
	if (!(strcmp(ent->classname,"func_train")==0)||!(ent->spawnflags&32))
		return false;
	else
		return true;
}

// ************************************************************************************************
// PushPull_stop
// -------------
// ************************************************************************************************

void PushPull_stop(edict_t *self)
{
/*
	playerinfo_t *playerinfo;

	playerinfo=&self->target_ent->client->playerinfo;

	if ((playerinfo->lowerseq!=ASEQ_PUSH)&&(playerinfo->lowerseq!=ASEQ_PULL))
		VectorClear(self->velocity);
	else if (Vec3IsZero(self->target_ent->velocity))
		VectorClear(self->target_ent->velocity);
*/
}

// ************************************************************************************************
// G_PlayerActionMoveItem
// ----------------------
// ************************************************************************************************

void G_PlayerActionMoveItem(playerinfo_t *playerinfo,float distance)
{
	vec3_t player_facing,pushdir;

	VectorCopy(playerinfo->angles,player_facing);
	player_facing[PITCH]=player_facing[ROLL]=0;
	AngleVectors(player_facing, pushdir, NULL, NULL);

	VectorScale (pushdir, distance, ((edict_t *)playerinfo->target_ent)->velocity);

	((edict_t *)(playerinfo->self))->target_ent->think = PushPull_stop;
	((edict_t *)(playerinfo->self))->target_ent->nextthink = level.time + 2 * FRAMETIME;
	((edict_t *)(playerinfo->self))->target_ent->target_ent = (playerinfo->self);
}

// ************************************************************************************************
// G_PlayerActionCheckPushButton
// -----------------------------
// ************************************************************************************************

#define MAX_PUSH_BUTTON_RANGE	80.0

qboolean G_PlayerActionCheckPushButton(playerinfo_t *playerinfo)
{
	edict_t *t;
	vec3_t	v,dir;
	float	len1, dot;
	vec3_t	forward;

	// Are you near a button?

	if (!(playerinfo->self)->target)
	{
		// No button so return.

		return false;
	}

	// A button is nearby, so look to see if it's in reach.

	t = NULL;
	t = G_Find(t,FOFS(targetname),(playerinfo->self)->target);

	if (!t)
		return false;

// 	if (!(strcmp(t->classname,"func_train")==0))
	if (t->classID == CID_BUTTON)
	{
		// Get center of button
		VectorAverage(t->mins, t->maxs, v);
		// Get distance from player origin to center of button
		Vec3SubtractAssign(playerinfo->origin, v);
		len1 = VectorLength(v);
	}
	else
		return false;

	if (len1 < MAX_PUSH_BUTTON_RANGE)
	{
		VectorCopy((playerinfo->self)->client->playerinfo.aimangles, dir);
		dir[PITCH] = 0;

		AngleVectors(dir, forward, NULL, NULL);
		VectorNormalize(v);
		// Both these vectors are normalized so result is cos of angle
		dot = DotProduct(v, forward);

		// 41 degree range either way
		if (dot > 0.75)
			return true;
	}

	return false;
}

// ************************************************************************************************
// G_PlayerActionPushButton
// ------------------------
// ************************************************************************************************

void G_PlayerActionPushButton(playerinfo_t *playerinfo)
{
	G_UseTargets(playerinfo->self,(edict_t *)playerinfo->self);
}

// ************************************************************************************************
// G_PlayerActionCheckPushLever
// -----------------------------
// ************************************************************************************************

#define MAX_PUSH_LEVER_RANGE	80.0

qboolean G_PlayerActionCheckPushLever(playerinfo_t *playerinfo)
{
	edict_t *t;
	vec3_t	v,dir;
	float	len1, dot;
	vec3_t	forward;
	edict_t *self;

	self = (edict_t *) playerinfo->self;

	// Are you near a lever?

	if (!(self->target))
	{
		// No button so return.

		return false;
	}

	// A button is nearby, so look to see if it's in reach.

	t = NULL;
	t = G_Find(t,FOFS(targetname),self->target);

	if (!t)
		return false;

	if (t->classID == CID_LEVER)
	{
		// Get distance from player origin to center of lever
		VectorSubtract(playerinfo->origin, t->s.origin,v);
		len1 = VectorLength(v);
	}
	else
		return false;

	if (len1 < MAX_PUSH_LEVER_RANGE)
	{
		VectorCopy((playerinfo->self)->client->playerinfo.aimangles, dir);
		dir[PITCH] = 0;

		AngleVectors(dir, forward, NULL, NULL);
		VectorSubtract (t->s.origin, self->s.origin, v);
		VectorNormalize(v);
		// Both these vectors are normalized so result is cos of angle
		dot = DotProduct(v, forward);

		// 41 degree range either way
		if (dot > 0.70)
			return true;
	}

	return false;
}

// ************************************************************************************************
// G_PlayerActionPushLever
// ------------------------
// ************************************************************************************************

void G_PlayerActionPushLever(playerinfo_t *playerinfo)
{
	G_UseTargets(playerinfo->self,(edict_t *)playerinfo->self);
}

// ************************************************************************************************
// G_HandleTeleport
// ----------------
// ************************************************************************************************

qboolean
G_HandleTeleport(edict_t *self)
{
	// Are we teleporting or morphing?
	if (self->client->playerinfo.flags & (PLAYER_FLAG_TELEPORT | PLAYER_FLAG_MORPHING))
	{
		// Are we doing de-materialiZe or...

		if (self->client->tele_dest[0]!=-1)
		{
			// Are we done dematerialiZing? Or still fading?

			if (self->client->tele_count--)
			{
				self->s.color[3] -= TELE_FADE_OUT;

				return true;
			}
			else
			{
				// We have finished dematerialiZing, let's move the character.

				if (self->client->playerinfo.flags & PLAYER_FLAG_TELEPORT)
				{
					Perform_Teleport(self);
				}
				else
				{
					if (self->client->playerinfo.edictflags & FL_CHICKEN)
					{
						// We're set as a chicken.

						reset_morph_to_elf(self);
					}
					else
					{
						Perform_Morph(self);
					}
				}

				return true;
			}
		}
		else
		{
			// Are we done dematerialiZing? Or still fading?

			if (self->client->tele_count--)
			{
				self->s.color[3] += TELE_FADE;
			}
			else
			{
				// We are done re-materialiZing, let's kill all this BS and get back to the game.

				if (self->client->playerinfo.flags & PLAYER_FLAG_TELEPORT)
					CleanUpTeleport(self);
				else
					CleanUpMorph(self);
			}
		}

		if (!deathmatch->value)
			return true;
	}

	return false;
}

// ************************************************************************************************
// PlayerChickenDeath
// ------------------
// ************************************************************************************************

void PlayerChickenDeath(edict_t *self)
{
	//FIXME:

	//gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->client->playerinfo.deadflag = DEAD_DEAD;
	gi.CreateEffect(self, FX_CHICKEN_EXPLODE, CEF_OWNERS_ORIGIN, NULL, "" );

	// fix that respawning bug
	self->morph_timer = level.time -1;

	// Reset our thinking.

	self->think = self->oldthink;
	self->nextthink = level.time + FRAMETIME;

#ifdef COMP_FMOD
	self->model = "models/player/corvette/tris_c.fm";
#else
	self->model = "models/player/corvette/tris.fm";
#endif
	self->pain = player_pain;

	// Reset our skins.

	self->s.effects = 0;

	self->s.skinnum = 0;	// Hey, the skinnum stores the skin now, capiche?
	self->s.clientnum = self - g_edicts - 1;

	self->s.modelindex = CUSTOM_PLAYER_MODEL;		// will use the skin specified model
	self->s.frame = 0;

	// Turn our skeleton back on.

	self->s.skeletalType = SKEL_CORVUS;
	self->s.effects |= (EF_SWAPFRAME|EF_JOINTED);
	self->s.effects &= ~EF_CHICKEN;
	self->flags &= ~FL_CHICKEN;
	self->s.renderfx &= ~RF_IGNORE_REFS;

	// Reset our animations.

	playerExport->PlayerAnimReset(&self->client->playerinfo);
}

// ************************************************************************************************
// G_SetJointAngles
// ------------------
// Set the player model's joint angles.
// ************************************************************************************************

void G_SetJointAngles(edict_t *self)
{
	SetJointAngVel(self->s.rootJoint + CORVUS_HEAD,PITCH,
		self->client->playerinfo.targetjointangles[PITCH],M_PI / 4.0);
	SetJointAngVel(self->s.rootJoint + CORVUS_HEAD,ROLL,
		self->client->playerinfo.targetjointangles[YAW],M_PI / 4.0);

	if (!self->client->playerinfo.headjointonly)
	{
		SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK,PITCH,
			self->client->playerinfo.targetjointangles[PITCH],M_PI / 4.0);
		SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK,PITCH,
			self->client->playerinfo.targetjointangles[PITCH],M_PI / 4.0);
		SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK,ROLL,
			self->client->playerinfo.targetjointangles[YAW],M_PI / 4.0);
		SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK,ROLL,
			self->client->playerinfo.targetjointangles[YAW],M_PI / 4.0);
	}
	else
	{
		SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK, PITCH, 0, M_PI / 4.0);
		SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK, PITCH, 0, M_PI / 4.0);
		SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK, ROLL, 0, M_PI / 4.0);
		SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK, ROLL, 0, M_PI / 4.0);
	}
}

// ************************************************************************************************
// G_ResetJointAngles
// ------------------
// Reset the player model's joint angles.
// ************************************************************************************************

void G_ResetJointAngles(edict_t *self)
{
	SetJointAngVel(self->s.rootJoint + CORVUS_HEAD,PITCH, 0, M_PI / 4.0);
	SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK,PITCH, 0, M_PI / 4.0);
	SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK,PITCH, 0, M_PI / 4.0);

	SetJointAngVel(self->s.rootJoint + CORVUS_HEAD,ROLL,0,M_PI / 4.0);
	SetJointAngVel(self->s.rootJoint + CORVUS_UPPERBACK,ROLL,0, M_PI / 4.0);
	SetJointAngVel(self->s.rootJoint + CORVUS_LOWERBACK,ROLL,0, M_PI / 4.0);
}

// ************************************************************************************************
// G_PlayerActionChickenBite
// -------------------------
// ************************************************************************************************

void G_PlayerActionChickenBite(playerinfo_t *playerinfo)
{
	trace_t	trace;
	vec3_t	endpos, vf, mins;

	AngleVectors(playerinfo->aimangles, vf, NULL, NULL);
	VectorMA(playerinfo->origin, 64, vf, endpos);

	//Account for step height
	VectorSet(mins, playerinfo->mins[0], playerinfo->mins[1], playerinfo->mins[2] + 18);

	trace = gi.trace(playerinfo->origin, mins, playerinfo->maxs, endpos, (playerinfo->self), MASK_SHOT);

	if (trace.ent && trace.ent->takedamage)
	{
		if (playerinfo->edictflags & FL_SUPER_CHICKEN)
			T_Damage(trace.ent,(playerinfo->self),(playerinfo->self),vf,trace.endpos,trace.plane.normal,500,0,DAMAGE_AVOID_ARMOR,MOD_CHICKEN);
		else
			T_Damage(trace.ent,(playerinfo->self),(playerinfo->self),vf,trace.endpos,trace.plane.normal,1,0,DAMAGE_AVOID_ARMOR,MOD_CHICKEN);

		if (playerinfo->edictflags & FL_SUPER_CHICKEN)
		{
			// Sound for hitting.
			if (irand(0,1))
				gi.sound((playerinfo->self), CHAN_WEAPON, gi.soundindex("monsters/superchicken/bite1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound((playerinfo->self), CHAN_WEAPON, gi.soundindex("monsters/superchicken/bite2.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			// Sound for hitting.
			if (irand(0,1))
				gi.sound((playerinfo->self), CHAN_WEAPON, gi.soundindex("monsters/chicken/bite1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound((playerinfo->self), CHAN_WEAPON, gi.soundindex("monsters/chicken/bite2.wav"), 1, ATTN_NORM, 0);
		}
	}
	else
	{	// Sound for missing.
		if (playerinfo->edictflags & FL_SUPER_CHICKEN)
		{
			if (irand(0,1))
				gi.sound((playerinfo->self), CHAN_WEAPON, gi.soundindex("monsters/superchicken/peck1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound((playerinfo->self), CHAN_WEAPON, gi.soundindex("monsters/superchicken/peck2.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			if (irand(0,1))
				gi.sound((playerinfo->self), CHAN_WEAPON, gi.soundindex("monsters/chicken/peck1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound((playerinfo->self), CHAN_WEAPON, gi.soundindex("monsters/chicken/peck2.wav"), 1, ATTN_NORM, 0);
		}
	}
}

// ************************************************************************************************
// G_PlayerFallingDamage
// ---------------------
// ************************************************************************************************

void G_PlayerFallingDamage(playerinfo_t *playerinfo,float delta)
{
	edict_t *ent;
	vec3_t	dir;
	float	damage;

	ent=(edict_t *)playerinfo->self;

	ent->pain_debounce_time=level.time;

	if (delta > 50)
		damage = delta - 30;
	else if ((damage = (delta - 30) * 0.8) < 1.0f)
		damage = 1;

	VectorSet(dir,0.0,0.0,1.0);

	T_Damage(ent,world,world,dir,ent->s.origin,vec3_origin,damage,0,DAMAGE_AVOID_ARMOR,MOD_FALLING);

	if (deathmatch->value || coop->value)
	{
		if (ent->groundentity && ent->groundentity->takedamage)
		{
			vec3_t	victim_dir, impact_spot;

			if (playerinfo->edictflags & FL_SUPER_CHICKEN)
			{
				damage = 500;
			}
			else
			{
				damage *= 2;
			}

			VectorSubtract(ent->groundentity->s.origin, ent->s.origin, victim_dir);
			VectorNormalize(victim_dir);
			VectorMA(ent->s.origin, -1.2 * ent->mins[2], victim_dir, impact_spot);

			T_Damage(ent->groundentity, ent, ent, victim_dir, impact_spot, vec3_origin, damage, 0, DAMAGE_AVOID_ARMOR, 0);
			if (ent->groundentity->client)
			{
				if (ent->groundentity->health > 0)
				{
					if (!irand(0, 1))
					{
						playerExport->KnockDownPlayer(&ent->groundentity->client->playerinfo);
					}
				}
			}
		}
	}
}

// *******************************************************
// G_PlayerVaultKick
// -----------------------------
// Check to kick entities inside the pole vault animation
// *******************************************************
#define VAULTKICK_DIST	30			//Amount to trace outward from the player's origin
#define VAULTKICK_MODIFIER 0.25		//percentage of the velocity magnitude to use as damage

void G_PlayerVaultKick(playerinfo_t *playerinfo)
{
	edict_t *self = (playerinfo->self);
	trace_t trace;
	vec3_t	endpos, vf;
	float	kick_vel;

	//Ignore pitch
	VectorSet(vf, 0, self->s.angles[YAW], 0);
	AngleVectors(vf, vf, NULL, NULL);

	//Move ahead by a small amount
	VectorMA(self->s.origin, VAULTKICK_DIST, vf, endpos);

	//Trace out to see if we've hit anything
	trace = gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_PLAYERSOLID);

	//If we have...
	if (trace.fraction < 1 && (!(trace.startsolid || trace.allsolid)) )
	{
		if (trace.ent->takedamage)
		{
			//Find the velocity of the kick
			kick_vel = VectorLength(self->velocity);
			kick_vel *= VAULTKICK_MODIFIER;

			//FIXME: Get a real sound
			gi.sound(self, CHAN_WEAPON, gi.soundindex("monsters/plagueElf/hamhit.wav"), 1, ATTN_NORM, 0);
			T_Damage(trace.ent, self, self, vf, trace.endpos, trace.plane.normal, kick_vel, kick_vel*2, DAMAGE_NORMAL,MOD_KICKED);
			VectorMA(trace.ent->velocity, irand(300,500), vf, trace.ent->velocity);
			trace.ent->velocity[2] = 150;
			if (trace.ent->client)
			{
				if (trace.ent->health > 0)
				{
					if (infront(trace.ent, self) && !irand(0, 2))
					{
						playerExport->KnockDownPlayer(&trace.ent->client->playerinfo);
					}
				}
			}
		}
	}
}

// *******************************************************
// G_PlayerLightningShieldDamage
// -----------------------------
// *******************************************************

extern void SpellLightningShieldAttack(edict_t *self);
void G_PlayerSpellShieldAttack(edict_t *self)
{
	if (irand(0, (SHIELD_ATTACK_CHANCE-1)) == 0)
		SpellLightningShieldAttack(self);
}

// stop the attack and remove the persistant effect
void G_PlayerSpellStopShieldAttack(edict_t *self)
{
	if (self->PersistantCFX)
	{
		G_RemovePersistantEffect(self->PersistantCFX, REMOVE_SHIELD);
		self->PersistantCFX = 0;
		self->s.sound = 0;
	}

}

// ************************************************************************************************
// G_PlayerActionSwordAttack
// -------------------------
// ************************************************************************************************

void G_PlayerActionSwordAttack(edict_t *self, int value)
{
	WeaponThink_SwordStaffEx(self, "i", value);
}

// ************************************************************************************************
// G_PlayerActionSpellArray
// ------------------------
// ************************************************************************************************

void G_PlayerActionSpellArray(edict_t *self, int value)
{
	WeaponThink_MagicMissileSpreadEx(self,"i",value);
}

// ************************************************************************************************
// G_PlayerActionSpellSphereCreate
// -------------------------------
// ************************************************************************************************

void G_PlayerActionSpellSphereCreate(edict_t *self, qboolean *Charging)
{
	// Start a glow effect.
	WeaponThink_SphereOfAnnihilationEx(self, "g", Charging);
}

// ************************************************************************************************
// G_PlayerActionSpellDefensive
// ----------------------------
// ************************************************************************************************

void G_PlayerActionSpellDefensive(edict_t *self)
{
	int			index;
	gitem_t	*it;

	if (self->client->playerinfo.leveltime > self->client->playerinfo.defensive_debounce)
	{
//		playerinfo->pers.defence->use(playerinfo,playerinfo->pers.defence);
		self->client->pers.defence->weaponthink(self);
		self->client->playerinfo.defensive_debounce = self->client->playerinfo.leveltime + DEFENSE_DEBOUNCE;

		// if we've run out of defence shots, and we have the ring of repulsion - switch to that.
		it = FindItem("ring");
		index = ITEM_INDEX(it);
		if ((Defence_CurrentShotsLeft(&(self->client->playerinfo), 1) <=0)
			&& self->client->pers.inventory[index])
		{
			Cmd_Use_f(self);
		}
	}
}

// ************************************************************************************************
// G_EntIsAButton - this is exceedingly gay that this has to be done this way.
// ----------------------------
// ************************************************************************************************

qboolean G_EntIsAButton(edict_t *ent)
{
	if (ent->classID == CID_BUTTON)
		return (true);
	return (false);
}
