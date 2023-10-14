/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (C) 2016-2017 Daniel Gibson
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
 * Warps. Used on water surfaces und for skybox rotation.
 *
 * =======================================================================
 */

#include "header/local.h"

/*
 * Does a water warp on the pre-fragmented mpoly_t chain
 */
void
GL4_EmitWaterPolys(msurface_t *fa)
{
	mpoly_t *bp;
	float scroll = 0.0f;

	if (fa->texinfo->flags & SURF_FLOWING)
	{
		scroll = -64.0f * ((gl4_newrefdef.time * 0.5) - (int)(gl4_newrefdef.time * 0.5));
		if (scroll == 0.0f) // this is done in GL4_DrawGLFlowingPoly() TODO: keep?
		{
			scroll = -64.0f;
		}
	}

	qboolean updateUni3D = false;
	if(gl4state.uni3DData.scroll != scroll)
	{
		gl4state.uni3DData.scroll = scroll;
		updateUni3D = true;
	}
	// these surfaces (mostly water and lava, I think?) don't have a lightmap.
	// rendering water at full brightness looks bad (esp. for water in dark environments)
	// so default use a factor of 0.5 (ontop of intensity)
	// but lava should be bright and glowing, so use full brightness there
	float lightScale = fa->texinfo->image->is_lava ? 1.0f : 0.5f;
	if(lightScale != gl4state.uni3DData.lightScaleForTurb)
	{
		gl4state.uni3DData.lightScaleForTurb = lightScale;
		updateUni3D = true;
	}

	if(updateUni3D)
	{
		GL4_UpdateUBO3D();
	}

	GL4_UseProgram(gl4state.si3Dturb.shaderProgram);

	GL4_BindVAO(gl4state.vao3D);
	GL4_BindVBO(gl4state.vbo3D);

	for (bp = fa->polys; bp != NULL; bp = bp->next)
	{
		GL4_BufferAndDraw3D(bp->verts, bp->numverts, GL_TRIANGLE_FAN);
	}
}

// ########### below: Sky-specific stuff ##########

#define ON_EPSILON 0.1 /* point on plane side epsilon */
enum { MAX_CLIP_VERTS = 64 };

static float skyrotate;
static int skyautorotate;
static vec3_t skyaxis;
static gl4image_t *sky_images[6];
static const int skytexorder[6] = {0, 2, 1, 3, 4, 5};

/* 3dstudio environment map names */
static const char *suf[6] = {"rt", "bk", "lf", "ft", "up", "dn"};

static float skymins[2][6], skymaxs[2][6];
static float sky_min, sky_max;

void
GL4_SetSky(const char *name, float rotate, int autorotate, const vec3_t axis)
{
	char	skyname[MAX_QPATH];
	int		i;

	Q_strlcpy(skyname, name, sizeof(skyname));
	skyrotate = rotate;
	skyautorotate = autorotate;
	VectorCopy(axis, skyaxis);

	for (i = 0; i < 6; i++)
	{
		gl4image_t	*image;

		image = (gl4image_t *)GetSkyImage(skyname, suf[i],
			r_palettedtexture->value, (findimage_t)GL4_FindImage);

		if (!image)
		{
			R_Printf(PRINT_ALL, "%s: can't load %s:%s sky\n",
				__func__, skyname, suf[i]);
			image = gl4_notexture;
		}

		sky_images[i] = image;
	}

	sky_min = 1.0 / 512;
	sky_max = 511.0 / 512;
}

void
GL4_AddSkySurface(msurface_t *fa)
{
	R_AddSkySurface(fa, skymins, skymaxs, gl4_origin);
}

void
RE_ClearSkyBox(void)
{
	R_ClearSkyBox(skymins, skymaxs);
}

void
GL4_DrawSkyBox(void)
{
	int i;
	qboolean farsee;

	farsee = (r_farsee->value == 0);

	if (skyrotate)
	{   /* check for no sky at all */
		for (i = 0; i < 6; i++)
		{
			if ((skymins[0][i] < skymaxs[0][i]) &&
			    (skymins[1][i] < skymaxs[1][i]))
			{
				break;
			}
		}

		if (i == 6)
		{
			return; /* nothing visible */
		}
	}

	// glPushMatrix();
	hmm_mat4 origModelMat = gl4state.uni3DData.transModelMat4;

	// glTranslatef(gl4_origin[0], gl4_origin[1], gl4_origin[2]);
	hmm_vec3 transl = HMM_Vec3(gl4_origin[0], gl4_origin[1], gl4_origin[2]);
	hmm_mat4 modMVmat = HMM_MultiplyMat4(origModelMat, HMM_Translate(transl));
	if(skyrotate != 0.0f)
	{
		// glRotatef(gl4_newrefdef.time * skyrotate, skyaxis[0], skyaxis[1], skyaxis[2]);
		hmm_vec3 rotAxis = HMM_Vec3(skyaxis[0], skyaxis[1], skyaxis[2]);
		modMVmat = HMM_MultiplyMat4(modMVmat, HMM_Rotate(
			(skyautorotate ? gl4_newrefdef.time : 1.f) * skyrotate, rotAxis));
	}
	gl4state.uni3DData.transModelMat4 = modMVmat;
	GL4_UpdateUBO3D();

	GL4_UseProgram(gl4state.si3Dsky.shaderProgram);
	GL4_BindVAO(gl4state.vao3D);
	GL4_BindVBO(gl4state.vbo3D);

	// TODO: this could all be done in one drawcall.. but.. whatever, it's <= 6 drawcalls/frame

	mvtx_t skyVertices[4];

	for (i = 0; i < 6; i++)
	{
		if (skyrotate != 0.0f)
		{
			skymins[0][i] = -1;
			skymins[1][i] = -1;
			skymaxs[0][i] = 1;
			skymaxs[1][i] = 1;
		}

		if ((skymins[0][i] >= skymaxs[0][i]) ||
		    (skymins[1][i] >= skymaxs[1][i]))
		{
			continue;
		}

		GL4_Bind(sky_images[skytexorder[i]]->texnum);

		R_MakeSkyVec( skymins [ 0 ] [ i ], skymins [ 1 ] [ i ], i, &skyVertices[0],
			farsee, sky_min, sky_max);
		R_MakeSkyVec( skymins [ 0 ] [ i ], skymaxs [ 1 ] [ i ], i, &skyVertices[1],
			farsee, sky_min, sky_max);
		R_MakeSkyVec( skymaxs [ 0 ] [ i ], skymaxs [ 1 ] [ i ], i, &skyVertices[2],
			farsee, sky_min, sky_max);
		R_MakeSkyVec( skymaxs [ 0 ] [ i ], skymins [ 1 ] [ i ], i, &skyVertices[3],
			farsee, sky_min, sky_max);

		GL4_BufferAndDraw3D(skyVertices, 4, GL_TRIANGLE_FAN);
	}

	// glPopMatrix();
	gl4state.uni3DData.transModelMat4 = origModelMat;
	GL4_UpdateUBO3D();
}
