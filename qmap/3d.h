#ifndef INC_3D_H
#define INC_3D_H

#include "s.h"
#include "fix.h"
#include "bspfile.h"

typedef struct
{
   float x,y,z;
} vector;

typedef struct
{
   double tx,ty,tz;
} angvec;

typedef struct
{
   vector p;
   uchar  ccodes;
   uchar  pad0,pad1,pad2;
   fix    sx,sy;
   float  u,v;
} point_3d;   // 32 bytes

// vector math
extern double dot_vec(vector *a, vector *b);
extern double dot_vec_dbl(double *a, vector *b);
extern void normeq_vec(vector *vec);

// 3d math
extern void rotate(double *x, angvec *ang);
extern void rotate_c2w(vector *dest, vector *src);
extern void rotate_vec(vector *item);
extern void set_view_info(vector *loc, angvec *ang);
extern void transform_point_raw(vector *out, vector *in);
extern void rotate_point_raw(vector *out, vector *in);
extern void transform_vector(vector *out, vector *in);
extern double dist2_from_viewer(vector *loc);
extern void compute_view_frustrum(dplane_t *planes);


// point_3d stuff
extern void project_point(point_3d *p);
extern void transform_point(point_3d *p, vector *v);
extern void transform_rotated_point(point_3d *p);
extern void code_point(point_3d *p);
extern int clip_poly(int n, point_3d **vl, int codes_or, point_3d ***out_vl);

extern double clip_scale_x, clip_scale_y; // needed by poly clipper

extern float proj_ymod;

#define CC_OFF_LEFT 1
#define CC_OFF_RIGHT 2
#define CC_OFF_TOP 4
#define CC_OFF_BOT 8
#define CC_BEHIND 16

#endif
