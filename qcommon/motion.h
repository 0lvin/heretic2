#ifndef QCOMMON_MOTION_H
#define QCOMMON_MOTION_H

#include "qcommon.h"

#ifdef __cplusplus
extern "C" {
#endif

float GetTimeToReachDistance(float, float, float);
float GetDistanceOverTime(float, float, float);
void GetPositionOverTime(vec3_t, vec3_t, vec3_t, float, vec3_t);
void GetVelocityOverTime(vec3_t velocity, vec3_t accel, float time, vec3_t output);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif
// end