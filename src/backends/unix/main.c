/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * This file is the starting point of the program and implements
 * the main loop
 *
 * =======================================================================
 */

#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "../../common/header/common.h"
#include "header/unix.h"

qboolean is_portable;

int
main(int argc, char **argv)
{
	int verLen, i;
	long long oldtime, newtime;
	const char* versionString;

	// Time slept each frame.
#ifndef DEDICATED_ONLY
	struct timespec t = {0, 5000};
#else
	struct timespec t = {0, 850000};
#endif

	/* register signal handler */
	registerHandler();

	/* Setup FPU if necessary */
	Sys_SetupFPU();

	/* Are we portable? */
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-portable") == 0) {
			is_portable = true;
		}
	}

	/* Prevent running Quake II as root. Only very mad
	   minded or stupid people even think about it. :) */
	if (getuid() == 0)
	{
		printf("Quake II shouldn't be run as root! Backing out to save your ass. If\n");
		printf("you really know what you're doing, edit src/unix/main.c and remove\n");
		printf("this check. But don't complain if Quake II eats your dog afterwards!\n");

		return 1;
	}

	/* Enforce the real UID to
	   prevent setuid crap */
	if (getuid() != geteuid())
	{
		printf("The effective UID is not the real UID! Your binary is probably marked\n");
		printf("'setuid'. That is not good idea, please fix it :) If you really know\n");
		printf("what you're doing edit src/unix/main.c and remove this check. Don't\n");
		printf("complain if Quake II eats your dog afterwards!\n");

		return 1;
	}

	/* enforce C locale */
	setenv("LC_ALL", "C", 1);

	versionString = va("Yamagi Quake II v%s", YQ2VERSION);
	verLen = strlen(versionString);

	printf("\n%s\n", versionString);
	for(i=0; i<verLen; ++i)
	{
		putc('=', stdout);
	}
	puts("\n");

#ifndef DEDICATED_ONLY
	printf("Client build options:\n");
#ifdef SDL2
	printf(" + SDL2\n");
#else
	printf(" - SDL2 (using 1.2)\n");
#endif

#ifdef CDA
	printf(" + CD audio\n");
#else
	printf(" - CD audio\n");
#endif
#ifdef OGG
	printf(" + OGG/Vorbis\n");
#else
	printf(" - OGG/Vorbis\n");
#endif
#ifdef USE_OPENAL
	printf(" + OpenAL audio\n");
#else
	printf(" - OpenAL audio\n");
#endif
#ifdef ZIP
	printf(" + Zip file support\n");
#else
	printf(" - Zip file support\n");
#endif
#endif

	printf("Platform: %s\n", YQ2OSTYPE);
	printf("Architecture: %s\n", YQ2ARCH);

	/* Seed PRNG */
	randk_seed();

	/* Initialze the game */
	Qcommon_Init(argc, argv);

	/* Do not delay reads on stdin*/
	fcntl(fileno(stdin), F_SETFL, fcntl(fileno(stdin), F_GETFL, NULL) | FNDELAY);

	oldtime = Sys_Microseconds();

	/* The mainloop. The legend. */
	while (1)
	{
		// Throttle the game a little bit.
		nanosleep(&t, NULL);

		newtime = Sys_Microseconds();
		Qcommon_Frame(newtime - oldtime);
		oldtime = newtime;
	}

	return 0;
}

