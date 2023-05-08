//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef MATRIX_H
#define MATRIX_H
#include "../src/common/header/common.h"
#include "../src/common/header/common.h"

void CreateRollMatrix(matrix3_t, vec_t);
void CreateYawMatrix(matrix3_t, vec_t);
void CreatePitchMatrix(matrix3_t, vec_t);

void Matrix3MultByMatrix3(matrix3_t A, matrix3_t B, matrix3_t C);
void Matrix3MultByVec3(matrix3_t A, vec3_t B, vec3_t C);
void Matrix3FromAngles(vec3_t angles, matrix3_t rotationMatrix);
void IMatrix3FromAngles(vec3_t angles, matrix3_t rotationMatrix);
double Matricies3FromDirAndUp(vec3_t direction, vec3_t up, matrix3_t toWorld, matrix3_t partialToLocal);
void RotatePointAboutLocalOrigin(matrix3_t rotation, vec3_t origin, vec3_t point);
void TransformPoint(matrix3_t rotation, vec3_t origin, vec3_t newOrigin, vec3_t point);

#endif
