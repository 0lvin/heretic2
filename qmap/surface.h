#ifndef INC_SURFACE_H
#define INC_SURFACE_H

#include "s.h"
#include "tm.h"

extern void init_cache(void);
extern void get_tmap(bitmap *, int face, int tex, int mip, float *u, float *v);

typedef struct
{
   int face;
   int mip_level;
   int valid;
   bitmap *bm;
   float u,v;
} surface_t;

#define MAX_CACHE            (1024*1024)

// must be power of two
#define MAX_CACHED_SURFACES  (1024)

extern uchar colormap[64][256];

#endif
