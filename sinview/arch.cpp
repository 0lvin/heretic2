/*  SinView - Sin Pak File Viewer
    Copyright (C) 1998

    Trey Harrison trey@u.washington.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "arch.h"
#include "error.h"
#include <math.h>

scalar dot_product(vec3 *src1, vec3 *src2)
{
  return src1->x * src2->x + src1->y * src2->y + src1->z * src2->z;
}

void cross_product(vec3 *v1, vec3 *v2, vec3 *v3)
{
	v3->x = ((v1->y) * (v2->z)) - ((v2->y) * (v1->z));
	v3->y = ((v1->z) * (v2->x)) - ((v2->z) * (v1->x));
	v3->z = ((v1->x) * (v2->y)) - ((v2->x) * (v1->y));
}

void vec3_normalize(vec3 *v1, vec3 *dst)
{
  scalar oolength;
  scalar length = (scalar)sqrt((v1->x*v1->x) + (v1->y*v1->y) + (v1->z*v1->z));

  oolength = 1.f / length;
#ifdef _DEBUG
  //check for invalid oolength
  if (*((int *)&oolength) == 0x7F800000)
  {
    engine_error("Divide error","vec3_normalize()");
  }
#endif

  dst->x = v1->x * oolength;
  dst->y = v1->y * oolength;
  dst->z = v1->z * oolength;
}

void vec3_add(vec3 *s1, vec3 *s2, vec3 *dst)
{
  dst->x = s1->x + s2->x;
  dst->y = s1->y + s2->y;
  dst->z = s1->z + s2->z;
}

void vec3_sub(vec3 *s1, vec3 *s2, vec3 *dst)
{
  dst->x = s1->x - s2->x;
  dst->y = s1->y - s2->y;
  dst->z = s1->z - s2->z;
}

void vec3_scale(vec3 *s1, scalar *s, vec3 *dst)
{
  dst->x = *s * s1->x;
  dst->y = *s * s1->y;
  dst->z = *s * s1->z;
}

void SetIntelPrecision()
{
	long memvar;
	//taken directly from the Glide 2.0 programming Guide
	//dunno how or if this works but here it is anyway
	_asm {
		finit
		fwait
		fstcw memvar
		fwait
		mov eax,memvar
		and eax,0xfffffcff
		mov memvar,eax
		fldcw memvar
		fwait
	}
}
