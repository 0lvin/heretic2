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
 * The collision model. Slaps "boxes" through the world and checks if
 * they collide with the world model, entities or other boxes.
 *
 * =======================================================================
 */

#include "models.h"

#define MAX_MOD_KNOWN MAX_MODELS

typedef struct
{
	char name[MAX_QPATH];
	int extradatasize;
	void *extradata;
} model_t;
static int model_num;
static model_t mod_known[MAX_MOD_KNOWN];

static void
Mod_LoadSkinList_MD2(const char *mod_name, const void *buffer, int modfilelen,
	char **skins, int *numskins, char **frames, int *numframes)
{
	dmdl_t pinmodel;
	int i;

	if (modfilelen < sizeof(pinmodel))
	{
		Com_Printf("%s: %s has incorrect header size (%i should be " YQ2_COM_PRIdS ")\n",
				__func__, mod_name, modfilelen, sizeof(pinmodel));
		return;
	}

	for (i = 0; i < sizeof(pinmodel) / sizeof(int); i++)
	{
		((int *)&pinmodel)[i] = LittleLong(((int *)buffer)[i]);
	}

	if (pinmodel.version != ALIAS_VERSION)
	{
		Com_Printf("%s: %s has wrong version number (%i should be %i)\n",
				__func__, mod_name, pinmodel.version, ALIAS_VERSION);
	}

	if (pinmodel.ofs_end < 0 || pinmodel.ofs_end > modfilelen)
	{
		Com_Printf("%s: model %s file size(%d) too small, should be %d\n",
				__func__, mod_name, modfilelen, pinmodel.ofs_end);
	}

	if (pinmodel.num_skins < 0)
	{
		Com_Printf("%s: model %s file has incorrect skins count %d\n",
				__func__, mod_name, pinmodel.num_skins);
	}

	/* list of skins */
	*numskins = pinmodel.num_skins;
	*skins = malloc(pinmodel.num_skins * MAX_SKINNAME);

	memcpy(*skins, (char *)buffer + pinmodel.ofs_skins,
		pinmodel.num_skins * MAX_SKINNAME);

	/* list of frames */
	*numframes = pinmodel.num_frames;
	*frames = malloc(pinmodel.num_frames * 16);

	for (i = 0; i < pinmodel.num_frames; i++)
	{
		daliasframe_t *pinframe;

		pinframe = (daliasframe_t *)((char *)buffer + pinmodel.ofs_frames + i * pinmodel.framesize);

		memcpy((*frames) + 16 * i, pinframe->name, 16);
	}
}

static void
Mod_LoadSkinList(const char *mod_name, const void *buffer, int modfilelen,
	char **skins, int *numskins, char **frames, int *numframes)
{
	switch (LittleLong(*(unsigned *)buffer))
	{
		case IDALIASHEADER:
			Mod_LoadSkinList_MD2(mod_name, buffer, modfilelen,
				skins, numskins, frames, numframes);
			break;
	}
}

/* Add md5 to full file name */
static void
Mod_LoadFileInsertMD5(char *newname, const char *oldname, int size)
{
	const char *filename;

	filename = COM_SkipPath(oldname);
	memset(newname, 0, size);
	memcpy(newname, oldname, strlen(oldname) - strlen(filename));
	Q_strlcat(newname, "md5/", size);
	Q_strlcat(newname, filename, size);
}

static int
Mod_LoadFileMD5Merge(const char *namewe, void **buffer)
{
	int fullsize, filesize_anim, filesize, filesize_skins;
	char *final_buffer = NULL, *skins_list = NULL;
	void *anim_buffer = NULL, *skins_buffer = NULL;
	qboolean md5path = false;
	char newname[256];

	/* search mesh file */
	Q_strlcpy(newname, namewe, sizeof(newname));
	Q_strlcat(newname, ".md5mesh", sizeof(newname));
	filesize = FS_LoadFile(newname, buffer);

	/* check overwrite file */
	if (filesize <= 0)
	{
		char md5modelname[256];

		Mod_LoadFileInsertMD5(md5modelname, newname, sizeof(md5modelname));

		filesize = FS_LoadFile(md5modelname, buffer);
		/* no replace file */
		if (filesize <= 0)
		{
			return filesize;
		}

		md5path = true;
		strcpy(newname, md5modelname);
	}

	/* search animation file */
	memcpy(newname + strlen(newname) - strlen("mesh"), "anim", strlen("anim"));
	filesize_anim = FS_LoadFile(newname, &anim_buffer);
	if (filesize_anim <= 0)
	{
		FS_FreeFile(*buffer);
		return filesize;
	}

	/* search skins list */
	Q_strlcpy(newname, namewe, sizeof(newname));
	Q_strlcat(newname, ".md2", sizeof(newname));
	filesize_skins = FS_LoadFile(newname, &skins_buffer);
	if (filesize_skins > 0)
	{
		char *skins = NULL, *frames = NULL;
		int numskins = 0, numframes = 0, i;

		Mod_LoadSkinList(newname, skins_buffer, filesize_skins,
			&skins, &numskins, &frames, &numframes);
		FS_FreeFile(skins_buffer);

		/*
		 * 20 -> numSkins <num> | skin <num> "MAX_SKINNAME" + md5
		 * 25 -> numFramenames <num> | framename <num> 16 + md5
		 */
		skins_list = malloc((numskins + 1) * (MAX_SKINNAME + 20) +
			(numframes + 1) * (16 + 25));
		sprintf(skins_list, "\nnumSkins %d\n", numskins);
		for(i = 0; i < numskins; i++)
		{
			const char *skinname = skins + MAX_SKINNAME * i;

			if (!md5path)
			{
				sprintf(skins_list + strlen(skins_list), "skin %d \"%s\"\n",
					i, skinname);
			}
			else
			{
				char md5skinname[256];

				Mod_LoadFileInsertMD5(md5skinname, skinname, sizeof(md5skinname));

				sprintf(skins_list + strlen(skins_list), "skin %d \"%s\"\n",
					i, md5skinname);
			}
		}

		sprintf(skins_list + strlen(skins_list), "\nnumFramenames %d\n", numframes);
		for(i = 0; i < numframes; i++)
		{
			const char *framename = frames + 16 * i;

			sprintf(skins_list + strlen(skins_list), "framename %d \"%s\"\n",
				i, framename);
		}

		/* clean up original buffer */
		if (skins)
		{
			free(skins);
		}

		if (frames)
		{
			free(frames);
		}
	}

	/* prepare final file */
	fullsize = filesize + filesize_anim + 1;
	if (skins_list)
	{
		fullsize += strlen(skins_list);
	}

	/* allocate new buffer, ERR_FATAL on alloc fail */
	final_buffer = Z_Malloc(fullsize);

	/* copy combined information */
	memcpy(final_buffer, *buffer, filesize);
	if (skins_list)
	{
		memcpy(final_buffer + filesize, skins_list, strlen(skins_list));
		filesize += strlen(skins_list);
		free(skins_list);
	}
	final_buffer[filesize] = 0;
	memcpy(final_buffer + filesize + 1, anim_buffer, filesize_anim);

	/* Remove old buffers */
	FS_FreeFile(anim_buffer);
	FS_FreeFile(*buffer);

	*buffer = final_buffer;
	return fullsize;
}
/* Models cache logic */
void
Mod_AliasesInit(void)
{
	memset(mod_known, 0, sizeof(*mod_known));
	model_num = 0;
}

static void
Mod_AliasFree(model_t *mod)
{
	if (mod->extradata && mod->extradatasize)
	{
		Hunk_Free(mod->extradata);
	}

	memset(mod, 0, sizeof(model_t));
}

void
Mod_AliasesFreeAll(void)
{
	int i;

	for (i = 0; i < MAX_MOD_KNOWN; i++)
	{
		Mod_AliasFree(mod_known + i);
	}
	model_num = 0;
}

static const model_t *
Mod_AliasSave(const char *mod_name, int modfilelen, const void *buffer)
{
	model_t *mod;
	int i;

	mod = NULL;
	for (i = 0; i < MAX_MOD_KNOWN; i++)
	{
		if (!mod_known[i].extradatasize)
		{
			mod = &mod_known[i];
			break;
		}
	}

	if (!mod)
	{
		model_num = (model_num + 1) % MAX_MOD_KNOWN;
		mod = &mod_known[model_num];
		Com_DPrintf("%s: No free space. Clean up random for model: %s: %d Kb\n",
			__func__, mod_name, mod->extradatasize / 1024);
		/* free old stuff */
		Mod_AliasFree(mod);
	}

	mod->extradata = Mod_LoadModelFile(mod_name, buffer, modfilelen);
	if (!mod->extradata)
	{
		/* unrecognized format */
		return NULL;
	}

	mod->extradatasize = Hunk_End();

	strncpy(mod->name, mod_name, sizeof(mod->name) - 1);

	return mod;
}

static model_t *
Mod_FindModel(const char *name)
{
	size_t i;

	if (!name || !name[0])
	{
		return NULL;
	}

	for (i = 0; i < MAX_MOD_KNOWN; i++)
	{
		if (!strcmp(name, mod_known[i].name))
		{
			return &mod_known[i];
		}
	}
	return NULL;
}

static int
Mod_LoadFileWithoutExtModel(const char *namewe, size_t tlen, void **buffer)
{
	char newname[256];
	int filesize;

	/* Check ReRelease / Doom 3 / Quake 4 model */
	filesize = Mod_LoadFileMD5Merge(namewe, buffer);
	if (filesize > 0)
	{
		Com_DPrintf("%s: %s loaded as md5 (Doom 3)\n",
			__func__, namewe);
		return filesize;
	}

	/* Check Quake 3 model */
	Q_strlcpy(newname, namewe, sizeof(newname));
	Q_strlcpy(newname + tlen, ".mdr", sizeof(newname));
	filesize = FS_LoadFile(newname, buffer);
	if (filesize > 0)
	{
		Com_DPrintf("%s: %s loaded as mdr/md4 (Star Trek: Elite Force)\n",
			__func__, namewe);
		return filesize;
	}

	/* Check Quake 3 model */
	Q_strlcpy(newname, namewe, sizeof(newname));
	Q_strlcpy(newname + tlen, ".md3", sizeof(newname));
	filesize = FS_LoadFile(newname, buffer);
	if (filesize > 0)
	{
		Com_DPrintf("%s: %s loaded as md3 (Quake 3)\n",
			__func__, namewe);
		return filesize;
	}

	/* Check Heretic2 model */
	Q_strlcpy(newname, namewe, sizeof(newname));
	Q_strlcat(newname, ".fm", sizeof(newname));
	filesize = FS_LoadFile(newname, buffer);
	if (filesize > 0)
	{
		Com_DPrintf("%s: %s loaded as fm (Heretic 2)\n",
			__func__, namewe);
		return filesize;
	}

	/* Check SiN model def with include sbm/sam files */
	Q_strlcpy(newname + tlen, ".def", sizeof(newname));
	filesize = FS_LoadFile(newname, buffer);
	if (filesize > 0)
	{
		Com_DPrintf("%s: %s loaded as def/sbm/sam (SiN)\n",
			__func__, namewe);
		return filesize;
	}

	/* Check Anachronox model definition */
	Q_strlcpy(newname + tlen, ".mda", sizeof(newname));
	filesize = FS_LoadFile(newname, buffer);
	if (filesize > 0)
	{
		Com_DPrintf("%s: %s loaded as mda (Anachronox)\n",
			__func__, namewe);
		return filesize;
	}

	/* Check Quake 2 model */
	Q_strlcpy(newname + tlen, ".md2", sizeof(newname));
	filesize = FS_LoadFile(newname, buffer);
	if (filesize > 0)
	{
		Com_DPrintf("%s: %s loaded as md2 (Quake 2/Anachronox)\n",
			__func__, namewe);
		return filesize;
	}

	/* Check Kingpin model */
	Q_strlcpy(newname + tlen, ".mdx", sizeof(newname));
	filesize = FS_LoadFile(newname, buffer);
	if (filesize > 0)
	{
		Com_DPrintf("%s: %s loaded as mdx (Kingpin)\n",
			__func__, namewe);
		return filesize;
	}

	/* Check Daikatana model */
	Q_strlcpy(newname + tlen, ".dkm", sizeof(newname));
	filesize = FS_LoadFile(newname, buffer);
	if (filesize > 0)
	{
		Com_DPrintf("%s: %s loaded as dkm (Daikatana)\n",
			__func__, namewe);
		return filesize;
	}

	/* Check Quake model */
	Q_strlcpy(newname + tlen, ".mdl", sizeof(newname));
	filesize = FS_LoadFile(newname, buffer);
	if (filesize > 0)
	{
		Com_DPrintf("%s: %s loaded as mdl (Quake 1)\n",
			__func__, namewe);
		return filesize;
	}

	return -1;
}

typedef struct
{
	char *old;
	char *new;
} replacement_t;

/* Replacement of ReRelease models */
static const replacement_t replacements[] = {
	{"models/monsters/soldierh/tris", "models/monsters/soldier/tris"},
	{"models/monsters/gladb/tris", "models/monsters/gladiatr/tris"},
	{"models/monsters/boss5/tris", "models/monsters/boss1/tris"},
	{"models/monsters/bitch2/tris", "models/monsters/bitch/tris"},
	{"models/vault/monsters/tank/tris", "models/monsters/tank/tris"},
	{"models/vault/monsters/mutant/tris", "models/monsters/mutant/tris"},
	{"models/vault/monsters/flyer/tris", "models/monsters/flyer/tris"},
	{"models/vault/monsters/float/tris", "models/monsters/float/tris"}
};

const model_t *
Mod_StoreAliasModel(const char *name)
{
	char namewe[256];
	const char* ext;
	int filesize;
	void *buffer;
	size_t len;

	if (!name || !name[0])
	{
		return NULL;
	}

	ext = COM_FileExtension(name);
	if(!ext || !ext[0])
	{
		/* file has no extension */
		return NULL;
	}

	/* Remove the extension */
	len = (ext - name) - 1;
	if (len < 1)
	{
		return NULL;
	}

	memcpy(namewe, name, len);
	namewe[len] = 0;

	filesize = Mod_LoadFileWithoutExtModel(namewe, len, &buffer);
	if (filesize <= 0)
	{
		int i;

		/* Replace to other one if load failed */
		for (i = 0; i < sizeof(replacements) / sizeof(replacement_t); i++)
		{
			if (!strcmp(namewe, replacements[i].old))
			{
				Com_DPrintf("%s: %s tring to replace %s to %s.\n",
					__func__, name, namewe, replacements[i].new);
				filesize = Mod_LoadFileWithoutExtModel(replacements[i].new,
					strlen(replacements[i].new), &buffer);
				break;
			}
		}
	}

	if (filesize > 0)
	{
		const model_t *mod;

		/* save and convert */
		mod = Mod_AliasSave(name, filesize, buffer);
		if (buffer)
		{
			/* free old buffer */
			FS_FreeFile(buffer);
		}

		return mod;
	}

	return NULL;
}

const dmdxframegroup_t *
Mod_GetModelInfo(const char *name, int *num, float *mins, float *maxs)
{
	const model_t *mod;

	mod = Mod_FindModel(name);
	if (!mod)
	{
		mod = Mod_StoreAliasModel(name);
	}

	if (mod)
	{
		dmdx_t *paliashdr;

		paliashdr = (dmdx_t *)mod->extradata;

		if (num)
		{
			*num = paliashdr->num_animgroup;
		}

		if (mins && maxs && paliashdr->num_frames)
		{
			daliasxframe_t *frame;
			int i;

			frame = (daliasxframe_t *) ((byte *)mod->extradata + paliashdr->ofs_frames);

			VectorCopy(frame->translate, mins);
			VectorCopy(frame->translate, maxs);

			for (i = 0; i < paliashdr->num_frames; i++)
			{
				int j;

				frame = (daliasxframe_t *) ((byte *)mod->extradata
					+ paliashdr->ofs_frames + i * paliashdr->framesize);

				for (j = 0; j < 3; j++)
				{
					float curr;

					curr = frame->translate[j];

					if (mins[j] > curr)
					{
						mins[j] = curr;
					}

					curr += frame->scale[j] * 0xFFFF;

					if (maxs[j] < curr)
					{
						maxs[j] = curr;
					}
				}
			}
		}

		return (dmdxframegroup_t *)((char *)paliashdr + paliashdr->ofs_animgroup);
	}

	if (num)
	{
		*num = 0;
	}

	return NULL;
}

/*
=================
Mod_LoadFile
=================
*/
int
Mod_LoadFile(const char *name, void **buffer)
{
	char newname[256];
	char namewe[256];
	const char* ext;
	size_t len;

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

	*buffer = NULL;

	if (!strcmp(ext, "fm") ||
		!strcmp(ext, "ctc") ||
		!strcmp(ext, "def") ||
		!strcmp(ext, "dkm") ||
		!strcmp(ext, "mda") ||
		!strcmp(ext, "md2") ||
		!strcmp(ext, "md3") ||
		!strcmp(ext, "mdr") ||
		!strcmp(ext, "md5mesh") ||
		!strcmp(ext, "mdx") ||
		!strcmp(ext, "mdl"))
	{
		const model_t *mod;

		mod = Mod_FindModel(name);
		if (!mod)
		{
			mod = Mod_StoreAliasModel(name);
		}

		if (mod)
		{
			*buffer = Z_Malloc(mod->extradatasize);
			memcpy(*buffer, mod->extradata, mod->extradatasize);
			return mod->extradatasize;
		}
	}

	/* Remove the extension */
	len = (ext - name) - 1;
	if (len < 1)
	{
		return -1;
	}

	memcpy(namewe, name, len);
	namewe[len] = 0;

	if (!strcmp(ext, "bsp"))
	{
		int filesize;

		Q_strlcpy(newname, namewe, sizeof(newname));
		Q_strlcat(newname, ".", sizeof(newname));
		Q_strlcat(newname, ext, sizeof(newname));

		filesize = CM_LoadFile(newname, buffer);
		if (filesize > 0)
		{
			return filesize;
		}
	}

	Q_strlcpy(newname, namewe, sizeof(newname));
	Q_strlcat(newname, ".", sizeof(newname));
	Q_strlcat(newname, ext, sizeof(newname));

	return FS_LoadFile(newname, buffer);
}
