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
 * The models file format
 *
 * =======================================================================
 */

#include "../ref_shared.h"

/*
=================
Mod_LoadDLModel
=================
*/
static dmdl_t *
Mod_LoadDLModel (const char *mod_name, const void *buffer, int modfilelen, void **extradata,
	image_load_t image_load)
{
	const dmdlo_t		*pinmodel;
	int		version;
	dmdl_t		*pheader;

	/* local copy of all values */
	int skinwidth, skinheight, framesize;
	int num_skins, num_xyz, num_st, num_tris, num_glcmds, num_frames;
	int ofs_skins, ofs_st, ofs_tris, ofs_frames, ofs_glcmds, ofs_end;

	pinmodel = (dmdlo_t *)buffer;

	version = LittleLong (pinmodel->version);
	if (version != MDL_VERSION)
	{
		R_Printf(PRINT_ALL, "%s: %s has wrong version number (%i should be %i)",
				__func__, mod_name, version, MDL_VERSION);
		return NULL;
	}

	/* generate all offsets and sizes */
	num_skins = LittleLong (pinmodel->num_skins);
	skinwidth = LittleLong (pinmodel->skinwidth);
	skinheight = LittleLong (pinmodel->skinheight);
	num_xyz = LittleLong (pinmodel->num_xyz);
	num_st = num_xyz;
	num_tris = LittleLong (pinmodel->num_tris);
	num_glcmds = (
		(3 * num_tris) * sizeof(int) * 3 + /* 3 vert */
		(num_tris * sizeof(int)) + /* triangles count */
		sizeof(int) /* final zero */) / sizeof(int);
	num_frames = LittleLong (pinmodel->num_frames);
	framesize = sizeof(daliasframe_t) + sizeof (dtrivertx_t) * (num_xyz - 1);

	ofs_skins = sizeof(dmdl_t); // just skip header and go
	ofs_st = ofs_skins + num_skins * MAX_SKINNAME;
	ofs_tris = ofs_st + num_st * sizeof(dstvert_t);
	ofs_glcmds = ofs_tris + num_tris * sizeof(dtriangle_t);
	ofs_frames = ofs_glcmds + num_glcmds * sizeof(int);
	/* one less as single vertx in frame by default */
	ofs_end = ofs_frames + framesize * num_frames;

	/* validate */
	if (skinheight > MAX_LBM_HEIGHT)
	{
		R_Printf(PRINT_ALL, "%s: model %s has a skin taller than %d",
				__func__, mod_name, MAX_LBM_HEIGHT);
		return NULL;
	}

	if (skinwidth > MAX_LBM_HEIGHT)
	{
		R_Printf(PRINT_ALL, "%s: model %s has a skin wider than %d",
				__func__, mod_name, MAX_LBM_HEIGHT);
		return NULL;
	}

	if (num_xyz <= 0)
	{
		R_Printf(PRINT_ALL, "%s: model %s has no vertices",
				__func__, mod_name);
		return NULL;
	}

	if (num_xyz > MAX_VERTS)
	{
		R_Printf(PRINT_ALL, "%s: model %s has too many vertices",
				__func__, mod_name);
		return NULL;
	}

	if (num_tris <= 0)
	{
		R_Printf(PRINT_ALL, "%s: model %s has no triangles",
				__func__, mod_name);
		return NULL;
	}

	if (num_frames <= 0)
	{
		R_Printf(PRINT_ALL, "%s: model %s has no frames",
				__func__, mod_name);
		return NULL;
	}

	if (modfilelen < ofs_end)
	{
		R_Printf(PRINT_ALL, "%s: model %s is too big.",
				__func__, mod_name);
		return NULL;
	}

	*extradata = Hunk_Begin(ofs_end);
	pheader = Hunk_Alloc(ofs_end);

	/* copy back all values */
	pheader->ident = IDALIASHEADER;
	pheader->version = ALIAS_VERSION;
	pheader->skinwidth = skinwidth;
	pheader->skinheight = skinheight;
	pheader->framesize = framesize;

	pheader->num_skins = num_skins;
	pheader->num_xyz = num_xyz;
	pheader->num_st = num_st;
	pheader->num_tris = num_tris;
	pheader->num_glcmds = num_glcmds;
	pheader->num_frames = num_frames;

	pheader->ofs_skins = ofs_skins;
	pheader->ofs_st = ofs_st;
	pheader->ofs_tris = ofs_tris;
	pheader->ofs_frames = ofs_frames;
	pheader->ofs_glcmds = ofs_glcmds;
	pheader->ofs_end = ofs_end;

	{
		int i;
		const byte *curr_pos;

		struct mdl_triangle_t *triangles;
		struct mdl_texcoord_t *texcoords;

		curr_pos = (byte*)buffer + sizeof (struct mdl_header_t);

		// register all skins
		for (i = 0; i < num_skins; ++i)
		{
			char *out_pos;
			int skin_type;
			byte* image_buffer;

			out_pos = (char*)pheader + sizeof(dmdl_t);
			snprintf(out_pos + MAX_SKINNAME * i, MAX_SKINNAME, "%s#%d.tga", mod_name, i);

			/* skip type / int */
			/* 0 = simple, !0 = group */
			/* this program can't read models composed of group frames! */
			skin_type = LittleLong (((int *)curr_pos)[0]);
			curr_pos += sizeof(int);
			if (skin_type)
			{
				R_Printf(PRINT_ALL, "%s: model %s has unsupported skin type %d",
						__func__, mod_name, skin_type);
				return NULL;
			}

			image_buffer = (byte *)malloc (skinwidth * skinheight);
			memcpy(image_buffer, curr_pos, skinwidth * skinheight);
			curr_pos += skinwidth * skinheight;
			image_load(out_pos + MAX_SKINNAME * i, image_buffer,
				skinwidth, skinwidth,
				skinheight, skinheight,
				it_skin, 8);

			free (image_buffer);
		}

		/* texcoordinates */
		{
			dstvert_t *poutst = (dstvert_t *) ((byte *)pheader + ofs_st);

			texcoords = (struct mdl_texcoord_t *)curr_pos;
			curr_pos += sizeof (struct mdl_texcoord_t) * num_st;

			for(i = 0; i < num_st; i++)
			{
				/* Compute texture coordinates */
				poutst[i].s = LittleLong (texcoords[i].s);
				poutst[i].t = LittleLong (texcoords[i].t);

				if (texcoords[i].onseam)
				{
					poutst[i].s += skinwidth * 0.5f; /* Backface */
				}

				/* Scale s and t to range from 0.0 to 1.0 */
				poutst[i].s = (poutst[i].s + 0.5) / skinwidth;
				poutst[i].t = (poutst[i].t + 0.5) / skinheight;
			}
		}

		/* triangles */
		{
			dtriangle_t *pouttri = (dtriangle_t *) ((byte *)pheader + ofs_tris);

			triangles = (struct mdl_triangle_t *) curr_pos;
			curr_pos += sizeof (struct mdl_triangle_t) * num_tris;

			for (i=0 ; i<num_tris ; i++)
			{
				int j;

				for (j=0 ; j<3 ; j++)
				{
					pouttri[i].index_xyz[j] = LittleLong (triangles[i].vertex[j]);
					pouttri[i].index_st[j] = pouttri[i].index_xyz[j];
				}
			}
		}

		{
			int *glcmds = (int *) ((byte *)pheader + ofs_glcmds);

			/* commands */
			int j, *curr_com = glcmds;

			/* Draw each triangle */
			for (i = 0; i < num_tris; ++i)
			{
				*curr_com = 3;
				curr_com++;

				/* Draw each vertex */
				for (j = 0; j < 3; ++j)
				{
					float s,t;
					int index;

					index = triangles[i].vertex[j];

					/* Compute texture coordinates */
					s = LittleLong (texcoords[index].s);
					t = LittleLong (texcoords[index].t);

					if (!triangles[i].facesfront &&
						texcoords[index].onseam)
					{
						s += skinwidth * 0.5f; /* Backface */
					}

					/* Scale s and t to range from 0.0 to 1.0 */
					s = (s + 0.5) / skinwidth;
					t = (t + 0.5) / skinheight;

					memcpy(curr_com, &s, sizeof(s));
					curr_com++;
					memcpy(curr_com, &t, sizeof(t));
					curr_com++;
					memcpy(curr_com, &index, sizeof(index));
					curr_com++;
				}
			}

			*curr_com = 0;
			curr_com++;
		}

		/* register all frames */
		for (i = 0; i < num_frames; ++i)
		{
			daliasframe_t *frame;
			int frame_type;

			frame = (daliasframe_t *) ((byte *)pheader + ofs_frames + i * framesize);
			frame->scale[0] = LittleFloat (pinmodel->scale[0]);
			frame->scale[1] = LittleFloat (pinmodel->scale[1]);
			frame->scale[2] = LittleFloat (pinmodel->scale[2]);

			frame->translate[0] = LittleFloat (pinmodel->translate[0]);
			frame->translate[1] = LittleFloat (pinmodel->translate[1]);
			frame->translate[2] = LittleFloat (pinmodel->translate[2]);

			/* Read frame data */
			/* skip type / int */
			/* 0 = simple, !0 = group */
			/* this program can't read models composed of group frames! */
			frame_type = LittleLong (((int *)curr_pos)[0]);
			curr_pos += sizeof (frame_type);

			if (frame_type)
			{
				R_Printf(PRINT_ALL, "%s: model %s has unsupported frame type %d",
						__func__, mod_name, frame_type);
				return NULL;
			}
			/* skip bboxmin, bouding box min */
			curr_pos += sizeof(dtrivertx_t);
			/* skip bboxmax, bouding box max */
			curr_pos += sizeof(dtrivertx_t);

			memcpy(&frame->name, curr_pos, sizeof (char) * 16);
			curr_pos += sizeof (char) * 16;

			memcpy(&frame->verts[0], curr_pos,
				sizeof (dtrivertx_t) * num_xyz);
			curr_pos += sizeof (dtrivertx_t) * num_xyz;
		}
	}

	return pheader;
}

/*
=================
Mod_LoadAliasModel
=================
*/
static dmdl_t *
Mod_LoadAliasModel (const char *mod_name, const void *buffer, int modfilelen, void **extradata)
{
	int		i, j;
	dmdl_t		*pinmodel, *pheader;
	dstvert_t	*pinst, *poutst;
	dtriangle_t	*pintri, *pouttri;
	int		*pincmd, *poutcmd;
	int		version;
	int		ofs_end;

	pinmodel = (dmdl_t *)buffer;

	version = LittleLong (pinmodel->version);
	if (version != ALIAS_VERSION)
	{
		ri.Sys_Error(ERR_DROP, "%s: %s has wrong version number (%i should be %i)",
				__func__, mod_name, version, ALIAS_VERSION);
	}

	ofs_end = LittleLong(pinmodel->ofs_end);
	if (ofs_end < 0 || ofs_end > modfilelen)
	{
		R_Printf(PRINT_ALL, "%s: model %s file size(%d) too small, should be %d",
				__func__, mod_name, modfilelen, ofs_end);
		return NULL;
	}

	*extradata = Hunk_Begin(modfilelen);
	pheader = Hunk_Alloc(ofs_end);

	// byte swap the header fields and sanity check
	for (i=0 ; i<sizeof(dmdl_t)/sizeof(int) ; i++)
		((int *)pheader)[i] = LittleLong (((int *)buffer)[i]);

	if (pheader->skinheight > MAX_LBM_HEIGHT)
	{
		R_Printf(PRINT_ALL, "%s: model %s has a skin taller than %d",
				__func__, mod_name, MAX_LBM_HEIGHT);
		return NULL;
	}

	if (pheader->num_xyz <= 0)
	{
		R_Printf(PRINT_ALL, "%s: model %s has no vertices",
				__func__, mod_name);
		return NULL;
	}

	if (pheader->num_xyz > MAX_VERTS)
	{
		R_Printf(PRINT_ALL, "%s: model %s has too many vertices",
				__func__, mod_name);
		return NULL;
	}

	if (pheader->num_st <= 0)
	{
		R_Printf(PRINT_ALL, "%s: model %s has no st vertices",
				__func__, mod_name);
		return NULL;
	}

	if (pheader->num_tris <= 0)
	{
		R_Printf(PRINT_ALL, "%s: model %s has no triangles",
				__func__, mod_name);
		return NULL;
	}

	if (pheader->num_frames <= 0)
	{
		R_Printf(PRINT_ALL, "%s: model %s has no frames",
				__func__, mod_name);
		return NULL;
	}

	//
	// load base s and t vertices (not used in gl version)
	//
	pinst = (dstvert_t *) ((byte *)pinmodel + pheader->ofs_st);
	poutst = (dstvert_t *) ((byte *)pheader + pheader->ofs_st);

	for (i=0 ; i<pheader->num_st ; i++)
	{
		poutst[i].s = LittleShort (pinst[i].s);
		poutst[i].t = LittleShort (pinst[i].t);
	}

	//
	// load triangle lists
	//
	pintri = (dtriangle_t *) ((byte *)pinmodel + pheader->ofs_tris);
	pouttri = (dtriangle_t *) ((byte *)pheader + pheader->ofs_tris);

	for (i=0 ; i<pheader->num_tris ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			pouttri[i].index_xyz[j] = LittleShort (pintri[i].index_xyz[j]);
			pouttri[i].index_st[j] = LittleShort (pintri[i].index_st[j]);
		}
	}

	//
	// load the frames
	//
	for (i=0 ; i<pheader->num_frames ; i++)
	{
		daliasframe_t		*pinframe, *poutframe;

		pinframe = (daliasframe_t *) ((byte *)pinmodel
			+ pheader->ofs_frames + i * pheader->framesize);
		poutframe = (daliasframe_t *) ((byte *)pheader
			+ pheader->ofs_frames + i * pheader->framesize);

		memcpy (poutframe->name, pinframe->name, sizeof(poutframe->name));
		for (j=0 ; j<3 ; j++)
		{
			poutframe->scale[j] = LittleFloat (pinframe->scale[j]);
			poutframe->translate[j] = LittleFloat (pinframe->translate[j]);
		}
		// verts are all 8 bit, so no swapping needed
		memcpy (poutframe->verts, pinframe->verts,
			pheader->num_xyz*sizeof(dtrivertx_t));
	}

	//
	// load the glcmds
	//
	pincmd = (int *) ((byte *)pinmodel + pheader->ofs_glcmds);
	poutcmd = (int *) ((byte *)pheader + pheader->ofs_glcmds);
	for (i=0; i < pheader->num_glcmds; i++)
	{
		poutcmd[i] = LittleLong (pincmd[i]);
	}

	if (poutcmd[pheader->num_glcmds-1] != 0)
	{
		R_Printf(PRINT_ALL, "%s: Entity %s has possible last element issues with %d verts.\n",
			__func__, mod_name, poutcmd[pheader->num_glcmds-1]);
	}

	// register all skins
	memcpy ((char *)pheader + pheader->ofs_skins, (char *)pinmodel + pheader->ofs_skins,
		pheader->num_skins*MAX_SKINNAME);

	return pheader;
}

/*
=================
Mod_LoadDMDL
=================
*/
dmdl_t *
Mod_LoadDMDL (const char *mod_name, const void *buf, int modfilelen, void **extradata,
	image_load_t image_load)
{
	*extradata = NULL;

	switch (LittleLong(*(unsigned *)buf))
	{
	case IDALIASHEADER:
		return Mod_LoadAliasModel(mod_name, buf, modfilelen, extradata);

	case IDMDLHEADER:
		return Mod_LoadDLModel(mod_name, buf, modfilelen, extradata, image_load);

	default:
		return NULL;
	}
}

/*
=================
Mod_LoadFile
=================
*/
int
Mod_LoadFile(char *name, void **buffer)
{
	const char* ext;

	*buffer = NULL;

	if (!name)
	{
		return -1;
	}

	ext = COM_FileExtension(name);
	if(!ext[0])
	{
		/* file has no extension */
		return -1;
	}

	if (!strcmp(ext, "md2"))
	{
		char namewe[256], newname[256];
		int filesize, len;

		len = strlen(name);
		if (len < 5)
		{
			return -1;
		}

		/* Remove the extension */
		memset(namewe, 0, 256);
		memcpy(namewe, name, len - (strlen(ext) + 1));

		/* Check Quake model */
		snprintf(newname, sizeof(newname), "%s.mdl", namewe);
		filesize = ri.FS_LoadFile (name, buffer);
		if (filesize > 0)
		{
			return filesize;
		}
	}
	return ri.FS_LoadFile (name, buffer);
}
