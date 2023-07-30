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

#include "../../player/player.h"
#include "../header/local.h"
#include "../../common/header/common.h"

// Structure containing functions and data pointers exported from the player DLL.

player_export_t	*playerExport;

// Handle to player DLL.

static void *player_library;

// ************************************************************************************************
// P_Freelib
// ---------
// ************************************************************************************************

void P_Freelib()
{
	if(!player_library)
	{
		return;
	}

	if (playerExport)
	{
		playerExport->Shutdown();
	}

	dlclose (player_library);
	player_library = NULL;
}

// ************************************************************************************************
// P_Load
// ------
// ************************************************************************************************

void *
P_Load(void)
{

	player_export_t *(*P_GetPlayerAPI)(game_import_t *import);

	char name[MAX_OSPATH];
	char *path;
	char *str_p;
#ifdef __APPLE__
	const char *playername = "player.dylib";
#else
	const char *playername = "player.so";
#endif

	if (player_library)
	{
		Com_Error(ERR_FATAL, "%s without P_Freelib", __func__);
	}

	Com_Printf("Loading library: %s\n", playername);

	/* now run through the search paths */
	path = NULL;

	while (1)
	{
		FILE *fp;

		path = FS_NextPath(path);

		if (!path)
		{
			return NULL;     /* couldn't find one anywhere */
		}

		snprintf(name, MAX_OSPATH, "%s/%s", path, playername);

		/* skip it if it just doesn't exist */
		fp = fopen(name, "rb");

		if (fp == NULL)
		{
			continue;
		}

		fclose(fp);

#ifdef USE_SANITIZER
		player_library = dlopen(name, RTLD_NOW | RTLD_NODELETE);
#else
		player_library = dlopen(name, RTLD_NOW);
#endif

		if (player_library)
		{
			Com_MDPrintf("Loading library: %s\n", name);
			break;
		}
		else
		{
			Com_Printf("Loading library: %s\n: ", name);

			path = (char *)dlerror();
			str_p = strchr(path, ':');   /* skip the path (already shown) */

			if (str_p == NULL)
			{
				str_p = path;
			}
			else
			{
				str_p++;
			}

			Com_Printf("%s\n", str_p);

			return NULL;
		}
	}

	P_GetPlayerAPI = (void *)dlsym(player_library, "GetPlayerAPI");

	if (!P_GetPlayerAPI)
	{
		P_Freelib();
		return NULL;
	}

	playerExport = P_GetPlayerAPI(&gi);
	playerExport->Init();

	Com_Printf("------------------------------------\n");

	return playerExport;
}

