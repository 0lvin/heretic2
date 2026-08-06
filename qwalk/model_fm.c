/*
    QShed <http://www.icculus.org/qshed>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "anorms.h"
#include "global.h"
#include "model.h"
#include "palettes.h"
#include "model_md2.h"

/*
=================
Mod_LoadSTvertList

load base s and t vertices (not used in gl version)
=================
*/
void
Mod_LoadSTvertList (dmdl_t *pheader, dstvert_t *pinst)
{
	dstvert_t *poutst;
	int i;

	poutst = (dstvert_t *) ((byte *)pheader + pheader->ofs_st);

	for (i=0 ; i<pheader->num_st ; i++)
	{
		poutst[i].s = LittleShort (pinst[i].s);
		poutst[i].t = LittleShort (pinst[i].t);
	}
}

/*
=================
Mod_LoadCmdList

Load the glcmds
=================
*/
void
Mod_LoadCmdList (const char *mod_name, dmdl_t *pheader, int *pincmd)
{
	int *poutcmd;
	int i;

	poutcmd = (int *)((char*)pheader + pheader->ofs_glcmds);
	for (i = 0; i < pheader->num_glcmds; i++)
	{
		poutcmd[i] = LittleLong (pincmd[i]);
	}

	if (poutcmd[pheader->num_glcmds-1] != 0)
	{
		printf("%s: Entity %s has possible last element issues with %d verts.\n",
			__func__, mod_name, poutcmd[pheader->num_glcmds-1]);
	}
}

/*
=================
Mod_LoadFrames

Load the frames
=================
*/
void
Mod_LoadFrames (dmdl_t *pheader, byte *src, vec3_t translate)
{
	int i;

	for (i=0 ; i<pheader->num_frames ; i++)
	{
		daliasframe_t		*pinframe, *poutframe;
		int j;

		pinframe = (daliasframe_t *) (src + i * pheader->framesize);
		poutframe = (daliasframe_t *) ((byte *)pheader
			+ pheader->ofs_frames + i * pheader->framesize);

		memcpy (poutframe->name, pinframe->name, sizeof(poutframe->name));
		for (j=0 ; j<3 ; j++)
		{
			poutframe->scale[j] = LittleFloat (pinframe->scale[j]);
			poutframe->translate[j] = LittleFloat (pinframe->translate[j]);
			poutframe->translate[j] += translate[j];
		}
		// verts are all 8 bit, so no swapping needed
		memcpy (poutframe->verts, pinframe->verts,
			pheader->num_xyz*sizeof(dtrivertx_t));
	}
}

/*
=================
Mod_LoadDTriangleList

Load triangle lists
=================
*/
void
Mod_LoadDTriangleList (dmdl_t *pheader, dtriangle_t *pintri)
{
	dtriangle_t *pouttri;
	int i;

	pouttri = (dtriangle_t *) ((byte *)pheader + pheader->ofs_tris);

	for (i=0 ; i<pheader->num_tris ; i++)
	{
		int j;

		for (j=0 ; j<3 ; j++)
		{
			pouttri[i].index_xyz[j] = LittleShort (pintri[i].index_xyz[j]);
			pouttri[i].index_st[j] = LittleShort (pintri[i].index_st[j]);
		}
	}
}

bool_t model_fm_md2_load(void *filedata, size_t filesize, model_t *out_model, char **out_error)
{
	char *src = (char *)filedata;
	dmdl_t *pheader = NULL;
	int version, size;
	bool_t result;

	while (filesize > 0)
	{
		char blockname[32];

		memcpy(blockname, src, sizeof(blockname));

		src += sizeof(blockname);
		version = *(int*)src;
		src += sizeof(version);
		size = *(int*)src;
		src += sizeof(size);
		filesize = filesize - sizeof(blockname) - sizeof(version) - sizeof(size);

		if (strncasecmp(blockname, "header", sizeof(blockname)) == 0)
		{
			dmdl_t dmdlheader;
			fmheader_t *header = (fmheader_t *)src;

			if (sizeof(fmheader_t) > size)
			{
				return (void)(out_error && (*out_error = msprintf("%s: Too short header", __func__))), false;
			}

			if (version != 2)
			{
				return (void)(out_error && (*out_error = msprintf("%s: Invalid %s version %d",
					__func__, blockname, version))), false;
			}

			/* copy back all values */
			dmdlheader.ident = IDALIASHEADER;
			dmdlheader.version = ALIAS_VERSION;
			dmdlheader.skinwidth = LittleLong (header->skinwidth);
			dmdlheader.skinheight = LittleLong (header->skinheight);
			dmdlheader.framesize = LittleLong (header->framesize);

			dmdlheader.num_skins = LittleLong (header->num_skins);
			dmdlheader.num_xyz = LittleLong (header->num_xyz);
			dmdlheader.num_st = LittleLong (header->num_st);
			dmdlheader.num_tris = LittleLong (header->num_tris);
			dmdlheader.num_glcmds = LittleLong (header->num_glcmds);
			dmdlheader.num_frames = LittleLong (header->num_frames);

			// just skip header and meshes
			dmdlheader.ofs_skins = sizeof(dmdl_t) + sizeof(short) * 2 * LittleLong (header->num_mesh_nodes);
			dmdlheader.ofs_st = dmdlheader.ofs_skins + dmdlheader.num_skins * MAX_NAMESIZE;
			dmdlheader.ofs_tris = dmdlheader.ofs_st + dmdlheader.num_st * sizeof(dstvert_t);
			dmdlheader.ofs_frames = dmdlheader.ofs_tris + dmdlheader.num_tris * sizeof(dtriangle_t);
			dmdlheader.ofs_glcmds = dmdlheader.ofs_frames + dmdlheader.num_frames * dmdlheader.framesize;
			dmdlheader.ofs_end = dmdlheader.ofs_glcmds + dmdlheader.num_glcmds * sizeof(int);

			if (dmdlheader.skinheight > MAX_LBM_HEIGHT)
			{
				return (void)(out_error && (*out_error = msprintf("%s: model %s has a skin taller than %d",
						__func__, blockname, MAX_LBM_HEIGHT))), false;
			}

			if (dmdlheader.num_xyz <= 0)
			{
				return (void)(out_error && (*out_error = msprintf("%s: block %s has no vertices",
						__func__, blockname))), false;
			}

			if (dmdlheader.num_xyz > MAX_VERTS)
			{
				return (void)(out_error && (*out_error = msprintf("%s: block %s has too many vertices",
						__func__, blockname))), false;
			}

			if (dmdlheader.num_st <= 0)
			{
				return (void)(out_error && (*out_error = msprintf("%s: block %s has no st vertices",
						__func__, blockname))), false;
			}

			if (dmdlheader.num_tris <= 0)
			{
				return (void)(out_error && (*out_error = msprintf("%s: block %s has no triangles",
						__func__, blockname))), false;
			}

			if (dmdlheader.num_frames <= 0)
			{
				return (void)(out_error && (*out_error = msprintf("%s: block %s has no frames",
						__func__, blockname))), false;
			}

			pheader = malloc(max(filesize, dmdlheader.ofs_end));

			memcpy(pheader, &dmdlheader, sizeof(dmdl_t));
		}
		else {
			if (!pheader)
			{
				return (void)(out_error && (*out_error = msprintf("%s: %s has broken header.",
					__func__, blockname))), false;
			}
			else if (strncasecmp(blockname, "skin", sizeof(blockname)) == 0)
			{
				if (version != 1)
				{
					return (void)(out_error && (*out_error = msprintf("%s: Invalid %s version %d",
						__func__, blockname, version))), false;
				}
				if (size != (pheader->num_skins * MAX_NAMESIZE))
				{
					return (void)(out_error && (*out_error = msprintf("%s: Invalid %s size",
						__func__, blockname))), false;
				}
				memcpy((char*) pheader + pheader->ofs_skins, src, size);
			}
			else if (strncasecmp(blockname, "st coord", sizeof(blockname)) == 0)
			{
				if (version != 1)
				{
					return (void)(out_error && (*out_error = msprintf("%s: Invalid %s version %d",
						__func__, blockname, version))), false;
				}
				if (size != (pheader->num_st * sizeof(dstvert_t)))
				{
					return (void)(out_error && (*out_error = msprintf("%s: Invalid %s size",
						__func__, blockname))), false;
				}

				Mod_LoadSTvertList (pheader, (dstvert_t *)src);
			}
			else if (strncasecmp(blockname, "tris", sizeof(blockname)) == 0)
			{
				if (version != 1)
				{
					return (void)(out_error && (*out_error = msprintf("%s: Invalid %s version %d",
						__func__, blockname, version))), false;
				}
				if (size != (pheader->num_tris * sizeof(dtriangle_t)))
				{
					return (void)(out_error && (*out_error = msprintf("%s: Invalid %s size",
						__func__, blockname))), false;
				}

				Mod_LoadDTriangleList (pheader, (dtriangle_t *) src);
 			}
			else if (strncasecmp(blockname, "frames", sizeof(blockname)) == 0)
			{
				vec3_t translate = {0, 0, 0};

				if (version != 1)
				{
					return (void)(out_error && (*out_error = msprintf("%s: Invalid %s version %d",
						__func__, blockname, version))), false;
				}
				if (size != (pheader->num_frames * pheader->framesize))
				{
					return (void)(out_error && (*out_error = msprintf("%s: Invalid %s size",
						__func__, blockname))), false;
				}

				Mod_LoadFrames (pheader, (char *)src, translate);
			}
			else if (strncasecmp(blockname, "glcmds", sizeof(blockname)) == 0)
			{
				if (version != 1)
				{
					return (void)(out_error && (*out_error = msprintf("%s: Invalid %s version %d",
						__func__, blockname, version))), false;
				}
				if (size != (pheader->num_glcmds * sizeof(int)))
				{
					return (void)(out_error && (*out_error = msprintf("%s: Invalid %s size",
						__func__, blockname))), false;
				}

				Mod_LoadCmdList (blockname, pheader, (int *)src);
			}
			else if (strncasecmp(blockname, "mesh nodes", sizeof(blockname)) == 0)
			{
				int num_mesh_nodes;

				num_mesh_nodes = (pheader->ofs_skins - sizeof(dmdl_t)) / sizeof(short) / 2;

				if (version != 3)
				{
					return (void)(out_error && (*out_error = msprintf("%s: Invalid %s version %d",
						__func__, blockname, version))), false;
				}
				/* 516 mesh node size */
				if (size != (num_mesh_nodes * 516))
				{
					return (void)(out_error && (*out_error = msprintf("%s: Invalid %s size",
						__func__, blockname))), false;
				}

				if (num_mesh_nodes > 0)
				{
					short *mesh_nodes;
					char *in_mesh = src;
					int i;

					mesh_nodes = (short *)((char*)pheader + sizeof(dmdl_t));
					for (i = 0; i < num_mesh_nodes; i++)
					{
						char tri_data[256];
						char vert_data[256];
						int j;

						/* 256 bytes of tri data */
						/* 256 bytes of vert data */
						/* 2 bytes of start */
						/* 2 bytes of number commands */
						memcpy(tri_data, in_mesh, 256);
						in_mesh += 256;
						memcpy(vert_data, in_mesh, 256);
						in_mesh += 256;
						mesh_nodes[i * 2] = LittleShort(*(short *)in_mesh);
						in_mesh += 2;
						mesh_nodes[i * 2 + 1] = LittleShort(*(short *)in_mesh);
						in_mesh += 2;

						printf("mesh %d: %d -> %d\n",
							i, mesh_nodes[i * 2], mesh_nodes[i * 2 + 1]);
						printf("tri:\n");
						for (j=0; j<256; j++)
						{
							printf(" 0x%2x", tri_data[j]);
							if (j % 16 == 15)
							{
								printf("\n");
							}
						}
						printf("vert:\n");
						for (j=0; j<256; j++)
						{
							printf(" 0x%2x", vert_data[j]);
							if (j % 16 == 15)
							{
								printf("\n");
							}
						}
					}
				}
			}
			else if (strncasecmp(blockname, "normals", sizeof(blockname)) == 0 ||
					 strncasecmp(blockname, "short frames", sizeof(blockname)) == 0 ||
					 strncasecmp(blockname, "comp data", sizeof(blockname)) == 0 ||
					 strncasecmp(blockname, "skeleton", sizeof(blockname)) == 0 ||
					 strncasecmp(blockname, "references", sizeof(blockname)) == 0)
			{
				/* skipped block */
			}
			else
			{
				return (void)(out_error && (*out_error = msprintf("%s: %s Unknown block %s\n",
					__func__, blockname, blockname))), false;
			}
		}
		filesize -= size;
		src += size;
	}

	result = model_md2_load(pheader, pheader->ofs_end, out_model, out_error);
	if (pheader)
	{
		free(pheader);
	}
	return result;
}

