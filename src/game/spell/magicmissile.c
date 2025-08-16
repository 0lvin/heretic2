//
// spl_magicmissile.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "../header/local.h"
#include "../common/angles.h"
#include "../common/h2rand.h"


#define ARROW_RADIUS			2.0F

void create_magic(edict_t *MagicMissile);

static void MagicMissileThink2(edict_t *self);
static void MagicMissileTouch(edict_t *self,edict_t *Other,cplane_t *Plane,csurface_t *Surface);
// static void MagicMissileThink1(edict_t *self);


// ****************************************************************************
// MagicMissileThink2
// ****************************************************************************

static void MagicMissileThink2(edict_t *self)
{
	// Prevent any further transmission of this entity to clients.

//	self->s.effects=0;

	self->svflags|=SVF_NOCLIENT;
	self->think = NULL;
}




edict_t *MagicMissileReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t	*magicmissile;
	short shortyaw, shortpitch;

	// create a new missile to replace the old one - this is necessary cos physics will do nasty things
	// with the existing one,since we hit something. Hence, we create a new one totally.
	magicmissile = G_Spawn();

	// copy everything across
	VectorCopy(self->s.origin, magicmissile->s.origin);
	create_magic(magicmissile);
	VectorCopy(vel, magicmissile->velocity);
	VectorNormalize2(vel, magicmissile->movedir);
	AnglesFromDir(magicmissile->movedir, magicmissile->s.angles);
	magicmissile->owner = other;
	magicmissile->think = MagicMissileThink2;
	magicmissile->health = self->health;
	magicmissile->enemy = self->owner;
	magicmissile->flags |= (self->flags & FL_NO_KNOCKBACK);
	magicmissile->reflect_debounce_time = self->reflect_debounce_time -1; //so it doesn't infinitely reflect in one frame somehow
	magicmissile->reflected_time=self->reflected_time;
	G_LinkMissile(magicmissile);

	// create new trails for the new missile
	shortyaw = (short)(magicmissile->s.angles[YAW]*(65536.0/360.0));
	shortpitch = (short)(magicmissile->s.angles[PITCH]*(65536.0/360.0));

	gi.CreateEffect(magicmissile,
				FX_WEAPON_MAGICMISSILE,
				CEF_OWNERS_ORIGIN|CEF_FLAG6,
				0,
				"ss",
				shortyaw, shortpitch);

	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it.
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point
	gi.CreateEffect(magicmissile, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", magicmissile->velocity);

	return(magicmissile);
}



// ****************************************************************************
// MagicMissileTouch
// ****************************************************************************



static void MagicMissileTouch(edict_t *self,edict_t *Other,cplane_t *Plane,csurface_t *Surface)
{
	vec3_t		Origin;
	int			makeScorch;

	if (Surface&&(Surface->flags&SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	// has the target got reflection turned on ?
	if (self->reflect_debounce_time)
	{
		if (EntReflecting(Other, true, true))
		{
			Create_rand_relect_vect(self->velocity, self->velocity);
			// scale speed down
			Vec3ScaleAssign(MAGICMISSILE_SPEED/2, self->velocity);
			MagicMissileReflect(self, Other, self->velocity);

			return;
		}
	}

	if ((Other==self->owner)||(!strcmp(self->classname,Other->classname)))
	{
		return;
	}

	// Calculate the position for the explosion entity.

	VectorMA(self->s.origin,-0.02,self->velocity,Origin);

	AlertMonsters (self, self->owner, 1, false);
	if (Other->takedamage)
	{
		T_Damage(Other,self,self->owner,self->movedir,self->s.origin,Plane->normal,self->dmg,self->dmg,DAMAGE_SPELL,MOD_MMISSILE);
	}
	else
	{
		// Back off the origin for the damage a bit. We are a point and this will
		// help fix hitting base of a stair and not hurting a guy on next step up.
		VectorMA(self->s.origin,-8.0,self->movedir,self->s.origin);
	}

	// Okay, we have to do some blast damage no matter what.
	// They say that blast is too much.
	if (deathmatch->value)
	{	// Except in deathmatch the weapon is too wimpy.
		T_DamageRadius(self, self->owner, self->owner,
				MAGICMISSILE_RADIUS, MAGICMISSILE_DAMAGE_RAD, MAGICMISSILE_DAMAGE_RAD*0.25,
				DAMAGE_SPELL|DAMAGE_EXTRA_KNOCKBACK, MOD_MMISSILE);
	}

	// Attempt to apply a scorchmark decal to the thing I hit.
	makeScorch = 0;
	if (IsDecalApplicable(self,Other,self->s.origin,Surface,Plane,NULL))
	{
		makeScorch = CEF_FLAG6;
	}
	gi.CreateEffect(self, FX_WEAPON_MAGICMISSILEEXPLODE, CEF_OWNERS_ORIGIN | makeScorch, self->s.origin, "d", self->movedir);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/MagicMissileHit.wav"), 1, ATTN_NORM, 0);

	G_SetToFree(self);
}

// create guts of magice missile
void create_magic(edict_t *MagicMissile)
{
	MagicMissile->s.effects=EF_NODRAW_ALWAYS_SEND|EF_ALWAYS_ADD_EFFECTS;
	MagicMissile->movetype=MOVETYPE_FLYMISSILE;
	MagicMissile->solid=SOLID_BBOX;
	MagicMissile->classname="Spell_MagicMissile";
	MagicMissile->touch=MagicMissileTouch;
	if (deathmatch->value)
		MagicMissile->dmg=irand(MAGICMISSILE_DAMAGE_MIN/2, MAGICMISSILE_DAMAGE_MAX/2);//15 - 20
	else
		MagicMissile->dmg=irand(MAGICMISSILE_DAMAGE_MIN, MAGICMISSILE_DAMAGE_MAX);//30 - 40
	MagicMissile->clipmask=MASK_SHOT;
	VectorSet(MagicMissile->mins, -ARROW_RADIUS, -ARROW_RADIUS, -ARROW_RADIUS);
	VectorSet(MagicMissile->maxs, ARROW_RADIUS, ARROW_RADIUS, ARROW_RADIUS);
	MagicMissile->nextthink=level.time+0.1;

}

// ****************************************************************************
// SpellCastMagicMissile
// ****************************************************************************

void SpellCastMagicMissile(edict_t *caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir)
{
	edict_t	*magicmissile;
	trace_t trace;
	vec3_t	TempVec;
	short	shortyaw, shortpitch;

	// Spawn the magic-missile.

	magicmissile = G_Spawn();

	VectorNormalize2(AimDir, magicmissile->movedir);
	VectorMA(StartPos,1.0,AimDir, magicmissile->s.origin);

	create_magic(magicmissile);
	magicmissile->owner = caster;
	magicmissile->reflect_debounce_time = MAX_REFLECT;
	G_LinkMissile(magicmissile);

	trace = gi.trace(caster->s.origin, magicmissile->mins, magicmissile->maxs, magicmissile->s.origin, caster, MASK_PLAYERSOLID);
	if (trace.startsolid)
	{
		MagicMissileTouch(magicmissile, trace.ent, &trace.plane, trace.surface);
		return;
	}

	// Handle autotargeting by looking for the nearest monster that:
	// a) Lies in a 45 degree degree horizontal, 180 degree vertical cone from my facing.
	// b) Lies within 0 to 1000 meters of myself.
	// c) Is visible (i.e. LOS exists from the missile to myself).
	magicmissile->enemy = FindNearestVisibleActorInFrustum(magicmissile,
													AimAngles,
													0.0,
													1000.0,
													ANGLE_30,
													ANGLE_180,
													SVF_MONSTER,
													magicmissile->s.origin,
													NULL, NULL);

	if (magicmissile->enemy)
	{
		VectorCopy(magicmissile->s.origin, TempVec);
		VectorSubtract(magicmissile->enemy->s.origin, TempVec, TempVec);

		TempVec[0]+=(magicmissile->enemy->mins[0] + magicmissile->enemy->maxs[0])/2.0;
		TempVec[1]+=(magicmissile->enemy->mins[1] + magicmissile->enemy->maxs[1])/2.0;
		TempVec[2]+=(magicmissile->enemy->mins[2] + magicmissile->enemy->maxs[2])/2.0;

		VectorNormalize(TempVec);
		VectoAngles(TempVec, magicmissile->s.angles);
		// The pitch is flipped in these?
		magicmissile->s.angles[PITCH] = -magicmissile->s.angles[PITCH];
		VectorScale(TempVec, MAGICMISSILE_SPEED, magicmissile->velocity);
	}
	else
	{
		VectorScale(AimDir,MAGICMISSILE_SPEED, magicmissile->velocity);
		VectorCopy(AimAngles, magicmissile->s.angles);
	}

	shortyaw = (short)(magicmissile->s.angles[YAW]*(65536.0/360.0));
	shortpitch = (short)(magicmissile->s.angles[PITCH]*(65536.0/360.0));

	gi.CreateEffect(magicmissile,
				FX_WEAPON_MAGICMISSILE,
				CEF_OWNERS_ORIGIN,
				0,
				"ss",
				shortyaw, shortpitch);


	magicmissile->think = MagicMissileThink2;
	magicmissile->nextthink = level.time + 0.1;
}
