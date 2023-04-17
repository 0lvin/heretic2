#ifndef R_SKELETONS_H
#define R_SKELETONS_H

#include "m_skeleton.h"
#include "m_skeletalcluster.h"
#include "../qcommon/skeletons.h"

#ifdef __cplusplus
extern "C" {
#endif

extern M_SkeletalCluster_t SkeletalClusters[MAX_ARRAYED_SKELETAL_JOINTS];
extern struct ArrayedListNode_s ClusterNodes[MAX_ARRAYED_JOINT_NODES];

int CreateSkeleton(int structure);
void CreateSkeletonAsHunk(int structure, ModelSkeleton_t *skel);
void CreateSkeletonInPlace(int structure, ModelSkeleton_t *skel);
void ClearSkeleton(ModelSkeleton_t *skel, int root);

void SetupJointRotations(ModelSkeleton_t *skel, int jointIndex, int anglesIndex);
void FinishJointRotations(ModelSkeleton_t *skel, int jointIndex);
void LinearllyInterpolateJoints(ModelSkeleton_t *newSkel, int newIndex, 
	ModelSkeleton_t *oldSkel, int oldIndex, ModelSkeleton_t *liSkel, int liIndex,
	float move[3], float frontv[3], float backv[3]);
void SetupCompressedJoints(ModelSkeleton_t *liSkel, int liIndex,
	float *lerp);
void RotateModelSegments(ModelSkeleton_t *skel, int jointIndex, int modelClusterIndex, int anglesIndex, 
	vec3_t *modelVerticies);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif