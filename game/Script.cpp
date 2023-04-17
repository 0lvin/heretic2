//
// Script.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include "ICScript.h"
#include "g_local.h"
#include "SinglyLinkedList.h"

void ICScript_Con(ICScript_t *this_ptr, char *name)
{
	char path[1024];

	this_ptr->startFrame = level.framenum;

	sprintf(path, "cinematics/%s.ics", name);

	this_ptr->bufSize = gi.FS_LoadFile(path, (void **)&this_ptr->buf);

	this_ptr->count = 0;
}

void ICScript_Des(ICScript_t *_this)
{
	gi.FS_FreeFile(_this->buf);
}

static qboolean ICScript_Advance(ICScript_t *_this)
{
	int i, frame, msgFrame, msgID;
	char entityName[64];
	char format[64];
	size_t size;
	G_Message_t *newMsg;
	SinglyLinkedList_t *parms;
	edict_t *ent = NULL;

	frame = level.framenum - _this->startFrame;

	while(_this->count < _this->bufSize)
	{
		size = sizeof(int);
		memcpy(&msgFrame, _this->buf + _this->count, size);

		if(msgFrame > frame)
			break;

		_this->count += size;

		i = 0;

		do
		{
			entityName[i] = *(_this->buf + _this->count);
			++i;
		}while(*(_this->buf + _this->count++));

		size = sizeof(int);
		memcpy(&msgID, _this->buf + _this->count, size);
		_this->count += size;

		i = 0;

		do
		{
			format[i] = *(_this->buf + _this->count);
			++i;
		}while(*(_this->buf + _this->count++));

		i = 0;

		newMsg = G_Message_new((G_MsgID_t)msgID, PRI_SYSTEM);

		parms = &newMsg->parms;

		if(format[0])
		{
			_this->count += SetParms(&newMsg->parms, format, _this->buf + _this->count, true);
		}

		// _this sucks, sticking each string field that gets searched on into a binary search
		// tree or a hash table would probably be a good idea
		// That would a little overhead when an entity is created or destroyed, but it should
		// be worth it for the vastly improved search time
		ent = G_Find (ent, FOFS(targetname), entityName);

#ifdef _DEBUG
		if(ent)
		{
			edict_t *ent2 = NULL;

			ent2 = G_Find (ent, FOFS(targetname), entityName);

			assert(!ent2);	// each entityName should be unique!!!
		}
#endif

		if(ent)
		{
			QueueMessage(&ent->msgQ, newMsg);
		}
		else
		{
			G_Message_delete(newMsg);
		}

		assert(_this->count <= _this->bufSize);
	}

	if(_this->count >= _this->bufSize)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void RunICScript()
{
	if(level.cinActive)
	{
		if(ICScript_Advance(&level.inGameCin))
		{
			ICScript_Des(&level.inGameCin);
			level.cinActive = false;
		}
	}
}

void KillICScript()
{
	if(level.cinActive)
	{
		ICScript_Des(&level.inGameCin);
		level.cinActive = false;
	}
}