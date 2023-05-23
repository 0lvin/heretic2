//
// Heretic II
// Copyright 1998 Raven Software
//
#include "header/local.h"
#include "../../h2common/singlylinkedlist.h"

void G_InitMsgMngr();
void G_ReleaseMsgMngr();


void G_InitResourceManagers()
{
	InitResourceManager();

	G_InitMsgMngr();
}

void G_ReleaseResourceManagers()
{
	G_ReleaseMsgMngr();
	ShutdownResourceManager();
}
