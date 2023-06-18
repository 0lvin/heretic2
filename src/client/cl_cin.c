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
#include "input/header/input.h"
#include "smacker/smacker.h"

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

	/* smacker video */
	smk smk_video;
	void *smk_mem;
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
	cl.cinematictime = 0; /* done */

	if (cin.smk_video)
	{
		smk_close(cin.smk_video);
		cin.smk_video = NULL;
		FS_FreeFile(cin.smk_mem);
		cin.smk_mem = NULL;
	}

	if (cin.pic)
	{
		Z_Free(cin.pic);
		cin.pic = NULL;
	}

	if (cin.pic_pending)
	{
		Z_Free(cin.pic_pending);
		cin.pic_pending = NULL;
	}

	if (cl.cinematicpalette_active)
	{
		R_SetPalette(NULL);
		cl.cinematicpalette_active = false;
	}

	if (cl.cinematic_file)
	{
		FS_FCloseFile(cl.cinematic_file);
		cl.cinematic_file = 0;
	}

	if (cin.hnodes1)
	{
		Z_Free(cin.hnodes1);
		cin.hnodes1 = NULL;
	}

	/* switch back down to 11 khz sound if necessary */
	if (cin.restart_sound)
	{
		cin.restart_sound = false;
		CL_Snd_Restart_f();
	}
}

void
SCR_FinishCinematic(void)
{
	/* tell the server to advance to the next map / cinematic */
	MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
	SZ_Print(&cls.netchan.message, va("nextserver %i\n", cl.servercount));
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
SCR_ReadNextSMKFrame(void)
{
	size_t count;

	byte *buffer = Z_Malloc(cin.height * cin.width);

	/* audio */
	count = smk_get_audio_size(cin.smk_video, 0);
	if (count && cin.s_channels)
	{
		count /= (cin.s_width * cin.s_channels);
		S_RawSamples(count, cin.s_rate, cin.s_width, cin.s_channels,
			smk_get_audio(cin.smk_video, 0), Cvar_VariableValue("s_volume"));
	}

	/* update palette */
	memcpy(cl.cinematicpalette, smk_get_palette(cin.smk_video), sizeof(cl.cinematicpalette));
	cl.cinematicpalette_active = 0;

	/* get pic */
	memcpy(buffer, smk_get_video(cin.smk_video), cin.height * cin.width);
	cl.cinematicframe++;

	if (smk_next(cin.smk_video) != SMK_MORE)
	{
		Z_Free(buffer);
		return NULL;
	}

	return buffer;
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
	int frame;

	if (cl.cinematictime <= 0)
	{
		SCR_StopCinematic();
		return;
	}

	if (cl.cinematicframe == -1)
	{
		return; /* static image */
	}

	if (cls.key_dest != key_game)
	{
		/* pause if menu or console is up */
		cl.cinematictime = cls.realtime - cl.cinematicframe * 1000 / 14;
		return;
	}

	frame = (cls.realtime - cl.cinematictime) * 14.0 / 1000;

	if (frame <= cl.cinematicframe)
	{
		return;
	}

	if (frame > cl.cinematicframe + 1)
	{
		Com_Printf("Dropped frame: %i > %i\n", frame, cl.cinematicframe + 1);
		cl.cinematictime = cls.realtime - cl.cinematicframe * 1000 / 14;
	}

	if (cin.pic)
	{
		Z_Free(cin.pic);
	}

	cin.pic = cin.pic_pending;
	cin.pic_pending = NULL;
	if (!cin.smk_video)
	{
		cin.pic_pending = SCR_ReadNextFrame();
	}
	else
	{
		cin.pic_pending = SCR_ReadNextSMKFrame();
	}

	if (!cin.pic_pending)
	{
		SCR_StopCinematic();
		SCR_FinishCinematic();
		cl.cinematictime = 1; /* the black screen behind loading */
		SCR_BeginLoadingPlaque();
		cl.cinematictime = 0;
		return;
	}
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

#define ROQ_QUAD			0x1000
#define ROQ_QUAD_INFO		0x1001
#define ROQ_CODEBOOK		0x1002
#define ROQ_QUAD_VQ			0x1011
#define ROQ_QUAD_JPEG		0x1012
#define ROQ_QUAD_HANG		0x1013
#define ROQ_PACKET			0x1030
#define ZA_SOUND_MONO		0x1020
#define ZA_SOUND_STEREO		0x1021

static	long				ROQ_YY_tab[256];
static	long				ROQ_UB_tab[256];
static	long				ROQ_UG_tab[256];
static	long				ROQ_VG_tab[256];
static	long				ROQ_VR_tab[256];
static	unsigned short		vq2[256*16*4];
static	unsigned short		vq4[256*64*4];
static	unsigned short		vq8[256*256*4];

static void ROQ_GenYUVTables( void )
{
	float t_ub,t_vr,t_ug,t_vg;
	long i;

	t_ub = (1.77200f/2.0f) * (float)(1<<6) + 0.5f;
	t_vr = (1.40200f/2.0f) * (float)(1<<6) + 0.5f;
	t_ug = (0.34414f/2.0f) * (float)(1<<6) + 0.5f;
	t_vg = (0.71414f/2.0f) * (float)(1<<6) + 0.5f;
	for(i=0;i<256;i++) {
		float x = (float)(2 * i - 255);

		ROQ_UB_tab[i] = (long)( ( t_ub * x) + (1<<5));
		ROQ_VR_tab[i] = (long)( ( t_vr * x) + (1<<5));
		ROQ_UG_tab[i] = (long)( (-t_ug * x));
		ROQ_VG_tab[i] = (long)( (-t_vg * x) + (1<<5));
		ROQ_YY_tab[i] = (long)( (i << 6) | (i >> 2) );
	}
}

#define VQ2TO4(a,b,c,d) { \
	*c++ = a[0];	\
	*d++ = a[0];	\
	*d++ = a[0];	\
	*c++ = a[1];	\
	*d++ = a[1];	\
	*d++ = a[1];	\
	*c++ = b[0];	\
	*d++ = b[0];	\
	*d++ = b[0];	\
	*c++ = b[1];	\
	*d++ = b[1];	\
	*d++ = b[1];	\
	*d++ = a[0];	\
	*d++ = a[0];	\
	*d++ = a[1];	\
	*d++ = a[1];	\
	*d++ = b[0];	\
	*d++ = b[0];	\
	*d++ = b[1];	\
	*d++ = b[1];	\
	a += 2; b += 2; }

#define VQ2TO2(a,b,c,d) { \
	*c++ = *a;	\
	*d++ = *a;	\
	*d++ = *a;	\
	*c++ = *b;	\
	*d++ = *b;	\
	*d++ = *b;	\
	*d++ = *a;	\
	*d++ = *a;	\
	*d++ = *b;	\
	*d++ = *b;	\
	a++; b++; }

/******************************************************************************
*
* Function:
*
* Description:
*
******************************************************************************/
static unsigned int yuv_to_rgb24( long y, long u, long v )
{
	long r,g,b,YY = (long)(ROQ_YY_tab[(y)]);

	r = (YY + ROQ_VR_tab[v]) >> 6;
	g = (YY + ROQ_UG_tab[u] + ROQ_VG_tab[v]) >> 6;
	b = (YY + ROQ_UB_tab[u]) >> 6;

	if (r<0) r = 0;
	if (g<0) g = 0;
	if (b<0) b = 0;
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;

	return LittleLong ((unsigned long)((r)|(g<<8)|(b<<16))|(255UL<<24));
}

//-----------------------------------------------------------------------------
// RllSetupTable
//
// Allocates and initializes the square table.
//
// Parameters:	None
//
// Returns:		Nothing
//-----------------------------------------------------------------------------
static void RllSetupTable(short *sqrTable)
{
	int z;

	for (z=0;z<128;z++) {
		sqrTable[z] = (short)(z*z);
		sqrTable[z+128] = (short)(-sqrTable[z]);
	}
}

//-----------------------------------------------------------------------------
// RllDecodeMonoToStereo
//
// Decode mono source data into a stereo buffer. Output is 4 times the number
// of bytes in the input.
//
// Parameters:	from -> buffer holding encoded data
//				to ->	buffer to hold decoded data
//				size =	number of bytes of input (= 1/4 # of bytes of output)
//				signedOutput = 0 for unsigned output, non-zero for signed output
//				flag = flags from asset header
//
// Returns:		Number of samples placed in output buffer
//-----------------------------------------------------------------------------
long RllDecodeMonoToStereo(unsigned char *from, short *to, unsigned int size,
	char signedOutput, unsigned short flag, short *sqrTable)
{
	unsigned int z;
	int prev;

	if (signedOutput)
	{
		prev = flag - 0x8000;
	}
	else
	{
		prev = flag;
	}

	for (z = 0; z < size; z++) {
		prev = (short)(prev + sqrTable[from[z]]);
		to[z*2+0] = to[z*2+1] = (short)(prev);
	}

	return size;	// * 2 * sizeof(short));
}

//-----------------------------------------------------------------------------
// RllDecodeStereoToStereo
//
// Decode stereo source data into a stereo buffer.
//
// Parameters:	from -> buffer holding encoded data
//				to ->	buffer to hold decoded data
//				size =	number of bytes of input (= 1/2 # of bytes of output)
//				signedOutput = 0 for unsigned output, non-zero for signed output
//				flag = flags from asset header
//
// Returns:		Number of samples placed in output buffer
//-----------------------------------------------------------------------------
long RllDecodeStereoToStereo(unsigned char *from, short *to, unsigned int size,
	char signedOutput, unsigned short flag, short *sqrTable)
{
	unsigned int z;
	unsigned char *zz = from;
	int	prevL, prevR;

	if (signedOutput) {
		prevL = (flag & 0xff00) - 0x8000;
		prevR = ((flag & 0x00ff) << 8) - 0x8000;
	} else {
		prevL = flag & 0xff00;
		prevR = (flag & 0x00ff) << 8;
	}

	for (z=0;z<size;z+=2) {
		prevL = (short)(prevL + sqrTable[*zz++]);
		prevR = (short)(prevR + sqrTable[*zz++]);
		to[z+0] = (short)(prevL);
		to[z+1] = (short)(prevR);
	}

	return (size>>1);	//*sizeof(short));
}


void
SCR_PlayCinematic(char *arg)
{
	int width, height;
	byte *palette;
	char name[MAX_OSPATH], *dot;

	In_FlushQueue();
	abort_cinematic = INT_MAX;

	/* make sure background music is not playing */
	OGG_Stop();

	cl.cinematicframe = 0;
	dot = strstr(arg, ".");

	/* static pcx image */
	if (dot && !strcmp(dot, ".pcx"))
	{
		Com_sprintf(name, sizeof(name), "pics/%s", arg);
		SCR_LoadPCX(name, &cin.pic, &palette, &cin.width, &cin.height);
		cl.cinematicframe = -1;
		cl.cinematictime = 1;
		SCR_EndLoadingPlaque();
		cls.state = ca_active;

		if (!cin.pic)
		{
			Com_Printf("%s not found.\n", name);
			cl.cinematictime = 0;
		}
		else
		{
			memcpy(cl.cinematicpalette, palette, sizeof(cl.cinematicpalette));
			Z_Free(palette);
		}

		return;
	}

	if (dot && !strcmp(dot, ".roq"))
	{
		short value;
		size_t len, RoQPlayed;
		int numQuads;
		short sqrTable[256];

		Com_sprintf(name, sizeof(name), "video/%s", arg);

		len = FS_LoadFile(name, &cin.smk_mem);

		if (!cin.smk_mem || len <=0)
		{
			cl.cinematictime = 0; /* done */
			return;
		}

		value = LittleShort(*((int *)cin.smk_mem));

		if (value != 0x1084)
		{
			Com_Error(ERR_DROP, "Bad ident value 0x%x", value);
		}

		ROQ_GenYUVTables();
		RllSetupTable(sqrTable);

		value = LittleLong(*((int *)((byte *)cin.smk_mem + 2)));
		printf("Not implemeted %s, %d size\n", name, value);

		value = LittleShort(*((int *)((byte *)cin.smk_mem + 6)));
		printf("Not implemeted %s, %d fps\n", name, value);

		RoQPlayed = 8;
		numQuads = -1;

		while (RoQPlayed < len)
		{
			int roq_id, RoQFrameSize, roq_flags;

			roq_id = LittleShort(*((int *)((byte *)cin.smk_mem +  RoQPlayed)));
			RoQFrameSize = LittleLong(*((int *)((byte *)cin.smk_mem + RoQPlayed + 2)));
			roq_flags = LittleShort(*((int *)((byte *)cin.smk_mem +  RoQPlayed + 6)));

			switch (roq_id) {
				case ROQ_QUAD:
					{
						printf("0x%lx ident ROQ_QUAD(0x%x)\n", RoQPlayed, RoQFrameSize);
					}
					break;
				case ROQ_QUAD_INFO:
					{
						printf("0x%lx ident ROQ_QUAD_INFO(0x%x)\n", RoQPlayed, RoQFrameSize);

						if (numQuads == -1)
						{
							// Fixme
							// readQuadInfo( (byte *)cin.smk_mem +  RoQPlayed + 8 );
							// setupQuad( 0, 0 );
						}

						if (numQuads != 1)
						{
							numQuads = 0;
						}
					}
					break;
				case ROQ_CODEBOOK: printf("0x%lx ident ROQ_CODEBOOK(0x%x)\n", RoQPlayed, RoQFrameSize); break;
				case ROQ_QUAD_VQ: printf("0x%lx ident ROQ_QUAD_VQ(0x%x)\n", RoQPlayed, RoQFrameSize); break;
				case ROQ_QUAD_JPEG: printf("0x%lx ident ROQ_QUAD_JPEG(0x%x)\n", RoQPlayed, RoQFrameSize); break;
				case ROQ_QUAD_HANG: printf("0x%lx ident ROQ_QUAD_HANG(0x%x)\n", RoQPlayed, RoQFrameSize); break;
				case ROQ_PACKET: printf("0x%lx ident ROQ_PACKET(0x%x)\n", RoQPlayed, RoQFrameSize); break;
				case ZA_SOUND_MONO:
					{
						short		sbuf[32768];
						int ssize;

						printf("0x%lx ident ZA_SOUND_MONO(0x%x)\n", RoQPlayed, RoQFrameSize);
						ssize = RllDecodeMonoToStereo(
							(byte *)cin.smk_mem +  RoQPlayed + 8,
							sbuf,
							RoQFrameSize, 0,
							(unsigned short)roq_flags, sqrTable);
						S_RawSamples(ssize, 22050, 2, 1, (byte *)sbuf, Cvar_VariableValue("s_volume"));
					}
					break;
				case ZA_SOUND_STEREO:
					{
						short		sbuf[32768];
						int ssize;

						printf("0x%lx ident ZA_SOUND_STEREO(0x%x)\n", RoQPlayed, RoQFrameSize);
						ssize = RllDecodeStereoToStereo(
							(byte *)cin.smk_mem +  RoQPlayed + 8,
							sbuf,
							RoQFrameSize, 0,
							(unsigned short)roq_flags, sqrTable);
						S_RawSamples(ssize, 22050, 2, 1, (byte *)sbuf, Cvar_VariableValue("s_volume"));
					}
					break;
				default: printf("0x%lx ident value 0x%x(0x%x)\n", RoQPlayed, roq_id, RoQFrameSize); break;
			}

			RoQPlayed += (RoQFrameSize + 8);
			// printf("%s size value 0x%x (%d) %ld -> %ld\n", name, value, value, RoQPlayed, len);
		}

		printf("\n%lx => %lx\n\n", RoQPlayed, len);

		SCR_FinishCinematic();
		cl.cinematictime = 0; /* done */
		return;
	}

	if (dot && !strcmp(dot, ".smk"))
	{
		unsigned char trackmask, channels[7], depth[7];
		unsigned long width, height;
		unsigned long rate[7];
		size_t len;

		Com_sprintf(name, sizeof(name), "video/%s", arg);

		len = FS_LoadFile(name, &cin.smk_mem);

		if (!cin.smk_mem || len <=0)
		{
			cl.cinematictime = 0; /* done */
			return;
		}

		cin.smk_video = smk_open_memory(cin.smk_mem, len);
		if (!cin.smk_video)
		{
			FS_FreeFile(cin.smk_mem);
			cin.smk_mem = NULL;
			cl.cinematictime = 0; /* done */
			return;
		}

		smk_info_audio(cin.smk_video, &trackmask, channels, depth, rate);
		if (trackmask != SMK_AUDIO_TRACK_0)
		{
			Com_Printf("%s has different track mask %d.\n", name, trackmask);
			cin.s_channels = 0;
		}
		else
		{
			cin.s_rate = rate[0];
			cin.s_width = depth[0] / 8;
			cin.s_channels = channels[0];
			smk_enable_audio(cin.smk_video, 0, true);
		}

		smk_info_video(cin.smk_video, &width, &height, NULL);
		smk_enable_video(cin.smk_video, true);
		cin.width = width;
		cin.height = height;

		/* process first frame */
		smk_first(cin.smk_video);

		cl.cinematicframe = 0;
		cin.pic = SCR_ReadNextSMKFrame();
		cl.cinematictime = Sys_Milliseconds();

		return;
	}

	Com_sprintf(name, sizeof(name), "video/%s", arg);
	FS_FOpenFile(name, &cl.cinematic_file, false);

	if (!cl.cinematic_file)
	{
		SCR_FinishCinematic();
		cl.cinematictime = 0; /* done */
		return;
	}

	SCR_EndLoadingPlaque();

	cls.state = ca_active;

	FS_Read(&width, 4, cl.cinematic_file);
	FS_Read(&height, 4, cl.cinematic_file);
	cin.width = LittleLong(width);
	cin.height = LittleLong(height);

	FS_Read(&cin.s_rate, 4, cl.cinematic_file);
	cin.s_rate = LittleLong(cin.s_rate);
	FS_Read(&cin.s_width, 4, cl.cinematic_file);
	cin.s_width = LittleLong(cin.s_width);
	FS_Read(&cin.s_channels, 4, cl.cinematic_file);
	cin.s_channels = LittleLong(cin.s_channels);

	Huff1TableInit();

	cl.cinematicframe = 0;
	cin.pic = SCR_ReadNextFrame();
	cl.cinematictime = Sys_Milliseconds();
}

