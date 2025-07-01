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
#include "file.h"
#include "def_read.h"
#include <string.h>

class char_link_class
{
public:
  char_link_class()
  {
    name1=0;
    name2=0;
    next=0;
  }

  char *name1;
  char *name2;

  char_link_class *next;
};

def_info_class::def_info_class(char *file_name)
{
  num_animations = 0;
  animation_names = 0;
  num_skins = 0;
  skin_names = 0;

  file_class *f = open_file(file_name, FILE_READ);
  if (!f)
    return;

  char line[1024];

  //build these lists as we're reading the file
  char_link_class *a_names=0;
  char_link_class *s_names=0;

  bool in_comment = FALSE;
  while(!f->eof() && f->gets(line,1023) > 0)
  {
    //handle the star comments
    char *c;

    if (in_comment)
      c = line;
    else
      c = find_str(line,"/*");

    bool found_end=TRUE;
    while (c)
    {
      //clear out until you get to */
      found_end = FALSE;
      while (*c)
      {
        if ((c[0]=='*') && (c[1]=='/'))
        {
          c[0]=' '; c[1]=' ';
          found_end = TRUE;
          break;
        }

        *c = ' ';
        c++;
      }

      c = find_str(line,"/*");//any more?
    }

    if (!found_end)
    {
      in_comment = TRUE; //still in a comment. ignore stuff.
      continue;
    }
    else
      in_comment = FALSE;

    //check for basic things like a commented line or a blank line
    char token[1024];
    get_token(line,token);

    if (token[0]==0 || is_comment(token))
      continue; //line was commented out (or its blank)

    //something is there. check it out.
    c = line;
    c += get_token(line,token);

    char ext[16];
    if (strcmp(token,"path")==0)
    {
      c += get_token(c,token);
      strcpy(path,token);
    }
    else
    if (strcmp(token,"scale")==0)
    {
      c += get_token(c,token);
      sscanf(token,"%f",&scale);
    }
    else
    if (strcmp(token,"id")==0 || strcmp(token,"aliasprefix")==0 ||
        strcmp(token,"group")==0 || strcmp(token,"server")==0 ||
        strcmp(token,"!main:")==0 || strcmp(token,"!init:")==0 ||
        strcmp(token,"SDEF")==0 || strcmp(token,"client")==0)
    {
      //do nothing on these reserved words
    }
    else
    if (strcmp(get_ext(token,ext),"sbm")==0)
    {
      strcpy(base_name,token);
    }
    else
    if (strcmp(get_ext(token,ext),"tga")==0)
    {
      //add the skin to the list
      char_link_class *new_s = new char_link_class;

      new_s->name1 = new char[strlen(token)+1];
      strcpy(new_s->name1, token);

      new_s->next = s_names;
      s_names = new_s;

      num_skins++;
    }
    else
    if (strcmp(get_ext(token,ext),"sam")==0)
    {
      //hmm.. an unlabeled animation name. just add it to the list
      char_link_class *new_a = new char_link_class;

      new_a->name1 = new char[strlen(token)+1];
      new_a->name2 = new char[strlen(token)+1];
      strcpy(new_a->name1, token);
      strcpy(new_a->name2, token);

      new_a->next = a_names;
      a_names = new_a;

      num_animations++;
    }
    else
    {
      //get the animation name
      char anim_name[128];
      strcpy(anim_name,token);

      c += get_token(c,token);
      if (strcmp(get_ext(token,ext),"sam")==0)
      {
        //just add it to the animation list
        char_link_class *new_a = new char_link_class;

        new_a->name1 = new char[strlen(anim_name)+1];
        new_a->name2 = new char[strlen(token)+1];
        strcpy(new_a->name1, anim_name);
        strcpy(new_a->name2, token);

        new_a->next = a_names;
        a_names = new_a;

        num_animations++;
      }
    }
  }

  //close the file
  close_file(f);

  //convert the linked lists to an arrays
  animation_names = new char *[num_animations*2];
  char_link_class *c = a_names;
  sw32 i=0;
  while (c)
  {
    int len1 = strlen(c->name1)+1;
    int len2 = strlen(c->name2)+1;

    animation_names[i*2 + 0] = new char[len1];
    animation_names[i*2 + 1] = new char[len2];

    strcpy(animation_names[i*2+0],c->name1);
    strcpy(animation_names[i*2+1],c->name2);
    i++;

    char_link_class *temp = c;
    c = c->next;
    delete temp;
  }

  skin_names = new char *[num_skins];
  c = s_names;
  i=0;
  while (c)
  {
    skin_names[i] = new char[strlen(c->name1)+1];
    strcpy(skin_names[i],c->name1);

    i++;

    char_link_class *temp = c;
    c = c->next;
    delete temp;
  }

  cur_animation = 0;
  cur_skin = 0;
}
