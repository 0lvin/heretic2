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
#ifndef SIN_MODEL_H
#define SIN_MODEL_H

#include "arch.h"

struct trivertx_t
{
  unsigned char x,y,z,normal_index;
};

struct st_vert_t
{
	float	s,t;
};

struct sin_triangle_t
{
  short	index_xyz[3];
  short	index_st[3];
  int   id;
};

struct sin_trigroup_t
{
  int id;
  int num_tris;
  int num_glcmds;		// dwords in strip/fan command list
  int ofs_glcmds;
  int ofs_tris;
  int ofs_end;
};

struct sin_sbm_header_t
{
  int ident;
  int version;

  int num_xyz;
  int num_st;			// greater than num_xyz for seams
  int num_groups;

  int ofs_st;			// byte offset from start for stverts
  int ofs_end;		// end of file
  //sin_trigroup_t groups[1]; //variable number of these
};

struct sin_frame_t
{
  float movedelta[3]; // used for driving the model around
  float frametime;
	float	scale[3];	// multiply byte verts by this
	float	translate[3];	// then add this
  int   ofs_verts;
};

struct sin_sam_header_t
{
  int   ident;
  int   version;
  char  name[64];
  float	scale[3];	// multiply byte verts by this
  float	translate[3];	// then add this
  float totaldelta[3]; // total displacement of this animation
  float totaltime;
  int   num_xyz;
  int   num_frames;
  int   ofs_frames;
  int   ofs_end;		   // end of file
};

class sin_model_class
{
public:
  sin_model_class(char *base_model_name);

  char base_name[128]; //the name of the base model being used
  char animation_name[128]; //the animation this model is playing

  void set_animation(char *animation_name);

  ~sin_model_class();

  vec3 origin;
  vec3 orientation;

  //there are "groups" of triangles in each model
  int            num_tri_groups;
  int            *num_triangles; //num_tris[0] tells how many tris are in group 0
  sin_triangle_t **triangles; //(tri_group[0] holds a pointer to triangles group 0)

  //and one big collection of vertices
  int    num_frames;
  int    num_verts;
  vec3_t **vertices; //vertices[0] holds a pointer to vertices of frame 0
  w8     **normal_indices; //vertex normals for all the frames

  r_vert *t_vertices; //transformed verts used for rendering

  //(s,t) data doesnt change in animations/frames/etc
  int        num_st_verts;
  st_vert_t *st_verts;

  int frame;
  float ratio;

private:
  void free_animation();
  void free_base();
};

#endif