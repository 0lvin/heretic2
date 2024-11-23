//
// Heretic II
// Copyright 1998 Raven Software
//

#include "../../common/header/common.h"
#include "client_effects.h"
#include "client_entities.h"
#include "particle.h"
#include "../common/resourcemanager.h"
#include "../common/fx.h"
#include "ce_dlight.h"
#include "../common/h2rand.h"
#include "utilities.h"

#define CROSSHAIR_THINKTIME	20

#define	NUM_MODELS	1

static struct model_s *crosshair_models[NUM_MODELS];

void PreCacheCrosshair()
{
	crosshair_models[0] = fxi.RegisterModel("sprites/fx/crosshair.sp2");
}

static qboolean FXDrawCrosshair(struct client_entity_s *cross_hair, centity_t *owner)
{
	float			alpha;
	byte			type;

	cross_hair->flags |= CEF_CULLED | CEF_DISAPPEARED;

	// Get new destination
	if (fxi.Get_Crosshair(cross_hair->r.origin, &type))
	{
		float scale;

		if (type > 2)
		{
			cross_hair->r.frame = 0;
		}
		else
		{
			cross_hair->r.frame = type;
		}

		cross_hair->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST;

		alpha = 0.5 + (Q_fabs(sin(fxi.cl->time / 800.0)) * 0.5);

		if (alpha > 1.0f)
			alpha = 1.0f;

		if (alpha < 0.0f)
			alpha = 0.0f;

		cross_hair->alpha = 0.25 + alpha * 0.5;
		scale = alpha * 0.5;
		VectorSet(cross_hair->r.scale, scale, scale, scale);

		cross_hair->flags &= ~(CEF_CULLED | CEF_DISAPPEARED | CEF_NO_DRAW);
	}
	return true;
}

/*
==============
UpdateCrosshair

==============
*/
void FXCrosshair(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*xh;

	xh = ClientEntity_new(type, flags | CEF_NO_DRAW, origin, NULL, CROSSHAIR_THINKTIME);

	xh->r.model = crosshair_models[0];
	xh->Update = FXDrawCrosshair;

	AddEffect(owner, xh);
}

// end
