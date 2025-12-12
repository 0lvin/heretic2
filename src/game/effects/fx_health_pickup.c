//
// fx_healthpickup.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "../../common/header/common.h"
#include "ce_defaultmessagehandler.h"
#include "client_effects.h"
#include "client_entities.h"
#include "particle.h"
#include "../common/resourcemanager.h"
#include "../common/fx.h"
#include "../common/h2rand.h"
#include "utilities.h"
#include "../common/angles.h"

#define BOB_HEIGHT		6.0
#define BOB_SPEED		ANGLE_10
#define HEALTH_RADIUS	6.0

#define	NUM_HEALTH_MODELS	2
#define HEALTH_SMALL 0
#define HEALTH_BIG   1

void PreCacheHealth()
{
}

// --------------------------------------------------------------

static qboolean
FXHealthPickupThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*p;
	paletteRGBA_t		color;

	// Rotate and bob
	self->r.angles[YAW] += ANGLE_15;
	VectorCopy(owner->current.origin, self->r.origin);
	self->r.origin[2] += (cos(self->SpawnData) * BOB_HEIGHT);
	self->SpawnData += BOB_SPEED;

	// spawn particles
	if (self->flags & CEF_FLAG6)
	{
		color.g = irand(80, 120);
		color.r = irand(210, 255);
		color.b = color.r;
	}
	else
	{
		color.r = irand(80, 120);
		color.b = irand(210, 255);
		color.g = color.r;
	}

	color.a = 255;
	p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, color, 600);

	VectorSet(p->origin, crandk() * HEALTH_RADIUS, crandk() * HEALTH_RADIUS, 0.0);
	VectorSet(p->velocity, 0.0, 0.0, flrand(20.0, 40.0));
	p->acceleration[2] = 20.0;
	AddParticleToList(self, p);

	return true;
}

void FXHealthPickup(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*ce;

	flags &= ~CEF_OWNERS_ORIGIN;
	ce = ClientEntity_new(type, flags | CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_VIEWSTATUSCHANGED, origin, NULL, 50);

	VectorCopy(ce->r.origin, ce->origin);
	ce->r.model = NULL;
	ce->r.flags = RF_GLOW | RF_TRANSLUCENT | RF_TRANS_ADD;

	if ((flags & CEF_FLAG6) >> 5)	// Full health
		VectorSet(ce->r.scale, 1.0, 1.0, 1.0);
	else
		VectorSet(ce->r.scale, 1.5, 1.5, 1.5);
	ce->radius = 10.0;
	ce->alpha = 0.8;
	ce->Update = FXHealthPickupThink;

	AddEffect(owner, ce);
}

// end
