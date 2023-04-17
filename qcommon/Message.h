#ifndef MESSAGE_H
#define MESSAGE_H

#include "SinglyLinkedList.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MsgQueue_s
{
	SinglyLinkedList_t msgs;
} MsgQueue_t;

size_t SetParms(SinglyLinkedList_t *this_ptr, char *format, va_list marker, qboolean entsAsNames);
int GetParms(SinglyLinkedList_t *this_ptr, char *format, va_list marker);
void QueueMessage(MsgQueue_t *this_ptr, void *msg);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif