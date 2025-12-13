//
// fx_tornado.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "../../common/header/common.h"
#include "client_effects.h"
#include "client_entities.h"
#include "particle.h"
#include "../common/resourcemanager.h"
#include "../common/angles.h"
#include "../common/fx.h"
#include "../common/matrix.h"
#include "../common/h2rand.h"
#include "utilities.h"
#include "ce_dlight.h"
#include "../header/g_playstats.h"

#define TORN_RADIUS			80
#define ACCEL_SCALE			3.1
#define TELEPORT_RADIUS		400.0

#define	NUM_TORN_MODELS		4

static struct model_s *torn_models[NUM_TORN_MODELS];

void
PreCacheTorn()
{
	torn_models[0] = fxi.RegisterModel("Sprites/fx/haloblue.sp2");
	torn_models[1] = fxi.RegisterModel("Sprites/Spells/flyingfist.sp2");
	torn_models[2] = fxi.RegisterModel("sprites/fx/firestreak.sp2");
	torn_models[3] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

#define BALL_RAD			40
#define BALL_PART_SCALE		10.0
#define BALL_PART_NUM		4
#define BALL_EX_PART_NUM	20

// make the tornade ball spark
static qboolean
FXTornadoBallThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t		*ce;
	paletteRGBA_t			color;
	int						part = 0;
	vec3_t					rad, fwd;
	int						i;
	int						count;

	if ((owner->current.effects & EF_SPEED_ACTIVE))
	{
		if (!self->alpha)
			return false;
		self->alpha -= 1;
		return true;
	}

	color.c = 0xffffffff;
	count = GetScaledCount(BALL_PART_NUM, 0.4);

	for (i=0; i<count; i++)
	{
		if (irand(0,1))
			part = PART_16x16_LIGHTNING;
		else
			part = PART_16x16_SPARK_B;
		ce = ClientParticle_new(part, color, 1000);
		ce->type |= PFL_ADDITIVE;
		rad[PITCH] = flrand(180, 360.0);
		rad[YAW] = flrand(0, 360.0);
		rad[ROLL] = 0.0;
		AngleVectors(rad, fwd, NULL, NULL);
		VectorScale(fwd, BALL_RAD, ce->velocity);
		VectorScale(ce->velocity, -1.0, ce->acceleration);
		ce->color.a = 245;
		ce->scale = BALL_PART_SCALE;
		ce->d_scale = -0.5*BALL_PART_SCALE;
		AddParticleToList(self, ce);
	}

	self->updateTime = 150;
	return true;
}

// create the ball that gets tossed out of Crovus when he casts the tornado spell
void
FXTornadoBall(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;

	// create the dummy entity, so particles can be attached
	glow = ClientEntity_new(type, (flags | CEF_VIEWSTATUSCHANGED) , origin, 0, 60);
	glow->Update = FXTornadoBallThink;
	glow->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
	glow->radius = 50;
	glow->LifeTime = fxi.cl->time + (TORN_DUR * 1000) + 200;

	glow->r.model = torn_models[0];
	VectorSet(glow->r.scale, 0.4, 0.4, 0.4);

	AddEffect(owner, glow);
}

static qboolean
FXTornadoThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*ce;
	paletteRGBA_t		color;
	int					part;
	int					count, i;
	float				scale;

	// if the effect is dead, just return
	if (!(owner->current.effects & EF_SPEED_ACTIVE))
	{
		self->alpha -= 1;
		return true;
	}

	color.c = 0xc0ffffff;

	count = GetScaledCount(5, 0.8);

	for (i=0; i<count; i++)
	{
		if (irand(0,1))
		{
			part = PART_16x16_SPARK_B;
			scale = flrand(6.0, 8.0);
		}
		else
		{
			part = PART_16x16_LIGHTNING;
			scale = flrand(4.0, 5.0);
		}

		ce = ClientParticle_new(part| PFL_NEARCULL | PFL_MOVE_CYL_Z, color, 1750);

		ce->scale = scale;
		ce->d_scale = (r_detail->value + 1) * 2;
		if (r_detail->value == DETAIL_LOW)
			ce->d_alpha = -220;
		else if (r_detail->value == DETAIL_NORMAL)
			ce->d_alpha = -200;
		else if (r_detail->value > DETAIL_NORMAL)
			ce->d_alpha = -180;

		if (irand(0,1))
		{
			ce->type |= PFL_PULSE_ALPHA;
			ce->d_alpha *= -2;
			ce->color.a = 1;
		}

		ce->origin[CYL_RADIUS] = 0.1*TORN_RADIUS;
		ce->origin[CYL_YAW] = flrand(0, ANGLE_360);
		ce->origin[CYL_Z] = 0;

		ce->acceleration[CYL_RADIUS] = TORN_RADIUS;
		ce->acceleration[CYL_YAW] = ANGLE_360*2.5;
		ce->acceleration[CYL_Z] = flrand(TORN_RADIUS*1.5, TORN_RADIUS*1.75);

		AddParticleToList(self, ce);
	}

	FXTornadoBallThink(self, owner);

	return true;
}

void
FXTornado(centity_t *owner,int type,int flags,vec3_t origin)
{
	client_entity_t		*glow;
	paletteRGBA_t		color;
	int					dur = 60;

	flags &= ~CEF_OWNERS_ORIGIN;

	if (r_detail->value == DETAIL_NORMAL)
		dur = 100;
	else
	if (r_detail->value == DETAIL_LOW)
		dur = 150;

	glow = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS | CEF_CHECK_OWNER | CEF_DONT_LINK | CEF_NO_DRAW, origin, 0, dur);
	glow->Update = FXTornadoThink;
	glow->radius = 50;
	glow->LifeTime = 0;

	color.c = 0xffff4444;

	if (r_detail->value >= DETAIL_HIGH)
		glow->dlight = CE_DLight_new(color, 170.0F, 00.0F);

	glow->r.flags=RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;

	AddEffect(owner, glow);
}

// explode the ball in the middle of the shrine
void
FXTornadoBallExplode(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_particle_t	*ce;
	client_entity_t		*burst;
	int					i, count;
	paletteRGBA_t		color;
	int					part;
	vec3_t				rad, fwd;
	vec3_t				angles;

	color.c = 0xffffffff;

	// create the dummy entity, so particles can be attached
	burst = ClientEntity_new(type, (flags | CEF_NO_DRAW) & ~CEF_NOMOVE , origin, 0, 1400);
	burst->radius = 100;
	AddEffect(NULL, burst);

	count = GetScaledCount(BALL_EX_PART_NUM, 0.4);
	// create a bunch of exploding particles
	for (i=0; i< count; i++)
	{
		rad[PITCH] = flrand(0, 360.0);
		rad[YAW] = flrand(0, 360.0);
		rad[ROLL] = 0.0;

		if (irand(0,1))
			part = PART_16x16_LIGHTNING;
		else
			part = PART_16x16_SPARK_B;

		ce = ClientParticle_new(part, color, 1150);

		AngleVectors(rad, fwd, NULL, NULL);
		VectorScale(fwd, BALL_RAD, ce->velocity);
		VectorScale(ce->velocity, -0.7, ce->acceleration);
		ce->color.a = 245;
		ce->scale = BALL_PART_SCALE;
		ce->d_scale = -0.5*BALL_PART_SCALE;
		AddParticleToList(burst, ce);
	}

	// create a bunch of exploding particles
	count *= 3;
	for(i = 0; i < count; i++)
	{
		color.g = color.b = irand(0,255);
		ce = ClientParticle_new(PART_4x4_WHITE, color, 400);

		VectorSet(ce->origin, 1.0,1.0,1.0);
		VectorSet(angles, flrand(0, 6.28), flrand(0, 6.28), flrand(0, 6.28));
		DirFromAngles(angles,ce->origin);
		Vec3ScaleAssign(flrand(50-10,50), ce->origin);
		VectorScale(ce->origin, -15.1, ce->acceleration);

		AddParticleToList(burst, ce);

	}

	// Add an additional flash as well.
	// ...and a big-ass flash
	burst = ClientEntity_new(-1, flags, origin, NULL, 250);
	burst->r.model = torn_models[3];
	burst->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
	burst->r.frame = 1;
	burst->radius=64;
	VectorSet(burst->r.scale, 1.0, 1.0, 1.0);
	burst->d_alpha=-4.0;
	burst->d_scale=-4.0;
	AddEffect(NULL, burst);
}
