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
#include "../header/g_playstats.h"

void PreCacheTPortSmoke()
{
}

void FXTPortSmoke(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	int			numPuffs, i;
	client_entity_t		*TPortSmoke;
	client_particle_t	*ce;
	paletteRGBA_t color;

	TPortSmoke = ClientEntity_new(Type, Flags|CEF_NO_DRAW, Origin, NULL, 1650);

	color.r = 255;
	color.g = 255;
	color.b = 255;
	color.a = 128;

	TPortSmoke->r.color = color.c;

	TPortSmoke->radius = 10.0F;

	AddEffect(NULL, TPortSmoke);

	if (r_detail->value >= DETAIL_HIGH)
		numPuffs = irand(20,30);
	else
	if (r_detail->value == DETAIL_NORMAL)
		numPuffs = irand(10,20);
	else
		numPuffs = irand(8,13);

	fxi.S_StartSound(Origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/assassin/smoke.wav"), 1, ATTN_NORM, 0);
	for (i = 0; i < numPuffs; i++)
	{
		if (r_detail->value >= DETAIL_HIGH)
			ce = ClientParticle_new(PART_32x32_BLACKSMOKE | PFL_NEARCULL, color, 1600);
		else
		if (r_detail->value == DETAIL_NORMAL)
			ce = ClientParticle_new(PART_32x32_BLACKSMOKE | PFL_NEARCULL, color, 1500);
		else
			ce = ClientParticle_new(PART_32x32_BLACKSMOKE | PFL_NEARCULL, color, 1300);

		VectorClear(ce->origin);
		ce->velocity[0] = crandk() * 100.0F;
		ce->velocity[1] = crandk() * 100.0F;
		ce->velocity[2] = flrand(50.0F, 250.0F);

		VectorScale(ce->velocity, -1.23F, ce->acceleration);
		if (r_detail->value >= DETAIL_HIGH)
			ce->scale = flrand(60.0, 70.0);
		else
		if (r_detail->value == DETAIL_NORMAL)
			ce->scale = flrand(50.0, 60.0);
		else
			ce->scale = flrand(30.0, 45.0);
		ce->d_scale = -20.0F;
		ce->d_alpha = -77;
		AddParticleToList(TPortSmoke, ce);
	}
}

static qboolean FXAssSkinUpdaterThink(client_entity_t *assskinupdater, centity_t *owner)
{
	vec3_t sight_vec, endpos;

	if (owner->entity->skinnum != 100)
	{
		return false;
	}

	VectorCopy(owner->origin, sight_vec);
	VectorNormalize(sight_vec);
	VectorMA(owner->origin, 1024, sight_vec, endpos);
	/*
	 * TODO: rewrite, use skinname from surface? Unused in render
	 */
#if 0
	trace_t trace;

	//now look along that vector for first surface and take that texture
	trace = fxi.Trace(owner->origin, vec3_origin, vec3_origin, endpos, MASK_SHOT, CEF_CLIP_TO_WORLD);

	strcpy(string, "textures/");
	strcat(string, trace.surface->name);
	strcpy(owner->entity->skinname, string);
#endif
	return true;
}

void FXAssassinUseWallSkin(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t		*assskinupdater;

	assskinupdater = ClientEntity_new(Type, CEF_NO_DRAW, Origin, NULL, 100);

	assskinupdater->Update = FXAssSkinUpdaterThink;
	AddEffect(Owner, assskinupdater);
}
// end
