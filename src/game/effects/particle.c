//
// Particle.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include "../../common/header/common.h"
#include "particle.h"
#include "client_effects.h"
#include "../common/fx.h"
#include "../common/resourcemanager.h"
#include "client_entities.h"
#include "../common/h2rand.h"
#include "../header/g_playstats.h"

#define	MAX_PARTS_PER_CE	2048

ResourceManager_t ParticleMngr;
int ParticleUpdateTime = 0;

typedef struct {
	vec3_t	origin;
	unsigned	color;
	float	alpha;
	float	scale;
} h2particle_t;

void
InitParticleMngrMngr()
{
#define PARTICLE_BLOCK_SIZE 256

	ResMngr_Con(&ParticleMngr, sizeof(client_particle_t), PARTICLE_BLOCK_SIZE);
}

void
ReleaseParticleMngrMngr()
{
	ResMngr_Des(&ParticleMngr);
}

void
AddParticleToList(client_entity_t *ce, client_particle_t *fx)
{
	assert(ce);
	assert(fx);

	fx->next = ce->p_root;
	ce->p_root = fx;
}

void
RemoveParticleList(client_particle_t **root)
{
	client_particle_t *next;
	client_particle_t *toFree;

	assert(root);

	next = *root;

	while (next)
	{
		toFree = next;

		next = next->next;

		ResMngr_DeallocateResource(&ParticleMngr, toFree, sizeof(*toFree));
	}

	*root = NULL;
}

static vec_t
VectorSeparationSquared(vec3_t va, vec3_t vb)
{
	vec3_t		work;
	vec_t		result;

	VectorSubtract(va, vb, work);
	result = DotProduct(work, work);
	return(result);
}

int
AddParticlesToView(client_entity_t *ce)
{
	client_particle_t	*current;
	client_particle_t	**prev;
	float				d_time, d_time2;
	int					d_msec;
	int					alpha, ptype;
	qboolean			cull_parts;
	int					part_info;
	float				maxdepth2, mindepth2, depth;
	int					numparticles;
	float				yaw, pitch, radius;

	assert(ce->p_root);

	numparticles = 0;
	cull_parts = (r_detail->value == DETAIL_LOW);
	maxdepth2 = r_farclipdist->value * r_farclipdist->value;
	mindepth2 = r_nearclipdist->value * r_nearclipdist->value;

	for(prev = &ce->p_root, current = ce->p_root; current; current = current->next)
	{
		h2particle_t r;

		ptype = current->type & PFL_FLAG_MASK;

		d_msec = ParticleUpdateTime - current->startTime;
		d_time = d_msec * 0.001f;
		alpha = (int)current->color.a + Q_ftol(d_time * current->d_alpha);

		if (alpha > 255 && ((current->type & PFL_PULSE_ALPHA) || (ce->flags & CEF_PULSE_ALPHA)))
		{	// PULSE ALPHA means that once alpha is at max, reverse and head back down.
			alpha = 255 - (alpha - 255);	// A weird thing to do, but necessary because the alpha is
											// based off a dtime from the CREATION of the particle
		}

		//add to additive particle list
		if ((ce->flags & CEF_ADDITIVE_PARTS) || (current->type & PFL_ADDITIVE))
		{
			part_info = 1;
		}
		else
		{
			part_info = 2;
		}

		assert(ptype < NUM_PARTICLE_TYPES);

		r.color = current->color.c;

		if (alpha > 255 || !((ce->flags & CEF_ADDITIVE_PARTS) || (current->type & PFL_ADDITIVE)))
		{
			r.alpha = 255;
		}
		else
		{
			r.alpha = alpha;
		}

		r.scale = current->scale + (d_time * current->d_scale);

		d_time2 = d_time * d_time * 0.5;

		if (ce->flags & CEF_ABSOLUTE_PARTS)
		{
			r.origin[0] = current->origin[0] + (current->velocity[0] * d_time) + (current->acceleration[0] * d_time2);
			r.origin[1] = current->origin[1] + (current->velocity[1] * d_time) + (current->acceleration[1] * d_time2);
			r.origin[2] = current->origin[2] + (current->velocity[2] * d_time) + (current->acceleration[2] * d_time2);
		}
		else
		{
			switch(current->type & PFL_MOVE_MASK)
			{
			case PFL_MOVE_SPHERE:
				yaw = current->origin[SPH_YAW] + (current->velocity[SPH_YAW] * d_time) + (current->acceleration[SPH_YAW] * d_time2);
				pitch = current->origin[SPH_PITCH] + (current->velocity[SPH_PITCH] * d_time) + (current->acceleration[SPH_YAW] * d_time2);
				radius = current->origin[SPH_RADIUS] + (current->velocity[SPH_RADIUS] * d_time) + (current->acceleration[SPH_RADIUS] * d_time2);
				r.origin[0] = ce->r.origin[0] + cos(yaw) * cos(pitch) * radius;
				r.origin[1] = ce->r.origin[1] + sin(yaw) * cos(pitch) * radius;
				r.origin[2] = ce->r.origin[2] + sin(pitch) * radius;
				break;
			case PFL_MOVE_CYL_X:
				yaw = current->origin[CYL_YAW] + (current->velocity[CYL_YAW] * d_time) + (current->acceleration[CYL_YAW] * d_time2);
				radius = current->origin[CYL_RADIUS] + (current->velocity[CYL_RADIUS] * d_time) + (current->acceleration[CYL_RADIUS] * d_time2);
				r.origin[0] = ce->r.origin[0] + current->origin[CYL_Z] + (current->velocity[CYL_Z] * d_time) + (current->acceleration[CYL_Z] * d_time2);
				r.origin[1] = ce->r.origin[1] + cos(yaw) * radius;
				r.origin[2] = ce->r.origin[2] + sin(yaw) * radius;
				break;
			case PFL_MOVE_CYL_Y:
				yaw = current->origin[CYL_YAW] + (current->velocity[CYL_YAW] * d_time) + (current->acceleration[CYL_YAW] * d_time2);
				radius = current->origin[CYL_RADIUS] + (current->velocity[CYL_RADIUS] * d_time) + (current->acceleration[CYL_RADIUS] * d_time2);
				r.origin[0] = ce->r.origin[0] + cos(yaw) * radius;
				r.origin[1] = ce->r.origin[1] + current->origin[CYL_Z] + (current->velocity[CYL_Z] * d_time) + (current->acceleration[CYL_Z] * d_time2);
				r.origin[2] = ce->r.origin[2] + sin(yaw) * radius;
				break;
			case PFL_MOVE_CYL_Z:
				yaw = current->origin[CYL_YAW] + (current->velocity[CYL_YAW] * d_time) + (current->acceleration[CYL_YAW] * d_time2);
				radius = current->origin[CYL_RADIUS] + (current->velocity[CYL_RADIUS] * d_time) + (current->acceleration[CYL_RADIUS] * d_time2);
				r.origin[0] = ce->r.origin[0] + cos(yaw) * radius;
				r.origin[1] = ce->r.origin[1] + sin(yaw) * radius;
				r.origin[2] = ce->r.origin[2] + current->origin[CYL_Z] + (current->velocity[CYL_Z] * d_time) + (current->acceleration[CYL_Z] * d_time2);
				break;
			case PFL_MOVE_NORM:
			default:
				r.origin[0] = ce->r.origin[0] + current->origin[0] + (current->velocity[0] * d_time) + (current->acceleration[0] * d_time2);
				r.origin[1] = ce->r.origin[1] + current->origin[1] + (current->velocity[1] * d_time) + (current->acceleration[1] * d_time2);
				r.origin[2] = ce->r.origin[2] + current->origin[2] + (current->velocity[2] * d_time) + (current->acceleration[2] * d_time2);
				break;
			}
		}

		if (cull_parts || (current->type & PFL_NEARCULL))
		{
			depth = VectorSeparationSquared(r.origin, fxi.cl->refdef.vieworg);

			if ((depth > maxdepth2) || (depth < mindepth2))
			{
				part_info = 0;
			}

		}
		switch(part_info)
		{
		case 0:
			break;
		case 1:
		case 2:
			fxi.V_AddParticle(r.origin, r.color, r.alpha);
			break;
		default:
			assert(0);
			break;
		}
		numparticles++;
		prev = &(*prev)->next;
	}
	return(numparticles);
}

int
UpdateParticles(client_entity_t *ce)
{
	client_particle_t	*current;
	client_particle_t	**prev;
	float				d_time;
	int					d_msec, alpha, numparticles;

	assert(ce->p_root);
	numparticles = 0;

	for(prev = &ce->p_root, current = ce->p_root; current; current = current->next)
	{
		d_msec = ParticleUpdateTime - current->startTime;

		if (d_msec > current->duration)
		{
			*prev = current->next;

			ResMngr_DeallocateResource(&ParticleMngr, current, sizeof(*current));
			// current = current->next is still valid in the for loop.
			// a deallocated resource is guaranteed not to be changed until it is
			// reallocated, when the mananger is not shared between threads
			continue;
		}

		d_time = d_msec * 0.001f;

		alpha = (int)current->color.a + Q_ftol(d_time * current->d_alpha);

		if (alpha > 255 && ((ce->flags & CEF_PULSE_ALPHA) || (current->type & PFL_PULSE_ALPHA)))
		{	// PULSE ALPHA means that once alpha is at max, reverse and head back down.
			alpha = 255 - (alpha - 255);	// A weird thing to do, but necessary because the alpha is
											// based off a dtime from the CREATION of the particle
		}

		if (alpha <= 0)
		{
			*prev = current->next;

			ResMngr_DeallocateResource(&ParticleMngr, current, sizeof(*current));
			// current = current->next is still valid in the for loop.
			// a deallocated resource is guaranteed not to be changed until it is
			// reallocated, when the mananger is not shared between threads
			continue;
		}

		prev = &(*prev)->next;
		numparticles++;
	}
	return(numparticles);
}

// This frees all particles attached to the client entity
void
FreeParticles(client_entity_t *ce)
{
	client_particle_t	*current;
	client_particle_t	**prev;

	for(prev = &ce->p_root, current = ce->p_root; current; current = current->next)
	{
		*prev = current->next;
		ResMngr_DeallocateResource(&ParticleMngr, current, sizeof(*current));
	}
}

client_particle_t *
ClientParticle_new(ParticleTypes_t type, paletteRGBA_t color, int duration)
{
	client_particle_t	*p;

	p = (client_particle_t * )ResMngr_AllocateResource(&ParticleMngr, sizeof(client_particle_t));
	memset(p, 0, sizeof(client_particle_t));

	p->acceleration[2] = -PARTICLE_GRAVITY;

	p->startTime = ParticleUpdateTime;
	p->duration = duration;

	p->type = type;
	p->scale = 1.0F;
	p->color = color;

	p->d_alpha = -255.0 / (flrand(0.8F, 1.0F) * duration * (1.0F / 1000.0F));
	return(p);
}
