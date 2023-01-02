//
// Copyright 1998 Raven Software
//
// Heretic II
//
#ifndef COMMON_ARRAYEDLIST_H
#define COMMON_ARRAYEDLIST_H

#include "q_shared.h"
#include <assert.h>

typedef struct ArrayedListNode_s
{
	int data;
	int next;
	int inUse;
} ArrayedListNode_t;

#define ARRAYEDLISTNODE_NULL -1

YQ2_ATTR_INLINE int GetFreeNode(ArrayedListNode_t *nodeArray, int max)
{
	int i;

	for(i = 0; i < max; ++i)
	{
		if(!nodeArray[i].inUse)
		{
			nodeArray[i].inUse = 1;
			return i;
		}
	}

	assert(0);
	return -1;
}

YQ2_ATTR_INLINE void FreeNode(ArrayedListNode_t *nodeArray, int index)
{
	nodeArray[index].inUse = 0;
}

#endif
