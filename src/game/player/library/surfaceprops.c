//
// Heretic II
// Copyright 1998 Raven Software
//
#include "surfaceprops.h"

char *GetClientGroundSurfaceMaterialName(playerinfo_t *playerinfo)
{
#if 1
	return NULL;
#else
	/* TODO: Rewrite Ground is not set so disable it for now */
	csurface_t *groundSurface;
	char *result = NULL;

	groundSurface = playerinfo->GroundSurface; // jmarshall - check this.
	if (groundSurface)
	{
		const char *surfacePropNames[] = {
			"gravel",
			"metal",
			"stone",
			"wood",
		}
		result = surfacePropNames[(groundSurface->value >> 24) % 4];
	}
	return result;
#endif
}
