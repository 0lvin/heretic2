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
#ifndef RENDER_WINDOW_H
#define RENDER_WINDOW_H

#include "arch.h"
#include "matrix.h"

class sin_model_class;
struct st_vert_t;
struct sin_triangle_t;

class engine_face_class;
class engine_texture_class;

class engine_render_window_class
{
public:
  engine_render_window_class(w32 width, w32 height, w32 hwnd);
  ~engine_render_window_class();

  void activate();
  void deactivate();

  void flush(bool clear_after_flush);
  void take_screen_shot();
  void grab_framebuffer(w16 *dst_buffer);
  void draw_object(sin_model_class *o);

  //these are hardcoded to work only with this demo
  //texture.cpp
  engine_texture_class *register_texture(char *texname);
  void set_texture(engine_texture_class *t);
  void free_texture(engine_texture_class *t);

  sw32 width,height;
  float width_o2, height_o2;

  vec3 camera;
  vec3 camera_orientation;
  matrix_class camera_transform;
  void update_camera_transform(); //calculates camera_transform
                                  //according to camera and
                                  //camera_orientation




  vec3  light_dir; //lighting direction
  scalar l_intensity[3];//rgb light intensity

private:
  void clip_and_draw_tri(sin_triangle_t *t,
                         r_vert *t_vertices,
                         vertex_info *v_info);

  void draw_poly(sw32 num_vertices, r_vert *r_list);

  w32   hwnd;
  bool  glide_open;

  scalar s_scale,t_scale; //for whatever the current texture is
};

#endif