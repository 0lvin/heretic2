//
// Heretic II
// Copyright 1998 Raven Software
//
// H2SinglyLinkedList.c
//

#include "../../common/header/shared.h"
#include "singlylinkedlist.h"
#include "resourcemanager.h"

#include <stdint.h>

static ResourceManager_t globalResourceManager;

void SLList_DefaultCon(SinglyLinkedList_t *this_ptr)
{
	this_ptr->rearSentinel = (SinglyLinkedListNode_t *)ResMngr_AllocateResource(
		&globalResourceManager, sizeof(GenericUnion4_t));
	this_ptr->current = this_ptr->rearSentinel;
	this_ptr->front = this_ptr->rearSentinel;
}
void SLList_Des(SinglyLinkedList_t* this_ptr)
{
	SinglyLinkedListNode_t* node;

	node = this_ptr->front;
	while (node != this_ptr->rearSentinel)
	{
		node = node->next;
		ResMngr_AllocateResource(
			&globalResourceManager, sizeof(GenericUnion4_t));
	}
	this_ptr->current = this_ptr->rearSentinel;
	ResMngr_AllocateResource(&globalResourceManager, sizeof(GenericUnion4_t));
}

qboolean SLList_AtEnd(SinglyLinkedList_t *this_ptr)
{
	return this_ptr->current == this_ptr->rearSentinel;
}

qboolean SLList_AtLast(SinglyLinkedList_t *this_ptr)
{
	return this_ptr->current->next == this_ptr->rearSentinel;
}

qboolean SLList_IsEmpty(SinglyLinkedList_t *this_ptr)
{
	return this_ptr->front == this_ptr->rearSentinel;
}

const GenericUnion4_t SLList_Increment(SinglyLinkedList_t *this_ptr)
{
	struct SinglyLinkedListNode_s* nextNode;
	nextNode = this_ptr->current->next;
	this_ptr->current = nextNode;
	return nextNode->value;
}

const GenericUnion4_t SLList_PostIncrement(SinglyLinkedList_t *this_ptr)
{
	GenericUnion4_t value;
	SinglyLinkedListNode_t* currentNode;

	currentNode = this_ptr->current;
	value = currentNode->value;
	this_ptr->current = currentNode->next;
	return value;
}

GenericUnion4_t SLList_Front(SinglyLinkedList_t *this_ptr)
{
	SinglyLinkedListNode_t* frontNode;

	frontNode = this_ptr->front;
	this_ptr->current = frontNode;
	return frontNode->value;
}

GenericUnion4_t SLList_ReplaceCurrent(SinglyLinkedList_t *this_ptr, const GenericUnion4_t toReplace)
{
	GenericUnion4_t oldValue;
	SinglyLinkedListNode_t* node;

	node = this_ptr->current;
	oldValue = node->value;
	node->value = toReplace;
	return oldValue;
}

void SLList_PushEmpty(SinglyLinkedList_t *this_ptr)
{
	SinglyLinkedListNode_t* emptyNode;

	emptyNode = (SinglyLinkedListNode_t*)ResMngr_AllocateResource(
		&globalResourceManager, sizeof(GenericUnion4_t));
	emptyNode->next = this_ptr->front;
	this_ptr->front = emptyNode;
}

void SLList_Push(SinglyLinkedList_t *this_ptr, const GenericUnion4_t toInsert)
{
	SinglyLinkedListNode_t* newNode;

	newNode = (SinglyLinkedListNode_t*)ResMngr_AllocateResource(
		&globalResourceManager, sizeof(GenericUnion4_t));
	newNode->value = toInsert;
	newNode->next = this_ptr->front;
	this_ptr->front = newNode;
}

GenericUnion4_t SLList_Pop(SinglyLinkedList_t *this_ptr)
{
	GenericUnion4_t value;
	SinglyLinkedListNode_t* nextNode;
	SinglyLinkedListNode_t* currentNode;
	SinglyLinkedListNode_t* frontNode;

	frontNode = this_ptr->front;
	currentNode = this_ptr->current;
	nextNode = frontNode->next;
	this_ptr->front = nextNode;
	if (currentNode == frontNode)
		this_ptr->current = nextNode;
	value = frontNode->value;
	ResMngr_DeallocateResource(&globalResourceManager, currentNode, sizeof(GenericUnion4_t));
	return value;
}

void SLList_Chop(SinglyLinkedList_t *this_ptr)
{
	SinglyLinkedList_t* currentNode;
	SinglyLinkedList_t* nextNode;

	nextNode = (SinglyLinkedList_t*)this_ptr->current->next;
	if (nextNode == (SinglyLinkedList_t*)this_ptr->rearSentinel)
	{
		this_ptr->current = this_ptr->rearSentinel;
	}
	else
	{
		do
		{
			currentNode = nextNode;
			nextNode = (SinglyLinkedList_t*)nextNode->front;
			ResMngr_DeallocateResource(&globalResourceManager, currentNode, sizeof(GenericUnion4_t));
		} while (nextNode != (SinglyLinkedList_t*)this_ptr->rearSentinel);
		this_ptr->current = this_ptr->rearSentinel;
	}
}

void SLList_InsertAfter(SinglyLinkedList_t *this_ptr, const GenericUnion4_t toInsert)
{
	SinglyLinkedListNode_t* newNode;

	newNode = (SinglyLinkedListNode_t*)ResMngr_AllocateResource(
		&globalResourceManager, sizeof(GenericUnion4_t));
	newNode->value = toInsert;
	newNode->next = this_ptr->current->next;
	this_ptr->current->next = newNode;
}

/*
===============
InitResourceManager
===============
*/
void InitResourceManager()
{
	// jmarshall:
	// Based on decompiled output, Raven had globalResourceManager allocated to use 256 8 * sizeof(resourceNode) byte blocks,
	// this seems really small, in x64 this actually causes code to run past the block size. Which seems in line,
	// with some of the crashes people have experienced with Raven's binaries. I'm increasing this limit,
	// because were not that worried about OOMing, and seems safer then modifying a bunch of code on the game/client side.
	ResMngr_Con(&globalResourceManager, sizeof(GenericUnion4_t), 256);
}

/*
===============
ShutdownResourceManager
===============
*/

void ShutdownResourceManager()
{
	ResMngr_Des(&globalResourceManager);
}
