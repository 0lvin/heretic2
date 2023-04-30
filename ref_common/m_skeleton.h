#ifndef M_SKELETON_H
#define M_SKELETON_H

#include "../src/common/header/common.h"
#include "../qcommon/placement.h"
#include "../qcommon/matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct M_SkeletalJoint_s
{
	int children;		// must be the first field
	Placement_t model;	// relative to the model, used for dynamic software rotation
	Placement_t parent;	// relative to the parent joint (or model in case of root joint), used for
						// inverse kinematics
	matrix3_t rotation;
	qboolean inUse;
} M_SkeletalJoint_t;

typedef struct ModelSkeleton_s
{
	M_SkeletalJoint_t *rootJoint;
	ArrayedListNode_t *rootNode;
} ModelSkeleton_t;

#ifdef __cplusplus
} //end extern "C"
#endif

#endif
