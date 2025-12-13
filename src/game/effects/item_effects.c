//
// Heretic II
// Copyright 1998 Raven Software
//
#include "../../common/header/common.h"
#include "client_effects.h"
#include "client_entities.h"
#include "../common/fx.h"
#include "particle.h"
#include "../common/h2rand.h"

void ItemRespawn(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*effect;
	client_particle_t	*p;
	paletteRGBA_t		color;
	int					i;

	effect = ClientEntity_new(type, flags, origin, NULL, 750);

	effect->flags |= CEF_NO_DRAW;

	color.c = 0xFF00FFFF;

	AddEffect(NULL, effect);	// add the effect as independent world effect

	for(i = 0; i < 32; i++)
	{
		p = ClientParticle_new(PART_4x4_WHITE, color, 750);
		p->velocity[0] = crandk() * 20.0F;
		p->velocity[1] = crandk() * 20.0F;
		p->velocity[2] = flrand(30.0F, 80.0F);
		AddParticleToList(effect, p);
	}
}
