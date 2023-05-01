//
// Heretic II
// Copyright 1998 Raven Software
//
// H2Matrix.c
//

#include "../qcommon/resourcemanager.h"
#include "../qcommon/angles.h"
#include "../qcommon/matrix.h"
#include "../qcommon/vector.h"

int HACK_Pitch_Adjust = 0; // weak

void CreateIdentityMatrix(matrix4_t matrix)
{
	memset(matrix, 0, sizeof(matrix4_t));
	matrix[0][0] = 1.0f;
	matrix[1][1] = 1.0f;
	matrix[2][2] = 1.0f;
	matrix[3][3] = 1.0f;
}

void TranslateMatrix(matrix4_t matrix, vec3_t xyz)
{
	matrix[3][0] += xyz[0];
	matrix[3][1] += xyz[1];
	matrix[3][2] += xyz[2];
}

void CreatePitchMatrix(matrix3_t matrix, vec_t angles)
{
	memset(&matrix[0], 0, sizeof(matrix3_t));

	matrix[0][0] = cos(-angles);
	matrix[2][2] = matrix[0][0];
	matrix[1][1] = 1;
	matrix[2][0] = sin(-angles);
	matrix[0][2] = -matrix[2][0];
}

void CreateYawMatrix(matrix3_t matrix, vec_t angles)
{
	// Rotation about the z axis
	memset(&matrix[0], 0, sizeof(matrix3_t));

	matrix[0][0] = cos(angles);
	matrix[1][1] = matrix[0][0];
	matrix[0][1] = sin(angles);
	matrix[1][0] = -matrix[0][1];
	matrix[2][2] = 1;
}

void CreateRollMatrix(matrix3_t matrix, vec_t angles)
{
	memset(&matrix[0], 0, sizeof(matrix3_t));

	// Rotation about the x axis
	matrix[0][0] = 1;
	matrix[1][1] = cos(angles);
	matrix[2][2] = matrix[1][1];
	matrix[1][2] = sin(angles);
	matrix[2][1] = -matrix[1][2];
}


void Matrix3MultByMatrix3(matrix3_t A, matrix3_t B, matrix3_t C)
{
	C[0][0] = B[0][0] * A[0][0] + B[0][1] * A[1][0] +
		B[0][2] * A[2][0];
	C[0][1] = B[0][0] * A[0][1] + B[0][1] * A[1][1] +
		B[0][2] * A[2][1];
	C[0][2] = B[0][0] * A[0][2] + B[0][1] * A[1][2] +
		B[0][2] * A[2][2];
	C[1][0] = B[1][0] * A[0][0] + B[1][1] * A[1][0] +
		B[1][2] * A[2][0];
	C[1][1] = B[1][0] * A[0][1] + B[1][1] * A[1][1] +
		B[1][2] * A[2][1];
	C[1][2] = B[1][0] * A[0][2] + B[1][1] * A[1][2] +
		B[1][2] * A[2][2];
	C[2][0] = B[2][0] * A[0][0] + B[2][1] * A[1][0] +
		B[2][2] * A[2][0];
	C[2][1] = B[2][0] * A[0][1] + B[2][1] * A[1][1] +
		B[2][2] * A[2][1];
	C[2][2] = B[2][0] * A[0][2] + B[2][1] * A[1][2] +
		B[2][2] * A[2][2];
}


static void Matrix3MultByMartrix3(matrix3_t A, matrix3_t B, matrix3_t C)
{
	C[0][0] = B[0][0] * A[0][0] + B[0][1] * A[1][0] +
		B[0][2] * A[2][0];
	C[0][1] = B[0][0] * A[0][1] + B[0][1] * A[1][1] +
		B[0][2] * A[2][1];
	C[0][2] = B[0][0] * A[0][2] + B[0][1] * A[1][2] +
		B[0][2] * A[2][2];
	C[1][0] = B[1][0] * A[0][0] + B[1][1] * A[1][0] +
		B[1][2] * A[2][0];
	C[1][1] = B[1][0] * A[0][1] + B[1][1] * A[1][1] +
		B[1][2] * A[2][1];
	C[1][2] = B[1][0] * A[0][2] + B[1][1] * A[1][2] +
		B[1][2] * A[2][2];
	C[2][0] = B[2][0] * A[0][0] + B[2][1] * A[1][0] +
		B[2][2] * A[2][0];
	C[2][1] = B[2][0] * A[0][1] + B[2][1] * A[1][1] +
		B[2][2] * A[2][1];
	C[2][2] = B[2][0] * A[0][2] + B[2][1] * A[1][2] +
		B[2][2] * A[2][2];
 }

void Matrix3FromAngles(vec3_t angles, matrix3_t rotationMatrix)
{
	matrix3_t yawMatrix;
	matrix3_t pitchMatrix;
	matrix3_t rollMatrix;
	matrix3_t pitchRollMatrix;

#if 1
	memset(rollMatrix, 0, sizeof(rollMatrix));

	// Rotation about the x axis
	rollMatrix[0][0] = 1;
	rollMatrix[1][1] = cos(angles[PITCH]);
	rollMatrix[2][2] = rollMatrix[1][1];
	rollMatrix[1][2] = sin(angles[PITCH]);
	rollMatrix[2][1] = -rollMatrix[1][2];

	// Rotation about the y axis
	memset(pitchMatrix, 0, sizeof(pitchMatrix));

	pitchMatrix[0][0] = cos(angles[YAW]);
	pitchMatrix[2][2] = pitchMatrix[0][0];
	pitchMatrix[1][1] = 1;
	pitchMatrix[2][0] = sin(angles[YAW]);
	pitchMatrix[0][2] = -pitchMatrix[2][0];

	// Rotation about the z axis
	memset(yawMatrix, 0, sizeof(yawMatrix));

	yawMatrix[0][0] = cos(angles[ROLL]);
	yawMatrix[1][1] = yawMatrix[0][0];
	yawMatrix[0][1] = sin(angles[ROLL]);
	yawMatrix[1][0] = -yawMatrix[0][1];
	yawMatrix[2][2] = 1;

	Matrix3MultByMartrix3(pitchMatrix, rollMatrix, pitchRollMatrix);

	Matrix3MultByMartrix3(yawMatrix, pitchRollMatrix, rotationMatrix);
#else
	// Rotation about the y axis
	memset(pitchMatrix, 0, sizeof(pitchMatrix));

	pitchMatrix[0][0] = cos(angles[YAW]);
	pitchMatrix[2][2] = pitchMatrix[0][0];
	pitchMatrix[1][1] = 1;
	pitchMatrix[2][0] = sin(angles[YAW]);
	pitchMatrix[0][2] = -pitchMatrix[2][0];

	memset(rollMatrix, 0, sizeof(rollMatrix));

	// Rotation about the x axis
	rollMatrix[0][0] = 1;
	rollMatrix[1][1] = cos(angles[PITCH]);
	rollMatrix[2][2] = rollMatrix[1][1];
	rollMatrix[1][2] = sin(angles[PITCH]);
	rollMatrix[2][1] = -rollMatrix[1][2];

	// Rotation about the z axis
	memset(yawMatrix, 0, sizeof(yawMatrix));

	yawMatrix[0][0] = cos(angles[ROLL]);
	yawMatrix[1][1] = yawMatrix[0][0];
	yawMatrix[0][1] = sin(angles[ROLL]);
	yawMatrix[1][0] = -yawMatrix[0][1];
	yawMatrix[2][2] = 1;

	Matrix3MultByMartrix3(rollMatrix, pitchMatrix, pitchRollMatrix);

	Matrix3MultByMartrix3(yawMatrix, pitchRollMatrix, rotationMatrix);
#endif
}

void IMatrix3FromAngles(vec3_t angles, matrix3_t rotationMatrix)
{
	matrix3_t yawMatrix;
	matrix3_t pitchMatrix;
	matrix3_t rollMatrix;
	matrix3_t pitchRollMatrix;

#if 1
	memset(rollMatrix, 0, sizeof(rollMatrix));

	// Rotation about the x axis
	rollMatrix[0][0] = 1;
	rollMatrix[1][1] = cos(angles[PITCH]);
	rollMatrix[2][2] = rollMatrix[1][1];
	rollMatrix[1][2] = sin(angles[PITCH]);
	rollMatrix[2][1] = -rollMatrix[1][2];

	// Rotation about the y axis
	memset(pitchMatrix, 0, sizeof(pitchMatrix));

	pitchMatrix[0][0] = cos(angles[YAW]);
	pitchMatrix[2][2] = pitchMatrix[0][0];
	pitchMatrix[1][1] = 1;
	pitchMatrix[2][0] = sin(angles[YAW]);
	pitchMatrix[0][2] = -pitchMatrix[2][0];

	// Rotation about the z axis
	memset(yawMatrix, 0, sizeof(yawMatrix));

	yawMatrix[0][0] = cos(angles[ROLL]);
	yawMatrix[1][1] = yawMatrix[0][0];
	yawMatrix[0][1] = sin(angles[ROLL]);
	yawMatrix[1][0] = -yawMatrix[0][1];
	yawMatrix[2][2] = 1;

	Matrix3MultByMatrix3(pitchMatrix, rollMatrix, pitchRollMatrix);

	Matrix3MultByMatrix3(yawMatrix, pitchRollMatrix, rotationMatrix);
#else
	// Rotation about the y axis
	memset(pitchMatrix, 0, sizeof(pitchMatrix));

	pitchMatrix[0][0] = cos(angles[YAW]);
	pitchMatrix[2][2] = pitchMatrix[0][0];
	pitchMatrix[1][1] = 1;
	pitchMatrix[2][0] = sin(angles[YAW]);
	pitchMatrix[0][2] = -pitchMatrix[2][0];

	memset(rollMatrix, 0, sizeof(rollMatrix));

	// Rotation about the x axis
	rollMatrix[0][0] = 1;
	rollMatrix[1][1] = cos(angles[PITCH]);
	rollMatrix[2][2] = rollMatrix[1][1];
	rollMatrix[1][2] = sin(angles[PITCH]);
	rollMatrix[2][1] = -rollMatrix[1][2];

	// Rotation about the z axis
	memset(yawMatrix, 0, sizeof(yawMatrix));

	yawMatrix[0][0] = cos(angles[ROLL]);
	yawMatrix[1][1] = yawMatrix[0][0];
	yawMatrix[0][1] = sin(angles[ROLL]);
	yawMatrix[1][0] = -yawMatrix[0][1];
	yawMatrix[2][2] = 1;

	Matrix3MultByMartrix3(rollMatrix, pitchMatrix, pitchRollMatrix);

	Matrix3MultByMartrix3(yawMatrix, pitchRollMatrix, rotationMatrix);
#endif
}

void Matrix3MultByVec3(matrix3_t A, vec3_t B, vec3_t C)
{
	C[0] = B[0] * A[0][0] + B[1] * A[1][0] +
		B[2] * A[2][0];
	C[1] = B[0] * A[0][1] + B[1] * A[1][1] +
		B[2] * A[2][1];
	C[2] = B[0] * A[0][2] + B[1] * A[1][2] +
		B[2] * A[2][2];
}

float Vec3DotProduct(vec3_t v1, vec3_t v2)
{
	return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

float Vec3Normalize(vec3_t v1)
{
	float mag;
	float imag = 1;

	mag = Vec3DotProduct(v1, v1);

	if (!mag)
	{
		return 0;
	}

	mag = (float)sqrt(mag);

	imag /= mag;

	v1[0] *= imag;
	v1[1] *= imag;
	v1[2] *= imag;

	return mag;
}

double Vec3dDotProduct(vec3d_t v1, vec3d_t v2)
{
	return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

double Vec3dNormalize(vec3d_t v1)
{
	double mag;
	double imag = 1;

	mag = Vec3dDotProduct(v1, v1);

	if (!mag)
	{
		return 0;
	}

	mag = (float)sqrt(mag);

	imag /= mag;

	v1[0] *= imag;
	v1[1] *= imag;
	v1[2] *= imag;

	return mag;
}

double Matricies3FromDirAndUp(vec3_t direction, vec3_t up, matrix3_t toLocal, matrix3_t fromLocal)
{
	matrix3_t pitchYawMatrix;
	matrix3_t rollMatrix;
	float roll;
	vec3_t rotatedUp, up_d;
	vec3_t dir_d;

#ifndef NDEBUG
	float dot1, dot2;

#endif

	dir_d[0] = direction[0];
	dir_d[1] = direction[1];
	dir_d[2] = direction[2];

	up_d[0] = up[0];
	up_d[1] = up[1];
	up_d[2] = up[2];

#ifndef NDEBUG
	dot1 = Vec3DotProduct(up_d, dir_d);
#endif

#if 0  // not sure what's wrong with this

	// need to check for direction == {x, 0, 0}
	// ot maybe not.  Looks like the math will work out to be the I or -I,
	// which seems right;  haven't actually seen any problems with it yet. . .

	r = 1 - dir_d[0];
	s = sin(-acos(dir_d[0]));

	// matrix from dir_d vector
	pitchYawMatrix[0][0] = dir_d[0];
	pitchYawMatrix[1][0] = -dir_d[1] * s;
	pitchYawMatrix[2][0] = -dir_d[2] * s;
	pitchYawMatrix[0][1] = -pitchYawMatrix[1][0];
	pitchYawMatrix[1][1] = dir_d[2] * dir_d[2] * r + dir_d[0];
	pitchYawMatrix[2][1] = -dir_d[1] * dir_d[2] * r;
	pitchYawMatrix[0][2] = dir_d[2] * s;
	pitchYawMatrix[1][2] = pitchYawMatrix[2][1];
	pitchYawMatrix[2][2] = dir_d[1] * dir_d[1] * r + dir_d[0];
#else
	{
		float yaw, pitch;
		matrix3_t pitchMatrix, yawMatrix;

		yaw = atan2(dir_d[1], dir_d[0]);
		pitch = asin(dir_d[2]);

		// Rotation about the y axis
		memset(pitchMatrix, 0, sizeof(pitchMatrix));

		pitchMatrix[0][0] = cos(pitch);
		pitchMatrix[2][2] = pitchMatrix[0][0];
		pitchMatrix[1][1] = 1;
		pitchMatrix[2][0] = sin(pitch);
		pitchMatrix[0][2] = -pitchMatrix[2][0];

		// Rotation about the z axis
		memset(yawMatrix, 0, sizeof(yawMatrix));

		yawMatrix[0][0] = cos(yaw);
		yawMatrix[1][1] = yawMatrix[0][0];
		yawMatrix[0][1] = sin(yaw);
		yawMatrix[1][0] = -yawMatrix[0][1];
		yawMatrix[2][2] = 1;

		Matrix3MultByMartrix3(pitchMatrix, yawMatrix, pitchYawMatrix);

	}
#endif

	Matrix3MultByVec3(pitchYawMatrix, up_d, rotatedUp);

#ifndef NDEBUG
	{
		vec3_t xaxis = { 1, 0, 0 };
		float temp;

#if 0
		vec3_t rotatedDir;
		float dot3, dot4;

		Matrix3MultByVec3(pitchYawMatrix, dir_d, rotatedDir);

		dot3 = Vec3DotProduct(rotatedDir, xaxis);

		dot3 = acos(dot3);
		dot3 *= RAD_TO_ANGLE;

		dot4 = Vec3DotProduct(dir_d, xaxis);

		dot4 = acos(dot4);
		dot4 *= RAD_TO_ANGLE;
#endif

		dot2 = Vec3DotProduct(rotatedUp, xaxis);

		dot1 = acos(dot1);
		dot2 = acos(dot2);
		temp = (dot2 - dot1) * RAD_TO_ANGLE;

#if 0
		assert(fabs(temp) / 90.0 <= 0.1);	// greater than 10% error created by rotation of
		// up vector into world coordiantes
#else
		temp = fabs(temp);	// 0.1 for pelf, 0.024 for beetle
#endif
		dot1 *= RAD_TO_ANGLE;
		dot2 *= RAD_TO_ANGLE;
	}
#endif

	rotatedUp[0] = 0;

	Vec3Normalize(rotatedUp);

	roll = -(atan2(rotatedUp[2], rotatedUp[1]) - ANGLE_90);

	memset(rollMatrix, 0, sizeof(rollMatrix));

	// Rotation about the local x axis
	rollMatrix[0][0] = 1;
	rollMatrix[1][1] = cos(roll);
	rollMatrix[2][2] = rollMatrix[1][1];
	rollMatrix[1][2] = sin(roll);
	rollMatrix[2][1] = -rollMatrix[1][2];

	if (fromLocal)
	{
		Matrix3MultByMartrix3(rollMatrix, pitchYawMatrix, fromLocal);
	}

	// FIXME!!!!!!!
	// this is wrong, roll needs to rotate about the new local axis
	// I think the pitch yaw matix is still correct though
	if (toLocal)
	{
		roll *= -1;

#if 0

		memset(rollMatrix, 0, sizeof(rollMatrix));

#if 1
		// Rotation about the x axis
		rollMatrix[0][0] = 1;
		rollMatrix[1][1] = cos(roll);
		rollMatrix[2][2] = rollMatrix[1][1];
		rollMatrix[1][2] = sin(roll);
		rollMatrix[2][1] = -rollMatrix[1][2];
#else
		// Rotation about the local x axis
		rollMatrix[0][0] = 1;
		rollMatrix[1][1] = 1;
		rollMatrix[2][2] = 1;
#endif

#endif

#if 0	// not sure what's wrong with this!
		r = 1 - direction[0];
		s = sin(acos(direction[0]));

		// matrix from direction vector
		pitchYawMatrix[0][0] = direction[0];
		pitchYawMatrix[1][0] = -direction[1] * s;
		pitchYawMatrix[2][0] = -direction[2] * s;
		pitchYawMatrix[0][1] = -pitchYawMatrix[1][0];
		pitchYawMatrix[1][1] = direction[2] * direction[2] * r + direction[0];
		pitchYawMatrix[2][1] = -direction[1] * direction[2] * r;
		pitchYawMatrix[0][2] = direction[2] * s;
		pitchYawMatrix[1][2] = pitchYawMatrix[2][1];
		pitchYawMatrix[2][2] = direction[1] * direction[1] * r + direction[0];
#else
		{
			float yaw, pitch;
			matrix3_t pitchMatrix, yawMatrix;

			yaw = -atan2(dir_d[1], dir_d[0]);
			pitch = -asin(dir_d[2]);

			// Rotation about the y axis
			memset(pitchMatrix, 0, sizeof(pitchMatrix));

			pitchMatrix[0][0] = cos(pitch);
			pitchMatrix[2][2] = pitchMatrix[0][0];
			pitchMatrix[1][1] = 1;
			pitchMatrix[2][0] = sin(pitch);
			pitchMatrix[0][2] = -pitchMatrix[2][0];

			// Rotation about the z axis
			memset(yawMatrix, 0, sizeof(yawMatrix));

			yawMatrix[0][0] = cos(yaw);
			yawMatrix[1][1] = yawMatrix[0][0];
			yawMatrix[0][1] = sin(yaw);
			yawMatrix[1][0] = -yawMatrix[0][1];
			yawMatrix[2][2] = 1;

			//Matrix3dMultByMartrix3d(yawMatrix, pitchMatrix, pitchYawMatrix);
			Matrix3MultByMartrix3(yawMatrix, pitchMatrix, toLocal);
		}
#endif

		//Matrix3dMultByMartrix3d(pitchYawMatrix, rollMatrix, toLocal);
	}

	return roll;
}

void RotatePointAboutLocalOrigin(matrix3_t rotation, vec3_t origin, vec3_t point)
{
	vec3_t temp;

	point[0] -= origin[0];
	point[1] -= origin[1];
	point[2] -= origin[2];

	Matrix3MultByVec3(rotation, point, temp);

	point[0] = temp[0] + origin[0];
	point[1] = temp[1] + origin[1];
	point[2] = temp[2] + origin[2];
}

void TransformPoint(matrix3_t rotation, vec3_t origin, vec3_t newOrigin, vec3_t point)
{
	vec3_t vec;

	point[0] = point[0] - origin[0];
	point[1] = point[1] - origin[1];
	point[2] = point[2] - origin[2];
	Matrix3MultByVec3(rotation, point, vec);
	point[0] = vec[0] + newOrigin[0];
	point[1] = vec[1] + newOrigin[1];
	point[2] = vec[2] + newOrigin[2];
}
