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
#include "engine_object.h"
#include <memory.h>
#include <stdio.h>
#include <string.h>

void init_box(vec3_box *b)
{
  b->min.x = 9999999.f;
  b->min.y = 9999999.f;
  b->min.z = 9999999.f;

  b->max.x = -9999999.f;
  b->max.y = -9999999.f;
  b->max.z = -9999999.f;
}

void adjust_box(vec3 *v, vec3_box *box)
{
  if (v->x < box->min.x)
    box->min.x = v->x;
  if (v->x > box->max.x)
    box->max.x = v->x;

  if (v->y < box->min.y)
    box->min.y = v->y;
  if (v->y > box->max.y)
    box->max.y = v->y;

  if (v->z < box->min.z)
    box->min.z = v->z;
  if (v->z > box->max.z)
    box->max.z = v->z;
}

void make_box_verts(vec3 *v, vec3_box *box)
{
  v[0].x = box->min.x;
  v[0].y = box->min.y;
  v[0].z = box->min.z;

  v[1].x = box->max.x;
  v[1].y = box->min.y;
  v[1].z = box->min.z;

  v[2].x = box->max.x;
  v[2].y = box->max.y;
  v[2].z = box->min.z;

  v[3].x = box->min.x;
  v[3].y = box->max.y;
  v[3].z = box->min.z;

  v[4].x = box->min.x;
  v[4].y = box->min.y;
  v[4].z = box->max.z;

  v[5].x = box->max.x;
  v[5].y = box->min.y;
  v[5].z = box->max.z;

  v[6].x = box->max.x;
  v[6].y = box->max.y;
  v[6].z = box->max.z;

  v[7].x = box->min.x;
  v[7].y = box->max.y;
  v[7].z = box->max.z;
}

engine_object_class::engine_object_class(char *object_name)
{
  sw32 i,j;

  memset(&origin,sizeof(vec3),0);
  memset(box_verts,sizeof(vec3)*8,0);

  num_vertices = num_faces = 0;
  vertices = 0;
  t_vertices = 0;
  faces = 0;

  if (!object_name)
    return;

  FILE *f = fopen(object_name,"rb");

  if (!f)
    return;

  strcpy(name,object_name);

  while (!feof(f))
  {
    char block_identifier = fgetc(f);

    if (block_identifier==EOF)
      break;

    if (block_identifier=='f') //faces
    {
      w16 numf;
      fread(&numf, sizeof(w16), 1, f);
      num_faces = numf;

      faces = new engine_face_class[num_faces];

      for (i=0; i<num_faces; i++)
      {
        w16 numv;
        fread(&numv, sizeof(w16), 1, f);

        fread(&faces[i].flags,sizeof(w8), 1, f);

        faces[i].num_vertices = numv;
        faces[i].vertex_indices = new sw32[numv];
        faces[i].v_info         = new vertex_info[numv];

        for (j=0; j<numv; j++)
        {
          faces[i].v_info[j].r = 255.f;
          faces[i].v_info[j].g = 255.f;
          faces[i].v_info[j].b = 255.f;

          w16 index;
          fread(&index, sizeof(w16), 1, f);
          faces[i].vertex_indices[j] = index;
        }
      }
    }

    if (block_identifier=='v') //vertices
    {
      w16 numv;
      fread(&numv, sizeof(w16), 1, f);
      num_vertices = numv;

      vertices   = new vec3[num_vertices];
      t_vertices = new r_vert[num_vertices];
      v_normals  = new vec3[num_vertices];

      for (i=0; i<num_vertices; i++)
      {
        fread(&vertices[i], sizeof(float)*3, 1, f);
      }
    }
  }

  //calculate bounding box
  vec3_box b;
  init_box(&b);

  for (i=0; i<num_vertices; i++)
  {
    adjust_box(&vertices[i], &b);
  }

  make_box_verts(box_verts, &b);

  //calculate face normals
  for (i=0; i<num_faces; i++)
  {
    vec3 v0,v1,v2,v_0,v_1;
    sw32 i0, i1, i2;

    i0 = faces[i].vertex_indices[0];
    i1 = faces[i].vertex_indices[1];
    i2 = faces[i].vertex_indices[2];

    v0 = vertices[i0];
    v1 = vertices[i1];
    v2 = vertices[i2];

    v_0.x = v1.x - v0.x;
    v_0.y = v1.y - v0.y;
    v_0.z = v1.z - v0.z;

    v_1.x = v2.x - v0.x;
    v_1.y = v2.y - v0.y;
    v_1.z = v2.z - v0.z;

    cross_product(&v_0,&v_1,&faces[i].normal);

    if (faces[i].normal.x==0.f && faces[i].normal.y==0.f && faces[i].normal.z==0.f)
    {
      faces[i].normal.z = 1.f;
    }

    //dont normalize yet
    //this will allow the larger faces to have more
    //weight when calculating vertex normals
    //vec3_normalize(&faces[i].normal,&faces[i].normal);
  }

  sw32 k;
  //calculate vertex normals
  for (i=0; i<num_vertices; i++)
  {
    vec3 norms,last_normal;

    norms.x = norms.y = norms.z = 0.f;

    last_normal = norms;
    last_normal.z = 1.f;

    for (j=0; j<num_faces; j++)
    {
      for (k=0; k<faces[j].num_vertices; k++)
      {
        if (faces[j].vertex_indices[k]==i)
        {
          last_normal = faces[j].normal;
          vec3_add(&norms, &faces[j].normal, &norms);
          break;
        }
      }
    }

    if (norms.x==0.f && norms.y==0.f && norms.z==0.f)
    {
      norms = last_normal;
    }

    vec3_normalize(&norms, &norms);
    v_normals[i] = norms;
  }

  for (i=0; i<num_faces; i++)
  {
    //NOW, normalize the face normals
    vec3_normalize(&faces[i].normal,&faces[i].normal);
  }
}

engine_object_class::~engine_object_class()
{
  sw32 i,j;

  w16 numv = num_vertices;
  w16 numf = num_faces;


  //write it out
  FILE *f = fopen(name,"wb");
  fputc('v',f); //vertex section

  fwrite(&numv, sizeof(w16), 1, f);
  for (i=0; i<numv; i++)
  {
    fwrite(&vertices[i].x, sizeof(float), 1, f);
    fwrite(&vertices[i].y, sizeof(float), 1, f);
    fwrite(&vertices[i].z, sizeof(float), 1, f);
  }

  fputc('f',f); //face section
  fwrite(&numf, sizeof(w16), 1, f);
  for (i=0; i<numf; i++)
  {
    numv = faces[i].num_vertices;
    fwrite(&numv, sizeof(w16), 1, f);

    fwrite(&faces[i].flags, sizeof(w8), 1, f);

    for (j=0; j<numv; j++)
    {
      w16 index = faces[i].vertex_indices[j];
      fwrite(&index, sizeof(w16), 1, f);
    }
  }

  fclose(f);

  if (faces)
  {
    delete [num_faces] faces;
    faces = 0;
  }

  num_faces = 0;

  if (vertices)
  {
    delete [num_vertices] vertices;
    vertices = 0;
  }

  if (t_vertices)
  {
    delete [num_vertices] t_vertices;
    t_vertices = 0;
  }

  num_vertices = 0;
}

