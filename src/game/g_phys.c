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
 * Quake IIs legendary physic engine.
 *
 * =======================================================================
 */

#include "header/local.h"
#include "common/message.h"
#include "header/g_physics.h"
#include "common/angles.h"
#include "common/singlylinkedlist.h"
#include "player/library/p_main.h"
#include "common/h2rand.h"
#include "player/library/p_anim_branch.h"
#include "player/library/p_anims.h"
#include "player/library/p_actions.h"
#include <stdint.h>

#define KNOCKBACK_SCALING 0.5
#define KNOCKBACK_THRESH .5

#define STOP_EPSILON 0.1
#define MAX_CLIP_PLANES 5
#define FRICTION 6
#define WATERFRICTION 1

static void SV_Physics_NewToss(edict_t *ent);

typedef struct
{
	edict_t *ent;
	vec3_t origin;
	vec3_t angles;
	float deltayaw;
} pushed_t;

static pushed_t pushed[MAX_EDICTS], *pushed_p;
static edict_t *obstacle;

/*
 * pushmove objects do not obey gravity, and do not interact with each other or
 * trigger fields, but block normal movement and push normal objects when they move.
 *
 * onground is set for toss objects when they come to a complete rest. it is set for
 * steping or walking objects
 *
 *  - doors, plats, etc are SOLID_BSP, and MOVETYPE_PUSH
 *  - bonus items are SOLID_TRIGGER touch, and MOVETYPE_TOSS
 *  - corpses are SOLID_NOT and MOVETYPE_TOSS
 *  - crates are SOLID_BBOX and MOVETYPE_TOSS
 *  - walking monsters are SOLID_SLIDEBOX and MOVETYPE_STEP
 *  - flying/floating monsters are SOLID_SLIDEBOX and MOVETYPE_FLY
 *  - solid_edge items only clip against bsp models.
 *
 */

static edict_t *
SV_TestEntityPosition(edict_t *ent)
{
	trace_t trace;
	int mask;

	if (!ent)
	{
		return NULL;
	}

	/* dead bodies are supposed to not be solid so lets
	   ensure they only collide with BSP during pushmoves
	*/
	if (ent->clipmask && !(ent->svflags & SVF_DEADMONSTER))
	{
		mask = ent->clipmask;
	}
	else
	{
		mask = MASK_SOLID;
	}

	trace = gi.trace(ent->s.origin, ent->mins, ent->maxs,
			ent->s.origin, ent, mask);

	if (trace.startsolid)
	{
		return g_edicts;
	}

	return NULL;
}

static void
SV_CheckVelocity(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	if (VectorLength(ent->velocity) > sv_maxvelocity->value)
	{
		VectorNormalize(ent->velocity);
		VectorScale(ent->velocity, sv_maxvelocity->value, ent->velocity);
	}
}

/*
 * Runs thinking code for
 * this frame if necessary
 */
static qboolean
SV_RunThink(edict_t *ent)
{
	float thinktime;

	if (!ent)
	{
		return false;
	}

	thinktime = ent->nextthink;

	assert(ent->inuse);

	if (ent->msgHandler)
	{
		G_ProcessMessages(ent);
	}

	if (thinktime <= 0)
	{
		return true;
	}

	if (thinktime > level.time + 0.001)
	{
		return true;
	}

	ent->nextthink = 0;

	if (!ent->think)
	{
		gi.error("NULL ent->think");
	}

	ent->think(ent);

	// NOTENOTE Is this what we want to return if it gets this far?
	return true;
}

/*
 * Two entities have touched, so
 * run their touch functions
 */
static void
SV_Impact(edict_t *e1, trace_t *trace)
{
	edict_t *e2;

	if (!e1 || !trace)
	{
		return;
	}

	e2 = trace->ent;

	if (e1->touch && (e1->solid != SOLID_NOT))
	{
		e1->touch(e1, e2, &trace->plane, trace->surface);
	}

	if (e2->touch && (e2->solid != SOLID_NOT))
	{
		e2->touch(e2, e1, NULL, NULL);
	}
}

/*
 * Slide off of the impacting object
 * returns the blocked flags:
 *  1 = floor
 *  2 = step / wall
 */
static int
ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float backoff;
	float change;
	int i, blocked;

	blocked = 0;

	if (normal[2] > 0)
	{
		blocked |= 1; /* floor */
	}

	if (!normal[2])
	{
		blocked |= 2; /* step */
	}

	backoff = DotProduct(in, normal) * overbounce;

	for (i = 0; i < 3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;

		if ((out[i] > -STOP_EPSILON) && (out[i] < STOP_EPSILON))
		{
			out[i] = 0;
		}
	}

	return blocked;
}

/*
 * The basic solid body movement clip
 * that slides along multiple planes
 * Returns the clipflags if the velocity
 * was modified (hit something solid)
 *
 * 1 = floor
 * 2 = wall / step
 * 4 = dead stop
 */
static int
SV_FlyMove(edict_t *ent, float time, int mask)
{
	edict_t *hit;
	int bumpcount, numbumps;
	vec3_t dir;
	float d;
	int numplanes;
	vec3_t planes[MAX_CLIP_PLANES];
	vec3_t primal_velocity, original_velocity, new_velocity;
	int i, j;
	trace_t trace;
	vec3_t end;
	float time_left;
	int blocked;

	if (!ent)
	{
		return 0;
	}

	numbumps = 4;

	blocked = 0;
	VectorCopy(ent->velocity, original_velocity);
	VectorCopy(ent->velocity, primal_velocity);
	numplanes = 0;

	time_left = time;

	ent->groundentity = NULL;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		for (i = 0; i < 3; i++)
		{
			end[i] = ent->s.origin[i] + time_left * ent->velocity[i];
		}

		trace = gi.trace(ent->s.origin, ent->mins, ent->maxs, end, ent, mask);

		if (trace.allsolid)
		{
			/* entity is trapped in another solid */
			VectorCopy(vec3_origin, ent->velocity);
			return 3;
		}

		if (trace.fraction > 0)
		{
			/* actually covered some distance */
			VectorCopy(trace.endpos, ent->s.origin);
			VectorCopy(ent->velocity, original_velocity);
			numplanes = 0;
		}

		if (trace.fraction == 1)
		{
			break; /* moved the entire distance */
		}

		hit = trace.ent;

		if (trace.plane.normal[2] > 0.7)
		{
			blocked |= 1; /* floor */

			if (hit->solid == SOLID_BSP)
			{
				ent->groundentity = hit;
				ent->groundentity_linkcount = hit->linkcount;
			}
		}

		if (!trace.plane.normal[2])
		{
			blocked |= 2; /* step */
		}

		/* run the impact function */
		SV_Impact(ent, &trace);

		if (!ent->inuse)
		{
			break; /* removed by the impact function */
		}

		time_left -= time_left * trace.fraction;

		/* cliped to another plane */
		if (numplanes >= MAX_CLIP_PLANES)
		{
			/* this shouldn't really happen */
			VectorCopy(vec3_origin, ent->velocity);
			return 3;
		}

		VectorCopy(trace.plane.normal, planes[numplanes]);
		numplanes++;

		/* modify original_velocity so it
		   parallels all of the clip planes */
		for (i = 0; i < numplanes; i++)
		{
			ClipVelocity(original_velocity, planes[i], new_velocity, 1);

			for (j = 0; j < numplanes; j++)
			{
				if ((j != i) && !VectorCompare(planes[i], planes[j]))
				{
					if (DotProduct(new_velocity, planes[j]) < 0)
					{
						break; /* not ok */
					}
				}
			}

			if (j == numplanes)
			{
				break;
			}
		}

		if (i != numplanes)
		{
			/* go along this plane */
			VectorCopy(new_velocity, ent->velocity);
		}
		else
		{
			/* go along the crease */
			if (numplanes != 2)
			{
				VectorCopy(vec3_origin, ent->velocity);
				return 7;
			}

			CrossProduct(planes[0], planes[1], dir);
			d = DotProduct(dir, ent->velocity);
			VectorScale(dir, d, ent->velocity);
		}

		/* if original velocity is against the original
		   velocity, stop dead to avoid tiny occilations
		   in sloping corners */
		if (DotProduct(ent->velocity, primal_velocity) <= 0)
		{
			VectorCopy(vec3_origin, ent->velocity);
			return blocked;
		}
	}

	return blocked;
}

void
SV_AddGravity(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	if (ent->gravityVector[2] > 0)
	{
		VectorMA(ent->velocity, ent->gravity * sv_gravity->value * FRAMETIME,
				ent->gravityVector, ent->velocity);
	}
	else
	{
		ent->velocity[2] -= ent->gravity * sv_gravity->value * FRAMETIME;
	}
}

/*
 * Returns the actual bounding box of a bmodel.
 * This is a big improvement over what q2 normally
 * does with rotating bmodels - q2 sets absmin,
 * absmax to a cube that will completely contain
 * the bmodel at *any* rotation on *any* axis, whether
 * the bmodel can actually rotate to that angle or not.
 * This leads to a lot of false block tests in SV_Push
 * if another bmodel is in the vicinity.
 */
static void
RealBoundingBox(edict_t *ent, vec3_t mins, vec3_t maxs)
{
	vec3_t forward, left, up, f1, l1, u1;
	vec3_t p[8];
	int i, j, k, j2, k4;

	if (!ent)
	{
		return;
	}

	for (k = 0; k < 2; k++)
	{
		k4 = k * 4;

		if (k)
		{
			p[k4][2] = ent->maxs[2];
		}
		else
		{
			p[k4][2] = ent->mins[2];
		}

		p[k4 + 1][2] = p[k4][2];
		p[k4 + 2][2] = p[k4][2];
		p[k4 + 3][2] = p[k4][2];

		for (j = 0; j < 2; j++)
		{
			j2 = j * 2;

			if (j)
			{
				p[j2 + k4][1] = ent->maxs[1];
			}
			else
			{
				p[j2 + k4][1] = ent->mins[1];
			}

			p[j2 + k4 + 1][1] = p[j2 + k4][1];

			for (i = 0; i < 2; i++)
			{
				if (i)
				{
					p[i + j2 + k4][0] = ent->maxs[0];
				}
				else
				{
					p[i + j2 + k4][0] = ent->mins[0];
				}
			}
		}
	}

	AngleVectors(ent->s.angles, forward, left, up);

	for (i = 0; i < 8; i++)
	{
		VectorScale(forward, p[i][0], f1);
		VectorScale(left, -p[i][1], l1);
		VectorScale(up, p[i][2], u1);
		VectorAdd(ent->s.origin, f1, p[i]);
		VectorAdd(p[i], l1, p[i]);
		VectorAdd(p[i], u1, p[i]);
	}

	VectorCopy(p[0], mins);
	VectorCopy(p[0], maxs);

	for (i = 1; i < 8; i++)
	{
		if (mins[0] > p[i][0])
		{
			mins[0] = p[i][0];
		}

		if (mins[1] > p[i][1])
		{
			mins[1] = p[i][1];
		}

		if (mins[2] > p[i][2])
		{
			mins[2] = p[i][2];
		}

		if (maxs[0] < p[i][0])
		{
			maxs[0] = p[i][0];
		}

		if (maxs[1] < p[i][1])
		{
			maxs[1] = p[i][1];
		}

		if (maxs[2] < p[i][2])
		{
			maxs[2] = p[i][2];
		}
	}
}

/* ================================================================== */

/* PUSHMOVE */

/*
 * Does not change the entities velocity at all
 */
static trace_t
SV_PushEntity(edict_t *ent, vec3_t push)
{
	trace_t trace;
	vec3_t start;
	vec3_t end;
	int mask;

	VectorCopy(ent->s.origin, start);
	VectorAdd(start, push, end);

retry:

	if (ent->clipmask)
	{
		mask = ent->clipmask;
	}
	else
	{
		mask = MASK_SOLID;
	}

	trace = gi.trace(start, ent->mins, ent->maxs, end, ent, mask);

	/* startsolid treats different-content volumes
	   as continuous, like the bbox of a monster/player
	   and the floor of an elevator. So do another trace
	   that only collides with BSP so that we make a best
	   effort to keep these entities inside non-solid space
	*/
	if (trace.startsolid && (mask & ~MASK_SOLID))
	{
		trace = gi.trace (start, ent->mins, ent->maxs, end, ent, MASK_SOLID);
	}

	VectorCopy(trace.endpos, ent->s.origin);

	if (ent->movetype == MOVETYPE_FLYMISSILE)
	{
		G_LinkMissile(ent);
	}
	else
	{
		gi.linkentity(ent);
	}

	/* Push slightly away from non-horizontal surfaces,
	   prevent origin stuck in the plane which causes
	   the entity to be rendered in full black. */
	if (trace.plane.type != 2)
	{
		/* Limit the fix to gibs, debris and dead monsters.
		   Everything else may break existing maps. Items
		   may slide to unreachable locations, monsters may
		   get stuck, etc. */
		if (((strncmp(ent->classname, "monster_", 8) == 0) && ent->health < 1) ||
				(strcmp(ent->classname, "debris") == 0) || (ent->s.effects & EF_GIB))
		{
			VectorAdd(ent->s.origin, trace.plane.normal, ent->s.origin);
		}
	}

	if (trace.fraction != 1.0)
	{
		SV_Impact(ent, &trace);

		/* if the pushed entity went away
		   and the pusher is still there */
		if (!trace.ent->inuse && ent->inuse)
		{
			/* move the pusher back and try again */
			VectorCopy(start, ent->s.origin);
			gi.linkentity(ent);
			goto retry;
		}
	}

	ent->gravity = 1.0;

	if (ent->inuse)
	{
		G_TouchTriggers(ent);
	}

	return trace;
}

/*
 * Objects need to be moved back on a failed push,
 * otherwise riders would continue to slide.
 */
static qboolean
SV_Push(edict_t *pusher, vec3_t move, vec3_t amove)
{
	int i, e;
	edict_t *check, *block;
	pushed_t *p;
	vec3_t org, org2, move2, forward, right, up;
	vec3_t realmins, realmaxs;

	if (!pusher)
	{
		return false;
	}

	/* clamp the move to 1/8 units, so the position will
	   be accurate for client side prediction */
	for (i = 0; i < 3; i++)
	{
		float temp;
		temp = move[i] * 8.0;

		if (temp > 0.0)
		{
			temp += 0.5;
		}
		else
		{
			temp -= 0.5;
		}

		move[i] = 0.125 * (int)temp;
	}

	/* we need this for pushing things later */
	VectorSubtract(vec3_origin, amove, org);
	AngleVectors(org, forward, right, up);

	/* save the pusher's original position */
	pushed_p->ent = pusher;
	VectorCopy(pusher->s.origin, pushed_p->origin);
	VectorCopy(pusher->s.angles, pushed_p->angles);

	if (pusher->client)
	{
		pushed_p->deltayaw = pusher->client->ps.pmove.delta_angles[YAW];
	}

	pushed_p++;

	/* move the pusher to it's final position */
	VectorAdd(pusher->s.origin, move, pusher->s.origin);
	VectorAdd(pusher->s.angles, amove, pusher->s.angles);
	gi.linkentity(pusher);

	/* Create a real bounding box for
	   rotating brush models. */
	RealBoundingBox(pusher, realmins, realmaxs);

	/* see if any solid entities
	   are inside the final position */
	check = g_edicts + 1;

	for (e = 1; e < globals.num_edicts; e++, check++)
	{
		if (!check->inuse)
		{
			continue;
		}

		if ((check->movetype == MOVETYPE_PUSH) ||
			(check->movetype == MOVETYPE_STOP) ||
//			(check->movetype == MOVETYPE_NOCLIP) ||
			(check->movetype == MOVETYPE_SCRIPT_ANGULAR))
		{
			continue;
		}

		if (!check->area.prev)
		{
			continue; /* not linked in anywhere */
		}

		/* if the entity is standing on the pusher,
		   it will definitely be moved */
		if (check->groundentity != pusher)
		{
			/* see if the ent needs to be tested */
			if ((check->absmin[0] >= realmaxs[0]) ||
				(check->absmin[1] >= realmaxs[1]) ||
				(check->absmin[2] >= realmaxs[2]) ||
				(check->absmax[0] <= realmins[0]) ||
				(check->absmax[1] <= realmins[1]) ||
				(check->absmax[2] <= realmins[2]))
			{
				continue;
			}

			/* see if the ent's bbox is inside
			   the pusher's final position */
			if (!SV_TestEntityPosition(check))
			{
				continue;
			}
		}

		if ((pusher->movetype == MOVETYPE_PUSH) ||
			(check->groundentity == pusher))
		{
			/* move this entity */
			pushed_p->ent = check;
			VectorCopy(check->s.origin, pushed_p->origin);
			VectorCopy(check->s.angles, pushed_p->angles);
			pushed_p++;

			/* try moving the contacted entity */
			VectorAdd(check->s.origin, move, check->s.origin);

			if (check->client)
			{
				check->client->ps.pmove.delta_angles[YAW] += amove[YAW];
			}

			/* figure movement due to the pusher's amove */
			VectorSubtract(check->s.origin, pusher->s.origin, org);
			org2[0] = DotProduct(org, forward);
			org2[1] = -DotProduct(org, right);

			/* Quirk for blocking Elevators when
			   running under amd64. This is most
			   likey  caused by a too high float
			   precision. -_-  */
			if (((pusher->s.number == 285) &&
				 (Q_strcasecmp(level.mapname, "xcompnd2") == 0)) ||
				((pusher->s.number == 520) &&
				 (Q_strcasecmp(level.mapname, "xsewer2") == 0)))
			{
				org2[2] = DotProduct(org, up) + 2;
			}
			else
			{
				org2[2] = DotProduct(org, up);
			}

			VectorSubtract(org2, org, move2);
			VectorAdd(check->s.origin, move2, check->s.origin);

			/* may have pushed them off an edge */
			if (check->groundentity != pusher)
			{
				check->groundentity = NULL;
			}

			block = SV_TestEntityPosition(check);

			if (!block)
			{
				/* pushed ok */
				gi.linkentity(check);

				/* impact? */
				continue;
			}

			/* if it is ok to leave in the old position, do it
			   this is only relevent for riding entities, not
			   pushed */
			VectorSubtract(check->s.origin, move, check->s.origin);
			block = SV_TestEntityPosition(check);

			if (!block)
			{
				pushed_p--;
				continue;
			}
		}

		/* save off the obstacle so we can
		   call the block function */
		obstacle = check;

		/* move back any entities we already moved
		   go backwards, so if the same entity was pushed
		   twice, it goes back to the original position */
		for (p = pushed_p - 1; p >= pushed; p--)
		{
			VectorCopy(p->origin, p->ent->s.origin);
			VectorCopy(p->angles, p->ent->s.angles);

			if (p->ent->client)
			{
				p->ent->client->ps.pmove.delta_angles[YAW] = p->deltayaw;
			}

			gi.linkentity(p->ent);
		}

		return false;
	}

	/* see if anything we moved has touched a trigger */
	for (p = pushed_p - 1; p >= pushed; p--)
	{
		G_TouchTriggers(p->ent);
	}

	return true;
}

/*
 * Bmodel objects don't interact with each
 * other, but push all box objects
 */
static void
SV_Physics_Pusher(edict_t *ent)
{
	vec3_t move, amove;
	edict_t *part, *mv;

	if (!ent)
	{
		return;
	}

	/* if not a team captain, so movement
	   will be handled elsewhere */
	if (ent->flags & FL_TEAMSLAVE)
	{
		return;
	}

	/* make sure all team slaves can move before commiting
	   any moves or calling any think functions if the move
	   is blocked, all moved objects will be backed out */
	pushed_p = pushed;

	for (part = ent; part; part = part->teamchain)
	{
		if (part->velocity[0] || part->velocity[1] || part->velocity[2] ||
			part->avelocity[0] || part->avelocity[1] || part->avelocity[2])
		{
			/* object is moving */
			VectorScale(part->velocity, FRAMETIME, move);
			VectorScale(part->avelocity, FRAMETIME, amove);

			if (!SV_Push(part, move, amove))
			{
				break; /* move was blocked */
			}
		}
	}

	if (pushed_p > &pushed[MAX_EDICTS - 1])
	{
		gi.error("pushed_p > &pushed[MAX_EDICTS - 1], memory corrupted");
	}

	if (part)
	{
		/* the move failed, bump all nextthink
		   times and back out moves */
		for (mv = ent; mv; mv = mv->teamchain)
		{
			if (mv->nextthink > 0)
			{
				mv->nextthink += FRAMETIME;
			}
		}

		/* if the pusher has a "blocked" function, call it
		   otherwise, just stay in place until the obstacle
		   is gone */
		if (part->blocked)
		{
			part->blocked(part, obstacle);
		}
	}
	else
	{
		/* the move succeeded, so call all think functions */
		for (part = ent; part; part = part->teamchain)
		{
			/* prevent entities that are on trains that have gone away from thinking! */
			if (part->inuse)
			{
				SV_RunThink(part);
			}
		}
	}
}

/* ================================================================== */

/*
 * Non moving objects can only think
 */
static void
SV_Physics_None(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	/* regular thinking */
	SV_RunThink(ent);
}

/*
 * A moving object that doesn't obey physics
 */
static void
SV_Physics_Noclip(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	/* regular thinking */
	if (!SV_RunThink(ent))
	{
		return;
	}

	VectorMA(ent->s.angles, FRAMETIME, ent->avelocity, ent->s.angles);
	VectorMA(ent->s.origin, FRAMETIME, ent->velocity, ent->s.origin);

	gi.linkentity(ent);
}

void PhysicsCheckWaterTransition(edict_t *self);

/* ================================================================== */

/* TOSS / BOUNCE */

/*
 * Toss, bounce, and fly movement.
 * When onground, do nothing.
 */
static void
SV_Physics_Toss(edict_t *ent)
{
	trace_t trace;
	vec3_t move;
	float backoff;
	edict_t *slave;
//	qboolean wasinwater;
//	qboolean isinwater;
	vec3_t old_origin;

	if (!ent)
	{
		return;
	}

	/* regular thinking */
	SV_RunThink(ent);

	/* entities are very often freed during thinking */
	if (!ent->inuse)
	{
		return;
	}

	/* if not a team captain, so movement
	   will be handled elsewhere */
	if (ent->flags & FL_TEAMSLAVE)
	{
		return;
	}

	if (ent->velocity[2] > 0)
	{
		ent->groundentity = NULL;
	}

	/* check for the groundentity going away */
	if (ent->groundentity)
	{
		if (!ent->groundentity->inuse)
		{
			ent->groundentity = NULL;
		}
	}

	/* if onground, return without moving */
	if (ent->groundentity && (ent->gravity > 0.0))
	{
		return;
	}

	VectorCopy(ent->s.origin, old_origin);

	SV_CheckVelocity(ent);

	/* add gravity */
	if ((ent->movetype != MOVETYPE_FLY) &&
		(ent->movetype != MOVETYPE_FLYMISSILE))
	{
		SV_AddGravity(ent);
	}

	/* move angles */
	VectorMA(ent->s.angles, FRAMETIME, ent->avelocity, ent->s.angles);

	/* move origin */
	VectorScale(ent->velocity, FRAMETIME, move);
	trace = SV_PushEntity(ent, move);

	if (!ent->inuse)
	{
		return;
	}

	if (trace.fraction < 1)
	{
		if (ent->movetype == MOVETYPE_WALLBOUNCE)
		{
			backoff = 2.0;
		}
		else if (ent->movetype == MOVETYPE_BOUNCE)
		{
			backoff = 1.5;
		}
		else
		{
			backoff = 1;
		}

		ClipVelocity(ent->velocity, trace.plane.normal, ent->velocity, backoff);

		if (ent->movetype == MOVETYPE_WALLBOUNCE)
		{
			vectoangles(ent->velocity, ent->s.angles);
		}

		/* stop if on ground */
		if ((trace.plane.normal[2] > 0.7) &&
			(ent->movetype != MOVETYPE_WALLBOUNCE))
		{
			if ((ent->velocity[2] < 60) || (ent->movetype != MOVETYPE_BOUNCE))
			{
				ent->groundentity = trace.ent;
				ent->groundentity_linkcount = trace.ent->linkcount;
				VectorCopy(vec3_origin, ent->velocity);
				VectorCopy(vec3_origin, ent->avelocity);
			}
		}
	}

	/* check for water transition */
	PhysicsCheckWaterTransition(ent);

#if 0
	wasinwater = (ent->watertype & MASK_WATER);
	ent->watertype = gi.pointcontents(ent->s.origin);
	isinwater = ent->watertype & MASK_WATER;

	if (isinwater)
	{
		ent->waterlevel = 1;
	}
	else
	{
		ent->waterlevel = 0;
	}

	/* Don't do the sounds for the camera */
	if (Q_stricmp(ent->classname,"chasecam"))
	{
		if (!wasinwater && isinwater)
		{
			/* don't play splash sound for entities already in water on level start */
			if (level.framenum > 3)
			{
				gi.positioned_sound(old_origin, g_edicts, CHAN_AUTO,
						gi.soundindex("misc/h2ohit1.wav"), 1, 1, 0);
			}
		}
		else if (wasinwater && !isinwater)
		{
			gi.positioned_sound(ent->s.origin, g_edicts, CHAN_AUTO,
					gi.soundindex("misc/h2ohit1.wav"), 1, 1, 0);
		}
	}
#endif

	/* move teamslaves */
	for (slave = ent->teamchain; slave; slave = slave->teamchain)
	{
		VectorCopy(ent->s.origin, slave->s.origin);
		gi.linkentity(slave);
	}
}

/* ================================================================== */

/* STEPPING MOVEMENT */

/*
 * Monsters freefall when they don't have a ground
 * entity, otherwise all movement is done with
 * discrete steps.
 *
 * This is also used for objects that have become
 * still on the ground, but  will fall if the floor
 * is pulled out from under them.
 */
static void
SV_AddRotationalFriction(edict_t *ent)
{
	int n;
	float adjustment;

	if (!ent)
	{
		return;
	}

	VectorMA(ent->s.angles, FRAMETIME, ent->avelocity, ent->s.angles);
	adjustment = FRAMETIME * sv_stopspeed->value * FRICTION;

	for (n = 0; n < 3; n++)
	{
		if (ent->avelocity[n] > 0)
		{
			ent->avelocity[n] -= adjustment;

			if (ent->avelocity[n] < 0)
			{
				ent->avelocity[n] = 0;
			}
		}
		else
		{
			ent->avelocity[n] += adjustment;

			if (ent->avelocity[n] > 0)
			{
				ent->avelocity[n] = 0;
			}
		}
	}
}

static void
SV_Physics_Step(edict_t *ent)
{
	qboolean wasonground;
	qboolean hitsound = false;
	float *vel;
	float speed, newspeed, control;
	float friction;
	edict_t *groundentity;
	int mask;
	vec3_t origvel;

	int i, threshhit=0;


	if (!ent)
	{
		return;
	}

	/* airborn monsters should always check for ground */
	if (!ent->groundentity)
	{
		M_CheckGround(ent);
	}

	groundentity = ent->groundentity;

	SV_CheckVelocity(ent);

	if (groundentity)
	{
		wasonground = true;
	}
	else
	{
		wasonground = false;
	}

	if (ent->avelocity[0] || ent->avelocity[1] || ent->avelocity[2])
	{
		SV_AddRotationalFriction(ent);
	}

	/* add gravity except:
	    - flying monsters
	    - swimming monsters who are in the water */
	if (!wasonground)
	{
		if (!(ent->flags & FL_FLY))
		{
			if (!((ent->flags & FL_SWIM) && (ent->waterlevel > 2)))
			{
				if (ent->velocity[2] < sv_gravity->value * -0.1)
				{
					hitsound = true;
				}

				if (ent->waterlevel == 0)
				{
					SV_AddGravity(ent);
				}
			}
		}
	}

	/* friction for flying monsters that have been given vertical velocity */
	if ((ent->flags & FL_FLY) && (ent->velocity[2] != 0))
	{
		speed = fabs(ent->velocity[2]);
		control = speed < sv_stopspeed->value ? sv_stopspeed->value : speed;
		friction = FRICTION / 3;
		newspeed = speed - (FRAMETIME * control * friction);

		if (newspeed < 0)
		{
			newspeed = 0;
		}

		newspeed /= speed;
		ent->velocity[2] *= newspeed;
	}

	/* friction for flying monsters that have been given vertical velocity */
	if ((ent->flags & FL_SWIM) && (ent->velocity[2] != 0))
	{
		speed = fabs(ent->velocity[2]);
		control = speed < sv_stopspeed->value ? sv_stopspeed->value : speed;
		newspeed = speed - (FRAMETIME * control * WATERFRICTION * ent->waterlevel);

		if (newspeed < 0)
		{
			newspeed = 0;
		}

		newspeed /= speed;
		ent->velocity[2] *= newspeed;
	}

	if (ent->velocity[2] || ent->velocity[1] || ent->velocity[0] || ent->knockbackvel[0] || ent->knockbackvel[1] || ent->knockbackvel[2])
	{
		/* apply friction: let dead monsters who
		   aren't completely onground slide */
		if ((wasonground) || (ent->flags & (FL_SWIM | FL_FLY)))
		{
			if (!((ent->health <= 0.0) && !M_CheckBottom(ent)))
			{
				vel = ent->velocity;
				speed = sqrt(vel[0] * vel[0] + vel[1] * vel[1]);

				if (speed)
				{
					friction = FRICTION;

					control = speed < sv_stopspeed->value ? sv_stopspeed->value : speed;
					newspeed = speed - FRAMETIME * control * friction;

					if (newspeed < 0)
					{
						newspeed = 0;
					}

					newspeed /= speed;

					vel[0] *= newspeed;
					vel[1] *= newspeed;
				}
			}
		}

		if (ent->svflags & SVF_MONSTER)
		{
			mask = MASK_MONSTERSOLID;
		}
		else
		{
			mask = MASK_SOLID;
		}

		// Try to add velocity
		VectorAdd(ent->velocity, ent->knockbackvel, ent->velocity);
		VectorCopy(ent->velocity, origvel);
		// Returns value if velocity was clipped
		SV_FlyMove(ent, FRAMETIME, mask);

		for (i = 0; i < 3; i++)
		{
			if (ent->velocity[i] != origvel[i])
			{
				ent->knockbackvel[i] = 0;
			}
			else
			{
				ent->velocity[i] -= ent->knockbackvel[i];
				ent->knockbackvel[i] *= KNOCKBACK_SCALING;
				if (Q_fabs(ent->knockbackvel[i]) < KNOCKBACK_THRESH)
					threshhit++;
			}
		}
		if (threshhit >= 3)
			VectorClear(ent->knockbackvel);



		gi.linkentity(ent);
		ent->gravity = 1.0;
		G_TouchTriggers(ent);

		if (!ent->inuse)
		{
			return;
		}

		if (ent->groundentity)
		{
			if (!wasonground)
			{
				if (hitsound)
				{
					gi.sound(ent, 0, gi.soundindex("world/land.wav"), 1, 1, 0);
				}
			}
		}
	}

	if (!ent->inuse) /* g_touchtrigger free problem */
	{
		return;
	}

	/* regular thinking */
	SV_RunThink(ent);
}

void
SV_Physics_Script_Angular(edict_t *ent)
{
	vec3_t Forward;

	if (ent->owner)
	{
		AngleVectors(ent->owner->s.angles, NULL, NULL, Forward);

		Forward[0] = -Forward[0];
		Forward[1] = -Forward[1];
		Forward[2] = -Forward[2];

		VectorMA(ent->moveinfo.start_origin, ent->moveinfo.distance, Forward, ent->s.origin);

		gi.linkentity(ent);
	}

	/* regular thinking */
	SV_RunThink(ent);
}

/* ================================================================== */

void
G_RunEntity(edict_t *ent)
{
	trace_t trace;
	vec3_t previous_origin;
	qboolean saved_origin;

	if (!ent)
	{
		return;
	}

	if (ent->movetype == MOVETYPE_STEP)
	{
		VectorCopy(ent->s.origin, previous_origin);
		saved_origin = true;
	}
	else
	{
		saved_origin = false;
	}

	if (ent->prethink)
	{
		ent->prethink(ent);
	}

	switch ((int)ent->movetype)
	{
		//JABot[start]
		case MOVETYPE_WALK:
			SV_RunThink(ent);
			break;
		//[end]
		case MOVETYPE_PUSH:
		case MOVETYPE_STOP:
			SV_Physics_Pusher(ent);
			break;
		case MOVETYPE_NONE:
			SV_Physics_None(ent);
			break;
		case MOVETYPE_NOCLIP:
			SV_Physics_Noclip(ent);
			break;
		case MOVETYPE_STEP:
			SV_Physics_Step(ent);
			break;
		case MOVETYPE_TOSS:
		case MOVETYPE_BOUNCE:
		case MOVETYPE_FLY:
		case MOVETYPE_FLYMISSILE:
		case MOVETYPE_WALLBOUNCE:
			SV_Physics_Toss(ent);
			break;
		case MOVETYPE_NEWTOSS:
			SV_Physics_NewToss(ent);
			break;
		case MOVETYPE_SCRIPT_ANGULAR:
			SV_Physics_Script_Angular(ent);
			break;
		default:
			gi.error("SV_Physics: bad movetype %i", (int)ent->movetype);
	}

	/* if we moved, check and fix origin if needed */
	/* also check inuse since entities are very often freed while thinking */
	if (saved_origin && ent->inuse && !VectorCompare(ent->s.origin, previous_origin))
	{
		trace = gi.trace(ent->s.origin, ent->mins, ent->maxs,
				previous_origin, ent, MASK_MONSTERSOLID);

		if (trace.allsolid || trace.startsolid)
		{
			VectorCopy(previous_origin, ent->s.origin);
		}
	}
}

/*
 * Toss, bounce, and fly movement. When on ground and
 * no velocity, do nothing. With velocity, slide.
 */
static void
SV_Physics_NewToss(edict_t *ent)
{
	trace_t trace;
	vec3_t move;
	edict_t *slave;
	qboolean wasinwater;
	qboolean isinwater;
	float speed, newspeed;
	vec3_t old_origin;

	if (!ent)
	{
		return;
	}

	/* regular thinking */
	SV_RunThink(ent);

	/* if not a team captain, so movement will be handled elsewhere */
	if (ent->flags & FL_TEAMSLAVE)
	{
		return;
	}

	/* find out what we're sitting on. */
	VectorCopy(ent->s.origin, move);
	move[2] -= 0.25;
	trace = gi.trace(ent->s.origin, ent->mins, ent->maxs,
			move, ent, ent->clipmask);

	if (ent->groundentity && ent->groundentity->inuse)
	{
		ent->groundentity = trace.ent;
	}
	else
	{
		ent->groundentity = NULL;
	}

	/* if we're sitting on something flat and have no velocity of our own, return. */
	if (ent->groundentity && (trace.plane.normal[2] == 1.0) &&
		!ent->velocity[0] && !ent->velocity[1] && !ent->velocity[2])
	{
		return;
	}

	/* store the old origin */
	VectorCopy(ent->s.origin, old_origin);

	SV_CheckVelocity(ent);

	/* add gravity */
	SV_AddGravity(ent);

	if (ent->avelocity[0] || ent->avelocity[1] || ent->avelocity[2])
	{
		SV_AddRotationalFriction(ent);
	}

	/* add friction */
	speed = VectorLength(ent->velocity);

	if (ent->waterlevel) /* friction for water movement */
	{
		newspeed = speed - (WATERFRICTION * 6 * ent->waterlevel);

		if (newspeed < 0)
		{
			newspeed = 0;
		}

		newspeed /= speed;
		VectorScale(ent->velocity, newspeed, ent->velocity);
	}
	else if (!ent->groundentity) /* friction for air movement */
	{
		newspeed = speed - ((FRICTION));

		if (newspeed < 0)
		{
			newspeed = 0;
		}

		newspeed /= speed;
		VectorScale(ent->velocity, newspeed, ent->velocity);
	}
	else /* use ground friction */
	{
		newspeed = speed - (FRICTION * 6);

		if (newspeed < 0)
		{
			newspeed = 0;
		}

		newspeed /= speed;
		VectorScale(ent->velocity, newspeed, ent->velocity);
	}

	SV_FlyMove(ent, FRAMETIME, ent->clipmask);
	gi.linkentity(ent);

	G_TouchTriggers(ent);

	/* check for water transition */
	wasinwater = (ent->watertype & MASK_WATER);
	ent->watertype = gi.pointcontents(ent->s.origin);
	isinwater = ent->watertype & MASK_WATER;

	if (isinwater)
	{
		ent->waterlevel = 1;
	}
	else
	{
		ent->waterlevel = 0;
	}

	if (!wasinwater && isinwater)
	{
		gi.positioned_sound(old_origin, g_edicts, CHAN_AUTO, gi.soundindex("misc/h2ohit1.wav"), 1, 1, 0);
	}
	else if (wasinwater && !isinwater)
	{
		gi.positioned_sound(ent->s.origin, g_edicts, CHAN_AUTO, gi.soundindex("misc/h2ohit1.wav"), 1, 1, 0);
	}

	/* move teamslaves */
	for (slave = ent->teamchain; slave; slave = slave->teamchain)
	{
		VectorCopy(ent->s.origin, slave->s.origin);
		gi.linkentity(slave);
	}
}

static void Physics_None(edict_t *self);
static void Physics_Static(edict_t *self);
static void Physics_NoclipMove(edict_t *self);
static void Physics_FlyMove(edict_t *self);
static void Physics_StepMove(edict_t *self);
static void Physics_Push(edict_t *self);
static void Physics_ScriptAngular(edict_t *self);

void PhysicsCheckWaterTransition(edict_t *self)
{//fixme: a high detail option?  Or just not in netplay?- maybe a flag for client to take care of
	//disabling for now since it might cause too much net traffic
	qboolean wasinwater, isinwater;
	trace_t	trace;
	int	size;

	if (deathmatch->value || coop->value)
		return;

	// check for water transition
	wasinwater = (self->watertype & MASK_WATER);
	self->watertype = gi.pointcontents (self->s.origin);
	isinwater = self->watertype & MASK_WATER;

	if (!wasinwater && isinwater)
	{
		trace = gi.trace(self->s.old_origin, vec3_origin, vec3_origin, self->s.origin, self, MASK_WATER);
	}
	else if (wasinwater && !isinwater)
	{
		trace = gi.trace(self->s.origin, vec3_origin, vec3_origin, self->s.old_origin, self, MASK_WATER);
	}
	else
		return;

	if (trace.fraction==1.0)
		return;

//fixme: just put a flag on them and do the effect on the other side?
	size = ceil(VectorLength(self->size) + VectorLength(self->velocity)/10);
	if (size<10)
		size = 10;
	else if(size>255)
		size = 255;

	gi.CreateEffect(NULL,
					FX_WATER_ENTRYSPLASH,
					CEF_FLAG6 | CEF_FLAG7,
					trace.endpos,
					"bd",
					size,
					trace.plane.normal);
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
static void Physics_None(edict_t *self)
{
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
static void Physics_Static(edict_t *self)
{
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
static void Physics_NoclipMove(edict_t *self)
{
	VectorMA(self->s.angles, FRAMETIME, self->avelocity, self->s.angles);
	VectorMA(self->s.origin, FRAMETIME, self->velocity, self->s.origin);

	gi.linkentity(self);
}

// TODO: Rewrite
static qboolean
BoundVelocity(float *vel)
{
	int count, i;

	count = 0;
	i = 3;
	do
	{
		double tmp;

		tmp = *vel;
		if (tmp <= -0.1 || tmp >= 0.1)
		{
			if (tmp <= 2000.0)
			{
				if (tmp < -2000.0)
					*vel = 0;
			}
			else
			{
				*vel = 2000.0;
			}
		}
		else
		{
			*vel = 0;
			++count;
		}
		++vel;
		--i;
	} while (i);
	return count != 3;
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
static void Physics_FlyMove(edict_t *self)
{
	FormMove_t		formMove;

	if (self->physicsFlags & PF_RESIZE)
	{
		if (G_ResizeBoundingForm(self, &formMove))
		{
			self->physicsFlags &= ~PF_RESIZE;
		}
		else
		{
			return;	// if an ent can't be resized, then it probably can't be moved either
		}
	}

	// update angles
	VectorMA(self->s.angles, FRAMETIME, self->avelocity, self->s.angles);

	if (!BoundVelocity(self->velocity) && self->gravity <= 0.0f)
	{
		return;
	}

	if (self->velocity[2] > 0.0f)
	{
		self->groundentity = NULL;
	}
	else
	{
		if (self->groundentity)
		{	// onground, return without moving
			return;
		}
	}

	VectorCopy(self->mins, formMove.mins);
	VectorCopy(self->maxs, formMove.maxs);

	formMove.passEntity = self;
	formMove.clipMask = self->clipmask;

	MoveEntity_Bounce(self, &formMove);

	PhysicsCheckWaterTransition(self);

	gi.linkentity(self);

	ActivateTriggers(self);
}

//---------------------------------------------------------------------------------
//Monsters freefall when they don't have a ground entity, otherwise
//all movement is done with discrete steps.

//This is also used for objects that have become still on the ground, but
//will fall if the floor is pulled out from under them.
//---------------------------------------------------------------------------------
static void Physics_StepMove(edict_t *self)
{
	qboolean	hasVel;
	FormMove_t	formMove;
	float		gravity;
	float		friction;

	assert(self->gravity >= 0.0f);

	hasVel = Vec3NotZero(self->velocity);

//	gi.dprintf("hasVel %i\n", hasVel);

//	gi.dprintf("vel in Physics_StepMove %f, %f, %f\n", self->velocity[0],
//		self->velocity[1], self->velocity[2]);

	// Apply rotation friction if desired
	if (self->physicsFlags & PF_ROTATIONAL_FRICTION)
	{
		if (!Vec3IsZero(self->avelocity))
		{
			ApplyRotationalFriction(self);
		}
	}
	else
	{
		VectorMA(self->s.angles, FRAMETIME, self->avelocity, self->s.angles);
	}

	if (self->physicsFlags & PF_RESIZE)
	{
		if (G_ResizeBoundingForm(self, &formMove))
		{
			self->physicsFlags &= ~PF_RESIZE;
		}
	}

	gravity = self->gravity * sv_gravity->value;

	// check for submersion or nograv
	if (self->waterlevel <= 1 && gravity > 0.0f)
	{
		if (self->groundentity)
		{
			friction = self->friction * sv_friction->value;

			if (!hasVel)
			{
				if (self->groundNormal[2] >= GROUND_NORMAL && self->groundNormal[2] >= (gravity / (friction + gravity)))
				{	// not going anywhere without velocity on ground whose slope this ent won't slide on
					return;
				}
			}
		}

		MoveEntity_Slide(self);
	}
	else
	{
		if (!hasVel)
		{	// not going anywhere without vel
			return;
		}

		VectorCopy(self->mins, formMove.mins);
		VectorCopy(self->maxs, formMove.maxs);

		formMove.passEntity = self;
		formMove.clipMask = self->clipmask;

		MoveEntity_Bounce(self, &formMove);
	}

	if (!BoundVelocity(self->velocity))
	{
//		gi.dprintf("Doesn't have vel\n");

		if (hasVel)
		{	// stopped
			G_QPostMessage(self, G_MSG_RESTSTATE, PRI_PHYSICS, "i", hasVel);
		}
	}
	else if(!hasVel)
	{	// started
		G_QPostMessage(self, G_MSG_RESTSTATE, PRI_PHYSICS, "i", hasVel);
	}

	PhysicsCheckWaterTransition(self);

	gi.linkentity(self);

	ActivateTriggers(self);
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
void EntityPhysics(edict_t *self)
{
	assert(self->inuse);

	switch (self->movetype)
	{
		case MOVETYPE_NONE:
			Physics_None(self); break;
		case MOVETYPE_STATIC:
			Physics_Static(self); break;
		case MOVETYPE_NOCLIP:
			Physics_NoclipMove(self); break;
		case MOVETYPE_FLY:
			Physics_FlyMove(self); break;
		case MOVETYPE_WALK:
		case MOVETYPE_STEP:
		case MOVETYPE_TOSS:
		case MOVETYPE_BOUNCE:
			Physics_StepMove(self); break;
		case MOVETYPE_PUSH:
		case MOVETYPE_STOP:
			Physics_Push(self); break;
		case MOVETYPE_FLYMISSILE:
			Physics_FlyMove(self); break;
		case MOVETYPE_SCRIPT_ANGULAR:
			Physics_ScriptAngular(self); break;
		default:
			gi.error ("SV_Physics: bad movetype %i", self->movetype);
			return;
	};
}

//---------------------------------------------------------------------------------
// Determines which entity if any the self is on
// Also zeros z vel and moves onto ground if it was in the air
//---------------------------------------------------------------------------------
void CheckEntityOn(edict_t *self)
{
	vec3_t		point;
	FormMove_t formMove;

	assert(self);

	if (self->velocity[2] > Z_VEL_NOT_ONGROUND)
	{
		self->groundentity = NULL;
		return;
	}

	// if the hull point one-quarter unit down is solid the entity is on ground
	point[0] = self->s.origin[0];
	point[1] = self->s.origin[1];
	point[2] = self->s.origin[2] - (PHYSICS_Z_FUDGE + CHECK_BELOW_DIST);

	VectorCopy(self->mins, formMove.mins);
	VectorCopy(self->maxs, formMove.maxs);

	formMove.start = self->s.origin;
	formMove.end = point;
	formMove.passEntity = self;
	formMove.clipMask = MASK_MONSTERSOLID;

	G_TraceBoundingForm(&formMove);

	// check steepness
	if (!formMove.trace.ent || (formMove.trace.plane.normal[2] < GROUND_NORMAL && !formMove.trace.startsolid))
	{
		self->groundentity = NULL;
		return;
	}

	if (!formMove.trace.startsolid && !formMove.trace.allsolid)
	{
		VectorCopy(formMove.trace.endpos, self->s.origin);

		SetGroundEntFromTrace(self, &formMove.trace);

//		self->velocity[2] = 0;
	}
}

//---------------------------------------------------------------------------------
// set move to be based on gravity and velocity, and adjust velocity for gravity
//---------------------------------------------------------------------------------
void ApplyGravity(edict_t *self, vec3_t move)
{
	assert(self);

	if (move)
	{
		move[2] -= self->gravity * sv_gravity->value * (FRAMETIME * FRAMETIME * 0.5);
	}

	self->velocity[2] -= self->gravity * sv_gravity->value * FRAMETIME;
}

//Make things that hit each other do some damage
void DoImpactDamage(edict_t *self, trace_t *trace)
{
	vec3_t	normal, movedir;
	float	impact_dmg, total_health, tr_health, self_health, speed;
	int		tr_dmg, self_dmg;

	if (self->impact_debounce_time>level.time)
		return;

	if (self->svflags&SVF_DO_NO_IMPACT_DMG)
		return;

	if (self->client)
	{
		if (self->client->playerinfo.edictflags & FL_CHICKEN)
		{
			return;
		}
	}

	if (self->svflags&SVF_MONSTER||self->mass||self->client)
	{
		speed = VectorLength(self->velocity);

		if (speed<50)
			return;

		if (speed < 500 && self->watertype)
			return;

		impact_dmg = sqrt(speed/10);

		//monsters dont do impact damage to their own type
		if (self->classID && trace->ent->classID && self->classID == trace->ent->classID)
			return;

		if (impact_dmg>0)
		{
			VectorCopy(trace->plane.normal, normal);

			VectorSet(movedir, 0, 0, 1);
			if (!Vec3IsZero(self->velocity))
			{
				VectorCopy(self->velocity, movedir);
				VectorNormalize(movedir);
			}

			if (trace->ent->solid==SOLID_BSP)
			{
				if (self->health>0)
					impact_dmg = impact_dmg * self->health / 100;
				else if(speed<300)
					return;

				if ((!trace->ent->takedamage && self->health*10 > 1000) || self->health<=0)
					tr_health = self->health * 10;
				else
					tr_health = 1000;
			}
			else if(trace->ent->health>0)
				tr_health = trace->ent->health * 0.5;
			else
				tr_health = 1;

			if (self->health>0)
				self_health = self->health * 2;
			else
				self_health = 2;

			total_health = self_health + tr_health;

			if (trace->ent->solid==SOLID_BSP&&!trace->ent->takedamage)
				tr_dmg = 0;
			else
				tr_dmg = floor(impact_dmg * self_health/total_health);

			self_dmg = floor(impact_dmg - tr_dmg);

			if (tr_dmg>=1 && trace->ent->takedamage && !(trace->ent->svflags&SVF_TAKE_NO_IMPACT_DMG)&&!(trace->ent->svflags&SVF_BOSS))
			{
				if (skill->value < 2 && self->svflags & SVF_MONSTER && trace->ent->client)
					tr_dmg = ceil(tr_dmg * 0.5);//monsters do a bit less damage to player on normal and easy skill
				if (tr_dmg >= 1)
				{
					T_Damage(trace->ent, self, self, movedir, trace->endpos, normal, tr_dmg, tr_dmg, 0,MOD_CRUSH);
					if (trace->ent->health>0)
					{
						if (trace->ent->client)
						{
							if (tr_dmg > irand(25, 40) - (5 * (skill->value)))
							{
								if (trace->ent->client->playerinfo.lowerseq != ASEQ_KNOCKDOWN)
									playerExport->KnockDownPlayer(&trace->ent->client->playerinfo);
							}
						}
					}
				}
			}

			if (self_dmg>=1 && self->takedamage && (speed > 600 || self->health <= 0) && !(self->svflags&SVF_TAKE_NO_IMPACT_DMG) && !(self->svflags&SVF_BOSS) && self->jump_time < level.time)
			{
				if (skill->value && self->client && trace->ent->solid == SOLID_BSP)
					self_dmg = floor(self->dmg * 1.5);//more damage to player from falls

				if (self_dmg >= 3 && ((self->classID != CID_ASSASSIN && self->classID != CID_SSITHRA) || self->health<=0))//but what about ring of repulsion?
				{
					if (self_dmg < 5)
						T_Damage(self, self, self, movedir, trace->endpos, normal, self_dmg, 0, DAMAGE_NO_BLOOD|DAMAGE_AVOID_ARMOR,MOD_FALLING);
					else
						T_Damage(self, self, self, movedir, trace->endpos, normal, self_dmg, 0, DAMAGE_AVOID_ARMOR,MOD_FALLING);
					self->impact_debounce_time = level.time + 0.3;//don't collide again too soon
				}
			}
		}
	}
}

//---------------------------------------------------------------------------------
// Calls correct collision handling functions for ents involved
//---------------------------------------------------------------------------------
void HandleCollision(edict_t *self, trace_t *trace, vec3_t move, int forceful, int flags)
{
	edict_t *other = trace->ent;

//	assert(self->solid != SOLID_NOT);
//	assert(other->solid != SOLID_NOT);

	if (impact_damage->value)
	{
		if (flags&CH_ISBLOCKED || flags&CH_BOUNCED)
		{
			DoImpactDamage(self, trace);
		}
	}

	if (forceful)
	{
		HandleForcefulCollision(self, other, move, forceful);
	}

	if (flags&CH_ISBLOCKED && self->isBlocked)
	{
		self->isBlocked(self, trace);
	}

	if (flags&CH_BOUNCED && self->bounced)
	{
		self->bounced(self, trace);
	}

	if (flags&CH_ISBLOCKING && other->isBlocking)
	{
		trace_t temp = *trace;

		temp.ent = self;
		VectorInverse(temp.plane.normal);

		other->isBlocking(other, &temp);
	}
}

//---------------------------------------------------------------------------------------
// forceful < 0 indicates that the forcer shouldn't be knocked back
//---------------------------------------------------------------------------------------
void HandleForcefulCollision(edict_t *forcer, edict_t *forcee, vec3_t move, int forceful)
{
	vec3_t dir, vel;
	float knockback;
	qboolean hitWorld = false;

	assert(forcee);

	if (forcee == g_edicts)
	{
		hitWorld = true;
		VectorScale(move, -FRAMES_PER_SECOND*0.8, vel);
	}
	else
	{
		hitWorld = false;
		VectorScale(move, FRAMES_PER_SECOND*0.5, vel);
	}

	knockback = VectorNormalize2(vel, dir);

	knockback *= forcer->mass;
	knockback /= KNOCK_BACK_MULTIPLIER;

	// knock other entity back
	if (!hitWorld)
	{
		PostKnockBack(forcee, dir, knockback, 0);
		Vec3ScaleAssign(-1.0, dir);
	}

	if (forceful > 0)
	{
		// knock back running ent
		PostKnockBack(forcer, dir, knockback, 0);
	}
}

//---------------------------------------------------------------------------------
// takes into account gravity, and bounces an ent away from impacts based on elasticity
// fiction is ignored
//---------------------------------------------------------------------------------
void MoveEntity_Bounce(edict_t *self, FormMove_t *formMove)
{
	vec3_t		move, end;

	VectorScale(self->velocity, FRAMETIME, move);

	// create the delta move
	if (self->gravity > 0.0f)
	{
		ApplyGravity(self, move);
	}

	VectorAdd(self->s.origin, move, end);

	formMove->start = self->s.origin;
	formMove->end = end;

	G_TraceBoundingForm(formMove);

	VectorCopy(formMove->trace.endpos, self->s.origin);

	// handle bouncing and sliding
	if (formMove->trace.fraction < 1.0)
	{
		BounceVelocity(self->velocity, formMove->trace.plane.normal, self->velocity, self->elasticity);

		if (self->elasticity > ELASTICITY_SLIDE)
		{
			if ((self->velocity[2] < 60.0) && (formMove->trace.plane.normal[2] > GROUND_NORMAL))
			{
				SetGroundEntFromTrace(self, &formMove->trace);

				VectorClear(self->velocity);
				VectorClear(self->avelocity);
			}
		}

		HandleCollision(self, &formMove->trace, move, (self->physicsFlags & PF_FORCEFUL_COLLISIONS), CH_STANDARD);
	}
}

#define FLOAT_ZERO_EPSILON 0.0005f

static qboolean
FloatIsZeroEpsilon(float f)
{
	return (Q_fabs(f) < FLOAT_ZERO_EPSILON);
}

//---------------------------------------------------------------------------------
//	Moves an entity sliding or bouncing of off any planes collided with
//---------------------------------------------------------------------------------
void MoveEntity_Slide(edict_t *self)
{
#define	MAX_CLIP_PLANES					5
#define MAX_BUMPS						4
#define STEEP_SLOPE_FRICTION_MODIFIER	0.1

	edict_t		*hit;
	int			bumpcount;
	vec3_t		dir, gdir;
	int			numplanes = 0;
	vec3_t		original_velocity, new_velocity;
	int			i, j;
	vec3_t		end, delta;
	float		timeRemaining = FRAMETIME, timeRemaining2, timeMoved;
	static vec3_t planes[MAX_CLIP_PLANES];
	FormMove_t	formMove;
	float		base_friction, friction, gravity;
	float		dist, dot, accel, speed, faccel, gaccel;
	qboolean	slide;
	float		*groundNormal;
	int			fudgeIndex = -1;

	assert(self->clipmask);
	assert(self);

	gravity = self->gravity * sv_gravity->value;
	base_friction = self->friction * sv_friction->value;

	VectorCopy(self->velocity, original_velocity);

	groundNormal = self->groundNormal;

	if (!self->groundentity)
	{
		groundNormal[2] = 0.0;
	}

	VectorCopy(self->mins, formMove.mins);
	VectorCopy(self->maxs, formMove.maxs);

	formMove.start = self->s.origin;
	formMove.passEntity = self;
	formMove.clipMask = self->clipmask;

	for(bumpcount = 0; bumpcount < MAX_BUMPS; ++bumpcount)
	{
		friction = base_friction;

		VectorScale(self->velocity, timeRemaining, delta);

		timeRemaining2 = timeRemaining * timeRemaining;

		slide = false;

		if (groundNormal[2])
		{	// on some type of upward facing slope
			assert(groundNormal[2] > 0.0);

			if (Vec3IsZero(self->velocity))
			{	// no velocity

				if (groundNormal[2] >= GROUND_NORMAL)
				{
					if (groundNormal[2] >= (gravity / (friction + gravity)))
					{	// can't slide
						if (bumpcount)
						{	// not going anywhere
							return;
						}
						else
						{	// check in calling func
							assert(0);
						}
					}
				}

				// ( |gravity| X groundNormal ) X groundNormal yeilds the vector in the
				//direction of gravity applied to the the slope of groundNormal
				gdir[0] = groundNormal[0]*groundNormal[2];
				gdir[1] = groundNormal[1]*groundNormal[2];
				gdir[2] = -groundNormal[0]*groundNormal[0] - groundNormal[1]*groundNormal[1];

				VectorNormalize(gdir);

				dot = DotProduct(gdir, groundNormal);

				if (dot < -FLOAT_ZERO_EPSILON)
				{	// floating point error, shit, fudge it away from the plane a bit
					fudgeIndex = i;
					VectorMA(self->s.origin, PHYSICS_Z_FUDGE, planes[i], self->s.origin);
				}

				dist = 0;

				dot = 0;

				slide = true;
			}
			else
			{
				dist = VectorNormalize2(delta, dir);
				dot = DotProduct(dir, groundNormal);

				if (dot < -0.05)
				{ // the trace will fail, try to restructure inorder to skip it
				}
				else if(dot < 0.05) // parallel to ground
				{
					slide = true;
				}
				else
				{	// pulling away from ground
				}
			}

		}
		else
		{	// easy case, fall straight down, no surface friction needed
		}

		if (slide)
		{	// moving along ground, apply gravity and friction appropriatly
			gdir[0] = groundNormal[0]*groundNormal[2];
			gdir[1] = groundNormal[1]*groundNormal[2];
			gdir[2] = -groundNormal[0]*groundNormal[0] - groundNormal[1]*groundNormal[1];

			VectorNormalize(gdir);

			dot = DotProduct(gdir, dir);

			if (groundNormal[2] < GROUND_NORMAL)
			{	// turn down friction on a steep slope, the theory being that something wouldn't be able to maintain
				// good surface contact on such a slope
				faccel = -friction * groundNormal[2] * STEEP_SLOPE_FRICTION_MODIFIER;
			}
			else
			{
				faccel = -friction * groundNormal[2];
			}

			dist += 0.5 * faccel * timeRemaining2;

			if (dist < 0)
			{
				dist = 0;
				faccel = 0;
			}

			VectorScale(dir, dist, delta);
			dot = DotProduct(gdir, groundNormal);

			if (dot < -FLOAT_ZERO_EPSILON)
			{	// floating point error, shit, fudge it away from the plane a bit
				fudgeIndex = i;
				VectorMA(self->s.origin, PHYSICS_Z_FUDGE, planes[i], self->s.origin);
			}

			if (groundNormal[2] < GROUND_NORMAL)
			{	// turn down friction on a steep slope, the theory being that something wouldn't be able to maintain
				// good surface contact on such a slope
				gaccel = gravity * (1 - groundNormal[2]) - friction * groundNormal[2] * STEEP_SLOPE_FRICTION_MODIFIER;
			}
			else
			{
				gaccel = gravity * (1 - groundNormal[2]) - friction * groundNormal[2];
			}

			if (gaccel < 0)
			{
				gaccel = 0;
			}

			VectorMA(delta, gaccel * 0.5 * timeRemaining2, gdir, delta);

			if (dist + faccel + gaccel == 0.0)
			{
				VectorClear(self->velocity);
				return;
			}
		}
		else
		{	// else apply gravity straight down, no friction
			delta[2] -= 0.5 * gravity * timeRemaining2;
		}

		VectorCopy(self->s.origin, end);
		Vec3AddAssign(delta, end);

		formMove.end = end;

		G_TraceBoundingForm(&formMove);

		if (formMove.trace.startsolid)
		{
			if (fudgeIndex != -1)
			{	// undo fudge and let it try that again
				VectorMA(self->s.origin, -PHYSICS_Z_FUDGE, planes[fudgeIndex], self->s.origin);
				continue;
			}
			else
			{
				VectorClear(self->velocity);

				return;
			}
		}

		if (formMove.trace.allsolid)
		{	// entity is trapped in another solid
			VectorClear(self->velocity);

			self->s.origin[2] += 20;

			return;
		}

		timeMoved = timeRemaining * formMove.trace.fraction;

		if (formMove.trace.fraction > 0)
		{	// actually covered some distance
			VectorCopy(formMove.trace.endpos, self->s.origin);

			if (slide)
			{
				speed = VectorNormalize2(self->velocity, dir);

				dot = DotProduct(dir, groundNormal);

				speed += faccel * timeMoved;

				VectorScale(dir, speed, self->velocity);

				VectorMA(self->velocity, gaccel * timeMoved, gdir, self->velocity);
			}
			else
			{
				self->velocity[2] -= gravity * timeMoved;
			}

			if (formMove.trace.fraction == 1)
			{
				break;		// moved the entire distance
			}

			VectorCopy(self->velocity, original_velocity);

			numplanes = 0;
			fudgeIndex = -1;
		}
		else
		{
			if (Vec3IsZero(self->velocity) && self->groundNormal[2] >= (gravity / (friction + gravity)))
			{	// no velocity, and the trace failed, not going anywhere on ground the ent can't slide on
				break;
			}
		}

		hit = formMove.trace.ent;

		if (bumpcount == MAX_BUMPS - 1)
		{	// results in isBlocked being called on the last bounced
			HandleCollision(self, &formMove.trace, delta, -1, CH_BOUNCED|CH_STANDARD);
		}
		else
		{
			HandleCollision(self, &formMove.trace, delta, -1, CH_BOUNCED|CH_ISBLOCKING);
		}

		VectorCopy(formMove.trace.plane.normal, groundNormal);

		if (groundNormal[2] > 0 && hit->solid == SOLID_BSP)	// hit the floor
		{
			self->groundentity = formMove.trace.ent;
		}
		else
		{
			if (groundNormal[2] < 0.0)
			{
				groundNormal[2] = 0.0;
			}

			self->groundentity = NULL;
		}

		timeRemaining -= timeMoved;

		// clipped to another plane
		assert(numplanes < MAX_CLIP_PLANES);

		if (!numplanes || !VectorCompare(formMove.trace.plane.normal, planes[numplanes-1]))
		{
			VectorCopy(formMove.trace.plane.normal, planes[numplanes]);
			numplanes++;
		}
		else
		{
			fudgeIndex = numplanes-1;
			VectorMA(self->s.origin, PHYSICS_Z_FUDGE, planes[fudgeIndex], self->s.origin);
		}

		// modify original_velocity so it parallels all of the clip planes
		for(i = 0; i < numplanes; ++i)
		{
			float dirMag;

			assert(Vec3NotZero(planes[i]));

#if 0
#define ACCEL_A // velocity is maintained throughout, otherwise, elasticity is accounted for
#endif

#ifdef ACCEL_A	// top version is better for testing sliding with sv_friction at 0
			speed = VectorNormalize2(original_velocity, dir);

//			gi.dprintf("Speed %f\n", speed);

			BounceVelocity(dir, planes[i], dir, 1.0001f);	// only works with an elasticity a bit greater than 1.0
																	// going to need a different func
																	// for bouncing stuff

			dirMag = VectorNormalize(dir);

#else
			BounceVelocity(original_velocity, planes[i], new_velocity, self->elasticity);

			dirMag = speed = VectorNormalize2(new_velocity, dir);

#endif // ACCEL_A

			if (FloatIsZeroEpsilon(dirMag))
			{	// smacked into something exactly head on
				if (planes[i][2] > 0.0)
				{	// slide down slope
					dir[0] = -planes[i][0]*planes[i][2];
					dir[1] = -planes[i][1]*planes[i][2];
					dir[2] = planes[i][0]*planes[i][0] + planes[i][1]*planes[i][1];

					VectorNormalize(dir);
				}
				else
				{	// drop straight down
					dir[2] = -1.0;
				}

			}

			dot = DotProduct(dir, planes[i]);

			if (dot < -FLOAT_ZERO_EPSILON)
			{	// floating point error, shit, fudge it away from the plane a bit
//				gi.dprintf("Dot %f, Fudge for bump %i and plane %i\n", dot, bumpcount, i);
				fudgeIndex = i;
				VectorMA(self->s.origin, PHYSICS_Z_FUDGE, planes[i], self->s.origin);
			}

			slide = false;

			if (planes[i][2] > 0.0)
			{
				if (dot < -0.01)
				{ // the trace will fail, try to restructure inorder to skip it
#ifdef ACCEL_A
					assert(0);	// shouldn't happen
#endif // ACCEL_A
				}
				else if(Q_fabs(dot) < 0.01) // parallel to surface
				{
					slide = true;
				}
				else
				{	// pulling away from surface
				}
			}
			else
			{	// easy case, fall straight down, no surface friction needed
			}

#ifdef ACCEL_A
			if (slide)
			{	// moving along ground, apply gravity and friction appropriatly
				assert(Q_fabs(DotProduct(dir, planes[i])) < 0.1);

				accel = -friction * planes[i][2] - gravity * dir[2];
				speed += accel * timeRemaining;
				VectorScale(dir, speed, new_velocity);
//				gi.dprintf("Velocity slid, accel %f, speed %f\n", accel, speed);
			}
			else
			{	// else apply gravity straight down, no friction
				VectorScale(dir, speed, new_velocity);
				new_velocity[2] -= gravity * timeRemaining;
//				gi.dprintf("Velocity dropped 1, bump %i, plane %i\n", bumpcount, i);
//				gi.dprintf("dir %f, %f, %f\n", dir[0], dir[1], dir[2]);
			}

#endif // ACCEL_A

//			gi.dprintf("dirMag %f\n", dirMag);
//			gi.dprintf("Speed %f\n", VectorLength(new_velocity));

//			gi.dprintf("Bounce dot %f\n", dot);

			for(j = 0; j < numplanes; ++j)
			{
				if (j != i)
				{
					if (DotProduct(new_velocity, planes[j]) <= 0)
					{
						break;	// unacceptable slide
					}
				}
			}

			if (j == numplanes)
			{
				break;	// acceptable slide
			}
		}

		if (i != numplanes)
		{	// good slide
			VectorCopy(new_velocity, self->velocity);

//			gi.dprintf("Acceptable slide for bump %i\n", bumpcount);

		}
		else
		{	// go along the crease
			assert(numplanes);

			if (numplanes != 2)
			{
//				gi.dprintf ("slide, numplanes == %i\n",numplanes);
				VectorClear(self->velocity);
				return;
			}

//			gi.dprintf("Unacceptable slide for bump %i\n", bumpcount);

			CrossProduct (planes[0], planes[1], dir);

			speed = VectorLength(self->velocity);

			if (dir[2] <= 0)
			{
#ifdef ACCEL_A
				accel = -friction * (1 - dir[2]) - gravity * dir[2];
				speed += accel * timeRemaining;
#else
				slide = true;
#endif // ACCEL_A
			}

			VectorScale(dir, speed, self->velocity);
		}

#ifndef ACCEL_A
		speed = VectorNormalize2(self->velocity, dir);

		if (slide)
		{	// moving along ground, apply gravity and friction appropriatly
			accel = -friction * (1 - dir[2]) - gravity * dir[2];
			speed += accel * timeRemaining;
			VectorScale(dir, speed, new_velocity);
//				gi.dprintf("Velocity slid, accel %f, speed %f\n", accel, speed);
		}
		else
		{	// else apply gravity straight down, no friction
			VectorScale(dir, speed, new_velocity);
			new_velocity[2] -= gravity * timeRemaining;
//			gi.dprintf("Velocity dropped 2, bump %i, plane %i\n", bumpcount, i);
//			gi.dprintf("dir %f, %f, %f\n", dir[0], dir[1], dir[2]);
		}

#endif // ACCEL_A

		// if velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
#if 0	// haven't seen a problem with it yet. . .
		if (DotProduct(self->velocity, primal_velocity) <= 0)
		{
			gi.dprintf("Velocity was cleared at bump %i\n", bumpcount);

			VectorClear(self->velocity);
			break;
		}
#endif
	}

	if (formMove.trace.fraction < 1)
	{
		HandleCollision(self, &formMove.trace, delta, -1, CH_STANDARD);

		if (Vec3NotZero(self->velocity))
		{
			VectorClear(self->velocity);
//			gi.dprintf("Unsuccesful move with %i bumps and %i planes\n", bumpcount, numplanes);
		}

		if (formMove.trace.plane.normal[2] > GROUND_NORMAL)	// hit the floor
		{
			SetGroundEntFromTrace(self, &formMove.trace);
			return;
		}
	}
	CheckEntityOn(self);
}

//---------------------------------------------------------------------------------
// Searches for any triggers in the entities bounding box and their touch function
//---------------------------------------------------------------------------------
void ActivateTriggers(edict_t *self)
{
	edict_t *hit;
	GenericUnion4_t found;
	SinglyLinkedList_t list;

	// dead things don't activate triggers
	if (self->deadflag != DEAD_NO)
	{
		return;
	}

	SLList_DefaultCon(&list);	// this should be global, initialized at startup

	G_FindEntitiesInBounds(self->mins, self->maxs, &list, AREA_TRIGGERS);

	while(!SLList_IsEmpty(&list))
	{
		found = SLList_Pop(&list);

		hit = found.t_edict_p;

		if (!hit->inuse || !hit->touch)
		{
			continue;
		}

		hit->touch(hit, self, NULL, NULL);
	}

	SLList_Des(&list);			// kill on shut down
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
void ApplyRotationalFriction(edict_t *self)
{
	int		i;
	float	adjustment;

	VectorMA(self->s.angles, FRAMETIME, self->avelocity, self->s.angles);

	adjustment = FRAMETIME * FRICTION_STOPSPEED * FRICTION_SURFACE;

	for(i = 0; i < 3; ++i)
	{
		if (self->avelocity[i] > 0.0)
		{
			self->avelocity[i] -= adjustment;

			if (self->avelocity[i] < 0.0)
			{
				self->avelocity[i] = 0.0;
			}
		}
		else
		{
			self->avelocity[i] += adjustment;

			if (self->avelocity[i] > 0.0)
			{
				self->avelocity[i] = 0.0;
			}
		}
	}
}

//---------------------------------------------------------------------------------
// Sets the groundentity info contained in self based on trace
//---------------------------------------------------------------------------------
void SetGroundEntFromTrace(edict_t *self, trace_t *trace)
{
	assert(self);

	assert(trace->plane.normal[2] > GROUND_NORMAL);

	self->groundentity = trace->ent;

	self->groundentity_linkcount = trace->ent->linkcount;

	VectorCopy(trace->plane.normal, self->groundNormal);
}

//---------------------------------------------------------------------------------
// Sets the blockingEntity info contained in self based on trace
//---------------------------------------------------------------------------------
void SetBlockingEntFromTrace(edict_t *self, trace_t *trace)
{
	assert(self);

	self->blockingEntity = trace->ent;

	self->blockingEntity_linkcount = trace->ent->linkcount;

	VectorCopy(trace->plane.normal, self->blockingNormal);
}

//---------------------------------------------------------------------------------
// Determines if the self is in the water; if so, how submereged the self is
//---------------------------------------------------------------------------------
void CheckInWater(FormMove_t *formMove)
{
	int	contents;
	float halfHeight;

	assert(formMove);

	contents = G_GetContentsAtPoint(formMove->start);

	if (!(contents & MASK_WATER))
	{
		formMove->waterLevel = 0;
		formMove->waterType = 0;
		return;
	}

	formMove->waterType = contents;
	formMove->waterLevel = 1;

	halfHeight = (formMove->maxs[2] - (formMove->mins[2] + 1.0)) * 0.5;

	formMove->start[2] += halfHeight;

	contents = G_GetContentsAtPoint(formMove->start);

	if (!(contents & MASK_WATER))
	{
		return;
	}

	formMove->waterLevel = 2;

	formMove->start[2] += halfHeight;

	contents = G_GetContentsAtPoint(formMove->start);

	if (contents & MASK_WATER)
	{
		formMove->waterLevel = 3;
	}
}

//---------------------------------------------------------------------------------
// Returns false if any part of the bottom of the entity is off an edge that
// is not a staircase.
//---------------------------------------------------------------------------------
qboolean CheckFooting(edict_t *self, vec3_t origin)
{
	vec3_t	mins, maxs, start, stop;
	int		x, y;
	float	mid, bottom;
	qboolean solid;
	float stepHeight = classStatics[self->classID].moveInfo->stepHeight;
	FormMove_t formMove;

	VectorAdd(origin, self->mins, mins);
	VectorAdd(origin, self->maxs, maxs);

	// if all of the points under the corners are solid world, don't bother
	// with the tougher checks
	start[2] = mins[2] - 1;

	for(x = 0; x <= 1; ++x)
	{
		for(y = 0; y <= 1; ++y)
		{
			start[0] = x ? maxs[0] : mins[0];
			start[1] = y ? maxs[1] : mins[1];

			if (G_GetContentsAtPoint(start) != CONTENTS_SOLID)
			{
				solid = false;
			}
		}
	}

	if (solid)
	{
		return true;
	}

	start[0] = stop[0] = (mins[0] + maxs[0])*0.5;
	start[1] = stop[1] = (mins[1] + maxs[1])*0.5;
	start[2] = mins[2];

	stop[2] = start[2] - 2*stepHeight;

	VectorClear(formMove.mins);
	VectorClear(formMove.maxs);

	formMove.start = start;
	formMove.end = stop;
	formMove.passEntity = self;
	formMove.clipMask = MASK_MONSTERSOLID;

	G_TraceBoundingForm(&formMove);

	if (formMove.trace.fraction == 1.0)
	{
		return false;
	}

	mid = bottom = formMove.trace.endpos[2];

	for(x = 0; x <= 1; ++x)
	{
		for(y = 0; y <= 1; ++y)
		{
			start[0] = stop[0] = x ? maxs[0] : mins[0];
			start[1] = stop[1] = y ? maxs[1] : mins[1];

			formMove.start = start;
			formMove.end = stop;

			G_TraceBoundingForm(&formMove);

			if (formMove.trace.fraction != 1.0 && formMove.trace.endpos[2] > bottom)
			{
				bottom = formMove.trace.endpos[2];
			}

			if (formMove.trace.fraction == 1.0 || mid - formMove.trace.endpos[2] > stepHeight)
			{
				return false;
			}
		}
	}

	return true;
}

static void DiscreteMove_Step_FailedDueToCollision(edict_t *self, vec3_t move, FormMove_t *formMove, qboolean forceful)
{
	if (!(formMove->processFlags & PPF_INFO_GRAB))
	{
		SetBlockingEntFromTrace(self, &formMove->trace);
		HandleCollision(self, &formMove->trace, move, (self->physicsFlags & PF_FORCEFUL_COLLISIONS) & forceful, CH_STANDARD);
	}
}

//---------------------------------------------------------------------------------
// Attempts to move self as specified by move.  Allows walking on slopes up to
// GROUND_NORMAL, steps up or down to classStatics[self->classID].self->stepHeight,
// will drio off edges up to classStatics[self->classID].self->dropHeight,
// won't enter water higher than level 2
//
// Supports PPF_INFO_GRAB for checking a step without actually moving
//---------------------------------------------------------------------------------
qboolean DiscreteMove_Step(edict_t *self, vec3_t move, FormMove_t *formMove)
{
	vec3_t		neworg, start, end;
	vec3_t		test;
	qboolean	stepUp = false, setGroundEnt = false;

	assert(self);
	assert(formMove);

	if (self->physicsFlags & PF_RESIZE)
	{
		if (G_ResizeBoundingForm(self, formMove))
		{
			self->physicsFlags &= ~PF_RESIZE;
		}
		else
		{
			return false;
		}
	}

	VectorAdd(self->s.origin, move, neworg);

	start[0] = end[0] = neworg[0];
	start[1] = end[1] = neworg[1];
	start[2] = neworg[2] + formMove->stepHeight;

	if (neworg[2] > self->s.origin[2])
	{
		end[2] = self->s.origin[2] - formMove->dropHeight;
		stepUp = true;
	}
	else
	{
		end[2] = neworg[2] - formMove->dropHeight;
	}

	VectorCopy(self->mins, formMove->mins);
	VectorCopy(self->maxs, formMove->maxs);

	formMove->start = start;
	formMove->end = end;
	formMove->passEntity = self;
	formMove->clipMask = self->clipmask;

	G_TraceBoundingForm(formMove);

	if (formMove->trace.allsolid)
	{
		end[2] = neworg[2] + 0.25f;

		// find out what's blocking it
		formMove->start = self->s.origin;

		G_TraceBoundingForm(formMove);

		DiscreteMove_Step_FailedDueToCollision(self, move, formMove, true);
		return false;
	}

	if (formMove->trace.startsolid)
	{
		start[2] = neworg[2] + 0.25f;

		formMove->start = start;

		G_TraceBoundingForm(formMove);

		if (formMove->trace.allsolid)
		{
			// find out what's blocking it
			formMove->start = self->s.origin;
			formMove->end = neworg;

			G_TraceBoundingForm(formMove);

			DiscreteMove_Step_FailedDueToCollision(self, move, formMove, true);
			return false;
		}

		if (formMove->trace.startsolid)
		{
			// find out what's blocking it
			formMove->start = self->s.origin;
			formMove->end = neworg;

			G_TraceBoundingForm(formMove);

			DiscreteMove_Step_FailedDueToCollision(self, move, formMove, true);
			return false;
		}
	}

	test[0] = formMove->trace.endpos[0];
	test[1] = formMove->trace.endpos[1];
	test[2] = formMove->trace.endpos[2] + self->mins[2] + 1;

	formMove->start = test;

	CheckInWater(formMove);

	// don't go into deep water
	if (formMove->waterLevel > 2)
	{
		return false;
	}

	// don't walk off edges
	if (formMove->trace.fraction == 1)
	{
		return false;
	}

#if 0
	// check point traces down for dangling corners
	if (!CheckFooting(self, formMove->trace.endpos))
	{
		return false;
	}
#endif

	if (stepUp || formMove->trace.endpos[2] >= neworg[2] - formMove->dropHeight)
	{
		// make sure its still on the ground
		if (formMove->trace.plane.normal[2] > GROUND_NORMAL)
		{
			setGroundEnt = true;
		}
		else
		{
			DiscreteMove_Step_FailedDueToCollision(self, move, formMove, !stepUp);
			return false;
		}
	}

	if (!(formMove->processFlags & PPF_INFO_GRAB))
	{	// finalize new position
		self->waterlevel = formMove->waterLevel;
		self->watertype = formMove->waterType;

		self->flags &= ~FL_INWATER;

		if (formMove->waterLevel)
		{
			self->flags |= FL_INWATER;
		}

		if (setGroundEnt)
		{	// step up or down maintaing contact with the ground
			SetGroundEntFromTrace(self, &formMove->trace);
			VectorCopy(formMove->trace.endpos, self->s.origin);
		}
		else
		{	// drop off and edge
			self->groundentity = NULL;
			VectorCopy(neworg, self->s.origin);

			// should probably set a velocity here also
		}

		gi.linkentity(self);
		ActivateTriggers(self);
	}

	return true;
}

void CreateMove_Step(edict_t *self, vec3_t move, float dist)
{
	float yaw, pitch;
	float cosYaw;
	float sinYaw;

	assert(self->groundentity);	// need a groundentity if groundNormal is to be used

//	assert(self->groundNormal[2] > GROUND_NORMAL);

	yaw = self->s.angles[YAW]*ANGLE_TO_RAD + self->yawOffset;

	cosYaw = cos(yaw);
	sinYaw = sin(yaw);

	move[0] = cosYaw;
	move[1] = sinYaw;
	move[2] = 0;

	pitch = acos(DotProduct(self->groundNormal, move)) - ANGLE_90;

	// adjust for sloped ground
	if (pitch)
	{
		float cosPitch;

		cosPitch = cos(pitch);

		move[0] = cosYaw*cosPitch;
		move[1] = sinYaw*cosPitch;
		move[2] = sin(pitch);
	}

	Vec3ScaleAssign(dist, move);
}

//---------------------------------------------------------------------------------
//The move will be adjusted for slopes and stairs, but if the move isn't
//possible, no move is done, false is returned
//
//	FINISH ME!!!  These need a lot more work.
//
//---------------------------------------------------------------------------------

void AttemptAdjustOriginToValidPosition(vec3_t adjustFrom, edict_t *self, vec3_t origin)
{
	FormMove_t formMove;

	VectorCopy(self->mins, formMove.mins);
	VectorCopy(self->maxs, formMove.maxs);

	formMove.start = adjustFrom;
	formMove.end = origin;
	formMove.passEntity = self;
	formMove.clipMask = self->clipmask;

	G_TraceBoundingForm(&formMove);

	if (!(formMove.trace.allsolid || formMove.trace.startsolid))
	{
		VectorCopy(formMove.trace.endpos, origin);
	}
	else
	{
		VectorCopy(adjustFrom, origin);
	}
}

qboolean CheckAnimMove(edict_t *self, vec3_t origin, vec3_t move, float *dist, float attemptDist, int recursionLevel)
{
	vec3_t		desiredOrigin, end, temp;
	float		stepHeight, tempDist;
	FormMove_t	formMove, stepFormMove;
	vec3_t		test;
	vec3_t adjustFrom, fudgeOrigin;

	if (!checkanim->value)
	{	// globally disablled, allow full move
		*dist = attemptDist;
		return true;
	}

#if 1
	if (recursionLevel > 10)
	{
//		gi.dprintf("trace startsolid in CheckAnimMove, recursion > 10, failing\n");
		return false;
	}
#endif

	stepHeight = classStatics[self->classID].moveInfo->stepHeight;

	VectorCopy(origin, fudgeOrigin);
	fudgeOrigin[2] += 1.0;

	VectorAdd(fudgeOrigin, move, desiredOrigin);

	VectorCopy(self->mins, formMove.mins);
	VectorCopy(self->maxs, formMove.maxs);

	formMove.start = origin;
	formMove.end = desiredOrigin;
	formMove.passEntity = self;
	formMove.clipMask = self->clipmask;

	G_TraceBoundingForm(&formMove);

	if (formMove.trace.startsolid)
	{
		if (!Vec3IsZero(formMove.trace.plane.normal))
		{
			if (formMove.trace.plane.normal[2] > 0)
			{
//				gi.dprintf("trace startsolid in CheckAnimMove, attempting to adjust with normal\n");

				VectorMA(origin, 20.0, formMove.trace.plane.normal, adjustFrom);

				AttemptAdjustOriginToValidPosition(adjustFrom, self, origin);

				return CheckAnimMove(self, origin, move, dist, attemptDist, recursionLevel + 1);
			}
			else
			{
//				gi.dprintf("trace startsolid in CheckAnimMove, bad normal\n");
			}
		}
		else
		{
//			gi.dprintf("trace startsolid in CheckAnimMove, attempting to adjust without normal\n");

			VectorCopy(origin, adjustFrom);

			adjustFrom[2] += 20.0;

			AttemptAdjustOriginToValidPosition(adjustFrom, self, origin);

			return CheckAnimMove(self, origin, move, dist, attemptDist, recursionLevel + 1);
		}
	}

	if (formMove.trace.allsolid)
	{
//		gi.dprintf("trace allsolid in CheckAnimMove, attempting to adjust without normal\n");

		VectorCopy(origin, adjustFrom);

		adjustFrom[2] += 20.0;

		AttemptAdjustOriginToValidPosition(adjustFrom, self, origin);

		return CheckAnimMove(self, origin, move, dist, attemptDist, recursionLevel + 1);
	}

	VectorScale(move, formMove.trace.fraction, temp);

	tempDist = VectorLength(temp);

//	gi.dprintf("tempDist %f\n", tempDist);

	if (tempDist + *dist < TRACE_FRACTION_FAIL_FOR_WHOLE_MOVE_CHECK*attemptDist)
	{
		if (recursionLevel)
		{
			// don't try to step on recursive calls if the surface isn't ground, or
			// it was only able to move a little forward
			if (formMove.trace.plane.normal[2] <= GROUND_NORMAL ||
				tempDist < stepHeight)
			{
				return false;
			}
		}

		*dist += tempDist;

//		gi.dprintf("Recursion with fraction %f dist %f\n", formMove.trace.fraction, *dist);

		VectorAdd(origin, temp, adjustFrom);

		adjustFrom[2] += stepHeight;

		Vec3ScaleAssign(1.0 - formMove.trace.fraction, move);

		return CheckAnimMove(self, adjustFrom, move, dist, attemptDist, recursionLevel + 1);
	}

	VectorMA(fudgeOrigin, formMove.trace.fraction, move, desiredOrigin);

	// Check for going off an edge
	end[0] = desiredOrigin[0];
	end[1] = desiredOrigin[1];

	if (desiredOrigin[2] > origin[2])
	{
		end[2] = origin[2] - stepHeight;
	}
	else
	{
		end[2] = desiredOrigin[2] - stepHeight;
	}

	VectorCopy(formMove.mins, stepFormMove.mins);
	VectorCopy(formMove.maxs, stepFormMove.maxs);

	stepFormMove.start = desiredOrigin;
	stepFormMove.end = end;
	stepFormMove.passEntity = self;
	stepFormMove.clipMask = self->clipmask;

	G_TraceBoundingForm(&stepFormMove);

	if (stepFormMove.trace.startsolid)
	{
//		gi.dprintf("stepTrace startsolid CheckAnimMove\n");
		return false;
//		stepFormMove.trace.fraction = 0.0;
	}

	if (stepFormMove.trace.allsolid)
	{
//		gi.dprintf("stepTrace allsolid CheckAnimMove\n");
		return false;
//		stepFormMove.trace.fraction = 0.0;
	}

	// may have walked off an edge
	if (stepFormMove.trace.fraction == 1.0)
	{
		VectorCopy(origin, adjustFrom);

		adjustFrom[2] = stepFormMove.trace.endpos[2];

		stepFormMove.start = stepFormMove.trace.endpos;
		stepFormMove.end = adjustFrom;

		G_TraceBoundingForm(&stepFormMove);

		VectorCopy(temp, move);

		Vec3ScaleAssign(1.0 - stepFormMove.trace.fraction, temp);

		tempDist = VectorLength(temp);

		if (recursionLevel)
		{
			// don't try to step on recursive calls if the surface isn't ground, or
			// it was only able to move a little forward
			if (stepFormMove.trace.plane.normal[2] <= GROUND_NORMAL ||
				tempDist < stepHeight)
			{
				return false;
			}
		}

		*dist += tempDist;

		Vec3AddAssign(temp, adjustFrom);

//		gi.dprintf("Step off recursion with fraction %f dist %f\n", stepFormMove.trace.fraction, *dist);

		Vec3ScaleAssign(formMove.trace.fraction, move);

		return CheckAnimMove(self, adjustFrom, move, dist, attemptDist, recursionLevel + 1);
	}

	// stepped up onto something it can't stand on
	if (stepFormMove.trace.plane.normal[2] <= GROUND_NORMAL)
	{
		return false;
	}

	// don't go into water
	if (self->waterlevel == 0.0)
	{
		test[0] = stepFormMove.trace.endpos[0];
		test[1] = stepFormMove.trace.endpos[1];
		test[2] = stepFormMove.trace.endpos[2] + self->mins[2] + 1;

		stepFormMove.start = test;

		CheckInWater(&stepFormMove);

		// probably want to actually let monsters wade
		if (self->flags & FL_INWATER)
		{
			self->waterlevel = 0;
			self->watertype = 0;

			self->flags &= ~FL_INWATER;
			return false;
		}
	}

	VectorScale(move, formMove.trace.fraction, temp);

	*dist += VectorLength(temp);

	return true;
}

static edict_t *
TestEntityPosition(edict_t *self)
{
	FormMove_t formMove;

	VectorCopy(self->mins, formMove.mins);
	VectorCopy(self->maxs, formMove.maxs);

	formMove.start = self->s.origin;
	formMove.end = self->s.origin;
	formMove.passEntity = self;
	formMove.clipMask = self->clipmask;

	G_TraceBoundingForm(&formMove);

	if (formMove.trace.startsolid)
	{
		return g_edicts;
	}

	return NULL;
}

#define MAX_MATRIX (3)

static void BackSub(float a[][MAX_MATRIX],int *indx,float *b,int sz)
{
    int i,j,ii=-1,ip;
    float sum;
    for (i=0;i<sz;i++)
    {
		ip=indx[i];
		sum=b[ip];
        b[ip]=b[i];
        if (ii>=0)
        {
            for (j=ii;j<i;j++)
                sum-=a[i][j]*b[j];
        }
        else if (sum!=0.0)
        {
            ii=i;
        }
        b[i]=sum;
    }
    for (i=sz-1;i>=0;i--)
    {
        sum=b[i];
        for (j=i+1;j<sz;j++)
            sum-=a[i][j]*b[j];
        b[i]=sum/a[i][i];
    }
}

#define EPS_MATRIX (1E-15)

static int LUDecomposition(float a[][MAX_MATRIX],int indx[],int sz,float *d)
{
    int i,imax,j,k;
    float big,dum,sum,temp;
    float vv[MAX_MATRIX];
	*d=1;
    for (i=0;i<sz;i++)
    {
        big=0.;
        for (j=0;j<sz;j++)
            if ((temp=fabs(a[i][j]))>big)
                    big=temp;
        if (big<EPS_MATRIX)
			return 0;
        vv[i]=1.0/big;
    }
	imax = 0;//should have a def value just to make sure
    for (j=0;j<sz;j++)
    {
        for (i=0;i<j;i++)
        {
            sum=a[i][j];
            for (k=0;k<i;k++)
                sum-=a[i][k]*a[k][j];
            a[i][j]=sum;
        }
        big=0.;
        for (i=j;i<sz;i++)
        {
            sum=a[i][j];
            for (k=0;k<j;k++)
                sum-=a[i][k]*a[k][j];
            a[i][j]=sum;
            if ((dum=vv[i]*fabs(sum))>=big)
            {
                big=dum;
                imax=i;
            }
        }
        if (j!=imax)
        {
            for (k=0;k<sz;k++)
            {
                dum=a[imax][k];
                a[imax][k]=a[j][k];
                a[j][k]=dum;
            }
            vv[imax]=vv[j];
			*d=-(*d);
        }
        indx[j]=imax;
        if (fabs(a[j][j])<EPS_MATRIX)
			return 0;
        if (j!=sz-1)
        {
            dum=1.0/a[j][j];
            for (i=j+1;i<sz;i++)
                    a[i][j]*=dum;
        }
    }
	return 1;
}

/*
============
PushEntities

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
============
*/

#if 0
void VectorRound(vec3_t v)
{
	//assert(v != vec3_origin);

	v[0] = (float)floor(v[0] + 0.5);
	v[1] = (float)floor(v[1] + 0.5);
	v[2] = (float)floor(v[2] + 0.5);
}
#endif

#define GIL_PUSH 1
qboolean PushEntities(edict_t *pusher, vec3_t move, vec3_t amove)
{
	int			e;
	edict_t		*check, *block;
	vec3_t		mins, maxs;
	pushed_t	*p;
	vec3_t		org, org2, move2, forward, right, up,holdOrg;

#if GIL_PUSH
	float a[3][3];
	vec3_t		forwardInv, rightInv, upInv,OrgOrg;
	int indx[3];
	float d;
#endif
	// Commented out for Mr Rick
#if	0
// clamp the move to 1/8 units, so the position will
// be accurate for client side prediction
	Vec3ScaleAssign(8.0, move);
	VectorRound(move);
	Vec3ScaleAssign(0.125, move);
#endif

// find the bounding box

	if (move[0]<0)
	{
		mins[0]=pusher->absmin[0]+move[0];
		maxs[0]=pusher->absmax[0];
	}
	else
	{
		mins[0]=pusher->absmin[0];
		maxs[0]=pusher->absmax[0]+move[0];
	}
	if (move[1]<0)
	{
		mins[1]=pusher->absmin[1]+move[1];
		maxs[1]=pusher->absmax[1];
	}
	else
	{
		mins[1]=pusher->absmin[1];
		maxs[1]=pusher->absmax[1]+move[1];
	}
	if (move[2]<0)
	{
		mins[2]=pusher->absmin[2]+move[2];
		maxs[2]=pusher->absmax[2];
	}
	else
	{
		mins[2]=pusher->absmin[2];
		maxs[2]=pusher->absmax[2]+move[2];
	}

// we need this for pushing things later
#if GIL_PUSH
	VectorCopy(pusher->s.angles, org);
	Vec3AddAssign(amove, org);
	AngleVectors(org, forward, right, up);
/*
	a[0][0]=forward[0];
	a[0][1]=forward[1];
	a[0][2]=forward[2];
	a[1][0]=-right[0];
	a[1][1]=-right[1];
	a[1][2]=-right[2];
	a[2][0]=up[0];
	a[2][1]=up[1];
	a[2][2]=up[2];
*/
	a[0][0]=forward[0];
	a[1][0]=forward[1];
	a[2][0]=forward[2];
	a[0][1]=-right[0];
	a[1][1]=-right[1];
	a[2][1]=-right[2];
	a[0][2]=up[0];
	a[1][2]=up[1];
	a[2][2]=up[2];
	if (LUDecomposition(a,indx,3,&d))
	{
		VectorSet(forwardInv, 1.0, 0.0, 0.0);
		BackSub(a,indx,forwardInv,3);
		VectorSet(rightInv, 0.0, 1.0, 0.0);
		BackSub(a,indx,rightInv,3);
		VectorSet(upInv, 0.0, 0.0, 1.0);
		BackSub(a,indx,upInv,3);
	}
	else
	{
		VectorClear(forwardInv);
		VectorClear(rightInv);
		VectorClear(upInv);
	}
	AngleVectors(pusher->s.angles, forward, right, up);
#else
	VectorNegate(amove, org);
	AngleVectors(org, forward, right, up);
#endif

// save the pusher's original position
	pushed_p->ent = pusher;

	VectorCopy(pusher->s.origin, pushed_p->origin);
	VectorCopy(pusher->s.angles, pushed_p->angles);

	if (pusher->client)
	{
		pushed_p->deltayaw = pusher->client->ps.pmove.delta_angles[YAW];
	}

	pushed_p++;

// move the pusher to it's final position
#if GIL_PUSH
	VectorCopy(pusher->s.origin,OrgOrg);
#endif
	Vec3AddAssign(move, pusher->s.origin);
	Vec3AddAssign(amove, pusher->s.angles);

	gi.linkentity(pusher);

// see if any solid entities are inside the final position
	check = g_edicts+1;

	for(e = 1; e < globals.num_edicts; ++e, ++check)
	{
		if (!check->inuse)
		{
			continue;
		}

		switch(check->movetype)
		{
		case MOVETYPE_PUSH:
		case MOVETYPE_STOP:
		case MOVETYPE_NONE:
		case MOVETYPE_NOCLIP:
			continue;
		}

		// if the entity is standing on the pusher, it will definitely be moved
		if (check->groundentity != pusher)
		{
			// see if the self needs to be tested
			if (check->absmin[0] >= maxs[0]
			|| check->absmin[1] >= maxs[1]
			|| check->absmin[2] >= maxs[2]
			|| check->absmax[0] <= mins[0]
			|| check->absmax[1] <= mins[1]
			|| check->absmax[2] <= mins[2])
				continue;

			// see if the self's bbox is inside the pusher's final position
			if (!TestEntityPosition(check))
			{
				continue;
			}
		}

		if ((pusher->movetype == MOVETYPE_PUSH) || (check->groundentity == pusher))
		{
			// move this entity
			pushed_p->ent = check;

			VectorCopy(check->s.origin, pushed_p->origin);
			VectorCopy(check->s.angles, pushed_p->angles);
			VectorCopy(check->s.origin, holdOrg);

			++pushed_p;

			// try moving the contacted entity

			if (check->client)
			{	// FIXME: doesn't rotate monsters?
				check->client->ps.pmove.delta_angles[YAW] += amove[YAW];
			}
			// figure movement due to the pusher's amove
#if GIL_PUSH
			{
				int test;
				vec3_t testpnt;
				for (test=0;test<4;test++)
				{
					VectorCopy(holdOrg,testpnt);
					VectorCopy(holdOrg,check->s.origin);
					testpnt[2]+=check->mins[2];
					if (test&1)
						testpnt[0]+=check->mins[0];
					else
						testpnt[0]+=check->maxs[0];
					if (test&2)
						testpnt[1]+=check->mins[1];
					else
						testpnt[1]+=check->maxs[1];

					VectorSubtract(testpnt, OrgOrg, org);
					Vec3AddAssign(move, check->s.origin);
					org2[0] = DotProduct(org, forward);
					org2[1] = -DotProduct(org, right);
					org2[2] = DotProduct(org, up);
					move2[0]=DotProduct(org2,forwardInv)-org[0];
					move2[1]=DotProduct(org2,rightInv)-org[1];
					move2[2]=DotProduct(org2,upInv)-org[2];
					Vec3AddAssign(move2, check->s.origin);

					// may have pushed them off an edge
					if (check->groundentity != pusher)
					{
						check->groundentity = NULL;
					}

					block = TestEntityPosition(check);
					if (!block)
						break;
				}
			}
#else
			Vec3AddAssign(move, check->s.origin);
			VectorSubtract(check->s.origin, pusher->s.origin, org);
			org2[0] = DotProduct(org, forward);
			org2[1] = -DotProduct(org, right);
			org2[2] = DotProduct(org, up);
			VectorSubtract(org2, org, move2);
			Vec3AddAssign(move2, check->s.origin);

			// may have pushed them off an edge
			if (check->groundentity != pusher)
			{
				check->groundentity = NULL;
			}

			block = TestEntityPosition(check);
#endif

			if (!block)
			{	// pushed ok
				gi.linkentity(check);

				if (check->client)
				{
					check->client->playerinfo.flags|=PLAYER_FLAG_USE_ENT_POS;
				}

				// impact?
				continue;
			}
			// if it is ok to leave in the old position, do it
			// this is only relevent for riding entities, not pushed

			// No good, (A+B)-B!=A
			//Vec3SubtractAssign(move, check->s.origin);
			//Vec3SubtractAssign(move2, check->s.origin);

			VectorCopy(holdOrg,check->s.origin);
			block = TestEntityPosition(check);

			if (!block)
			{
				--pushed_p;
				continue;
			}
		}

		// save off the obstacle so we can call the block function
		obstacle = check;

		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for(p = pushed_p - 1; p >= pushed; --p)
		{
			VectorCopy(p->origin, p->ent->s.origin);
			VectorCopy(p->angles, p->ent->s.angles);

			if (p->ent->client)
			{
				p->ent->client->ps.pmove.delta_angles[YAW] = p->deltayaw;
			}

			gi.linkentity(p->ent);
		}

		return false;
	}

	//FIXME: is there a better way to handle this?
	// see if anything we moved has touched a trigger
	for(p = pushed_p - 1; p >= pushed; --p)
	{
		ActivateTriggers (p->ent);
	}

	return true;
}

/*
================
Physics_Push

Bmodel objects don't interact with each other, but
push all box objects
================
*/
void Physics_Push(edict_t *self)
{
	vec3_t		move, amove;
	edict_t		*part, *mv;

	// Not a team captain, so movement will be handled elsewhere
	if (self->flags & FL_TEAMSLAVE)
	{
		return;
	}

	// make sure all team slaves can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
	pushed_p = pushed;

	for(part = self; part; part = part->teamchain)
	{
		if (Vec3NotZero(part->velocity) || Vec3NotZero(part->avelocity))
		{	// object is moving
			VectorScale (part->velocity, FRAMETIME, move);
			VectorScale (part->avelocity, FRAMETIME, amove);

			if (!PushEntities(part, move, amove))
			{
				break;	// move was blocked
			}
		}
	}

	if (pushed_p > &pushed[MAX_EDICTS])
	{
		gi.error (ERR_FATAL, "pushed_p > &pushed[MAX_EDICTS], memory corrupted");
	}

	if (part)
	{
		// the move failed, bump all nextthink times and back out moves
		for(mv = self; mv; mv = mv->teamchain)
		{
			if (mv->nextthink > 0)
			{
				mv->nextthink += FRAMETIME;
			}
		}

		// if the pusher has a "blocked" function, call it
		// otherwise, just stay in place until the obstacle is gone
		if (part->blocked)
		{
			part->blocked(part, obstacle);
		}
	}
	else
	{
		// the move succeeded, so call all think functions
		for(part = self; part; part = part->teamchain)
		{
			if (ThinkTime(part))
			{
				part->think(part);
			}
		}
	}
}

static void Physics_ScriptAngular(edict_t *self)
{
	vec3_t		forward, dest, angle;

	if (self->owner)
	{
		VectorAdd(self->owner->s.angles, self->moveinfo.start_angles, angle);
		if (self->moveinfo.state & 1)
		{
			AngleVectors(angle, NULL, forward, NULL);
		}
		else
		{
			AngleVectors(angle, NULL, NULL, forward);
		}
		VectorInverse(forward);
		VectorMA(self->moveinfo.start_origin, self->moveinfo.distance, forward, dest);
		// Distance moved this frame
		Vec3SubtractAssign(self->s.origin, dest);
		// Hence velocity
		VectorScale(dest, (1.0 / FRAMETIME), self->velocity);

		if (self->moveinfo.state & 1)
		{
			VectorCopy(angle, self->s.angles);
		}
	}
	Physics_Push(self);
}

