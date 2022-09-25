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
 */

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/common/header/shared.h"
#include "../src/common/header/files.h"
#include "../src/client/refresh/ref_shared.h"

/* GL command packet in memory */
struct mem_glcmd_t
{
	float s;
	float t;
	int index;
};

static GLuint skins[16];
static int skin_count = 0;

/* Palette */
static unsigned char colormap[256][3] = {
#include "colormap.h"
};

static void
insert_skin(const char *name, byte *data, int width, int realwidth,
	int height, int realheight, imagetype_t type, int bits)
{
	if (skin_count >= 16)
	{
		return;
	}

	int i;
	GLuint id;

	GLubyte *pixels = (GLubyte *)
		malloc (width * height * 3);

	/* Convert indexed 8 bits texture to RGB 24 bits */
	for (i = 0; i < width * height; ++i)
	{
		pixels[(i * 3) + 0] = colormap[data[i]][0];
		pixels[(i * 3) + 1] = colormap[data[i]][1];
		pixels[(i * 3) + 2] = colormap[data[i]][2];
	}

	/* Generate OpenGL texture */
	glGenTextures (1, &id);
	glBindTexture (GL_TEXTURE_2D, id);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps (GL_TEXTURE_2D, GL_RGB, width,
		height, GL_RGB, GL_UNSIGNED_BYTE,
		pixels);

	/* OpenGL has its own copy of image data */
	free (pixels);
	skins[skin_count] = id;
}

dmdl_t *mem_mod;
void *extradata;

/* Table of precalculated normals */
static vec3_t anorms_table[162] = {
#include "../src/client/refresh/constants/anorms.h"
};

static int iskin = 0, yrotate = -90, xrotate = -90;

static void
RenderPacket(float interp, struct mem_glcmd_t *packet,
	const daliasframe_t *pframe1, const daliasframe_t *pframe2)
{
	vec3_t v_curr, v_next, v, norm;
	const dtrivertx_t *pvert1, *pvert2;
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
RenderPacketWithFrame(const dmdl_t *mod, int n, float interp)
{
	int i;
	int *pglcmds = (int*)((byte*)mod + mod->ofs_glcmds);
	int num_frames = mod->num_frames;

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
			const daliasframe_t *pframe1, *pframe2;
			const byte *pframes;

			pframes = (byte*)mod + mod->ofs_frames;
			packet = (struct mem_glcmd_t *)pglcmds;
			pframe1 = (const daliasframe_t*)(pframes + mod->framesize * n);
			pframe2 = (const daliasframe_t*)(pframes + mod->framesize * ((n + 1) % num_frames));

			RenderPacket(interp, packet, pframe1, pframe2);
		}

		glEnd ();
	}
}

/**
 * Free resources allocated for the model.
 */
static void
FreeModel (dmdl_t *mod)
{
	free(mod);

	if (skin_count)
	{
		/* Delete OpenGL textures */
		glDeleteTextures (skin_count, skins);
	}
}

/**
 * Render the model with interpolation between frame n and n+1
 * using model's GL command list.
 * interp is the interpolation percent. (from 0.0 to 1.0)
 */
static void
RenderFrameItp (int n, float interp, const dmdl_t *mod)
{
	/* Check if n is in a valid range */
	if ((n < 0) || (n > mod->num_frames))
		return;

	/* Enable model's texture */
	if (skin_count)
	{
		glBindTexture (GL_TEXTURE_2D, skins[iskin % skin_count]);
	}

	/* pglcmds points at the start of the command list */
	RenderPacketWithFrame(mod, n, interp);
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

	mem_mod = Mod_LoadDMDL (filename, buffer, i, &extradata, insert_skin);
	if (mem_mod)
	{
		free(buffer);
		return;
	};

	exit (EXIT_FAILURE);
}

static void
cleanup ()
{
	FreeModel (mem_mod);
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
	Animate (0, mem_mod->num_frames - 1, &n, &interp);

	glTranslatef (0.0f, 0.0f, -100.0f);
	glRotatef (xrotate, 1.0, 0.0, 0.0);
	glRotatef (yrotate, 0.0, 0.0, 1.0);

	/* Draw the model */
	RenderFrameItp (n, interp, mem_mod);

	glutSwapBuffers ();
	glutPostRedisplay ();
}

static void
keyboard (unsigned char key, int x, int y)
{
	iskin += skin_count;
	xrotate += 360;
	yrotate += 360;

	switch (key)
	{
		case '+':
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

		case 27: /* escape */
			exit (0);
			break;
	}

	iskin %= skin_count;
	xrotate %= 360;
	yrotate %= 360;
}

int
main (int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf (stderr, "usage: %s <filename.md2>\n", argv[0]);
		return 0;
	}

	// quake swap prepere
	Swap_Init();

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
