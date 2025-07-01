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

#include <SDL.h>

#ifdef WIN32
# include <windows.h>
#endif
#include <GL/gl.h>

#include "global.h"
#include "model.h"
#include "matrix.h"
#include "v_font.h"

int texwidth = -1; /* unused by viewer but needed by md2 exporter */
int texheight = -1;
const char *g_skinpath = NULL;

int vid_width = -1;
int vid_height = -1;

float time = 0.0f;
float oldtime = -0.1f;
float frametime = 0.1f;

float cam_pitch = 0.0f;
float cam_yaw = 180.0f;
float cam_dist = 64.0f;
float model_z = 0.0f;
mat4x4f_t viewmatrix;
mat4x4f_t invviewmatrix;

mat4x4f_t invmodelmatrix;

bool_t mousedown = false;
int mousedownx = 0;
int mousedowny = 0;

bool_t mouse2down = false;
int mouse2downx = 0;
int mouse2downy = 0;

bool_t texturefiltering = false;
bool_t firstperson = false;
bool_t nolerp = false;
bool_t wireframe = false;
int g_frame = -1;

bool_t showoverlay = false;

bool_t freezelighting = false;
vec3f_t g_lightpos;

int g_skin = 0;

model_t *model;

typedef struct r_state_s
{
	int max_numvertices;
	int max_numtriangles;

	float *vertex3f;
	float *normal3f;
	unsigned char *colour4ub;
	float *plane4f;
} r_state_t;

static r_state_t r_state;

void r_init(void);

static void frame(void)
{
	int x, y;

	SDL_GetMouseState(&x, &y);

	if (mousedown)
	{
		cam_yaw += (x - mousedownx);
		cam_pitch -= (y - mousedowny);
		mousedownx = x;
		mousedowny = y;
	}
	if (mouse2down)
	{
		model_z += (y - mouse2downy);
		mouse2downx = x;
		mouse2downy = y;
	}
}

static bool_t setvideomode_sdl(int width, int height, int bpp, bool_t fullscreen)
{
	SDL_Surface *window = SDL_GetVideoSurface();

/*	SDL_SetVideoMode(0, 0, 0, 0);*/ /* avoid SDL bugs when calling SDL_GL_SetAttribute */

	switch (bpp)
	{
	case 16:
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 1);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 1);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 1);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
		break;
	case 32:
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		break;
	default:
		return false;
	}

/*	if (window)
		SDL_FreeSurface(window);*/

	window = SDL_SetVideoMode(width, height, bpp, SDL_RESIZABLE | SDL_ANYFORMAT | SDL_OPENGL | (fullscreen ? SDL_FULLSCREEN : 0));
	if (!window)
		return false;

	vid_width = width;
	vid_height = height;

	return true;
}

static bool_t setvideomode(int width, int height, int bpp, bool_t fullscreen)
{
	if (!setvideomode_sdl(width, height, bpp, fullscreen))
	{
		fprintf(stderr, "Failed to set video mode %dx%d %dbpp (%s).\n", width, height, bpp, fullscreen ? "fullscreen" : "windowed");
		return false;
	}

	printf("Video mode set to %dx%d %dbpp (%s).\n", width, height, bpp, fullscreen ? "fullscreen" : "windowed");

	r_init();

	return true;
}

static void CHECKGLERROR_(const char *file, int line)
{
	GLenum glerror;
	if ((glerror = glGetError()))
	{
	/* *cough* */
#if defined(WIN32) && !defined(__MINGW32__)
		__asm int 3;
#else
		printf("CHECKGLERROR failed (0x%04x) on %s ln %d\n", glerror, file, line);
		*(int*)0 = 42;
#endif
	}
}
#define CHECKGLERROR() CHECKGLERROR_(__FILE__,__LINE__)

void r_init(void)
{
	int i, j, k;

	glClearColor(0, 0, 0, 0); CHECKGLERROR();
	glClearDepth(1.0f); CHECKGLERROR();

	glColor4f(1, 1, 1, 1); CHECKGLERROR();

	glCullFace(GL_BACK); CHECKGLERROR();
	glFrontFace(GL_CW); CHECKGLERROR();
	glEnable(GL_CULL_FACE); CHECKGLERROR();

	glDisable(GL_BLEND); CHECKGLERROR();

	glEnable(GL_DEPTH_TEST); CHECKGLERROR();
	glDepthFunc(GL_LESS); CHECKGLERROR();
	glDepthMask(GL_TRUE); CHECKGLERROR();

	glDisable(GL_TEXTURE_2D); CHECKGLERROR();

/* create and upload font */
	loadfont();

/* upload mesh textures */
	model_generaterenderdata(model);

	for (i = 0; i < model->num_meshes; i++)
	{
		mesh_t *mesh = &model->meshes[i];

		for (j = 0; j < model->total_skins; j++)
		{
			for (k = 0; k < SKIN_NUMTYPES; k++)
			{
				if (mesh->renderdata.skins[j].components[k].image)
				{
					glGenTextures(1, &mesh->renderdata.skins[j].components[k].handle); CHECKGLERROR();
					glBindTexture(GL_TEXTURE_2D, mesh->renderdata.skins[j].components[k].handle); CHECKGLERROR();
					glTexImage2D(GL_TEXTURE_2D, 0, 4, mesh->renderdata.skins[j].components[k].image->width, mesh->renderdata.skins[j].components[k].image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, mesh->renderdata.skins[j].components[k].image->pixels); CHECKGLERROR();
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texturefiltering ? GL_LINEAR : GL_NEAREST); CHECKGLERROR();
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texturefiltering ? GL_LINEAR : GL_NEAREST); CHECKGLERROR();
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); CHECKGLERROR();
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); CHECKGLERROR();
				}
			}
		}
	}
}

float anim_progress = 0.0f;

static struct
{
	int num_frames;

	struct
	{
		int offset;
		float frac;
	} frames[2];
} animblend;

static void calc_anim(void)
{
	if (g_frame != -1)
	{
	/* view an individual frame or framegroup */
		const frameinfo_t *frameinfo = &model->frameinfo[g_frame];

		if (frameinfo->num_frames > 1)
		{
		/* framegroup */
			if (nolerp)
			{
				int frame = (int)(anim_progress / frameinfo->frametime) % frameinfo->num_frames;

				animblend.num_frames = 1;
				animblend.frames[0].offset = frameinfo->frames[frame].offset;
				animblend.frames[0].frac = 1.0f;
			}
			else
			{
				float frame;
				int frame0, frame1;

				frame = anim_progress / frameinfo->frametime;
				frame -= (float)floor(frame / frameinfo->num_frames) * frameinfo->num_frames;

				frame0 = (int)floor(frame) % frameinfo->num_frames;
				frame1 = (frame0 + 1) % frameinfo->num_frames;

				animblend.num_frames = 2;
				animblend.frames[0].offset = frameinfo->frames[frame0].offset;
				animblend.frames[0].frac = 1.0f - (frame - (float)frame0);
				animblend.frames[1].offset = frameinfo->frames[frame1].offset;
				animblend.frames[1].frac = frame - (float)frame0;
			}
		}
		else
		{
		/* single frame */
			animblend.num_frames = 1;
			animblend.frames[0].offset = frameinfo->frames[0].offset;
			animblend.frames[0].frac = 1.0f;
		}
	}
	else
	{
	/* loop through all animation frames */
		if (nolerp)
		{
			int frame = (int)(anim_progress / 0.1f) % model->num_frames;

			animblend.num_frames = 1;
			animblend.frames[0].offset = model->frameinfo[frame].frames[0].offset;
			animblend.frames[0].frac = 1.0f;
		}
		else
		{
			float frame;
			int frame0, frame1;

			frame = anim_progress / 0.1f;
			frame -= (float)floor(frame / model->num_frames) * model->num_frames;

			frame0 = (int)floor(frame) % model->num_frames;
			frame1 = (frame0 + 1) % model->num_frames;

			animblend.num_frames = 2;
			animblend.frames[0].offset = model->frameinfo[frame0].frames[0].offset;
			animblend.frames[0].frac = 1.0f - (frame - (float)frame0);
			animblend.frames[1].offset = model->frameinfo[frame1].frames[0].offset;
			animblend.frames[1].frac = frame - (float)frame0;
		}
	}
}

static void animate_mesh(const mesh_t *mesh)
{
	if (animblend.num_frames == 1)
	{
		int offset = animblend.frames[0].offset;

		memcpy(r_state.vertex3f, mesh->vertex3f + offset * mesh->num_xyz * 3, mesh->num_xyz * sizeof(float[3]));
		memcpy(r_state.normal3f, mesh->normal3f + offset * mesh->num_xyz * 3, mesh->num_xyz * sizeof(float[3]));
	}
	if (animblend.num_frames == 2)
	{
		int offset0 = animblend.frames[0].offset;
		int offset1 = animblend.frames[1].offset;
		float frac0 = animblend.frames[0].frac;
		float frac1 = animblend.frames[1].frac;
		int i;

		for (i = 0; i < mesh->num_xyz; i++)
		{
			const float *v0 = mesh->vertex3f + (offset0 * mesh->num_xyz + i) * 3;
			const float *n0 = mesh->normal3f + (offset0 * mesh->num_xyz + i) * 3;
			const float *v1 = mesh->vertex3f + (offset1 * mesh->num_xyz + i) * 3;
			const float *n1 = mesh->normal3f + (offset1 * mesh->num_xyz + i) * 3;

			r_state.vertex3f[i*3+0] = v0[0] * frac0 + v1[0] * frac1;
			r_state.vertex3f[i*3+1] = v0[1] * frac0 + v1[1] * frac1;
			r_state.vertex3f[i*3+2] = v0[2] * frac0 + v1[2] * frac1;

			r_state.normal3f[i*3+0] = n0[0] * frac0 + n1[0] * frac1;
			r_state.normal3f[i*3+1] = n0[1] * frac0 + n1[1] * frac1;
			r_state.normal3f[i*3+2] = n0[2] * frac0 + n1[2] * frac1;

			/* TODO - renormalize? */
		}
	}
}

static void light_mesh(const mesh_t *mesh)
{
	float tlightpos[3], lightnormal[3], dot;
	const float *v, *n;
	int i;
	unsigned char *c;

	mat4x4f_transform(&invmodelmatrix, g_lightpos, tlightpos);

	for (i = 0, v = r_state.vertex3f, n = r_state.normal3f, c = r_state.colour4ub; i < mesh->num_xyz; i++, v += 3, n += 3, c += 4)
	{
		VectorSubtract(tlightpos, v, lightnormal);
		VectorNormalize(lightnormal);

		dot = DotProduct(n, lightnormal);

		if (dot < 0)
			dot = 0;

		c[0] = c[1] = c[2] = (unsigned char)(dot * 255);
		c[3] = 255;
	}
}

#if 0
static void calculate_planes(mesh_t *mesh)
{
	int i;
	int *tri;
	float *plane4f;

	for (i = 0, tri = mesh->triangle3i, plane4f = r_state.plane4f; i < mesh->num_tris; i++, tri += 3, plane4f += 4)
	{
		const float *p0 = r_state.vertex3f + tri[0] * 3;
		const float *p1 = r_state.vertex3f + tri[1] * 3;
		const float *p2 = r_state.vertex3f + tri[2] * 3;
		float edge01[3], edge21[3];

		VectorSubtract(p0, p1, edge01);
		VectorSubtract(p2, p1, edge21);
		CrossProduct(edge01, edge21, plane4f);

		plane4f[3] = DotProduct(p0, plane4f);
	}
}
#endif
static void draw_tags(float alpha)
{
	int i;

	glBegin(GL_LINES);
	for (i = 0; i < model->num_tags; i++)
	{
		mat4x4f_t m;
		float x, y, z;

		if (animblend.num_frames == 2)
		{
			int offset0 = animblend.frames[0].offset;
			int offset1 = animblend.frames[1].offset;
			float t = animblend.frames[1].frac;

			mat4x4f_blend(&m, &model->tags[i].matrix[offset0], &model->tags[i].matrix[offset1], t);
		}
		else
		{
			int offset = animblend.frames[0].offset;

			m = model->tags[i].matrix[offset];
		}

		x = m.m[0][3];
		y = m.m[1][3];
		z = m.m[2][3];

		glColor4f(1, 0, 0, alpha);
		glVertex3f(x, y, z);
		glVertex3f(x + m.m[0][0] * 8, y + m.m[1][0] * 8, z + m.m[2][0] * 8);

		glColor4f(0, 1, 0, alpha);
		glVertex3f(x, y, z);
		glVertex3f(x + m.m[0][1] * 8, y + m.m[1][1] * 8, z + m.m[2][1] * 8);

		glColor4f(0, 0, 1, alpha);
		glVertex3f(x, y, z);
		glVertex3f(x + m.m[0][2] * 8, y + m.m[1][2] * 8, z + m.m[2][2] * 8);
	}
	glEnd();

	glColor4f(1, 1, 1, 1);
}

static void calc_crap(float *xmin, float *xmax, float *ymin, float *ymax, float *znear, float *zfar)
{
	float w = (float)vid_width;
	float h = (float)vid_height;
	float fovy = 90;
	float pixelheight = 1.0f;
	*znear = 1.0f;
	*zfar = 1024.0f;
	*ymax = *znear * (float)tan(fovy * M_PI / 360.0f) * (3.0f / 4.0f);
	*xmax = *ymax * w / h / pixelheight;
	*ymin = -*ymax;
	*xmin = -*xmax;
}

static void gl_uploadmatrix(GLenum mode, const mat4x4f_t *m)
{
	mat4x4f_t transposed = *m;
	mat4x4f_transpose(&transposed);

	glMatrixMode(mode);
	glLoadMatrixf((GLfloat*)transposed.m);
}

static void setviewmatrix(const mat4x4f_t *m)
{
	mat4x4f_t t;

/* copy forward matrix */
	viewmatrix = *m;

/* create inverse matrix */
	mat4x4f_invert_simple(&invviewmatrix, &viewmatrix);

/* transform inverse matrix into screen coordinate space (+X = forward, +Y = left, +Z = up) */
	mat4x4f_create_rotate(&t, -90, 1, 0, 0);
	mat4x4f_concat_rotate(&t,  90, 0, 0, 1);
	mat4x4f_concat_with(&t, &invviewmatrix);
	invviewmatrix = t;
}

static void setmodelmatrix(const mat4x4f_t *modelmatrix)
{
	mat4x4f_t m;

	mat4x4f_concat(&m, &invviewmatrix, modelmatrix);

	gl_uploadmatrix(GL_MODELVIEW, &m);

	mat4x4f_invert_simple(&invmodelmatrix, modelmatrix);
}

static void render(void)
{
	int i;

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	{
		float xmin, xmax, ymin, ymax, znear, zfar;
		calc_crap(&xmin, &xmax, &ymin, &ymax, &znear, &zfar);
		glFrustum(xmin, xmax, ymin, ymax, znear, zfar);
	}

/* create view matrix */
	mat4x4f_create_translate(&viewmatrix, 0, 0, model_z);
	mat4x4f_concat_rotate(&viewmatrix, -cam_yaw, 0, 0, 1);
	mat4x4f_concat_rotate(&viewmatrix, -cam_pitch, 0, 1, 0);
	mat4x4f_concat_translate(&viewmatrix, -cam_dist, 0, 0);
	setviewmatrix(&viewmatrix);

	if (!freezelighting)
	{
		g_lightpos[0] = viewmatrix.m[0][3];
		g_lightpos[1] = viewmatrix.m[1][3];
		g_lightpos[2] = viewmatrix.m[2][3];
	}

/* upload modelview matrix */
	if (firstperson)
		setmodelmatrix(&viewmatrix);
	else
		setmodelmatrix(&mat4x4f_identity);

	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);
	}

	calc_anim();

	for (i = 0; i < model->num_meshes; i++)
	{
		const mesh_t *mesh = &model->meshes[i];

		unsigned int diffuse_handle = 0;
		unsigned int fullbright_handle = 0;
		const float *diffuse_texcoord2f = NULL;
		const float *fullbright_texcoord2f = NULL;

		if (g_skin >= 0 && g_skin < model->num_skins)
		{
			const skininfo_t *skininfo = &model->skininfo[g_skin];
			int sframe = (int)(anim_progress / skininfo->frametime) % skininfo->num_skins;
			int skinoffset = skininfo->skins[sframe].offset;

			diffuse_handle = mesh->renderdata.skins[skinoffset].components[SKIN_DIFFUSE].handle;
			fullbright_handle = mesh->renderdata.skins[skinoffset].components[SKIN_FULLBRIGHT].handle;
			diffuse_texcoord2f = mesh->renderdata.skins[skinoffset].components[SKIN_DIFFUSE].texcoord2f;
			fullbright_texcoord2f = mesh->renderdata.skins[skinoffset].components[SKIN_FULLBRIGHT].texcoord2f;
		}

		animate_mesh(mesh);

		if (!wireframe)
			light_mesh(mesh);

		glEnableClientState(GL_VERTEX_ARRAY); CHECKGLERROR();
		glVertexPointer(3, GL_FLOAT, sizeof(float[3]), r_state.vertex3f); CHECKGLERROR();

	/* draw diffuse pass */
		if (diffuse_handle && diffuse_texcoord2f && !wireframe)
		{
			glEnable(GL_TEXTURE_2D); CHECKGLERROR();
			glBindTexture(GL_TEXTURE_2D, diffuse_handle); CHECKGLERROR();

			glEnableClientState(GL_TEXTURE_COORD_ARRAY); CHECKGLERROR();
			glTexCoordPointer(2, GL_FLOAT, sizeof(float[2]), diffuse_texcoord2f); CHECKGLERROR();
		}
		else
		{
			glDisable(GL_TEXTURE_2D); CHECKGLERROR();
		}

		if (!wireframe)
		{
			glEnableClientState(GL_COLOR_ARRAY); CHECKGLERROR();
			glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(unsigned char[4]), r_state.colour4ub); CHECKGLERROR();
		}

		glDrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_INT, mesh->triangle3i); CHECKGLERROR(); /* glDrawElements must take GL_UNSIGNED_INT, GL_INT is not accepted... */

		if (!wireframe)
		{
			glDisableClientState(GL_COLOR_ARRAY); CHECKGLERROR();
		}
		if (diffuse_handle && diffuse_texcoord2f && !wireframe)
		{
			glDisableClientState(GL_TEXTURE_COORD_ARRAY); CHECKGLERROR();
		}

	/* draw fullbright pass (who needs multitexture!) */
		if (fullbright_handle && fullbright_texcoord2f && !wireframe)
		{
			glDepthFunc(GL_LEQUAL); CHECKGLERROR();
			glBlendFunc(GL_ONE, GL_ONE); CHECKGLERROR();
			glEnable(GL_BLEND); CHECKGLERROR();

			glEnable(GL_TEXTURE_2D); CHECKGLERROR();
			glBindTexture(GL_TEXTURE_2D, fullbright_handle); CHECKGLERROR();

			glEnableClientState(GL_TEXTURE_COORD_ARRAY); CHECKGLERROR();
			glTexCoordPointer(2, GL_FLOAT, sizeof(float[2]), fullbright_texcoord2f); CHECKGLERROR();

			glDrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_INT, mesh->triangle3i); CHECKGLERROR();

			glDisableClientState(GL_TEXTURE_COORD_ARRAY); CHECKGLERROR();

			glDisable(GL_BLEND); CHECKGLERROR();
			glDepthFunc(GL_LESS); CHECKGLERROR();
		}

	/* clean up */
		glDisableClientState(GL_VERTEX_ARRAY); CHECKGLERROR();
	}

	glDisable(GL_TEXTURE_2D);

	if (wireframe)
	{
		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

/* draw tags (draw them translucent when they are behind geometry) */
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	draw_tags(1.0f);

	glDepthFunc(GL_GEQUAL);

	draw_tags(0.1f);

	glDepthFunc(GL_LESS);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

/* draw grid */
	setmodelmatrix(&mat4x4f_identity);

	glColor4f(0, 0, 0.5f, 1);
	glBegin(GL_LINES);
	for (i = -16; i <= 16; i++)
	{
		glVertex3f(-256.0f, i * 16.0f, -24.0f);
		glVertex3f( 256.0f, i * 16.0f, -24.0f);
		glVertex3f(i * 16.0f, -256.0f, -24.0f);
		glVertex3f(i * 16.0f,  256.0f, -24.0f);
	}
	glEnd();
	glColor4f(1, 1, 1, 1);

/* set 2d matrix */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, vid_width, vid_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

/* draw 2d stuff */
	if (showoverlay)
	{
		drawstringf(0, 0, "Press F1 to hide this text");
		drawstringf(0, 12, "Skin %d of %d (press / to cycle)", g_skin + 1, model->num_skins);
	}

/* done */
	SDL_GL_SwapBuffers();
}

static bool_t replacetexture(const char *filename)
{
	char *error;
	image_rgba_t *image;
	int i;
	mesh_t *mesh;

	image = image_load_from_file(mem_globalpool, filename, &error);
	if (!image)
	{
		fprintf(stderr, "Failed to load %s: %s.\n", filename, error);
		qfree(error);
		return false;
	}

/* clear old skins */
	model_clear_skins(model);

/* add new skin */
	model->total_skins = 1;
	model->num_skins = 1;
	model->skininfo = (skininfo_t*)qmalloc(sizeof(skininfo_t));
	model->skininfo[0].frametime = 0.1f;
	model->skininfo[0].num_skins = 1;
	model->skininfo[0].skins = (singleskin_t*)qmalloc(sizeof(singleframe_t));
	model->skininfo[0].skins[0].name = copystring(filename);
	model->skininfo[0].skins[0].offset = 0;

	for (i = 0, mesh = model->meshes; i < model->num_meshes; i++, mesh++)
	{
		mesh->skins = (meshskin_t*)qmalloc(sizeof(meshskin_t));
		mesh->skins[0].components[SKIN_DIFFUSE] = image_clone(mem_globalpool, image);
		mesh->skins[0].components[SKIN_FULLBRIGHT] = NULL;
	}

	image_free(&image);
	return true;
}
#if 0
static void vandalize_skin(void)
{
	mesh_t *mesh;
	int i, j, k;

	for (i = 0, mesh = model->meshes; i < model->num_meshes; i++, mesh++)
	{
		for (j = 0; j < model->total_skins; j++)
		{
			image_rgba_t *image = mesh->skins[j].components[SKIN_DIFFUSE];

			if (image)
			{
				for (k = 0; k < mesh->num_tris; k++)
				{
					int s0 = (int)(mesh->texcoord2f[mesh->triangle3i[k*3+0]*2+0] * image->width);
					int t0 = (int)(mesh->texcoord2f[mesh->triangle3i[k*3+0]*2+1] * image->height);
					int s1 = (int)(mesh->texcoord2f[mesh->triangle3i[k*3+1]*2+0] * image->width);
					int t1 = (int)(mesh->texcoord2f[mesh->triangle3i[k*3+1]*2+1] * image->height);
					int s2 = (int)(mesh->texcoord2f[mesh->triangle3i[k*3+2]*2+0] * image->width);
					int t2 = (int)(mesh->texcoord2f[mesh->triangle3i[k*3+2]*2+1] * image->height);

					image_drawline(image, s0, t0, s1, t1, 255, 255, 255);
					image_drawline(image, s1, t1, s2, t2, 255, 255, 255);
					image_drawline(image, s2, t2, s0, t2, 255, 255, 255);
				}
			}
		}
	}
}
#endif
static void showfps(void)
{
	static float lastupdate = 0;
	static unsigned int numframes = 0;
	char s[64];

	numframes++;

	if (time - lastupdate >= 1.0f)
	{
		sprintf(s, "%d fps", numframes);
		SDL_WM_SetCaption(s, s);

		lastupdate = time;
		numframes = 0;
	}
}

int main(int argc, char **argv)
{
	char *error;
	bool_t done;
	int i;
	char texfilename[1024] = {0};
	char infilename[1024] = {0};
	int width = 640, height = 480;
	int bpp = 32;

	mem_init();

	set_atexit_final_event(mem_shutdown);
	atexit(call_atexit_events);

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (!strcmp(argv[i], "-tex"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-tex'\n", argv[0]);
					return 0;
				}

				strlcpy(texfilename, argv[i], sizeof(texfilename));
			}
			else if (!strcmp(argv[i], "-width"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-width'\n", argv[0]);
					return 0;
				}

				width = (int)atoi(argv[i]);

				if (width < 1 || width > 8192)
				{
					printf("%s: invalid value for option '-width'\n", argv[0]);
					return 0;
				}
			}
			else if (!strcmp(argv[i], "-height"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-height'\n", argv[0]);
					return 0;
				}

				height = (int)atoi(argv[i]);

				if (height < 1 || height > 8192)
				{
					printf("%s: invalid value for option '-height'\n", argv[0]);
					return 0;
				}
			}
			else if (!strcmp(argv[i], "-bpp"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-bpp'\n", argv[0]);
					return 0;
				}

				bpp = (int)atoi(argv[i]);

				if (bpp != 16 && bpp != 32)
				{
					printf("%s: invalid value for option '-bpp' (accepted values: 16, 32 (default))\n", argv[0]);
					return 0;
				}
			}
			else if (!strcmp(argv[i], "-frame"))
			{
				if (++i == argc)
				{
					printf("%s: missing argument for option '-frame'\n", argv[0]);
					return 0;
				}

				g_frame = (int)atoi(argv[i]);

				if (g_frame < 0) /* maximum bound will be checked after the model is loaded */
				{
					printf("%s: invalid value for option '-frame'\n", argv[0]);
					return 0;
				}
			}
			else if (!strcmp(argv[i], "-texturefiltering"))
			{
				texturefiltering = true;
			}
			else if (!strcmp(argv[i], "-firstperson"))
			{
				firstperson = true;
			}
			else if (!strcmp(argv[i], "-nolerp"))
			{
				nolerp = true;
			}
			else if (!strcmp(argv[i], "-wireframe"))
			{
				wireframe = true;
			}
			else
			{
				printf("%s: unrecognized option '%s'\n", argv[0], argv[i]);
				return 0;
			}
		}
		else
		{
			strlcpy(infilename, argv[i], sizeof(infilename));
		}
	}

	if (!infilename[0])
	{
		printf("usage: %s modelfile [-tex texturefile] [-width 640] [-height 480] [-bpp 16/32] [-frame #] [-texturefiltering] [-firstperson] [-nolerp] [-wireframe]\n", argv[0]);
		return 0;
	}

	model = model_load_from_file(infilename, &error);
	if (!model)
	{
		printf("Failed to load model: %s.\n", error);
		qfree(error);
		return 1;
	}

	printf("Loaded %s.\n", infilename);

	if (g_frame >= 0 && g_frame >= model->num_frames)
	{
		printf("%s: frame %d does not exist (model has %d frames)\n", argv[0], g_frame, model->num_frames);
		model_free(model);
		return 0;
	}

	if (texfilename[0])
	{
		if (!replacetexture(texfilename))
		{
			model_free(model);
			return 1;
		}
	}
#if 0
	vandalize_skin();
#endif
	r_state.max_numvertices = 0;
	r_state.max_numtriangles = 0;
	for (i = 0; i < model->num_meshes; i++)
	{
		if (model->meshes[i].num_xyz > r_state.max_numvertices)
			r_state.max_numvertices = model->meshes[i].num_xyz;
		if (model->meshes[i].num_tris > r_state.max_numtriangles)
			r_state.max_numtriangles = model->meshes[i].num_tris;
	}
	r_state.vertex3f = (float*)qmalloc(sizeof(float[3]) * r_state.max_numvertices);
	r_state.normal3f = (float*)qmalloc(sizeof(float[3]) * r_state.max_numvertices);
	r_state.colour4ub = (unsigned char*)qmalloc(sizeof(unsigned char[4]) * r_state.max_numvertices);
	r_state.plane4f = (float*)qmalloc(sizeof(float[4]) * r_state.max_numtriangles);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) < 0)
	{
		fprintf(stderr, "SDL failed to initialize: %s\n", SDL_GetError());
		qfree(r_state.vertex3f);
		qfree(r_state.normal3f);
		qfree(r_state.colour4ub);
		qfree(r_state.plane4f);
		model_free(model);
		return 1;
	}

	if (!setvideomode(width, height, bpp, false))
	{
		qfree(r_state.vertex3f);
		qfree(r_state.normal3f);
		qfree(r_state.colour4ub);
		qfree(r_state.plane4f);
		model_free(model);
		return 1;
	}

	for (done = false; !done; )
	{
		SDL_Event event;

		oldtime = time;
		time = SDL_GetTicks() * (1.0f / 1000.0f);
		frametime = time - oldtime;

		showfps();

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				done = true;
				break;
			case SDL_VIDEORESIZE:
				setvideomode(event.resize.w, event.resize.h, 32, false);
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					done = true;
				if (event.key.keysym.sym == SDLK_SPACE)
					freezelighting = !freezelighting;
				if (event.key.keysym.sym == SDLK_SLASH)
				{
					g_skin++;
					if (g_skin >= model->num_skins)
						g_skin = 0;
				}
				if (event.key.keysym.sym == SDLK_F1)
					showoverlay = !showoverlay;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					mousedown = true;
					mousedownx = event.button.x;
					mousedowny = event.button.y;
				}
				if (event.button.button == SDL_BUTTON_RIGHT)
				{
					mouse2down = true;
					mouse2downx = event.button.x;
					mouse2downy = event.button.y;
				}
				if (event.button.button == SDL_BUTTON_WHEELUP)
					cam_dist -= 8.0f;
				if (event.button.button == SDL_BUTTON_WHEELDOWN)
					cam_dist += 8.0f;
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					mousedown = false;
				}
				if (event.button.button == SDL_BUTTON_RIGHT)
				{
					mouse2down = false;
				}
				break;
			}
		}

		frame();

		render();

		anim_progress += frametime;
	}

	model_free(model);
	qfree(r_state.plane4f);
	qfree(r_state.colour4ub);
	qfree(r_state.normal3f);
	qfree(r_state.vertex3f);

	SDL_Quit();
	return 0;
}
