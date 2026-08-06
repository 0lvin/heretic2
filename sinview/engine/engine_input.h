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
#ifndef _ENGINE_INPUT_H_
#define _ENGINE_INPUT_H_

#include "arch.h"
#include <windows.h>

class engine_input_class;
extern engine_input_class *main_input;

class engine_input_class
{
public:
  void init(HWND input_window);

  void lock_mouse();
  void unlock_mouse();

  bool key_down(w32 key);
  bool key_hit(w32 key);
  void get_mouse_move(sw32 *dx, sw32 *dy);

  sw32 last_x,last_y;
  bool mouse_locked;
  HWND hwnd;

  engine_input_class()
  {
    last_x       = -1;
    last_y       = -1;
    mouse_locked = FALSE;
    main_input   = this;
  }
};

#endif