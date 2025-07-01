#ifndef INC_RENDER_H
#define INC_RENDER_H

#include "bspfile.h"
#include "3d.h"

extern int bbox_inside_plane(short *mins, short *maxs, dplane_t *plane);
extern int node_in_frustrum(dnode_t *node, dplane_t *planes);
extern int leaf_in_frustrum(dleaf_t *node, dplane_t *planes);
extern void mark_leaf_faces(int leaf);
extern int visit_visible_leaves(vector *cam_loc);
extern void render_node_faces(int node, int side);
extern void render_world(vector *loc);

#endif
