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
#include <stdio.h>
#include <string.h>
#include "sin_model.h"
#include "error.h"
#include "file.h"

void sin_model_class::set_animation(char *anim_name)
{
  free_animation();

  file_class *f = open_file(anim_name, FILE_READ);

  if (!f)
  {
    engine_warning("Unable to load animation","sin_model_class()");
    return;
  }

  //assumes F is pointing right at the sam header
  sin_sam_header_t sam_header;

  //read the header
  f->read(&sam_header,sizeof(sin_sam_header_t));

  num_verts  = sam_header.num_xyz;
  num_frames = sam_header.num_frames;

  //allocate t_vertices
  t_vertices = new r_vert[num_verts];

  //read in the frame information
  f->seek(sam_header.ofs_frames);

  sin_frame_t *frames = new sin_frame_t[num_frames];
  f->read(frames, sizeof(sin_frame_t) * num_frames);

  //now read vertex info for each frame
  vertices = new vec3_t *[num_frames];
  normal_indices = new w8 *[num_frames];

  sw32 i,j;
  for (i=0; i<num_frames; i++)
  {
    //allocate room for the vertices of this frame
    vertices[i] = new vec3_t[num_verts];
    normal_indices[i] = new w8[num_verts];

    //the ofs_verts variable is .. a strange offset
    int seek_offset = sizeof(sin_sam_header_t) + sizeof(sin_frame_t)*i + frames[i].ofs_verts;

    f->seek(seek_offset);

    vec3_t *v = vertices[i];
    w8 *n = normal_indices[i];
    for (j=0; j<num_verts; j++,v++,n++)
    {
      trivertx_t vtx;
      f->read(&vtx, 4);

      //do this crap during the load. could do it during the render
      //to save memory space but this is just a viewer =)
      v->x = (frames[i].scale[0] * vtx.x) + frames[i].translate[0];
      v->y = (frames[i].scale[1] * vtx.y) + frames[i].translate[1];
      v->z = (frames[i].scale[2] * vtx.z) + frames[i].translate[2];
      *n = vtx.normal_index;
    }
  }

  //dont need this crap anymore
  delete [sam_header.num_frames] frames;

  close_file(f);
  strcpy(animation_name,anim_name);
}

sin_model_class::sin_model_class(char *_base_name)
{
  num_tri_groups = 0;
  triangles      = 0;
  num_triangles  = 0;
  num_st_verts   = 0;
  st_verts       = 0;
  base_name[0]   = 0;
  animation_name[0] = 0;
  num_verts  = 0;
  num_frames = 1;
  t_vertices = 0;
  vertices   = 0;
  normal_indices = 0;

  //assumes F is pointing right at the sbm header
  file_class *f = open_file(_base_name, FILE_READ);

  if (!f)
  {
    engine_warning("Unable to load base model","sin_model_class()");
    return;
  }

  sin_sbm_header_t sbm_header;

  //read the header
  f->read(&sbm_header, sizeof(sin_sbm_header_t));

  //read in the tri groups
  sin_trigroup_t *tg = new sin_trigroup_t[sbm_header.num_groups];
  f->read(tg, sbm_header.num_groups * sizeof(sin_trigroup_t));

  num_tri_groups = sbm_header.num_groups;

  //read in the tri data for each group
  triangles     = new sin_triangle_t *[sbm_header.num_groups];
  num_triangles = new int [sbm_header.num_groups];

  sw32 i;
  for (i=0; i<sbm_header.num_groups; i++)
  {
    num_triangles[i] = tg[i].num_tris;
    triangles[i]     = new sin_triangle_t[tg[i].num_tris];

    //the ofs_tris variable is .. a strange offset
    int seek_offset = sizeof(sin_sbm_header_t) + sizeof(sin_trigroup_t)*i + tg[i].ofs_tris;

    f->seek(seek_offset);
    f->read(triangles[i], tg[i].num_tris * sizeof(sin_triangle_t));
  }

  //dont need this anymore
  delete tg;

  //read in the st data
  num_st_verts = sbm_header.num_st;

  f->seek(sbm_header.ofs_st);
  st_verts = new st_vert_t[sbm_header.num_st];

  f->read(st_verts, sbm_header.num_st * sizeof(st_vert_t));

  close_file(f);
  strcpy(base_name, _base_name);
}

void sin_model_class::free_animation()
{
  if (vertices)
  {
    sw32 i;
    for (i=0; i<num_frames; i++)
    {
      delete [num_verts] vertices[i];
      vertices[i] = 0;
    }
    delete [num_frames] vertices;
    vertices = 0;
  }

  if (normal_indices)
  {
    sw32 i;
    for (i=0; i<num_frames; i++)
    {
      delete [num_verts] normal_indices[i];
      normal_indices[i] = 0;
    }
    delete [num_frames] normal_indices;
    normal_indices = 0;
  }

  if (t_vertices)
  {
    delete [num_verts] t_vertices;
    t_vertices = 0;
  }

  num_frames=1;
  num_verts=0;
  animation_name[0] = 0;
}

void sin_model_class::free_base()
{
  if (triangles)
  {
    if (!num_triangles)
      engine_error("sin_model_class::free_base()","Unexpected error");

    sw32 i;
    for (i=0; i<num_tri_groups; i++)
    {
      delete [num_triangles[i]] triangles[i];
      triangles[i] = 0;
    }

    delete [num_tri_groups] triangles;
    triangles = 0;
  }

  if (num_triangles)
  {
    delete [num_tri_groups] num_triangles;
    num_triangles = 0;
  }

  if (st_verts)
  {
    delete [num_st_verts] st_verts;
    st_verts = 0;
  }

  num_st_verts = 0;
  num_tri_groups = 0;
  base_name[0] = 0;
}

sin_model_class::~sin_model_class()
{
  free_animation();
  free_base();
}