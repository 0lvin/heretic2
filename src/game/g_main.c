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
 * Jump in into the game.so and support functions.
 *
 * =======================================================================
 */

#include "header/local.h"
#include "header/g_skeletons.h"
#include "common/arrayed_list.h"
#include "header/g_physics.h"
#include "header/g_volume_effect.h"
#include "common/h2rand.h"
#include "header/g_playstats.h"
#include "player/library/p_anims.h"
#include "common/cl_strings.h"

game_locals_t game;
level_locals_t level;
game_import_t gi;
game_export_t globals;
spawn_temp_t st;

int sm_meat_index;
int snd_fry;
int meansOfDeath;

edict_t *g_edicts;

cvar_t *deathmatch;
cvar_t *coop;
cvar_t *coop_baseq2;	/* treat spawnflags according to baseq2 rules */
cvar_t *coop_pickup_weapons;
cvar_t *coop_elevator_delay;
cvar_t *dmflags;
cvar_t *skill;
cvar_t *fraglimit;
cvar_t *timelimit;
cvar_t *capturelimit;
cvar_t *instantweap;
cvar_t *password;
cvar_t *spectator_password;
cvar_t *needpass;
cvar_t *maxclients;
cvar_t *maxspectators;
cvar_t *maxentities;
cvar_t *g_select_empty;
cvar_t *dedicated;
cvar_t *g_footsteps;
cvar_t *g_monsterfootsteps;
cvar_t *g_fix_triggered;
cvar_t *g_commanderbody_nogod;

cvar_t *filterban;

cvar_t *sv_maxvelocity;
cvar_t *sv_gravity;

cvar_t *sv_rollspeed;
cvar_t *sv_rollangle;
cvar_t *gun_x;
cvar_t *gun_y;
cvar_t *gun_z;

cvar_t *run_pitch;
cvar_t *run_roll;
cvar_t *bob_up;
cvar_t *bob_pitch;
cvar_t *bob_roll;

cvar_t *sv_cheats;

cvar_t *flood_msgs;
cvar_t *flood_persecond;
cvar_t *flood_waitdelay;

cvar_t *sv_maplist;
cvar_t *sv_stopspeed;

cvar_t *gib_on;
cvar_t *g_showlogic;
cvar_t *gamerules;
cvar_t *huntercam;
cvar_t *strong_mines;
cvar_t *randomrespawn;

cvar_t *g_disruptor;

cvar_t *aimfix;
cvar_t *g_machinegun_norecoil;
cvar_t *g_quick_weap;
cvar_t *g_swap_speed;
cvar_t *g_language;
cvar_t *g_itemsbobeffect;
cvar_t *g_start_items;
cvar_t *ai_model_scale;
cvar_t *g_game;

static void G_RunFrame(void);

cvar_t *advancedstaff;
cvar_t *sv_friction;
cvar_t *flood_killdelay;
cvar_t *no_runshrine;
cvar_t *no_tornado;
cvar_t *no_irondoom;
cvar_t *no_phoenix;
cvar_t *no_morph;
cvar_t *no_shield;
cvar_t *no_teleport;
cvar_t *game_test;
cvar_t *dm_no_bodies;

cvar_t *sv_nomonsters = NULL;
cvar_t *sv_freezemonsters;

cvar_t *autorotate;
cvar_t *blood;

cvar_t *checkanim;
cvar_t *allowillegalskins;

cvar_t *pvs_cull;

cvar_t *showbuoys;
cvar_t *showlitebuoys;
cvar_t *mgai_debug;
cvar_t *deactivate_buoys;
cvar_t *anarchy;
cvar_t *impact_damage;
cvar_t *cheating_monsters;
cvar_t *singing_ogles;
cvar_t *sv_cinematicfreeze;
cvar_t *sv_jumpcinematic;
cvar_t *blood_level;

qboolean MonsterAdvanceFrame = false;

/* ========================================================= */

void
G_CPrintf(edict_t* ent, int printlevel, short stringid)
{
	if (stringid > MAX_MESSAGESTRINGS || !game_msgtxt[stringid].string[0])
	{
		gi.dprintf("%s: Unknow message %d\n", __func__, stringid);
		return;
	}

	gi.cprintf(ent, printlevel, "%s\n", game_msgtxt[stringid].string);
	if (game_msgtxt[stringid].wav && game_msgtxt[stringid].wav[0])
	{
		gi.sound(NULL, CHAN_AUTO, gi.soundindex(game_msgtxt[stringid].wav), 1, ATTN_NORM, 0);
	}
}

void
G_BCaption(int printlevel, short stringid)
{
	char message[10];
	int sound_index = 0;

	snprintf(message, sizeof(message), "%d", stringid);
	gi.bprintf(printlevel, "%s", LocalizationMessage(message, &sound_index));

	if (sound_index)
	{
		gi.sound(NULL, CHAN_AUTO, sound_index, 1, ATTN_NORM, 0);
	}
}

void
G_LevelMsgCenterPrintf(edict_t* ent, short stringid)
{
	char message[10];
	int sound_index = 0;

	snprintf(message, sizeof(message), "%d", stringid);
	gi.centerprintf(ent, "%s", LocalizationMessage(message, &sound_index));

	if (sound_index)
	{
		gi.sound(ent, CHAN_AUTO, sound_index, 1, ATTN_NORM, 0);
	}
}

void
G_CaptionPrintf(edict_t* ent, short stringid)
{
	char message[10];
	int sound_index = 0;

	snprintf(message, sizeof(message), "%d", stringid);
	gi.centerprintf(ent, "%s", LocalizationMessage(message, &sound_index));

	if (sound_index)
	{
		gi.sound(ent, CHAN_AUTO, sound_index, 1, ATTN_NORM, 0);
	}
}

void
G_BroadcastObituary(int printlevel, short stringid, short client1, short client2)
{
	char message[10];
	int sound_index = 0;

	snprintf(message, sizeof(message), "%d", stringid);
	gi.bprintf(printlevel, "%s", LocalizationMessage(message, &sound_index));

	if (sound_index)
	{
		gi.sound(NULL, CHAN_AUTO, sound_index, 1, ATTN_NORM, 0);
	}
}

int
G_GetContentsAtPoint(vec3_t point)
{
	return gi.pointcontents(point); // Not correct.
}

int
G_FindEntitiesInBounds(vec3_t mins, vec3_t maxs, struct SinglyLinkedList_s* list, int areatype)
{
	edict_t* idlist[1024];
	int numEnts;

	numEnts = gi.BoxEdicts(mins, maxs, idlist, 1024, areatype);

	for (int i = 0; i < numEnts; i++)
	{
		GenericUnion4_t temp;

		temp.t_void_p = idlist[i];
		SLList_Push(list, temp);
	}

	return numEnts;
}

void
G_TraceBoundingForm(FormMove_t* formMove)
{
	formMove->trace = gi.trace(formMove->start, formMove->mins, formMove->maxs, formMove->end, (edict_t *)formMove->passEntity, formMove->clipMask);
	//formMove->trace = CM_BoxTrace(formMove->start, formMove->end, formMove->mins, formMove->maxs, 0, formMove->clipMask);
}

void G_MsgVarCenterPrintf(edict_t* ent, short msg, int vari)
{
	char message[10];
	int sound_index = 0;

	snprintf(message, sizeof(message), "%d", msg);
	gi.centerprintf(ent, "%s", LocalizationMessage(message, &sound_index));

	if (sound_index)
	{
		gi.sound(ent, CHAN_AUTO, sound_index, 1, ATTN_NORM, 0);
	}
}

void G_MsgDualCenterPrintf(edict_t* ent, short msg1, short msg2)
{
	char message[10];

	snprintf(message, sizeof(message), "%d", msg1);
	gi.centerprintf(ent, "%s", LocalizationMessage(message, NULL));
	snprintf(message, sizeof(message), "%d", msg2);
	gi.centerprintf(ent, "%s", LocalizationMessage(message, NULL));
}

qboolean G_ResizeBoundingForm(edict_t* self, struct FormMove_s* formMove)
{
	gi.dprintf("%s: TODO: Unimplemented\n", __func__);
	return false;
}

qboolean G_CheckDistances(vec3_t origin, float dist)
{
	gi.dprintf("%s: TODO: Unimplemented\n", __func__);
	return false;
}

void G_SoundRemove(char* name)
{
	gi.dprintf("%s: TODO: Unimplemented (%s)\n", __func__, name);
}

void G_CleanLevel(void)
{
	gi.dprintf("%s: TODO: Unimplemented\n", __func__);
}

void G_SoundEvent(byte EventId, float leveltime, edict_t* ent, int channel, int soundindex, float volume, float attenuation, float timeofs)
{
	gi.sound(ent, channel, soundindex, volume, attenuation, timeofs);
}


/* =================================================================== */

static void
ShutdownGame(void)
{
	void G_ReleaseResourceManagers();

	edict_t *ent;
	int i;

	gi.dprintf("==== ShutdownGame ====\n");

	ShutdownScripts(true);

	if(game.entitiesSpawned)
	{
		G_ClearMessageQueues();

		for(i = 0, ent = g_edicts; i < game.maxentities ; ++i, ++ent)
		{
			SLList_Des(&ent->msgQ.msgs);
			G_FreeEdict(ent);
		}

		G_ReleaseResourceManagers();
	}
	game.entitiesSpawned = false;

	gi.FreeFile(game_msgbuf);

	gi.FreeTags(TAG_LEVEL);
	gi.FreeTags(TAG_GAME);
	SpawnFree();
	LocalizationFree();

	P_Freelib();	// free the player lib
}

/*
 * Returns a pointer to the structure
 * with all entry points and global
 * variables
 */
Q2_DLL_EXPORTED game_export_t *
GetGameAPI(game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

	memset(&game, 0, sizeof(game));

	/* Initalize the PRNG */
	randk_seed();

	return &globals;
}

/*
 * this is only here so the functions
 * in shared source files can link
 */
void
Sys_Error(const char *error, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, error);
	vsnprintf(text, sizeof(text), error, argptr);
	va_end(argptr);

	gi.error("%s", text);
}

void
Com_Printf(const char *msg, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, msg);
	vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

	gi.dprintf("%s", text);
}

/* ====================================================================== */

static void
ClientEndServerFrames(void)
{
	int i;
	edict_t *ent;

	/* calc the player views now that all
	   pushing and damage has been added */
	for (i = 0; i < maxclients->value; i++)
	{
		ent = g_edicts + 1 + i;

		if (!ent->inuse || !ent->client)
		{
			continue;
		}

		ClientEndServerFrame(ent);
	}
}

/*
 * Returns the created target changelevel
 */
static edict_t *
CreateTargetChangeLevel(char *map)
{
	edict_t *ent;

	if (!map)
	{
		return NULL;
	}

	ent = G_Spawn();
	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;
	return ent;
}

/*
 * The timelimit or fraglimit has been exceeded
 */
void
EndDMLevel(void)
{
	edict_t *ent;
	char *s, *t, *f;
	static const char *seps = " ,\n\r";

	/* stay on same level flag */
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		BeginIntermission(CreateTargetChangeLevel(level.mapname));
		return;
	}

	if (*level.forcemap)
	{
		BeginIntermission(CreateTargetChangeLevel(level.forcemap));
		return;
	}

	/* see if it's in the map list */
	if (*sv_maplist->string)
	{
		s = strdup(sv_maplist->string);
		f = NULL;
		t = strtok(s, seps);

		while (t != NULL)
		{
			if (Q_stricmp(t, level.mapname) == 0)
			{
				/* it's in the list, go to the next one */
				t = strtok(NULL, seps);

				if (t == NULL) /* end of list, go to first one */
				{
					if (f == NULL) /* there isn't a first one, same level */
					{
						BeginIntermission(CreateTargetChangeLevel(level.mapname));
					}
					else
					{
						BeginIntermission(CreateTargetChangeLevel(f));
					}
				}
				else
				{
					BeginIntermission(CreateTargetChangeLevel(t));
				}

				free(s);
				return;
			}

			if (!f)
			{
				f = t;
			}

			t = strtok(NULL, seps);
		}

		free(s);
	}

	if (level.nextmap[0]) /* go to a specific map */
	{
		BeginIntermission(CreateTargetChangeLevel(level.nextmap));
	}
	else    /* search for a changelevel */
	{
		ent = G_Find(NULL, FOFS(classname), "target_changelevel");

		if (!ent)
		{
			/* the map designer didn't include a changelevel,
			   so create a fake ent that goes back to the same
			   level */
			BeginIntermission(CreateTargetChangeLevel(level.mapname));
			return;
		}

		BeginIntermission(ent);
	}
}

static void
CheckNeedPass(void)
{
	int need;

	/* if password or spectator_password has
	   changed, update needpass as needed */
	if (password->modified || spectator_password->modified)
	{
		password->modified = spectator_password->modified = false;

		need = 0;

		if (*password->string && Q_stricmp(password->string, "none"))
		{
			need |= 1;
		}

		if (*spectator_password->string &&
			Q_stricmp(spectator_password->string, "none"))
		{
			need |= 2;
		}

		gi.cvar_set("needpass", va("%d", need));
	}
}

void
CheckDMRules(void)
{
	int i;
	gclient_t *cl;

	if (level.intermissiontime)
	{
		return;
	}

	if (!deathmatch->value)
	{
		return;
	}

	if (timelimit->value)
	{
		if (level.time >= timelimit->value * 60)
		{
			gi.bprintf(PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel();
			return;
		}
	}

	if (fraglimit->value)
	{
		for (i = 0; i < maxclients->value; i++)
		{
			cl = game.clients + i;

			if (!g_edicts[i + 1].inuse)
			{
				continue;
			}

			if (cl->resp.score >= fraglimit->value)
			{
				gi.bprintf(PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel();
				return;
			}
		}
	}
}

static void
ExitLevel(void)
{
	char command[256];

	level.exitintermission = 0;
	level.intermissiontime = 0;

	Com_sprintf(command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString(command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames();

	G_ClearMessageQueues();
}

void CheckContinuousAutomaticEffects(edict_t *self)
{
	//only used for fire damage for now
	edict_t *damager;
	vec3_t	checkpoint;

	if(self->fire_damage_time > level.time)
	{
		VectorCopy(self->s.origin, checkpoint);
		checkpoint[2] += self->mins[2];
		checkpoint[2] += self->size[2] * 0.5;
		if(gi.pointcontents(checkpoint) & (CONTENTS_WATER|CONTENTS_SLIME))			// Not lava
		{//FIXME: make hiss and smoke too
			gi.dprintf("%s fire doused\n", self->classname);
			self->fire_damage_time = 0;
//			gi.RemoveEffects(self, FX_FIRE_ON_ENTITY);//turn off CFX too
			self->s.effects &= ~EF_ON_FIRE;			// Use this to instead notify the fire to stop.
			gi.CreateEffect(NULL,
					FX_ENVSMOKE,
					CEF_FLAG6,
					checkpoint,
					"");
			return;
		}

		if(self->health <= 0)
			return;

		if(self->fire_damage_enemy)
			damager = self->fire_damage_enemy;
		else
			damager = world;

		if (self->client)
		{	// Take less damage than a monster.
			if (!(((byte)(level.time*10))&0x07))
			{
				T_Damage(self, damager, damager, vec3_origin, self->s.origin, vec3_origin,
							1, 0, DAMAGE_BURNING,MOD_BURNT);
			}
		}
		else	// For monsters
		{	// Only account for damage every .4 second.
			if (!(((byte)(level.time*10))&0x03))
			{
				T_Damage(self, damager, damager, vec3_origin, self->s.origin, vec3_origin,
							irand(FIRE_LINGER_DMG_MIN, FIRE_LINGER_DMG_MAX), 0, DAMAGE_BURNING,MOD_BURNT);
			}
			//tint it darker brown as goes on?  How to get back? no, scorched art would look better
		}

		if(self->client)
		{
			if(self->client->playerinfo.lowerseq == ASEQ_ROLLDIVEF_W || self->client->playerinfo.lowerseq == ASEQ_ROLLDIVEF_R || self->client->playerinfo.lowerseq == ASEQ_ROLL_FROM_FFLIP ||
				self->client->playerinfo.upperseq == ASEQ_ROLLDIVEF_W || self->client->playerinfo.upperseq == ASEQ_ROLLDIVEF_R || self->client->playerinfo.upperseq == ASEQ_ROLL_FROM_FFLIP ||
				self->client->playerinfo.lowerseq == ASEQ_ROLL_L || self->client->playerinfo.lowerseq == ASEQ_ROLL_R || self->client->playerinfo.lowerseq == ASEQ_ROLL_B ||
				self->client->playerinfo.upperseq == ASEQ_ROLL_L || self->client->playerinfo.upperseq == ASEQ_ROLL_R || self->client->playerinfo.upperseq == ASEQ_ROLL_B)
			{
				float waterlevel;

				waterlevel = self->waterlevel/5.0;
				if (self->watertype & CONTENTS_LAVA)
					waterlevel = 0;
				self->fire_damage_time -= (0.15 + (waterlevel*0.5));//stop, drop and roll!
			}
		}
	}
	else if(self->fire_damage_time>0)
	{
		self->fire_damage_time = 0;
//		gi.RemoveEffects(self, FX_FIRE_ON_ENTITY);//turn off CFX too
		self->s.effects &= ~EF_ON_FIRE;		// Use this to instead notify the fire to stop.
		return;
	}

}

static void EntityThink(edict_t *self)
{
	float	thinktime;

	//see if anything is happening to us we need to update...
	CheckContinuousAutomaticEffects(self);

	thinktime = self->nextthink;

	if(self->pre_think && self->next_pre_think > 0.0f && self->next_pre_think < level.time)
	{//not used for guides anymore, but nice for effects
		//like tinting/fading, etc that should continue
		//while the entity is doing other stuff
		self->pre_think(self);
	}
	if(!ThinkTime(self))
	{
		return;
	}
	self->think(self);

	assert(!self->inuse || !self->think || thinktime != self->nextthink);
	//NOTENOTE: This is a Quake oldy... it's common practice to do this!
	/*assert(self->nextthink == -1);*/
}

static void EntityPostThink(edict_t *self)
{
	if(self->post_think && self->next_post_think > 0.0f && self->next_post_think < level.time)
	{//for effects that rely on accurate physics info
		self->post_think (self);
	}
}

static void SetNumPlayers (void)
{
	int			i;
	edict_t		*ent;

	ent = g_edicts;
	game.num_clients = 0;
	for(i = 0; i < MAX_CLIENTS; i++, ent++)
	{//If player hasn't moved, don't clear this
		if(ent)
		{
			if(ent->client)
			{
				game.num_clients++;
			}
		}
	}
}

static void
UpdatePlayerBuoys(void)
{
	qboolean	dont_null = true; // jmarshall: set this default, hope this is right
	int			i, j;
	edict_t		*ent;
	vec3_t		v;

	for(i = 0; i<MAX_CLIENTS; i++)
	{
		if(level.player_buoy[i] > NULL_BUOY)
		{
			ent = g_edicts;
			for(j = 0; j < globals.num_edicts; j++, ent++)
			{//If player hasn't moved, don't clear this
				if(ent->s.number - 1 == i)
				{
					VectorSubtract(level.buoy_list[level.player_buoy[i]].origin, ent->s.origin, v);
					if(VectorLengthSquared(v) > 576)//24 squared
						dont_null = false;
					else
						dont_null = true;
					break;
				}
			}
		}

		if(!dont_null)
		{
			level.player_last_buoy[i] = level.player_buoy[i];//save it so monsters can check this first- FIXME: should this expire?
			level.player_buoy[i] = NULL_BUOY;//this is for monsters following buoys- only the first monster who's searching for the player has to do a buoy connection to him this frame, the rest can use this- reset each frame
		}
	}
}

/*
 * Advances the world by 0.1 seconds
 */
static void
G_RunFrame(void)
{
	void		UpdateSkeletons();
	int i;
	edict_t *ent;

	if(deathmatch->value || coop->value)
	{
		blood_level->value = Clamp(blood_level->value, VIOLENCE_NONE, VIOLENCE_NORMAL);
	}

	// Update server ticks
	level.framenum++;
	level.time = level.framenum * FRAMETIME;

	gibsthisframe = 0;
	debristhisframe = 0;

	/* choose a client for monsters to target this frame */
	AI_SetSightClient();

	/* exit intermissions */
	if (level.exitintermission)
	{
		ExitLevel();
		return;
	}

	// Update any joints that need to be
	UpdateSkeletons();

	//Keep track of player buoys
	if(!deathmatch->value)
		UpdatePlayerBuoys();
	else
		SetNumPlayers();//for shrines and pick-ups

	/* treat each object in turn
	   even the world gets a chance
	   to think */
	ent = g_edicts;
	for (i = 0; i < globals.num_edicts; i++, ent++)
	{

		if (sv_cinematicfreeze->value)
		{
			if ((ent->svflags & SVF_MONSTER)  && (!ent->monsterinfo.c_mode))
				continue;
		}

		// If entity not in use - don`t process
		if (!ent->inuse)
		{
			continue;
		}

		//
		// Don`t let monster think or move if its not in the PVS and not hunting
		//

		// If the ent is a monster (but not a  cinematic monster) and the culling is active...
		if ((ent->svflags & SVF_MONSTER) && (!ent->monsterinfo.c_mode) && pvs_cull->value)
		{
			// Ent cannot be hunting an enemy or moving to a goalentity.

			if (!ent->enemy && !ent->goalentity && level.sight_client)
			{
				int		j;
				edict_t *client_ent;

				// If not in our PVS, we don't care.

				for (j = 0; j < maxclients->value; j++)
				{
					client_ent = g_edicts + 1 + j;

					if (client_ent->inuse)
					{
						if (!gi.inPVS(ent->s.origin, client_ent->s.origin))
						{
							continue;
						}
					}
				}
			}
		}

		level.current_entity = ent;

		if (ent->msgHandler)	// eventually this check wont be needed
		{
			G_ProcessMessages(ent);
		}

		if (ent->flags & FL_SUSPENDED)
		{
			continue;
		}

		// Remember original origin
		VectorCopy(ent->s.origin, ent->s.old_origin);

		// Make sure the entity still has something to stand on
		if(ent->groundentity)
		{
			// check for the groundentity being freed
			if (!ent->groundentity->inuse)
			{
				ent->groundentity = NULL;
			}
			else if(ent->groundentity->linkcount != ent->groundentity_linkcount)
			{	// if the ground entity moved, make sure we are still on it
				ent->groundentity = NULL;

				if(ent->svflags & SVF_MONSTER)
				{
					CheckEntityOn(ent);
				}
			}
		}

		if (i > 0 && i <= maxclients->value)
		{
			ClientBeginServerFrame (ent);
			// ok, we need to hack in some bits here - the players think function never appears to get called. Why, I don't know
			// kinda defies the point of having a think based system if your not going to use it. Still, never mind.
			// we need the think function for when the player is a chicken, in order to keep track of how long he should remain a chicken

			if (ent->flags & FL_CHICKEN)	// We're set as a chicken
				EntityThink(ent);

			continue;
		}

		// Use new physics for everything except flymissile (and movetype none)
		// The scripts work using the new physics now
		if (ent->movetype != MOVETYPE_FLYMISSILE)
		{
			EntityThink(ent);

			assert(ent->movetype <= MOVETYPE_SCRIPT_ANGULAR);

			if (!ent->inuse)
			{
				continue;
			}

			EntityPhysics(ent);
			EntityPostThink(ent);
		}
		else
		// Use old physics for missiles (for compatibility)
		{
			G_RunEntity(ent);
		}
	}

	// If the monsters are frozen, we wanted a single frame advance.
	if (MonsterAdvanceFrame)
	{
		MonsterAdvanceFrame = false;
	}

	ProcessScripts();

	/* see if it is time to end a deathmatch */
	CheckDMRules();

	/* see if needpass needs updated */
	CheckNeedPass();

	/* build the playerstate_t structures for all players */
	ClientEndServerFrames();

#if 0
	//JABot[start]
	AITools_Frame();	//give think time to AI debug tools
	//[end]
#endif
}
