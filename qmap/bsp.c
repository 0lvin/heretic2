/* bsp.c - routines that recurse through bsp tree
 *   Copyright 1997 Sean Barrett (Public domain)
 */

#include "bspfile.h"
#include "3d.h"
#include "bsp.h"
#include "render.h"

int point_plane_test(vector *loc, dplane_t *plane)
{
   return plane->normal[0] * loc->x + plane->normal[1] * loc->y
        + plane->normal[2]*loc->z < plane->dist;
}

int find_leaf(vector *loc)
{
   int n = dmodels[0].headnode[0];
   while (n >= 0) {
      dnode_t *node = &dnodes[n];
      n = node->children[point_plane_test(loc, &dplanes[node->planenum])];
   }
   return ~n;
}

char vis_node[MAX_MAP_NODES];
extern char vis_leaf[];

static dplane_t *planes;
static vector *loc;

void bsp_render_node(int node)
{
    if (node >= 0 && vis_node[node]) {
       if (point_plane_test(loc, &dplanes[dnodes[node].planenum])) {
          bsp_render_node(dnodes[node].children[0]);
          render_node_faces(node, 1);
          bsp_render_node(dnodes[node].children[1]);
       } else {
          bsp_render_node(dnodes[node].children[1]);
          render_node_faces(node, 0);
          bsp_render_node(dnodes[node].children[0]);
       }
   }
}

void bsp_render_world(vector *cam_loc, dplane_t *pl)
{
   planes = pl;
   loc = cam_loc;
   bsp_render_node((int) dmodels[0].headnode[0]);
}

// recursively determine which nodes need exploring (so we
// don't look for polygons on _every_ node in the level)
int bsp_find_visible_nodes(int node)
{
   if (node >= 0) {
      vis_node[node] = !!(bsp_find_visible_nodes(dnodes[node].children[0])
                       | bsp_find_visible_nodes(dnodes[node].children[1]));
      return vis_node[node];
   }
   else {
      node = ~node;
      return (vis_leaf[node >> 3] & (1 << (node & 7)));
   }
}

void bsp_explore_node(int node)
{
   if (node < 0) {
      node = ~node;
      if (vis_leaf[node >> 3] & (1 << (node & 7)))
         if (leaf_in_frustrum(&dleafs[node], planes))
            mark_leaf_faces(node);
      return;
   }

   if (vis_node[node]) {
      if (!node_in_frustrum(&dnodes[node], planes))
         vis_node[node] = 0;
      else {
         bsp_explore_node(dnodes[node].children[0]);
         bsp_explore_node(dnodes[node].children[1]);
      }
   }
}

void bsp_visit_visible_leaves(vector *cam_loc, dplane_t *pl)
{
   planes = pl;
   loc = cam_loc;
   bsp_find_visible_nodes((int) dmodels[0].headnode[0]);
   bsp_explore_node((int) dmodels[0].headnode[0]);
}
