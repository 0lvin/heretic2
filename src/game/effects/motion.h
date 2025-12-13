//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef QCOMMON_MOTION_H
#define QCOMMON_MOTION_H

#include "../../common/header/common.h"

float GetTimeToReachDistance(float, float, float);
float GetDistanceOverTime(float, float, float);
void GetPositionOverTime(vec3_t, vec3_t, vec3_t, float, vec3_t);
void GetVelocityOverTime(vec3_t velocity, vec3_t accel, float time, vec3_t output);

#endif
