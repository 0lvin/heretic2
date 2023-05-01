//
// ResourceManager.h
//
// Copyright 1998 Raven Software
//

#include <stdlib.h>		// needed here for size_t

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ResourceManager_s
{
	size_t resSize;
	unsigned int resPerBlock;
	unsigned int nodeSize;
	struct ResMngr_Block_s *blockList;
	char **free;
} ResourceManager_t;

void ResMngr_Con(ResourceManager_t *resource, size_t init_resSize, unsigned int init_resPerBlock);
void ResMngr_Des(ResourceManager_t *resource);
void *ResMngr_AllocateResource(ResourceManager_t *resource, size_t size);
void ResMngr_DeallocateResource(ResourceManager_t *resource, void *toDeallocate, size_t size);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif
