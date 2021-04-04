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
 * Header for the model stuff.
 *
 * =======================================================================
 */

#ifndef REF_SHARED_MODEL_H
#define REF_SHARED_MODEL_H

enum {
	SIDE_FRONT = 0,
	SIDE_BACK = 1,
	SIDE_ON = 2
};

enum {
	SURF_PLANEBACK = 2,
	SURF_DRAWSKY = 4, // sky brush face
	SURF_DRAWTURB = 0x10,
	SURF_DRAWBACKGROUND = 0x40,
	SURF_UNDERWATER = 0x80
};

/*

d*_t structures are on-disk representations
m*_t structures are in-memory

*/


/*
==============================================================================

BRUSH MODELS

==============================================================================
*/

/* in memory representation */
typedef struct
{
	vec3_t position;
} mvertex_t;

typedef struct
{
	unsigned short	v[2];
	unsigned int	cachededgeoffset;
} medge_t;

#endif /* REF_SHARED_MODEL_H */
