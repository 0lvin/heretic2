/* poly.c - Render a Quake polygon:
 *      read it from the db
 *      transform it into 3d
 *      clip it in 3d
 *      compute the 2d texture gradients
 *      scan convert
 *      pass off the spans
 *
 *   Copyright 1997 Sean Barett (Public domain)
 */

#include "bspfile.h"
#include "fix.h"
#include "3d.h"
#include "tm.h"
#include "tmap3d.h"
#include "surface.h"

point_3d pts[32], *default_vlist[32];

void setup_default_point_list(void)
{
   int i;
   for (i=0; i < 32; ++i)
      default_vlist[i] = &pts[i];
}

fix scan[768][2];

void scan_convert(point_3d *a, point_3d *b)
{
   void *temp;
   int right;
   fix x,dx;
   int y,ey;

   if (a->sy == b->sy)
      return;

   if (a->sy < b->sy) {
      right = 0;
   } else {
      temp = a;
      a = b;
      b = temp;
      right = 1;
   }

   // compute dxdy
   dx = FLOAT_TO_INT(65536.0 * (b->sx - a->sx) / (b->sy - a->sy));
   x = a->sx;
   y = fix_cint(a->sy);
   ey = fix_cint(b->sy);

   // fixup x location to 'y' (subpixel correction in y)
   x += FLOAT_TO_INT(((double) dx * ((y << 16) - a->sy)) / 65536.0);

   while (y < ey) {
      scan[y][right] = x;
      x += dx;
      ++y;
   }
}

void draw_poly(int n, point_3d **vl)
{
   int i,j, y,ey;
   fix ymin,ymax;

   // find max and min y height
   ymin = ymax = vl[0]->sy;
   for (i=1; i < n; ++i) {
           if (vl[i]->sy < ymin) ymin = vl[i]->sy;
      else if (vl[i]->sy > ymax) ymax = vl[i]->sy;
   }

   // scan out each edge
   j = n-1;
   for (i=0; i < n; ++i) {
      scan_convert(vl[i], vl[j]);
      j = i;
   }

    y = fix_cint(ymin);
   ey = fix_cint(ymax);

   // iterate over all spans and draw

   while (y < ey) {
      int sx = fix_cint(scan[y][0]), ex = fix_cint(scan[y][1]);
      if (sx < ex)
         qmap_draw_span(y, sx, ex);
      ++y;
   }
}

void draw_face(int face)
{
   int n = dfaces[face].numedges;
   int se = dfaces[face].firstedge;
   int i,codes_or=0,codes_and=0xff;
   point_3d **vlist;

   for (i=0; i < n; ++i) {
      int edge = dsurfedges[se+i];
      if (edge < 0)
         transform_point(&pts[i], (vector *) VERTEX(dedges[-edge].v[1]));
      else
         transform_point(&pts[i], (vector *) VERTEX(dedges[ edge].v[0]));
      codes_or  |= pts[i].ccodes;
      codes_and &= pts[i].ccodes;
   }

   if (codes_and) return;  // abort if poly outside frustrum

   if (codes_or) {
      // poly crosses frustrum, so clip it
      n = clip_poly(n, default_vlist, codes_or, &vlist);
   } else
      vlist = default_vlist;

   if (n) {
      bitmap bm;
      float u,v;
      int tex = dfaces[face].texinfo;
      int mip = compute_mip_level(face);
      get_tmap(&bm, face, texinfo[tex].miptex, mip, &u, &v);
      qmap_set_texture(&bm);
      compute_texture_gradients(face, tex, mip, u, v);
      draw_poly(n, vlist);
   }
}
