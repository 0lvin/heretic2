//
// g_Message.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include "g_Message.h"
#include "Message.h"

#include "g_local.h"
#include "ResourceManager.h"
#include "SinglyLinkedList.h"

static ResourceManager_t MsgMngr;

void G_InitMsgMngr()
{
#define MESSAGE_BLOCK_SIZE 256

	ResMngr_Con(&MsgMngr, sizeof(G_Message_t), MESSAGE_BLOCK_SIZE);
}

void G_ReleaseMsgMngr()
{
	ResMngr_Des(&MsgMngr);
}

void G_Message_DefaultCon(G_Message_t *_this)
{
	SinglyLinkedList_t *parms;

	parms = &_this->parms;

	// Fix Me !!!
	SLList_DefaultCon(parms);  // whoops, need to port object manager to C
	SLList_PushEmpty(parms);			// should make a constructor fo G_Message_t too
}

G_Message_t *G_Message_new(G_MsgID_t ID, G_MsgPriority_t priority)
{
	G_Message_t *newMsg;

	newMsg = (G_Message_t *) ResMngr_AllocateResource(&MsgMngr, sizeof(G_Message_t));

	G_Message_DefaultCon(newMsg);

	newMsg->ID = ID;
	newMsg->priority = priority;

	return newMsg;
}

void G_Message_Des(G_Message_t *_this)
{
	SLList_Des(&_this->parms);
}

void G_Message_delete(G_Message_t *_this)
{
	G_Message_Des(_this);

	ResMngr_DeallocateResource(&MsgMngr, _this, sizeof(G_Message_t));
}

void G_QPostMessage(edict_t *to, G_MsgID_t ID, G_MsgPriority_t priority, char *format, ...)
{
	G_Message_t *newMsg;
	qboolean append = false;
	SinglyLinkedList_t *parms;
	va_list marker;

	if(!to->msgHandler)	// everything should really have one, but at _this point everything
						// doesn't so, the messages will never get popped of the queue
						// so don't push them on in the first place
	{
		return;
	}

	newMsg = (G_Message_t*)ResMngr_AllocateResource(&MsgMngr, sizeof(G_Message_t));

	// Fix Me !!!
	G_Message_DefaultCon(newMsg);	// whoops, need to port object manager to C

	parms = &newMsg->parms;

	newMsg->ID = ID;
	newMsg->priority = priority;

	if(format)
	{
		va_start(marker, format);

		SetParms(parms, format, marker, false);

		va_end(marker);
	}

	QueueMessage(&to->msgQ, newMsg);
}

void PostGameMessage(edict_t *to, G_MsgID_t ID, G_MsgPriority_t priority, char *format, ...)
{
	G_Message_t *newMsg;
	qboolean append = false;
	SinglyLinkedList_t *parms;
	va_list marker;

	if(!to->msgHandler)	// everything should really have one, but at _this point everything
						// doesn't so, the messages will never get popped of the queue
						// so don't push them on in the first place
	{
		return;
	}

	newMsg = (G_Message_t*)ResMngr_AllocateResource(&MsgMngr, sizeof(G_Message_t));

	// Fix Me !!!
	G_Message_DefaultCon(newMsg);	// whoops, need to port object manager to C

	parms = &newMsg->parms;

	newMsg->ID = ID;
	newMsg->priority = priority;

	if(format)
	{
		va_start(marker, format);

		SetParms(parms, format, marker, false);

		va_end(marker);
	}

	QueueMessage(&to->msgQ, newMsg);
}

int G_ParseMsgParms(G_Message_t *_this, char *format, ...)
{
	SinglyLinkedList_t *parms;
	va_list marker;
	int args_filled;

	assert(_this);

	parms = &_this->parms;

	SLList_Front(parms);

	va_start(marker, format);

	args_filled = GetParms(parms, format, marker);

	va_end(marker);

	return args_filled;
}

void G_ProcessMessages(edict_t *self)
{
	SinglyLinkedList_t *msgs;
	SinglyLinkedList_t *parms;
	G_Message_t *msg;

	assert(self->msgHandler);

	msgs = &self->msgQ.msgs;

	if(!SLList_IsEmpty(msgs))
	{
		self->flags &= ~FL_SUSPENDED;
	}

	while(!SLList_IsEmpty(msgs))
	{
		msg = (G_Message_t*)SLList_Pop(msgs).t_void_p;

		parms = &msg->parms;

		if(!SLList_AtLast(parms) && !SLList_AtEnd(parms))
		{
			SLList_Chop(parms);
		}

		self->msgHandler(self, msg);

		// Fix Me !!!
		G_Message_Des(msg); // whoops, need to port object manager to C

		ResMngr_DeallocateResource(&MsgMngr, msg, sizeof(G_Message_t));
	}
}

void G_ClearMessageQueue(edict_t *self)
{
	SinglyLinkedList_t *msgs;
	SinglyLinkedList_t *parms;
	G_Message_t *msg;

	msgs = &self->msgQ.msgs;

	// If either of these fire - do a rebuild all
	// otherwise it will try to free random memory and lead to an unstable system
	assert(msgs->front);
	assert(msgs->rearSentinel);

	while(!SLList_IsEmpty(msgs))
	{
		msg = (G_Message_t*)SLList_Pop(msgs).t_void_p;

		parms = &msg->parms;

		// Fix Me !!!
		SLList_Des(parms); // whoops, need to port object manager to C

		ResMngr_DeallocateResource(&MsgMngr, msg, sizeof(G_Message_t));
	}
}

void G_ClearMessageQueues()
{
	edict_t *ent;
	int i;

	for (i=0, ent = &g_edicts[0]; i<globals.num_edicts ; i++, ent++)
	{
		G_ClearMessageQueue(ent);
	}
}