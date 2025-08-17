//
// Heretic II
// Copyright 1998 Raven Software
//
// Created by JJS

#include "../../common/header/common.h"
#include "client_effects.h"
#include "client_entities.h"
#include "ce_defaultmessagehandler.h"
#include "particle.h"
#include "../common/resourcemanager.h"
#include "../common/fx.h"
#include "../common/h2rand.h"
#include "utilities.h"

#define	MIST_ALPHA	0.6F
#define MIST_FAR	512.0F
#define MIST_NEAR	96.0F

#define	NUM_MIST_MODELS	1
static struct model_s *mist_models[NUM_MIST_MODELS];
void PreCacheMist()
{
	mist_models[0] = fxi.RegisterModel("sprites/fx/mist.sp2");
}

// -----------------------------------------------------------------------------------------

// TODO: Rewrite
static float
Approach(float curr, float dest, float rate)
{
	double diff;

	diff = dest - curr;
	if (diff < 0.0)
		return curr - rate;
	else if (diff > 0.0)
		return curr + rate;
	else
		return curr;
}

static qboolean
FXMistThink(client_entity_t *mist, centity_t *owner)
{
	float	mod, scale;

	mist->flags &= ~CEF_DISAPPEARED;

	mist->Scale += flrand(-0.05, 0.05) * mist->SpawnData;
	mist->Scale = Q_min( Q_max(mist->Scale, 0.6 * mist->SpawnData), 1.4 * mist->SpawnData);
	scale = Approach(AVG_VEC3T(mist->r.scale), mist->Scale, 0.003 * mist->SpawnData);
	VectorSet(mist->r.scale, scale, scale, scale);

	mod = (AVG_VEC3T(mist->r.scale) / mist->SpawnData) * MIST_ALPHA;
	if (mist->r.depth > MIST_FAR)
	{
		mist->alpha = mod;
		return true;
	}
	if (mist->r.depth > MIST_NEAR)
	{
		mist->alpha = mod * (mist->r.depth - MIST_NEAR) * (1.0F / (MIST_FAR - MIST_NEAR));
		return true;
	}
	mist->alpha = 0.0F;
	return true;
}

void FXMist(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*mist;
	byte				scale;

	FXGetEffect(owner, flags, clientEffectSpawners[FX_MIST].formatString, &scale);
	mist = ClientEntity_new(type, flags, origin, NULL, 100);

	mist->SpawnData = scale * 0.1;

	mist->r.model = mist_models[0];
	mist->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	VectorSet(mist->r.scale, mist->SpawnData, mist->SpawnData, mist->SpawnData);

	mist->flags |= CEF_NOMOVE;
	mist->Update = FXMistThink;
	mist->radius = 1.0F;
	mist->alpha = 0.5F;

	AddEffect(NULL, mist);
}
// end
