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
#include <windows.h>
#include "engine_input.h"

engine_input_class *main_input = 0;

void engine_input_class::init(HWND inputwindow)
{
  POINT pt;
  GetCursorPos(&pt);
  ScreenToClient(inputwindow,&pt);

  last_x = pt.x;
	last_y = pt.y;
  hwnd   = inputwindow;
}

void engine_input_class::lock_mouse()
{
  mouse_locked = TRUE;

  //lock it in the middle of the screen
  SetCursorPos(GetSystemMetrics(SM_CXSCREEN)/2, GetSystemMetrics(SM_CYSCREEN)/2);

  POINT pt;
  GetCursorPos(&pt);
  ScreenToClient(hwnd,&pt);
  last_x = pt.x;
  last_y = pt.y;
}

void engine_input_class::unlock_mouse()
{
  mouse_locked = FALSE;
}

bool engine_input_class::key_down(w32 key)
{
  if (GetAsyncKeyState(key) & (1<<31)) return TRUE;
  return FALSE;
}

bool engine_input_class::key_hit(w32 key)
{
  if (GetAsyncKeyState(key) & 1) return TRUE;
  return FALSE;
}

void engine_input_class::get_mouse_move(sw32 *dx, sw32 *dy)
{
  POINT pt;
  GetCursorPos(&pt);
  ScreenToClient(hwnd,&pt);

  *dx = pt.x - last_x;
	*dy = pt.y - last_y;

  if (mouse_locked)
  {
    pt.x = last_x;
    pt.y = last_y;
    ClientToScreen(hwnd,&pt);
    SetCursorPos(pt.x,pt.y);
  }
  else
  {
    last_x = pt.x;
    last_y = pt.y;
  }
}
