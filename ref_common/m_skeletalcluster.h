#ifndef M_SKELETALCLUSTER_H
#define M_SKELETALCLUSTER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct M_SkeletalCluster_s
{
	int children;		// must be the first field
	int numVerticies;
	int *verticies;
	qboolean inUse;
} M_SkeletalCluster_t;

#ifdef __cplusplus
} //end extern "C"
#endif

#endif