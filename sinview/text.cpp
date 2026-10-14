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
#include "text.h"
#include "arch.h"
#include <stdlib.h>
#include <string.h>

int get_token(char *s, char *token)
{
  int chars_used=0;

  while (*s && is_whitespace(*s))
  {
    chars_used++;
    s++;
  }

  while (*s && !is_whitespace(*s))
  {
    *token = *s;
    chars_used++;
    token++;
    s++;
  }

  *token=0;
  return chars_used;
}

void unix_path(char *c)
{
  while (*c)
  {
    if (*c=='\\')
      *c = '/';

    c++;
  }
}

void dos_path(char *c)
{
  while (*c)
  {
    if (*c=='/')
      *c = '\\';

    c++;
  }
}

char *get_ext(char *s, char *dst)
{
  char temp_buffer[1024];
  char *d = temp_buffer;

  bool had_ext = FALSE;
  while (*s)
  {
    if (*s=='.')
    {
      had_ext = TRUE;
      d = temp_buffer;
      *d = 0;
    }
    else
    {
      *d = tolower(*s);
      d++;
    }

    s++;
  }

  if (had_ext)
  {
    *d=0;
    strcpy(dst,temp_buffer);
  }
  else
    *dst = 0;

  return dst;
}

bool is_comment(char *s)
{
  if (s[0]=='/' && s[1]=='/') return 1;
  return 0;
}

char *find_str(char *src, char *search)
{
  char *s = src;

  while (*s)
  {
    char *s1=search;
    char *s2=s;

    bool found=TRUE;
    while (*s1 && *s2)
    {
      if (*s1 != *s2)
      {
        found=FALSE;
        break;
      }

      s1++;
      s2++;
    }

    if (found && s1[0]==0)
      return s;

    s++;
  }

  return 0;
}