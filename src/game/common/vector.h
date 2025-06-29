//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef VECTOR_H
#define VECTOR_H

extern vec3_t vec3_up;

void DirFromAngles(vec3_t angles, vec3_t direction);
void AnglesFromDir(vec3_t direction, vec3_t angles);
void AnglesFromDirI(vec3_t direction, vec3_t angles);
void VectoAngles(vec3_t in, vec3_t out);
void AnglesFromDirAndUp(vec3_t direction, vec3_t up, vec3_t angles);
void VectorAverage(vec3_t veca, vec3_t vecb, vec3_t vecc);
void VectorGetOffsetOrigin(vec3_t off, vec3_t org, vec_t degree, vec3_t out);
vec_t VectorSeparation(vec3_t ina, vec3_t inb);
void VectorRandomCopy(vec3_t, vec3_t, float);
void VectorRandomAdd(vec3_t veca, vec3_t vecb, vec3_t out);
float Vector2Length(vec3_t p1, vec3_t p2);
void Create_rand_relect_vect(vec3_t in, vec3_t out);
void VectorDegreesToRadians(vec3_t in, vec3_t out);
void Vec3SubtractAssign(vec3_t value, vec3_t subFrom);
void Vec3AddAssign(vec3_t value, vec3_t addTo);
void Vec3ScaleAssign(vec_t value, vec3_t scaleBy);
qboolean Vec3IsZero(vec3_t vec);
qboolean Vec3NotZero(vec3_t vec);
float NormalizeAngle(float angle);

#endif
