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
#include "texture_view.h"
#include "file.h"
#include "text.h"
#include "gfx_read.h"
extern HWND main_hwnd;

texture_view_class::texture_view_class(char *tex_name)
{
  width=height=0;
  bitmap_info=0;
  bitmap=0;

  BITMAPINFOHEADER *bi;
  //setup the device independent bitmap
	//(windows bullshit, basically)
	bitmap_info = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD));
	memset(bitmap_info,0,(sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)));
  bi = (BITMAPINFOHEADER *)&bitmap_info->bmiHeader;
	bi->biSize = sizeof(BITMAPINFOHEADER);

  //negative height in a windows bitmap makes it display
	//like normal vga ((0,0) in the upper left corner, (width,height) in
	//the lower right
	bi->biPlanes = 1;
	bi->biCompression = BI_RGB;
	bi->biClrUsed = 256;

  RGBQUAD *bitmap_pal = bitmap_info->bmiColors;

  w8 palette[256*4];
  sw32 i;
  char ext[16];
  if (strcmp(get_ext(tex_name,ext),"swl")==0)
  {
    //load the swl
    int ret = read_swl(tex_name, &bitmap, palette, &width, &height);
    if (ret==-1)
      return;

    bi->biBitCount = 8;
    for (i=0; i<256; i++)
    {
      bitmap_pal[i].rgbRed   = palette[i*4 + 0];
      bitmap_pal[i].rgbGreen = palette[i*4 + 1];
      bitmap_pal[i].rgbBlue  = palette[i*4 + 2];
    }
  }
  else
  if (strcmp(get_ext(tex_name,ext),"tga")==0)
  {
    //load the tga
    int ret = read_tga(tex_name, &bitmap, palette, &width, &height);
    if (ret==-1)
      return;

    if (ret==0)
    {
      bi->biBitCount = 8;
      for (i=0; i<256; i++)
      {
        bitmap_pal[i].rgbRed   = palette[i*4 + 0];
        bitmap_pal[i].rgbGreen = palette[i*4 + 1];
        bitmap_pal[i].rgbBlue  = palette[i*4 + 2];
      }
    }
    else
    {
      //convert from rgba order to bgra order - another stupid win32 thing
      //(swap red and blue channels)
      for (i=0; i<width*height; i++)
      {
        w8 temp = bitmap[i*4 + 0];
        bitmap[i*4 + 0] = bitmap[i*4 + 2];
        bitmap[i*4 + 2] = temp;
      }
      bi->biBitCount = 32;
    }
  }

  bitmap_info->bmiHeader.biHeight = -height;
  bitmap_info->bmiHeader.biWidth  = width;
}

texture_view_class::~texture_view_class()
{
  if (bitmap)
  {
    delete [width*height] bitmap;
    bitmap = 0;
  }

  if (bitmap_info)
  {
    free(bitmap_info);
    bitmap_info = 0;
  }
}

void texture_view_class::draw()
{
  if (!bitmap)
    return;

  HDC  hdc;

	RECT rect;
  GetClientRect(main_hwnd, &rect);
  rect.left = 200;

  sw32 mid_x = (rect.left + rect.right)/2;
  sw32 mid_y = (rect.top  + rect.bottom)/2;

  hdc = GetDC(main_hwnd);

  //my favorite windows blit
  SetDIBitsToDevice(hdc,
                    mid_x - width/2, mid_y - height/2,
                    mid_x + width/2, mid_y + height/2,
                    0,0,0,height,bitmap,bitmap_info,DIB_RGB_COLORS);

	ReleaseDC(main_hwnd,hdc);
}