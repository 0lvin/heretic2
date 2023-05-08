//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef M_SKELETALCLUSTER_H
#define M_SKELETALCLUSTER_H

typedef struct M_SkeletalCluster_s
{
	int children;		// must be the first field
	int numVerticies;
	int *verticies;
	qboolean inUse;
} M_SkeletalCluster_t;

#endif
