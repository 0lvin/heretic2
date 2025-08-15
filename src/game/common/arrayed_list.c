//
// Heretic II
// Copyright 1998 Raven Software
//
#include "arrayed_list.h"

int
GetFreeNode(ArrayedListNode_t *nodeArray, int max)
{
	int i;

	for(i = 0; i < max; ++i)
	{
		if (!nodeArray[i].inUse)
		{
			nodeArray[i].inUse = 1;
			return i;
		}
	}

	assert(0);
	return -1;
}

void
FreeNode(ArrayedListNode_t *nodeArray, int index)
{
	nodeArray[index].inUse = 0;
}
