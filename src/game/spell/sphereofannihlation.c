//
// spl_sphereofannihilation.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "../header/local.h"
#include "../common/fx.h"
#include "../common/h2rand.h"
#include "../header/utilities.h"
#include "../header/decals.h"
#include "../header/g_playstats.h"
#include "../player/library/p_main.h"	//for ITEM_WEAPON_SPHEREOFANNIHILATION
#include "../monster/beast/beast.h"

#define SPHERE_INIT_SCALE				0.7
#define SPHERE_MAX_SCALE				2.0
#define SPHERE_SCALE_RANGE				(SPHERE_MAX_SCALE - SPHERE_INIT_SCALE)
#define SPHERE_SCALE_PER_CHARGE			(SPHERE_SCALE_RANGE / SPHERE_MAX_CHARGES)
#define SPHERE_SCALE_PULSE				0.5

#define SPHERE_RADIUS_DIFF				(SPHERE_RADIUS_MAX - SPHERE_RADIUS_MIN)
#define SPHERE_RADIUS_PER_CHARGE		(SPHERE_RADIUS_DIFF / SPHERE_MAX_CHARGES)
#define SPHERE_GROW_MIN_TIME			2
#define SPHERE_GROW_SPEED				(SPHERE_RADIUS_DIFF / SPHERE_MAX_CHARGES)
#define SPHERE_GROW_START				(SPHERE_RADIUS_MIN - (SPHERE_GROW_SPEED * SPHERE_GROW_MIN_TIME))

#define SPHERE_COUNT_MIN				3
#define SPHERE_COUNT_PER_CHARGE			2

#define SPHERE_RADIUS					2.0

static void SphereOfAnnihilationGrowThink(edict_t *self);
static void SphereOfAnnihilationTouch(edict_t *self,edict_t *Other,cplane_t *Plane,csurface_t *Surface);
static void SphereWatcherGrowThink(edict_t *self);
static void SphereWatcherTouch(edict_t *self, edict_t *Other, cplane_t *Plane, csurface_t *surface);

extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);
void create_sphere(edict_t *Sphere);

// ****************************************************************************
// SphereOfAnnihilationGrowThink
// ****************************************************************************

static void SphereOfAnnihilationGrowThink(edict_t *self)
{
	vec3_t	Forward, Up, endpos;
	trace_t trace;


	if (self->owner->client)
		AngleVectors(self->owner->client->aimangles,Forward,NULL,Up);
	else
		AngleVectors(self->owner->s.angles,Forward,NULL,Up);

	// NOTE: 'edict_t'->combattarget is used as a pointer to a 'qboolean' which flags
	// whether or not I have been released. Would like a dedicated value in the 'edict_t' but this
	// is unlikely to happen, sooooo...

	// if we have released, or we are dead, or a chicken, release the sphere
	if(*(qboolean *)self->combattarget && !(self->owner->deadflag & (DEAD_DYING|DEAD_DEAD))
		&& (self->owner->client && !(self->owner->client->playerinfo.edictflags & FL_CHICKEN)) &&
		(!(self->owner->client->playerinfo.flags & PLAYER_FLAG_KNOCKDOWN)))
	{
		if (self->count < SPHERE_MAX_CHARGES)
		{
			float scale;

			self->count++;
			scale = SPHERE_INIT_SCALE + (SPHERE_SCALE_PER_CHARGE * self->count);
			VectorSet(self->rrs.scale, scale, scale, scale);
		}
		else
		{
			/* If at max size, pulse like crazy! */
			float scale;

			scale = SPHERE_MAX_SCALE + flrand(0, SPHERE_SCALE_PULSE);
			VectorSet(self->rrs.scale, scale, scale, scale);
		}

		// detect if we have teleported, since we need to move with the player if thats so
		if(self->owner->client)
		{
			VectorCopy(self->owner->s.origin, self->s.origin);
			self->s.origin[0] += Forward[0]*20.0;
			self->s.origin[1] += Forward[1]*20.0;
			self->s.origin[2] += self->owner->viewheight-5.0;
		}

		self->nextthink=level.time+0.1;
	}
	else
	{
		vec3_t	angles;
		// My caster has released me, so I am now a missile and I will fly like the wind.

		self->svflags &= ~SVF_NOCLIENT;

		self->s.effects&=~EF_MARCUS_FLAG1;

		VectorCopy(self->owner->movedir,self->movedir);

		//FIXME: Need to get rid of the 'client' reference so that anything can cast this spell.

		//Check ahead first to see if it's going to hit anything at this angle
		if(self->owner->client)
			VectorCopy(self->owner->client->aimangles, angles);
		else
			VectorCopy(self->owner->s.angles, angles);
		AngleVectors(angles, Forward, NULL, NULL);
		VectorMA(self->s.origin, SPHERE_FLY_SPEED, Forward, endpos);
		trace = gi.trace(self->s.origin, vec3_origin, vec3_origin, endpos, self->owner, MASK_MONSTERSOLID);
		if(trace.ent && ok_to_autotarget(self->owner, trace.ent))
		{//already going to hit a valid target at this angle- so don't autotarget
			VectorScale(Forward, SPHERE_FLY_SPEED, self->velocity);
		}
		else
		{//autotarget current enemy
			GetAimVelocity(self->owner->enemy, self->s.origin, SPHERE_FLY_SPEED, self->owner->client->aimangles, self->velocity);
		}
		VectorNormalize2(self->velocity, self->movedir);

		self->movetype=MOVETYPE_FLYMISSILE;
		self->solid=SOLID_BBOX;
		self->health=0;
		self->dmg = SPHERE_DAMAGE;
		self->dmg_radius = SPHERE_RADIUS_MIN + (SPHERE_RADIUS_PER_CHARGE * self->count);
		self->touch=SphereOfAnnihilationTouch;
		self->think=NULL;

		VectorSet(self->mins, -SPHERE_RADIUS, -SPHERE_RADIUS, -SPHERE_RADIUS);
		VectorSet(self->maxs, SPHERE_RADIUS, SPHERE_RADIUS, SPHERE_RADIUS);

		self->s.sound = 0;
		gi.sound(self,CHAN_WEAPON, gi.soundindex("weapons/SphereFire.wav"), 1, ATTN_NORM, 0);

		trace = gi.trace(self->s.origin, vec3_origin, vec3_origin, self->s.origin, self->owner, MASK_PLAYERSOLID);
		if (trace.startsolid)
		{
			SphereOfAnnihilationTouch(self, trace.ent, &trace.plane, trace.surface);
			return;
		}
	}
}


static void SpherePowerLaserThink(edict_t *self)
{
	vec3_t	shootDir;
	trace_t	tr;
	vec3_t	min = {-16, -16, -16};
	vec3_t	max = { 16,  16,  16};
	vec3_t	startPos;
	vec3_t	endPos;
	vec3_t	tempVect;
	vec3_t	aimangles;
	float	traceDist;
	edict_t	*traceBuddy;
	float	sphere_dist;
	int		numHit = 0;	// can hit no more than 8 guys...
	vec3_t	planedir;
	int		flags=CEF_FLAG7;
	int		damage;

	VectorCopy(self->s.origin, startPos);
	traceBuddy = self;
	sphere_dist = 2048.0;
	VectorCopy(self->s.angles,aimangles);
	AngleVectors(aimangles, shootDir, NULL, NULL);
	VectorMA(startPos, 12, shootDir, startPos);
	do
	{

		AngleVectors(aimangles, shootDir, NULL, NULL);
		VectorMA(startPos, sphere_dist, shootDir, endPos);
		tr = gi.trace(startPos, min, max, endPos, traceBuddy, MASK_SHOT);
		if(level.fighting_beast)
		{
			edict_t *ent;

			if((ent = check_hit_beast(startPos, tr.endpos)))
				tr.ent = ent;
		}

		if (!tr.ent->takedamage)
			break;

		if((tr.startsolid || tr.fraction < .99) && tr.ent)
		{
			if(EntReflecting(tr.ent, true, true))
			{

				// reflect it off into space

				// draw line to this point
				VectorSubtract(tr.endpos, startPos, tempVect);
				traceDist = VectorLength(tempVect);
				gi.CreateEffect(NULL, FX_WEAPON_SPHEREPOWER, 0, startPos, "xbb",
					shootDir, (byte)(AVG_VEC3T(self->rrs.scale) * 7.5), (byte)(traceDist/8));

				// re-constitute aimangle
				aimangles[1] += flrand(160,200);
				aimangles[0] += flrand(-20,20);

				break;
			}
			else
			{
				if (tr.ent->fire_timestamp < self->fire_timestamp)
				{
					damage = SPHERE_DAMAGE;
					T_Damage(tr.ent, self, self->owner, shootDir, tr.endpos, shootDir,
							damage, damage, DAMAGE_SPELL, MOD_P_SPHERE);
					tr.ent->fire_timestamp = self->fire_timestamp;
				}
			}
		}

		VectorSubtract(tr.endpos, startPos, tempVect);
		sphere_dist -= VectorLength(tempVect);

		VectorCopy(tr.endpos, startPos);
		// this seems to alleviate the problem of a trace hitting the same ent multiple times...
		VectorSubtract(endPos, startPos, tempVect);
		if(VectorLength(tempVect) > 16.0)
		{
			VectorMA(startPos, 16.0, shootDir, startPos);
		}
		traceBuddy = tr.ent;
		numHit++;
	}while((tr.fraction < .99)&&(tr.contents != MASK_SOLID)&&(numHit < MAX_REFLECT));

	VectorSubtract(tr.endpos, startPos, tempVect);
	traceDist = VectorLength(tempVect);
	// decide if a decal is appropriate or not

	if (self->health==2)
		flags|=CEF_FLAG8;

	if(IsDecalApplicable(self, tr.ent, self->s.origin, tr.surface, &tr.plane, planedir))
		flags|=CEF_FLAG6;

	gi.CreateEffect(NULL, FX_WEAPON_SPHEREPOWER, flags, startPos, "xbb",
			shootDir, (byte)(AVG_VEC3T(self->rrs.scale) * 7.5), (byte)(traceDist/8));

	if (--self->count <= 0)
		G_SetToFree(self);
	else
		self->nextthink = level.time + 0.1;
}


static void SpherePowerLaserTouch(edict_t *self,edict_t *Other,cplane_t *Plane, csurface_t *Surface)
{
	G_SetToFree(self);
}


static void SphereOfAnnihilationGrowThinkPower(edict_t *self)
{
	vec3_t	Forward, Right, Up;
	edict_t *laser;

	if (self->owner->client)
		AngleVectors(self->owner->client->aimangles,Forward,Right,Up);
	else
		AngleVectors(self->owner->s.angles,Forward,Right,Up);

	// NOTE: 'edict_t'->combattarget is used as a pointer to a 'qboolean' which flags
	// whether or not I have been released. Would like a dedicated value in the 'edict_t' but this
	// is unlikely to happen, sooooo...

	if(*(qboolean *)self->combattarget && !(self->owner->deadflag & (DEAD_DYING|DEAD_DEAD))
		&& (self->owner->client && !(self->owner->client->playerinfo.edictflags & FL_CHICKEN)) &&
		(!(self->owner->client->playerinfo.flags & PLAYER_FLAG_KNOCKDOWN)))
	{
		if (self->count < SPHERE_MAX_CHARGES)
		{
			float scale;

			self->count++;
			scale = SPHERE_INIT_SCALE + (SPHERE_SCALE_PER_CHARGE * self->count);
			VectorSet(self->rrs.scale, scale, scale, scale);
		}
		else
		{
			/* If at max size, pulse like crazy! */
			float scale;

			scale = SPHERE_MAX_SCALE + flrand(0, SPHERE_SCALE_PULSE);
			VectorSet(self->rrs.scale, scale, scale, scale);
		}

		// detect if we have teleported, since we need to move with the player if thats so
		if(self->owner->client)
		{
			VectorCopy(self->owner->s.origin, self->s.origin);
			self->s.origin[0] += Forward[0] * 20.0;
			self->s.origin[1] += Forward[1] * 20.0;
			self->s.origin[2] += self->owner->viewheight-5.0;
		}

		self->nextthink=level.time+0.1;
	}
	else
	{
		laser = G_Spawn();
		laser->owner = self->owner;
		laser->count = self->count-SPHERE_COUNT_MIN;
		VectorMA(self->s.origin, 10.0, Right, laser->s.origin);
		if (self->owner->client)
			VectorCopy(self->owner->client->aimangles, laser->s.angles);
		else
			VectorCopy(self->owner->s.angles, laser->s.angles);
		VectorScale(Right, SPHERE_LASER_SPEED, laser->velocity);
		laser->health = 1;		// right
		laser->movetype=MOVETYPE_FLYMISSILE;
		laser->solid=SOLID_BBOX;
		laser->clipmask=MASK_SOLID;
		laser->think = SpherePowerLaserThink;
		laser->touch = SpherePowerLaserTouch;
		laser->fire_timestamp = level.time;
		G_LinkMissile(laser);
		SpherePowerLaserThink(laser);

		laser = G_Spawn();
		laser->owner = self->owner;
		laser->count = self->count-SPHERE_COUNT_MIN;
		VectorMA(self->s.origin, -10.0, Right, laser->s.origin);
		if (self->owner->client)
			VectorCopy(self->owner->client->aimangles, laser->s.angles);
		else
			VectorCopy(self->owner->s.angles, laser->s.angles);
		VectorScale(Right, -SPHERE_LASER_SPEED, laser->velocity);
		laser->health = 2;		// left
		laser->movetype=MOVETYPE_FLYMISSILE;
		laser->solid=SOLID_BBOX;
		laser->clipmask=MASK_SOLID;
		laser->think = SpherePowerLaserThink;
		laser->touch = SpherePowerLaserTouch;
		laser->fire_timestamp = level.time;
		G_LinkMissile(laser);
		SpherePowerLaserThink(laser);

		gi.sound(self->owner,CHAN_WEAPON, gi.soundindex("weapons/SpherePowerFire.wav"), 1, ATTN_NORM, 0);
		G_SetToFree(self);
	}
}

edict_t *SphereReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t	*Sphere;
	Sphere = G_Spawn();
	create_sphere(Sphere);
	Sphere->owner = other;
	Sphere->enemy = self->enemy;
	Sphere->reflect_debounce_time = self->reflect_debounce_time -1;
	Sphere->reflected_time=self->reflected_time;

	Sphere->count=self->count;
	Sphere->solid=self->solid;
	Sphere->dmg=self->dmg;
	Sphere->dmg_radius=self->dmg_radius;
	VectorCopy(self->rrs.scale, Sphere->rrs.scale);

	VectorCopy(vel, Sphere->velocity);

	Sphere->touch=SphereOfAnnihilationTouch;
	Sphere->nextthink=level.time+0.1;

	VectorCopy(self->mins, Sphere->mins);
	VectorCopy(self->maxs, Sphere->maxs);

	VectorCopy(self->s.origin, Sphere->s.origin);
	G_LinkMissile(Sphere);

	gi.CreateEffect(Sphere,
				FX_WEAPON_SPHERE,
				CEF_OWNERS_ORIGIN,
				NULL,
				"s",
				(short)Sphere->owner->s.number);

	gi.CreateEffect(Sphere,
				FX_WEAPON_SPHEREGLOWBALLS,
				CEF_OWNERS_ORIGIN,
				NULL,
				"s",
				-1);


	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it.
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point
	gi.CreateEffect(Sphere, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", Sphere->velocity);

	return(Sphere);
}


void SphereExplodeThink(edict_t *self)
{
	edict_t *ent=NULL;

	while((ent = newfindradius(ent, self->s.origin, self->dmg_radius)))
	{
		if (ent->takedamage && ent != self->owner && ent->fire_timestamp < self->fire_timestamp)
		{
			T_Damage(ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, self->dmg, 0, 0, MOD_SPHERE);
			ent->fire_timestamp = self->fire_timestamp;
			gi.CreateEffect(ent, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", vec3_origin);
		}
	}

	self->count--;
	self->dmg_radius += SPHERE_GROW_SPEED;
	self->nextthink = level.time + 0.1;

	if (self->count < 0)
		G_SetToFree(self);
}


// ****************************************************************************
// SphereOfAnnihilationTouch
// ****************************************************************************

static void SphereOfAnnihilationTouch(edict_t *self, edict_t *other, cplane_t *Plane, csurface_t *surface)
{
	edict_t		*explosion;

	// has the target got reflection turned on ?
	if(EntReflecting(other, true, true) && self->reflect_debounce_time)
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(SPHERE_FLY_SPEED/2, self->velocity);
		SphereReflect(self, other, self->velocity);
		return;
	}

	if(surface && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	explosion = G_Spawn();
	VectorCopy(self->s.origin, explosion->s.origin);
	explosion->solid = SOLID_NOT;
	explosion->dmg_radius = SPHERE_GROW_START;
	explosion->count = self->count + SPHERE_GROW_MIN_TIME;
	explosion->fire_timestamp = level.time;
	explosion->think = SphereExplodeThink;
	explosion->owner = self->owner;
	explosion->classname = "sphere_damager";
	explosion->dmg = self->dmg;

	gi.linkentity(explosion);

	AlertMonsters (self, self->owner, 3, false);
	// Back off the origin for the damage a bit. We are a point and this will
	// help fix hitting base of a stair and not hurting a guy on next step up.
	VectorMA(self->s.origin, -8.0, self->movedir, self->s.origin);

	gi.CreateEffect(self, FX_WEAPON_SPHEREPLAYEREXPLODE, CEF_OWNERS_ORIGIN, NULL,
					"db", self->movedir, (byte)(self->count));

	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/SphereImpact.wav"), 1, ATTN_NORM, 0);

	G_SetToFree(self);

	// Do damage directly to the thing you hit.  This is mainly for big creatures, like the trial beast.
	// The sphere will not damage the other after this initial impact.
	if (other && other->takedamage)
	{
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, vec3_origin, self->dmg, 0, 0, MOD_SPHERE);
		other->fire_timestamp = level.time;
		gi.CreateEffect(other, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", vec3_origin);
	}

	SphereExplodeThink(explosion);
}

// guts of create sphere
void create_sphere(edict_t *Sphere)
{
	Sphere->svflags |= SVF_ALWAYS_SEND;
	Sphere->s.effects |= EF_ALWAYS_ADD_EFFECTS|EF_MARCUS_FLAG1;
	Sphere->classname="Spell_SphereOfAnnihilation";
	Sphere->clipmask=MASK_SHOT;
	Sphere->movetype = MOVETYPE_FLYMISSILE;
	Sphere->nextthink=level.time+0.1;

}

// ****************************************************************************
// SpellCastSphereOfAnnihilation
// ****************************************************************************

void SpellCastSphereOfAnnihilation(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,
								  float Value, float *ReleaseFlagsPtr)
{
	edict_t	*Sphere;
	int	flags;

	// Spawn the sphere of annihilation as an invisible entity (i.e. modelindex=0).

	Sphere=G_Spawn();

	if(Caster->client)
	{
		VectorCopy(Caster->s.origin, Sphere->s.origin);
		Sphere->s.origin[0] += AimDir[0]*20.0;
		Sphere->s.origin[1] += AimDir[1]*20.0;
		Sphere->s.origin[2] += Caster->viewheight-5.0;
	}
	else
		VectorCopy(StartPos, Sphere->s.origin);

	VectorCopy(AimAngles,Sphere->s.angles);

	Sphere->avelocity[YAW]=100.0;
	Sphere->avelocity[ROLL]=100.0;

	// NOTE: 'edict_t'->combattarget is used as a pointer to a 'qboolean' which flags
	// whether or not I have been released. Would like a dedicated value in the 'edict_t' but this
	// is unlikely to happen, sooooo...


	Sphere->combattarget=(char *)ReleaseFlagsPtr;

	Sphere->count = 0;
	Sphere->solid = SOLID_NOT;
	Sphere->dmg = 0;
	VectorSet(Sphere->rrs.scale,
		SPHERE_INIT_SCALE, SPHERE_INIT_SCALE, SPHERE_INIT_SCALE);
	Sphere->owner = Caster;
	Sphere->enemy = Caster->enemy;
	create_sphere(Sphere);
	Sphere->reflect_debounce_time = MAX_REFLECT;

	if (Caster->client && Caster->client->playerinfo.powerup_timer > level.time)
	{
		Sphere->think=SphereOfAnnihilationGrowThinkPower;
	}
	else
	{
		if (Caster->client)
			Sphere->think=SphereOfAnnihilationGrowThink;
		else	// The celestial watcher can also cast a sphere, but a different kind of one.
			Sphere->think=SphereWatcherGrowThink;
	}

	gi.linkentity(Sphere);

	if(!Caster->client)
		flags = 0;
	else
		flags = CEF_OWNERS_ORIGIN;

	gi.CreateEffect(Sphere,
					FX_WEAPON_SPHERE,
					flags,
					StartPos,
					"s",
					(short)Caster->s.number);

	if(!Caster->client)
		flags = CEF_FLAG6;
	else
		flags = CEF_OWNERS_ORIGIN;

	gi.CreateEffect(Sphere,
					FX_WEAPON_SPHEREGLOWBALLS,
					flags,
					StartPos,
					"s",
					(short)Caster->s.number);

	Sphere->s.sound = gi.soundindex("weapons/SphereGrow.wav");
	Sphere->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;
}




//////////////////////////////////
// For Celestial Watcher
//////////////////////////////////
#define SPHERE_WATCHER_DAMAGE_MIN				50
#define SPHERE_WATCHER_DAMAGE_RANGE				150
#define SPHERE_WATCHER_EXPLOSION_RADIUS_MIN		50.0
#define SPHERE_WATCHER_EXPLOSION_RADIUS_MAX		200.0

// ****************************************************************************
// SphereWatcherFlyThink
// ****************************************************************************

void SphereWatcherFlyThink(edict_t *self)
{
	self->count++;

	if(self->count > 20)
	{
		// End the circling...
		G_SetToFree(self);
	}
	else
	{
		self->nextthink=level.time+0.2;
	}
}


static void SphereWatcherGrowThink(edict_t *self)
{
	vec3_t	Forward, Up, endpos;
	trace_t trace;


	if (self->owner->client)
		AngleVectors(self->owner->client->aimangles,Forward,NULL,Up);
	else
		AngleVectors(self->owner->s.angles,Forward,NULL,Up);

	// NOTE: 'edict_t'->combattarget is used as a pointer to a 'qboolean' which flags
	// whether or not I have been released. Would like a dedicated value in the 'edict_t' but this
	// is unlikely to happen, sooooo...

	// if we have released, or we are dead, or a chicken, release the sphere
	if(*(qboolean *)self->combattarget && !(self->owner->deadflag & (DEAD_DYING|DEAD_DEAD)))
	{

		self->count+=irand(1,2);

		if((self->count > 10) && (AVG_VEC3T(self->rrs.scale) < SPHERE_MAX_SCALE))
		{
			if(self->count > 20)
			{
				self->rrs.scale[0] -= 0.01;
				self->rrs.scale[1] -= 0.01;
				self->rrs.scale[2] -= 0.01;
			}
			else
			{
				self->rrs.scale[0] += 0.1;
				self->rrs.scale[1] += 0.1;
				self->rrs.scale[2] += 0.1;
			}

			if(self->count>25)
			{
				self->count&=3;
			}

		}

		self->nextthink=level.time+0.1;
	}
	else
	{
		vec3_t	angles;
		// My caster has released me, so I am now a missile and I will fly like the wind.

		self->svflags &= ~SVF_NOCLIENT;

		self->s.effects&=~EF_MARCUS_FLAG1;

		VectorCopy(self->owner->movedir,self->movedir);

		//Check ahead first to see if it's going to hit anything at this angle
		VectorCopy(self->owner->s.angles, angles);
		AngleVectors(angles, Forward, NULL, NULL);
		VectorMA(self->s.origin, SPHERE_FLY_SPEED, Forward, endpos);
		trace = gi.trace(self->s.origin, vec3_origin, vec3_origin, endpos, self->owner, MASK_MONSTERSOLID);
		if(trace.ent && ok_to_autotarget(self->owner, trace.ent))
		{//already going to hit a valid target at this angle- so don't autotarget
			VectorScale(Forward, SPHERE_FLY_SPEED, self->velocity);
		}
		else
		{//autotarget current enemy
			GetAimVelocity(self->owner->enemy, self->s.origin, SPHERE_FLY_SPEED, self->s.angles, self->velocity);
		}
		VectorNormalize2(self->velocity, self->movedir);

		self->movetype=MOVETYPE_FLYMISSILE;
		self->solid=SOLID_BBOX;
		self->health=0;
		self->count=0;
		self->dmg=	SPHERE_WATCHER_DAMAGE_MIN +
					(SPHERE_WATCHER_DAMAGE_RANGE*((AVG_VEC3T(self->rrs.scale) - SPHERE_INIT_SCALE)/SPHERE_SCALE_RANGE));
		self->dmg_radius =
					SPHERE_WATCHER_EXPLOSION_RADIUS_MIN +
					(SPHERE_WATCHER_EXPLOSION_RADIUS_MAX - SPHERE_WATCHER_EXPLOSION_RADIUS_MIN) *
							(AVG_VEC3T(self->rrs.scale) - SPHERE_INIT_SCALE)/SPHERE_SCALE_RANGE;
		self->touch=SphereWatcherTouch;
		self->think=SphereWatcherFlyThink;
		self->nextthink=level.time+0.1;

		VectorSet(self->mins, -SPHERE_RADIUS, -SPHERE_RADIUS, -SPHERE_RADIUS);
		VectorSet(self->maxs, SPHERE_RADIUS, SPHERE_RADIUS, SPHERE_RADIUS);

		self->s.sound = 0;
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/SphereFire.wav"), 1, ATTN_NORM, 0);

		trace = gi.trace(self->s.origin, vec3_origin, vec3_origin, self->s.origin, self->owner, MASK_PLAYERSOLID);
		if (trace.startsolid)
		{
			SphereWatcherTouch(self, trace.ent, &trace.plane, trace.surface);
			return;
		}
	}
}



edict_t *SphereWatcherReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t	*Sphere;
	Sphere = G_Spawn();
	create_sphere(Sphere);
	Sphere->owner = other;
	Sphere->enemy = self->enemy;
	Sphere->reflect_debounce_time = self->reflect_debounce_time -1;
	Sphere->reflected_time=self->reflected_time;

	Sphere->count=self->count;
	Sphere->solid=self->solid;
	Sphere->dmg=self->dmg;
	Sphere->dmg_radius=self->dmg_radius;
	VectorCopy(self->rrs.scale, Sphere->rrs.scale);

	VectorCopy(vel, Sphere->velocity);

	Sphere->touch=SphereWatcherTouch;
	Sphere->think=SphereWatcherFlyThink;
	Sphere->nextthink=level.time+0.1;

	VectorCopy(self->mins, Sphere->mins);
	VectorCopy(self->maxs, Sphere->maxs);

	VectorCopy(self->s.origin, Sphere->s.origin);
	G_LinkMissile(Sphere);

	gi.CreateEffect(Sphere,
				FX_WEAPON_SPHERE,
				CEF_OWNERS_ORIGIN,
				NULL,
				"s",
				(short)Sphere->owner->s.number);

	gi.CreateEffect(Sphere,
				FX_WEAPON_SPHEREGLOWBALLS,
				CEF_OWNERS_ORIGIN,
				NULL,
				"s",
				-1);


	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it.
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point
	gi.CreateEffect(Sphere, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", Sphere->velocity);

	return(Sphere);
}



static void SphereWatcherTouch(edict_t *self, edict_t *Other, cplane_t *Plane, csurface_t *surface)
{
	int			makeScorch;

	// has the target got reflection turned on ?
	if(EntReflecting(Other, true, true) && self->reflect_debounce_time)
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(SPHERE_FLY_SPEED/2, self->velocity);
		SphereWatcherReflect(self, Other, self->velocity);
		return;
	}


	if(surface && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	AlertMonsters (self, self->owner, 3, false);
	if(Other->takedamage)
	{
		T_Damage(Other, self, self->owner, self->movedir, self->s.origin, Plane->normal, self->dmg, self->dmg, DAMAGE_SPELL,MOD_SPHERE);
	}
	else
	{
		// Back off the origin for the damage a bit. We are a point and this will
		// help fix hitting base of a stair and not hurting a guy on next step up.
		VectorMA(self->s.origin, -8.0, self->movedir, self->s.origin);
	}

	T_DamageRadius(self, self->owner, self, self->dmg_radius, self->dmg, self->dmg / 8, DAMAGE_ATTACKER_KNOCKBACK,MOD_SPHERE);
	makeScorch = 0;
	if(IsDecalApplicable(self, Other, self->s.origin, surface, Plane, NULL))
	{
		makeScorch = CEF_FLAG6;
	}
	gi.CreateEffect(self, FX_WEAPON_SPHEREEXPLODE, CEF_OWNERS_ORIGIN | makeScorch, NULL,
					"db", self->movedir, (byte)(AVG_VEC3T(self->rrs.scale) * 7.5));

	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/SphereImpact.wav"), 1, ATTN_NORM, 0);

	G_SetToFree(self);
}
