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
#include <stdio.h>
#include <memory.h>
#include <glide.h>
#include "render_window.h"
#include "error.h"
#include "text.h"
#include "gfx_read.h"

//this stuff is hardcoded and wont handle >1 texture
class engine_texture_class
{
public:
  GrTexInfo info;
  float s_scale,t_scale;
  sw32  width,height;
  ~engine_texture_class()
  {
  }

  engine_texture_class(char *fname)
  {
    //this doesnt do any sort of pcx-verification. it had better be a standard
    //pcx file with an 8-bit palette. otherwise you'll probably get a bunch
    //of warped garbage, assuming the program doesnt just crash.

    int res = -1;
    w8 *oldmap;
    w8 palette[256*4];

    char ext[16]; //only read .tga files for the skins
    if (strcmp(get_ext(fname,ext),"tga")==0)
    {
      res = read_tga(fname, &oldmap, palette, &width, &height);
    }

    //couldnt read the skin for some reason
    if (res!=0)
    {
      if (res!=-1)
      {
        //it read a 32-bit thing. delete it
        delete [width*height*4] oldmap;
      }

      //make a replacement texture
      width  = 16;
      height = 16;
      memset(palette,0xFF,256*4); //white palette
      oldmap = new w8[16*16]; //16x16 white texture
      memset(oldmap,0,256);
    }

	  //3dfx palette table setup
	  sw32 i,j;
    GuTexPalette fx_pal;
    for (i=0; i<256; i++)
    {
  		fx_pal.data[i] = 0;
		  fx_pal.data[i] |= (w32)(palette[i*4+0]) << 16;
		  fx_pal.data[i] |= (w32)(palette[i*4+1]) << 8;
		  fx_pal.data[i] |= (w32)(palette[i*4+2]);
	  }

	  grTexDownloadTable(GR_TMU0,GR_TEXTABLE_PALETTE,&fx_pal);

	  //Pick the closest compatible texture size
    sw32 tw,th;
    for (tw=1; tw<width; tw*=2);
    if (tw>256) tw=256;
    for (th=1; th<height; th*=2);
    if (th>256) th=256;

	  //Figure out which aspect ratio it is
    GrAspectRatio_t asp;
	  GrLOD_t lod;

    if (tw>th)
    {
	    switch (tw)
      {
      case 2:   lod = GR_LOD_2;   break;
      case 4:   lod = GR_LOD_4;   break;
      case 8:   lod = GR_LOD_8;   break;
      case 16:  lod = GR_LOD_16;  break;
      case 32:  lod = GR_LOD_32;  break;
      case 64:  lod = GR_LOD_64;  break;
      case 128: lod = GR_LOD_128; break;
      case 256: lod = GR_LOD_256; break;
      }

      switch(tw/th)
      {
  		case 1: asp = GR_ASPECT_1x1; break;
		  case 2: asp = GR_ASPECT_2x1; break;
    	case 4: asp = GR_ASPECT_4x1; break;
    	case 8:	asp = GR_ASPECT_8x1; break;
		  default: asp = -1;
		  }
	  }
    else
    {
		  switch (th)
      {
      case 2:   lod = GR_LOD_2;   break;
      case 4:   lod = GR_LOD_4;   break;
      case 8:   lod = GR_LOD_8;   break;
      case 16:  lod = GR_LOD_16;  break;
      case 32:  lod = GR_LOD_32;  break;
      case 64:  lod = GR_LOD_64;  break;
      case 128: lod = GR_LOD_128; break;
      case 256: lod = GR_LOD_256; break;
      }

      switch(th/tw)
      {
		    case 1: asp = GR_ASPECT_1x1; break;
    		case 2: asp = GR_ASPECT_1x2; break;
		    case 4: asp = GR_ASPECT_1x4; break;
    		case 8:	asp = GR_ASPECT_1x8; break;
		    default: asp = -1;
      }
	  }

    if (asp==-1)
		  engine_error("engine_texture_class()","Unable to determine aspect ratio");

	  //only 1 Lod ("level of detail")
	  info.smallLod = lod;
	  info.largeLod = lod;
	  info.aspectRatio = asp;
	  info.format = GR_TEXFMT_P_8;

    //make sure we havent fucked anything
    sw32 textureSize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &info);
	  if (textureSize != tw*th)
      engine_error("engine_texture_class()","Miscalculated required memory");

	  //do we need to scale?
    w8 *texdata = oldmap;

    if (tw!=width || th!=height)
    {
      //allocate memory for the scaled texture
	    w8 *texdata = new w8[tw*th];

	    //scale the old texture
      for (i=0;i<th;i++)
      {
  	    for(j=0;j<tw;j++)
        {
		      texdata[i*tw + j] =
		      //scaling formula. pretty simple. also pretty slow. =)
          oldmap[
          (int)((float)j*(float)width/(float)tw) +
          (int)((float)i*(float)height/(float)th)*width
          ];
        }
	    }

      delete oldmap;
      oldmap = 0;
    }

    //point the info.data to the new scaled texture
	  info.data = texdata;

	  w32 startAddress = grTexMinAddress(GR_TMU0);

	  //download it into memory
	  grTexDownloadMipMap(GR_TMU0,startAddress,GR_MIPMAPLEVELMASK_BOTH,&info);

	  //store the scaled width and height into the model structure
    if (tw>th)
    {
      s_scale = 256.f;
	    t_scale = 256.f * (float)th/(float)tw;
    }
    else
    {
      t_scale = 256.f;
	    s_scale = 256.f * (float)tw/(float)th;
    }

    delete [tw*th] texdata;
  }
};


engine_texture_class *engine_render_window_class::register_texture(char *texname)
{
  return new engine_texture_class(texname);
}

void engine_render_window_class::set_texture(engine_texture_class *t)
{
  grTexSource(GR_TMU0,grTexMinAddress(GR_TMU0),GR_MIPMAPLEVELMASK_BOTH,&t->info);
  s_scale = t->s_scale;
  t_scale = t->t_scale;
}

void engine_render_window_class::free_texture(engine_texture_class *t)
{
  delete t;
}
