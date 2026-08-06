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
#include <stdio.h>
#include <windows.h>
#include "config.h"
#include "text.h"

char SIN_BASE_PATH[1024];

void read_config(char *config_file, bool always_ask)
{
  FILE *f = fopen(config_file,"rt");

  bool need_path = always_ask;

  if (f)
  {
    char buffer[1024];
    char p_str[1024];
    fgets(buffer,1023,f);

    char *b = buffer;
    b += get_token(b,p_str);

    if (strcmp(p_str,"path")!=0)
      need_path = TRUE;
    else
    {
      b += get_token(b,p_str);
      strcpy(SIN_BASE_PATH,p_str);
    }

    fclose(f);
  }
  else
    need_path = TRUE;

  if (need_path)
  {
    //gotta make one
    AllocConsole();
    SetConsoleTitle("SinView - Config");
    HANDLE stdin_handle  = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    char ask_for_path[] = "Where is the 'base' path for Sin? (ex: C:\\SinDemo\\base)\n>";

    DWORD num_read;
    WriteFile(stdout_handle, ask_for_path, strlen(ask_for_path), &num_read, 0);

    char buffer[1024];
    ReadFile(stdin_handle, buffer, 1023, &num_read, 0);
    buffer[num_read] = 0; //terminate the string

    char *b = buffer;
    while (*b)
    {
      if (*b=='/')
        *b='\\';

      if (*b<32)
        *b=0;

      b++;
    }

    if (buffer[strlen(buffer)-1]=='\\')
      buffer[strlen(buffer)-1] = 0; //if they put an extra \, kill it

    f = fopen(config_file,"wt");
    fprintf(f,"path %s\n",buffer);
    fclose(f);
    FreeConsole();

    strcpy(SIN_BASE_PATH,buffer);
  }
}
