#pragma once

#include "../qcommon/Placement.h"
#include "../qcommon/Matrix.h"

typedef int qboolean;

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
	struct ArrayedListNode_s *rootNode;
} ModelSkeleton_t;

