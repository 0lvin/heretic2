#include "global.h"
#include "model.h"
#include "palettes.h"

#include <string.h>

extern const float anorms[162][3];

typedef struct mdo_header_s
{
	char ident[4]; /* "MDO_" */
	int version; /* 1 */
	float offsets[3];
	int bitmap_count;
	int bitmap_width;
	int bitmap_height;
	int skin_vertex_count;
	int triangle_count;
	int frame_vertex_count;
	int frame_count;
	int synctype;
	int flags;
	int global_palette;
} mdo_header_t;

typedef struct mdo_stvert_s
{
	int onseam;
	int s, t;
} mdo_stvert_t;

typedef struct mdo_triangle_s
{
	int facesfront;
	int vertindex[3];
	int skinvertindex[3];
} mdo_triangle_t;

typedef struct mdo_vertex_s
{
	float x, y, z;
	unsigned char lightnormalindex;
} mdo_vertex_t;

typedef struct mdo_frame_s
{
	mdo_vertex_t lower_bounds;
	mdo_vertex_t upper_bounds;
	char name[16];
} mdo_frame_t;

/* intermediate (not disk) struct */
typedef struct meshvert_s
{
	int vertex;
	int s, t, back;
} meshvert_t;

/* data in mdo files is unaligned so we have to load it carefully (we can't
 * just load integers or structs straight from the file) */

static unsigned char mdo_read_byte(unsigned char **fptr)
{
	return *(*fptr)++;
}

static int mdo_read_int(unsigned char **fptr)
{
	int i;

	i = mdo_read_byte(fptr);
	i |= mdo_read_byte(fptr) << 8;
	i |= mdo_read_byte(fptr) << 16;
	i |= mdo_read_byte(fptr) << 24;

	return i;
}

static float mdo_read_float(unsigned char **fptr)
{
	union { int i; float f; } u;
	u.i = mdo_read_int(fptr);
	return u.f;
}

static void mdo_read_stvert(unsigned char **fptr, mdo_stvert_t *out_stvert)
{
	if (out_stvert)
	{
		out_stvert->onseam = mdo_read_int(fptr);
		out_stvert->s = mdo_read_int(fptr);
		out_stvert->t = mdo_read_int(fptr);
	}
	else
	{
		*fptr += 12;
	}
}

static void mdo_read_triangle(unsigned char **fptr, mdo_triangle_t *out_triangle)
{
	if (out_triangle)
	{
		out_triangle->facesfront = mdo_read_int(fptr);
		out_triangle->vertindex[0] = mdo_read_int(fptr);
		out_triangle->vertindex[1] = mdo_read_int(fptr);
		out_triangle->vertindex[2] = mdo_read_int(fptr);
		out_triangle->skinvertindex[0] = mdo_read_int(fptr);
		out_triangle->skinvertindex[1] = mdo_read_int(fptr);
		out_triangle->skinvertindex[2] = mdo_read_int(fptr);
	}
	else
	{
		*fptr += 28;
	}
}

static void mdo_read_vertex(unsigned char **fptr, mdo_vertex_t *out_vertex)
{
	if (out_vertex)
	{
		out_vertex->x = mdo_read_float(fptr);
		out_vertex->y = mdo_read_float(fptr);
		out_vertex->z = mdo_read_float(fptr);
		out_vertex->lightnormalindex = mdo_read_byte(fptr);
	}
	else
	{
		*fptr += 13;
	}
}

static void mdo_read_frame(unsigned char **fptr, mdo_frame_t *out_frame)
{
	if (out_frame)
	{
		mdo_read_vertex(fptr, &out_frame->lower_bounds);
		mdo_read_vertex(fptr, &out_frame->upper_bounds);

		memcpy(out_frame->name, *fptr, 16); *fptr += 16;
	}
	else
	{
		*fptr += 42;
	}
}

static int mdo_count_skins(const mdo_header_t *header, unsigned char *f)
{
	int total_skins = 0;
	int i, j;

	for (i = 0; i < header->bitmap_count; i++)
	{
		int skincount;

		if (mdo_read_int(&f) == 0)
			skincount = 1;
		else
		{
			skincount = mdo_read_int(&f);
			f += skincount * sizeof(float); /* skip intervals */
		}

		for (j = 0; j < skincount; j++)
		{
			f += mdo_read_int(&f); /* skip pcx image */

			if (header->global_palette != 1)
			{
				f += 1024; /* skip palette */
				f += 16385; /* skip colormap */
				f += sizeof(float); /* skip colormap range */
				f++; /* skip number of fullbrights */
			}
		}

		total_skins += skincount;
	}

	return total_skins;
}

static bool_t mdo_load_skins(const mdo_header_t *header, model_t *model, mem_pool_t *pool, unsigned char **fptr, char **out_error)
{
	unsigned char *f = *fptr;
	mesh_t *mesh = &model->meshes[0];
	skininfo_t *skininfo;
	int num_strings;
	int offset;
	int i, j, k;

	model->num_skins = header->bitmap_count;
	model->skininfo = (skininfo_t*)mem_alloc(pool, sizeof(skininfo_t) * model->num_skins);

	model->total_skins = mdo_count_skins(header, f);

	mesh->skins = (meshskin_t*)mem_alloc(pool, sizeof(meshskin_t) * model->total_skins);

	offset = 0;

	for (i = 0, skininfo = model->skininfo; i < header->bitmap_count; i++, skininfo++)
	{
	/* determine if this is a single skin or a skingroup */
		if (mdo_read_int(&f) == 0)
		{
			skininfo->frametime = 0.1f;
			skininfo->num_skins = 1;
		}
		else
		{
			int skincount = mdo_read_int(&f);

			float frametime = 0.01f;
			float prevstoptime = 0.0f;

			for (j = 0; j < skincount; j++)
			{
				float stoptime = mdo_read_float(&f);
				float interval = stoptime - prevstoptime;
				prevstoptime = stoptime;

				frametime = max(frametime, interval);
			}

			skininfo->frametime = frametime;
			skininfo->num_skins = skincount;
		}

		skininfo->skins = (singleskin_t*)mem_alloc(pool, sizeof(singleskin_t) * skininfo->num_skins);

	/* load the skins from the stream */
		for (j = 0; j < skininfo->num_skins; j++, offset++)
		{
			image_paletted_t *image;
			char *error;

			int size = mdo_read_int(&f);

		/* load skin from file */
			image = image_pcx_load_paletted(pool, f, size, &error);
			if (!image)
			{
				if (out_error)
					*out_error = msprintf("failed to load skin: %s", error);
				qfree(error);
				return false;
			}
			else
			{
				if (image->width != header->bitmap_width || image->height != header->bitmap_height)
					return (void)(out_error && (*out_error = msprintf("failed to load skin: wrong size"))), false;

				skininfo->skins[j].name = NULL; /* loaded later */
				skininfo->skins[j].offset = offset;
			}

			f += size;

		/* read local palette and colormap (the palette from the pcx is ignored) */
			if (header->global_palette != 1)
			{
				int num_fullbrights;

				for (k = 0; k < 256; k++)
				{
					image->palette.rgb[k*3+0] = f[k*4+2];
					image->palette.rgb[k*3+1] = f[k*4+1];
					image->palette.rgb[k*3+2] = f[k*4+0];
				}

				f += 1024; /* skip palette (stored as R,G,B,zero quads) */

				f += 16385; /* skip colormap (which has one extra byte, just like colormap.lmp, for some reason...) */

				mdo_read_float(&f); /* skip colormap range */

				num_fullbrights = mdo_read_byte(&f);

				memset(image->palette.fullbright_flags, 0, sizeof(image->palette.fullbright_flags));
				for (k = 256 - num_fullbrights; k < 256; k++)
					image->palette.fullbright_flags[k >> 5] |= 1U << (k & 31);
			}
			else
			{
			/* this is my guess as to what to do in this situation */
				image->palette = palette_quake;
			}

		/* create 32-bit skin */
			mesh->skins[offset].components[SKIN_DIFFUSE]    = image_alloc(pool, header->bitmap_width, header->bitmap_height);
			mesh->skins[offset].components[SKIN_FULLBRIGHT] = image_alloc(pool, header->bitmap_width, header->bitmap_height);

			for (k = 0; k < header->bitmap_width * header->bitmap_height; k++)
			{
				unsigned char c = image->pixels[k];

				if (image->palette.fullbright_flags[c >> 5] & (1U << (c & 31)))
				{
				/* fullbright */
					mesh->skins[offset].components[SKIN_DIFFUSE]->pixels[k*4+0] = 0;
					mesh->skins[offset].components[SKIN_DIFFUSE]->pixels[k*4+1] = 0;
					mesh->skins[offset].components[SKIN_DIFFUSE]->pixels[k*4+2] = 0;
					mesh->skins[offset].components[SKIN_DIFFUSE]->pixels[k*4+3] = 255;

					mesh->skins[offset].components[SKIN_FULLBRIGHT]->pixels[k*4+0] = image->palette.rgb[c*3+0];
					mesh->skins[offset].components[SKIN_FULLBRIGHT]->pixels[k*4+1] = image->palette.rgb[c*3+1];
					mesh->skins[offset].components[SKIN_FULLBRIGHT]->pixels[k*4+2] = image->palette.rgb[c*3+2];
					mesh->skins[offset].components[SKIN_FULLBRIGHT]->pixels[k*4+3] = 255;
				}
				else
				{
				/* normal colour */
					mesh->skins[offset].components[SKIN_DIFFUSE]->pixels[k*4+0] = image->palette.rgb[c*3+0];
					mesh->skins[offset].components[SKIN_DIFFUSE]->pixels[k*4+1] = image->palette.rgb[c*3+1];
					mesh->skins[offset].components[SKIN_DIFFUSE]->pixels[k*4+2] = image->palette.rgb[c*3+2];
					mesh->skins[offset].components[SKIN_DIFFUSE]->pixels[k*4+3] = 255;

					mesh->skins[offset].components[SKIN_FULLBRIGHT]->pixels[k*4+0] = 0;
					mesh->skins[offset].components[SKIN_FULLBRIGHT]->pixels[k*4+1] = 0;
					mesh->skins[offset].components[SKIN_FULLBRIGHT]->pixels[k*4+2] = 0;
					mesh->skins[offset].components[SKIN_FULLBRIGHT]->pixels[k*4+3] = 255;
				}
			}

			image_paletted_free(&image);
		}
	}

/* load skin names */
	num_strings = mdo_read_int(&f);

	if (num_strings < model->total_skins)
		return (void)(out_error && (*out_error = msprintf("number of skin names is less than total number of skins"))), false;

	for (i = 0, skininfo = model->skininfo; i < header->bitmap_count; i++, skininfo++)
	{
		for (j = 0; j < skininfo->num_skins; j++)
		{
			int length = mdo_read_byte(&f);
			char string[257];

			memcpy(string, f, length);
			string[length] = '\0';
			f += length;

			skininfo->skins[j].name = mem_copystring(pool, string);
		}
	}

/* no point erroring out if there are extra strings in the file for some reason
 * (not that i've ever encountered that, but...) */
	for (i = model->total_skins; i < num_strings; i++)
		f += mdo_read_byte(&f);

	*fptr = f;
	return true;
}

static int mdo_count_frames(const mdo_header_t *header, unsigned char *f)
{
	int total_frames = 0;
	int i, j;

	for (i = 0; i < header->frame_count; i++)
	{
		int framecount;

		if (mdo_read_int(&f) == 0)
			framecount = 1;
		else
		{
			framecount = mdo_read_int(&f);

		/* skip framegroup bounds */
			mdo_read_vertex(&f, NULL);
			mdo_read_vertex(&f, NULL);

		/* skip intervals */
			f += framecount * sizeof(float);
		}

		for (j = 0; j < framecount; j++)
		{
			mdo_read_frame(&f, NULL);
			f += header->frame_vertex_count * 13;
		}

		total_frames += framecount;
	}

	return total_frames;
}

static void mdo_load_frames(const mdo_header_t *header, model_t *model, const meshvert_t *meshverts, mem_pool_t *pool, unsigned char **fptr)
{
	unsigned char *f = *fptr;
	mesh_t *mesh = &model->meshes[0];
	unsigned char **framevertstart;
	frameinfo_t *frameinfo;
	int offset;
	int i, j, k;

	model->num_frames = header->frame_count;
	model->frameinfo = (frameinfo_t*)mem_alloc(pool, sizeof(frameinfo_t) * model->num_frames);

	model->total_frames = mdo_count_frames(header, f);

	mesh->vertex3f = (float*)mem_alloc(pool, model->total_frames * sizeof(float) * mesh->num_xyz * 3);
	mesh->normal3f = (float*)mem_alloc(pool, model->total_frames * sizeof(float) * mesh->num_xyz * 3);

	framevertstart = (unsigned char**)mem_alloc(pool, sizeof(unsigned char*) * model->total_frames);

	offset = 0;

	for (i = 0, frameinfo = model->frameinfo; i < header->frame_count; i++, frameinfo++)
	{
		if (mdo_read_int(&f) == 0)
		{
			frameinfo->frametime = 0.1f;
			frameinfo->num_frames = 1;
		}
		else
		{
			int framecount = mdo_read_int(&f);

			float frametime = 0.01f;
			float prevstoptime = 0.0f;

			mdo_read_vertex(&f, NULL); /* skip framegroup bounds */
			mdo_read_vertex(&f, NULL);

			for (j = 0; j < framecount; j++)
			{
				float stoptime = mdo_read_float(&f);
				float interval = stoptime - prevstoptime;
				prevstoptime = stoptime;

				frametime = max(frametime, interval);
			}

			frameinfo->frametime = frametime;
			frameinfo->num_frames = framecount;
		}

		frameinfo->frames = (singleframe_t*)mem_alloc(pool, sizeof(singleframe_t) * frameinfo->num_frames);

		for (j = 0; j < frameinfo->num_frames; j++, offset++)
		{
			mdo_frame_t frame;

			mdo_read_frame(&f, &frame);

			frameinfo->frames[j].name = mem_copystring(pool, frame.name);
			frameinfo->frames[j].offset = offset;

			framevertstart[offset] = f;

			f += header->frame_vertex_count * 13;
		}
	}

	for (i = 0, frameinfo = model->frameinfo; i < model->num_frames; i++, frameinfo++)
	{
		for (j = 0; j < frameinfo->num_frames; j++)
		{
			int firstvertex = frameinfo->frames[j].offset * mesh->num_xyz * 3;
			float *v = mesh->vertex3f + firstvertex;
			float *n = mesh->normal3f + firstvertex;

			for (k = 0; k < mesh->num_xyz; k++, v += 3, n += 3)
			{
				unsigned char *ff = framevertstart[frameinfo->frames[j].offset] + meshverts[k].vertex * 13;
				mdo_vertex_t vertex;

				mdo_read_vertex(&ff, &vertex);

				v[0] = vertex.x;
				v[1] = vertex.y;
				v[2] = vertex.z;

				n[0] = anorms[vertex.lightnormalindex][0];
				n[1] = anorms[vertex.lightnormalindex][1];
				n[2] = anorms[vertex.lightnormalindex][2];
			}
		}
	}

	mem_free(framevertstart);

	*fptr = f;
}

bool_t model_mdo_load(void *filedata, size_t filesize, model_t *out_model, char **out_error)
{
	unsigned char *f = (unsigned char*)filedata;
	mem_pool_t *pool;
	model_t model;
	mesh_t *mesh;
	mdo_header_t header;
	int i, j;
	mdo_stvert_t *mdostverts;
	mdo_triangle_t *mdotriangles;
	meshvert_t *meshverts;
	float iwidth, iheight;

	if (out_error)
		*out_error = NULL;

	if (filesize < sizeof(mdo_header_t))
		return (void)(out_error && (*out_error = msprintf("wrong format"))), false;

	memcpy(header.ident, f, 4); f += 4;
	header.version            = mdo_read_int(&f);
	header.offsets[0]         = mdo_read_float(&f);
	header.offsets[1]         = mdo_read_float(&f);
	header.offsets[2]         = mdo_read_float(&f);
	header.bitmap_count       = mdo_read_int(&f);
	header.bitmap_width       = mdo_read_int(&f);
	header.bitmap_height      = mdo_read_int(&f);
	header.skin_vertex_count  = mdo_read_int(&f);
	header.triangle_count     = mdo_read_int(&f);
	header.frame_vertex_count = mdo_read_int(&f);
	header.frame_count        = mdo_read_int(&f);
	header.synctype           = mdo_read_int(&f);
	header.flags              = mdo_read_int(&f);
	header.global_palette     = mdo_read_int(&f);

/* validate header */
	if (memcmp(header.ident, "MDO_", 4) != 0)
		return (void)(out_error && (*out_error = msprintf("wrong format (not MDO_)"))), false;
	if (LittleLong(header.version) != 1)
		return (void)(out_error && (*out_error = msprintf("wrong format (version not 1)"))), false;

/* allocate memory pool so we don't have to clean things up by hand if we have
 * to return an error somewhere in the middle of this function... */
	pool = mem_create_pool();

/* create mesh */
	model.num_meshes = 1;
	model.meshes = (mesh_t*)mem_alloc(pool, sizeof(mesh_t));

	model.num_tags = 0;
	model.tags = NULL;

	mesh = &model.meshes[0];
	mesh_initialize(&model, mesh);

	mesh->name = mem_copystring(pool, "mdomesh");

/* load skins */
	if (!mdo_load_skins(&header, &model, pool, &f, out_error))
	{
		mem_free_pool(pool);
		return false;
	}

/* load skin vertices */
	mdostverts = (mdo_stvert_t*)mem_alloc(pool, sizeof(mdo_stvert_t) * header.skin_vertex_count);
	for (i = 0; i < header.skin_vertex_count; i++)
		mdo_read_stvert(&f, &mdostverts[i]);

/* load triangles */
	mdotriangles = (mdo_triangle_t*)mem_alloc(pool, sizeof(mdo_triangle_t) * header.triangle_count);
	for (i = 0; i < header.triangle_count; i++)
		mdo_read_triangle(&f, &mdotriangles[i]);

/* mesh the vertices to create a single array of vertices with texcoords and
 * normals (the MDO format, like MD2, stores separate arrays of vertices and
 * "skin vertices") */
	mesh->num_tris = header.triangle_count;
	mesh->triangle3i = (int*)mem_alloc(pool, sizeof(int) * mesh->num_tris * 3);

	mesh->num_xyz = 0;
	meshverts = (meshvert_t*)mem_alloc(pool, sizeof(meshvert_t) * mesh->num_tris * 3);
	for (i = 0; i < mesh->num_tris; i++)
	{
		for (j = 0; j < 3; j++)
		{
			int vertnum;
			meshvert_t *mv;

			int xyz = mdotriangles[i].vertindex[j];
			int st = mdotriangles[i].skinvertindex[j];

			int s = mdostverts[st].s;
			int t = mdostverts[st].t;
			int back = mdostverts[st].onseam && !mdotriangles[i].facesfront;

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

	mem_free(mdostverts);
	mem_free(mdotriangles);

/* generate texcoords */
/* note: the specific rounding and stuff is different from our mdl loader...
 * these values seem to better match the texture mapping in qme's renderer */
	mesh->texcoord2f = (float*)mem_alloc(pool, sizeof(float) * mesh->num_xyz * 2);
	iwidth = 1.0f / max(1, header.bitmap_width - 1);
	iheight = 1.0f / max(1, header.bitmap_height - 1);
	for (i = 0; i < mesh->num_xyz; i++)
	{
		float s = (float)bound(0, meshverts[i].s, header.bitmap_width - 1);
		float t = (float)bound(0, meshverts[i].t, header.bitmap_height - 1);

		if (meshverts[i].back)
			s += header.bitmap_width >> 1;

		mesh->texcoord2f[i*2+0] = s * iwidth;
		mesh->texcoord2f[i*2+1] = t * iheight;
	}

/* load frames */
	mdo_load_frames(&header, &model, meshverts, pool, &f);

/* done */
	mem_free(meshverts);

	mem_merge_pool(pool);

	model.flags = header.flags;
	model.synctype = header.synctype;
	model.offsets[0] = header.offsets[0];
	model.offsets[1] = header.offsets[1];
	model.offsets[2] = header.offsets[2];

	*out_model = model;
	return true;
}
