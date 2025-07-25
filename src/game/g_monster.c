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
 * Monster utility functions. While unused by the CTF code they must
 * persist here since most of the other game codes has ties to it.
 *
 * =======================================================================
 */

#include <limits.h>

#include "header/local.h"
#include "header/g_monster.h"
#include "monster/stats/stats.h"
#include "common/fx.h"
#include "header/g_defaultmessagehandler.h"
#include "header/g_playstats.h"

#include "common/h2rand.h"

//JWEIER START INCLUDES
#include "header/g_misc.h"
#include "header/utilities.h"
//JWEIER END INCLUDES
#include "header/g_hitlocation.h"
#include "header/mg_guide.h"

#define FALLDAMAGE_MODIFIER	8

qboolean defaultMonsterAlerted (edict_t *self, alertent_t *alerter, edict_t *enemy);
// ************************************************************************************************
// MonsterHealth
// -------------
// Returns the modified health for a monster based on the number of players who will be in a game.
// ************************************************************************************************

int
MonsterHealth(int health)
{
	if (skill->value > 1.0)
		health += (int)((float)health * (skill->value - 1.0) / 2.0);	// 150% on Armageddon (skill 3)

	return(health+(health*((game.maxclients-1)*0.25)));		// 175% with 4 players.
}

/*-------------------------------------------------------------------------
	DeadMsgHandler
-------------------------------------------------------------------------*/
void
DeadMsgHandler(edict_t *self, G_Message_t *msg)
{
	G_MsgReceiver_t receiver;

	if(msg->ID!=MSG_DEATH_PAIN)
		return;

	receiver = classStatics[self->classID].msgReceivers[msg->ID];

	if(receiver)
	{
		receiver(self, msg);
	}
	else
	{
		// if and when there are a good number of defaults, change the NULL to be an Empty
		// function, overall that should be faster to just always call the function then
		// do the check
		receiver = DefaultMessageReceivers[msg->ID];

		if(receiver)
		{
			DefaultMessageReceivers[msg->ID](self, msg);
		}
	}
}

/*-------------------------------------------------------------------------
	DyingMsgHandler

	Allows only dimemberment to still be called
-------------------------------------------------------------------------*/

void
DyingMsgHandler(edict_t *self, G_Message_t *msg)
{
	G_MsgReceiver_t receiver;

	if(msg->ID!=MSG_DISMEMBER)
		return;

	receiver = classStatics[self->classID].msgReceivers[msg->ID];

	if(receiver)
	{
		receiver(self, msg);
	}
	else
	{
		// if and when there are a good number of defaults, change the NULL to be an Empty
		// function, overall that should be faster to just always call the function then
		// do the check
		receiver = DefaultMessageReceivers[msg->ID];

		if(receiver)
		{
			DefaultMessageReceivers[msg->ID](self, msg);
		}
	}
}

/*-------------------------------------------------------------------------
	PauseTime
-------------------------------------------------------------------------*/
void
PauseTime(edict_t *self, float time)
{
	self->monsterinfo.pausetime = level.time + time;
}

void
M_MonsterCatPos(edict_t *ent)
{
	vec3_t point;
	int cont;

//
// get waterlevel
//
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] + ent->mins[2] + 1;
	cont = gi.pointcontents (point);

	if (!(cont & MASK_WATER))
	{
		ent->waterlevel = 0;
		ent->watertype = 0;
		return;
	}

	point[2] -= 1;
	ent->watertype = cont;
	ent->waterlevel = 1;//below knees
	point[2] += ent->size[2] * 0.25;// quarter of way up
	cont = gi.pointcontents (point);
	if (!(cont & MASK_WATER))
		return;

	ent->waterlevel = 2;//between knees and head
	point[2] = ent->absmax[2];//over head
	cont = gi.pointcontents (point);
	if (cont & MASK_WATER)
		ent->waterlevel = 3;//all the way in
}

void monster_start_go(edict_t *self);

/* Monster weapons */

void
monster_fire_bullet(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int kick, int hspread, int vspread, int flashtype)
{
	if (!self)
	{
		return;
	}

	fire_bullet(self, start, dir, damage, kick, hspread, vspread, MOD_UNKNOWN);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

void
monster_fire_shotgun(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int kick, int hspread, int vspread, int count, int flashtype)
{
	if (!self)
	{
		return;
	}

	fire_shotgun(self, start, aimdir, damage, kick, hspread,
			vspread, count, MOD_UNKNOWN);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

void
monster_fire_blaster(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, int flashtype, int effect)
{
	if (!self)
	{
		return;
	}

	fire_blaster(self, start, dir, damage, speed, effect, false);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

void
monster_fire_blueblaster(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, int flashtype, int effect)
{
	if (!self)
	{
		return;
	}

	fire_blueblaster(self, start, dir, damage, speed, effect);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(MZ_BLUEHYPERBLASTER);
	gi.multicast(start, MULTICAST_PVS);
}

void
monster_fire_blaster2(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, int flashtype, int effect)
{
	if (!self)
	{
		return;
	}

	fire_blaster2(self, start, dir, damage, speed, effect, false);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

void
monster_fire_ionripper(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, int flashtype, int effect)
{
	if (!self)
	{
		return;
	}

	fire_ionripper(self, start, dir, damage, speed, effect);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

void
monster_fire_heat(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, int flashtype)
{
	if (!self)
	{
		return;
	}

	fire_heat(self, start, dir, damage, speed, damage, damage);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

void
monster_fire_tracker(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, edict_t *enemy, int flashtype)
{
	if (!self || !enemy)
	{
		return;
	}

	fire_tracker(self, start, dir, damage, speed, enemy);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

void
monster_fire_heatbeam(edict_t *self, vec3_t start, vec3_t dir, vec3_t offset,
		int damage, int kick, int flashtype)
{
	if (!self)
	{
		return;
	}

	fire_heatbeam(self, start, dir, offset, damage, kick, true);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

void
dabeam_hit(edict_t *self)
{
	edict_t *ignore;
	vec3_t start;
	vec3_t end;
	trace_t tr;

	if (!self)
	{
		return;
	}

	ignore = self;
	VectorCopy(self->s.origin, start);
	VectorMA(start, 2048, self->movedir, end);

	while (1)
	{
		tr = gi.trace(start, NULL, NULL, end, ignore,
				CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER);

		if (!tr.ent)
		{
			break;
		}

		/* hurt it if we can */
		if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) &&
			(tr.ent != self->owner))
		{
			T_Damage(tr.ent, self, self->owner, self->movedir, tr.endpos,
					vec3_origin, self->dmg, skill->value, DAMAGE_ENERGY,
					MOD_TARGET_LASER);
		}

		if (self->dmg < 0) /* healer ray */
		{
			/* when player is at 100 health
			   just undo health fix */
			if (tr.ent->client && (tr.ent->health > 100))
			{
				tr.ent->health += self->dmg;
			}
		}

		/* if we hit something that's not a monster or
		   player or is immune to lasers, we're done */
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		{
			if (self->spawnflags & 0x80000000)
			{
				self->spawnflags &= ~0x80000000;
				gi.WriteByte(svc_temp_entity);
				gi.WriteByte(TE_LASER_SPARKS);
				gi.WriteByte(10);
				gi.WritePosition(tr.endpos);
				gi.WriteDir(tr.plane.normal);
				gi.WriteByte(self->s.skinnum);
				gi.multicast(tr.endpos, MULTICAST_PVS);
			}

			break;
		}

		ignore = tr.ent;
		VectorCopy(tr.endpos, start);
	}

	VectorCopy(tr.endpos, self->s.old_origin);
	self->nextthink = level.time + 0.1;
	self->think = G_FreeEdict;
}

void
monster_dabeam(edict_t *self)
{
	vec3_t last_movedir;
	vec3_t point;

	if (!self)
	{
		return;
	}

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM | RF_TRANSLUCENT;
	self->s.modelindex = 1;

	self->s.frame = 2;

	if (self->owner->monsterinfo.aiflags & AI_MEDIC)
	{
		self->s.skinnum = 0xf3f3f1f1;
	}
	else
	{
		self->s.skinnum = 0xf2f2f0f0;
	}

	if (self->enemy)
	{
		VectorCopy(self->movedir, last_movedir);
		VectorMA(self->enemy->absmin, 0.5, self->enemy->size, point);

		if (self->owner->monsterinfo.aiflags & AI_MEDIC)
		{
			point[0] += sin(level.time) * 8;
		}

		VectorSubtract(point, self->s.origin, self->movedir);
		VectorNormalize(self->movedir);

		if (!VectorCompare(self->movedir, last_movedir))
		{
			self->spawnflags |= 0x80000000;
		}
	}
	else
	{
		G_SetMovedir(self->s.angles, self->movedir);
	}

	self->think = dabeam_hit;
	self->nextthink = level.time + 0.1;
	VectorSet(self->mins, -8, -8, -8);
	VectorSet(self->maxs, 8, 8, 8);
	gi.linkentity(self);

	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
}

void
monster_fire_grenade(edict_t *self, vec3_t start, vec3_t aimdir,
		int damage, int speed, int flashtype)
{
	if (!self)
	{
		return;
	}

	fire_grenade(self, start, aimdir, damage, speed, 2.5, damage + 40);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

void
monster_fire_rocket(edict_t *self, vec3_t start, vec3_t dir,
		int damage, int speed, int flashtype)
{
	if (!self)
	{
		return;
	}

	fire_rocket(self, start, dir, damage, speed, damage + 20, damage);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

void
monster_fire_railgun(edict_t *self, vec3_t start, vec3_t aimdir,
		int damage, int kick, int flashtype)
{
	if (!self)
	{
		return;
	}

	if (!(gi.pointcontents(start) & MASK_SOLID))
	{
		fire_rail(self, start, aimdir, damage, kick);
	}

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

void
monster_fire_bfg(edict_t *self, vec3_t start, vec3_t aimdir,
		int damage, int speed, int kick /* unused */, float damage_radius,
		int flashtype)
{
	if (!self)
	{
		return;
	}

	fire_bfg(self, start, aimdir, damage, speed, damage_radius);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

/* ================================================================== */

/* Monster utility functions */

void
M_FliesOff(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
}

void
M_FliesOn(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (self->waterlevel)
	{
		return;
	}

	self->s.effects |= EF_FLIES;
	self->s.sound = gi.soundindex("infantry/inflies1.wav");
	self->think = M_FliesOff;
	self->nextthink = level.time + 60;
}

void
M_FlyCheck(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (self->waterlevel)
	{
		return;
	}

	if (random() > 0.5)
	{
		return;
	}

	self->think = M_FliesOn;
	self->nextthink = level.time + 5 + 10 * random();
}


void
AttackFinished(edict_t *self, float time)
{
	if (!self)
	{
		return;
	}

	self->monsterinfo.attack_finished = level.time + time;
}

void
M_CheckGround(edict_t *ent)
{
	vec3_t point;
	trace_t trace;

	if (!ent)
	{
		return;
	}

	if (ent->flags & (FL_SWIM | FL_FLY))
	{
		return;
	}

	if (ent->velocity[2] >= 50)
	{
//		ent->groundentity = NULL;
		return;
	}

	/* if the hull point one-quarter unit down
	   is solid the entity is on ground */
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] - 0.25;

	trace = gi.trace(ent->s.origin, ent->mins, ent->maxs, point,
			ent, MASK_MONSTERSOLID);

	/* check steepness */
	if ((trace.plane.normal[2] < 0.7) && !trace.startsolid)
	{
//		ent->groundentity = NULL;
		return;
	}

	if (!trace.startsolid && !trace.allsolid)
	{
		VectorCopy(trace.endpos, ent->s.origin);
		ent->groundentity = trace.ent;
		ent->groundentity_linkcount = trace.ent->linkcount;
		ent->velocity[2] = 0;
	}
}

void
M_CatagorizePosition(edict_t *ent)
{
	vec3_t point;
	int cont;

	if (!ent)
	{
		return;
	}

	if(!ent->client)
	{
		M_MonsterCatPos(ent);
		return;
	}

	/* get waterlevel */
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] + ent->mins[2] + 1;
	cont = gi.pointcontents(point);

	if (!(cont & MASK_WATER))
	{
		ent->waterlevel = 0;
		ent->watertype = 0;
		return;
	}

	ent->watertype = cont;
	ent->waterlevel = 1;
	point[2] += 26;
	cont = gi.pointcontents(point);

	if (!(cont & MASK_WATER))
	{
		return;
	}

	ent->waterlevel = 2;
	point[2] += 22;
	cont = gi.pointcontents(point);

	if (cont & MASK_WATER)
	{
		ent->waterlevel = 3;
	}
}

void
M_WorldEffects(edict_t *ent)
{
	int dmg;

	if (!ent)
	{
		return;
	}

	if (ent->health > 0)
	{
		if (!(ent->flags & FL_SWIM))
		{
			if (ent->waterlevel < 3 || (ent->monsterinfo.aiflags&AI_SWIM_OK))
			{
				ent->air_finished = level.time + M_HOLD_BREATH_TIME;
			}
			else if (ent->air_finished < level.time && !(ent->flags & FL_AMPHIBIAN))
			{
				/* drown! */
				if (ent->pain_debounce_time < level.time)
				{
					dmg = 2 + 2 * floor(level.time - ent->air_finished);

					if (dmg > 15)
					{
						dmg = 15;
					}

					T_Damage(ent, world, world, vec3_origin, ent->s.origin,
							vec3_origin, dmg, 0, DAMAGE_SUFFOCATION,MOD_WATER);
					ent->pain_debounce_time = level.time + 1;
				}
			}
		}
		else
		{
			if (ent->waterlevel > 0 || (ent->monsterinfo.aiflags&AI_SWIM_OK))
			{
				ent->air_finished = level.time + 9;
			}
			else if (ent->air_finished < level.time && !(ent->flags & FL_AMPHIBIAN))
			{
				/* suffocate! */
				if (ent->pain_debounce_time < level.time)
				{
					dmg = 2 + 2 * floor(level.time - ent->air_finished);

					if (dmg > 15)
					{
						dmg = 15;
					}

					T_Damage(ent, world, world, vec3_origin, ent->s.origin,
							vec3_origin, dmg, 0, DAMAGE_SUFFOCATION,MOD_WATER);
					ent->pain_debounce_time = level.time + 1;
				}
			}
		}
	}

	if (ent->waterlevel == 0)
	{
		// INWATER is set whether in lava, slime or water.
		if (ent->flags & FL_INWATER)
		{
			if (ent->flags & FL_INLAVA)
			{
				gi.sound(ent, CHAN_BODY, gi.soundindex("player/inlava.wav"), 1, ATTN_NORM, 0);
				ent->flags &= ~FL_INLAVA;
			}
			else if (ent->flags & FL_INSLIME)
			{
				gi.sound(ent, CHAN_BODY, gi.soundindex("player/muckexit.wav"), 1, ATTN_NORM, 0);
				ent->flags &= ~FL_INSLIME;
			}
			else
			{
				gi.sound(ent, CHAN_BODY, gi.soundindex("player/Water Exit.wav"), 1, ATTN_NORM, 0);
			}

			ent->flags &= ~FL_INWATER;
		}

		return;
	}

	if ((ent->watertype & CONTENTS_LAVA) && !(ent->flags & FL_IMMUNE_LAVA))
	{
		if (ent->damage_debounce_time < level.time)
		{
			ent->damage_debounce_time = level.time + 0.2;
			T_Damage(ent, world, world, vec3_origin, ent->s.origin,
					vec3_origin, 10 * ent->waterlevel, 0, DAMAGE_LAVA,MOD_LAVA);
		}
	}

	if ((ent->watertype & CONTENTS_SLIME) && !(ent->flags & FL_IMMUNE_SLIME))
	{
		if (ent->damage_debounce_time < level.time)
		{
			ent->damage_debounce_time = level.time + 1;
			T_Damage(ent, world, world, vec3_origin, ent->s.origin,
					vec3_origin, 4 * ent->waterlevel, 0, DAMAGE_SLIME,MOD_SLIME);
		}
	}

	if (!(ent->flags & FL_INWATER))
	{
		if (ent->watertype & CONTENTS_LAVA)
		{
			gi.sound(ent, CHAN_BODY, gi.soundindex("player/inlava.wav"), 1, ATTN_NORM, 0);
			ent->flags |= FL_INLAVA;
		}
		else
			if (ent->watertype & CONTENTS_SLIME)
		{
			gi.sound(ent, CHAN_BODY, gi.soundindex("player/muckin.wav"), 1, ATTN_NORM, 0);
			ent->flags |= FL_INSLIME;
		}
		else
		{
			gi.sound(ent, CHAN_BODY, gi.soundindex("player/Water Enter.wav"), 1, ATTN_NORM, 0);
		}

		// INWATER is set whether in lava, slime or water.
		ent->flags |= FL_INWATER;
		ent->damage_debounce_time = 0;
	}
}

void
M_droptofloor(edict_t *ent)
{
	vec3_t end;
	trace_t trace;

	if (!ent)
	{
		return;
	}

	if(Vec3IsZero(ent->mins)&&Vec3IsZero(ent->maxs))
	{
		gi.dprintf("ERROR : %s at %s called drop to floor before having size set\n", ent->classname, vtos(ent->s.origin));
		if(ent->think == M_droptofloor)
			ent->think = NULL;//don't try again
		return;
	}

	ent->nextthink = level.time + FRAMETIME;

	ent->s.origin[2] += 1.0;
	VectorCopy(ent->s.origin, end);
	end[2] -= 256;

	trace = gi.trace(ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

	if(trace.allsolid||trace.startsolid)
	{
		gi.dprintf("ERROR : Object (%s) at %s started in solid\n", ent->classname, vtos(ent->s.origin));
		if(ent->think == M_droptofloor)
		{
			ent->think = NULL;//don't try again
			gi.linkentity(ent);
			M_CatagorizePosition (ent);
		}
		return;
	}

	if (trace.fraction == 1.0)
	{
		gi.dprintf("ERROR : Object (%s) more than 256 off ground, waiting to fall\n", ent->classname);
		return;
	}

	if(mgai_debug->value)
		gi.dprintf("%s at %s dropped to floor at %s\n", ent->classname, vtos(ent->s.origin), vtos(trace.endpos));

	VectorCopy(trace.endpos, ent->s.origin);

	gi.linkentity(ent);
	M_CheckGround(ent);
	M_CatagorizePosition(ent);

	// No need to think anymore if on the ground
	ent->think = NULL;
}

void
M_SetEffects(edict_t *ent)
{
	int remaining;

	if (!ent)
	{
		return;
	}

	ent->s.effects &= ~(EF_COLOR_SHELL | EF_POWERSCREEN | EF_DOUBLE | EF_QUAD | EF_PENT);
	ent->s.renderfx &= ~(RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE);

	if (ent->monsterinfo.aiflags & AI_RESURRECTING)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_RED;
	}

	if (ent->health <= 0)
	{
		return;
	}

	if (ent->powerarmor_time > level.time)
	{
		if (ent->monsterinfo.power_armor_type == POWER_ARMOR_SCREEN)
		{
			ent->s.effects |= EF_POWERSCREEN;
		}
		else if (ent->monsterinfo.power_armor_type == POWER_ARMOR_SHIELD)
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}
	}

	if (ent->monsterinfo.quad_framenum > level.framenum)
	{
		remaining = ent->monsterinfo.quad_framenum - level.framenum;

		if ((remaining > 30) || (remaining & 4))
		{
			ent->s.effects |= EF_QUAD;
		}
	}
	else
	{
		ent->s.effects &= ~EF_QUAD;
	}

	if (ent->monsterinfo.double_framenum > level.framenum)
	{
		remaining = ent->monsterinfo.double_framenum - level.framenum;

		if ((remaining > 30) || (remaining & 4))
		{
			ent->s.effects |= EF_DOUBLE;
		}
	}
	else
	{
		ent->s.effects &= ~EF_DOUBLE;
	}

	if (ent->monsterinfo.invincible_framenum > level.framenum)
	{
		remaining = ent->monsterinfo.invincible_framenum - level.framenum;

		if ((remaining > 30) || (remaining & 4))
		{
			ent->s.effects |= EF_PENT;
		}
	}
	else
	{
		ent->s.effects &= ~EF_PENT;
	}
}

static void
M_SetAnimGroupFrameValues(edict_t *self, const char *name,
	int *ofs_frames, int *num_frames)
{
	const dmdxframegroup_t * frames;
	int num, i;

	frames = gi.GetModelInfo(self->s.modelindex, &num, NULL, NULL);
	for (i = 0; i < num; i++)
	{
		if (!strcmp(frames[i].name, name))
		{
			*ofs_frames = frames[i].ofs;
			*num_frames = frames[i].num;
			break;
		}
	}
}

void
M_SetAnimGroupFrame(edict_t *self, const char *name)
{
	int i, ofs_frames = 0, num_frames = 1;

	M_SetAnimGroupFrameValues(self, name, &ofs_frames, &num_frames);

	i = self->s.frame - ofs_frames;
	if (i < 0)
	{
		i = 0;
	}
	i++;

	self->s.frame = ofs_frames + i % num_frames;
	gi.GetModelFrameInfo(self->s.modelindex, self->s.frame,
		self->mins, self->maxs);
}

/* ------------------------------------------------------------------------------
	M_MoveFrame - unless a nextframe is specified, advance to the next frame listed in
	the Animation Frame Array.   Execute any aifunction or think function specified
	with the given frame.
 --------------------------------------------------------------------------------*/
extern qboolean MonsterAdvanceFrame;

void
M_MoveFrame(edict_t *self)
{
	mmove_t *move;
	int index;

	if (!self)
	{
		return;
	}

	move = self->monsterinfo.currentmove;

	if (!move)
	{
		/* if move is NULL, then this monster needs to have an anim set on it or all is lost. */
		gi.dprintf("MONSTER: '%s', at %s has no move pointer.  Setting to move zero.\n", self->classname, self->s.origin);
		self->think = NULL;
		self->nextthink = -1;
		return;
	}

	self->nextthink = level.time + self->monsterinfo.thinkinc;

	//There is a voice sound waiting to play
	if (self->monsterinfo.sound_pending && self->monsterinfo.sound_start <= level.time)
	{
		//Post a message and make the monster speak
		G_QPostMessage(self, MSG_VOICE_PUPPET, PRI_DIRECTIVE, "i", self->monsterinfo.sound_pending);

		//Sound queue is free
		self->monsterinfo.sound_pending = 0;
	}

	// If this is set, the monster runs absolutely no animations or ai
	if (sv_freezemonsters->value != 0 && !MonsterAdvanceFrame)
		return;

	// Forcing the next frame index - usually the start of an animation
	if (self->monsterinfo.nextframeindex > -1)
	{
		self->monsterinfo.currframeindex = self->monsterinfo.nextframeindex;
		self->monsterinfo.nextframeindex = -1;
	}
	else
	{
		if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
		{
			++self->monsterinfo.currframeindex;
			if (self->monsterinfo.currframeindex >= move->numframes)
				self->monsterinfo.currframeindex = 0;
		}

		if (self->monsterinfo.currframeindex == (move->numframes - 1))
		{
			if (move->endfunc)
			{
				move->endfunc(self);

				/* regrab move, endfunc is very likely to change it */
				move = self->monsterinfo.currentmove;

				/* check for death */
				if (self->svflags & SVF_DEADMONSTER)
				{
					return;
				}
			}
		}
	}

	index = self->monsterinfo.currframeindex;
	self->s.frame = move->frame[index].framenum;

	//this is consistent with the mmove_t in the monster anims.
	//currently all of the *real* movement happens in the
	//"actionfunc" instead of the move func
	if(!(self->monsterinfo.aiflags & AI_DONT_THINK))
	{
		if(move->frame[index].movefunc)
		{
			move->frame[index].movefunc(self, move->frame[index].var1, move->frame[index].var2,
				move->frame[index].var3);
		}

		if (move->frame[index].actionfunc)
		{
			if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
			{//Put scaling into SV_Movestep since this isn't ALWAYS the movement function
				move->frame[index].actionfunc (self, move->frame[index].var4);//* self->monsterinfo.scale);
			}
			else
				move->frame[index].actionfunc (self, 0);
		}

		if (move->frame[index].thinkfunc)
		{
			move->frame[index].thinkfunc(self);
		}
	}
}

void
monster_think(edict_t *self)
{
	if (!self)
	{
		return;
	}

	M_MoveFrame(self);
/*
	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround(self);
	}
*/
	M_CatagorizePosition(self);
	M_WorldEffects(self);
}

/*
 * Using a monster makes it angry
 * at the current activator
 */
void
monster_use(edict_t *self, edict_t *other /* unused */, edict_t *activator)
{
	if (!self || !activator)
	{
		return;
	}

	if (self->enemy)
	{
		return;
	}

	if (self->health <= 0)
	{
		return;
	}

	if (activator->flags & FL_NOTARGET)
	{
		return;
	}

	if (!(activator->client) && !(activator->monsterinfo.aiflags & AI_GOOD_GUY))
	{
		return;
	}

	/* delay reaction so if the monster is
	   teleported, its sound is still heard */
	self->enemy = activator;
	FoundTarget(self, true);
}


void monster_start_go (edict_t *self);

void
monster_triggered_spawn(edict_t *self)
{
	vec3_t	pos;

	if (!self)
	{
		return;
	}

	self->s.origin[2] += 1;
	KillBox(self);

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_STEP;
	self->svflags &= ~SVF_NOCLIENT;
	self->air_finished = level.time + M_HOLD_BREATH_TIME;
	gi.linkentity(self);

	monster_start_go(self);

	if (strcmp(self->classname, "monster_fixbot") == 0)
	{
		if (self->spawnflags & 16 || self->spawnflags & 8 || self->spawnflags &
			4)
		{
			self->enemy = NULL;
			return;
		}
	}

	if((self->classID == CID_ASSASSIN) && (self->spawnflags & MSF_ASS_TPORTAMBUSH))
	{
		FoundTarget (self, true);
		VectorCopy(self->s.origin, pos);
		pos[2]+=self->mins[2];
		gi.CreateEffect(NULL, FX_TPORTSMOKE, 0, pos, "");
	}
	else if (self->enemy && !(self->spawnflags & MSF_AMBUSH) && !(self->enemy->flags & FL_NOTARGET))
	{
		FoundTarget(self, true);
	}
	else
	{
		self->enemy = NULL;
	}
}

void
monster_triggered_spawn_use(edict_t *self, edict_t *other /* unused */, edict_t *activator)
{
	if (!self || !activator)
	{
		return;
	}

	/* we have a one frame delay here so we
	   don't telefrag the guy who activated us */
	self->spawnflags &= ~MSF_ASLEEP;
	self->think = monster_triggered_spawn;
	self->nextthink = level.time + FRAMETIME;

	if (activator->client)
	{
		self->enemy = activator;
	}

	self->use = monster_use;
}

void
monster_triggered_start(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
	self->use = monster_triggered_spawn_use;
}

/*
 * When a monster dies, it fires all of its targets
 * with the current enemy as activator.
 */
void
monster_death_use(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->flags &= ~(FL_FLY | FL_SWIM);
//	self->monsterinfo.aiflags &= AI_GOOD_GUY;//WHY mask out everything above this flag???

	if (self->item)
	{
		Drop_Item(self, self->item);
		self->item = NULL;
	}

	if (self->deathtarget)
	{
		self->target = self->deathtarget;
	}

	if (!self->target)
	{
		return;
	}

	G_UseTargets(self, self->enemy);
}

/* ================================================================== */

void
MG_CheckInGround(edict_t *self)
{
	if(gi.pointcontents(self->s.origin)&CONTENTS_SOLID)
	{
		gi.dprintf("%s's origin at %s in solid!!!\n", self->classname, vtos(self->s.origin));
	}
	else
	{//check down against world- does not check against entities! Does not check up against cieling (why would they put one close to a cieling???)
		vec3_t		top, bottom, mins, maxs;
		trace_t		trace;

		VectorCopy(self->s.origin, top);
		VectorCopy(self->s.origin, bottom);
		top[2] += self->maxs[2] - 1;
		bottom[2] += self->mins[2];

		VectorSet(mins, self->mins[0], self->mins[1], 0);
		VectorSet(maxs, self->maxs[0], self->maxs[1], 1);

		trace = gi.trace(top, mins, maxs, bottom, self, MASK_SOLID);
		if(trace.allsolid || trace.startsolid)//monster in solid, can't be fixed
		{
			gi.dprintf("top of %s at %s in solid architecture(%s)!!!\n", self->classname, vtos(self->s.origin), trace.ent->classname);
		}
		else if(trace.fraction < 1.0f)
		{//buoy is in the ground
			VectorCopy(trace.endpos, bottom);
			bottom[2] -= self->mins[2];
			if((int)(trace.endpos[2]) != (int)(self->s.origin[2]))
				gi.dprintf("%s at %s was in ground(%s), moved to %s...!!!\n", self->classname, vtos(self->s.origin), trace.ent->classname, vtos(bottom));
			VectorCopy(bottom, self->s.origin);
		}
		//fixme- check against other ents too? same trace or second one?
	}
}

static void
M_FixStuckMonster(edict_t *self)
{
	trace_t tr;

	tr = gi.trace(self->s.origin, self->mins, self->maxs, self->s.origin, self, MASK_SOLID);
	if (!tr.startsolid)
	{
		return;
	}

	FixEntityPosition(self);

	tr = gi.trace(self->s.origin, self->mins, self->maxs, self->s.origin, self, MASK_SOLID);

	if (tr.startsolid)
	{
		gi.dprintf("%s: %s startsolid at %s\n",
				__func__,
				self->classname,
				vtos(self->s.origin));
	}
}

qboolean
monster_start(edict_t *self)
{
	float scale;
	int i;

	if (!self)
	{
		return false;
	}

	if (deathmatch->value && !((int)sv_cheats->value & self_spawn))
	{
		G_FreeEdict(self);
		return false;
	}

	if (!(self->monsterinfo.aiflags & AI_GOOD_GUY))
	{
		level.total_monsters++;
	}

	self->monsterinfo.awake = false;
	self->nextthink = level.time + FRAMETIME;
	self->svflags |= SVF_MONSTER;
	self->s.renderfx |= RF_FRAMELERP;
	self->takedamage = DAMAGE_AIM;
	self->air_finished = level.time + M_HOLD_BREATH_TIME;

	self->use = monster_use;
	self->touch = M_Touch;
	self->monsterinfo.alert = defaultMonsterAlerted;//I don't understand why I get a warning here...

	if(!self->max_health)
	{
		self->max_health = self->health;
	}

	self->clipmask = MASK_MONSTERSOLID;
	if(!self->materialtype)
		self->materialtype = MAT_FLESH;

	// Stop the camera clipping with monsters, except the trial beast.

	if(self->classID != CID_TBEAST)
	{
		self->s.effects|=EF_CAMERA_NO_CLIP;
	}

	self->s.skinnum = 0;
	self->deadflag = DEAD_NO;
	self->svflags &= ~SVF_DEADMONSTER;
	self->monsterinfo.thinkinc = MONSTER_THINK_INC;//FRAMETIME;
	self->monsterinfo.nextframeindex = -1;

	if (!self->monsterinfo.checkattack)
	{
		self->monsterinfo.checkattack = M_CheckAttack;
	}

	if (ai_model_scale->value > 0)
	{
		scale = ai_model_scale->value;
		VectorSet(self->rrs.scale, scale, scale, scale);
	}

	scale = 0;

	for (i = 0; i < 3; i++)
	{
		if (!self->rrs.scale[i])
		{
			/* fix empty scale */
			self->rrs.scale[i] = 1.0f;
		}

		scale += self->rrs.scale[i];
	}

	scale /= 3;

	/* non default scale */
	if (scale != 1.0)
	{
		int i;

		self->monsterinfo.scale *= scale;
		self->mass *= scale;


		for (i = 0; i < 3; i++)
		{
			self->mins[i] *= self->rrs.scale[i];
			self->maxs[i] *= self->rrs.scale[i];
		}
	}

	VectorCopy(self->s.origin, self->s.old_origin);

	M_FixStuckMonster(self);

	if (st.item)
	{
		self->item = playerExport->FindItemByClassname (st.item);
		if (!self->item)
		{
			gi.dprintf("%s at %s has bad item: %s\n", self->classname,
					vtos(self->s.origin), st.item);
		}
	}

	// randomize what frame they start on
//	if (self->monsterinfo.currentmove)
//		self->s.frame = self->monsterinfo.currentmove->firstframe + (rand() % (self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1));

	if (!self->mass)
		self->mass = 200;

	self->s.frame = 1;

	self->oldenemy_debounce_time = -1;

	return true;
}

void
MG_BBoxAndOriginAdjustForScale (edict_t *self)
{
	float	o_mins2;

	if(!self->rrs.scale[0] ||
		!self->rrs.scale[1] ||
		!self->rrs.scale[2])
	{
		if(!self->monsterinfo.scale)
		{
			self->monsterinfo.scale = 1.0f;
			VectorSet(self->rrs.scale,
				self->monsterinfo.scale,
				self->monsterinfo.scale,
				self->monsterinfo.scale);
		}
	}
	else if(!self->monsterinfo.scale)
	{
		self->monsterinfo.scale = AVG_VEC3T(self->rrs.scale);
	}

	o_mins2 = self->mins[2];

	Vec3ScaleAssign(AVG_VEC3T(self->rrs.scale), self->mins);
	Vec3ScaleAssign(AVG_VEC3T(self->rrs.scale), self->maxs);

	self->s.origin[2] += o_mins2 - self->mins[2];

	gi.linkentity(self);
}

void
monster_start_go(edict_t *self)
{
	vec3_t v;
	float	volume;

	if (!self)
	{
		return;
	}

	self->nextthink = level.time + FRAMETIME;

	if (self->health <= 0)
	{
		return;
	}

	MG_BBoxAndOriginAdjustForScale(self);
	MG_CheckInGround(self);

	if(!self->mass)
		self->mass = 100;

	if(AVG_VEC3T(self->rrs.scale))
	{
		self->mass *= AVG_VEC3T(self->rrs.scale);
	}

	if(self->spawnflags & MSF_COWARD)//start off running away- FIXME: let them specify a flee_time and use AI_FLEE if one is set?  Would anyone ever use this?!?!?
		self->monsterinfo.aiflags |= AI_COWARD;

	if(self->spawnflags&MSF_STALK)//stalks enemies- only approaches and attacks from behind
		self->ai_mood_flags |= AI_MOOD_FLAG_BACKSTAB;

	if(self->spawnflags&MSF_MELEE_LEAD)//lead enemies in melee and tries to cut them off
		self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;

	if(!self->wakeup_distance)
		self->wakeup_distance = MAX_SIGHT_PLAYER_DIST;

	volume = VectorLength(self->size);
	if(volume < 32)
		self->svflags |= SVF_DO_NO_IMPACT_DMG;

	self->jump_time = level.time + 2;//so they don't take damage from the fall after spawning...

	// check for target to combat_point and change to combattarget
	self->monsterinfo.coop_check_debounce_time = 0;
	self->monsterinfo.pausetime = -1;
	if(self->enemy)
	{//spawned mad
		FoundTarget(self, false);
	}
	else
	{
		if (self->target)
		{
			qboolean	notcombat;
			qboolean	fixup;
			edict_t		*target;

			target = NULL;
			notcombat = false;
			fixup = false;
			while ((target = G_Find (target, FOFS(targetname), self->target)) != NULL)
			{
				if (strcmp(target->classname, "point_combat") == 0)
				{
					self->combattarget = self->target;
					fixup = true;
				}
				else
				{
					notcombat = true;
				}
			}
			if (notcombat && self->combattarget)
				gi.dprintf("%s at %s has target with mixed types\n", self->classname, vtos(self->s.origin));
			if (fixup)
				self->target = NULL;
		}

		/* validate combattarget */
		if (self->combattarget)
		{
			edict_t		*target;

			target = NULL;
			while ((target = G_Find (target, FOFS(targetname), self->combattarget)) != NULL)
			{
				if (strcmp(target->classname, "point_combat") != 0)
				{
					gi.dprintf("%s at (%i %i %i) has a bad combattarget %s : %s at (%i %i %i)\n",
						self->classname, (int)self->s.origin[0], (int)self->s.origin[1], (int)self->s.origin[2],
						self->combattarget, target->classname, (int)target->s.origin[0], (int)target->s.origin[1],
						(int)target->s.origin[2]);
				}
			}
		}

		if (self->target)
		{
			self->goalentity = self->movetarget = G_PickTarget(self->target);
			if (!self->movetarget)
			{
				gi.dprintf ("%s can't find target %s at %s\n", self->classname, self->target, vtos(self->s.origin));
				self->target = NULL;
				self->monsterinfo.pausetime = 100000000;

				if (!self->monsterinfo.c_mode)	// Not in cinematic mode
					G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
				else
					G_QPostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige",0,0,0,NULL,NULL);
			}
			else if (strcmp (self->movetarget->classname, "path_corner") == 0)
			{
				if(self->classID != CID_SERAPH_OVERLORD)
				{
					VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
					self->ideal_yaw = self->s.angles[YAW] = vectoyaw(v);
					gi.dprintf("Monster start go to walk\n");
					G_QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
					self->monsterinfo.pausetime = 0;
				}
				else
				{
					self->goalentity = self->movetarget = NULL;
					self->monsterinfo.pausetime = 100000000;
					if (!self->monsterinfo.c_mode)	// Not in cinematic mode
						G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
					else
						G_QPostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige",0,0,0,NULL,NULL);
				}
				self->target = NULL;
			}
			else
			{
				self->goalentity = self->movetarget = NULL;
				self->monsterinfo.pausetime = 100000000;
				if (!self->monsterinfo.c_mode)	// Not in cinematic mode
					G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
				else
					G_QPostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige",0,0,0,NULL,NULL);
			}
		}
		else
		{
			self->monsterinfo.pausetime = 100000000;
			if (self->monsterinfo.aiflags & AI_EATING)
			{
				G_QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
			}
			else
			{
				if (!self->monsterinfo.c_mode)	// Not in cinematic mode
					G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
				else
					G_QPostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige",0,0,0,NULL,NULL);
			}
		}
	}

	self->think = monster_think;
}

void
walkmonster_start_go(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (!(self->spawnflags & MSF_ASLEEP) && (level.time < 1))
	{
		M_droptofloor(self);

		if (self->groundentity)
		{
			if (!M_walkmove(self, 0, 0))
			{
				gi.dprintf("%s in solid at %s\n", self->classname,
						vtos(self->s.origin));
			}
		}
	}

	if (!self->yaw_speed)
	{
		self->yaw_speed = 20;
	}

	if (!self->viewheight)
	{
		self->viewheight = 25;
	}

	monster_start_go(self);

	if (self->spawnflags & MSF_ASLEEP)
	{
		monster_triggered_start(self);
	}
}

void
walkmonster_start(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->think = walkmonster_start_go;
	monster_start(self);
}

void
flymonster_start_go(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (!M_walkmove(self, 0, 0))
	{
		gi.dprintf("%s in solid at %s\n", self->classname, vtos(self->s.origin));
	}

	if (!self->yaw_speed)
	{
		self->yaw_speed = 10;
	}

	if (!self->viewheight)
	{
		self->viewheight = 25;
	}

	monster_start_go(self);

	if (self->spawnflags & MSF_ASLEEP)
	{
		monster_triggered_start(self);
	}
}

void
flymonster_start(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->flags |= FL_FLY;
	self->think = flymonster_start_go;
	monster_start(self);
}

void
swimmonster_start_go(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (!self->yaw_speed)
	{
		self->yaw_speed = 10;
	}

	if (!self->viewheight)
	{
		self->viewheight = 10;
	}

	monster_start_go(self);

	if (self->spawnflags & MSF_ASLEEP)
	{
		monster_triggered_start(self);
	}
}

void
swimmonster_start(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->flags |= FL_SWIM;
	self->think = swimmonster_start_go;
	M_CatagorizePosition(self);
	monster_start(self);
}

void stationarymonster_start_go(edict_t *self);

void
stationarymonster_triggered_spawn(edict_t *self)
{
	if (!self)
	{
		return;
	}

	KillBox(self);

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_NONE;
	self->svflags &= ~SVF_NOCLIENT;
	self->air_finished = level.time + 12;
	gi.linkentity(self);

	monster_start_go(self);

	if (self->enemy && !(self->spawnflags & 1) &&
		!(self->enemy->flags & FL_NOTARGET))
	{
		if (!(self->enemy->flags & FL_DISGUISED))
		{
			FoundTarget(self, false);
		}
		else
		{
			self->enemy = NULL;
		}
	}
	else
	{
		self->enemy = NULL;
	}
}

void
stationarymonster_triggered_spawn_use(edict_t *self, edict_t *other /* unused */, edict_t *activator)
{
	if (!self || !activator)
	{
		return;
	}

	/* we have a one frame delay here so we don't telefrag the guy who activated us */
	self->think = stationarymonster_triggered_spawn;
	self->nextthink = level.time + FRAMETIME;

	if (activator->client)
	{
		self->enemy = activator;
	}

	self->use = monster_use;
}

void
stationarymonster_triggered_start(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
	self->use = stationarymonster_triggered_spawn_use;
}

void
stationarymonster_start_go(edict_t *self)
{

	if (!self)
	{
		return;
	}

	if (!self->yaw_speed)
	{
		self->yaw_speed = 20;
	}

	if (self->spawnflags & 2)
	{
		stationarymonster_triggered_start(self);
	}
	else
	{
		monster_start_go(self);
	}
}

void
stationarymonster_start(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->think = stationarymonster_start_go;
	monster_start(self);
}

void
monster_done_dodge(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->monsterinfo.aiflags &= ~AI_DODGING;
}

/*
====================================================================
void pitch_roll_for_slope (edict_t *forwhom, vec3_t *slope)

MG

This will adjust the pitch and roll of a monster to match
a given slope - if a non-'0 0 0' slope is passed, it will
use that value, otherwise it will use the ground underneath
the monster.  If it doesn't find a surface, it does nothinh\g
and returns.
====================================================================
*/
void pitch_roll_for_slope (edict_t *forwhom, vec3_t *pass_slope)
{
	vec3_t	slope;
	vec3_t	nvf, ovf, ovr, startspot, endspot, new_angles = { 0, 0, 0 };
	float	pitch, mod, dot;

	if(!pass_slope)
	{
		trace_t trace;

		VectorCopy(forwhom->s.origin, startspot);
		startspot[2] += forwhom->mins[2];
		VectorCopy(startspot, endspot);
		endspot[2] -= 300;
		trace = gi.trace(forwhom->s.origin, vec3_origin, vec3_origin, endspot, forwhom, MASK_SOLID);
//		if(trace_fraction>0.05&&forwhom.movetype==MOVETYPE_STEP)
//			forwhom.flags(-)FL_ONGROUND;

		if(trace.fraction==1.0)
			return;

		if(Vec3IsZero(trace.plane.normal))
			return;

		VectorCopy(trace.plane.normal, slope);
	}
	else {
		VectorCopy(*pass_slope, slope); // jmarshall: switched pass_slope to a pointer to match function prototype.
	}


//C stuff

	AngleVectors(forwhom->s.angles, ovf, ovr, NULL);

	VectoAngles(slope, new_angles);
	pitch = new_angles[PITCH] - 90;
	new_angles[ROLL] = new_angles[PITCH] = 0;

	AngleVectors(new_angles, nvf, NULL, NULL);

	mod = DotProduct(nvf, ovr);

	if(mod<0)
		mod = -1;
	else
		mod = 1;

	dot = DotProduct(nvf, ovf);

	forwhom->s.angles[PITCH] = dot * pitch;
	forwhom->s.angles[ROLL] = (1-Q_fabs(dot)) * pitch * mod;
}

//JWEIER START HELPER BLOCK

/*----------------------------------------------------------------------------------------------------------------

	Monster Helper Functions

----------------------------------------------------------------------------------------------------------------*/

/*====================================================================================================================

	void M_Touch

		Tests to see whether the thing touching it is on its head, and if so, it tries to correct that situation.

		Returns:

		All parameters are passed to the function from the touch callback

======================================================================================================================*/

void M_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	pos1, pos2, dir;
	float	zdiff;

	if ((other->svflags & SVF_MONSTER) || (!Q_stricmp(other->classname, "player")))
	{
		VectorCopy(other->s.origin, pos1);
		pos1[2] += other->mins[2];

		VectorCopy(self->s.origin, pos2);
		pos2[2] += self->maxs[2];

		zdiff = pos1[2] - pos2[2];

		// On top
		if (zdiff >= 0 )
		{
			//We want the full magnitude of the vector, not just drop magnitude
			VectorCopy(other->velocity, dir);

			//Do damage to the thing getting hit based on how hard the collision was
//			T_Damage (self, other, other, dir, pos2, vec3_origin, 1 + (dropmag/FALLDAMAGE_MODIFIER), 0, DAMAGE_NO_BLOOD | DAMAGE_NO_KNOCKBACK);
			if(!irand(0, 9))//10% chance
			{
				int damage;

				damage = irand(1, 3);
				gi.dprintf("%s doing %d damage to %s by standing on it\n", other->classname, damage, self->classname);

				T_Damage (self, other, other, dir, pos2, vec3_origin, damage, 0, DAMAGE_NO_KNOCKBACK|DAMAGE_AVOID_ARMOR,MOD_DIED);
			}

			//Setup a random velocity for the first entity
			other->velocity[0] = flrand(100.0, 150.0);
			other->velocity[1] = flrand(100.0, 150.0);
			other->velocity[2] += 110;

			//Randomly reverse those random numbers
			if (irand(0,1))
				VectorScale(other->velocity, -1, other->velocity);

			//Let the other entity move at its velocity
//			other->groundentity = NULL;
		}
	}
}

/*====================================================================================================================

	edict_t *M_CheckMeleeHit

		Test a melee strike to see if it has hit its target.

		Returns:	"trace.ent" if a valid entity is struck (may not be intended target)
					"NULL" if nothing hit
					"attacker" if hit a wall, but no entity (used for spark effects)

		attacker	-	the entity attacking
		max_dist	-	the distance it checks forward
		trace		-	passed parameter filled with the trace information (can be overkill, or very useful)

======================================================================================================================*/

edict_t	*M_CheckMeleeHit( edict_t *attacker, float max_dist, trace_t *trace )
{
	vec3_t		targPos, vf;

	//Trace forward the maximum amount of the melee distance
	AngleVectors(attacker->s.angles, vf, NULL, NULL);
	VectorMA(attacker->s.origin, max_dist, vf, targPos);

	*trace = gi.trace(attacker->s.origin, attacker->mins, attacker->maxs, targPos, attacker, MASK_MONSTERSOLID);

	//Check to see if the trace was successful (miss)
	if (trace->fraction < 1)
	{
		//Check an entity collision
		if (trace->ent)
		{
			//Can take damage, so pass it back
			if (trace->ent->takedamage)
			{
				//VectorCopy(trace.endpos, hitPos);
				return trace->ent;
			}
		}

		//Wasn't an entity, but we were blocked (world brush)
		return attacker;
	}

	//Nothing found (missed)
	return NULL;
}

/*====================================================================================================================

	edict_t *M_CheckMeleeLineHit

		Test a melee attack along a directed line.

		Returns:	"trace.ent" if a valid entity is struck (may not be intended target)
					"NULL" if nothing hit
					"attacker" if hit a wall, but no entity (used for spark effects)

		attacker	-	what's attacking
		start		-	starting position of the attack (offsets from the character (f,r,u)
		end			-	ending position of the attack (offsets from the character (f,r,u)
		mins, maxs	-	the size of the box to trace by
		trace		-	passed parameter filled with the trace information (can be overkill, or very useful)

======================================================================================================================*/

edict_t	*M_CheckMeleeLineHit( edict_t *attacker, vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, trace_t *trace, vec3_t direction)
{
	vec3_t		startv, endv, midv, swipedir,
				vf, vr, vu;

	//Apply the offsets to the positions passed
	AngleVectors(attacker->s.angles, vf, vr, vu);

	VectorMA(attacker->s.origin, start[0], vf, startv);
	VectorMA(startv, start[1], vr, startv);
	VectorMA(startv, start[2], vu, startv);

	VectorMA(attacker->s.origin, end[0], vf, endv);
	VectorMA(endv, end[1], vr, endv);
	VectorMA(endv, end[2], vu, endv);

	VectorSubtract(endv, startv, swipedir);
	//make sure line to start of swipe is clear
	*trace = gi.trace(attacker->s.origin, mins, maxs, startv, attacker, MASK_SHOT);

	if(trace->fraction == 1.0)//line to start of trace not blocked
	{
		*trace = gi.trace(startv, mins, maxs, endv, attacker, MASK_SHOT);//MASK_MONSTERSOLID);
	}

	if(trace->fraction == 1.0)
	{//hit nothing, trace to middle of line from origin to see if reached too far
		VectorMA(startv, 0.5, swipedir, midv);
		*trace = gi.trace(attacker->s.origin, mins, maxs, midv, attacker, MASK_SHOT);
	}

	if(trace->fraction == 1.0)
	{//Last Chance: trace to end of swipe from origin to see if reached too far
		*trace = gi.trace(attacker->s.origin, mins, maxs, endv, attacker, MASK_SHOT);
	}

	VectorNormalize(swipedir);
	if(direction)
		VectorCopy(swipedir, direction);

	//Check to see if the trace was successful (miss)
	if (trace->fraction < 1 || trace->startsolid || trace->allsolid)
	{
		//Check an entity collision
		if (trace->ent)
		{
			//Can take damage, so pass it back
			if (trace->ent->takedamage)
				return trace->ent;
		}

		//Wasn't an entity, but we were blocked (world brush)
		return attacker;
	}

	//Nothing found (missed)
	return NULL;
}

/*====================================================================================================================

	float M_DistanceToTarget

		Make sure we have a live enemy, and then return a distance to him.

		Returns:	distance to target

		self	-	what's attacking
		target	-	what were looking for

======================================================================================================================*/

float M_DistanceToTarget ( edict_t *self, edict_t *target )
{
	vec3_t	vec;

	assert (target);

	VectorSubtract(target->s.origin, self->s.origin, vec);

	return VectorLength(vec);
}

/*====================================================================================================================

	qboolean M_ValidTarget

		Make sure we have a live enemy, and then return a distance to it.

		Returns:	true if the enemy is valid, false if it is dead.

		self	-	what's attacking
		target	-	what were looking for

======================================================================================================================*/

qboolean M_ValidOldEnemy (edict_t *self)
{
	if (!self->oldenemy)
		return false;

	if (self->oldenemy->health <= 0 || self->oldenemy == self)
		return false;

	if(self->monsterinfo.last_successful_enemy_tracking_time + MONSTER_SEARCH_TIME < level.time)
		if(!visible(self, self->oldenemy))
			return false;

	self->monsterinfo.aiflags &= ~AI_STRAIGHT_TO_ENEMY;
	self->enemy = self->goalentity = self->oldenemy;
	self->oldenemy = NULL;
	return true;
}

qboolean M_ValidTarget( edict_t *self, edict_t *target )
{
	qboolean	checkold = false;

	if(self->oldenemy_debounce_time > 0)
	{
		if(self->oldenemy_debounce_time < level.time)
		{
			self->oldenemy_debounce_time = -1;
			if(M_ValidOldEnemy(self))
				return true;
		}
	}

	if(target == self->enemy)
		checkold = true;

	if (!target)
	{
		self->monsterinfo.aiflags &= ~AI_STRAIGHT_TO_ENEMY;

		if(checkold)
			if(M_ValidOldEnemy(self))
				return true;

		if (!FindTarget(self))
			return false;

		target = self->enemy;
	}

	if(!target)
		return false;

	//See if the target has died
	if (target->health <= 0 || target == self)
	{
		self->monsterinfo.aiflags &= ~AI_STRAIGHT_TO_ENEMY;
		//See if there is another valid target to go after
		if(checkold)
			if(M_ValidOldEnemy(self))
				return true;

		if (!FindTarget(self))
		{
			if(self->enemy)
				self->oldenemy = self->enemy;
			self->enemy = NULL;
			return false;
		}
	}

	if(coop->value)
	{
		if(self->monsterinfo.awake)
		{
			if(self->enemy && self->monsterinfo.coop_check_debounce_time < level.time)
			{
				int		c_dist[MAX_CLIENTS];
				float	e_dist;
				int		i;
				edict_t	*newenemy = NULL;
				edict_t	*client = NULL;

				//only do this check once a second per monster
				self->monsterinfo.coop_check_debounce_time = level.time + 1;

				e_dist = M_DistanceToTarget(self, self->enemy);

				for(i = 0; i <= game.maxclients; i++)
				{
					c_dist[i] = INT_MAX;

					client = &g_edicts[i];
					if(client->client && client->health > 0)
					{
						c_dist[i] = M_DistanceToTarget(self, client);
					}
				}

				for(i = 0; i <= game.maxclients; i++)
				{
					if(c_dist[i] < e_dist)
					{
						client = &g_edicts[i];
						if(visible(self, client))
						{
							newenemy = client;
							e_dist = c_dist[i];
						}
					}
				}

				if(newenemy)
				{
					if(self->enemy->client && self->enemy->health > 0)
						self->oldenemy = self->enemy;

					self->enemy = newenemy;
					FoundTarget(self, false);
					self->monsterinfo.searchType = SEARCH_COMMON;
				}
			}
		}
	}

	return true;
}

/*====================================================================================================================

	int M_PredictTargetEvasion

		Predicts where the target will be a few frames later based on current velocity and facing, and predicts where
		the attacker will be at that same time.  It then decides whether or not it will be able to melee from there.
		This is necessary for melee striking creatures who tend to run up to the player, swing, then stand for a few
		frames while the player backs up.

		NOTE:	Does not detect whether or not a target and attacker will collide during the course of movement, but ai_run will
				find this for us.

		Returns:	0 - target will be out of range at end of movements (suggest: run after)
					1 - target will be within range at the end of the movements at current velocities (suggest: continue motion)

		attacker	-	the entity pursuing the target
		target		-   what's being pursued
		pursue_vel  -	attacker's desired movement velocity (passed as parameter so an average velocity for frames can be used)
		evade_vel	-   target's estimated evade velocity (again, passed as parameter in case you have special knowledge of a movement)
		strike_dist	-	maximum distance a melee attack can occur at, this is the range checked at the end of prediction
		pred_frames -	number of frames (1/10th second) to predict over (prediction accuracy decreases over large amounts of time)

======================================================================================================================*/

int M_PredictTargetEvasion( edict_t *attacker, edict_t *target, vec3_t pursue_vel, vec3_t evade_vel, float strike_dist, float pred_frames )
{
	vec3_t		pTargetPos, pAttackPos, targetMove, attackMove, vec;
	float		dist, targetDist, attackDist;

	//Setup the movement directions
	VectorCopy(pursue_vel, attackMove);
	VectorCopy(evade_vel,  targetMove);

	//Setup the distances of attack
	attackDist = VectorNormalize(attackMove);
	targetDist = VectorNormalize(targetMove);

	//Obtain movement per frame, then apply it over the number of predicted frames
	attackDist = pred_frames * (attackDist * FRAMETIME);
	targetDist = pred_frames * (targetDist * FRAMETIME);

	VectorMA(attacker->s.origin, attackDist, attackMove, pAttackPos);
	VectorMA(target->s.origin,   targetDist, targetMove, pTargetPos);

	//Find the distance between them
	VectorSubtract(pAttackPos, pTargetPos, vec);
	dist = VectorLength(vec);

	//If dist is too far, we won't hit
	if (dist > strike_dist)
		return 0;

	return 1;
}

/*====================================================================================================================

	vec3_t M_PredictTargetPosition

		Predicts where the target will be a few frames later based on current velocity and facing.

		NOTE:	Does not detect whether or not a target and attacker will collide during the course of movement, but ai_run will
				find this for us.

		Returns:	Position the target may be at in the predicted period

		target		-   what's being pursued
		evade_vel	-   target's estimated evade velocity (again, passed as parameter in case you have special knowledge of a movement)
		pred_frames -	number of frames (1/10th second) to predict over (prediction accuracy decreases over large amounts of time)
		pTargetPos	-	where the enemy will be

======================================================================================================================*/

void M_PredictTargetPosition( edict_t *target, vec3_t evade_vel, float pred_frames, vec3_t	pTargetPos)
{
	vec3_t	targetMove;
	float	targetDist;

	//Setup the movement directions
	VectorCopy(evade_vel,  targetMove);

	//Setup the distances of attack
	targetDist = VectorNormalize(targetMove);

	//Obtain movement per frame, then apply it over the number of predicted frames
	targetDist = pred_frames * (targetDist * FRAMETIME);

	VectorMA(target->s.origin,   targetDist, targetMove, pTargetPos);
}

/*====================================================================================================================

	void M_StartDeath

		Sets various states and sets up the monster to play his death frames.  Passing -1 in the sound field will
		skip the sound playing

		Returns:

		self	-	the entity dying
		sound	-	the sound ID to play, -1 if no sound

======================================================================================================================*/

void M_StartDeath( edict_t *self,  int animID)
{
	self->msgHandler = DeadMsgHandler;

	/*if(self->monsterinfo.aiflags & AI_DONT_THINK)
	{
		SetAnim(self, animID);
		return;
	}*/

	//Dead but still being hit
	if(self->deadflag == DEAD_DEAD)
		return;

	self->deadflag = DEAD_DEAD;

	//gib death
	if(self->health <= -80)
	{
		//The monster much check and play its own sound if a gib occured
		self->think = BecomeDebris;
		self->nextthink = level.time + FRAMETIME;
		return;
	}
}

/*====================================================================================================================

	void M_EndDeath

		The monster is dead completely.  Set all information to reflect this.

		Returns:

		self	-	the entity that is dead

======================================================================================================================*/

void M_EndDeath( edict_t *self)
{
	self->mood_nextthink = -1;//never mood_think again
	self->maxs[2] = self->mins[2] + 16;

	if (self->s.effects & EF_ON_FIRE)
	{
		self->think = M_EndDeath;
		self->nextthink = level.time + 1.0;
		self->s.effects &= ~EF_ON_FIRE;
	}
	else
	{
		self->think = NULL;
		self->nextthink = -1;//stop thinkin'

		gi.RemoveEffects(self, 0);
	}
	self->s.effects |= EF_DISABLE_EXTRA_FX;

	gi.linkentity(self);
}


/*====================================================================================================================

	int M_FindSupport

		Look for monsters of a similar race around the current position of this monster.

		Returns:	Number of monsters around the current monster

		range	-	The radius to check inside

======================================================================================================================*/

int M_FindSupport( edict_t *self, int range )
{
	edict_t *ent = NULL;
	int		numSupport = 0;

	while((ent = newfindradius(ent, self->s.origin, range)) != NULL)
	{
		if (ent==self)
			continue;

		if (ent->classID != self->classID)
			continue;

		if (ent->health <= 0)
			continue;

		numSupport++;
	}

	return numSupport;
}

/*====================================================================================================================

	qboolean M_FindSupport

		Look for monsters of a similar race and if they are already trying to alert others

		Returns:	Whether or not to alert other monsters

		range	-	The radius to check inside

======================================================================================================================*/

qboolean M_CheckAlert( edict_t *self, int range )
{
	edict_t *ent = NULL;

	while((ent = newfindradius(ent, self->s.origin, range)) != NULL)
	{
		if (ent==self)
			continue;

		if (ent->classID != self->classID)
			continue;

		if (ent->enemy != self->enemy)
			continue;

		if (ent->health <= 0)
			continue;

		if (ent->monsterinfo.sound_finished < level.time || ent->monsterinfo.sound_pending)
			continue;

		return false;
	}

	return true;
}

/*----------------------------------------------------------------------
	Generic Jump
-----------------------------------------------------------------------*/

void M_jump(edict_t *self, G_Message_t *msg)
{
	vec3_t	jvec, fwd;
	float	dist;

	if (!self->goalentity)
		return;

	if(self->spawnflags&MSF_FIXED)
		return;

	dist = M_DistanceToTarget(self, self->goalentity);

	if (dist > 256)
		return;

	self->jump_time = level.time + 0.5;

	AngleVectors(self->s.angles, fwd, NULL, NULL);
	VectorScale(fwd, 256, jvec);
	jvec[2] += 101;

	if(classStatics[self->classID].msgReceivers[MSG_CHECK_MOOD])
	{
		VectorCopy(jvec, self->movedir);
		self->ai_mood = AI_MOOD_JUMP;//don't technically need this line
		self->mood_nextthink = level.time + 0.5;
		//as an alternative, call self->forced_jump(self);
		G_QPostMessage(self, MSG_CHECK_MOOD, PRI_DIRECTIVE, "i", AI_MOOD_JUMP);
	}
	else
		VectorCopy(jvec, self->velocity);
}

// get the dismember message and send it to my dismember code

void MG_parse_dismember_msg(edict_t *self, G_Message_t *msg)
{
	HitLocation_t	HitLocation;
	int			damage;

	if(!self->monsterinfo.dismember)
	{
		gi.dprintf("ERROR: %s with dismember message handler but no dismember function\n", self->classname);

		return;
	}

	G_ParseMsgParms(msg, "ii", &damage, &HitLocation);

	self->monsterinfo.dismember(self, damage, HitLocation);
}

/*----------------------------------------------------------------------
	Generic Monster Reaction to being alerted
-----------------------------------------------------------------------*/
qboolean defaultMonsterAlerted (edict_t *self, alertent_t *alerter, edict_t *enemy)//I don't understand why I get a warning here...
{
	if(self->alert_time < level.time)
	{//not already alerted
		if(!(alerter->alert_svflags&SVF_ALERT_NO_SHADE) && skill->value < 3.0 && !(self->monsterinfo.aiflags & AI_NIGHTVISION))
		{
			if(enemy->light_level < flrand(6, 77))
			{
				return false;
			}
		}
	}

	if(alerter->lifetime < level.time + 2)
		self->alert_time = level.time + 2;//be ready for 2 seconds to wake up if alerted again
	else
		self->alert_time = alerter->lifetime;//be alert as long as the alert sticks around

	if(enemy->svflags&SVF_MONSTER)
		self->enemy = alerter->enemy;
	else
		self->enemy = enemy;

	FoundTarget(self, true);
	return true;
}


/*
===============
MG_ChangePitchForZVel

===============
*/
float MG_ChangePitch(edict_t *self, float ideal, float speed)
{
	float	current;
	float	move;

	current = anglemod(self->s.angles[PITCH]);
	ideal = anglemod(ideal);

	if (current == ideal)
		return false;

	move = ideal - current;
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

	self->s.angles[PITCH] = anglemod (current + move);
	return move;
}

float MG_ChangePitchForZVel(edict_t *self, float speed, float cap_vel, float max_angle)
{
	float	ideal;

	ideal = self->velocity[PITCH];

	if(fabs(ideal) > cap_vel)
	{
		if(ideal > 0)
			ideal = max_angle;
		else
			ideal = -max_angle;
	}
	else
		ideal = ideal/cap_vel * max_angle;

	return MG_ChangePitch(self, ideal, 10);
}


/*
===============
MG_SetNormalizeVelToGoal

===============
*/
void MG_SetNormalizeVelToGoal(edict_t *self, vec3_t vec)
{
	vec3_t		targVec;
	qboolean	charge_enemy = false;

	if(self->monsterinfo.aiflags&AI_STRAIGHT_TO_ENEMY && self->enemy)
		charge_enemy = true;

	if (self->monsterinfo.searchType == SEARCH_BUOY && !charge_enemy)
	{
		if(mgai_debug->value)
			gi.dprintf("Vec to navgoal!\n");

		if(self->buoy_index < 0 || self->buoy_index > level.active_buoys)
		{
			gi.dprintf("Error: SEARCH_BUOY but invalid index!!!\n");

			VectorClear(vec);
			return;
		}

		VectorCopy(level.buoy_list[self->buoy_index].origin, self->monsterinfo.nav_goal);
		VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, vec);
	}
	else if(self->goalentity && !charge_enemy)
	{
		if(mgai_debug->value)
			gi.dprintf("Vec to goalentity!\n");

		if(self->goalentity == self->enemy && self->ai_mood_flags & AI_MOOD_FLAG_PREDICT)
		{//predict where he's goin
			M_PredictTargetPosition( self->enemy, self->enemy->velocity, 8, targVec);
		}
		else
		{
			VectorCopy(self->goalentity->s.origin, targVec);
		}

		VectorSubtract(targVec, self->s.origin, vec);
	}
	else if(self->enemy)
	{
		if(mgai_debug->value)
			gi.dprintf("Vec to enemy!\n");

		if (self->ai_mood_flags & AI_MOOD_FLAG_PREDICT)
		{//predict where he's goin
			M_PredictTargetPosition( self->enemy, self->enemy->velocity, 8, targVec);
		}
		else
		{
			VectorCopy(self->enemy->s.origin, targVec);
		}

		VectorSubtract(targVec, self->s.origin, vec);
	}
	else
	{
		if(mgai_debug->value)
			gi.dprintf("No goal to face!\n");

		VectorClear(vec);
		return;
	}

	VectorNormalize(vec);
}

/*====================================================================================================================

	void M_ShowLifeMeter

		Overides the lung meter and displays the creature's life meter to all clients

		Returns:	Nothing

======================================================================================================================*/

#define LIFEBAR_SCALE 16

void M_ShowLifeMeter( edict_t *self, int value, int max_value )
{
	player_state_t		*ps;
	int					i;

	//Update all clients
	for (i = 0 ; i < maxclients->value  ; i++)
	{
		ps = &game.clients[i].ps;

		ps->stats[STAT_LIFEBAR_XSIZE]  = (max_value / LIFEBAR_SCALE);
		ps->stats[STAT_LIFEBAR_YSIZE]  = 16;
		ps->stats[STAT_LIFEBAR_ICON] = gi.imageindex("icons/powerup.m8");
		ps->stats[STAT_LIFEBAR_BACK]  = gi.imageindex("icons/lifebar_back.m8");
		if (max_value)
			ps->stats[STAT_LIFEBAR_VALUE] = (100 * value) / max_value;
		else
			ps->stats[STAT_LIFEBAR_VALUE] = 0;
	}
}
