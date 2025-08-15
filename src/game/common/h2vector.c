//
// Heretic II
// Copyright 1998 Raven Software
//
// H2Vector.c
//

#include "../../common/header/shared.h"
#include "angles.h"
#include "vector.h"
#include "matrix.h"
#include "h2rand.h"

vec3_t vec3_right = { 1.0f, 0.0f, 0.0f };
vec3_t vec3_up = { 0.0f, 0.0f, 1.0f };

void AnglesFromDir(vec3_t direction, vec3_t angles)
{
	angles[1] = atan2(direction[1], direction[0]);
	angles[0] = asin(direction[2]);
	angles[2] = 0;
}

void AnglesFromDirI(vec3_t direction, vec3_t angles)
{
	angles[1] = atan2(direction[1], direction[0]);
	angles[0] = asin(direction[2]);
	angles[2] = -angles[1];
}

void Create_rand_relect_vect(vec3_t in, vec3_t out)
{
	double v3;
	float v4;
	double v5;
	float v6;
	vec3_t angles;

	angles[2] = 0.0f;

	if (VectorNormalize(in) >= 0.00050000002)
	{
		in[0] = in[0] * -1.0;
		in[1] = in[1] * -1.0;
		v3 = in[2] * -1.0;
		in[2] = v3;
		AnglesFromDir(in, angles);
		flrand(0.0, 0.5);
		flrand(0.0, 0.30000001);
		v6 = v3;
		if (irand(0, 1))
			v6 = -v6;
		v4 = v3;
		v5 = v4;
		if (irand(0, 1))
			v5 = -v5;
		angles[0] = angles[0] + v6;
		angles[1] = angles[1] + v5;
		DirFromAngles(angles, out);
	}
	else
	{
		out[0] = 1;
		out[1] = 0.0f;
		out[2] = 0.0f;
	}
}

// https://github.com/mdeguzis/ftequake
void VectorAngles(float* forward, float* up, float* result)	//up may be NULL
{
	float	yaw, pitch, roll;

	if (forward[1] == 0 && forward[0] == 0)
	{
		if (forward[2] > 0)
		{
			pitch = -M_PI * 0.5;
			yaw = up ? atan2(-up[1], -up[0]) : 0;
		}
		else
		{
			pitch = M_PI * 0.5;
			yaw = up ? atan2(up[1], up[0]) : 0;
		}
		roll = 0;
	}
	else
	{
		yaw = atan2(forward[1], forward[0]);
		pitch = -atan2(forward[2], sqrt(forward[0] * forward[0] + forward[1] * forward[1]));

		if (up)
		{
			vec_t cp = cos(pitch), sp = sin(pitch);
			vec_t cy = cos(yaw), sy = sin(yaw);
			vec3_t tleft, tup;
			tleft[0] = -sy;
			tleft[1] = cy;
			tleft[2] = 0;
			tup[0] = sp * cy;
			tup[1] = sp * sy;
			tup[2] = cp;
			roll = -atan2(DotProduct(up, tleft), DotProduct(up, tup));
		}
		else
			roll = 0;
	}

	pitch *= -180 / M_PI;
	yaw *= 180 / M_PI;
	roll *= 180 / M_PI;
	if (pitch < 0)
		pitch += 360;
	if (yaw < 0)
		yaw += 360;
	if (roll < 0)
		roll += 360;

	result[0] = pitch;
	result[1] = yaw;
	result[2] = roll;
}
// https://github.com/mdeguzis/ftequake

void AnglesFromDirAndUp(vec3_t direction, vec3_t up, vec3_t angles)
{
	VectorAngles(direction, up, angles);
}

void VectoAngles(vec3_t vec, vec3_t angles) {
	vec3_t v3;

	if (*vec != 0.0 || vec[1] != 0.0 || vec[2] != 0.0)
	{
		VectorNormalize2(vec, v3);
		AnglesFromDir(v3, angles);
		angles[0] = angles[0] * RAD_TO_ANGLE;
		angles[1] = angles[1] * RAD_TO_ANGLE;
		angles[2] = angles[2] * RAD_TO_ANGLE;
	}
	else
	{
		angles[0] = 0;
		angles[1] = 0;
		angles[2] = 0;
	}
}

void DirFromAngles(vec3_t angles, vec3_t direction)
{
	float v4;
	float v5;
	float v6;
	float v7;
	float v8;

	v4 = angles[1];
	v7 = cos(v4);
	direction[0] = v7;
	v8 = sin(v4);
	direction[1] = v8;
	v5 = angles[0];
	v6 = cos(v5);
	direction[1] = v8 * v6;
	direction[2] = sin(v5);
}

void VectorAverage(vec3_t veca, vec3_t vecb, vec3_t vecc)
{
	vecc[0] = (veca[0] + vecb[0]) * 0.5;
	vecc[1] = (veca[1] + vecb[1]) * 0.5;
	vecc[2] = (veca[2] + vecb[2]) * 0.5;
}

void VectorGetOffsetOrigin(vec3_t off, vec3_t org, vec_t degree, vec3_t out)
{
	float v4;
	matrix3_t v7;

	v4 = degree * ANGLE_1;
	CreateYawMatrix(v7, v4);

	Matrix3MultByVec3(v7, off, out);
	out[0] = org[0] + out[0];
	out[1] = org[1] + out[1];
	out[2] = org[2] + out[2];
}

vec_t VectorSeparation(vec3_t ina, vec3_t inb)
{
	int i;
	double length;

	length = 0;

	for (i = 0; i < 3; i++)
	{
		length += ina[i] * inb[i];
	}

	return sqrt(length);
}

void VectorRandomCopy(vec3_t a1, vec3_t a2, float a3)
{
	float v3; // ST28_4@1

	v3 = -a3;
	a2[0] = flrand(v3, a3) + a1[0];
	a2[1] = flrand(v3, a3) + a1[1];
	a2[2] = flrand(v3, a3) + a1[2];
}

void VectorRandomAdd(vec3_t a1, vec3_t a2, vec3_t a3)
{
	a3[0] = flrand(-a2[0], a2[0]) + a1[0];
	a3[1] = flrand(-a2[1], a2[1]) + a1[1];
	a3[2] = flrand(-a2[2], a2[2]) + a1[2];
}

float Vector2Length(vec3_t p1, vec3_t p2)
{
	float x, y;

	x = p1[0] - p2[0];
	y = p1[1] - p2[1];
	return sqrt(x * x + y * y);
}

void VectorDegreesToRadians (vec3_t in, vec3_t out)
{
	//assert(out != vec3_origin);

	out[0] = in[0] * ANGLE_TO_RAD;
	out[1] = in[1] * ANGLE_TO_RAD;
	out[2] = in[2] * ANGLE_TO_RAD;
}

void Vec3SubtractAssign(vec3_t value, vec3_t subFrom)
{
	//assert(subFrom != vec3_origin);

	subFrom[0] -= value[0];
	subFrom[1] -= value[1];
	subFrom[2] -= value[2];
}

void Vec3AddAssign(vec3_t value, vec3_t addTo)
{
	//assert(addTo != vec3_origin);

	addTo[0] += value[0];
	addTo[1] += value[1];
	addTo[2] += value[2];
}

void Vec3ScaleAssign(vec_t value, vec3_t scaleBy)
{
	//assert(scaleBy != vec3_origin);

	scaleBy[0] *= value;
	scaleBy[1] *= value;
	scaleBy[2] *= value;
}

qboolean Vec3IsZero(vec3_t vec)
{
	return !( vec[0] != 0.0 || vec[1] != 0.0 || vec[2] != 0.0 );
}

qboolean Vec3NotZero(vec3_t vec)
{
	return ( vec[0] != 0.0 || vec[1] != 0.0 || vec[2] != 0.0 );
}

float NormalizeAngle(float angle)
{
	// Returns the remainder
	angle = fmod(angle, ANGLE_360);

	// Makes the angle signed
	if (angle >= ANGLE_180)
	{
		angle -= ANGLE_360;
	}

	if (angle <= -ANGLE_180)
	{
		angle += ANGLE_360;
	}

	return angle;
}
