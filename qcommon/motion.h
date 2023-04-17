#ifndef QCOMMON_MOTION_H
#define QCOMMON_MOTION_H

#include "../qcommon/qcommon.h"

#ifdef __cplusplus
extern "C" {
#endif

QUAKE2_API float GetTimeToReachDistance(float, float, float);
QUAKE2_API float GetDistanceOverTime(float, float, float);
QUAKE2_API void GetPositionOverTime(vec3_t, vec3_t, vec3_t, float, vec3_t);
QUAKE2_API void GetVelocityOverTime(vec3_t velocity, vec3_t accel, float time, vec3_t output);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif
// end