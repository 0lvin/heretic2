/* tm_c.c - Texture mapper in C
 *   takes any size texture map, renders
 *   it point-sampled with no lighting
 *
 *   Copyright 1997 Sean Barrett
 */

#include "s.h"
#include "fix.h"
#include "tm.h"
#include "mode.h"

char  *qmap_buf,    *qmap_tex;
int    qmap_buf_row, qmap_tex_row;
int    qmap_wid, qmap_ht;

#define NUMROWS (768)
int    qmap_row_table[NUMROWS];      // max height of screen
int    qmap_tex_row_table[258];  // max height of texture + 2

static double qmap_tmap[9];

// draw an affine (linear) span starting at dest, n pixels long,
// starting at (u,v) in the texture and stepping by (du,dv) each pixel

int is_cached;

void qmap_draw_affine(int n, char *dest, fix u, fix v, fix du, fix dv)
{
   if (is_cached) {
      while (n--) {
         int iu = fix_int(u);
         int iv = fix_int(v);
         *dest++ = qmap_tex[qmap_tex_row_table[iv+1] + iu];
         u += du;
         v += dv;
      }
   } else {
      while (n--) {
         int iu = (fix_int(u)) & qmap_wid;
         int iv = (fix_int(v)) & qmap_ht;
         *dest++ = qmap_tex[qmap_tex_row_table[iv+1] + iu];
         u += du;
         v += dv;
      }
   }
}

#define SUBDIV_SHIFT  4
#define SUBDIV        (1 << SUBDIV_SHIFT)

// given a span (x0,y)..(x1,y), draw a perspective-correct span for it
void qmap_draw_span(int y, int sx, int ex)
{
   double u0,v0,w0, u1,v1,w1, z;
   int len, e, last = 0;
   fix u,v,du,dv;

   // compute (u,v) at left end

   u0 = qmap_tmap[0] + sx * qmap_tmap[1] + y * qmap_tmap[2];
   v0 = qmap_tmap[3] + sx * qmap_tmap[4] + y * qmap_tmap[5];
   w0 = qmap_tmap[6] + sx * qmap_tmap[7] + y * qmap_tmap[8];

   z  =  1 / w0;
   u0 = u0 * z;
   v0 = v0 * z;

#ifdef CLAMPING
   // if you turn on clamping, you have to change qmap_wid & qmap_ht
   // back to being wid&ht, not wid-1 & ht-1 as they are right now
   if (u0 < 0) u0 = 0; else if (u0 >= qmap_wid) u0 = qmap_wid-0.01;
   if (v0 < 0) v0 = 0; else if (v0 >= qmap_ht) v0 = qmap_ht-0.01;
#endif

   for (;;) {
      len = ex - sx;
      if (len > SUBDIV)
         len = SUBDIV;
      else
         last = 1;

      u = FLOAT_TO_FIX(u0);
      v = FLOAT_TO_FIX(v0);

      if (len == 1) {
         // shortcut out to avoid divide by 0 below
         qmap_draw_affine(len, qmap_buf + qmap_row_table[y] + sx, u, v, 0, 0);
         return;
      }

      e = sx + len - last;

      u1 = qmap_tmap[0] + e * qmap_tmap[1] + y * qmap_tmap[2];
      v1 = qmap_tmap[3] + e * qmap_tmap[4] + y * qmap_tmap[5];
      w1 = qmap_tmap[6] + e * qmap_tmap[7] + y * qmap_tmap[8];

      z = 1 / w1;
      u1 = u1 * z;
      v1 = v1 * z;

#ifdef CLAMPING
      if (u1 < 0) u1 = 0; else if (u1 >= qmap_wid) u1 = qmap_wid-0.01;
      if (v1 < 0) v1 = 0; else if (v1 >= qmap_ht) v1 = qmap_ht-0.01;
#endif

      if (len == SUBDIV) {
         du = (FLOAT_TO_FIX(u1) - u) >> SUBDIV_SHIFT;
         dv = (FLOAT_TO_FIX(v1) - v) >> SUBDIV_SHIFT;
      } else {
         du = FLOAT_TO_FIX((u1-u0)/(len-last));
         dv = FLOAT_TO_FIX((v1-v0)/(len-last));
      }
      if (du < 0) ++du;
      if (dv < 0) ++dv;

      qmap_draw_affine(len, qmap_buf + qmap_row_table[y] + sx, u, v, du, dv);
      if (last)
         break;

      sx += len;
      u0 = u1;
      v0 = v1;
   }
}

///  setup global variables code

void qmap_set_output(char *where, int row)
{
   qmap_buf = where;
   if (qmap_buf_row != row) {
      int i;
      qmap_buf_row = row;
      for (i=0; i < NUMROWS; ++i)
         qmap_row_table[i] = i * row;
   }
}

void qmap_set_texture(bitmap *bm)
{
   int row = bm->wid, ht = bm->ht;
   qmap_tex = bm->bits;
   qmap_wid = bm->wid-1;

   if (qmap_tex_row != row || qmap_ht != ht-1) {
      int i;
      qmap_tex_row = row;
      qmap_ht = ht-1;
      for (i=0; i < ht; ++i)
         qmap_tex_row_table[i+1] = row * i;
      qmap_tex_row_table[0] = qmap_tex_row_table[1];
      qmap_tex_row_table[ht+1] = qmap_tex_row_table[ht];
   }
}

void qmap_set_texture_gradients(double *tmap_data)
{
   int i;
   for (i=0; i < 9; ++i)
      qmap_tmap[i] = tmap_data[i];
}
