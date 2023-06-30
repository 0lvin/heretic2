//
// Heretic II
// Copyright 1998 Raven Software
//
// H2Physics.c
//

#include "../src/common/header/common.h"
#include "q_physics.h"

#include <stdint.h>

// TODO: Rewrite
void BounceVelocity(vec3_t in, vec3_t normal, vec3_t out, float elasticity)
{
	double v5;

	v5 = -((normal[2] * in[2] + in[1] * normal[1] + in[0] * normal[0]) * elasticity);
	out[0] = v5 * normal[0] + in[0];
	out[1] = v5 * normal[1] + in[1];
	out[2] = v5 * normal[2] + in[2];
}
