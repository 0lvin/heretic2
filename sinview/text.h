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
#ifndef TEXT_H
#define TEXT_H

#include "arch.h"

#define is_whitespace(c) (c<=' ') //dunno how well this will work

int get_token(char *s, char *token);
void unix_path(char *s);
void dos_path(char *s);
char *get_ext(char *s, char *dst);
bool is_comment(char *s);
char *find_str(char *src, char *search);

#endif