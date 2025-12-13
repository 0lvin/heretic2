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
#include "../common/h2rand.h"

void
DoWaterSplash(client_entity_t *effect, paletteRGBA_t color, int count)
{
	client_particle_t	*p;
	int					i;

	if (count>500)
		count=500;
	for(i = 0; i < count; i++)
	{
		p = ClientParticle_new(PART_16x16_WATERDROP, color, 1000);

		p->d_alpha = 0;
		p->d_scale = -1.0;
		p->velocity[0] = crandk() * 20.0F;
		p->velocity[1] = crandk() * 20.0F;
		p->velocity[2] = flrand(20.0F, 30.0F);

		AddParticleToList(effect, p);
	}
}

void
WaterSplash(centity_t *owner, int type, int flags, vec3_t origin)
{
	int					cnt;
	client_entity_t		*effect;
	paletteRGBA_t		color;

	FXGetEffect(owner, flags, clientEffectSpawners[FX_SPLASH].formatString, &cnt);

	effect = ClientEntity_new(type, flags, origin, NULL, 500);
	effect->flags |= CEF_NO_DRAW | CEF_NOMOVE;

	AddEffect(NULL, effect);

	color.c = 0xffffffff;
	DoWaterSplash(effect, color, cnt);
}
