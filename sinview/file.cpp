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
#include <string.h>
#include <windows.h>
#include <commctrl.h>
#include "file.h"
#include "config.h"
#include "text.h"
#include "error.h"

//structures for pak reading
struct pakheader_t
{
  w8  magic[4];
  w32 diroffset;
  w32 dirsize;
};

struct pakentry_t
{
  char filename[0x38];
  w32  offset;
  w32  size;
};

class reg_pak_file_class
{
public:
  reg_pak_file_class()
  {
    full_path_pak_name[0]=0;
    pak_entries=0;
    num_pak_entries=0;
    next=0;
  }

  ~reg_pak_file_class()
  {
    if (pak_entries)
    {
      delete [num_pak_entries] pak_entries;
      pak_entries = 0;
    }
    num_pak_entries = 0;
  }

  char        full_path_pak_name[1024];
  pakentry_t *pak_entries;
  sw32        num_pak_entries;

  reg_pak_file_class *next;
};

reg_pak_file_class *registered_pak_files=0;

void register_pak_file(char *pak_name)
{
  if (!pak_name || pak_name[0]==0)
    return;

  char full_pak_name[1024];
  sprintf(full_pak_name,"%s\\%s",SIN_BASE_PATH,pak_name);
  dos_path(full_pak_name);

  FILE *f = fopen(full_pak_name,"rb");
  if (!f)
    return;

  reg_pak_file_class *rpf = new reg_pak_file_class;

  //initialize the pak class
  strcpy(rpf->full_path_pak_name,full_pak_name);
  rpf->next = registered_pak_files;
  registered_pak_files = rpf;

  //read the directory
  pakheader_t pakheader;
  fread(&pakheader,sizeof(pakheader_t),1,f);

  fseek(f,pakheader.diroffset,SEEK_SET);
  rpf->num_pak_entries = pakheader.dirsize / (sizeof(pakentry_t));

  rpf->pak_entries = new pakentry_t[rpf->num_pak_entries];
  fread(rpf->pak_entries,pakheader.dirsize,1,f);

  //make sure all the files are unix paths
  sw32 i;
  for (i=0; i<rpf->num_pak_entries; i++)
    unix_path(rpf->pak_entries[i].filename);

  fclose(f);
}

void pak_shutdown()
{
  reg_pak_file_class *rpf = registered_pak_files;
  while (rpf)
  {
    reg_pak_file_class *temp = rpf;
    rpf = rpf->next;
    delete temp;
  }
  registered_pak_files=0;
}

//this reads regular old files
class std_file_class : public file_class
{
public:
  std_file_class() { f=0; size_var=-1; }
  ~std_file_class() { if (f) { fclose(f); f=0; } }

  sw32 size_var;

  w32  tell() { return ftell(f); }
  w32  size()
  {
    if (size_var==-1)
    {
      w32 cur_ofs = ftell(f);
      fseek(f,0,SEEK_END);
      size_var = ftell(f);
    }
    return size_var;
  }
  void seek(w32 offset) { fseek(f,offset,SEEK_SET); }
  w8   eof() { if (feof(f)) return TRUE; return FALSE; }
};

//this reads files in a "library" type situation (.pak files)
//(ie, multiple files stored inside of one larger file)
class pak_file_class : public file_class
{
public:
  pak_file_class() { f=0; }
  ~pak_file_class() { if (f) { fclose(f); f=0; } }

  w32 offset;
  w32 size_var;

  w32  tell() { return ftell(f) - offset; }
  w32  size() { return size_var; }
  void seek(w32 ofs) { fseek(f, offset + ofs, SEEK_SET); }
  w8   eof() { if (ftell(f)>=offset+size_var) return TRUE; return FALSE; }
};

file_class *open_file(char *name, w8 open_flags)
{
  if (!name || name[0]==0) //trivially bad filenames
    return 0;

  char filename[1024];

  if (open_flags & FILE_WRITE)
  {
    //writing files always goes right to disk (no .pak files involved)
    sprintf(filename,"%s\\%s",SIN_BASE_PATH,name);
    dos_path(filename);

    FILE *f = fopen(filename,"wb");
    if (!f)
      return 0;

    std_file_class *std_f = new std_file_class;
    std_f->f = f;
    return std_f;
  }
  else
  if (open_flags & FILE_READ)
  {
    //first look in regular directory structure
    sprintf(filename,"%s\\%s",SIN_BASE_PATH,name);
    dos_path(filename);

    FILE *f = fopen(filename,"rb");
    if (f)
    {
      std_file_class *std_f = new std_file_class;
      std_f->f = f;
      return std_f;
    }
    else
    {
      //get rid of the full path, just go with relative unix path so that .pak
      //search works
      strcpy(filename,name);
      unix_path(filename);

      reg_pak_file_class *rpf = registered_pak_files;
      while (rpf)
      {
        sw32 i;
        bool found=FALSE;
        for (i=0; i<rpf->num_pak_entries; i++)
        {
          if (!stricmp(rpf->pak_entries[i].filename,filename))
          {
            //found it.
            found = TRUE;
            break;
          }
        }

        if (found)
        {
          FILE *f = fopen(rpf->full_path_pak_name,"rb");
          if (!f)
            engine_error("file_open()","Unexpected error");

          fseek(f,rpf->pak_entries[i].offset,SEEK_SET);
          pak_file_class *pak_f = new pak_file_class;

          pak_f->f        = f;
          pak_f->size_var = rpf->pak_entries[i].size;
          pak_f->offset   = rpf->pak_entries[i].offset;

          return pak_f;
        }

        rpf = rpf->next;
      }
    }
  }

  return 0;
}

void close_file(file_class *f)
{
  delete f;
}

sw32 calc_num_below(directory_class *c)
{
  int num_below = 0;

  while (c)
  {
    if (c->children)
    {
      c->num_below = calc_num_below(c->children);
      num_below += c->num_below;
    }
    else
      num_below++;

    c = c->next;
  }

  return num_below;
}

directory_class *get_directory_tree(int num_ext, char **valid_ext)
{
  reg_pak_file_class *rpf = registered_pak_files;

  directory_class *root = new directory_class;
  strcpy(root->entry_name,SIN_BASE_PATH);
  unix_path(root->entry_name);

  rpf = registered_pak_files;
  while (rpf)
  {
    sw32 i,j;
    for (i=0; i<rpf->num_pak_entries; i++)
    {
      char ext[16];
      get_ext(rpf->pak_entries[i].filename,ext);

      bool valid = FALSE;
      for (j=0; j<num_ext; j++)
      {
        if (strcmp(valid_ext[j], ext)==0)
        {
          valid = TRUE;
          break;
        }
      }

      if (!valid && num_ext!=0)
        continue;

      char *c = rpf->pak_entries[i].filename;

      char entry_name[128];
      char *e = entry_name;
      *e = 0;

      directory_class *cur_parent = root;

      while (*c)
      {
        if (*c=='/' && entry_name[0]!=0)
        {
          directory_class *d = cur_parent->children;
          *e = 0;

          bool found = FALSE;
          while (d)
          {
            if (strcmp(d->entry_name,entry_name)==0)
            {
              found = TRUE;
              break;
            }
            d = d->next;
          }

          if (!found)
          {
            d = new directory_class;

            d->next = cur_parent->children;
            cur_parent->children = d;

            strcpy(d->entry_name, entry_name);
          }

          cur_parent = d;

          e = entry_name;
        }
        else
        {
          *e = *c;
          e++;
        }

        c++;
      }

      *e = 0;

      if (entry_name[0])
      {
        directory_class *d = new directory_class;

        d->next = cur_parent->children;
        cur_parent->children = d;

        strcpy(d->entry_name, entry_name);
      }
    }

    rpf = rpf->next;
  }

  calc_num_below(root);
  return root;
}
