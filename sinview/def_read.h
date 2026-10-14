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
#ifndef DEF_READ_H
#define DEF_READ_H

#include "arch.h"

class def_info_class
{
public:
  def_info_class(char *name);

  sw32 num_animations;
  sw32 cur_animation;
  char **animation_names;

  sw32 num_skins;
  sw32 cur_skin;
  char **skin_names;

  float scale;

  char path[64];
  char base_name[64];
};

#endif