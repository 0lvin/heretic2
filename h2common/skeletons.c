//
// Skeletons.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include "skeletons.h"
#include "arrayed_list.h"

int numJointsInSkeleton[] =
{
	NUM_JOINTS_RAVEN,
	NUM_JOINTS_BOX,
	NUM_JOINTS_BEETLE,
	NUM_JOINTS_ELFLORD,
	NUM_JOINTS_PLAGUE_ELF,
	NUM_JOINTS_CORVUS,
};

static void CreateRavenSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int root);
static void CreateBoxSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int root);
static void CreateBeetleSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex);
static void CreateElfLordSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex);
static void CreatePlagueElfSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex);

CreateSkeleton_t SkeletonCreators[NUM_SKELETONS] =
{
	CreateRavenSkel,
	CreateBoxSkel,
	CreateBeetleSkel,
	CreateElfLordSkel,
	CreatePlagueElfSkel,
	CreatePlagueElfSkel,	// Corvus has the same structure as the Plague Elf
};

static void
CreateRavenSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex)
{
	char *root;
	int *children;
	int nodeIndex;

	root = (char *)skeletalJoints + rootIndex * jointSize;

	children = (int *)(root + RAVEN_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + RAVEN_UPPERBACK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + RAVEN_HEAD;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + RAVEN_LOWERBACK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + RAVEN_UPPERBACK;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

static void
CreateBoxSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex)
{
	char *root;
	int *children;

	root = (char *)skeletalJoints + rootIndex * jointSize;

	children = (int *)(root + RAVEN_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;
}

static void
CreateBeetleSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex)
{
	char *root;
	int *children;
	int nodeIndex;

	root = (char *)skeletalJoints + rootIndex * jointSize;

	children = (int *)(root + BEETLE_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + BEETLE_NECK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + BEETLE_HEAD;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

static void
CreateElfLordSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex)
{
	char *root;
	int *children;
	int nodeIndex;

	root = (char *)skeletalJoints + rootIndex * jointSize;

	children = (int *)(root + BEETLE_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + BEETLE_NECK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + BEETLE_HEAD;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

static void
CreatePlagueElfSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex)
{
	char *root;
	int *children;
	int nodeIndex;

	root = (char *)skeletalJoints + rootIndex * jointSize;

	children = (int *)(root + PLAGUE_ELF_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + PLAGUE_ELF_UPPERBACK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + PLAGUE_ELF_HEAD;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + PLAGUE_ELF_LOWERBACK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + PLAGUE_ELF_UPPERBACK;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}
