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

bool_t model_md3_load(void *filedata, size_t filesize, model_t *out_model, char **out_error)
{
	unsigned char *f = (unsigned char*)filedata;
	mem_pool_t *pool;
	md3_header_t *header;
	int i, j;
	model_t model;

	header = (md3_header_t*)f;

	/* validate format */
	if (LittleLong(header->ident) != ID3HEADER)
		return (void)(out_error && (*out_error = msprintf("wrong format (not IDP3)"))), false;
	if (LittleLong(header->version) != ID3_VERSION)
		return (void)(out_error && (*out_error = msprintf("wrong format (version not 15)"))), false;

	pool = mem_create_pool();

	/* byteswap header */
	header->version        = LittleLong(header->version);
	header->flags          = LittleLong(header->flags);
	header->num_frames     = LittleLong(header->num_frames);
	header->num_tags       = LittleLong(header->num_tags);
	header->num_meshes     = LittleLong(header->num_meshes);
	header->num_skins      = LittleLong(header->num_skins);
	header->ofs_frames     = LittleLong(header->ofs_frames);
	header->ofs_tags       = LittleLong(header->ofs_tags);
	header->ofs_meshes     = LittleLong(header->ofs_meshes);
	header->ofs_end        = LittleLong(header->ofs_end);

	/* read skins */
	model.total_skins = 0;
	model.num_skins = 0;
	model.skininfo = NULL;

	/* read frames */
	model.total_frames = header->num_frames;

	model.num_frames = header->num_frames;
	model.frameinfo = (frameinfo_t*)mem_alloc(pool, sizeof(frameinfo_t) * model.num_frames);

	printf("skins: %d\n", header->num_skins);
	printf("frames: %d\n", model.num_frames);
	f = (unsigned char*)filedata + header->ofs_frames;
	for (i = 0; i < model.num_frames; i++)
	{
		md3_frameinfo_t *md3_frameinfo = (md3_frameinfo_t*)f;

		model.frameinfo[i].frametime = 0.1f;
		model.frameinfo[i].num_frames = 1;
		model.frameinfo[i].frames = (singleframe_t*)mem_alloc(pool, sizeof(singleframe_t));
		model.frameinfo[i].frames[0].name = mem_copystring(pool, md3_frameinfo->name);
		printf("frame %d: %s\n", i, model.frameinfo[i].frames[0].name);
		model.frameinfo[i].frames[0].offset = i;

		f += sizeof(md3_frameinfo_t);
	}

	/* read tags */
	model.num_tags = header->num_tags;
	model.tags = (tag_t*)mem_alloc(pool, sizeof(tag_t) * model.num_tags);

	printf("tags: %d\n", model.num_tags);
	f = (unsigned char*)filedata + header->ofs_tags;
	for (i = 0; i < header->num_tags; i++)
		model.tags[i].matrix = (mat4x4f_t*)mem_alloc(pool, sizeof(mat4x4f_t) * model.num_frames);
	for (i = 0; i < model.num_frames; i++)
	{
		for (j = 0; j < header->num_tags; j++)
		{
			md3_tag_t *md3_tag = (md3_tag_t*)f;
			tag_t *tag = &model.tags[j];

			if (i == 0)
				tag->name = mem_copystring(pool, md3_tag->name);

			tag->matrix[i].m[0][0] = LittleFloat(md3_tag->rotationmatrix[0]);
			tag->matrix[i].m[0][1] = LittleFloat(md3_tag->rotationmatrix[3]);
			tag->matrix[i].m[0][2] = LittleFloat(md3_tag->rotationmatrix[6]);
			tag->matrix[i].m[0][3] = LittleFloat(md3_tag->origin[0]);
			tag->matrix[i].m[1][0] = LittleFloat(md3_tag->rotationmatrix[1]);
			tag->matrix[i].m[1][1] = LittleFloat(md3_tag->rotationmatrix[4]);
			tag->matrix[i].m[1][2] = LittleFloat(md3_tag->rotationmatrix[7]);
			tag->matrix[i].m[1][3] = LittleFloat(md3_tag->origin[1]);
			tag->matrix[i].m[2][0] = LittleFloat(md3_tag->rotationmatrix[2]);
			tag->matrix[i].m[2][1] = LittleFloat(md3_tag->rotationmatrix[5]);
			tag->matrix[i].m[2][2] = LittleFloat(md3_tag->rotationmatrix[8]);
			tag->matrix[i].m[2][3] = LittleFloat(md3_tag->origin[2]);
			tag->matrix[i].m[3][0] = 0;
			tag->matrix[i].m[3][1] = 0;
			tag->matrix[i].m[3][2] = 0;
			tag->matrix[i].m[3][3] = 1;

			f += sizeof(md3_tag_t);
		}
	}

	/* read meshes */
	model.num_meshes = header->num_meshes;
	model.meshes = (mesh_t*)mem_alloc(pool, sizeof(mesh_t) * model.num_meshes);

	printf("meshes: %d\n", header->num_meshes);
	f = (unsigned char*)filedata + header->ofs_meshes;
	for (i = 0; i < header->num_meshes; i++)
	{
		md3_mesh_t *md3_mesh = (md3_mesh_t*)f;
		mesh_t *mesh = &model.meshes[i];
		md3_vertex_t *md3_vertex;

		md3_mesh->flags       = LittleLong(md3_mesh->flags);
		md3_mesh->num_frames  = LittleLong(md3_mesh->num_frames);
		md3_mesh->num_shaders = LittleLong(md3_mesh->num_shaders);
		md3_mesh->num_xyz     = LittleLong(md3_mesh->num_xyz);
		md3_mesh->num_tris    = LittleLong(md3_mesh->num_tris);
		md3_mesh->ofs_tris    = LittleLong(md3_mesh->ofs_tris);
		md3_mesh->ofs_shaders = LittleLong(md3_mesh->ofs_shaders);
		md3_mesh->ofs_st      = LittleLong(md3_mesh->ofs_st);
		md3_mesh->ofs_verts   = LittleLong(md3_mesh->ofs_verts);
		md3_mesh->ofs_end     = LittleLong(md3_mesh->ofs_end);

		mesh_initialize(&model, mesh);

		mesh->name = mem_copystring(pool, md3_mesh->name);
		mesh->num_xyz = md3_mesh->num_xyz;
		mesh->num_tris = md3_mesh->num_tris;

		printf("mesh %d:\n", i);
		printf("\tname: \"%s\"\n", md3_mesh->name);
		printf("\tframes: %d\n", md3_mesh->num_frames);
		printf("\tvert: %d\n", mesh->num_xyz);
		printf("\ttris: %d\n", mesh->num_tris);

		/* load triangles */
		mesh->triangle3i = (int*)mem_alloc(pool, sizeof(int) * mesh->num_tris * 3);

		const int *p = (const int*)(f + md3_mesh->ofs_tris);
		for (j = 0; j < mesh->num_tris * 3; j++)
		{
			mesh->triangle3i[j] = LittleLong(p[j]);
		}

		/* load texcoords */
		mesh->texcoord2f = (float*)mem_alloc(pool, sizeof(float) * mesh->num_xyz * 2);
		const float *fst = (const float*)(f + md3_mesh->ofs_st);

		for (j = 0; j < mesh->num_xyz * 2; j++)
		{
			mesh->texcoord2f[j] = LittleFloat(fst[j]);
		}

		/* load frames */
		mesh->vertex3f = (float*)mem_alloc(pool, sizeof(float) * model.num_frames * mesh->num_xyz * 3);
		mesh->normal3f = (float*)mem_alloc(pool, sizeof(float) * model.num_frames * mesh->num_xyz * 3);

		for (j = 0, md3_vertex = (md3_vertex_t*)(f + md3_mesh->ofs_verts); j < model.num_frames * mesh->num_xyz; j++, md3_vertex++)
		{
			double npitch, nyaw;

			mesh->vertex3f[j*3+0] = (signed short)LittleShort(md3_vertex->origin[0]) * (1.0f / 64.0f);
			mesh->vertex3f[j*3+1] = (signed short)LittleShort(md3_vertex->origin[1]) * (1.0f / 64.0f);
			mesh->vertex3f[j*3+2] = (signed short)LittleShort(md3_vertex->origin[2]) * (1.0f / 64.0f);

			/* decompress the vertex normal */
			md3_vertex->normalpitchyaw = LittleShort(md3_vertex->normalpitchyaw);
			npitch = (md3_vertex->normalpitchyaw & 255) * (2 * M_PI) / 256.0;
			nyaw = ((md3_vertex->normalpitchyaw >> 8) & 255) * (2 * M_PI) / 256.0;

			mesh->normal3f[j*3+0] = (float)(sin(npitch) * cos(nyaw));
			mesh->normal3f[j*3+1] = (float)(sin(npitch) * sin(nyaw));
			mesh->normal3f[j*3+2] = (float)cos(npitch);
		}

		/* load shaders */
 		for (j = 0; j < md3_mesh->num_shaders; j++)
		{
			md3_shader_t *md3_shader = (md3_shader_t*)(f + md3_mesh->ofs_shaders) + j;

			printf(" - shader %d: \"%s\"\n", j, md3_shader->name);
		}

		/* load skin */
		mesh->skins = NULL;

		f += md3_mesh->ofs_end;
	}

	mem_merge_pool(pool);

	model.flags = header->flags;
	model.synctype = 0;
	model.offsets[0] = 0.0f;
	model.offsets[1] = 0.0f;
	model.offsets[2] = 0.0f;

	*out_model = model;
	return true;
}

static unsigned short md3_encodenormal(const float n[3])
{
	int blat, blng;

	/* check for singularities */
	if (n[0] == 0 && n[1] == 0)
	{
		if (n[2] > 0)
		{
			blat = 0;
			blng = 0;
		}
		else
		{
			blat = 0;
			blng = 128;
		}
	}
	else
	{
		blat = (int)(atan2(n[1], n[0]) * (255.0 / (2 * M_PI)));
		blng = (int)(acos(n[2]) * (255.0 / (2 * M_PI)));
	}

	return ((blat & 0xff) << 8) | (blng & 0xff);
}

bool_t model_md3_save(const model_t *model, xbuf_t *xbuf, char **out_error)
{
	md3_header_t header;
	const frameinfo_t *frameinfo;
	const tag_t *tag;
	const mesh_t *mesh;
	int i, j, k, m;

	header.ident      = LittleLong(ID3HEADER);
	header.version    = LittleLong(ID3_VERSION);
	strlcpy(header.name, "md3model", sizeof(header.name));
	header.flags      = LittleLong(model->flags);
	header.num_frames = LittleLong(model->num_frames);
	header.num_tags   = LittleLong(model->num_tags);
	header.num_meshes = LittleLong(model->num_meshes);
	header.num_skins  = LittleLong(model->num_skins);

	/* calculate lump offsets */
	i = sizeof(md3_header_t);

	header.ofs_frames = LittleLong(i);
	i += sizeof(md3_frameinfo_t) * model->num_frames;

	header.ofs_tags = LittleLong(i);
	i += sizeof(md3_tag_t) * model->num_frames * model->num_tags;

	header.ofs_meshes = i;
	for (j = 0, mesh = model->meshes; j < model->num_meshes; j++, mesh++)
	{
		i += sizeof(md3_mesh_t); /* mesh header */
		i += mesh->num_tris * sizeof(int[3]); /* triangle elements */
		/* FIXME - shaders? */
		i += mesh->num_xyz * sizeof(float[2]); /* texcoords */
		i += mesh->num_xyz * model->num_frames * sizeof(md3_vertex_t); /* framevertices */
	}

	header.ofs_end = i;

	/* write header */
	xbuf_write_data(xbuf, sizeof(md3_header_t), &header);

	/* write frameinfo */
	for (i = 0, frameinfo = model->frameinfo; i < model->num_frames; i++, frameinfo++)
	{
		const singleframe_t *singleframe = &frameinfo->frames[0];
		md3_frameinfo_t md3_frameinfo;
		float mins[3], maxs[3], dist[3];
		bool_t first = true;

		VectorClear(mins);
		VectorClear(maxs);

		for (j = 0, mesh = model->meshes; j < model->num_meshes; j++, mesh++)
		{
			const float *v = mesh->vertex3f + mesh->num_xyz * singleframe->offset * 3;

			for (k = 0; k < mesh->num_xyz; k++, v += 3)
			{
				for (m = 0; m < 3; m++)
				{
					mins[m] = first ? v[m] : min(mins[m], v[m]);
					maxs[m] = first ? v[m] : max(maxs[m], v[m]);
					first = false;
				}
			}
		}

		for (m = 0; m < 3; m++)
			dist[m] = (fabs(mins[m]) > fabs(maxs[m])) ? mins[m] : maxs[m];

		VectorCopy(md3_frameinfo.mins, mins);
		VectorCopy(md3_frameinfo.maxs, maxs);
		VectorClear(md3_frameinfo.origin);
		md3_frameinfo.radius = (float)sqrt(DotProduct(dist, dist));
		strlcpy(md3_frameinfo.name, singleframe->name, sizeof(md3_frameinfo.name));

		xbuf_write_data(xbuf, sizeof(md3_frameinfo_t), &md3_frameinfo);
	}

	/* write tags */
	for (i = 0, frameinfo = model->frameinfo; i < model->num_frames; i++, frameinfo++)
	{
		for (j = 0, tag = model->tags; j < model->num_tags; j++, tag++)
		{
			int ofs = frameinfo->frames[0].offset;
			md3_tag_t md3_tag;

			strlcpy(md3_tag.name, tag->name, sizeof(md3_tag.name));

			md3_tag.rotationmatrix[0] = LittleFloat(tag->matrix[ofs].m[0][0]);
			md3_tag.rotationmatrix[3] = LittleFloat(tag->matrix[ofs].m[0][1]);
			md3_tag.rotationmatrix[6] = LittleFloat(tag->matrix[ofs].m[0][2]);
			md3_tag.origin[0] = LittleFloat(tag->matrix[ofs].m[0][3]);
			md3_tag.rotationmatrix[1] = LittleFloat(tag->matrix[ofs].m[1][0]);
			md3_tag.rotationmatrix[4] = LittleFloat(tag->matrix[ofs].m[1][1]);
			md3_tag.rotationmatrix[7] = LittleFloat(tag->matrix[ofs].m[1][2]);
			md3_tag.origin[1] = LittleFloat(tag->matrix[ofs].m[1][3]);
			md3_tag.rotationmatrix[2] = LittleFloat(tag->matrix[ofs].m[2][0]);
			md3_tag.rotationmatrix[5] = LittleFloat(tag->matrix[ofs].m[2][1]);
			md3_tag.rotationmatrix[8] = LittleFloat(tag->matrix[ofs].m[2][2]);
			md3_tag.origin[2] = LittleFloat(tag->matrix[ofs].m[2][3]);

			xbuf_write_data(xbuf, sizeof(md3_tag_t), &md3_tag);
		}
	}

	/* write meshes */
	for (i = 0, mesh = model->meshes; i < model->num_meshes; i++, mesh++)
	{
		md3_mesh_t md3_mesh;

		md3_mesh.ident = LittleLong(ID3HEADER);
		strlcpy(md3_mesh.name, mesh->name, sizeof(md3_mesh.name));
		md3_mesh.flags = 0; /* unused */

		md3_mesh.num_frames = LittleLong(model->num_frames);
		md3_mesh.num_shaders = 0; // FIXME - TODO
		md3_mesh.num_xyz = LittleLong(mesh->num_xyz);
		md3_mesh.num_tris = LittleLong(mesh->num_tris);

		j = sizeof(md3_mesh_t);

		md3_mesh.ofs_tris = j;
		j += mesh->num_tris * sizeof(int[3]);

		md3_mesh.ofs_shaders = j;
		j += 0; // FIXME - TODO

		md3_mesh.ofs_st = j;
		j += mesh->num_xyz * sizeof(float[2]);

		md3_mesh.ofs_verts = j;
		j += mesh->num_xyz * model->num_frames * sizeof(md3_vertex_t);

		md3_mesh.ofs_end = j;

		xbuf_write_data(xbuf, sizeof(md3_mesh_t), &md3_mesh);

		/* write triangles */
		for (j = 0; j < mesh->num_tris; j++)
		{
			int triangle[3];

			triangle[0] = LittleLong(mesh->triangle3i[j*3+0]);
			triangle[1] = LittleLong(mesh->triangle3i[j*3+1]);
			triangle[2] = LittleLong(mesh->triangle3i[j*3+2]);

			xbuf_write_data(xbuf, sizeof(triangle), triangle);
		}

		/* write shaders */
		// FIXME - TODO

		/* write texcoords */
		for (j = 0; j < mesh->num_xyz; j++)
		{
			float texcoord[2];

			texcoord[0] = LittleFloat(mesh->texcoord2f[j*2+0]);
			texcoord[1] = LittleFloat(mesh->texcoord2f[j*2+1]);

			xbuf_write_data(xbuf, sizeof(texcoord), texcoord);
		}

		/* write framevertices */
		for (j = 0, frameinfo = model->frameinfo; j < model->num_frames; j++, frameinfo++)
		{
			const singleframe_t *singleframe = &frameinfo->frames[0];
			const float *v = mesh->vertex3f + singleframe->offset * mesh->num_xyz * 3;
			const float *n = mesh->normal3f + singleframe->offset * mesh->num_xyz * 3;

			for (k = 0; k < mesh->num_xyz; k++, v += 3, n += 3)
			{
				md3_vertex_t md3_vertex;

				int x = (int)(v[0] * 64.0f);
				int y = (int)(v[1] * 64.0f);
				int z = (int)(v[2] * 64.0f);

				md3_vertex.origin[0] = LittleShort(bound(-32768, x, 32767));
				md3_vertex.origin[1] = LittleShort(bound(-32768, y, 32767));
				md3_vertex.origin[2] = LittleShort(bound(-32768, z, 32767));
				md3_vertex.normalpitchyaw = md3_encodenormal(n);

				xbuf_write_data(xbuf, sizeof(md3_vertex_t), &md3_vertex);
			}
		}
	}

	return true;
}
