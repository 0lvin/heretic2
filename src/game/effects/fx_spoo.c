//
// fx_flyingfist.c
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

#define	NUM_SPOO_MODELS	2
static struct model_s *spoo_models[NUM_SPOO_MODELS];
void
PreCacheSpoo()
{
	spoo_models[0] = fxi.RegisterModel("sprites/fx/spoo.sp2");
	spoo_models[1] = fxi.RegisterModel("sprites/fx/spoo2.sp2");
}

// --------------------------------------------------------------

// ************************************************************************************************
// FXSpooTrailThink
// ************************************************************************************************

static qboolean
FXSpooTrailThink(struct client_entity_s *self, centity_t *owner)
{
	client_entity_t	*TrailEnt;
	//vec3_t			org, dir;
	//float			len;
	int				count;

	count = GetScaledCount(8, 0.85);

	while (count--)
	{
		//VectorAdd(org, dir, org);

		TrailEnt=ClientEntity_new(FX_SPOO,
								  self->flags & ~(CEF_OWNERS_ORIGIN|CEF_NO_DRAW),
								  owner->origin,
								  NULL,
								  1000);

		TrailEnt->r.model = spoo_models[irand(0, 1)];

		TrailEnt->r.origin[0] += crandk() * 3.0;
		TrailEnt->r.origin[1] += crandk() * 3.0;
		TrailEnt->r.origin[2] += crandk() * 3.0;

		VectorSet(TrailEnt->velocity, crandk() * 64.0, crandk() * 64.0, -64.0F);

		VectorSet(TrailEnt->r.scale, 0.65, 0.65, 0.65);
		TrailEnt->alpha = 1.0f;
		TrailEnt->r.flags |= RF_TRANSLUCENT | RF_FULLBRIGHT;
		TrailEnt->d_scale = flrand(-4.0, -3.5);
		TrailEnt->d_alpha = -2.0f;
		TrailEnt->color.c = 0xFFFFFFFF;
		TrailEnt->radius=20.0;

		AddEffect(NULL,TrailEnt);
	}

	VectorCopy(owner->current.origin, self->startpos);

	return true;
}

void
FXSpoo(centity_t *owner,int type,int Flags,vec3_t origin)
{
	client_entity_t	*Trail;

	Trail=ClientEntity_new(type,Flags,origin,NULL,20);

	Trail->Update=FXSpooTrailThink;
	Trail->flags|=CEF_NO_DRAW;
	VectorCopy(origin, Trail->startpos);

	AddEffect(owner,Trail);

	FXSpooTrailThink(Trail,owner);
}

void
FXSpooSplat(centity_t *owner,int type,int Flags,vec3_t origin)
{
	client_entity_t	*TrailEnt;
	vec3_t			dir;
	int				count;

	FXGetEffect(owner, Flags, clientEffectSpawners[FX_SPOO_SPLAT].formatString, &dir);

	count = GetScaledCount(16, 0.85);

	while (count--)
	{
		float scale;

		TrailEnt=ClientEntity_new(FX_SPOO,
								  0,
								  origin,
								  NULL,
								  1000);

		TrailEnt->r.model = spoo_models[irand(0, 1)];

		VectorRandomCopy(dir, TrailEnt->velocity, 16.0f);
		VectorNormalize(TrailEnt->velocity);
		VectorScale(TrailEnt->velocity, flrand(100.0f, 200.0f), TrailEnt->velocity);

		VectorSet(TrailEnt->acceleration, 0, 0, -128);

		scale = flrand(0.75, 1.0);
		VectorSet(TrailEnt->r.scale, scale, scale, scale);
		TrailEnt->alpha=1.0;

		TrailEnt->r.flags |= RF_TRANSLUCENT;

		TrailEnt->r.frame=0;
		TrailEnt->d_scale=flrand( -1.25, -1.0);
		TrailEnt->d_alpha=flrand(-1, -0.5);
		TrailEnt->color.c = 0xA0FFFFFF;
		TrailEnt->radius=20.0;

		AddEffect(NULL,TrailEnt);
	}
}
