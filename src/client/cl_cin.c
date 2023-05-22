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
 * This file implements the .cin video codec and the corresponding .pcx
 * bitmap decoder. .cin files are just a bunch of .pcx images.
 *
 * =======================================================================
 */

#include <limits.h>

#include "header/client.h"
#include "../common/libsmacker/smacker.h"
#include "input/header/input.h"

extern cvar_t *vid_renderer;

cvar_t *cin_force43;
int abort_cinematic;

typedef struct
{
	byte *data;
	int count;
} cblock_t;

typedef struct
{
	qboolean restart_sound;
	int s_rate;
	int s_width;
	int s_channels;

	int width;
	int height;
	byte *pic;
	byte *pic_pending;

	/* order 1 huffman stuff */
	int *hnodes1;

	/* [256][256][2]; */
	int numhnodes1[256];

	int h_used[512];
	int h_count[512];

	smk video;
	unsigned long frameCount;

	/* microseconds per frame */
	double usf;
	unsigned char	a_t, a_c[7], a_d[7];
	unsigned long	a_r[7];
} cinematics_t;

cinematics_t cin;

void
SCR_LoadPCX(char *filename, byte **pic, byte **palette, int *width, int *height)
{
	byte *raw;
	pcx_t *pcx;
	int x, y;
	int len, full_size;
	int dataByte, runLength;
	byte *out, *pix;

	*pic = NULL;

	/* load the file */
	len = FS_LoadFile(filename, (void **)&raw);

	if (!raw || len < sizeof(pcx_t))
	{
		return;
	}

	/* parse the PCX file */
	pcx = (pcx_t *)raw;
	raw = &pcx->data;

	if ((pcx->manufacturer != 0x0a) ||
		(pcx->version != 5) ||
		(pcx->encoding != 1) ||
		(pcx->bits_per_pixel != 8) ||
		(pcx->xmax >= 640) ||
		(pcx->ymax >= 480))
	{
		Com_Printf("Bad pcx file %s\n", filename);
		return;
	}

	full_size = (pcx->ymax + 1) * (pcx->xmax + 1);
	out = Z_Malloc(full_size);

	*pic = out;

	pix = out;

	if (palette)
	{
		*palette = Z_Malloc(768);
		memcpy(*palette, (byte *)pcx + len - 768, 768);
	}

	if (width)
	{
		*width = pcx->xmax + 1;
	}

	if (height)
	{
		*height = pcx->ymax + 1;
	}

	for (y = 0; y <= pcx->ymax; y++, pix += pcx->xmax + 1)
	{
		for (x = 0; x <= pcx->xmax; )
		{
			dataByte = *raw++;

			if ((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else
			{
				runLength = 1;
			}

			while (runLength-- > 0)
			{
				if ((*pic + full_size) <= (pix + x))
				{
					x += runLength;
					runLength = 0;
				}
				else
				{
					pix[x++] = dataByte;
				}
			}
		}
	}

	if (raw - (byte *)pcx > len)
	{
		Com_Printf("PCX file %s was malformed", filename);
		Z_Free(*pic);
		*pic = NULL;
	}

	FS_FreeFile(pcx);
}

void
SCR_StopCinematic(void)
{
	SCR_FinishCinematic();
}

void
SCR_FinishCinematic(void)
{
	if (cin.video == NULL)
		return;

	/* tell the server to advance to the next map / cinematic */
	MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
	SZ_Print(&cls.netchan.message, va("nextserver %i\n", cl.servercount));

	smk_close(cin.video);
	cin.video = NULL;
}

int
SmallestNode1(int numhnodes)
{
	int i;
	int best, bestnode;

	best = 99999999;
	bestnode = -1;

	for (i = 0; i < numhnodes; i++)
	{
		if (cin.h_used[i])
		{
			continue;
		}

		if (!cin.h_count[i])
		{
			continue;
		}

		if (cin.h_count[i] < best)
		{
			best = cin.h_count[i];
			bestnode = i;
		}
	}

	if (bestnode == -1)
	{
		return -1;
	}

	cin.h_used[bestnode] = true;
	return bestnode;
}

/*
 * Reads the 64k counts table and initializes the node trees
 */
void
Huff1TableInit(void)
{
	int prev;
	int j;
	int *node, *nodebase;
	byte counts[256];
	int numhnodes;

	cin.hnodes1 = Z_Malloc(256 * 256 * 2 * 4);
	memset(cin.hnodes1, 0, 256 * 256 * 2 * 4);

	for (prev = 0; prev < 256; prev++)
	{
		memset(cin.h_count, 0, sizeof(cin.h_count));
		memset(cin.h_used, 0, sizeof(cin.h_used));

		/* read a row of counts */
		FS_Read(counts, sizeof(counts), cl.cinematic_file);

		for (j = 0; j < 256; j++)
		{
			cin.h_count[j] = counts[j];
		}

		/* build the nodes */
		numhnodes = 256;
		nodebase = cin.hnodes1 + prev * 256 * 2;

		while (numhnodes != 511)
		{
			node = nodebase + (numhnodes - 256) * 2;

			/* pick two lowest counts */
			node[0] = SmallestNode1(numhnodes);

			if (node[0] == -1)
			{
				break;  /* no more counts */
			}

			node[1] = SmallestNode1(numhnodes);

			if (node[1] == -1)
			{
				break;
			}

			cin.h_count[numhnodes] = cin.h_count[node[0]] +
									 cin.h_count[node[1]];
			numhnodes++;
		}

		cin.numhnodes1[prev] = numhnodes - 1;
	}
}

cblock_t
Huff1Decompress(cblock_t in)
{
	byte *input;
	byte *out_p;
	int nodenum;
	int count;
	cblock_t out;
	int inbyte;
	int *hnodes, *hnodesbase;

	/* get decompressed count */
	count = in.data[0] + (in.data[1] << 8) + (in.data[2] << 16) + (in.data[3] << 24);
	input = in.data + 4;
	out_p = out.data = Z_Malloc(count);

	/* read bits */
	hnodesbase = cin.hnodes1 - 256 * 2; /* nodes 0-255 aren't stored */

	hnodes = hnodesbase;
	nodenum = cin.numhnodes1[0];

	while (count)
	{
		inbyte = *input++;

		int i = 0;

		for (i = 0; i < 8; i++)
		{
			if (nodenum < 256)
			{
				hnodes = hnodesbase + (nodenum << 9);
				*out_p++ = nodenum;

				if (!--count)
				{
					break;
				}

				nodenum = cin.numhnodes1[nodenum];
			}

			nodenum = hnodes[nodenum * 2 + (inbyte & 1)];
			inbyte >>= 1;
		}
	}

	if ((input - in.data != in.count) && (input - in.data != in.count + 1))
	{
		Com_Printf("Decompression overread by %i", (int)(input - in.data) - in.count);
	}

	out.count = out_p - out.data;

	return out;
}

byte *
SCR_ReadNextFrame(void)
{
	int r;
	int command;

	// the samples array is used as bytes or shorts, depending on bitrate (cin.s_width)
	// so we need to make sure to align it correctly
	YQ2_ALIGNAS_TYPE(short) byte samples[22050 / 14 * 4];

	byte compressed[0x20000];
	int size;
	byte *pic;
	cblock_t in, huf1;
	int start, end, count;

	/* read the next frame */
	r = FS_FRead(&command, 4, 1, cl.cinematic_file);

	if (r == 0)
	{
		/* we'll give it one more chance */
		r = FS_FRead(&command, 4, 1, cl.cinematic_file);
	}

	if (r != 4)
	{
		return NULL;
	}

	command = LittleLong(command);

	if (command == 2)
	{
		return NULL;  /* last frame marker */
	}

	if (command == 1)
	{
		/* read palette */
		FS_Read(cl.cinematicpalette, sizeof(cl.cinematicpalette),
				cl.cinematic_file);
		cl.cinematicpalette_active = 0;
	}

	/* decompress the next frame */
	FS_Read(&size, 4, cl.cinematic_file);
	size = LittleLong(size);

	if (((size_t)size > sizeof(compressed)) || (size < 1))
	{
		Com_Error(ERR_DROP, "Bad compressed frame size");
	}

	FS_Read(compressed, size, cl.cinematic_file);

	/* read sound */
	start = cl.cinematicframe * cin.s_rate / 14;
	end = (cl.cinematicframe + 1) * cin.s_rate / 14;
	count = end - start;

	FS_Read(samples, count * cin.s_width * cin.s_channels,
			cl.cinematic_file);

	if (cin.s_width == 2)
	{
		for (r = 0; r < count * cin.s_channels; r++)
		{
			((short *)samples)[r] = LittleShort(((short *)samples)[r]);
		}
	}

	S_RawSamples(count, cin.s_rate, cin.s_width, cin.s_channels,
			samples, Cvar_VariableValue("s_volume"));

	in.data = compressed;
	in.count = size;

	huf1 = Huff1Decompress(in);

	pic = huf1.data;

	cl.cinematicframe++;

	return pic;
}

void
SCR_RunCinematic(void)
{
	const unsigned char* palette_data;
	const unsigned char* image_data;
	int x,y, h, w;

	if (cin.video == NULL)
		return;

	smk_next(cin.video);

	palette_data = smk_get_palette(cin.video);
	image_data = smk_get_video(cin.video);

	if (cin_force43->value)
	{
		w = viddef.height * 4 / 3;
		if (w > viddef.width)
		{
			w = viddef.width;
		}
		w &= ~3;
		h = w * 3 / 4;
		x = (viddef.width - w) / 2;
		y = (viddef.height - h) / 2;
	}
	else
	{
		x = y = 0;
		w = viddef.width;
		h = viddef.height;
	}

	R_SetPalette(palette_data);

	Draw_StretchRaw(x, y, w, h, cin.width, cin.height, image_data);
}

static int
SCR_MinimalColor(void)
{
	int i, min_color, min_index;

	min_color = 255 * 3;
	min_index = 0;

	for(i=0; i<255; i++)
	{
		int current_color = (cl.cinematicpalette[i*3+0] +
				     cl.cinematicpalette[i*3+1] +
				     cl.cinematicpalette[i*3+2]);

		if (min_color > current_color)
		{
			min_color = current_color;
			min_index = i;
		}
	}

	return min_index;
}


/*
 * Returns true if a cinematic is active, meaning the
 * view rendering should be skipped
 */
qboolean
SCR_DrawCinematic(void)
{
	int x, y, w, h, color;

	if (cl.cinematictime <= 0)
	{
		return false;
	}

	/* blank screen and pause if menu is up */
	if (cls.key_dest == key_menu)
	{
		R_SetPalette(NULL);
		cl.cinematicpalette_active = false;
		return true;
	}

	if (!cl.cinematicpalette_active)
	{
		R_SetPalette(cl.cinematicpalette);
		cl.cinematicpalette_active = true;
	}

	if (!cin.pic)
	{
		return true;
	}

	if (cin_force43->value)
	{
		w = viddef.height * 4 / 3;
		if (w > viddef.width)
		{
			w = viddef.width;
		}
		w &= ~3;
		h = w * 3 / 4;
		x = (viddef.width - w) / 2;
		y = (viddef.height - h) / 2;
	}
	else
	{
		x = y = 0;
		w = viddef.width;
		h = viddef.height;
	}

	if (!vid_renderer)
	{
		vid_renderer = Cvar_Get("vid_renderer", "gl1", CVAR_ARCHIVE);
	}

	if (Q_stricmp(vid_renderer->string, "soft") == 0)
	{
		color = SCR_MinimalColor();
	}
	else
	{
		color = 0;
	}

	if (x > 0)
	{
		Draw_Fill(0, 0, x, viddef.height, color);
	}
	if (x + w < viddef.width)
	{
		Draw_Fill(x + w, 0, viddef.width - (x + w), viddef.height, color);
	}
	if (y > 0)
	{
		Draw_Fill(x, 0, w, y, color);
	}
	if (y + h < viddef.height)
	{
		Draw_Fill(x, y + h, w, viddef.height - (y + h), color);
	}

	Draw_StretchRaw(x, y, w, h, cin.width, cin.height, cin.pic);

	return true;
}

void
SCR_PlayCinematic(char *arg)
{
	char name[MAX_OSPATH], *dot;

	dot = strstr(arg, ".");

	Com_sprintf(name, sizeof(name), "base/video/%s", arg);
	// TODO: Use smk smk_open_memory(buffer, size);
	cin.video = smk_open_file(name, SMK_MODE_DISK);

	if (cin.video == NULL)
	{
		Com_Printf("Failed to open SMK %s\n", name);
		return;
	}

	{
		unsigned long width, height;

		smk_info_all(cin.video, NULL, &cin.frameCount, &cin.usf);
		smk_info_video(cin.video, &width, &height, NULL);
		smk_info_audio(cin.video, &cin.a_t, cin.a_c, cin.a_d, cin.a_r);

		smk_enable_all(cin.video, cin.a_t);
		smk_enable_video(cin.video, true);

		/* process first frame */
		smk_first(cin.video);

		cin.width = width;
		cin.height = height;
	}
}

qboolean
CIN_IsCinematicRunning(void) {
	return cin.video != NULL;
}

