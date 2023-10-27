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
#include "cinema/smacker.h"

#ifdef AVMEDIADECODE
#include "cinema/avdecode.h"
#endif

#define PL_MPEG_IMPLEMENTATION
#include "cinema/pl_mpeg.h"

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
#include "refresh/files/stb_image.h"


extern cvar_t *vid_renderer;

cvar_t *cin_force43;
int abort_cinematic;

typedef struct
{
	byte *data;
	int count;
} cblock_t;

typedef enum
{
	video_cin,
	video_av,
	video_smk,
	video_mpg
} cinema_t;

typedef struct
{
	qboolean restart_sound;
	int s_rate;
	int s_width;
	int s_channels;

	int width;
	int height;
	float fps;
	int color_bits;
	cinema_t video_type;
	byte *pic;
	byte *pic_pending;

	/* cin video */
	/* order 1 huffman stuff */
	int *hnodes1;

	/* [256][256][2]; */
	int numhnodes1[256];

	int h_used[512];
	int h_count[512];

	/* shared video buffer */
	void *raw_video;
	byte *audio_buf;
	size_t audio_pos;

	/* smacker video */
	smk smk_video;

	/* mpg video */
	plm_t *plm_video;

#ifdef AVMEDIADECODE
	/* ffmpeg avideo */
	cinavdecode_t *av_video;
#endif
} cinematics_t;

cinematics_t cin;

static void
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

	if (cin.av_video)
	{
		cinavdecode_close(cin.av_video);
		cin.av_video = NULL;
	}

	if (cin.smk_video)
	{
		smk_close(cin.smk_video);
		cin.smk_video = NULL;
	}

	if (cin.plm_video)
	{
		plm_destroy(cin.plm_video);
		cin.plm_video = NULL;
	}

	if (cin.audio_buf)
	{
		Z_Free(cin.audio_buf);
		cin.audio_buf = NULL;
	}

	if (cin.raw_video)
	{
		FS_FreeFile(cin.raw_video);
		cin.raw_video = NULL;
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

static int
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
static void
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

static cblock_t
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

static byte *
SCR_ReadNextMPGFrame(void)
{
	size_t count, i;
	byte *buffer;
	plm_frame_t *frame;

	if (plm_has_ended(cin.plm_video))
	{
		return NULL;
	}

	count = cin.height * cin.width * cin.color_bits / 8;
	buffer = Z_Malloc(count);
	frame = plm_decode_video(cin.plm_video);
	if (!frame)
	{
		Z_Free(buffer);
		return NULL;
	}
	plm_frame_to_rgba(frame, buffer, frame->width * 4);

	/* force untransparent image show */
	for (i=0; i < count; i += 4)
	{
		buffer[i + 3] = 255;
	}

	if (cin.s_channels > 0)
	{
		/* Fix here if audio not in sync */
		count = cin.s_rate * cin.s_channels * cin.s_width / cin.fps;
		/* round up to channels and width */
		count = (count + (cin.s_channels * cin.s_width) - 1) & (~(cin.s_channels * cin.s_width) - 1);
		/* load enough sound data for single frame*/
		while (cin.audio_pos < count && cin.s_channels > 0)
		{
			plm_samples_t *samples;
			short *audiobuffer;

			samples = plm_decode_audio(cin.plm_video);
			if (!samples || samples->count <= 0)
			{
				break;
			}

			audiobuffer = (short *)(cin.audio_buf + cin.audio_pos);
			for (i=0; i < samples->count * cin.s_channels; i++)
			{
				audiobuffer[i] = samples->interleaved[i] * (1 << 15);
			}

			cin.audio_pos += samples->count * cin.s_channels * cin.s_width;
		}

		if (count > cin.audio_pos)
		{
			count = cin.audio_pos;
		}

		S_RawSamples(count / (cin.s_width * cin.s_channels), cin.s_rate, cin.s_width, cin.s_channels,
			cin.audio_buf, Cvar_VariableValue("s_volume"));

		/* cleanup already played buffer part */
		memmove(cin.audio_buf, cin.audio_buf + count, cin.audio_pos - count);
		cin.audio_pos -= count;
	}

	cl.cinematicframe++;

	return buffer;
}

static byte *
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

static byte *
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
	start = cl.cinematicframe * cin.s_rate / cin.fps;
	end = (cl.cinematicframe + 1) * cin.s_rate / cin.fps;
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


#ifdef AVMEDIADECODE

static byte *
SCR_ReadNextAVFrame(void)
{
	size_t count, i;
	byte *buffer;

	count = cin.height * cin.width * cin.color_bits / 8;
	buffer = Z_Malloc(count);
	if (cinavdecode_next_frame(cin.av_video, buffer, cin.audio_buf) < 0)
	{
		Z_Free(buffer);
		return NULL;
	}

	Com_DPrintf("Audio %.2f (%.2f): Video %.2f (%.2f)\n",
		(float)cin.av_video->audio_pos / cin.av_video->audio_frame_size,
		cin.av_video->audio_timestamp,
		(float)cin.av_video->video_pos / cin.av_video->video_frame_size,
		cin.av_video->video_timestamp);

	/* force untransparent image show */
	for (i=0; i < count; i += 4)
	{
		buffer[i + 3] = 255;
	}

	if (cin.s_channels > 0)
	{
		S_RawSamples(cin.av_video->audio_frame_size / (cin.s_width * cin.s_channels),
			cin.s_rate, cin.s_width, cin.s_channels,
			cin.audio_buf, Cvar_VariableValue("s_volume"));
	}

	cl.cinematicframe++;

	return buffer;
}

static qboolean
SCR_LoadAVcodec(const char *arg, const char *dot)
{
	char name[MAX_OSPATH], *path = NULL;

	while (1)
	{
		path = FS_NextPath(path);

		if (!path)
		{
			printf("can't decode or open %s\n", path);
			break;
		}

		Com_sprintf(name, sizeof(name), "%s/video/%s", path, arg);
		cin.av_video = cinavdecode_open(name);
		if (cin.av_video)
		{
			break;
		}
	}

	if (!cin.av_video)
	{
		/* Can't open file */
		return false;
	}

	return true;
}
#endif

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
		cl.cinematictime = cls.realtime - cl.cinematicframe * 1000 / cin.fps;
		return;
	}

	frame = (cls.realtime - cl.cinematictime) * cin.fps / 1000;

	if (frame <= cl.cinematicframe)
	{
		return;
	}

	if (frame > cl.cinematicframe + 1)
	{
		Com_Printf("Dropped frame: %i > %i\n", frame, cl.cinematicframe + 1);
		cl.cinematictime = cls.realtime - cl.cinematicframe * 1000 / cin.fps;
	}

	if (cin.pic)
	{
		Z_Free(cin.pic);
	}

	cin.pic = cin.pic_pending;
	switch (cin.video_type)
	{
		case video_cin:
			cin.pic_pending = SCR_ReadNextFrame();
			break;
		case video_smk:
			cin.pic_pending = SCR_ReadNextSMKFrame();
			break;
		case video_mpg:
			cin.pic_pending = SCR_ReadNextMPGFrame();
			break;
#ifdef AVMEDIADECODE
		case video_av:
			cin.pic_pending = SCR_ReadNextAVFrame();
			break;
#endif
		default:
			cin.pic_pending = NULL;
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

		/* Soft render requires to reset palette before show RGBA image */
		if (cin.color_bits == 32)
		{
			R_SetPalette(NULL);
		}
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

	Draw_StretchRaw(x, y, w, h, cin.width, cin.height, cin.pic, cin.color_bits);

	return true;
}

static byte *
SCR_LoadHiColor(const char* namewe, const char *ext, int *width, int *height)
{
	char filename[256];
	int bytesPerPixel;
	byte *pic, *data = NULL;
	void *rawdata;
	size_t len;

	Q_strlcpy(filename, namewe, sizeof(filename));
	Q_strlcat(filename, ".", sizeof(filename));
	Q_strlcat(filename, ext, sizeof(filename));

	len = FS_LoadFile(filename, &rawdata);

	if (!rawdata || len <=0)
	{
		return NULL;
	}

	data = stbi_load_from_memory(rawdata, len, width, height,
		&bytesPerPixel, STBI_rgb_alpha);
	if (data == NULL)
	{
		FS_FreeFile(rawdata);
		return NULL;
	}

	pic = Z_Malloc(cin.height * cin.width * 4);
	memcpy(pic, data, cin.height * cin.width * 4);
	free(data);

	return pic;
}

void
SCR_PlayCinematic(char *arg)
{
	int width, height;
	byte *palette = NULL;
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
		cvar_t	*r_retexturing;

		Com_sprintf(name, sizeof(name), "pics/%s", arg);
		r_retexturing = Cvar_Get("r_retexturing", "1", CVAR_ARCHIVE);

		if (r_retexturing->value)
		{
			char namewe[256];

			cin.color_bits = 32;

			/* Remove the extension */
			memset(namewe, 0, 256);
			memcpy(namewe, name, strlen(name) - strlen(dot));
			cin.pic = SCR_LoadHiColor(namewe, "tga", &cin.width, &cin.height);

			if (!cin.pic)
			{
				cin.pic = SCR_LoadHiColor(namewe, "png", &cin.width, &cin.height);
			}

			if (!cin.pic)
			{
				cin.pic = SCR_LoadHiColor(namewe, "jpg", &cin.width, &cin.height);
			}
		}

		if (!cin.pic)
		{
			SCR_LoadPCX(name, &cin.pic, &palette, &cin.width, &cin.height);
			cin.color_bits = 8;
		}

		cl.cinematicframe = -1;
		cl.cinematictime = 1;
		SCR_EndLoadingPlaque();

		cls.state = ca_active;

		if (!cin.pic)
		{
			Com_Printf("%s not found.\n", name);
			cl.cinematictime = 0;
		}
		else if (palette)
		{
			memcpy(cl.cinematicpalette, palette, sizeof(cl.cinematicpalette));
			Z_Free(palette);
		}

		return;
	}

#ifdef AVMEDIADECODE
	if (dot && (!strcmp(dot, ".ogv") ||
				!strcmp(dot, ".mpg") ||
				!strcmp(dot, ".smk") ||
				!strcmp(dot, ".roq")))
	{
		if (!SCR_LoadAVcodec(arg, dot))
		{
			cin.av_video = NULL;
			cl.cinematictime = 0; /* done */
			return;
		}

		SCR_EndLoadingPlaque();

		cin.color_bits = 32;
		cls.state = ca_active;

		cin.s_rate = cin.av_video->rate;
		cin.s_width = 2;
		cin.s_channels = cin.av_video->channels;
		cin.audio_buf = Z_Malloc(cin.av_video->audio_frame_size);

		cin.width = cin.av_video->width;
		cin.height = cin.av_video->height;
		cin.fps = cin.av_video->fps;

		cl.cinematicframe = 0;
		cin.pic = SCR_ReadNextAVFrame();
		cl.cinematictime = Sys_Milliseconds();

		cin.video_type = video_av;
		return;
	}

#else

	/* buildin decoders */
	if (dot && !strcmp(dot, ".mpg"))
	{
		int len;

		Com_sprintf(name, sizeof(name), "video/%s", arg);

		len = FS_LoadFile(name, &cin.raw_video);
		if (!cin.raw_video || len <= 0)
		{
			cl.cinematictime = 0; /* done */
			return;
		}

		cin.plm_video = plm_create_with_memory(cin.raw_video, len, 0);
		if (!cin.plm_video || !cin.plm_video->demux)
		{
			FS_FreeFile(cin.raw_video);
			cin.raw_video = NULL;
			cl.cinematictime = 0; /* done */
			return;
		}

		SCR_EndLoadingPlaque();

		cin.color_bits = 32;
		cls.state = ca_active;

		plm_set_loop(cin.plm_video, 0);
		plm_set_audio_enabled(cin.plm_video, 1);
		plm_set_audio_stream(cin.plm_video, 0);

		cin.fps = plm_get_framerate(cin.plm_video);
		cin.width = plm_get_width(cin.plm_video);
		cin.height = plm_get_height(cin.plm_video);

		if (plm_get_num_audio_streams(cin.plm_video) == 0)
		{
			/* No Sound */
			cin.s_channels = 0;
		}
		else
		{
			cin.s_rate = plm_get_samplerate(cin.plm_video);
			/* set to default 2 bytes with 2 channels */
			cin.s_width = 2;
			cin.s_channels = 2;

			/* Adjust the audio lead time according to the audio_spec buffer size */
			plm_set_audio_lead_time(cin.plm_video, 1.0f / cin.fps);

			/* Allocate audio buffer for 2 frames */
			cin.audio_buf = Z_Malloc(cin.s_channels * cin.s_width * cin.s_rate * 2 / cin.fps);
			cin.audio_pos = 0;
		}

		cl.cinematicframe = 0;
		cin.pic = SCR_ReadNextMPGFrame();
		cl.cinematictime = Sys_Milliseconds();

		cin.video_type = video_mpg;
		return;
	}

	if (dot && !strcmp(dot, ".smk"))
	{
		unsigned char trackmask, channels[7], depth[7];
		unsigned long width, height;
		unsigned long rate[7];
		double usf; /* microseconds per frame */
		size_t len;

		Com_sprintf(name, sizeof(name), "video/%s", arg);
		len = FS_LoadFile(name, &cin.raw_video);

		if (!cin.raw_video || len <=0)
		{
			cl.cinematictime = 0; /* done */
			return;
		}

		cin.smk_video = smk_open_memory(cin.raw_video, len);
		if (!cin.smk_video)
		{
			FS_FreeFile(cin.raw_video);
			cin.raw_video = NULL;
			cl.cinematictime = 0; /* done */
			return;
		}

		SCR_EndLoadingPlaque();

		cin.color_bits = 8;
		cls.state = ca_active;

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

		smk_info_all(cin.smk_video, NULL, NULL, &usf);
		smk_info_video(cin.smk_video, &width, &height, NULL);
		smk_enable_video(cin.smk_video, true);
		cin.width = width;
		cin.height = height;
		cin.fps = 1000000.0f / usf;

		/* process first frame */
		smk_first(cin.smk_video);

		cl.cinematicframe = 0;
		cin.pic = SCR_ReadNextSMKFrame();
		cl.cinematictime = Sys_Milliseconds();

		cin.video_type = video_smk;
		return;
	}
#endif

	Com_sprintf(name, sizeof(name), "video/%s", arg);
	FS_FOpenFile(name, &cl.cinematic_file, false);

	if (!cl.cinematic_file)
	{
		SCR_FinishCinematic();
		cl.cinematictime = 0; /* done */
		return;
	}

	SCR_EndLoadingPlaque();

	cin.color_bits = 8;
	cls.state = ca_active;

	FS_Read(&width, 4, cl.cinematic_file);
	FS_Read(&height, 4, cl.cinematic_file);
	cin.width = LittleLong(width);
	cin.height = LittleLong(height);
	cin.fps = 14.0f;

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

	cin.video_type = video_cin;
}
