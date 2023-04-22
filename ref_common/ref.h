/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef REF_COMMON_REF_H
#define REF_COMMON_REF_H

#include "../client/ref.h"

/*
====================================================================

IMPORTED FUNCTIONS

====================================================================
*/

extern	refimport_t	ri;

typedef enum
{
	it_skin,
	it_sprite,
	it_wall,
	it_pic,
	it_sky
} imagetype_t;

#define	MAX_LBM_HEIGHT		480

typedef enum {
	mod_bad,
	mod_brush,
	mod_sprite,
	mod_alias,
	mod_flex
} modtype_t;

typedef struct image_s* (*findimage_t)(const char *name, imagetype_t type);
extern void *Mod_LoadAliasModel (const char *mod_name, const void *buffer, int modfilelen,
	vec3_t mins, vec3_t maxs, struct image_s **skins,
	findimage_t find_image, modtype_t *type);
extern void *Mod_LoadSP2 (const char *mod_name, const void *buffer, int modfilelen,
	struct image_s **skins, findimage_t find_image, modtype_t *type);
extern int Mod_ReLoadSkins(struct image_s **skins, findimage_t find_image,
	void *extradata, modtype_t type);

#endif
