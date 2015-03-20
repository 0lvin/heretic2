/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (C) 2015 Daniel Gibson
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
 * File formats supported by stb_image, for now only tga, png, jpg
 * See also https://github.com/nothings/stb
 *
 * =======================================================================
 */

#include <stdlib.h>

#include "../header/local.h"

// don't need HDR stuff
#define STBI_NO_LINEAR
#define STBI_NO_HDR
// make sure STB_image uses standard malloc(), as we'll use standard free() to deallocate
#define STBI_MALLOC(sz)    malloc(sz)
#define STBI_REALLOC(p,sz) realloc(p,sz)
#define STBI_FREE(p)       free(p)
// include implementation part of stb_image into this file
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*
 * origname: the filename to be opened, might be without extension
 * type: extension of the type we wanna open ("jpg", "png" or "tga")
 * pic: pointer RGBA pixel data will be assigned to
 */
qboolean
LoadSTB(const char *origname, const char* type, byte **pic, int *width, int *height)
{
	char filename[256];

	Q_strlcpy(filename, origname, sizeof(filename));

	/* Add the extension */
	if (strcmp(COM_FileExtension(filename), type) != 0)
	{
		Q_strlcat(filename, ".", sizeof(filename));
		Q_strlcat(filename, type, sizeof(filename));
	}

	*pic = NULL;

	byte* rawdata = NULL;
	int rawsize = FS_LoadFile(filename, (void **)&rawdata);
	if (rawdata == NULL)
	{
		return false;
	}

	int w, h, bytesPerPixel;
	byte* data = NULL;
	data = stbi_load_from_memory(rawdata, rawsize, &w, &h, &bytesPerPixel, STBI_rgb_alpha);
	if (data == NULL)
	{
		VID_Printf(PRINT_ALL, "stb_image couldn't load data from %s: %s!\n", filename, stbi_failure_reason());
		FS_FreeFile(rawdata);
		return false;
	}

	FS_FreeFile(rawdata);

	VID_Printf(PRINT_DEVELOPER, "LoadSTB() loaded: %s\n", filename);

	*pic = data;
	*width = w;
	*height = h;
	return true;
}



