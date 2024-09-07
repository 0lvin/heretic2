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
#include <glide.h>
#include "render_window.h"

int file_num=0;

void fputw(short w, FILE *f)
{
  //no damn fputw method in standard c
  fwrite(&w,2,1,f);
}

void engine_render_window_class::take_screen_shot()
{
  FILE *f;
  char filename[256];

  sprintf(filename,"%s%d.tga","shot",file_num);
  file_num++; //increment the file number

  f = fopen(filename,"wb");

  //TGA header bullshit.
  //amounts to, basically, no compression, no color table, etc
  fputc(0,f);
  fputc(0,f);
  fputc(2,f);
  fputw(0,f);
  fputw(0,f);
  fputc(0,f);

  fputw(0,f);
  fputw(0,f);

  //width
  fputw((w16)width,f);
  //height
  fputw((w16)height,f);

  //24 bits per pixel
  fputc(24,f);

  //some extra flag that should be 0
  fputc(0,f);

  w16 *frame_buffer = new w16[width*height];

  //read the frame buffer
  grab_framebuffer(frame_buffer);

  w8 *frame_buffer_8 = (w8 *)frame_buffer;

  sw32 i,j;
  for (i=0; i<height; i++)
  for (j=0; j<width; j++)
  {
    //have to write it from the bottom up, apparently
    //(flipped, vertically)

    w16 color_565 = frame_buffer[(479-i)*640 + j];

    //unpack the 565 color
    w8 r = ((color_565 >> 11) & 31) << 3;
    w8 g = ((color_565 >>  5) & 63) << 2;
    w8 b = ((color_565 >>  0) & 31) << 3;

    //write it to the tga in bgr order
    fputc(b,f);
    fputc(g,f);
    fputc(r,f);
  }

  fclose(f);
}

