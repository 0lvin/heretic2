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
#include "file.h"
#include "error.h"
#include <memory.h>

#define SIN_PALETTE_SIZE 256*4
#define MIPLEVELS 4

struct sinmiptex_t
{
	char  name[64];
	w32   width, height;
  w8    palette[SIN_PALETTE_SIZE];
  w16   palcrc;
  w32   offsets[MIPLEVELS];		// four mip maps stored
	char  animname[64];			// next frame in animation chain
	w32   flags;
	w32   contents;
	w16   value;
  w16   direct;
  float animtime;
  float nonlit;
  w16   directangle;
  w16   trans_angle;
  float directstyle;
  float translucence;
  float friction;
  float restitution;
  float trans_mag;
  float color[3];
};

//WARNING
//auto-align fucks with this
//you must read each member in
//one at a time - call the read function
struct tga_header_t
{
  w8  id_length;
  w8  colormap_type;
  w8  image_type;
  w16 colormap_index;
  w16 colormap_length;
  w8  colormap_size;
  w16 x_origin;
  w16 y_origin;
  w16 width;
  w16 height;
  w8  pixel_size;
  w8  attributes;

  void read(file_class *f)
  {
    id_length = f->get_c();
    colormap_type = f->get_c();
    image_type = f->get_c();
    colormap_index = f->get_w();
    colormap_length = f->get_w();
    colormap_size = f->get_c();
    x_origin = f->get_w();
    y_origin = f->get_w();
    width = f->get_w();
    height = f->get_w();
    pixel_size = f->get_c();
    attributes = f->get_c();
  }
};

//return -1 if error
//return 0 if paletted
//return 1 if 32bit argb
int read_tga(char *name, w8 **dst_bitmap, w8 *palette, sw32 *dst_width, sw32 *dst_height)
{
  //thanks to markd at Ritual for sharing this tga code
  tga_header_t tga_header;

  file_class *f = open_file(name, FILE_READ);

  if (!f)
    return -1;

  tga_header.read(f);

  switch (tga_header.image_type)
  {
  case 1:
  case 2:
  case 10:
    break;
  default:
    engine_warning("Unrecognized .tga type","tga_read()");
    close_file(f);
    return -1;
  }

  switch (tga_header.colormap_type)
  {
  case 0:
  case 1:
    break;
  default:
    engine_warning("Unrecognized colormap type","tga_read()");
    close_file(f);
    return -1;
  }

  switch (tga_header.pixel_size)
  {
  case 8:
  case 24:
  case 32:
    break;
  default:
    engine_warning("Unrecognized pixel size","tga_read()");
    close_file(f);
    return -1;
  }

  sw32 columns   = tga_header.width;
  sw32 rows      = tga_header.height;
  sw32 numPixels = columns * rows;

  w8 *bitmap=0;

  if (tga_header.image_type==1)
    bitmap = new w8[numPixels];
  else
    bitmap = new w8[numPixels*4]; //32 / 24 bit

  if (tga_header.id_length != 0) // skip TARGA image comment
    f->seek(f->tell() + tga_header.id_length);

  //fill out this stuff
  *dst_bitmap = bitmap;
  *dst_width  = tga_header.width;
  *dst_height = tga_header.height;

  sw32 i;

  for (i=0; i<tga_header.colormap_length; i++)
  {
    w8 b = f->get_c();
    w8 g = f->get_c();
    w8 r = f->get_c();
    w8 a = 0xFF;

    if (tga_header.colormap_size == 4)
      a = f->get_c();

    palette[i*4+0] = r;
    palette[i*4+1] = g;
    palette[i*4+2] = b;
    palette[i*4+3] = a;
  }

  w8 *pixbuf;
  w8 r,g,b,a;
  w8 packetHeader,packetSize,j;
  sw32 row,column;

  if (tga_header.image_type==1) //paletted
  {
    for (row=rows-1; row>=0; row--)
    {
      pixbuf = bitmap + row*columns;
      for (column=0; column<columns; column++)
      {
        *pixbuf++ = f->get_c();
      }
    }
  }
  else
  if (tga_header.image_type==2)// Uncompressed, RGB images
  {
    for (row=rows-1; row>=0; row--)
    {
      pixbuf = bitmap + row*columns*4;
      for (column=0; column<columns; column++)
      {
        switch (tga_header.pixel_size)
        {
        case 24:
          b = f->get_c();
          g = f->get_c();
          r = f->get_c();
          *pixbuf++ = r;
          *pixbuf++ = g;
          *pixbuf++ = b;
          *pixbuf++ = 255;
          break;
        case 32:
          b = f->get_c();
          g = f->get_c();
          r = f->get_c();
          a = f->get_c();
          *pixbuf++ = r;
          *pixbuf++ = g;
          *pixbuf++ = b;
          *pixbuf++ = a;
        }
      }
    }
  }
  else
  if (tga_header.image_type==10)// Runlength encoded RGB images
  {
    for (row=rows-1; row>=0; row--)
    {
      pixbuf = bitmap + row*columns*4;
      for (column=0; column<columns;)
      {
        packetHeader = f->get_c();
        packetSize = 1 + (packetHeader & 0x7f);
        if (packetHeader & 0x80) // run-length packet
        {
          switch (tga_header.pixel_size)
          {
          case 24:
            b = f->get_c();
            g = f->get_c();
            r = f->get_c();
            a = 255;
            break;
          case 32:
            b = f->get_c();
            g = f->get_c();
            r = f->get_c();
            a = f->get_c();
          }

          for (j=0; j<packetSize; j++)
          {
            *pixbuf++ = r;
            *pixbuf++ = g;
            *pixbuf++ = b;
            *pixbuf++ = a;
            column++;
            if (column==columns) // run spans across rows
            {
              column=0;
              if (row>0)
                row--;
              else
                goto breakOut;
              pixbuf = bitmap + row*columns*4;
            }
          }
        }
        else // non run-length packet
        {
          for(j=0;j<packetSize;j++)
          {
            switch (tga_header.pixel_size)
            {
            case 24:
              b = f->get_c();
              g = f->get_c();
              r = f->get_c();
              *pixbuf++ = r;
              *pixbuf++ = g;
              *pixbuf++ = b;
              *pixbuf++ = 255;
              break;
            case 32:
              b = f->get_c();
              g = f->get_c();
              r = f->get_c();
              a = f->get_c();
              *pixbuf++ = r;
              *pixbuf++ = g;
              *pixbuf++ = b;
              *pixbuf++ = a;
            }
            column++;
            if (column==columns) // pixel packet run spans across rows
            {
              column=0;
              if (row>0)
                row--;
              else
                goto breakOut;
              pixbuf = bitmap + row*columns*4;
            }
          }
        }
      }
      breakOut:;
    }
  }

  close_file(f);

  switch (tga_header.pixel_size)
  {
  case 8:
    return 0;
  case 24:
  case 32:
    return 1;
  }

  return -1;//shouldnt get here
}

//return -1 if error
//return 0 if successful
int read_swl(char *name, w8 **dst_bitmap, w8 *palette, sw32 *dst_width, sw32 *dst_height)
{
  file_class *f = open_file(name, FILE_READ);

  if (!f)
    return -1;

  sinmiptex_t mip_header;
  f->read(&mip_header, sizeof(sinmiptex_t));

  sw32 width = mip_header.width;
  sw32 height = mip_header.height;

  w8 *bitmap = new w8[width * height];

  f->seek(mip_header.offsets[0]);
  f->read(bitmap, width * height);

  close_file(f);

  //fill these out
  *dst_bitmap = bitmap;
  *dst_width      = width;
  *dst_height     = height;
  memcpy(palette, mip_header.palette, 256*4);

  return 0;
}