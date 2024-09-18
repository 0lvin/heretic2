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
	char model_path[MAX_QPATH];
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
} dynamicentity_t;

static dynamicentity_t *dynamicentities;
static int ndynamicentities;

static void
DynamicSpawn(edict_t *self, dynamicentity_t *data)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex(data->model_path);

	VectorCopy(data->mins, self->mins);
	VectorCopy(data->maxs, self->maxs);

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

/*
 * Finds the spawn function for
 * the entity and calls it
 */
void
ED_CallSpawn(edict_t *ent)
{
	spawn_t *s;
	gitem_t *item;
	int i;

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

	if((item = IsItem(ent)))
	{
		SpawnItem(ent, item);

		return;
	}

	/* check normal spawn functions */
	for (s = spawns; s->name; s++)
	{
		if (!strcmp(s->name, ent->classname))
		{
			// found it
			if((s->CID != -1) && !Cid_init[s->CID])	 	// Need to call once per level that item is on
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
	}

	if (dynamicentities && ndynamicentities)
	{
		i = DynamicSpawnSearch(ent->classname);
		if (i >= 0)
		{
			DynamicSpawn(ent, &dynamicentities[i]);

			return;
		}
	}

	gi.dprintf("%s doesn't have a spawn function\n", ent->classname);
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

void
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
				line = DynamicStringParse(line, dynamicentities[curr_pos].model_path, MAX_QPATH, ',');
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
				line = DynamicStringParse(line, dynamicentities[curr_pos].model_path, MAX_QPATH, '|');
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
