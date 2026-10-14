/* clippoly.c - clip polygon to view frustrum, in viewspace
 *   Copyright 1997 Sean Barrett
 */

#include "3d.h"

static point_3d pts[16], *clip_list1[40],*clip_list2[40];

#define X p.x
#define Y p.y
#define Z p.z

static void intersect(point_3d *out, point_3d *a, point_3d *b, float where)
{
   // intersection occurs 'where' % along the line from a to b

   out->X = a->X + (b->X - a->X) * where;
   out->Y = a->Y + (b->Y - a->Y) * where;
   out->Z = a->Z + (b->Z - a->Z) * where;

   transform_rotated_point(out);
}

// compute 'where' for various clip planes

static double left_loc(point_3d *a, point_3d *b)
{
   return -(a->Z+a->X*clip_scale_x)/((b->X-a->X)*clip_scale_x + b->Z-a->Z);
}

static double right_loc(point_3d *a, point_3d *b)
{
   return  (a->Z-a->X*clip_scale_x)/((b->X-a->X)*clip_scale_x - b->Z+a->Z);
}

static double top_loc(point_3d *a, point_3d *b)
{
   return  (a->Z-a->Y*clip_scale_y)/((b->Y-a->Y)*clip_scale_y - b->Z+a->Z);
}

static double bottom_loc(point_3d *a, point_3d *b)
{
   return -(a->Z+a->Y*clip_scale_y)/((b->Y-a->Y)*clip_scale_y + b->Z-a->Z);
}

// clip the polygon to each of the view frustrum planes
int clip_poly(int n, point_3d **vl, int codes_or, point_3d ***out_vl)
{
   int i,j,k,p=0; // p = index into temporary point pool
   point_3d **cur;

   if (codes_or & CC_OFF_LEFT) {
      cur = clip_list1;
      k = 0;
      j = n-1;
      for (i=0; i < n; ++i) {
         // process edge from j..i
         // if j is inside, add it

         if (!(vl[j]->ccodes & CC_OFF_LEFT))
            cur[k++] = vl[j];

         // if it crosses, add the intersection point

         if ((vl[j]->ccodes ^ vl[i]->ccodes) & CC_OFF_LEFT) {
            intersect(&pts[p], vl[i], vl[j], left_loc(vl[i],vl[j]));
            cur[k++] = &pts[p++];
         }
         j = i;
      }
      // move output list to be input
      n = k;
      vl = cur;
   }
   if (codes_or & CC_OFF_RIGHT) {
      cur = (vl == clip_list1) ? clip_list2 : clip_list1;
      k = 0;
      j = n-1;
      for (i=0; i < n; ++i) {
         if (!(vl[j]->ccodes & CC_OFF_RIGHT))
            cur[k++] = vl[j];
         if ((vl[j]->ccodes ^ vl[i]->ccodes) & CC_OFF_RIGHT) {
            intersect(&pts[p], vl[i], vl[j], right_loc(vl[i],vl[j]));
            cur[k++] = &pts[p++];
         }
         j = i;
      }
      n = k;
      vl = cur;
   }
   if (codes_or & CC_OFF_TOP) {
      cur = (vl == clip_list1) ? clip_list2 : clip_list1;
      k = 0;
      j = n-1;
      for (i=0; i < n; ++i) {
         if (!(vl[j]->ccodes & CC_OFF_TOP))
            cur[k++] = vl[j];
         if ((vl[j]->ccodes ^ vl[i]->ccodes) & CC_OFF_TOP) {
            intersect(&pts[p], vl[i], vl[j], top_loc(vl[i],vl[j]));
            cur[k++] = &pts[p++];
         }
         j = i;
      }
      n = k;
      vl = cur;
   }
   if (codes_or & CC_OFF_BOT) {
      cur = (vl == clip_list1) ? clip_list2 : clip_list1;
      k = 0;
      j = n-1;
      for (i=0; i < n; ++i) {
         if (!(vl[j]->ccodes & CC_OFF_BOT))
            cur[k++] = vl[j];
         if ((vl[j]->ccodes ^ vl[i]->ccodes) & CC_OFF_BOT) {
            intersect(&pts[p], vl[i], vl[j], bottom_loc(vl[i],vl[j]));
            cur[k++] = &pts[p++];
         }
         j = i;
      }
      n = k;
      vl = cur;
   }
   for (i=0; i < n; ++i)
      if (vl[i]->ccodes & CC_BEHIND)
         return 0;

   *out_vl = vl;
   return n;
}
