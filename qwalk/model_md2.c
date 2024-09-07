/*
    QShed <http://www.icculus.org/qshed>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "anorms.h"
#include "global.h"
#include "model.h"
#include "palettes.h"
#include "model_md2.h"

extern int texwidth, texheight;
extern const char *g_skinpath;

bool_t model_md2_load(void *filedata, size_t filesize, model_t *out_model, char **out_error)
{
	typedef struct md2_meshvert_s
	{
		unsigned short vertex;
		unsigned short texcoord;
	} md2_meshvert_t;

	unsigned char * const f = (unsigned char*)filedata;
	mem_pool_t *pool;
	dmdl_t *header;
	int i, j;
	model_t model;
	mesh_t *mesh;
	skininfo_t *skininfo;
	frameinfo_t *frameinfo;
	md2_meshvert_t *meshverts;
	float iwidth, iheight;
	float *v, *n;

	header = (dmdl_t*)f;

	/* validate format */
	if (LittleLong(header->ident) != IDALIASHEADER)
		return (void)(out_error && (*out_error = msprintf("wrong format (not %d)", IDALIASHEADER))), false;
	if (LittleLong(header->version) != ALIAS_VERSION)
		return (void)(out_error && (*out_error = msprintf("wrong format (version not %d)", ALIAS_VERSION))), false;

	pool = mem_create_pool();

	/* byteswap file */
	header->version       = LittleLong(header->version);
	header->skinwidth     = LittleLong(header->skinwidth);
	header->skinheight    = LittleLong(header->skinheight);
	header->framesize     = LittleLong(header->framesize);
	header->num_skins     = LittleLong(header->num_skins);
	header->num_xyz       = LittleLong(header->num_xyz);
	header->num_st        = LittleLong(header->num_st);
	header->num_tris      = LittleLong(header->num_tris);
	header->num_glcmds    = LittleLong(header->num_glcmds);
	header->num_frames    = LittleLong(header->num_frames);
	header->ofs_skins     = LittleLong(header->ofs_skins);
	header->ofs_st        = LittleLong(header->ofs_st);
	header->ofs_tris      = LittleLong(header->ofs_tris);
	header->ofs_frames    = LittleLong(header->ofs_frames);
	header->ofs_glcmds    = LittleLong(header->ofs_glcmds);
	header->ofs_end       = LittleLong(header->ofs_end);

	/* stuff */
	model.total_skins = header->num_skins;
	model.num_skins = header->num_skins;
	model.skininfo = (skininfo_t*)mem_alloc(pool, sizeof(skininfo_t) * model.num_skins);

	for (i = 0, skininfo = model.skininfo; i < model.num_skins; i++, skininfo++)
	{
		const md2_skin_t *md2skin = (const md2_skin_t*)(f + header->ofs_skins) + i;

		skininfo->frametime = 0.1f;
		skininfo->num_skins = 1;
		skininfo->skins = (singleskin_t*)mem_alloc(pool, sizeof(skininfo_t));
		skininfo->skins[0].name = mem_copystring(pool, md2skin->name);
		skininfo->skins[0].offset = i;
	}

	model.total_frames = header->num_frames;
	model.num_frames = header->num_frames;
	model.frameinfo = (frameinfo_t*)mem_alloc(pool, sizeof(frameinfo_t) * model.num_frames);

	for (i = 0, frameinfo = model.frameinfo; i < model.num_frames; i++, frameinfo++)
	{
		const qwalk_daliasframe_t *md2frame = (const qwalk_daliasframe_t*)(f + header->ofs_frames + i * header->framesize);

		frameinfo->frametime = 0.1f;
		frameinfo->num_frames = 1;
		frameinfo->frames = (singleframe_t*)mem_alloc(pool, sizeof(singleframe_t));
		frameinfo->frames[0].name = mem_copystring(pool, md2frame->name);
		frameinfo->frames[0].offset = i;
	}

	model.num_meshes = 1;
	model.meshes = (mesh_t*)mem_alloc(pool, sizeof(mesh_t));

	model.num_tags = 0;
	model.tags = NULL;

	model.flags = 0;
	model.synctype = 0;
	model.offsets[0] = 0.0f;
	model.offsets[1] = 0.0f;
	model.offsets[2] = 0.0f;

	mesh = &model.meshes[0];
	mesh_initialize(&model, mesh);

	mesh->name = mem_copystring(pool, "md2mesh");

	/* read skins */
	mesh->skins = (meshskin_t*)mem_alloc(pool, sizeof(meshskin_t) * model.num_skins);
	for (i = 0; i < model.num_skins; i++)
	{
		const md2_skin_t *md2skin = (const md2_skin_t*)(f + header->ofs_skins) + i;
		char *error;
		image_rgba_t *image;

		for (j = 0; j < SKIN_NUMTYPES; j++)
			mesh->skins[i].components[j] = NULL;

		/* try to load the image file mentioned in the md2 */
		/* if any of the skins fail to load, they will be left as null */
		image = image_load_from_file(pool, md2skin->name, &error);
		if (!image)
		{
			/* this is a warning. FIXME - return warnings too, don't print them here */
			printf("md2: failed to load image \"%s\": %s\n", md2skin->name, error);
			qfree(error);
			continue;
		}

		mesh->skins[i].components[SKIN_DIFFUSE] = image;
	}

	/* read triangles */
	mesh->num_tris = header->num_tris;
	mesh->triangle3i = (int*)mem_alloc(pool, sizeof(int) * mesh->num_tris * 3);

	mesh->num_xyz = 0;
	meshverts = (md2_meshvert_t*)mem_alloc(pool, sizeof(md2_meshvert_t) * mesh->num_tris * 3);
	for (i = 0; i < mesh->num_tris; i++)
	{
		const dtriangle_t *dtriangle = (const dtriangle_t*)(f + header->ofs_tris) + i;

		for (j = 0; j < 3; j++)
		{
			int vertnum;
			unsigned short xyz = LittleShort(dtriangle->index_xyz[j]);
			unsigned short st = LittleShort(dtriangle->index_st[j]);

			for (vertnum = 0; vertnum < mesh->num_xyz; vertnum++)
			{
				/* see if this xyz+st combination exists in the buffer yet */
				md2_meshvert_t *mv = &meshverts[vertnum];

				if (mv->vertex == xyz && mv->texcoord == st)
					break;
			}
			if (vertnum == mesh->num_xyz)
			{
				/* it isn't in the list yet, add it */
				meshverts[vertnum].vertex = xyz;
				meshverts[vertnum].texcoord = st;
				mesh->num_xyz++;
			}

			mesh->triangle3i[i * 3 + j] = vertnum; /* (clockwise winding) */
		}
	}

	/* read texcoords */
	mesh->texcoord2f = (float*)mem_alloc(pool, sizeof(float) * mesh->num_xyz * 2);
	iwidth = 1.0f / header->skinwidth;
	iheight = 1.0f / header->skinheight;
	for (i = 0; i < mesh->num_xyz; i++)
	{
		const dstvert_t *dstvert = (const dstvert_t*)(f + header->ofs_st) + meshverts[i].texcoord;
		mesh->texcoord2f[i*2+0] = (LittleFloat(dstvert->s) + 0.5f) * iwidth;
		mesh->texcoord2f[i*2+1] = (LittleFloat(dstvert->t) + 0.5f) * iheight;
	}

	/* read frames */
	v = mesh->vertex3f = (float*)mem_alloc(pool, model.num_frames * sizeof(float) * mesh->num_xyz * 3);
	n = mesh->normal3f = (float*)mem_alloc(pool, model.num_frames * sizeof(float) * mesh->num_xyz * 3);
	for (i = 0; i < model.num_frames; i++)
	{
		const qwalk_daliasframe_t *frame = (const qwalk_daliasframe_t*)(f + header->ofs_frames + i * header->framesize);
		const dtrivertx_t *vtxbase = (const dtrivertx_t*)(frame + 1);
		float scale[3], translate[3];

		for (j = 0; j < 3; j++)
		{
			scale[j] = LittleFloat(frame->scale[j]);
			translate[j] = LittleFloat(frame->translate[j]);
		}

		for (j = 0; j < mesh->num_xyz; j++, v += 3, n += 3)
		{
			const dtrivertx_t *vtx = vtxbase + meshverts[j].vertex;
			v[0] = translate[0] + scale[0] * vtx->v[0];
			v[1] = translate[1] + scale[1] * vtx->v[1];
			v[2] = translate[2] + scale[2] * vtx->v[2];

			n[0] = anorms[vtx->lightnormalindex][0];
			n[1] = anorms[vtx->lightnormalindex][1];
			n[2] = anorms[vtx->lightnormalindex][2];
		}
	}

	mem_free(meshverts);

	mem_merge_pool(pool);

	*out_model = model;
	return true;
}

static char *md2_create_skin_filename(const char *skinname)
{
	char temp[1024];
	char *c;

/* in case skinname is already a file path, strip path and extension (so "models/something/skin.pcx" becomes "skin") */
	if ((c = strrchr(skinname, '/')))
		strcpy(temp, c + 1);
	else
		strcpy(temp, skinname);

	if ((c = strchr(temp, '.')))
		*c = '\0';

	if (g_skinpath && g_skinpath[0])
		return msprintf("%s/%s.pcx", g_skinpath, temp);
	else
		return msprintf("%s.pcx", temp);
}

typedef struct md2_data_s
{
	dstvert_t *texcoords; /* [numtexcoords] */
	int numtexcoords; /* <= mesh->num_xyz */
	int *texcoord_lookup; /* [mesh->num_xyz] */

	qwalk_daliasframe_t *frames; /* [model->num_frames] */

	dtrivertx_t *original_vertices; /* [mesh->num_xyz * model->num_frames] */

	int *vertices; /* [numvertices] ; index into original_vertices */
	int numvertices;
	int *vertex_lookup; /* [mesh->num_xyz] index into vertices */
} md2_data_t;

static md2_data_t *md2_process_vertices(const model_t *model, const mesh_t *mesh, int skinwidth, int skinheight)
{
	md2_data_t *data;
	int i, j, k;

	data = (md2_data_t*)qmalloc(sizeof(md2_data_t));

	/* convert texcoords to integer and combine duplicates */
	data->texcoords = (dstvert_t*)qmalloc(sizeof(dstvert_t) * mesh->num_xyz);
	data->numtexcoords = 0;
	data->texcoord_lookup = (int*)qmalloc(sizeof(int) * mesh->num_xyz);

	for (i = 0; i < mesh->num_xyz; i++)
	{
		dstvert_t md2texcoord;

		md2texcoord.s = (int)(mesh->texcoord2f[i*2+0] * skinwidth);
		md2texcoord.t = (int)(mesh->texcoord2f[i*2+1] * skinheight);

		for (j = 0; j < data->numtexcoords; j++)
			if (md2texcoord.s == data->texcoords[j].s && md2texcoord.t == data->texcoords[j].t)
				break;
		if (j == data->numtexcoords)
			data->texcoords[data->numtexcoords++] = md2texcoord;
		data->texcoord_lookup[i] = j;
	}

	/* compress vertices */
	data->frames = (qwalk_daliasframe_t*)qmalloc(sizeof(qwalk_daliasframe_t) * model->num_frames);
	data->original_vertices = (dtrivertx_t*)qmalloc(sizeof(dtrivertx_t) * mesh->num_xyz * model->num_frames);

	for (i = 0; i < model->num_frames; i++)
	{
		qwalk_daliasframe_t *md2frame = &data->frames[i];
		const float *v, *n;
		float mins[3], maxs[3], iscale[3];

		/* calculate bounds of frame */
		VectorClear(mins);
		VectorClear(maxs);
		v = mesh->vertex3f + model->frameinfo[i].frames[0].offset * mesh->num_xyz * 3;
		for (j = 0; j < mesh->num_xyz; j++, v += 3)
		{
			for (k = 0; k < 3; k++)
			{
				mins[k] = (j == 0) ? v[k] : min(mins[k], v[k]);
				maxs[k] = (j == 0) ? v[k] : max(maxs[k], v[k]);
			}
		}

		for (j = 0; j < 3; j++)
		{
			md2frame->scale[j] = (maxs[j] - mins[j]) * (1.0f / 255.0f);
			md2frame->translate[j] = mins[j];

			iscale[j] = md2frame->scale[j] ? (1.0f / md2frame->scale[j]) : 0.0f;
		}

		/* compress vertices */
		v = mesh->vertex3f + model->frameinfo[i].frames[0].offset * mesh->num_xyz * 3;
		n = mesh->normal3f + model->frameinfo[i].frames[0].offset * mesh->num_xyz * 3;
		for (j = 0; j < mesh->num_xyz; j++, v += 3, n += 3)
		{
			dtrivertx_t *md2vertex = &data->original_vertices[j * model->num_frames + i];

			for (k = 0; k < 3; k++)
			{
				float pos = (v[k] - md2frame->translate[k]) * iscale[k];

				pos = (float)floor(pos + 0.5f);
				pos = bound(0.0f, pos, 255.0f);

				md2vertex->v[k] = (unsigned char)pos;
			}

			md2vertex->lightnormalindex = compress_normal(n);
		}
	}

	/* combine duplicate vertices */
	data->vertices = (int*)qmalloc(sizeof(int) * mesh->num_xyz);
	data->numvertices = 0;
	data->vertex_lookup = (int*)qmalloc(sizeof(int) * mesh->num_xyz);

	for (i = 0; i < mesh->num_xyz; i++)
	{
		const dtrivertx_t *v1 = data->original_vertices + i * model->num_frames;

		/* see if a vertex at this position already exists */
		for (j = 0; j < data->numvertices; j++)
		{
			const dtrivertx_t *v2 = data->original_vertices + data->vertices[j] * model->num_frames;

			for (k = 0; k < model->num_frames; k++)
				if (v1[k].v[0] != v2[k].v[0] || v1[k].v[1] != v2[k].v[1] || v1[k].v[2] != v2[k].v[2] || v1[k].lightnormalindex != v2[k].lightnormalindex)
					break;
			if (k == model->num_frames)
				break;
		}
		if (j == data->numvertices)
		{
			/* no match, add this one */
			data->vertices[data->numvertices++] = i;
		}
		data->vertex_lookup[i] = j;
	}

	return data;
}

static void md2_free_data(md2_data_t *data)
{
	qfree(data->texcoords);
	qfree(data->texcoord_lookup);
	qfree(data->frames);
	qfree(data->original_vertices);
	qfree(data->vertices);
	qfree(data->vertex_lookup);
	qfree(data);
}

/* md2 glcmds generation, taken from quake2 source (models.c) */

static int commands[16384];
static int numcommands;
static int *used;

static int strip_xyz[128];
static int strip_st[128];
static int strip_tris[128];
static int stripcount;

static dtriangle_t *triangles;
static int num_tris;

static int md2_strip_length(int starttri, int startv)
{
	int m1, m2;
	int st1, st2;
	int j;
	dtriangle_t *last, *check;
	int k;

	used[starttri] = 2;

	last = &triangles[starttri];

	strip_xyz[0] = last->index_xyz[(startv  )%3];
	strip_xyz[1] = last->index_xyz[(startv+1)%3];
	strip_xyz[2] = last->index_xyz[(startv+2)%3];
	strip_st[0] = last->index_st[(startv  )%3];
	strip_st[1] = last->index_st[(startv+1)%3];
	strip_st[2] = last->index_st[(startv+2)%3];

	strip_tris[0] = starttri;
	stripcount = 1;

	m1 = last->index_xyz[(startv+2)%3];
	st1 = last->index_st[(startv+2)%3];
	m2 = last->index_xyz[(startv+1)%3];
	st2 = last->index_st[(startv+1)%3];

/* look for a matching triangle */
nexttri:
	for (j = starttri + 1, check = &triangles[starttri + 1]; j < num_tris; j++, check++)
	{
		for (k = 0; k < 3; k++)
		{
			if (check->index_xyz[k] != m1)
				continue;
			if (check->index_st[k] != st1)
				continue;
			if (check->index_xyz[(k+1)%3] != m2)
				continue;
			if (check->index_st[(k+1)%3] != st2)
				continue;

		/* this is the next part of the fan */

		/* if we can't use this triangle, this tristrip is done */
			if (used[j])
				goto done;

		/* the new edge */
			if (stripcount & 1)
			{
				m2 = check->index_xyz[(k+2)%3];
				st2 = check->index_st[(k+2)%3];
			}
			else
			{
				m1 = check->index_xyz[(k+2)%3];
				st1 = check->index_st[(k+2)%3];
			}

			strip_xyz[stripcount+2] = check->index_xyz[(k+2)%3];
			strip_st[stripcount+2] = check->index_st[(k+2)%3];
			strip_tris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}
done:

/* clear the temp used flags */
	for (j = starttri + 1; j < num_tris; j++)
		if (used[j] == 2)
			used[j] = 0;

	return stripcount;
}

static int md2_fan_length(int starttri, int startv)
{
	int m1, m2;
	int st1, st2;
	int j;
	dtriangle_t *last, *check;
	int k;

	used[starttri] = 2;

	last = &triangles[starttri];

	strip_xyz[0] = last->index_xyz[(startv  )%3];
	strip_xyz[1] = last->index_xyz[(startv+1)%3];
	strip_xyz[2] = last->index_xyz[(startv+2)%3];
	strip_st[0] = last->index_st[(startv  )%3];
	strip_st[1] = last->index_st[(startv+1)%3];
	strip_st[2] = last->index_st[(startv+2)%3];

	strip_tris[0] = starttri;
	stripcount = 1;

	m1 = last->index_xyz[(startv+0)%3];
	st1 = last->index_st[(startv+0)%3];
	m2 = last->index_xyz[(startv+2)%3];
	st2 = last->index_st[(startv+2)%3];


/* look for a matching triangle */
nexttri:
	for (j = starttri + 1, check = &triangles[starttri + 1]; j < num_tris; j++, check++)
	{
		for (k = 0; k < 3; k++)
		{
			if (check->index_xyz[k] != m1)
				continue;
			if (check->index_st[k] != st1)
				continue;
			if (check->index_xyz[(k+1)%3] != m2)
				continue;
			if (check->index_st[(k+1)%3] != st2)
				continue;

		/* this is the next part of the fan */

		/* if we can't use this triangle, this tristrip is done */
			if (used[j])
				goto done;

		/* the new edge */
			m2 = check->index_xyz[(k+2)%3];
			st2 = check->index_st[(k+2)%3];

			strip_xyz[stripcount+2] = m2;
			strip_st[stripcount+2] = st2;
			strip_tris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}
done:

/* clear the temp used flags */
	for (j = starttri + 1; j < num_tris; j++)
		if (used[j] == 2)
			used[j] = 0;

	return stripcount;
}

static void md2_build_glcmds(const dstvert_t *texcoords, int skinwidth, int skinheight)
{
	int i, j;
	int startv;
	int len, bestlen, besttype = -1;
	int best_xyz[1024];
	int best_st[1024];
	int best_tris[1024];
	int type;

	numcommands = 0;
	used = (int*)qmalloc(sizeof(int) * num_tris);
	memset(used, 0, sizeof(int) * num_tris);
	for (i = 0; i < num_tris; i++)
	{
		/* pick an unused triangle and start the trifan */
		if (used[i])
			continue;

		bestlen = 0;
		for (type = 0; type < 2; type++)
		{
			for (startv = 0; startv < 3; startv++)
			{
				if (type == 1)
					len = md2_strip_length(i, startv);
				else
					len = md2_fan_length(i, startv);

				if (len > bestlen)
				{
					besttype = type;
					bestlen = len;
					for (j = 0; j < bestlen + 2; j++)
					{
						best_st[j] = strip_st[j];
						best_xyz[j] = strip_xyz[j];
					}
					for (j = 0; j < bestlen; j++)
						best_tris[j] = strip_tris[j];
				}
			}
		}

		/* mark the tris on the best strip/fan as used */
		for (j = 0; j < bestlen; j++)
			used[best_tris[j]] = 1;

		if (besttype == 1)
			commands[numcommands++] = (bestlen+2);
		else
			commands[numcommands++] = -(bestlen+2);

		for (j = 0; j < bestlen + 2; j++)
		{
			union { float f; int i; } u;
			u.f = (texcoords[best_st[j]].s + 0.5f) / skinwidth;
			commands[numcommands++] = u.i;
			u.f = (texcoords[best_st[j]].t + 0.5f) / skinheight;
			commands[numcommands++] = u.i;
			commands[numcommands++] = best_xyz[j];
		}
	}

	commands[numcommands++] = 0; /* end of list marker */

	qfree(used);
}

bool_t model_md2_save(const model_t *orig_model, xbuf_t *xbuf, char **out_error)
{
	char *error;
	int skinwidth, skinheight;
	char **skinfilenames;
	const skininfo_t *skininfo;
	model_t *model;
	const mesh_t *mesh;
	md2_data_t *md2data;
	dmdl_t *header;
	dtriangle_t *dtriangles;
	int i, j, k;

	model = model_merge_meshes(orig_model);

	mesh = &model->meshes[0];

	skinwidth = (texwidth != -1) ? texwidth : 0;
	skinheight = (texheight != -1) ? texheight : 0;
	for (i = 0, skininfo = model->skininfo; i < model->num_skins; i++, skininfo++)
	{
		for (j = 0; j < skininfo->num_skins; j++)
		{
			int offset = skininfo->skins[j].offset;

			if (!mesh->skins[offset].components[SKIN_DIFFUSE])
			{
				if (out_error)
					*out_error = msprintf("Model has missing skin.");
				model_free(model);
				return false;
			}

			if (skinwidth && skinheight && (skinwidth != mesh->skins[offset].components[SKIN_DIFFUSE]->width || skinheight != mesh->skins[offset].components[SKIN_DIFFUSE]->height))
			{
				if (out_error)
					*out_error = msprintf("Model has skins of different sizes. Use -texwidth and -texheight to resize all images to the same size");
				model_free(model);
				return false;
			}
			skinwidth = mesh->skins[offset].components[SKIN_DIFFUSE]->width;
			skinheight = mesh->skins[offset].components[SKIN_DIFFUSE]->height;

			/* if fullbright texture is a different size, resample it to match the diffuse texture */
			if (mesh->skins[offset].components[SKIN_FULLBRIGHT] && (mesh->skins[offset].components[SKIN_FULLBRIGHT]->width != skinwidth || mesh->skins[offset].components[SKIN_FULLBRIGHT]->height != skinheight))
			{
				image_rgba_t *image = image_resize(mem_globalpool, mesh->skins[offset].components[SKIN_FULLBRIGHT], skinwidth, skinheight);
				image_free(&mesh->skins[offset].components[SKIN_FULLBRIGHT]);
				mesh->skins[offset].components[SKIN_FULLBRIGHT] = image;
			}
		}
	}

	if (!skinwidth || !skinheight)
	{
		if (out_error)
			*out_error = msprintf("Model has no skin. Use -texwidth and -texheight to set the skin dimensions, or -tex to import a skin");
		model_free(model);
		return false;
	}

	/* create 8-bit skins and save them to PCX files */
	skinfilenames = (char**)qmalloc(sizeof(char*) * model->num_skins);

	for (i = 0, skininfo = model->skininfo; i < model->num_skins; i++, skininfo++)
	{
		image_paletted_t *pimage;

		int offset = skininfo->skins[0].offset; /* skingroups not supported, just take the first skin from the group */

		skinfilenames[i] = md2_create_skin_filename(skininfo->skins[0].name);

		pimage = image_palettize(mem_globalpool, &palette_quake2, mesh->skins[offset].components[SKIN_DIFFUSE], mesh->skins[offset].components[SKIN_FULLBRIGHT]);

		/* FIXME - this shouldn't be a fatal error */
		if (!image_paletted_save(skinfilenames[i], pimage, &error))
		{
			if (out_error)
				*out_error = msprintf("Failed to write %s: %s", skinfilenames[i], error);
			qfree(error);
			qfree(pimage);
			for (j = 0; j < i; j++)
				qfree(skinfilenames[j]);
			qfree(skinfilenames);
			model_free(model);
			return false;
		}

		qfree(pimage);
	}

	/* optimize vertices for md2 format */
	md2data = md2_process_vertices(model, mesh, skinwidth, skinheight);

	/* write header */
	header = (dmdl_t*)xbuf_reserve_data(xbuf, sizeof(dmdl_t));

	header->ident         = LittleLong(IDALIASHEADER);
	header->version       = LittleLong(ALIAS_VERSION);
	header->skinwidth     = LittleLong(skinwidth);
	header->skinheight    = LittleLong(skinheight);
	header->framesize     = LittleLong(sizeof(qwalk_daliasframe_t) + sizeof(dtrivertx_t) * md2data->numvertices);
	header->num_skins     = LittleLong(model->num_skins);
	header->num_xyz       = LittleLong(md2data->numvertices);
	header->num_st        = LittleLong(md2data->numtexcoords);
	header->num_tris      = LittleLong(mesh->num_tris);
	header->num_glcmds    = 0; /* filled in later */
	header->num_frames    = LittleLong(model->num_frames);
	header->ofs_skins     = 0;
	header->ofs_st        = 0;
	header->ofs_tris      = 0;
	header->ofs_frames    = 0;
	header->ofs_glcmds    = 0;
	header->ofs_end       = 0;

	/* write skins */
	header->ofs_skins = LittleLong(xbuf_get_bytes_written(xbuf));

	for (i = 0; i < model->num_skins; i++)
	{
		md2_skin_t md2skin;

		strlcpy(md2skin.name, skinfilenames[i], sizeof(md2skin.name));

		xbuf_write_data(xbuf, sizeof(md2_skin_t), &md2skin);
	}

	/* write texcoords */
	for (i = 0; i < md2data->numtexcoords; i++)
	{
		md2data->texcoords[i].s = LittleShort(md2data->texcoords[i].s);
		md2data->texcoords[i].t = LittleShort(md2data->texcoords[i].t);
	}

	header->ofs_st = LittleLong(xbuf_get_bytes_written(xbuf));

	xbuf_write_data(xbuf, sizeof(dstvert_t) * md2data->numtexcoords, md2data->texcoords);

	/* write triangles */
	dtriangles = (dtriangle_t*)qmalloc(sizeof(dtriangle_t) * mesh->num_tris);
	for (i = 0; i < mesh->num_tris; i++)
	{
		for (j = 0; j < 3; j++)
		{
			dtriangles[i].index_xyz[j] = LittleShort(md2data->vertex_lookup[mesh->triangle3i[i*3+j]]);
			dtriangles[i].index_st[j] = LittleShort(md2data->texcoord_lookup[mesh->triangle3i[i*3+j]]);
		}
	}

	header->ofs_tris = LittleLong(xbuf_get_bytes_written(xbuf));

	xbuf_write_data(xbuf, sizeof(dtriangle_t) * mesh->num_tris, dtriangles);

	/* write frames */
	header->ofs_frames = LittleLong(xbuf_get_bytes_written(xbuf));

	for (i = 0; i < model->num_frames; i++)
	{
		qwalk_daliasframe_t md2frame;

		for (j = 0; j < 3; j++)
		{
			md2frame.scale[j] = LittleFloat(md2data->frames[i].scale[j]);
			md2frame.translate[j] = LittleFloat(md2data->frames[i].translate[j]);
		}

		strlcpy(md2frame.name, model->frameinfo[i].frames[0].name, sizeof(md2frame.name));

		xbuf_write_data(xbuf, sizeof(qwalk_daliasframe_t), &md2frame);

		/* write vertices */
		for (j = 0; j < md2data->numvertices; j++)
		{
			const dtrivertx_t *md2vertex = md2data->original_vertices + md2data->vertices[j] * model->num_frames + i;
			dtrivertx_t vtx;

			for (k = 0; k < 3; k++)
				vtx.v[k] = md2vertex->v[k];

			vtx.lightnormalindex = md2vertex->lightnormalindex;

			xbuf_write_data(xbuf, sizeof(dtrivertx_t), &vtx);
		}
	}

	/* write glcmds */
	triangles = dtriangles;
	num_tris = mesh->num_tris;

	md2_build_glcmds(md2data->texcoords, skinwidth, skinheight);

	header->num_glcmds = LittleLong(numcommands);
	header->ofs_glcmds = LittleLong(xbuf_get_bytes_written(xbuf));

	for (i = 0; i < numcommands; i++)
		commands[i] = LittleLong(commands[i]);
	xbuf_write_data(xbuf, sizeof(int) * numcommands, commands);

	/* write end */
	header->ofs_end = LittleLong(xbuf_get_bytes_written(xbuf));

	/* done */
	qfree(dtriangles);
	md2_free_data(md2data);

	for (i = 0; i < model->num_skins; i++)
		qfree(skinfilenames[i]);
	qfree(skinfilenames);

	model_free(model);
	return true;
}
