// H2Physics.c
//

#include "../src/common/header/common.h"
#include "../qcommon/q_physics.h"

#include <stdint.h>

void BounceVelocity(vec3_t in, vec3_t normal, vec3_t out, float elasticity)
{
	double v5;

	v5 = -((normal[2] * in[2] + in[1] * normal[1] + in[0] * normal[0]) * elasticity);
	out[0] = v5 * normal[0] + in[0];
	out[1] = v5 * normal[1] + in[1];
	out[2] = v5 * normal[2] + in[2];
}

qboolean BoundVelocity(float *vel)
{
	float *v1;
	int v2;
	signed int v3;
	double v4; 

	v1 = vel;
	v2 = 0;
	v3 = 3;
	do
	{
		v4 = *v1;
		if (v4 <= -0.1 || v4 >= 0.1)
		{
			if (v4 <= 2000.0)
			{
				if (v4 < -2000.0)
					*(uint32_t *)v1 = -990248960;
			}
			else
			{
				*(uint32_t *)v1 = 1157234688;
			}
		}
		else
		{
			*(uint32_t *)v1 = 0;
			++v2;
		}
		++v1;
		--v3;
	} while (v3);
	return v2 != 3;
}