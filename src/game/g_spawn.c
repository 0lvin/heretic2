/*
 * Copyright (C) 1997-2001 Id Software, Inc.
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
 * Item spawning.
 *
 * =======================================================================
 */

#include "header/local.h"
#include "savegame/tables/spawnfunc_decs.h"
#include "header/g_skeletons.h"

#define LEG_WAIT_TIME 1
#define MAX_LEGSFRAME 23

#define SPAWNGROW_LIFESPAN 0.3
#define STEPSIZE 18

typedef struct
{
	const char *name;
	void (*spawn)(edict_t *ent);
	int		CID;
} spawn_t;

static spawn_t spawns[] = {
#include "savegame/tables/spawnfunc_list.h"
};

/* Definition of dynamic object */
typedef struct
{
	char classname[MAX_QPATH];
	/* could be up to three models */
	char model_path[MAX_QPATH * 3];
	vec3_t scale;
	char entity_type[MAX_QPATH];
	vec3_t mins;
	vec3_t maxs;
	char noshadow[MAX_QPATH];
	int solidflag;
	float walk_speed;
	float run_speed;
	int speed;
	int lighting;
	int blending;
	char target_sequence[MAX_QPATH];
	int misc_value;
	int no_mip;
	char spawn_sequence[MAX_QPATH];
	char description[MAX_QPATH];
	/* Additional fields */
	vec3_t color;
} dynamicentity_t;

static dynamicentity_t *dynamicentities;
static int ndynamicentities;
static int nstaticentities;

static void
DynamicSpawnSetScale(edict_t *self)
{
	/* copy to other parts if zero */
	if (!st.scale[1])
	{
		st.scale[1] = st.scale[0];
	}

	if (!st.scale[2])
	{
		st.scale[2] = st.scale[0];
	}

	/* Copy to entity scale field */
	VectorCopy(st.scale, self->rrs.scale);
}

static void
DynamicSpawnUpdate(edict_t *self, dynamicentity_t *data)
{
	/* update properties by dynamic properties */
	char model_path[MAX_QPATH * 3];
	char *semicolon, *curr;

	strncpy(model_path, data->model_path, sizeof(model_path));

	/* first model */
	curr = model_path;
	semicolon = strchr(curr, ';');
	if (semicolon)
	{
		*semicolon = 0;
		semicolon ++;
	}

	self->s.modelindex = gi.modelindex(curr);

	/* second model */
	if (semicolon)
	{
		curr = semicolon;
		semicolon = strchr(curr, ';');
		if (semicolon)
		{
			*semicolon = 0;
			semicolon ++;
		}
		self->s.modelindex2 = gi.modelindex(curr);
	}

	/* third model */
	if (semicolon)
	{
		curr = semicolon;
		self->s.modelindex3 = gi.modelindex(curr);
	}

	VectorCopy(data->mins, self->mins);
	VectorCopy(data->maxs, self->maxs);

	/* has updated scale */
	if (st.scale[0] || st.scale[1] || st.scale[2])
	{
		DynamicSpawnSetScale(self);
	}
	else
	{
		VectorCopy(data->scale, self->rrs.scale);
	}
}

void
dynamicspawn_touch(edict_t *self, edict_t *other, cplane_t *plane /* unused */,
		csurface_t *surf /* unused */)
{
	if (!self || !other)
	{
		return;
	}

	if (!self->message || !self->message[0])
	{
		gi.centerprintf(other, "Entity classname: %s", self->classname);
		return;
	}

	gi.centerprintf(other, "Entity description: %s", self->message);
}

static void
DynamicSpawn(edict_t *self, dynamicentity_t *data)
{
	/* All other properties could be updated in DynamicSpawnUpdate */
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;

	/* set message only if it has description */
	if (data->description[0])
	{
		self->message = data->description;
	}
	self->touch = dynamicspawn_touch;

	gi.linkentity(self);
}

static int
DynamicSpawnSearch(const char *name)
{
	int start, end;

	start = 0;
	end = ndynamicentities - 1;

	while (start <= end)
	{
		int i, res;

		i = start + (end - start) / 2;

		res = Q_stricmp(dynamicentities[i].classname, name);
		if (res == 0)
		{
			return i;
		}
		else if (res < 0)
		{
			start = i + 1;
		}
		else
		{
			end = i - 1;
		}
	}

	return -1;
}

static qboolean
Spawn_CheckCoop_MapHacks(edict_t *ent)
{
	if(!coop->value || !ent)
	{
		return false;
	}

	if(!Q_stricmp(level.mapname, "xsewer1"))
	{
		if(ent->classname && !Q_stricmp(ent->classname, "trigger_relay") && ent->target && !Q_stricmp(ent->target, "t3") && ent->targetname && !Q_stricmp(ent->targetname, "t2"))
		{
			return true;
		}
		if(ent->classname && !Q_stricmp(ent->classname, "func_button") && ent->target && !Q_stricmp(ent->target, "t16") && ent->model && !Q_stricmp(ent->model, "*71"))
		{
			ent->message = "Overflow valve maintenance\nhatch A opened.";
			return false;
		}

		if(ent->classname && !Q_stricmp(ent->classname, "trigger_once") && ent->model && !Q_stricmp(ent->model, "*3"))
		{
			ent->message = "Overflow valve maintenance\nhatch B opened.";
			return false;
		}
	}

	return false;
}

static const spawn_t *
StaticSpawnSearch(const char *classname)
{
	int start, end;

	start = 0;
	end = nstaticentities - 1;

	while (start <= end)
	{
		int i, res;

		i = start + (end - start) / 2;

		res = Q_stricmp(spawns[i].name, classname);
		if (res == 0)
		{
			return &spawns[i];
		}
		else if (res < 0)
		{
			start = i + 1;
		}
		else
		{
			end = i - 1;
		}
	}

	return NULL;
}


/*
 * Finds the spawn function for
 * the entity and calls it
 */
void
ED_CallSpawn(edict_t *ent)
{
	const spawn_t *s;
	gitem_t *item;
	int i, dyn_id;

	if (!ent)
	{
		return;
	}

	if (!ent->classname)
	{
		gi.dprintf("ED_CallSpawn: NULL classname\n");
		G_FreeEdict(ent);
		return;
	}

	ent->gravityVector[0] = 0.0;
	ent->gravityVector[1] = 0.0;
	ent->gravityVector[2] = -1.0;

	if (st.health_multiplier <= 0)
	{
			st.health_multiplier = 1.0;
	}

	if (!strcmp(ent->classname, "weapon_nailgun"))
	{
		ent->classname = (FindItem("ETF Rifle"))->classname;
	}

	if (!strcmp(ent->classname, "ammo_nails"))
	{
		ent->classname = (FindItem("Flechettes"))->classname;
	}

	if (!strcmp(ent->classname, "weapon_heatbeam"))
	{
		ent->classname = (FindItem("Plasma Beam"))->classname;
	}

	/* search dynamic definitions */
	dyn_id = -1;
	if (dynamicentities && ndynamicentities)
	{
		dyn_id = DynamicSpawnSearch(ent->classname);

		if (dyn_id >= 0)
		{
			DynamicSpawnUpdate(ent, &dynamicentities[dyn_id]);
		}
	}

	/* No dynamic description */
	if (dyn_id < 0)
	{
		if (st.scale[0])
		{
			DynamicSpawnSetScale(ent);
		}
		else
		{
			VectorSet(ent->rrs.scale, 1.0, 1.0, 1.0);
		}
	}

	/* check item spawn functions */
	if((item = IsItem(ent)))
	{
		SpawnItem(ent, item);

		return;
	}

	/* check normal spawn functions */
	s = StaticSpawnSearch(ent->classname);
	if (s)
	{
		// found it
		if((s->CID != -1) && !Cid_init[s->CID])		// Need to call once per level that item is on
		{
			classStaticsInits[s->CID]();
			Cid_init[s->CID] = -1;
			ent->classID = s->CID;						// Make sure classID is set
		}

		ent->classID = 0;
		if(s->CID != -1)
		{
			ent->classID = s->CID;
		}

		/* found it */
		s->spawn(ent);
		return;
	}

	if (dyn_id >= 0 && dynamicentities[dyn_id].model_path[0])
	{
		/* spawn only if know model */
		DynamicSpawn(ent, &dynamicentities[dyn_id]);

		return;
	}

	/* SiN entity could have model path as model field */
	if (ent->model && (ent->model[0] != '*') && (strlen(ent->model) > 4))
	{
		const char *ext;

		ext = COM_FileExtension(ent->model);
		if(!strcmp(ext, "def"))
		{
			dynamicentity_t self = {0};

			strncpy(self.classname, ent->classname, sizeof(self.classname));
			snprintf(self.model_path, sizeof(self.model_path), "models/%s", ent->model);

			if (gi.FS_LoadFile(self.model_path, NULL) > 4)
			{
				/* Set default size */
				VectorSet(self.mins, -16, -16, -16);
				VectorSet(self.maxs, 16, 16, 16);

				DynamicSpawnUpdate(ent, &self);
				DynamicSpawn(ent, &self);
				return;
			}
		}
	}

	gi.dprintf("%s doesn't have a spawn function\n", ent->classname);
}

char *
ED_NewString(const char *string, qboolean raw)
{
	char *newb;
	int l;

	if (!string)
	{
		return NULL;
	}

	l = strlen(string) + 1;

	newb = gi.TagMalloc(l, TAG_LEVEL);

	if (!raw)
	{
		char *new_p;
		int i;

		new_p = newb;

		for (i = 0; i < l; i++)
		{
			if ((string[i] == '\\') && (i < l - 1))
			{
				i++;

				if (string[i] == 'n')
				{
					*new_p++ = '\n';
				}
				else
				{
					*new_p++ = '\\';
				}
			}
			else
			{
				*new_p++ = string[i];
			}
		}
	}
	else
	{
		/* just copy without convert */
		memcpy(newb, string, l);
	}

	return newb;
}

static unsigned
ED_ParseColorField(const char *value)
{
	/* space means rgba as values */
	if (strchr(value, ' '))
	{
		float v[4] = { 0, 0, 0, 1.0f };
		qboolean is_float = true;
		char color_buffer[16], *tmp;
		int i;

		strncpy(color_buffer, value, sizeof(color_buffer) - 1);
		tmp = color_buffer;

		for (i = 0; i < 4; i++)
		{
			v[i] = (float)strtod(COM_Parse(&tmp), (char **)NULL);

			if (v[i] > 1.0f)
			{
				is_float = false;
			}

			if (!tmp)
			{
				break;
			}
		}

		/* convert to bytes */
		if (is_float)
		{
			for (i = 0; i < 4; i++)
			{
				v[i] *= 255.f;
			}
		}

		return ((byte)v[3]) |
				(((byte)v[2]) << 8) |
				(((byte)v[1]) << 16) |
				(((byte)v[0]) << 24);
	}

	/* integral */
	return atoi(value);
}

/*
 * Takes a key/value pair and sets
 * the binary values in an edict
 */
static void
ED_ParseField(const char *key, const char *value, edict_t *ent)
{
	field_t *f;
	byte *b;
	float v;
	vec3_t vec;

	if (!ent || !value || !key)
	{
		return;
	}

	for (f = fields; f->name; f++)
	{
		if (!(f->flags & FFL_NOSPAWN) && !Q_strcasecmp(f->name, (char *)key))
		{
			/* found it */
			if (f->flags & FFL_SPAWNTEMP)
			{
				b = (byte *)&st;
			}
			else
			{
				b = (byte *)ent;
			}

			switch (f->type)
			{
				case F_LRAWSTRING:
					*(char **)(b + f->ofs) = ED_NewString(value, true);
					break;
				case F_LSTRING:
					*(char **)(b + f->ofs) = ED_NewString(value, false);
					break;
				case F_VECTOR:
					VectorClear(vec);
					sscanf(value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
					((float *)(b + f->ofs))[0] = vec[0];
					((float *)(b + f->ofs))[1] = vec[1];
					((float *)(b + f->ofs))[2] = vec[2];
					break;
				case F_INT:
					*(int *)(b + f->ofs) = (int)strtol(value, (char **)NULL, 10);
					break;
				case F_FLOAT:
					*(float *)(b + f->ofs) = (float)strtod(value, (char **)NULL);
					break;
				case F_ANGLEHACK:
					v = (float)strtod(value, (char **)NULL);
					((float *)(b + f->ofs))[0] = 0;
					((float *)(b + f->ofs))[1] = v;
					((float *)(b + f->ofs))[2] = 0;
					break;
				case F_RGBA:
					*(unsigned *)(b + f->ofs) = ED_ParseColorField(value);
					break;
				case F_IGNORE:
					break;
				default:
					break;
			}

			return;
		}
	}

	gi.dprintf("%s is not a field. Value is %s\n", key, value);
}

/*
 * Parses an edict out of the given string,
 * returning the new position. ed should be
 * a properly initialized empty edict.
 */
static char *
ED_ParseEdict(char *data, edict_t *ent)
{
	qboolean init;
	char keyname[256];
	const char *com_token;

	if (!ent)
	{
		return NULL;
	}

	init = false;
	memset(&st, 0, sizeof(st));
	st.skyautorotate = 1;

	/* go through all the dictionary pairs */
	while (1)
	{
		/* parse key */
		com_token = COM_Parse(&data);

		if (com_token[0] == '}')
		{
			break;
		}

		if (!data)
		{
			gi.error("ED_ParseEntity: EOF without closing brace");
		}

		Q_strlcpy(keyname, com_token, sizeof(keyname));

		/* parse value */
		com_token = COM_Parse(&data);

		if (!data)
		{
			gi.error("ED_ParseEntity: EOF without closing brace");
		}

		if (com_token[0] == '}')
		{
			gi.error("ED_ParseEntity: closing brace without data");
		}

		init = true;

		/* keynames with a leading underscore are
		   used for utility comments, and are
		   immediately discarded by quake */
		if (keyname[0] == '_')
		{
			continue;
		}

		ED_ParseField(keyname, com_token, ent);
	}

	if (!init)
	{
		memset(ent, 0, sizeof(*ent));
	}

	return data;
}

/*
 * Chain together all entities with a matching team field.
 *
 * All but the first will have the FL_TEAMSLAVE flag set.
 * All but the last will have the teamchain field set to the next one
 */
static void
G_FixTeams(void)
{
	edict_t *e, *e2, *chain;
	int i, j;
	int c, c2;

	c = 0;
	c2 = 0;

	for (i = 1, e = g_edicts + i; i < globals.num_edicts; i++, e++)
	{
		if (!e->inuse)
		{
			continue;
		}

		if (!e->team)
		{
			continue;
		}

		if (!strcmp(e->classname, "func_train"))
		{
			if (e->flags & FL_TEAMSLAVE)
			{
				chain = e;
				e->teammaster = e;
				e->teamchain = NULL;
				e->flags &= ~FL_TEAMSLAVE;
				c++;
				c2++;

				for (j = 1, e2 = g_edicts + j;
					 j < globals.num_edicts;
					 j++, e2++)
				{
					if (e2 == e)
					{
						continue;
					}

					if (!e2->inuse)
					{
						continue;
					}

					if (!e2->team)
					{
						continue;
					}

					if (!strcmp(e->team, e2->team))
					{
						c2++;
						chain->teamchain = e2;
						e2->teammaster = e;
						e2->teamchain = NULL;
						chain = e2;
						e2->flags |= FL_TEAMSLAVE;
						e2->movetype = MOVETYPE_PUSH;
						e2->speed = e->speed;
					}
				}
			}
		}
	}

	gi.dprintf("%i teams repaired\n", c);
}

static void
G_FindTeams(void)
{
	edict_t *e, *e2, *chain;
	int i, j;
	int c, c2;

	c = 0;
	c2 = 0;

	for (i = 1, e = g_edicts + i; i < globals.num_edicts; i++, e++)
	{
		if (!e->inuse)
		{
			continue;
		}

		if (!e->team)
		{
			continue;
		}

		if (e->flags & FL_TEAMSLAVE)
		{
			continue;
		}

		chain = e;
		e->teammaster = e;
		c++;
		c2++;

		for (j = i + 1, e2 = e + 1; j < globals.num_edicts; j++, e2++)
		{
			if (!e2->inuse)
			{
				continue;
			}

			if (!e2->team)
			{
				continue;
			}

			if (e2->flags & FL_TEAMSLAVE)
			{
				continue;
			}

			if (!strcmp(e->team, e2->team))
			{
				c2++;
				chain->teamchain = e2;
				e2->teammaster = e;
				chain = e2;
				e2->flags |= FL_TEAMSLAVE;
			}
		}
	}

	G_FixTeams();

	gi.dprintf("%i teams with %i entities.\n", c, c2);
}

/*
 * Creates a server's entity / program execution context by
 * parsing textual entity definitions out of an ent file.
 */
void
SpawnEntities(const char *mapname, char *entities, const char *spawnpoint)
{
	edict_t *ent;
	int inhibit;
	const char *com_token;
	int i;
	float skill_level;

	if (!mapname || !entities || !spawnpoint)
	{
		return;
	}

	skill_level = floor(skill->value);

	if (skill_level < 0)
	{
		skill_level = 0;
	}

	if (skill_level > 3)
	{
		skill_level = 3;
	}

	if (skill->value != skill_level)
	{
		gi.cvar_forceset("skill", va("%f", skill_level));
	}

	SaveClientData();

	ShutdownScripts(false);
	gi.FreeTags(TAG_LEVEL);

	memset(&level, 0, sizeof(level));
	memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0]));

	memset(skeletalJoints, 0, sizeof(skeletalJoints));
	memset(jointNodes, 0, sizeof(jointNodes));
	memset(classStatics, 0, sizeof(classStatics));
	memset(Cid_init, 0, sizeof(Cid_init));

	Q_strlcpy(level.mapname, mapname, sizeof(level.mapname));
	Q_strlcpy(game.spawnpoint, spawnpoint, sizeof(game.spawnpoint));

	/* set client fields on player ents */
	for (i = 0; i < game.maxclients; i++)
	{
		g_edicts[i + 1].client = game.clients + i;
	}

	ent = NULL;
	inhibit = 0;

	/* parse ents */
	while (1)
	{
		/* parse the opening brace */
		com_token = COM_Parse(&entities);

		if (!entities)
		{
			break;
		}

		if (com_token[0] != '{')
		{
			gi.error("ED_LoadFromFile: found %s when expecting {", com_token);
		}

		if (!ent)
		{
			ent = g_edicts;
		}
		else
		{
			ent = G_Spawn();
		}

		entities = ED_ParseEdict(entities, ent);

		/* yet another map hack */
		if (!Q_stricmp(level.mapname, "command") &&
			!Q_stricmp(ent->classname, "trigger_once") &&
			!Q_stricmp(ent->model, "*27"))
		{
			ent->spawnflags &= ~SPAWNFLAG_NOT_HARD;
		}

		/* ahh, the joys of map hacks .. */
		if (!Q_stricmp(level.mapname, "rhangar2") &&
			!Q_stricmp(ent->classname, "func_door_rotating") &&
			ent->targetname && !Q_stricmp(ent->targetname, "t265"))
		{
			ent->spawnflags &= ~SPAWNFLAG_NOT_COOP;
		}

		if (!Q_stricmp(level.mapname, "rhangar2") &&
			!Q_stricmp(ent->classname, "trigger_always") &&
			ent->target && !Q_stricmp(ent->target, "t265"))
		{
			ent->spawnflags |= SPAWNFLAG_NOT_COOP;
		}

		if (!Q_stricmp(level.mapname, "rhangar2") &&
			!Q_stricmp(ent->classname, "func_wall") &&
			!Q_stricmp(ent->model, "*15"))
		{
			ent->spawnflags |= SPAWNFLAG_NOT_COOP;
		}

		/* remove things (except the world) from
		   different skill levels or deathmatch */
		if (ent != g_edicts)
		{
			if (deathmatch->value)
			{
				if (ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH)
				{
					G_FreeEdict(ent);
					inhibit++;
					continue;
				}
			}
			else
			{
				if (((coop->value) && (ent->spawnflags & SPAWNFLAG_NOT_COOP)) ||
					((skill->value == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
					((skill->value == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
					((skill->value >= 2) && (ent->spawnflags & SPAWNFLAG_NOT_HARD))
					)
					{
						G_FreeEdict(ent);
						inhibit++;
						continue;
					}
			}

			// Check if it's a monster and if we're nomonster here...
			if (sv_nomonsters && sv_nomonsters->value && strstr(ent->classname, "monster_"))
			{
				gi.dprintf("monster '%s' not spawned.\n", ent->classname);

				G_FreeEdict(ent);
				inhibit++;
				continue;
			}

			ent->spawnflags &=
				~(SPAWNFLAG_NOT_EASY | SPAWNFLAG_NOT_MEDIUM |
				  SPAWNFLAG_NOT_HARD |
				  SPAWNFLAG_NOT_COOP | SPAWNFLAG_NOT_DEATHMATCH);
		}

		ED_CallSpawn(ent);
	}

	gi.dprintf("%i entities inhibited.\n", inhibit);

	G_FindTeams();
}

typedef struct
{
	char	*level_name;
	int		default_preset;
} eax_level_info_t;

static eax_level_info_t eax_level_info[] = {
	{"ssdocks",			EAX_CITY_AND_SEWERS},
	{"sswarehouse",		EAX_CITY_AND_SEWERS},
	{"sstown",			EAX_CITY_AND_ALLEYS},
	{"sspalace",		EAX_CITY_AND_ALLEYS},
	{"dmireswamp",		EAX_CITY_AND_SEWERS},
	{"andhealer",		EAX_CITY_AND_SEWERS},
	{"andplaza",		EAX_CITY_AND_SEWERS},
	{"andslums",		EAX_CITY_AND_SEWERS},
	{"andacademic",		EAX_CITY_AND_SEWERS},
	{"kellcaves",		EAX_ALL_STONE},
	{"canyon",			EAX_ALL_STONE},
	{"hive1",			EAX_ALL_STONE},
	{"hive2",			EAX_ALL_STONE},
	{"gauntlet",		EAX_ALL_STONE},
	{"hivetrialpit",	EAX_ARENA},
	{"hivepriestess",	EAX_ALL_STONE},
	{"oglemine1",		EAX_ALL_STONE},
	{"oglemine2",		EAX_ALL_STONE},
	{"dungeon",			EAX_CITY_AND_ALLEYS},
	{"cloudhub",		EAX_CITY_AND_ALLEYS},
	{"cloudlabs",		EAX_CITY_AND_ALLEYS},
	{"cloudquarters",	EAX_CITY_AND_ALLEYS},
	{"cloudsanctum",	EAX_CITY_AND_ALLEYS},
	{"tutorial",		EAX_CITY_AND_ALLEYS},
	{"tutorial2",		EAX_CITY_AND_ALLEYS},
	{"dmandoria",		EAX_CITY_AND_ALLEYS},
	{"dmhive",			EAX_ALL_STONE},
	{"dmcanyon",		EAX_ALL_STONE},
	{"dmcloud",			EAX_CITY_AND_ALLEYS},
};

#define MAX_CURRENT_LEVELS (sizeof(eax_level_info) / sizeof(*eax_level_info))

//===================================================================

#if 0
	// cursor positioning
	xl <value>
	xr <value>
	yb <value>
	yt <value>

	// drawing
	statpic <name>
	pic <stat>
	num <fieldwidth> <stat>
	string <stat>

	// control
	if <stat>
	ifeq <stat> <value>
	ifbit <stat> <value>
	endif

#endif

static char *single_statusbar =
"yb	-74 "
"xl 16 "		// green mana
"bar 8 16 60 "

"yb -44 "

"xl	40 "
"pic 4 "		// Weapon

"xl 76 "		// Ammo
"pic 2 "
"am "

"xr -152 "		// Armour
"pic 34 "
"arm "

"xr -112 "
"pic 0 "
"hnum "			// Health

"if 6 "
"yb -44 "
"xr -72 "
"pic 6 "		// Defence
"endif "

"yb	-74 "
"xr -32 "
"bar 11 16 60 "		// blue mana

" yt 16 "

"if 28 "
" xl 32 "
" bar 26 60 16 " 	// Lung time left
"endif "

"if 25 "
" xr -96 "
" bar 23 60 16 "	// Powerup time left
"endif "

"yt	16 "

"xc 0 "				// Inventory Puzzle Item 1
"pici 18 "

"xc 40 "			// Puzzle 2
"pici 19 "

"xc 80 "			// Puzzle 3
"pici 20 "

"xc 120 "			// Puzzle 4
"pici 21 "

"if 31 "
" xl 32 "
" gbar 29 "			// Boss Life Meter
"endif "
;

static char *dm_statusbar =
"yb	-74 "
"xl 16 "		// green mana
"bar 8 16 60 "

"yb -44 "

"xl	40 "
"pic 4 "		// Weapon

"xl 76 "		// Ammo
"pic 2 "
"am "

"xr -152 "		// Armour
"pic 34 "
"arm "

"xr -112 "
"pic 0 "
"hnum "			// Health

"yb -44 "
"xr -72 "
"pic 6 "		// Defence

"yb	-74 "
"xr -32 "
"bar 11 16 60 " // blue mana

" yt 16 "

"if 28 "
" xl 32 "
" bar 26 60 16 " // Lung time left
"endif "

"if 25 "
" xr -96 "
" bar 23 60 16 " // Powerup time left
"endif "

#if	0
"xc 0 "			// Frag
"num 3 15 "
#endif
;

/*QUAKED worldspawn (0 0 0) ? NOBODIES

Only used for the world.

NOBODIES - In DM, no bodies will be left behind by players- for maps with large amounts of visibility

"sky"			environment map name:

	andoria
	desert
	hive
	sky1	- Night Sky
	storm
	swamp
	town

"skyaxis"		vector axis for rotating sky
"skyrotate"		speed of rotation in degrees/second
"sounds"		music cd track number
"gravity"		800 is default gravity
"message"		text to print at user logon
"skinnum"		plague level for corvus: 0-2
"cooptimeout"	time to wait (in seconds) for all clients to have joined a map in coop (default is 0).
"scale"	EAX environment type for this map.

 0 EAX_GENERIC,
 1 EAX_ALL_STONE,
 2 EAX_ARENA,
 3 EAX_CITY_AND_SEWERS,
 4 EAX_CITY_AND_ALLEYS,
 5 EAX_FOREST,
 6 EAX_PSYCHOTIC,

"offensive"		starting offensive weapons (flag bits):

  1		- swordstaff
  2		- fireball
  4		- hellstaff
  8		- magic missile array
  16	- red-rain bow
  32	- sphere of annihlation
  64	- phoenix bow
  128	- mace balls
  256	- firewall

"defensive"		starting defensive weapons (flag bits):

  1		- ring of repulsion
  2		- lightning shield
  4		- teleport
  8		- morph ovum
  16	- meteor barrier

*/

void
SP_worldspawn(edict_t *ent)
{
	int	i;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->inuse = true; /* since the world doesn't use G_Spawn() */
	ent->s.modelindex = 1; /* world model is always index 1 */

	/* --------------- */

	/* reserve some spots for dead
	   player bodies for coop / deathmatch */
	InitBodyQue();

	if((ent->spawnflags & 1) && (deathmatch->value || coop->value))
		level.body_que = -1;

	// Set configstrings for items.

	SetItemNames();

	if (st.nextmap)
		strcpy (level.nextmap, st.nextmap);

	// Make some data visible to the server.

	if (ent->message && ent->message[0])
	{
		gi.configstring (CS_LEVEL_NUMBER, ent->message );
		gi.configstring (CS_NAME, LocalizationMessage(ent->message, NULL));
		strncpy (level.level_name, ent->message, sizeof(level.level_name));
		gi.dprintf("Unique Level Index : %d\n", atoi(ent->message));
	}
	else
	{
		if(ent->text_msg)
			gi.configstring (CS_NAME, ent->text_msg);
		strncpy (level.level_name, level.mapname, sizeof(level.level_name));
		gi.dprintf("Warning : No Unique Level Index\n");
	}

	// this is a tremendous hack, but given the state of the code at this point, there is no other way to do this.
	for (i=0 ; i<MAX_CURRENT_LEVELS;i++)
	{
		// search through all the currently defined world maps, looking for names, so we can set
		// the EAX default sound type for this level.
		if (!Q_stricmp(eax_level_info[i].level_name, level.mapname))
		{
			int eax_preset = -1;

			// keep in sync with openal
			switch (eax_level_info[i].default_preset)
			{
				case EAX_GENERIC: eax_preset = 0; break;
				case EAX_ALL_STONE: eax_preset = 13; break;
				case EAX_ARENA: eax_preset = 9; break;
				case EAX_CITY_AND_SEWERS: eax_preset = 16; break;
				case EAX_CITY_AND_ALLEYS: eax_preset = 14; break;
				case EAX_FOREST: eax_preset = 15; break;
				case EAX_PSYCHOTIC: eax_preset = 25; break;
			}

			gi.cvar_set("s_reverb_preset", va("%i", eax_preset));

			break;
		}
	}

	// if we didn't find it in the current level list, lets just set it to generic
	if (i == MAX_CURRENT_LEVELS)
	{
		gi.cvar_set("s_reverb_preset", "0");
	}

	/* just in case, fix scale */
	VectorClear(ent->rrs.scale);

	if (st.sky && st.sky[0])
	{
		gi.configstring (CS_SKY, st.sky);
	}
	else
	{
		gi.configstring (CS_SKY, "desert");
	}

	gi.configstring (CS_SKYROTATE, va("%f", st.skyrotate) );

	gi.configstring (CS_SKYAXIS, va("%f %f %f",
		st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]) );

	gi.configstring (CS_CDTRACK, va("%i", ent->sounds) );

	gi.configstring (CS_MAXCLIENTS, va("%i", (int)(maxclients->value) ) );

	// Status bar program.

	if (deathmatch->value)
		gi.configstring (CS_STATUSBAR, dm_statusbar);
	else
		gi.configstring (CS_STATUSBAR, single_statusbar);

	// Starting weapons for players entering a coop game.

	level.offensive_weapons=(!st.offensive)?0:st.offensive;
	level.defensive_weapons=(!st.defensive)?0:st.defensive;

	// Save away cooptimeout so it is accessible to the server (SV_) functions.
	{
		char tmp[10];
		Com_sprintf (tmp, sizeof(tmp), "%d", (!st.cooptimeout)?0:st.cooptimeout);
		gi.cvar_set("sv_cooptimeout", tmp);
	}

	//---------------

	// GRAVITY for all games.

	if (!st.gravity)
		gi.cvar_set("sv_gravity", "675");
	else
		gi.cvar_set("sv_gravity", st.gravity);

	// FRICTION for all games.

	sv_friction = gi.cvar ("sv_friction", "1600.0", 0);

	//
	// Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
	//

	// 0 normal
	gi.configstring(CS_LIGHTS+0, "m");

	// 1 FLICKER (first variety)
	gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");

	// 2 SLOW STRONG PULSE
	gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

	// 3 CANDLE (first variety)
	gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

	// 4 FAST STROBE
	gi.configstring(CS_LIGHTS+4, "mamamamamama");

	// 5 GENTLE PULSE 1
	gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");

	// 6 FLICKER (second variety)
	gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");

	// 7 CANDLE (second variety)
	gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");

	// 8 CANDLE (third variety)
	gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

	// 9 SLOW STROBE (fourth variety)
	gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");

	// 10 FLUORESCENT FLICKER
	gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	gi.configstring(CS_LIGHTS+63, "a");
}

static char *
DynamicStringParse(char *line, char *field, int size, char separator)
{
	char *next_section, *current_section;

	/* search line end */
	current_section = line;
	next_section = strchr(line, separator);
	if (next_section)
	{
		*next_section = 0;
		line = next_section + 1;
	}

	/* copy current line state */
	strncpy(field, current_section, size);

	return line;
}

static char *
DynamicIntParse(char *line, int *field)
{
	char *next_section;

	next_section = strchr(line, '|');
	if (next_section)
	{
		*next_section = 0;
		*field = (int)strtol(line, (char **)NULL, 10);
		line = next_section + 1;
	}

	return line;
}

static char *
DynamicFloatParse(char *line, float *field, int size, char separator)
{
	int i;

	for (i = 0; i < size; i++)
	{
		char *next_section, *current_section;

		current_section = line;
		next_section = strchr(line, separator);
		if (next_section)
		{
			*next_section = 0;
			line = next_section + 1;
		}
		field[i] = (float)strtod(current_section, (char **)NULL);
	}
	return line;
}

static char *
DynamicSkipParse(char *line, int size, char separator)
{
	int i;

	for (i = 0; i < size; i++)
	{
		char *next_section;

		next_section = strchr(line, separator);
		if (next_section)
		{
			*next_section = 0;
			line = next_section + 1;
		}
	}
	return line;
}

static int
DynamicSort(const void *p1, const void *p2)
{
	dynamicentity_t *ent1, *ent2;

	ent1 = (dynamicentity_t*)p1;
	ent2 = (dynamicentity_t*)p2;
	return Q_stricmp(ent1->classname, ent2->classname);
}

static void
DynamicSpawnInit(void)
{
	char *buf_ent, *buf_ai, *raw;
	int len_ent, len_ai, curr_pos;

	buf_ent = NULL;
	len_ent = 0;
	buf_ai = NULL;
	len_ai = 0;

	dynamicentities = NULL;
	ndynamicentities = 0;

	/* load the aidata file */
	len_ai = gi.FS_LoadFile("aidata.vsc", (void **)&raw);
	if (len_ai > 1)
	{
		if (len_ai > 4 && !strncmp(raw, "CVSC", 4))
		{
			int i;

			len_ai -= 4;
			buf_ai = malloc(len_ai + 1);
			memcpy(buf_ai, raw + 4, len_ai);
			for (i = 0; i < len_ai; i++)
			{
				buf_ai[i] = buf_ai[i] ^ 0x96;
			}
			buf_ai[len_ai] = 0;
		}
		gi.FS_FreeFile(raw);
	}

	/* load the file */
	len_ent = gi.FS_LoadFile("models/entity.dat", (void **)&raw);
	if (len_ent > 1)
	{
		buf_ent = malloc(len_ent + 1);
		memcpy(buf_ent, raw, len_ent);
		buf_ent[len_ent] = 0;
		gi.FS_FreeFile(raw);
	}

	/* aidata definition lines count */
	if (buf_ai)
	{
		char *curr;

		/* get lines count */
		curr = buf_ai;
		while(*curr)
		{
			size_t linesize = 0;

			linesize = strcspn(curr, "\n\r");
			if (*curr &&  *curr != '\n' && *curr != '\r' && *curr != ',')
			{
				ndynamicentities ++;
			}

			curr += linesize;
			if (curr >= (buf_ai + len_ai))
			{
				break;
			}
			/* skip our endline */
			curr++;
		}
	}

	/* entitiyty definition lines count */
	if (buf_ent)
	{
		char *curr;

		/* get lines count */
		curr = buf_ent;
		while(*curr)
		{
			size_t linesize = 0;

			linesize = strcspn(curr, "\n\r");
			if (*curr && strncmp(curr, "//", 2) &&
				*curr != '\n' && *curr != '\r' && *curr != ';')
			{
				ndynamicentities ++;
			}
			curr += linesize;
			if (curr >= (buf_ent + len_ent))
			{
				break;
			}
			/* skip our endline */
			curr++;
		}
	}

	if (ndynamicentities)
	{
		dynamicentities = gi.TagMalloc(ndynamicentities * sizeof(*dynamicentities), TAG_GAME);
		memset(dynamicentities, 0, ndynamicentities * sizeof(*dynamicentities));
	}
	curr_pos = 0;

	if (buf_ai)
	{
		char *curr;

		/* get lines count */
		curr = buf_ai;
		while(*curr)
		{
			size_t linesize = 0;

			if (curr_pos >= ndynamicentities)
			{
				break;
			}

			/* skip empty */
			linesize = strspn(curr, "\n\r\t ");
			curr += linesize;

			/* mark end line */
			linesize = strcspn(curr, "\n\r");
			curr[linesize] = 0;

			if (*curr &&  *curr != '\n' && *curr != '\r' && *curr != ',')
			{
				char *line, scale[MAX_QPATH];

				line = curr;
				line = DynamicStringParse(line, dynamicentities[curr_pos].classname, MAX_QPATH, ',');
				line = DynamicStringParse(line, dynamicentities[curr_pos].model_path,
					sizeof(dynamicentities[curr_pos].model_path), ',');
				/*
				 * Skipped:
					 * audio file definition
					 * health
					 * basehealth
					 * elasticity
					 * mass
					 * angle speed
				*/
				line = DynamicSkipParse(line, 6, ',');
				line = DynamicFloatParse(line, dynamicentities[curr_pos].mins, 3, ',');
				line = DynamicFloatParse(line, dynamicentities[curr_pos].maxs, 3, ',');
				line = DynamicStringParse(line, scale, MAX_QPATH, ',');
				/* parse to 3 floats */
				DynamicFloatParse(scale, dynamicentities[curr_pos].scale, 3, ' ');
				/*
				 * Ignored fields:
					* active distance
					* attack distance
					* jump attack distance
					* upward velocity
					* run speed
					* walk speed
					* attack speed
					* fov
					* X weapon1Offset
					* Y weapon1Offset
					* Z weapon1Offset
					* base damage
					* random damage
					* spread x
					* spread z
					* speed
					* distance
					* X weapon2Offset
					* Y weapon2Offset
					* Z weapon2Offset
					* base damage
					* random damage
					* spread x
					* spread z
					* speed
					* distance
					* X weapon3Offset
					* Y weapon3Offset
					* Z weapon3Offset
					* base damage
					* random damage
					* spread x
					* spread z
					* speed
					* distance
					* min attenuation
					* max attenuation
				 */

				/* Fix path */
				Q_replacebackslash(dynamicentities[curr_pos].model_path);

				/* go to next row */
				curr_pos ++;
			}

			curr += linesize;
			if (curr >= (buf_ai + len_ai))
			{
				break;
			}

			/* skip our endline */
			curr++;
		}
		free(buf_ai);
	}

	/* load definitons count */
	if (buf_ent)
	{
		char *curr;

		/* get lines count */
		curr = buf_ent;
		while(*curr)
		{
			size_t linesize = 0;

			if (curr_pos >= ndynamicentities)
			{
				break;
			}

			/* skip empty */
			linesize = strspn(curr, "\n\r\t ");
			curr += linesize;

			/* mark end line */
			linesize = strcspn(curr, "\n\r");
			curr[linesize] = 0;

			if (*curr && strncmp(curr, "//", 2) &&
				*curr != '\n' && *curr != '\r' && *curr != ';')
			{
				char *line;

				line = curr;
				line = DynamicStringParse(line, dynamicentities[curr_pos].classname, MAX_QPATH, '|');
				line = DynamicStringParse(line, dynamicentities[curr_pos].model_path,
					sizeof(dynamicentities[curr_pos].model_path), '|');
				line = DynamicFloatParse(line, dynamicentities[curr_pos].scale, 3, '|');
				line = DynamicStringParse(line, dynamicentities[curr_pos].entity_type, MAX_QPATH, '|');
				line = DynamicFloatParse(line, dynamicentities[curr_pos].mins, 3, '|');
				line = DynamicFloatParse(line, dynamicentities[curr_pos].maxs, 3, '|');
				line = DynamicStringParse(line, dynamicentities[curr_pos].noshadow, MAX_QPATH, '|');
				line = DynamicIntParse(line, &dynamicentities[curr_pos].solidflag);
				line = DynamicFloatParse(line, &dynamicentities[curr_pos].walk_speed, 1, '|');
				line = DynamicFloatParse(line, &dynamicentities[curr_pos].run_speed, 1, '|');
				line = DynamicIntParse(line, &dynamicentities[curr_pos].speed);
				line = DynamicIntParse(line, &dynamicentities[curr_pos].lighting);
				line = DynamicIntParse(line, &dynamicentities[curr_pos].blending);
				line = DynamicStringParse(line, dynamicentities[curr_pos].target_sequence, MAX_QPATH, '|');
				line = DynamicIntParse(line, &dynamicentities[curr_pos].misc_value);
				line = DynamicIntParse(line, &dynamicentities[curr_pos].no_mip);
				line = DynamicStringParse(line, dynamicentities[curr_pos].spawn_sequence, MAX_QPATH, '|');
				line = DynamicStringParse(line, dynamicentities[curr_pos].description, MAX_QPATH, '|');
				/* Additional field for cover for color from QUAKED */
				line = DynamicFloatParse(line, dynamicentities[curr_pos].color, 3, '|');

				/* Fix path */
				Q_replacebackslash(dynamicentities[curr_pos].model_path);

				/* go to next row */
				curr_pos ++;
			}
			curr += linesize;
			if (curr >= (buf_ent + len_ent))
			{
				break;
			}
			/* skip our endline */
			curr++;
		}

		free(buf_ent);
	}

	/* save last used position */
	ndynamicentities = curr_pos;

	if (!curr_pos)
	{
		return;
	}

	gi.dprintf("Found %d dynamic definitions\n", ndynamicentities);

	/* sort definitions */
	qsort(dynamicentities, ndynamicentities, sizeof(dynamicentity_t), DynamicSort);
}

static int
StaticSort(const void *p1, const void *p2)
{
	spawn_t *ent1, *ent2;

	ent1 = (spawn_t*)p1;
	ent2 = (spawn_t*)p2;
	return Q_stricmp(ent1->name, ent2->name);
}

static void
StaticSpawnInit(void)
{
	const spawn_t *s;

	/* check count of spawn functions */
	for (s = spawns; s->name; s++)
	{
	}

	nstaticentities = s - spawns;

	gi.dprintf("Found %d static definitions\n", nstaticentities);

	/* sort definitions */
	qsort(spawns, nstaticentities, sizeof(spawn_t), StaticSort);
}

void
SpawnInit(void)
{
	StaticSpawnInit();
	DynamicSpawnInit();
}
