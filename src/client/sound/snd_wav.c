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
 * This file implements a subset of the WAVE audio file format
 *
 * =======================================================================
 */

#include "../header/client.h"
#include "snd_local.h"

byte	*data_p;
byte 	*iff_end;
byte 	*last_chunk;
byte 	*iff_data;
int 	iff_chunk_len;

short GetLittleShort(void) {
	short val = 0;
	val = *data_p;
	val = val + (*(data_p+1)<<8);
	data_p += 2;
	return val;
}

int GetLittleLong(void) {
	int val = 0;
	val = *data_p;
	val = val + (*(data_p+1)<<8);
	val = val + (*(data_p+2)<<16);
	val = val + (*(data_p+3)<<24);
	data_p += 4;
	return val;
}

void FindNextChunk(char *name) {
	while (1) {
		data_p=last_chunk;

		if (data_p >= iff_end) {
			/* didn't find the chunk */
			data_p = NULL;
			return;
		}

		data_p += 4;
		iff_chunk_len = GetLittleLong();

		if (iff_chunk_len < 0) {
			data_p = NULL;
			return;
		}

		data_p -= 8;
		last_chunk = data_p + 8 + ( (iff_chunk_len + 1) & ~1 );

		if (!strncmp((const char *)data_p, name, 4))
			return;
	}
}

void FindChunk(char *name) {
	last_chunk = iff_data;
	FindNextChunk (name);
}

void DumpChunks(void) {
	char	str[5];

	str[4] = 0;
	data_p=iff_data;

	do {
		memcpy (str, data_p, 4);
		data_p += 4;
		iff_chunk_len = GetLittleLong();
		Com_Printf ("0x%x : %s (%d)\n", data_p - 4, str, iff_chunk_len);
		data_p += (iff_chunk_len + 1) & ~1;
	} while (data_p < iff_end);
}

wavinfo_t GetWavinfo (char *name, byte *wav, int wavlength) {
	wavinfo_t	info;
	int     i;
	int     format;
	int		samples;

	memset (&info, 0, sizeof(info));

	if (!wav)
		return info;

	iff_data = wav;
	iff_end = wav + wavlength;

	/* find "RIFF" chunk */
	FindChunk("RIFF");

	if (!(data_p && !strncmp((const char *)data_p+8, "WAVE", 4))) {
		Com_Printf("Missing RIFF/WAVE chunks\n");
		return info;
	}

	/* get "fmt " chunk */
	iff_data = data_p + 12;

	FindChunk("fmt ");

	if (!data_p) {
		Com_Printf("Missing fmt chunk\n");
		return info;
	}

	data_p += 8;
	format = GetLittleShort();

	if (format != 1) {
		Com_Printf("Microsoft PCM format only\n");
		return info;
	}

	info.channels = GetLittleShort();
	info.rate = GetLittleLong();
	data_p += 4+2;
	info.width = GetLittleShort() / 8;

	/* get cue chunk */
	FindChunk("cue ");

	if (data_p) {
		data_p += 32;
		info.loopstart = GetLittleLong();

		/* if the next chunk is a LIST chunk, look for a cue length marker */
		FindNextChunk ("LIST");

		if (data_p) {
			if (!strncmp ((const char *)data_p + 28, "mark", 4)) {
				/* this is not a proper parse, but it works with cooledit... */
				data_p += 24;
				i = GetLittleLong (); /* samples in loop */
				info.samples = info.loopstart + i;
			}
		}

	} else
		info.loopstart = -1;

	/* find data chunk */
	FindChunk("data");

	if (!data_p) {
		Com_Printf("Missing data chunk\n");
		return info;
	}

	data_p += 4;
	samples = GetLittleLong () / info.width;

	if (info.samples) {
		if (samples < info.samples)
			Com_Error (ERR_DROP, "Sound %s has a bad loop length", name);

	} else
		info.samples = samples;

	info.dataofs = data_p - wav;

	return info;
}

