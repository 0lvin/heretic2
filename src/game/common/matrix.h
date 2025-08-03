//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef MATRIX_H
#define MATRIX_H
#include "../../common/header/shared.h"

typedef float matrix3_t[3][3];

void CreateYawMatrix(matrix3_t, vec_t);
void Matrix3MultByVec3(matrix3_t A, vec3_t B, vec3_t C);
void Matrix3FromAngles(vec3_t angles, matrix3_t rotationMatrix);

#endif
