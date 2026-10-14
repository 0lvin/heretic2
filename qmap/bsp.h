
#ifndef INC_BSP_H
#define INC_BSP_H

#include "3d.h"
#include "bspfile.h"

extern int find_leaf(vector *loc);
extern void bsp_visit_visible_leaves(vector *cam_loc, dplane_t *pl);
extern void bsp_render_world(vector *cam_loc, dplane_t *pl);

#endif
