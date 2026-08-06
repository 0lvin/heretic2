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
#include "tree_view.h"
#include "file.h"
#include "text.h"
#include "def_read.h"
#include "error.h"

extern HWND main_hwnd;
extern HINSTANCE main_instance;

void setup_bitmap(char *asdf);
void setup_model(def_info_class *def, bool full);

tree_view_class::tree_view_class()
{
  dir=0;
  hwnd=0;
}

tree_view_class::~tree_view_class()
{
  if (dir)
  {
    delete dir;
    dir = 0;
  }
}

void add_tree_entries(directory_class *d, HTREEITEM parent, HWND tree_hwnd)
{
  while (d)
  {
    TV_ITEM tvI;
    tvI.mask           = TVIF_TEXT | TVIF_PARAM;// | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
    tvI.pszText        = d->entry_name;
    tvI.cchTextMax     = strlen(d->entry_name);
    tvI.iImage         = 0;//iImage;
    tvI.iSelectedImage = 0;//iImage;

    TV_INSERTSTRUCT tvIns;
    tvIns.item         = tvI;
    tvIns.hInsertAfter = 0; //i guess this just inserts it wherever
    tvIns.hParent      = parent;

    HTREEITEM h = TreeView_InsertItem(tree_hwnd, &tvIns);
    d->user_var = (w32)h;

    add_tree_entries(d->children, h, tree_hwnd);

    d = d->next;
  }
}

void expand_def_files(directory_class *d, char *path_name, HWND p_wnd)
{
  while (d)
  {
    if (d->children)
    {
      //add this to the path
      char old_path[1024];
      strcpy(old_path,path_name);

      strcat(path_name,d->entry_name);
      strcat(path_name,"/");
      expand_def_files(d->children, path_name, p_wnd);

      strcpy(path_name,old_path);
    }
    else
    {
      // Increment (step) the progress bar.
      SendMessage(p_wnd, PBM_STEPIT,0,0);

      char ext[16];
      if (strcmp(get_ext(d->entry_name,ext),"def")==0)
      {
        char full_name[1024];
        sprintf(full_name,"%s%s",path_name,d->entry_name);
        def_info_class *def = new def_info_class(full_name);

        //add the animation names to the children list
        sw32 i;
        for (i=0; i<def->num_animations; i++)
        {
          directory_class *d2 = new directory_class;
          strcpy(d2->entry_name, def->animation_names[i*2]);

          //point this back to the .def class for this animation
          d2->user_ptr = (void *)def;

          d2->next = d->children;
          d->children = d2;
        }
      }
    }

    d = d->next;
  }
}

directory_class *find_htreeitem(directory_class *d, HTREEITEM h, char *full_name)
{
  while (d)
  {
    if (d->user_var==(w32)h)
    {
      strcpy(full_name,d->entry_name);
      return d;
    }

    directory_class *d2 = find_htreeitem(d->children, h, full_name);

    if (d2)
    {
      char new_str[1024];
      sprintf(new_str,"%s/%s",d->entry_name,full_name);
      strcpy(full_name,new_str);
      return d2;
    }

    d = d->next;
  }

  return 0;
}

void tree_view_class::init()
{
  //I HATE WIN32

  //create a progress bar
  HWND progress_hwnd = CreateWindowEx(
       0,
       PROGRESS_CLASS,
       "Reading Directory..",
       WS_CHILD | WS_VISIBLE | WS_CAPTION | WS_THICKFRAME | WS_OVERLAPPED,
       0, 0, 200, 60,
       main_hwnd,
       0,
       main_instance,
       NULL);

  //only want files we recognize
  char *valid_ext[3] = {"def","tga","swl"};
  dir = get_directory_tree(3,valid_ext);   //this should go quick?

  if (!dir->children || !dir->num_below) //these vars should both be 0
  {
    //nothing there? at least make a "error" message
    dir->children = new directory_class;
    strcpy(dir->children->entry_name,"Failed to find pak0.pak");
  }

  // Set the range for the progress bar.
  SendMessage(progress_hwnd, PBM_SETRANGE, 0, MAKELONG(0,dir->num_below));

  // Set the step.
  SendMessage(progress_hwnd, PBM_SETSTEP, 1, 0);

  //expand the .def files (this is what takes time)
  char full_name[1024]; full_name[0]=0;
  expand_def_files(dir->children, full_name, progress_hwnd);

  //kill the progress window
  DestroyWindow(progress_hwnd);

  RECT rect;
  GetClientRect(main_hwnd,&rect);

  // Create the tree view window.
  hwnd = CreateWindowEx(
    0,
    WC_TREEVIEW, // tree view class
    dir->entry_name, //title
    WS_VISIBLE | WS_CLIPCHILDREN | WS_CHILD | WS_BORDER |
    TVS_HASBUTTONS |  TVS_HASLINES | TVS_LINESATROOT,
    0, 0, 200, rect.bottom,
    main_hwnd,
    (void *)0, //the idFrom paramater in notify messages
    main_instance,
    NULL);

  add_tree_entries(dir->children, TVI_ROOT, hwnd);

  ShowWindow(hwnd, SW_SHOWNORMAL);
  UpdateWindow(hwnd);
}

void tree_view_class::resize()
{
  RECT rect;
  GetClientRect(main_hwnd,&rect);

  SetWindowPos(hwnd,
               HWND_TOP,
               0, 0,
               200, rect.bottom,0);

  UpdateWindow(hwnd);
}

void delete_def_expansion(directory_class *d)
{
  bool found_it = FALSE;
  while (d)
  {
    if (d->children)
      delete_def_expansion(d->children);

    if (d->user_ptr)
    {
      if (!found_it)
      {
        //there is only 1 def_info for a set of animations
        //(they all point to it)
        //so we only have to delete it once.
        def_info_class *def = (def_info_class *)d->user_ptr;
        delete def; //delete these
        found_it = TRUE;
      }

      d->user_ptr = 0;
    }

    d = d->next;
  }
}

void tree_view_class::shutdown()
{
  if (hwnd)
  {
    DestroyWindow(hwnd);
    hwnd = 0;
  }

  //remove all the expanded .def files from the directory
  delete_def_expansion(dir);

  if (dir)
  {
    delete dir;
    dir = 0;
  }
}

void tree_view_class::update_view(HTREEITEM h)
{
  char full_name[1024];
  full_name[0]=0;

  directory_class *c = find_htreeitem(dir->children,h,full_name);

  if (c)
  {
    char ext[16];
    get_ext(c->entry_name,ext);

    if (strcmp(ext,"swl")==0 || strcmp(ext,"tga")==0)
    {
      setup_bitmap(full_name);
    }
    else
    if (c->user_ptr)
    {
      def_info_class *def = (def_info_class *)c->user_ptr;
      sw32 i;
      for (i=0; i<def->num_animations; i++)
      {
        if (strcmp(def->animation_names[i*2],c->entry_name)==0)
        {
          def->cur_animation = i;
          break;
        }
      }

      def->cur_skin = def->num_skins-1;
      setup_model(def,FALSE);
    }
  }
}
