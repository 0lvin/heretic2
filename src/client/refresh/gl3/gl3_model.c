/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (C) 2016-2017 Daniel Gibson
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
 * Model loading and caching for OpenGL3. Includes the .bsp file format
 *
 * =======================================================================
 */

#include "header/local.h"

static YQ2_ALIGNAS_TYPE(int) byte mod_novis[MAX_MAP_LEAFS / 8];

static gl3model_t mod_known[MAX_MOD_KNOWN];
static int mod_numknown = 0;
static int mod_max = 0;
int registration_sequence;

//===============================================================================

static qboolean
Mod_HasFreeSpace(void)
{
	int		i, used;
	gl3model_t	*mod;

	used = 0;

	for (i=0, mod=mod_known ; i < mod_numknown ; i++, mod++)
	{
		if (!mod->name[0])
			continue;
		if (mod->registration_sequence == registration_sequence)
		{
			used ++;
		}
	}

	if (mod_max < used)
	{
		mod_max = used;
	}

	// should same size of free slots as currently used
	return (mod_numknown + mod_max) < MAX_MOD_KNOWN;
}

const byte *
GL3_Mod_ClusterPVS(int cluster, const gl3model_t *model)
{
	if ((cluster == -1) || !model->vis)
	{
		return mod_novis;
	}

	return Mod_DecompressVis((byte *)model->vis +
			model->vis->bitofs[cluster][DVIS_PVS],
			(byte *)model->vis + model->numvisibility,
			(model->vis->numclusters + 7) >> 3);
}

void
GL3_Mod_Modellist_f(void)
{
	int i, total, used;
	gl3model_t *mod;
	qboolean freeup;

	total = 0;
	used = 0;
	R_Printf(PRINT_ALL, "Loaded models:\n");

	for (i = 0, mod = mod_known; i < mod_numknown; i++, mod++)
	{
		char *in_use = "";

		if (mod->registration_sequence == registration_sequence)
		{
			in_use = "*";
			used ++;
		}

		if (!mod->name[0])
		{
			continue;
		}

		R_Printf(PRINT_ALL, "%8i : %s %s r: %.2f #%d\n",
			 mod->extradatasize, mod->name, in_use, mod->radius, mod->numsubmodels);
		total += mod->extradatasize;
	}

	R_Printf(PRINT_ALL, "Total resident: %i\n", total);
	// update statistics
	freeup = Mod_HasFreeSpace();
	R_Printf(PRINT_ALL, "Used %d of %d models%s.\n", used, mod_max, freeup ? ", has free space" : "");
}

void
GL3_Mod_Init(void)
{
	mod_max = 0;
	memset(mod_novis, 0xff, sizeof(mod_novis));
}

static void
Mod_LoadSubmodels(gl3model_t *loadmodel, const byte *mod_base, const lump_t *l)
{
	dmodel_t *in;
	gl3model_t *out;
	int i, j, count;

	in = (void *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
	{
		Com_Error(ERR_DROP, "%s: funny lump size in %s",
				__func__, loadmodel->name);
	}

	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc(count * sizeof(*out));

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for (i = 0; i < count; i++, in++, out++)
	{
		if (i == 0)
		{
			// copy parent as template for first model
			memcpy(out, loadmodel, sizeof(*out));
		}
		else
		{
			// copy first as template for model
			memcpy(out, loadmodel->submodels, sizeof(*out));
		}

		Com_sprintf (out->name, sizeof(out->name), "*%d", i);

		for (j = 0; j < 3; j++)
		{
			/* spread the mins / maxs by a pixel */
			out->mins[j] = LittleFloat(in->mins[j]) - 1;
			out->maxs[j] = LittleFloat(in->maxs[j]) + 1;
			out->origin[j] = LittleFloat(in->origin[j]);
		}

		out->radius = Mod_RadiusFromBounds(out->mins, out->maxs);
		out->firstnode = LittleLong(in->headnode);
		out->firstmodelsurface = LittleLong(in->firstface);
		out->nummodelsurfaces = LittleLong(in->numfaces);
		// visleafs
		out->numleafs = 0;
		//  check limits
		if (out->firstnode >= loadmodel->numnodes)
		{
			Com_Error(ERR_DROP, "%s: Inline model %i has bad firstnode",
					__func__, i);
		}
	}
}

static void
Mod_LoadFaces(gl3model_t *loadmodel, const byte *mod_base, const lump_t *l,
	const bspx_header_t *bspx_header)
{
	int i, count, surfnum, lminfosize;
	const dlminfo_t *lminfos;
	msurface_t *out;
	dface_t *in;

	in = (void *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
	{
		Com_Error(ERR_DROP, "%s: funny lump size in %s",
				__func__, loadmodel->name);
	}

	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc((count + EXTRA_LUMP_FACES) * sizeof(*out));

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	lminfos = Mod_LoadBSPXFindLump(bspx_header, "DECOUPLED_LM", &lminfosize, mod_base);
	if ((lminfos != NULL) &&
		(lminfosize / sizeof(dlminfo_t) != loadmodel->numsurfaces))
	{
		R_Printf(PRINT_ALL, "%s: [%s] decoupled_lm size " YQ2_COM_PRIdS " does not match surface count %d\n",
			__func__, loadmodel->name, lminfosize / sizeof(dlminfo_t), loadmodel->numsurfaces);
		lminfos = NULL;
	}

	LM_BeginBuildingLightmaps(loadmodel);

	for (surfnum = 0; surfnum < count; surfnum++, in++, out++)
	{
		int	side, ti, planenum, lightofs;

		out->firstedge = LittleLong(in->firstedge);
		out->numedges = LittleShort(in->numedges);

		if (out->numedges < 3)
		{
			Com_Error(ERR_DROP, "%s: Surface with %d edges",
					__func__, out->numedges);
		}
		out->flags = 0;
		out->polys = NULL;

		planenum = LittleShort(in->planenum);
		side = LittleShort(in->side);

		if (side)
		{
			out->flags |= SURF_PLANEBACK;
		}

		if (planenum < 0 || planenum >= loadmodel->numplanes)
		{
			Com_Error(ERR_DROP, "%s: Incorrect %d planenum.",
					__func__, planenum);
		}
		out->plane = loadmodel->planes + planenum;

		ti = LittleShort(in->texinfo);

		if ((ti < 0) || (ti >= loadmodel->numtexinfo))
		{
			Com_Error(ERR_DROP, "%s: bad texinfo number",
					__func__);
		}

		out->texinfo = loadmodel->texinfo + ti;

		lightofs = Mod_LoadBSPXDecoupledLM(lminfos, surfnum, out);
		if (lightofs < 0) {
			memcpy(out->lmvecs, out->texinfo->vecs, sizeof(out->lmvecs));
			out->lmshift = DEFAULT_LMSHIFT;
			out->lmvlen[0] = 1.0f;
			out->lmvlen[1] = 1.0f;

			Mod_CalcSurfaceExtents(loadmodel->surfedges, loadmodel->vertexes,
				loadmodel->edges, out);

			lightofs = in->lightofs;
		}

		Mod_LoadSetSurfaceLighting(loadmodel->lightdata, loadmodel->numlightdata,
			out, in->styles, lightofs);

		/* set the drawing flags */
		if (out->texinfo->flags & SURF_WARP)
		{
			out->flags |= SURF_DRAWTURB;

			for (i = 0; i < 2; i++)
			{
				out->extents[i] = 16384;
				out->texturemins[i] = -8192;
			}

			R_SubdivideSurface(loadmodel->surfedges, loadmodel->vertexes,
				loadmodel->edges, out); /* cut up polygon for warps */
		}

		if (r_fixsurfsky->value)
		{
			if (out->texinfo->flags & SURF_SKY)
			{
				out->flags |= SURF_DRAWSKY;
			}
		}

		/* create lightmaps and polygons */
		if (!(out->texinfo->flags & (SURF_SKY | SURF_TRANSPARENT | SURF_WARP)))
		{
			LM_CreateSurfaceLightmap(out);
		}

		if (!(out->texinfo->flags & SURF_WARP))
		{
			LM_BuildPolygonFromSurface(loadmodel, out);
		}
	}

	LM_EndBuildingLightmaps();
}

static void
Mod_LoadQFaces(gl3model_t *loadmodel, const byte *mod_base, const lump_t *l,
	const bspx_header_t *bspx_header)
{
	int i, count, surfnum, lminfosize;
	const dlminfo_t *lminfos;
	msurface_t *out;
	dqface_t *in;

	in = (void *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
	{
		Com_Error(ERR_DROP, "%s: funny lump size in %s",
				__func__, loadmodel->name);
	}

	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc((count + EXTRA_LUMP_FACES) * sizeof(*out));

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	lminfos = Mod_LoadBSPXFindLump(bspx_header, "DECOUPLED_LM", &lminfosize, mod_base);
	if ((lminfos != NULL) &&
		(lminfosize / sizeof(dlminfo_t) != loadmodel->numsurfaces))
	{
		R_Printf(PRINT_ALL, "%s: [%s] decoupled_lm size " YQ2_COM_PRIdS " does not match surface count %d\n",
			__func__, loadmodel->name, lminfosize / sizeof(dlminfo_t), loadmodel->numsurfaces);
		lminfos = NULL;
	}

	LM_BeginBuildingLightmaps(loadmodel);

	for (surfnum = 0; surfnum < count; surfnum++, in++, out++)
	{
		int	side, ti, planenum, lightofs;

		out->firstedge = LittleLong(in->firstedge);
		out->numedges = LittleLong(in->numedges);

		if (out->numedges < 3)
		{
			Com_Error(ERR_DROP, "%s: Surface with %d edges",
					__func__, out->numedges);
		}
		out->flags = 0;
		out->polys = NULL;

		planenum = LittleLong(in->planenum);
		side = LittleLong(in->side);

		if (side)
		{
			out->flags |= SURF_PLANEBACK;
		}

		if (planenum < 0 || planenum >= loadmodel->numplanes)
		{
			Com_Error(ERR_DROP, "%s: Incorrect %d planenum.",
					__func__, planenum);
		}
		out->plane = loadmodel->planes + planenum;

		ti = LittleLong(in->texinfo);

		if ((ti < 0) || (ti >= loadmodel->numtexinfo))
		{
			Com_Error(ERR_DROP, "%s: bad texinfo number",
					__func__);
		}

		out->texinfo = loadmodel->texinfo + ti;

		lightofs = Mod_LoadBSPXDecoupledLM(lminfos, surfnum, out);
		if (lightofs < 0) {
			memcpy(out->lmvecs, out->texinfo->vecs, sizeof(out->lmvecs));
			out->lmshift = DEFAULT_LMSHIFT;
			out->lmvlen[0] = 1.0f;
			out->lmvlen[1] = 1.0f;

			Mod_CalcSurfaceExtents(loadmodel->surfedges, loadmodel->vertexes,
				loadmodel->edges, out);

			lightofs = in->lightofs;
		}

		Mod_LoadSetSurfaceLighting(loadmodel->lightdata, loadmodel->numlightdata,
			out, in->styles, lightofs);

		/* set the drawing flags */
		if (out->texinfo->flags & SURF_WARP)
		{
			out->flags |= SURF_DRAWTURB;

			for (i = 0; i < 2; i++)
			{
				out->extents[i] = 16384;
				out->texturemins[i] = -8192;
			}

			R_SubdivideSurface(loadmodel->surfedges, loadmodel->vertexes,
				loadmodel->edges, out); /* cut up polygon for warps */
		}

		if (r_fixsurfsky->value)
		{
			if (out->texinfo->flags & SURF_SKY)
			{
				out->flags |= SURF_DRAWSKY;
			}
		}

		/* create lightmaps and polygons */
		if (!(out->texinfo->flags & (SURF_SKY | SURF_TRANSPARENT | SURF_WARP)))
		{
			LM_CreateSurfaceLightmap(out);
		}

		if (!(out->texinfo->flags & SURF_WARP))
		{
			LM_BuildPolygonFromSurface(loadmodel, out);
		}
	}

	LM_EndBuildingLightmaps();
}

static void
Mod_LoadBrushModel(gl3model_t *mod, const void *buffer, int modfilelen)
{
	const bspx_header_t *bspx_header;
	int i, lightgridsize = 0;
	maptype_t maptype;
	dheader_t *header;
	byte *mod_base;

	if (mod != mod_known)
	{
		Com_Error(ERR_DROP, "%s: Loaded a brush model after the world", __func__);
	}

	header = (dheader_t *)buffer;

	i = LittleLong(header->ident);

	if (i != IDBSPHEADER && i != QBSPHEADER)
	{
		Com_Error(ERR_DROP, "%s: %s has wrong ident (%i should be %i)",
				__func__, mod->name, i, IDBSPHEADER);
	}

	i = LittleLong(header->version);

	if (i != BSPVERSION && i != BSPDKMVERSION)
	{
		Com_Error(ERR_DROP, "%s: %s has wrong version number (%i should be %i)",
				__func__, mod->name, i, BSPVERSION);
	}

	/* swap all the lumps */
	mod_base = (byte *)header;

	for (i = 0; i < sizeof(dheader_t) / 4; i++)
	{
		((int *)header)[i] = LittleLong(((int *)header)[i]);
	}

	maptype = Mod_LoadValidateLumps(mod->name, header);
	if (maptype == map_quake2)
	{
		/* Can't detect use provided */
		maptype = r_maptype->value;
	}

	/* check for BSPX extensions */
	bspx_header = Mod_LoadBSPX(modfilelen, (byte*)header, maptype);

	// calculate the needed hunksize from the lumps
	int hunkSize = 0;
	hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_VERTEXES],
		sizeof(dvertex_t), sizeof(mvertex_t), EXTRA_LUMP_VERTEXES);
	if (header->ident == IDBSPHEADER)
	{
		hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_EDGES],
			sizeof(dedge_t), sizeof(medge_t), EXTRA_LUMP_EDGES);
	}
	else
	{
		hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_EDGES],
			sizeof(dqedge_t), sizeof(medge_t), EXTRA_LUMP_EDGES);
	}
	hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_SURFEDGES],
		sizeof(int), sizeof(int), EXTRA_LUMP_SURFEDGES);
	hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_LIGHTING],
		1, 1, 0);
	hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_PLANES],
		sizeof(dplane_t), sizeof(cplane_t), EXTRA_LUMP_PLANES);
	hunkSize += calcTexinfoFacesLeafsSize(mod_base, header);
	hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_VISIBILITY],
		1, 1, 0);
	if (header->ident == IDBSPHEADER)
	{
		if ((maptype == map_daikatana) &&
			(header->lumps[LUMP_LEAFS].filelen % sizeof(ddkleaf_t) == 0))
		{
			hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_LEAFS],
				sizeof(ddkleaf_t), sizeof(mleaf_t), 0);
		}
		else
		{
			hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_LEAFS],
				sizeof(dleaf_t), sizeof(mleaf_t), 0);
		}

		hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_NODES],
			sizeof(dnode_t), sizeof(mnode_t), EXTRA_LUMP_NODES);
	}
	else
	{
		hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_LEAFS],
			sizeof(dqleaf_t), sizeof(mleaf_t), 0);
		hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_NODES],
			sizeof(dqnode_t), sizeof(mnode_t), EXTRA_LUMP_NODES);
	}
	hunkSize += Mod_CalcLumpHunkSize(&header->lumps[LUMP_MODELS],
		sizeof(dmodel_t), sizeof(gl3model_t), 0);

	/* Get size of octree on disk, need to recheck real size */
	if (Mod_LoadBSPXFindLump(bspx_header, "LIGHTGRID_OCTREE", &lightgridsize, mod_base))
	{
		hunkSize += lightgridsize * 4;
	}

	mod->extradata = Hunk_Begin(hunkSize);
	mod->type = mod_brush;

	if (bspx_header)
	{
		mod->grid = Mod_LoadBSPXLightGrid(bspx_header, mod_base);
	}
	else
	{
		mod->grid = NULL;
	}

	/* load into heap */
	Mod_LoadVertexes(mod->name, &mod->vertexes, &mod->numvertexes, mod_base,
		&header->lumps[LUMP_VERTEXES]);
	Mod_LoadQBSPEdges(mod->name, &mod->edges, &mod->numedges,
		mod_base, &header->lumps[LUMP_EDGES], header->ident);
	Mod_LoadSurfedges(mod->name, &mod->surfedges, &mod->numsurfedges,
		mod_base, &header->lumps[LUMP_SURFEDGES]);
	Mod_LoadLighting(&mod->lightdata, &mod->numlightdata, mod_base,
		&header->lumps[LUMP_LIGHTING]);
	Mod_LoadPlanes(mod->name, &mod->planes, &mod->numplanes,
		mod_base, &header->lumps[LUMP_PLANES]);
	Mod_LoadTexinfo(mod->name, &mod->texinfo, &mod->numtexinfo,
		mod_base, &header->lumps[LUMP_TEXINFO], (findimage_t)GL3_FindImage,
		gl3_notexture, maptype);
	if (header->ident == IDBSPHEADER)
	{
		Mod_LoadFaces(mod, mod_base, &header->lumps[LUMP_FACES], bspx_header);
	}
	else
	{
		Mod_LoadQFaces(mod, mod_base, &header->lumps[LUMP_FACES], bspx_header);
	}
	Mod_LoadQBSPMarksurfaces(mod->name, &mod->marksurfaces, &mod->nummarksurfaces,
		mod->surfaces, mod->numsurfaces, mod_base, &header->lumps[LUMP_LEAFFACES],
		header->ident);
	Mod_LoadVisibility(mod->name, &mod->vis, &mod->numvisibility, mod_base,
		&header->lumps[LUMP_VISIBILITY]);
	Mod_LoadQBSPLeafs(mod->name, &mod->leafs, &mod->numleafs,
		mod->marksurfaces, mod->nummarksurfaces, mod_base,
		&header->lumps[LUMP_LEAFS], header->ident, maptype);
	Mod_LoadQBSPNodes(mod->name, mod->planes, mod->numplanes, mod->leafs,
		mod->numleafs, &mod->nodes, &mod->numnodes, mod_base,
		&header->lumps[LUMP_NODES], header->ident);
	Mod_LoadSubmodels(mod, mod_base, &header->lumps[LUMP_MODELS]);
	mod->numframes = 2; /* regular and alternate animation */
}

/*
 * Loads in a model for the given name
 */
static gl3model_t *
Mod_ForName(const char *name, gl3model_t *parent_model, qboolean crash)
{
	gl3model_t *mod;
	void *buf;
	int i, modfilelen;

	if (!name[0])
	{
		Com_Error(ERR_DROP, "%s: NULL name", __func__);
	}

	/* inline models are grabbed only from worldmodel */
	if (name[0] == '*' && parent_model)
	{
		i = (int)strtol(name + 1, (char **)NULL, 10);

		if (i < 1 || i >= parent_model->numsubmodels)
		{
			Com_Error(ERR_DROP, "%s: bad inline model number",
					__func__);
		}

		return &parent_model->submodels[i];
	}

	/* search the currently loaded models */
	for (i = 0, mod = mod_known; i < mod_numknown; i++, mod++)
	{
		if (!mod->name[0])
		{
			continue;
		}

		if (!strcmp(mod->name, name))
		{
			return mod;
		}
	}

	/* find a free model slot spot */
	for (i = 0, mod = mod_known; i < mod_numknown; i++, mod++)
	{
		if (!mod->name[0])
		{
			break; /* free spot */
		}
	}

	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
		{
			Com_Error(ERR_DROP, "%s: mod_numknown == MAX_MOD_KNOWN", __func__);
		}

		mod_numknown++;
	}

	strcpy(mod->name, name);

	/* load the file */
	modfilelen = Mod_LoadFile(mod->name, &buf);

	if (!buf)
	{
		if (crash)
		{
			Com_Error(ERR_DROP, "%s: %s not found",
					__func__, mod->name);
		}

		if (r_validation->value > 0)
		{
			R_Printf(PRINT_ALL, "%s: Can't load %s\n", __func__, mod->name);
		}

		memset(mod->name, 0, sizeof(mod->name));
		return NULL;
	}

	/* call the apropriate loader */
	switch (LittleLong(*(unsigned *)buf))
	{
		case MDXHEADER:
			/* fall through */
		case DKMHEADER:
			/* fall through */
		case RAVENFMHEADER:
			/* fall through */
		case IDALIASHEADER:
			/* fall through */
		case IDMDLHEADER:
			/* fall through */
		case ID3HEADER:
			/* fall through */
		case IDMD5HEADER:
			/* fall through */
		case IDSPRITEHEADER:
			{
				mod->extradata = Mod_LoadModel(mod->name, buf, modfilelen,
					mod->mins, mod->maxs,
					(struct image_s ***)&mod->skins, &mod->numskins,
					(findimage_t)GL3_FindImage, (loadimage_t)GL3_LoadPic,
					&(mod->type));
				if (!mod->extradata)
				{
					Com_Error(ERR_DROP, "%s: Failed to load %s",
						__func__, mod->name);
				}
			};
			break;

		case IDBSPHEADER:
			/* fall through */
		case QBSPHEADER:
			Mod_LoadBrushModel(mod, buf, modfilelen);
			break;

		default:
			Com_Error(ERR_DROP, "%s: unknown fileid for %s",
					__func__, mod->name);
			break;
	}

	mod->radius = Mod_RadiusFromBounds(mod->mins, mod->maxs);
	if (mod->extradata)
	{
		mod->extradatasize = Hunk_End();
	}
	else
	{
		mod->extradatasize = 0;
	}

	ri.FS_FreeFile(buf);

	return mod;
}

static void
Mod_Free(gl3model_t *mod)
{
	if (!mod->extradata)
	{
		/* looks as empty model */
		memset (mod, 0, sizeof(*mod));
		return;
	}

	if (r_validation->value > 0)
	{
		R_Printf(PRINT_ALL, "%s: Unload %s\n", __func__, mod->name);
	}

	Hunk_Free(mod->extradata);
	memset(mod, 0, sizeof(*mod));
}

void
GL3_Mod_FreeAll(void)
{
	int i;

	for (i = 0; i < mod_numknown; i++)
	{
		if (mod_known[i].extradatasize)
		{
			Mod_Free(&mod_known[i]);
		}
	}
}

/*
 * Specifies the model that will be used as the world
 */
void
GL3_BeginRegistration(const char *model)
{
	char fullname[MAX_QPATH];
	const cvar_t *flushmap;

	registration_sequence++;
	gl3_oldviewcluster = -1; /* force markleafs */

	gl3state.currentlightmap = -1;

	Com_sprintf(fullname, sizeof(fullname), "maps/%s.bsp", model);

	/* explicitly free the old map if different
	   this guarantees that mod_known[0] is the
	   world map */
	flushmap = ri.Cvar_Get("flushmap", "0", 0);

	if (strcmp(mod_known[0].name, fullname) || flushmap->value)
	{
		Mod_Free(&mod_known[0]);
	}

	gl3_worldmodel = Mod_ForName(fullname, NULL, true);

	gl3_viewcluster = -1;
}

struct model_s *
GL3_RegisterModel(const char *name)
{
	gl3model_t *mod;

	mod = Mod_ForName(name, gl3_worldmodel, false);

	if (mod)
	{
		mod->registration_sequence = registration_sequence;

		/* register any images used by the models */
		if (mod->type == mod_brush)
		{
			int i;

			for (i = 0; i < mod->numtexinfo; i++)
			{
				mod->texinfo[i].image->registration_sequence =
					registration_sequence;
			}
		}
		else
		{
			/* numframes is unused for SP2 but lets set it also  */
			mod->numframes = Mod_ReLoadSkins((struct image_s **)mod->skins,
				(findimage_t)GL3_FindImage, (loadimage_t)GL3_LoadPic,
				mod->extradata, mod->type);
		}
	}

	return mod;
}

void
GL3_EndRegistration(void)
{
	int i;
	gl3model_t *mod;

	if (Mod_HasFreeSpace() && GL3_ImageHasFreeSpace())
	{
		// should be enough space for load next maps
		return;
	}

	for (i = 0, mod = mod_known; i < mod_numknown; i++, mod++)
	{
		if (!mod->name[0])
		{
			continue;
		}

		if (mod->registration_sequence != registration_sequence)
		{
			/* don't need this model */
			Mod_Free(mod);
		}
	}

	GL3_FreeUnusedImages();
}
