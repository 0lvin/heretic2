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
 * Mesh logic
 *
 * =======================================================================
 */

#include "../ref_shared.h"

static const float r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "../constants/anorms.h"
};

static vec4_t *lerpbuff = NULL;
static int lerpbuffnum = 0;

vec4_t *
R_VertBufferRealloc(int num)
{
	void *ptr;

	if (num < lerpbuffnum)
	{
		return lerpbuff;
	}

	lerpbuffnum = num * 2;
	ptr = realloc(lerpbuff, lerpbuffnum * sizeof(vec4_t));
	if (!ptr)
	{
		Com_Error(ERR_FATAL, "%s: can't allocate memory", __func__);

		return NULL;
	}

	lerpbuff = ptr;

	return lerpbuff;
}

void
R_VertBufferInit(void)
{
	lerpbuff = NULL;
	lerpbuffnum = 0;
	R_VertBufferRealloc(MAX_VERTS);
}

void
R_VertBufferFree(void)
{
	if (lerpbuff)
	{
		free(lerpbuff);
		lerpbuff = NULL;
	}
	lerpbuffnum = 0;
}

/* compressed vertex normals used by mdl and md2 model formats */
void
R_ConvertNormalMDL(byte in_normal, signed char *normal)
{
	const float *norm;
	int n;

	norm = r_avertexnormals[in_normal % NUMVERTEXNORMALS];

	for (n = 0; n < 3; n ++)
	{
		normal[n] = norm[n] * 127.f;
	}
}

void
R_LerpVerts(qboolean powerUpEffect, int nverts,
		const dxtrivertx_t *v, const dxtrivertx_t *ov,
		float *lerp, const float move[3],
		const float frontv[3], const float backv[3])
{
	int i;

	if (powerUpEffect)
	{
		for (i = 0; i < nverts; i++, v++, ov++, lerp += 4)
		{
			int n;

			for (n = 0; n < 3; n ++)
			{
				float normal;

				normal = v->normal[n] / 127.f;

				lerp[n] = move[n] + ov->v[n] * backv[n] + v->v[n] * frontv[n] +
						  normal * POWERSUIT_SCALE;
			}
		}
	}
	else
	{
		for (i = 0; i < nverts; i++, v++, ov++, lerp += 4)
		{
			lerp[0] = move[0] + ov->v[0] * backv[0] + v->v[0] * frontv[0];
			lerp[1] = move[1] + ov->v[1] * backv[1] + v->v[1] * frontv[1];
			lerp[2] = move[2] + ov->v[2] * backv[2] + v->v[2] * frontv[2];
		}
	}
}

qboolean
R_CullAliasMeshModel(dmdx_t *paliashdr, cplane_t *frustum, int frame, int oldframe, vec3_t e_angles,
	vec3_t e_origin, vec3_t bbox[8])
{
	int i;
	vec3_t mins, maxs;
	vec3_t vectors[3];
	vec3_t thismins, oldmins, thismaxs, oldmaxs;
	daliasxframe_t *pframe, *poldframe;
	vec3_t angles;

	pframe = (daliasxframe_t *)((byte *)paliashdr + paliashdr->ofs_frames +
			frame * paliashdr->framesize);

	poldframe = (daliasxframe_t *)((byte *)paliashdr + paliashdr->ofs_frames +
			oldframe * paliashdr->framesize);

	/* compute axially aligned mins and maxs */
	if (pframe == poldframe)
	{
		for (i = 0; i < 3; i++)
		{
			mins[i] = pframe->translate[i];
			maxs[i] = mins[i] + pframe->scale[i] * 0xFFFF;
		}
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			thismins[i] = pframe->translate[i];
			thismaxs[i] = thismins[i] + pframe->scale[i] * 0xFFFF;

			oldmins[i] = poldframe->translate[i];
			oldmaxs[i] = oldmins[i] + poldframe->scale[i] * 0xFFFF;

			if (thismins[i] < oldmins[i])
			{
				mins[i] = thismins[i];
			}
			else
			{
				mins[i] = oldmins[i];
			}

			if (thismaxs[i] > oldmaxs[i])
			{
				maxs[i] = thismaxs[i];
			}
			else
			{
				maxs[i] = oldmaxs[i];
			}
		}
	}

	/* compute a full bounding box */
	for (i = 0; i < 8; i++)
	{
		vec3_t tmp;

		if (i & 1)
		{
			tmp[0] = mins[0];
		}
		else
		{
			tmp[0] = maxs[0];
		}

		if (i & 2)
		{
			tmp[1] = mins[1];
		}
		else
		{
			tmp[1] = maxs[1];
		}

		if (i & 4)
		{
			tmp[2] = mins[2];
		}
		else
		{
			tmp[2] = maxs[2];
		}

		VectorCopy(tmp, bbox[i]);
	}

	/* rotate the bounding box */
	VectorCopy(e_angles, angles);
	angles[YAW] = -angles[YAW];
	AngleVectors(angles, vectors[0], vectors[1], vectors[2]);

	for (i = 0; i < 8; i++)
	{
		vec3_t tmp;

		VectorCopy(bbox[i], tmp);

		bbox[i][0] = DotProduct(vectors[0], tmp);
		bbox[i][1] = -DotProduct(vectors[1], tmp);
		bbox[i][2] = DotProduct(vectors[2], tmp);

		VectorAdd(e_origin, bbox[i], bbox[i]);
	}

	int p, f, aggregatemask = ~0;

	for (p = 0; p < 8; p++)
	{
		int mask = 0;

		for (f = 0; f < 4; f++)
		{
			float dp = DotProduct(frustum[f].normal, bbox[p]);

			if ((dp - frustum[f].dist) < 0)
			{
				mask |= (1 << f);
			}
		}

		aggregatemask &= mask;
	}

	if (aggregatemask)
	{
		return true;
	}

	return false;
}
