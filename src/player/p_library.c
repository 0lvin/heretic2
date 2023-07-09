//
// p_dll.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "player.h"
#include "../game/header/local.h"
#include "../common/header/common.h"

// Structure containing functions and data pointers exported from the player DLL.

player_export_t	playerExport;

// Handle to player DLL.

static void *player_library;

// ************************************************************************************************
// P_Freelib
// ---------
// ************************************************************************************************

void P_Freelib()
{
	// TODO: Rewrite, add real library load
	if(!player_library)
	{
		return;
	}

	playerExport.Shutdown();

	dlclose (player_library);
	player_library = NULL;
}

// ************************************************************************************************
// P_Load
// ------
// ************************************************************************************************

unsigned int P_Load(char *name)
{
	// TODO: Rewrite, add real library load
	playerExport = GetPlayerAPI();
	playerExport.Init();
	return 0;

	char	curpath[MAX_OSPATH];
	char	gamepath[MAX_OSPATH];

	int			playerdll_chksum;
	GetPlayerAPI_t	P_GetPlayerAPI;

	P_Freelib();

	setreuid(getuid(), getuid());
	setegid(getgid());


	getcwd(curpath, sizeof(curpath));

	Com_Printf("------- Loading %s -------\n", name);
	sprintf (gamepath, "%s/%s", curpath, name);
	// now run through the search paths
	player_library = dlopen (gamepath, RTLD_LAZY );
	if (player_library)
	{
		Com_Printf ("LoadLibrary (%s)\n", gamepath);
	}
	else
	{
		Sys_Error ("Failed LoadLibrary %s: %s\n", gamepath, dlerror());
	}

	if((P_GetPlayerAPI = (void *)dlsym (player_library, "GetPlayerAPI")) == 0)
		Sys_Error ("GetProcAddress failed on GetPlayerAPI for library %s", name);

	playerExport = P_GetPlayerAPI();

	playerExport.Init();

	Com_Printf("------------------------------------\n");
	return(playerdll_chksum);
}
