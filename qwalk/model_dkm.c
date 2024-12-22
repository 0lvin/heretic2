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
Mod_LoadDTriangleList

Load DKM triangle lists
=================
*/
void
Mod_LoadDkmTriangleList (dmdl_t *pheader, dkmtriangle_t *pintri)
{
	dtriangle_t *pouttri;
	int i;

	pouttri = (dtriangle_t *) ((char *)pheader + pheader->ofs_tris);

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

/*
=================
Mod_DkmLoadFrames

Load the Dkm v2 frames
=================
*/
void
Mod_LoadDkmFrames (dmdl_t *pheader, const byte *src, size_t infamesize, vec3_t translate)
{
	int i;

	for (i=0 ; i<pheader->num_frames ; i++)
	{
		daliasframe_t	*pinframe, *poutframe;
		dtrivertx_t	*outverts;
		byte	*inverts;
		int j;

		pinframe = (daliasframe_t *) (src + i * infamesize);
		poutframe = (daliasframe_t *) ((byte *)pheader
			+ pheader->ofs_frames + i * pheader->framesize);

		memcpy (poutframe->name, pinframe->name, sizeof(poutframe->name));
		for (j=0 ; j<3 ; j++)
		{
			poutframe->scale[j] = LittleFloat (pinframe->scale[j]);
			poutframe->translate[j] = LittleFloat (pinframe->translate[j]);
			poutframe->translate[j] += translate[j];
		}

		poutframe->scale[0] *= 8;
		poutframe->scale[1] *= 4;
		poutframe->scale[2] *= 8;

		inverts = (char *)&pinframe->verts;
		outverts = poutframe->verts;
		/* dkm vert version 2 has unalligned by int size struct */
		for(j=0; j < pheader->num_xyz; j++)
		{
			int xyz;

			xyz = LittleLong (*((int *)inverts));
			outverts[j].v[0] = ((xyz & 0xFFE00000) >> (21 + 3)) & 0xFF;
			outverts[j].v[1] = ((xyz & 0x1FF800) >> (11 + 2)) & 0xFF;
			outverts[j].v[2] = ((xyz & 0x7FF) >> 3) & 0xFF;
			inverts += sizeof(int);
			outverts[j].lightnormalindex = *inverts;
			inverts ++;
		}
	}
}

bool_t
model_dkm_md2_load(void *filedata, size_t filesize, model_t *out_model, char **out_error)
{
	dkm_header_t header;
	dmdl_t dmdlheader, *pheader;
	int ofs_end, i;

	if (sizeof(dkm_header_t) > filesize)
	{
		return (void)(out_error && (*out_error = msprintf("too short file"))), false;
	}

	// byte swap the header fields and sanity check
	for (i=0 ; i<sizeof(dkm_header_t)/sizeof(int) ; i++)
		((int *)&header)[i] = LittleLong (((int *)filedata)[i]);

	if (header.ident != DKMHEADER)
	{
		return (void)(out_error && (*out_error = msprintf("wrong format (not %d)", DKMHEADER))), false;
	}

	if (header.version != DKM1_VERSION && header.version != DKM2_VERSION)
	{
		return (void)(out_error && (*out_error = msprintf("Unimplemented"))), false;
	}

	if (header.ofs_end < 0 || header.ofs_end > filesize)
	{
		return (void)(out_error && (*out_error = msprintf("Incorrect file size"))), false;
	}

	/* copy back all values */
	dmdlheader.ident = IDALIASHEADER;
	dmdlheader.version = ALIAS_VERSION;
	dmdlheader.skinwidth = 256;
	dmdlheader.skinheight = 256;
	if (header.version != DKM2_VERSION)
	{
		// has same frmae size structure
		dmdlheader.framesize = header.framesize;
	}
	else
	{
		dmdlheader.framesize = sizeof(daliasframe_t) - sizeof(dtrivertx_t);
		dmdlheader.framesize += header.num_xyz * sizeof(dtrivertx_t);
	}

	dmdlheader.num_skins = header.num_skins;
	dmdlheader.num_xyz = header.num_xyz;
	dmdlheader.num_st = header.num_st;
	dmdlheader.num_tris = header.num_tris;
	dmdlheader.num_glcmds = header.num_glcmds;
	dmdlheader.num_frames = header.num_frames;

	// just skip header and meshes
	dmdlheader.ofs_skins = sizeof(dmdl_t);
	dmdlheader.ofs_st = dmdlheader.ofs_skins + dmdlheader.num_skins * MAX_NAMESIZE;
	dmdlheader.ofs_tris = dmdlheader.ofs_st + dmdlheader.num_st * sizeof(dstvert_t);
	dmdlheader.ofs_frames = dmdlheader.ofs_tris + dmdlheader.num_tris * sizeof(dtriangle_t);
	dmdlheader.ofs_glcmds = dmdlheader.ofs_frames + dmdlheader.num_frames * dmdlheader.framesize;
	dmdlheader.ofs_end = dmdlheader.ofs_glcmds + dmdlheader.num_glcmds * sizeof(int);

	pheader = malloc(max(filesize, dmdlheader.ofs_end));
	memcpy(pheader, &dmdlheader, sizeof(dmdl_t));

	memcpy ((byte*)pheader + pheader->ofs_skins, (byte *)filedata + header.ofs_skins,
		pheader->num_skins * MAX_NAMESIZE);
	Mod_LoadSTvertList (pheader,
		(dstvert_t *)((byte *)filedata + header.ofs_st));
	Mod_LoadCmdList ("dkm", pheader,
		(int *)((byte *)filedata + header.ofs_glcmds));
	if (header.version == DKM1_VERSION)
	{
		Mod_LoadFrames (pheader, (byte *)filedata + header.ofs_frames,
			header.translate);
	}
	else
	{
		Mod_LoadDkmFrames (pheader, (byte *)filedata + header.ofs_frames,
			header.framesize, header.translate);
	}

	Mod_LoadDkmTriangleList (pheader,
		(dkmtriangle_t *)((byte *)filedata + header.ofs_tris));

	bool_t result = model_md2_load(pheader, pheader->ofs_end, out_model, out_error);
	if (pheader)
	{
		free(pheader);
	}
	return result;
}
