//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef DECALS_H
#define DECALS_H

#include "../common/header/common.h"

qboolean IsDecalApplicable(edict_t *owner, edict_t *target, vec3_t origin, csurface_t *surface,cplane_t *plane, vec3_t planeDir);

#endif
