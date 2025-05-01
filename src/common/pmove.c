/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * Player movement code. This is the core of Quake IIs legendary physics
 * engine
 *
 * =======================================================================
 */

#include "header/common.h"
#include "../client/sound/header/local.h"
#include "../client/header/client.h"

#define STEPSIZE 18

#include "../game/header/g_physics.h"

#if !defined(DEDICATED_ONLY) && defined(USE_OPENAL)
void AL_Underwater();
void AL_Overwater();
#endif

#define OVERCLIP 1.001f
/* TODO: Rewrite: Gravity is hardcoded? */
#define GRAVITY 675.0f

/* all of the locals will be zeroed before each
 * pmove, just to make damn sure we don't have
 * any differences when running on client or server */

typedef struct
{
	vec3_t origin; /* full float precision */
	vec3_t velocity; /* full float precision */

	vec3_t forward, right, up;
	float frametime;

	csurface_t *groundsurface;
	cplane_t groundplane;
	int groundcontents;

	int previous_origin[3];
	int current_origin[3];
	qboolean ladder;

	qboolean walking;
	qboolean groundPlane;
	trace_t groundTrace;
} pml_t;

static pmove_t *pm;
static pml_t pml;

/* movement parameters */
static float pm_stopspeed = 100;
static float pm_maxspeed = 300;
static float pm_duckspeed = 100;
static float pm_accelerate = 10;
float pm_airaccelerate = 0;
static float pm_wateraccelerate = 10;
static float pm_friction = 6;
static float pm_waterfriction = 1;
static float pm_waterspeed = 400;

#define STOP_EPSILON 0.1 /* Slide off of the impacting object returns the blocked flags (1 = floor, 2 = step / wall) */
#define MIN_STEP_NORMAL 0.7 /* can't step up onto very steep slopes */
#define MAX_CLIP_PLANES 5

static void
PM_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float backoff;
	float change;
	int i;

	backoff = DotProduct(in, normal) * overbounce;

	for (i = 0; i < 3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;

		if ((out[i] > -STOP_EPSILON) && (out[i] < STOP_EPSILON))
		{
			out[i] = 0;
		}
	}
}

/*
 * Each intersection will try to step over the obstruction instead of
 * sliding along it.
 *
 * Returns a new origin, velocity, and contact entity
 * Does not modify any world state?
 */
static void
PM_StepSlideMove_(void)
{
	int bumpcount, numbumps;
	int numplanes;
	vec3_t planes[MAX_CLIP_PLANES];
	vec3_t primal_velocity;
	float time_left;

	numbumps = 4;

	VectorCopy(pml.velocity, primal_velocity);
	numplanes = 0;

	time_left = pml.frametime;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		vec3_t end, dir;
		trace_t trace;
		int i, j;
		float d;

		for (i = 0; i < 3; i++)
		{
			end[i] = pml.origin[i] + time_left * pml.velocity[i];
		}

		trace = pm->trace(pml.origin, pm->mins, pm->maxs, end);

		if (trace.allsolid)
		{
			/* entity is trapped in another solid */
			pml.velocity[2] = 0; /* don't build up falling damage */
			return;
		}

		if (trace.fraction > 0)
		{
			/* actually covered some distance */
			VectorCopy(trace.endpos, pml.origin);
			numplanes = 0;
		}

		if (trace.fraction == 1)
		{
			break; /* moved the entire distance */
		}

		/* save entity for contact */
		if ((pm->numtouch < MAXTOUCH) && trace.ent)
		{
			pm->touchents[pm->numtouch] = trace.ent;
			pm->numtouch++;
		}

		time_left -= time_left * trace.fraction;

		/* slide along this plane */
		if (numplanes >= MAX_CLIP_PLANES)
		{
			/* this shouldn't really happen */
			VectorCopy(vec3_origin, pml.velocity);
			break;
		}

		VectorCopy(trace.plane.normal, planes[numplanes]);
		numplanes++;

		/* modify original_velocity so it parallels all of the clip planes */
		for (i = 0; i < numplanes; i++)
		{
			PM_ClipVelocity(pml.velocity, planes[i], pml.velocity, 1.01f);

			for (j = 0; j < numplanes; j++)
			{
				if (j != i)
				{
					if (DotProduct(pml.velocity, planes[j]) < 0)
					{
						break; /* not ok */
					}
				}
			}

			if (j == numplanes)
			{
				break;
			}
		}

		if (i != numplanes)
		{
			/* go along this plane */
		}
		else
		{
			/* go along the crease */
			if (numplanes != 2)
			{
				VectorCopy(vec3_origin, pml.velocity);
				break;
			}

			CrossProduct(planes[0], planes[1], dir);
			d = DotProduct(dir, pml.velocity);
			VectorScale(dir, d, pml.velocity);
		}

		/* if velocity is against the original velocity, stop dead
		   to avoid tiny occilations in sloping corners */
		if (DotProduct(pml.velocity, primal_velocity) <= 0)
		{
			VectorCopy(vec3_origin, pml.velocity);
			break;
		}
	}

	if (pm->s.pm_time)
	{
		VectorCopy(primal_velocity, pml.velocity);
	}
}

/*
==================
PM_SlideMove
Returns qtrue if the velocity was clipped in some way
==================
*/
#define	MAX_CLIP_PLANES	5
static qboolean
PM_SlideMove(qboolean gravity)
{
	int bumpcount, numbumps;
	vec3_t dir;
	float d;
	int numplanes;
	vec3_t planes[MAX_CLIP_PLANES];
	vec3_t clipVelocity;
	int i, j, k;
	trace_t trace;
	vec3_t end;
	float time_left;
	float into;
	vec3_t endVelocity;
	vec3_t endClipVelocity;

	numbumps = 4;

	if (gravity)
	{
		VectorCopy(pml.velocity, endVelocity);
		endVelocity[2] -= (GRAVITY * 50) * pml.frametime;
		pml.velocity[2] = (pml.velocity[2] + endVelocity[2]) * 0.5;
		if (pml.groundPlane)
		{
			// slide along the ground plane
			PM_ClipVelocity(pml.velocity, pml.groundTrace.plane.normal,
				pml.velocity, OVERCLIP);
		}
	}

	time_left = pml.frametime;

	/* never turn against the ground plane */
	if (pml.groundPlane)
	{
		numplanes = 1;
		VectorCopy(pml.groundTrace.plane.normal, planes[0]);
	}
	else
	{
		numplanes = 0;
	}

	/* never turn against original velocity */
	VectorNormalize2(pml.velocity, planes[numplanes]);
	numplanes++;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		/* calculate position we are trying to move to */
		VectorMA(pml.origin, time_left, pml.velocity, end);

		/* see if we can make it there */
		trace = pm->trace(pml.origin, pm->mins, pm->maxs, end);

		if (trace.allsolid)
		{
			/* entity is completely trapped in another solid */
			pml.velocity[2] = 0;	// don't build up falling damage, but allow sideways acceleration
			return true;
		}

		if (trace.fraction > 0)
		{
			/* actually covered some distance */
			VectorCopy(trace.endpos, pml.origin);
		}

		if (trace.fraction == 1)
		{
			break;		// moved the entire distance
		}

		time_left -= time_left * trace.fraction;

		if (numplanes >= MAX_CLIP_PLANES)
		{
			// this shouldn't really happen
			VectorClear(pml.velocity);
			return true;
		}

		//
		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		//
		for (i = 0; i < numplanes; i++)
		{
			if (DotProduct(trace.plane.normal, planes[i]) > 0.99)
			{
				VectorAdd(trace.plane.normal, pml.velocity, pml.velocity);
				break;
			}
		}
		if (i < numplanes)
		{
			continue;
		}
		VectorCopy(trace.plane.normal, planes[numplanes]);
		numplanes++;

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		for (i = 0; i < numplanes; i++)
		{
			into = DotProduct(pml.velocity, planes[i]);
			if (into >= 0.1)
			{
				continue;		// move doesn't interact with the plane
			}

			// slide along the plane
			PM_ClipVelocity(pml.velocity, planes[i], clipVelocity, OVERCLIP);

			// slide along the plane
			PM_ClipVelocity(endVelocity, planes[i], endClipVelocity, OVERCLIP);

			// see if there is a second plane that the new move enters
			for (j = 0; j < numplanes; j++)
			{
				if (j == i)
				{
					continue;
				}

				if (DotProduct(clipVelocity, planes[j]) >= 0.1)
				{
					continue;		// move doesn't interact with the plane
				}

				// try clipping the move to the plane
				PM_ClipVelocity(clipVelocity, planes[j], clipVelocity, OVERCLIP);
				PM_ClipVelocity(endClipVelocity, planes[j], endClipVelocity, OVERCLIP);

				// see if it goes back into the first clip plane
				if (DotProduct(clipVelocity, planes[i]) >= 0)
				{
					continue;
				}

				// slide the original velocity along the crease
				CrossProduct(planes[i], planes[j], dir);
				VectorNormalize(dir);
				d = DotProduct(dir, pml.velocity);
				VectorScale(dir, d, clipVelocity);

				CrossProduct(planes[i], planes[j], dir);
				VectorNormalize(dir);
				d = DotProduct(dir, endVelocity);
				VectorScale(dir, d, endClipVelocity);

				// see if there is a third plane the the new move enters
				for (k = 0; k < numplanes; k++)
				{
					if (k == i || k == j)
					{
						continue;
					}

					if (DotProduct(clipVelocity, planes[k]) >= 0.1)
					{
						continue;		// move doesn't interact with the plane
					}

					// stop dead at a tripple plane interaction
					VectorClear(pml.velocity);
					return true;
				}
			}

			// if we have fixed all interactions, try another move
			VectorCopy(clipVelocity, pml.velocity);
			VectorCopy(endClipVelocity, endVelocity);
			break;
		}
	}

	if (gravity)
	{
		VectorCopy(endVelocity, pml.velocity);
	}

	return (bumpcount != 0);
}

static void
PM_StepSlideMove(qboolean gravity)
{
	vec3_t start_o, start_v;
	trace_t trace;
	vec3_t up, down;
	float stepSize;

	VectorCopy(pml.origin, start_o);
	VectorCopy(pml.velocity, start_v);

	if (PM_SlideMove(gravity) == 0)
	{
		return;		// we got exactly where we wanted to go first try
	}

	VectorCopy(start_o, down);
	down[2] -= STEPSIZE;
	trace = pm->trace(start_o, pm->mins, pm->maxs, down);
	VectorSet(up, 0, 0, 1);
	// never step up when you still have up velocity
	if (pml.velocity[2] > 0 && (trace.fraction == 1.0 ||
		DotProduct(trace.plane.normal, up) < 0.7))
	{
		return;
	}

	VectorCopy(start_o, up);
	up[2] += STEPSIZE;

	// test the player position if they were a stepheight higher
	trace = pm->trace(start_o, pm->mins, pm->maxs, up);
	if (trace.allsolid)
	{
		return; /* can't step up */
	}

	stepSize = trace.endpos[2] - start_o[2];
	// try slidemove from this position
	VectorCopy(trace.endpos, pml.origin);
	VectorCopy(start_v, pml.velocity);

	PM_SlideMove(gravity);

	/* push down the final amount */
	VectorCopy(pml.origin, down);
	down[2] -= stepSize;
	trace = pm->trace(pml.origin, pm->mins, pm->maxs, down);

	if (!trace.allsolid)
	{
		VectorCopy(trace.endpos, pml.origin);
	}
	if (trace.fraction < 1.0)
	{
		PM_ClipVelocity(pml.velocity, trace.plane.normal, pml.velocity, OVERCLIP);
	}
}

/*
 * Handles both ground friction and water friction
 */
static void
PM_Friction(void)
{
	float *vel;
	float speed, newspeed, control;
	float drop;

	vel = pml.velocity;

	speed = VectorLength(vel);

	if (speed < 1)
	{
		vel[0] = 0;
		vel[1] = 0;
		return;
	}

	drop = 0;

	/* apply ground friction */
	if (pm->waterlevel <= 1)
	{
		if (pml.walking)
		{
			/* if getting knocked back, no friction */
			control = speed < pm_stopspeed ? pm_stopspeed : speed;
			drop += control * pm_friction * pml.frametime;
		}
	}

	/* apply water friction */
	if (pm->waterlevel && !pml.ladder)
	{
		drop += speed * pm_waterfriction * pm->waterlevel * pml.frametime;
	}

	/* scale the velocity */
	newspeed = speed - drop;

	if (newspeed < 0)
	{
		newspeed = 0;
	}

	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}

/* Used for speedoomter display. */
void GetPlayerSpeed(float *speed, float *speedxy)
{
	*speedxy = sqrt(pml.velocity[0] * pml.velocity[0] + pml.velocity[1] * pml.velocity[1]);
	*speed = VectorLength(pml.velocity);
}

/*
 * Handles user intended acceleration
 */
static void
PM_Accelerate(vec3_t wishdir, float wishspeed, float accel)
{
	int i;
	float addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct(pml.velocity, wishdir);
	currentspeed *= 8;
	addspeed = wishspeed - currentspeed;

	if (addspeed <= 0)
	{
		return;
	}

	accelspeed = accel * pml.frametime * wishspeed;

	if (accelspeed > addspeed)
	{
		accelspeed = addspeed;
	}

	for (i = 0; i < 3; i++)
	{
		pml.velocity[i] += accelspeed * wishdir[i];
	}
}

static void
PM_AirAccelerate(vec3_t wishdir, float wishspeed, float accel)
{
	int i;
	float addspeed, accelspeed, currentspeed, wishspd = wishspeed;

	if (wishspd > 30)
	{
		wishspd = 30;
	}

	currentspeed = DotProduct(pml.velocity, wishdir);
	addspeed = wishspd - currentspeed;

	if (addspeed <= 0)
	{
		return;
	}

	accelspeed = accel * wishspeed * pml.frametime;

	if (accelspeed > addspeed)
	{
		accelspeed = addspeed;
	}

	for (i = 0; i < 3; i++)
	{
		pml.velocity[i] += accelspeed * wishdir[i];
	}
}

static void
PM_AddCurrents(vec3_t wishvel)
{
	/* account for ladders */
	if (pml.ladder && (fabs(pml.velocity[2]) <= 200))
	{
		if ((pm->viewangles[PITCH] <= -15) && (pm->cmd.forwardmove > 0))
		{
			wishvel[2] = 200;
		}
		else if ((pm->viewangles[PITCH] >= 15) && (pm->cmd.forwardmove > 0))
		{
			wishvel[2] = -200;
		}
		else if (pm->cmd.upmove > 0)
		{
			wishvel[2] = 200;
		}
		else if (pm->cmd.upmove < 0)
		{
			wishvel[2] = -200;
		}
		else
		{
			wishvel[2] = 0;
		}

		/* limit horizontal speed when on a ladder */
		if (wishvel[0] < -25)
		{
			wishvel[0] = -25;
		}
		else if (wishvel[0] > 25)
		{
			wishvel[0] = 25;
		}

		if (wishvel[1] < -25)
		{
			wishvel[1] = -25;
		}
		else if (wishvel[1] > 25)
		{
			wishvel[1] = 25;
		}
	}

	/* add water currents  */
	if (pm->watertype & MASK_CURRENT)
	{
		vec3_t v;
		float s;

		VectorClear(v);

		if (pm->watertype & CONTENTS_CURRENT_0)
		{
			v[0] += 1;
		}

		if (pm->watertype & CONTENTS_CURRENT_90)
		{
			v[1] += 1;
		}

		if (pm->watertype & CONTENTS_CURRENT_180)
		{
			v[0] -= 1;
		}

		if (pm->watertype & CONTENTS_CURRENT_270)
		{
			v[1] -= 1;
		}

		if (pm->watertype & CONTENTS_CURRENT_UP)
		{
			v[2] += 1;
		}

		if (pm->watertype & CONTENTS_CURRENT_DOWN)
		{
			v[2] -= 1;
		}

		s = pm_waterspeed;

		if ((pm->waterlevel == 1) && (pm->groundentity))
		{
			s /= 2;
		}

		VectorMA(wishvel, s, v, wishvel);
	}

	/* add conveyor belt velocities */
	if (pm->groundentity)
	{
		vec3_t v;

		VectorClear(v);

		if (pml.groundcontents & CONTENTS_CURRENT_0)
		{
			v[0] += 1;
		}

		if (pml.groundcontents & CONTENTS_CURRENT_90)
		{
			v[1] += 1;
		}

		if (pml.groundcontents & CONTENTS_CURRENT_180)
		{
			v[0] -= 1;
		}

		if (pml.groundcontents & CONTENTS_CURRENT_270)
		{
			v[1] -= 1;
		}

		if (pml.groundcontents & CONTENTS_CURRENT_UP)
		{
			v[2] += 1;
		}

		if (pml.groundcontents & CONTENTS_CURRENT_DOWN)
		{
			v[2] -= 1;
		}

		VectorMA(wishvel, 100, v, wishvel);
	}
}

// TODO: Rewrite
static void
PM_BoundVelocity(vec3_t vel, vec3_t norm, qboolean runshrine, qboolean high_max)
{
	vec_t v;

	if (high_max || runshrine)
	{
		v = 600.0;
	}
	else
	{
		v = 300.0;
	}

	if (v < VectorNormalize2(vel, norm))
	{
		vel[0] = norm[0] * v;
		vel[1] = norm[1] * v;
		vel[2] = norm[2] * v;
	}
}

// TODO: Rewrite
static void
PM_SetVelInLiquid(float a1)
{
	float forwardmove;
	qboolean runshrine;
	float v15;
	float v16;
	vec3_t norm;
	vec3_t vel;

	PM_Friction();
	forwardmove = pm->cmd.forwardmove;
	v15 = forwardmove;
	runshrine = false;
	v16 = (float)pm->cmd.sidemove;
	if (pm->run_shrine && forwardmove > 0.0)
	{
		runshrine = true;
		v15 = forwardmove * 1.65;
	}
	vel[0] = pml.forward[0] * v15 + pml.right[0] * v16;
	vel[1] = pml.forward[1] * v15 + pml.right[1] * v16;
	vel[2] = v15 * pml.forward[2] + v16 * pml.right[2];
	PM_AddCurrents(vel);
	PM_BoundVelocity(vel, norm, runshrine, 0);
	PM_Accelerate(vel, pm_airaccelerate, 10.0);
	if (pm->groundentity)
	{
		pml.velocity[0] = vel[0];
		pml.velocity[1] = vel[1];
		pml.velocity[2] = vel[2];
	}
}

static void
PM_WaterMove(void)
{
	PM_SetVelInLiquid(0.5);
	PM_StepSlideMove(false);
}

static void
PM_AirMove(void)
{
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	float maxspeed;

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.sidemove;

	PM_Friction();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;
	VectorNormalize(pml.forward);
	VectorNormalize(pml.right);

	for (i = 0; i < 2; i++)
	{
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	}

	wishvel[2] = 0;

	VectorCopy(wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	/* clamp to server defined max speed */
	maxspeed = (pm->s.pm_flags & PMF_DUCKED) ? pm_duckspeed : pm_maxspeed;

	if (wishspeed > maxspeed)
	{
		VectorScale(wishvel, maxspeed / wishspeed, wishvel);
		wishspeed = maxspeed;
	}

	// not on ground, so little effect on velocity
	PM_Accelerate(wishdir, wishspeed, pm_accelerate);

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if (pml.groundPlane)
	{
		PM_ClipVelocity(pml.velocity, pml.groundTrace.plane.normal,
			pml.velocity, OVERCLIP);
	}

	PM_StepSlideMove(true);
}

static void
PM_CatagorizePosition(void)
{
	vec3_t point;
	int cont;
	trace_t trace;
	float sample1;
	float sample2;

	/* if the player hull point one unit down
	   is solid, the player is on ground */

	/* see if standing on something solid */
	point[0] = pml.origin[0];
	point[1] = pml.origin[1];
	point[2] = pml.origin[2] - 0.25f;

	if (pml.velocity[2] > 180)
	{
		pm->s.pm_flags &= ~PMF_ON_GROUND;
		pm->groundentity = NULL;
	}
	else
	{
		trace = pm->trace(pml.origin, pm->mins, pm->maxs, point);
		pml.groundplane = trace.plane;
		pml.groundsurface = trace.surface;
		pml.groundcontents = trace.contents;

		if (!trace.ent || ((trace.plane.normal[2] < 0.7) && !trace.startsolid))
		{
			pm->groundentity = NULL;
			pm->s.pm_flags &= ~PMF_ON_GROUND;
		}
		else
		{
			pm->groundentity = trace.ent;

			/* hitting solid ground will end a waterjump */
			if (pm->s.pm_flags & PMF_TIME_WATERJUMP)
			{
				pm->s.pm_flags &=
					~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
				pm->s.pm_time = 0;
			}

			if (!(pm->s.pm_flags & PMF_ON_GROUND))
			{
				/* just hit the ground */
				pm->s.pm_flags |= PMF_ON_GROUND;

				/* don't do landing time if we were just going down a slope */
				if (pml.velocity[2] < -200)
				{
					pm->s.pm_flags |= PMF_TIME_LAND;

					/* don't allow another jump for a little while */
					if (pml.velocity[2] < -400)
					{
						pm->s.pm_time = 25;
					}
					else
					{
						pm->s.pm_time = 18;
					}
				}
			}
		}

		if ((pm->numtouch < MAXTOUCH) && trace.ent)
		{
			pm->touchents[pm->numtouch] = trace.ent;
			pm->numtouch++;
		}
	}

	/* get waterlevel, accounting for ducking */
	pm->waterlevel = 0;
	pm->watertype = 0;

	sample2 = pm->viewheight - pm->mins[2];
	sample1 = sample2 / 2;

	point[2] = pml.origin[2] + pm->mins[2] + 1;
	cont = pm->pointcontents(point);

	if (cont & MASK_WATER)
	{
		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pml.origin[2] + pm->mins[2] + sample1;
		cont = pm->pointcontents(point);

		if (cont & MASK_WATER)
		{
			pm->waterlevel = 2;
			point[2] = pml.origin[2] + pm->mins[2] + sample2;
			cont = pm->pointcontents(point);

			if (cont & MASK_WATER)
			{
				pm->waterlevel = 3;
			}
		}
	}
}

static void
PM_CheckJump(void)
{
	if (pm->s.pm_flags & PMF_TIME_LAND)
	{
		/* hasn't been long enough since landing to jump again */
		return;
	}

	if (pm->cmd.upmove < 10)
	{
		/* not holding jump */
		pm->s.pm_flags &= ~PMF_JUMP_HELD;
		return;
	}

	/* must wait for jump to be released */
	if (pm->s.pm_flags & PMF_JUMP_HELD)
	{
		return;
	}

	if (pm->s.pm_type == PM_DEAD)
	{
		return;
	}

	if ((pm->s.pm_flags & 8) == 0 && pm->cmd.upmove > 9)
	{
		pm->groundentity = NULL;
		pml.velocity[2] = 200.0;
		pml.walking = false;
	}
}

static void
PM_CheckSpecialMovement(void)
{
	vec3_t spot;
	int cont;
	vec3_t flatforward;
	trace_t trace;

	if (pm->s.pm_time)
	{
		return;
	}

	pml.ladder = false;

	/* check for ladder */
	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize(flatforward);

	VectorMA(pml.origin, 1, flatforward, spot);
	trace = pm->trace(pml.origin, pm->mins, pm->maxs, spot);

	if ((trace.fraction < 1) && (trace.contents & CONTENTS_LADDER))
	{
		pml.ladder = true;
	}

	/* check for water jump */
	if (pm->waterlevel != 2)
	{
		return;
	}

	VectorMA(pml.origin, 30, flatforward, spot);
	spot[2] += 4;
	cont = pm->pointcontents(spot);

	if (!(cont & CONTENTS_SOLID))
	{
		return;
	}

	spot[2] += 16;
	cont = pm->pointcontents(spot);

	if (cont)
	{
		return;
	}

	/* jump out of water */
	VectorScale(flatforward, 50, pml.velocity);
	pml.velocity[2] = 350;

	pm->s.pm_flags |= PMF_TIME_WATERJUMP;
	pm->s.pm_time = 255;
}

static void
PM_FlyMove(qboolean doclip)
{
	float speed, drop, friction, control, newspeed;
	float currentspeed, addspeed, accelspeed;
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	vec3_t end;
	trace_t trace;

	pm->viewheight = 22;

	/* friction */
	speed = VectorLength(pml.velocity);

	if (speed < 1)
	{
		VectorCopy(vec3_origin, pml.velocity);
	}
	else
	{
		drop = 0;

		friction = pm_friction * 1.5f; /* extra friction */
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control * friction * pml.frametime;

		/* scale the velocity */
		newspeed = speed - drop;

		if (newspeed < 0)
		{
			newspeed = 0;
		}

		newspeed /= speed;

		VectorScale(pml.velocity, newspeed, pml.velocity);
	}

	/* accelerate */
	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.sidemove;

	VectorNormalize(pml.forward);
	VectorNormalize(pml.right);

	for (i = 0; i < 3; i++)
	{
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	}

	wishvel[2] += pm->cmd.upmove;

	VectorCopy(wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	/* clamp to server defined max speed */
	if (wishspeed > pm_maxspeed)
	{
		VectorScale(wishvel, pm_maxspeed / wishspeed, wishvel);
		wishspeed = pm_maxspeed;
	}

	currentspeed = DotProduct(pml.velocity, wishdir);
	addspeed = wishspeed - currentspeed;

	if (addspeed <= 0)
	{
		return;
	}

	accelspeed = pm_accelerate * pml.frametime * wishspeed;

	if (accelspeed > addspeed)
	{
		accelspeed = addspeed;
	}

	for (i = 0; i < 3; i++)
	{
		pml.velocity[i] += accelspeed * wishdir[i];
	}

	if (doclip)
	{
		for (i = 0; i < 3; i++)
		{
			end[i] = pml.origin[i] + pml.frametime * pml.velocity[i];
		}

		trace = pm->trace(pml.origin, pm->mins, pm->maxs, end);

		VectorCopy(trace.endpos, pml.origin);
	}
	else
	{
		/* move */
		VectorMA(pml.origin, pml.frametime, pml.velocity, pml.origin);
	}
}

/*
 * Sets mins, maxs, and pm->viewheight
 */
static void
PM_CheckDuck(void)
{
	trace_t trace;

	pm->mins[0] = -16;
	pm->mins[1] = -16;

	pm->maxs[0] = 16;
	pm->maxs[1] = 16;

	if (pm->s.pm_type == PM_GIB)
	{
		pm->mins[2] = 0;
		pm->maxs[2] = 16;
		pm->viewheight = 8;
		return;
	}

	pm->mins[2] = -24;

	if (pm->s.pm_type == PM_DEAD)
	{
		pm->s.pm_flags |= PMF_DUCKED;
	}
	else if ((pm->cmd.upmove < 0) && (pm->s.pm_flags & PMF_ON_GROUND))
	{
		/* duck */
		pm->s.pm_flags |= PMF_DUCKED;
	}
	else
	{
		/* stand up if possible */
		if (pm->s.pm_flags & PMF_DUCKED)
		{
			/* try to stand up */
			pm->maxs[2] = 32;
			trace = pm->trace(pml.origin, pm->mins, pm->maxs, pml.origin);

			if (!trace.allsolid)
			{
				pm->s.pm_flags &= ~PMF_DUCKED;
			}
		}
	}

	if (pm->s.pm_flags & PMF_DUCKED)
	{
		pm->maxs[2] = 4;
		pm->viewheight = -2;
	}
	else
	{
		pm->maxs[2] = 32;
		pm->viewheight = 22;
	}
}

static void
PM_DeadMove(void)
{
	float forward;

	if (!pm->groundentity)
	{
		return;
	}

	/* extra friction */
	forward = VectorLength(pml.velocity);
	forward -= 20;

	if (forward <= 0)
	{
		VectorClear(pml.velocity);
	}
	else
	{
		VectorNormalize(pml.velocity);
		VectorScale(pml.velocity, forward, pml.velocity);
	}
}

static qboolean
PM_GoodPosition(void)
{
	trace_t trace;
	vec3_t origin, end;
	int i;

	if (pm->s.pm_type == PM_SPECTATOR)
	{
		return true;
	}

	for (i = 0; i < 3; i++)
	{
		origin[i] = end[i] = pml.current_origin[i] * 0.125f;
	}

	trace = pm->trace(origin, pm->mins, pm->maxs, end);

	return !trace.allsolid;
}

/*
 * On exit, the origin will have a value that is pre-quantized to the 0.125
 * precision of the network channel and in a valid position.
 */
static void
PM_SnapPosition(void)
{
	/* try all single bits first */
	static const int jitterbits[8] = {0, 4, 1, 2, 3, 5, 6, 7};
	int base[3], sign[3], i, j;

	/* snap velocity to eigths */
	for (i = 0; i < 3; i++)
	{
		pm->s.velocity[i] = (int)(pml.velocity[i] * 8);
	}

	for (i = 0; i < 3; i++)
	{
		if (pml.origin[i] >= 0)
		{
			sign[i] = 1;
		}
		else
		{
			sign[i] = -1;
		}

		pml.current_origin[i] = (int)(pml.origin[i] * 8);

		if (pml.current_origin[i] * 0.125f == pml.origin[i])
		{
			sign[i] = 0;
		}
	}

	VectorCopy(pml.current_origin, base);

	/* try all combinations */
	for (j = 0; j < 8; j++)
	{
		int bits;

		bits = jitterbits[j];
		VectorCopy(base, pml.current_origin);

		for (i = 0; i < 3; i++)
		{
			if (bits & (1 << i))
			{
				pml.current_origin[i] += sign[i];
			}
		}

		if (PM_GoodPosition())
		{
			return;
		}
	}

	/* go back to the last position */
	VectorCopy(pml.previous_origin, pml.current_origin);
}

static void
PM_InitialSnapPosition(void)
{
	static const int offset[3] = {0, -1, 1};
	int base[3], z;

	VectorCopy(pml.current_origin, base);

	for (z = 0; z < 3; z++)
	{
		int y;

		pml.current_origin[2] = base[2] + offset[z];

		for (y = 0; y < 3; y++)
		{
			int x;

			pml.current_origin[1] = base[1] + offset[y];

			for (x = 0; x < 3; x++)
			{
				pml.current_origin[0] = base[0] + offset[x];

				if (PM_GoodPosition())
				{
					pml.origin[0] = pml.current_origin[0] * 0.125f;
					pml.origin[1] = pml.current_origin[1] * 0.125f;
					pml.origin[2] = pml.current_origin[2] * 0.125f;
					VectorCopy(pml.current_origin, pml.previous_origin);
					return;
				}
			}
		}
	}

	Com_DPrintf("Bad InitialSnapPosition\n");
}

static void
PM_ClampAngles(void)
{
	if (pm->s.pm_flags & PMF_TIME_TELEPORT)
	{
		pm->viewangles[YAW] = SHORT2ANGLE(
				pm->cmd.angles[YAW] + pm->s.delta_angles[YAW]);
		pm->viewangles[PITCH] = 0;
		pm->viewangles[ROLL] = 0;
	}
	else
	{
		int i;

		/* circularly clamp the angles with deltas */
		for (i = 0; i < 3; i++)
		{
			short temp;

			temp = pm->cmd.angles[i] + pm->s.delta_angles[i];
			pm->viewangles[i] = SHORT2ANGLE(temp);
		}

		/* don't let the player look up or down more than 90 degrees */
		if ((pm->viewangles[PITCH] > 89) && (pm->viewangles[PITCH] < 180))
		{
			pm->viewangles[PITCH] = 89;
		}
		else if ((pm->viewangles[PITCH] < 271) && (pm->viewangles[PITCH] >= 180))
		{
			pm->viewangles[PITCH] = 271;
		}
	}

	AngleVectors(pm->viewangles, pml.forward, pml.right, pml.up);
}

#if !defined(DEDICATED_ONLY)
static void
PM_CalculateViewHeightForDemo()
{
	if (pm->s.pm_type == PM_GIB)
	{
		pm->viewheight = 8;
	}
	else
	{
		if ((pm->s.pm_flags & PMF_DUCKED) != 0)
		{
			pm->viewheight = -2;
		}
		else
		{
			pm->viewheight = 22;
		}
	}
}

static void
PM_CalculateWaterLevelForDemo(void)
{
	vec3_t point;
	int cont;

	point[0] = pml.origin[0];
	point[1] = pml.origin[1];
	point[2] = pml.origin[2] + pm->viewheight;

	pm->waterlevel = 0;
	pm->watertype = 0;

	cont = pm->pointcontents(point);

	if ((cont & MASK_WATER) != 0)
	{
		pm->waterlevel = 3;
		pm->watertype = cont;
	}
}

static void
PM_UpdateUnderwaterSfx()
{
	static int underwater;

	if ((pm->waterlevel == 3) && !underwater)
	{
		underwater = 1;
		snd_is_underwater = 1;

#ifdef USE_OPENAL
		AL_Underwater();
#endif
	}

	if ((pm->waterlevel < 3) && underwater)
	{
		underwater = 0;
		snd_is_underwater = 0;

#ifdef USE_OPENAL
		AL_Overwater();
#endif
	}
}
#endif

/*
 * walking up a step should kill some velocity
 */
#define MIN_WALK_NORMAL 0.7f

static int
PM_CorrectAllSolid(trace_t* trace)
{
	int i, j, k;
	vec3_t point;

	/* jitter around */
	for (i = -1; i <= 1; i++)
	{
		for (j = -1; j <= 1; j++)
		{
			for (k = -1; k <= 1; k++)
			{
				VectorCopy(pml.origin, point);
				point[0] += (float)i;
				point[1] += (float)j;
				point[2] += (float)k;
				*trace = pm->trace(point, pm->mins, pm->maxs, point);
				if (!trace->allsolid)
				{
					point[0] = pml.origin[0];
					point[1] = pml.origin[1];
					point[2] = pml.origin[2] - 0.25;

					*trace = pm->trace(pml.origin, pm->mins, pm->maxs, point);
					pml.groundTrace = *trace;
					return true;
				}
			}
		}
	}

	pm->groundentity = NULL;
	pml.groundPlane = false;
	pml.walking = false;

	return false;
}

static void
PM_GroundTrace(void)
{
	vec3_t point, start;
	trace_t trace;

	start[0] = pml.origin[0];
	start[1] = pml.origin[1];
	start[2] = pml.origin[2];

	point[0] = pml.origin[0];
	point[1] = pml.origin[1];
	point[2] = pml.origin[2] - 0.5f;

	trace = pm->trace(start, pm->mins, pm->maxs, point);
	pml.groundTrace = trace;

	/* do something corrective if the trace starts in a solid... */
	if (trace.allsolid)
	{
		if (!PM_CorrectAllSolid(&trace))
		{
			return;
		}
	}

	pm->groundentity = trace.ent;
	pm->GroundContents = &trace.contents;

	if (trace.allsolid)
	{
		pml.groundPlane = false;
		pml.walking = false;
		return;
	}

	// slopes that are too steep will not be considered onground
	if (trace.plane.normal[2] < MIN_WALK_NORMAL)
	{
		pml.groundPlane = true;
		pml.walking = false;
		return;
	}

	pm->numtouch = 0;

	if (pm->numtouch < 31)
	{
		if (pm->groundentity)
		{
			pm->touchents[pm->numtouch] = pm->groundentity;
			pm->numtouch++;
		}
	}

	pml.groundPlane = true;
	pml.walking = true;
}

static void
PM_WalkMove(float fmove, float smove)
{
	int i;
	vec3_t wishvel;
	vec3_t wishdir;
	float wishspeed;

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity(pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP);
	PM_ClipVelocity(pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP);
	//
	VectorNormalize(pml.forward);
	VectorNormalize(pml.right);

	for (i = 0; i < 3; i++)
	{
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	}

	VectorCopy(wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	PM_Accelerate(wishdir, wishspeed, pm_accelerate);

	float vel = VectorLength(pml.velocity);
	// slide along the ground plane
	PM_ClipVelocity(pml.velocity, pml.groundTrace.plane.normal, pml.velocity, OVERCLIP);

	VectorNormalize(pml.velocity);
	pml.velocity[0] = pml.velocity[0] * vel;
	pml.velocity[1] = pml.velocity[1] * vel;
	pml.velocity[2] = pml.velocity[2] * vel;

	// don't do anything if standing still
	if (!pml.velocity[0] && !pml.velocity[1])
	{
		return;
	}

	PM_StepSlideMove(false);
}

static void
PM_CheckInWater()
{
	int contents;
	pmove_t* _pm;
	trace_t tr;
	float origin2[3];
	float origin[3];

	origin[0] = pml.origin[0];
	origin[1] = pml.origin[1];
	origin[2] = pml.origin[2] + pm->mins[2] + 1.0;
	contents = pm->pointcontents(origin);
	if ((contents & 0x38) != 0)
	{
		_pm = pm;
		pm->watertype = contents;
		_pm->waterlevel = 1;
		origin2[0] = origin[0];
		origin2[1] = origin[1];
		origin2[2] = pml.origin[2] + _pm->maxs[2];
		if ((_pm->pointcontents(origin2) & 0x38) != 0)
		{
			_pm = pm;
			pm->waterlevel = 3;
			_pm->waterheight = _pm->maxs[2];
		}
		else
		{
			vec3_t tminmax = {0, 0, 0};
			tr = pm->trace(origin2, tminmax, tminmax, origin);
			_pm = pm;
			pm->waterheight = tr.endpos[2] - pml.origin[2];
			if (tr.fraction < 1.0 /*&& *(float*)&pml.desiredWaterHeight < (long double)_pm->waterheight*/)
				_pm->waterlevel = 2;
		}
	}
	else
	{
		_pm = pm;
		pm->waterlevel = 0;
		_pm->watertype = 0;
		_pm->waterheight = _pm->mins[2];
	}
}

static void
PM_WaterSurfMove()
{
	byte flags;

	PM_SetVelInLiquid(0.5);
	flags = pm->s.w_flags;
	if (flags & WF_SINK)
	{
		pm->s.w_flags = flags & ~WF_SINK;
	}
	else
	{
		pml.velocity[2] = (pm->waterheight/* - *(float*)&pml.desiredWaterHeight*/) / pml.frametime;
		pml.velocity[2] = sin(Sys_Milliseconds() / 150.0) * 8.0 + pml.velocity[2];
	}

	PM_StepSlideMove(true);
}

/*
 * Can be called by either the server or the client
 */
static void
Pmove_(void)
{
	/* clear results */
	pm->numtouch = 0;
	VectorClear(pm->viewangles);
	pm->viewheight = 0;
	pm->groundentity = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	/* convert origin and velocity to float values */
	pml.origin[0] = pml.current_origin[0] * 0.125f;
	pml.origin[1] = pml.current_origin[1] * 0.125f;
	pml.origin[2] = pml.current_origin[2] * 0.125f;

	pml.velocity[0] = pm->s.velocity[0] * 0.125f;
	pml.velocity[1] = pm->s.velocity[1] * 0.125f;
	pml.velocity[2] = pm->s.velocity[2] * 0.125f;

	/* save old org in case we get stuck */
	VectorCopy(pml.current_origin, pml.previous_origin);

	pml.frametime = pm->cmd.msec * 0.001f;

	PM_ClampAngles();

	if (pm->s.pm_type == PM_SPECTATOR)
	{
		PM_FlyMove(false);
		PM_SnapPosition();
		return;
	}

	AngleVectors(pm->viewangles, pml.forward, pml.right, pml.up);

	if (pm->s.pm_type >= PM_DEAD)
	{
		pm->cmd.forwardmove = 0;
		pm->cmd.sidemove = 0;
		pm->cmd.upmove = 0;
	}

	if (pm->s.pm_type == PM_FREEZE)
	{
#if !defined(DEDICATED_ONLY)
		if (cl.attractloop)
		{
			PM_CalculateViewHeightForDemo();
			PM_CalculateWaterLevelForDemo();
			PM_UpdateUnderwaterSfx();
		}
#endif

		return; /* no movement at all */
	}

	/* set mins, maxs, and viewheight */
	PM_CheckDuck();

	if (pm->snapinitial)
	{
		PM_InitialSnapPosition();
	}

	/* set groundentity, watertype, and waterlevel */
	PM_CatagorizePosition();

	if (pm->s.pm_type == PM_DEAD)
	{
		PM_DeadMove();
	}

	PM_CheckSpecialMovement();

	/* drop timing counter */
	if (pm->s.pm_time)
	{
		int msec;

		msec = pm->cmd.msec >> 3;

		if (!msec)
		{
			msec = 1;
		}

		if (msec >= pm->s.pm_time)
		{
			pm->s.pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
			pm->s.pm_time = 0;
		}
		else
		{
			pm->s.pm_time -= msec;
		}
	}

#ifndef DEDICATED_ONLY
	VectorCopy(cl.frame.playerstate.mins, pm->mins);
	VectorCopy(cl.frame.playerstate.maxs, pm->maxs);
#endif

	PM_GroundTrace();

	PM_CheckJump();

	PM_CheckInWater();

	if (pm->waterlevel == 1)
	{
		PM_WaterSurfMove();
	}
	else if (pm->waterlevel == 2)
	{
		if (pm->viewangles[0] > 40.0)
		{
			PM_WaterMove();
		}
		else
		{
			PM_WaterSurfMove();
		}
	}
	else if (pm->waterlevel >= 2)
	{
		PM_WaterMove();
	}
	else if (pml.walking)
	{
		// walking on ground
		PM_WalkMove(pm->cmd.forwardmove * 8, pm->cmd.sidemove * 8);
	}
	else
	{
		// airborne
		PM_AirMove();
	}

	PM_SnapPosition();
}

/* Old version of pmove */
void
Pmove(pmove_t *pmove)
{
	pm = pmove;

	/* clear all pmove local vars */
	memset(&pml, 0, sizeof(pml));

	/* 28.3 -> 12.3 coordiantes */
	VectorCopy(pmove->s.origin, pml.current_origin);
	Pmove_();
	/* 28.3 -> 12.3 coordiantes */
	VectorCopy(pml.current_origin, pmove->s.origin);
}

void
PmoveEx(pmove_t *pmove, int *origin)
{
	pm = pmove;

	/* clear all pmove local vars */
	memset(&pml, 0, sizeof(pml));

	/* 28.3 -> 12.3 coordiantes */
	VectorCopy(origin, pml.current_origin);
	Pmove_();
	/* 28.3 -> 12.3 coordiantes */
	VectorCopy(pml.current_origin, origin);
}
