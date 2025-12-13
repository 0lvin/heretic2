//
// fx_DefensePickup.c
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
#include "ce_dlight.h"
#include "../header/g_items.h"

#define BOB_HEIGHT				6.0
#define NUM_DEF_PICKUP_SPARKS	4
#define EGG_TRAIL_DELAY			100
#define EGG_RADIUS				10.0
#define	NUM_ITEMDEFENSE			6

static struct model_s *defense_models[NUM_ITEMDEFENSE];
void PreCacheItemDefense()
{
	defense_models[0] = fxi.RegisterModel("sprites/spells/spark_cyan.sp2");		// ITEM_DEFENSE_REPULSION + cyan spark
	defense_models[1] = fxi.RegisterModel("sprites/spells/meteorbarrier.sp2");	// ITEM_DEFENSE_METEORBARRIER + Meteor cloud
	defense_models[2] = fxi.RegisterModel("sprites/spells/spark_green.sp2");	// ITEM_DEFENSE_POLYMORPH + green spark
	defense_models[3] = fxi.RegisterModel("sprites/spells/spark_red.sp2");		// ITEM_DEFENSE_TELEPORT + red spark
	defense_models[4] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");	// ITEM_DEFENSE_SHIELD + blue spark
	defense_models[5] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");	// ITEM_DEFENSE_TORNADO + blue spark
}

static qboolean FXEggSparkThink(struct client_entity_s *shield, centity_t *owner)
{
	vec3_t angvect;
	vec3_t	origin = {0,0,0};

	VectorCopy(shield->origin, origin);
	origin[2] = shield->origin[2] + (cos(shield->d_scale2) * BOB_HEIGHT);
	shield->d_scale2 += BOB_SPEED;

	// Update the angle of the spark.
	VectorMA(shield->direction, (float)(fxi.cl->time-shield->lastThinkTime) / 1000.0,
		shield->velocity2, shield->direction);

	// Update the position of the spark.
	AngleVectors(shield->direction, angvect, NULL, NULL);
	VectorMA(origin, shield->radius, angvect, shield->r.origin);

	shield->lastThinkTime = fxi.cl->time;
	return true;
}

// Create effect FX_PICKUP_DEFENSE
void FXDefensePickup(centity_t *owner, int type, int flags, vec3_t origin)
{
	byte tag;
	int i;

	FXGetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_DEFENSE].formatString, &tag);

	if (!owner || tag >= NUM_ITEMDEFENSE)
	{
		return;
	}

	flags &= ~CEF_OWNERS_ORIGIN;

	// if we are looking at the polymorph egg, put a special effect around it
	// stolen wholesale from Pats Shield Effect. Cheers Pat.
	// Add spinning electrical sparks
	for(i = 0; i < NUM_DEF_PICKUP_SPARKS; i++)
	{
		client_entity_t	*shield;

		shield = ClientEntity_new(type, flags, origin, 0, 50);
		shield->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS | CEF_VIEWSTATUSCHANGED;
		shield->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		shield->r.model = defense_models[tag];
		shield->radius = EGG_RADIUS;
		shield->color.c = 0xffffffff;
		shield->alpha = 0.1;
		shield->d_alpha = 0.5;
		if (tag == 1)
		{
			VectorSet(shield->r.scale, 0.2, 0.2, 0.2);
		}
		else
		{
			VectorSet(shield->r.scale, 0.8, 0.8, 0.8);
		}

		shield->SpawnData = tag;

		shield->Update = FXEggSparkThink;
		VectorCopy(shield->r.origin, shield->origin);

		VectorClear(shield->direction);
		shield->direction[YAW] = flrand(0, 360.0);		// This angle is kept at a constant distance from org.
		shield->direction[PITCH] = flrand(0, 360.0);

		shield->velocity2[YAW] = crandk() * 180.0;
		if (shield->velocity2[YAW] < 0)			// Assure that the sparks are moving around at a pretty good clip.
			shield->velocity2[YAW] -= 180.0;
		else
			shield->velocity2[YAW] += 180.0;

		shield->velocity2[PITCH] = crandk() * 180.0;	// This is a velocity around the sphere.
		if (shield->velocity2[PITCH] < 0)		// Assure that the sparks are moving around at a pretty good clip.
			shield->velocity2[PITCH] -= 180.0;
		else
			shield->velocity2[PITCH] += 180.0;

		shield->SpawnDelay = fxi.cl->time + EGG_TRAIL_DELAY;
		shield->lastThinkTime = fxi.cl->time;

		AddEffect(owner, shield);
	}
}

// end
