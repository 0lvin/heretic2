//
// ResourceManager.c
//
// Copyright 1998 Raven Software
//

#include <stdio.h>
#include "resourcemanager.h"
#include <assert.h>
#include "../../common/header/shared.h"

typedef struct ResMngr_Block_s
{
	char *start;
	unsigned int size;
	struct ResMngr_Block_s *next;
} ResMngr_Block_t;

static void ResMngr_CreateBlock(ResourceManager_t *resource)
{
	unsigned int _blockSize;
	char *block;
	char **current;
	ResMngr_Block_t *temp;
	unsigned int i;

	_blockSize = resource->nodeSize * resource->resPerBlock;

	block = (char *)malloc(_blockSize);

	assert(block);

	temp = (ResMngr_Block_t *)malloc(sizeof(*temp));

	temp->start = block;
	temp->size = _blockSize;
	temp->next = resource->blockList;

	resource->blockList = temp;

	resource->free = (char **)(block);

	current = resource->free;

	for (i = 1; i < resource->resPerBlock; ++i)
	{
		// set current->next to point to next node
		*current = (char *)(current) + resource->nodeSize;

		// set current node to current->next
		current = (char **)(*current);
	}

	*current = NULL;
}

void ResMngr_Con(ResourceManager_t *resource, size_t init_resSize, unsigned int init_resPerBlock)
{
	resource->resSize = init_resSize;

	resource->resPerBlock = init_resPerBlock;

	resource->nodeSize = (resource->resSize + sizeof(*resource->free) + 31) & ~31;

	resource->blockList = NULL;

	ResMngr_CreateBlock(resource);
}

void ResMngr_Des(ResourceManager_t *resource)
{
	ResMngr_Block_t *toDelete;

	while (resource->blockList)
	{
		toDelete = resource->blockList;
		resource->blockList = resource->blockList->next;
		free(toDelete->start);
		free(toDelete);
	}
	memset(resource, 0, sizeof(ResourceManager_t));
}

void *ResMngr_AllocateResource(ResourceManager_t *resource, size_t size)
{
	char **toPop;

	assert(size <= resource->resSize);

	assert(resource->free);	// constructor not called; possibly due to a static object
								// containing a static ResourceManagerFastLarge member being
								// constructed before its own static members

	toPop = resource->free;

	// set unallocated to the next node and check for NULL (end of list)
	if (!(resource->free = (char **)(*resource->free)))
	{	// if at end create new block
		ResMngr_CreateBlock(resource);
	}

	// set next to NULL
	*toPop = NULL;

	// return the resource for the node
	return (void *)(toPop + 1);
}

void ResMngr_DeallocateResource(ResourceManager_t *resource, void *toDeallocate, size_t size)
{
	char **toPush;

	//	assert(size == resource->resSize);

	toPush = (char **)(toDeallocate)-1;

	assert(resource->free);	// see same assert at top of AllocateResource

	// set toPop->next to current unallocated front
	*toPush = (char *)(resource->free);

	// set unallocated to the node removed from allocated
	resource->free = toPush;
}

// end
