//
// p_dll.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#endif

#include "../player/player.h"
#include "../game/g_local.h"
#include "../src/common/header/common.h"

PLAYER_API player_export_t GetPlayerAPI(void);

// Structure containing functions and data pointers exported from the player DLL.

player_export_t	playerExport;

// Handle to player DLL.

#ifdef _WIN32
static HINSTANCE player_library = NULL;
#endif

// ************************************************************************************************
// P_Freelib
// ---------
// ************************************************************************************************

void P_Freelib()
{
	P_Shutdown();
}

// ************************************************************************************************
// P_Load
// ------
// ************************************************************************************************
unsigned int P_Load(char *name)
{
	P_Init();

	playerExport = GetPlayerAPI();
	return 0;
}
