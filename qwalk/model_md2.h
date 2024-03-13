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

#ifndef MODEL_MD2_H
#define MODEL_MD2_H

#define MAX_LBM_HEIGHT 480

extern const float anorms[162][3];

typedef struct md2_skin_s
{
	char name[MAX_NAMESIZE];
} md2_skin_t;

typedef struct qwalk_daliasframe_s
{
	float scale[3];
	float translate[3];
	char name[16];
/*	dtrivertx_t verts[1];*/
} qwalk_daliasframe_t;

void Mod_LoadSTvertList (dmdl_t *pheader, dstvert_t *pinst);
void Mod_LoadCmdList (const char *mod_name, dmdl_t *pheader, int *pincmd);
void Mod_LoadFrames (dmdl_t *pheader, byte *src, vec3_t translate);
void Mod_LoadDTriangleList (dmdl_t *pheader, dtriangle_t *pintri);

#endif
