//
// p_dll.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include <stdio.h>
#include <stdlib.h>

#include "../player/library/player.h"
#include "../header/local.h"
#include "../../common/header/common.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// Structure containing functions and data pointers exported from the player DLL.

player_export_t	*playerExport;
player_import_t	playerImport;

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

#if _WIN32
	FreeLibrary(player_library);
#else
	dlclose (player_library);
#endif

	player_library = NULL;
}

static int
GetItemIndex(const gitem_t *item)
{
	return ITEM_INDEX(item);
}

// ************************************************************************************************
// P_Load
// ------
// ************************************************************************************************

void *
P_Load(void)
{
	player_export_t *(*P_GetPlayerAPI)(player_import_t *import);

	char name[MAX_OSPATH];
	const char *path;
#ifdef _WIN32
	WCHAR wname[MAX_OSPATH];
	const char *playername = "player.dll";
#else
#ifdef __APPLE__
	const char *playername = "player.dylib";
#else
	const char *playername = "player.so";
#endif
#endif

	if (player_library)
	{
		Com_Printf("%s without P_Freelib", __func__);
		return NULL;
	}

	Com_Printf("Loading library: %s\n", playername);

	/* now run through the search paths */
	path = NULL;

	while (1)
	{
		FILE *fp;

		path = gi.FS_NextPath(path);

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

#ifdef _WIN32
		MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, MAX_OSPATH);
		player_library = LoadLibraryW(wname);
#else

#ifdef USE_SANITIZER
		player_library = dlopen(name, RTLD_NOW | RTLD_NODELETE);
#else
		player_library = dlopen(name, RTLD_NOW);
#endif

#endif

		if (player_library)
		{
			Com_Printf("Loading library: %s\n", name);
			break;
		}
#ifndef _WIN32
		else
		{
			const char *str_p;

			Com_Printf("Failed to load library: %s\n: ", name);

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
#endif
}

#ifdef _WIN32
	P_GetPlayerAPI = (void *)GetProcAddress(player_library, "GetPlayerAPI");
#else
	P_GetPlayerAPI = (void *)dlsym(player_library, "GetPlayerAPI");
#endif

	if (!P_GetPlayerAPI)
	{
		P_Freelib();
		return NULL;
	}

	playerImport.dprintf = gi.dprintf;
	playerImport.FindItem = FindItem;
	playerImport.GetItemIndex = GetItemIndex;

	playerExport = P_GetPlayerAPI(&playerImport);
	playerExport->Init();

	Com_Printf("------------------------------------\n");

	return playerExport;
}
