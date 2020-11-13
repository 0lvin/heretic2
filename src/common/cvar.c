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
 * The Quake II CVAR subsystem. Implements dynamic variable handling.
 *
 * =======================================================================
 */

#include "header/common.h"

cvar_t *cvar_vars;


typedef struct
{
	char *old;
	char *new;
} replacement_t;

/* An ugly hack to rewrite CVARs loaded from config.cfg */
replacement_t replacements[] = {
	{"cl_drawfps", "cl_showfps"},
	{"gl_drawentities", "r_drawentities"},
	{"gl_drawworld", "r_drawworld"},
	{"gl_fullbright", "r_fullbright"},
	{"gl_lerpmodels", "r_lerpmodels"},
	{"gl_lightlevel", "r_lightlevel"},
	{"gl_norefresh", "r_norefresh"},
	{"gl_novis", "r_novis"},
	{"gl_speeds", "r_speeds"},
	{"gl_clear", "r_clear"},
	{"gl_consolescale", "r_consolescale"},
	{"gl_hudscale", "r_hudscale"},
	{"gl_menuscale", "r_scale"},
	{"gl_customheight", "r_customheight"},
	{"gl_customwidth", "r_customheight"}
};


static qboolean
Cvar_InfoValidate(char *s)
{
	if (strstr(s, "\\"))
	{
		return false;
	}

	if (strstr(s, "\""))
	{
		return false;
	}

	if (strstr(s, ";"))
	{
		return false;
	}

	return true;
}

static cvar_t *
Cvar_FindVar(const char *var_name)
{
	cvar_t *var;
	int i;

	for (var = cvar_vars; var; var = var->next)
	{
		/* An ugly hack to rewrite changed CVARs */
		for (i = 0; i < sizeof(replacements) / sizeof(replacement_t); i++)
		{
			if (!strcmp(var_name, replacements[i].old))
			{
				Com_Printf("cvar %s ist deprecated, use %s instead\n", replacements[i].old, replacements[i].new);

				var_name = replacements[i].new;
			}
		}

		if (!strcmp(var_name, var->name))
		{
			return var;
		}
	}

	return NULL;
}

float
Cvar_VariableValue(char *var_name)
{
	cvar_t *var;

	var = Cvar_FindVar(var_name);

	if (!var)
	{
		return 0;
	}

	return strtod(var->string, (char **)NULL);
}

const char *
Cvar_VariableString(const char *var_name)
{
	cvar_t *var;

	var = Cvar_FindVar(var_name);

	if (!var)
	{
		return "";
	}

	return var->string;
}

/*
 * If the variable already exists, the value will not be set
 * The flags will be or'ed in if the variable exists.
 */
cvar_t *
Cvar_Get(char *var_name, char *var_value, int flags)
{
	cvar_t *var;
	cvar_t **pos;

	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate(var_name))
		{
			Com_Printf("invalid info cvar name\n");
			return NULL;
		}
	}

	var = Cvar_FindVar(var_name);

	if (var)
	{
		var->flags |= flags;
		return var;
	}

	if (!var_value)
	{
		return NULL;
	}

	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate(var_value))
		{
			Com_Printf("invalid info cvar value\n");
			return NULL;
		}
	}

	var = Z_Malloc(sizeof(*var));
	var->name = CopyString(var_name);
	var->string = CopyString(var_value);
	var->modified = true;
	var->value = strtod(var->string, (char **)NULL);

	/* link the variable in */
	pos = &cvar_vars;
	while (*pos && strcmp((*pos)->name, var->name) < 0)
	{
		pos = &(*pos)->next;
	}
	var->next = *pos;
	*pos = var;

	var->flags = flags;

	return var;
}

cvar_t *
Cvar_Set2(char *var_name, char *value, qboolean force)
{
	cvar_t *var;

	var = Cvar_FindVar(var_name);

	if (!var)
	{
		return Cvar_Get(var_name, value, 0);
	}

	if (var->flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!Cvar_InfoValidate(value))
		{
			Com_Printf("invalid info cvar value\n");
			return var;
		}
	}

	if (!force)
	{
		if (var->flags & CVAR_NOSET)
		{
			Com_Printf("%s is write protected.\n", var_name);
			return var;
		}

		if (var->flags & CVAR_LATCH)
		{
			if (var->latched_string)
			{
				if (strcmp(value, var->latched_string) == 0)
				{
					return var;
				}

				Z_Free(var->latched_string);
			}

			else
			{
				if (strcmp(value, var->string) == 0)
				{
					return var;
				}
			}

			if (Com_ServerState())
			{
				Com_Printf("%s will be changed for next game.\n", var_name);
				var->latched_string = CopyString(value);
			}

			else
			{
				var->string = CopyString(value);
				var->value = (float)strtod(var->string, (char **)NULL);

				if (!strcmp(var->name, "game"))
				{
					FS_BuildGameSpecificSearchPath(var->string);
				}
			}

			return var;
		}
	}

	else
	{
		if (var->latched_string)
		{
			Z_Free(var->latched_string);
			var->latched_string = NULL;
		}
	}

	if (!strcmp(value, var->string))
	{
		return var;
	}

	var->modified = true;

	if (var->flags & CVAR_USERINFO)
	{
		userinfo_modified = true;
	}

	Z_Free(var->string);

	var->string = CopyString(value);
	var->value = strtod(var->string, (char **)NULL);

	return var;
}

cvar_t *
Cvar_ForceSet(char *var_name, char *value)
{
	return Cvar_Set2(var_name, value, true);
}

cvar_t *
Cvar_Set(char *var_name, char *value)
{
	return Cvar_Set2(var_name, value, false);
}

cvar_t *
Cvar_FullSet(char *var_name, char *value, int flags)
{
	cvar_t *var;

	var = Cvar_FindVar(var_name);

	if (!var)
	{
		return Cvar_Get(var_name, value, flags);
	}

	var->modified = true;

	if (var->flags & CVAR_USERINFO)
	{
		userinfo_modified = true;
	}

	Z_Free(var->string);

	var->string = CopyString(value);
	var->value = (float)strtod(var->string, (char **)NULL);

	var->flags = flags;

	return var;
}

void
Cvar_SetValue(char *var_name, float value)
{
	char val[32];

	if (value == (int)value)
	{
		Com_sprintf(val, sizeof(val), "%i", (int)value);
	}

	else
	{
		Com_sprintf(val, sizeof(val), "%f", value);
	}

	Cvar_Set(var_name, val);
}

/*
 * Any variables with latched values will now be updated
 */
void
Cvar_GetLatchedVars(void)
{
	cvar_t *var;

	for (var = cvar_vars; var; var = var->next)
	{
		if (!var->latched_string)
		{
			continue;
		}

		Z_Free(var->string);
		var->string = var->latched_string;
		var->latched_string = NULL;
		var->value = strtod(var->string, (char **)NULL);

		if (!strcmp(var->name, "game"))
		{
			FS_BuildGameSpecificSearchPath(var->string);
		}
	}
}

/*
 * Handles variable inspection and changing from the console
 */
qboolean
Cvar_Command(void)
{
	cvar_t *v;

	/* check variables */
	v = Cvar_FindVar(Cmd_Argv(0));

	if (!v)
	{
		return false;
	}

	/* perform a variable print or set */
	if (Cmd_Argc() == 1)
	{
		Com_Printf("\"%s\" is \"%s\"\n", v->name, v->string);
		return true;
	}

	Cvar_Set(v->name, Cmd_Argv(1));
	return true;
}

/*
 * Allows setting and defining of arbitrary cvars from console
 */
void
Cvar_Set_f(void)
{
	char *firstarg;
	int c, flags, i;

	c = Cmd_Argc();

	if ((c != 3) && (c != 4))
	{
		Com_Printf("usage: set <variable> <value> [u / s]\n");
		return;
	}

	firstarg = Cmd_Argv(1);

	/* An ugly hack to rewrite changed CVARs */
	for (i = 0; i < sizeof(replacements) / sizeof(replacement_t); i++)
	{
		if (!strcmp(firstarg, replacements[i].old))
		{
			firstarg = replacements[i].new;
		}
	}

	if (c == 4)
	{
		if (!strcmp(Cmd_Argv(3), "u"))
		{
			flags = CVAR_USERINFO;
		}

		else if (!strcmp(Cmd_Argv(3), "s"))
		{
			flags = CVAR_SERVERINFO;
		}

		else
		{
			Com_Printf("flags can only be 'u' or 's'\n");
			return;
		}

		Cvar_FullSet(firstarg, Cmd_Argv(2), flags);
	}
	else
	{
		Cvar_Set(firstarg, Cmd_Argv(2));
	}
}

/*
 * Appends lines containing "set variable value" for all variables
 * with the archive flag set to true.
 */
void
Cvar_WriteVariables(char *path)
{
	cvar_t *var;
	char buffer[1024];
	FILE *f;

	f = fopen(path, "a");

	for (var = cvar_vars; var; var = var->next)
	{
		if (var->flags & CVAR_ARCHIVE)
		{
			Com_sprintf(buffer, sizeof(buffer), "set %s \"%s\"\n",
					var->name, var->string);
			fprintf(f, "%s", buffer);
		}
	}

	fflush(f);
	fclose(f);
}

void
Cvar_List_f(void)
{
	cvar_t *var;
	int i;

	i = 0;

	for (var = cvar_vars; var; var = var->next, i++)
	{
		if (var->flags & CVAR_ARCHIVE)
		{
			Com_Printf("*");
		}

		else
		{
			Com_Printf(" ");
		}

		if (var->flags & CVAR_USERINFO)
		{
			Com_Printf("U");
		}

		else
		{
			Com_Printf(" ");
		}

		if (var->flags & CVAR_SERVERINFO)
		{
			Com_Printf("S");
		}

		else
		{
			Com_Printf(" ");
		}

		if (var->flags & CVAR_NOSET)
		{
			Com_Printf("-");
		}

		else if (var->flags & CVAR_LATCH)
		{
			Com_Printf("L");
		}

		else
		{
			Com_Printf(" ");
		}

		Com_Printf(" %s \"%s\"\n", var->name, var->string);
	}

	Com_Printf("%i cvars\n", i);
}

qboolean userinfo_modified;

char *
Cvar_BitInfo(int bit)
{
	static char info[MAX_INFO_STRING];
	cvar_t *var;

	info[0] = 0;

	for (var = cvar_vars; var; var = var->next)
	{
		if (var->flags & bit)
		{
			Info_SetValueForKey(info, var->name, var->string);
		}
	}

	return info;
}

/*
 * returns an info string containing
 * all the CVAR_USERINFO cvars
 */
char *
Cvar_Userinfo(void)
{
	return Cvar_BitInfo(CVAR_USERINFO);
}

/*
 * returns an info string containing
 * all the CVAR_SERVERINFO cvars
 */
char *
Cvar_Serverinfo(void)
{
	return Cvar_BitInfo(CVAR_SERVERINFO);
}

/*
 * Reads in all archived cvars
 */
void
Cvar_Init(void)
{
	Cmd_AddCommand("set", Cvar_Set_f);
	Cmd_AddCommand("cvarlist", Cvar_List_f);
}

/*
 * Free list of cvars
 */
void
Cvar_Fini(void)
{
	cvar_t *var;

	for (var = cvar_vars; var;)
	{
		cvar_t *c = var->next;
		Z_Free(var->string);
		Z_Free(var->name);
		Z_Free(var);
        var = c;
	}

	Cmd_RemoveCommand("cvarlist");
	Cmd_RemoveCommand("set");
}

