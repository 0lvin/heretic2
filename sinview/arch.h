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
#ifndef ARCH_H
#define ARCH_H

typedef unsigned int   w32;
typedef signed int     sw32;
typedef unsigned short w16;
typedef signed short   sw16;
typedef unsigned char  w8;
typedef signed char    sw8;

typedef float scalar;

//less than zero (IEEE, is sign bit set?)
#define ltz(a)       (*((w32 *)a) & (1<<31))
#define nan_check(a) ( (*((w32 *)a) & (255<<23))==(255<<23) )
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef bool
#define bool w8
#endif

//vec3 is padded to 16 bytes
//matrix_asm.h depends on this!
typedef struct
{
  scalar x,y,z,w;
} vec3;

typedef struct
{
  vec3 min,max;
} vec3_box;

#define vec3_t vec3

typedef struct
{
  scalar r,g,b,a,s,t;
} vertex_info;

#define R_VERT_OUTCODE_CALCULATED (1)

typedef struct
{
  float x,y,z;
  float r,g,b,a,s,t;
  char  outcode;
  char  flags;
} r_vert;

scalar dot_product(vec3 *src1, vec3 *src2);
void   vec3_add(vec3 *s1, vec3 *s2, vec3 *dst);
void   vec3_sub(vec3 *s1, vec3 *s2, vec3 *dst);
void   vec3_scale(vec3 *s, scalar *scale, vec3 *dst);
void   cross_product(vec3 *src,vec3 *src2, vec3 *dst);
void   vec3_normalize(vec3 *src, vec3 *dst);
void   SetIntelPrecision();

#endif