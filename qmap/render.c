/*  QMAP: Quake level viewer
 *
 *   render.c  Copyright 1997 Sean Barrett
 *
 *   "render" scenes by traversing over the
 *   database, marking already-visited things,
 *   using the PVS information and the "marksurface"
 *   info; use one bsp routine to sort
 */

#include "bspfile.h"
#include "3d.h"
#include "render.h"
#include "bsp.h"
#include "scr.h"
#include "poly.h"
#include "mode.h"

char vis_face[MAX_MAP_FACES/8+1];
char vis_leaf[MAX_MAP_LEAFS/8+1];

#define is_marked(x)     (vis_face[(x) >> 3] &   (1 << ((x) & 7)))
#define mark_face(x)     (vis_face[(x) >> 3] |=  (1 << ((x) & 7)))
#define unmark_face(x)   (vis_face[(x) >> 3] &= ~(1 << ((x) & 7)))

#if 1
  // this works if IEEE, sizeof(float) == sizeof(int)
  #define FLOAT_POSITIVE(x)   (* (int *) (x) >= 0)
#else
  #define FLOAT_POSITIVE(x)   ((x) >= 0)
#endif

int bbox_inside_plane(short *mins, short *maxs, dplane_t *plane)
{
   int i;
   short pt[3];

   // use quick test from graphics gems

   for (i=0; i < 3; ++i)
      if (FLOAT_POSITIVE(&plane->normal[i])) // fast test assuming IEEE
         pt[i] = maxs[i];
      else
         pt[i] = mins[i];

   return plane->normal[0]*pt[0] + plane->normal[1]*pt[1]
        + plane->normal[2]*pt[2] >= plane->dist;
}

int node_in_frustrum(dnode_t *node, dplane_t *planes)
{
   if (!bbox_inside_plane(node->mins, node->maxs, planes+0)
    || !bbox_inside_plane(node->mins, node->maxs, planes+1)
    || !bbox_inside_plane(node->mins, node->maxs, planes+2)
    || !bbox_inside_plane(node->mins, node->maxs, planes+3))
      return 0;
   return 1;
}

int leaf_in_frustrum(dleaf_t *node, dplane_t *planes)
{
   if (!bbox_inside_plane(node->mins, node->maxs, planes+0)
    || !bbox_inside_plane(node->mins, node->maxs, planes+1)
    || !bbox_inside_plane(node->mins, node->maxs, planes+2)
    || !bbox_inside_plane(node->mins, node->maxs, planes+3))
      return 0;
   return 1;
}

void mark_leaf_faces(int leaf)
{
   int n = dleafs[leaf].nummarksurfaces;
   int ms = dleafs[leaf].firstmarksurface;
   int i;

   for (i=0; i < n; ++i) {
      int s = dmarksurfaces[ms+i];
      if (!is_marked(s)) {
         mark_face(s);
      }
   }
}

int visit_visible_leaves(vector *cam_loc)
{
   int n, v, i;

   memset(vis_leaf, 0, sizeof(vis_leaf));

   n = find_leaf(cam_loc);

   if (n == 0 || dleafs[n].visofs < 0)
      return 0;

   v = dleafs[n].visofs;
   for (i = 1; i < numleafs; ) {
      if (dvisdata[v] == 0) {
         i += 8 * dvisdata[v+1];    // skip some leaves
         v+=2;
      } else {
         int j;
         for (j = 0; j <8; j++, i++)
            if (dvisdata[v] & (1<<j) )
               vis_leaf[i>>3] |= (1 << (i & 7));
         ++v;
      }
   }
   return 1;
}

// during a bsp recursion, draw all of the faces
// stored on this node which are visible (i.e. just
// test their mark flag)
void render_node_faces(int node, int side)
{
   int i,n,f;
   n = dnodes[node].numfaces;
   f = dnodes[node].firstface;
   for (i=0; i < n; ++i) {
      if (is_marked(f)) {
         if (dfaces[f].side == side)
            draw_face(f);
         unmark_face(f);
      }
      ++f;
   }
}

void render_world(vector *loc)
{
   dplane_t planes[4];
   compute_view_frustrum(planes);

   if (!visit_visible_leaves(loc)) {
      memset(scr_buf, 0, SCREENW * SCREENH);
      memset(vis_leaf, 255, sizeof(vis_leaf));
   }

   bsp_visit_visible_leaves(loc, planes);
   bsp_render_world(loc, planes);
}
