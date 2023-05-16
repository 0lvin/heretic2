//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef G_SKELETONS_H
#define G_SKELETONS_H

#include "../../common/header/common.h"
#include "../../../h2common/arrayed_list.h"
#include "../../../h2common/skeletons.h"

typedef struct G_SkeletalJoint_s
{
	int children;		// must be the first field
	float destAngles[3];
	float angVels[3];
	float angles[3];
	int changed[3];
	qboolean inUse;
} G_SkeletalJoint_t;

extern G_SkeletalJoint_t skeletalJoints[MAX_ARRAYED_SKELETAL_JOINTS];
extern ArrayedListNode_t jointNodes[MAX_ARRAYED_JOINT_NODES];

int CreateSkeleton(int structure);
void FreeSkeleton(int root);
float GetJointAngle(int jointIndex, int angleIndex);
qboolean SetJointAngVel(int jointIndex, int angleIndex, float destAngle, float angSpeed);

#endif
