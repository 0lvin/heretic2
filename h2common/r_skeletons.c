//
// r_Skeletons.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include "../src/common/header/shared.h"
#include "vector.h"
#include "skeletons.h"
#include "m_skeletalcluster.h"
#include "m_skeleton.h"
#include "arrayed_list.h"

#include <memory.h>

M_SkeletalCluster_t SkeletalClusters[MAX_ARRAYED_SKELETAL_JOINTS];
ArrayedListNode_t ClusterNodes[MAX_ARRAYED_JOINT_NODES];

extern void *Hunk_Alloc (int size);

static void
Skeleton_LerpVert(vec3_t newPoint, vec3_t oldPoint, vec3_t interpolatedPoint, float move[3], float frontv[3], float backv[3])
{
	interpolatedPoint[0] = move[0] + oldPoint[0] * backv[0] + newPoint[0] * frontv[0];
	interpolatedPoint[1] = move[1] + oldPoint[1] * backv[1] + newPoint[1] * frontv[1];
	interpolatedPoint[2] = move[2] + oldPoint[2] * backv[2] + newPoint[2] * frontv[2];
}

void CreateSkeletonAsHunk(int structure, ModelSkeleton_t *skel)
{
	skel->rootJoint = (M_SkeletalJoint_t *) Hunk_Alloc(numJointsInSkeleton[structure]*sizeof(M_SkeletalJoint_t));
	skel->rootNode = (ArrayedListNode_t *) Hunk_Alloc(numNodesInSkeleton[structure]*sizeof(ArrayedListNode_t));

	SkeletonCreators[structure](skel->rootJoint, sizeof(M_SkeletalJoint_t), skel->rootNode, 0);
}

void CreateSkeletonInPlace(int structure, ModelSkeleton_t *skel)
{
	SkeletonCreators[structure](skel->rootJoint, sizeof(M_SkeletalJoint_t), skel->rootNode, 0);
}

static int GetRootIndex(int max, int numJoints)
{
	int i, j, max2;
	qboolean cont = false;

	for(i = 0; i < max; ++i)
	{
		if(!SkeletalClusters[i].inUse)
		{
			max2 = i + numJoints;

			// check the size of the array
			if(max2 > max)
			{
				assert(0);
				return -1;
			}

			// check for a big enough unused block
			for(j = i + 1; j < max2; ++j)
			{
				if(SkeletalClusters[j].inUse)
				{
					i = j;
					cont = true;
					break;
				}
			}

			if(cont) // not a big enough block, so continue searching
			{
				cont = false;
				continue;
			}

			// found a block, mark it as used
			for(j = i; j < max2; ++j)
			{
				SkeletalClusters[j].inUse = true;
			}

			return i;
		}
	}

	// couldn't find a block
	assert(0);
	return -1;
}


int CreateSkeleton(int structure)
{
	int index;

	index = GetRootIndex(MAX_ARRAYED_SKELETAL_JOINTS, numJointsInSkeleton[structure]);

	SkeletonCreators[structure](SkeletalClusters, sizeof(M_SkeletalCluster_t), ClusterNodes, index);

	return index;
}

void ClearSkeleton(ModelSkeleton_t *skel, int root)
{
	int child;

	for(child = skel->rootJoint[root].children; child != ARRAYEDLISTNODE_NULL; child = skel->rootNode[child].next)
	{
		ClearSkeleton(skel, skel->rootNode[child].data);

		FreeNode(skel->rootNode, child);
	}

	skel->rootJoint[root].inUse = false;
}

static void RotateModelSegment(M_SkeletalJoint_t *joint, vec3_t *modelVerticies, vec3_t angles, M_SkeletalCluster_t *modelCluster)
{
	int i;
	matrix3_t rotation, rotation2, toWorld, partialBackToLocal;
	vec3_t localAngles;
	float roll, orig_roll;

//	Com_Printf("direction: %f, %f, %f\n", joint->model.direction[0], joint->model.direction[1], joint->model.direction[2]);

	localAngles[0] = angles[0];
	localAngles[1] = angles[1];
	orig_roll = localAngles[2] = angles[2];

	memset(rotation, 0, sizeof(rotation));

//	localAngles[ROLL] += Matricies3FromDirAndUp(joint->model.direction, joint->model.up, toWorld, partialBackToLocal);
	localAngles[ROLL] += roll = Matricies3FromDirAndUp(joint->model.direction, joint->model.up, toWorld, partialBackToLocal);

	Matrix3FromAngles(localAngles, rotation);

	Matrix3MultByMatrix3(rotation, toWorld, rotation2);

//	Com_Printf("rotation matrix: %f, %f, %f\n", rotation2[0][0], rotation2[1][1], rotation2
//		[2][2]);

	Matrix3MultByMatrix3(partialBackToLocal, rotation2, rotation);

	for(i = 0; i < modelCluster->numVerticies; ++i)
	{
		RotatePointAboutLocalOrigin(rotation, joint->model.origin, modelVerticies[modelCluster->verticies[i]]);
	}
}

static void TransformPlacement(matrix3_t rotation, vec3_t origin, Placement_t *placement)
{
	RotatePointAboutLocalOrigin(rotation, origin, placement->origin);
	RotatePointAboutLocalOrigin(rotation, origin, placement->direction);
	RotatePointAboutLocalOrigin(rotation, origin, placement->up);
}

static void RotateDecendents(ModelSkeleton_t *skel, M_SkeletalJoint_t *joint, M_SkeletalJoint_t *ancestor)
{
	int jointChild;

	for(jointChild = joint->children; jointChild != ARRAYEDLISTNODE_NULL;
		jointChild = skel->rootNode[jointChild].next)
	{
		joint = skel->rootJoint + skel->rootNode[jointChild].data;

		TransformPlacement(ancestor->rotation, ancestor->parent.origin, &joint->parent);

		RotateDecendents(skel, joint, ancestor);
	}
}

static void RotateJoint(ModelSkeleton_t *skel, M_SkeletalJoint_t *joint, vec3_t angles)
{
	matrix3_t rotation, rotation2, toWorld, partialBackToLocal;
	vec3_t localAngles;

	VectorCopy(angles, localAngles);

	memset(rotation, 0, sizeof(rotation));

	localAngles[ROLL] += Matricies3FromDirAndUp(joint->model.direction, joint->model.up, toWorld, partialBackToLocal);

	Matrix3FromAngles(localAngles, rotation);

	Matrix3MultByMatrix3(rotation, toWorld, rotation2);

	Matrix3MultByMatrix3(partialBackToLocal, rotation2, joint->rotation);

	VectorCopy(joint->model.origin, joint->parent.origin);

	Matrix3MultByVec3(joint->rotation, joint->model.direction, joint->parent.direction);
	Vec3ScaleAssign(10.0, joint->parent.direction);
	Vec3AddAssign(joint->parent.origin, joint->parent.direction);

	Matrix3MultByVec3(joint->rotation, joint->model.up, joint->parent.up);
	Vec3ScaleAssign(10.0, joint->parent.up);
	Vec3AddAssign(joint->parent.origin, joint->parent.up);

	RotateDecendents(skel, joint, joint);
}

void FinishJointRotations(ModelSkeleton_t *skel, int jointIndex)
{
	int jointChild;
	M_SkeletalJoint_t *joint;
	matrix3_t rotation, rotation2, toWorld, partialBackToLocal;
	vec3_t localAngles;

	joint = skel->rootJoint + jointIndex;

	for(jointChild = joint->children; jointChild != ARRAYEDLISTNODE_NULL;
		jointChild = skel->rootNode[jointChild].next)
	{
		FinishJointRotations(skel, skel->rootNode[jointChild].data);
	}

	localAngles[YAW] = 0;
	localAngles[PITCH] = 0;

	memset(rotation, 0, sizeof(rotation));

	Vec3SubtractAssign(joint->parent.origin, joint->parent.direction);
	Vec3SubtractAssign(joint->parent.origin, joint->parent.up);

	VectorNormalize(joint->parent.direction);
	VectorNormalize(joint->parent.up);

	localAngles[ROLL] = Matricies3FromDirAndUp(joint->parent.direction, joint->parent.up, toWorld, partialBackToLocal);

	Matricies3FromDirAndUp(joint->model.direction, joint->model.up, toWorld, NULL);

	Matrix3FromAngles(localAngles, rotation);

	Matrix3MultByMatrix3(rotation, toWorld, rotation2);

	Matrix3MultByMatrix3(partialBackToLocal, rotation2, joint->rotation);
}

void LinearllyInterpolateJoints(ModelSkeleton_t *newSkel, int newIndex,
	ModelSkeleton_t *oldSkel, int oldIndex, ModelSkeleton_t *liSkel, int liIndex,
	float move[3], float frontv[3], float backv[3])
{
	M_SkeletalJoint_t *newJoint, *oldJoint, *liJoint;
	int newChild, oldChild, liChild;

	newJoint = newSkel->rootJoint + newIndex;
	oldJoint = oldSkel->rootJoint + oldIndex;
	liJoint = liSkel->rootJoint + liIndex;

	if(newJoint->children != ARRAYEDLISTNODE_NULL)
	{
		assert(oldJoint->children != ARRAYEDLISTNODE_NULL);
		assert(liJoint->children != ARRAYEDLISTNODE_NULL);

		for(newChild = newJoint->children, oldChild = oldJoint->children,
			liChild = liJoint->children;

			newChild != ARRAYEDLISTNODE_NULL;

			newChild = newSkel->rootNode[newChild].next,
			oldChild = oldSkel->rootNode[oldChild].next,
			liChild = liSkel->rootNode[liChild].next)
		{
			assert(oldChild != ARRAYEDLISTNODE_NULL);
			assert(liChild != ARRAYEDLISTNODE_NULL);

			LinearllyInterpolateJoints(newSkel, newSkel->rootNode[newChild].data,
				oldSkel, oldSkel->rootNode[oldChild].data, liSkel,
				liSkel->rootNode[liChild].data, move, frontv, backv);
		}
	}

	Skeleton_LerpVert(newJoint->model.origin, oldJoint->model.origin, liJoint->model.origin, move, frontv, backv);

	// linerally interpolater direction and up vectors, which will unnormalize them relative to their local origin
	Skeleton_LerpVert(newJoint->model.direction, oldJoint->model.direction, liJoint->model.direction, move, frontv, backv);
	Skeleton_LerpVert(newJoint->model.up, oldJoint->model.up, liJoint->model.up, move, frontv, backv);

	Vec3SubtractAssign(liJoint->model.origin, liJoint->model.direction);
	Vec3SubtractAssign(liJoint->model.origin, liJoint->model.up);

	// renormalize them
	Vec3Normalize(liJoint->model.direction);
	Vec3Normalize(liJoint->model.up);
}

void SetupCompressedJoints(ModelSkeleton_t *liSkel, int liIndex,
	float *lerp)
{
	M_SkeletalJoint_t *liJoint;
	int liChild;

	liJoint = liSkel->rootJoint + liIndex;

	if(liJoint->children != ARRAYEDLISTNODE_NULL)
	{
		for(liChild = liJoint->children; liChild != ARRAYEDLISTNODE_NULL;
			liChild = liSkel->rootNode[liChild].next)
		{
			SetupCompressedJoints(liSkel, liSkel->rootNode[liChild].data, lerp + 9);
		}
	}

	VectorCopy(lerp, liJoint->model.origin);
	lerp+=3;
	VectorCopy(lerp, liJoint->model.direction);
	lerp+=3;
	VectorCopy(lerp, liJoint->model.up);

	Vec3SubtractAssign(liJoint->model.origin, liJoint->model.direction);
	Vec3SubtractAssign(liJoint->model.origin, liJoint->model.up);

	// renormalize them
	Vec3Normalize(liJoint->model.direction);
	Vec3Normalize(liJoint->model.up);
}
