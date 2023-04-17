#ifndef MATRIX_H
#define MATRIX_H
#include "qcommon.h"
#include "qcommon.h"

#ifdef __cplusplus
extern "C" {
#endif

QUAKE2_API void CreateRollMatrix(matrix3_t, vec_t);
QUAKE2_API void CreateYawMatrix(matrix3_t, vec_t);
QUAKE2_API void CreatePitchMatrix(matrix3_t, vec_t);

QUAKE2_API void Matrix3MultByMatrix3(matrix3_t A, matrix3_t B, matrix3_t C);
QUAKE2_API void Matrix3MultByVec3(matrix3_t A, vec3_t B, vec3_t C);
QUAKE2_API void Matrix3FromAngles(vec3_t angles, matrix3_t rotationMatrix);
QUAKE2_API void IMatrix3FromAngles(vec3_t angles, matrix3_t rotationMatrix);
QUAKE2_API double Matricies3FromDirAndUp(vec3_t direction, vec3_t up, matrix3_t toWorld, matrix3_t partialToLocal);
QUAKE2_API void RotatePointAboutLocalOrigin(matrix3_t rotation, vec3_t origin, vec3_t point);
QUAKE2_API void TransformPoint(matrix3_t rotation, vec3_t origin, vec3_t newOrigin, vec3_t point);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif