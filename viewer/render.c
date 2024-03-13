#include <stdio.h>
#include <stdarg.h>
#include <GL/glut.h>
#include "render.h"

static GLuint skins[16];
int skin_count = 0, iskin = 0, yrotate = 0, xrotate = -90, zrotate = -90,
	imesh = 0, irender = 0, zdist = -150.0;
dmdx_t *mem_mod;

refimport_t ri;

void R_Printf(int level, const char* msg, ...)
{
	va_list argptr;
	va_start(argptr, msg);
	vprintf(msg, argptr);
	va_end(argptr);
}

void
Com_Printf (const char *msg, ...)
{
	va_list argptr;
	va_start(argptr, msg);
	vprintf(msg, argptr);
	va_end(argptr);
}

void
Sys_Error(const char *error, ...)
{
	va_list argptr;
	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);

	exit (0);
}

void
Com_Error(int error_code, const char *error, ...)
{
	va_list argptr;
	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);

	exit (0);
}

// don't need HDR stuff
#define STBI_NO_LINEAR
#define STBI_NO_HDR
// make sure STB_image uses standard malloc(), as we'll use standard free() to deallocate
#define STBI_MALLOC(sz)    malloc(sz)
#define STBI_REALLOC(p,sz) realloc(p,sz)
#define STBI_FREE(p)       free(p)
// Switch of the thread local stuff. Breaks mingw under Windows.
#define STBI_NO_THREAD_LOCALS
// include implementation part of stb_image into this file
#define STB_IMAGE_IMPLEMENTATION
#include "../src/client/refresh/files/stb_image.h"

byte *
image_load(const char* name, int *w, int *h, int *b)
{
	return stbi_load(name, w, h, b, STBI_rgb_alpha);
}

void
upload_texture(GLubyte *pixels, int width, int height)
{
	GLuint id;

	if (skin_count >= 16)
	{
		return;
	}

	/* Generate OpenGL texture */
	glGenTextures (1, &id);
	glBindTexture (GL_TEXTURE_2D, id);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps (GL_TEXTURE_2D, GL_RGBA, width,
		height, GL_RGBA, GL_UNSIGNED_BYTE,
		pixels);

	skins[skin_count] = id;

	skin_count ++;
}

/* Palette */
static unsigned char colormap[256][3] = {
#include "colormap.h"
};

struct image_s *
insert_skin(const char *name, byte *data, int width, int realwidth,
	int height, int realheight, size_t size, imagetype_t type, int bits)
{
	int i;

	GLubyte *pixels = (GLubyte *)malloc(width * height * 4);

	/* Convert indexed 8 bits texture to RGB 24 bits */
	for (i = 0; i < width * height; ++i)
	{
		pixels[(i * 4) + 0] = colormap[data[i]][0];
		pixels[(i * 4) + 1] = colormap[data[i]][1];
		pixels[(i * 4) + 2] = colormap[data[i]][2];
		pixels[(i * 4) + 2] = 0;
	}

	upload_texture(pixels, width, height);

	/* OpenGL has its own copy of image data */
	free(pixels);

	return NULL;
}

/* Table of precalculated normals */
static const vec3_t anorms_table[162] = {
#include "../src/client/refresh/constants/anorms.h"
};

static void
RenderPacket(float interp, mem_glcmd_t *packet,
	const daliasxframe_t *pframe1, const daliasxframe_t *pframe2)
{
	vec3_t v_curr, v_next, v, norm;
	const dxtrivertx_t *pvert1, *pvert2;
	const float *n_curr, *n_next;

	pvert1 = &pframe1->verts[packet->index];
	pvert2 = &pframe2->verts[packet->index];

	/* Pass texture coordinates to OpenGL */
	glTexCoord2f (packet->s, packet->t);

	/* Interpolate normals */
	n_curr = anorms_table[pvert1->lightnormalindex];
	n_next = anorms_table[pvert2->lightnormalindex];

	norm[0] = n_curr[0] + interp * (n_next[0] - n_curr[0]);
	norm[1] = n_curr[1] + interp * (n_next[1] - n_curr[1]);
	norm[2] = n_curr[2] + interp * (n_next[2] - n_curr[2]);

	glNormal3fv (norm);

	/* Interpolate vertices */
	v_curr[0] = pframe1->scale[0] * pvert1->v[0] + pframe1->translate[0];
	v_curr[1] = pframe1->scale[1] * pvert1->v[1] + pframe1->translate[1];
	v_curr[2] = pframe1->scale[2] * pvert1->v[2] + pframe1->translate[2];

	v_next[0] = pframe2->scale[0] * pvert2->v[0] + pframe2->translate[0];
	v_next[1] = pframe2->scale[1] * pvert2->v[1] + pframe2->translate[1];
	v_next[2] = pframe2->scale[2] * pvert2->v[2] + pframe2->translate[2];

	v[0] = v_curr[0] + interp * (v_next[0] - v_curr[0]);
	v[1] = v_curr[1] + interp * (v_next[1] - v_curr[1]);
	v[2] = v_curr[2] + interp * (v_next[2] - v_curr[2]);

	glVertex3fv(v);
}

static void
RenderPacketWithFrame(const dmdx_t *mod, int n, float interp, int *pglcmds)
{
	int i;
	int num_frames = mod->num_frames;

	/* Draw the model */
	while ((i = *(pglcmds++)) != 0)
	{
		if (i < 0)
		{
			glBegin(GL_TRIANGLE_FAN);
			i = -i;
		}
		else
		{
			glBegin(GL_TRIANGLE_STRIP);
		}

		/* Draw each vertex of this group */
		for (/* Nothing */; i > 0; --i, pglcmds += 3)
		{
			mem_glcmd_t *packet;
			const daliasxframe_t *pframe1, *pframe2;
			const byte *pframes;

			pframes = (byte*)mod + mod->ofs_frames;
			packet = (mem_glcmd_t *)pglcmds;
			pframe1 = (const daliasxframe_t*)(pframes + mod->framesize * n);
			pframe2 = (const daliasxframe_t*)(pframes + mod->framesize * ((n + 1) % num_frames));

			RenderPacket(interp, packet, pframe1, pframe2);
		}

		glEnd ();
	}
}

/**
 * Free resources allocated for the model.
 */
static void
FreeModel(dmdx_t *mod)
{
	Hunk_Free(mod);

	if (skin_count)
	{
		/* Delete OpenGL textures */
		glDeleteTextures(skin_count, skins);
	}
}

/**
 * Render the model with interpolation between frame n and n+1
 * using model's GL command list.
 * interp is the interpolation percent. (from 0.0 to 1.0)
 */
void
RenderFrameItp(int n, float interp, const dmdx_t *mod)
{
	int num_mesh_nodes, i;
	dmdxmesh_t *mesh_nodes;

	n += mod->num_frames;
	n %= mod->num_frames;

	/* Enable model's texture */
	if (skin_count)
	{
		glBindTexture(GL_TEXTURE_2D, skins[iskin % skin_count]);
	}

	irender %= 2;

	num_mesh_nodes = mod->num_meshes;
	mesh_nodes = (dmdxmesh_t *)((char*)mod + mod->ofs_meshes);

	imesh = (imesh + num_mesh_nodes + 1) % (num_mesh_nodes + 1);
	for (i = 0; i < num_mesh_nodes; i++)
	{
		if (imesh == 0 || (i == (num_mesh_nodes - imesh)))
		{

			if (!irender)
			{
				int *pglcmds;

				pglcmds = (int*)((byte*)mod + mod->ofs_glcmds);

				/* pglcmds points at the start of the command list */
				RenderPacketWithFrame(mod, n, interp,
					pglcmds + mesh_nodes[i].ofs_glcmds);
			}
			else
			{
				const daliasxframe_t *pframe1, *pframe2;
				dtriangle_t *ptri;
				dstvert_t *pstverts;

				const byte *pframes;
				int k, tri_max;

				pframes = (byte*)mod + mod->ofs_frames;
				pframe1 = (const daliasxframe_t*)(pframes + mod->framesize * n);
				pframe2 = (const daliasxframe_t*)(pframes + mod->framesize * ((n + 1) % mod->num_frames));
				ptri = (dtriangle_t *)((byte *)mod + mod->ofs_tris);
				pstverts = (dstvert_t *)((byte *)mod + mod->ofs_st);

				tri_max = mesh_nodes[i].ofs_tris + mesh_nodes[i].num_tris;

				for (k = mesh_nodes[i].ofs_tris; k < tri_max; ++k)
				{
					int j;

					glBegin(GL_TRIANGLE_FAN);

					for (j = 0; j < 3; j++)
					{
						mem_glcmd_t packet;

						packet.index = ptri[k].index_xyz[j];
						packet.s = (float)pstverts[ptri[k].index_st[j]].s / mod->skinwidth;
						packet.t = (float)pstverts[ptri[k].index_st[j]].t / mod->skinheight;

						RenderPacket(interp, &packet, pframe1, pframe2);
					}

					glEnd ();
				}
			}
		}
	}
}

/**
 * Calculate the current frame in animation beginning at frame
 * 'start' and ending at frame 'end', given interpolation percent.
 * interp will be reseted to 0.0 if the next frame is reached.
 */
void
Animate(int start, int end, int *frame, float *interp)
{
	if ((*frame < start) || (*frame > end))
		*frame = start;

	if (*interp >= 1.0f)
	{
		/* Move to next frame */
		*interp = 0.0f;
		(*frame)++;

		if (*frame >= end)
			*frame = start;
	}
}

struct image_s*
find_image(const char *name, imagetype_t type)
{
	return NULL;
}

void
cleanup(void)
{
	FreeModel(mem_mod);
}

void
reshape(int w, int h)
{
	if (h == 0)
	{
		h = 1;
	}

	glViewport (0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (45.0, w/(GLdouble)h, 0.1, 1000.0);

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

void
keyboard(unsigned char key, int x, int y)
{
	iskin += skin_count;
	xrotate += 360;
	yrotate += 360;
	zrotate += 360;

	switch (key)
	{
		case '0':
			imesh++;
			break;

		case '=':
			iskin++;
			break;

		case '-':
			iskin--;
			break;

		case 'w':
			xrotate += 5;
			break;

		case 's':
			xrotate -= 5;
			break;

		case 'd':
			yrotate += 5;
			break;

		case 'a':
			yrotate -= 5;
			break;

		case 'z':
			zrotate += 5;
			break;

		case 'c':
			zrotate -= 5;
			break;

		case 'x':
			irender++;
			break;

		case 'v':
			zdist -= 5;
			break;

		case 'b':
			zdist += 5;
			break;


		case 27: /* escape */
			exit (0);
			break;
	}

	iskin %= skin_count;
	xrotate %= 360;
	yrotate %= 360;
	zrotate %= 360;
}
