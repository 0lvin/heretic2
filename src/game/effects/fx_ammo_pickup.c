//
// fx_AmmoPickup.c
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
#include "../header/g_items.h"

/* keep it same as in game/header/local.h */
typedef enum
{
	AMMO_MANA_DEFENSIVE_HALF,
	AMMO_MANA_DEFENSIVE_FULL,
	AMMO_MANA_OFFENSIVE_HALF,
	AMMO_MANA_OFFENSIVE_FULL,
	AMMO_MANA_COMBO_QUARTER,
	AMMO_MANA_COMBO_HALF,
	AMMO_HELLSTAFF,
	AMMO_REDRAIN,
	AMMO_PHOENIX,
} itemammo_t;

#define BOB_HEIGHT		6.0
#define HEALTH_RADIUS	6.0

void PreCacheItemAmmo()
{
}

// --------------------------------------------------------------

static qboolean FXAmmoPickupThink(struct client_entity_s *self, centity_t *owner)
{

	client_particle_t	*p;
	paletteRGBA_t		color;

	if (self->SpawnInfo >= AMMO_HELLSTAFF)
	{
		return true;
	}

	/* Rotate and bob */
	self->r.angles[YAW] += ANGLE_5;
	VectorCopy(owner->current.origin, self->r.origin);
	self->r.origin[2] += (cos(self->SpawnData) * BOB_HEIGHT);
	self->SpawnData += BOB_SPEED;

	switch(self->SpawnInfo)
	{
		case AMMO_MANA_DEFENSIVE_HALF:
		case AMMO_MANA_DEFENSIVE_FULL:
			color.g = irand(50, 90);
			color.b = irand(210, 255);
			color.r = color.g;
			break;

		case AMMO_MANA_OFFENSIVE_HALF:
		case AMMO_MANA_OFFENSIVE_FULL:
			color.r = irand(50, 90);
			color.g = irand(210, 255);
			color.b = color.r;
			break;

		case AMMO_MANA_COMBO_QUARTER:
		case AMMO_MANA_COMBO_HALF:
			if (irand(0,1))
			{
				color.g = irand(50, 90);
				color.b = irand(210, 255);
				color.r = color.g;
			}
			else
			{
				color.r = irand(50, 90);
				color.g = irand(210, 255);
				color.b = color.r;
			}
			break;
	}

	/* spawn particles */
	color.a = 255;
	p = ClientParticle_new(PART_4x4_WHITE, color, 600);

	VectorSet(p->origin, crandk() * HEALTH_RADIUS, crandk() * HEALTH_RADIUS, 0.0);
	VectorSet(p->velocity, 0.0, 0.0, flrand(20.0, 40.0));
	p->acceleration[2] = 20.0;
	AddParticleToList(self, p);

	return true;
}

void FXAmmoPickup(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*ce;
	byte				tag;

	FXGetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_AMMO].formatString, &tag);

	flags &= ~CEF_OWNERS_ORIGIN;
	ce = ClientEntity_new(type, flags | CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_VIEWSTATUSCHANGED, origin, NULL, 50);

	VectorCopy(ce->r.origin, ce->origin);
	ce->r.model = NULL;
	ce->r.flags = RF_TRANSLUCENT | RF_GLOW;

	if ((tag == AMMO_MANA_COMBO_HALF) ||
		(tag == AMMO_MANA_DEFENSIVE_FULL) ||
		(tag == AMMO_MANA_OFFENSIVE_FULL))
	{
		VectorSet(ce->r.scale, 1.25, 1.25, 1.25);
	}
	else
	{
		VectorSet(ce->r.scale, 1.0, 1.0, 1.0);
	}
	ce->radius = 10.0;
	ce->alpha = 0.8;
	ce->Update = FXAmmoPickupThink;
	ce->SpawnInfo = tag;

	AddEffect(owner, ce);
}
