/*  QMAP: Quake level viewer
 *
 *   readbsp.c
 *
 *  This file is a compilation of several
 *  files released by John Carmack as part
 *  of the Quake utilities source release.
 *  No redistribution limitations appeared
 *  in that distribution.
 *
 *  I don't claim any copyright either.
 */

#include "bspfile.h"
#include "mode.h"

// cmdlib.c

#include <sys/types.h>

/*
=================
Error

For abnormal program terminations
=================
*/
void Error (char *error, ...)
{
   va_list argptr;

   close_sdl();
   printf ("************ ERROR ************\n");

   va_start (argptr,error);
   vprintf (error,argptr);
   va_end (argptr);
   printf ("\n");
   exit (1);
}

/*
================
filelength
================
*/
int filelength (FILE *f)
{
   int      pos;
   int      end;

   pos = ftell (f);
   fseek (f, 0, SEEK_END);
   end = ftell (f);
   fseek (f, pos, SEEK_SET);

   return end;
}


FILE *SafeOpenRead (char *filename)
{
   FILE   *f;

   f = fopen(filename, "rb");

   if (!f)
      Error ("Error opening %s: %s",filename,strerror(errno));

   return f;
}


void SafeRead (FILE *f, void *buffer, int count)
{
   if (fread (buffer, 1, count, f) != (size_t)count)
      Error ("File read failure");
}



/*
==============
LoadFile
==============
*/
int    LoadFile (char *filename, void **bufferptr)
{
   FILE   *f;
   int    length;
   void    *buffer;

   f = SafeOpenRead (filename);
   length = filelength (f);
   buffer = malloc (length+1);
   ((char *)buffer)[length] = 0;
   SafeRead (f, buffer, length);
   fclose (f);

   *bufferptr = buffer;
   return length;
}






/*
============================================================================

               BYTE ORDER FUNCTIONS

============================================================================
*/

#ifdef _SGI_SOURCE
#define   __BIG_ENDIAN__
#endif

#ifdef __BIG_ENDIAN__

short   LittleShort (short l)
{
   byte    b1,b2;

   b1 = l&255;
   b2 = (l>>8)&255;

   return (b1<<8) + b2;
}

short   BigShort (short l)
{
   return l;
}


int    LittleLong (int l)
{
   byte    b1,b2,b3,b4;

   b1 = l&255;
   b2 = (l>>8)&255;
   b3 = (l>>16)&255;
   b4 = (l>>24)&255;

   return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

int    BigLong (int l)
{
   return l;
}


float   LittleFloat (float l)
{
   union {byte b[4]; float f;} in, out;

   in.f = l;
   out.b[0] = in.b[3];
   out.b[1] = in.b[2];
   out.b[2] = in.b[1];
   out.b[3] = in.b[0];

   return out.f;
}

float   BigFloat (float l)
{
   return l;
}


#else


short   BigShort (short l)
{
   byte    b1,b2;

   b1 = l&255;
   b2 = (l>>8)&255;

   return (b1<<8) + b2;
}

short   LittleShort (short l)
{
   return l;
}


int    BigLong (int l)
{
   byte    b1,b2,b3,b4;

   b1 = l&255;
   b2 = (l>>8)&255;
   b3 = (l>>16)&255;
   b4 = (l>>24)&255;

   return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

int    LittleLong (int l)
{
   return l;
}

float   BigFloat (float l)
{
   union {byte b[4]; float f;} in, out;

   in.f = l;
   out.b[0] = in.b[3];
   out.b[1] = in.b[2];
   out.b[2] = in.b[1];
   out.b[3] = in.b[0];

   return out.f;
}

float   LittleFloat (float l)
{
   return l;
}


#endif


//=============================================================================

int       nummodels;
dmodel_t    dmodels[MAX_MAP_MODELS];

int          visdatasize;
byte        dvisdata[MAX_MAP_VISIBILITY];

int          lightdatasize;
byte        dlightdata[MAX_MAP_LIGHTING];

int          texdatasize;
byte        dtexdata[MAX_MAP_MIPTEX]; // (dmiptexlump_t)

int          entdatasize;
char        dentdata[MAX_MAP_ENTSTRING];

int        numleafs;
dleaf_t      dleafs[MAX_MAP_LEAFS];

int        numplanes;
dplane_t     dplanes[MAX_MAP_PLANES];

int        numvertexes;
dvertex_t    dvertexes[MAX_MAP_VERTS];

int        numnodes;
dnode_t      dnodes[MAX_MAP_NODES];

int       numtexinfo;
texinfo_t    texinfo[MAX_MAP_TEXINFO];

int        numfaces;
dface_t      dfaces[MAX_MAP_FACES];

int        numclipnodes;
dclipnode_t  dclipnodes[MAX_MAP_CLIPNODES];

int        numedges;
dedge_t      dedges[MAX_MAP_EDGES];

int               nummarksurfaces;
unsigned short      dmarksurfaces[MAX_MAP_MARKSURFACES];

int       numsurfedges;
int         dsurfedges[MAX_MAP_SURFEDGES];

//=============================================================================

/*
=============
SwapBSPFile

Byte swaps all data in a bsp file.
=============
*/
void SwapBSPFile (qboolean todisk)
{
   int            i, j, c;
   dmodel_t      *d;
   dmiptexlump_t   *mtl;


// models
   for (i=0 ; i<nummodels ; i++)
   {
      d = &dmodels[i];

      for (j=0 ; j<MAX_MAP_HULLS ; j++)
         d->headnode[j] = LittleLong (d->headnode[j]);

      d->visleafs = LittleLong (d->visleafs);
      d->firstface = LittleLong (d->firstface);
      d->numfaces = LittleLong (d->numfaces);

      for (j=0 ; j<3 ; j++)
      {
         d->mins[j] = LittleFloat(d->mins[j]);
         d->maxs[j] = LittleFloat(d->maxs[j]);
         d->origin[j] = LittleFloat(d->origin[j]);
      }
   }

//
// vertexes
//
   for (i=0 ; i<numvertexes ; i++)
   {
      for (j=0 ; j<3 ; j++)
         dvertexes[i].point[j] = LittleFloat (dvertexes[i].point[j]);
   }

//
// planes
//
   for (i=0 ; i<numplanes ; i++)
   {
      for (j=0 ; j<3 ; j++)
         dplanes[i].normal[j] = LittleFloat (dplanes[i].normal[j]);
      dplanes[i].dist = LittleFloat (dplanes[i].dist);
      dplanes[i].type = LittleLong (dplanes[i].type);
   }

//
// texinfos
//
   for (i=0 ; i<numtexinfo ; i++)
   {
      for (j=0 ; j<8 ; j++)
         texinfo[i].vecs[0][j] = LittleFloat (texinfo[i].vecs[0][j]);
      texinfo[i].miptex = LittleLong (texinfo[i].miptex);
      texinfo[i].flags = LittleLong (texinfo[i].flags);
   }

//
// faces
//
   for (i=0 ; i<numfaces ; i++)
   {
      dfaces[i].texinfo = LittleShort (dfaces[i].texinfo);
      dfaces[i].planenum = LittleShort (dfaces[i].planenum);
      dfaces[i].side = LittleShort (dfaces[i].side);
      dfaces[i].lightofs = LittleLong (dfaces[i].lightofs);
      dfaces[i].firstedge = LittleLong (dfaces[i].firstedge);
      dfaces[i].numedges = LittleShort (dfaces[i].numedges);
   }

//
// nodes
//
   for (i=0 ; i<numnodes ; i++)
   {
      dnodes[i].planenum = LittleLong (dnodes[i].planenum);
      for (j=0 ; j<3 ; j++)
      {
         dnodes[i].mins[j] = LittleShort (dnodes[i].mins[j]);
         dnodes[i].maxs[j] = LittleShort (dnodes[i].maxs[j]);
      }
      dnodes[i].children[0] = LittleShort (dnodes[i].children[0]);
      dnodes[i].children[1] = LittleShort (dnodes[i].children[1]);
      dnodes[i].firstface = LittleShort (dnodes[i].firstface);
      dnodes[i].numfaces = LittleShort (dnodes[i].numfaces);
   }

//
// leafs
//
   for (i=0 ; i<numleafs ; i++)
   {
      dleafs[i].contents = LittleLong (dleafs[i].contents);
      for (j=0 ; j<3 ; j++)
      {
         dleafs[i].mins[j] = LittleShort (dleafs[i].mins[j]);
         dleafs[i].maxs[j] = LittleShort (dleafs[i].maxs[j]);
      }

      dleafs[i].firstmarksurface = LittleShort (dleafs[i].firstmarksurface);
      dleafs[i].nummarksurfaces = LittleShort (dleafs[i].nummarksurfaces);
      dleafs[i].visofs = LittleLong (dleafs[i].visofs);
   }

//
// clipnodes
//
   for (i=0 ; i<numclipnodes ; i++)
   {
      dclipnodes[i].planenum = LittleLong (dclipnodes[i].planenum);
      dclipnodes[i].children[0] = LittleShort (dclipnodes[i].children[0]);
      dclipnodes[i].children[1] = LittleShort (dclipnodes[i].children[1]);
   }

//
// miptex
//
   if (texdatasize)
   {
      mtl = (dmiptexlump_t *)dtexdata;
      if (todisk)
         c = mtl->nummiptex;
      else
         c = LittleLong(mtl->nummiptex);
      mtl->nummiptex = LittleLong (mtl->nummiptex);
      for (i=0 ; i<c ; i++)
         mtl->dataofs[i] = LittleLong(mtl->dataofs[i]);
   }

//
// marksurfaces
//
   for (i=0 ; i<nummarksurfaces ; i++)
      dmarksurfaces[i] = LittleShort (dmarksurfaces[i]);

//
// surfedges
//
   for (i=0 ; i<numsurfedges ; i++)
      dsurfedges[i] = LittleLong (dsurfedges[i]);

//
// edges
//
   for (i=0 ; i<numedges ; i++)
   {
      dedges[i].v[0] = LittleShort (dedges[i].v[0]);
      dedges[i].v[1] = LittleShort (dedges[i].v[1]);
   }
}


dheader_t   *header;

int CopyLump (int lump, void *dest, int size)
{
   int      length, ofs;

   length = header->lumps[lump].filelen;
   ofs = header->lumps[lump].fileofs;

   if (length % size)
      Error ("LoadBSPFile: odd lump size");

   memcpy (dest, (byte *)header + ofs, length);

   return length / size;
}

/*
=============
LoadBSPFile
=============
*/
void LoadBSPFile (char *filename)
{
   size_t i;

//
// load the file header
//
   LoadFile (filename, (void **)&header);

// swap the header
   for (i=0 ; i < (sizeof(dheader_t)/4); i++)
      ((int *)header)[i] = LittleLong ( ((int *)header)[i]);

   switch (header->version) {
   case BSPVERSION:
      break;
   case BSP2VERSION_2PSB:
      Error("RMQ format maps are not supported");
      break;
   case BSP2VERSION_BSP2:
      Error("BSP2 format maps are not supported");
      break;
   default:
      Error("%s is version %i, not %i", filename, header->version, BSPVERSION);
      break;
   }

   nummodels = CopyLump (LUMP_MODELS, dmodels, sizeof(dmodel_t));
   numvertexes = CopyLump (LUMP_VERTEXES, dvertexes, sizeof(dvertex_t));
   numplanes = CopyLump (LUMP_PLANES, dplanes, sizeof(dplane_t));
   numleafs = CopyLump (LUMP_LEAFS, dleafs, sizeof(dleaf_t));
   numnodes = CopyLump (LUMP_NODES, dnodes, sizeof(dnode_t));
   numtexinfo = CopyLump (LUMP_TEXINFO, texinfo, sizeof(texinfo_t));
   numclipnodes = CopyLump (LUMP_CLIPNODES, dclipnodes, sizeof(dclipnode_t));
   numfaces = CopyLump (LUMP_FACES, dfaces, sizeof(dface_t));
   nummarksurfaces = CopyLump (LUMP_MARKSURFACES, dmarksurfaces, sizeof(dmarksurfaces[0]));
   numsurfedges = CopyLump (LUMP_SURFEDGES, dsurfedges, sizeof(dsurfedges[0]));
   numedges = CopyLump (LUMP_EDGES, dedges, sizeof(dedge_t));

   texdatasize = CopyLump (LUMP_TEXTURES, dtexdata, 1);
   visdatasize = CopyLump (LUMP_VISIBILITY, dvisdata, 1);
   lightdatasize = CopyLump (LUMP_LIGHTING, dlightdata, 1);
   entdatasize = CopyLump (LUMP_ENTITIES, dentdata, 1);

   free (header);      // everything has been copied out

//
// swap everything
//
   SwapBSPFile (false);
}
