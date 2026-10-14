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
#ifndef _CLIP_DELTA_H_
#define _CLIP_DELTA_H_

#include "arch.h"

//i thought about writing this stuff in asm but
//never did

class clip_delta_class
{
public:
  scalar dx,dy,dz,dr,dg,db,da,ds,dt;

  void calculate(r_vert *v1, r_vert *v2)
  {
#ifdef USE_CLIP_DELTA_ASM
    calc_clip_deltas(this,v1,v2);
#else
    dx = v1->x - v2->x;
    dy = v1->y - v2->y;
    dz = v1->z - v2->z;
    dr = v1->r - v2->r;
    dg = v1->g - v2->g;
    db = v1->b - v2->b;
    da = v1->a - v2->a;
    ds = v1->s - v2->s;
    dt = v1->t - v2->t;
#endif
  }

  void generate_point_extras(r_vert *clip_point, r_vert *v, scalar *t)
  {
#ifdef USE_CLIP_DELTA_ASM
    generate_clip_point_extras(this,clip_point,v,t);
#else
    clip_point->r     = v->r + ((*t) * dr);
    clip_point->g     = v->g + ((*t) * dg);
    clip_point->b     = v->b + ((*t) * db);
    clip_point->a     = v->a + ((*t) * da);
    clip_point->s     = v->s + ((*t) * ds);
    clip_point->t     = v->t + ((*t) * dt);
    clip_point->flags = 0;
#endif
  }
};

#endif