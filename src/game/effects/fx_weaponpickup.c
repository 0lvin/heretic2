//
// fx_WeaponPickup.c
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

#define BOB_HEIGHT		6.0
#define WP_PART_RADIUS	16.0

void PreCacheItemWeapons()
{
}

// --------------------------------------------------------------

static qboolean FXWeaponPickupThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*spark;
	paletteRGBA_t		color;
	int part;

	// Rotate and bob
	self->r.angles[YAW] += ANGLE_15;
	VectorCopy(owner->current.origin, self->r.origin);
	self->r.origin[2] += (cos(self->SpawnData) * BOB_HEIGHT);
	self->SpawnData += BOB_SPEED;

	switch(self->SpawnInfo)
	{
	case 0:		// Hellstaff
		part = PART_16x16_SPARK_R;
		color.c = 0xff0000ff;
		break;
	case 1:		// Magic Missile
		part = PART_16x16_SPARK_I;
		color.c = 0xff00ff00;
		break;
	case 2:		// Red rain bow
		part = PART_16x16_SPARK_R;
		color.c = 0xff0000ff;
		break;
	case 3:		// Sphere
		part = PART_16x16_SPARK_B;
		color.c = 0xffff0000;
		break;
	case 4:		// Phoenix bow
		part = irand(PART_32x32_FIRE0, PART_32x32_FIRE2);
		color.c = 0xff0080ff;
		break;
	case 5:		// Maceballs
		part = PART_16x16_SPARK_G;
		color.c = 0xff00ff00;
		break;
	case 6:		// firewall
		part = irand(PART_16x16_FIRE1, PART_16x16_FIRE3);
		color.c = 0xff0080ff;
		break;
	default:
		return true;		// No effect
	}

	if (r_detail->value != DETAIL_HIGH)
	{
		part |= PFL_SOFT_MASK;
	}
	else
	{
		color.c = 0xffffffff;
	}

	spark = ClientParticle_new(part, color, 500);
	spark->origin[0] = cos(self->SpawnData*4.0) * WP_PART_RADIUS;
	spark->origin[1] = sin(self->SpawnData*4.0) * WP_PART_RADIUS;
	spark->origin[2] = -cos(self->SpawnData) * BOB_HEIGHT;
	spark->acceleration[2] = flrand(128.0, 256.0);
	spark->scale = 6.0;
	AddParticleToList(self, spark);

	return true;
}

void FXWeaponPickup(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*ce;
	byte				tag;

	FXGetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_WEAPON].formatString, &tag);

	flags &= ~CEF_OWNERS_ORIGIN;
	ce = ClientEntity_new(type, flags | CEF_DONT_LINK | CEF_CHECK_OWNER | CEF_ADDITIVE_PARTS | CEF_VIEWSTATUSCHANGED, origin, NULL, 50);

	VectorCopy(ce->r.origin, ce->origin);
	ce->r.flags = RF_TRANSLUCENT | RF_GLOW;

	if (!tag)//sorry bob, just temporary...
	{
		ce->flags |= CEF_NO_DRAW;
	}
	else
	{
		ce->r.model = NULL;
	}

	VectorSet(ce->r.scale, 0.5, 0.5, 0.5);
	ce->radius = 10.0;
	ce->alpha = 0.8;
	ce->Update = FXWeaponPickupThink;

	if (tag == ITEM_WEAPON_FIREWALL)
	{
		VectorSet(ce->r.scale, 1.0, 1.0, 1.0);
	}

	ce->SpawnInfo = tag - 2;

	AddEffect(owner, ce);
}

// end
