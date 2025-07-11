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
 *  =======================================================================
 *
 * This file implements the camera, e.g the player's view
 *
 * =======================================================================
 */

#include "header/client.h"
#include "input/header/input.h"
#include "../game/header/client_effects.h"

/* development tools for weapons */
int gun_frame;
struct model_s *gun_model;

cvar_t *crosshair;
cvar_t *crosshair_3d;
cvar_t *crosshair_3d_glow;

cvar_t *crosshair_scale;
cvar_t *cl_testparticles;
cvar_t *cl_testentities;
cvar_t *cl_testlights;
cvar_t *cl_testblend;
cvar_t *crosshair_3d_glow_r;
cvar_t *crosshair_3d_glow_g;
cvar_t *crosshair_3d_glow_b;

cvar_t *cl_stats;

int r_numdlights;
dlight_t r_dlights[MAX_DLIGHTS];

int r_numentities;
entity_t r_entities[MAX_ENTITIES];

int r_numparticles;
particle_t r_particles[MAX_PARTICLES];

static lightstyle_t r_lightstyles[MAX_LIGHTSTYLES];

char cl_weaponmodels[MAX_CLIENTWEAPONMODELS][MAX_QPATH];
int num_cl_weaponmodels;

/*
 * Specifies the model that will be used as the world
 */
void
V_ClearScene(void)
{
	r_numdlights = 0;
	r_numentities = 0;
	r_numparticles = 0;
}

void
V_AddEntity(entity_t *ent)
{
	if (r_numentities >= MAX_ENTITIES)
	{
		return;
	}

	r_entities[r_numentities++] = *ent;
}

void
V_AddParticle(vec3_t org, unsigned int color, float alpha)
{
	particle_t *p;

	if (r_numparticles >= MAX_PARTICLES)
	{
		return;
	}

	p = &r_particles[r_numparticles++];
	VectorCopy(org, p->origin);
	p->color = color;
	p->alpha = alpha;
}

void
V_AddLight(vec3_t org, float intensity, float r, float g, float b)
{
	dlight_t *dl;

	if (r_numdlights >= MAX_DLIGHTS)
	{
		return;
	}

	dl = &r_dlights[r_numdlights++];
	VectorCopy(org, dl->origin);
	dl->intensity = intensity;
	dl->color[0] = r;
	dl->color[1] = g;
	dl->color[2] = b;
}

void
V_AddLightStyle(int style, float r, float g, float b)
{
	lightstyle_t *ls;

	if ((style < 0) || (style >= MAX_LIGHTSTYLES))
	{
		Com_Error(ERR_DROP, "Bad light style %i", style);
	}

	ls = &r_lightstyles[style];

	ls->white = r + g + b;
	ls->rgb[0] = r;
	ls->rgb[1] = g;
	ls->rgb[2] = b;
}

/*
 *If cl_testparticles is set, create 4096 particles in the view
 */
static void
V_TestParticles(void)
{
	particle_t *p;
	int i, j;
	float d, r, u;

	r_numparticles = MAX_PARTICLES;

	for (i = 0; i < r_numparticles; i++)
	{
		d = i * 0.25f;
		r = 4 * ((i & 7) - 3.5f);
		u = 4 * (((i >> 3) & 7) - 3.5f);
		p = &r_particles[i];

		for (j = 0; j < 3; j++)
		{
			p->origin[j] = cl.refdef.vieworg[j] + cl.v_forward[j] * d +
						   cl.v_right[j] * r + cl.v_up[j] * u;
		}

		/* grey */
		p->color = 0xFF7B7B7B;
		p->alpha = cl_testparticles->value;
	}
}

/*
 * If cl_testentities is set, create 32 player models
 */
static void
V_TestEntities(void)
{
	int i, j;
	float f, r;
	entity_t *ent;

	r_numentities = 32;
	memset(r_entities, 0, sizeof(r_entities));

	for (i = 0; i < r_numentities; i++)
	{
		ent = &r_entities[i];

		r = 64.0f * ((float)(i % 4) - 1.5f);
		f = (float)(64 * (i / 4) + 128);

		for (j = 0; j < 3; j++)
		{
			ent->origin[j] = cl.refdef.vieworg[j] + cl.v_forward[j] * f +
							 cl.v_right[j] * r;
		}

		ent->model = cl.baseclientinfo.model;
		ent->skin = cl.baseclientinfo.skin;
	}
}

/*
 * If cl_testlights is set, create 32 lights models
 */
static void
V_TestLights(void)
{
	int i;

	r_numdlights = MAX_DLIGHTS;
	memset(r_dlights, 0, sizeof(r_dlights));

	for (i = 0; i < r_numdlights; i++)
	{
		dlight_t *dl;
		float f, r;
		int j;

		dl = &r_dlights[i];

		r = 64 * ((i % 4) - 1.5f);
		f = 64 * (i / 4.0f) + 128;

		for (j = 0; j < 3; j++)
		{
			dl->origin[j] = cl.refdef.vieworg[j] + cl.v_forward[j] * f +
							cl.v_right[j] * r;
		}

		dl->color[0] = (float)(((i % 6) + 1) & 1);
		dl->color[1] = (float)((((i % 6) + 1) & 2) >> 1);
		dl->color[2] = (float)((((i % 6) + 1) & 4) >> 2);
		dl->intensity = 200;
	}
}

static void
V_Listlights_f(void)
{
	int i;

	Com_Printf("LigthStyle:\n");
	for (i = 0; i < MAX_LIGHTSTYLES; i++)
	{
		lightstyle_t *ls;

		ls = &r_lightstyles[i];

		if (ls->white == 0)
		{
			continue;
		}

		Com_Printf("%d: rgb: (%.2f, %.2f, %.2f)\n",
			i, ls->rgb[0], ls->rgb[1], ls->rgb[2]);
	}

	Com_Printf("DLigths:\n");
	for (i = 0; i < r_numdlights; i++)
	{
		dlight_t *dl;

		dl = &r_dlights[i];

		Com_Printf("%d: intensity: %.2f rgb: (%.2f, %.2f, %.2f) origin: (%.2f, %.2f, %.2f)\n",
			i, dl->intensity, dl->color[0], dl->color[1], dl->color[2],
			dl->origin[0], dl->origin[1], dl->origin[2]
		);
	}

	Com_Printf("DLigths: %d\n", r_numdlights);
}

/* time relative to */
static int sec_start = 0;

static void
CL_PrintInSameLine(const char *message)
{
	char emptyline[80]; /* clear full 25x80 line*/
	float scale;
	int cols, linesize;

	if (developer->value)
	{
		Com_Printf("%s: %.2fs:%s\n",
			__func__, (Sys_Milliseconds() - sec_start) / 1000.0, message);
		return;
	}

	scale = SCR_GetConsoleScale();
	if (scale < 1)
	{
		scale = 1;
	}

	cols = viddef.width / (8 * scale) - 1;

	if (cols > (sizeof(emptyline) - 1))
	{
		cols = sizeof(emptyline) - 1;
	}

	/* go to line start */
	linesize = snprintf(emptyline, cols - 2, "\r%s", message);
	/* fill all with spaces */
	memset(emptyline + linesize, ' ', cols - linesize);
	/* go to begin after print */
	emptyline[cols - 1] = '\r';
	emptyline[cols] = 0;

	Com_Printf("%s", emptyline);
}

/*
 * Call before entering a new level, or after changing dlls
 */
void
CL_PrepRefresh(void)
{
	char mapname[MAX_QPATH];
	int i;
	float rotate = 0;
	int autorotate = 1;
	vec3_t axis;

	if (!cl.configstrings[CS_MODELS + 1][0])
	{
		return;
	}

	sec_start = Sys_Milliseconds();
	SCR_AddDirtyPoint(0, 0);
	SCR_AddDirtyPoint(viddef.width - 1, viddef.height - 1);

	/* let the refresher load the map */
	Q_strlcpy(mapname, cl.configstrings[CS_MODELS + 1] + 5, sizeof(mapname)); /* skip "maps/" */
	mapname[strlen(mapname) - 4] = 0; /* cut off ".bsp" */

	/* register models, pics, and skins */
	Com_Printf("Map: %s\n", mapname);
	SCR_UpdateScreen();
	CL_PrintInSameLine("Map is loading...");
	R_BeginRegistration(mapname);

	/* precache status bar pics */
	CL_PrintInSameLine("Pics");
	SCR_UpdateScreen();
	SCR_TouchPics();
	CL_PrintInSameLine("Temporary models");

#ifdef NATIVEQUAKE2
	CL_RegisterTEntModels();

	num_cl_weaponmodels = 1;
	strcpy(cl_weaponmodels[0], "weapon.md2");
#else
	if (fxe && fxe->RegisterModels)
	{
		fxe->RegisterModels();
	}
#endif

	CL_PrintInSameLine("Models");
	SCR_UpdateScreen();

	for (i = 1; i < MAX_MODELS && cl.configstrings[CS_MODELS + i][0]; i++)
	{
		const char *name;

		name = cl.configstrings[CS_MODELS + i];

		if (developer->value && name[0] != '*')
		{
			CL_PrintInSameLine(name);
			SCR_UpdateScreen();
			IN_Update();
		}

		if (*name == '#')
		{
			/* special player weapon model */
			if (num_cl_weaponmodels < MAX_CLIENTWEAPONMODELS)
			{
				Q_strlcpy(cl_weaponmodels[num_cl_weaponmodels],
						cl.configstrings[CS_MODELS + i] + 1,
						sizeof(cl_weaponmodels[num_cl_weaponmodels]));

				num_cl_weaponmodels++;
			}
		}
		else
		{
			cl.model_draw[i] = R_RegisterModel(cl.configstrings[CS_MODELS + i]);

			cl.model_clip[i] = (*name == '*') ?
				CM_InlineModel(name) :
				NULL;
		}
	}

	CL_PrintInSameLine("Images");
	SCR_UpdateScreen();
	IN_Update();

	for (i = 1; i < MAX_IMAGES && cl.configstrings[CS_IMAGES + i][0]; i++)
	{
		cl.image_precache[i] = Draw_FindPic(cl.configstrings[CS_IMAGES + i]);
	}

	CL_PrintInSameLine("Clients");
	SCR_UpdateScreen();

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (cl.configstrings[CS_PLAYERSKINS + i][0])
		{
			if (developer->value)
			{
				Com_Printf("Client %i\r", i);
				SCR_UpdateScreen();
				IN_Update();
			}

			CL_ParseClientinfo(i);
		}
	}

	CL_LoadClientinfo(&cl.baseclientinfo, "unnamed\\male/grunt");

	/* set sky textures and speed */
	CL_PrintInSameLine("Sky");
	SCR_UpdateScreen();
	sscanf(cl.configstrings[CS_SKYROTATE], "%f %d", &rotate, &autorotate);
	sscanf(cl.configstrings[CS_SKYAXIS], "%f %f %f", &axis[0], &axis[1], &axis[2]);
	R_SetSky(cl.configstrings[CS_SKY], rotate, autorotate, axis);
	CL_PrintInSameLine("Cleanup.....");

	/* the renderer can now free unneeded stuff */
	R_EndRegistration();
	CL_PrintInSameLine("Map loaded.");

	/* clear any lines of console text */
	Con_ClearNotify();

	SCR_UpdateScreen();
	cl.refresh_prepped = true;
	cl.force_refdef = true; /* make sure we have a valid refdef */

	/* start the cd track */
	OGG_PlayTrack(cl.configstrings[CS_CDTRACK], true, true);
}

float
CalcFov(float fov_x, float width, float height)
{
	float a;
	float x;

	if ((fov_x < 1) || (fov_x > 179))
	{
		Com_Error(ERR_DROP, "Bad fov: %f", fov_x);
	}

	x = width / (float)tan(fov_x / 360 * M_PI);

	a = (float)atan(height / x);

	a = a * 360 / M_PI;

	return a;
}

/* gun frame debugging functions */
static void
V_Gun_Next_f(void)
{
	gun_frame++;
	Com_Printf("frame %i\n", gun_frame);
}

static void
V_Gun_Prev_f(void)
{
	gun_frame--;

	if (gun_frame < 0)
	{
		gun_frame = 0;
	}

	Com_Printf("frame %i\n", gun_frame);
}

static void
V_Gun_Model_f(void)
{
	char name[MAX_QPATH];

	if (Cmd_Argc() != 2)
	{
		gun_model = NULL;
		return;
	}

	Com_sprintf(name, sizeof(name), "models/%s/tris.md2", Cmd_Argv(1));
	gun_model = R_RegisterModel(name);
}

static int
entitycmpfnc(const entity_t *a, const entity_t *b)
{
	/* all other models are sorted by model then skin */
	if (a->model == b->model)
	{
		return (a->skin == b->skin) ? 0 :
			(a->skin > b->skin) ? 1 : -1;
	}
	else
	{
		return (a->model > b->model) ? 1 : -1;
	}
}

static void
V_Render3dCrosshair(void)
{
	trace_t crosshair_trace;
	vec3_t end;

	crosshair_3d = Cvar_Get("crosshair_3d", "0", CVAR_ARCHIVE);
	crosshair_3d_glow = Cvar_Get("crosshair_3d_glow", "0", CVAR_ARCHIVE);


	if(crosshair_3d->value || crosshair_3d_glow->value){
		VectorMA(cl.refdef.vieworg, 8192, cl.v_forward,end);
		crosshair_trace = CL_PMTrace(cl.refdef.vieworg, vec3_origin, vec3_origin, end);

		if(crosshair_3d_glow->value){
			crosshair_3d_glow_r = Cvar_Get("crosshair_3d_glow_r", "5", CVAR_ARCHIVE);
			crosshair_3d_glow_g = Cvar_Get("crosshair_3d_glow_g", "1", CVAR_ARCHIVE);
			crosshair_3d_glow_b = Cvar_Get("crosshair_3d_glow_b", "4", CVAR_ARCHIVE);

			V_AddLight(
				crosshair_trace.endpos,
				crosshair_3d_glow->value,
				crosshair_3d_glow_r->value,
				crosshair_3d_glow_g->value,
				crosshair_3d_glow_b->value
			);
		}

		if(crosshair_3d->value){
			entity_t crosshair_ent = {0};

			crosshair_ent.origin[0] = crosshair_trace.endpos[0];
			crosshair_ent.origin[1] = crosshair_trace.endpos[1];
			crosshair_ent.origin[2] = crosshair_trace.endpos[2];

			crosshair_ent.model = R_RegisterModel("models/crosshair/tris.md2");
			//crosshair_ent.skin = R_RegisterSkin("models/crosshair/skin.pcx");

			AngleVectors2(crosshair_trace.plane.normal, crosshair_ent.angles);
			crosshair_ent.flags = RF_DEPTHHACK | RF_FULLBRIGHT | RF_NOSHADOW;

			V_AddEntity(&crosshair_ent);
		}
	}
}

void
V_RenderView(float stereo_separation)
{
	if (cls.state != ca_active)
	{
		R_EndWorldRenderpass();
		return;
	}

	if (!cl.refresh_prepped)
	{
		R_EndWorldRenderpass();
		return;			// still loading
	}

	if (cl_timedemo->value)
	{
		if (!cl.timedemo_start)
		{
			cl.timedemo_start = Sys_Milliseconds();
		}

		cl.timedemo_frames++;
	}

	/* an invalid frame will just use the exact previous refdef
	   we can't use the old frame if the video mode has changed, though... */
	if (cl.frame.valid && (cl.force_refdef || !cl_paused->value))
	{
		cl.force_refdef = false;

		V_ClearScene();

		/* build a refresh entity list and calc cl.sim*
		   this also calls CL_CalcViewValues which loads
		   v_forward, etc. */
		CL_AddEntities();

		// before changing viewport we should trace the crosshair position
		V_Render3dCrosshair();

		if (cl_testparticles->value)
		{
			V_TestParticles();
		}

		if (cl_testentities->value)
		{
			V_TestEntities();
		}

		if (cl_testlights->value)
		{
			V_TestLights();
		}

		if (cl_testblend->value)
		{
			cl.refdef.blend[0] = 1;
			cl.refdef.blend[1] = 0.5;
			cl.refdef.blend[2] = 0.25;
			cl.refdef.blend[3] = 0.5;
		}

		/* offset vieworg appropriately if
		   we're doing stereo separation */

		if (stereo_separation != 0)
		{
			vec3_t tmp;

			VectorScale(cl.v_right, stereo_separation, tmp);
			VectorAdd(cl.refdef.vieworg, tmp, cl.refdef.vieworg);
		}

		/* never let it sit exactly on a node line, because a water plane can
		   dissapear when viewed with the eye exactly on it. the server protocol
		   only specifies to 1/8 pixel, so add 1/16 in each axis */
		cl.refdef.vieworg[0] += 1.0 / 16;
		cl.refdef.vieworg[1] += 1.0 / 16;
		cl.refdef.vieworg[2] += 1.0 / 16;

		cl.refdef.time = cl.time * 0.001f;

		cl.refdef.areabits = cl.frame.areabits;

		if (!cl_add_entities->value)
		{
			r_numentities = 0;
		}

		if (!cl_add_particles->value)
		{
			r_numparticles = 0;
		}

		if (!cl_add_lights->value)
		{
			r_numdlights = 0;
		}

		if (!cl_add_blend->value)
		{
			VectorClear(cl.refdef.blend);
		}

		cl.refdef.num_entities = r_numentities;
		cl.refdef.entities = r_entities;
		cl.refdef.num_particles = r_numparticles;
		cl.refdef.particles = r_particles;
		cl.refdef.num_dlights = r_numdlights;
		cl.refdef.dlights = r_dlights;
		cl.refdef.lightstyles = r_lightstyles;

		cl.refdef.rdflags = cl.frame.playerstate.rdflags;

		/* sort entities for better cache locality */
		qsort(cl.refdef.entities, cl.refdef.num_entities,
				sizeof(cl.refdef.entities[0]), (int (*)(const void *, const void *))
				entitycmpfnc);
	} else if (cl.frame.valid && cl_paused->value && gl1_stereo->value) {
		// We need to adjust the refdef in stereo mode when paused.
		vec3_t tmp;
		CL_CalcViewValues();
		VectorScale( cl.v_right, stereo_separation, tmp );
		VectorAdd( cl.refdef.vieworg, tmp, cl.refdef.vieworg );

		cl.refdef.vieworg[0] += 1.0/16;
		cl.refdef.vieworg[1] += 1.0/16;
		cl.refdef.vieworg[2] += 1.0/16;

		cl.refdef.time = cl.time*0.001;
	}

	cl.refdef.x = scr_vrect.x;
	cl.refdef.y = scr_vrect.y;
	cl.refdef.width = scr_vrect.width;
	cl.refdef.height = scr_vrect.height;
	cl.refdef.fov_y = CalcFov(cl.refdef.fov_x, (float)cl.refdef.width,
				(float)cl.refdef.height);

	R_RenderFrame(&cl.refdef);

	if (cl_stats->value)
	{
		Com_Printf("ent:%i  lt:%i  part:%i\n", r_numentities,
				r_numdlights, r_numparticles);
	}

	if (log_stats->value && (log_stats_file != 0))
	{
		fprintf(log_stats_file, "%i,%i,%i,", r_numentities,
				r_numdlights, r_numparticles);
	}

	SCR_AddDirtyPoint(scr_vrect.x, scr_vrect.y);
	SCR_AddDirtyPoint(scr_vrect.x + scr_vrect.width - 1,
			scr_vrect.y + scr_vrect.height - 1);

	SCR_DrawCrosshair();
}

static void
V_Viewpos_f(void)
{
	Com_Printf("position: %i %i %i, angles: %i %i %i\n",
			(int)cl.refdef.vieworg[0], (int)cl.refdef.vieworg[1],
			(int)cl.refdef.vieworg[2],
			(int)cl.refdef.viewangles[PITCH], (int)cl.refdef.viewangles[YAW],
			(int)cl.refdef.viewangles[ROLL]);
}

void
V_Init(void)
{
	Cmd_AddCommand("gun_next", V_Gun_Next_f);
	Cmd_AddCommand("gun_prev", V_Gun_Prev_f);
	Cmd_AddCommand("gun_model", V_Gun_Model_f);

	Cmd_AddCommand("viewpos", V_Viewpos_f);
	Cmd_AddCommand("listlights", V_Listlights_f);

	crosshair = Cvar_Get("crosshair", "0", CVAR_ARCHIVE);
	crosshair_scale = Cvar_Get("crosshair_scale", "-1", CVAR_ARCHIVE);
	cl_testblend = Cvar_Get("cl_testblend", "0", 0);
	cl_testparticles = Cvar_Get("cl_testparticles", "0", 0);
	cl_testentities = Cvar_Get("cl_testentities", "0", 0);
	cl_testlights = Cvar_Get("cl_testlights", "0", 0);

	cl_stats = Cvar_Get("cl_stats", "0", 0);
}

