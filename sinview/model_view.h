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
#ifndef MODEL_VIEW_H
#define MODEL_VIEW_H

//damnit i hate including this fucking file in .h files
#include <windows.h>

class def_info_class;
class sin_model_class;
class engine_render_window_class;
class engine_texture_class;

class model_view_class
{
public:
  model_view_class();
  ~model_view_class();

  void view_windowed(def_info_class *def);
  void view_fullscreen(def_info_class *def);

private:
  HWND model_hwnd;
  sin_model_class *cur_model;
  engine_render_window_class *cur_render;
  engine_texture_class *cur_texture;
  char cur_skin_name[128];
  scalar frame;
  scalar speed;
  BITMAPINFO *bitmap_info;
  w16 bitmap[256*256];
};

#endif