//
// fx_pickup.c
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

#define	NUM_PICKUP_MODELS	1
static struct model_s *pickup_models[NUM_PICKUP_MODELS];

void PreCachePickup()
{
	pickup_models[0] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

// --------------------------------------------------------------

void FXPickup(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*ce;

	ce = ClientEntity_new(type, flags | CEF_DONT_LINK | CEF_PULSE_ALPHA, origin, NULL, 500);

	ce->r.model = pickup_models[0];
	ce->r.frame = 1;
	ce->r.flags = RF_TRANSLUCENT | RF_FLARE | RF_TRANS_ADD_ALPHA;
	VectorSet(ce->r.scale, 0.4, 0.4, 0.4);
	ce->alpha = .75;
	ce->d_scale = 1.0;
	ce->d_alpha = 2.5;
	ce->color.c = 0xc0ffffff;
	ce->radius = 10.0;
	ce->alpha = 0.8;
	ce->r.origin[2] += 8.0;

	AddEffect(NULL, ce);
}

// end
