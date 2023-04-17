//
// SinglyLinkedList.h
//
// Copyright 1998 Raven Software
//

#ifndef	SINGLYLINKEDLIST_H
#define	SINGLYLINKEDLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "GenericUnions.h"

// jmarshall 
typedef struct SinglyLinkedListNode_s
{
	GenericUnion4_t value;
	struct SinglyLinkedListNode_s *next;
} SinglyLinkedListNode_t;
// jmarshall end

typedef struct SinglyLinkedList_s
{
	struct SinglyLinkedListNode_s *rearSentinel;
	struct SinglyLinkedListNode_s *front;
	struct SinglyLinkedListNode_s *current;
} SinglyLinkedList_t;

QUAKE2_API void SLList_DefaultCon(SinglyLinkedList_t *this_ptr);
QUAKE2_API void SLList_Des(SinglyLinkedList_t *this_ptr);
QUAKE2_API qboolean SLList_AtEnd(SinglyLinkedList_t *this_ptr);
QUAKE2_API qboolean SLList_AtLast(SinglyLinkedList_t *this_ptr);
QUAKE2_API qboolean SLList_IsEmpty(SinglyLinkedList_t *this_ptr);
QUAKE2_API const GenericUnion4_t SLList_Increment(SinglyLinkedList_t *this_ptr);
QUAKE2_API const GenericUnion4_t SLList_PostIncrement(SinglyLinkedList_t *this_ptr);
QUAKE2_API GenericUnion4_t SLList_Front(SinglyLinkedList_t *this_ptr);
QUAKE2_API GenericUnion4_t SLList_ReplaceCurrent(SinglyLinkedList_t *this_ptr, const GenericUnion4_t toReplace);
QUAKE2_API void SLList_PushEmpty(SinglyLinkedList_t *this_ptr);
QUAKE2_API void SLList_Push(SinglyLinkedList_t *this_ptr, const GenericUnion4_t toInsert);
QUAKE2_API GenericUnion4_t SLList_Pop(SinglyLinkedList_t *this_ptr);
QUAKE2_API void SLList_Chop(SinglyLinkedList_t *this_ptr);
QUAKE2_API void SLList_InsertAfter(SinglyLinkedList_t *this_ptr, const GenericUnion4_t toInsert);


#ifdef __cplusplus
} //end extern "C"
#endif

#endif