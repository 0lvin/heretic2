/*
 * md2.c -- md2 model loader
 * last modification: aug. 14, 2007
 *
 * Copyright (c) 2005-2007 David HENRY
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * gcc -Wall -ansi -lGL -lGLU -lglut md2.c -o md2
 */

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

/* Vector */
typedef float vec3_t[3];

#include "../src/common/header/files.h"
/* Model frame in memory */
struct mem_frame_t
{
	vec3_t scale;
	vec3_t translate;
	char name[16];
	dtrivertx_t *verts;  /* vertex list of the frame */
};

/* GL command packet in memory */
struct mem_glcmd_t
{
	float s;
	float t;
	int index;
};

/* Table of precalculated normals */
static vec3_t anorms_table[162] = {
#include "../src/client/refresh/constants/anorms.h"
};

static void
RenderPacket(float interp, struct mem_glcmd_t *packet,
	struct mem_frame_t *pframe1, struct mem_frame_t *pframe2)
{
	vec3_t v_curr, v_next, v, norm;
	dtrivertx_t *pvert1, *pvert2;
	float *n_curr, *n_next;

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

	glVertex3fv (v);
}

static void
RenderPacketWithFrame(int *pglcmds, struct mem_frame_t *frames, int n,
	int num_frames, float interp)
{
	int i;

	/* Draw the model */
	while ((i = *(pglcmds++)) != 0)
	{
		if (i < 0)
		{
			glBegin (GL_TRIANGLE_FAN);
			i = -i;
		}
		else
		{
			glBegin (GL_TRIANGLE_STRIP);
		}

		/* Draw each vertex of this group */
		for (/* Nothing */; i > 0; --i, pglcmds += 3)
		{
			struct mem_glcmd_t *packet;
			struct mem_frame_t *pframe1, *pframe2;

			packet = (struct mem_glcmd_t *)pglcmds;
			pframe1 = &frames[n];
			pframe2 = &frames[(n + 1) % num_frames];

			RenderPacket(interp, packet, pframe1, pframe2);
		}

		glEnd ();
	}
}

/* Texture name in memory */
struct mem_skin_t
{
	char name[64]; /* texture file name */
	GLubyte *data;  /* texture data */
};

/* MD2 model structure in memory */
struct mem_model_t
{
	struct mem_skin_t *skins;
	struct mem_frame_t *frames;
	int *glcmds;

	GLuint *tex_id;
	int iskin;
	int num_frames;
	int num_skins;
};

/* Palette */
static unsigned char colormap[256][3] = {
#include "colormap.h"
};

/*** An MD2 model ***/
static struct mem_model_t mmdfile;


/**
 * Make a texture given a skin index 'n'.
 */
static GLuint
MakeTextureFromSkin (int n, const struct mem_model_t *mdl,
	struct mdl_header_t *header)
{
	int i;
	GLuint id;

	GLubyte *pixels = (GLubyte *)
		malloc (header->skinwidth * header->skinheight * 3);

	/* Convert indexed 8 bits texture to RGB 24 bits */
	for (i = 0; i < header->skinwidth * header->skinheight; ++i)
	{
		pixels[(i * 3) + 0] = colormap[mdl->skins[n].data[i]][0];
		pixels[(i * 3) + 1] = colormap[mdl->skins[n].data[i]][1];
		pixels[(i * 3) + 2] = colormap[mdl->skins[n].data[i]][2];
	}

	/* Generate OpenGL texture */
	glGenTextures (1, &id);
	glBindTexture (GL_TEXTURE_2D, id);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps (GL_TEXTURE_2D, GL_RGB, header->skinwidth,
		header->skinheight, GL_RGB, GL_UNSIGNED_BYTE,
		pixels);

	/* OpenGL has its own copy of image data */
	free (pixels);
	return id;
}

/**
 * Load an MDL model from file.
 *
 * Note: MDL format stores model's data in little-endian ordering.  On
 * big-endian machines, you'll have to perform proper conversions.
 */
static int
ReadMDLModel (const byte *buffer, struct mem_model_t *mdl)
{
	int i;
	const byte *curr_pos;
	struct mdl_triangle_t *triangles;
	struct mdl_texcoord_t *texcoords;
	struct mdl_header_t *header;

	/* Read header */
	header = (struct mdl_header_t *)buffer;

	if ((header->ident != IDMDLHEADER) ||
		(header->version != MDL_VERSION))
	{
		/* Error! */
		fprintf (stderr, "Error: bad version or identifier\n");
		return 0;
	}

	/* Memory allocations */
	mdl->skins = (struct mem_skin_t *)
		malloc (sizeof (struct mem_skin_t) * header->num_skins);
	mdl->frames = (struct mem_frame_t *)
		malloc (sizeof (struct mem_frame_t) * header->num_frames);
	mdl->tex_id = (GLuint *)
		malloc (sizeof (GLuint) * header->num_skins);
	mdl->glcmds = (int *)
		malloc (
		(3 * header->num_tris) * sizeof(struct mem_glcmd_t) + /* 3 vert */
		(header->num_tris * sizeof(int)) + /* triangles count */
		sizeof(int) /* final zero */);

	mdl->iskin = 0;

	memset(mdl->skins, 0, sizeof (struct mem_skin_t) * header->num_skins);

	curr_pos = buffer + sizeof (struct mdl_header_t);

	/* Read texture data */
	for (i = 0; i < header->num_skins; ++i)
	{
		int skin_type;

		mdl->skins[i].data = (GLubyte *)malloc (sizeof (GLubyte)
			* header->skinwidth * header->skinheight);

		/* skip type / int */
		/* 0 = simple, !0 = group */
		/* this program can't read models composed of group frames! */
		memcpy(&skin_type, curr_pos, sizeof (skin_type));
		curr_pos += sizeof (skin_type);

		if (skin_type)
		{
			printf("unsupported skin type %d\n", skin_type);
			return 0;
		}

		memcpy(mdl->skins[i].data, curr_pos,
			header->skinwidth * header->skinheight);
		curr_pos += header->skinwidth * header->skinheight;

		mdl->tex_id[i] = MakeTextureFromSkin (i, mdl, header);

		free (mdl->skins[i].data);
		mdl->skins[i].data = NULL;
	}
	mdl->num_skins = header->num_skins;

	texcoords = (struct mdl_texcoord_t*)curr_pos;
	curr_pos += sizeof (struct mdl_texcoord_t) * header->num_xyz;

	triangles = (struct mdl_triangle_t*)curr_pos;
	curr_pos += sizeof (struct mdl_triangle_t) * header->num_tris;

	/* Read frames */
	for (i = 0; i < header->num_frames; ++i)
	{
		int frame_type;

		/* Memory allocation for vertices of this frame */
		mdl->frames[i].verts = (dtrivertx_t *)
			malloc (sizeof (dtrivertx_t) * header->num_xyz);

		mdl->frames[i].scale[0] = header->scale[0];
		mdl->frames[i].scale[1] = header->scale[1];
		mdl->frames[i].scale[2] = header->scale[2];

		mdl->frames[i].translate[0] = header->translate[0];
		mdl->frames[i].translate[1] = header->translate[1];
		mdl->frames[i].translate[2] = header->translate[2];

		/* Read frame data */
		/* skip type / int */
		/* 0 = simple, !0 = group */
		/* this program can't read models composed of group frames! */
		memcpy(&frame_type, curr_pos, sizeof (frame_type));
		curr_pos += sizeof (frame_type);

		if (frame_type)
		{
			printf("unsupported frame type %d\n", frame_type);
			return 0;
		}
		/* skip bboxmin, bouding box min */
		curr_pos += sizeof(dtrivertx_t);
		/* skip bboxmax, bouding box max */
		curr_pos += sizeof(dtrivertx_t);

		memcpy(mdl->frames[i].name, curr_pos, sizeof (char) * 16);
		curr_pos += sizeof (char) * 16;

		memcpy(mdl->frames[i].verts, curr_pos,
			sizeof (dtrivertx_t) * header->num_xyz);
		curr_pos += sizeof (dtrivertx_t) * header->num_xyz;
	}
	mdl->num_frames = header->num_frames;

	int j, *curr_com = mdl->glcmds;

	/* Draw each triangle */
	for (i = 0; i < header->num_tris; ++i)
	{
		*curr_com = 3;
		curr_com++;

		/* Draw each vertex */
		for (j = 0; j < 3; ++j)
		{
			struct mem_glcmd_t packet;

			packet.index = triangles[i].vertex[j];

			/* Compute texture coordinates */
			packet.s = texcoords[packet.index].s;
			packet.t = texcoords[packet.index].t;

			if (!triangles[i].facesfront &&
				texcoords[packet.index].onseam)
			{
				packet.s += header->skinwidth * 0.5f; /* Backface */
			}

			/* Scale s and t to range from 0.0 to 1.0 */
			packet.s = (packet.s + 0.5) / header->skinwidth;
			packet.t = (packet.t + 0.5) / header->skinheight;

			memcpy(curr_com, &packet, sizeof(struct mem_glcmd_t));
			curr_com  += (sizeof(struct mem_glcmd_t) / sizeof(int));
		}
	}

	*curr_com = 0;
	curr_com++;

	return 1;
}

/**
 * Load an MD2 model from file.
 *
 * Note: MD2 format stores model's data in little-endian ordering.	On
 * big-endian machines, you'll have to perform proper conversions.
 */
static int
ReadMD2Model (const byte *buffer, struct mem_model_t *mdl)
{
	int i;
	const byte *curr_pos;
	struct md2_header_t *header;

	/* Read header */
	header = (struct md2_header_t *)buffer;

	if ((header->ident != IDALIASHEADER) ||
		(header->version != ALIAS_VERSION))
	{
		/* Error! */
		fprintf (stderr, "Error: bad version or identifier\n");
		return 0;
	}

	/* Memory allocations */
	mdl->skins = (struct mem_skin_t *)
		malloc (sizeof (struct mem_skin_t) * header->num_skins);
	mdl->frames = (struct mem_frame_t *)
		malloc (sizeof (struct mem_frame_t) * header->num_frames);
	mdl->tex_id = (GLuint *)
		malloc (sizeof (GLuint) * header->num_skins);
	mdl->glcmds = (int *)malloc (sizeof (int) * header->num_glcmds);

	/* Read model data */
	memcpy(mdl->skins, buffer + header->ofs_skins,
		MAX_SKINNAME * header->num_skins);

	for(i=0; i<header->num_skins; i++)
	{
		char names[MAX_SKINNAME + 1];

		strncpy(names, mdl->skins[i].name, MAX_SKINNAME);
		printf("skin%d: %s\n", i, names);
	}
	mdl->num_frames = header->num_skins;

	memcpy(mdl->glcmds, buffer + header->ofs_glcmds,
		sizeof (int) * header->num_glcmds);

	/* Read frames */
	curr_pos = buffer + header->ofs_frames;
	for (i = 0; i < header->num_frames; ++i)
	{
		/* Memory allocation for vertices of this frame */
		mdl->frames[i].verts = (dtrivertx_t *)
			malloc (sizeof (dtrivertx_t) * header->num_xyz);

		/* Read frame data */
		memcpy(mdl->frames[i].scale, curr_pos, sizeof (vec3_t));
		curr_pos += sizeof (vec3_t);
		memcpy(mdl->frames[i].translate, curr_pos, sizeof (vec3_t));
		curr_pos += sizeof (vec3_t);
		memcpy(mdl->frames[i].name, curr_pos, 16);
		curr_pos += 16;
		memcpy(mdl->frames[i].verts, curr_pos,
			sizeof (dtrivertx_t) * header->num_xyz);
		curr_pos += sizeof (dtrivertx_t) * header->num_xyz;
	}
	mdl->num_frames = header->num_frames;

	return 1;
}

/**
 * Free resources allocated for the model.
 */
static void
FreeModel (struct mem_model_t *mdl)
{
	if (mdl->skins)
	{
		free (mdl->skins);
		mdl->skins = NULL;
	}

	if (mdl->glcmds)
	{
		free (mdl->glcmds);
		mdl->glcmds = NULL;
	}

	if (mdl->tex_id)
	{
		/* Delete OpenGL textures */
		glDeleteTextures (mdl->num_skins, mdl->tex_id);

		free (mdl->tex_id);
		mdl->tex_id = NULL;
	}

	if (mdl->frames)
	{
		int i;

		for (i = 0; i < mdl->num_frames; ++i)
		{
			free (mdl->frames[i].verts);
			mdl->frames[i].verts = NULL;
		}

		free (mdl->frames);
		mdl->frames = NULL;
	}
}

/**
 * Render the model with interpolation between frame n and n+1
 * using model's GL command list.
 * interp is the interpolation percent. (from 0.0 to 1.0)
 */
static void
RenderFrameItp (int n, float interp, const struct mem_model_t *mdl)
{
	/* Check if n is in a valid range */
	if ((n < 0) || (n > mdl->num_frames))
		return;

	/* Enable model's texture */
	if (mdl->num_skins)
	{
		glBindTexture (GL_TEXTURE_2D, mdl->tex_id[mdl->iskin % mdl->num_skins]);
	}

	/* pglcmds points at the start of the command list */
	RenderPacketWithFrame(mdl->glcmds, mdl->frames, n,
		mdl->num_frames, interp);
}

/**
 * Calculate the current frame in animation beginning at frame
 * 'start' and ending at frame 'end', given interpolation percent.
 * interp will be reseted to 0.0 if the next frame is reached.
 */
static void
Animate (int start, int end, int *frame, float *interp)
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

static void
init (const char *filename)
{
	FILE *fp;
	byte *buffer;
	int i;

	GLfloat lightpos[] = { 5.0f, 10.0f, 0.0f, 1.0f };

	/* Initialize OpenGL context */
	glClearColor (0.5f, 0.5f, 0.5f, 1.0f);
	glShadeModel (GL_SMOOTH);

	glEnable (GL_DEPTH_TEST);
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_LIGHTING);
	glEnable (GL_LIGHT0);

	glLightfv (GL_LIGHT0, GL_POSITION, lightpos);


	fp = fopen (filename, "rb");
	if (!fp)
	{
		fprintf (stderr, "Error: couldn't open \"%s\"!\n", filename);
		exit (EXIT_FAILURE);
	}

	fseek (fp, 0, SEEK_END);
	i = ftell(fp);
	buffer = malloc(i);
	fseek (fp, 0, SEEK_SET);
	fread(buffer, i, 1, fp);
	fclose(fp);

	switch (*(unsigned *)buffer)
	{
		/* Load MD2 model file */
		case IDMDLHEADER:
			if (ReadMDLModel (buffer, &mmdfile))
			{
				free(buffer);
				return;
			};
			break;
		/* Load MDL model file */
		case IDALIASHEADER:
			if (ReadMD2Model (buffer, &mmdfile))
			{
				free(buffer);
				return;
			};
			break;
	}

	exit (EXIT_FAILURE);
}

static void
cleanup ()
{
	FreeModel (&mmdfile);
}

static void
reshape (int w, int h)
{
	if (h == 0)
		h = 1;

	glViewport (0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (45.0, w/(GLdouble)h, 0.1, 1000.0);

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

static void
display ()
{
	static int n = 0; /* The current frame */
	static float interp = 0.0;
	static double curent_time = 0;
	static double last_time = 0;

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity ();

	last_time = curent_time;
	curent_time = (double)glutGet (GLUT_ELAPSED_TIME) / 1000.0;

	/* Animate model from frames 0 to num_frames-1 */
	interp += 10 * (curent_time - last_time);
	Animate (0, mmdfile.num_frames - 1, &n, &interp);

	glTranslatef (0.0f, 0.0f, -100.0f);
	glRotatef (-90.0f, 1.0, 0.0, 0.0);
	glRotatef (-90.0f, 0.0, 0.0, 1.0);

	/* Draw the model */
	RenderFrameItp (n, interp, &mmdfile);

	glutSwapBuffers ();
	glutPostRedisplay ();
}

static void
keyboard (unsigned char key, int x, int y)
{
	mmdfile.iskin += mmdfile.num_skins;

	switch (key)
	{
		case '+':
			mmdfile.iskin++;
			break;

		case '-':
			mmdfile.iskin--;
			break;

		case 27: /* escape */
			exit (0);
			break;
	}

	mmdfile.iskin %= mmdfile.num_skins;

	if (mmdfile.num_skins)
	{
		printf("Selected skin %d: %s\n", mmdfile.iskin, mmdfile.skins[mmdfile.iskin].name);
	}
}

int
main (int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf (stderr, "usage: %s <filename.md2>\n", argv[0]);
		return 0;
	}

	glutInit (&argc, argv);
	glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize (640, 480);
	glutCreateWindow ("MD2/MDL Model");

	atexit (cleanup);
	init (argv[1]);

	glutReshapeFunc (reshape);
	glutDisplayFunc (display);
	glutKeyboardFunc (keyboard);

	glutMainLoop ();

	return 0;
}
