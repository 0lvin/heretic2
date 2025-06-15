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
 * The basic AI functions like enemy detection, attacking and so on.
 * While mostly unused by the CTF code the functions must be here since
 * big parts of the game logic rely on them.
 *
 * =======================================================================
 */

#include "header/local.h"
#include <assert.h>
#include "header/g_monster.h"
#include "common/h2rand.h"
#include "header/buoy.h"
#include "monster/stats/stats.h"
#include "player/library/p_anims.h"
#include "header/g_hitlocation.h"
#include "header/utilities.h"
#include "header/g_playstats.h"
#include "common/fx.h"

#define YAW_IDEAL 1
#define YAW_BEST_MOVE 0

extern cvar_t *maxclients;

static qboolean enemy_vis;
static qboolean enemy_infront;
static int enemy_range;
static float enemy_yaw;

void SV_NewChaseDir(edict_t *actor, edict_t *enemy, float dist);

void ssithraCheckJump(edict_t *self);
void SV_FixCheckBottom(edict_t *ent);
qboolean MG_MoveToGoal(edict_t *self, float dist);
float MG_ChangeYaw(edict_t *self);
void MG_BuoyNavigate(edict_t *self);
void MG_Pathfind(edict_t *self, qboolean check_clear_path);
float MG_FaceGoal(edict_t *self, qboolean doturn);
qboolean ok_to_wake(edict_t *monster, qboolean gorgon_roar, qboolean ignore_ambush);
qboolean EqualAngle(float angle1, float angle2, float leniency);
void alert_timed_out(alertent_t *self);
qboolean MG_ReachedBuoy(edict_t *self, vec3_t pspot);
qboolean ogle_findtarget(edict_t *self);

/* ========================================================================== */

/*
 * Called once each frame to set level.sight_client
 * to the player to be checked for in findtarget.
 * If all clients are either dead or in notarget,
 * sight_client will be null.
 * In coop games, sight_client will cycle
 * between the clients.
 */
void
AI_SetSightClient(void)
{
	edict_t *ent;
	int start, check;

	if (level.sight_client == NULL)
	{
		start = 1;
	}
	else
	{
		start = level.sight_client - g_edicts;
	}

	check = start;

	while (1)
	{
		check++;

		if (check > game.maxclients)
		{
			check = 1;
		}

		ent = &g_edicts[check];

		if (ent->inuse && (ent->health > 0) &&
			!(ent->flags & (FL_NOTARGET | FL_DISGUISED)))
		{
			level.sight_client = ent;
			return; /* got one */
		}

		if (check == start)
		{
			level.sight_client = NULL;
			return; /* nobody to see */
		}
	}
}

/* ============================================================================ */

/*
 * Move the specified distance at current
 * facing. This replaces the QC functions:
 * ai_forward, ai_back, ai_pain, and ai_painforward
 */
void
ai_move(edict_t *self, float dist)
{
	if (!self)
	{
		return;
	}

	qboolean trace_succeeded = false;
	MG_WalkMove(self, self->s.angles[YAW], dist, &trace_succeeded);
}

// ****************************************************************************
// ai_trystep
//
// Like other functions to attempt a step, but this will return exactly
// what happened to make it fail.
// ****************************************************************************
int
ai_trystep(edict_t *ent, vec3_t move)
{
	vec3_t oldorg, neworg, end, inf_mins, inf_maxs;
	trace_t trace;
	int contents;
	vec3_t test;

	// try the move
	VectorCopy(ent->s.origin, oldorg);

	VectorCopy(ent->mins, inf_mins);
	VectorCopy(ent->maxs, inf_maxs);

	VectorAdd (ent->s.origin, move, neworg);

	VectorScale(inf_mins, 2, inf_mins);
	VectorScale(inf_maxs, 2, inf_maxs);

	// push down from a step height above the wished position
	neworg[2] += STEPSIZE;
	VectorCopy(neworg, end);
	end[2] -= STEPSIZE * 2;

	trace = gi.trace(neworg, inf_mins, inf_maxs, end, ent, MASK_MONSTERSOLID);

	if (trace.allsolid)
		return TRYSTEP_ALLSOLID;

	if (trace.startsolid)
	{
		neworg[2] -= STEPSIZE;
		trace = gi.trace(neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);
		if (trace.allsolid || trace.startsolid)
			return TRYSTEP_STARTSOLID;
	}

	// don't go in to water unless only 40% hieght deep
	if (ent->waterlevel == 0)
	{
		test[0] = trace.endpos[0];
		test[1] = trace.endpos[1];
		test[2] = trace.endpos[2] + ent->mins[2];// + 1;
		test[2] += (ent->maxs[2] - ent->mins[2]) * 0.4;
		contents = gi.pointcontents(test);

		if (contents & MASK_WATER)
			return TRYSTEP_INWATER;
	}

	if (trace.fraction == 1)
	{
		// if monster had the ground pulled out, go ahead and fall
		if ( ent->flags & FL_PARTIALGROUND )
		{
			VectorAdd (ent->s.origin, move, ent->s.origin);
			ent->groundentity = NULL;
			return TRYSTEP_OK;
		}

		// walked off an edge
		return TRYSTEP_OFFEDGE;
	}

	// check point traces down for dangling corners
	VectorCopy(trace.endpos, ent->s.origin);

	if (!M_CheckBottom (ent))
	{
		if ( ent->flags & FL_PARTIALGROUND )
		{
			VectorCopy(oldorg, ent->s.origin);
			return TRYSTEP_OK;
		}

		VectorCopy(oldorg, ent->s.origin);
		return TRYSTEP_NOSUPPORT;
	}

	if ( ent->flags & FL_PARTIALGROUND )
		ent->flags &= ~FL_PARTIALGROUND;

	ent->groundentity = trace.ent;
	ent->groundentity_linkcount = trace.ent->linkcount;

	VectorCopy(oldorg, ent->s.origin);

	return TRYSTEP_OK;
}

// ****************************************************************************
// ai_maintain_waypoints
//
// checks all waypoints and corrects all buoys
// ****************************************************************************

void
ai_maintain_waypoints(edict_t *self, float mintel, float foo1, float foo2)
{
	vec3_t	vec;
	float	len;

	if (self->enemy)
	{
		if (visible(self, self->enemy))
		{
			VectorCopy(self->pos1, self->pos2);
			VectorCopy(self->enemy->s.origin, self->pos1);
		}
	}

	if (self->monsterinfo.searchType == SEARCH_COMMON)
		return;
	else if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		VectorSubtract(self->s.origin, self->monsterinfo.nav_goal, vec);
		len = VectorLength(vec);

		if (len < 24)
		{
			//gi.dprintf("gkrokon_maintain_waypoints: arrived at target buoy\n");

			if ((self->monsterinfo.stepState == PATHDIR_FORWARD && self->goalentity->enemy == NULL) ||
				(self->monsterinfo.stepState == PATHDIR_BACKWARD && self->goalentity->owner == NULL))
			{
				//gi.dprintf("gkrokon_maintain_waypoints: path exhausted, seeking target");
				FindTarget(self);
				self->monsterinfo.searchType = SEARCH_COMMON;

				return;
			}

			if (self->monsterinfo.stepState == PATHDIR_FORWARD)
			{
				//gi.dprintf("gkrokon_maintain_waypoints: forward targetting %s\n", self->goalentity->targetname);
				self->enemy = self->movetarget = self->goalentity = self->goalentity->enemy;
				VectorCopy(self->goalentity->s.origin, self->monsterinfo.nav_goal);
			}
			else if (self->monsterinfo.stepState == PATHDIR_BACKWARD)
			{
				//gi.dprintf("gkrokon_maintain_waypoints: reverse targetting %s\n", self->goalentity->owner->targetname);
				self->enemy = self->movetarget = self->goalentity = self->goalentity->owner;
				VectorCopy(self->goalentity->s.origin, self->monsterinfo.nav_goal);
			}
			/*else
				gi.dprintf("gkrokon_maintain_waypoints: volatile assignment\n");*/
		}
	}
}

// ****************************************************************************
// ai_hopdown
//
// Checks to see if entity can hop down safely
// ****************************************************************************
int
ai_hopdown(edict_t *self, vec3_t goalpos, float height_max)
{
	vec3_t		vf, source, source2;
	vec3_t		maxs;
	trace_t		trace;

	//Setup the trace
	VectorCopy(self->maxs, maxs);
	VectorCopy(self->s.origin, source);
	AngleVectors(self->s.angles, vf, NULL, NULL);
	VectorMA(source, 128, vf, source);

	maxs[2] += 128;
	trace = gi.trace(self->s.origin, self->mins, maxs, source, self, MASK_ALL);

	if (trace.fraction == 1)
	{
		VectorCopy(source, source2);

		source2[2] -= height_max;

		trace = gi.trace(source, self->mins, self->maxs, source2, self, MASK_ALL);

		if (trace.allsolid || trace.startsolid)
			return false;

		if (trace.fraction == 1)
		{
			if (Q_stricmp(trace.ent->classname, "worldspawn"))
				return false;
		}
		else
		{
			if (trace.ent == WALL_ENTITY)
			{
				return false;
			}

			if (trace.contents != CONTENTS_SOLID)
				return false;
			else
			{
				VectorSubtract(trace.endpos, self->s.origin, source2);
				VectorNormalize(source2);
				self->ideal_yaw = vectoyaw(source2);

				return true;
			}
		}
	}

	return false;
}

//============================================================================

/*
=============
ai_eat

Monster will eat until harm is done to him or the player moves within range
==============
*/
void
ai_eat(edict_t *self, float dist)
{

	self->enemy = NULL;

	FindTarget (self);
}

/*
 *
 * Used for standing around and looking
 * for players. Distance is for slight
 * position adjustments needed by the
 * animations
 */
void
ai_stand(edict_t *self, float dist)
{
	vec3_t v;
	qboolean retval;

	if (!self)
	{
		return;
	}

	if (dist)
	{
		M_walkmove(self, self->s.angles[YAW], dist);
	}

	if(self->enemy)
		return;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		if (self->enemy)
		{
			VectorSubtract(self->enemy->s.origin, self->s.origin, v);
			self->ideal_yaw = vectoyaw(v);

			if ((self->s.angles[YAW] != self->ideal_yaw) &&
				self->monsterinfo.aiflags & AI_TEMP_STAND_GROUND)
			{
				self->monsterinfo.aiflags &=
					~(AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
				G_QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			}

			if (!(self->monsterinfo.aiflags & AI_MANUAL_STEERING))
			{
				M_ChangeYaw(self);
			}

			/* find out if we're going to be shooting */
			retval = ai_checkattack(self, 0);

			/* record sightings of player */
			if ((self->enemy) && (self->enemy->inuse) &&
				(visible(self, self->enemy)))
			{
				self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
				VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);
				VectorCopy(self->enemy->s.origin, self->monsterinfo.blind_fire_target);
				self->monsterinfo.trail_time = level.time;
				self->monsterinfo.blind_fire_delay = 0;
			}
			/* check retval to make sure we're not blindfiring */
			else if (!retval)
			{
				FindTarget(self);
				return;
			}
		}
		else
		{
			FindTarget(self);
		}

		return;
	}

	if (FindTarget(self))
	{
		return;
	}

	//FIXME: walking a beat monsters, but that may not be working, need to test!
	//check for target also?
	if (self->monsterinfo.pausetime == -1)
	{
		self->spawnflags |= MSF_WANDER;
		self->ai_mood = AI_MOOD_WANDER;
		G_QPostMessage(self, MSG_CHECK_MOOD, PRI_DIRECTIVE, "i", AI_MOOD_WANDER);
		return;
	}
	else if (level.time > self->monsterinfo.pausetime)
	{
		self->ai_mood = AI_MOOD_WALK;
		G_QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
		return;
	}

	if (!(self->spawnflags & MSF_AMBUSH) && (self->monsterinfo.idle) && (level.time > self->monsterinfo.idle_time))
	{
		if (self->monsterinfo.idle_time)
		{
			self->monsterinfo.idle(self);
			self->monsterinfo.idle_time = level.time + 15 + random() * 15;
		}
		else
		{
			self->monsterinfo.idle_time = level.time + random() * 15;
		}
	}
}

/*
 * The monster is walking it's beat
 */
void
ai_walk(edict_t *self, float dist)
{
	if (!self)
	{
		return;
	}

	M_MoveToGoal(self, dist);

	/* check for noticing a player */
	if (FindTarget(self))
	{
		G_QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		return;
	}

	if(self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if(!(self->monsterinfo.aiflags&AI_STRAIGHT_TO_ENEMY))
		{
			MG_BuoyNavigate(self);//only called if already wandering
			MG_Pathfind(self, false);
		}
	}
	else if(!self->enemy && !self->pathtarget)
	{
		if(self->movetarget)
		{
			if(self->movetarget->classname && strcmp(self->movetarget->classname, "path_corner"))
			{
				if(MG_ReachedBuoy(self, self->movetarget->s.origin))
				{
					self->movetarget = NULL;
					G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
				}
			}
		}
		else if(!irand(0, 30) || (Vec3NotZero(self->monsterinfo.nav_goal) && MG_ReachedBuoy(self, self->monsterinfo.nav_goal)))
		{
			self->monsterinfo.pausetime = 0;
			G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		}
	}
}

/*
 * Turns towards target and advances
 * Use this call with a distance of 0
 * to replace ai_face
 */
void
ai_charge(edict_t *self, float dist)
{
	vec3_t v;

	if (!self)
	{
		return;
	}

	if (!self->enemy || !self->enemy->inuse)
	{
		return;
	}

	if (!self->enemy)
	{
		if(mgai_debug->value)
			gi.dprintf("ERROR: AI_CHARGE at a NULL enemy!\n");
		MG_FaceGoal(self, true);//get ideal yaw and turn
		return;//send stand MSG?
	}

	if (!(self->monsterinfo.aiflags & AI_MANUAL_STEERING))
	{
		VectorSubtract(self->enemy->s.origin, self->s.origin, v);
		self->ideal_yaw = vectoyaw(v);
	}

	MG_ChangeYaw(self);

	if(self->spawnflags & MSF_FIXED)
		return;

	if (dist)
	{
		qboolean trace_succeeded = false;

		MG_WalkMove(self, self->s.angles[YAW], dist, &trace_succeeded);
	}
}

/*
=============
ai_moveright

simple sliding right (or left if dist <0)
==============
*/
void
ai_moveright(edict_t *self, float dist)
{
	vec3_t right;
	float movedir;

	if (self->spawnflags & MSF_FIXED)
	{
		return;
	}

	if (!dist)
		return;

	AngleVectors(self->s.angles, NULL, right, NULL);

	movedir = vectoyaw(right);

	qboolean trace_succeeded = false;
	MG_WalkMove (self, movedir, dist, &trace_succeeded);
}

/*
=============
ai_goal_charge

Turns towards target and advances
Use this call with a distnace of 0 to replace ai_face
==============
*/
void
ai_goal_charge(edict_t *self, float dist)
{
	MG_FaceGoal(self, true);

	if(self->spawnflags&MSF_FIXED)
		return;

	if (dist)
	{
		qboolean trace_succeeded = false;

		MG_WalkMove (self, self->s.angles[YAW], dist, &trace_succeeded);
	}
}

/*
=============
ai_charge2

Turns towards target and advances
Use this call with a distnace of 0 to replace ai_face
==============
*/
void
ai_charge2(edict_t *self, float dist)
{
	vec3_t	v;

	if(!self->enemy)
		return;

	VectorSubtract(self->enemy->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);
	MG_ChangeYaw(self);

	if(self->spawnflags & MSF_FIXED)
	{
		return;
	}

	if (dist)
	{
		qboolean trace_succeeded = false;

		MG_WalkMove(self, self->s.angles[YAW], dist, &trace_succeeded);
	}
}

/*
 * Don't move, but turn towards
 * ideal_yaw. Distance is for
 * slight position adjustments
 * needed by the animations
 */
void
ai_turn(edict_t *self, float dist)
{
	if (!self)
	{
		return;
	}

	if (dist)
	{
		if(!(self->spawnflags & MSF_FIXED))
		{
			M_walkmove(self, self->s.angles[YAW], dist);
		}
	}

	if (FindTarget(self))
	{
		return;
	}

	if (!(self->monsterinfo.aiflags & AI_MANUAL_STEERING))
	{
		M_ChangeYaw(self);
	}
}

/* ============================================================================ */

/*
 *
 * .enemy
 * Will be world if not currently angry at anyone.
 *
 * .movetarget
 * The next path spot to walk toward.  If .enemy,
 * ignore .movetarget. When an enemy is killed,
 * the monster will try to return to it's path.
 *
 * .hunt_time
 * Set to time + something when the player is in
 * sight, but movement straight for him is blocked.
 * This causes the monster to use wall following code for
 * movement direction instead of sighting on the player.
 *
 * .ideal_yaw
 * A yaw angle of the intended direction, which will be
 * turned towards at up to 45 deg / state. If the enemy
 * is in view and hunt_time is not active, this will be
 * the exact line towards the enemy.
 *
 * .pausetime
 * A monster will leave it's stand state and head towards
 * it's .movetarget when time > .pausetime.
 *
 * walkmove(angle, speed) primitive is all or nothing
 */

int
categorize_range(edict_t *self, edict_t *other, float len)
{
	float	dist;

	if (self->monsterinfo.aiflags & AI_EATING) // Eating
	{
		if (len < MELEE_DISTANCE)	// Melee distance
		{
			if ( len < (self->maxs[0] + other->maxs[0] + 15))
				return RANGE_MELEE;
		}

		if (len < 175)				// Attacking distance
			return RANGE_NEAR;
		if (len < 350)				// Hissing distance
			return RANGE_MID;
		return RANGE_FAR;
	}
	else				// Not eating
	{
		if (len < MELEE_DISTANCE)
		{
			if ( len < (self->maxs[0] + other->maxs[0] + 25))
				return RANGE_MELEE;
		}
		if (len < 500)
			return RANGE_NEAR;

		if(self->wakeup_distance)
			dist = self->wakeup_distance;
		else
			dist = MAX_SIGHT_PLAYER_DIST;

		if (len < dist)
			return RANGE_MID;
		return RANGE_FAR;
	}
}


/* ============================================================================ */

/*
 * returns the range categorization of an entity relative to self
 * 0	melee range, will become hostile even if back is turned
 * 1	visibility and infront, or visibility and show hostile
 * 2	infront and show hostile
 * 3	only triggered by damage
 */
int
range(edict_t *self, edict_t *other)
{
	vec3_t v;
	float len;

	if (!self || !other)
	{
		return 0;
	}

	VectorSubtract(self->s.origin, other->s.origin, v);
	len = VectorLength(v);

	return categorize_range(self, other, len);
}

/*
 * returns 1 if the entity is visible
 * to self, even if not infront
 */
qboolean
visible(edict_t *self, edict_t *other)
{
	vec3_t spot1;
	vec3_t spot2;
	trace_t trace;

	if (!self || !other)
	{
		return false;
	}

	/*
	 * [Paril-KEX] bit of a hack, but we'll tweak monster-player visibility
	 * if they have the invisibility powerup.
	 */
	if (other->client)
	{
		if (other->client->invisible_framenum > level.framenum)
		{
			return false;
		}
	}

	VectorCopy(self->s.origin, spot1);
	spot1[2] += self->viewheight;
	if(self->classID == CID_TBEAST)
	{
		vec3_t	forward;

		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorMA(spot1, self->maxs[0], forward, spot1);
	}
	VectorCopy(other->s.origin, spot2);
	spot2[2] += other->viewheight;
	trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE);

	if ((trace.fraction == 1.0) || (trace.ent == other))
	{
		return true;
	}

	return false;
}

/*
=============
clear_visible

returns 1 if the entity is visible, but not through transparencies
=============
*/
qboolean
clear_visible(edict_t *self, edict_t *other)
{
	vec3_t spot1;
	vec3_t spot2;
	trace_t trace;

	if (!self || !other)
	{
		return false;
	}

	VectorCopy(self->s.origin, spot1);
	spot1[2] += self->viewheight;
	VectorCopy(other->s.origin, spot2);
	spot2[2] += other->viewheight;
	trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, self, MASK_SOLID);

	if (trace.fraction == 1.0)
	{
		return true;
	}

	return false;
}

/*
 * returns 1 if the entity is in
 * front (in sight) of self
 */
qboolean
infront(edict_t *self, edict_t *other)
{
	vec3_t vec;
	float dot;
	vec3_t forward, check_angles;

	if (!self || !other)
	{
		return false;
	}

	if(Vec3NotZero(self->v_angle_ofs))
		VectorAdd(self->v_angle_ofs,self->s.angles,check_angles);
	else
		VectorCopy(self->s.angles,check_angles);

	AngleVectors(check_angles, forward, NULL, NULL);

	VectorSubtract(other->s.origin, self->s.origin, vec);
	VectorNormalize(vec);
	dot = DotProduct(vec, forward);

	if (dot > 0.3)
	{
		return true;
	}

	return false;
}

/* ============================================================================ */


/*
===========
HuntTarget - a target has been found, it is visible, so do we attack it, watch it, or stand there?

============
*/
void
HuntTarget(edict_t *self)
{
	vec3_t vec;
	int r;

	if (!self)
	{
		return;
	}

	self->goalentity = self->enemy;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
	else
	{
		r = range(self,self->enemy);
		if ((self->monsterinfo.aiflags & AI_EATING) && (r == RANGE_MID))
		{
			G_QPostMessage(self, MSG_WATCH, PRI_DIRECTIVE, NULL);
		}
		else
		{
			G_QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		}
	}

	if(visible(self, self->enemy))
	{
		VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	}
	else
	{
		VectorClear(vec);
	}

	self->ideal_yaw = vectoyaw(vec);

	/* wait a while before first attack */
	if (!(self->monsterinfo.aiflags & AI_STAND_GROUND))
	{
		AttackFinished(self, 1);
	}
}

/*
===========
FoundTarget - a target has been found, let other monsters know this. Then hunt it
============
*/
void
FoundTarget(edict_t *self, qboolean setsightent)
{
	edict_t *combatpoint;
	vec3_t v;
	char *o_target;

	if (!self || !self->enemy || !self->enemy->inuse)
	{
		return;
	}

	if(self->classID == CID_OGLE)
	{
		self->spawnflags = 0;
	}

	self->monsterinfo.awake = true;
	self->spawnflags &= ~MSF_AMBUSH;
	self->targetname = NULL;
	self->monsterinfo.pausetime = -1;//was 0;

	if (self->wakeup_target)
	{
		o_target = self->target;
		self->target = self->wakeup_target;
		G_UseTargets(self, self->enemy);
		self->target = o_target;
		self->wakeup_target = NULL;
	}

	/* let other monsters see this monster for a while */
	if (self->enemy->client && setsightent)
	{
		level.sight_entity = self;
		level.sight_entity_framenum = level.framenum;
		level.sight_entity->light_level = 128;
	}

	self->show_hostile = level.time + 1; /* wake up other monsters */

	VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);

	if (!self->combattarget)
	{
		// Not going for a combat point?
		if (self->classID != CID_FISH)
		{
			// dont want to do this if we are a fish
			HuntTarget(self);
		}

		if (!self->oldenemy)
		{
			if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET))
			{
				G_QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be",
					SIGHT_SOUND_TARGET, self->enemy);
			}
			else
			{
				G_QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be",
					SIGHT_VISIBLE_TARGET, self->enemy);
			}
		}

		self->spawnflags &= ~MSF_AMBUSH;
		return;
	}

	combatpoint = G_PickTarget(self->combattarget);

	if (!combatpoint)
	{
		gi.dprintf("%s at %s, combattarget %s not found\n",
				self->classname,
				vtos(self->s.origin),
				self->combattarget);

		if (self->classID != CID_FISH)
		{
			/* dont want to do this if we are a fish */
			HuntTarget(self);
		}

		if (!self->oldenemy)
		{
			if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET))
			{
				G_QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be",
					SIGHT_SOUND_TARGET, self->enemy);
			}
			else
			{
				G_QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be",
					SIGHT_VISIBLE_TARGET, self->enemy);
			}
		}

		self->spawnflags &= ~MSF_AMBUSH;
		return;
	}

	VectorSubtract(combatpoint->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);

	/* clear out our combattarget, these are a one shot deal */
	self->combattarget = NULL;
	self->monsterinfo.aiflags |= AI_COMBAT_POINT;
	self->monsterinfo.pausetime = 0;

	/* clear the targetname, that point is ours! */
	combatpoint->targetname = NULL;
	self->goalentity = self->movetarget = combatpoint;

	/* run for it , assuming we aren't a fish */
	if (self->classID != CID_FISH)
	{
		G_QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
	}

	/* Make a sight sound */
	if (!self->oldenemy)
	{
		if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET))
		{
			G_QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be",
				SIGHT_SOUND_TARGET, self->enemy);
		}
		else
		{
			G_QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "be",
				SIGHT_VISIBLE_TARGET, self->enemy);
		}
	}

	self->spawnflags &= ~MSF_AMBUSH;
}

static qboolean
PlayerCreeping(playerinfo_t *playerinfo)
{
	if(playerinfo->upperseq == ASEQ_CREEPF ||
		playerinfo->upperseq == ASEQ_STAND ||
		playerinfo->upperseq == ASEQ_CREEPB ||
		playerinfo->upperseq == ASEQ_CREEPB_END ||
		playerinfo->upperseq == ASEQ_CROUCH_GO ||
		playerinfo->upperseq == ASEQ_CROUCH ||
		playerinfo->upperseq == ASEQ_CROUCH_END ||
		playerinfo->upperseq == ASEQ_CROUCH_WALK_F ||
		playerinfo->upperseq == ASEQ_CROUCH_WALK_B ||
		playerinfo->upperseq == ASEQ_CROUCH_WALK_L ||
		playerinfo->upperseq == ASEQ_CROUCH_WALK_R ||
		playerinfo->lowerseq == ASEQ_CREEPF ||
		playerinfo->lowerseq == ASEQ_STAND ||
		playerinfo->lowerseq == ASEQ_CREEPB ||
		playerinfo->lowerseq == ASEQ_CREEPB_END ||
		playerinfo->lowerseq == ASEQ_CROUCH_GO ||
		playerinfo->lowerseq == ASEQ_CROUCH ||
		playerinfo->lowerseq == ASEQ_CROUCH_END||
		playerinfo->lowerseq == ASEQ_CROUCH_WALK_F ||
		playerinfo->lowerseq == ASEQ_CROUCH_WALK_B ||
		playerinfo->lowerseq == ASEQ_CROUCH_WALK_L ||
		playerinfo->lowerseq == ASEQ_CROUCH_WALK_R)
		return (true);

	return (false);
}

/*
Alerted

Checks and see if an alert entity is capable of waking up a monster
*/
static qboolean
Alerted(edict_t *self)
{
	//This alert entity wakes up monsters, let's see what's up...
	edict_t		*enemy = NULL;
	alertent_t	*alerter;
	vec3_t		e_spot, viewspot, dir;
	qboolean	saw_it = false;
	float		dist;

	if(self->monsterinfo.aiflags&AI_NO_ALERT)
		return false;

	//start the search from the most recent alert to the oldest
	alerter = level.last_alert;//OOPS, SKIPS LAST

alertloop:

	if(self->enemy)
		goto loopagain;

	//alerter is gone
	if(!alerter)
		goto loopagain;

	if(!alerter->inuse)//loading a saved game invalidates all alerts
		goto loopagain;

	if(alerter->lifetime < level.time)
	{//alert timed out, remove from list
		alert_timed_out(alerter);
		goto loopagain;
	}

	if(self->last_alert)//don't be woken up by the same alert twice
	{
		if(self->last_alert->inuse)
		{
			if(alerter == self->last_alert && self->last_alert->inuse)
				goto loopagain;
		}
		else
			self->last_alert = NULL;
	}

	//alerter's enemy is gone
	if(!alerter->enemy)
		goto loopagain;

	if(alerter->enemy->client)
	{//no alerts for notarget players
		if(alerter->enemy->flags & FL_NOTARGET)
			goto loopagain;
	}

	//NEVER alert ambush monsters?
	//if(!(self->spawnflags & MSF_AMBUSH))
	//	goto loopagain;

	 if(!(self->svflags&SVF_MONSTER))
		goto loopagain;

	if(self->health<=0)
		goto loopagain;

	//eating or in a cinematic or not awake, leave them alone
	if(!ok_to_wake(self, false, true))
		goto loopagain;

	//should we keep track of owner in case they move to move the alert with them?  Only for monsters
	//if(alerter->owner)
	//	VectorCopy(alerter->owner->s.origin, alerter->s.origin);

	VectorSubtract(self->s.origin, alerter->origin, dir);
	dist = VectorLength(dir);

	//if monster's wakeup_distance is shorter than dist to alerter, leave it alone
	if(dist > self->wakeup_distance)
		goto loopagain;

	//closer means better chance to alert
	//problem - different alerts might be more likely to be heard/seen...
	if(dist > flrand(100, self->wakeup_distance))//if within 100 always wake up?
		goto loopagain;

	//if not a player, a player's missile or a monster, goto loopagain?
	//get center of alert enemy
	enemy = alerter->enemy;

	VectorAdd(enemy->s.origin, enemy->mins, e_spot);
	VectorMA(e_spot, 0.5, enemy->size, e_spot);

	//get my view spot
	VectorCopy(self->s.origin, viewspot);
	viewspot[2] += self->viewheight;

	//if being alerted by a monster and not waiting to ambush
	if(alerter->alert_svflags&SVF_MONSTER && !(self->spawnflags&MSF_AMBUSH))
	{//can "see" the owner of the alert even around a corner
		saw_it = gi.inPVS(e_spot, viewspot);
	}
	else
	{//alerter not a monster, a projectile or player
		//can I see (even through translucent brushes) the owner of the alert?
		//if so, ok even if I'm an ambush monster
		saw_it = visible_pos(self, e_spot);

		if(!saw_it&&!(self->spawnflags&MSF_AMBUSH))
		{//no line of sight and not an ambush monster
			if(gi.inPVS(viewspot, alerter->origin))
			{//25% chance will see impact(alerter) and detect alert owner anyway
				if(!irand(0,3))
					saw_it = true;//go ahead and go for it
			}
		}
	}

	if(!saw_it)
		goto loopagain;

	if(!self->monsterinfo.alert)
		goto loopagain;

	self->last_alert = alerter;
	return self->monsterinfo.alert(self, alerter, enemy);

loopagain:
	if(alerter)
	{
		alerter = alerter->prev_alert;
		if(alerter)
			goto alertloop;
	}

	return false;
}

/*
 * Self is currently not attacking anything,
 * so try to find a target
 *
 * Returns TRUE if an enemy was sighted
 *
 * When a player fires a missile or does other things to make noise, the point
 * of impact becomes an alertent so that
 * monsters that see the impact will respond
 * as if they had seen the player.
 *
 * Since FindTarget is not called every frame for monsters (average
 * about once every 3 frames per monster), this does two potential checks.
 *
 * First it checks against the current sight_client which cycles through
 * the players.
 *
 * If that check fails, it will check for all the secondary alerts and
 * enemies.  if it can't find any, it will check for another player if
 * it can find one other than the first one it checked.
 */
qboolean
FindTarget(edict_t *self)
{
	edict_t *client;
	qboolean heardit;
	int r;
	edict_t *ent, *firstclient = NULL;
	int flag;
	qboolean clientonly = true;
	qboolean e_infront = false;
	vec3_t v;
	float dist;

	if (!self)
	{
		return false;
	}

//FIXME: wakeup_distance -1 never look?
	if(self->classID == CID_OGLE)
		return ogle_findtarget(self);

	if (self->monsterinfo.aiflags & AI_GOOD_GUY)
	{
		if (self->goalentity)
		{
			if (strcmp(self->goalentity->classname, "target_actor") == 0)
				return false;
		}

		//FIXME look for monsters?
		return false;
	}

	/* if we're going to a combat point, just proceed */
	if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
	{
		return false;
	}

	/* if the first spawnflag bit is set, the monster
	   will only wake up on really seeing the player,
	   not another monster getting angry or hearing
	   something */

	heardit = false;

	if(self->ai_mood_flags & AI_MOOD_FLAG_IGNORE_ENEMY)
	{
		/* being forced to use buoys, and ignore enemy until get to forced_buoy */
		return false;
	}

// revised behavior so they will wake up if they "see" a player make a noise
// but not weapon impact/explosion noises

startcheck:
	flag = 1;
	if(clientonly)
	{//look oly at the level.sight_client
		firstclient = client = level.sight_client;
	}
	else
	{
		if(anarchy->value)
		{//crazy monsters mode
			int	checkcnt = 0;
			client = self;
			while((!client || !client->inuse || !(client->svflags & SVF_MONSTER)||client->health<=0||client == self) && checkcnt < globals.num_edicts)
			{
				client = &g_edicts[irand(0, globals.num_edicts)];
				checkcnt++;
			}
		}
		else
		{
			if(level.sight_entity == self)
			{
				level.sight_entity = NULL;
				return false;
			}

			if (level.sight_entity && (level.sight_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & MSF_AMBUSH) )
			{//go after the enemy another monster saw saw, but only if not in ambush
				client = level.sight_entity;
				if (client->enemy == self->enemy)
				{
					return false;
				}
			}
			else if (Alerted(self))
			{//picked up an enemy from an alert
				return true;
			}
			else
			{
				client = NULL;
				// Looking for secondary enemies
				if ((self->monsterinfo.otherenemyname) && (self->monsterinfo.chase_finished < level.time))
				{
					ent = NULL;
					while((ent = newfindradius(ent,self->s.origin,175)) != NULL)
					{
						if (!strcmp(ent->classname,self->monsterinfo.otherenemyname)&&ent!=self)
						{
							flag = 0;
							client = ent;
							break;
						}
					}
				}

				//  Look at the sight client
				if (!client)
				{//found no non-clients, cycle to next client and check it for second check
					AI_SetSightClient();
					if(firstclient == level.sight_client)
						return false;//same as first check, and that failed if we're here, so return.
					client = level.sight_client;
				}
			}
		}
	}
	if (!client)
		goto nextcheck;	// no clients to get mad at

	// if the entity went away, forget it
	if (!client->inuse)
		goto nextcheck;

	if (client == self)
		goto nextcheck;	//????

	if (client == self->enemy)
	{
		return true;
	}

	if (self->monsterinfo.otherenemyname)
	{
		if (!strcmp(client->classname,self->monsterinfo.otherenemyname))
			client->light_level = 128; // Let it be seen
	}

	// if we are a fish - is the target in the water - have to be at least waist deep?
	if (self->classID == CID_FISH && client->waterlevel < 2)
		goto nextcheck;	//????

	if (client->client)
	{
		if (client->flags & FL_NOTARGET)
			goto nextcheck;
	}
	else if (client->svflags & SVF_MONSTER)
	{
		if(flag)
		{//not a secondary enemy
			if(!anarchy->value)
			{
				if (ok_to_wake(self, false, true))
				{//eating or in a cinematic or not awake or targeted, leave them alone
					goto nextcheck;
				}
			}

			if (!client->enemy)
			{
				if(!anarchy->value)
					goto nextcheck;
			}
			else
			{
				if (client->enemy->health<0 && !anarchy->value)
					goto nextcheck;

				if (client->enemy->flags & FL_NOTARGET)
					goto nextcheck;
			}

			if(!visible(self, client))
				goto nextcheck;

			if(!anarchy->value)
				self->enemy = client->enemy;
			else
				self->enemy = client;

			if(client->ai_mood == AI_FLEE)
				FoundTarget(self, false);//let them stay the sight entity
			else
				FoundTarget(self, true);//make me the sight entity

			/*if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET))
				G_QPostMessage(self, MSG_VOICE_SIGHT, PRI_DIRECTIVE, "e", self->enemy);*/
				//self->monsterinfo.sight (self, self->enemy);

			return true;
		}
	}
	else if (heardit)
	{
		if ((client->owner) && (client->owner->flags & FL_NOTARGET))
		{
			return false;
		}
	}
	else
		goto nextcheck;

	if (!heardit)
	{
		if(self->classID == CID_ASSASSIN)
			e_infront = true;
		else
			e_infront = infront(self, client);

		if (!e_infront && client->client)
		{
			if(PlayerCreeping(&client->client->playerinfo))
				goto nextcheck;
		}

		VectorSubtract(client->s.origin, self->s.origin, v);
		dist = VectorLength(v);

		if(dist > self->wakeup_distance)
			goto nextcheck;

		r = categorize_range (self, client, dist);

		if (r == RANGE_FAR)
			goto nextcheck;

		if ((self->monsterinfo.aiflags & AI_EATING) && (r > RANGE_MID))
		{
			self->enemy = client;
			goto nextcheck;
		}

// this is where we would check invisibility

		// is client in an spot too dark to be seen?
		if (client->light_level <= 5)
			goto nextcheck;

		if(self->svflags&SVF_MONSTER && client->client)
		{
			if(skill->value < 2.0 && !(self->monsterinfo.aiflags & AI_NIGHTVISION))
			{
				if(client->light_level < flrand(6, 77))
				{
					goto nextcheck;
				}
			}
		}

		if (r == RANGE_NEAR)
		{
			if (client->show_hostile < level.time && !e_infront)
			{
				goto nextcheck;
			}
		}
		else if (r == RANGE_MID)
		{
			if (!e_infront)
			{
				goto nextcheck;
			}
		}

		//sfs--this check is much less trivial than infront: prolly wasn't a noticeable deal,
		//		since RANGE_FAR was first rejection check, but it's still better to try the
		//		dotproduct before the traceline
		if (!visible (self, client))
		{
			goto nextcheck;
		}

		self->enemy = client;

		flag=1;
		if (self->monsterinfo.otherenemyname)
		{
			if (strcmp(self->enemy->classname, self->monsterinfo.otherenemyname) == 0)	// This is a secondary enemy
			{
				self->monsterinfo.chase_finished = level.time + 15;
				flag=0;
			}
		}

		if (flag)	// This is not a secondary enemy
		{
			self->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

			if (!self->enemy->client)
			{
				self->enemy = self->enemy->enemy;

				if (!self->enemy->client)
				{
					self->enemy = NULL;
					goto nextcheck;
				}
			}

			if ((self->enemy->client) &&
				(self->enemy->client->invisible_framenum > level.framenum))
			{
				self->enemy = NULL;
				return false;
			}
		}
	}
	else /* heardit */
	{
		goto nextcheck;

	}

//
// got one
//
	FoundTarget (self, true);

	return true;

nextcheck:
	if(clientonly)
	{
		clientonly = false;
		goto startcheck;
	}
	else
		return false;
}

/* ============================================================================= */

qboolean
FacingIdeal(edict_t *self)
{
	float delta;

	if (!self)
	{
		return false;
	}

	delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);

	if ((delta > 45) && (delta < 315))
	{
		return false;
	}

	return true;
}

/* ============================================================================= */

qboolean
M_CheckAttack(edict_t *self)
{
	vec3_t spot1, spot2;
	float chance;
	trace_t tr;

	if (!self || !self->enemy || !self->enemy->inuse)
	{
		return false;
	}

	if (self->enemy->health > 0)
	{
		if (self->enemy->client)
		{
			if (self->enemy->client->invisible_framenum > level.framenum)
			{
				return false;
			}
		}

		/* see if any entities are in the way of the shot */
		VectorCopy(self->s.origin, spot1);
		spot1[2] += self->viewheight;
		VectorCopy(self->enemy->s.origin, spot2);
		spot2[2] += self->enemy->viewheight;

		tr = gi.trace(spot1, vec3_origin, vec3_origin, spot2, self,
				CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_SLIME |
				CONTENTS_LAVA);

		/* do we have a clear shot? */
		if (tr.ent != self->enemy)
		{
			return false;
		}
	}

	/* melee attack */
	if (enemy_range == RANGE_MELEE)
	{
		/* don't always melee in easy mode */
		if ((skill->value == SKILL_EASY) && (randk() & 3))
		{
			return false;
		}

		if (classStatics[self->classID].msgReceivers[MSG_MELEE])
		{
			self->monsterinfo.attack_state = AS_MELEE;
		}
		else
		{
			self->monsterinfo.attack_state = AS_MISSILE;
		}

		return true;
	}

	/* missile attack */
	if (!classStatics[self->classID].msgReceivers[MSG_MISSILE])
	{
		return false;
	}

	if (level.time < self->monsterinfo.attack_finished)
	{
		return false;
	}

	if (enemy_range == RANGE_FAR)
	{
		return false;
	}

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_MELEE)
	{
		chance = 0.2;
	}
	else if (enemy_range == RANGE_NEAR)
	{
		chance = 0.1;
	}
	else if (enemy_range == RANGE_MID)
	{
		chance = 0.02;
	}
	else
	{
		return false;
	}

	if (skill->value == SKILL_EASY)
	{
		chance *= 0.5;
	}
	else if (skill->value >= SKILL_HARD)
	{
		chance *= 2;
	}

	/* go ahead and shoot every time if it's a info_notnull */
	if ((random() < chance) || (self->enemy->solid == SOLID_NOT))
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + 2 * random();
		return true;
	}

	/* daedalus should strafe more.. this can be done
	   here or in a customized check_attack code for
	   the hover. */
	if (self->flags & FL_FLY)
	{
		/* originally, just 0.3 */
		float strafe_chance;

		if (!(strcmp(self->classname, "monster_daedalus")))
		{
			strafe_chance = 0.8;
		}
		else
		{
			strafe_chance = 0.6;
		}

		/* if enemy is tesla, never strafe */
		if ((self->enemy->classname) &&
			(!strcmp(self->enemy->classname, "tesla")))
		{
			strafe_chance = 0;
		}

		if (random() < strafe_chance)
		{
			self->monsterinfo.attack_state = AS_SLIDING;
		}
		else
		{
			self->monsterinfo.attack_state = AS_STRAIGHT;
		}
	}
	else
	{
		/* do we want the monsters strafing? */
		if (random() < 0.4)
		{
			self->monsterinfo.attack_state = AS_SLIDING;
		}
		else
		{
			self->monsterinfo.attack_state = AS_STRAIGHT;
		}
	}

	return false;
}

/*
 * Turn and close until within an
 * angle to launch a melee attack
 */
void
ai_run_melee(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->ideal_yaw = enemy_yaw;

	if (!(self->monsterinfo.aiflags & AI_MANUAL_STEERING))
	{
		M_ChangeYaw(self);
	}

	if (FacingIdeal(self))
	{
		G_QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
}

/*
 * Turn in place until within an
 * angle to launch a missile attack
 */
void
ai_run_missile(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->ideal_yaw = enemy_yaw;

	if (!(self->monsterinfo.aiflags & AI_MANUAL_STEERING))
	{
		M_ChangeYaw(self);
	}

	if (FacingIdeal(self))
	{
		G_QPostMessage(self, MSG_MISSILE, PRI_DIRECTIVE, NULL);
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
};


/*
 * Strafe sideways, but stay at
 * aproximately the same range
 */
void
ai_run_slide(edict_t *self, float distance)
{
	float ofs;
	float angle;

	if (!self)
	{
		return;
	}

	self->ideal_yaw = enemy_yaw;
	angle = 90;

	if (self->monsterinfo.lefty)
	{
		ofs = angle;
	}
	else
	{
		ofs = -angle;
	}

	if (!(self->monsterinfo.aiflags & AI_MANUAL_STEERING))
	{
		M_ChangeYaw(self);
	}

	if (M_walkmove(self, self->ideal_yaw + ofs, distance))
	{
		return;
	}

	self->monsterinfo.lefty = 1 - self->monsterinfo.lefty;
	M_walkmove(self, self->ideal_yaw - ofs, distance);
}

/*
=============
ai_checkattack

Decides if we're going to attack or do something else
used by ai_run, and ai_stand
=============
*/
qboolean
ai_checkattack(edict_t *self, float dist)
{
	vec3_t		temp;
	qboolean	hesDeadJim;

	if ((self->monsterinfo.aiflags & AI_FLEE)||(self->monsterinfo.aiflags & AI_COWARD))	// He's running away, not attacking
	{
		return false;
	}

	enemy_vis = false;

// see if the enemy is dead
	hesDeadJim = false;
	if ((!self->enemy) || (!self->enemy->inuse))
	{
		hesDeadJim = true;
	}
	else
	{
		if (self->monsterinfo.aiflags & AI_BRUTAL)
		{
			if (self->enemy->health <= -80)
				hesDeadJim = true;
		}
		else
		{
			if (self->enemy->health <= 0)
				hesDeadJim = true;
		}
	}

	if (hesDeadJim)
	{
		self->enemy = NULL;
	// FIXME: look all around for other targets
		if (self->oldenemy && self->oldenemy->health > 0)
		{
			self->enemy = self->oldenemy;
			self->oldenemy = NULL;
			HuntTarget (self);
		}
		else
		{
			if (self->movetarget)
			{
				self->goalentity = self->movetarget;
				G_QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			}
			else
			{
				// we need the pausetime otherwise the stand code
				// will just revert to walking with no target and
				// the monsters will wonder around aimlessly trying
				// to hunt the world entity
				self->monsterinfo.pausetime = level.time + 100000000;
				G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			}
			return true;
		}
	}

	self->show_hostile = level.time + 1;		// wake up other monsters

	/* check knowledge of enemy */
	enemy_vis = clear_visible(self, self->enemy);
	if (enemy_vis)
	{
		self->monsterinfo.search_time = level.time + 5;
		VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);
	}

	enemy_infront = infront(self, self->enemy);
	enemy_range = range(self, self->enemy);
	VectorSubtract(self->enemy->s.origin, self->s.origin, temp);
	enemy_yaw = vectoyaw(temp);


	// JDC self->ideal_yaw = enemy_yaw;

	if (self->monsterinfo.attack_state == AS_MISSILE)
	{
		ai_run_missile (self);
		return true;
	}
	if (self->monsterinfo.attack_state == AS_MELEE)
	{
		ai_run_melee (self);
		return true;
	}

	// if enemy is not currently visible, we will never attack
	if (!enemy_vis)
		return false;

	return self->monsterinfo.checkattack (self);
}

qboolean
ai_inpack(edict_t *self)
{
	edict_t *ent;

	ent = NULL;
	while ((ent = newfindradius(ent, self->s.origin, 1024)) != NULL)
	{
		if (ent == self)
			continue;

		if (Q_stricmp(ent->classname, self->classname))
			continue;

		return true;
	}

	return false;
}

/*
=============
ai_runaway

The monster has an enemy it is trying to flee
=============
*/
void
ai_runaway(edict_t *self, float dist)
{
	vec3_t		move;
	vec3_t		vec, source, vf;
	vec3_t		mins;
	trace_t		trace;
	vec3_t		na, nvr;
	float		yaw;
	int			ret;

	if (!self->enemy)
		return;

	//Setup the trace
	VectorCopy(self->mins, mins);
	VectorCopy(self->s.origin, source);
	AngleVectors(self->s.angles, vf, NULL, NULL);
	VectorMA(source, dist*2, vf, source);

	//Account for STEPSIZE
	mins[2] += STEPSIZE;

	trace = gi.trace(self->s.origin, mins, self->maxs, source, self, MASK_SHOT);

	//We hit something
	if (trace.fraction < 1)
	{
		VectoAngles(trace.plane.normal,na);
		AngleVectors(na,NULL,nvr,NULL);

		if(DotProduct(nvr,vf)>0)
			self->ideal_yaw=vectoyaw(nvr);
		else
		{
			VectorScale(nvr, -1, nvr);
			self->ideal_yaw=vectoyaw(nvr);
		}

		VectorCopy(self->mins, mins);
		VectorCopy(self->s.origin, source);
		AngleVectors(self->s.angles, vf, NULL, NULL);
		VectorMA(source, dist*4, nvr, source);

		//Account for STEPSIZE
		mins[2] += STEPSIZE;

		trace = gi.trace(self->s.origin, mins, self->maxs, source, self, MASK_SHOT);

		if (trace.fraction < 1)
		{
			//gi.dprintf("Failed next move!\n");
			VectorScale(nvr, -1, nvr);
			self->ideal_yaw=vectoyaw(nvr);
		}

		M_ChangeYaw(self);

		if (fabs(self->s.angles[YAW] - self->ideal_yaw) < self->yaw_speed)
		{
			yaw = self->s.angles[YAW];
			yaw = yaw*M_PI*2 / 360;

			move[0] = cos(yaw)*dist * 4;
			move[1] = sin(yaw)*dist * 4;
			move[2] = 0;

			ret = ai_trystep(self, move);

			if ((ret != TRYSTEP_OK) && (self->monsterinfo.idle_time < level.time))
			{
				self->monsterinfo.idle_time = level.time + flrand(1.0, 2.0);
				SV_NewChaseDir(self, self->enemy, dist);
			}
			else if (!M_walkmove(self, self->s.angles[YAW], dist))
			{
				self->monsterinfo.idle_time = level.time + flrand(1.0, 2.0);
				SV_NewChaseDir(self, self->enemy, dist);
			}
		}

		return;
	}

	VectorSubtract(self->enemy->s.origin, self->s.origin, vec);

	if (self->monsterinfo.idle_time < level.time)
	{
		self->ideal_yaw = vectoyaw(vec);
		self->ideal_yaw += 180;
		M_ChangeYaw(self);
	}

	if (dist) //Going somewhere?
	{
		if(self->classID == CID_SSITHRA)
			ssithraCheckJump(self);
		if (!M_walkmove(self, self->s.angles[YAW], dist))
		{
			if (self->monsterinfo.searchType == SEARCH_BUOY)
				return;

			yaw = self->s.angles[YAW];
			yaw = yaw*M_PI*2 / 360;

			move[0] = cos(yaw)*dist;
			move[1] = sin(yaw)*dist;
			move[2] = 0;

			ret = ai_trystep(self, move);

			if ((ret != TRYSTEP_OK) && (self->monsterinfo.idle_time < level.time))
			{
				self->monsterinfo.idle_time = level.time + flrand(1.0, 2.0);
				SV_NewChaseDir(self, self->enemy, dist);
			}
			else
			{
				M_ChangeYaw(self);

				if (!M_walkmove(self, self->s.angles[YAW], dist/2)&&(self->classID != CID_SSITHRA))
				{
					if (ai_hopdown(self, self->enemy->s.origin, 1024))
					{
						if (self->monsterinfo.jump_time < level.time)
						{
							//FIXME: Re-implement this!
							//gkrokon_hopdown(self);
							self->monsterinfo.jump_time = level.time + 1;
						}
						else return;
					}
				}
			}
		}
	}
}

float
ai_face_goal(edict_t *self)
{
	vec3_t vec;

	if (self->monsterinfo.searchType == SEARCH_BUOY)
		VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, vec);
	else if(self->goalentity)
		VectorSubtract(self->goalentity->s.origin, self->s.origin, vec);
	else if(self->enemy)
		VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	else
		return false;

	if (self->monsterinfo.idle_time < level.time)
	{
		self->ideal_yaw = vectoyaw(vec);
		return M_ChangeYaw(self);
	}
	else
		return false;
}

void
old_ai_run(edict_t *self, float dist)
{
	//FIXME: only ssithra in water use this, let;s junk it!
	vec3_t		move;
	vec3_t		vec, source, vf;
	vec3_t		mins;
	trace_t		trace;
	vec3_t		na, nvr;
	float		yaw, turnamt, distloss;
	int			ret;

	if (!self->enemy)
		return;


	if(self->classID == CID_GORGON)
	{
		if(visible(self, self->enemy))
		{
			if(Vec3IsZero(self->enemy->velocity))
			{
				VectorCopy(self->enemy->s.origin, self->pos1);
			}
			else
				VectorClear(self->pos1);
		}
		else
			VectorClear(self->pos1);
	}

	if (self->monsterinfo.attack_state == AS_SLIDING)
	{
		ai_run_slide (self, dist);
		return;
	}

	/* He's done fleeing, time to stand and see what's happening */
	if ((self->monsterinfo.aiflags & AI_FLEE) && (self->monsterinfo.flee_finished < level.time))
	{
		self->monsterinfo.aiflags &= ~AI_FLEE;
		if(irand(0,10<2))
		{
			self->enemy = NULL;
			G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		}
		return;
	}

	if (self->monsterinfo.aiflags & AI_FLEE||self->monsterinfo.aiflags & AI_COWARD)
	{
		VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
		self->ideal_yaw = vectoyaw(vec);
		self->ideal_yaw += 180;
		ai_runaway(self, dist);
		return;
	}

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		turnamt = Q_fabs(ai_face_goal(self));
		return;
	}

	if (self->monsterinfo.aiflags & AI_SOUND_TARGET)
	{
		self->monsterinfo.aiflags &= ~AI_SOUND_TARGET;
		if (!FindTarget (self))
		{
			G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			return;
		}
	}

	if (!self->enemy)
	{
		G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	/* Setup the trace */
	VectorCopy(self->mins, mins);
	VectorCopy(self->s.origin, source);
	AngleVectors(self->s.angles, vf, NULL, NULL);

	VectorMA(source, (dist*2)/*+bbox*/, vf, source);

	//Account for STEPSIZE
	mins[2] += STEPSIZE;

	trace = gi.trace(self->s.origin, mins, self->maxs, source, self, MASK_SHOT);

	//We hit something
	if (trace.fraction < 1)
	{
		if (trace.ent == self->enemy)
		{
			G_QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			return;
		}
		else if(self->monsterinfo.otherenemyname)
		{
			if(!Q_stricmp(trace.ent->classname, self->monsterinfo.otherenemyname))//&&irand(0,10)<3)
			{
				G_QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
				return;
			}
		}

		if(trace.ent->takedamage && self->monsterinfo.aiflags & AI_SHOVE && trace.fraction <= 0.5)
		{//hurt them some too?
			if(self->classID == CID_GORGON &&
				(AVG_VEC3T(self->rrs.scale) > 1.5) &&
				trace.ent->classID!=self->classID &&
				trace.ent->health <= 200&&
				infront(self, self->enemy))//chomp!
			{
				self->oldenemy = self->enemy;
				self->enemy = trace.ent;
				G_QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			}
			else
			{
				VectorSubtract(trace.ent->s.origin,self->s.origin,vec);
				if(vec[2] < 30)
				{
					vec[2] = 30;
				}

				VectorNormalize(vec);
				VectorMA(trace.ent->velocity,200,vec,trace.ent->velocity);
				trace.ent->groundentity = NULL;
			}
		}

		VectoAngles(trace.plane.normal,na);
		AngleVectors(na,NULL,nvr,NULL);

		if(DotProduct(nvr,vf)>0)
		{
			self->ideal_yaw=vectoyaw(nvr);
		}
		else
		{
			VectorScale(nvr, -1, nvr);
			self->ideal_yaw=vectoyaw(nvr);
		}

		VectorCopy(self->mins, mins);
		VectorCopy(self->s.origin, source);
		AngleVectors(self->s.angles, vf, NULL, NULL);
		VectorMA(source, dist, nvr, source);//was 4?

		//Account for STEPSIZE
		mins[2] += STEPSIZE;

		trace = gi.trace(self->s.origin, mins, self->maxs, source, self, MASK_SOLID);//was MASK_SHOT

		if (trace.fraction < 1||trace.allsolid||trace.startsolid)
		{
			VectorScale(nvr, -1, nvr);
			self->ideal_yaw=vectoyaw(nvr);
		}

		turnamt = Q_fabs(M_ChangeYaw(self));

		if (fabs(anglemod(self->s.angles[YAW] - self->ideal_yaw)) < self->yaw_speed)
		{
			yaw = self->s.angles[YAW];
			yaw = yaw*M_PI*2 / 360;

			move[0] = cos(yaw)*dist * 2;//4
			move[1] = sin(yaw)*dist * 2;//4
			move[2] = 0;

			ret = ai_trystep(self, move);

			if ((ret != TRYSTEP_OK) && (self->monsterinfo.idle_time < level.time))
			{
				self->ideal_yaw = -vectoyaw(nvr);
				turnamt = Q_fabs(M_ChangeYaw(self));
				distloss = turnamt/self->yaw_speed * 0.3;
				dist -= (dist * distloss);
				if (!M_walkmove(self, self->s.angles[YAW], dist))
				{
					self->monsterinfo.idle_time = level.time + flrand(0.5, 1.2);
					SV_NewChaseDir(self, self->enemy, dist);
				}
			}
			else
			{
				distloss = turnamt/self->yaw_speed * 0.3;
				dist -= (dist * distloss);
				M_walkmove(self, self->s.angles[YAW], dist);
			}
			//actually tried move
			return;
		}
	}
	else
		turnamt = Q_fabs(ai_face_goal(self));

	//greater the turn, lesser the dist, full yawspeed turn is 50% dist
	//FIXME: also make this dist less if turn is high and close to enemy?
	//(so you don't run around it in a circle)?
	distloss = turnamt/self->yaw_speed * 0.3;

	dist -= (dist * distloss);
	if (dist) //Going somewhere?
	{
		if(self->classID == CID_SSITHRA)
			ssithraCheckJump(self);
		if (!M_walkmove(self, self->s.angles[YAW], dist))
		{
			yaw = self->s.angles[YAW];
			yaw = yaw*M_PI*2 / 360;

			move[0] = cos(yaw)*dist;
			move[1] = sin(yaw)*dist;
			move[2] = 0;

			ret = ai_trystep(self, move);

			if ((ret != TRYSTEP_OK) && (self->monsterinfo.idle_time < level.time))
			{
				//Jump down?
				if ((self->enemy->s.origin[2] < self->s.origin[2])&&(self->classID != CID_SSITHRA))
				{
					if (ai_hopdown(self, self->enemy->s.origin, 1024))
					{
						if (self->monsterinfo.jump_time < level.time)
						{
							self->monsterinfo.jump_time = level.time + 1;

							return;
						}
						else return;
					}
					else
					{
						self->monsterinfo.idle_time = level.time + flrand(1.0, 2.0);
						SV_NewChaseDir(self, self->enemy, dist);
					}
				}
				else
				{
					self->monsterinfo.idle_time = level.time + flrand(0.5, 1.2);
					SV_NewChaseDir(self, self->enemy, dist);
				}
			}
			else
			{
				turnamt = Q_fabs(M_ChangeYaw(self));

				if (!M_walkmove(self, self->s.angles[YAW], dist/2)&&(self->classID != CID_SSITHRA))
				{
					if (ai_hopdown(self, self->enemy->s.origin, 1024))
					{
						if (self->monsterinfo.jump_time < level.time)
						{
							self->monsterinfo.jump_time = level.time + 1;

							return;
						}
						else return;
					}
				}
			}
		}
	}
}

/*
=============
ai_flee

The monster has an enemy it is trying to get away from
=============
*/
void
ai_flee(edict_t *self, float dist)
{
	vec3_t	vec;

	if (self->enemy)
	{
		VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
		self->ideal_yaw = vectoyaw(vec);
		self->ideal_yaw = anglemod(self->ideal_yaw + self->best_move_yaw);
		M_ChangeYaw(self);
		if(!M_walkmove(self, self->s.angles[YAW], dist) && EqualAngle(self->s.angles[YAW], self->ideal_yaw, 5))
			self->best_move_yaw = flrand(60, 300);
		else
			self->best_move_yaw = 180;

		/*
		VectorSubtract(self->s.origin, self->enemy->s.origin, vec);
		self->ideal_yaw = vectoyaw(vec);
		M_ChangeYaw(self);
		M_MoveAwayFromGoal (self, dist);*/
	}
}

/*
=============================================================
void extrapolateFiredir (edict_t *self,vec3_t p1,float pspeed,edict_t *targ,float accept,vec3_t vec2)

MG

Estimates where the "targ" will be by the time a projectile
travelling at "pspeed" leaving "org" arrives at "targ"'s origin.
It then calculates a new spot to shoot at so that the
projectile will arrive at such spot at the same time as
"targ".  Will return '0 0 0' (FALSE) if there is not a clear
line of fire to the spot or if the new vector is out of the
acceptable range (based on dot product of original vec and
the new vec).

PROPOSAL: Factor in skill->value? 0 = no leading, 4 = perfect leading, 1-3 innaccuracy levels for leading
=============================================================
*/
void
extrapolateFiredir(edict_t *self, vec3_t p1, float pspeed, edict_t *targ, float accept, vec3_t vec2)
{
	float dist1, dist2, tspeed, dot, eta1, eta2, eta_delta, tdist;
	qboolean failed = false;
	vec3_t p2, p3, targ_dir, vec1, tempv;
	trace_t trace;
	float offset;

	if(!targ)
	{
		VectorClear(vec2);
		return;
	}

	tdist = Vector2Length(targ->s.origin, self->s.origin);
	if(!skill->value)
	{//poor shot, take forward and screw it up
		AngleVectors(self->s.angles, tempv, NULL, NULL);
		VectorMA(p1, pspeed, tempv, p2);

		if(tdist < 128)
			offset = 48 * tdist/128;
		else
			offset = 48;

		p2[0] += flrand(-offset, offset);
		p2[1] += flrand(-offset, offset);
		p2[2] += flrand(-offset/2, offset * 0.666);
		VectorSubtract(p2, p1, vec2);
		VectorNormalize(vec2);
		return;
	}

	offset = 2 - skill->value;//skill >= 2 = perfect aim, skill 1 is very poor

	if(tdist < 128)
		offset *= tdist/128;

	if(offset<0)
		offset = 0;

	if(skill->value < 2.0 && !(self->monsterinfo.aiflags & AI_NIGHTVISION))
	{
		if(targ->client)
		{
			offset += targ->light_level/32;
		}
	}

	VectorCopy(targ->s.origin, p2);
	if(offset)
	{
		p2[0] += flrand(-offset*12, offset*12);
		p2[1] += flrand(-offset*12, offset*12);
		p2[2] += flrand(-offset*8, offset*8);
	}

	VectorSubtract(p2, p1, vec1);

	dist1 = VectorNormalize(vec1);

	VectorCopy(targ->velocity, targ_dir);

	tspeed = VectorNormalize(targ_dir);
	eta1 = dist1/pspeed;					//Estimated time of arrival of projectile to p2

	VectorMA(p2, tspeed*eta1, targ_dir, p3);
	VectorSubtract(p3, p1, tempv);

	dist2 = VectorNormalize(tempv);
	eta2 = dist2/pspeed;					//ETA of projectile to p3
	eta_delta = eta2-eta1;				//change in ETA's

	VectorMA(p3, tspeed*eta_delta*flrand(0, 1), targ_dir, p3);
//careful,  above version does not modify targ_dir

	trace = gi.trace(p1, vec3_origin, vec3_origin, p3, self, MASK_SOLID);
	if(trace.fraction<1.0)
		failed = true;

	VectorSubtract(p3, p1, vec2);
	VectorNormalize(vec2);

	dot  =  DotProduct(vec1, vec2);

	if(dot<accept)						//Change in dir too great
	{
		failed = true;
	}
	if(failed)
	{
		VectorClear(vec2);
	}
}

/*
ClearLastAlerts

takes last alerts away from any monster who's last alert was the "self" alert
*/
void
ClearLastAlerts(alertent_t *self)
{
	int	i;

	for(i = 0; i < game.maxentities; i++)
	{
		if(g_edicts[i].svflags & SVF_MONSTER)
		{
			if(g_edicts[i].last_alert == self)
			{
				g_edicts[i].last_alert = NULL;
			}
		}
	}
}

/*
alert_timed_out

clears out all the fields for an alert, removes it from the linked list and sets it's slot to being empty to make room for insertion of others later
*/
void
alert_timed_out(alertent_t *self)
{
	if (self==NULL)
		return;

	//take self out of alert chain
	if(self->prev_alert)
	{
		if(self->next_alert)
			self->prev_alert->next_alert = self->next_alert;
		else
		{//I'm the last one!
			level.last_alert = self->prev_alert;
			self->prev_alert->next_alert = NULL;
		}
	}
	else
	{//I'm the first one!
		if(self->next_alert)
			level.alert_entity = self->next_alert;
		else
			level.last_alert = level.alert_entity = NULL;
	}

	//Clear out all fields
	self->next_alert = NULL;
	self->prev_alert = NULL;
	self->enemy = NULL;
	VectorClear(self->origin);
	self->alert_svflags = 0;
	self->lifetime = 0;
	self->inuse = false;
	ClearLastAlerts(self);

	level.num_alert_ents--;
}

/*
GetFirstEmptyAlertInList

returns the first alert in the level.alertents list that isn;t in use
*/
alertent_t
*GetFirstEmptyAlertInList(void)
{
	int	i;
	//have max number of alerts, remove the first one
	if(level.num_alert_ents >= MAX_ALERT_ENTS)
		alert_timed_out(level.alert_entity);

	for(i = 0; i < MAX_ALERT_ENTS; i++)
	{
		if(!level.alertents[i].inuse)
		{
			return &level.alertents[i];
		}
	}
	return NULL;
}

/*
AlertMonsters

allots an alertent monsters will check during FindTarget to see if they should be alerted by it

self = used for sv_flags info and positioning of the alert entity
enemy = entity to make the monsters mad at if they're alerted
(float)lifetime = how many seconds the alert exists for
(qboolean)ignore_shadows = this alert gives away enemy's position, even if he is in shadows (I use this for staff hits on the floor and any other effect the player makes at his own location(like shooting a weapon), not for projectiles impacting)
*/
void
AlertMonsters(edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows)
{
	//FIXME: if  and stick new one at the end
	alertent_t	*alerter = level.alert_entity;
	alertent_t	*last_alert = NULL;

	if (deathmatch->value)		// Don't need this if no monsters...
		return;

	if(!lifetime)
		lifetime = 1.0;//stick around for 1 second

	//stick into the level's alerter chain
	if(alerter)
	{//go down the list and find an empty slot
		//fixme: just store the entnum?
		while(alerter->next_alert)
		{
			last_alert = alerter;
			alerter = alerter->next_alert;
		}
		level.last_alert = alerter = GetFirstEmptyAlertInList();
	}
	else//we're the first one!
		level.alert_entity = level.last_alert = alerter = GetFirstEmptyAlertInList();

	if(!alerter)
	{
		gi.dprintf("Error: out of alerts and can't find any empty slots!\n");
		return;
	}

	//I'm active, don't let my slot be used until I'm freed
	alerter->inuse = true;
	//point to the previous alerter, if any
	alerter->prev_alert = last_alert;
	//make the previous alerter point to me as the next one
	if(alerter->prev_alert)
		alerter->prev_alert->next_alert = alerter;
	//put me in the "self"'s spot
	VectorCopy(self->s.origin, alerter->origin);
	alerter->enemy = enemy;
	//should we keep track of owner in case they move to move the alert with them?  Only for monsters
	//alerter->owner = self;
	alerter->alert_svflags = self->svflags;
	if(ignore_shadows)//whatever happened would give away enemy's position, even in shadows
		alerter->alert_svflags |= SVF_ALERT_NO_SHADE;

	//stick around until after this point in time
	alerter->lifetime = level.time + lifetime;

	level.num_alert_ents++;
}

void
ai_spin(edict_t *self, float amount)
{
	self->s.angles[YAW] += amount;
}

qboolean
ai_have_enemy(edict_t *self)
{
	qboolean enemy_gone = false;

	if(!self->enemy)
		enemy_gone = true;
	else if(self->enemy->health <= 0)
		enemy_gone = true;

	if(!enemy_gone)
	{
		return true;
	}
	else
	{
		if(self->oldenemy)
		{
			if(self->oldenemy->health>0)
			{
				self->enemy=self->oldenemy;
				self->oldenemy = NULL;
				return true;
			}
		}
	}

	G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	return false;
}

qboolean
movable(edict_t *ent)
{
	if(	ent->movetype!=MOVETYPE_NONE &&
		ent->movetype!=MOVETYPE_PUSH &&
		ent->movetype!=MOVETYPE_PUSH)
	{
		return true;
	}

	return false;
}

#if 0
/*
 * The monster has an enemy
 * it is trying to kill
 */
void
ai_run(edict_t *self, float dist)
{
	vec3_t v;
	edict_t *tempgoal;
	edict_t *save;
	qboolean new;
	edict_t *marker;
	float d1, d2;
	trace_t tr;
	vec3_t v_forward, v_right;
	float left, center, right;
	vec3_t left_target, right_target;
	qboolean retval;
	qboolean alreadyMoved = false;
	qboolean gotcha = false;
	edict_t *realEnemy;

	if (!self)
	{
		return;
	}

	/* if we're going to a combat point, just proceed */
	if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
	{
		M_MoveToGoal(self, dist);
		return;
	}

	if (self->monsterinfo.aiflags & AI_DUCKED)
	{
		self->monsterinfo.aiflags &= ~AI_DUCKED;
	}

	if (self->maxs[2] != self->monsterinfo.base_height)
	{
		monster_duck_up(self);
	}

	/* if we're currently looking for a hint path */
	if (self->monsterinfo.aiflags & AI_HINT_PATH)
	{
		M_MoveToGoal(self, dist);

		if (!self->inuse)
		{
			return;
		}

		/* first off, make sure we're looking for
		   the player, not a noise he made */
		if (self->enemy)
		{
			if (self->enemy->inuse)
			{
				if (strcmp(self->enemy->classname, "player_noise") != 0)
				{
					realEnemy = self->enemy;
				}
				else if (self->enemy->owner)
				{
					realEnemy = self->enemy->owner;
				}
				else /* uh oh, can't figure out enemy, bail */
				{
					self->enemy = NULL;
					hintpath_stop(self);
					return;
				}
			}
			else
			{
				self->enemy = NULL;
				hintpath_stop(self);
				return;
			}
		}
		else
		{
			hintpath_stop(self);
			return;
		}

		if (visible(self, realEnemy))
		{
			gotcha = true;
		}
		else if (coop->value)
		{
			FindTarget(self);
		}

		/* if we see the player, stop following hintpaths. */
		if (gotcha)
		{
			/* disconnect from hintpaths and start looking normally for players. */
			hintpath_stop(self);
		}

		return;
	}

	if (self->monsterinfo.aiflags & AI_SOUND_TARGET)
	{
		/* Special case: Some projectiles like grenades or rockets are
		   classified as an enemy. When they explode they generate a
		   sound entity, triggering this code path. Since they're gone
		   after the explosion their entity pointer is NULL. Therefor
		   self->enemy is also NULL and we're crashing. Work around
		   this by predending that the enemy is still there, and move
		   to it. */
		if (self->enemy)
		{
			VectorSubtract(self->s.origin, self->enemy->s.origin, v);
		}

		if ((!self->enemy) || (VectorLength(v) < 64))
		{
			self->monsterinfo.aiflags |= (AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
			self->monsterinfo.stand(self);
			return;
		}

		M_MoveToGoal(self, dist);
		/* prevent double moves for sound_targets */
		alreadyMoved = true;

		if (!self->inuse)
		{
			return;
		}

		if (!FindTarget(self))
		{
			return;
		}
	}

	retval = ai_checkattack(self);

	/* don't strafe if we can't see our enemy */
	if ((!enemy_vis) && (self->monsterinfo.attack_state == AS_SLIDING))
	{
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}

	/* unless we're dodging (dodging out of view looks smart) */
	if (self->monsterinfo.aiflags & AI_DODGING)
	{
		self->monsterinfo.attack_state = AS_SLIDING;
	}

	if (self->monsterinfo.attack_state == AS_SLIDING)
	{
		/* protect against double moves */
		if (!alreadyMoved)
		{
			ai_run_slide(self, dist);
		}

		/* we're using attack_state as the return value out of
		   ai_run_slide to indicate whether or not the move
		   succeeded.  If the move succeeded, and we're still
		   sliding, we're done in here (since we've  had our
		   chance to shoot in ai_checkattack, and have moved).
		   if the move failed, our state is as_straight, and
		   it will be taken care of below */
		if ((!retval) && (self->monsterinfo.attack_state == AS_SLIDING))
		{
			return;
		}
	}
	else if (self->monsterinfo.aiflags & AI_CHARGING)
	{
		self->ideal_yaw = enemy_yaw;

		if (!(self->monsterinfo.aiflags & AI_MANUAL_STEERING))
		{
			M_ChangeYaw(self);
		}
	}

	if (retval)
	{
		if ((dist != 0) && (!alreadyMoved) &&
			(self->monsterinfo.attack_state == AS_STRAIGHT) &&
			(!(self->monsterinfo.aiflags & AI_STAND_GROUND)))
		{
			M_MoveToGoal(self, dist);
		}

		if ((self->enemy) && (self->enemy->inuse) && (enemy_vis))
		{
			self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
			VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);
			self->monsterinfo.trail_time = level.time;
			VectorCopy(self->enemy->s.origin, self->monsterinfo.blind_fire_target);
			self->monsterinfo.blind_fire_delay = 0;
		}

		return;
	}

	if ((self->enemy) && (self->enemy->inuse) && (enemy_vis))
	{
		/* check for alreadyMoved */
		if (!alreadyMoved)
		{
			M_MoveToGoal(self, dist);
		}

		if (!self->inuse)
		{
			return;
		}

		self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
		VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);
		self->monsterinfo.trail_time = level.time;
		VectorCopy(self->enemy->s.origin, self->monsterinfo.blind_fire_target);
		self->monsterinfo.blind_fire_delay = 0;

		return;
	}

	if ((self->monsterinfo.trail_time + 5) <= level.time)
	{
		/* and we haven't checked for valid hint paths in the last 10 seconds */
		if ((self->monsterinfo.last_hint_time + 10) <= level.time)
		{
			/* check for hint_paths. */
			self->monsterinfo.last_hint_time = level.time;

			if (monsterlost_checkhint(self))
			{
				return;
			}
		}
	}

	if ((self->monsterinfo.search_time) &&
		(level.time > (self->monsterinfo.search_time + 20)))
	{
		/* double move protection */
		if (!alreadyMoved)
		{
			M_MoveToGoal(self, dist);
		}

		self->monsterinfo.search_time = 0;
		return;
	}

	tempgoal = G_SpawnOptional();

	if (!tempgoal)
	{
		M_MoveToGoal(self, dist);
		return;
	}

	save = self->goalentity;
	self->goalentity = tempgoal;

	new = false;

	if (!(self->monsterinfo.aiflags & AI_LOST_SIGHT))
	{
		/* just lost sight of the player, decide where to go first */
		self->monsterinfo.aiflags |= (AI_LOST_SIGHT | AI_PURSUIT_LAST_SEEN);
		self->monsterinfo.aiflags &= ~(AI_PURSUE_NEXT | AI_PURSUE_TEMP);
		new = true;
	}

	if (self->monsterinfo.aiflags & AI_PURSUE_NEXT)
	{
		self->monsterinfo.aiflags &= ~AI_PURSUE_NEXT;

		/* give ourself more time since we got this far */
		self->monsterinfo.search_time = level.time + 5;

		if (self->monsterinfo.aiflags & AI_PURSUE_TEMP)
		{
			self->monsterinfo.aiflags &= ~AI_PURSUE_TEMP;
			marker = NULL;
			VectorCopy(self->monsterinfo.saved_goal,
					self->monsterinfo.last_sighting);
			new = true;
		}
		else if (self->monsterinfo.aiflags & AI_PURSUIT_LAST_SEEN)
		{
			self->monsterinfo.aiflags &= ~AI_PURSUIT_LAST_SEEN;
			marker = PlayerTrail_PickFirst(self);
		}
		else
		{
			marker = PlayerTrail_PickNext(self);
		}

		if (marker)
		{
			VectorCopy(marker->s.origin, self->monsterinfo.last_sighting);
			self->monsterinfo.trail_time = marker->timestamp;
			self->s.angles[YAW] = self->ideal_yaw = marker->s.angles[YAW];
			new = true;
		}
	}

	VectorSubtract(self->s.origin, self->monsterinfo.last_sighting, v);
	d1 = VectorLength(v);

	if (d1 <= dist)
	{
		self->monsterinfo.aiflags |= AI_PURSUE_NEXT;
		dist = d1;
	}

	VectorCopy(self->monsterinfo.last_sighting, self->goalentity->s.origin);

	if (new)
	{
		tr = gi.trace(self->s.origin, self->mins, self->maxs,
				self->monsterinfo.last_sighting, self,
				MASK_PLAYERSOLID);

		if (tr.fraction < 1)
		{
			VectorSubtract(self->goalentity->s.origin, self->s.origin, v);
			d1 = VectorLength(v);
			center = tr.fraction;
			d2 = d1 * ((center + 1) / 2);
			self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
			AngleVectors(self->s.angles, v_forward, v_right, NULL);

			VectorSet(v, d2, -16, 0);
			G_ProjectSource(self->s.origin, v, v_forward, v_right, left_target);
			tr = gi.trace(self->s.origin, self->mins, self->maxs, left_target,
					self, MASK_PLAYERSOLID);
			left = tr.fraction;

			VectorSet(v, d2, 16, 0);
			G_ProjectSource(self->s.origin, v, v_forward, v_right, right_target);
			tr = gi.trace(self->s.origin, self->mins, self->maxs, right_target,
					self, MASK_PLAYERSOLID);
			right = tr.fraction;

			center = (d1 * center) / d2;

			if ((left >= center) && (left > right))
			{
				if (left < 1)
				{
					VectorSet(v, d2 * left * 0.5, -16, 0);
					G_ProjectSource(self->s.origin, v, v_forward,
							v_right, left_target);
				}

				VectorCopy(self->monsterinfo.last_sighting,
						self->monsterinfo.saved_goal);
				self->monsterinfo.aiflags |= AI_PURSUE_TEMP;
				VectorCopy(left_target, self->goalentity->s.origin);
				VectorCopy(left_target, self->monsterinfo.last_sighting);
				VectorSubtract(self->goalentity->s.origin, self->s.origin, v);
				self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
			}
			else if ((right >= center) && (right > left))
			{
				if (right < 1)
				{
					VectorSet(v, d2 * right * 0.5, 16, 0);
					G_ProjectSource(self->s.origin, v, v_forward, v_right,
							right_target);
				}

				VectorCopy(self->monsterinfo.last_sighting,
						self->monsterinfo.saved_goal);
				self->monsterinfo.aiflags |= AI_PURSUE_TEMP;
				VectorCopy(right_target, self->goalentity->s.origin);
				VectorCopy(right_target, self->monsterinfo.last_sighting);
				VectorSubtract(self->goalentity->s.origin, self->s.origin, v);
				self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
			}
		}
	}

	M_MoveToGoal(self, dist);

	if (!self->inuse)
	{
		return;
	}

	G_FreeEdict(tempgoal);

	self->goalentity = save;
}
#endif

/*
qboolean ok_to_wake (edict_t *monster, qboolean gorgon_roar)

Can this monster be woken up by something other than direct line of sight to player
*/
qboolean
ok_to_wake(edict_t *monster, qboolean gorgon_roar, qboolean ignore_ambush)
{
	if(gorgon_roar)
	{
		if(monster->monsterinfo.c_mode)
			return false;
	}
	else if(monster->monsterinfo.aiflags & AI_EATING ||//eating or perching
		monster->targetname ||//a monster that's supposed to be triggered - problem, one a monster is used and woken up, won't respond to alerts like others...?
		monster->monsterinfo.c_mode ||//cinematic
		monster->spawnflags & MSF_ASLEEP ||//shouldn't happen, but just in case
		(monster->spawnflags & MSF_AMBUSH && !ignore_ambush))
		return false;

	return true;
}

/*
==============================

  MG AI
  NEW LOW-LEVEL MOVEMENT CODE

==============================
*/

void pitch_roll_for_slope (edict_t *forwhom, vec3_t *slope);
void BecomeDebris(edict_t *self);
qboolean MG_GoToRandomBuoy(edict_t *self);

qboolean TB_CheckBottom (edict_t *self);
qboolean TB_CheckJump (edict_t *self);

static vec3_t JUMP_MINS = {-8, -8, 0};
static vec3_t JUMP_MAXS = {8, 8, 4};

char *HitLocName [hl_Max] =
{
	"NonSpecific",//0
	"Head",//1
	"TorsoFront",
	"TorsoBack",
	"ArmUpperLeft",
	"ArmLowerLeft",
	"ArmUpperRight",
	"ArmLowerRight",
	"LegUpperLeft",
	"LegLowerLeft",
	"LegUpperRight",
	"LegLowerRight",
	"BipedPoints",
	"WingedPoints",
	"extra14",
	"extra15",
	"MeleeHit",//16
};

//============================================================================

/*
=============
ahead

returns 1 if the entity is in front (dot > 0.8) of self
=============
*/
qboolean ahead (edict_t *self, edict_t *other)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward, check_angles;

	if(Vec3NotZero(self->v_angle_ofs))
	{
		VectorAdd(self->v_angle_ofs,self->s.angles, check_angles);
	}
	else
	{
		VectorCopy(self->s.angles,check_angles);
	}

	AngleVectors(check_angles, forward, NULL, NULL);
	VectorSubtract (other->s.origin, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);

	if (dot > 0.8)
		return true;
	return false;
}

/*
=============
LOS

returns true if there is an unobstructed spot from point1 to point2
=============
*/
qboolean LOS (edict_t *self, vec3_t point1, vec3_t point2)
{
	trace_t	trace;

	trace = gi.trace(point1, vec3_origin, vec3_origin, point2, self, MASK_SOLID);

	if (trace.fraction == 1.0)
		return true;
	return false;
}

/*
=============
visible_pos

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
qboolean visible_pos (edict_t *self, vec3_t spot2)
{
	vec3_t	spot1;
	trace_t	trace;

	VectorCopy(self->s.origin, spot1);
	spot1[2] += self->viewheight;
	trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE);

	if (trace.fraction == 1.0)
		return true;
	return false;
}

/*
=============
MG_CheckBottom

Returns false if any 2 adjacent cornerpoints of the bottom of the
entity is off an edge that is not a staircase.
=============
*/
qboolean MG_CheckBottom (edict_t *ent)
{
	vec3_t	mins, maxs, start, stop;
	trace_t	trace;
	int		x, y, corner;
	float	stepsize;

	qboolean	corner_ok[4];
	qboolean	easy_ok[2][2];
	qboolean	realcheck = false;

//normal corner checking
	if(ent->classID == CID_TBEAST)
	{
		return TB_CheckBottom(ent);
//		VectorAdd (ent->s.origin, ent->mins, mins);
//		VectorAdd (ent->s.origin, ent->maxs, maxs);
//		stepsize = 54;//ent->size[2];//very lenient for stepping off stuff
	}
	else//lenient, max 16 corner checking
	{
		VectorCopy(ent->mins, mins);
		VectorCopy(ent->maxs, maxs);

		//some leniency is ok here, no?
		mins[0] *= 0.75;
		mins[1] *= 0.75;
		maxs[0] *= 0.75;
		maxs[1] *= 0.75;

		//keep corner checks within 16 of center
		if(mins[0] < -16)
			mins[0] = -16;
		if(mins[1] < -16)
			mins[1] = -16;
		if(maxs[0] > 16)
			maxs[0] = 16;
		if(maxs[1] > 16)
			maxs[1] = 16;

		if(ent->maxs[0] > maxs[0])
			stepsize = STEPSIZE + (ent->maxs[0] - maxs[0]);

		VectorAdd (ent->s.origin, mins, mins);
		VectorAdd (ent->s.origin, maxs, maxs);
	}

// if all of the points under the corners are solid world, don't bother
// with the tougher checks
// the corners must be within 16 of the midpoint

	start[2] = mins[2] - 1;
	corner = 0;
	for	(x=0 ; x<=1 ; x++)//0, 0; 0, 1; 1, 0; 1, 1;
	{
		for	(y=0 ; y<=1 ; y++)
		{
			start[0] = x ? maxs[0] : mins[0];
			start[1] = y ? maxs[1] : mins[1];
			if (gi.pointcontents (start) != CONTENTS_SOLID)
			{//only do realcheck if two adjecent corners off ledge
				switch(corner)
				{
				case 0:
					break;
				case 1:
				case 2:
					if(!corner_ok[0])
						realcheck = true;
					break;
				case 3:
					if(!corner_ok[2] || !corner_ok[1])
						realcheck = true;
					break;
				}
				easy_ok[x][y] = corner_ok[corner] = false;
			}
			else
			{
//				if(ent->classID == CID_TBEAST)
//				{
//					ent->groundentity = world;
//					return true;//super hack- let big guy go up slopes
//				}
				//check them all to make realcheck faster
				easy_ok[x][y] = corner_ok[corner] = true;
			}

			corner++;
		}
	}

	if(!realcheck)
		return true;		// we got out easy

//
// check it for real...
//
	start[2] = mins[2];//bottom
	stop[2] = start[2] - stepsize + 1;//2*STEPSIZE;//bottom - 36

// the corners must be within 16 of the midpoint
	corner = 0;
	for	(x=0 ; x<=1 ; x++)
	{
		for	(y=0 ; y<=1 ; y++)
		{
			if(!easy_ok[x][y])
			{//don't trace the ones that were ok in the easy check
				start[0] = stop[0] = x ? maxs[0] : mins[0];
				start[1] = stop[1] = y ? maxs[1] : mins[1];

				trace = gi.trace(start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID);

				if (trace.fraction >= 1.0)// || start[2] - trace.endpos[2] > STEPSIZE)
				{//this point is off too high of a step
					switch(corner)
					{
					case 0:
						break;
					case 1:
					case 2:
						if(!corner_ok[0])
							return false;
						break;
					case 3:
						if(!corner_ok[2] || !corner_ok[1])
							return false;
						break;
					}
					corner_ok[corner] = false;
				}
				else//only return false if two adjacent corners off a ledge
				{
					if(ent->classID == CID_TBEAST)
					{
//						ent->groundentity = trace.ent;
						return true;//super hack- let big guy go up slopes
					}
					corner_ok[corner] = true;
				}
			}
			else
				corner_ok[corner] = true;

			corner++;
		}
	}

	return true;
}

/*
=============
MG_MoveStep

Called by monster program code.
The move will be adjusted for slopes and stairs, but if the move isn't
possible, no move is done, false is returned, and
pr_global_struct->trace_normal is set to the normal of the blocking wall
=============
*/
//FIXME since we need to test end position contents here, can we avoid doing
//it again later in catagorize position?
trace_t
MG_MoveStep(edict_t *self, vec3_t move, qboolean relink, qboolean *trace_succeeded)
{//only relinks if move succeeds
	float		dz;
	vec3_t		save_org, test_org, end;
	trace_t		trace;
	int			i;
	float		stepsize;
	vec3_t		test;
	int			contents = 0, clipmask;
	qboolean	slip_under = false;

	*trace_succeeded = false;

	// try the move
	VectorCopy(self->s.origin, save_org);
	if(self->monsterinfo.scale)
	{//scale movement by monster's scale
	//scale here, not before since any function can call this
		VectorScale(move, self->monsterinfo.scale, move);
	}

	VectorAdd (self->s.origin, move, test_org);


//SWIM AND FLY MONSTERS
	// flying monsters don't step up
	if ( self->flags & (FL_SWIM | FL_FLY) )
	{
	// try one move with vertical motion, then one without
		for (i=0 ; i<2 ; i++)
		{
			VectorAdd (self->s.origin, move, test_org);
			if (i == 0 && self->enemy)
			{
				if (!self->goalentity)
					self->goalentity = self->enemy;
				dz = self->s.origin[2] - self->goalentity->s.origin[2];
				if (self->goalentity->client)
				{
					if (dz > 40)
						test_org[2] -= 8;
					if (!((self->flags & FL_SWIM) && (self->waterlevel < 2)))
						if (dz < 30)
							test_org[2] += 8;
				}
				else
				{
					if (dz > 8)
						test_org[2] -= 8;
					else if (dz > 0)
						test_org[2] -= dz;
					else if (dz < -8)
						test_org[2] += 8;
					else
						test_org[2] += dz;
				}
			}

			trace = gi.trace(self->s.origin, self->mins, self->maxs, test_org, self, MASK_MONSTERSOLID);

			// fly monsters don't enter water voluntarily
			if (self->flags & FL_FLY)
			{
				if (!self->waterlevel)
				{
					test[0] = trace.endpos[0];
					test[1] = trace.endpos[1];
					test[2] = trace.endpos[2] + self->mins[2] + 1;
					contents = gi.pointcontents(test);
					if (contents & MASK_WATER)
					{
						G_QPostMessage(self, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
						return trace;
					}
				}
			}

			// swim monsters don't exit water voluntarily
			if (self->flags & FL_SWIM)
			{
				if (self->waterlevel < 2)
				{
					test[0] = trace.endpos[0];
					test[1] = trace.endpos[1];
					test[2] = trace.endpos[2] + self->mins[2] + 1;
					contents = gi.pointcontents(test);
					if (!(contents & MASK_WATER))
					{
						G_QPostMessage(self, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
						return trace;
					}
				}
			}

			if (trace.fraction == 1)
			{
				VectorCopy(trace.endpos, self->s.origin);
				if (relink)
				{
					gi.linkentity(self);
					G_TouchTriggers (self);
				}
				return trace;//true
			}

			if (!self->enemy)
				break;
		}

		return trace;
	}
//WALK MONSTERS
// push down from a step height above the wished position
	clipmask = MASK_MONSTERSOLID;
	if (!(self->monsterinfo.aiflags & AI_NOSTEP))
	{
		if(self->classID == CID_TBEAST)
		{
			clipmask = MASK_SOLID;
			stepsize = STEPSIZE * 3;
		}
		else
			stepsize = STEPSIZE;
	}
	else
		stepsize = 1;

	test_org[2] += stepsize;
	VectorCopy(test_org, end);
	end[2] -= stepsize*2;

	trace = gi.trace(test_org, self->mins, self->maxs, end, self, clipmask);

	if (trace.allsolid)
	{//the step up/down is all solid in front
		G_QPostMessage(self, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
		return trace;
	}

	//NOTE: if did the forward trace above, CAN'T have startsolid, so this is ok
	if (trace.startsolid)
	{//can't step up, try down
		test_org[2] -= stepsize;
		trace = gi.trace(test_org, self->mins, self->maxs, end, self, clipmask);
		if (trace.allsolid || trace.startsolid)
		{
			if(trace.ent)
			{
				if(trace.ent->client)
				{
					slip_under = true;//lets rats walk between legs
				}
			}
			if(!slip_under)
			{
				G_QPostMessage(self, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
				return trace;
			}
		}
	}

	// don't go in to water unless only 40% hieght deep or an amphibian
	if (self->waterlevel == 0)
	{//not currently in water
		test[0] = trace.endpos[0];
		test[1] = trace.endpos[1];
		test[2] = trace.endpos[2] + self->mins[2];// + 1;
		test[2] += (self->maxs[2] - self->mins[2]) * 0.4;
		contents = gi.pointcontents(test);

		if (contents & MASK_WATER  && !(self->flags & FL_AMPHIBIAN))
			return trace;
	}

	if (trace.fraction == 1)
	{//too long of a step down
		if ( self->flags & FL_PARTIALGROUND ||
			self->svflags & SVF_FLOAT ||
			self->classID == CID_TBEAST ||
			(contents&MASK_WATER && self->flags & FL_AMPHIBIAN))//allow swimming monsters to step off ledges into water
		{// if monster had the ground pulled out, go ahead and fall
			//DO THE MOVE!
			VectorAdd (self->s.origin, move, self->s.origin);
			if (relink)
			{
				gi.linkentity(self);
				G_TouchTriggers (self);
			}
			self->groundentity = NULL;
//	SV_Printf ("fall down\n");
			*trace_succeeded = true;
			return trace;//true!
		}
		G_QPostMessage(self, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
		return trace;		// walked off an edge
	}

// check point traces down for dangling corners
	//DO THE MOVE!
	//ok, put me there
	VectorCopy(trace.endpos, self->s.origin);

	if(contents&MASK_WATER && self->flags & FL_AMPHIBIAN);
	else if (!MG_CheckBottom(self))// && self->classID!=CID_TBEAST)
	{//uh oh, not completely on solid ground
		if ( self->flags & FL_PARTIALGROUND || self->svflags & SVF_FLOAT)
		{	// entity had floor mostly pulled out from underneath it
			// and is trying to correct or can float
			if (relink)
			{
				gi.linkentity(self);
				G_TouchTriggers (self);
			}
			*trace_succeeded = true;
			return trace;//true!
		}
		//whoops, let's not make that move after all
		VectorCopy(save_org, self->s.origin);
		G_QPostMessage(self, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
		return trace;
	}

	//OK, we're on the ground completely now
	if ( self->flags & FL_PARTIALGROUND )
	{
		self->flags &= ~FL_PARTIALGROUND;
	}
	self->groundentity = trace.ent;
	self->groundentity_linkcount = trace.ent->linkcount;

// the move is ok
	if (relink)
	{
		gi.linkentity(self);
		G_TouchTriggers (self);
	}
	else
		VectorCopy(save_org, self->s.origin);

	*trace_succeeded = true;
	return trace;//true!
}

/*
===============
MG_ChangeYaw

===============
*/
float
MG_ChangeWhichYaw(edict_t *self, qboolean ideal_yaw)
{
	float	ideal;
	float	current;
	float	move;
	float	speed;

	current = anglemod(self->s.angles[YAW]);
	if(ideal_yaw)
		ideal = self->ideal_yaw;
	else
		ideal = self->best_move_yaw;

	if (current == ideal)
		return false;

	move = ideal - current;
	speed = self->yaw_speed;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}

	//normal anglemod doesn't have the precision I need to slide along walls
	self->s.angles[YAW] = anglemod(current + move);
	return move;
}

float MG_ChangeYaw (edict_t *self)
{
	return MG_ChangeWhichYaw(self, YAW_IDEAL);
}

qboolean MG_GetGoalPos (edict_t *self, vec3_t goalpos)
{
	qboolean	charge_enemy = false;

	if(self->monsterinfo.aiflags&AI_STRAIGHT_TO_ENEMY && self->enemy)
		charge_enemy = true;

	if (self->monsterinfo.searchType == SEARCH_BUOY && !charge_enemy)
	{
		if(self->buoy_index < 0 || self->buoy_index > level.active_buoys)
		{
			gi.dprintf("Error: SEARCH_BUOY but invalid index!!!\n");

			return false;
		}

		VectorCopy(level.buoy_list[self->buoy_index].origin, self->monsterinfo.nav_goal);
		VectorCopy(self->monsterinfo.nav_goal, goalpos);
	}
	else if(self->goalentity && !charge_enemy)
	{
		if(self->goalentity == self->enemy && self->ai_mood_flags & AI_MOOD_FLAG_PREDICT && !(self->spawnflags & MSF_FIXED))
		{//predict where he's goin
			M_PredictTargetPosition( self->enemy, self->enemy->velocity, 8, goalpos);
		}
		else
		{
			VectorCopy(self->goalentity->s.origin, goalpos);
		}
	}
	else if(self->enemy)
	{
		if (self->ai_mood_flags & AI_MOOD_FLAG_PREDICT && !(self->spawnflags & MSF_FIXED))
		{//predict where he's goin
			M_PredictTargetPosition( self->enemy, self->enemy->velocity, 8, goalpos);
		}
		else
		{
			VectorCopy(self->enemy->s.origin, goalpos);
		}
	}
	else
	{
		if(mgai_debug->value)
			gi.dprintf("No goal to face!\n");

		VectorClear(goalpos);
		return false;
	}

	return true;
}

float MG_FaceGoal (edict_t *self, qboolean doturn)
{
	vec3_t		vec, goalpos;

	if(MG_GetGoalPos(self, goalpos))
	{
		VectorSubtract(goalpos, self->s.origin, vec);
	}
	else
	{
		if(mgai_debug->value)
			gi.dprintf("No goal to face!\n");

		return false;
	}

	self->ideal_yaw = vectoyaw(vec);

	if(doturn)
		return MG_ChangeYaw(self);

	return 0;
}

/*
======================
MG_StepDirection

Turns to the movement direction, and walks the current distance if
facing it.

======================
*/
qboolean MG_StepDirection (edict_t *self, float yaw, float dist)
{
	vec3_t		move, forward, test_angles;

	//find vector offset (move to add to origin)
	test_angles[PITCH] = test_angles[ROLL] = 0;
	test_angles[YAW] = yaw;
	AngleVectors(test_angles, forward, NULL, NULL);
	VectorScale(forward, dist, move);

	//see if can move that way, but don't actually move
	qboolean trace_succeeded = false;
	MG_MoveStep (self, move, false, &trace_succeeded);

	if (trace_succeeded)
	{
		/* move was allowed */
		self->best_move_yaw = yaw;//new
		self->monsterinfo.idle_time = level.time + flrand(0.5, 1.25);
//		MG_ChangeWhichYaw (self, YAW_BEST_MOVE);//new
		return true;
	}
	return false;
}

/*
================
MG_NewDir

================
*/
#define	DI_NODIR	-1
void MG_NewDir (edict_t *self, float dist)
{
	float	deltax, deltay;
	float	d[3];
	float	test_ideal_yaw, old_yaw, turnaround;//, save_yaw;
	vec3_t	goal_org;

	//FIXME: how did we get here with no enemy
	if(!MG_GetGoalPos (self, goal_org))
		return;

	old_yaw = anglemod( (int)(self->ideal_yaw/45)*45 );
	turnaround = anglemod(old_yaw - 180);

	deltax = goal_org[0] - self->s.origin[0];
	deltay = goal_org[1] - self->s.origin[1];
	if (deltax>10)
		d[1]= 0;
	else if (deltax<-10)
		d[1]= 180;
	else
		d[1]= DI_NODIR;
	if (deltay<-10)
		d[2]= 270;
	else if (deltay>10)
		d[2]= 90;
	else
		d[2]= DI_NODIR;

// try direct route
	if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	{
		if (d[1] == 0)
			test_ideal_yaw = d[2] == 90 ? 45 : 315;
		else
			test_ideal_yaw = d[2] == 90 ? 135 : 215;

		if (test_ideal_yaw != turnaround && MG_StepDirection(self, test_ideal_yaw, dist))
			return;
	}

// try other directions
	if ( irand(0, 1) ||  fabs(deltay)>fabs(deltax))
	{
		test_ideal_yaw=d[1];
		d[1]=d[2];
		d[2]=test_ideal_yaw;
	}

	if (d[1]!=DI_NODIR && d[1]!=turnaround
	&& MG_StepDirection(self, d[1], dist))
			return;

	if (d[2]!=DI_NODIR && d[2]!=turnaround
	&& MG_StepDirection(self, d[2], dist))
			return;

/* there is no direct path to the player, so pick another direction */

	if (old_yaw!=DI_NODIR && MG_StepDirection(self, old_yaw, dist))
			return;

	if (irand(0, 1))	/*randomly determine direction of search*/
	{
		for (test_ideal_yaw=0 ; test_ideal_yaw<=315 ; test_ideal_yaw += 45)
			if (test_ideal_yaw!=turnaround && MG_StepDirection(self, test_ideal_yaw, dist) )
					return;
	}
	else
	{
		for (test_ideal_yaw=315 ; test_ideal_yaw >=0 ; test_ideal_yaw -= 45)
			if (test_ideal_yaw!=turnaround && MG_StepDirection(self, test_ideal_yaw, dist) )
					return;
	}

	if (turnaround != DI_NODIR && MG_StepDirection(self, turnaround, dist) )
			return;

	// can't move, restore yaw?

// if a bridge was pulled out from underneath a monster, it may not have
// a valid standing position at all

	if (!MG_CheckBottom (self))// && self->classID!=CID_TBEAST)
		SV_FixCheckBottom (self);
}

/*
=============
infront_pos

returns 1 if the spot is in front (in sight) of self
=============
*/
qboolean infront_pos (edict_t *self, vec3_t pos)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward, check_angles;

	if(Vec3NotZero(self->v_angle_ofs))
		VectorAdd(self->v_angle_ofs,self->s.angles,check_angles);
	else
		VectorCopy(self->s.angles,check_angles);

	AngleVectors(check_angles, forward, NULL, NULL);
	VectorSubtract (pos, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);

	if (dot > 0.3)
		return true;
	return false;
}

qboolean MG_ExtraCheckJump (edict_t *self)
{
	vec3_t		vf, source, source2, targ_org, targ_mins;
	vec3_t		maxs, mins, save_org;
	trace_t		trace;
	float		hgt_diff, jump_fdist;
	qboolean	check_down = false;
	qboolean	can_move = false;

	if(mgai_debug->value)
		gi.dprintf("Extra Check Jump\n");

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if(self->buoy_index < 0 || self->buoy_index > level.active_buoys)
			return false;

		VectorCopy(level.buoy_list[self->buoy_index].origin, targ_org);

		if (!(infront_pos(self, targ_org)))
			return false;

		if (targ_org[2] < self->s.origin[2] - 28)
		{
			check_down = true;
		}
		else
		{
			check_down = false;
		}
		VectorClear(targ_mins);
	}
	else
	{
		if(!self->goalentity)
			return false;

		if (!(infront(self, self->goalentity)))
			return false;

		if (self->goalentity->s.origin[2] < self->s.origin[2] - 28)
		{
			check_down = true;
		}
		else
		{
			check_down = false;
		}

		VectorCopy(self->goalentity->s.origin, targ_org);
		VectorCopy(self->goalentity->mins, targ_mins);
	}

	if (check_down)
	{//jumping down
		//Setup the trace
		int inwater;

		if(mgai_debug->value)
			gi.dprintf("check jump down\n");

		inwater = (gi.pointcontents(self->s.origin) & CONTENTS_WATER);

		if(mgai_debug->value)
			gi.dprintf("checking jump down: ");

		if(inwater)
		{
			if(mgai_debug->value)
				gi.dprintf("checkdown allsolid\n");

			return false;
		}

		VectorCopy(self->maxs, maxs);
		VectorCopy(self->s.origin, source);
		AngleVectors(self->s.angles, vf, NULL, NULL);
		jump_fdist = Vector2Length(targ_org, self->s.origin);
		if(jump_fdist > 128)
			jump_fdist = 128;

		VectorMA(source, 128, vf, source);

		maxs[2] += 16;
		trace = gi.trace(self->s.origin, self->mins, maxs, source, self, MASK_MONSTERSOLID);

		if (trace.fraction == 1)
		{//clear ahead and above
			VectorCopy(source, source2);

			source2[2] -= 1024;
			//trace down
			trace = gi.trace(source, self->mins, self->maxs, source2, self, MASK_ALL);

			if (trace.allsolid || trace.startsolid)
			{
				if(mgai_debug->value)
					gi.dprintf("checkdown allsolid\n");

				return false;
			}

			if (trace.fraction == 1)
			{
				if(mgai_debug->value)
					gi.dprintf("checkdown- too far\n");

				return false;
			}
			else
			{
				if (trace.contents != CONTENTS_SOLID && trace.ent != self->enemy)
				{//didn't hit ground
					return false;
				}
				else
				{
					if(mgai_debug->value)
						gi.dprintf("checkjump down->whichjump\n");

					VectorSubtract(trace.endpos, self->s.origin, source2);
					VectorNormalize(source2);
					self->ideal_yaw = vectoyaw(source2);

					VectorMA(self->velocity, 300, vf, self->velocity);
					self->velocity[2]+=150;

					if(classStatics[self->classID].msgReceivers[MSG_JUMP])
					{
						if(self->classID != CID_RAT && self->classID != CID_SSITHRA)
						{//save vel so can crouch first
							VectorCopy(self->velocity, self->movedir);
							VectorClear(self->velocity);
						}
						G_QPostMessage(self, MSG_JUMP, PRI_DIRECTIVE, NULL);
						self->nextthink = level.time + 0.01;
					}
					else
						self->nextthink = level.time + 0.3;

					self->monsterinfo.jump_time = level.time + 1;
					if(mgai_debug->value)
						gi.dprintf("Extra jump down\n");
				}
			}
		}
		else if(mgai_debug->value)
			gi.dprintf("checkdown: not clear infront\n");
	}
	else
	{
		VectorCopy(self->s.origin, save_org);
		can_move = M_walkmove (self, self->s.angles[YAW], 64);
		VectorCopy(save_org, self->s.origin);

		if(can_move)
			return false;
		else
		{//check to jump over something

			if(mgai_debug->value)
				gi.dprintf("check jump over\n");

			AngleVectors(self->s.angles, vf, NULL, NULL);
			VectorCopy(self->s.origin, source);
			VectorMA(source, 128, vf, source2);
			VectorCopy(self->mins, mins);
			mins[2]+=24;//can clear it
			trace = gi.trace(source, mins, self->maxs, source2, self, MASK_SOLID);

			if((!trace.allsolid&&!trace.startsolid&&trace.fraction==1.0) || trace.ent == self->enemy)
			{//Go for it!

				VectorMA(self->velocity, 500*trace.fraction, vf, self->velocity);
				self->velocity[2] += 225;

				if(classStatics[self->classID].msgReceivers[MSG_JUMP])
				{
					if(self->classID != CID_RAT && self->classID != CID_SSITHRA)
					{//save vel so can crouch first
						VectorCopy(self->velocity, self->movedir);
						VectorClear(self->velocity);
					}
					G_QPostMessage(self, MSG_JUMP, PRI_DIRECTIVE, NULL);
					self->nextthink = level.time + 0.01;
				}
				else
					self->nextthink = level.time + 0.3;
				if(mgai_debug->value)
					gi.dprintf("Extra jump over\n");

			}
			else
			{
				if(mgai_debug->value)
					gi.dprintf("check jump up\n");

				VectorCopy(self->maxs, maxs);
				VectorCopy(self->s.origin, source);

				hgt_diff = (targ_org[2] + targ_mins[2]) - (self->s.origin[2] + self->mins[2]) + 32;
				source[2] += hgt_diff;
				trace = gi.trace(self->s.origin, self->mins, self->maxs, source, self, MASK_MONSTERSOLID);

				if (trace.fraction == 1)
				{//clear above
					VectorCopy(source, source2);

					AngleVectors(self->s.angles, vf, NULL, NULL);
					VectorMA(source, 64, vf, source2);
					source2[2] -= 24;
					//trace forward and down a little
					trace = gi.trace(source, self->mins, self->maxs, source2, self, MASK_ALL);

					if (trace.allsolid || trace.startsolid)
						return false;

					if (trace.fraction < 0.1)
					{
						if(mgai_debug->value)
							gi.dprintf("Can't jump up, no ledge\n");

						return false;
					}
		//			{
		//				if (Q_stricmp(trace.ent->classname, "worldspawn"))
		//					return;
		//			}
					else
					{
						VectorSubtract(trace.endpos, self->s.origin, source2);
						source2[2] = 0;
						VectorNormalize(source2);
						self->ideal_yaw = vectoyaw(source2);

						VectorMA(self->s.origin, 64, source2, source);
						trace = gi.trace(self->s.origin, vec3_origin, vec3_origin, source, self, MASK_SOLID);

						VectorScale(source2, 480*trace.fraction, self->velocity);
						self->velocity[2] = hgt_diff*3 + 200;

						if(classStatics[self->classID].msgReceivers[MSG_JUMP])
						{
							if(self->classID != CID_RAT && self->classID != CID_SSITHRA)
							{//save vel so can crouch first
								VectorCopy(self->velocity, self->movedir);
								VectorClear(self->velocity);
							}
							G_QPostMessage(self, MSG_JUMP, PRI_DIRECTIVE, NULL);
							self->nextthink = level.time + 0.01;
						}
						else
							self->nextthink = level.time + 0.3;
						self->monsterinfo.jump_time = level.time + 1;
						if(mgai_debug->value)
							gi.dprintf("Extra jump up\n");

					}
				}
				else
				{
					if(mgai_debug->value)
						gi.dprintf("Can't jump up, blocked\n");

					return false;
				}

			}
		}

	}
	return true;
}

/*
==================================================================
MG_CheckJump()
Checks to see if the enemy is not at the same level as monster
or something is blocking the path of the monster.  If there is
a clear jump arc to the enemy and the monster will not land in
water or lava, the monster will attempt to jump the distance.
==================================================================
*/
qboolean MG_CheckJump (edict_t *self)
{
	vec3_t spot1, spot2, jumpdir, forward, right, targ_absmin;
	vec3_t up, cont_spot, vis_check_spot, end_spot, targ_org;
	float jump_height, sub_len;
	int	contents;
	qboolean ignore_height = false;
	qboolean jumpup = false;
	trace_t trace;

	if(irand(1,100) > self->jump_chance)//JumpChanceForClass[self->classID])
		return false;

	if(self->classID == CID_TBEAST)
		return TB_CheckJump(self);

	if(mgai_debug->value)
		gi.dprintf("Check Jump\n");

	//FIXME: Allow jump in/out of water if not too deep
	if(self->flags & FL_INWATER)// && !(self->flags&FL_AMPHIBIAN))?
	{
		if(mgai_debug->value)
			gi.dprintf("Can't jump- inwater\n");

		return false;
	}

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if(self->buoy_index < 0 || self->buoy_index > level.active_buoys)
			return false;

		VectorCopy(level.buoy_list[self->buoy_index].origin, targ_org);
		VectorCopy(targ_org, targ_absmin);

		if (!(infront_pos(self, targ_org)))
			return false;
	}
	else
	{
		if(!self->goalentity)
			return false;

		if (!(infront(self, self->goalentity)))
			return false;

		if(!self->goalentity->groundentity && self->classID != CID_GORGON)
		{
			if(mgai_debug->value)
				gi.dprintf("goalentity in air\n");

			return false;
		}

		VectorCopy(self->goalentity->s.origin, targ_org);
		VectorAdd(targ_org, self->goalentity->mins, targ_absmin);
	}

	AngleVectors(self->s.angles, forward, right, up);
	VectorSubtract(targ_org, self->s.origin, jumpdir);
	VectorNormalize(jumpdir);
	jumpdir[2] = 0;
	jump_height=DotProduct(jumpdir, forward);

	if(jump_height<0.3)
	{
		if(mgai_debug->value)
			gi.dprintf("jump direction more than 60 degrees off of forward\n");

		return false;
	}

    VectorCopy(self->s.origin, spot1);
    VectorCopy(targ_org, spot2);

	jump_height = 16;

	VectorMA(spot1, 24, forward, cont_spot);
	cont_spot[2] -= 10;
	if(!(gi.pointcontents(cont_spot)&CONTENTS_SOLID))
		ignore_height = true;

	sub_len = Vector2Length(spot1, spot2);
//	if(self->classname!="monster_mezzoman"&&!self->spiderType&&self->model!="models/yakman.mdl")
	if(sub_len > 256)
		ignore_height = false;

	VectorMA(spot1, self->size[0] * 2, forward, vis_check_spot);
	VectorMA(vis_check_spot, self->size[2] * 1.5, up, vis_check_spot);
//also check to make sure you can't walkmove forward
    if(self->monsterinfo.jump_time > level.time)            //Don't jump too many times in a row
	{
		if(mgai_debug->value)
			gi.dprintf("just jumped\n");

		return false;
	}
    else if(!ignore_height&&targ_absmin[2]+36>=self->absmin[2])//&&self->think!=SpiderJumpBegin&&self->classname!="monster_mezzoman"&&self->model!="models/yakman.mdl")
	{
		if(mgai_debug->value)
			gi.dprintf("not above goalentity, and not spider\n");

		return false;
	}
    else if(!self->groundentity)//flags&FL_ONGROUND)
	{
		if(mgai_debug->value)
			gi.dprintf("not on ground\n");

		return false;
	}
    else if(sub_len>777 && !ignore_height)
	{
		if(mgai_debug->value)
			gi.dprintf("too far away\n");

		return false;
	}
    else if(sub_len <= 100)//&&self->think!=SpiderMeleeBegin)
	{
		if(mgai_debug->value)
			gi.dprintf("too close & not spider\n");

		return false;
	}
	//sfs--sure, it's just a dotproduct, but the other checks are a little cheaper
    else if(!infront_pos(self, targ_org))
	{
		if(mgai_debug->value)
			gi.dprintf("goalentity not in front\n");

		return false;
	}
	//sfs--save the trace line for after the easy checks
    else if(!clear_visible_pos(self, targ_org)&&!LOS(self, vis_check_spot, spot2))
	{
		if(mgai_debug->value)
			gi.dprintf("can't see goalentity\n");

		return false;
	}

	//sfs--holding off on the point contents too
	contents = gi.pointcontents(spot2);
	if(!(self->monsterinfo.aiflags&AI_SWIM_OK)&&
		(contents&CONTENTS_WATER||contents&CONTENTS_SLIME||contents&CONTENTS_LAVA))
	{
		if(mgai_debug->value)
			gi.dprintf("goalentity in water or lava\n");

		return false;
	}

	VectorCopy(self->s.origin, spot1);
	//	spot1=self->s.origin;
	spot1[2] += self->maxs[2];
    //spot1_z=self->absmax_z;
	VectorCopy(spot1, spot2);
	//spot2=spot1;
	spot2[2] += 36;
	//spot2_z+=36;

	trace = gi.trace(spot1, self->mins, self->maxs, spot2, self, MASK_MONSTERSOLID);

	if(trace.fraction<1.0||trace.allsolid||trace.startsolid)
	{
		if(mgai_debug->value)
			gi.dprintf("not enough room above\n");

		return false;
	}

	if(!jumpup)		// This variable is used without being initialised
	{
//	        spot1+=normalize(v_forward)*((self->maxs_x+self->maxs_y)*0.5);
	    VectorMA(spot1, (self->maxs[0]+self->maxs[1])*0.5, jumpdir, spot1);
		//spot1+=jumpdir*((self->maxs_x+self->maxs_y)*0.5);
		VectorCopy(spot1, end_spot);
		end_spot[2] += 36;

		trace = gi.trace(self->s.origin, self->mins, self->maxs, end_spot, self, MASK_MONSTERSOLID);

	    if(trace.fraction<1.0||trace.allsolid||trace.startsolid)
		{
			if(mgai_debug->value)
				gi.dprintf("not enough room in front\n");

			return false;
		}
		VectorMA(spot1, 64, jumpdir, end_spot);
		end_spot[2] -= 500;
		trace = gi.trace(spot1, JUMP_MINS, JUMP_MAXS, end_spot, self, MASK_MONSTERSOLID);
//	        traceline(spot1,spot1+jumpdir*64 - '0 0 500',false,self);

		contents = gi.pointcontents(trace.endpos);
		if(contents&CONTENTS_WATER||contents&CONTENTS_SLIME||contents&CONTENTS_LAVA)
		{
			if(mgai_debug->value)
				gi.dprintf("won't jump in water\n");

			return false;
		}
	}

	MG_FaceGoal(self, true);
//FIXME: make them do whatever jump function they have if they have one
	self->monsterinfo.jump_time = level.time + 2;        //Only try to jump once every 7 seconds

	if (!self->rrs.scale[0] ||
		!self->rrs.scale[1] ||
		!self->rrs.scale[2])
	{
		VectorSet(self->rrs.scale, 1.0, 1.0, 1.0);
	}

	if(!jumpup)
	{
		VectorScale(jumpdir, jump_height*18 * AVG_VEC3T(self->rrs.scale), self->velocity);
		//self->velocity=jumpdir*jump_height*18*self->scale;//was 18
		self->velocity[2] = jump_height*14 * AVG_VEC3T(self->rrs.scale);//was 12
	}
	else
	{
		VectorScale(jumpdir, jump_height*14 * AVG_VEC3T(self->rrs.scale), self->velocity);
		//self->velocity=jumpdir*jump_height*14*self->scale;//was 10
		self->velocity[2] = jump_height*17 * AVG_VEC3T(self->rrs.scale);//was 14
	}
	//self->groundentity = NULL?
	if(classStatics[self->classID].msgReceivers[MSG_JUMP])
	{
		if(self->classID != CID_RAT && self->classID != CID_SSITHRA)
		{//save vel so can crouch first
			VectorCopy(self->velocity, self->movedir);
			VectorClear(self->velocity);
		}
		G_QPostMessage(self, MSG_JUMP, PRI_DIRECTIVE, NULL);
		self->nextthink = level.time + 0.01;
	}
	else
		self->nextthink = level.time + 0.3;

	if(mgai_debug->value)
		gi.dprintf("JUMP!!!\n");

	return true;
}

/*
===============
MG_WalkMove

  Tries to step forward dist, returns the trace
===============
*/
trace_t
MG_WalkMove (edict_t *self, float yaw, float dist, 	qboolean *trace_succeeded)
{
	vec3_t	move, endpos;
	trace_t trace;

	yaw = yaw*M_PI*2 / 360;

	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

	trace = MG_MoveStep(self, move, true, trace_succeeded);
	if (*trace_succeeded)
	{
		return trace;
	}
//FaileD? ok, so what's in front of us
	VectorAdd(self->s.origin, move, endpos);
	//up mins for stairs?
	trace = gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID);
	*trace_succeeded = false;
	return trace;
}

/*
===============
MG_BoolWalkMove

  Tries to step forward dist, returns true/false
===============
*/
qboolean MG_BoolWalkMove (edict_t *self, float yaw, float dist)
{
	vec3_t	move;

	yaw = yaw*M_PI*2 / 360;

	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

	qboolean trace_succeeded = false;
	MG_MoveStep(self, move, true, &trace_succeeded);
	if(trace_succeeded)
	{
		return true;
	}

	return false;
}

/*
===============
MG_TestMove

  Sees if it can step dist in yaw, but doesn't do the move
===============
*/
qboolean MG_TestMove (edict_t *self, float yaw, float dist)
{
	vec3_t	move;

	yaw = yaw*M_PI*2 / 360;

	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

	qboolean trace_succeeded = false;
	MG_MoveStep(self, move, false, &trace_succeeded);
	if(trace_succeeded)
		return true;

	return false;
}

void MG_CheckEvade (edict_t *self)
{//FIXME: only check my enemy?  See if he's fired (last_attack) recently?
	int hl;
	float ent_dist, proj_offset;
	vec3_t proj_dir, endpos, bad_dir;
	trace_t trace;
	edict_t	*ent = NULL;
	vec3_t	total_dist;
	float	eta;

	if(!skill->value)
		return;
	//else if(flrand(0, 3) > skill->value)
	//	return;

	while((ent = newfindradius(ent, self->s.origin, 500)))
	{
		if(ent->movetype == MOVETYPE_FLYMISSILE && ent->solid && ent->owner!=self)
		{
			if(mgai_debug->value)
				gi.dprintf("Monster checking evade from %s projectile\n", ent->classname);

			if(Vec3IsZero(ent->velocity))
			{
				if(mgai_debug->value)
					gi.dprintf("ERROR: NULL velocity on %s projectile!\n", ent->classname);

			}
			else
			{
				VectorCopy(ent->velocity, proj_dir);
				VectorNormalize(proj_dir);
				VectorMA(ent->s.origin, 600, proj_dir, endpos);

				trace = gi.trace(ent->s.origin, ent->mins, ent->maxs, endpos, ent, MASK_MONSTERSOLID);
				if(trace.ent == self)
				{//going to get hit!
					if(mgai_debug->value)
						gi.dprintf("Dodging projectile impact, going to hit %s\n", HitLocName[hl]);

					hl = T_GetHitLocation(self, ent, trace.endpos);
					VectorSubtract(trace.endpos, ent->s.origin, total_dist);
					eta = VectorLength(total_dist)/VectorLength(ent->velocity);
					G_QPostMessage(self, MSG_EVADE, PRI_DIRECTIVE, "eif", ent, hl, eta);
				}
				else if(!irand(0,2))
				{
					VectorSubtract(self->s.origin, ent->s.origin, bad_dir);
					ent_dist = VectorNormalize(bad_dir);
					proj_offset = DotProduct(bad_dir, proj_dir);
					if(mgai_debug->value)
						gi.dprintf("Proj dot prod: %f\n", proj_offset);

					if(proj_offset > ent_dist/600)//farther it is, smaller angle deviation allowed for evasion
					{//coming pretty close
						VectorMA(ent->s.origin, ent_dist, proj_dir, endpos);//extrapolate to close to me
						trace = gi.trace(endpos, ent->mins, ent->maxs, self->s.origin, ent, MASK_MONSTERSOLID);
						if(trace.ent == self)
						{
							if(mgai_debug->value)
								gi.dprintf("Dodging projectile close pass, going to hit %s\n", HitLocName[hl]);

							hl = T_GetHitLocation(self, ent, trace.endpos);
							VectorSubtract(trace.endpos, ent->s.origin, total_dist);
							eta = VectorLength(total_dist)/VectorLength(ent->velocity);
							G_QPostMessage(self, MSG_EVADE, PRI_DIRECTIVE, "eif", ent, hl, eta);
						}
					}
				}
			}
		}
	}
}

/*
=============
ai_run

The monster has an enemy it is trying to kill or the monster is fleeing
=============
*/
void old_ai_run (edict_t *self, float dist);

void
ai_run(edict_t *self, float dist)
{
	//if dumb fleeing or fleeing and can't use buoys...
	if((deactivate_buoys->value||!(self->monsterinfo.aiflags & AI_USING_BUOYS))
				&&
				(self->monsterinfo.aiflags & AI_COWARD ||
					(self->monsterinfo.aiflags&AI_FLEE
					&&
					self->monsterinfo.flee_finished >= level.time)
				)
			)
	{
		ai_flee(self, dist);
		return;
	}
	else if(self->ai_mood_flags & AI_MOOD_FLAG_DUMB_FLEE)
	{
		if(MG_GoToRandomBuoy(self))
			self->monsterinfo.searchType = SEARCH_BUOY;
		else
		{
			ai_flee(self, dist);
			return;
		}
	}

	if(!deactivate_buoys->value)
	{
		if(self->monsterinfo.aiflags & AI_USING_BUOYS)
		{
			if(!(self->monsterinfo.aiflags&AI_STRAIGHT_TO_ENEMY))
			{
				if(self->pathfind_nextthink<=level.time)
				{
					MG_BuoyNavigate(self);
					self->pathfind_nextthink = level.time + 0.1;//0.3;-maybe TOO optimized, trying every frame again, take out of generic mood think?
					//don't pathfind for the next 3 frames.
				}
			}
		}
	}

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{//just face enemy
		return;
	}

	if(self->goalentity == self->enemy)
	{
		//sfs--only do this visibility check when debugging (gets expensive at big distances)
		if(mgai_debug->value && !visible(self, self->enemy))
		{
			gi.dprintf("ERROR: goal is invis enemy!\n");
		}
	}

	if(dist)
		if(!MG_MoveToGoal (self, dist))
		{
			if(self->classID == CID_SSITHRA)
				ssithraCheckJump(self);
		}

	if(self->classID!=CID_ASSASSIN)//does his own checks
		if(classStatics[self->classID].msgReceivers[MSG_EVADE])
		{//check for if going to be hit and evade
			MG_CheckEvade(self);
		}
}

void mg_ai_charge (edict_t *self, float dist)
{
	vec3_t	v;

	if(!self->enemy)
	{
		gi.dprintf("ERROR: AI_CHARGE at a NULL enemy!\n");

		return;//send stand MSG?
	}

	VectorSubtract (self->enemy->s.origin, self->s.origin, v);

	self->ideal_yaw = vectoyaw(v);

	MG_ChangeYaw (self);

	if (dist)
	{
		qboolean trace_succeeded = false;
		MG_WalkMove (self, self->s.angles[YAW], dist, &trace_succeeded);
	}

	if(self->classID!=CID_ASSASSIN)//does his own checks
		if(classStatics[self->classID].msgReceivers[MSG_EVADE])
		{//check for if going to be hit and evade
			MG_CheckEvade(self);
		}
}

qboolean canmove (edict_t *self)
{
	if(	self->movetype!=MOVETYPE_NONE &&
		self->movetype!=MOVETYPE_PUSH)
		return true;
	return false;
}

void mg_remove_body(edict_t *self)
{
	vec3_t	origin;
	int		flag = 0;

	VectorCopy(self->s.origin, origin);
	origin[2] += (self->mins[2] + 8.0f);
	if(self->classID == CID_RAT)
		flag |= CEF_FLAG6;
	gi.CreateEffect(NULL, FX_CORPSE_REMOVE, flag, origin, "");
	G_SetToFree(self);
}

void body_phase_out (edict_t *self)
{
	int	interval = 30;

	if(self->s.color[3] > interval)
	{
		self->s.color[3] -= irand(interval/2, interval);
		self->post_think = body_phase_out;
		self->next_post_think = level.time + 0.05;
	}
	else
	{
		self->s.color[3] = 0;
		self->post_think = NULL;
		self->next_post_think = -1;

		G_SetToFree(self);
	}
}

static trace_t
MG_AirMove(edict_t *self, vec3_t goalpos, float dist, qboolean *trace_succeeded)
{
	trace_t		trace;
	vec3_t		endpos, movedir;

	VectorSubtract(goalpos, self->s.origin, movedir);
	VectorNormalize(movedir);

	VectorMA(self->s.origin, dist, movedir, endpos);

	trace = gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID);

	if(trace.allsolid || trace.startsolid || trace.fraction <= 0.01)
	{
		*trace_succeeded = false;
		return trace;
	}

	VectorCopy(trace.endpos, self->s.origin);

	gi.linkentity(self);

	*trace_succeeded = true;
	return trace;
}

static int FRONT = 0;
static int BACK	= 1;
static int RIGHT = 2;
static int LEFT	= 3;
static float MIN_DROP_DIST = 0.125f;

void MG_PostDeathThink (edict_t *self)
{
	float	mostdist;
	trace_t trace1, trace2, trace3, trace4, movetrace;
	vec3_t	org, endpos, startpos, forward, right;
	int		whichtrace = 0;
	float	cornerdist[4];
	qboolean	frontbackbothclear = false;
	qboolean	rightleftbothclear = false;

	self->post_think = body_phase_out;
	self->next_post_think = level.time + 10;

	if(!self->groundentity || Vec3NotZero(self->velocity))
	{
		if(mgai_debug->value)
			gi.dprintf("Falling!\n");

		if(self->groundentity&&self->friction == 1.0)//check avelocity?
			pitch_roll_for_slope(self, NULL);

		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 0.1;
		return;
	}

	cornerdist[FRONT] = cornerdist[BACK] = cornerdist[RIGHT] = cornerdist[LEFT] = 0.0f;

	mostdist = MIN_DROP_DIST;

	AngleVectors(self->s.angles, forward, right, NULL);
	VectorCopy(self->s.origin, org);
	org[2]+=self->mins[2];

	VectorMA(org, self->dead_size, forward, startpos);
	VectorCopy(startpos, endpos);
	endpos[2]-=128;
	trace1 = gi.trace(startpos, vec3_origin, vec3_origin, endpos, self, MASK_SOLID);
	if(!trace1.allsolid&&!trace1.startsolid)
	{
		cornerdist[FRONT] = trace1.fraction;
		if(trace1.fraction>mostdist)
		{
			mostdist = trace1.fraction;
			whichtrace = 1;
		}
	}

	VectorMA(org, -self->dead_size, forward, startpos);
	VectorCopy(startpos, endpos);
	endpos[2]-=128;
	trace2 = gi.trace(startpos, vec3_origin, vec3_origin, endpos, self, MASK_SOLID);
	if(!trace2.allsolid&&!trace2.startsolid)
	{
		cornerdist[BACK] = trace2.fraction;
		if(trace2.fraction>mostdist)
		{
			mostdist = trace2.fraction;
			whichtrace = 2;
		}
	}

	VectorMA(org, self->dead_size/2, right, startpos);
	VectorCopy(startpos, endpos);
	endpos[2]-=128;
	trace3 = gi.trace(startpos, vec3_origin, vec3_origin, endpos, self, MASK_SOLID);
	if(!trace3.allsolid&&!trace3.startsolid)
	{
		cornerdist[RIGHT] = trace3.fraction;
		if(trace3.fraction>mostdist)
		{
			mostdist = trace3.fraction;
			whichtrace = 3;
		}
	}

	VectorMA(org, -self->dead_size/2, right, startpos);
	VectorCopy(startpos, endpos);
	endpos[2]-=128;
	trace4 = gi.trace(startpos, vec3_origin, vec3_origin, endpos, self, MASK_SOLID);
	if(!trace4.allsolid&&!trace4.startsolid)
	{
		cornerdist[LEFT] = trace4.fraction;
		if(trace4.fraction>mostdist)
		{
			mostdist = trace4.fraction;
			whichtrace = 4;
		}
	}

	//OK!  Now if two opposite sides are hanging, use a third if any, else, do nothing
	if(cornerdist[FRONT] > MIN_DROP_DIST && cornerdist[BACK] > MIN_DROP_DIST)
		frontbackbothclear = true;

	if(cornerdist[RIGHT] > MIN_DROP_DIST && cornerdist[LEFT] > MIN_DROP_DIST)
		rightleftbothclear = true;

	if(frontbackbothclear && rightleftbothclear)
		return;

	if(frontbackbothclear)
	{
		if(cornerdist[RIGHT] > MIN_DROP_DIST)
			whichtrace = 3;
		else if(cornerdist[LEFT] > MIN_DROP_DIST)
			whichtrace = 4;
		else
			return;
	}

	if(rightleftbothclear)
	{
		if(cornerdist[FRONT] > MIN_DROP_DIST)
			whichtrace = 1;
		else if(cornerdist[BACK] > MIN_DROP_DIST)
			whichtrace = 2;
		else
			return;
	}

	switch(whichtrace)
	{//check for stuck
	case 1:
		VectorMA(self->s.origin, self->maxs[0], forward, endpos);
		movetrace = gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID);
		if(movetrace.allsolid||movetrace.startsolid||movetrace.fraction<1.0)
		{
			if(canmove(movetrace.ent))
				whichtrace = -1;
			else
				whichtrace = 0;
		}
		break;
	case 2:
		VectorMA(self->s.origin, -self->maxs[0], forward, endpos);
		movetrace = gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID);
		if(movetrace.allsolid||movetrace.startsolid||movetrace.fraction<1.0)
		{
			if(canmove(movetrace.ent))
				whichtrace = -1;
			else
				whichtrace = 0;
		}
		break;
	case 3:
		VectorMA(self->s.origin, self->maxs[0], right, endpos);
		movetrace = gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID);
		if(movetrace.allsolid||movetrace.startsolid||movetrace.fraction<1.0)
		{
			if(canmove(movetrace.ent))
				whichtrace = -1;
			else
				whichtrace = 0;
		}
		break;
	case 4:
		VectorMA(self->s.origin, -self->maxs[0], right, endpos);
		movetrace = gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID);
		if(movetrace.allsolid||movetrace.startsolid||movetrace.fraction<1.0)
		{
			if(canmove(movetrace.ent))
				whichtrace = -1;
			else
				whichtrace = 0;
		}
		break;
	}

	switch(whichtrace)
	{
	case 1:
		if(mgai_debug->value)
			gi.dprintf("Forward trace %f\n", trace1.fraction);

		VectorMA(self->velocity, 200, forward, self->velocity);
		if(trace1.fraction >= 0.9)
		{
//can't anymore, origin not in center of deathframe!
//			self->avelocity[PITCH] = -300;
			self->friction = 1.0;
		}
		else
		{
			pitch_roll_for_slope(self, &trace1.plane.normal);
			self->friction = trace1.plane.normal[2] * 0.1;
		}
		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 0.1;
		return;
		break;

	case 2:
		if(mgai_debug->value)
			gi.dprintf("back trace %f\n", trace2.fraction);

		VectorMA(self->velocity, -200, forward, self->velocity);
		if(trace2.fraction >= 0.9)
		{
//can't anymore, origin not in center of deathframe!
//			self->avelocity[PITCH] = 300;
			self->friction = 1.0;
		}
		else
		{
			pitch_roll_for_slope(self, &trace2.plane.normal);
			self->friction = trace2.plane.normal[2] * 0.1;
		}
		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 0.1;
		return;
		break;

	case 3:
		if(mgai_debug->value)
			gi.dprintf("Right trace %f\n", trace3.fraction);

		VectorMA(self->velocity, 200, right, self->velocity);
		if(trace3.fraction >= 0.9)
		{
//can't anymore, origin not in center of deathframe!
//			self->avelocity[ROLL] = -300;
			self->friction = 1.0;
		}
		else
		{
			pitch_roll_for_slope(self, &trace3.plane.normal);
			self->friction = trace3.plane.normal[2] * 0.1;
		}
		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 0.1;
		return;
		break;

	case 4:
		if(mgai_debug->value)
			gi.dprintf("Left trace %f\n", trace4.fraction);

		VectorMA(self->velocity, -200, right, self->velocity);
		if(trace4.fraction >= 0.9)
		{
//can't anymore, origin not in center of deathframe!
//			self->avelocity[ROLL] = 300;
			self->friction = 1.0;
		}
		else
		{
			pitch_roll_for_slope(self, &trace4.plane.normal);
			self->friction = trace4.plane.normal[2] * 0.1;
		}
		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 0.1;
		return;
		break;
	}
	//on solid ground
	if(whichtrace == -1)
	{
		if(mgai_debug->value)
			gi.dprintf("Deadmonster slide = stuck! (size is %4.2f)\n", self->dead_size);

		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 2;
		return;
	}
	else if(mgai_debug->value)
		gi.dprintf("Deadmonster slide = On ground (size was %4.2f)\n", self->dead_size);

	self->friction = 1.0;

	VectorClear(self->avelocity);
	pitch_roll_for_slope(self, NULL);

	if(!self->s.color[0])
		self->s.color[0] = 255;
	if(!self->s.color[1])
		self->s.color[1] = 255;
	if(!self->s.color[2])
		self->s.color[2] = 255;
	self->s.color[3] = 255;

	self->post_think = body_phase_out;
	if(self->classID == CID_RAT)
		self->next_post_think = level.time + flrand(3, 7);
	else
		self->next_post_think = level.time + flrand(10, 20);

	gi.linkentity(self);
}

void MG_CheckLanded (edict_t *self, float next_anim)
{
	vec3_t pos;

	if(mgai_debug->value)
		gi.dprintf("self->velocity %f %f %f\n", self->velocity[0], self->velocity[1], self->velocity[2]);


	if(self->groundentity)
		SetAnim(self, (int)next_anim);
	else if(self->velocity[2]<0)
	{
		VectorCopy(self->s.origin, pos);
		pos[2] += self->mins[2];
		VectorMA(pos, 0.5, self->velocity, pos);
		if(gi.pointcontents(pos)&CONTENTS_SOLID)
			SetAnim(self, (int)next_anim);
	}
}

void MG_InAirMove (edict_t *self, float fwdspd,float upspd,float rtspd)
{//simple addition of velocity, if on ground or not
	vec3_t up, forward, right;

	if(self->groundentity)//on ground
		return;

	AngleVectors(self->s.angles, forward, right, up);

	VectorMA(self->velocity, upspd, up, self->velocity);
	VectorMA(self->velocity, fwdspd, forward, self->velocity);
	VectorMA(self->velocity, rtspd, right, self->velocity);
}

void MG_ApplyJump (edict_t *self)
{
	self->jump_time = level.time + 0.5;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
	if(mgai_debug->value)
		gi.dprintf("Jump velocity will be: %4.2f %4.2f %4.2f\n", self->velocity[0], self->velocity[1], self->velocity[2]);

	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
}

void MG_NoBlocking (edict_t *self)
{
	self->svflags |= SVF_DEADMONSTER;	// now treat as a different content type
	self->msgHandler = DeadMsgHandler;	// no more messages at all
}

qboolean MG_GetTargOrg (edict_t *self, vec3_t targ_org)
{
	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if(self->buoy_index < 0 || self->buoy_index > level.active_buoys)
		{
			VectorClear(targ_org);
			return false;
		}
		VectorCopy(level.buoy_list[self->buoy_index].origin, self->monsterinfo.nav_goal);
		VectorCopy(self->monsterinfo.nav_goal, targ_org);
	}
	else
	{
		if(!self->goalentity)
		{
			VectorClear(targ_org);
			return false;
		}

		VectorCopy(self->goalentity->s.origin, targ_org);
	}
	return true;
}

/*
qboolean EqualAngle(float angle1, float angle2, float leniency)

Sees if the two angles are within leniency degrees of each other
*/
qboolean EqualAngle(float angle1, float angle2, float leniency)
{
	float diff;

	if(angle1 < -180)
		angle1 += 360;
	else if(angle1 > 180)
		angle1 -= 360;

	if(angle2 < -180)
		angle2 += 360;
	else if(angle2 > 180)
		angle2 -= 360;

	diff = angle1 - angle2;

	if(diff < -180)
		diff += 360;
	else if(diff > 180)
		diff -= 360;

	if(fabs(diff) > leniency)
		return false;

	return true;
}

/*
qboolean ok_to_break (edict_t *target)

  Ok to just smash this damn thing in my way?
*/
qboolean ok_to_break (edict_t *target)
{
	if(!target)
		return false;

	if(!target->takedamage)
		return false;

	if(target->health>MAX_BLOCKING_THING_HEALTH)//general damage for pots, barrels, etc.
		return false;

	if(target->targetname)//supposed to be triggered for some reason
		return false;

	if(target->svflags&SVF_MONSTER)//another monster
		return false;

	if(Vec3IsZero(target->s.origin))//breakable_brushes have no origin
		return false;

	return true;//break it!
}

qboolean MG_MoveToGoal (edict_t *self, float dist)
{
	trace_t trace;
	float turnamt, distloss, adj_dist, save_yaw, save_yaw_speed, WallDot;//, save_ideal_yaw;
	vec3_t	mins, source, goal_dir;
	qboolean	goal_vis=false, hitworld = false, new_best_yaw = false;
	float		stepsize, goal_dist, oby;

	if(self->classID == CID_TBEAST)
		stepsize = STEPSIZE * 3;
	else
		stepsize = STEPSIZE;

	if(!self->groundentity&&!(self->flags&FL_SWIM)&&!(self->flags&FL_FLY))
		return false;//in air!

	if(self->classID != CID_GORGON)//they do their own yawing
		MG_FaceGoal(self, false);//get ideal yaw, but don't turn

	//are we very close to our goal? problem: what if something in between?
	if(!EqualAngle(self->s.angles[YAW], self->ideal_yaw, self->yaw_speed))
	{//we aren't really facing our ideal yet
		if(self->monsterinfo.searchType == SEARCH_BUOY||self->ai_mood == AI_MOOD_NAVIGATE)
		{
			VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, goal_dir);
			goal_dist = VectorNormalize(goal_dir);
			if(goal_dist < (self->maxs[0] + 24 + dist))
			{//we're close to our goal
				MG_ChangeWhichYaw(self, YAW_IDEAL);
				return true;//so close to enemy, just turn, no movement - not if rat?
			}
		}
		else if(self->enemy)
		{
			VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, goal_dir);
			goal_dist = VectorNormalize(goal_dir);
			if(goal_dist < (self->maxs[0] + self->enemy->maxs[0] + dist*2))
			{//we're close to our goal
				MG_ChangeWhichYaw(self, YAW_IDEAL);
				return true;//so close to enemy, just turn, no movement - not if rat?
			}
		}
	}

	if(self->monsterinfo.idle_time == -1)
	{//have been told to just turn to ideal_yaw
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_IDEAL));
		//keep turning towards ideal until facing it
		if(turnamt < 1)
			self->monsterinfo.idle_time = 0;
		else
			return true;
	}
	else if(self->monsterinfo.idle_time > level.time)
	{//using best_move_yaw
		if(EqualAngle(self->s.angles[YAW], self->best_move_yaw, 5))
		{//do a test move in the direction I would like to go:
			if(MG_TestMove(self, self->ideal_yaw, dist))
			{//can move in that dir turn there for rest of this
				if(mgai_debug->value)
					gi.dprintf("Move in ideal tested true while using best_move...!\n");

				turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_IDEAL));
				//keep turning towards ideal until facing it
				if(turnamt < 1)
					self->monsterinfo.idle_time = 0;
				else
				{
					self->monsterinfo.idle_time = -1;
					return true;
				}
			}
		}
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));//turn to temp yaw
	}
	else
	{//using ideal_yaw
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_IDEAL));
	}

	distloss = turnamt/self->yaw_speed * 0.8;//0.3;
	adj_dist = dist - (dist * distloss);

	qboolean trace_succeeded = false;
	trace = MG_WalkMove(self, self->s.angles[YAW], dist, &trace_succeeded);
	if(trace_succeeded)
	{
		if(mgai_debug->value)
			gi.dprintf("Move forward succeeded!\n");

		return true;
	}
	else if(self->classID == CID_TBEAST)
	{
		if(trace.fraction<1.0)
			VectorCopy(trace.endpos, self->s.origin);
	}

//if facing best_move_yaw and can't move that way, stop trying in that dir now.
	if(self->monsterinfo.idle_time > level.time && self->s.angles[YAW] == self->best_move_yaw)
	{
		new_best_yaw = true;
		oby = self->best_move_yaw;
		self->monsterinfo.idle_time = 0;
	}

//bumped into something
	if(trace.ent)
	{
		if(!Q_stricmp(trace.ent->classname, "worldspawn"))
			hitworld = true;
		else
			hitworld = false;

		if(trace.ent == self->enemy)
		{//bumped into enemy, go get him!
			if(!(self->monsterinfo.aiflags & AI_COWARD) &&
				(!(self->monsterinfo.aiflags&AI_FLEE) || self->monsterinfo.flee_finished < level.time))
			{
				if(!(self->monsterinfo.aiflags&AI_NO_MELEE))
				{
					if(!(self->ai_mood_flags&AI_MOOD_FLAG_IGNORE_ENEMY))
					{
						if(classStatics[self->classID].msgReceivers[MSG_MELEE] && infront(self, self->enemy))
						{
							G_QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
							return true;
						}
					}
				}
			}
		}
		else if(trace.ent->svflags & SVF_MONSTER)
		{//if bumped into a monster that's not after an enemy but not ambushing, bring him along
			if(!trace.ent->enemy)
			{
				if(trace.ent->health>0 && !trace.ent->monsterinfo.awake)
				{
					if(!(trace.ent->spawnflags & MSF_AMBUSH))
					{
						if(self->enemy)
						{
							if(self->enemy->client)
							{
								trace.ent->enemy = self->enemy;
								FoundTarget(trace.ent, false);
							}
						}
					}
				}
			}
		}

/*
//FIXME: this needs to make sure they can break it
//also: do not do this if the monsters' enemy is visible (MASK_SOLID check, though)
		else if(!irand(0, 5))
		{//fixme: need to make sure the melee anims can break this
			if(self->classID > CID_RAT && classStatics[self->classID].msgReceivers[MSG_MELEE])
			{
				if(!(self->monsterinfo.aiflags&AI_NO_MELEE))
				{
					if(ok_to_break(trace.ent))
					{
						if(infront(self, trace.ent))
						{//smash it!
							if(mgai_debug->value)
								gi.dprintf("%s breaking blocking %s!\n", self->classname, trace.ent->classname);
							self->monsterinfo.aiflags |= AI_STRAIGHT_TO_ENEMY;//go straight at enemy, not buoys
							self->oldenemy_debounce_time = level.time + 7;//attack it for 7 seconds max
							self->oldenemy = self->enemy;//remember who I was after
							self->enemy = self->goalentity = trace.ent;//let's nail this sucker
							G_QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);//SMACK!
							return true;
						}
					}
				}
			}
		}
*/
		if(!hitworld)
		{
			if(self->monsterinfo.idle_time < level.time)
			{//not already following a weird dir

				if(mgai_debug->value)
					gi.dprintf("Move forward hit wall, newdir\n");

				self->monsterinfo.idle_time = level.time + flrand(0.5, 1.2);
				self->best_move_yaw = anglemod(180 + self->ideal_yaw);
				MG_NewDir(self, dist);
			}
			return false;
		}

		else if(mgai_debug->value)
			gi.dprintf("Bumped world - t_f: %f t_allsolid: %d, t_startsolid %d\n",trace.fraction, trace.allsolid, trace.startsolid);
	}

//Ledge?
	if((trace.fraction >= 0.5 + distloss ||self->classID == CID_ASSASSIN) && !trace.allsolid && !trace.startsolid)//a ledge
	{//why not tracefraction == 1.0?
		if(!(self->spawnflags & MSF_FIXED))
		{
			if(MG_CheckJump(self))//can jump off it
			{
				if(mgai_debug->value)
					gi.dprintf("Move forward jumped off ledge!\n");

				return true;
			}
			else if(self->classID == CID_ASSASSIN)
			{
				if(MG_ExtraCheckJump(self))
				{
					if(mgai_debug->value)
						gi.dprintf("Move straight to goal extra jumped off ledge!\n");

					return true;
				}
			}
		}

		if(trace.fraction >= 0.5)//even assassins skip this
		{

			if(mgai_debug->value)
				gi.dprintf("Can't jump off, getting newdir\n");

			if(self->monsterinfo.idle_time < level.time)
			{//not already following some other dir, pick one
				self->monsterinfo.idle_time = level.time + flrand(1, 2);
				self->best_move_yaw = anglemod(180 + self->ideal_yaw);
				MG_NewDir(self, dist);//what if this fails to set one?
			}
			return false;
		}
	}

	if(mgai_debug->value)
		if(trace.allsolid || trace.startsolid)
			gi.dprintf("Move forward allsolid or startsolid!\n");

	//FROM HERE ON, ONLY CHANGES DIR, WILL NOT MOVE!

	//otherwise, go around it... this ONLY???
	//lock into this new yaw for a bit
	if(self->monsterinfo.idle_time > level.time)
	{//heading somewhere for a few secs, turn here
		if(mgai_debug->value)
			gi.dprintf("Turning to newdir, not bumping\n");

		/*		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));
		distloss = turnamt/self->yaw_speed * 0.3;
		dist -= (dist * distloss);*/
		return false;
	}

	if((hitworld || irand(0,10)<6)&&!goal_vis)
		self->monsterinfo.idle_time = level.time + flrand(1, 2);
	else
		self->monsterinfo.idle_time = level.time + flrand(0.5, 1.25);

	self->best_move_yaw = anglemod(180 + self->ideal_yaw);

	//if hit a wall and close to ideal yaw (with 5), try a new dir
	if(Vec3NotZero(trace.plane.normal)&&
		EqualAngle(self->s.angles[YAW], self->ideal_yaw, 5))
	{//a wall?
		vec3_t	wall_angles, wall_right, self_forward, new_forward, vf;

		if(mgai_debug->value)
			gi.dprintf("Move forward hit wall, checking left/right/back...\n");


		//If facing a wall, turn faster, more facing the wall, faster the turn
		save_yaw_speed = self->yaw_speed;
		AngleVectors(self->s.angles, self_forward, NULL, NULL);
		WallDot = DotProduct(trace.plane.normal, self_forward);
		if(WallDot>0)
			WallDot = 0;//-1 to 0
		self->yaw_speed *= 1.25 - WallDot;//facing wall head-on = 2.25 times normal yaw speed

		VectoAngles(trace.plane.normal, wall_angles);
		AngleVectors(wall_angles, NULL, wall_right, NULL);

		if(goal_vis)
		{//can see goal, turn towards IT first
			VectorSubtract(self->goalentity->s.origin, self->s.origin, self_forward);
			VectorNormalize(self_forward);
		}

		//Get closest angle off that wall to move in
		if(DotProduct(wall_right,self_forward)>0)
		{
			if(mgai_debug->value)
				gi.dprintf("turning left\n");

			VectorCopy(wall_right, new_forward);
		}
		else
		{
			if(mgai_debug->value)
				gi.dprintf("turning right\n");

			VectorScale(wall_right, -1, new_forward);
		}

		if(irand(0,10)<3)//30% chance of trying other way first
			VectorScale(new_forward, -1, new_forward);

		self->best_move_yaw=vectoyaw(new_forward);

		if(new_best_yaw && self->best_move_yaw == oby)
		{
			VectorScale(new_forward, -1, new_forward);

			self->best_move_yaw=vectoyaw(new_forward);
		}

		//make sure we can move in chosen dir
		//set up mins and maxes for these moves
		VectorCopy(self->mins, mins);

		//Account for STEPSIZE
		mins[2] += stepsize;
		if(mins[2] >= self->maxs[2])
			mins[2] = self->maxs[2] - 1;
		//remember yaw in case all these fail!
		save_yaw = self->s.angles[YAW];

		//Haven't yawed yet, so this is okay
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));
		distloss = turnamt/self->yaw_speed * 0.8;//0.3;
		adj_dist = dist - (dist * distloss);

		VectorCopy(new_forward, vf);
		//AngleVectors(self->s.angles, vf, NULL, NULL);

		VectorCopy(self->s.origin, source);
		VectorMA(source, adj_dist, vf, source);

		trace = gi.trace(self->s.origin, mins, self->maxs, source, self, MASK_SOLID);//was MASK_SHOT

		if (trace.fraction < 1||trace.allsolid||trace.startsolid)
		{//Uh oh, try other way
			if(mgai_debug->value)
				gi.dprintf("turn other way\n");

			VectorScale(new_forward, -1, new_forward);
			self->best_move_yaw=vectoyaw(new_forward);
			//restore yaw
			self->s.angles[YAW] = save_yaw;
			//try new dir
			turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));
			distloss = turnamt/self->yaw_speed * 0.8;//0.3;
			adj_dist = dist - (dist * distloss);

			VectorCopy(new_forward, vf);
			//AngleVectors(self->s.angles, vf, NULL, NULL);

			VectorMA(source, adj_dist, vf, source);

			//Account for STEPSIZE
			mins[2] += stepsize;
			if(mins[2] >= self->maxs[2])
				mins[2] = self->maxs[2] - 1;

			trace = gi.trace(self->s.origin, mins, self->maxs, source, self, MASK_SOLID);//was MASK_SHOT
			if (trace.fraction < 1||trace.allsolid||trace.startsolid)
			{//Uh oh!  Go straight away from wall
				if(mgai_debug->value)
					gi.dprintf("turn all the way around\n");

				self->best_move_yaw=wall_angles[YAW];
				//restore yaw
				self->s.angles[YAW] = save_yaw;
				//start turning this move, but don't actually move until next time
				MG_ChangeWhichYaw(self, YAW_BEST_MOVE);
			}
		}
		self->yaw_speed = save_yaw_speed;
		return false;
	}
	else//keep turning to ideal
		self->monsterinfo.idle_time = 0;

	//Must have bumped into something very strange (other monster?)
	//just pick a new random dir

	if(mgai_debug->value)
		gi.dprintf("Don't know what I hit, choosing newdir for a second\n");

	MG_NewDir(self, dist);
	return false;
}

qboolean
MG_SwimFlyToGoal(edict_t *self, float dist)
{
	trace_t		trace;
	float		turnamt, distloss, adj_dist, save_yaw, save_yaw_speed, WallDot;//, save_ideal_yaw;
	vec3_t		mins, source, goal_dir, goalpos;
	qboolean	goal_vis=false, hitworld = false, new_best_yaw = false;
	float		goal_dist, oby;

	if(self->classID != CID_GORGON)//they do their own yawing
		MG_FaceGoal(self, false);//get ideal yaw, but don't turn

	//are we very close to our goal? problem: what if something in between?
	if(!EqualAngle(self->s.angles[YAW], self->ideal_yaw, self->yaw_speed))
	{//we aren't really facing our ideal yet
		if(self->monsterinfo.searchType == SEARCH_BUOY||self->ai_mood == AI_MOOD_NAVIGATE)
		{
			VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, goal_dir);
			goal_dist = VectorNormalize(goal_dir);
			if(goal_dist < (self->maxs[0] + 24 + dist))
			{//we're close to our goal
				MG_ChangeWhichYaw(self, YAW_IDEAL);
				return true;//so close to enemy, just turn, no movement - not if rat?
			}
		}
		else if(self->enemy)
		{
			VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, goal_dir);
			goal_dist = VectorNormalize(goal_dir);
			if(goal_dist < (self->maxs[0] + self->enemy->maxs[0] + dist*2))
			{//we're close to our goal
				MG_ChangeWhichYaw(self, YAW_IDEAL);
				return true;//so close to enemy, just turn, no movement - not if rat?
			}
		}
	}

	if(self->monsterinfo.idle_time == -1)
	{//have been told to just turn to ideal_yaw
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_IDEAL));
		//keep turning towards ideal until facing it
		if(turnamt < 1)
			self->monsterinfo.idle_time = 0;
		else
			return true;
	}
	else if(self->monsterinfo.idle_time > level.time)
	{//using best_move_yaw
		if(EqualAngle(self->s.angles[YAW], self->best_move_yaw, 5))
		{//do a test move in the direction I would like to go:
			if(MG_TestMove(self, self->ideal_yaw, dist))
			{//can move in that dir turn there for rest of this
				if(mgai_debug->value)
					gi.dprintf("Move in ideal tested true while using best_move...!\n");

				turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_IDEAL));
				//keep turning towards ideal until facing it
				if(turnamt < 1)
					self->monsterinfo.idle_time = 0;
				else
				{
					self->monsterinfo.idle_time = -1;
					return true;
				}
			}
		}
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));//turn to temp yaw
	}
	else
	{//using ideal_yaw
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_IDEAL));
	}

	distloss = turnamt/self->yaw_speed * 0.8;//0.3;
	adj_dist = dist - (dist * distloss);

	MG_GetGoalPos(self, goalpos);

	qboolean trace_succeeded = false;
	trace = MG_AirMove(self, goalpos, dist, &trace_succeeded);
	if (trace_succeeded)
	{
		if (mgai_debug->value)
		{
			gi.dprintf("Move forward succeeded!\n");
		}

		return true;
	}

//if facing best_move_yaw and can't move that way, stop trying in that dir now.
	if(self->monsterinfo.idle_time > level.time && self->s.angles[YAW] == self->best_move_yaw)
	{
		new_best_yaw = true;
		oby = self->best_move_yaw;
		self->monsterinfo.idle_time = 0;
	}

//bumped into something
	if(trace.ent)
	{
		if(!Q_stricmp(trace.ent->classname, "worldspawn"))
			hitworld = true;
		else
			hitworld = false;

		if(trace.ent == self->enemy)
		{//bumped into enemy, go get him!
			if(!(self->monsterinfo.aiflags & AI_COWARD) &&
				(!(self->monsterinfo.aiflags&AI_FLEE) || self->monsterinfo.flee_finished < level.time))
			{
				if(!(self->monsterinfo.aiflags&AI_NO_MELEE))
				{
					if(!(self->ai_mood_flags&AI_MOOD_FLAG_IGNORE_ENEMY))
					{
						if(classStatics[self->classID].msgReceivers[MSG_MELEE] && infront(self, self->enemy))
						{
							G_QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
							return true;
						}
					}
				}
			}
		}
		else if(trace.ent->svflags & SVF_MONSTER)
		{//if bumped into a monster that's not after an enemy but not ambushing, bring him along
			if(!trace.ent->enemy)
			{
				if(trace.ent->health>0 && !trace.ent->monsterinfo.awake)
				{
					if(!(trace.ent->spawnflags & MSF_AMBUSH))
					{
						if(self->enemy)
						{
							if(self->enemy->client)
							{
								trace.ent->enemy = self->enemy;
								FoundTarget(trace.ent, false);
							}
						}
					}
				}
			}
		}

		if(!hitworld)
		{
			if(self->monsterinfo.idle_time < level.time)
			{//not already following a weird dir
				if(mgai_debug->value)
					gi.dprintf("Move forward hit wall, newdir\n");

				self->monsterinfo.idle_time = level.time + flrand(0.5, 1.2);
				self->best_move_yaw = anglemod(180 + self->ideal_yaw);
				MG_NewDir(self, dist);
			}
			return false;
		}

		else if(mgai_debug->value)
			gi.dprintf("Bumped world - t_f: %f t_allsolid: %d, t_startsolid %d\n",trace.fraction, trace.allsolid, trace.startsolid);

	}

//Ledge?
/*	if(trace.fraction >= 0.5 + distloss && !trace.allsolid && !trace.startsolid)//a ledge
	{//why not tracefraction == 1.0?
		if(trace.fraction >= 0.5)//even assassins skip this
		{
			if(mgai_debug->value)
				gi.dprintf("Can't jump off, getting newdir\n");

			if(self->monsterinfo.idle_time < level.time)
			{//not already following some other dir, pick one
				self->monsterinfo.idle_time = level.time + flrand(1, 2);
				self->best_move_yaw = anglemod(180 + self->ideal_yaw);
				MG_NewDir(self, dist);//what if this fails to set one?
			}
			return false;
		}
	}*/

	if(mgai_debug->value)
		if(trace.allsolid || trace.startsolid)
			gi.dprintf("Move forward allsolid or startsolid!\n");

	//FROM HERE ON, ONLY CHANGES DIR, WILL NOT MOVE!

	//otherwise, go around it... this ONLY???
	//lock into this new yaw for a bit
	if(self->monsterinfo.idle_time > level.time)
	{//heading somewhere for a few secs, turn here
		if(mgai_debug->value)
			gi.dprintf("Turning to newdir, not bumping\n");

		/*		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));
		distloss = turnamt/self->yaw_speed * 0.3;
		dist -= (dist * distloss);*/
		return false;
	}

	if((hitworld || irand(0,10)<6)&&!goal_vis)
		self->monsterinfo.idle_time = level.time + flrand(1, 2);
	else
		self->monsterinfo.idle_time = level.time + flrand(0.5, 1.25);

	self->best_move_yaw = anglemod(180 + self->ideal_yaw);

	//if hit a wall and close to ideal yaw (with 5), try a new dir
	if(Vec3NotZero(trace.plane.normal)&&
		EqualAngle(self->s.angles[YAW], self->ideal_yaw, 5))
	{//a wall?
		vec3_t	wall_angles, wall_right, self_forward, new_forward, vf;

		if(mgai_debug->value)
			gi.dprintf("Move forward hit wall, checking left/right/back...\n");

		//If facing a wall, turn faster, more facing the wall, faster the turn
		save_yaw_speed = self->yaw_speed;
		AngleVectors(self->s.angles, self_forward, NULL, NULL);
		WallDot = DotProduct(trace.plane.normal, self_forward);
		if(WallDot>0)
			WallDot = 0;//-1 to 0
		self->yaw_speed *= 1.25 - WallDot;//facing wall head-on = 2.25 times normal yaw speed

		VectoAngles(trace.plane.normal, wall_angles);
		AngleVectors(wall_angles, NULL, wall_right, NULL);

		if(goal_vis)
		{//can see goal, turn towards IT first
			VectorSubtract(self->goalentity->s.origin, self->s.origin, self_forward);
			VectorNormalize(self_forward);
		}

		//Get closest angle off that wall to move in
		if(DotProduct(wall_right,self_forward)>0)
		{
			if(mgai_debug->value)
				gi.dprintf("turning left\n");

			VectorCopy(wall_right, new_forward);
		}
		else
		{
			if(mgai_debug->value)
				gi.dprintf("turning right\n");

			VectorScale(wall_right, -1, new_forward);
		}

		if(irand(0,10)<3)//30% chance of trying other way first
			VectorScale(new_forward, -1, new_forward);

		self->best_move_yaw=vectoyaw(new_forward);

		if(new_best_yaw && self->best_move_yaw == oby)
		{
			VectorScale(new_forward, -1, new_forward);

			self->best_move_yaw=vectoyaw(new_forward);
		}

		//make sure we can move in chosen dir
		//set up mins and maxes for these moves
		VectorCopy(self->mins, mins);

		//remember yaw in case all these fail!
		save_yaw = self->s.angles[YAW];

		//Haven't yawed yet, so this is okay
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));
		distloss = turnamt/self->yaw_speed * 0.8;//0.3;
		adj_dist = dist - (dist * distloss);

		VectorCopy(new_forward, vf);
		//AngleVectors(self->s.angles, vf, NULL, NULL);

		VectorCopy(self->s.origin, source);
		VectorMA(source, adj_dist, vf, source);

		trace = gi.trace(self->s.origin, mins, self->maxs, source, self, MASK_SOLID);//was MASK_SHOT

		if (trace.fraction < 1||trace.allsolid||trace.startsolid)
		{//Uh oh, try other way
			if(mgai_debug->value)
				gi.dprintf("turn other way\n");

			VectorScale(new_forward, -1, new_forward);
			self->best_move_yaw=vectoyaw(new_forward);
			//restore yaw
			self->s.angles[YAW] = save_yaw;
			//try new dir
			turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));
			distloss = turnamt/self->yaw_speed * 0.8;//0.3;
			adj_dist = dist - (dist * distloss);

			VectorCopy(new_forward, vf);
			//AngleVectors(self->s.angles, vf, NULL, NULL);

			VectorMA(source, adj_dist, vf, source);

			trace = gi.trace(self->s.origin, mins, self->maxs, source, self, MASK_SOLID);//was MASK_SHOT
			if (trace.fraction < 1||trace.allsolid||trace.startsolid)
			{//Uh oh!  Go straight away from wall
				if(mgai_debug->value)
					gi.dprintf("turn all the way around\n");

				self->best_move_yaw=wall_angles[YAW];
				//restore yaw
				self->s.angles[YAW] = save_yaw;
				//start turning this move, but don't actually move until next time
				MG_ChangeWhichYaw(self, YAW_BEST_MOVE);
			}
		}
		self->yaw_speed = save_yaw_speed;
		return false;
	}
	else//keep turning to ideal
		self->monsterinfo.idle_time = 0;

	//Must have bumped into something very strange (other monster?)
	//just pick a new random dir
	if(mgai_debug->value)
		gi.dprintf("Don't know what I hit, choosing newdir for a second\n");


	MG_NewDir(self, dist);
	return false;
}
