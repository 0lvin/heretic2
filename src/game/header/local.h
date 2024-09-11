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
 * Main header file for the game module.
 *
 * =======================================================================
 */

#ifndef GAME_LOCAL_H
#define GAME_LOCAL_H

#include "../../common/header/shared.h"

/* define GAME_INCLUDE so that game.h does not define the
   short, server-visible gclient_t and edict_t structures,
   because we define the full size ones in this file */
#define GAME_INCLUDE
#include "game.h"
#include "buoy.h"
#include "../player/library/player.h"

// The "gameversion" client command will print this plus compile date.

#define	GAMEVERSION	"yQRHeretic2"

// Protocol bytes that can be directly added to messages.

// volume mask for ent->sound_data - makes room for attn value in the lower bits
#define ENT_VOL_MASK	0xf8

// ************************************************************************************************
// AI_MOOD_XXX
// -------------
// Held in 'edict_t'->ai_mood. Used by higher level AI functions to relay states to lower functions
// ************************************************************************************************

#define	AI_MOOD_NORMAL		0		//Not using any high level functionality (TEMP)
#define	AI_MOOD_ATTACK		1		//Used in conjuntion with ai_mood_flags to attack the target
#define	AI_MOOD_NAVIGATE	2		//Just walk towards the guide, ignoring everything else
#define	AI_MOOD_STAND		3		//Just stand there and wait to be adivsed
#define	AI_MOOD_PURSUE		4		//Run towards your enemy but don't attack
#define	AI_MOOD_FALLBACK	5		//Back away from your enemy, but face him
#define	AI_MOOD_DELAY		6		//Same as stand, but will allow interruption anywhere
#define AI_MOOD_WANDER		7		//Wandering around buoy to buoy in a walk
#define AI_MOOD_JUMP		8		//Jump towards goalentity
#define AI_MOOD_REST		9		//The Ogle at rest
#define AI_MOOD_POINT_NAVIGATE	10	//Navigate to a point, not an entity
#define AI_MOOD_FLEE		11		//run away!
#define AI_MOOD_BACKUP		12		//backstep while attacking
#define AI_MOOD_WALK		13		//walking, no buoys
#define AI_MOOD_EAT			14		//sitting around, eating

// ************************************************************************************************
// AI_MOOD_FLAG_XXX
// -------------
// Held in 'edict_t'->ai_mood_flags. Used in conjuction with ai_mood
// ************************************************************************************************

#define	AI_MOOD_FLAG_MISSILE		0x00000001		//Check for a missile attack
#define	AI_MOOD_FLAG_MELEE			0x00000002		//Check for a melee attack
#define AI_MOOD_FLAG_WHIP			0x00000004		//Check for a whipping attack (no damage)
#define AI_MOOD_FLAG_PREDICT		0x00000008		//Monster will predict movement on target
#define AI_MOOD_FLAG_IGNORE			0x00000010		//Monster will ignore moods
#define AI_MOOD_FLAG_FORCED_BUOY	0x00000020		//Monster will head towards it's forced_buoy
#define AI_MOOD_FLAG_IGNORE_ENEMY	0x00000040		//Monster will ignore it's enemy unless attacked or otherwise directed
#define AI_MOOD_FLAG_BACKSTAB		0x00000080		//Monster will advance on and attack enemy only from behind
#define AI_MOOD_FLAG_DUMB_FLEE		0x00000100		//Monster will flee by simply running directly away from player
#define AI_MOOD_FLAG_GOTO_FIXED		0x00000200		//Monster will become fixed upon getting to it's forced_buoy
#define AI_MOOD_FLAG_GOTO_STAND		0x00000400		//Monster will stand upon getting to it's forced_buoy
#define AI_MOOD_FLAG_GOTO_WANDER	0x00000800		//Monster will wander upon getting to it's forced_buoy
#define AIMF_CANT_FIND_ENEMY		0x00001000		//Monster can't find enemy with buoys or vision
#define AIMF_SEARCHING				0x00002000		//Monster now in dumb search mode...

// ************************************************************************************************
// SPAWNFLAG_XXX
// -------------
// Held in 'edict_t'->spawnflags. These are set with checkboxes on each entity in the map editor.
// ************************************************************************************************

#define	SPAWNFLAG_NOT_EASY			0x00000100
#define	SPAWNFLAG_NOT_MEDIUM		0x00000200
#define	SPAWNFLAG_NOT_HARD			0x00000400
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800
#define	SPAWNFLAG_NOT_COOP			0x00001000

// ************************************************************************************************
// Timing constants that define the world heartbeat.
// ************************************************************************************************

#define	FRAMETIME			0.1
#define MONSTER_THINK_INC   0.099
#define FRAMES_PER_SECOND	10.0

// ************************************************************************************************
// TAG_XXX
// -------
// Memory tags to allow dynamic memory to be selectively cleaned up.
// ************************************************************************************************

#define	TAG_GAME	765			// clear when unloading the dll
#define	TAG_LEVEL	766			// clear when loading a new level

// ************************************************************************************************
// damage_t
// --------
// ************************************************************************************************

typedef enum
{
	DAMAGE_NO,
	DAMAGE_YES, /* will take damage if hit */
	DAMAGE_AIM, /* auto targeting recognizes this */
	DAMAGE_NO_RADIUS, /* Will not take damage from radius blasts */
} damage_t;

#define GIB_ORGANIC 1

#define BODY_QUEUE_SIZE		8

// ************************************************************************************************
// RANGE_XXX
// ---------
// ************************************************************************************************

#define RANGE_MELEE 0
#define RANGE_NEAR 1
#define RANGE_MID 2
#define RANGE_FAR 3

#define MELEE_DISTANCE	80

/* armor types */
#define ARMOR_NONE 0
#define ARMOR_JACKET 1
#define ARMOR_COMBAT 2
#define ARMOR_BODY 3
#define ARMOR_SHARD 4

// ************************************************************************************************
// SHRINE_XXX
// ----------
// ************************************************************************************************

enum
{
	SHRINE_MANA,
	SHRINE_LUNGS,
	SHRINE_ARMOR_SILVER,
	SHRINE_ARMOR_GOLD,
	SHRINE_LIGHT,
	SHRINE_SPEED,
	SHRINE_HEAL,
	SHRINE_STAFF,
	SHRINE_GHOST,
	SHRINE_REFLECT,
	SHRINE_POWERUP,
	SHRINE_RANDOM
};

/* game.serverflags values */
#define SFL_CROSS_TRIGGER_1 0x00000001
#define SFL_CROSS_TRIGGER_2 0x00000002
#define SFL_CROSS_TRIGGER_3 0x00000004
#define SFL_CROSS_TRIGGER_4 0x00000008
#define SFL_CROSS_TRIGGER_5 0x00000010
#define SFL_CROSS_TRIGGER_6 0x00000020
#define SFL_CROSS_TRIGGER_7 0x00000040
#define SFL_CROSS_TRIGGER_8 0x00000080
#define SFL_CROSS_TRIGGER_MASK 0x000000ff

#define MAX_MESSAGESTRINGS 1000
typedef struct
{
	char *string;
	char *wav;
} trig_message_t;

// jmarshall: this wasn't extern in the original code,
// this is now correct, wondering if this will cause knock ons?
extern unsigned	*level_msgbuf;
extern unsigned	*game_msgbuf;
extern trig_message_t level_msgtxt[];
extern trig_message_t game_msgtxt[];


/* this structure is left intact through an entire game
   it should be initialized at dll load time, and read/written to
   the server.ssv file for savegames */
typedef struct
{
	char helpmessage1[512];
	char helpmessage2[512];
	int helpchanged; /* flash F1 icon if non 0, play sound
					    and increment only if 1, 2, or 3 */

	gclient_t *clients; /* [maxclients] */

	/* can't store spawnpoint in level, because
	   it would get overwritten by the savegame
	   restore */
	char spawnpoint[512]; /* needed for coop respawns */

	/* store latched cvars here that we want to get at often */
	int maxclients;
	int maxentities;

	/* cross level triggers */
	int serverflags;

	/* items */
	int num_items;

	qboolean autosaved;

	//updated every frame in DM, for pick-up and shrine respawn times
	int num_clients;

	qboolean entitiesSpawned;
} game_locals_t;

// ************************************************************************************************
// alertent_t
// ---------
// This structure is used for alert entities, which are spawned a lot
// ************************************************************************************************
#define MAX_ALERT_ENTS	1024//no more that 1024 alertents allowed
typedef struct alertent_s alertent_t;
struct alertent_s
{
	alertent_t	*next_alert;
	alertent_t	*prev_alert;
	edict_t		*enemy;
	vec3_t		origin;
	qboolean	inuse;
	int			alert_svflags;
	float		lifetime;
};

/* this structure is cleared as each map is entered
   it is read/written to the level.sav file for savegames */
typedef struct
{
	int framenum;
	float time;

	char level_name[MAX_QPATH]; /* the descriptive name (Outer Base, etc) */
	char mapname[MAX_QPATH]; /* the server name (base1, etc) */
	char nextmap[MAX_QPATH]; /* go here when fraglimit is hit */

	/* intermission state */
	float intermissiontime; /* time the intermission was started */
	char *changemap;
	int exitintermission;
	vec3_t intermission_origin;
	vec3_t intermission_angle;

	edict_t *sight_client; /* changed once each frame for coop games */

	edict_t *sight_entity;
	int sight_entity_framenum;
	edict_t *sound_entity;
	int sound_entity_framenum;
	edict_t *sound2_entity;
	int sound2_entity_framenum;

	int pic_health;

	int total_secrets;
	int found_secrets;

	int total_goals;
	int found_goals;

	int total_monsters;
	int killed_monsters;

	edict_t *current_entity; /* entity running from G_RunFrame */
	int body_que; /* dead bodies */

	float		far_clip_dist_f;
	float		fog;
	float		fog_density;

	qboolean	cinActive;

	buoy_t		buoy_list[MAX_MAP_BUOYS];	//Buoy information for this map
	int		active_buoys;				//Number of actual buoys on the level
	int		fucked_buoys;				//Number of buoys that can't be fixed
	int		fixed_buoys;				//Number of buoys that had to be fixed

	int		player_buoy[MAX_CLIENTS];				//stores current bestbuoy for a player enemy (if any)
	int		player_last_buoy[MAX_CLIENTS];		//when player_buoy is invalid, saves it here so monsters can check it first instead of having to do a whole search


	int		offensive_weapons,
			defensive_weapons;

	alertent_t	alertents[MAX_ALERT_ENTS];	//all the alert ents on the map
	int			num_alert_ents;				//Number of actual alert entities on the level
	alertent_t	*alert_entity,				//the alert entity linked list start
				*last_alert;				//the last entity in alert entity linked list

	qboolean	fighting_beast;				//fighting a beast, do extra checks with trace instant weapons
} level_locals_t;

// ************************************************************************************************
// MOD_XXX
// -------
// Means of death.
// ************************************************************************************************

typedef enum
{
	MOD_UNKNOWN			,

	MOD_STAFF			,
	MOD_FIREBALL		,
	MOD_MMISSILE		,
	MOD_SPHERE			,
	MOD_SPHERE_SPL		,
	MOD_IRONDOOM		,
	MOD_FIREWALL		,
	MOD_STORM			,
	MOD_PHOENIX			,
	MOD_PHOENIX_SPL		,
	MOD_HELLSTAFF		,

	MOD_P_STAFF			,
	MOD_P_FIREBALL		,
	MOD_P_MMISSILE		,
	MOD_P_SPHERE	 	,
	MOD_P_SPHERE_SPL 	,
	MOD_P_IRONDOOM		,
	MOD_P_FIREWALL		,
	MOD_P_STORM			,
	MOD_P_PHOENIX	 	,
	MOD_P_PHOENIX_SPL	,
	MOD_P_HELLSTAFF		,

	MOD_KICKED			,
	MOD_METEORS			,
	MOD_ROR				,
	MOD_SHIELD			,
	MOD_CHICKEN			,
	MOD_TELEFRAG		,
	MOD_WATER			,
	MOD_SLIME			,
	MOD_LAVA			,
	MOD_CRUSH			,
	MOD_FALLING			,
	MOD_SUICIDE			,
	MOD_BARREL			,
	MOD_EXIT			,
	MOD_BURNT			,
	MOD_BLEED			,
	MOD_SPEAR			,
	MOD_DIED			,
	MOD_KILLED_SLF		,
	MOD_DECAP			,
	MOD_TORN			,
	MOD_MAX
} MOD_t;

#define MOD_FRIENDLY_FIRE	0x8000000


/* spawn_temp_t is only used to hold entity field values that
   can be set from the editor, but aren't actualy present
   in edict_t during gameplay */
typedef struct
{
	/* world vars */
	char *sky;
	float skyrotate;
	int skyautorotate;
	vec3_t skyaxis;
	char *nextmap;

	int lip;
	int distance;
	int height;
	char *noise;
	float pausetime;
	char *item;
	char *gravity;

	float minyaw;
	float maxyaw;
	float minpitch;
	float maxpitch;

	int		rotate;
	float	zangle;
	char	*file;
	int		radius;

	// Weapons to be given to the player on spawning.

	int		offensive;
	int		defensive;
	int		spawnflags2;

	// Time to wait (in seconds) for all clients to have joined a map in coop.

	int		cooptimeout;

	// Scripting stuff.

	char	*script;
	char	*parms[16];
} spawn_temp_t;

typedef struct
{
	/* fixed data */
	vec3_t start_origin;
	vec3_t start_angles;
	vec3_t end_origin;
	vec3_t end_angles;

	int sound_start;
	int sound_middle;
	int sound_end;

	float accel;
	float speed;
	float decel;
	float distance;

	float wait;

	/* state data */
	int state;
	vec3_t dir;
	float current_speed;
	float move_speed;
	float next_speed;
	float remaining_distance;
	float decel_distance;
	void (*endfunc)(edict_t *);
} moveinfo_t;

// ************************************************************************************************
// AI_XXX
// ------
// Monster AI flags.
// ************************************************************************************************

#define AI_STAND_GROUND			0x00000001
#define AI_TEMP_STAND_GROUND	0x00000002
#define AI_SOUND_TARGET			0x00000004
#define AI_LOST_SIGHT			0x00000008
#define AI_PURSUIT_LAST_SEEN	0x00000010
#define AI_PURSUE_NEXT			0x00000020
#define AI_PURSUE_TEMP			0x00000040
#define AI_HOLD_FRAME			0x00000080
#define AI_GOOD_GUY				0x00000100
#define AI_BRUTAL				0x00000200
#define AI_NOSTEP				0x00000400	//1024
#define AI_DUCKED				0x00000800
#define AI_COMBAT_POINT			0x00001000
#define AI_EATING				0x00002000
#define AI_RESURRECTING			0x00004000
#define AI_FLEE					0x00008000
#define AI_FALLBACK				0x00010000
#define AI_COWARD				0x00020000	//Babies (FLEE to certain distance & WATCH)
#define AI_AGRESSIVE			0x00040000	//never run away
#define AI_SHOVE				0x00080000	//shove others out of the way.
#define AI_DONT_THINK			0x00100000	//animate, don't think or move
#define AI_SWIM_OK				0x00200000	//ok to go in water
#define AI_OVERRIDE_GUIDE		0x00400000
#define AI_NO_MELEE				0x00800000	//not allowed to melee
#define AI_NO_MISSILE			0x01000000	//not allowed to missile
#define AI_USING_BUOYS			0x02000000	//Using Buoyah! Navigation System(tm)
#define AI_STRAIGHT_TO_ENEMY	0x04000000	//Charge straight at enemy no matter what anything else tells you
#define AI_NIGHTVISION			0x08000000	//light level does not effect this monster's vision or aim
#define AI_NO_ALERT				0x10000000	//monster does not pay attemntion to alerts

// ************************************************************************************************
// AS_XXX
// ------
// Monster attack states.
// ************************************************************************************************

#define AS_STRAIGHT	1
#define AS_SLIDING	2
#define	AS_MELEE	3
#define	AS_MISSILE	4
#define	AS_DIVING	5

// ************************************************************************************************
// C_ANIM_XXX
// ------
// Cinmatic Animation flags
// ************************************************************************************************
#define C_ANIM_MOVE		1
#define C_ANIM_REPEAT	2
#define C_ANIM_DONE		4
#define C_ANIM_IDLE		8

// ************************************************************************************************
// OBJ_XXX
// ------
// Flags for object entities
// ************************************************************************************************
#define OBJ_INVULNERABLE	1
#define OBJ_ANIMATE			2
#define OBJ_EXPLODING		4
#define OBJ_NOPUSH			8


// ************************************************************************************************
// SIGHT_XXX
// ------
// Type of target aquisition
// ************************************************************************************************
#define SIGHT_SOUND_TARGET 0		//Heard the target make this noise
#define SIGHT_VISIBLE_TARGET 1		//Saw this target
#define SIGHT_ANNOUNCED_TARGET 2	//Target was announced by another monster


// ************************************************************************************************
// mframe_t
// --------
// ************************************************************************************************

typedef struct
{
	int framenum; // Index to current animation frame.
	void (*aifunc)(edict_t *self, float dist);
	float dist;
	void (*thinkfunc)(edict_t *self);
} mframe_t;

// ************************************************************************************************
// mmove_t
// -------
// ************************************************************************************************

typedef struct
{
	int framecount;					// Number of frames in the animation frame array.
	mframe_t *frame;
	void (*endfunc)(edict_t *self);
} mmove_t;


// ************************************************************************************************
// animframe_t
// -----------
// ************************************************************************************************

typedef struct
{
	int		framenum;
	void	(*movefunc)(edict_t *self, float var1, float var2, float var3);
	float	var1, var2, var3;
	void	(*actionfunc)(edict_t *self, float var4);
	float	var4;
	void	(*thinkfunc)(edict_t *self);
} animframe_t;

// ************************************************************************************************
// animmove_t
// ----------
// ************************************************************************************************

typedef struct
{
	int			numframes;
	animframe_t	*frame;
	void		(*endfunc)(edict_t *self);
} animmove_t;


// ************************************************************************************************
// c_animflags_t
// ----------
// ************************************************************************************************
typedef struct
{
	qboolean moving;		// Does this action support moving
	qboolean repeat;		// Does this action support repeating
	qboolean turning;	// Does this action support turning
} c_animflags_t;


// ************************************************************************************************
// monsterinfo_t
// -------------
// ************************************************************************************************

typedef struct
{
// Not used in new system
	char		*otherenemyname;				// ClassName of secondary enemy (other than player).
												// E.g. a Rat's secondary enemy is a gib.

	animmove_t	*currentmove;
	int			aiflags;
	int			aistate;						// Last order given to the monster (ORD_XXX).
	int			currframeindex;					// Index to current monster frame.
	int			nextframeindex;					// Used to force the next frameindex.
	float		thinkinc;						// Time between thinks for this entity.
	float		scale;

	void		(*idle)(edict_t *self);
	void		(*search)(edict_t *self);
	void		(*dodge)(edict_t *self, edict_t *other, float eta);
	int			(*attack)(edict_t *self);
	void		(*sight)(edict_t *self, edict_t *other);
	void		(*dismember)(edict_t *self, int damage, int HitLocation);
	qboolean	(*alert)(edict_t *self, alertent_t *alerter, edict_t *enemy);
	qboolean	(*checkattack)(edict_t *self);

	float		pausetime;
	float		attack_finished;
	float		flee_finished;					// When a monster is done fleeing
	float		chase_finished;					// When the monster can look for secondary monsters.

	vec3_t		saved_goal;
	float		search_time;
	float		misc_debounce_time;
	vec3_t		last_sighting;
	int			attack_state;
	int			lefty;
	float		idle_time;
	int			linkcount;

	int			searchType;
	vec3_t		nav_goal;
	float		jump_time;

	int			stepState;

	int			ogleflags;		//Ogles have special spawnflags stored in here at spawntime

	int			supporters;		//Number of supporting monsters (with common type) in the area when awoken

	float		sound_finished;	//Amount of time until the monster will be finishing talking (used for voices)
	float		sound_start;	//The amount of time to wait before playing the pending sound
	int			sound_pending;	//This monster is waiting to make a sound (used for voices) (0 if false, else sound ID)

	// Cinematic fields
	int			c_dist;			// Distance left to move
	int			c_repeat;		// # of times to repeat the anim cycle
	void (*c_callback)(struct edict_s *self);	// Callback function when action is done
	int			c_anim_flag;		// Shows if current cinematic anim supports moving, turning, or repeating
	qboolean	c_mode;			// in cinematic mode or not?
	edict_t		*c_ent;			// entity passed from a cinematic command

	qboolean	awake;			// has found an anemy AND gone after it.
	qboolean	roared;			// Gorgon has roared or been woken up by a roar

	float		last_successful_enemy_tracking_time;	//last time successfully saw enemy or found a path to him
	float		coop_check_debounce_time;
} monsterinfo_t;

// ************************************************************************************************
// aceldata_t
// ----------
// ************************************************************************************************

typedef struct
{
	animmove_t	*move;
	short		fly;
	short		lockmove;
	int			playerflags;
} aceldata_t;

// ************************************************************************************************
// acelsizes_t
// -----------
// ************************************************************************************************

typedef struct
{
	vec3_t	boundbox[2];
	int		altmove;
	float	viewheight;
	float	waterheight;
} acelsizes_t;

// The structure for each monster class.

extern edict_t *g_edicts;

#define FOFS(x) (size_t)&(((edict_t *)NULL)->x)
#define	STOFS(x) (size_t)&(((spawn_temp_t *)NULL)->x)
#define	LLOFS(x) (size_t)&(((level_locals_t *)NULL)->x)
#define	CLOFS(x) (size_t)&(((gclient_t *)NULL)->x)
#define	BYOFS(x) (size_t)&(((buoy_t *)NULL)->x)

extern	game_locals_t	game;
extern	level_locals_t	level;
extern	game_import_t	gi;
extern	spawn_temp_t	st;
extern	game_export_t	globals;

extern	int				sm_meat_index;
extern	int				snd_fry;

extern cvar_t *maxentities;
extern cvar_t *deathmatch;
extern cvar_t *coop;
extern cvar_t *dmflags;
extern cvar_t *advancedstaff;
extern cvar_t *skill;
extern cvar_t *fraglimit;
extern cvar_t *timelimit;
extern cvar_t *password;
extern cvar_t *spectator_password;
extern cvar_t *needpass;
extern cvar_t *g_select_empty;
extern cvar_t *filterban;

extern cvar_t *sv_gravity;
extern cvar_t *sv_friction;
extern cvar_t *sv_maxvelocity;

extern cvar_t *gun_x, *gun_y, *gun_z;
extern cvar_t *sv_rollspeed;
extern cvar_t *sv_rollangle;

extern cvar_t *run_pitch;
extern cvar_t *run_roll;
extern cvar_t *bob_up;
extern cvar_t *bob_pitch;
extern cvar_t *bob_roll;

extern cvar_t *sv_cheats;
extern cvar_t *sv_nomonsters;
extern cvar_t *blood_level;
extern cvar_t *showbuoys;
extern cvar_t *showlitebuoys;
extern cvar_t *mgai_debug;
extern cvar_t *deactivate_buoys;
extern cvar_t *anarchy;
extern cvar_t *impact_damage;
extern cvar_t *cheating_monsters;
extern cvar_t *singing_ogles;
extern cvar_t *no_runshrine;
extern cvar_t *no_tornado;
extern cvar_t *no_irondoom;
extern cvar_t *no_phoenix;
extern cvar_t *no_morph;
extern cvar_t *no_shield;
extern cvar_t *no_teleport;
extern cvar_t *log_file_name;
extern cvar_t *log_file_header;
extern cvar_t *log_file_footer;
extern cvar_t *log_file_line_header;

extern cvar_t *sv_cinematicfreeze;
extern cvar_t *sv_jumpcinematic;


extern cvar_t *sv_freezemonsters;

extern cvar_t *maxclients;
extern cvar_t *maxspectators;
extern cvar_t *sv_maplist;

extern cvar_t *autorotate;
extern cvar_t *blood;

extern cvar_t *checkanim;		// specifies whether monsters should check to see if most of the
										// distance of a move animation is unobstructed before setting it
extern cvar_t *allowillegalskins;

extern cvar_t *monster_speeds;
extern cvar_t *pvs_cull;

extern cvar_t *game_test; // sfs--for testing the speed impact of code changes
extern cvar_t *dm_no_bodies;

extern cvar_t *player_dll;

extern cvar_t *flood_msgs;
extern cvar_t *flood_persecond;
extern cvar_t *flood_waitdelay;
extern cvar_t *flood_killdelay;

extern	edict_t			*g_edicts;

extern	int				self_spawn;
#define world			(&g_edicts[0])

// ************************************************************************************************
// 'DROPPED_XXX'.
// --------------
// ************************************************************************************************

#define DROPPED_ITEM		0x00008000
#define	DROPPED_PLAYER_ITEM	0x00010000

// fields are needed for spawning from the entity string
// and saving / loading games

#define FFL_SPAWNTEMP		1

// ************************************************************************************************
// fieldtype_t
// -----------
// ************************************************************************************************

typedef enum
{
	F_INT,
	F_FLOAT,
	F_LSTRING, /* string on disk, pointer in memory, TAG_LEVEL */
	F_GSTRING, /* string on disk, pointer in memory, TAG_GAME */
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT, /* index on disk, pointer in memory */
	F_ITEM, /* index on disk, pointer in memory */
	F_CLIENT, /* index on disk, pointer in memory */
	F_RGBA,
	F_RGB,
	F_IGNORE
} fieldtype_t;

typedef struct
{
	const char *name;
	size_t ofs;
	fieldtype_t type;
	int flags;
	short save_ver;
} field_t;

extern field_t fields[];
// extern gitem_t itemlist[];


/* g_cmds.c */
void Cmd_Help_f(edict_t *ent);
void Cmd_Score_f(edict_t *ent);

/* g_items.c */
void PrecacheItem(gitem_t *it);
void G_InitItems(void);
void SetItemNames(void);
edict_t *Drop_Item(edict_t *ent, gitem_t *item);
void SetRespawn(edict_t *ent);
void SpawnItem(edict_t *ent, gitem_t *item);
void SpawnItemEffect(edict_t *ent, gitem_t *item);
gitem_t	*IsItem(edict_t *ent);
void Think_Weapon(edict_t *ent);
int ArmorIndex(edict_t *ent);
qboolean Add_Ammo(edict_t *ent, gitem_t *item, int count);

/* g_utils.c */
qboolean KillBox(edict_t *ent);
void G_ProjectSource(vec3_t point, vec3_t distance, vec3_t forward,
		vec3_t right, vec3_t result);
edict_t *G_Find(edict_t *from, int fieldofs, const char *match);
edict_t *G_Spawn(void);


edict_t *oldfindradius (edict_t *from, vec3_t org, float rad);
edict_t *findradius (edict_t *from, vec3_t org, float rad);
edict_t *findinblocking (edict_t *from, edict_t *checkent);
edict_t *findinbounds(edict_t *from, vec3_t min, vec3_t max);
edict_t *oldfindinbounds(edict_t *from, vec3_t min, vec3_t max);
edict_t *finddistance (edict_t *from, vec3_t org, float mindist, float maxdist);
edict_t *findonpath(edict_t *startent, vec3_t startpos, vec3_t endpos, vec3_t mins, vec3_t maxs, vec3_t *resultpos);
edict_t *G_PickTarget (char *targetname);

//commonly used functions
int range (edict_t *self, edict_t *other);
qboolean clear_visible (edict_t *self, edict_t *other);
qboolean visible (edict_t *self, edict_t *other);
qboolean visible_pos (edict_t *self, vec3_t spot2);
qboolean infront (edict_t *self, edict_t *other);
qboolean infront_pos (edict_t *self, vec3_t pos);
qboolean ahead (edict_t *self, edict_t *other);

void G_UseTargets (edict_t *ent, edict_t *activator);
void G_SetMovedir (vec3_t angles, vec3_t movedir);
void G_InitEdict (edict_t *e);
void G_FreeEdict (edict_t *e);
void G_SetToFree (edict_t *);
void G_TouchTriggers (edict_t *ent);
void G_TouchSolids (edict_t *ent);
void G_LinkMissile(edict_t *ent);
char *G_CopyString (char *in);
char *vtos (vec3_t v);
float vectoyaw (vec3_t vec);

/* g_combat.c */
qboolean OnSameTeam(edict_t *ent1, edict_t *ent2);
qboolean CanDamage(edict_t *targ, edict_t *inflictor);
qboolean CanDamageFromLoc(edict_t *targ, edict_t *inflictor, vec3_t origin);
void T_Damage(edict_t *targ, edict_t *inflictor, edict_t *attacker,
		vec3_t dir, vec3_t point, vec3_t normal, int damage,
		int knockback, int dflags, int mod);
void T_DamageRadius(edict_t *inflictor, edict_t *attacker,
		edict_t *ignore, float radius,
		float maxdamage, float mindamage, int dflags,int MeansOfDeath);
void T_DamageRadiusFromLoc(vec3_t origin, edict_t *inflictor, edict_t *attacker, edict_t *ignore, float radius,
							float maxdamage, float mindamage, int dflags,int MeansOfDeath);

/* damage flags */
#define DAMAGE_NORMAL 0x00000000 /* No modifiers to damage */
#define DAMAGE_RADIUS 0x00000001 /* damage was indirect */
#define DAMAGE_NO_KNOCKBACK			0x00000002	// do not affect velocity, just view angles
#define DAMAGE_ALL_KNOCKBACK		0x00000004  // Ignore damage
#define DAMAGE_EXTRA_KNOCKBACK		0x00000008	// throw in some extra z
#define DAMAGE_NO_PROTECTION		0x00000010  // invulnerability, and godmode have no effect
#define DAMAGE_NO_BLOOD				0x00000020  // don't spawn any blood
#define DAMAGE_EXTRA_BLOOD			0x00000040	// Lots of blood
#define DAMAGE_SPELL				0x00000080  // this came from a spell, - for use in calcing armor effects
#define DAMAGE_DISMEMBER			0x00000100  // Force this hit to use dismemberment message
#define DAMAGE_ATTACKER_IMMUNE		0x00000200  // Inflictor receives no effect
#define DAMAGE_ATTACKER_KNOCKBACK	0x00000400  // Inflictor takes knockback only
#define DAMAGE_REDRAIN				0x00000800	// Red rain acid damage
#define DAMAGE_BUBBLE				0x00001000	// Drowning damage
#define DAMAGE_FIRE					0x00002000  // Fire damage
#define DAMAGE_ALIVE_ONLY			0x00004000	// Only damage living things made of flesh
#define DAMAGE_BLEEDING				0x00008000	// No protection
#define DAMAGE_AVOID_ARMOR			0x00010000	// don't do the armor effect
#define DAMAGE_DOUBLE_DISMEMBER		0x00020000  // Force this hit to use dismemberment message with TWICE the chance of cutting
#define DAMAGE_HURT_FRIENDLY		0x00040000  // Always hurt friendly entities (e.g. fellow coop players).
#define DAMAGE_POWERPHOENIX			0x00080000	// Extra knockback to shooter, 1/4 damage.
#define DAMAGE_FIRE_LINGER			0x00100000	// Do extra fire linger damage.
#define DAMAGE_ENEMY_MAX			0x00200000	// Do maximum damage directly to the enemy in radius
#define DAMAGE_ONFIRE				0x00400000	// If the damage is FROM a fire...
#define DAMAGE_PHOENIX				0x00800000	// Phoenix-oriented damage.  Do minimal fire for show, but short duration.

#define DAMAGE_SUFFOCATION			(DAMAGE_NO_KNOCKBACK|DAMAGE_NO_BLOOD|DAMAGE_BUBBLE|DAMAGE_AVOID_ARMOR)
#define DAMAGE_LAVA					(DAMAGE_NO_KNOCKBACK|DAMAGE_NO_BLOOD|DAMAGE_FIRE|DAMAGE_AVOID_ARMOR)
#define DAMAGE_SLIME				(DAMAGE_NO_KNOCKBACK|DAMAGE_NO_BLOOD|DAMAGE_AVOID_ARMOR)
#define DAMAGE_BURNING				(DAMAGE_ONFIRE|DAMAGE_NO_KNOCKBACK|DAMAGE_NO_BLOOD|DAMAGE_FIRE|DAMAGE_AVOID_ARMOR)

/* g_monster.c */
void M_droptofloor(edict_t *ent);
void monster_think(edict_t *self);
qboolean walkmonster_start(edict_t *self);
qboolean swimmonster_start(edict_t *self);
qboolean flymonster_start(edict_t *self);
void AttackFinished(edict_t *self, float time);
void PauseTime(edict_t *self, float time);
void monster_death_use(edict_t *self);
void M_CatagorizePosition(edict_t *ent);
qboolean M_CheckAttack(edict_t *self);
void M_CheckGround(edict_t *ent);

/* g_misc.c */
void ThrowClientHead(edict_t *self, int damage);
void ThrowGib(edict_t *self, char *gibname, int damage, int type);
void BecomeExplosion1(edict_t *self);

/* g_ai.c */
void AI_SetSightClient(void);

void ai_stand(edict_t *self, float dist);
void ai_move(edict_t *self, float dist);
void ai_walk(edict_t *self, float dist);
void ai_turn(edict_t *self, float dist);
void ai_run(edict_t *self, float dist);
void ai_charge(edict_t *self, float dist);
void ai_eat(edict_t *self, float dist);
void ai_generic(edict_t *self);
void ai_flee(edict_t *self, float dist);
int range(edict_t *self, edict_t *other);

void FoundTarget(edict_t *self, qboolean setsightent);
qboolean infront(edict_t *self, edict_t *other);
qboolean visible(edict_t *self, edict_t *other);
qboolean FacingIdeal(edict_t *self);

/* g_client.c */
void respawn(edict_t *ent);
void BeginIntermission(edict_t *targ);
void PutClientInServer(edict_t *ent);
void InitClientPersistant(edict_t *player);
void InitClientResp(gclient_t *client);
void InitBodyQue(void);
void ClientBeginServerFrame(edict_t *ent);
int SexedSoundIndex(edict_t *ent, char *base);

/* g_player.c */
int player_pain(edict_t *self, edict_t *other, float kick, int damage);
void player_die(edict_t *self, edict_t *inflictor, edict_t *attacker,
		int damage, vec3_t point);
void player_dismember(edict_t *self, edict_t *other, int damage, int HitLocation);
void ResetPlayerBaseNodes(edict_t *ent);
void player_repair_skin(edict_t *self);

/* g_svcmds.c */
void ServerCommand(void);
qboolean SV_FilterPacket(char *from);

/* p_view.c */
void ClientEndServerFrame(edict_t *ent);
qboolean CheckButton(edict_t *self);
void SetupPlayerinfo(edict_t *ent);
void WritePlayerinfo(edict_t *ent);
void SetupPlayerinfo_effects(edict_t *ent);
void WritePlayerinfo_effects(edict_t *ent);
void InitPlayerinfo(edict_t *ent);

/* p_hud.c */
void MoveClientToIntermission(edict_t *client);
void MoveClientsToIntermission(vec3_t ViewOrigin, vec3_t ViewAngles);
void G_SetStats(edict_t *ent);
void ValidateSelectedItem(edict_t *ent);
void SelectPrevItem(edict_t *ent, int itflags);
void SelectNextItem(edict_t *ent, int itflags);
void DeathmatchScoreboardMessage(edict_t *client, edict_t *killer);

/* g_pweapon.c */
void PlayerNoise(edict_t *who, vec3_t where, int type);

/* g_resourcemanagers.c */
void G_InitResourceManagers();

/* m_move.c */
qboolean M_CheckBottom(edict_t *ent);
qboolean M_CheckTop(edict_t *ent);
qboolean M_walkmove(edict_t *ent, float yaw, float dist);
void M_MoveToGoal(edict_t *ent, float dist);
float M_ChangeYaw(edict_t *ent);
void M_ChangePitch(edict_t *ent);
void M_MoveAwayFromGoal(edict_t *ent, float dist);

/* g_phys.c */
void G_RunEntity(edict_t *ent);

/* g_main.c */
void SaveClientData(void);


/* g_breakable.c */
void KillBrush(edict_t *targ,edict_t *inflictor,edict_t *attacker,int damage);

/* g_obj.c */
void ObjectInit(edict_t *self,int health,int mass, int materialtype,int solid);


/* g_spawnf.c */
//sfs--this is used to get a classname for guys spawned while game is running
char *ED_NewString(const char *string);
void ED_CallSpawn(edict_t *ent);


//============================================================================


// ************************************************************************************************
// TRYSTEP_
// --------
// Used for ai_trystep (g_ai)
// ************************************************************************************************

#define TRYSTEP_OK			0
#define TRYSTEP_ALLSOLID	1
#define TRYSTEP_STARTSOLID	2
#define TRYSTEP_OFFEDGE		3
#define TRYSTEP_NOSUPPORT	4
#define TRYSTEP_INWATER		5

/* client data that stays across deathmatch respawns */
typedef struct
{
	client_persistant_t coop_respawn; /* what to set client->pers to on a respawn */
	int enterframe; /* level.framenum the client entered the game */
	int score; /* frags, etc */
	vec3_t cmd_angles; /* angles sent over in the last command */

	int					game_helpchanged;
	int					helpchanged;
	qboolean spectator;             /* client is a spectator */
} client_respawn_t;

/* this structure is cleared on each PutClientInServer(),
   except for 'client->pers' */
struct gclient_s
{
	/* known to server */
	player_state_t ps; /* communicated by server to clients */
	int ping;

	// All other fields below are private to the game.

	client_respawn_t	resp;
	pmove_state_t		old_pmove;				// For detecting out-of-pmove changes.

	// Damage stuff. Sum up damage over an entire frame.

	qboolean			damage_gas;				// Did damage come from plague mist?
	int					damage_blood;			// Damage taken out of health.
	int					damage_knockback;		// Impact damage.
	vec3_t				damage_from;			// Origin for vector calculation.

	//

	usercmd_t			pcmd;
	short				oldcmdangles[3];
	vec3_t				aimangles;				// Spell / weapon aiming direction.
	vec3_t				oldviewangles;
	vec3_t				v_angle;				// Entity facing angles.
	float				bobtime;				// So off-ground doesn't change it.
	float				next_drown_time;
	int					old_waterlevel;

	// Client can respawn when time > respawn_time.

	float				respawn_time;
	int					complete_reset;

	//  Remote and walkby camera stuff.

	int					RemoteCameraLockCount;
	int					RemoteCameraNumber;
	int					savedtargetcount;
	edict_t				*savedtarget;

	// Teleport stuff.

	vec3_t				tele_dest;
	vec3_t				tele_angles;
	int					tele_count;
	int					tele_type;				/// Note only a byte of this is used.
	int					old_solid;

	// Weapon / defense stuff.

	edict_t				*lastentityhit;
	edict_t				*Meteors[4];
	vec3_t				laststaffpos;
	float				laststaffuse;

	// Powerup timers.

	float				invincible_framenum;

	// Shrine stuff.

	float				shrine_framenum;

	// Data for the player obituaries

	MOD_t				meansofdeath;

	// Anti flooding vars

	float				flood_locktill;			// locked from talking
	float				flood_when[10];			// when messages were said
	int					flood_whenhead;			// head pointer for when said
	float				flood_nextnamechange;	// next time for valid nick change
	float				flood_nextkill;			// next time for suicide

	playerinfo_t		playerinfo;

	/* Third person view */
	int chasetoggle;
	edict_t *chasecam;
	edict_t *oldplayer;
	int use;
	int zoom;
	int delayedstart;
};

// sides for a nonrotating box
typedef enum Box_BoundingForm_Sides_e
{
	BOX_BOUNDINGFORM_SIDE_WEST,
	BOX_BOUNDINGFORM_SIDE_NORTH,
	BOX_BOUNDINGFORM_SIDE_SOUTH,
	BOX_BOUNDINGFORM_SIDE_EAST,
	BOX_BOUNDINGFORM_SIDE_BOTTOM,
	BOX_BOUNDINGFORM_SIDE_TOP,
	NUM_BOX_BOUNDINGFORM_SIDES
} Box_BoundingForm_Sides_t;

qboolean FindTarget (edict_t *self);
void MG_PostDeathThink (edict_t *self);
qboolean movable (edict_t *ent);
qboolean EntReflecting(edict_t *ent, qboolean checkmonster, qboolean checkplayer);
void SkyFly (edict_t *self);
void Use_Multi(edict_t *self, edict_t *other, edict_t *activator);
void c_swapplayer(edict_t *Self,edict_t *Cinematic);
void remove_non_cinematic_entites(edict_t *owner);
void reinstate_non_cinematic_entites(edict_t *owner);

//For simplicity of use.. take it out later

#define BUOY_DEBUG	showbuoys->value
#define BUOY_DEBUG_LITE	showlitebuoys->value
#define MGAI_DEBUG	mgai_debug->value
#define DEACTIVATE_BUOYS deactivate_buoys->value
#define ANARCHY anarchy->value
#define IMPACT_DAMAGE impact_damage->value
#define CHEATING_MONSTERS cheating_monsters->value

void ProcessScripts(void);
void ShutdownScripts(qboolean Complete);

typedef struct pushed_s
{
	edict_t* ent;
	vec3_t	origin;
	vec3_t	angles;
	float	deltayaw;
} pushed_t;
extern pushed_t	pushed[MAX_EDICTS], * pushed_p;

extern edict_t *obstacle;
extern player_export_t *playerExport;	// interface to player library.

#include "../../common/header/common.h"
#include "g_message.h"
#include "../common/message.h"
#include "g_classstatics.h"

#define MAX_BUOY_BRANCHES 3

struct edict_s
{
	entity_state_t s;
	struct gclient_s *client; /* NULL if not a player
							     the server expects the first part
							     of gclient_s to be a player_state_t
							     but the rest of it is opaque */

	qboolean inuse;
	int linkcount;

	link_t area; /* linked to a division node or leaf */

	int num_clusters; /* if -1, use headnode instead */
	int clusternums[MAX_ENT_CLUSTERS];
	int headnode; /* unused if num_clusters != -1 */
	int areanum, areanum2;

	/* ================================ */

	int svflags;
	vec3_t mins, maxs;
	vec3_t absmin, absmax, size;
	solid_t solid;
	int clipmask;
	edict_t *owner;

	/* DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER */
	/* EXPECTS THE FIELDS IN THAT ORDER! */

	/* ================================ */
	int movetype;
	int flags;

	edict_t *groundentity;		// entity serving as ground
	int groundentity_linkcount;	// if self and groundentity's don't match, groundentity should be cleared
	vec3_t groundNormal;		// normal of the ground

	vec3_t intentMins, intentMaxs;	// if PF_RESIZE is set, then physics will attempt to change
					// the ents bounding form to the new one indicated
					// If it was succesfully resized, the PF_RESIZE is turned off
					// otherwise it will remain on.

	// called when self is the collidee in a collision, resulting in the impediment or bouncing of trace->ent
	void				(*isBlocking)(edict_t *self, trace_t *trace);

	MsgQueue_t			msgQ;
	G_MessageHandler_t	msgHandler;
	int					classID;

	void				(*think)(edict_t *self);
	void				(*ai)(edict_t *self);
	float				freetime;			// Server time when the object was freed.
	char				*classname;
	int					spawnflags;

	// Used by the game physics.

	int					physicsFlags;

	edict_t				*blockingEntity;			// entity serving as ground
	int					blockingEntity_linkcount;	// if self and blockingEntity's don't match, blockingEntity should be
													// cleared
	vec3_t				blockingNormal;				// normal of the blocking surface

	// called when self bounces off of something and continues to move unimpeded
	void				(*bounced)(edict_t *self, trace_t *trace);

	// called when self is the collider in a collision, resulting in the impediment of self's movement
	void				(*isBlocked)(edict_t *self, trace_t *trace);

	float				friction;		// friction multiplier; defaults to 1.0
	// Used to determine whether something will stop, slide, or bounce on impact
	float				elasticity;

	// Used by anything that can collide (physics).

	void				(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

	// Used to indicate teams, (a way to group things).

	char				*team;			// Team name.

	union {
	edict_t				*teamchain;
	edict_t				*targetEnt;
	edict_t				*slavechain;
	edict_t				*rope_grab;		//Used to by the rope to hold the part of the rope which the player is holding
	};

	union {
		edict_t				*guidemaster;
		edict_t				*teammaster;
	};

	float				nextthink;

	// Fields used by only one class of game entity (monster, player, poly, trigger, etc).

	char				*model;			// Model name, which could instead be stored in

	char				*message;		// index to message_text for printing and wave files

	char				*text_msg;		// Text printed to con for door, polys, triggers, etc.

	// These really all could be changed to ints or hashed or something (currently, we do a search
	// on all the active edicts using strcmps). We should be able to assign indexes in the BSP, by
	// doing the string strcmps at BSP time. The player seem to use any of the target stuff.

	char				*target;		// Name of the entity's target.
	char				*targetname;	// What other entities use to target-lock this entity.
	char				*scripttarget;
	char				*killtarget;	// Used only in G_UseTargets, which fires all
										// of its targets.
	char				*pathtarget;	// Used by trains, also used by monsters in some
										// way as well.
	union {
	char				*deathtarget;	// Target to be triggered on my death.
	char				*pathtargetname; // Used to target buoys to doors or plats or etc
	};

	union {
	char				*combattarget;	// Monsters are the primary user of this.
	char				*jumptarget;	// for buoys only
	};

	union {
	edict_t				*target_ent;	// Used by player, trains, and turrets. Monsters
										// should be able to use this for current target as well.
	edict_t				*slave;
	edict_t				*rope_end;		//Used by the rope to store the rope end entity
	};

	vec3_t				movedir;		// Used by just about everything that moves, but not used in
										// physics.

	float				air_finished;	// Used by things that can breath (monsters and player).

	edict_t				*goalentity;	// Used primarily by monsters.
	edict_t				*movetarget;	// Used primarily by monsters, also a little use
										// by poly/trigger
	float				yaw_speed;		// Used by monsters and player.
	float				ideal_yaw;		// Used by monsters and player.
	float				ideal_pitch;	// Used by monsters and player.
	float				yawOffset;		// Used in CreateMove_Step

	float				accel;			// Used mostly in g_func.c.
	float				decel;			// Used mostly in g_func.c.

	float timestamp;		// Used by a couple of ojects.

	// Used by just about every type of entity.

	void (*use)(edict_t *self, edict_t *other, edict_t *activator);

	int health;			// Used by anything that can be destroyed.
	int max_health;		// Used by anything that can be destroyed.
	int bloodType;		// type of stuff to spawn off when hit

	union {
	int deadflag;		// More like a dead state, used by things that can die.
										// Would probably be better off with a more general state
										// (or two or three).
	int deadState;
	};

	union {
	qboolean			show_hostile;	// Only used by monsters (or g_ai.c at least)- not really
										// sure what for.
	void				(*TriggerActivated)(edict_t *self, edict_t *activator);
	};

	char				*map;			// target_changelevel (used only by).

	int					viewheight;		// height above origin where eyesight is determined
										// used by anything which can "see", player and monsters
	float				reflected_time;	// used by objects to tell if they've been repulsed by something..

	// Except for a DAMAGE_AIM value in the turret and one commented out in the player this looks
	// like a flag indicating if it takes damage anyone ever heard of using a bit for a flag, or
	// even better a whole bunch of flags in an 'int'.

	int					takedamage;

	// Unless something will do both normal and radius damage, we only need one field. In fact we
	// may want to move this into class statics or something.

	int					dmg;			// the damage something does.
	float				dmg_radius;		// the radius of damage.

	int					sounds;			// used by a trigger and a splash, could be a class static

	union {
	int					count;			// used by polys, triggers, and items.
	int					curr_model;		// used by player during cinematics
	};

	int					targeted;		// used by Ogle to denote a targeted action queued up
	int					lastbuoy;		// used to save a buoy in checking

	edict_t				*chain;			// used by items and player in the body queue.
	edict_t				*enemy;			// used by monsters, player, and a poly or two.
	edict_t				*oldenemy;		// used by monsters.
	edict_t				*activator;		// this that used something, used by monsters, items, and
										// polys.
	// Used by player only.

	edict_t				*mynoise;		// Can go in client only.
	edict_t				*mynoise2;

	edict_t				*last_buoyed_enemy;		// used by monsters.

	int					noise_index;	// Used only by targets

	float				volume;			// used only by target_speaker

	union {
	float				attenuation;	// used only by target_speaker
	float				maxrange;		// used for ai
	};

	// Timing variables.

	float				wait;			// Used by polys, triggers, and targets.
	float				delay;			// Delay before firing targets. Used by a few polys and targets.
	float				random;			// Used by func_timer and spl_meteorbarrier.

	// Used to delay monster 5 before going after a player sound. Only set on player.

	union
	{
	float				teleport_time;
	float				time;			// misc. time for whatever
	};

	// Move these to clientinfo?

	// What it's made of, i.e. MAT_XXX. Used to determine gibs to throw. Curently used only by the
	// barrel, but applicable to anything generically gibbable.

	int					materialtype;
	int					PersistantCFX;	// index to client effect linked to edict
	int					Leader_PersistantCFX;	// non of this should really go in here.. really it should be in the client, but its 2 in the morning, so fuck it

	vec3_t				velocity;			// linear velocity
	vec3_t				avelocity;			// angular velocity
	vec3_t				knockbackvel;

	// Used for determining effects of liquids in the environment.

	float				speed;
	int					watertype;		// Used to indicate current liquid actor is in.
	int					waterlevel;		// Used by monsters and players and grenades.

	int					mass;
	float				gravity;		// Per entity gravity multiplier (1.0 is normal) Used for
										// lowgrav artifact, flares.

	// Not currently used by anyone, but it's a part of physics. Probably should remove it.
	void				(*prethink) (edict_t *ent);

	// Move into the moveinfo structure? Used by polys and turret and in physics.
	void				(*blocked)(edict_t *self, edict_t *other);

	// Used by animating entities.

	int					curAnimID;
	int					lastAnimID;

	// Used by monsters and player.

	int				(*pain)(edict_t *self, edict_t *other, float kick, int damage);

	// Used by monsters, player, and some polys.

	void				(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

	// used by the Morph Ovum

	void				(*oldthink)(edict_t *self);

	// Used by polys and triggers.

	float				touch_debounce_time;

	// Used by monsters and player.

	float				pain_debounce_time;

	// Used by monsters and player.

	float				damage_debounce_time;
	float				attack_debounce_time;

	//used by reflecting projectiles
	int					reflect_debounce_time;

	float				impact_debounce_time;	// impact damage debounce time

	float				fire_damage_time;		// fire damage length
	float				fire_timestamp;			// timestamp weapons and damaged entities,
												//		so that the same weapon can't hurt an entity twice

	// Used by shrines

	void				(*oldtouch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
// jmarshall - 64bit
	union {
	size_t			shrine_type;
	size_t			morph_timer;
	size_t			buoy_index;
	};
// jmarshall end

	// Only set in trigger_push_touch and probably only on players.

	union {
	float				last_buoy_time;
	float				fly_sound_debounce_time;
	};

	union {
	float				last_move_time;	// Only used by target_earthquake (poly/trigger)
	float				old_yaw;		// Used by the Seraph to return to his exact position and angles
	};

	vec3_t				pos1,pos2;		// Used by polys and turrets.

	/* move this to clientinfo? */
	int light_level;

	int style; /* also used as areaportal number */

	gitem_t *item; /* for bonus items */

	/* common data blocks */
	moveinfo_t moveinfo;
	monsterinfo_t monsterinfo;

	vec3_t				v_angle_ofs;			//View Angle ofset- for when monsters look around, for line of sight checks

	int					ai_mood;		// Used by high level ai to relay simple moods to lower level functions (INTEGRAL FOR SWITCH)
	int					ai_mood_flags;	// Used in conjunction with ai_mood to provide more information to the lower functions
	byte				mintel;			//number of buoys allowed to follow

	char				*target2;

	vec3_t				last_org;

	//next 4 in a table in m_stats
	float				min_melee_range;// Min distance at which it is ok for this monster to use it's melee attack
	float				melee_range;	//Max distance to do a melee attack, if negative, closest monster should get to enemy
	float				missile_range;	//Max distance to do a missile attack
	float				min_missile_range;	//Least distance to do a missile attack
	int					bypass_missile_chance;//chance to not use missile attack even if you can (0-100)
	void				(*cant_attack_think)(edict_t *self, float enemydist, qboolean enemyvis, qboolean enemyinfront);//called only when monster cannot attack player

	int					jump_chance;//chance to jump when have opportunity

	float				wakeup_distance;//how far the player can be when I see him to wake me up

	float				evade_debounce_time;//How long to evade for
	float				oldenemy_debounce_time;//How long to hunt enemy before looking for oldenemy again

	float				best_move_yaw;
//	int					jump_chance;	//0 - 100 chance that monster will try to jump to get around when possible/neccessary - in a table in m_stats

	float				mood_nextthink;			//not used anymore?
	void				(*mood_think)(edict_t *self);//you mood setting function

	float				next_pre_think;//any prethinking you want a monster to do
	void				(*pre_think)(edict_t *self);//nextthink time for prethinks

	float				next_post_think;//any prethinking you want a monster to do
	void				(*post_think)(edict_t *self);//nextthink time for prethinks

	int					forced_buoy;	//monster is forced to go to this buoy
	buoy_t				*enemy_buoy;	//monster's enemy's closest buoy
	float				pathfind_nextthink;			//not used anymore?
	edict_t				*nextbuoy[MAX_BUOY_BRANCHES];

	float				dead_size;			//for dead thinking
	struct volume_effect_s	*volfx;

	//New monster stuff
	char				*wakeup_target;		//target to fire when find an enemy
	char				*pain_target;		//target to fire when take pain (only once)
	char				*homebuoy;			//yeah, buoyyyyyyy!

	float				alert_time;			//time when a monster is no longer startled
	alertent_t			*last_alert;		//last alert_ent to startle me, if it's the same one, skip it when going through the list

	edict_t				*placeholder;		//used by assassin to hold his teleport destination

	float				jump_time;			//time that a monster's protection from falling damage runs out after a jump

	int					red_rain_count;		//number of red rains you can have at once

	int					deathtype;			//how you died

	edict_t				*fire_damage_enemy;	//who burnt you to death- for proper burning death credit

	void			*script;

	/* Third person view */
	int chasedist1;
	int chasedist2;
};

void Cmd_Chasecam_Toggle(edict_t *ent);
void ChasecamStart(edict_t *ent);
void ChasecamRemove(edict_t *ent);
void CheckChasecam_Viewent(edict_t *ent);
void ChasecamTrack(edict_t *ent);

// NOTE: 1 means that the last entity was a wall...
#define WALL_ENTITY (struct edict_s *)1

/*
 * Uncomment for check that exported functions declarations are same as in
 * implementation. (-Wmissing-prototypes )
 *
 */
#if 0
#include "../savegame/savegame.h"
#include "../savegame/tables/gamefunc_decs.h"
#endif

/* Heretic 2 */
void G_CPrintf(edict_t* ent, int printlevel, short stringid);
void G_BCaption(int printlevel, short stringid);
void G_LevelMsgCenterPrintf(edict_t* ent, short msg);
void G_CaptionPrintf(edict_t* ent, short msg);
void G_BroadcastObituary(int printlevel, short stringid, short client1, short client2);
int G_GetContentsAtPoint(vec3_t point);
int G_FindEntitiesInBounds(vec3_t mins, vec3_t maxs, struct SinglyLinkedList_s* list, int areatype);
void G_TraceBoundingForm(FormMove_t* formMove);
void G_MsgVarCenterPrintf(edict_t* ent, short msg, int vari);
void G_MsgDualCenterPrintf(edict_t* ent, short msg1, short msg2);
qboolean G_ResizeBoundingForm(edict_t* self, struct FormMove_s* formMove);
qboolean G_CheckDistances(vec3_t origin, float dist);
void G_SoundRemove(char* name);
void G_CleanLevel(void);
void G_SoundEvent(byte EventId, float leveltime, edict_t* ent, int channel, int soundindex, float volume, float attenuation, float timeofs);

#endif /* GAME_LOCAL_H */
