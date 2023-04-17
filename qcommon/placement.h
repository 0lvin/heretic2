#ifndef PLACEMENT_H
#define PLACEMENT_H

#include "qcommon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Placement_s
{
	vec3_t origin;
	vec3_t direction;
	vec3_t up;
} Placement_t;
#ifdef __cplusplus
} //end extern "C"
#endif

#endif