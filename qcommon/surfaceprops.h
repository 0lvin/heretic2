//
// surfaceprops.h
//
// Heretic II
// Copyright 1998 Raven Software
//

#ifndef _SURFACEPROPS_H_
#define _SURFACEPROPS_H_

//#include "../player/p_types.h"
#include "../player/player.h"

#define MAX_SURFACE_MAT_NAMES 256

extern char *SurfaceMaterialNames[MAX_SURFACE_MAT_NAMES];

extern char *GetClientGroundSurfaceMaterialName(playerinfo_t *playerinfo);

#endif // _SURFACEPROPS_H_