//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef VECTOR_H
#define VECTOR_H

void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );
void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
void PerpendicularVector( vec3_t dst, const vec3_t src );
void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void RealAngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);

void DirFromAngles(vec3_t angles, vec3_t direction);
void DirAndUpFromAngles(vec3_t angles, vec3_t direction, vec3_t up);
void AnglesFromDir(vec3_t direction, vec3_t angles);
void AnglesFromDirI(vec3_t direction, vec3_t angles);
void vectoangles(vec3_t in, vec3_t out);
void AnglesFromDirAndUp(vec3_t direction, vec3_t up, vec3_t angles);
int VectorCompare (vec3_t v1, vec3_t v2);
vec_t VectorNormalize (vec3_t v);		// returns vector length
float Vec3Normalize(vec3_t v1);
vec_t VectorNormalize2 (vec3_t v, vec3_t out);
void VectorAverage (vec3_t veca, vec3_t vecb, vec3_t vecc);
void VectorGetOffsetOrigin(vec3_t off, vec3_t org, vec_t degree, vec3_t out);
vec_t VectorSeparation(vec3_t, vec3_t);
void VectorRandomCopy(vec3_t, vec3_t, float);
void CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross);
vec_t VectorLengthSquared (vec3_t v);
void VectorRandomAdd (vec3_t veca, vec3_t vecb, vec3_t out);
float vhlen (vec3_t p1, vec3_t p2);
void Create_rand_relect_vect(vec3_t in, vec3_t out);
void VectorDegreesToRadians (vec3_t in, vec3_t out);
void VectorScaleByVector (vec3_t in, vec3_t scale, vec3_t out);
void Vec3SubtractAssign(vec3_t value, vec3_t subFrom);
void Vec3AddAssign(vec3_t value, vec3_t addTo);
void Vec3ScaleAssign(vec_t value, vec3_t scaleBy);
qboolean FloatIsZeroEpsilon(float f);
qboolean FloatIsZero(float f, float epsilon);
qboolean Vec3EqualsEpsilon(vec3_t v1, vec3_t v2);
qboolean Vec3IsZero(vec3_t vec);
qboolean Vec3NotZero(vec3_t vec);

#endif
