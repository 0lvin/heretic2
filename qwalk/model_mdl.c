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
#include <stdio.h> /* FIXME - don't print to console in this file */
#include <string.h>

#include "global.h"
#include "model.h"
#include "palettes.h"

extern const float anorms[162][3];
int compress_normal(const float *normal);

/* mdl_texcoord_t::onseam */
#define ALIAS_ONSEAM 0x0020

/* mdl_itriangle_t::facesfront */
/*#define DT_FACES_FRONT 0x0010*/

/* mdl_header_t::synctype */
#define ST_SYNC 0
#define ST_RAND 1

typedef enum aliasframetype_e { ALIAS_SINGLE = 0, ALIAS_GROUP } aliasframetype_t;
typedef enum aliasskintype_e { ALIAS_SKIN_SINGLE = 0, ALIAS_SKIN_GROUP } aliasskintype_t;

typedef struct daliasframetype_s
{
	aliasframetype_t type;
} daliasframetype_t;

typedef struct qwalk_daliasframe_s
{
	dtrivertx_t bboxmin; /* lightnormal isn't used */
	dtrivertx_t bboxmax; /* lightnormal isn't used */
	char name[16];
} qwalk_daliasframe_t;

typedef struct daliasgroup_s
{
	int numframes;
	dtrivertx_t bboxmin; /* lightnormal isn't used */
	dtrivertx_t bboxmax; /* lightnormal isn't used */
} daliasgroup_t;

typedef struct daliasinterval_s
{
	float interval;
} daliasinterval_t;

typedef struct daliasskintype_s
{
	aliasskintype_t type;
} daliasskintype_t;

typedef struct daliasskingroup_s
{
	int numskins;
} daliasskingroup_t;

typedef struct daliasskininterval_s
{
	float interval;
} daliasskininterval_t;

bool_t model_mdl_load(void *filedata, size_t filesize, model_t *out_model, char **out_error)
{
	typedef struct mdl_meshvert_s
	{
		int vertex;
		int s, t, back;
	} mdl_meshvert_t;

	unsigned char *f = (unsigned char*)filedata;
	mem_pool_t *pool;
	const unsigned char *tf;
	mdl_header_t *header;
	int i, j, k, offset;
	int total_skins, total_frames;
	mdl_texcoord_t *stverts;
	mdl_triangle_t *dtriangles;
	dtrivertx_t **framevertstart;
	unsigned char **skintexstart;
	skininfo_t *skininfo;
	frameinfo_t *frameinfo;
	model_t model;
	mesh_t *mesh;
	mdl_meshvert_t *meshverts;
	float iwidth, iheight;

	header = (mdl_header_t*)f;

	/* validate format */
	if (LittleLong(header->ident) != IDMDLHEADER)
		return (void)(out_error && (*out_error = msprintf("wrong format (not IDPO)"))), false;
	if (LittleLong(header->version) != MDL_VERSION)
		return (void)(out_error && (*out_error = msprintf("wrong format (version not 6)"))), false;

	pool = mem_create_pool();

	/* byteswap header */
	header->version    = LittleLong(header->version);
	header->scale[0]   = LittleFloat(header->scale[0]);
	header->scale[1]   = LittleFloat(header->scale[1]);
	header->scale[2]   = LittleFloat(header->scale[2]);
	header->translate[0]  = LittleFloat(header->translate[0]);
	header->translate[1]  = LittleFloat(header->translate[1]);
	header->translate[2]  = LittleFloat(header->translate[2]);
	header->boundingradius     = LittleFloat(header->boundingradius);
	header->eyeposition[0] = LittleFloat(header->eyeposition[0]);
	header->eyeposition[1] = LittleFloat(header->eyeposition[1]);
	header->eyeposition[2] = LittleFloat(header->eyeposition[2]);
	header->num_skins   = LittleLong(header->num_skins);
	header->skinwidth  = LittleLong(header->skinwidth);
	header->skinheight = LittleLong(header->skinheight);
	header->num_xyz   = LittleLong(header->num_xyz);
	header->num_tris    = LittleLong(header->num_tris);
	header->num_frames  = LittleLong(header->num_frames);
	header->synctype   = LittleLong(header->synctype);
	header->flags      = LittleLong(header->flags);
	header->size       = LittleFloat(header->size);

	f += sizeof(mdl_header_t);

/* count total number of skins */
	tf = f;

	total_skins = 0;

	for (i = 0; i < header->num_skins; i++)
	{
		int num_skins;
		const daliasskintype_t *skintype = (const daliasskintype_t*)tf;
		tf += sizeof(daliasskintype_t);

		if (LittleLong(skintype->type) == ALIAS_SKIN_SINGLE)
			num_skins = 1;
		else
		{
			const daliasskingroup_t *group = (const daliasskingroup_t*)tf;
			tf += sizeof(daliasskingroup_t);

			num_skins = LittleLong(group->numskins);

			tf += num_skins * sizeof(daliasskininterval_t);
		}

		tf += num_skins * header->skinwidth * header->skinheight;

		total_skins += num_skins;
	}

/* read skins */
	model.num_skins = header->num_skins;
	model.skininfo = (skininfo_t*)mem_alloc(pool, sizeof(skininfo_t) * model.num_skins);

	model.total_skins = total_skins;

	offset = 0;

	skintexstart = (unsigned char**)mem_alloc(pool, sizeof(unsigned char*) * model.total_skins);
	for (i = 0, skininfo = model.skininfo; i < header->num_skins; i++, skininfo++)
	{
		daliasskintype_t *skintype = (daliasskintype_t*)f;
		f += sizeof(daliasskintype_t);

		skintype->type = LittleLong(skintype->type);

		if (skintype->type == ALIAS_SKIN_SINGLE)
		{
		/* ordinary skin */
			skininfo->frametime = 0.1f;
			skininfo->num_skins = 1;
		}
		else
		{
		/* skin group */
			daliasskingroup_t *group;
			daliasskininterval_t *intervals;

			group = (daliasskingroup_t*)f;
			f += sizeof(daliasskingroup_t);

			intervals = (daliasskininterval_t*)f;
			f += LittleLong(group->numskins) * sizeof(daliasskininterval_t);

		/* load into model */
			skininfo->frametime = max(0.01f, LittleFloat(intervals[0].interval));
			skininfo->num_skins = LittleLong(group->numskins);
		}

		skininfo->skins = (singleskin_t*)mem_alloc(pool, sizeof(singleskin_t) * skininfo->num_skins);

		for (j = 0; j < skininfo->num_skins; j++)
		{
			skininfo->skins[j].name = mem_sprintf(pool, "skin%d", offset);
			skininfo->skins[j].offset = offset;

			skintexstart[offset] = f;
			f += header->skinwidth * header->skinheight;

			offset++;
		}
	}

	/* read texcoords */
	stverts = (mdl_texcoord_t*)f;
	f += sizeof(mdl_texcoord_t) * header->num_xyz;

	for (i = 0; i < header->num_xyz; i++)
	{
		stverts[i].onseam = LittleLong(stverts[i].onseam);
		stverts[i].s = LittleLong(stverts[i].s);
		stverts[i].t = LittleLong(stverts[i].t);
	}

	/* read triangles */
	dtriangles = (mdl_triangle_t*)f;
	f += sizeof(mdl_triangle_t) * header->num_tris;

	for (i = 0; i < header->num_xyz; i++)
	{
		dtriangles[i].facesfront = LittleLong(dtriangles[i].facesfront);
		dtriangles[i].vertex[0] = LittleLong(dtriangles[i].vertex[0]);
		dtriangles[i].vertex[1] = LittleLong(dtriangles[i].vertex[1]);
		dtriangles[i].vertex[2] = LittleLong(dtriangles[i].vertex[2]);
	}

	/* count total number of frames */
	tf = f;

	total_frames = 0;

	for (i = 0; i < header->num_frames; i++)
	{
		int num_frames;
		const daliasframetype_t *frametype = (const daliasframetype_t*)tf;
		tf += sizeof(daliasframetype_t);

		if (LittleLong(frametype->type) == ALIAS_SINGLE)
			num_frames = 1;
		else
		{
			const daliasgroup_t *group = (const daliasgroup_t*)tf;
			tf += sizeof(daliasgroup_t);

			printf("->%d\n", LittleLong(group->numframes));
			num_frames = LittleLong(group->numframes);

			tf += num_frames * sizeof(daliasinterval_t);
		}

		tf += num_frames * (sizeof(qwalk_daliasframe_t) + sizeof(dtrivertx_t) * header->num_xyz);

		total_frames += num_frames;
	}
	printf("%d === %d\n", total_frames, header->num_frames);

	/* read frames */
	model.num_frames = header->num_frames;
	model.frameinfo = (frameinfo_t*)mem_alloc(pool, sizeof(frameinfo_t) * model.num_frames);

	model.total_frames = total_frames;

	offset = 0;

	framevertstart = (dtrivertx_t**)mem_alloc(pool, sizeof(dtrivertx_t*) * model.total_frames);
	for (i = 0, frameinfo = model.frameinfo; i < header->num_frames; i++, frameinfo++)
	{
		daliasframetype_t *frametype = (daliasframetype_t*)f;
		f += sizeof(daliasframetype_t);

		frametype->type = LittleLong(frametype->type);

		unsigned char * toskip = f;

		if (frametype->type == ALIAS_SINGLE)
		{
		/* ordinary frame */
			frameinfo->frametime = 0.1f;
			frameinfo->num_frames = 1;
		}
		else
		{
		/* frame group */
			daliasgroup_t *group;
			daliasinterval_t *intervals;

			group = (daliasgroup_t*)f;
			f += sizeof(daliasgroup_t);

			printf("%d has %d (%d)\n", i, frameinfo->num_frames, f - toskip);

			intervals = (daliasinterval_t*)f;
			f += LittleLong(group->numframes) * sizeof(daliasinterval_t);

			printf("%d has %d (%d)\n", i, frameinfo->num_frames, f - toskip);

		/* load into model */
			frameinfo->frametime = max(0.01f, LittleFloat(intervals[0].interval));
			frameinfo->num_frames = LittleLong(group->numframes);
		}

		frameinfo->frames = (singleframe_t*)mem_alloc(pool, sizeof(singleframe_t) * frameinfo->num_frames);

		printf("%d has %d (%d)\n", i, frameinfo->num_frames, f - toskip);

		for (j = 0; j < frameinfo->num_frames; j++)
		{
			qwalk_daliasframe_t *sframe = (qwalk_daliasframe_t*)f;
			f += sizeof(qwalk_daliasframe_t);

			frameinfo->frames[j].offset = offset;
			frameinfo->frames[j].name = mem_copystring(pool, sframe->name);
			printf("name %d: %s\n", j, frameinfo->frames[j].name);

			framevertstart[offset] = (dtrivertx_t*)f;
			f += header->num_xyz * sizeof(dtrivertx_t);

			offset++;
		}
	}

	model.num_meshes = 1;
	model.meshes = (mesh_t*)mem_alloc(pool, sizeof(mesh_t));

	model.num_tags = 0;
	model.tags = NULL;

	model.flags = header->flags;
	model.synctype = header->synctype;
	model.offsets[0] = header->eyeposition[0];
	model.offsets[1] = header->eyeposition[1];
	model.offsets[2] = header->eyeposition[2];

	mesh = &model.meshes[0];
	mesh_initialize(&model, mesh);

	mesh->name = mem_copystring(pool, "mdlmesh");

	mesh->num_tris = header->num_tris;
	mesh->triangle3i = (int*)mem_alloc(pool, sizeof(int) * mesh->num_tris * 3);

	mesh->num_xyz = 0;
	meshverts = (mdl_meshvert_t*)mem_alloc(pool, sizeof(mdl_meshvert_t) * mesh->num_tris * 3);
	for (i = 0; i < mesh->num_tris; i++)
	{
		for (j = 0; j < 3; j++)
		{
			int vertnum;
			mdl_meshvert_t *mv;

			int xyz = dtriangles[i].vertex[j];

			int s = stverts[xyz].s;
			int t = stverts[xyz].t;
			int back = stverts[xyz].onseam && !dtriangles[i].facesfront;

			/* add the vertex if it doesn't exist, otherwise use the old one */
			for (vertnum = 0, mv = meshverts; vertnum < mesh->num_xyz; vertnum++, mv++)
				if (mv->vertex == xyz && mv->s == s && mv->t == t && mv->back == back)
					break;
			if (vertnum == mesh->num_xyz)
			{
				mv = &meshverts[mesh->num_xyz++];
				mv->vertex = xyz;
				mv->s = s;
				mv->t = t;
				mv->back = back;
			}

			mesh->triangle3i[i * 3 + j] = vertnum; /* (clockwise winding) */
		}
	}

	mesh->texcoord2f = (float*)mem_alloc(pool, sizeof(float) * mesh->num_xyz * 2);
	iwidth = 1.0f / header->skinwidth;
	iheight = 1.0f / header->skinheight;
	for (i = 0; i < mesh->num_xyz; i++)
	{
		float s = (float)meshverts[i].s;
		float t = (float)meshverts[i].t;

		if (meshverts[i].back)
			s += header->skinwidth >> 1;

		mesh->texcoord2f[i*2+0] = (s + 0.5f) * iwidth;
		mesh->texcoord2f[i*2+1] = (t + 0.5f) * iheight;
	}

	mesh->vertex3f = (float*)mem_alloc(pool, model.total_frames * sizeof(float) * mesh->num_xyz * 3);
	mesh->normal3f = (float*)mem_alloc(pool, model.total_frames * sizeof(float) * mesh->num_xyz * 3);
	for (i = 0; i < model.num_frames; i++)
	{
		for (j = 0; j < model.frameinfo[i].num_frames; j++)
		{
			int firstvertex = model.frameinfo[i].frames[j].offset * mesh->num_xyz * 3;
			float *v = mesh->vertex3f + firstvertex;
			float *n = mesh->normal3f + firstvertex;

			for (k = 0; k < mesh->num_xyz; k++, v += 3, n += 3)
			{
				const dtrivertx_t *trivertx = &framevertstart[model.frameinfo[i].frames[j].offset][meshverts[k].vertex];

				v[0] = header->translate[0] + header->scale[0] * trivertx->v[0];
				v[1] = header->translate[1] + header->scale[1] * trivertx->v[1];
				v[2] = header->translate[2] + header->scale[2] * trivertx->v[2];

				n[0] = anorms[trivertx->lightnormalindex][0];
				n[1] = anorms[trivertx->lightnormalindex][1];
				n[2] = anorms[trivertx->lightnormalindex][2];
			}
		}
	}

	mem_free(meshverts);
	mem_free(framevertstart);

	mesh->skins = (meshskin_t*)mem_alloc(pool, sizeof(meshskin_t) * model.total_skins);
	for (i = 0; i < model.total_skins; i++)
	{
		mesh->skins[i].components[SKIN_DIFFUSE]    = image_alloc(pool, header->skinwidth, header->skinheight);
		mesh->skins[i].components[SKIN_FULLBRIGHT] = image_alloc(pool, header->skinwidth, header->skinheight);

		for (j = 0; j < header->skinwidth * header->skinheight; j++)
		{
			unsigned char c = skintexstart[i][j];

			if (palette_quake.fullbright_flags[c >> 5] & (1U << (c & 31)))
			{
			/* fullbright */
				mesh->skins[i].components[SKIN_DIFFUSE]->pixels[j*4+0] = 0;
				mesh->skins[i].components[SKIN_DIFFUSE]->pixels[j*4+1] = 0;
				mesh->skins[i].components[SKIN_DIFFUSE]->pixels[j*4+2] = 0;
				mesh->skins[i].components[SKIN_DIFFUSE]->pixels[j*4+3] = 255;

				mesh->skins[i].components[SKIN_FULLBRIGHT]->pixels[j*4+0] = palette_quake.rgb[c*3+0];
				mesh->skins[i].components[SKIN_FULLBRIGHT]->pixels[j*4+1] = palette_quake.rgb[c*3+1];
				mesh->skins[i].components[SKIN_FULLBRIGHT]->pixels[j*4+2] = palette_quake.rgb[c*3+2];
				mesh->skins[i].components[SKIN_FULLBRIGHT]->pixels[j*4+3] = 255;
			}
			else
			{
			/* normal colour */
				mesh->skins[i].components[SKIN_DIFFUSE]->pixels[j*4+0] = palette_quake.rgb[c*3+0];
				mesh->skins[i].components[SKIN_DIFFUSE]->pixels[j*4+1] = palette_quake.rgb[c*3+1];
				mesh->skins[i].components[SKIN_DIFFUSE]->pixels[j*4+2] = palette_quake.rgb[c*3+2];
				mesh->skins[i].components[SKIN_DIFFUSE]->pixels[j*4+3] = 255;

				mesh->skins[i].components[SKIN_FULLBRIGHT]->pixels[j*4+0] = 0;
				mesh->skins[i].components[SKIN_FULLBRIGHT]->pixels[j*4+1] = 0;
				mesh->skins[i].components[SKIN_FULLBRIGHT]->pixels[j*4+2] = 0;
				mesh->skins[i].components[SKIN_FULLBRIGHT]->pixels[j*4+3] = 255;
			}
		}
	}

	mem_free(skintexstart);

	mem_merge_pool(pool);

	*out_model = model;
	return true;
}

bool_t model_mdl_save(const model_t *orig_model, xbuf_t *xbuf, char **out_error)
{
	model_t *model;
	const mesh_t *mesh;
	float mins[3], maxs[3], origin[3], scale[3], iscale[3], dist[3], totalsize;
	mdl_header_t header;
	int i, j, k;
	int skinwidth, skinheight;
	image_paletted_t **skinimages;

	model = model_merge_meshes(orig_model);

	mesh = &model->meshes[0];

	skinwidth = 0;
	skinheight = 0;
	for (i = 0; i < model->num_skins; i++)
	{
		const skininfo_t *skininfo = &model->skininfo[i];

		for (j = 0; j < skininfo->num_skins; j++)
		{
			int offset = skininfo->skins[j].offset;

			if (!mesh->skins[offset].components[SKIN_DIFFUSE])
			{
				model_free(model);
				return (void)(out_error && (*out_error = msprintf("Model has missing skin"))), false;
			}

			if (skinwidth && skinheight && (skinwidth != mesh->skins[offset].components[SKIN_DIFFUSE]->width || skinheight != mesh->skins[offset].components[SKIN_DIFFUSE]->height))
			{
				model_free(model);
				return (void)(out_error && (*out_error = msprintf("Model has skin of different sizes. Use -texwidth and -texheight to resize all images to the same size"))), false;
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
		model_free(model);
		return (void)(out_error && (*out_error = msprintf("Model has no skin. Use -tex to import a skin"))), false;
	}

/* create 8-bit textures */
	skinimages = (image_paletted_t**)qmalloc(sizeof(image_paletted_t*) * model->total_skins);

	for (i = 0; i < model->num_skins; i++)
	{
		const skininfo_t *skininfo = &model->skininfo[i];

		for (j = 0; j < skininfo->num_skins; j++)
		{
			int offset = skininfo->skins[j].offset;

			skinimages[offset] = image_palettize(mem_globalpool, &palette_quake, mesh->skins[offset].components[SKIN_DIFFUSE], mesh->skins[offset].components[SKIN_FULLBRIGHT]);
		}
	}

	/* calculate bounds */
	VectorClear(mins);
	VectorClear(maxs);
	for (i = 0; i < model->total_frames * mesh->num_xyz; i++)
	{
		const float *xyz = mesh->vertex3f + i * 3;

		for (j = 0; j < 3; j++)
		{
			mins[j] = (i == 0) ? xyz[j] : min(mins[j], xyz[j]);
			maxs[j] = (i == 0) ? xyz[j] : max(maxs[j], xyz[j]);
		}
	}

	for (i = 0; i < 3; i++)
	{
		origin[i] = mins[i];
		scale[i] = (maxs[i] - mins[i]) * (1.0f / 255.9f);
		iscale[i] = 1.0f / scale[i];

		dist[i] = (fabs(mins[i]) > fabs(maxs[i])) ? mins[i] : maxs[i];
	}

/* calculate average polygon size (used by software engines for LOD) */
	totalsize = 0.0f;
	for (i = 0; i < mesh->num_tris; i++)
	{
		const float *v0 = mesh->vertex3f+mesh->triangle3i[i*3+0]*3;
		const float *v1 = mesh->vertex3f+mesh->triangle3i[i*3+1]*3;
		const float *v2 = mesh->vertex3f+mesh->triangle3i[i*3+2]*3;
		float vtemp1[3], vtemp2[3], normal[3];

		VectorSubtract(v0, v1, vtemp1);
		VectorSubtract(v2, v1, vtemp2);
		CrossProduct(vtemp1, vtemp2, normal);

		totalsize += (float)sqrt(DotProduct(normal, normal)) * 0.5f;
	}

	/* write header */
	header.ident          = LittleLong(IDMDLHEADER);
	header.version        = LittleLong(MDL_VERSION);
	header.scale[0]       = LittleFloat(scale[0]);
	header.scale[1]       = LittleFloat(scale[1]);
	header.scale[2]       = LittleFloat(scale[2]);
	header.translate[0]   = LittleFloat(origin[0]);
	header.translate[1]   = LittleFloat(origin[1]);
	header.translate[2]   = LittleFloat(origin[2]);
	header.boundingradius = LittleFloat((float)sqrt(dist[0]*dist[0] + dist[1]*dist[1] + dist[2]*dist[2]));
	header.eyeposition[0] = LittleFloat(model->offsets[0]);
	header.eyeposition[1] = LittleFloat(model->offsets[1]);
	header.eyeposition[2] = LittleFloat(model->offsets[2]);
	header.num_skins      = LittleLong(model->num_skins);
	header.skinwidth      = LittleLong((skinwidth + 3) & ~3); /* skin width is padded up to a multiple of 4 to maintain alignment of subsequent data */
	header.skinheight     = LittleLong(skinheight);
	header.num_xyz        = LittleLong(mesh->num_xyz);
	header.num_tris       = LittleLong(mesh->num_tris);
	header.num_frames     = LittleLong(model->num_frames);
	header.synctype       = LittleLong(model->synctype);
	header.flags          = LittleLong(model->flags);
	header.size           = LittleFloat(totalsize / mesh->num_tris);

	xbuf_write_data(xbuf, sizeof(mdl_header_t), &header);

/* write skins */
	for (i = 0; i < model->num_skins; i++)
	{
		const skininfo_t *skininfo = &model->skininfo[i];

		if (skininfo->num_skins > 1)
		{
		/* skin group */
			daliasskintype_t skintype;
			daliasskingroup_t group;

			skintype.type = LittleLong(ALIAS_SKIN_GROUP);
			xbuf_write_data(xbuf, sizeof(daliasskintype_t), &skintype);

			group.numskins = LittleLong(skininfo->num_skins);
			xbuf_write_data(xbuf, sizeof(daliasskingroup_t), &group);

			for (j = 0; j < skininfo->num_skins; j++)
			{
				daliasskininterval_t interval;
				interval.interval = LittleFloat(skininfo->frametime * (j + 1));
				xbuf_write_data(xbuf, sizeof(daliasskininterval_t), &interval);
			}
		}
		else
		{
		/* single skin */
			daliasskintype_t skintype;
			skintype.type = LittleLong(ALIAS_SKIN_SINGLE);
			xbuf_write_data(xbuf, sizeof(daliasskintype_t), &skintype);
		}

	/* write the skin images, padding the width up to a multiple of 4 */
		for (j = 0; j < skininfo->num_skins; j++)
		{
			const image_paletted_t *image = skinimages[skininfo->skins[j].offset];
			int x, y;

			for (y = 0; y < image->height; y++)
			{
				xbuf_write_data(xbuf, image->width, image->pixels + y * image->width);

				for (x = image->width; x & 3; x++)
					xbuf_write_byte(xbuf, image->pixels[y * image->width + image->width - 1]);
			}
		}
	}

/* write texcoords */
	for (i = 0; i < mesh->num_xyz; i++)
	{
		mdl_texcoord_t stvert;

		stvert.onseam = LittleLong(0);
		stvert.s = LittleLong((int)(mesh->texcoord2f[i*2+0] * skinwidth)); /* generate texcoords with non-padded skinwidth */
		stvert.t = LittleLong((int)(mesh->texcoord2f[i*2+1] * skinheight));

		xbuf_write_data(xbuf, sizeof(mdl_texcoord_t), &stvert);
	}

/* write triangles */
	for (i = 0; i < mesh->num_tris; i++)
	{
		mdl_triangle_t dtriangle;

		dtriangle.facesfront = LittleLong(1);
		dtriangle.vertex[0] = LittleLong(mesh->triangle3i[i*3+0]);
		dtriangle.vertex[1] = LittleLong(mesh->triangle3i[i*3+1]);
		dtriangle.vertex[2] = LittleLong(mesh->triangle3i[i*3+2]);

		xbuf_write_data(xbuf, sizeof(mdl_triangle_t), &dtriangle);
	}

/* write frames */
	for (i = 0; i < model->num_frames; i++)
	{
		const frameinfo_t *frameinfo = &model->frameinfo[i];
		daliasgroup_t *aliasgroup = NULL;

		if (frameinfo->num_frames > 1)
		{
		/* frame group */
			daliasframetype_t frametype;

			frametype.type = LittleLong(ALIAS_GROUP);
			xbuf_write_data(xbuf, sizeof(daliasframetype_t), &frametype);

			aliasgroup = (daliasgroup_t*)xbuf_reserve_data(xbuf, sizeof(daliasgroup_t));

			aliasgroup->bboxmin.v[0] = 0; /* these will be set later */
			aliasgroup->bboxmin.v[1] = 0;
			aliasgroup->bboxmin.v[2] = 0;
			aliasgroup->bboxmin.lightnormalindex = 0;
			aliasgroup->bboxmax.v[0] = 0;
			aliasgroup->bboxmax.v[1] = 0;
			aliasgroup->bboxmax.v[2] = 0;
			aliasgroup->bboxmax.lightnormalindex = 0;
			aliasgroup->numframes = LittleLong(frameinfo->num_frames);

			for (j = 0; j < frameinfo->num_frames; j++)
			{
				daliasinterval_t interval;
				interval.interval = LittleFloat(frameinfo->frametime * (j + 1));
				xbuf_write_data(xbuf, sizeof(daliasinterval_t), &interval);
			}
		}
		else
		{
		/* single frame */
			daliasframetype_t frametype;
			frametype.type = LittleLong(ALIAS_SINGLE);
			xbuf_write_data(xbuf, sizeof(daliasframetype_t), &frametype);
		}

		for (j = 0; j < frameinfo->num_frames; j++)
		{
			qwalk_daliasframe_t *simpleframe;
			int offset = frameinfo->frames[j].offset;
			const float *v, *n;

			simpleframe = (qwalk_daliasframe_t*)xbuf_reserve_data(xbuf, sizeof(qwalk_daliasframe_t));

			simpleframe->bboxmin.v[0] = 0; /* these will be set in the following loop */
			simpleframe->bboxmin.v[1] = 0;
			simpleframe->bboxmin.v[2] = 0;
			simpleframe->bboxmin.lightnormalindex = 0;
			simpleframe->bboxmax.v[0] = 0;
			simpleframe->bboxmax.v[1] = 0;
			simpleframe->bboxmax.v[2] = 0;
			simpleframe->bboxmax.lightnormalindex = 0;
			strlcpy(simpleframe->name, frameinfo->frames[j].name, sizeof(simpleframe->name));

			v = mesh->vertex3f + offset * mesh->num_xyz * 3;
			n = mesh->normal3f + offset * mesh->num_xyz * 3;

			for (k = 0; k < mesh->num_xyz; k++, v += 3, n += 3)
			{
				dtrivertx_t trivertx;
				float pos[3];

				pos[0] = (v[0] - origin[0]) * iscale[0];
				pos[1] = (v[1] - origin[1]) * iscale[1];
				pos[2] = (v[2] - origin[2]) * iscale[2];

				trivertx.v[0] = (unsigned char)bound(0.0f, pos[0], 255.0f);
				trivertx.v[1] = (unsigned char)bound(0.0f, pos[1], 255.0f);
				trivertx.v[2] = (unsigned char)bound(0.0f, pos[2], 255.0f);
				trivertx.lightnormalindex = compress_normal(n);

				if (k == 0 || trivertx.v[0] < simpleframe->bboxmin.v[0])
					simpleframe->bboxmin.v[0] = trivertx.v[0];
				if (k == 0 || trivertx.v[1] < simpleframe->bboxmin.v[1])
					simpleframe->bboxmin.v[1] = trivertx.v[1];
				if (k == 0 || trivertx.v[2] < simpleframe->bboxmin.v[2])
					simpleframe->bboxmin.v[2] = trivertx.v[2];

				if (k == 0 || trivertx.v[0] > simpleframe->bboxmax.v[0])
					simpleframe->bboxmax.v[0] = trivertx.v[0];
				if (k == 0 || trivertx.v[1] > simpleframe->bboxmax.v[1])
					simpleframe->bboxmax.v[1] = trivertx.v[1];
				if (k == 0 || trivertx.v[2] > simpleframe->bboxmax.v[2])
					simpleframe->bboxmax.v[2] = trivertx.v[2];

				if (aliasgroup)
				{
					if ((j == 0 && k == 0) || trivertx.v[0] < aliasgroup->bboxmin.v[0])
						aliasgroup->bboxmin.v[0] = trivertx.v[0];
					if ((j == 0 && k == 0) || trivertx.v[1] < aliasgroup->bboxmin.v[1])
						aliasgroup->bboxmin.v[1] = trivertx.v[1];
					if ((j == 0 && k == 0) || trivertx.v[2] < aliasgroup->bboxmin.v[2])
						aliasgroup->bboxmin.v[2] = trivertx.v[2];

					if ((j == 0 && k == 0) || trivertx.v[0] > aliasgroup->bboxmax.v[0])
						aliasgroup->bboxmax.v[0] = trivertx.v[0];
					if ((j == 0 && k == 0) || trivertx.v[1] > aliasgroup->bboxmax.v[1])
						aliasgroup->bboxmax.v[1] = trivertx.v[1];
					if ((j == 0 && k == 0) || trivertx.v[2] > aliasgroup->bboxmax.v[2])
						aliasgroup->bboxmax.v[2] = trivertx.v[2];
				}

				xbuf_write_data(xbuf, sizeof(dtrivertx_t), &trivertx);
			}
		}
	}

/* done */
	for (i = 0; i < model->total_skins; i++)
		qfree(skinimages[i]);
	qfree(skinimages);

/* print some compatibility notes (FIXME - split out to a separate function so we can also analyze existing MDLs for compatibility issues) */
	printf("Compatibility notes:\n");
	i = 0;

	if (skinheight > 200)
	{
		printf("- This model won't run in DOSQuake because it has a skin height greater than 200. Use the -texheight parameter to resize the skin.\n");
		i++;
	}
	if (!IS_POWER_OF_TWO(skinwidth) || !IS_POWER_OF_TWO(skinheight))
	{
		printf("- This model's skin is not power-of-two and will be resampled badly in GLQuake.\n"); /* FIXME - unconfirmed */
		i++;
	}
	if (model->num_frames > 256)
	{
		printf("- This model has more than 256 frames and won't work in the default Quake networking protocol.\n");
		i++;
	}
	if (mesh->num_xyz > 1024)
	{
		printf("- This model has more than 1024 vertices and won't load in GLQuake.\n");
		i++;
	}
	if (mesh->num_tris > 2048)
	{
		printf("- This model has more than 2048 triangles and won't load in GLQuake.\n");
		i++;
	}

	if (!i)
		printf("- This model should run fine in all engines.\n");

	model_free(model);
	return true;
}
