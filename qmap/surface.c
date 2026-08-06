/* surface.c - Build surfaces and cache them
 *   The cache is moronic, using malloc
 *
 *   Copyright 1997 Sean Barrett (Public domain)
 */

#include <stdlib.h>
#include <string.h>
#include "s.h"
#include "tm.h"
#include "bspfile.h"
#include "surface.h"
#include "tmap3d.h"

static surface_t surface[MAX_CACHED_SURFACES];  // circular queue
static int surface_head, surface_tail;          // index into surfaces
static int cur_cache;                           // current storage in use

static short surface_cache[MAX_MAP_FACES];      // cache entry for each face

void init_cache(void)
{
   int i;
   for (i=0; i < MAX_MAP_FACES; ++i)
      surface_cache[i] = -1;
}

void get_raw_tmap(bitmap *bm, int tex, int ml)
{
   dmiptexlump_t *mtl = (dmiptexlump_t *) dtexdata;
   miptex_t *mip;
   mip = (miptex_t *) (dtexdata + mtl->dataofs[tex]);
   bm->bits = (char *) mip + mip->offsets[ml];
   bm->wid  = mip->width >> ml;
   bm->ht   = mip->height >> ml;
}

#define ADJ_SURFACE(x)    ((x) & (MAX_CACHED_SURFACES-1))

void free_surface(int surf)
{
   if (surface[surf].valid)
      surface_cache[surface[surf].face] = -1;

   cur_cache -= surface[surf].bm->wid * surface[surf].bm->ht + sizeof(bitmap);
   free(surface[surf].bm);
}

void free_next_surface(void)
{
   if (surface_head == surface_tail) return;
   free_surface(surface_tail);
   surface_tail = ADJ_SURFACE(surface_tail+1);
}

int allocate_cached_surface(int size)
{
   int surf;

   // make sure there's a free surface entry
   if (ADJ_SURFACE(surface_head+1) == surface_tail)
      free_next_surface();

   surf = surface_head;
   surface_head = ADJ_SURFACE(surface_head+1);

   size += sizeof(bitmap);

   // make sure there's enough storage
   while (cur_cache + size > MAX_CACHE)
      free_next_surface();

   surface[surf].bm = malloc(size);
   cur_cache += size;

   return surf;
}

extern int is_cached;

void build_block(char *out, bitmap *raw, int x, int y);
static int shift, global_step, global_row, lightmap_width;
static uchar *light_index;
static uchar blank_light[512];

void get_tmap(bitmap *bm, int face, int tex, int ml, float *u, float *v)
{
   int i,j, surf, u0,v0,u1,v1, step, x,y,x0;
   bitmap raw;

   is_cached = 1;

   surf = surface_cache[face];
   if (surf >= 0) {
      if (surface[surf].mip_level == ml) {
         *bm = *surface[surf].bm;
         *u  =  surface[surf].u;
         *v  =  surface[surf].v;
         return;
      }
      surface[surf].valid = 0;
      surface_cache[face] = -1;
   }

   get_face_extent(face, &u0, &v0, &u1, &v1);

   bm->wid  = (u1-u0) >> ml;
   bm->ht   = (v1-v0) >> ml;

   if (bm->wid > 256 || bm->ht > 256) {
      is_cached = 0;
      get_raw_tmap(bm, tex, ml);
      return;
   }
   get_raw_tmap(&raw, tex, ml);

   surf = surface_cache[face] = allocate_cached_surface(bm->wid * bm->ht);
   bm->bits = (char *) &surface[surf].bm[1];
   *surface[surf].bm = *bm;

   surface[surf].face = face;
   surface[surf].mip_level = ml;
   surface[surf].valid = 1;
   *u = surface[surf].u = u0;
   *v = surface[surf].v = v0;

   step = 16 >> ml;   // width of lightmap after mipmapping
   shift = 4-ml;      // 1 << shift  == step

   lightmap_width = ((u1 - u0) >> 4)+1;

   // compute texture offsets
   u0 >>= ml;
   v0 >>= ml;

   global_step = step;
   global_row  = bm->wid;
   if (dfaces[face].lightofs == -1)
      light_index = blank_light;
   else
      light_index = &dlightdata[dfaces[face].lightofs];

   y  = v0%raw.ht;  if ( y < 0)  y += raw.ht;   // fixup for signed mod
   x0 = u0%raw.wid; if (x0 < 0) x0 += raw.wid;  // fixup for signed mod

   for (j=0; j < bm->ht; j += step) {
      x = x0;
      for (i=0; i < bm->wid; i += step, ++light_index) {
         build_block(bm->bits+j*bm->wid+i, &raw, x,y);
         x += step; if (x >= raw.wid) x -= raw.wid;
      }
      ++light_index;
      y += step; if (y >= raw.ht) y -= raw.ht;
   }
}

// compute one lightmap square of surface
void build_block(char *out, bitmap *raw, int x, int y)
{
   fix c,dc;
   int a,b,h,c0,c1,c2,c3, step = global_step, row = global_row - step;
   int y_max = raw->ht, x_max = raw->wid;
   uchar *s = (uchar*)raw->bits + y*raw->wid;

   c0 = (255 - light_index[0]) << 16;
   c1 = (255 - light_index[1]) << 16;
   c2 = (255 - light_index[lightmap_width]) << 16;
   c3 = (255 - light_index[lightmap_width+1]) << 16;

   c2 = (c2 - c0) >> shift;
   c3 = (c3 - c1) >> shift;

   for (b=0; b < step; ++b) {
      h = x;
      c = c0;
      dc = (c1 - c0) >> shift;
      for (a=0; a < step; ++a) {
         *out++ = colormap[c >> 18][s[h]];
         c += dc;
         if (++h == x_max) h = 0;
      }
      out += row;
      c0 += c2;
      c1 += c3;

      if (++y == y_max) {
          y = 0;
          s = (uchar*)raw->bits;
      } else
         s += raw->wid;
   }
}
