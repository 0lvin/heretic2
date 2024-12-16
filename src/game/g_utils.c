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
 * Misc. utility functions for the game logic.
 *
 * =======================================================================
 */

#include "header/local.h"
#include "common/fx.h"
#include "header/g_skeletons.h"
#include "common/h2rand.h"
#include "header/g_physics.h"

#define MAXCHOICES 8

static vec3_t VEC_UP = {0, -1, 0};
static vec3_t MOVEDIR_UP = {0, 0, 1};
static vec3_t VEC_DOWN = {0, -2, 0};
static vec3_t MOVEDIR_DOWN = {0, 0, -1};

void
G_ProjectSource(vec3_t point, vec3_t distance, vec3_t forward,
		vec3_t right, vec3_t result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] +
				distance[2];
}

void
G_ProjectSource2(vec3_t point, vec3_t distance, vec3_t forward,
		vec3_t right, vec3_t up, vec3_t result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1] +
				up[0] * distance[2];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1] +
				up[1] * distance[2];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] +
				up[2] * distance[2];
}

/*
 * Searches all active entities for the next
 * one that holds the matching string at fieldofs
 * (use the FOFS() macro) in the structure.
 *
 * Searches beginning at the edict after from, or
 * the beginning. If NULL, NULL will be returned
 * if the end of the list is reached.
 */
edict_t *
G_Find(edict_t *from, int fieldofs, const char *match)
{
	char *s;

	if (!match)
	{
		return NULL;
	}

	if (!from)
	{
		from = g_edicts;
	}
	else
	{
		from++;
	}

	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
		{
			continue;
		}

		s = *(char **)((byte *)from + fieldofs);

		if (!s)
		{
			continue;
		}

		if (!Q_stricmp(s, match))
		{
			return from;
		}
	}

	return NULL;
}

/*
 * Returns entities that have origins
 * within a spherical area
 */
edict_t *
findradius(edict_t *from, vec3_t org, float rad)
{
	vec3_t eorg;
	int j;

	if (!from)
	{
		from = g_edicts;
	}
	else
	{
		from++;
	}

	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
		{
			continue;
		}

		if (from->solid == SOLID_NOT)
		{
			continue;
		}

		for (j = 0; j < 3; j++)
		{
			eorg[j] = org[j] - (from->s.origin[j] +
					   (from->mins[j] + from->maxs[j]) * 0.5);
		}

		if (VectorLength(eorg) > rad)
		{
			continue;
		}

		return from;
	}

	return NULL;
}

/*
 * Returns entities that have origins within a spherical area
 */
edict_t *
findradius2(edict_t *from, vec3_t org, float rad)
{
	/* rad must be positive */
	vec3_t eorg;
	int j;

	if (!from)
	{
		from = g_edicts;
	}
	else
	{
		from++;
	}

	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
		{
			continue;
		}

		if (from->solid == SOLID_NOT)
		{
			continue;
		}

		if (!from->takedamage)
		{
			continue;
		}

		for (j = 0; j < 3; j++)
		{
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j]) * 0.5);
		}

		if (VectorLength(eorg) > rad)
		{
			continue;
		}

		return from;
	}

	return NULL;
}

/*
 * Searches all active entities for
 * the next one that holds the matching
 * string at fieldofs (use the FOFS() macro)
 * in the structure.
 *
 * Searches beginning at the edict after from,
 * or the beginning. If NULL, NULL will be
 * returned if the end of the list is reached.
 */
edict_t *
G_PickTarget(char *targetname)
{
	edict_t *ent = NULL;
	int num_choices = 0;
	edict_t *choice[MAXCHOICES];

	if (!targetname)
	{
		gi.dprintf("G_PickTarget called with NULL targetname\n");
		return NULL;
	}

	while (1)
	{
		ent = G_Find(ent, FOFS(targetname), targetname);

		if (!ent)
		{
			break;
		}

		choice[num_choices++] = ent;

		if (num_choices == MAXCHOICES)
		{
			break;
		}
	}

	if (!num_choices)
	{
		gi.dprintf("G_PickTarget: target %s not found\n", targetname);
		return NULL;
	}

	return choice[randk() % num_choices];
}

void
Think_Delay(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	G_UseTargets(ent, ent->activator);
	G_FreeEdict(ent);
}

/*
 * The global "activator" should be set to
 * the entity that initiated the firing.
 *
 * If self.delay is set, a DelayedUse entity
 * will be created that will actually do the
 * SUB_UseTargets after that many seconds have passed.
 *
 * Centerprints any self.message to the activator.
 *
 * Search for (string)targetname in all entities that
 * match (string)self.target and call their .use function
 */
void
G_UseTargets(edict_t *ent, edict_t *activator)
{
	edict_t *t;


	if (!ent)
	{
		return;
	}

	/* check for a delay */
	if (ent->delay)
	{
		/* create a temp object to fire at a later time */
		t = G_Spawn();
		t->movetype = MOVETYPE_NONE;
		t->classname = "DelayedUse";
		t->nextthink = level.time + ent->delay;
		t->think = Think_Delay;
		t->activator = activator;

		if (!activator)
		{
			gi.dprintf("Think_Delay with no activator\n");
		}

		t->message = ent->message;
		t->text_msg = ent->text_msg;
		t->target = ent->target;
		t->killtarget = ent->killtarget;
		return;
	}

	/* print the message */
	if (activator && (ent->message) && !(activator->svflags & SVF_MONSTER))
	{
		int sound_index;

		if (ent->noise_index)
		{
			sound_index = ent->noise_index;
		}
		else
		{
			sound_index = gi.soundindex("misc/talk1.wav");
		}

		gi.centerprintf(activator, "%s", LocalizationMessage(ent->message, &sound_index));
		gi.sound(activator, CHAN_AUTO, sound_index, 1, ATTN_NORM, 0);
	}

	if (activator && (ent->text_msg) && !(activator->svflags & SVF_MONSTER))
	{
		gi.centerprintf (activator, "%s", ent->text_msg);
	}

	/* kill killtargets */
	if (ent->killtarget)
	{
		t = NULL;

		while ((t = G_Find(t, FOFS(targetname), ent->killtarget)))
		{
			G_QPostMessage(t,MSG_DEATH,PRI_DIRECTIVE,"eeei",t,ent,activator,100000);

			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using killtargets\n");
				return;
			}
		}
	}

	/* fire targets */
	if (ent->target)
	{
		t = NULL;

		while ((t = G_Find(t, FOFS(targetname), ent->target)))
		{
			/* doors fire area portals in a specific way */
			if (!Q_stricmp(t->classname, "func_areaportal") &&
				(!Q_stricmp(ent->classname, "func_door") ||
				 !Q_stricmp(ent->classname, "func_door_rotating")))
			{
				continue;
			}

			if (t == ent)
			{
				gi.dprintf ("WARNING: %s used itself.\n", t->classname);
			}
			else
			{
				if (t->use)
				{
					t->use(t, ent, activator);
				}
			}

			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using targets\n");
				return;
			}
		}
	}
}

/*
 * This is just a convenience function
 * for making temporary vectors for function calls
 */
float *
tv(float x, float y, float z)
{
	static int index;
	static vec3_t vecs[8];
	float *v;

	/* use an array so that multiple
	   tempvectors won't collide
	   for a while */
	v = vecs[index];
	index = (index + 1) & 7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}

/*
 * This is just a convenience function
 * for printing vectors
 */
char *
vtos(vec3_t v)
{
	static int index;
	static char str[8][32];
	char *s;

	/* use an array so that multiple vtos won't collide */
	s = str[index];
	index = (index + 1) & 7;

	Com_sprintf(s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

	return s;
}

void
get_normal_vector(const cplane_t *p, vec3_t normal)
{
	if (p)
	{
		VectorCopy(p->normal, normal);
	}
	else
	{
		VectorCopy(vec3_origin, normal);
	}
}

void
G_SetMovedir(vec3_t angles, vec3_t movedir)
{
	if (VectorCompare(angles, VEC_UP))
	{
		VectorCopy(MOVEDIR_UP, movedir);
	}
	else if (VectorCompare(angles, VEC_DOWN))
	{
		VectorCopy(MOVEDIR_DOWN, movedir);
	}
	else
	{
		AngleVectors(angles, movedir, NULL, NULL);
	}

	VectorClear(angles);
}

float
vectoyaw(vec3_t vec)
{
	float yaw;

	if (vec[YAW] == 0 && vec[PITCH] == 0)
	{
		yaw = 0;
	}
	else
	{
		yaw = (float) (atan2(vec[YAW], vec[PITCH]) * (180 / M_PI));
		if (yaw < 0)
		{
			yaw += 360;
		}
	}

	return yaw;
}

float
vectoyaw2(vec3_t vec)
{
	float yaw;

	if (vec[PITCH] == 0)
	{
		if (vec[YAW] == 0)
		{
			yaw = 0;
		}
		else if (vec[YAW] > 0)
		{
			yaw = 90;
		}
		else
		{
			yaw = 270;
		}
	}
	else
	{
		yaw = (atan2(vec[YAW], vec[PITCH]) * 180 / M_PI);

		if (yaw < 0)
		{
			yaw += 360;
		}
	}

	return yaw;
}

void
vectoangles(vec3_t value1, vec3_t angles)
{
	float forward;
	float yaw, pitch;

	if ((value1[1] == 0) && (value1[0] == 0))
	{
		yaw = 0;

		if (value1[2] > 0)
		{
			pitch = 90;
		}
		else
		{
			pitch = 270;
		}
	}
	else
	{
		if (value1[0])
		{
			yaw = (int)(atan2(value1[1], value1[0]) * 180 / M_PI);
		}
		else if (value1[1] > 0)
		{
			yaw = 90;
		}
		else
		{
			yaw = -90;
		}

		if (yaw < 0)
		{
			yaw += 360;
		}

		forward = sqrt(value1[0] * value1[0] + value1[1] * value1[1]);
		pitch = (int)(atan2(value1[2], forward) * 180 / M_PI);

		if (pitch < 0)
		{
			pitch += 360;
		}
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

void
vectoangles2(vec3_t value1, vec3_t angles)
{
	float forward;
	float yaw, pitch;

	if ((value1[1] == 0) && (value1[0] == 0))
	{
		yaw = 0;

		if (value1[2] > 0)
		{
			pitch = 90;
		}
		else
		{
			pitch = 270;
		}
	}
	else
	{
		if (value1[0])
		{
			yaw = (atan2(value1[1], value1[0]) * 180 / M_PI);
		}
		else if (value1[1] > 0)
		{
			yaw = 90;
		}
		else
		{
			yaw = 270;
		}

		if (yaw < 0)
		{
			yaw += 360;
		}

		forward = sqrt(value1[0] * value1[0] + value1[1] * value1[1]);
		pitch = (atan2(value1[2], forward) * 180 / M_PI);

		if (pitch < 0)
		{
			pitch += 360;
		}
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

char *
G_CopyString(char *in)
{
	char *out;

	if (!in)
	{
		return NULL;
	}

	out = gi.TagMalloc(strlen(in) + 1, TAG_LEVEL);
	strcpy(out, in);
	return out;
}

void
G_InitEdict(edict_t *e)
{
	if (!e)
	{
		return;
	}

	if (e->nextthink)
	{
		e->nextthink = 0;
	}

	e->inuse = true;
	e->classname = "noclass";
	e->gravity = 1.0;
	e->s.number = e - g_edicts;

	e->s.clientEffects.buf = NULL;
	e->s.clientEffects.bufSize = 0;
	e->s.clientEffects.freeBlock = 0;
	e->s.clientEffects.numEffects = 0;
	e->movetype = MOVETYPE_NONE;
	e->friction = 1.0;
	e->elasticity = ELASTICITY_SLIDE;
	VectorSet(e->rrs.scale, 1.0, 1.0, 1.0);
	e->msgHandler = NULL;
	e->svflags = 0;
	e->reflected_time = level.time;
}

/*
=================
G_Spawn

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_t *
G_Spawn(void)
{
	int			i;
	edict_t		*e;
	//static unsigned int entID = 0;

	e = &g_edicts[(int)maxclients->value+1];
	for(i=maxclients->value + 1; i < globals.num_edicts; ++i, ++e)
	{
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if(!e->inuse && e->freetime <= level.time)
		{
			G_InitEdict(e);

			++e->s.usageCount;
			return e;
		}
	}

	if (i == game.maxentities)
	{
		assert(0);
		gi.error("%s: Spawning more than %d edicts",
			__func__, game.maxentities);
	}

	globals.num_edicts++;
	G_InitEdict(e);
	return e;
}

/*
 * Marks the edict as free
 */
void
G_FreeEdict(edict_t *ed)
{
	SinglyLinkedList_t msgs;
	char *temp;
	unsigned int	usageCount;
	int		entnum;

	if (!ed)
	{
		return;
	}

	gi.unlinkentity(ed);		// unlink from world

	// From Quake2 3.17 code release.

	if ((ed - g_edicts) <= (maxclients->value + BODY_QUEUE_SIZE))
	{
		gi.dprintf("tried to free special edict\n");
		return;
	}

	// Start non-quake2.

	// Portals need to be marked as open even if they are freed in deathmatch, only when deliberately removed for netplay.
	if (ed->classname && level.time <= 0.2)			// Just upon startup
	{
		if (Q_stricmp(ed->classname, "func_areaportal") == 0)
			gi.SetAreaPortalState (ed->style, true);
	}

	if(ed->s.effects & EF_JOINTED)
	{
		FreeSkeleton(ed->s.rootJoint);
	}

	if(ed->s.clientEffects.buf)
	{
		temp = (char *)ed->s.clientEffects.buf; // buffer needs to be stored to be cleared by the engine
	}
	else
	{
		temp = NULL;
	}

	msgs = ed->msgQ.msgs;
	usageCount = ed->s.usageCount;
	entnum = ed->s.number;

	// End non-quake2.

	memset(ed, 0, sizeof(*ed));

	// Start non-quake2.

	ed->s.usageCount = usageCount;
	ed->msgQ.msgs = msgs;
	ed->s.clientEffects.buf = (byte *)temp;
	ed->s.number = entnum;

	// End non-quake2.

	ed->classname = "freed";
	ed->freetime = level.time + 2.0;
	ed->inuse = false;
	ed->s.skeletalType = SKEL_NULL;

	ed->svflags = SVF_NOCLIENT;	// so it will get removed from the client properly
}

void
G_TouchTriggers(edict_t *ent)
{
	int i, num;
	edict_t *touch[MAX_EDICTS], *hit;

	if (!ent)
	{
		return;
	}

	/* dead things don't activate triggers! */
	if ((ent->client || (ent->svflags & SVF_MONSTER)) && (ent->health <= 0))
	{
		return;
	}

	num = gi.BoxEdicts(ent->absmin, ent->absmax, touch,
			MAX_EDICTS, AREA_TRIGGERS);

	/* be careful, it is possible to have an entity in this
	   list removed before we get to it (killtriggered) */
	for (i = 0; i < num; i++)
	{
		hit = touch[i];

		if (!hit->inuse)
		{
			continue;
		}

		if (!hit->touch)
		{
			continue;
		}

		hit->touch(hit, ent, NULL, NULL);
	}
}

/*
 * Call after linking a new trigger
 * in during gameplay to force all
 * entities it covers to immediately
 * touch it
 */
void
G_TouchSolids(edict_t *ent)
{
	int i, num;
	edict_t *touch[MAX_EDICTS], *hit;

	if (!ent)
	{
		return;
	}

	num = gi.BoxEdicts(ent->absmin, ent->absmax, touch,
			MAX_EDICTS, AREA_SOLID);

	/* be careful, it is possible to have an entity in this
	   list removed before we get to it (killtriggered) */
	for (i = 0; i < num; i++)
	{
		hit = touch[i];

		if (!hit->inuse)
		{
			continue;
		}

		if (ent->touch)
		{
			ent->touch(hit, ent, NULL, NULL);
		}

		if (!ent->inuse)
		{
			break;
		}
	}
}

/*
 * ==============================================================================
 *
 * Kill box
 *
 * ==============================================================================
 */

/*
 * Kills all entities that would touch the
 * proposed new positioning of ent. Ent s
 * hould be unlinked before calling this!
 */
qboolean
KillBox(edict_t *ent)
{
	edict_t *current = NULL;
	vec3_t	mins, maxs;

	// since we can't trust the absmin and absmax to be set correctly on entry, I'll create my own versions

	VectorAdd(ent->s.origin, ent->mins, mins);
	VectorAdd(ent->s.origin, ent->maxs, maxs);

	while (1)
	{
		current = findinbounds(current, mins, maxs);

		// don't allow us to kill the player
		if(current == ent)
			continue;

		// we've checked everything
		if(!current)
			break;

		// nail it
		if (current->takedamage)
			T_Damage (current, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0,
					  DAMAGE_NO_PROTECTION|DAMAGE_AVOID_ARMOR|DAMAGE_HURT_FRIENDLY,MOD_TELEFRAG);

	}

	return true; /* all clear */
}

/*
ClearBBox

returns true if there is nothing in you BBOX
*/

qboolean ClearBBox (edict_t *self)
{
	vec3_t	top, bottom, mins, maxs;
	trace_t	trace;
	VectorSet(mins, self->mins[0], self->mins[1], 0);
	VectorSet(maxs, self->maxs[0], self->maxs[1], 1);
	VectorSet(bottom, self->s.origin[0], self->s.origin[1], self->absmin[2]);
	VectorSet(top, self->s.origin[0], self->s.origin[1], self->absmax[2] - 1);

	trace = gi.trace(top, mins, maxs, bottom, self, self->clipmask);
	if(trace.startsolid || trace.allsolid)
		return false;

	if(trace.fraction == 1.0)
		return true;

	return false;
}

// ========================================
// LinkMissile(edict_t *self)
//
// This is a way to kinda "cheat" the system.
// We don't want missiles to be considered for collision,
// yet we want them to collide with other things.
// So when we link the entity (for rendering, etc) we set
// SOLID_NOT so certain things don't happen.
// ========================================
void G_LinkMissile(edict_t *ed)
{
	int oldsolid;

	oldsolid = ed->solid;

    gi.linkentity(ed);
    ed->solid = (solid_t) oldsolid;
}

void G_SetToFree(edict_t *self)
{
	if(self->PersistantCFX)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_ENTITY);
		self->PersistantCFX = 0;
	}

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
	self->svflags &= ~SVF_NOCLIENT;

	self->next_pre_think = -1;
	self->next_post_think = -1;

	self->takedamage = DAMAGE_NO;
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->touch = NULL;
	self->blocked = NULL;
	self->isBlocked = NULL;
	self->isBlocking = NULL;
	self->bounced = NULL;
	VectorClear(self->mins);
	VectorClear(self->maxs);

	gi.linkentity(self);
}

qboolean PossessCorrectItem(edict_t *ent, gitem_t *item)
{
	edict_t	*t;

	if(!ent->target_ent)
	{
		return false;
	}
	ent = ent->target_ent;
	t = NULL;
	while ((t = G_Find (t, FOFS(targetname), ent->target)))
	{
		// doors fire area portals in a specific way
		if (!Q_stricmp(t->classname, "func_areaportal") &&
			(!Q_stricmp(ent->classname, "func_door") || !Q_stricmp(ent->classname, "func_door_rotating")))
			continue;

		if(t->item == item)
		{
			return true;
		}
	}
	return false;
}

/*
=================
finddistance

Returns entities that have origins within a spherical shell area

finddistance (origin, mindist, maxdist)
=================
*/
edict_t *finddistance (edict_t *from, vec3_t org, float mindist, float maxdist)
{
	static float min2;
	static float max2;
	static vec3_t min;
	static vec3_t max;
	vec3_t	eorg;
	int		j;
	float elen;

	if (!from)
	{
		min2=mindist*mindist;
		max2=maxdist*maxdist;
		VectorCopy(org,min);
		VectorCopy(org,max);
		for (j=0 ; j<3 ; j++)
		{
			min[j]-=maxdist;
			max[j]+=maxdist;
		}
	}
	while (1)
	{
		from=findinbounds(from,min,max);
		if (!from)
			return 0;
		if (!from->inuse)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		elen = DotProduct(eorg,eorg);
		if (elen > max2)
			continue;
		if (elen < min2)
			continue;
		return from;
	}
}

edict_t *
findinbounds(edict_t *from, vec3_t min, vec3_t max)
{
	static edict_t *touchlist[MAX_EDICTS];
	static int index=-1;
	static int num;

	if (!from)
	{
		num = gi.BoxEdicts(min,max, touchlist, MAX_EDICTS, AREA_SOLID);
		index=0;
	}
	else
	{
		assert(touchlist[index]==from);
		// you cannot adjust the pointers yourself...
		// this means you did not call it with the previous edict
		index++;
	}
	for (;index<num;index++)
	{
		if (!touchlist[index]->inuse)
			continue;
		return touchlist[index];
	}
	return NULL;
}

//
//=================
// FindOnPath
//
// Returns damageable entities that lie along a given pathway.  This is NOT 100% guaranteed to return a given edict only once.
//
//=================

edict_t *
findonpath(edict_t *startent, vec3_t startpos, vec3_t endpos, vec3_t mins, vec3_t maxs, vec3_t *resultpos)
{
	vec3_t	vect, curpos;
	trace_t trace;
	float	skipamount;
	edict_t *tracebuddy;

	VectorCopy(startpos, curpos);
	tracebuddy = startent;
	while(1)
	{
		trace = gi.trace(curpos, mins, maxs, endpos, tracebuddy, MASK_SHOT);

		// If we started inside something.
		if (trace.startsolid || trace.allsolid)
		{
			if (trace.ent && trace.ent->takedamage)
			{	// Found an item.  Skip forward a distance and return the ent.
				skipamount = maxs[2];
				if (skipamount < 4)
					skipamount = 4;
				VectorSubtract(endpos, curpos, vect);
				if (VectorNormalize(vect) < skipamount)	// skip to the end.
					VectorCopy(endpos, *resultpos);
				else
					VectorMA(curpos, skipamount, vect, *resultpos);

				return(trace.ent);
			}
			else
			{	// Didn't stop on anything useful, continue to next trace.
				skipamount = maxs[2];	// Skip forward a bit.
				if (skipamount < 4)
					skipamount = 4;
				VectorSubtract(endpos, curpos, vect);
				if (VectorNormalize(vect) < skipamount)	// skip to the end.
					return(NULL);		// Didn't find anything.
				else
					VectorMA(curpos, skipamount, vect, curpos);
				if (trace.ent)
					tracebuddy = trace.ent;
				continue;	// Do another trace.
			}
		}

		// If we did not start inside something, but stopped at something.
		if (trace.fraction < .99)
		{
			if (trace.ent && trace.ent->takedamage)
			{	// Found an item.  Skip forward a distance and return the ent.
				skipamount = maxs[2];
				if (skipamount < 4)
					skipamount = 4;
				VectorSubtract(endpos, trace.endpos, vect);
				if (VectorNormalize(vect) < skipamount)	// skip to the end.
					VectorCopy(endpos, *resultpos);
				else
					VectorMA(trace.endpos, skipamount, vect, *resultpos);

				return(trace.ent);
			}
			else
			{	// Didn't stop on anything useful, continue to next trace.
				skipamount = maxs[2];	// Skip forward a bit.
				if (skipamount < 4)
					skipamount = 4;
				VectorSubtract(endpos, trace.endpos, vect);
				if (VectorNormalize(vect) < skipamount)	// skip to the end.
					return(NULL);		// Didn't find anything.
				else
					VectorMA(trace.endpos, skipamount, vect, curpos);
				if (trace.ent)
					tracebuddy = trace.ent;
				continue;	// Do another trace.
			}
		}

		// If we finished the whole move.
		{
			VectorCopy(endpos, *resultpos);
			return(NULL);
		}
	};

	return(NULL);	// Never gets here.
}

// THis works like findradius, except it uses the bbox of an ent to indicate the area to check.
edict_t *findinblocking(edict_t *from, edict_t *checkent)
{
	static vec3_t	min, max;

	if (!from)
	{
		VectorAdd(checkent->s.origin, checkent->mins, min);
		VectorAdd(checkent->s.origin, checkent->maxs, max);
	}
	while (1)
	{
		from=findinbounds(from,min,max);
		if (!from)
			return 0;
		if (!from->inuse)
			continue;
		if (from == checkent)
			continue;
		return from;
	}
}

/*
 * Returns entities that have origins
 * within a spherical area
 */
edict_t *
newfindradius(edict_t *from, vec3_t org, float rad)
{
	vec3_t eorg;
	int j;
	static float max2;
	static vec3_t min;
	static vec3_t max;
	float elen;

	if (!from)
	{
		max2=rad*rad;
		VectorCopy(org,min);
		VectorCopy(org,max);
		for (j=0 ; j<3 ; j++)
		{
			min[j]-=rad;
			max[j]+=rad;
		}
	}

	while (1)
	{
		from = findinbounds(from,min,max);
		if (!from)
		{
			return 0;
		}

		if (!from->inuse)
		{
			continue;
		}

		for (j = 0; j < 3; j++)
		{
			eorg[j] = org[j] - (from->s.origin[j] +
					   (from->mins[j] + from->maxs[j]) * 0.5);
		}

		elen = DotProduct(eorg,eorg);
		if (elen > max2)
		{
			continue;
		}

		return from;
	}

	return NULL;
}

unsigned
GenNoDrawInfo(fmnodeinfo_t *fmnodeinfo)
{
	unsigned rr_mesh = 0;
	int i;

	for(i = 0; i < MAX_FM_MESH_NODES; i++)
	{
		if (fmnodeinfo[i].flags & FMNI_NO_DRAW)
		{
			rr_mesh |= (1 << i);
		}
	}
	return rr_mesh;
}
