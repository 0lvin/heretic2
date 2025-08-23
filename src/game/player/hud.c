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
 * HUD, deathmatch scoreboard, help computer and intermission stuff.
 *
 * =======================================================================
 */

#include "../header/local.h"
#include "../common/h2rand.h"
#include "../header/g_playstats.h"
#include "../header/g_itemstats.h"

qboolean PossessCorrectItem(edict_t *ent, gitem_t *item);

/*
 * ======================================================================
 *
 * INTERMISSION
 *
 * ======================================================================
 */

void
MoveClientToIntermission(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	if (deathmatch->value || coop->value)
	{
		ent->client->playerinfo.showscores = true;
	}

	/*
	 * set ps.pmove.origin is not required as server uses ent.origin instead
	 */
	VectorCopy(level.intermission_origin, ent->s.origin);
	VectorCopy(level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_INTERMISSION;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	/* clean up powerup info */
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->invisible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;
	ent->client->quadfire_framenum = 0;
	ent->client->trap_blew_up = false;
	ent->client->trap_time = 0;
	ent->client->ir_framenum = 0;
	ent->client->nuke_framenum = 0;
	ent->client->double_framenum = 0;

	ent->client->ps.rdflags &= ~RDF_IRGOGGLES;


	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	gi.linkentity(ent);

	/* add the layout */
	if (deathmatch->value || coop->value)
	{
		DeathmatchScoreboardMessage(ent, NULL);
		gi.unicast(ent, true);
	}
}

void
BeginIntermission(edict_t *targ)
{
	int i;
	edict_t *ent;

	if (!targ)
	{
		return;
	}

	if (level.intermissiontime)
	{
		return; /* already activated */
	}

	game.autosaved = false;

	/* respawn any dead clients */
	for (i = 0; i < maxclients->value; i++)
	{
		edict_t *client;

		client = g_edicts + 1 + i;

		if (!client->inuse)
		{
			continue;
		}

		if (client->health <= 0)
		{
			respawn(client);
		}

		/* Save third person view */
		client->client->playerinfo.pers.chasetoggle = client->client->chasetoggle;
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	if (!deathmatch->value)
	{
		level.exitintermission = 1; /* go immediately to the next level */
		return;
	}

	level.exitintermission = 0;

	/* find an intermission spot */
	ent = G_Find(NULL, FOFS(classname), "info_player_intermission");

	if (!ent)
	{
		/* the map creator forgot to put in an intermission point... */
		ent = G_Find(NULL, FOFS(classname), "info_player_start");

		if (!ent)
		{
			ent = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
		}
	}
	else
	{
		/* chose one of four spots */
		i = randk() & 3;

		while (i--)
		{
			ent = G_Find(ent, FOFS(classname), "info_player_intermission");

			if (!ent) /* wrap around the list */
			{
				ent = G_Find(ent, FOFS(classname), "info_player_intermission");
			}
		}
	}

	VectorCopy(ent->s.origin, level.intermission_origin);
	VectorCopy(ent->s.angles, level.intermission_angle);

	/* In fact1 the intermission collides
	   with an area portal, resulting in
	   clutterings */
	if (!Q_stricmp(level.mapname, "fact1"))
	{
		level.intermission_origin[0] = 1037.0;
		level.intermission_origin[1] = 1100.0;
		level.intermission_origin[2] = 222.0;
	}

	/* move all clients to the intermission point */
	for (i = 0; i < maxclients->value; i++)
	{
		edict_t *client;

		client = g_edicts + 1 + i;

		if (!client->inuse)
		{
			continue;
		}

		MoveClientToIntermission(client);
	}
}


/*
==================
DeathmatchScoreboardMessage

==================
*/
#define MAX_STRING_SIZE 1400

typedef struct
{
	int			sorted;
	int			scores;
} team_sort_t;

typedef struct
{
	char		teamname[200];
	int			teamscore;
	int			count_for_team;
	team_sort_t	team_sort[MAX_CLIENTS];
} team_scores_t;

void
DeathmatchScoreboardMessage(edict_t *ent, edict_t *killer)
{
	char entry[MAX_STRING_SIZE];
	char value[512];
	char string[MAX_STRING_SIZE];
	int stringlength;
	int i, j, k, z;
	int sorted[MAX_CLIENTS];
	int sortedscores[MAX_CLIENTS];
	int score, total, total_team, max_team_display;
	int x, y;
	qboolean bubble;
	gclient_t *cl;
	edict_t	*cl_ent;
	char *p;
	team_scores_t team_scores[MAX_CLIENTS];
	team_scores_t temp_point;


	if (!ent) /* killer can be NULL */
	{
		return;
	}

	string[0] = 0;
	stringlength = 0;
	total = 0;

	// sort the clients by score and team if we are playing team play.
	// then resort them by team score.
	if ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))
	{
		// ensure we have an empty table
		memset(team_scores,0,sizeof(team_scores));
		total_team = 0;

		for(i = 0; i < game.maxclients; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;

			// determine score and team type
			score = game.clients[i].resp.score;
			strcpy(value, Info_ValueForKey (cl_ent->client->playerinfo.pers.userinfo, "skin"));

			if (!value[0])
				continue;

			p = strchr(value, '/');

			if (p== NULL)
				p=&value[0];
			else
			// didn't find a team name
			if ((int)(dmflags->value) & DF_SKINTEAMS)
				p++;
			else
			{
				*p = 0;
				p = &value[0];
			}

			// now find a place in the team list to insert it
			for(j = 0; j<total_team;j++)
			{
				// is it the same as our current one ?
				if (!Q_stricmp(team_scores[j].teamname, p))
				{
					break;
				}
			}

			// find the position within the team mates array we should be, given score
			for(k=0; k<team_scores[j].count_for_team;k++)
			{
				if (team_scores[j].team_sort[k].scores < score)
					break;
			}

			// shuffle all the othe scores down if they need to be
			for(x = team_scores[j].count_for_team ; x >k ; x--)
				team_scores[j].team_sort[x] = team_scores[j].team_sort[x-1];

			// insert us into this team/sorted player slot within the team structure
			strcpy(team_scores[j].teamname, p);
			team_scores[j].teamscore +=	score;
			team_scores[j].team_sort[k].scores = score;
			team_scores[j].team_sort[k].sorted = i;
			team_scores[j].count_for_team++;

			if (j==total_team)
				total_team++;

		}

		//determine how many of each team gets displayed
		if (total_team)
			max_team_display = 10/total_team;
		else
			max_team_display = 0;

		if (max_team_display < 1)
			max_team_display = 1;

		// now order the teams into team score order - nasty little bubble sort here
		do
		{
			bubble = false;
			for (i=0; i<total_team-1; i++)
			{
				if (team_scores[i].teamscore < team_scores[i+1].teamscore)
				{
					bubble = true;
					temp_point = team_scores[i];
					team_scores[i] = team_scores[i+1];
					team_scores[i+1] = temp_point;
				}

			}
		} while (bubble);

		// now display the data
		if (total_team > 10)
			total_team = 10;

		y = 32;
		for(i = 0, k = 0; i < total_team; i++)
		{
			x = (k >= 5) ? 180 : 0;
			if (k == 5)
				y = 32;

			Com_sprintf (entry, sizeof(entry), "tm %i %i %i %s ",x,y, team_scores[i].teamscore, team_scores[i].teamname);
			j = strlen(entry);
			if (stringlength + j > MAX_STRING_SIZE)
				break;

			strcpy (string + stringlength, entry);
			stringlength += j;
			y += 16;

			for (j = 0; j < max_team_display; j++)
			{
				// don't try and print more than there are for this team
				if (j >= team_scores[i].count_for_team)
					continue;

				x = (k >= 5) ? 180 : 0;
				cl = &game.clients[team_scores[i].team_sort[j].sorted];
				cl_ent = g_edicts + 1 + team_scores[i].team_sort[j].sorted;
				// Send the layout.
				Com_sprintf (entry, sizeof(entry), "client %i %i %i %i %i %i ",
					x, y, team_scores[i].team_sort[j].sorted,team_scores[i].team_sort[j].scores , cl->ping, (level.framenum - cl->resp.enterframe) / 600);
				z = strlen(entry);
				if (stringlength + z > MAX_STRING_SIZE)
					break;

				strcpy (string + stringlength, entry);
				stringlength += z;
				y += 32;
				k++;
				if (k == 5)
					y = 32;
			}
			y += 8;
		}
	}
	// Sort the clients by score - for normal deathmatch play.
	else
	{
		for(i = 0; i < game.maxclients; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;

			score = game.clients[i].resp.score;
			for(j = 0; j < total; j++)
			{
				if (score > sortedscores[j])
					break;
			}
			for(k = total; k > j; k--)
			{
				sorted[k] = sorted[k - 1];
				sortedscores[k] = sortedscores[k - 1];
			}
			sorted[j] = i;
			sortedscores[j] = score;
			total++;
		}

		if (total > 12)
		{
			total = 12;
		}
		// now display the data

		y = 32;
		for(i = 0; i < total; i++)
		{
			cl = &game.clients[sorted[i]];
			cl_ent = g_edicts + 1 + sorted[i];
			x = (i >= 6) ? 160 : 0;
			if (i == 6)
				y = 32;

			// Send the layout.
			Com_sprintf (entry, sizeof(entry), "client %i %i %i %i %i %i ",
				x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe) / 600);
			j = strlen(entry);
			if (stringlength + j > MAX_STRING_SIZE)
			{
				break;
			}
			strcpy (string + stringlength, entry);
			stringlength += j;
			y += 32;
		}
	}

	// Print level name and exit rules.
	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}

/*
 * Draw help computer.
 */
void
HelpComputerMessage(edict_t *ent)
{
	char string[1024];
	const char *sk;

	if (!ent)
	{
		return;
	}

	if (skill->value == SKILL_EASY)
	{
		sk = LocalizationUIMessage("$m_easy", "easy");
	}
	else if (skill->value == SKILL_MEDIUM)
	{
		sk = LocalizationUIMessage("$m_medium", "medium");
	}
	else if (skill->value == SKILL_HARD)
	{
		sk = LocalizationUIMessage("$m_hard", "hard");
	}
	else
	{
		sk = LocalizationUIMessage("$m_nightmare", "hard+");
	}

	/* send the layout */
	Com_sprintf(string, sizeof(string),
			"xv 32 yv 8 picn help " /* background */
			"xv 202 yv 12 string2 \"%s\" " /* skill */
			"xv 0 yv 24 cstring2 \"%s\" " /* level name */
			"xv 0 yv 54 cstring2 \"%s\" " /* help 1 */
			"xv 0 yv 110 cstring2 \"%s\" " /* help 2 */
			"xv 50 yv 164 string2 \" %-9s %-8s %-9s\" "
			"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ",
			sk,
			level.level_name,
			LocalizationMessage(game.helpmessage1, NULL),
			LocalizationMessage(game.helpmessage2, NULL),
			LocalizationUIMessage("$g_pc_kills", "kills"),
			LocalizationUIMessage("$g_pc_goals", "goals"),
			LocalizationUIMessage("$g_pc_secrets", "secrets"),
			level.killed_monsters, level.total_monsters,
			level.found_goals, level.total_goals,
			level.found_secrets, level.total_secrets);

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
	gi.unicast(ent, true);
}

/*
 * Display the current help message
 */
void
InventoryMessage(edict_t *ent)
{
#if 0
	int i;

	if (!ent)
	{
		return;
	}

	gi.WriteByte(svc_inventory);

	for (i = 0; i < MAX_ITEMS; i++)
	{
		gi.WriteShort(ent->client->pers.inventory[i]);
	}
#endif
}

/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void
DeathmatchScoreboard(edict_t *ent)
{
	DeathmatchScoreboardMessage(ent, ent->enemy);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
	if (!deathmatch->value)
		return;

	if (ent->client->playerinfo.showscores)
	{
		ent->client->playerinfo.showscores = false;
		return;
	}

	ent->client->playerinfo.showscores = true;
	DeathmatchScoreboard (ent);
}

//=======================================================================

/*
===============
G_GetShrineTime
===============
*/

short
GetShrineTime(float time)
{
	float		duration;
	short		result;

	duration = time - level.time;
	if (duration < 0.0)
	{
		return(0);
	}
	result = (short)ceil(duration);
	return(result);
}

static char *healthicons[2] =
{
	"icons/i_health.m8",
	"icons/i_health2.m8",
};

/* ======================================================================= */

void
G_SetStats(edict_t *ent)
{
	int					i, count;
	gitem_t *item;
	gclient_t			*pi;
	player_state_t		*ps;
	player_persistant_t	*pers;
	float				time;

	if (!ent)
	{
		return;
	}

	pi = ent->client;
	ps = &ent->client->ps;
	pers = &ent->client->playerinfo.pers;

	// ********************************************************************************************
	// Frags
	// ********************************************************************************************

	ps->stats[STAT_FRAGS] = pi->resp.score;

	// ********************************************************************************************
	// Health.
	// ********************************************************************************************

	ps->stats[STAT_HEALTH_ICON] = gi.imageindex(healthicons[Q_ftol(level.time * 2) & 1]);
	ps->stats[STAT_HEALTH] = ent->health;

	// ********************************************************************************************
	// Weapon / defence.
	// ********************************************************************************************

	ps->stats[STAT_WEAPON_ICON] = gi.imageindex(pers->weapon->icon);
	if (pers->defence)
	{
		ps->stats[STAT_DEFENCE_ICON] = gi.imageindex(pers->defence->icon);
	}

	// ********************************************************************************************
	// Weapon ammo.
	// ********************************************************************************************

	if (pers->weapon->ammo && pers->weapon->count_width)
	{
		item=FindItem(pers->weapon->ammo);
		ps->stats[STAT_AMMO_ICON] = gi.imageindex(item->icon);
		ps->stats[STAT_AMMO] = pers->inventory[ITEM_INDEX(item)];
	}
	else
	{
		ps->stats[STAT_AMMO_ICON] = 0;
	}

	// ********************************************************************************************
	// Offensive mana.
	// ********************************************************************************************

	ps->stats[STAT_OFFMANA_ICON] = gi.imageindex("icons/green-mana");
	ps->stats[STAT_OFFMANA_BACK] = gi.imageindex("icons/green-mana2");
	item = FindItem("Off-mana");
	ps->stats[STAT_OFFMANA] = (pers->inventory[ITEM_INDEX(item)] * 100) / MAX_OFF_MANA;
	if (ps->stats[STAT_OFFMANA] < 0)
	{
		ps->stats[STAT_OFFMANA] = 0;
	}

	// ********************************************************************************************
	// Defensive mana.
	// ********************************************************************************************

	ps->stats[STAT_DEFMANA_ICON] = gi.imageindex("icons/blue-mana");
	ps->stats[STAT_DEFMANA_BACK] = gi.imageindex("icons/blue-mana2");
	item = FindItem("Def-mana");
	ps->stats[STAT_DEFMANA] = (pers->inventory[ITEM_INDEX(item)] * 100) / MAX_DEF_MANA;
	if (ps->stats[STAT_DEFMANA] < 0)
	{
		ps->stats[STAT_DEFMANA] = 0;
	}

	// ********************************************************************************************
	// Shrine timers.
	// ********************************************************************************************

	ps->stats[STAT_POWERUP_BACK] = gi.imageindex("icons/powerup2");
	ps->stats[STAT_POWERUP_ICON] = gi.imageindex("icons/powerup");
	ps->stats[STAT_POWERUP_TIMER] = (GetShrineTime(pi->playerinfo.powerup_timer) * 100) / POWERUP_DURATION;
	// Cheating sets the powerup timer to something huge, so let's avoid a crash here.
	if (ps->stats[STAT_POWERUP_TIMER] > 100)
	{
		ps->stats[STAT_POWERUP_TIMER] = 100;
	}

	ps->stats[STAT_LUNG_BACK] =	gi.imageindex("icons/breath2");
	ps->stats[STAT_LUNG_ICON] =	gi.imageindex("icons/breath");
	ps->stats[STAT_LUNG_TIMER] = 0;
	if ((ent->waterlevel > 2) && !(ent->flags & FL_INLAVA))
	{
		// Make negative if we have lungs powerup.
		if (pi->playerinfo.lungs_timer)
		{
			time = pi->playerinfo.lungs_timer + ent->air_finished - level.time;
			if (time > 0)
			{
				ps->stats[STAT_LUNG_TIMER] = -(time * 100) / (HOLD_BREATH_TIME + LUNGS_DURATION);
			}
		}
		else
		{
			time = ent->air_finished - level.time;
			if (time > 0)
			{
				ps->stats[STAT_LUNG_TIMER] = (time * 100) / HOLD_BREATH_TIME;
			}
		}
	}

	// ********************************************************************************************
	// Armour items.
	// ********************************************************************************************

	ps->stats[STAT_ARMOUR_ICON] = 0;
	ps->stats[STAT_ARMOUR] = 0;
	if (pers->armortype == ARMOR_TYPE_SILVER)
	{
		ps->stats[STAT_ARMOUR_ICON] = gi.imageindex("icons/arm_silver");
		ps->stats[STAT_ARMOUR] = (pi->playerinfo.pers.armor_count * 100) / MAX_SILVER_ARMOR;
	}
	if (pers->armortype == ARMOR_TYPE_GOLD)
	{
		ps->stats[STAT_ARMOUR_ICON] = gi.imageindex("icons/arm_gold");
		ps->stats[STAT_ARMOUR] = (pi->playerinfo.pers.armor_count * 250) / MAX_GOLD_ARMOR;
	}

	// ********************************************************************************************
	// Puzzle items.
	// ********************************************************************************************

	ps->stats[STAT_PUZZLE_ITEM1] = 0;
	ps->stats[STAT_PUZZLE_ITEM2] = 0;
	ps->stats[STAT_PUZZLE_ITEM3] = 0;
	ps->stats[STAT_PUZZLE_ITEM4] = 0;

	// Scan through inventory to handle puzzle pieces.

	item = itemlist;
	count = STAT_PUZZLE_ITEM1;
	ps->stats[STAT_PUZZLE_COUNT] = 0;
	for(i = 0; i < MAX_ITEMS; i++, item++)
	{
		if (i >= game.num_items)
		{
			break;
		}

		if ((item->flags & IT_PUZZLE) && pers->inventory[i])
		{
			 if(count > STAT_PUZZLE_ITEM4)
			 {
				break;
			 }
			 else
			 {
				ps->stats[count] = gi.imageindex(item->icon);
				ps->stats[STAT_PUZZLE_COUNT]++;
				if (PossessCorrectItem(ent, item))
				{
					ps->stats[count] |= 0x8000;
				}
				count++;
			 }
		}
	}

	// ********************************************************************************************
	// Layouts.
	// ********************************************************************************************

	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	/* Inventory gets activated when player is in a use puzzle trigger field. */
	if (ent->target_ent)
	{
		if (!strcmp(ent->target_ent->classname, "trigger_playerusepuzzle"))
		{
			ps->stats[STAT_LAYOUTS] |= 4;
		}
	}

	if (ent->client->playerinfo.showpuzzleinventory)
	{
		/* Show puzzle inventory. */
		ps->stats[STAT_LAYOUTS] |= 4;
	}

	if (deathmatch->value)
	{
		if (pers->health <= 0 || level.intermissiontime || ent->client->playerinfo.showscores)
		{
			ps->stats[STAT_LAYOUTS] |= 1;
		}
	}
	else
	{
		if (ent->client->playerinfo.showscores)
		{
			ps->stats[STAT_LAYOUTS] |= 1;
		}
	}
}
