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

#ifndef MODEL_H
#define MODEL_H

#include <stdlib.h>

#include "matrix.h"
#include "image.h"

#define MAX_QPATH 64

#include "../src/common/header/files.h"

#define MAX_NAMESIZE 64

typedef struct singleskin_s
{
	char *name;
	int offset; /* how many "actual skins" into the mesh skin lumps */
} singleskin_t;

typedef struct skininfo_s
{
	int num_skins;
	singleskin_t *skins;

	float frametime; /* 0.1 for single skins, otherwise based on MDL skingroup interval */
} skininfo_t;

typedef struct singleframe_s
{
	char *name;
	int offset; /* how many "actual frames" into the vertex3f/normal3f lumps */
} singleframe_t;

typedef struct frameinfo_s
{
	int num_frames;
	singleframe_t *frames;

	float frametime; /* 0.1 for single frames, otherwise based on MDL framegroup interval */
} frameinfo_t;

typedef struct tag_s
{
	char *name;

	mat4x4f_t *matrix; /* [model.total_frames] */
} tag_t;

typedef enum skintype_e
{
	SKIN_DIFFUSE = 0,
	SKIN_FULLBRIGHT,

	SKIN_NUMTYPES
} skintype_t;

typedef struct meshskin_s
{
	image_rgba_t *components[SKIN_NUMTYPES];
} meshskin_t;

typedef struct mesh_s
{
	char *name;

	int num_xyz;
	int num_tris;

	float *vertex3f;
	float *normal3f;
	float *texcoord2f;

	int *triangle3i;

	meshskin_t *skins; /* [model.total_skins] */

	struct
	{
		bool_t initialized;

		/* the texture may need to be padded to power-of-two dimensions to be rendered, so we need extra texturing information */
		/* each texture layer might be a different size, so we may have multiple sets of differently-padded texcoords... */
		struct renderdata_skin
		{
			struct
			{
				float *texcoord2f;
				image_rgba_t *image;
				unsigned int handle;
			} components[SKIN_NUMTYPES];
		} *skins; /* [model.total_skins] */
	} renderdata;
} mesh_t;

typedef struct model_s
{
	int total_skins; /* including skingroup frames */
	int num_skins;
	skininfo_t *skininfo;

	int total_frames; /* including framegroup frames */
	int num_frames;
	frameinfo_t *frameinfo;

	int num_meshes;
	mesh_t *meshes;

	int num_tags;
	tag_t *tags;

	int flags; /* quake only */
	int synctype; /* quake only, possible values: 0 (sync), 1 (rand) */
	float offsets[3]; /* quake only, unused but i'm including it for completeness */
} model_t;

void mesh_initialize(model_t *model, mesh_t *mesh);
void mesh_free(model_t *model, mesh_t *mesh);
void mesh_generaterenderdata(model_t *model, mesh_t *mesh);
void mesh_freerenderdata(model_t *model, mesh_t *mesh);

void model_initialize(model_t *model);
void model_free(model_t *model);
void model_generaterenderdata(model_t *model);
void model_freerenderdata(model_t *model);

void model_clear_skins(model_t *model);

/* note that the filedata pointer is not const, because it may be modified (most likely by byteswapping) */
bool_t model_load(const char *filename, void *filedata, size_t filesize, model_t *out_model, char **out_error);
model_t *model_load_from_file(const char *filename, char **out_error);

bool_t model_save(const char *filename, const model_t *model, char **out_error);

bool_t model_mdo_load(void *filedata, size_t filesize, model_t *out_model, char **out_error);
bool_t model_mdl_load(void *filedata, size_t filesize, model_t *out_model, char **out_error);
bool_t model_md2_load(void *filedata, size_t filesize, model_t *out_model, char **out_error);
bool_t model_md3_load(void *filedata, size_t filesize, model_t *out_model, char **out_error);
bool_t model_md3_md2_load(void *filedata, size_t filesize, model_t *out_model, char **out_error);
bool_t model_dkm_md2_load(void *filedata, size_t filesize, model_t *out_model, char **out_error);
bool_t model_fm_md2_load(void *filedata, size_t filesize, model_t *out_model, char **out_error);

bool_t model_mdl_save(const model_t *model, xbuf_t *xbuf, char **out_error);
bool_t model_md2_save(const model_t *model, xbuf_t *xbuf, char **out_error);
bool_t model_md3_save(const model_t *model, xbuf_t *xbuf, char **out_error);

model_t *model_clone(const model_t *model);

model_t *model_merge_meshes(const model_t *model);

void model_recalculate_normals(model_t *model);
void model_facetize(model_t *model);
void model_rename_frames(model_t *model);

#endif
