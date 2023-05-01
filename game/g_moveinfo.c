//
// Heretic II
// Copyright 1998 Raven Software
//
#include "../src/common/header/common.h"
#include "g_moveinfo.h"
#include "../qcommon/resourcemanager.h"
#include "g_local.h"

static ResourceManager_t MoveInfoMngr;

void InitMoveInfoMngr()
{
#define MOVEINFO_BLOCK_SIZE 32

	ResMngr_Con(&MoveInfoMngr, sizeof(MoveInfo_t), MOVEINFO_BLOCK_SIZE);
}

void ReleaseMoveInfoMngr()
{
	ResMngr_Des(&MoveInfoMngr);
}

MoveInfo_t *MoveInfo_new()
{
	MoveInfo_t *newInfo;

	newInfo = (MoveInfo_t *)ResMngr_AllocateResource(&MoveInfoMngr, sizeof(*newInfo));

	memset(newInfo, 0, sizeof(*newInfo));

	newInfo->pivotDirection = 1.0;

	return newInfo;
}

void MoveInfo_delete(MoveInfo_t *toDelete)
{
	ResMngr_DeallocateResource(&MoveInfoMngr, toDelete, sizeof(*toDelete));
}
