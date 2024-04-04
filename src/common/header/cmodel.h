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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 * =======================================================================
 *
 * Shared model load code
 *
 * =======================================================================
 */

#ifndef COMMON_CMODEL_H
#define COMMON_CMODEL_H

#define EXTRA_LUMP_VERTEXES 8
#define EXTRA_LUMP_TEXINFO 6
#define EXTRA_LUMP_SURFEDGES 24
#define EXTRA_LUMP_EDGES 13
#define EXTRA_LUMP_FACES 6
#define EXTRA_LUMP_PLANES 12
#define EXTRA_LUMP_LEAFBRUSHES 2
#define EXTRA_LUMP_BRUSHES 1
#define EXTRA_LUMP_NODES 6
#define EXTRA_LUMP_BRUSHSIDES 6
#define EXTRA_LUMP_LEAFS 1

typedef enum
{
	map_quake2 = 0,
	map_heretic2 = 1,
	map_daikatana = 2,
	map_kingpin = 3,
	map_anachronox = 4,
} maptype_t;

extern int Mod_LoadSurfConvertFlags(int flags, maptype_t maptype);
extern int Mod_CalcLumpHunkSize(const lump_t *l, int inSize, int outSize, int extra);
extern void Mod_LoadVisibility(const char *name, dvis_t **vis, int *numvisibility,
	const byte *mod_base, const lump_t *l);
extern void Mod_LoadPlanes(const char *name, cplane_t **planes, int *numplanes,
	const byte *mod_base, const lump_t *l);
extern maptype_t Mod_LoadValidateLumps(const char *name, const dheader_t *header);

#endif
