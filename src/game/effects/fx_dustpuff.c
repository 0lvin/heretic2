//
// Heretic II
// Copyright 1998 Raven Software
//
// Created by JDW

#include "../../common/header/common.h"
#include "client_effects.h"
#include "client_entities.h"
#include "ce_defaultmessagehandler.h"
#include "particle.h"
#include "../common/resourcemanager.h"
#include "../common/fx.h"
#include "../common/h2rand.h"
#include "utilities.h"

void FXDustPuff(client_entity_t *owner, float scale)
{
	client_particle_t	*puff;
	paletteRGBA_t		color;

	color.c = 0x80c0c0c0;
	puff = ClientParticle_new(PART_32x32_STEAM, color, 500);

	VectorSet(puff->velocity, crandk() * 50.0, crandk() * 50.0, flrand(0.0F, 25.0F));
	VectorScale(puff->velocity, -1.23F, puff->acceleration);
	puff->scale = scale;
	puff->d_scale = 10.0;
	puff->d_alpha *= 0.5;

	AddParticleToList(owner, puff);
}

void CreateSinglePuff(vec3_t origin, float scale)
{
	client_entity_t	*ce;

	ce = ClientEntity_new(-1, CEF_NOMOVE | CEF_NO_DRAW, origin, NULL, 500);
	FXDustPuff(ce, scale);
	AddEffect(NULL, ce);
}
void FXDustPuffOnGround(centity_t *owner, int type, int flags, vec3_t origin)
{
	int				numPuffs, i;
	vec3_t			endpos;
	trace_t			trace;
	client_entity_t	*ce;

	VectorCopy(origin, endpos);
	endpos[2] -= 128.0;

	//Find out where the ground is
	trace = fxi.Trace(origin,
				vec3_origin,
				vec3_origin,
				endpos,
				CONTENTS_SOLID,
				CEF_CLIP_TO_WORLD);

	if (trace.fraction == 1.0)
		return;

	ce = ClientEntity_new(-1, CEF_NOMOVE | CEF_NO_DRAW, trace.endpos, NULL, 500);
	numPuffs = irand(3,4);
	for (i = 0; i < numPuffs; i++)
	{
		FXDustPuff(ce, 5.0F);
	}
	AddEffect(NULL, ce);
}
