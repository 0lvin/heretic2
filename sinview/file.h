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
#ifndef FILE_H
#define FILE_H

#include "arch.h"
#include <stdio.h>
#include <stdarg.h>

//abstract file class
//2 implementations: (see file.cpp)
//  1 from regular files (std_file_class)
//  1 from within .pak files (pak_file_class)

class file_class;
class directory_class;

#define FILE_READ  (1)
#define FILE_WRITE (2)

//these functions are the interface to the file system
file_class *open_file(char *name, w8 open_flags);
void close_file(file_class *f);
void register_pak_file(char *pak_name);
void pak_shutdown();

//supply which extensions you want included in the list
//(supplying nothing returns the entire tree)
directory_class *get_directory_tree(int num_ext=0, char **valid_ext=0);

class file_class
{
public:
  FILE *f;
  void put_c(char c) { fputc(c,f); }
  void put_w(w16 w)  { fwrite(&w,2,1,f); }
  w8   get_c() { return fgetc(f); }
  w16  get_w()  { w16 w; fread(&w,2,1,f); return w; }
  w32  write(void *dst, w32 size) { return fwrite(dst, size, 1, f); }
  w32  read(void *dst, w32 size)  { return fread(dst, size, 1, f); }
  sw32 gets(char *dst, w32 dst_size) { return (sw32)fgets(dst,dst_size,f); }
  void printf(char *fmt, ...)
  {
    va_list argptr;
    char message[4096];
	  va_start (argptr,fmt);
	  vsprintf (message,fmt,argptr);
	  va_end (argptr);
    fprintf(f,"%s",message);
  }

  virtual w32  size() = 0;
  virtual w32  tell() = 0;
  virtual void seek(w32 offset) = 0;
  virtual w8   eof() = 0; //if its at the end of file (or past it)
};

class directory_class
{
public:
  directory_class()
  {
    entry_name[0] = 0;
    user_var = 0;
    user_ptr = 0;
    next = 0;
    children = 0;
    num_below = 0;
  }

  //delete the root and it should take care of the rest
  ~directory_class()
  {
    if (children)
    {
      delete children;
      children=0;
    }

    if (next)
    {
      delete next;
      next=0;
    }
  }

  char entry_name[128];
  w32  num_below; //# of entries in ->children (and their children, etc)
  w32  user_var;  //sinview uses this to store HTREEVIEW handles
  void *user_ptr; //uses this to store pointers to whatever
  directory_class *next;
  directory_class *children;
};

#endif