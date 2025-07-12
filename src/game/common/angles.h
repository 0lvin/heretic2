//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef QCOMMON_ANGLES_H
#define QCOMMON_ANGLES_H

// Angles in radians

#define ANGLE_1			0.017453292F
#define ANGLE_5			0.087266462F
#define ANGLE_10		0.174532925F
#define ANGLE_15		0.261799387F
#define ANGLE_30		0.523598775F
#define ANGLE_45		0.785398163F
#define ANGLE_90		1.570796327F
#define ANGLE_180		3.141592653F
#define ANGLE_360		6.283185307F

// Conversion routines

#define ANGLE_TO_RAD	ANGLE_1
#define RAD_TO_ANGLE	(180.0F / ANGLE_180)

#endif
