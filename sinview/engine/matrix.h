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
#ifndef _MATRIX_H
#define _MATRIX_H

#include "arch.h"
#include <math.h>

#define USE_TRANSFORM_ASM

#include "matrix_asm.h"

class matrix_class
{
public:
  vec3_t x,y,z,t;

  matrix_class()
  {
    x.x = 1.f; x.y = 0.f; x.z = 0.f;
    y.x = 0.f; y.y = 1.f; y.z = 0.f;
    z.x = 0.f; z.y = 0.f; z.z = 1.f;

    t.x = 0.f; t.y = 0.f; t.z = 0.f;
  }

  void translate(scalar _x, scalar _y, scalar _z)
  {
    x.x = 1.f; x.y = 0.f; x.z = 0.f;
    y.x = 0.f; y.y = 1.f; y.z = 0.f;
    z.x = 0.f; z.y = 0.f; z.z = 1.f;

    t.x = _x; t.y = _y; t.z = _z;
  }

  void identity()
  {
    x.x = 1.f; x.y = 0.f; x.z = 0.f;
    y.x = 0.f; y.y = 1.f; y.z = 0.f;
    z.x = 0.f; z.y = 0.f; z.z = 1.f;

    t.x = 0.f; t.y = 0.f; t.z = 0.f;
  }

  void multiply(matrix_class &a, matrix_class &b)
  {
    // expanded transform multiply
    x.x = a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z;
    x.y = a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z;
    x.z = a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z;
    y.x = a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z;
    y.y = a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z;
    y.z = a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z;
    z.x = a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z;
    z.y = a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z;
    z.z = a.x.z*b.z.x + a.y.z*b.z.y + a.z.z*b.z.z;
    t.x = a.x.x*b.t.x + a.y.x*b.t.y + a.z.x*b.t.z + a.t.x;
    t.y = a.x.y*b.t.x + a.y.y*b.t.y + a.z.y*b.t.z + a.t.y;
    t.z = a.x.z*b.t.x + a.y.z*b.t.y + a.z.z*b.t.z + a.t.z;
  }

  void rotate_z(scalar th)
  {
    scalar cos_th = (scalar)cos(th);
    scalar sin_th = (scalar)sin(th);

    x.x = cos_th;  x.y = sin_th; x.z = 0.f;
    y.x = -sin_th; y.y = cos_th; y.z = 0.f;
    z.x = 0.f;     z.y = 0.f;    z.z = 1.f;

    t.x = 0.f;     t.y = 0.f;    t.z = 0.f;
  }

  void rotate_y(scalar th)
  {
    scalar cos_th = (scalar)cos(th);
    scalar sin_th = (scalar)sin(th);

    x.x = cos_th;  x.y = 0.f;  x.z = -sin_th;
    y.x = 0.f;     y.y = 1.f;  y.z = 0.f;
    z.x = sin_th;  z.y = 0.f;  z.z = cos_th;

    t.x = 0.f;     t.y = 0.f;  t.z = 0.f;
  }

  void rotate_x(scalar th)
  {
    scalar cos_th = (scalar)cos(th);
    scalar sin_th = (scalar)sin(th);

    x.x = 1.f;  x.y = 0.f;      x.z = 0.f;
    y.x = 0.f;  y.y = cos_th;   y.z = sin_th;
    z.x = 0.f;  z.y = -sin_th;  z.z = cos_th;

    t.x = 0.f;  t.y = 0.f;      t.z = 0.f;
  }

  //a concatenate multiply
  void multiply(matrix_class &b)
  {
    matrix_class tmp;
    tmp.multiply(*this,b);
    *this = tmp;
  }

  void inverse_transform(vec3_t &src, vec3_t &dst)
  {
    vec3_t tmp = src;

    tmp.x -= t.x;
    tmp.y -= t.y;
    tmp.z -= t.z;

    dst.x = tmp.x*x.x + tmp.y*x.y + tmp.z*x.z;
    dst.y = tmp.x*y.x + tmp.y*y.y + tmp.z*y.z;
    dst.z = tmp.x*z.x + tmp.y*z.y + tmp.z*z.z;
  }

  void inverse_transform_3x3(vec3_t &src, vec3_t &dst)
  {
    vec3_t tmp = src;

    dst.x = tmp.x*x.x + tmp.y*x.y + tmp.z*x.z;
    dst.y = tmp.x*y.x + tmp.y*y.y + tmp.z*y.z;
    dst.z = tmp.x*z.x + tmp.y*z.y + tmp.z*z.z;
  }

#ifdef USE_TRANSFORM_ASM

  void transform(vec3_t &src, vec3_t &dst)
  {
    trans_4x3(this, &src, &dst);
  }

  void transform_3x3(vec3_t &src, vec3_t &dst)
  {
    trans_3x3(this, &src, &dst);
  }

#else //#ifdef USE_TRANSFORM_ASM

  void transform(vec3_t &src, vec3_t &dst)
  {
    dst.x = x.x*src.x + y.x*src.y + z.x*src.z + t.x;
    dst.y = x.y*src.x + y.y*src.y + z.y*src.z + t.y;
    dst.z = x.z*src.x + y.z*src.y + z.z*src.z + t.z;
  }

  void transform_3x3(vec3_t &src, vec3_t &dst)
  {
    dst.x = x.x*src.x + y.x*src.y + z.x*src.z;
    dst.y = x.y*src.x + y.y*src.y + z.y*src.z;
    dst.z = x.z*src.x + y.z*src.y + z.z*src.z;
  }

#endif//#ifdef USE_TRANSFORM_ASM

};

#endif

