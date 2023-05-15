/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * Drawing of all images that are not textures
 *
 * =======================================================================
 */

#include "header/local.h"

image_t *draw_chars;
image_t		*atlas_particle;
image_t		*atlas_aparticle;

extern qboolean scrap_dirty;
void Scrap_Upload(void);

extern unsigned r_rawpalette[256];

void
Draw_InitLocal(void)
{
	/* load console characters */
	draw_chars = R_FindPic("conchars", (findimage_t)R_FindImage);
	if (!draw_chars)
	{
		ri.Sys_Error(ERR_FATAL, "%s: Couldn't load pics/conchars.pcx",
			__func__);
	}
// jmarshall
	atlas_particle = R_FindImage("pics/misc/particle.m32", it_pic);
	atlas_aparticle = R_FindImage("pics/misc/aparticle.m8", it_pic);
// jmarshall end
	R_Bind(draw_chars->texnum);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

/*
 * Draws one 8*8 graphics character with 0 being transparent.
 * It can be clipped to the top of the screen to allow the console to be
 * smoothly scrolled off.
 */
void
Draw_Char(int x, int y, int num)
{
	int row, col;
	float frow, fcol, size;

	num &= 255;

	if ((num & 127) == 32)
	{
		return; /* space */
	}

	if (y <= -8)
	{
		return; /* totally off screen */
	}

	row = num >> 4;
	col = num & 15;

	frow = row * 0.0625;
	fcol = col * 0.0625;
	size = 0.0625;

	R_Bind (draw_chars->texnum);

	glBegin (GL_QUADS);
	glTexCoord2f (fcol, frow);
	glVertex2f (x, y);
	glTexCoord2f (fcol + size, frow);
	glVertex2f (x+8, y);
	glTexCoord2f (fcol + size, frow + size);
	glVertex2f (x+8, y+8);
	glTexCoord2f (fcol, frow + size);
	glVertex2f (x, y+8);
	glEnd ();
}

image_t *
RDraw_FindPic(char *name)
{
	image_t *gl;
	char	fullname[MAX_QPATH];

	if (name[0] != '/' && name[0] != '\\')
	{
		Com_sprintf (fullname, sizeof(fullname), "pics/%s", name);
		gl = R_FindImage (fullname, it_pic);
	}
	else
		gl = R_FindImage (name+1, it_pic);

	return gl;
}

void
RDraw_GetPicSize(int *w, int *h, char *pic)
{
	image_t *gl;

	gl = RDraw_FindPic (pic);
	if (!gl)
	{
		*w = *h = -1;
		return;
	}

	*w = gl->width;
	*h = gl->height;
}

void
Draw_Image(int x, int y, int w, int h, float alpha, qboolean scale, image_t *gl)
{
	if (scale)
	{
		float scaled_width, scaled_height, scaled_x, scaled_y;

		scaled_x = x * ((int)vid.width) / 640;
		scaled_width = ((int)vid.width) * (x + w) / 640 - scaled_x;
		scaled_y = y * ((int)vid.height) / 480;
		scaled_height = ((int)vid.height) * (y + h) / 480 - scaled_y;

		w = scaled_width;
		h = scaled_height;

		x = scaled_x;
		y = scaled_y;
	}

	if (scrap_dirty)
	{
		Scrap_Upload();
	}

	R_Bind(gl->texnum);
	glBegin(GL_QUADS);
	glTexCoord2f(gl->sl, gl->tl);
	glVertex2f(x, y);
	glTexCoord2f(gl->sh, gl->tl);
	glVertex2f(x + w, y);
	glTexCoord2f(gl->sh, gl->th);
	glVertex2f(x + w, y + h);
	glTexCoord2f(gl->sl, gl->th);
	glVertex2f(x, y + h);
	glEnd();
}

void
Draw_StretchPic(int x, int y, int w, int h, char *pic, float alpha, qboolean scale)
{
	image_t *gl;

	gl = RDraw_FindPic (pic);
	if (!gl)
	{
		//ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	Draw_Image(x, y, w, h, alpha, scale, gl);
}

void
Draw_Pic(int x, int y, char *pic)
{
	image_t *gl;

	gl = RDraw_FindPic (pic);
	if (!gl)
	{
		R_Printf(PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	if (scrap_dirty)
	{
		Scrap_Upload();
	}

	R_Bind (gl->texnum);
	glBegin (GL_QUADS);
	glTexCoord2f (gl->sl, gl->tl);
	glVertex2f (x, y);
	glTexCoord2f (gl->sh, gl->tl);
	glVertex2f (x+gl->width, y);
	glTexCoord2f (gl->sh, gl->th);
	glVertex2f (x+gl->width, y+gl->height);
	glTexCoord2f (gl->sl, gl->th);
	glVertex2f (x, y+gl->height);
	glEnd ();
}

/*
 * This repeats a 64*64 tile graphic to fill
 * the screen around a sized down
 * refresh window.
 */
void
RDraw_TileClear(int x, int y, int w, int h, char *pic)
{
	image_t *image;

	image = RDraw_FindPic (pic);
	if (!image)
	{
		R_Printf(PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	R_Bind (image->texnum);
	glBegin (GL_QUADS);
	glTexCoord2f (x/64.0, y/64.0);
	glVertex2f (x, y);
	glTexCoord2f ( (x+w)/64.0, y/64.0);
	glVertex2f (x+w, y);
	glTexCoord2f ( (x+w)/64.0, (y+h)/64.0);
	glVertex2f (x+w, y+h);
	glTexCoord2f ( x/64.0, (y+h)/64.0 );
	glVertex2f (x, y+h);
	glEnd ();
}

/*
 * Fills a box of pixels with a single color
 */
void
RDraw_Fill(int x, int y, int w, int h, byte r, byte g, byte b)
{

	glDisable (GL_TEXTURE_2D);

	glColor3f (r/255.0,
		g/255.0,
		b/255.0);

	glBegin (GL_QUADS);

	glVertex2f (x,y);
	glVertex2f (x+w, y);
	glVertex2f (x+w, y+h);
	glVertex2f (x, y+h);

	glEnd ();
	glColor3f (1,1,1);
	glEnable (GL_TEXTURE_2D);
}

void
RDraw_FadeScreen(void)
{
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glColor4f(0, 0, 0, 0.8);
	glBegin (GL_QUADS);

	glVertex2f (0,0);
	glVertex2f (vid.width, 0);
	glVertex2f (vid.width, vid.height);
	glVertex2f (0, vid.height);

	glEnd ();
	glColor4f(1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void
RDraw_StretchRaw(int x, int y, int w, int h, int cols, int rows, byte *data)
{
	unsigned	image32[256*256];
	unsigned char image8[256*256];
	int			i, j, trows;
	byte		*source;
	int			frac, fracstep;
	float		hscale;
	int			row;
	float		t;

	R_Bind(0);

	if(rows <= 256)
	{
		hscale = 1;
		trows = rows;
	}
	else
	{
		hscale = rows / 256.0;
		trows = 256;
	}
	t = rows*hscale / 256;

	{
		unsigned *dest;

		for (i=0 ; i<trows ; i++)
		{
			row = (int)(i*hscale);
			if (row > rows)
				break;
			source = data + cols*row;
			dest = &image32[i*256];
			fracstep = cols*0x10000/256;
			frac = fracstep >> 1;
			for (j=0 ; j<256 ; j++)
			{
				dest[j] = r_rawpalette[source[frac>>16]];
				frac += fracstep;
			}
		}

		glTexImage2D (GL_TEXTURE_2D, 0, gl_tex_solid_format, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, image32);
	}
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(x, y);
	glTexCoord2f(1, 0);
	glVertex2f(x+w, y);
	glTexCoord2f(1, t);
	glVertex2f(x+w, y+h);
	glTexCoord2f(0, t);
	glVertex2f(x, y+h);
	glEnd();
}
