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
 * Miscellaneos entities, functs and functions.
 *
 * =======================================================================
 */

#include "header/local.h"
#include "monster/misc/player.h"
#include "header/g_defaultmessagehandler.h"
#include "header/g_misc.h"
#include "common/fx.h"
#include "common/h2rand.h"
#include "header/g_monster.h"
#include "header/g_teleport.h"
#include "header/g_hitlocation.h"
#include "monster/stats/stats.h"
#include "header/g_playstats.h"


int debristhisframe;
int gibsthisframe;

void ED_CallSpawn(edict_t *ent);

extern	vec3_t	mins;

#define CAMERA_SCRIPTED 2

int AndoriaSoundID[AS_MAX] =
{
	AS_NOTHING,
	AS_SMALLFOUNTAIN,
	AS_LARGEFOUNTAIN,
	AS_SEWERWATER,
	AS_OUTSIDEWATERWAY,
	AS_WINDCHIME,
};



int CloudSoundID[AS_MAX] =
{
	AS_NOTHING,
	AS_CAULDRONBUBBLE,
	AS_WINDEERIE,
	AS_WINDNOISY,
	AS_WINDSOFTHI,
	AS_WINDSOFTLO,
	AS_WINDSTRONG1,
	AS_WINDSTRONG2,
	AS_WINDWHISTLE,
};

int HiveSoundID[AS_MAX] =
{
	AS_NOTHING,
	AS_GONG,
	AS_WINDEERIE,
	AS_WINDNOISY,
	AS_WINDSOFTHI,
	AS_WINDSOFTLO,
	AS_WINDSTRONG1,
	AS_WINDSTRONG2,
	AS_WINDWHISTLE,
};

int MineSoundID[AS_MAX] =
{
	AS_NOTHING,
	AS_MUDPOOL,
	AS_ROCKS,
	AS_WINDEERIE,
	AS_WINDSOFTLO,
	AS_CONVEYOR,
	AS_BUCKETCONVEYOR,
	AS_CAVECREAK,
};

int SilverSpringSoundID[AS_MAX] =
{
	AS_NOTHING,
	AS_FIRE,
	AS_WATERLAPPING,
	AS_SEAGULLS,
	AS_OCEAN,
	AS_BIRDS,
	AS_CRICKETS,
	AS_FROGS,
	AS_CRYING,
	AS_MOSQUITOES,
	AS_BUBBLES,
	AS_BELL,
	AS_FOOTSTEPS,
	AS_MOANS,
	AS_SEWERDRIPS,
	AS_WATERDRIPS,
	AS_HEAVYDRIPS,
	AS_CAULDRONBUBBLE,
	AS_SPIT,
};

int SwampCanyonSoundID[AS_MAX] =
{
	AS_NOTHING,
	AS_BIRD1,
	AS_BIRD2,
	AS_HUGEWATERFALL,
	AS_MUDPOOL,
	AS_WINDEERIE,
	AS_WINDNOISY,
	AS_WINDSOFTHI,
	AS_WINDSOFTLO,
	AS_WINDSTRONG1,
	AS_WINDSTRONG2,
	AS_WINDWHISTLE,
};


typedef struct DebrisSound
{
	char	*Name;
} DebrisSound_t;

DebrisSound_t DebrisSound [NUM_MAT]=
{
	{"misc/breakstone.wav"},	// MAT_STONE
	{"misc/breakstone.wav"},	// MAT_GREYSTONE
	{"misc/tearcloth.wav"},	// MAT_CLOTH
	{"misc/metalbreak.wav"},	// MAT_METAL
	{"misc/fleshbreak.wav"},	// MAT_FLESH
	{"misc/potbreak.wav"},	// MAT_POTTERY
	{"misc/glassbreak2.wav"},	// MAT_GLASS
	{"misc/breakstone.wav"},	// MAT_LEAF	FIXME
	{"misc/breakwood.wav"},	// MAT_WOOD
	{"misc/breakstone.wav"},	// MAT_BROWNSTONE
	{"misc/bushbreak.wav"},	// MAT_NONE
	{NULL},					// MAT_INSECT
};


/*
 * QUAKED func_group (0 0 0) ?
 *
 * Used to group brushes together just for editor convenience.
 */

//=====================================================

void Use_Areaportal (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->count ^= 1;		// toggle state
//	gi.dprintf ("portalstate: %i = %i\n", ent->style, ent->count);
	gi.SetAreaPortalState (ent->style, ent->count);
}

/*QUAKED func_areaportal (0 0 0) ?

	This is a non-visible object that divides the world into
	areas that are seperated when this portal is not activated.
	Usually enclosed in the middle of a door.
*/
void SP_func_areaportal (edict_t *ent)
{
	ent->use = Use_Areaportal;
	ent->count = 0;		// allways start closed;
}

//=====================================================

/*
=================
Misc functions
=================
*/

void VelocityForDamage (int damage, vec3_t v)
{
	v[0] = flrand(-100.0F, 100.0F);
	v[1] = flrand(-100.0F, 100.0F);
	v[2] = flrand(200.0F, 300.0F);

	if (damage < 50)
		VectorScale (v, 0.7, v);
	else
		VectorScale (v, 1.2, v);
}

void
ClipGibVelocity(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	if (ent->velocity[0] < -300)
	{
		ent->velocity[0] = -300;
	}
	else if (ent->velocity[0] > 300)
	{
		ent->velocity[0] = 300;
	}

	if (ent->velocity[1] < -300)
	{
		ent->velocity[1] = -300;
	}
	else if (ent->velocity[1] > 300)
	{
		ent->velocity[1] = 300;
	}

	if (ent->velocity[2] < 200)
	{
		ent->velocity[2] = 200; /* always some upwards */
	}
	else if (ent->velocity[2] > 500)
	{
		ent->velocity[2] = 500;
	}
}

/* ===================================================== */

void
gib_think(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->s.frame++;
	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == 10)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 8 + random() * 10;
	}
}

void
gib_touch(edict_t *self, edict_t *other /* unused */, cplane_t *plane, csurface_t *surf /* unused */)
{
	vec3_t normal_angles, right;

	if (!self)
	{
		return;
	}

	if (!self->groundentity)
	{
		return;
	}

	self->touch = NULL;

	if (plane)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex(
						"misc/fhit3.wav"), 1, ATTN_NORM, 0);

		vectoangles(plane->normal, normal_angles);
		AngleVectors(normal_angles, NULL, right, NULL);
		vectoangles(right, self->s.angles);

		if (self->s.modelindex == sm_meat_index)
		{
			self->s.frame++;
			self->think = gib_think;
			self->nextthink = level.time + FRAMETIME;
		}
	}
}

void
gib_die(edict_t *self, edict_t *inflictor /* unused */, edict_t *attacker /* unused */,
		int damage /* unused */, vec3_t point /* unused */)
{
	if (!self)
	{
		return;
	}

	G_FreeEdict(self);
}

void
ThrowGib(edict_t *self, const char *gibname, int damage, int type)
{
	edict_t *gib;
	vec3_t vd;
	vec3_t origin;
	vec3_t size;
	float vscale;

	if (!self || !gibname)
	{
		return;
	}

	gib = G_Spawn();

	if (!gib)
	{
		return;
	}

	VectorScale(self->size, 0.5, size);
	VectorAdd(self->absmin, size, origin);
	gib->s.origin[0] = origin[0] + crandom() * size[0];
	gib->s.origin[1] = origin[1] + crandom() * size[1];
	gib->s.origin[2] = origin[2] + crandom() * size[2];

	gi.setmodel(gib, gibname);
	gib->solid = SOLID_NOT;

	gi.CreateEffect(gib, FX_GIB_TRAIL, CEF_OWNERS_ORIGIN, gib->s.origin, NULL);

	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_YES;
	gib->die = gib_die;

	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_STEP;
		gib->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		gib->movetype = MOVETYPE_STEP;
		vscale = 1.0;
	}

	VelocityForDamage(damage, vd);
	VectorMA(self->velocity, vscale, vd, gib->velocity);
	ClipGibVelocity(gib);
	gib->avelocity[0] = random() * 600;
	gib->avelocity[1] = random() * 600;
	gib->avelocity[2] = random() * 600;

	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 10 + random() * 10;

	gi.linkentity(gib);
}

void
ThrowHeadACID(edict_t *self, const char *gibname, int damage, int type)
{
	vec3_t vd;
	float vscale;

	if (!self || !gibname)
	{
		return;
	}

	self->s.skinnum = 0;
	self->s.frame = 0;
	VectorClear(self->mins);
	VectorClear(self->maxs);

	self->s.modelindex2 = 0;
	gi.setmodel(self, gibname);

	self->clipmask = MASK_SHOT;
	self->solid = SOLID_BBOX;

	self->s.effects |= EF_GREENGIB;
	self->s.effects &= ~EF_FLIES;
	self->s.effects |= RF_FULLBRIGHT;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;
	self->svflags &= ~SVF_MONSTER;
	self->takedamage = DAMAGE_YES;
	self->die = gib_die;
	self->dmg = 2;

	if (type == GIB_ORGANIC)
	{
		self->movetype = MOVETYPE_TOSS;
		vscale = 0.5;
	}
	else
	{
		self->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage(damage, vd);
	VectorMA(self->velocity, vscale, vd, self->velocity);
	ClipGibVelocity(self);

	self->avelocity[YAW] = crandom() * 600;

	self->think = G_FreeEdict;
	self->nextthink = level.time + 10 + random() * 10;

	gi.linkentity(self);
}

void
ThrowClientHead(edict_t *self, int damage)
{
	vec3_t vd;
	char *gibname;

	if (!self)
	{
		return;
	}

	if (randk() & 1)
	{
		gibname = "models/objects/gibs/head2/tris.md2";
		self->s.skinnum = 1; /* second skin is player */
	}
	else
	{
		gibname = "models/objects/gibs/skull/tris.md2";
		self->s.skinnum = 0;
	}

	self->s.origin[2] += 32;
	self->s.frame = 0;
	gi.setmodel(self, gibname);
	VectorSet(self->mins, -16, -16, 0);
	VectorSet(self->maxs, 16, 16, 16);

	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_BBOX;
	self->s.effects = EF_GIB;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;

	// The entity still has the monsters clipmaks.
	// Reset it to MASK_SHOT to be on the save side.
	// (MASK_SHOT is used by xatrix)
	self->clipmask = MASK_SHOT;

	self->movetype = MOVETYPE_BOUNCE;
	VelocityForDamage(damage, vd);
	VectorAdd(self->velocity, vd, self->velocity);

	if (self->client) /* bodies in the queue don't have a client anymore */
	{
		// self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = self->s.frame;
	}
	else
	{
		self->think = NULL;
		self->nextthink = 0;
	}

	gi.linkentity(self);
}

/* ===================================================== */

void
debris_die(edict_t *self, edict_t *inflictor /* unused */, edict_t *attacker /* unused */,
		int damage /* unused */, vec3_t point /* unused */)
{
	if (!self)
	{
		return;
	}

	G_FreeEdict(self);
}

void
ThrowDebris(edict_t *self, char *modelname, float speed, vec3_t origin)
{
	edict_t *chunk;
	vec3_t v;

	if (!self || !modelname)
	{
		return;
	}

	if (debristhisframe >= MAX_DEBRIS)
	{
		return;
	}

	chunk = G_SpawnOptional();

	if (!chunk)
	{
		return;
	}

	debristhisframe++;

	VectorCopy(origin, chunk->s.origin);
	gi.setmodel(chunk, modelname);
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA(self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_BOUNCE;
	chunk->solid = SOLID_NOT;
	chunk->avelocity[0] = random() * 600;
	chunk->avelocity[1] = random() * 600;
	chunk->avelocity[2] = random() * 600;
	chunk->think = G_FreeEdict;
	chunk->nextthink = level.time + 5 + random() * 5;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "debris";
	chunk->takedamage = DAMAGE_YES;
	chunk->die = debris_die;
	chunk->health = 250;
	gi.linkentity(chunk);
}
void
BecomeExplosion1(edict_t *self)
{
	gi.CreateEffect(NULL, FX_EXPLOSION1, 0, self->s.origin, NULL);

	G_FreeEdict(self);
}

void
BecomeExplosion2(edict_t *self)
{
	gi.CreateEffect(NULL, FX_EXPLOSION2, 0, self->s.origin, NULL);

	G_FreeEdict(self);
}

void
SpawnDebris(edict_t *self, float size, vec3_t origin)
{
	byte		sizeb, magb;
	vec3_t		halfsize;
	float		mag;
	int			flags = 0;
	int			violence=VIOLENCE_DEFAULT;

	size /=10;
	sizeb = Clamp(size, 1.0, 255.0);
	VectorScale(self->size, 0.5, halfsize);
	mag = VectorLength(halfsize);
	magb = Clamp(mag, 1.0, 255.0);

	if(self->fire_damage_time > level.time || self->svflags&SVF_ONFIRE)
		flags |= CEF_FLAG6;

	if (self->materialtype == MAT_FLESH || self->materialtype == MAT_INSECT)
	{
		if (blood_level)
			violence = blood_level->value;

		if(violence < VIOLENCE_NORMAL)
		{
			size /= 10;
			sizeb = Clamp(size, 1.0, 255.0);
			gi.CreateEffect(NULL,
							FX_DEBRIS,
							flags, origin,
							"bbdb",
							sizeb,
							MAT_STONE,
							halfsize, magb);
		}
		else
		{
			if(violence > VIOLENCE_NORMAL)
			{
				sizeb *= (violence - VIOLENCE_NORMAL);
				if(sizeb > 255)
					sizeb = 255;
			}

			if(self->materialtype == MAT_INSECT)
				flags |= CEF_FLAG8;

			if(!Q_stricmp(self->classname, "monster_tcheckrik_male"))
				flags |= CEF_FLAG7;//use male insect skin on chunks

			gi.CreateEffect(NULL,
							FX_FLESH_DEBRIS,
							flags, origin,
							"bdb",
							sizeb,
							halfsize, magb);
		}
	}
	else
	{
		if(self->s.renderfx & RF_REFLECTION)
			flags |= CEF_FLAG8;

		gi.CreateEffect(NULL,
						FX_DEBRIS,
						flags, origin,
						"bbdb",
						sizeb,
						(byte)self->materialtype,
						halfsize, magb);
	}

	if(self->classID == CID_OBJECT)
		if(!strcmp(self->classname, "obj_larvabrokenegg") || !strcmp(self->classname, "obj_larvaegg"))
			self->materialtype = MAT_POTTERY;

	if (DebrisSound[self->materialtype].Name)
	{
		gi.sound(self, CHAN_VOICE,
			gi.soundindex(DebrisSound[self->materialtype].Name), 1, ATTN_NORM, 0);
	}
}

void
BecomeDebris2(edict_t *self, float damage)
{
	float		size;
	int			violence=VIOLENCE_DEFAULT;

	if (blood_level)
		violence = blood_level->value;

	if (violence > VIOLENCE_BLOOD)
	{
		if(!(self->svflags & SVF_PARTS_GIBBED))
		{//haven't yet thrown parts
			if(self->monsterinfo.dismember)
			{//FIXME:have a generic GibParts effect that throws flesh and several body parts- much cheaper?
				int	i, num_limbs;

				num_limbs = irand(3, 10);

				if(violence > VIOLENCE_NORMAL)
					num_limbs *= (violence - VIOLENCE_NORMAL);

				for(i = 0; i < num_limbs; i++)
				{
					if(self->svflags&SVF_MONSTER)
						self->monsterinfo.dismember(self, flrand(80, 160), irand(hl_Head, hl_LegLowerRight) | hl_MeleeHit);
				}
				self->svflags |= SVF_PARTS_GIBBED;
				self->think = BecomeDebris;
				self->nextthink = level.time + 0.1;
				return;
			}
		}
	}
	// Set my message handler to the special message handler for dead entities.
	self->msgHandler=DeadMsgHandler;

	//What the hell is this???
	if (self->spawnflags & 4 && !(self->svflags&SVF_MONSTER))
	{   // Need to create an explosion effect for this
		if(self->owner)
		{
			T_DamageRadius(self, self->owner, self, 60.0,
						self->dmg, self->dmg/2, DAMAGE_NORMAL|DAMAGE_AVOID_ARMOR,MOD_DIED);
		}
		else
		{
			T_DamageRadius(self, self, self, 60.0,
						self->dmg, self->dmg/2, DAMAGE_NORMAL|DAMAGE_AVOID_ARMOR,MOD_DIED);
		}
	}

	// A zero mass is well and truly illegal!
	if (self->mass<0)
		gi.dprintf("ERROR: %s needs a mass to generate debris",self->classname);

	// Create a chunk-spitting client effect and remove me now that I've been chunked.

	// This only yields 4, 8, 12, or 16 chunks, generally seems to yield 16
	if(self->svflags&SVF_MONSTER && self->classID != CID_MOTHER)
	{
		size = VectorLength(self->size);
		size *= 100;
		assert(size >= 0);
	}
	else
	{
		if (Vec3IsZero(self->s.origin))
		{// Set this brush up as if it were an object so the debris will be thrown properly
		// If I'm a BModel (and therefore don't have an origin), calculate one to use instead and
		// slap that into my origin.
//			self->solid = SOLID_NOT;		// This causes the breakable brushes to generate a sound
			VectorMA(self->absmin,0.5,self->size,self->s.origin);
		}

		size = VectorLength(self->size) * 3;

		if(self->solid == SOLID_BSP)
			size *= 3;
		else if(self->classID == CID_MOTHER)
			size *= 10;

		if (!self->mass)
			self->mass = size / 10;
	}

	SpawnDebris(self, size, self->s.origin);

	self->s.modelindex = 0;
	self->solid = SOLID_NOT;
	self->deadflag = DEAD_DEAD;

	G_SetToFree(self);
	self->nextthink = level.time + 2;
}

void
BecomeDebris(edict_t *self)
{
	if(self->health<0)
		BecomeDebris2(self, abs(self->health)+10.0f);
	else
		BecomeDebris2(self, 10.0f);
}

void
SprayDebris(edict_t *self, vec3_t spot, byte NoOfChunks, float damage)
{
	byte		magb, mat;
	float		mag, size;
	int			flags = 0;
	int			violence=VIOLENCE_DEFAULT;

	mag = VectorLength(self->mins);

	mat = (byte)(self->materialtype);
	magb = Clamp(mag, 1.0, 255.0);

	if(mat == MAT_FLESH || mat == MAT_INSECT)
	{
		if (blood_level)
			violence = blood_level->value;

		if (violence < VIOLENCE_NORMAL)
		{
			mat = MAT_STONE;
		}
		else if(violence > VIOLENCE_NORMAL)
		{
			NoOfChunks *= (violence - VIOLENCE_NORMAL);
			if(NoOfChunks > 255)
				NoOfChunks = 255;
		}
	}

	if(mat == MAT_FLESH || mat == MAT_INSECT)
	{
		if(self->materialtype == MAT_INSECT)
		{
			flags |= CEF_FLAG8;
			if(!Q_stricmp(self->classname, "monster_tcheckrik_male"))
				flags |= CEF_FLAG7;//use male insect skin on chunks
		}

		if(self->fire_damage_time > level.time || self->svflags&SVF_ONFIRE)
			flags |= CEF_FLAG6;

		gi.CreateEffect(NULL,
						FX_FLESH_DEBRIS,
						flags,
						spot,
						"bdb",
						NoOfChunks, self->mins, magb);
	}
	else
	{
		size = (float)(NoOfChunks/100);
		NoOfChunks = Clamp(size, 1.0, 255.0);

		gi.CreateEffect(NULL,
						FX_DEBRIS,
						flags, spot,
						"bbdb",
						NoOfChunks,
						MAT_STONE,
						self->mins, magb);
	}
}

void
DefaultObjectDieHandler(edict_t *self, G_Message_t *msg)
{
	edict_t *inflictor;

	G_ParseMsgParms(msg, "ee", &inflictor, &inflictor);

	G_UseTargets(self, inflictor);

	if (self->target_ent)
		BecomeDebris(self->target_ent);

	BecomeDebris(self);
}

void harpy_take_head(edict_t *self, edict_t *victim, int BodyPart, int frame, int flags);
void ThrowBodyPart(edict_t *self, vec3_t *spot, int BodyPart, float damage, int frame)
{//add blood spew to sever loc and blood trail on flying part
	vec3_t	spot2;
	int	flags;

	if (damage)
	{
		if (damage>255)
		{
			damage = 255;
		}
		gi.sound (self, CHAN_VOICE, gi.soundindex("misc/fleshbreak.wav") , 1, ATTN_NORM, 0);
	}

	VectorAdd(self->s.origin, *spot, spot2);

	if(self->fire_damage_time > level.time || self->svflags&SVF_ONFIRE)
		flags = CEF_FLAG6;
	else
		flags = 0;

	if(self->materialtype == MAT_INSECT)
		flags |= CEF_FLAG8;

	if(give_head_to_harpy && take_head_from == self)
	{
		harpy_take_head(give_head_to_harpy, self, BodyPart, frame, flags);
		SprayDebris(self, *spot, 5, damage);
		return;
	}

	gi.CreateEffect(NULL,//owner
					FX_BODYPART,//type
					flags,//can't mess with this, sends only 1st byte and effects message
					spot2,//spot,
					"ssbbb",//int int float byte
					(short)frame,//only 1 frame, sorry no anim
					(short)BodyPart,//bitwise - node(s) to leave on
					(byte)damage,//speed
					self->s.modelindex,//my modelindex
					self->s.number);//my number
}

void
ThrowWeapon(edict_t *self, vec3_t *spot, int BodyPart, float damage, int frame)
{//same but no blood and metal sound when hits ground
	vec3_t	spot2;

	if (damage>255)
	{
		damage = 255;
	}

	VectorAdd(self->s.origin, *spot, spot2);
	gi.CreateEffect(NULL,//owner
					FX_THROWWEAPON,//type
					0,//can't mess with this, sends only 1st byte and effects message
					spot2,//spot,
					"ssbbb",// int int float byte
					(short)frame,//only 1 frame, sorry no anim
					(short)BodyPart,//bitwise - node(s) to leave on
					(byte)damage,//speed
					self->s.modelindex,//my modelindex
					self->s.number);//my number
}
/* ===================================================== */

/*
 * QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
 *
 * Target: next path corner
 * Pathtarget: gets used when an entity that has
 *  this path_corner targeted touches it
 */
void
path_corner_touch(edict_t *self, edict_t *other, cplane_t *plane /* unused */,
		csurface_t *surf /* unused */)
{
	vec3_t v;
	edict_t *next;

	if (!self || !other)
	{
		return;
	}

	if (other->movetarget != self)
	{
		return;
	}

	if (other->enemy)
	{
		return;
	}

	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		G_UseTargets(self, other);
		self->target = savetarget;
	}

	if (self->target)
	{
		next = G_PickTarget(self->target);
	}
	else
	{
		next = NULL;
	}

	if ((next) && (next->spawnflags & MSF_AMBUSH))
	{
		VectorCopy(next->s.origin, v);
		v[2] += next->mins[2];
		v[2] -= other->mins[2];
		VectorCopy(v, other->s.origin);
		next = G_PickTarget(next->target);
	}

	other->goalentity = other->movetarget = next;

	if (self->wait)
	{
		other->monsterinfo.pausetime = level.time + self->wait;
		G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
//		M_GiveOrder(ORD_STAND,other,other,PR_LOW, 0,0,0,0,0,0);
//		other->monsterinfo.stand (other);
		return;
	}

	if (!other->movetarget)
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
//		M_GiveOrder(ORD_STAND,other,other,PR_LOW, 0,0,0,0,0,0);
//		other->monsterinfo.stand (other);
	}
	else
	{
		VectorSubtract(other->goalentity->s.origin, other->s.origin, v);
		other->ideal_yaw = vectoyaw(v);
	}
}

void
SP_path_corner(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (!self->targetname)
	{
		gi.dprintf("path_corner with no targetname at %s\n",
				vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	if (st.noise)
		self->moveinfo.sound_middle = gi.soundindex(st.noise);

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	self->touch = path_corner_touch;
	VectorSet(self->mins, -8, -8, -8);
	VectorSet(self->maxs, 8, 8, 8);
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity(self);
}

/* ===================================================== */

/*
 * QUAKED point_combat (0.5 0.3 0) (-8 -8 -8) (8 8 8) Hold
 *
 * Makes this the target of a monster and it will head here
 * when first activated before going after the activator.  If
 * hold is selected, it will stay here.
 */
void
point_combat_touch(edict_t *self, edict_t *other, cplane_t *plane /* unused */,
		csurface_t *surf /* unused */)
{
	edict_t *activator;
	vec3_t v;

	if (!self || !other)
	{
		return;
	}

	if (other->movetarget != self)
	{
		return;
	}

	if (self->target)
	{
		other->target = self->target;
		other->goalentity = other->movetarget = G_PickTarget(other->target);

		if (other->goalentity)
		{
			VectorSubtract(other->goalentity->s.origin, other->s.origin, v);
			other->ideal_yaw = vectoyaw(v);
		}
		else
		{
			gi.dprintf("%s at %s target %s does not exist\n",
					self->classname,
					vtos(self->s.origin),
					self->target);

			other->movetarget = self;
		}

		self->target = NULL;
	}
	else if ((self->spawnflags & 1) && !(other->flags & (FL_SWIM | FL_FLY)))
	{
		//HOLD
		other->spawnflags |= MSF_FIXED;//stay here forever now
		G_QPostMessage(other, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	if (other->movetarget == self)
	{
		other->target = NULL;
		other->movetarget = NULL;
		other->goalentity = other->enemy;
		other->monsterinfo.aiflags &= ~AI_COMBAT_POINT;
	}

	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;

		if (other->enemy && other->enemy->client)
		{
			activator = other->enemy;
		}
		else if (other->oldenemy && other->oldenemy->client)
		{
			activator = other->oldenemy;
		}
		else if (other->activator && other->activator->client)
		{
			activator = other->activator;
		}
		else
		{
			activator = other;
		}

		G_UseTargets(self, activator);
		self->target = savetarget;
	}
}

void
SP_point_combat(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (deathmatch->value)
	{
		G_FreeEdict(self);
		return;
	}

	self->solid = SOLID_TRIGGER;
	self->touch = point_combat_touch;
	VectorSet(self->mins, -8, -8, -16);
	VectorSet(self->maxs, 8, 8, 16);
	self->svflags = SVF_NOCLIENT;
	gi.linkentity(self);
}

/* ===================================================== */

/*
 * QUAKED viewthing (0 .5 .8) (-8 -8 -8) (8 8 8)
 *
 * Just for the debugging level.  Don't use
 */
void
viewthing_think(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	ent->s.frame = (ent->s.frame + 1) % 7;
	ent->nextthink = level.time + FRAMETIME;

	if (ent->spawnflags)
	{
		if (ent->s.frame == 0)
		{
			ent->spawnflags = (ent->spawnflags + 1) % 4 + 1;
		}
	}
}

void
SP_viewthing(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	gi.dprintf("viewthing spawned\n");

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.renderfx = RF_FRAMELERP;
	VectorSet(ent->mins, -16, -16, -24);
	VectorSet(ent->maxs, 16, 16, 32);
	ent->s.modelindex = gi.modelindex("models/objects/banner/tris.md2");
	gi.linkentity(ent);
	ent->nextthink = level.time + 0.5;
	ent->think = viewthing_think;
	return;
}

/* ===================================================== */

/*
 * QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
 *
 * Used as a positional target for spotlights, etc.
 */
void
SP_info_null(edict_t *self)
{
	if (!self)
	{
		return;
	}

	G_FreeEdict(self);
}

/*
 * QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
 *
 * Used as a positional target for lighting.
 */
void
SP_info_notnull(edict_t *self)
{
	if (!self)
	{
		return;
	}

	VectorCopy(self->s.origin, self->absmin);
	VectorCopy(self->s.origin, self->absmax);

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;

};


/*QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST

	This is just a solid wall if not inhibited

	TRIGGER_SPAWN	the wall will not be present until triggered
					it will then blink in to existance; it will
					kill anything that was in it's way

	TOGGLE			only valid for TRIGGER_SPAWN walls
					this allows the wall to be turned on and off

	START_ON		only valid for TRIGGER_SPAWN walls
					the wall will initially be present
*/
void
func_wall_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (!self)
	{
		return;
	}

	if (self->solid == SOLID_NOT)
	{
		self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;
		KillBox(self);
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}

	gi.linkentity(self);

	if (!(self->spawnflags & 2))
	{
		self->use = NULL;
	}
}

void
SP_func_wall(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->movetype = MOVETYPE_PUSH;
	gi.setmodel(self, self->model);

	if (self->spawnflags & 8)
	{
		self->s.effects |= EF_ANIM_ALL;
	}

	if (self->spawnflags & 16)
	{
		self->s.effects |= EF_ANIM_ALLFAST;
	}

	/* just a wall */
	if ((self->spawnflags & 7) == 0)
	{
		self->solid = SOLID_BSP;
		gi.linkentity(self);
		return;
	}

	/* it must be TRIGGER_SPAWN */
	if (!(self->spawnflags & 1))
	{
		self->spawnflags |= 1;
	}

	/* yell if the spawnflags are odd */
	if (self->spawnflags & 4)
	{
		if (!(self->spawnflags & 2))
		{
			gi.dprintf("func_wall START_ON without TOGGLE\n");
			self->spawnflags |= 2;
		}
	}

	self->use = func_wall_use;

	if (self->spawnflags & 4)
	{
		self->solid = SOLID_BSP;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}

	gi.linkentity(self);
}

/* ===================================================== */

/*
 * QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
 *
 * This is solid bmodel that will fall if it's support it removed.
 */

void
func_object_touch(edict_t *self, edict_t *other, cplane_t *plane,
		csurface_t *surf /* unused */)
{
	/* only squash thing we fall on top of */

	if (!self || !other || !plane)
	{
		return;
	}

	/* only squash thing we fall on top of */
	if (plane && plane->normal[2] < 1.0)
	{
		return;
	}

	if (other->takedamage == DAMAGE_NO)
	{
		return;
	}

	T_Damage(other, self, self, vec3_origin, self->s.origin,
			vec3_origin, self->dmg, 1, DAMAGE_AVOID_ARMOR,MOD_DIED);
}

void
func_object_release(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->touch = func_object_touch;
}

void
func_object_use(edict_t *self, edict_t *other /* unused */, edict_t *activator /* unused */)
{
	if (!self)
	{
		return;
	}

	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox(self);
	func_object_release(self);
}

void
SP_func_object(edict_t *self)
{
	if (!self)
	{
		return;
	}

	gi.setmodel(self, self->model);

	self->mins[0] += 1;
	self->mins[1] += 1;
	self->mins[2] += 1;
	self->maxs[0] -= 1;
	self->maxs[1] -= 1;
	self->maxs[2] -= 1;

	if (!self->dmg)
	{
		self->dmg = 100;
	}

	if (self->spawnflags == 0)
	{
		self->solid = SOLID_BSP;
		self->movetype = MOVETYPE_PUSH;
		self->think = func_object_release;
		self->nextthink = level.time + 2 * FRAMETIME;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->movetype = MOVETYPE_PUSH;
		self->use = func_object_use;
		self->svflags |= SVF_NOCLIENT;
	}

	if (self->spawnflags & 2)
	{
		self->s.effects |= EF_ANIM_ALL;
	}

	if (self->spawnflags & 4)
	{
		self->s.effects |= EF_ANIM_ALLFAST;
	}

	self->clipmask = MASK_MONSTERSOLID;

	gi.linkentity(self);
}


void ItemSpitterSpit(edict_t *self,edict_t *owner,edict_t *attacker)
{
	int i1;
	gitem_t	*item;
	edict_t *newitem;
	vec3_t forward, holdangles,holdorigin;
	float delta;

	if ((!self->target) || (!self->style))
	{
		return;
	}

	self->style = 0;	// Show spitter has been used

	delta =(float) 360 / self->count;
	VectorCopy(owner->s.angles,holdangles);
	holdangles[YAW]= 0;

	for (i1 = 0;i1 < self->count;++i1)
	{
		item = playerExport->FindItemByClassname(self->target);

		if (!item)	// Must be an object not an item
		{
			newitem = G_Spawn();
			newitem->classname = ED_NewString(self->target, true);

			AngleVectors(holdangles,forward,NULL,NULL);

			VectorCopy(self->s.origin,newitem->s.origin);
			VectorMA(newitem->s.origin, self->dmg_radius, forward, newitem->s.origin);

			if (self->mass)
				newitem->spawnflags |= self->mass;

			ED_CallSpawn(newitem);

			VectorCopy(newitem->s.origin,holdorigin);
			if (!(self->spawnflags & 1))
				gi.CreateEffect(NULL, FX_PICKUP, 0, holdorigin, "");

		}
		else
		{
			newitem = G_Spawn();
			newitem->movetype = MOVETYPE_STEP;
			AngleVectors(holdangles,forward,NULL,NULL);

			VectorCopy(self->s.origin,newitem->s.origin);
			VectorMA(newitem->s.origin, self->dmg_radius, forward, newitem->s.origin);

			if (self->mass)
				newitem->spawnflags |= self->mass;

			SpawnItem(newitem, item);

			VectorCopy(newitem->s.origin,holdorigin);
			if (!(self->spawnflags & 1))
				gi.CreateEffect(NULL, FX_PICKUP, 0, holdorigin, "");
		}

		holdangles[YAW] += delta;
	}

}

/*QUAKED item_spitter (0 .5 .8) (-4 -4 -4)  (4 4 4)	NOFLASH
When targeted it will spit out an number of items in various directions
-------SPAWN FLAGS -----------
NOFLASH - no flash is created when item is 'spit out'
---------KEYS-----------------
target - classname of item or object being spit out
count - number of items being spit out (default 1)
radius - distance from item_spitter origin that items will be spawned
spawnflags2 - the spawnflags for the item being created.
*/
void SP_item_spitter(edict_t *self)
{
	self->style = 1;	// To show it hasn't been used yet

	self->movetype = MOVETYPE_NONE;

	VectorSet(self->mins,-4,-4,-4);
	VectorSet(self->maxs,4,4,4);

	self->use = ItemSpitterSpit;

	self->solid = SOLID_NOT;

	if (!self->count)
		self->count = 1;

	self->dmg_radius = st.radius;
	if (st.spawnflags2)
		self->mass = st.spawnflags2;
	else
		self->mass = 0;

	gi.linkentity(self);
}

//=================================================================================

// update the spawner so that we will rematerialise in a different position
void respawner_touch	(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*spot = NULL;

	// if we aren't a player, forget it
	if (!other->client)
		return;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
	{
		if (!game.spawnpoint[0] && !spot->targetname)
			break;

		if (!game.spawnpoint[0] || !spot->targetname)
			continue;

		if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
			break;
	}

	if (!spot)
	{
		if (!game.spawnpoint[0])
		{	// there wasn't a spawnpoint without a target, so use any
			spot = G_Find (spot, FOFS(classname), "info_player_start");
		}
		if (!spot)
			gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
	}

	VectorSet(spot->s.origin, self->mins[0]+((self->size[0]) /2), self->mins[1]+((self->size[1]) /2) ,self->mins[2]+self->size[2]);
	VectorCopy(self->s.angles, spot->s.angles);

	G_FreeEdict(self);
}

/*QUAKED misc_update_spawner (.5 .5 .5) ?
	This creates the spawner update entity, which upates the spawner position when triggered
*/
void misc_update_spawner (edict_t *ent)
{

	ent->movetype = MOVETYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->touch = respawner_touch;

	gi.setmodel(ent,ent->model);
	gi.linkentity(ent);

}

void Teleporter_Deactivate(edict_t *self, G_Message_t *msg)
{
	self->touch = NULL;
	// if there's an effect out there, kill it
	if (self->enemy)
	{
		gi.RemoveEffects(self->enemy, FX_TELEPORT_PAD);
		if (self->enemy->PersistantCFX)
		{
			gi.RemovePersistantEffect(self->enemy->PersistantCFX, REMOVE_TELEPORT_PAD);
			self->enemy->PersistantCFX = 0;
		}
		self->enemy = NULL;
	}
}

void Teleporter_Activate(edict_t *self, G_Message_t *msg)
{
	vec3_t	real_origin;
	edict_t	*effect;

	self->touch = teleporter_touch;

	// if there's no effect already, create a new one
	if (!self->enemy)
	{
		effect = G_Spawn();
		VectorCopy(self->maxs, effect->maxs);
		VectorCopy(self->mins, effect->mins);
		effect->solid = SOLID_NOT;
		effect->s.effects |= EF_NODRAW_ALWAYS_SEND|EF_ALWAYS_ADD_EFFECTS;
		self->enemy = effect;
		gi.linkentity(effect);

		real_origin[0] = ((self->maxs[0] - self->mins[0]) / 2.0) + self->mins[0];
		real_origin[1] = ((self->maxs[1] - self->mins[1]) / 2.0) + self->mins[1];
		real_origin[2] = ((self->maxs[2] - self->mins[2]) / 2.0) + self->mins[2];

		if (!(self->spawnflags & 1))
			effect->PersistantCFX = gi.CreatePersistantEffect(effect, FX_TELEPORT_PAD, CEF_BROADCAST, real_origin, "");
	}
}

void TeleporterStaticsInit()
{
	classStatics[CID_TELEPORTER].msgReceivers[G_MSG_SUSPEND] = Teleporter_Deactivate;
	classStatics[CID_TELEPORTER].msgReceivers[G_MSG_UNSUSPEND] = Teleporter_Activate;
}

/*QUAKED misc_teleporter (1 0 0) ? NO_MODEL DEATHMATCH_RANDOM START_OFF MULT_DEST
This creates the teleporter disc that will send us places
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
-------  FIELDS  ------------------
NO_MODEL - makes teleporter invisible
DEATHMATCH_RANDOM - makes the teleporter dump you at random spawn points in deathmatch
START_OFF - Pad has no effect, and won't teleport you anywhere till its activated
MULT_DEST - pad is targeted at more than one destination
---------- KEYS -----------------
style - number of destinations this pad has.

*/
void SP_misc_teleporter (edict_t *ent)
{
	vec3_t	real_origin;
	edict_t	*effect;

	if (!ent->target && (!(ent->spawnflags&DEATHMATCH_RANDOM) || !deathmatch->value))
	{
		gi.dprintf ("teleporter without a target.\n");
		G_FreeEdict(ent);
		return;
	}

	ent->msgHandler = DefaultMsgHandler;
	ent->classID = CID_TELEPORTER;

	ent->movetype = MOVETYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	// if we don't have mult dests - probably redundant
	if (!(ent->spawnflags & 8))
		ent->style = 0;

	// if we want an effect on spawn, create it.
	if (!(ent->spawnflags & 4))
	{
		ent->touch = teleporter_touch;

		effect = G_Spawn();
		VectorCopy(ent->maxs, effect->maxs);
		VectorCopy(ent->mins, effect->mins);
		effect->solid = SOLID_NOT;
		effect->s.effects |= EF_NODRAW_ALWAYS_SEND|EF_ALWAYS_ADD_EFFECTS;
		ent->enemy = effect;
		gi.linkentity(effect);

		real_origin[0] = ((ent->maxs[0] - ent->mins[0]) / 2.0) + ent->mins[0];
		real_origin[1] = ((ent->maxs[1] - ent->mins[1]) / 2.0) + ent->mins[1];
		real_origin[2] = ((ent->maxs[2] - ent->mins[2]) / 2.0) + ent->mins[2];

		if (!(ent->spawnflags & 1))
			effect->PersistantCFX = gi.CreatePersistantEffect(effect, FX_TELEPORT_PAD, CEF_BROADCAST, real_origin, "");
	}

}

/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)

	Point teleporters at these.
*/
void SP_misc_teleporter_dest (edict_t *ent)
{
	trace_t		tr;
	vec3_t		endpos;

	ent->s.skinnum = 0;
	ent->solid = SOLID_NOT;
	VectorSet(ent->mins, -32, -32, -24);
	VectorSet(ent->maxs, 32, 32, -16);
	gi.linkentity(ent);

	VectorCopy(ent->s.origin, endpos);
	endpos[2] -= 500;
	tr = gi.trace(ent->s.origin, vec3_origin, vec3_origin, endpos, NULL, CONTENTS_WORLD_ONLY|MASK_PLAYERSOLID);

	VectorCopy(tr.endpos,ent->last_org);
	ent->last_org[2] -= mins[2];

}



extern void use_target_changelevel (edict_t *self, edict_t *other, edict_t *activator);
void misc_magic_portal_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t *ent=NULL;

	if (level.time < self->touch_debounce_time)
		return;

	if (!other->client)		// Not a player
		return;

	ent = G_Find (ent, FOFS(targetname), self->target);
	if (!ent)
	{	// No target.  Don't do anything.
//		Com_Printf("Portal has no target.\n");
	}
	else
	{
		use_target_changelevel(ent, self, other);
	}

	self->touch_debounce_time = level.time + 4.0;

	return;
}


void misc_magic_portal_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (level.time < self->impact_debounce_time)
		return;

	if (self->solid == SOLID_NOT)
	{
		int style = self->style;
		int count = self->count;
		// We aren't engaged yet.  Make solid and start the effect.
		self->solid = SOLID_TRIGGER;
		self->touch = misc_magic_portal_touch;
		self->PersistantCFX = gi.CreatePersistantEffect(self, FX_MAGIC_PORTAL, CEF_BROADCAST, self->s.origin,
								"vbb", self->s.angles, (byte)style, (byte)count);
		self->s.effects &= ~EF_DISABLE_EXTRA_FX;
	}
	else
	{	// We were on, now turn it off.
		self->solid = SOLID_NOT;
		self->touch = NULL;
		// remove the persistant effect
		if (self->PersistantCFX)
		{
			gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_PORTAL);
			self->PersistantCFX = 0;
		}

		self->s.effects |= EF_DISABLE_EXTRA_FX;
	}

	gi.linkentity(self);

	self->impact_debounce_time = level.time + 4.0;
}



#define START_OFF	1

/*QUAKED misc_magic_portal (1 .5 0) (-16 -16 -32) (16 16 32)  START_OFF
A magical glowing portal. Triggerable.
-------  FIELDS  ------------------
START_OFF - portal will start off
-----------------------------------
angles - manipulates the facing of the effect as normal.
style - 0-blue, 1-red, 2-green
count - Close after 1-255 seconds.  0 means stay until triggered.

In order to be functional as a world teleport,
  it must target a target_changelevel

*/
void SP_misc_magic_portal (edict_t *self)
{
	// Set up the basics.
	VectorSet(self->mins, -16, -16, -32);
	VectorSet(self->maxs, 16, 16, 32);
	VectorSet(self->rrs.scale, 1, 1, 1);
	self->mass = 250;
	self->friction = 0;
	self->gravity = 0;
	self->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	self->svflags |= SVF_ALWAYS_SEND;
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->touch = NULL;

	self->use = misc_magic_portal_use;

	if (!(self->spawnflags & START_OFF))
	{	// Set up the touch function, since this baby is live.
		misc_magic_portal_use(self, NULL, NULL);
	}

	gi.linkentity(self);
}



void flame_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->damage_debounce_time < level.time)
	{
		other->damage_debounce_time = level.time + 0.2;

		T_Damage (other, world, world, other->s.origin, vec3_origin, vec3_origin, 10, 0, DAMAGE_SPELL|DAMAGE_AVOID_ARMOR,MOD_DIED);//Fixme: propper params.
	}
}

void flame_think (edict_t *self)
{
//	self->s.frame++;
//	if (self->s.frame > 5)
//		self->s.frame = 0;

//	self->nextthink = level.time + FRAMETIME;
	self->nextthink = level.time + 200;
}


void soundambient_think(edict_t *self)
{
	byte	style,wait,attenuation,volume;

	attenuation = Q_ftol(self->attenuation);
	volume = Q_ftol(self->volume * 255);

	self->s.sound_data = (volume & ENT_VOL_MASK) | attenuation;

	// if its a looping sound, create it on this entity
	switch((int)(self->style))
	{
	case AS_FIRE:
		self->s.sound = gi.soundindex("ambient/fireplace.wav");
		break;
	case AS_WATERLAPPING:
		self->s.sound = gi.soundindex("ambient/waterlap.wav");
		break;
	case AS_OCEAN:
		self->s.sound = gi.soundindex("ambient/ocean.wav");
		break;
	case AS_SMALLFOUNTAIN:
		self->s.sound = gi.soundindex("ambient/smallfountain.wav");
		break;
	case AS_LARGEFOUNTAIN:
		self->s.sound = gi.soundindex("ambient/fountainloop.wav");
		break;
	case AS_SEWERWATER:
		self->s.sound = gi.soundindex("ambient/sewerflow.wav");
		break;
	case AS_OUTSIDEWATERWAY:
		self->s.sound = gi.soundindex("ambient/river.wav");
		break;
	case AS_CAULDRONBUBBLE:
		self->s.sound = gi.soundindex("ambient/cauldronbubble.wav");
		break;
	case AS_HUGEWATERFALL:
		self->s.sound = gi.soundindex("ambient/hugewaterfall.wav");
		break;
	case AS_MUDPOOL:
		self->s.sound = gi.soundindex("ambient/mudpool.wav");
		break;
	case AS_WINDEERIE:
		self->s.sound = gi.soundindex("ambient/windeerie.wav");
		break;
	case AS_WINDNOISY:
		self->s.sound = gi.soundindex("ambient/windnoisy.wav");
		break;
	case AS_WINDSOFTHI:
		self->s.sound = gi.soundindex("ambient/windsofthi.wav");
		break;
	case AS_WINDSOFTLO:
		self->s.sound = gi.soundindex("ambient/windsoftlow.wav");
		break;
	case AS_WINDSTRONG1:
		self->s.sound = gi.soundindex("ambient/windstrong1.wav");
		break;
	case AS_WINDSTRONG2:
		self->s.sound = gi.soundindex("ambient/windstrong2.wav");
		break;
	case AS_WINDWHISTLE:
		self->s.sound = gi.soundindex("ambient/windwhistle.wav");
		break;
	case AS_CONVEYOR:
		self->s.sound = gi.soundindex("objects/conveyor.wav");
		break;
	case AS_BUCKETCONVEYOR:
		self->s.sound = gi.soundindex("objects/bucketconveyor.wav");
		break;
	case AS_SPIT:
		self->s.sound = gi.soundindex("objects/spit.wav");
		break;
	default:
		style = Q_ftol(self->style);
		wait = Q_ftol(self->wait);
		gi.CreatePersistantEffect(self,
					FX_SOUND,
					CEF_BROADCAST | CEF_OWNERS_ORIGIN,
					self->s.origin,
					"bbbb",
					style,attenuation,volume,wait);
		break;
	}
	self->count = 1;	// This is just a flag to show it's on

	self->think = NULL;
}


void sound_ambient_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->count)	// This is just a flag to show it's on
	{
		self->count = 0;
		gi.RemoveEffects(self,0);
	}
	else
		soundambient_think(self);
}


void sound_ambient_init(edict_t *self)
{
	VectorSet(self->mins,-4,-4,-4);
	VectorSet(self->maxs,4,4,4);

	self->movetype = MOVETYPE_NONE;

	if (self->attenuation <= 0.01)
		self->attenuation = 1;

	if (self->volume <= 0.01)
		self->volume = .5;

	if (self->wait<1)
		self->wait = 10;

	self->s.effects |= EF_NODRAW_ALWAYS_SEND;

	if (!(self->spawnflags & 2))
	{
		self->nextthink = level.time + 2.5;
		self->think = soundambient_think;
	}

	// if we are asked to do a sound of type zero, free this edict, since its obviously bogus
	if (!self->style)
	{
		gi.dprintf("Bogus ambient sound at x:%f y:%f z:%f\n",self->s.origin[0], self->s.origin[1],self->s.origin[2]);
		G_SetToFree(self);
		return;
	}

	// if we are non local, clear the origin of this object
	if (self->spawnflags & 1)
	{
		VectorClear(self->s.origin);
	}
	else
		// if we are here, then this ambient sound should have an origin
		assert(Vec3NotZero(self->s.origin));

	self->use = sound_ambient_use;

	gi.linkentity(self);
}




/*QUAKED sound_ambient_cloudfortress (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL START_OFF
Generates an ambient sound for cloud fortress levels
-------  FLAGS  ------------------
NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
wait    amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds)
START_OFF - starts off, can be triggered on
-------  KEYS  ------------------
style
1 - Cauldron bubbling (looping sound)
2 - wind, low, eerie (looping)
3 - wind, low, noisy (looping)
4 - wind, high, soft (looping)
5 - wind, low, soft (looping)
6 - wind, low, strong (looping)
7 - wind, high, strong (looping)
8 - wind, whistling, strong (looping)

attenuation  (how quickly sound drops off from origin)
   0 - heard over entire level (default)
   1 -
   2 -
   3 - diminish very rapidly with distance

volume   range of .1 to 1   (default .5)
  0 - silent
  1 - full volume
-----------------------------------
*/
void SP_sound_ambient_cloudfortress (edict_t *self)
{
	sound_ambient_init(self);

	self->style = CloudSoundID[self->style];
}


/*QUAKED sound_ambient_mine (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL START_OFF
Generates an ambient sound for mine levels
-------  FLAGS  ------------------
NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
wait    amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds)
START_OFF - starts off, can be triggered on
-------  KEYS  ------------------
style
1 - Mud pool bubbling (looping)
2 - Rocks falling (3 sounds)
3 - wind, low, eerie (looping)
4 - wind, low, soft (looping)
5 - conveyor belt (looping)
6 - bucket conveyor belt (looping)
7 - three different creaks of heavy timbers

attenuation  (how quickly sound drops off from origin)
   0 - heard over entire level (default)
   1 -
   2 -
   3 - diminish very rapidly with distance

volume   range of .1 to 1   (default .5)
  0 - silent
  1 - full volume
-----------------------------------
*/
void SP_sound_ambient_mine (edict_t *self)
{
	sound_ambient_init(self);

	self->style = MineSoundID[self->style];
}


/*QUAKED sound_ambient_hive (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL  START_OFF
Generates an ambient sound for hive levels
-------  FLAGS  ------------------
NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
wait    amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds)
START_OFF - starts off, can be triggered on
-------  KEYS  ------------------
style
1 - gong
2 - wind, low, eerie (looping)
3 - wind, low, noisy (looping)
4 - wind, high, soft (looping)
5 - wind, low, soft (looping)
6 - wind, low, strong (looping)
7 - wind, high, strong (looping)
8 - wind, whistling, strong (looping)

attenuation  (how quickly sound drops off from origin)
   0 - heard over entire level (default)
   1 -
   2 -
   3 - diminish very rapidly with distance

volume   range of .1 to 1   (default .5)
  0 - silent
  1 - full volume
-----------------------------------
*/
void SP_sound_ambient_hive (edict_t *self)
{
	sound_ambient_init(self);

	self->style = HiveSoundID[self->style];
}


/*QUAKED sound_ambient_andoria (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL  START_OFF
Generates an ambient sound for andoria levels
-------  FLAGS  ------------------
NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
wait    amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds)
START_OFF - starts off, can be triggered on
-------  KEYS  ------------------
style
1 - small fountain (constant loop)
2 - large fountain (constant loop)
3 - water running out of sewer (constant loop)
4 - rushing waterway outside (constant loop)
5 - wind chime

attenuation  (how quickly sound drops off from origin)
   0 - heard over entire level (default)
   1 -
   2 -
   3 - diminish very rapidly with distance

volume   range of .1 to 1   (default .5)
  0 - silent
  1 - full volume
-----------------------------------
*/
void SP_sound_ambient_andoria (edict_t *self)
{
	sound_ambient_init(self);

	self->style = AndoriaSoundID[self->style];
}

/*QUAKED sound_ambient_swampcanyon (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL  START_OFF
Generates an ambient sound for swamp or canyon levels
-------  FLAGS  ------------------
NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
wait    amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds)
START_OFF - starts off, can be triggered on
-------  KEYS  ------------------
style
1 - bird, quick, high pitch
2 - bird, low, medium pitch
3 - huge waterfall
4 - mud pool bubbling (looping)
5 - wind, low, eerie (looping)
6 - wind, low, noisy (looping)
7 - wind, high, soft (looping)
8 - wind, low, soft (looping)
9 - wind, low, strong (looping)
10 - wind, high, strong (looping)
11 - wind, whistling, strong (looping)


attenuation  (how quickly sound drops off from origin)
   0 - heard over entire level (default)
   1 -
   2 -
   3 - diminish very rapidly with distance

volume   range of .1 to 1   (default .5)
  0 - silent
  1 - full volume
-----------------------------------
*/
void SP_sound_ambient_swampcanyon (edict_t *self)
{
	self->style = SwampCanyonSoundID[self->style];
	sound_ambient_init(self);
}

/*QUAKED sound_ambient_silverspring (1 0 0) (-4 -4 -4) (4 4 4) NON_LOCAL  START_OFF
Generates an ambient sound for silverspring levels
-------  FLAGS  ------------------
NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
wait    amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds)
START_OFF - starts off, can be triggered on
-------  KEYS  ------------------
style :
1 - fire (looping)
2 - water lapping (looping)
3 - seagulls (2 random calls)
4 - ocean
5 - birds (10 random bird calls)
6 - crickets (3 random chirps)
7 - frogs (2 random ribbets)
8 - distant women/children crying (4 total)
9 - mosquitoes (2 random sounds)
10 - bubbles
11 - bell tolling
12 - footsteps (3 random sounds)
13 - moans/screams/coughing (5 random sounds)
14 - Sewer drips (3 random sounds)
15 - Water drips (3 random sounds)
16 - Solid drips - heavy liquid (3 random sounds)
17 - Cauldron bubbling (looping sound)
18 - Creaking for the spit that's holding the elf over a fire

attenuation  (how quickly sound drops off from origin)
0 - heard over entire level (default)
1 -
2 -
3 - diminish very rapidly with distance

volume   range of .1 to 1   (default .5)
  0 - silent
  1 - full volume
-----------------------------------
*/
void
SP_sound_ambient_silverspring(edict_t *self)
{
	self->style = SilverSpringSoundID[self->style];
	sound_ambient_init(self);
}

/*QUAKED misc_remote_camera (0 0.5 0.8) (-4 -4 -4) (4 4 4) ACTIVATING SCRIPTED NO_DELETE

	pathtarget	- holds the name of the camera's owner entity (if any).
	target		- holds the name of the entity to be looked at.
	spawnflags	- 1 only the activating client will see the remote camera view.
	            - 2 this is a scripted camera
				- 4 don't delete camera
*/

void
remove_camera(edict_t *self)
{
	if(self->spawnflags & 1)
	{
		/* Just for the activator. */
		self->activator->client->RemoteCameraLockCount--;
	}
	else
	{
		/* for all clients */
		int i;

		for(i = 0; i < game.maxclients; i++)
		{
			edict_t *cl_ent;

			cl_ent = g_edicts + 1 + i;

			if(!cl_ent->inuse)
			{
				continue;
			}

			cl_ent->client->RemoteCameraLockCount--;
		}
	}

	if(!(self->spawnflags & 4))
	{
		G_FreeEdict(self);
	}
}

void
misc_remote_camera_think(edict_t *self)
{
	// ********************************************************************************************
	// Attempt to find my owner entity (i.e. what I'm fixed to). If nothing is found, then my
	// position will remain unchanged.
	// ********************************************************************************************

	if(self->pathtarget)
	{
		self->enemy = G_Find(NULL, FOFS(targetname), self->pathtarget);
	}

	if(self->enemy || (self->spawnflags & 2))
	{
		/* I am attatched to another (possibly moving) entity, so update my position. */
		if(self->enemy)
		{
			VectorCopy(self->enemy->s.origin, self->s.origin);
		}

#if 0
		// Update the position on client(s).

		if(self->spawnflags & 1)
		{
			// Just for the activator.

			if(self->activator->client->RemoteCameraNumber == self->s.number)
			{
				VectorCopy(self->s.origin, self->activator->client->ps.remote_vieworigin);
			}
		}
		else
		{
			/* For all clients. */
			int i;

			for(i = 0; i < game.maxclients; i++)
			{
				edict_t *cl_ent;

				cl_ent = g_edicts + 1 + i;

				if(!cl_ent->inuse)
				{
					continue;
				}

				if(cl_ent->client->RemoteCameraNumber == self->s.number)
				{
					VectorCopy(self->s.origin, cl_ent->client->ps.remote_vieworigin);
				}
			}
		}
#endif
	}

	// ********************************************************************************************
	// Find my target entity and then orientate myself to look at it.
	// ********************************************************************************************

	if((self->teamchain = G_Find(NULL, FOFS(targetname), self->target)))
	{
		/* Calculate the angles from myself to my target. */
		vec3_t	forward;

		VectorSubtract(self->teamchain->s.origin, self->s.origin, forward);
		VectorNormalize(forward);
		VectoAngles(forward, self->s.angles);
		self->s.angles[PITCH] = -self->s.angles[PITCH];

#if 0
		/* Update the angles on client(s). */
		if(self->spawnflags & 1)
		{
			/* Just for the activator. */
			if(self->activator->client->RemoteCameraNumber == self->s.number)
			{
				VectorCopy(self->s.angles, self->activator->client->ps.remote_viewangles);
			}
		}
		else
		{
			/* For all clients */
			int i;

			for(i=0;i<game.maxclients;i++)
			{
				edict_t *cl_ent;

				cl_ent = g_edicts + 1 + i;

				if(!cl_ent->inuse)
				{
					continue;
				}

				if(cl_ent->client->RemoteCameraNumber == self->s.number)
				{
					VectorCopy(self->s.angles, cl_ent->client->ps.remote_viewangles);
				}
			}
		}
#endif
	}

	// Think again or remove myself?

	if (self->spawnflags & 2)
	{
		self->nextthink = level.time+FRAMETIME;
	}
	else
	{
		self->delay -= 0.1;

		if (self->delay >= 0.0)
		{
			self->nextthink = level.time + 0.1;
		}
		else
		{
			remove_camera(self);
		}
	}
}

void
Use_misc_remote_camera(edict_t *self, edict_t *other, edict_t *activator)
{
	vec3_t forward;

	// ********************************************************************************************
	// If I am already active, just return, else flag that I am active.
	// ********************************************************************************************

	if (self->count)
	{
		if (self->spawnflags & 2)
		{
			/* I am a scripted camera, so free myself before returning. */
			remove_camera(self);
		}

		return;
	}

	self->count = 1;

	// ********************************************************************************************
	// Signal to client(s) that a remote camera view is active,
	// ********************************************************************************************

	if (self->spawnflags & 1)
	{
		// Signal to just the activator (i.e. person who was ultimately responsible for triggering the
		// remote camera) that their camera view has changed to a remote camera view..

		self->activator = activator;
		self->activator->client->RemoteCameraLockCount++;
		self->activator->client->RemoteCameraNumber = self->s.number;
	}
	else
	{
		/* Signal to all clients that their camera view has changed to a remote camera view.. */
		int i;

		for (i = 0; i < game.maxclients; i++)
		{
			edict_t *cl_ent;

			cl_ent = g_edicts + 1 + i;

			if (!cl_ent->inuse)
			{
				continue;
			}

			cl_ent->client->RemoteCameraLockCount ++;

			cl_ent->client->RemoteCameraNumber = self->s.number;
		}
	}

	// ********************************************************************************************
	// Attempt to find my owner entity (i.e. what I'm fixed to). If nothing is found, then I am a
	// static camera so set up my position here (it will remain unchanged hereafter).
	// ********************************************************************************************

	if(!self->pathtarget)
	{
		// I am static, so set up my position (which will not change hereafter).

		if(self->spawnflags & 1)
		{
			/* Just for the activator. */
			self->enemy = NULL;
#if 0
			VectorCopy(self->s.origin, self->activator->client->ps.remote_vieworigin);
#endif
		}
		else
		{
			/* For all clients. */
			int i;

			self->enemy = NULL;

			for (i = 0; i < game.maxclients; i++)
			{
				edict_t *cl_ent;

				cl_ent = g_edicts + 1 + i;

				if (!cl_ent->inuse)
				{
					continue;
				}
#if 0
				VectorCopy(self->s.origin, cl_ent->client->ps.remote_vieworigin);
#endif
			}
		}
	}
	else
	{
		self->enemy = G_Find(NULL, FOFS(targetname), self->pathtarget);

		if (self->enemy || (self->spawnflags & 2))
		{
			/* I am attatched to another (possibly moving) entity, so update my position. */
			if(self->enemy)
			{
				VectorCopy(self->enemy->s.origin, self->s.origin);
			}

#if 0
			/* Update the position on client(s). */
			if (self->spawnflags&1)
			{
				/* Just for the activator. */
				if(self->activator->client->RemoteCameraNumber == self->s.number)
				{
					VectorCopy(self->s.origin, self->activator->client->ps.remote_vieworigin);
				}
			}
			else
			{
				/* For all clients. */
				int	i;

				for (i = 0; i < game.maxclients; i++)
				{
					edict_t *cl_ent;

					cl_ent=g_edicts+1+i;

					if(!cl_ent->inuse)
					{
						continue;
					}

					if(cl_ent->client->RemoteCameraNumber == self->s.number)
					{
						VectorCopy(self->s.origin, cl_ent->client->ps.remote_vieworigin);
					}
				}
			}
#endif
		}
	}
	// ********************************************************************************************
	// Find my target entity and then orientate myself to look at it.
	// ********************************************************************************************

	self->teamchain = G_Find(NULL, FOFS(targetname), self->target);
	VectorSubtract(self->teamchain->s.origin, self->s.origin, forward);
	VectorNormalize(forward);
	VectoAngles(forward, self->s.angles);
	self->s.angles[PITCH] = -self->s.angles[PITCH];

#if 0
	/* Update the angles on client(s). */
	if (self->spawnflags & 1)
	{
		/* Just for the activator. */
		if(self->activator->client->RemoteCameraNumber == self->s.number)
		{
			VectorCopy(self->s.angles, self->activator->client->ps.remote_viewangles);
		}
	}
	else
	{
		/* For all clients. */
		int		i;

		for (i = 0; i < game.maxclients; i++)
		{
			edict_t *cl_ent;

			cl_ent = g_edicts + 1 + i;

			if(!cl_ent->inuse)
			{
				continue;
			}

			if (cl_ent->client->RemoteCameraNumber == self->s.number)
			{
				VectorCopy(self->s.angles[j], cl_ent->client->ps.remote_viewangles);
			}
		}
	}
#endif

	// ********************************************************************************************
	// Setup next think stuff.
	// ********************************************************************************************

	self->think = misc_remote_camera_think;
	self->nextthink = level.time + FRAMETIME;
}

void
SP_misc_remote_camera(edict_t *self)
{
	self->enemy = self->teamchain = NULL;

	if (!self->target)
	{
		gi.dprintf("Object 'misc_remote_camera' without a target.\n");
		G_FreeEdict(self);
		return;
	}

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet(self->mins,-4, -4, -4);
	VectorSet(self->maxs, 4, 4, 4);
	self->count = 0;

	self->use = Use_misc_remote_camera;
	gi.linkentity(self);
}

// Spawns a client model animation
// spawnflags & 2 is a designer flag whether to animate or not
// If the model is supposed to animate, the hi bit of the type is set
// If the model is static, then the default frame stored on the client is used
// Valid scale ranges from 1/50th to 5
void
SpawnClientAnim(edict_t *self, byte type, char *sound)
{
	int		scale, skin;

	if (self->spawnflags & 2)	// Animate it
	{
		type |= 0x80;
		if(sound)
		{
			self->s.sound = gi.soundindex(sound);
			self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;
		}
	}
	scale = (byte)(AVG_VEC3T(self->rrs.scale) * 50);
	assert((scale > 0) && (scale < 255));
	skin = (byte)self->s.skinnum;

//	self->svflags |= SVF_ALWAYS_SEND;
	self->PersistantCFX = gi.CreatePersistantEffect(self,
							FX_ANIMATE,
							CEF_BROADCAST,
							self->s.origin,
							"bbbv", type, scale, skin, self->s.angles);

	self->s.effects |= EF_ALWAYS_ADD_EFFECTS;
}

//A check to see if ent should reflect
qboolean
EntReflecting(edict_t *ent, qboolean checkmonster, qboolean checkplayer)
{
	if(!ent)
	{
		return false;
	}

	if(checkmonster)
	{
		if(ent->svflags & SVF_MONSTER && ent->svflags & SVF_REFLECT)
		{
			return true;
		}
	}

	if(checkplayer)
	{
		if(ent->client)
		{
			if(ent->client->playerinfo.reflect_timer > level.time)
			{
				return true;
			}
			// possibly, we might want to reflect this if the player has gold armor
			else
			if((ent->client->playerinfo.pers.armortype == ARMOR_TYPE_GOLD) && (ent->client->playerinfo.pers.armor_count) && (irand(0,100) < 30))
				return true;

		}
	}

	return false;
}

void
SkyFlyCheck(edict_t *self)
{
	if(self->s.origin[2] > 3900)
		G_FreeEdict(self);
	else
		self->nextthink = level.time + 0.1;
}

void
SkyFly(edict_t *self)
{
/*	if(deathmatch->value)
	{*/
		G_SetToFree(self);
		return;
/*	}
//They're not being drawn, even after this is set- why not?  Did they stop?
	self->svflags |= SVF_ALWAYS_SEND;
	self->movetype = MOVETYPE_NOCLIP;
	self->solid = SOLID_NOT;

	self->touch = NULL;
	self->isBlocked = NULL;
	self->isBlocking = NULL;
	self->bounced = NULL;
//or just remove self after a time
	self->think = SkyFlyCheck;
	self->nextthink = level.time + 0.1;
*/
}

void
fire_spark_think(edict_t *self)
{
	if(self->delay && self->delay < level.time)
	{
		G_FreeEdict(self);
		return;
	}

	self->think = fire_spark_think;
	self->nextthink = level.time + 0.1;//self->wait;
}

void fire_spark_gone (edict_t *self, edict_t *other, edict_t *activator)
{
	self->use = NULL;
	gi.RemoveEffects(self, FX_SPARKS);
	G_FreeEdict(self);
}

void
fire_spark_use(edict_t *self, edict_t *other, edict_t *activator)
{
	gi.CreateEffect(self, FX_SPARKS, CEF_FLAG6|CEF_FLAG7|CEF_FLAG8, self->s.origin, "d", vec3_up);

	self->use = fire_spark_gone;

	self->think = fire_spark_think;
	self->nextthink = level.time + 0.1;
}

/*QUAKED misc_fire_sparker (0 0 0) (-4 -4 0) (4 4 8) FIREBALL

  FIREBALL - more of a poofy fireball trail

  Fires of sparks when used...
  used a second time removes it

  "delay" - how long to live for... (default is forever)
*/

void SP_misc_fire_sparker (edict_t *self)
{

	if(self->spawnflags & 1)
		self->s.effects |= EF_MARCUS_FLAG1;

	self->svflags |= SVF_ALWAYS_SEND;
	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NOCLIP;
	self->clipmask = 0;

	self->use = fire_spark_use;
}

/*
 * QUAKED misc_flare (1.0 1.0 0.0) (-32 -32 -32) (32 32 32) RED GREEN BLUE LOCK_ANGLE
 * Creates a flare seen in the N64 version.
 */
#define SPAWNFLAG_FLARE_RED 1
#define SPAWNFLAG_FLARE_GREEN 2
#define SPAWNFLAG_FLARE_BLUE 4
#define SPAWNFLAG_FLARE_LOCK_ANGLE 8

void
misc_flare_use(edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->svflags ^= SVF_NOCLIENT;
	gi.linkentity(ent);
}

void
SP_misc_flare(edict_t* ent)
{
	int i;

	ent->s.modelindex = gi.modelindex("misc/flare.tga");
	ent->s.frame = 0;
	ent->s.renderfx = RF_FLARE;
	ent->solid = SOLID_NOT;

	/* Radius saved to scale */
	for (i = 0; i < 3; i++)
	{
		ent->rrs.scale[i] = st.radius;
	}

	if (ent->spawnflags & SPAWNFLAG_FLARE_RED)
	{
		ent->s.renderfx |= RF_SHELL_RED;
	}

	if (ent->spawnflags & SPAWNFLAG_FLARE_GREEN)
	{
		ent->s.renderfx |= RF_SHELL_GREEN;
	}

	if (ent->spawnflags & SPAWNFLAG_FLARE_BLUE)
	{
		ent->s.renderfx |= RF_SHELL_BLUE;
	}

	if (ent->spawnflags & SPAWNFLAG_FLARE_LOCK_ANGLE)
	{
		ent->s.renderfx |= RF_FLARE_LOCK_ANGLE;
	}

	if (st.image && *st.image)
	{
		ent->s.renderfx |= RF_CUSTOMSKIN;
		ent->s.modelindex = gi.modelindex(st.image);
	}

	VectorSet(ent->mins, -32, -32, -32);
	VectorSet(ent->maxs, 32, 32, 32);

	ent->s.modelindex2 = st.fade_start_dist;
	ent->s.modelindex3 = st.fade_end_dist;
	ent->s.skinnum = st.rgba;
	if (!ent->s.skinnum)
	{
		ent->s.skinnum = -1;
	}

	if (ent->targetname)
	{
		ent->use = misc_flare_use;
	}

	gi.linkentity(ent);
}

void
misc_player_mannequin_use(edict_t * self, edict_t * other, edict_t * activator)
{
	self->monsterinfo.aiflags |= AI_TARGET_ANGER;
	self->enemy = activator;

#if 0
	switch ( self->count )
	{
		case GESTURE_FLIP_OFF:
			self->s.frame = FRAME_flip01;
			self->monsterinfo.nextframe = FRAME_flip12;
			break;

		case GESTURE_SALUTE:
			self->s.frame = FRAME_salute01;
			self->monsterinfo.nextframe = FRAME_salute11;
			break;

		case GESTURE_TAUNT:
			self->s.frame = FRAME_taunt01;
			self->monsterinfo.nextframe = FRAME_taunt17;
			break;

		case GESTURE_WAVE:
			self->s.frame = FRAME_wave01;
			self->monsterinfo.nextframe = FRAME_wave11;
			break;

		case GESTURE_POINT:
			self->s.frame = FRAME_point01;
			self->monsterinfo.nextframe = FRAME_point12;
			break;
	}
#endif
}

void
misc_player_mannequin_think(edict_t * self)
{
	if (self->last_sound_time <= level.time)
	{
		self->s.frame++;
#if 0
		if ((self->monsterinfo.aiflags & AI_TARGET_ANGER) == 0)
		{
			if (self->s.frame > FRAME_stand40)
			{
				self->s.frame = FRAME_stand01;
			}
		}
		else
		{
			if (self->s.frame > self->monsterinfo.nextframe)
			{
				self->s.frame = FRAME_stand01;
				self->monsterinfo.aiflags &= ~AI_TARGET_ANGER;
				self->enemy = NULL;
			}
		}
#endif

		self->last_sound_time = level.time + FRAMETIME;
	}

	if (self->enemy)
	{
		vec3_t vec;

		VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
		self->ideal_yaw = vectoyaw(vec);
		M_ChangeYaw(self);
	}

	self->nextthink = level.time + FRAMETIME;
}

void
SetupMannequinModel(edict_t * self, int modelType, const char *weapon, const char *skin)
{
	const char *model_name = NULL;
	const char *default_skin = NULL;

	switch (modelType)
	{
		case 1:
			{
				self->s.skinnum = (MAX_CLIENTS - 1);
				model_name = "female";
				default_skin = "venus";
				break;
			}

		case 2:
			{
				self->s.skinnum = (MAX_CLIENTS - 2);
				model_name = "male";
				default_skin = "rampage";
				break;
			}

		case 3:
			{
				self->s.skinnum = (MAX_CLIENTS - 3);
				model_name = "cyborg";
				default_skin = "oni911";
				break;
			}

		default:
			{
				self->s.skinnum = (MAX_CLIENTS - 1);
				model_name = "female";
				default_skin = "venus";
				break;
			}
	}

	if (model_name)
	{
		char line[MAX_QPATH] = {0};

		snprintf(line, sizeof(line), "players/%s/tris.md2", model_name);
		self->model = ED_NewString(line, true);

		if (weapon)
		{
			snprintf(line, sizeof(line), "players/%s/%s.md2", model_name, weapon);
		}
		else
		{
			snprintf(line, sizeof(line), "players/%s/w_hyperblaster.md2", model_name);
		}
		self->s.modelindex2 = gi.modelindex(line);

		if (skin)
		{
			snprintf(line, sizeof(line), "%s/%s", model_name, skin);
		}
		else
		{
			snprintf(line, sizeof(line), "%s/%s", model_name, default_skin);
		}
		gi.configstring(CS_PLAYERSKINS + self->s.skinnum, line);
	}
}

/*
 * QUAKED misc_player_mannequin (1.0 1.0 0.0) (-32 -32 -32) (32 32 32)
 * Creates a player mannequin that stands around.
 *
 * NOTE: this is currently very limited, and only allows one unique model
 * from each of the three player model types.
 *
 *
 * "distance"		- Sets the type of gesture mannequin when use when triggered
 * "height"		- Sets the type of model to use ( valid numbers: 1 - 3 )
 * "goals"		- Name of the weapon to use.
 * "image"		- Name of the player skin to use.
 * "radius"		- How much to scale the model in-game
 */
void
SP_misc_player_mannequin(edict_t * self)
{
	int i;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	if (!st.effects)
	{
		self->s.effects = 0;
	}

	if (!st.renderfx)
	{
		self->s.renderfx = RF_MINLIGHT;
	}

	VectorSet(self->mins, -16, -16, -24);
	VectorSet(self->maxs, 16, 16, 32);
	self->yaw_speed = 30;
	self->ideal_yaw = 0;
	self->last_sound_time = level.time + FRAMETIME;
	self->s.modelindex = CUSTOM_PLAYER_MODEL;
	self->count = st.distance;

	SetupMannequinModel(self, st.height, st.goals, st.image);

	VectorSet(self->rrs.scale, 1.0f, 1.0f, 1.0f);
	if (ai_model_scale->value > 0.0f)
	{
		VectorSet(self->rrs.scale,
			ai_model_scale->value, ai_model_scale->value, ai_model_scale->value);
	}
	else if (st.radius > 0.0f)
	{
		VectorSet(self->rrs.scale,
			st.radius, st.radius, st.radius);
	}

	for (i = 0;i < 3; i++)
	{
		self->mins[i] *= self->rrs.scale[i];
		self->maxs[i] *= self->rrs.scale[i];
	}

	self->think = misc_player_mannequin_think;
	self->nextthink = level.time + FRAMETIME;

	if (self->targetname)
	{
		self->use = misc_player_mannequin_use;
	}

	gi.linkentity(self);
}

/*
 * QUAKED misc_model (1 0 0) (-8 -8 -8) (8 8 8)
 */
void SP_misc_model(edict_t *ent)
{
	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);
}

/*
 * QUAKED npc_timeminder (0 1 0) (-8 -8 -8) (8 8 8)
 *
 * Anachronox: Save menu open.
 */
void
touch_npc_timeminder(edict_t *self, edict_t *other, cplane_t *plane /* unused */,
		csurface_t *surf /* unused */)
{
	gi.AddCommandString("menu_savegame\n");
}

void
npc_timeminder_think(edict_t *self)
{
	M_SetAnimGroupFrame(self, "amb_b");
	self->nextthink = level.time + FRAMETIME;
}

void
SP_npc_timeminder(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (deathmatch->value)
	{
		G_FreeEdict(self);
		return;
	}

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	self->touch = touch_npc_timeminder;
	self->think = npc_timeminder_think;

	self->nextthink = level.time + FRAMETIME;

	gi.linkentity(self);
}
