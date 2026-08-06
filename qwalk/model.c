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

#include "global.h"
#include "model.h"

typedef struct model_format_s
{
	const char *name;
	const char *file_extension;
	const unsigned ident;

	bool_t (*load)(void *filedata, size_t filesize, model_t *out_model, char **out_error);
	bool_t (*save)(const model_t *model, xbuf_t *xbuf, char **out_error);
} model_format_t;

#define IDMDOHEADER			(('_' << 24) + ('O' << 16) + ('D' << 8) + 'M')

static model_format_t model_formats[] =
{
	{ "MDL", ".mdl", IDMDLHEADER, model_mdl_load, model_mdl_save },
	{ "MD2", ".md2", IDALIASHEADER, model_md2_load, model_md2_save },
	{ "MD3", ".md3", ID3HEADER, model_md3_load, model_md3_save },
	{ "DKM", ".dkm", DKMHEADER, model_dkm_md2_load, NULL },
	{ "FM", ".fm", RAVENFMHEADER, model_fm_md2_load, NULL },
	{ "MDO", ".mdo", IDMDOHEADER, model_mdo_load, NULL }
};

static const model_format_t *get_model_format(const char *filename, void *filedata)
{
	const char *ext = strrchr(filename, '.');
	size_t i;

	if (filedata)
		for (i = 0; i < sizeof(model_formats) / sizeof(model_formats[0]); i++)
			if (model_formats[i].ident == *((unsigned *)filedata))
				return &model_formats[i];

	if (ext)
		for (i = 0; i < sizeof(model_formats) / sizeof(model_formats[0]); i++)
			if (!strcasecmp(ext, model_formats[i].file_extension))
				return &model_formats[i];

	return NULL;
}

void mesh_initialize(model_t *model, mesh_t *mesh)
{
	memset(mesh, 0, sizeof(mesh_t));
}

void mesh_free(model_t *model, mesh_t *mesh)
{
	int i, j;

	mesh_freerenderdata(model, mesh);

	qfree(mesh->name);
	qfree(mesh->vertex3f);
	qfree(mesh->normal3f);
	qfree(mesh->texcoord2f);
	qfree(mesh->triangle3i);

	for (i = 0; i < model->total_skins; i++)
		for (j = 0; j < SKIN_NUMTYPES; j++)
			image_free(&mesh->skins[i].components[j]);
	qfree(mesh->skins);
}

void mesh_generaterenderdata(model_t *model, mesh_t *mesh)
{
	int i, j, k;
	int w, h;
	float fw, fh;

	if (mesh->renderdata.initialized)
		return;

	mesh->renderdata.skins = (struct renderdata_skin*)qmalloc(sizeof(struct renderdata_skin) * model->total_skins);

	for (i = 0; i < model->total_skins; i++)
	{
		for (j = 0; j < SKIN_NUMTYPES; j++)
		{
			const image_rgba_t *component = mesh->skins[i].components[j];

			if (component)
			{
			/* pad the skin up to a power of two */
				for (w = 1; w < component->width; w <<= 1);
				for (h = 1; h < component->height; h <<= 1);

			/* generate padded texcoords */
				mesh->renderdata.skins[i].components[j].texcoord2f = (float*)qmalloc(sizeof(float[2]) * mesh->num_xyz);

				fw = (float)component->width / (float)w;
				fh = (float)component->height / (float)h;
				for (k = 0; k < mesh->num_xyz; k++)
				{
					mesh->renderdata.skins[i].components[j].texcoord2f[k*2+0] = mesh->texcoord2f[k*2+0] * fw;
					mesh->renderdata.skins[i].components[j].texcoord2f[k*2+1] = mesh->texcoord2f[k*2+1] * fh;
				}

			/* pad the skin image */
				mesh->renderdata.skins[i].components[j].image = image_pad(mem_globalpool, component, w, h);
			}
			else
			{
				mesh->renderdata.skins[i].components[j].texcoord2f = NULL;
				mesh->renderdata.skins[i].components[j].image = NULL;
			}

			mesh->renderdata.skins[i].components[j].handle = 0;
		}
	}

	mesh->renderdata.initialized = true;
}

void mesh_freerenderdata(model_t *model, mesh_t *mesh)
{
	int i, j;

	if (!mesh->renderdata.initialized)
		return;

	for (i = 0; i < model->total_skins; i++)
	{
		for (j = 0; j < SKIN_NUMTYPES; j++)
		{
			qfree(mesh->renderdata.skins[i].components[j].texcoord2f);
			image_free(&mesh->renderdata.skins[i].components[j].image);
			/* FIXME - release the uploaded texture */
		}
	}

	qfree(mesh->renderdata.skins);

	mesh->renderdata.initialized = false;
}

void model_initialize(model_t *model)
{
	memset(model, 0, sizeof(model_t));
}

void model_free(model_t *model)
{
	int i, j;
	skininfo_t *skininfo;
	singleskin_t *singleskin;
	frameinfo_t *frameinfo;
	singleframe_t *singleframe;
	tag_t *tag;
	mesh_t *mesh;

	for (i = 0, skininfo = model->skininfo; i < model->num_skins; i++, skininfo++)
	{
		for (j = 0, singleskin = skininfo->skins; j < skininfo->num_skins; j++, singleskin++)
			qfree(singleskin->name);
		qfree(skininfo->skins);
	}
	qfree(model->skininfo);

	for (i = 0, frameinfo = model->frameinfo; i < model->num_frames; i++, frameinfo++)
	{
		for (j = 0, singleframe = frameinfo->frames; j < frameinfo->num_frames; j++, singleframe++)
			qfree(singleframe->name);
		qfree(frameinfo->frames);
	}
	qfree(model->frameinfo);

	for (i = 0, tag = model->tags; i < model->num_tags; i++, tag++)
	{
		qfree(tag->name);
		qfree(tag->matrix);
	}
	qfree(model->tags);

	for (i = 0, mesh = model->meshes; i < model->num_meshes; i++, mesh++)
		mesh_free(model, mesh);
	qfree(model->meshes);

	qfree(model);
}

void model_clear_skins(model_t *model)
{
	int i, j, k;
	mesh_t *mesh;

	for (i = 0; i < model->num_skins; i++)
	{
		for (j = 0; j < model->skininfo[i].num_skins; j++)
			qfree(model->skininfo[i].skins[j].name);
		qfree(model->skininfo[i].skins);
	}
	qfree(model->skininfo);

	for (i = 0, mesh = model->meshes; i < model->num_meshes; i++, mesh++)
	{
		for (j = 0; j < model->total_skins; j++)
			for (k = 0; k < SKIN_NUMTYPES; k++)
				image_free(&mesh->skins[j].components[k]);
		qfree(mesh->skins);

		mesh->skins = NULL;
	}

	model->total_skins = 0;
	model->num_skins = 0;
	model->skininfo = NULL;
}

bool_t model_load(const char *filename, void *filedata, size_t filesize, model_t *out_model, char **out_error)
{
	const model_format_t *format = get_model_format(filename, filedata);

	if (!format)
		return (void)(out_error && (*out_error = msprintf("unrecognized file extension"))), false;
	if (!format->load)
		return (void)(out_error && (*out_error = msprintf("loading not implemented for %s format", format->name))), false;

	return (*format->load)(filedata, filesize, out_model, out_error);
}

model_t *model_load_from_file(const char *filename, char **out_error)
{
	void *filedata;
	size_t filesize;
	model_t *model;

	if (!loadfile(filename, &filedata, &filesize, out_error))
		return NULL;

	model = (model_t*)qmalloc(sizeof(model_t));

	if (!model_load(filename, filedata, filesize, model, out_error))
	{
		qfree(model);
		qfree(filedata);
		return NULL;
	}

	qfree(filedata);
	return model;
}

bool_t model_save(const char *filename, const model_t *model, char **out_error)
{
	const model_format_t *format = get_model_format(filename, NULL);
	xbuf_t *xbuf;

	if (!format)
		return (void)(out_error && (*out_error = msprintf("unrecognized file extension"))), false;
	if (!format->save)
		return (void)(out_error && (*out_error = msprintf("saving not implemented for %s format", format->name))), false;

	/* allocate write buffer, use a memory buffer because we need to be able to rewind at times (not supported with file buffers) */
	xbuf = xbuf_create_memory(262144, out_error);
	if (!xbuf)
		return false;

	/* write the model data into the buffer */
	if (!(*format->save)(model, xbuf, out_error))
	{
		xbuf_free(xbuf, NULL);
		return false;
	}

	/* save buffer contents to file */
	if (!xbuf_write_to_file(xbuf, filename, out_error))
	{
		xbuf_free(xbuf, NULL);
		return false;
	}

	xbuf_free(xbuf, NULL);
	return true;
}

void model_generaterenderdata(model_t *model)
{
	int i;

	for (i = 0; i < model->num_meshes; i++)
		mesh_generaterenderdata(model, &model->meshes[i]);
}

void model_freerenderdata(model_t *model)
{
	int i;

	for (i = 0; i < model->num_meshes; i++)
		mesh_freerenderdata(model, &model->meshes[i]);
}

static model_t *model_clone_except_meshes(const model_t *model)
{
	model_t *newmodel = (model_t*)qmalloc(sizeof(model_t));
	int i, j;

	model_initialize(newmodel);

	newmodel->total_skins = model->total_skins;
	newmodel->num_skins = model->num_skins;
	newmodel->skininfo = (skininfo_t*)qmalloc(sizeof(skininfo_t) * model->num_skins);
	for (i = 0; i < model->num_skins; i++)
	{
		newmodel->skininfo[i].frametime = model->skininfo[i].frametime;
		newmodel->skininfo[i].num_skins = model->skininfo[i].num_skins;
		newmodel->skininfo[i].skins = (singleskin_t*)qmalloc(sizeof(singleskin_t) * model->skininfo[i].num_skins);
		for (j = 0; j < model->skininfo[i].num_skins; j++)
		{
			newmodel->skininfo[i].skins[j].name = copystring(model->skininfo[i].skins[j].name);
			newmodel->skininfo[i].skins[j].offset = model->skininfo[i].skins[j].offset;
		}
	}

	newmodel->total_frames = model->total_frames;
	newmodel->num_frames = model->num_frames;
	newmodel->frameinfo = (frameinfo_t*)qmalloc(sizeof(frameinfo_t) * model->num_frames);
	for (i = 0; i < model->num_frames; i++)
	{
		newmodel->frameinfo[i].frametime = model->frameinfo[i].frametime;
		newmodel->frameinfo[i].num_frames = model->frameinfo[i].num_frames;
		newmodel->frameinfo[i].frames = (singleframe_t*)qmalloc(sizeof(singleframe_t) * model->frameinfo[i].num_frames);
		for (j = 0; j < model->frameinfo[i].num_frames; j++)
		{
			newmodel->frameinfo[i].frames[j].name = copystring(model->frameinfo[i].frames[j].name);
			newmodel->frameinfo[i].frames[j].offset = model->frameinfo[i].frames[j].offset;
		}
	}

	newmodel->num_meshes = 0;
	newmodel->meshes = NULL;

	newmodel->num_tags = model->num_tags;
	newmodel->tags = (tag_t*)qmalloc(sizeof(tag_t) * model->num_tags);
	for (i = 0; i < model->num_tags; i++)
	{
		newmodel->tags[i].name = copystring(model->tags[i].name);
		newmodel->tags[i].matrix = (mat4x4f_t*)qmalloc(sizeof(mat4x4f_t) * model->total_frames);
		memcpy(newmodel->tags[i].matrix, model->tags[i].matrix, sizeof(mat4x4f_t) * model->total_frames);
	}

	newmodel->flags = model->flags;
	newmodel->synctype = model->synctype;
	for (i = 0; i < 3; i++)
		newmodel->offsets[i] = model->offsets[i];

	return newmodel;
}

model_t *model_clone(const model_t *model)
{
	model_t *newmodel = model_clone_except_meshes(model);
	int i, j, k;

	newmodel->num_meshes = model->num_meshes;
	newmodel->meshes = (mesh_t*)qmalloc(sizeof(mesh_t) * model->num_meshes);
	for (i = 0; i < model->num_meshes; i++)
	{
		mesh_initialize(newmodel, &newmodel->meshes[i]);

		newmodel->meshes[i].name = copystring(model->meshes[i].name);

		newmodel->meshes[i].num_xyz = model->meshes[i].num_xyz;
		newmodel->meshes[i].num_tris = model->meshes[i].num_tris;

		newmodel->meshes[i].vertex3f = (float*)qmalloc(sizeof(float[3]) * model->total_frames * model->meshes[i].num_xyz);
		memcpy(newmodel->meshes[i].vertex3f, model->meshes[i].vertex3f, sizeof(float[3]) * model->total_frames * model->meshes[i].num_xyz);
		newmodel->meshes[i].normal3f = (float*)qmalloc(sizeof(float[3]) * model->total_frames * model->meshes[i].num_xyz);
		memcpy(newmodel->meshes[i].normal3f, model->meshes[i].normal3f, sizeof(float[3]) * model->total_frames * model->meshes[i].num_xyz);
		newmodel->meshes[i].texcoord2f = (float*)qmalloc(sizeof(float[2]) * model->meshes[i].num_xyz);
		memcpy(newmodel->meshes[i].texcoord2f, model->meshes[i].texcoord2f, sizeof(float[2]) * model->meshes[i].num_xyz);
		newmodel->meshes[i].triangle3i = (int*)qmalloc(sizeof(int[3]) * model->meshes[i].num_tris);
		memcpy(newmodel->meshes[i].triangle3i, model->meshes[i].triangle3i, sizeof(int[3]) * model->meshes[i].num_tris);

		newmodel->meshes[i].skins = (meshskin_t*)qmalloc(sizeof(meshskin_t) * model->total_skins);
		for (j = 0; j < model->total_skins; j++)
			for (k = 0; k < SKIN_NUMTYPES; k++)
				newmodel->meshes[i].skins[j].components[k] = image_clone(mem_globalpool, model->meshes[i].skins[j].components[k]);
	}

	return newmodel;
}

/* create a copy of a model that has all meshes merged into one */
/* FIXME - currently assumes that all meshes use the same texture (other cases will get very complicated) */
model_t *model_merge_meshes(const model_t *model)
{
	model_t *newmodel;
	mesh_t *newmesh;
	int i, j, k;
	int ofs_verts, ofs_tris;

	if (model->num_meshes < 1)
		return NULL;
	if (model->num_meshes == 1)
		return model_clone(model);

	newmodel = model_clone_except_meshes(model);

	newmodel->num_meshes = 1;
	newmodel->meshes = (mesh_t*)qmalloc(sizeof(mesh_t));

	newmesh = &newmodel->meshes[0];

	mesh_initialize(newmodel, newmesh);

	newmesh->name = copystring("merged");

	for (i = 0; i < model->num_meshes; i++)
	{
		newmesh->num_xyz += model->meshes[i].num_xyz;
		newmesh->num_tris += model->meshes[i].num_tris;
	}

	printf("vert: %d, tris %d in %d frames\n", newmesh->num_xyz, newmesh->num_tris, newmodel->total_frames);

	newmesh->vertex3f = (float*)qmalloc(sizeof(float[3]) * newmesh->num_xyz * newmodel->total_frames);
	newmesh->normal3f = (float*)qmalloc(sizeof(float[3]) * newmesh->num_xyz * newmodel->total_frames);
	newmesh->texcoord2f = (float*)qmalloc(sizeof(float[2]) * newmesh->num_xyz);
	newmesh->triangle3i = (int*)qmalloc(sizeof(int[3]) * newmesh->num_tris);

	ofs_verts = 0;
	ofs_tris = 0;

	for (i = 0; i < model->num_meshes; i++)
	{
		const mesh_t *mesh = &model->meshes[i];


		for (j = 0; j < mesh->num_xyz * 2; j++)
		{
			newmesh->texcoord2f[ofs_verts * 2 + j] = mesh->texcoord2f[j];
		}

		for (j = 0; j < newmodel->total_frames; j++)
		{
			float *v, *n;
			const float *iv, *in;

			v = newmesh->vertex3f + j * newmesh->num_xyz * 3 + ofs_verts * 3;
			n = newmesh->normal3f + j * newmesh->num_xyz * 3 + ofs_verts * 3;
			iv = mesh->vertex3f + j * mesh->num_xyz * 3;
			in = mesh->normal3f + j * mesh->num_xyz * 3;

			for (k = 0; k < mesh->num_xyz * 3; k++)
			{
				v[k] = iv[k];
				n[k] = in[k];
			}
		}

		for (j = 0; j < mesh->num_tris * 3; j++)
		{
			newmesh->triangle3i[ofs_tris * 3 + j] = ofs_verts + mesh->triangle3i[j];
		}

		ofs_verts += mesh->num_xyz;
		ofs_tris += mesh->num_tris;
	}

	/* FIXME - this just grabs the first mesh's skin */
	newmesh->skins = (meshskin_t*)qmalloc(sizeof(meshskin_t) * newmodel->total_skins);

	for (i = 0; i < newmodel->total_skins; i++)
		for (j = 0; j < SKIN_NUMTYPES; j++)
			newmesh->skins[i].components[j] = image_clone(mem_globalpool, model->meshes[0].skins[i].components[j]);

	return newmodel;
}

void model_recalculate_normals(model_t *model)
{
	int m, f, t, v;
	mesh_t *mesh;

	for (m = 0, mesh = model->meshes; m < model->num_meshes; m++, mesh++)
	{
		/* clear all normals */
		memset(mesh->normal3f, 0, sizeof(float[3]) * mesh->num_xyz * model->total_frames);

		/* add up all the unnormalized "normals" */
		for (f = 0; f < model->total_frames; f++)
		{
			const int firstvertex = f * mesh->num_xyz * 3;
			const int *tri;

			for (tri = mesh->triangle3i, t = 0; t < mesh->num_tris; tri += 3, t++)
			{
				const float *v0 = mesh->vertex3f + firstvertex + tri[0] * 3;
				const float *v1 = mesh->vertex3f + firstvertex + tri[1] * 3;
				const float *v2 = mesh->vertex3f + firstvertex + tri[2] * 3;
				float *n0 = mesh->normal3f + firstvertex + tri[0] * 3;
				float *n1 = mesh->normal3f + firstvertex + tri[1] * 3;
				float *n2 = mesh->normal3f + firstvertex + tri[2] * 3;
				float q[3], v[3], normal[3];

				/* find the face normal but don't normalize it yet,
				 * so large faces contribute more to the vertex normal than
				 * small faces */
				VectorSubtract(v1, v0, q);
				VectorSubtract(v1, v2, v);
				CrossProduct(q, v, normal);

				/* add normal to the three vertices of this face */
				VectorAdd(n0, normal, n0);
				VectorAdd(n1, normal, n1);
				VectorAdd(n2, normal, n2);
			}

			/* now go back and normalize all the normals */
			for (v = 0; v < mesh->num_xyz; v++)
				VectorNormalize(mesh->normal3f + firstvertex + v * 3);
		}
	}
}

void model_facetize(model_t *model)
{
	int i, j, k, f;
	mesh_t *mesh;

	for (i = 0, mesh = model->meshes; i < model->num_meshes; i++, mesh++)
	{
		float *vertex3f = (float*)qmalloc(model->total_frames * sizeof(float[3]) * mesh->num_tris * 3);
		float *normal3f = (float*)qmalloc(model->total_frames * sizeof(float[3]) * mesh->num_tris * 3);
		float *texcoord2f = (float*)qmalloc(sizeof(float[2]) * mesh->num_tris * 3);
		float *tc = texcoord2f;

		for (j = 0; j < mesh->num_tris; j++)
		{
			for (f = 0; f < model->total_frames; f++)
			{
				const float *mvs = mesh->vertex3f + f * mesh->num_xyz * 3;
				const float *mns = mesh->normal3f + f * mesh->num_xyz * 3;
				float *a, *b, *c, q[3], v[3], normal[3];

				for (k = 0; k < 3; k++)
				{
					int vertindex = mesh->triangle3i[j * 3 + k];

					float *v = vertex3f + f * (mesh->num_tris * 3) * 3 + (j * 3 + k) * 3;
					float *n = normal3f + f * (mesh->num_tris * 3) * 3 + (j * 3 + k) * 3;

					VectorCopy(v, mvs + vertindex * 3);
					VectorCopy(n, mns + vertindex * 3);
				}

				c = vertex3f + f * (mesh->num_tris * 3) * 3 + (j * 3 + 0) * 3;
				b = vertex3f + f * (mesh->num_tris * 3) * 3 + (j * 3 + 1) * 3;
				a = vertex3f + f * (mesh->num_tris * 3) * 3 + (j * 3 + 2) * 3;

				VectorSubtract(b, c, q);
				VectorSubtract(b, a, v);
				CrossProduct(q, v, normal);
				VectorNormalize(normal);

				c = normal3f + f * (mesh->num_tris * 3) * 3 + (j * 3 + 0) * 3;
				b = normal3f + f * (mesh->num_tris * 3) * 3 + (j * 3 + 1) * 3;
				a = normal3f + f * (mesh->num_tris * 3) * 3 + (j * 3 + 2) * 3;

				VectorCopy(a, normal);
				VectorCopy(b, normal);
				VectorCopy(c, normal);
			}

			for (k = 0; k < 3; k++)
			{
				int vertindex = mesh->triangle3i[j * 3 + k];

				tc[0] = mesh->texcoord2f[vertindex*2+0];
				tc[1] = mesh->texcoord2f[vertindex*2+1];

				tc += 2;
			}

			mesh->triangle3i[j * 3 + 0] = j * 3 + 0;
			mesh->triangle3i[j * 3 + 1] = j * 3 + 1;
			mesh->triangle3i[j * 3 + 2] = j * 3 + 2;
		}

		qfree(mesh->vertex3f);
		qfree(mesh->normal3f);
		qfree(mesh->texcoord2f);
		mesh->vertex3f = vertex3f;
		mesh->normal3f = normal3f;
		mesh->texcoord2f = texcoord2f;
		mesh->num_xyz = mesh->num_tris * 3;
	}
}

void model_rename_frames(model_t *model)
{
	int i, j;
	frameinfo_t *frameinfo;
	singleframe_t *singleframe;

	for (i = 0, frameinfo = model->frameinfo; i < model->num_frames; i++, frameinfo++)
	{
		if (frameinfo->num_frames == 1)
		{
			singleframe = frameinfo->frames;
			if (singleframe->name)
				qfree(singleframe->name);
			singleframe->name = msprintf("frame%d", i + 1);
		}
		else
		{
			for (j = 0, singleframe = frameinfo->frames; j < frameinfo->num_frames; j++, singleframe++)
			{
				if (singleframe->name)
					qfree(singleframe->name);
				singleframe->name = msprintf("frame%d_%d", i + 1, j + 1);
			}
		}
	}
}
