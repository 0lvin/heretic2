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

void DoWaterSplash(client_entity_t *effect, paletteRGBA_t color, int count)
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

void WaterSplash(centity_t *owner, int type, int flags, vec3_t origin)
{
	int					cnt;
	client_entity_t		*effect;
	paletteRGBA_t		color;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPLASH].formatString, &cnt);

	effect = ClientEntity_new(type, flags, origin, NULL, 500);
	effect->flags |= CEF_NO_DRAW | CEF_NOMOVE;

	AddEffect(NULL, effect);

	color.c = 0xffffffff;
	DoWaterSplash(effect, color, cnt);
}

#define FLY_EFFECT_THINK_TIME 100 // creates new flies every 0.1 sec

static float avertexnormals[NUMVERTEXNORMALS][3] =
{
#include "../../common/models/anorms.h"
};

// This needs work. . .
qboolean CreateFlyParticles(client_entity_t *_this, centity_t *owner)
{
#define NUMVERTEXNORMALS	162
#define	BEAMLENGTH			16
//	int		n;
	int		count;
//	int		starttime;

//	int fly_stoptime;

	int			i;
	client_particle_t *p;
	float		angle;
	float		sp, sy, cp, cy;
	vec3_t		forward;
	float		dist = 64;
	float		ltime;
	static vec3_t avelocities [NUMVERTEXNORMALS];

	count = NUMVERTEXNORMALS;

	if (!avelocities[0][0])
	{
		for (i=0 ; i<NUMVERTEXNORMALS*3 ; i++)
			avelocities[0][i] = flrand(0.0, 2.55);
	}

	ltime = (float)fxi.cl->time / 1000.0;
	for (i=0 ; i<count ; i+=2)
	{
		p = (client_particle_t *)ResMngr_AllocateResource(&ParticleMngr, sizeof(*_this->p_root));

		AddParticleToList(_this, p);

		angle = ltime * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = ltime * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = ltime * avelocities[i][2];

		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		p->startTime = fxi.cl->time;
		p->duration = 50;

		dist = sin(ltime + i)*64;
		p->origin[0] = avertexnormals[i][0]*dist + forward[0]*BEAMLENGTH;
		p->origin[1] = avertexnormals[i][1]*dist + forward[1]*BEAMLENGTH;
		p->origin[2] = avertexnormals[i][2]*dist + forward[2]*BEAMLENGTH;

		VectorClear (p->velocity);
		VectorClear (p->acceleration);

		p->color.c = 0x000000FF;

		p->d_alpha = -100;

		p->scale = 1.0;
		p->d_scale = 0.0;
	}

	return true; // never goes away. . .
}

void FlyEffect(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t *effect;

	effect = ClientEntity_new(type, flags, origin, NULL, FLY_EFFECT_THINK_TIME);

	effect->flags |= CEF_NO_DRAW;

	effect->Update = CreateFlyParticles;

	AddEffect(owner, effect);
}
