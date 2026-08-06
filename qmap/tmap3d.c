/* tmap3d.c - compute texture-mapping info that has to
 *   look at stuff in 3d: mipmapping, texture gradient
 *
 *   Copyright 1997 Sean Barrett
 */

#include <math.h>
#include "3d.h"
#include "bspfile.h"
#include "tmap3d.h"
#include "tm.h"
#include "mode.h"

#define DIST  256.0    // dist to switch first mip level
#define DIST2 (DIST*DIST)

int compute_mip_level(int face)
{
   // dumb algorithm: grab 3d coordinate of some vertex,
   // compute dist from viewer
   int se = dfaces[face].firstedge;
   int e = dsurfedges[se];
   double dist;
   if (e < 0) e = -e;
   dist = dist2_from_viewer((vector *) &dvertexes[dedges[e].v[0]]) / DIST2;
   if (dist < 1) return 0;
   if (dist < 4) return 1;
   if (dist < 16) return 2;
   return 3;
}

// iterate over vertices of face, compute u&v coords, compute min & max
void get_face_extent(int face, int *u0, int *v0, int *u1, int *v1)
{
   float uv[32][2], u[4],v[4], umin,umax,vmin,vmax;
   int tex = dfaces[face].texinfo;
   int i,n = dfaces[face].numedges;
   int se = dfaces[face].firstedge;
   vector *loc;

   memcpy(u, texinfo[tex].vecs[0], sizeof(u));
   memcpy(v, texinfo[tex].vecs[1], sizeof(v));

   for (i=0; i < n; ++i) {
      int j = dsurfedges[se+i];
      if (j < 0)
         loc = (vector *) VERTEX(dedges[-j].v[1]);
      else
         loc = (vector *) VERTEX(dedges[ j].v[0]);
      uv[i][0] = loc->x*u[0] + loc->y*u[1] + loc->z*u[2] + u[3];
      uv[i][1] = loc->x*v[0] + loc->y*v[1] + loc->z*v[2] + v[3];
   }
   umin = umax = uv[0][0];
   vmin = vmax = uv[0][1];
   for (i=1; i < n; ++i) {
      if (uv[i][0] < umin) {
         umin = uv[i][0];
      } else if (uv[i][0] > umax) {
         umax = uv[i][0];
      }

      if (uv[i][1] < vmin) {
         vmin = uv[i][1];
      } else if (uv[i][1] > vmax) {
         vmax = uv[i][1];
      }
   }
   *u0 = FLOAT_TO_INT(umin) & ~15;
   *v0 = FLOAT_TO_INT(vmin) & ~15;
   *u1 = FLOAT_TO_INT(ceil(umax/16)) << 4;
   *v1 = FLOAT_TO_INT(ceil(vmax/16)) << 4;
}

void compute_texture_normal(vector *out, vector *u, vector *v)
{
   out->x = u->y*v->z - u->z*v->y;
   out->y = u->z*v->x - u->x*v->z;
   out->z = u->x*v->y - u->y*v->x;
}

void setup_uv_vector(vector *out, vector *in, vector *norm, float *plane)
{
   double dot = -(in->x*plane[0] + in->y*plane[1] + in->z*plane[2])
              / (norm->x*plane[0] + norm->y*plane[1] + norm->z*plane[2]);
   if (dot != 0) {
      vector temp = *in;
      temp.x += norm->x*dot;
      temp.y += norm->y*dot;
      temp.z += norm->z*dot;
      transform_vector(out, &temp);
   } else
      transform_vector(out, in);
}

// compute location of origin of texture (should precompute)
void setup_origin_vector(vector *out, dplane_t *plane, vector *norm)
{
   vector temp;
   float d =  plane->dist / (norm->x*plane->normal[0]
                   + norm->y*plane->normal[1] + norm->z*plane->normal[2]);
   temp.x = d * norm->x;
   temp.y = d * norm->y;
   temp.z = d * norm->z;

   transform_point_raw(out, &temp);
}

void compute_texture_gradients(int face, int tex, int mip, float u, float v)
{
   double tmap_data[9];
   vector P,M,N;
   vector norm;
   dplane_t *plane = &dplanes[dfaces[face].planenum];
   float rescale = (8 >> mip) / 8.0;

   compute_texture_normal(&norm,
         (vector *) texinfo[tex].vecs[0], (vector *) texinfo[tex].vecs[1]);
      // project vectors onto face's plane, and transform
   setup_uv_vector(&M, (vector *) texinfo[tex].vecs[0], &norm, plane->normal);
   setup_uv_vector(&N, (vector *) texinfo[tex].vecs[1], &norm, plane->normal);
   setup_origin_vector(&P, plane, &norm);

   u -= texinfo[tex].vecs[0][3];  // adjust according to face's info
   v -= texinfo[tex].vecs[1][3];  // passed in values adjust for lightmap

   // we could just subtract (u,v) every time we compute a new (u,v);
   // instead we fold it into P:
   P.x += u * M.x + v * N.x;
   P.y += u * M.y + v * N.y;
   P.z += u * M.z + v * N.z;

   tmap_data[0] = P.x*N.y - P.y*N.x;
   tmap_data[1] = P.y*N.z - P.z*N.y;
   tmap_data[2] = P.x*N.z - P.z*N.x;
   tmap_data[3] = P.y*M.x - P.x*M.y;
   tmap_data[4] = P.z*M.y - P.y*M.z;
   tmap_data[5] = P.z*M.x - P.x*M.z;
   tmap_data[6] = N.x*M.y - N.y*M.x;
   tmap_data[7] = N.y*M.z - N.z*M.y;
   tmap_data[8] = N.x*M.z - N.z*M.x;

      // offset by center of screen--if this were folded into
      // transform translation we could avoid it
   const double xofs = (double)SCREENW / 2.0 - 0.5;
   const double yofs = (double)SCREENH / 2.0 - 0.5;

   tmap_data[0] -= tmap_data[1] * xofs + tmap_data[2] * yofs;
   tmap_data[3] -= tmap_data[4] * xofs + tmap_data[5] * yofs;
   tmap_data[6] -= tmap_data[7] * xofs + tmap_data[8] * yofs;

   tmap_data[0] *= rescale;
   tmap_data[1] *= rescale;
   tmap_data[2] *= rescale;
   tmap_data[3] *= rescale;
   tmap_data[4] *= rescale;
   tmap_data[5] *= rescale;

   qmap_set_texture_gradients(tmap_data);
}
