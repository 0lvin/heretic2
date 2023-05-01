//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef DECALS_H
#define DECALS_H

#include "../src/common/header/common.h"

#ifdef __cplusplus
extern "C" {
#endif

qboolean IsDecalApplicable(edict_t *owner, edict_t *target, vec3_t origin, csurface_t *surface,cplane_t *plane, vec3_t planeDir);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif
