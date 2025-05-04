//
// Heretic II
// Copyright 1998 Raven Software
//
// H2Physics.c
//

#include "../../common/header/common.h"

/* TODO: Rewrite */
void
BounceVelocity(vec3_t in, vec3_t normal, vec3_t out, float elasticity)
{
	float scale;

	scale = -DotProduct(in, normal) * elasticity;
	VectorMA(in, scale, normal, out);
}
