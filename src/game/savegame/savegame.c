/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (C) 2011 Knightmare
 * Copyright (C) 2011 Yamagi Burmeister
 * Copyright (c) ZeniMax Media Inc.
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
 * The savegame system. Unused by the CTF game but nevertheless called
 * during game initialization. Therefor no new savegame code ist
 * imported.
 *
 * =======================================================================
 */

/*
 * This is the Quake 2 savegame system, fixed by Yamagi
 * based on an idea by Knightmare of kmquake2. This major
 * rewrite of the original g_save.c is much more robust
 * and portable since it doesn't use any function pointers.
 *
 * Inner workings:
 * When the game is saved all function pointers are
 * translated into human readable function definition strings.
 * The same way all mmove_t pointers are translated. This
 * human readable strings are then written into the file.
 * At game load the human readable strings are retranslated
 * into the actual function pointers and struct pointers. The
 * pointers are generated at each compilation / start of the
 * client, thus the pointers are always correct.
 *
 * Limitations:
 * While savegames survive recompilations of the game source
 * and bigger changes in the source, there are some limitation
 * which a nearly impossible to fix without a object orientated
 * rewrite of the game.
 *  - If functions or mmove_t structs that a referencenced
 *    inside savegames are added or removed (e.g. the files
 *    in tables/ are altered) the load functions cannot
 *    reconnect all pointers and thus not restore the game.
 *  - If the operating system is changed internal structures
 *    may change in an unrepairable way.
 *  - If the architecture is changed pointer length and
 *    other internal datastructures change in an
 *    incompatible way.
 *  - If the edict_t struct is changed, savegames
 *    will break.
 * This is not so bad as it looks since functions and
 * struct won't be added and edict_t won't be changed
 * if no big, sweeping changes are done. The operating
 * system and architecture are in the hands of the user.
 */

#include "../../common/header/common.h" // YQ2ARCH
#include "../header/local.h"
#include "savegame.h"
/*
 * When ever the savegame version is changed, q2 will refuse to
 * load older savegames. This should be bumped if the files
 * in tables/ are changed, otherwise strange things may happen.
 */
#define SAVEGAMEVER "YQ2-6"

#ifndef BUILD_DATE
#define BUILD_DATE __DATE__
#endif

/*
 * This macros are used to prohibit loading of savegames
 * created on other systems or architectures. This will
 * crash q2 in spectacular ways
 */
#ifndef YQ2OSTYPE
#error YQ2OSTYPE should be defined by the build system
#endif

#ifndef YQ2ARCH
#error YQ2ARCH should be defined by the build system
#endif

/*
 * Older operating system and architecture detection
 * macros, implemented by savegame version YQ2-1.
 */
#if defined(__APPLE__)
#define OSTYPE_1 "MacOS X"
#elif defined(__FreeBSD__)
#define OSTYPE_1 "FreeBSD"
#elif defined(__OpenBSD__)
#define OSTYPE_1 "OpenBSD"
#elif defined(__linux__)
 #define OSTYPE_1 "Linux"
#elif defined(_WIN32)
 #define OSTYPE_1 "Windows"
#else
 #define OSTYPE_1 "Unknown"
#endif

#if defined(__i386__)
#define ARCH_1 "i386"
#elif defined(__x86_64__)
#define ARCH_1 "amd64"
#elif defined(__sparc__)
#define ARCH_1 "sparc64"
#elif defined(__ia64__)
 #define ARCH_1 "ia64"
#else
 #define ARCH_1 "unknown"
#endif

/* ========================================================= */

/*
 * Prototypes for forward
 * declaration for all game
 * functions.
 */
#include "tables/gamefunc_decs.h"

/*
 * List with function pointer
 * to each of the functions
 * prototyped above.
 */
static functionList_t functionList[] = {
	#include "tables/gamefunc_list.h"
};

/*
 * Prototypes for forward
 * declaration for all game
 * mmove_t functions.
 */
#include "tables/gamemmove_decs.h"

/*
 * List with pointers to
 * each of the mmove_t
 * functions prototyped
 * above.
 */
static mmoveList_t mmoveList[] = {
	#include "tables/gamemmove_list.h"
};

/*
 * Fields to be saved (used in g_spawn.c)
 */
field_t fields[] = {
	#include "tables/fields.h"
};

/*
 * Level fields to
 * be saved
 */
static field_t levelfields[] = {
	#include "tables/levelfields.h"
};

/*
 * Client fields to
 * be saved
 */
static field_t clientfields[] = {
	#include "tables/clientfields.h"
};

/* ========================================================= */

static field_t bouyfields[] = {
	{"", BYOFS(pathtarget), F_LSTRING},
	{"", BYOFS(target), F_LSTRING},
	{"", BYOFS(targetname), F_LSTRING},
	{"", BYOFS(jump_target), F_LSTRING},
	{NULL, 0, F_INT}
};

void LoadScripts(FILE* FH, qboolean DoGlobals);
void SaveScripts(FILE* FH, qboolean DoGlobals);

// -------- just for savegames ----------
// all pointer fields should be listed here, or savegames
// won't work properly (they will crash and burn).
// this wasn't just tacked on to the fields array, because
// these don't need names, we wouldn't want map fields using
// some of these, and if one were accidentally present twice
// it would double swizzle (fuck) the pointer.

trig_message_t	game_msgtxt[MAX_MESSAGESTRINGS];
unsigned *game_msgbuf;

static int
LoadTextFile(char *name, char **addr)
{
	int length;
	char *buffer = NULL;

	length = gi.LoadFile(name, (void **)&buffer);
	if (length <= 0)
	{
		Sys_Error("Unable to load %s", name);
		return(0);
	}
	*addr = (char *)gi.TagMalloc(length + 1, 0);
	memcpy(*addr, buffer, length);
	*(*addr + length) = 0;
	gi.FreeFile(buffer);

	return(length + 1);
}

static void
Load_FileStrings(char *buffer, trig_message_t *msgtxt, int length)
{
	char *p, *startp,*return_p;
	int i;

	startp = buffer;
	p = 0;
	for (i = 1; p < (buffer + length); ++i)
	{
		if (i > MAX_MESSAGESTRINGS)
		{
			Com_Printf ("Too many strings\n");
			return;
		}

		// Read in string up to return
		return_p = strchr(startp,13);	// Search for return characters 13 10
		if (!return_p)	// At end of file
		{
			break;
		}
		else
			*return_p = 0;

		// Search string for #
		p = strchr(startp,'#');	// Search for # which signifies a wav file
		if ((p) && (p < return_p))
		{
			*p = 0;
			msgtxt[i].wav = ++p;	// Save stuff after #
		}

		// Save stuff before #
		msgtxt[i].string = startp;

		do
		{
			p = strchr(startp,'@');	// Search for # which signifies a wav file
			if (p)
				*p = '\n';
		} while (p);

		return_p += 2;	// Hop over 13 10
		startp = return_p;	// Advance to next string
	}
}

static void
Load_Strings(void)
{
	cvar_t *gamemsg_name;
	char *buffer;
	int length;

	gamemsg_name = gi.cvar("file_gamemsg", "gamemsg.txt", 1);
	length = LoadTextFile (gamemsg_name->string, &buffer);
	game_msgbuf = (unsigned *) buffer;
	Load_FileStrings(buffer, game_msgtxt, length);
}

static void
InitAllocations(void)
{
	int num_c = maxclients->value;
	int num_e = maxentities->value;

	if (num_c < 1)
	{
		num_c = 1;
		gi.cvar_forceset("maxclients", "1");
	}

	if (num_e < (num_c + 1))
	{
		num_e = num_c + 1;
		gi.cvar_forceset("maxentities", va("%d", num_c + 1));
	}

	g_edicts = gi.TagMalloc (num_e * sizeof(g_edicts[0]), TAG_GAME);
	game.maxentities = num_e;

	globals.edicts = g_edicts;
	globals.num_edicts = num_c + 1;
	globals.max_edicts = num_e;

	game.clients = gi.TagMalloc (num_c * sizeof(game.clients[0]), TAG_GAME);
	game.maxclients = num_c;
}

void
ReinitGameEntities(int ent_cnt)
{
	int num_c = maxclients->value;
	int num_e = maxentities->value;
	edict_t *edicts;

	if (num_c < 1)
	{
		num_c = 1;
		gi.cvar_forceset("maxclients", "1");
	}

	if (num_e < (num_c + 1 + ent_cnt))
	{
		num_e = num_c + 1 + ent_cnt;
		gi.cvar_forceset("maxentities", va("%d", num_e));
	}

	if (num_e < game.maxentities)
	{
		return;
	}

	edicts = gi.TagRealloc(g_edicts, num_e * sizeof(g_edicts[0]), TAG_GAME);
	if (!edicts)
	{
		gi.error("Can't extend edict list to %d\n", num_e);
		return;
	}

	g_edicts = edicts;

	game.maxentities = num_e;

	globals.edicts = g_edicts;
	globals.num_edicts = num_c + 1;
	globals.max_edicts = num_e;
}

/*
 * This will be called when the dll is first loaded,
 * which only happens when a new game is started or
 * a save game is loaded.
 */
void
InitGame(void)
{
	gi.dprintf("Game is starting up.\n");
	gi.dprintf("Game is %s built on %s.\n", GAMEVERSION, BUILD_DATE);

	gun_x = gi.cvar("gun_x", "0", 0);
	gun_y = gi.cvar("gun_y", "0", 0);
	gun_z = gi.cvar("gun_z", "0", 0);
	sv_rollspeed = gi.cvar("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar("sv_gravity", "675.0", 0);
	sv_stopspeed = gi.cvar("sv_stopspeed", "100", 0);
	g_showlogic = gi.cvar("g_showlogic", "0", 0);
	huntercam = gi.cvar("huntercam", "1", CVAR_SERVERINFO|CVAR_LATCH);
	strong_mines = gi.cvar("strong_mines", "0", 0);
	randomrespawn = gi.cvar("randomrespawn", "0", 0);

	/* noset vars */
	dedicated = gi.cvar("dedicated", "0", CVAR_NOSET);

	/* latched vars */
	sv_cheats = gi.cvar("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar("gamename", GAMEVERSION, CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar("gamedate", BUILD_DATE, CVAR_SERVERINFO | CVAR_LATCH);
	maxclients = gi.cvar("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	maxspectators = gi.cvar("maxspectators", "4", CVAR_SERVERINFO);
	deathmatch = gi.cvar("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar("coop", "0", CVAR_LATCH);
	coop_pickup_weapons = gi.cvar("coop_pickup_weapons", "1", CVAR_ARCHIVE);
	coop_baseq2 = gi.cvar("coop_baseq2", "0", CVAR_LATCH);
	coop_elevator_delay = gi.cvar("coop_elevator_delay", "1.0", CVAR_ARCHIVE);
	skill = gi.cvar("skill", "1", CVAR_LATCH);
	maxentities = gi.cvar("maxentities", "1024", CVAR_LATCH);
	gamerules = gi.cvar("gamerules", "0", CVAR_LATCH);			//PGM
	g_footsteps = gi.cvar("g_footsteps", "1", CVAR_ARCHIVE);
	g_monsterfootsteps = gi.cvar("g_monsterfootsteps", "0", CVAR_ARCHIVE);
	g_fix_triggered = gi.cvar("g_fix_triggered", "0", 0);
	g_commanderbody_nogod = gi.cvar("g_commanderbody_nogod", "0", CVAR_ARCHIVE);

	/* change anytime vars */
	dmflags = gi.cvar("dmflags", "0", CVAR_SERVERINFO);
	fraglimit = gi.cvar("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar("timelimit", "0", CVAR_SERVERINFO);
	capturelimit = gi.cvar("capturelimit", "0", CVAR_SERVERINFO);
	instantweap = gi.cvar("instantweap", "0", CVAR_SERVERINFO);
	password = gi.cvar("password", "", CVAR_USERINFO);
	spectator_password = gi.cvar("spectator_password", "", CVAR_USERINFO);
	needpass = gi.cvar("needpass", "0", CVAR_SERVERINFO);
	filterban = gi.cvar("filterban", "1", 0);
	g_select_empty = gi.cvar("g_select_empty", "0", CVAR_ARCHIVE);
	run_pitch = gi.cvar("run_pitch", "0.002", 0);
	run_roll = gi.cvar("run_roll", "0.005", 0);
	bob_up = gi.cvar("bob_up", "0.005", 0);
	bob_pitch = gi.cvar("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar("bob_roll", "0.002", 0);

	/* flood control */
	flood_msgs = gi.cvar("flood_msgs", "4", 0);
	flood_persecond = gi.cvar("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar("flood_waitdelay", "10", 0);

	/* dm map list */
	sv_maplist = gi.cvar("sv_maplist", "", 0);

	/* disruptor availability */
	g_disruptor = gi.cvar("g_disruptor", "0", 0);

	/* others */
	aimfix = gi.cvar("aimfix", "0", CVAR_ARCHIVE);
	g_machinegun_norecoil = gi.cvar("g_machinegun_norecoil", "0", CVAR_ARCHIVE);
	g_quick_weap = gi.cvar("g_quick_weap", "1", CVAR_ARCHIVE);
	g_swap_speed = gi.cvar("g_swap_speed", "1", CVAR_ARCHIVE);
	g_itemsbobeffect = gi.cvar("g_itemsbobeffect", "0", CVAR_ARCHIVE);
	g_game = gi.cvar("game", "", 0);
	g_start_items = gi.cvar("g_start_items", "", 0);
	ai_model_scale = gi.cvar("ai_model_scale", "0", 0);

	sv_cinematicfreeze = gi.cvar("sv_cinematicfreeze", "0", 0);
	sv_jumpcinematic = gi.cvar("sv_jumpcinematic", "0", 0);

	blood_level = gi.cvar("blood_level", VIOLENCE_DEFAULT_STR, CVAR_ARCHIVE);

	gi.cvar("flash_screen", "1", 0);

	flood_killdelay = gi.cvar("flood_killdelay", "10", 0);

	advancedstaff = gi.cvar("advancedstaff", "1", CVAR_SERVERINFO);
	sv_friction = gi.cvar("sv_friction", "1600.0", 0);
	sv_nomonsters = gi.cvar("nomonsters", "0", CVAR_SERVERINFO|CVAR_LATCH);
	sv_freezemonsters = gi.cvar("freezemonsters", "0", 0);

	autorotate = gi.cvar("autorotate", "0", 0);
	blood = gi.cvar("blood", "1", 0);

	checkanim = gi.cvar("checkanim", "0", 0);
	allowillegalskins = gi.cvar("allowillegalskins", "0", CVAR_ARCHIVE);

	pvs_cull = gi.cvar("pvs_cull", "1", 0);

	showbuoys = gi.cvar("showbuoys", "0", 0);
	showlitebuoys = gi.cvar("showlitebuoys", "0", 0);
	mgai_debug = gi.cvar("mgai_debug", "0", 0);
	deactivate_buoys = gi.cvar("deactivate_buoys", "0", 0);
	anarchy = gi.cvar("anarchy", "0", 0);
	impact_damage = gi.cvar("impact_damage", "1", 0);
	cheating_monsters = gi.cvar("cheating_monsters", "1", 0);
	singing_ogles = gi.cvar("singing_ogles", "0", 0);
	no_runshrine = gi.cvar("no_runshrine", "0", 0);
	no_tornado = gi.cvar("no_tornado", "0", 0);
	no_teleport = gi.cvar("no_teleport","0",0);
	no_phoenix = gi.cvar("no_phoenix","0",0);
	no_irondoom = gi.cvar("no_irondoom","0",0);
	no_morph = gi.cvar("no_morph","0",0);
	no_shield = gi.cvar("no_shield","0",0);

	/* initilize dynamic object spawn */
	SpawnInit();

	memset(&game, 0, sizeof(game));

	G_InitResourceManagers();

	if (!P_Load())
	{
		Sys_Error("Unable to player library");
	}

	G_ClearPersistantEffects();

	InitItems();

	/* initialize entities and clients arrays */
	InitAllocations();

	if (gamerules)
	{
		InitGameRules();
	}

	CTFInit();

	AI_Init();//JABot

	Load_Strings();
}

/* ========================================================= */

/*
 * Helper function to get
 * the human readable function
 * definition by an address.
 * Called by WriteField1 and
 * WriteField2.
 */
static functionList_t *
GetFunctionByAddress(byte *adr)
{
	int i;

	for (i = 0; functionList[i].funcStr; i++)
	{
		if (functionList[i].funcPtr == adr)
		{
			return &functionList[i];
		}
	}

	return NULL;
}

/*
 * Helper function to get the
 * pointer to a function by
 * it's human readable name.
 * Called by WriteField1 and
 * WriteField2.
 */
static byte *
FindFunctionByName(char *name)
{
	int i;

	for (i = 0; functionList[i].funcStr; i++)
	{
		if (!strcmp(name, functionList[i].funcStr))
		{
			return functionList[i].funcPtr;
		}
	}

	return NULL;
}

/*
 * Helper function to get the
 * human readable definition of
 * a mmove_t struct by a pointer.
 */
static mmoveList_t *
GetMmoveByAddress(mmove_t *adr)
{
	int i;

	for (i = 0; mmoveList[i].mmoveStr; i++)
	{
		if (mmoveList[i].mmovePtr == adr)
		{
			return &mmoveList[i];
		}
	}

	return NULL;
}

/*
 * Helper function to get the
 * pointer to a mmove_t struct
 * by a human readable definition.
 */
static mmove_t *
FindMmoveByName(char *name)
{
	int i;

	for (i = 0; mmoveList[i].mmoveStr; i++)
	{
		if (!strcmp(name, mmoveList[i].mmoveStr))
		{
			return mmoveList[i].mmovePtr;
		}
	}

	return NULL;
}

/* ========================================================= */

/*
 * The following two functions are
 * doing the dirty work to write the
 * data generated by the functions
 * below this block into files.
 */
static void
WriteField1(FILE *f, field_t *field, byte *base)
{
	void *p;
	size_t len;
	int index;
	functionList_t *func;
	mmoveList_t *mmove;

	if (field->flags & FFL_SPAWNTEMP)
	{
		return;
	}

	p = (void *)(base + field->ofs);

	switch (field->type)
	{
		case F_INT:
		case F_FLOAT:
		case F_ANGLEHACK:
		case F_VECTOR:
		case F_IGNORE:
		case F_RGBA:
			break;

		case F_LSTRING:
		case F_LRAWSTRING:
		case F_GSTRING:

			if (*(char **)p)
			{
				len = strlen(*(char **)p) + 1;
			}
			else
			{
				len = 0;
			}

			*(int *)p = len;
			break;
		case F_EDICT:

			if (*(edict_t **)p == NULL)
			{
				index = -1;
			}
			else
			{
				index = *(edict_t **)p - g_edicts;
			}

			*(int *)p = index;
			break;
		case F_CLIENT:

			if (*(gclient_t **)p == NULL)
			{
				index = -1;
			}
			else
			{
				index = *(gclient_t **)p - game.clients;
			}

			*(int *)p = index;
			break;
		case F_ITEM:

			if (*(edict_t **)p == NULL)
			{
				index = -1;
			}
			else
			{
				index = *(gitem_t **)p - itemlist;
			}

			*(int *)p = index;
			break;
		case F_FUNCTION:

			if (*(byte **)p == NULL)
			{
				len = 0;
			}
			else
			{
				func = GetFunctionByAddress (*(byte **)p);

				if (!func)
				{
					gi.error("%s: function not in list, can't save game",
						__func__);
					return;
				}

				len = strlen(func->funcStr) + 1;
			}

			*(int *)p = len;
			break;
		case F_MMOVE:

			if (*(byte **)p == NULL)
			{
				len = 0;
			}
			else
			{
				mmove = GetMmoveByAddress (*(mmove_t **)p);

				if (!mmove)
				{
					gi.error("%s: mmove not in list, can't save game",
						__func__);
					return;
				}

				len = strlen(mmove->mmoveStr) + 1;
			}

			*(int *)p = len;
			break;
		default:
			gi.error("%s: unknown field type", __func__);
	}
}

static void
WriteField2(FILE *f, field_t *field, byte *base)
{
	size_t len;
	void *p;
	functionList_t *func;
	mmoveList_t *mmove;

	if (field->flags & FFL_SPAWNTEMP)
	{
		return;
	}

	p = (void *)(base + field->ofs);

	switch (field->type)
	{
		case F_GSTRING:
		case F_LSTRING:
		case F_LRAWSTRING:

			if (*(char **)p)
			{
				len = strlen(*(char **)p) + 1;
				fwrite(*(char **)p, len, 1, f);
			}

			break;
		case F_FUNCTION:

			if (*(byte **)p)
			{
				func = GetFunctionByAddress (*(byte **)p);

				if (!func)
				{
					gi.error("%s: function not in list, can't save game",
						__func__);
					return;
				}

				len = strlen(func->funcStr)+1;
				fwrite (func->funcStr, len, 1, f);
			}

			break;
		case F_MMOVE:

			if (*(byte **)p)
			{
				mmove = GetMmoveByAddress (*(mmove_t **)p);
				if (!mmove)
				{
					gi.error("%s: mmove not in list, can't save game",
						__func__);
					return;
				}

				len = strlen(mmove->mmoveStr)+1;
				fwrite (mmove->mmoveStr, len, 1, f);
			}

			break;
		default:
			break;
	}
}

/* ========================================================= */

/*
 * This function does the dirty
 * work to read the data from a
 * file. The processing of the
 * data is done in the functions
 * below
 */
static void
ReadField(FILE *f, field_t *field, byte *base)
{
	void *p;
	int len;
	int index;
	char funcStr[2048];

	if (field->flags & FFL_SPAWNTEMP)
	{
		return;
	}

	p = (void *)(base + field->ofs);

	switch (field->type)
	{
		case F_INT:
		case F_FLOAT:
		case F_ANGLEHACK:
		case F_VECTOR:
		case F_IGNORE:
		case F_RGBA:
			break;

		case F_LSTRING:
		case F_LRAWSTRING:
			len = *(int *)p;

			if (!len)
			{
				*(char **)p = NULL;
			}
			else
			{
				char *s;

				s = gi.TagMalloc(len + 1, TAG_LEVEL);
				if (!s)
				{
					gi.error("%s: can't allocate string field", __func__);
					return;
				}

				if (fread(s, len, 1, f) != 1)
				{
					gi.error("%s: can't read string field", __func__);
					return;
				}

				s[len] = 0;
				*(char **)p = s;
			}

			break;
		case F_GSTRING:
			len = *(int *)p;
			if (!len)
				*(char **)p = NULL;
			else
			{
				*(char **)p = (char *)gi.TagMalloc (len, TAG_GAME);
				fread (*(char **)p, len, 1, f);
			}
			break;
		case F_EDICT:
			index = *(int *)p;

			if (index == -1)
			{
				*(edict_t **)p = NULL;
			}
			else
			{
				*(edict_t **)p = &g_edicts[index];
			}

			break;
		case F_CLIENT:
			index = *(int *)p;

			if (index == -1)
			{
				*(gclient_t **)p = NULL;
			}
			else
			{
				*(gclient_t **)p = &game.clients[index];
			}

			break;
		case F_ITEM:
			index = *(int *)p;

			if (index == -1)
			{
				*(gitem_t **)p = NULL;
			}
			else
			{
				*(gitem_t **)p = &itemlist[index];
			}

			break;
		case F_FUNCTION:
			len = *(int *)p;

			if (!len)
			{
				*(byte **)p = NULL;
			}
			else
			{
				if (len > sizeof(funcStr))
				{
					gi.error("%s: function name is longer than buffer (%i chars)",
							__func__, (int)sizeof(funcStr));
					return;
				}

				if (fread (funcStr, len, 1, f) != 1)
				{
					gi.error("%s: can't get function name", __func__);
					return;
				}

				funcStr[sizeof(funcStr) - 1] = 0;

				if ( !(*(byte **)p = FindFunctionByName (funcStr)) )
				{
					gi.error("%s: function %s not found in table, can't load game",
						__func__, funcStr);
				}

			}
			break;
		case F_MMOVE:
			len = *(int *)p;

			if (!len)
			{
				*(byte **)p = NULL;
			}
			else
			{
				if (len > sizeof(funcStr))
				{
					gi.error("%s: mmove name is longer than buffer (%i chars)",
							__func__, (int)sizeof(funcStr));
					return;
				}

				if (fread(funcStr, len, 1, f) != 1)
				{
					gi.error("%s: can't get move name", __func__);
					return;
				}

				funcStr[sizeof(funcStr) - 1] = 0;

				if ( !(*(mmove_t **)p = FindMmoveByName (funcStr)) )
				{
					gi.error("%s: mmove %s not found in table, can't load game",
						__func__, funcStr);
				}
			}
			break;

		default:
			gi.error("%s: unknown field type", __func__);
	}
}

/* ========================================================= */

/*
 * Write the client struct into a file.
 */
static void
WriteClient(FILE *f, gclient_t *client)
{
	field_t *field;
	gclient_t temp;

	/* all of the ints, floats, and vectors stay as they are */
	temp = *client;

	/* change the pointers to indexes */
	for (field = clientfields; field->name; field++)
	{
		WriteField1(f, field, (byte *)&temp);
	}

	/* write the block */
	fwrite(&temp, sizeof(temp), 1, f);

	/* now write any allocated data following the edict */
	for (field = clientfields; field->name; field++)
	{
		WriteField2(f, field, (byte *)client);
	}
}

/*
 * Read the client struct from a file
 */
static void
ReadClient(FILE *f, gclient_t *client, short save_ver)
{
	field_t *field;

	if (fread(client, sizeof(*client), 1, f) != 1)
	{
		fclose(f);
		gi.error("%s: can't read client", __func__);
		return;
	}

	for (field = clientfields; field->name; field++)
	{
		if (field->save_ver <= save_ver)
		{
			ReadField(f, field, (byte *)client);
		}
	}

	if (save_ver < 3)
	{
		InitClientResp(client);
	}
}

/* ========================================================= */

/*
 * Writes the game struct into
 * a file. This is called whenever
 * the game goes to a new level or
 * the user saves the game. The saved
 * information consists of:
 * - cross level data
 * - client states
 * - help computer info
 */
void
WriteGame(const char *filename, qboolean autosave)
{
	savegameHeader_t sv;
	FILE *f;
	int i;
	PerEffectsBuffer_t	*peffect;

	if (!autosave)
	{
		SaveClientData();
	}

	f = Q_fopen(filename, "wb");

	if (!f)
	{
		gi.error("%s: Couldn't open %s", __func__, filename);
		return;
	}

	/* Savegame identification */
	memset(&sv, 0, sizeof(sv));

	Q_strlcpy(sv.ver, SAVEGAMEVER, sizeof(sv.ver) - 1);
	Q_strlcpy(sv.game, GAMEVERSION, sizeof(sv.game) - 1);
	Q_strlcpy(sv.os, YQ2OSTYPE, sizeof(sv.os) - 1);
	Q_strlcpy(sv.arch, YQ2ARCH, sizeof(sv.arch) - 1);

	fwrite(&sv, sizeof(sv), 1, f);

	game.autosaved = autosave;
	fwrite(&game, sizeof(game), 1, f);
	game.autosaved = false;

	for (i = 0; i < game.maxclients; i++)
	{
		WriteClient(f, &game.clients[i]);
	}

	SaveScripts(f, true);

	// this is a bit bogus - search through the client effects and kill all the FX_PLAYER_EFFECTS before saving, since they will be re-created
	// upon players re-joining the game after a load anyway.
	peffect = (PerEffectsBuffer_t*) gi.Persistant_Effects_Array;
	for (i=0; i<MAX_PERSISTANT_EFFECTS; i++, peffect++)
	{
		if (peffect->fx_num == FX_PLAYER_PERSISTANT)
			peffect->numEffects = 0;
	}

	// save all the current persistant effects
	fwrite (gi.Persistant_Effects_Array, (sizeof(PerEffectsBuffer_t) * MAX_PERSISTANT_EFFECTS), 1, f);

	fclose(f);

	// this is a bit bogus - search through the client effects and renable all FX_PLAYER_EFFECTS
	peffect = (PerEffectsBuffer_t*) gi.Persistant_Effects_Array;
	for (i=0; i<MAX_PERSISTANT_EFFECTS; i++, peffect++)
	{
		if (peffect->fx_num == FX_PLAYER_PERSISTANT)
			peffect->numEffects = 1;
	}


}

/*
 * Read the game structs from
 * a file. Called when ever a
 * savegames is loaded.
 */
void
ReadGame(const char *filename)
{
	savegameHeader_t sv;
	FILE *f;
	int i;

	short save_ver = 0;

	gi.FreeTags(TAG_GAME);

	f = Q_fopen(filename, "rb");

	if (!f)
	{
		gi.error("%s: Couldn't open %s", __func__, filename);
		return;
	}

	/* Sanity checks */
	if (fread(&sv, sizeof(sv), 1, f) != 1)
	{
		fclose(f);
		gi.error("%s: can't read save file", __func__);
		return;
	}

	static const struct {
		const char* verstr;
		int vernum;
	} version_mappings[] = {
		{"YQ2-1", 1},
		{"YQ2-2", 2},
		{"YQ2-3", 3},
		{"YQ2-4", 4},
		{"YQ2-5", 5},
		{"YQ2-6", 6},
	};

	for (i=0; i < sizeof(version_mappings)/sizeof(version_mappings[0]); ++i)
	{
		if (strcmp(version_mappings[i].verstr, sv.ver) == 0)
		{
			save_ver = version_mappings[i].vernum;
			break;
		}
	}

	if (save_ver == 0) // not found in mappings table
	{
		fclose(f);
		gi.error("Savegame from an incompatible version.\n");
		return;
	}
	else if (save_ver == 1)
	{
		if (strcmp(sv.game, GAMEVERSION) != 0)
		{
			fclose(f);
			gi.error("Savegame from another game.so.\n");
			return;
		}
		else if (strcmp(sv.os, OSTYPE_1) != 0)
		{
			fclose(f);
			gi.error("Savegame from another os.\n");
			return;
		}

#ifdef _WIN32
		/* Windows was forced to i386 */
		if (strcmp(sv.arch, "i386") != 0)
		{
			fclose(f);
			gi.error("Savegame from another architecture.\n");
			return;
		}
#else
		if (strcmp(sv.arch, ARCH_1) != 0)
		{
			fclose(f);
			gi.error("Savegame from another architecture.\n");
			return;
		}
#endif
	}
	else // all newer savegame versions
	{
		if (strcmp(sv.game, GAMEVERSION) != 0)
		{
			fclose(f);
			gi.error("Savegame from another game.so.\n");
			return;
		}
		else if (strcmp(sv.os, YQ2OSTYPE) != 0)
		{
			fclose(f);
			gi.error("Savegame from another os.\n");
			return;
		}
		else if (strcmp(sv.arch, YQ2ARCH) != 0)
		{
#if defined(_WIN32) && (defined(__i386__) || defined(_M_IX86))
			// before savegame version "YQ2-4" (and after version 1),
			// the official Win32 binaries accidentally had the YQ2ARCH "AMD64"
			// instead of "i386" set due to a bug in the Makefile.
			// This quirk allows loading those savegames anyway
			if (save_ver >= 4 || strcmp(sv.arch, "AMD64") != 0)
#endif
			{
				fclose(f);
				gi.error("Savegame from another architecture.\n");
				return;
			}
		}
	}

	/* we should not trust this value from savegames */
	int num_items = game.num_items;

	if (fread(&game, sizeof(game), 1, f) != 1)
	{
		fclose(f);
		gi.error("%s: can't read game", __func__);
		return;
	}

	/* initialize entities and clients arrays */
	InitAllocations();

	game.num_items = num_items;

	for (i = 0; i < game.maxclients; i++)
	{
		ReadClient(f, &game.clients[i], save_ver);
	}

	LoadScripts(f, true);

	fclose(f);
}

/* ========================================================== */

/*
 * Helper function to write the
 * edict into a file. Called by
 * WriteLevel.
 */
static void
WriteEdict(FILE *f, edict_t *ent)
{
	field_t *field;
	edict_t temp;

	/* all of the ints, floats, and vectors stay as they are */
	temp = *ent;

	/* change the pointers to lengths or indexes */
	for (field = fields; field->name; field++)
	{
		WriteField1(f, field, (byte *)&temp);
	}

	/* write the block */
	fwrite(&temp, sizeof(temp), 1, f);

	/* now write any allocated data following the edict */
	for (field = fields; field->name; field++)
	{
		WriteField2(f, field, (byte *)ent);
	}
}

/*
 * Helper function to write the
 * level local data into a file.
 * Called by WriteLevel.
 */
static void
WriteLevelLocals(FILE *f)
{
	field_t *field;
	level_locals_t temp;
	int i;

	/* all of the ints, floats, and vectors stay as they are */
	temp = level;

	/* change the pointers to lengths or indexes */
	for (field = levelfields; field->name; field++)
	{
		WriteField1(f, field, (byte *)&temp);
	}

	for (i = 0; i< level.active_buoys; i++)
	{
		// change the pointers to lengths or indexes
		for (field=bouyfields ; field->name ; field++)
		{
			WriteField1 (f, field, (byte *)&temp.buoy_list[i]);
		}
	}

	/* write the block */
	fwrite(&temp, sizeof(temp), 1, f);

	/* now write any allocated data following the edict */
	for (field = levelfields; field->name; field++)
	{
		WriteField2(f, field, (byte *)&level);
	}

	for (i = 0; i< level.active_buoys; i++)
	{
		// change the pointers to lengths or indexes
		for (field=bouyfields ; field->name ; field++)
		{
			WriteField2 (f, field, (byte *)&level.buoy_list[i]);
		}
	}

}

/*
 * Writes the current level
 * into a file.
 */
void
WriteLevel(const char *filename)
{
	int i;
	edict_t *ent;
	FILE *f;
	PerEffectsBuffer_t	*peffect;

	f = Q_fopen(filename, "wb");

	if (!f)
	{
		gi.error("%s: Couldn't open %s", __func__, filename);
		return;
	}

	/* write out edict size for checking */
	i = sizeof(edict_t);
	fwrite(&i, sizeof(i), 1, f);

	/* write out level_locals_t */
	WriteLevelLocals(f);

	/* write out all the entities */
	for (i = 0; i < globals.num_edicts; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
		{
			continue;
		}

		fwrite(&i, sizeof(i), 1, f);
		WriteEdict(f, ent);
	}

	i = -1;
	fwrite(&i, sizeof(i), 1, f);

	SaveScripts(f, false);

	// this is a bit bogus - search through the client effects and kill all the FX_PLAYER_EFFECTS before saving, since they will be re-created
	// upon players re-joining the game after a load anyway.
	peffect = (PerEffectsBuffer_t*) gi.Persistant_Effects_Array;
	for (i=0; i<MAX_PERSISTANT_EFFECTS; i++, peffect++)
	{
		if (peffect->fx_num == FX_PLAYER_PERSISTANT)
			peffect->numEffects = 0;
	}

	// save all the current persistant effects
	fwrite (gi.Persistant_Effects_Array, (sizeof(PerEffectsBuffer_t) * MAX_PERSISTANT_EFFECTS), 1, f);

	fclose(f);

	// this is a bit bogus - search through the client effects and renable all FX_PLAYER_EFFECTS
	peffect = (PerEffectsBuffer_t*) gi.Persistant_Effects_Array;
	for (i=0; i<MAX_PERSISTANT_EFFECTS; i++, peffect++)
	{
		if (peffect->fx_num == FX_PLAYER_PERSISTANT)
			peffect->numEffects = 1;

	}
}

/* ========================================================== */

/*
 * A helper function to
 * read the edict back
 * into the memory. Called
 * by ReadLevel.
 */
static void
ReadEdict(FILE *f, edict_t *ent)
{
	field_t *field;
	SinglyLinkedList_t msgs;
	void *s;

	msgs = ent->msgQ.msgs;

	s = ent->script;
	if (fread(ent, sizeof(*ent), 1, f) != 1)
	{
		fclose(f);
		gi.error("%s: can't read edict", __func__);
		return;
	}

	ent->script = s;

	ent->msgQ.msgs = msgs;
	ent->last_alert = NULL;

	for (field = fields; field->name; field++)
	{
		ReadField(f, field, (byte *)ent);
	}
}

/*
 * A helper function to
 * read the level local
 * data from a file.
 * Called by ReadLevel.
 */
static void
ReadLevelLocals(FILE *f)
{
	field_t *field;
	int			i;

	if (fread(&level, sizeof(level), 1, f) != 1)
	{
		fclose(f);
		gi.error("%s: can't read level", __func__);
		return;
	}

	for (field = levelfields; field->name; field++)
	{
		ReadField(f, field, (byte *)&level);
	}

	for (i = 0; i< level.active_buoys; i++)
	{
		// change the pointers to lengths or indexes
		for (field=bouyfields ; field->name ; field++)
		{
			ReadField (f, field, (byte *)&level.buoy_list[i]);
		}
	}

	// these are pointers and should be reset.
	level.alert_entity = NULL;
	level.last_alert = NULL;
	for (i=0; i<MAX_ALERT_ENTS; i++)
	{
		level.alertents[i].inuse = false;
		level.alertents[i].prev_alert = NULL;
		level.alertents[i].next_alert = NULL;
	}
}

/*
 * Reads a level back into the memory.
 * SpawnEntities were already called
 * in the same way when the level was
 * saved. All world links were cleared
 * before this function was called. When
 * this function is called, no clients
 * are connected to the server.
 */
void
ReadLevel(const char *filename)
{
	int entnum;
	FILE *f;
	int i;
	edict_t *ent;

	f = Q_fopen(filename, "rb");

	if (!f)
	{
		gi.error("%s: Couldn't open %s", __func__, filename);
		return;
	}

	/* free any dynamic memory allocated by
	   loading the level  base state */
	gi.FreeTags(TAG_LEVEL);

	/* wipe all the entities */
	memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0]));
	globals.num_edicts = maxclients->value + 1;

	/* check edict size */
	if (fread(&i, sizeof(i), 1, f) != 1)
	{
		fclose(f);
		gi.error("%s: can't read edict size", __func__);
		return;
	}

	if (i != sizeof(edict_t))
	{
		fclose(f);
		gi.error("%s: mismatched edict size", __func__);
		return;
	}

	/* load the level locals */
	ReadLevelLocals(f);

	/* load all the entities */
	while (1)
	{
		if (fread(&entnum, sizeof(entnum), 1, f) != 1)
		{
			fclose(f);
			gi.error("%s: failed to read entnum", __func__);
			return;
		}

		if ((entnum < -1) || (entnum >= game.maxentities))
		{
			fclose(f);
			gi.error("%s: entnum out of bounds: %d", __func__, entnum);
			return;
		}

		if (entnum == -1)
		{
			break;
		}

		if (entnum >= globals.num_edicts)
		{
			globals.num_edicts = entnum + 1;
		}

		ent = &g_edicts[entnum];
		ReadEdict(f, ent);

		/* let the server rebuild world links for this ent */
		memset(&ent->area, 0, sizeof(ent->area));

		ent->last_alert = NULL;

		// NOTE NOTE
		// Missiles must be linked in specially.  G_LinkMissile links as a SOLID_NOT, even though the entity is SOLID_BBOX
		if (ent->movetype == MOVETYPE_FLYMISSILE && ent->solid == SOLID_BBOX)
		{
			G_LinkMissile(ent);
		}
		else
		{
			gi.linkentity(ent);
		}

		// Force the monsters just loaded to point at the right anim.

		if ((ent->classID > 0) && (!Cid_init[ent->classID]) && (ent->classID < NUM_CLASSIDS))	 	// Need to call once per level that item is on
		{
			classStaticsInits[ent->classID]();
			Cid_init[ent->classID] = -1;
		}

		if ( ((ent->classname) && (*ent->classname)) && strcmp(ent->classname, "player") && ent->classID && classStatics[ent->classID].resInfo && ent->curAnimID)
			SetAnim(ent, ent->curAnimID);
	}

	LoadScripts(f, false);

	// Load up all the persistant effects and fire them off.

	fread (gi.Persistant_Effects_Array, (sizeof(PerEffectsBuffer_t) * MAX_PERSISTANT_EFFECTS), 1, f);
	G_ClearPersistantEffects();

	fclose(f);

	/* mark all clients as unconnected */
	for (i = 0; i < maxclients->value; i++)
	{
		ent = &g_edicts[i + 1];
		ent->client = game.clients + i;
		ent->client->pers.connected = false;
		SetupPlayerinfo(ent);
		playerExport->PlayerBasicAnimReset(&ent->client->playerinfo);
	}

	/* do any load time things at this point */
	for (i = 0; i < globals.num_edicts; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
		{
			continue;
		}

		/* fire any cross-level triggers */
		if (ent->classname)
		{
			if (strcmp(ent->classname, "target_crosslevel_target") == 0)
			{
				ent->nextthink = level.time + ent->delay;
			}
		}
	}
}
