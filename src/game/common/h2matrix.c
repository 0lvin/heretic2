//
// Heretic II
// Copyright 1998 Raven Software
//
// H2Matrix.c
//

#include "matrix.h"

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

void Matrix3MultByVec3(matrix3_t A, vec3_t B, vec3_t C)
{
	C[0] = B[0] * A[0][0] + B[1] * A[1][0] +
		B[2] * A[2][0];
	C[1] = B[0] * A[0][1] + B[1] * A[1][1] +
		B[2] * A[2][1];
	C[2] = B[0] * A[0][2] + B[1] * A[1][2] +
		B[2] * A[2][2];
}
