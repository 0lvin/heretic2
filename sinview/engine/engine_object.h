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
#ifndef ENGINE_MODEL_H
#define ENGINE_MODEL_H

#include "arch.h"

class engine_face_class
{
public:
  enum {MAX_EDGES=32, MAX_VERTICES=32};

  vec3 normal;
  sw32 *vertex_indices;

  sw32 num_vertices;

  vertex_info *v_info;

  enum {ENV_FLAG=1};

  w8 flags;

  engine_face_class()
  {
    vertex_indices=0;
    flags=0;
    v_info=0;
    num_vertices=0;
    normal.x=normal.y=normal.z=0.f;
  }

  ~engine_face_class()
  {
  }
};

class engine_object_class
{
public:
  char name[64];

  vec3 origin;
  vec3 orientation;

  sw32 num_vertices;
  sw32 num_faces;

  vec3   *vertices;    //an array of vertices
  vec3   *v_normals;     //array of vertex normals
  r_vert *t_vertices;  //an array of transformed vertices

  engine_face_class *faces;

  vec3 box_verts[8];

  engine_object_class(char *object_name);
  ~engine_object_class();
};

#endif
