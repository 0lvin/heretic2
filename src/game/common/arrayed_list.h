//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef QCOMMON_ARRAYED_LIST_H
#define QCOMMON_ARRAYED_LIST_H

#include <assert.h>

typedef struct ArrayedListNode_s
{
	int data;
	int next;
	int inUse;
} ArrayedListNode_t;

#define ARRAYEDLISTNODE_NULL -1

int GetFreeNode(ArrayedListNode_t *nodeArray, int max);
void FreeNode(ArrayedListNode_t *nodeArray, int index);

#endif
