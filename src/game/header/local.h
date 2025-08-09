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
#include "../../common/header/common.h"
#include "../common/vector.h"
#include "g_itemstats.h"
#include "../common/q_physics.h"
#include "../header/g_playstats.h"
#include "../header/utilities.h"
#include "../common/fx.h"

/* the "gameversion" client command will print this plus compile date */
#define GAMEVERSION "yQRHeretic2"

/* protocol bytes that can be directly added to messages */
#define svc_muzzleflash 1
#define svc_muzzleflash2 2
#define svc_temp_entity 3
#define svc_layout 4
#define svc_inventory 5
#define svc_stufftext 11

/* ================================================================== */

/* view pitching times */
#define DAMAGE_TIME 0.5
#define FALL_TIME 0.3

/* these are set with checkboxes on each entity in the map editor */

/* edict->spawnflags */
#define SPAWNFLAG_NOT_EASY 0x00000100
#define SPAWNFLAG_NOT_MEDIUM 0x00000200
#define SPAWNFLAG_NOT_HARD 0x00000400
#define SPAWNFLAG_NOT_DEATHMATCH 0x00000800
#define SPAWNFLAG_NOT_COOP 0x00001000

/* edict->flags */
#define FL_FLY 0x00000001
#define FL_SWIM 0x00000002                  /* implied immunity to drowining */
#define FL_IMMUNE_LASER 0x00000004
#define FL_INWATER 0x00000008
#define FL_GODMODE 0x00000010
#define FL_NOTARGET 0x00000020
#define FL_IMMUNE_SLIME 0x00000040
#define FL_IMMUNE_LAVA 0x00000080
#define FL_PARTIALGROUND 0x00000100         /* not all corners are valid */
#define FL_WATERJUMP 0x00000200             /* player jumping out of water */
#define FL_TEAMSLAVE 0x00000400             /* not the first on the team */
#define FL_NO_KNOCKBACK 0x00000800
#define FL_POWER_ARMOR 0x00001000           /* power armor (if any) is active */
#define FL_COOP_TAKEN 0x00002000            /* Another client has already taken it */
#define FL_RESPAWN 0x80000000               /* used for item respawning */

#define FL_MECHANICAL 0x00002000            /* entity is mechanical, use sparks not blood */
#define FL_SAM_RAIMI 0x00004000             /* entity is in sam raimi cam mode */
#define FL_DISGUISED 0x00008000             /* entity is in disguise, monsters will not recognize. */
#define FL_NOGIB 0x00010000                 /* player has been vaporized by a nuke, drop no gibs */
#define FL_FLASHLIGHT 0x00020000            /* enable flashlight */

#define FRAMETIME 0.1

/* memory tags to allow dynamic memory to be cleaned up */
#define TAG_GAME 765        /* clear when unloading the dll */
#define TAG_LEVEL 766       /* clear when loading a new level */

#define MELEE_DISTANCE 80
#define BODY_QUEUE_SIZE 8
#define STEPSIZE 18

typedef enum
{
	DAMAGE_NO,
	DAMAGE_YES,         /* will take damage if hit */
	DAMAGE_AIM          /* auto targeting recognizes this */
} damage_t;

typedef enum
{
	WEAPON_READY,
	WEAPON_ACTIVATING,
	WEAPON_DROPPING,
	WEAPON_FIRING
} weaponstate_t;

typedef enum
{
	AMMO_MANA_DEFENSIVE_HALF,
	AMMO_MANA_DEFENSIVE_FULL,
	AMMO_MANA_OFFENSIVE_HALF,
	AMMO_MANA_OFFENSIVE_FULL,
	AMMO_MANA_COMBO_QUARTER,
	AMMO_MANA_COMBO_HALF,
	AMMO_HELLSTAFF,
	AMMO_REDRAIN,
	AMMO_PHOENIX,
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_GRENADES,
	AMMO_CELLS,
	AMMO_SLUGS,
	AMMO_MAGSLUG,
	AMMO_TRAP,
	AMMO_FLECHETTES,
	AMMO_TESLA,
	AMMO_PROX,
	AMMO_DISRUPTOR
} ammo_t;

/* Maximum debris / gibs per frame */
#define MAX_GIBS 20
#define MAX_DEBRIS 20

/* deadflag */
#define DEAD_NO 0
#define DEAD_DYING 1
#define DEAD_DEAD 2
#define DEAD_RESPAWNABLE 3

/* range */
#define RANGE_MELEE 0
#define RANGE_NEAR 1
#define RANGE_MID 2
#define RANGE_FAR 3

/* gib types */
#define GIB_ORGANIC 0
#define GIB_METALLIC 1

/* monster ai flags */
#define AI_STAND_GROUND 0x00000001
#define AI_TEMP_STAND_GROUND 0x00000002
#define AI_SOUND_TARGET 0x00000004
#define AI_LOST_SIGHT 0x00000008
#define AI_PURSUIT_LAST_SEEN 0x00000010
#define AI_PURSUE_NEXT 0x00000020
#define AI_PURSUE_TEMP 0x00000040
#define AI_HOLD_FRAME 0x00000080
#define AI_GOOD_GUY 0x00000100
#define AI_BRUTAL 0x00000200
#define AI_NOSTEP 0x00000400
#define AI_DUCKED 0x00000800
#define AI_COMBAT_POINT 0x00001000
#define AI_MEDIC 0x00002000
#define AI_RESURRECTING 0x00004000
#define AI_IGNORE_PAIN 0x00008000

/* ROGUE */
#define AI_WALK_WALLS 0x00008000
#define AI_MANUAL_STEERING 0x00010000
#define AI_TARGET_ANGER 0x00020000
#define AI_DODGING 0x00040000
#define AI_CHARGING 0x00080000
#define AI_HINT_PATH 0x00100000
#define AI_IGNORE_SHOTS 0x00200000
#define AI_DO_NOT_COUNT 0x00400000          /* set for healed monsters */
#define AI_SPAWNED_CARRIER 0x00800000       /* both do_not_count and spawned are set for spawned monsters */
#define AI_SPAWNED_MEDIC_C 0x01000000       /* both do_not_count and spawned are set for spawned monsters */
#define AI_SPAWNED_WIDOW 0x02000000         /* both do_not_count and spawned are set for spawned monsters */
#define AI_SPAWNED_MASK 0x03800000          /* mask to catch all three flavors of spawned */
#define AI_BLOCKED 0x04000000               /* used by blocked_checkattack: set to say I'm attacking while blocked */
                                            /* (prevents run-attacks) */
/* monster attack state */
#define AS_STRAIGHT 1
#define AS_SLIDING 2
#define AS_MELEE 3
#define AS_MISSILE 4
#define AS_BLIND 5

/* armor types */
#define ARMOR_NONE 0
#define ARMOR_JACKET 1
#define ARMOR_COMBAT 2
#define ARMOR_BODY 3
#define ARMOR_SHARD 4

/* power armor types */
#define POWER_ARMOR_NONE 0
#define POWER_ARMOR_SCREEN 1
#define POWER_ARMOR_SHIELD 2

/* handedness values */
#define RIGHT_HANDED 0
#define LEFT_HANDED 1
#define CENTER_HANDED 2

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

/* noise types for PlayerNoise */
#define PNOISE_SELF 0
#define PNOISE_WEAPON 1
#define PNOISE_IMPACT 2

/* edict->movetype values */
typedef enum
{
	MOVETYPE_NONE,      /* never moves */
	MOVETYPE_STATIC,
	MOVETYPE_NOCLIP,    /* origin and angles change with no interaction */
	MOVETYPE_STEP,      /* gravity, special edge handling */
	MOVETYPE_FLY,
	MOVETYPE_PUSH,      /* no clip to world, push on box contact */
	MOVETYPE_STOP,      /* no clip to world, stops on box contact */

	MOVETYPE_WALK,      /* gravity */
	MOVETYPE_TOSS,      /* gravity */
	MOVETYPE_FLYMISSILE, /* extra size to monsters */
	MOVETYPE_BOUNCE, /* added this (the comma at the end of line) */
	MOVETYPE_WALLBOUNCE,
	MOVETYPE_NEWTOSS,    /* for deathball */
	MOVETYPE_SCRIPT_ANGULAR,	// moves with the rotation of another entity
} movetype_t;

typedef struct
{
	int base_count;
	int max_count;
	float normal_protection;
	float energy_protection;
	int armor;
	float max_armor;
	float spell_protection;
} gitem_armor_t;

/* gitem_t->flags */
#define IT_WEAPON 0x00000001                /* use makes active weapon */
#define IT_AMMO 0x00000002
#define IT_ARMOR 0x00000004
#define IT_STAY_COOP 0x00000008
#define IT_KEY 0x00000010
#define IT_POWERUP 0x00000020
#define IT_MELEE 0x00000040
#define IT_NOT_GIVEABLE 0x00000080      /* item can not be given */
#define IT_INSTANT_USE 0x000000100		/* item is insta-used on pickup if dmflag is set */
#define IT_TECH 0x000000200 /* CTF */

/* gitem_t->weapmodel for weapons indicates model index */
#define IT_HEALTH 0x000000400 /* JABot */
#define IT_FLAG	0x000000800 /* JABot */

/* gitem_t->weapmodel for weapons indicates model index */
#define WEAP_BLASTER 1
#define WEAP_SHOTGUN 2
#define WEAP_SUPERSHOTGUN 3
#define WEAP_MACHINEGUN 4
#define WEAP_CHAINGUN 5
#define WEAP_GRENADES 6
#define WEAP_GRENADELAUNCHER 7
#define WEAP_ROCKETLAUNCHER 8
#define WEAP_HYPERBLASTER 9
#define WEAP_RAILGUN 10
#define WEAP_BFG 11
#define WEAP_PHALANX 12
#define WEAP_BOOMER 13
#define WEAP_DISRUPTOR 14
#define WEAP_ETFRIFLE 15
#define WEAP_PLASMA 16
#define WEAP_PROXLAUNCH 17
#define WEAP_CHAINFIST 18
#define WEAP_GRAPPLE 19

//JABot[start]
#define WEAP_NONE			0
#define WEAP_TOTAL			20
//JABot[end]

typedef struct gitem_s
{
	char *classname; /* spawning name */
	qboolean (*pickup)(struct edict_s *ent, struct edict_s *other);
	void (*use)(struct edict_s *ent, struct gitem_s *item);
	void (*drop)(struct edict_s *ent, struct gitem_s *item);
	void (*weaponthink)(struct edict_s *ent, char *Format,...);
	char *pickup_sound;
	char *world_model;
	int world_model_flags;
	char *view_model;

	/* client side info */
	char *icon;
	char *pickup_name;          /* for printing on pickup */
	int count_width;            /* number of digits to display by icon */

	int quantity;               /* for ammo how much, for weapons how much is used per shot */
	char *ammo;                 /* for weapons */
	int flags;                  /* IT_* flags */

	int weapmodel;              /* weapon model index (for weapons) */

	void *info;
	int tag;

	char *precaches;            /* string of all models, sounds, and images this item will use */
	short msg_pickup;           /* Pickup string id. */
	short msg_nouse;            /* Can`t use. */
	vec3_t mins;                /* Bounding box */
	vec3_t maxs;                /* Bounding box */
	int playeranimseq;          /* The ASEQ_ player sequence that should be engaged when this item is used. */
	int altanimseq;             /* Powerup animation sequence */
	int MaxActive;              /* Maximum allowable active uses of items of this type by a single
				 	player, at any instant in time. -1 indicates no limit. */
} gitem_t;

/* this structure is left intact through an entire game
   it should be initialized at dll load time, and read/written to
   the server.ssv file for savegames */
typedef struct
{
	char helpmessage1[512];
	char helpmessage2[512];
	int helpchanged;            /* flash F1 icon if non 0, play sound */
	                            /* and increment only if 1, 2, or 3 */

	gclient_t *clients;         /* [maxclients] */

	/* can't store spawnpoint in level, because
	   it would get overwritten by the savegame
	   restore */
	char spawnpoint[512];       /* needed for coop respawns */

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

// Forward define 'playerinfo_t' for use in 'panimframe_t' and 'panimmove_t'.
typedef struct playerinfo_s playerinfo_t;

// ************************************************************************************************
// panimframe_t
// ------------
// ************************************************************************************************

typedef struct
{
	int		framenum;
	void	(*movefunc)(playerinfo_t *playerinfo,float var1,float var2,float var3);
	float	var1,var2,var3;
	void	(*actionfunc)(playerinfo_t *playerinfo,float var4);
	float	var4;
	void	(*thinkfunc)(playerinfo_t *playerinfo);
} panimframe_t;

// ************************************************************************************************
// panimmove_t
// -----------
// ************************************************************************************************

typedef struct
{
	int				numframes;
	panimframe_t	*frame;
	void			(*endfunc)(playerinfo_t *playerinfo);
} panimmove_t;

// ************************************************************************************************
// paceldata_t
// -----------
// ************************************************************************************************

typedef struct
{
	panimmove_t	*move;
	short		fly;
	short		lockmove;
	int			playerflags;
} paceldata_t;

// ************************************************************************************************
// pacelsizes_t
// ------------
// ************************************************************************************************

typedef struct
{
	vec3_t	boundbox[2];
	int		altmove;
	float	viewheight;
	float	waterheight;
} pacelsizes_t;

// ************************************************************************************************
// weaponready_e
// -------------
// Indicates what actual weapon model the player has readied.
// ************************************************************************************************

enum weaponready_e
{
	WEAPON_READY_NONE,
	WEAPON_READY_HANDS,
	WEAPON_READY_STAFFSTUB,
	WEAPON_READY_SWORDSTAFF,
	WEAPON_READY_HELLSTAFF,
	WEAPON_READY_BOW,
	WEAPON_READY_MAX
};

// ************************************************************************************************
// armortype_e
// -----------
// Indicates what actual armor the player is wearing.
// ************************************************************************************************

enum armortype_e
{
	ARMOR_TYPE_NONE,
	ARMOR_TYPE_SILVER,
	ARMOR_TYPE_GOLD
};

// ************************************************************************************************
// bowtype_e
// -----------
// Indicates what actual bow the player has currently on his back.
// ************************************************************************************************

enum bowtype_e
{
	BOW_TYPE_NONE,
	BOW_TYPE_REDRAIN,
	BOW_TYPE_PHOENIX
};

// ************************************************************************************************
// stafftype_e
// -----------
// Indicates what powerup level of the staff the player has.
// ************************************************************************************************

enum stafftype_e
{
	STAFF_LEVEL_NONE,
	STAFF_LEVEL_BASIC,
	STAFF_LEVEL_POWER1,
	STAFF_LEVEL_POWER2,
	STAFF_LEVEL_MAX
};

// ************************************************************************************************
// helltype_e
// -----------
// Indicates what powerup level of the staff the player has.
// ************************************************************************************************

enum helltype_e
{
	HELL_TYPE_NONE,
	HELL_TYPE_BASIC,
	HELL_TYPE_POWER
};

#define PICKUP_MIN  {0, 0, 0}
#define PICKUP_MAX  {0, 0, 0}

// ************************************************************************************************
// PNOISE_XXX
// ----------
// Noise types for 'PlayerNoise'.
// ************************************************************************************************

#define PNOISE_SELF		0
#define PNOISE_WEAPON	1
#define PNOISE_IMPACT	2

// ************************************************************************************************
// FL_XXX
// ------
// Held in 'edict_t'->flags.
// ************************************************************************************************

#define	FL_FLY				0x00000001
#define	FL_SWIM				0x00000002	// implied immunity to drowining
#define FL_SUSPENDED		0x00000004
#define	FL_INWATER			0x00000008
#define	FL_GODMODE			0x00000010
#define	FL_NOTARGET			0x00000020
#define FL_IMMUNE_SLIME		0x00000040
#define FL_IMMUNE_LAVA		0x00000080
#define	FL_PARTIALGROUND	0x00000100	// not all corners are valid
#define	FL_INLAVA			0x00000200	// INWATER is set when in lava, but this is a modifier so we know when we leave
#define	FL_TEAMSLAVE		0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK		0x00000800
#define	FL_INSLIME			0x00001000	// INWATER is set when in muck, but this is a modifier so we know when we leave
#define FL_LOCKMOVE			0x00002000	// Move updates should not process, actor can only move explicitly
#define FL_DONTANIMATE		0x00004000	// stop animating
#define FL_AVERAGE_CHICKEN	0x00008000	// Currently a chicken.
#define	FL_AMPHIBIAN		0x00010000	// Does not drown on land or in water, but is damaged by muck and lava
#define FL_SUPER_CHICKEN	0x00020000	// Ah yeah...
#define FL_RESPAWN			0x80000000	// used for item respawning

#define FL_CHICKEN			(FL_AVERAGE_CHICKEN | FL_SUPER_CHICKEN)

// ************************************************************************************************
// HANDFX_XXX
// ----------
// Hand effects, glowing for spells. Can be used for staff and bow, or others that are used by the
// upper torso half and toggle.
// ************************************************************************************************

typedef enum handfx_e
{
	HANDFX_NONE=0,
	HANDFX_FIREBALL,
	HANDFX_MISSILE,
	HANDFX_SPHERE,
	HANDFX_MACEBALL,
	HANDFX_FIREWALL,
	HANDFX_REDRAIN,
	HANDFX_POWERREDRAIN,
	HANDFX_PHOENIX,
	HANDFX_POWERPHOENIX,
	HANDFX_STAFF1,
	HANDFX_STAFF2,
	HANDFX_STAFF3,
	HANDFX_MAX,
} handfx_t;

// Unique sound IDs required for correct prediction of sounds played from player.dll. Each ID MUST
// be used ONCE only as it identifies the exact sound call in the player .dll that is responsible
// for playing a sound. These can be be compared with ID's recieved from the server sent sound
// packets (generated by running player.dll code on the server) so that we can decide whether to
// play the server sent sound or not (it may have already been played on the client by client
// prediction. IDs must never exeed +127.

enum
{
	SND_PRED_NULL	= -1,
	SND_PRED_ID0	=  0,
	SND_PRED_ID1,
	SND_PRED_ID2,
	SND_PRED_ID3,
	SND_PRED_ID4,	// Unused.
	SND_PRED_ID5,
	SND_PRED_ID6,
	SND_PRED_ID7,
	SND_PRED_ID8,
	SND_PRED_ID9,
	SND_PRED_ID10,
	SND_PRED_ID11,
	SND_PRED_ID12,
	SND_PRED_ID13,
	SND_PRED_ID14,
	SND_PRED_ID15,
	SND_PRED_ID16,
	SND_PRED_ID17,
	SND_PRED_ID18,
	SND_PRED_ID19,
	SND_PRED_ID20,
	SND_PRED_ID21,
	SND_PRED_ID22,
	SND_PRED_ID23,
	SND_PRED_ID24,
	SND_PRED_ID25,
	SND_PRED_ID26,
	SND_PRED_ID27,
	SND_PRED_ID28,
	SND_PRED_ID29,
	SND_PRED_ID30,
	SND_PRED_ID31,
	SND_PRED_ID32,
	SND_PRED_ID33,
	SND_PRED_ID34,
	SND_PRED_ID35,
	SND_PRED_ID36,
	SND_PRED_ID37,
	SND_PRED_ID38,
	SND_PRED_ID39,
	SND_PRED_ID40,
	SND_PRED_ID41,
	SND_PRED_ID42,
	SND_PRED_ID43,
	SND_PRED_ID44,
	SND_PRED_ID45,
	SND_PRED_ID46,
	SND_PRED_ID47,
	SND_PRED_ID48,
	SND_PRED_ID49,
	SND_PRED_ID50,
	SND_PRED_ID51,
	SND_PRED_ID52,
	SND_PRED_ID53,
	SND_PRED_MAX		= 127
};

// Unique client-effect IDs required for correct prediction of effects started from player.dll. Each ID MUST
// be used ONCE only as it identifies the exact client-effects call in the player .dll that is responsible
// for starting a client-effect. These can be be compared with ID's recieved from the server sent client-effects
// (generated by running player.dll code on the server) so that we can decide whether to start the server sent
// client-effect or not (it may have already been started on the client by client prediction. IDs must never
// exeed +127.

enum
{
	EFFECT_PRED_NULL	= -1,
	EFFECT_PRED_ID0		=  0,
	EFFECT_PRED_ID1,
	EFFECT_PRED_ID2,
	EFFECT_PRED_ID3,
	EFFECT_PRED_ID4,
	EFFECT_PRED_ID5,
	EFFECT_PRED_ID6,
	EFFECT_PRED_ID7,
	EFFECT_PRED_ID8,
	EFFECT_PRED_ID9,
	EFFECT_PRED_ID10,
	EFFECT_PRED_ID11,
	EFFECT_PRED_ID12,
	EFFECT_PRED_ID13,
	EFFECT_PRED_ID14,
	EFFECT_PRED_ID15,
	EFFECT_PRED_ID16,
	EFFECT_PRED_ID17,
	EFFECT_PRED_ID18,
	EFFECT_PRED_ID19,
	EFFECT_PRED_ID20,
	EFFECT_PRED_ID21,
	EFFECT_PRED_ID22,
	EFFECT_PRED_ID23,
	EFFECT_PRED_ID24,
	EFFECT_PRED_ID25,
	EFFECT_PRED_ID26,
	EFFECT_PRED_ID27,
	EFFECT_PRED_ID28,
	EFFECT_PRED_ID29,
	EFFECT_PRED_ID30,
	EFFECT_PRED_ID31,
	EFFECT_PRED_ID32,
	EFFECT_PRED_ID33,
	EFFECT_PRED_ID34,
	EFFECT_PRED_MAX		=127
};

// **************
// Movement rates
// **************

#define IN_MOVE_CREEP_MIN	16
#define IN_MOVE_CREEP		32
#define IN_MOVE_WALK_MIN	48
#define IN_MOVE_WALK		64
#define IN_MOVE_RUN_MIN		80
#define IN_MOVE_RUN			96

#define IN_MOVE_THRESHOLD	IN_MOVE_CREEP_MIN

#define BUTTON_WALK			0

enum movefwd_e
{
	MOVE_BACK_RUN,
	MOVE_BACK_WALK,
	MOVE_BACK_CREEP,
	MOVE_FWD_NONE,
	MOVE_FWD_CREEP,
	MOVE_FWD_WALK,
	MOVE_FWD_RUN,
	MOVE_FWD_MAX
};

enum moveright_e
{
	MOVE_LEFT_RUN,
	MOVE_LEFT_WALK,
	MOVE_LEFT_CREEP,
	MOVE_RIGHT_NONE,
	MOVE_RIGHT_CREEP,
	MOVE_RIGHT_WALK,
	MOVE_RIGHT_RUN,
	MOVE_RIGHT_MAX
};

enum moveplus_e
{
	MOVE_NORM,
	MOVE_NOFWD,
	MOVE_NOSIDE,
};

// ************************************************************************************************
// Skin defines
// -----------
// Indicates what skin Corvus has.
// When indicated on the model, each odd-numbered skin is the damaged version of the previous skin.
// ************************************************************************************************

// For code clarity
#define PLAGUE_NUM_LEVELS 3
#define DAMAGE_NUM_LEVELS 2

#define SKIN_REFLECTION	(DAMAGE_NUM_LEVELS)		// We don't maintain a skin for every plague level anymore.

#define SKIN_MAX		(SKIN_REFLECTION + 1)

#define DEFAULT_PLAYER_LIB "player.so"

void P_Freelib(void);
void* P_Load(void);

void P_Init(void);
void P_Shutdown(void);

void PlayerReleaseRope(playerinfo_t* playerinfo);
void KnockDownPlayer(playerinfo_t* playerinfo);
void PlayFly(playerinfo_t* playerinfo, float dist);
void PlaySlap(playerinfo_t* playerinfo, float dist);
void PlayScratch(playerinfo_t* playerinfo, float dist);
void PlaySigh(playerinfo_t* playerinfo, float dist);
void SpawnDustPuff(playerinfo_t* playerinfo, float dist);
void PlayerInterruptAction(playerinfo_t* playerinfo);

qboolean BranchCheckDismemberAction(playerinfo_t* playerinfo, int weapon);

void TurnOffPlayerEffects(playerinfo_t* playerinfo);
void AnimUpdateFrame(playerinfo_t* playerinfo);
void PlayerFallingDamage(playerinfo_t* playerinfo);

void PlayerBasicAnimReset(playerinfo_t* playerinfo);
void PlayerAnimReset(playerinfo_t* playerinfo);
void PlayerAnimSetLowerSeq(playerinfo_t* playerinfo, int seq);
void PlayerAnimSetUpperSeq(playerinfo_t* playerinfo, int seq);
void PlayerAnimUpperIdle(playerinfo_t* playerinfo);
void PlayerAnimLowerIdle(playerinfo_t* playerinfo);
void PlayerAnimUpperUpdate(playerinfo_t* playerinfo);
void PlayerAnimLowerUpdate(playerinfo_t* playerinfo);
void PlayerAnimSetVault(playerinfo_t* playerinfo, int seq);
void PlayerPlayPain(playerinfo_t* playerinfo, int type);

void PlayerIntLand(playerinfo_t* playerinfo_t, float landspeed);

void PlayerInit(playerinfo_t* playerinfo, int complete_reset);
void PlayerClearEffects(playerinfo_t* playerinfo);
void PlayerUpdate(playerinfo_t* playerinfo);
void PlayerUpdateCmdFlags(playerinfo_t* playerinfo);
void PlayerUpdateModelAttributes(playerinfo_t* playerinfo);

const char *GetClientGroundSurfaceMaterialName(playerinfo_t *playerinfo);

#define	CONTENTS_EMPTY			0x00000000	// nothing
// Only do the trace against the world, not entities within it. Not stored in the .bsp and passed
// only as an argument to trace fucntions.
#define CONTENTS_WORLD_ONLY		0x80000000
#define MONSTER_THINK_INC   0.099
#define FRAMES_PER_SECOND	10.0

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

#define BODY_QUEUE_SIZE		8

#define MELEE_DISTANCE	80

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
#define MAX_MESSAGESTRINGS 1000
typedef struct
{
	char *string;
	char *wav;
} trig_message_t;

// jmarshall: this wasn't extern in the original code,
// this is now correct, wondering if this will cause knock ons?
extern unsigned	*game_msgbuf;
extern trig_message_t game_msgtxt[];

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

	char level_name[MAX_QPATH];         /* the descriptive name (Outer Base, etc) */
	char mapname[MAX_QPATH];            /* the server name (base1, etc) */
	char nextmap[MAX_QPATH];            /* go here when fraglimit is hit */
	char forcemap[MAX_QPATH];           /* go here */

	/* intermission state */
	float intermissiontime;             /* time the intermission was started */
	char *changemap;
	int exitintermission;
	vec3_t intermission_origin;
	vec3_t intermission_angle;

	edict_t *sight_client;			     /* changed once each frame for coop games */

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

	edict_t *current_entity;        /* entity running from G_RunFrame */
	int body_que;                   /* dead bodies */

	int power_cubes;                /* ugly necessity for coop */

	edict_t *disguise_violator;
	int disguise_violation_framenum;

	char *start_items;             /* level start items */
	float next_auto_save;          /* target_autosave */

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
   can be set from the editor, but aren't actualy present/
   in edict_t during gameplay */
typedef struct
{
	/* world vars */
	char *sky;
	float skyrotate;
	int skyautorotate;
	vec3_t skyaxis;
	char *nextmap;
	char *music;

	int lip;
	int distance;
	int height;
	char *noise;
	float pausetime;
	char *item;
	char *gravity;
	char *start_items;

	float minyaw;
	float maxyaw;
	float minpitch;
	float maxpitch;

	/* misc_flare */
	float radius;
	float fade_start_dist;
	float fade_end_dist;
	char *image;
	unsigned rgba;
	char *goals;
	int effects;
	int renderfx;

	/* Addional fields for models */
	vec3_t scale;
	float health_multiplier;

	int weight;//JABot

	/* Heretic 2 */
	int rotate;
	float zangle;
	char *file;

	/* Weapons to be given to the player on spawning. */
	int offensive;
	int defensive;
	int spawnflags2;

	/* Time to wait (in seconds) for all clients to have joined a map in coop. */
	int cooptimeout;

	/* Scripting stuff. */
	char *script;
	char *parms[16];
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

typedef struct
{
	int framenum;
	void (*movefunc)(edict_t *self, float var1, float var2, float var3);
	float var1, var2, var3;
	void (*actionfunc)(edict_t *self, float var4);
	float var4;
	void (*thinkfunc)(edict_t *self);
} mframe_t;

typedef struct
{
	int numframes;
	mframe_t *frame;
	void (*endfunc)(edict_t *self);
} mmove_t;

typedef struct
{
	mmove_t *currentmove;
	unsigned int aiflags;           /* unsigned, since we're close to the max */
	int nextframe;
	float scale;

	void (*stand)(edict_t *self);
	void (*idle)(edict_t *self);
	void (*search)(edict_t *self);
	void (*walk)(edict_t *self);
	void (*run)(edict_t *self);
	void (*dodge)(edict_t *self, edict_t *other, float eta, trace_t *tr);
	void (*attack)(edict_t *self);
	void (*melee)(edict_t *self);
	void (*sight)(edict_t *self, edict_t *other);
	qboolean (*checkattack)(edict_t *self);
	void (*dismember)(edict_t *self, int damage, int HitLocation);
	qboolean (*alert)(edict_t *self, alertent_t *alerter, edict_t *enemy);

	float pausetime;
	float attack_finished;

	vec3_t saved_goal;
	float search_time;
	float trail_time;
	vec3_t last_sighting;
	int attack_state;
	int lefty;
	float idle_time;
	int linkcount;

	int power_armor_type;
	int power_armor_power;

	qboolean (*blocked)(edict_t *self, float dist);
	float last_hint_time;           /* last time the monster checked for hintpaths. */
	edict_t *goal_hint;             /* which hint_path we're trying to get to */
	int medicTries;
	edict_t *badMedic1, *badMedic2; /* these medics have declared this monster "unhealable" */
	edict_t *healer;				/* this is who is healing this monster */
	void (*duck)(edict_t *self, float eta);
	void (*unduck)(edict_t *self);
	void (*sidestep)(edict_t *self);
	float base_height;
	float next_duck_time;
	float duck_wait_time;
	edict_t *last_player_enemy;
	qboolean blindfire;				/* will the monster blindfire? */
	float blind_fire_delay;
	vec3_t blind_fire_target;

	/* used by the spawners to not spawn too much and keep track of #s of monsters spawned */
	int monster_slots;
	int monster_used;
	edict_t *commander;

	/* powerup timers, used by widow, our friend */
	float quad_framenum;
	float invincible_framenum;
	float double_framenum;

	int aistate;						// Last order given to the monster (ORD_XXX).
	int currframeindex;					// Index to current monster frame.
	int nextframeindex;					// Used to force the next frameindex.
	float thinkinc;						// Time between thinks for this entity.
	char		*otherenemyname;				// ClassName of secondary enemy (other than player).
												// E.g. a Rat's secondary enemy is a gib.
	float misc_debounce_time;
	float		flee_finished;					// When a monster is done fleeing
	float		chase_finished;					// When the monster can look for secondary monsters.

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

/* this determines how long to wait after a duck to duck again.
   this needs to be longer than the time after the monster_duck_up
   in all of the animation sequences */
#define DUCK_INTERVAL 0.5

extern game_locals_t game;
extern level_locals_t level;
extern game_import_t gi;
extern game_export_t globals;
extern spawn_temp_t st;

extern int sm_meat_index;
extern int snd_fry;

extern int debristhisframe;
extern int gibsthisframe;

/* means of death */
#define MOD_UNKNOWN 0
#define MOD_BLASTER 1
#define MOD_SHOTGUN 2
#define MOD_SSHOTGUN 3
#define MOD_MACHINEGUN 4
#define MOD_CHAINGUN 5
#define MOD_GRENADE 6
#define MOD_G_SPLASH 7
#define MOD_ROCKET 8
#define MOD_R_SPLASH 9
#define MOD_HYPERBLASTER 10
#define MOD_RAILGUN 11
#define MOD_BFG_LASER 12
#define MOD_BFG_BLAST 13
#define MOD_BFG_EFFECT 14
#define MOD_HANDGRENADE 15
#define MOD_HG_SPLASH 16
#define MOD_WATER 17
#define MOD_SLIME 18
#define MOD_LAVA 19
#define MOD_CRUSH 20
#define MOD_TELEFRAG 21
#define MOD_FALLING 22
#define MOD_SUICIDE 23
#define MOD_HELD_GRENADE 24
#define MOD_EXPLOSIVE 25
#define MOD_BARREL 26
#define MOD_BOMB 27
#define MOD_EXIT 28
#define MOD_SPLASH 29
#define MOD_TARGET_LASER 30
#define MOD_TRIGGER_HURT 31
#define MOD_HIT 32
#define MOD_TARGET_BLASTER 33
#define MOD_RIPPER 34
#define MOD_PHALANX 35
#define MOD_BRAINTENTACLE 36
#define MOD_BLASTOFF 37
#define MOD_GEKK 38
#define MOD_TRAP 39
#define MOD_GRAPPLE 40
#define MOD_FRIENDLY_FIRE 0x8000000
#define MOD_CHAINFIST 41
#define MOD_DISINTEGRATOR 42
#define MOD_ETF_RIFLE 43
#define MOD_BLASTER2 44
#define MOD_HEATBEAM 45
#define MOD_TESLA 46
#define MOD_PROX 47
#define MOD_NUKE 48
#define MOD_VENGEANCE_SPHERE 49
#define MOD_HUNTER_SPHERE 50
#define MOD_DEFENDER_SPHERE 51
#define MOD_TRACKER 52
#define MOD_DBALL_CRUSH 53
#define MOD_DOPPLE_EXPLODE 54
#define MOD_DOPPLE_VENGEANCE 55
#define MOD_DOPPLE_HUNTER 56

/* Easier handling of AI skill levels */
#define SKILL_EASY 0
#define SKILL_MEDIUM 1
#define SKILL_HARD 2
#define SKILL_HARDPLUS 3

extern int meansOfDeath;
extern edict_t *g_edicts;

#define FOFS(x) (size_t)&(((edict_t *)NULL)->x)
#define STOFS(x) (size_t)&(((spawn_temp_t *)NULL)->x)
#define LLOFS(x) (size_t)&(((level_locals_t *)NULL)->x)
#define CLOFS(x) (size_t)&(((gclient_t *)NULL)->x)
#define	BYOFS(x) (size_t)&(((buoy_t *)NULL)->x)

#define random() ((randk() & 0x7fff) / ((float)0x7fff))
#define crandom() (2.0 * (random() - 0.5))

extern cvar_t *maxentities;
extern cvar_t *deathmatch;
extern cvar_t *coop;
extern cvar_t *coop_baseq2;	/* treat spawnflags according to baseq2 rules */
extern cvar_t *coop_pickup_weapons;
extern cvar_t *coop_elevator_delay;
extern cvar_t *coop_pickup_weapons;
extern cvar_t *dmflags;
extern cvar_t *skill;
extern cvar_t *fraglimit;
extern cvar_t *timelimit;
extern cvar_t *capturelimit;
extern cvar_t *instantweap;
extern cvar_t *password;
extern cvar_t *spectator_password;
extern cvar_t *needpass;
extern cvar_t *g_select_empty;
extern cvar_t *dedicated;
extern cvar_t *g_footsteps;
extern cvar_t *g_monsterfootsteps;
extern cvar_t *g_fix_triggered;
extern cvar_t *g_commanderbody_nogod;

extern cvar_t *filterban;

extern cvar_t *sv_gravity;
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
extern cvar_t *maxclients;
extern cvar_t *maxspectators;

extern cvar_t *flood_msgs;
extern cvar_t *flood_persecond;
extern cvar_t *flood_waitdelay;

extern cvar_t *sv_maplist;

extern cvar_t *sv_stopspeed;

extern cvar_t *g_showlogic;
extern cvar_t *gamerules;
extern cvar_t *huntercam;
extern cvar_t *strong_mines;
extern cvar_t *randomrespawn;

extern cvar_t *g_disruptor;

extern cvar_t *aimfix;
extern cvar_t *g_machinegun_norecoil;
extern cvar_t *g_quick_weap;
extern cvar_t *g_swap_speed;
extern cvar_t *g_language;
extern cvar_t *g_itemsbobeffect;
extern cvar_t *g_start_items;
extern cvar_t *ai_model_scale;
extern cvar_t *g_game;

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
extern cvar_t *advancedstaff;
extern cvar_t *sv_friction;
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
extern cvar_t *sv_cinematicfreeze;
extern cvar_t *sv_jumpcinematic;
extern cvar_t *sv_freezemonsters;
extern cvar_t *flood_killdelay;

extern int self_spawn;

/* this is for the count of monsters */
#define ENT_SLOTS_LEFT \
	(ent->monsterinfo.monster_slots - \
	 ent->monsterinfo.monster_used)
#define SELF_SLOTS_LEFT \
	(self->monsterinfo.monster_slots - \
	 self->monsterinfo.monster_used)

#define world (&g_edicts[0])

/* item spawnflags */
#define ITEM_TRIGGER_SPAWN 0x00000001
#define ITEM_NO_TOUCH 0x00000002
/* 6 bits reserved for editor flags */
/* 8 bits used as power cube id bits for coop games */
#define DROPPED_ITEM 0x00010000
#define DROPPED_PLAYER_ITEM 0x00020000
#define ITEM_TARGETS_USED 0x00040000

/* fields are needed for spawning from the entity
   string and saving / loading games */
#define FFL_SPAWNTEMP 1
#define FFL_NOSPAWN 2

typedef enum
{
	F_INT,
	F_FLOAT,
	F_LSTRING,          /* string on disk, pointer in memory, TAG_LEVEL */
	F_GSTRING,          /* string on disk, pointer in memory, TAG_GAME */
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT,            /* index on disk, pointer in memory */
	F_ITEM,             /* index on disk, pointer in memory */
	F_CLIENT,           /* index on disk, pointer in memory */
	F_FUNCTION,
	F_MMOVE,
	F_IGNORE,
	F_RGBA,
	F_LRAWSTRING,       /* raw string on disk, pointer in memory, TAG_LEVEL */
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
extern gitem_t itemlist[];

/* player/client.c */
void ClientBegin(edict_t *ent);
void ClientDisconnect(edict_t *ent);
void ClientUserinfoChanged(edict_t *ent, char *userinfo);
qboolean ClientConnect(edict_t *ent, char *userinfo);
void ClientThink(edict_t *ent, usercmd_t *cmd);

/* g_cmds.c */
qboolean CheckFlood(edict_t *ent);
void Cmd_Help_f(edict_t *ent);
void ClientCommand(edict_t *ent);
void Cmd_Score_f(edict_t *ent);

/* g_items.c */
void droptofloor(edict_t *ent);
void FixEntityPosition(edict_t *ent);
void PrecacheItem(gitem_t *it);
void InitItems(void);
void SetItemNames(void);
gitem_t *FindItem(const char *pickup_name);
gitem_t *FindItemByClassname(const char *classname);

#define ITEM_INDEX(x) ((x) - itemlist)

edict_t *Drop_Item(edict_t *ent, gitem_t *item);
void SetRespawn(edict_t *ent, float delay);
void ChangeWeapon(edict_t *ent);
void SpawnItem(edict_t *ent, gitem_t *item);
void Think_Weapon(edict_t *ent);
int ArmorIndex(edict_t *ent);
int PowerArmorType(edict_t *ent);
gitem_t *GetItemByIndex(int index);
qboolean Add_Ammo(edict_t *ent, gitem_t *item, int count);
void Touch_Item(edict_t *ent, edict_t *other, cplane_t *plane,
		csurface_t *surf);
void Use_Quad(edict_t *ent, gitem_t *item);
void Use_QuadFire(edict_t *ent, gitem_t *item);

/* g_utils.c */
qboolean KillBox(edict_t *ent);
void G_ProjectSource(const vec3_t point, const vec3_t distance, const vec3_t forward,
		const vec3_t right, vec3_t result);
edict_t *G_Find(edict_t *from, int fieldofs, const char *match);
edict_t *findradius(edict_t *from, vec3_t org, float rad);
edict_t *G_PickTarget(char *targetname);
void G_UseTargets(edict_t *ent, edict_t *activator);
void G_SetMovedir(vec3_t angles, vec3_t movedir);

void G_InitEdict(edict_t *e);
edict_t *G_SpawnOptional(void);
edict_t *G_Spawn(void);
void G_FreeEdict(edict_t *e);

void G_TouchTriggers(edict_t *ent);
void G_TouchSolids(edict_t *ent);

char *G_CopyString(char *in);

float *tv(float x, float y, float z);
char *vtos(vec3_t v);
void get_normal_vector(const cplane_t *p, vec3_t normal);

float vectoyaw(vec3_t vec);
void vectoangles(vec3_t vec, vec3_t angles);

void G_ProjectSource2(const vec3_t point, const vec3_t distance, const vec3_t forward,
		const vec3_t right, const vec3_t up, vec3_t result);
float vectoyaw2(vec3_t vec);
void vectoangles2(vec3_t vec, vec3_t angles);
edict_t *findradius2(edict_t *from, vec3_t org, float rad);

/* g_combat.c */
qboolean OnSameTeam(edict_t *ent1, edict_t *ent2);
qboolean CanDamage(edict_t *targ, edict_t *inflictor);
qboolean CheckTeamDamage(edict_t *targ, edict_t *attacker);
void T_Damage(edict_t *targ, edict_t *inflictor, edict_t *attacker,
		vec3_t dir, vec3_t point, vec3_t normal, int damage,
		int knockback, int dflags, int mod);
void T_RadiusDamage(edict_t *inflictor, edict_t *attacker,
		float damage, edict_t *ignore, float radius,
		int mod);
void T_RadiusNukeDamage(edict_t *inflictor, edict_t *attacker, float damage,
		edict_t *ignore, float radius, int mod);
void cleanupHealTarget(edict_t *ent);

/* damage flags */
#define DAMAGE_NORMAL 0x00000000            /* No modifiers to damage */
#define DAMAGE_RADIUS 0x00000001            /* damage was indirect */
#define DAMAGE_NO_ARMOR 0x00000002          /* armour does not protect from this damage */
#define DAMAGE_ENERGY 0x00000004            /* damage is from an energy based weapon */
#define DAMAGE_NO_KNOCKBACK 0x00000008      /* do not affect velocity, just view angles */
#define DAMAGE_BULLET 0x00000010            /* damage is from a bullet (used for ricochets) */
#define DAMAGE_NO_PROTECTION 0x00000020     /* armor, shields, invulnerability, and godmode have no effect */
#define DAMAGE_DESTROY_ARMOR 0x00000040     /* damage is done to armor and health. */
#define DAMAGE_NO_REG_ARMOR 0x00000080      /* damage skips regular armor */
#define DAMAGE_NO_POWER_ARMOR 0x00000100    /* damage skips power armor */
#define DAMAGE_ALL_KNOCKBACK 0x00000200     /* Ignore damage */
#define DAMAGE_EXTRA_KNOCKBACK 0x00000400   /* throw in some extra z */
#define DAMAGE_NO_BLOOD 0x00000800          /* don't spawn any blood */
#define DAMAGE_EXTRA_BLOOD 0x00001000       /* Lots of blood */
#define DAMAGE_SPELL 0x00002000             /* this came from a spell, - for use in calcing armor effects */
#define DAMAGE_DISMEMBER 0x00004000         /* Force this hit to use dismemberment message */
#define DAMAGE_ATTACKER_IMMUNE 0x00008000   /* Inflictor receives no effect */
#define DAMAGE_ATTACKER_KNOCKBACK 0x00010000 /* Inflictor takes knockback only */
#define DAMAGE_REDRAIN 0x00020000           /* Red rain acid damage */
#define DAMAGE_BUBBLE 0x00040000            /* Drowning damage */
#define DAMAGE_FIRE 0x00080000              /* Fire damage */
#define DAMAGE_ALIVE_ONLY 0x00100000        /* Only damage living things made of flesh */
#define DAMAGE_BLEEDING 0x00200000          /* No protection */
#define DAMAGE_AVOID_ARMOR 0x00400000       /* don't do the armor effect */
#define DAMAGE_DOUBLE_DISMEMBER 0x00800000  /* Force this hit to use dismemberment message with TWICE the chance of cutting */
#define DAMAGE_HURT_FRIENDLY 0x01000000     /* Always hurt friendly entities (e.g. fellow coop players). */
#define DAMAGE_POWERPHOENIX 0x02000000      /* Extra knockback to shooter, 1/4 damage. */
#define DAMAGE_FIRE_LINGER 0x04000000       /* Do extra fire linger damage. */
#define DAMAGE_ENEMY_MAX 0x08000000         /* Do maximum damage directly to the enemy in radius */
#define DAMAGE_ONFIRE 0x10000000            /* If the damage is FROM a fire... */
#define DAMAGE_PHOENIX 0x20000800           /* Phoenix-oriented damage.  Do minimal fire for show, but short duration. */

#define DAMAGE_SUFFOCATION			(DAMAGE_NO_KNOCKBACK|DAMAGE_NO_BLOOD|DAMAGE_BUBBLE|DAMAGE_AVOID_ARMOR)
#define DAMAGE_LAVA					(DAMAGE_NO_KNOCKBACK|DAMAGE_NO_BLOOD|DAMAGE_FIRE|DAMAGE_AVOID_ARMOR)
#define DAMAGE_SLIME				(DAMAGE_NO_KNOCKBACK|DAMAGE_NO_BLOOD|DAMAGE_AVOID_ARMOR)
#define DAMAGE_BURNING				(DAMAGE_ONFIRE|DAMAGE_NO_KNOCKBACK|DAMAGE_NO_BLOOD|DAMAGE_FIRE|DAMAGE_AVOID_ARMOR)

#define DEFAULT_BULLET_HSPREAD 300
#define DEFAULT_BULLET_VSPREAD 500
#define DEFAULT_SHOTGUN_HSPREAD 1000
#define DEFAULT_SHOTGUN_VSPREAD 500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT 12
#define DEFAULT_SHOTGUN_COUNT 12
#define DEFAULT_SSHOTGUN_COUNT 20

/* g_monster.c */
void monster_fire_bullet(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int kick, int hspread, int vspread, int flashtype);
void monster_fire_shotgun(edict_t *self, vec3_t start, vec3_t aimdir,
		int damage, int kick, int hspread, int vspread, int count,
		int flashtype);
void monster_fire_blaster(edict_t *self, vec3_t start, vec3_t dir,
		int damage, int speed, int flashtype, int effect);
void monster_fire_grenade(edict_t *self, vec3_t start, vec3_t aimdir,
		int damage, int speed, int flashtype);
void monster_fire_rocket(edict_t *self, vec3_t start, vec3_t dir,
		int damage, int speed, int flashtype);
void monster_fire_railgun(edict_t *self, vec3_t start, vec3_t aimdir,
		int damage, int kick, int flashtype);
void monster_fire_bfg(edict_t *self, vec3_t start, vec3_t aimdir,
		int damage, int speed, int kick, float damage_radius,
		int flashtype);
void monster_fire_ionripper(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, int flashtype, int effect);
void monster_fire_heat(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, int flashtype);
void monster_fire_heatbeam(edict_t *self, vec3_t start, vec3_t dir, vec3_t offset,
		int damage, int kick, int flashtype);
void monster_dabeam(edict_t *self);
void monster_fire_blueblaster(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, int flashtype, int effect);

void M_droptofloor(edict_t *ent);
void monster_think(edict_t *self);
void walkmonster_start(edict_t *self);
void swimmonster_start(edict_t *self);
void flymonster_start(edict_t *self);
void AttackFinished(edict_t *self, float time);
void monster_death_use(edict_t *self);
void M_CatagorizePosition(edict_t *ent);
qboolean M_CheckAttack(edict_t *self);
void M_FlyCheck(edict_t *self);
void M_CheckGround(edict_t *ent);
void M_FliesOff(edict_t *self);
void M_FliesOn(edict_t *self);
void M_SetEffects(edict_t *ent);

void monster_fire_blaster2(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, int flashtype, int effect);
void monster_fire_tracker(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, edict_t *enemy, int flashtype);
void stationarymonster_start(edict_t *self);
void monster_done_dodge(edict_t *self);

/* g_misc.c */
void ThrowHead(edict_t *self, const char *gibname, int damage, int type);
void ThrowClientHead(edict_t *self, int damage);
void ThrowGib(edict_t *self, const char *gibname, int damage, int type);
void BecomeExplosion1(edict_t *self);
void ThrowHeadACID(edict_t *self, const char *gibname, int damage, int type);
void ThrowGibACID(edict_t *self, const char *gibname, int damage, int type);
void barrel_delay (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

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
qboolean FindTarget(edict_t *self);
qboolean infront(edict_t *self, edict_t *other);
qboolean visible(edict_t *self, edict_t *other);
qboolean FacingIdeal(edict_t *self);
void HuntTarget(edict_t *self);
qboolean ai_checkattack(edict_t *self, float dist);

/* g_weapon.c */
void ThrowDebris(edict_t *self, char *modelname, float speed, vec3_t origin);
qboolean fire_hit(edict_t *self, vec3_t aim, int damage, int kick);
void fire_bullet(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int kick, int hspread, int vspread, int mod);
void fire_shotgun(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int kick, int hspread, int vspread, int count, int mod);
void fire_blaster(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int speed, int effect, qboolean hyper);
void fire_grenade(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int speed, float timer, float damage_radius);
void fire_grenade2(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int speed, float timer, float damage_radius, qboolean held);
void fire_rocket(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, float damage_radius, int radius_damage);
void fire_rail(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void fire_bfg(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, float damage_radius);
void fire_ionripper(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int speed, int effect);
void fire_heat(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed,
		float damage_radius, int radius_damage);
void fire_heatbeam(edict_t *self, vec3_t start, vec3_t aimdir, vec3_t offset,
		int damage, int kick, qboolean monster);
void fire_blueblaster(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int speed, int effect);
void fire_plasma(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed,
		float damage_radius, int radius_damage);
void fire_trap(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int speed, float timer, float damage_radius, qboolean held);

/* g_ptrail.c */
void PlayerTrail_Init(void);
void PlayerTrail_Add(vec3_t spot);
void PlayerTrail_New(vec3_t spot);
edict_t *PlayerTrail_PickFirst(edict_t *self);
edict_t *PlayerTrail_PickNext(edict_t *self);
edict_t *PlayerTrail_LastSpot(void);

/* g_client.c */
void respawn(edict_t *ent);
void BeginIntermission(edict_t *targ);
void PutClientInServer(edict_t *ent);
void InitClientPersistant(edict_t *ent);
void InitClientResp(gclient_t *client);
void InitBodyQue(void);
void ClientBeginServerFrame(edict_t *ent);
void ClientUserinfoChanged(edict_t *ent, char *userinfo);

/* g_player.c */
void player_pain(edict_t *self, edict_t *other, float kick, int damage);
void player_die(edict_t *self, edict_t *inflictor, edict_t *attacker,
		int damage, vec3_t point);
int SexedSoundIndex(edict_t *ent, char *base);
void player_dismember(edict_t *self, edict_t *other, int damage, int HitLocation);
void ResetPlayerBaseNodes(edict_t *ent);
void player_repair_skin(edict_t *self);

/* g_svcmds.c */
void ServerCommand(void);
qboolean SV_FilterPacket(char *from);

/* p_view.c */
void G_SetClientFrame(edict_t *ent, float speed);
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
void G_SetSpectatorStats(edict_t *ent);
void G_CheckChaseStats(edict_t *ent);
void ValidateSelectedItem(edict_t *ent);
void DeathmatchScoreboardMessage(edict_t *client, edict_t *killer);
void HelpComputerMessage(edict_t *client);
void InventoryMessage(edict_t *client);

/* g_pweapon.c */
void PlayerNoise(edict_t *who, vec3_t where, int type);
void P_ProjectSource(const edict_t *ent, const vec3_t distance,
		vec3_t forward, const vec3_t right, vec3_t result);
void Weapon_Generic(edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST,
		int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames,
		int *fire_frames, void (*fire)(edict_t *ent));
qboolean Pickup_Weapon(edict_t *ent, edict_t *other);
void Use_Weapon(edict_t *ent, gitem_t *inv);
void Use_Weapon2(edict_t *ent, gitem_t *inv);
void Drop_Weapon(edict_t *ent, gitem_t *inv);
void Weapon_Blaster(edict_t *ent);
void Weapon_Shotgun(edict_t *ent);
void Weapon_SuperShotgun(edict_t *ent);
void Weapon_Machinegun(edict_t *ent);
void Weapon_Chaingun(edict_t *ent);
void Weapon_HyperBlaster(edict_t *ent);
void Weapon_RocketLauncher(edict_t *ent);
void Weapon_Grenade(edict_t *ent);
void Weapon_GrenadeLauncher(edict_t *ent);
void Weapon_Railgun(edict_t *ent);
void Weapon_BFG(edict_t *ent);
void Weapon_ChainFist(edict_t *ent);
void Weapon_Disintegrator(edict_t *ent);
void Weapon_ETF_Rifle(edict_t *ent);
void Weapon_Heatbeam(edict_t *ent);
void Weapon_Prox(edict_t *ent);
void Weapon_Tesla(edict_t *ent);
void Weapon_ProxLauncher(edict_t *ent);
void Weapon_Ionripper(edict_t *ent);
void Weapon_Phalanx(edict_t *ent);
void Weapon_Trap(edict_t *ent);

/* m_move.c */
qboolean M_CheckBottom(edict_t *ent);
qboolean M_walkmove(edict_t *ent, float yaw, float dist);
void M_MoveToGoal(edict_t *ent, float dist);
float M_ChangeYaw(edict_t *ent);
void M_MoveAwayFromGoal(edict_t *ent, float dist);
void M_SetAnimGroupFrame(edict_t *self, const char *name);

/* g_phys.c */
void G_RunEntity(edict_t *ent);
void SV_AddGravity(edict_t *ent);

/* g_main.c */
void SaveClientData(void);
void EndDMLevel(void);

/* g_translate.c */
void LocalizationInit(void);
void LocalizationFree(void);
const char* LocalizationMessage(const char *message, int *sound_index);

/* g_chase.c */
void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
void GetChaseTarget(edict_t *ent);

/* savegame */
void ConstructEntities(void);
void G_ClearMessageQueues();
void InitGame(void);
void ReadLevel(const char *filename);
void WriteLevel(const char *filename);
void ReadGame(const char *filename);
void WriteGame(const char *filename, qboolean autosave);
void SpawnEntities(const char *mapname, char *entities, const char *spawnpoint);

void fire_flechette(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, int kick);
void fire_prox(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int speed);
void fire_nuke(edict_t *self, vec3_t start, vec3_t aimdir, int speed);
void fire_flame(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int speed);
void fire_burst(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int speed);
void fire_maintain(edict_t *, edict_t *, vec3_t start, vec3_t aimdir,
		int damage, int speed);
void fire_incendiary_grenade(edict_t *self, vec3_t start, vec3_t aimdir,
		int damage, int speed, float timer, float damage_radius);
void fire_player_melee(edict_t *self, vec3_t start, vec3_t aim, int reach,
		int damage, int kick, int quiet, int mod);
void fire_tesla(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int speed);
void fire_blaster2(edict_t *self, vec3_t start, vec3_t aimdir, int damage,
		int speed, int effect, qboolean hyper);
void fire_tracker(edict_t *self, vec3_t start, vec3_t dir, int damage,
		int speed, edict_t *enemy);

/* g_newai.c */
qboolean blind_rocket_ok(edict_t *self, vec3_t start, vec3_t right, vec3_t target, float ofs,
	vec3_t dir);
qboolean blocked_checkplat(edict_t *self, float dist);
qboolean blocked_checkjump(edict_t *self, float dist, float maxDown, float maxUp);
qboolean blocked_checknewenemy(edict_t *self);
qboolean monsterlost_checkhint(edict_t *self);
qboolean inback(edict_t *self, edict_t *other);
float realrange(edict_t *self, edict_t *other);
edict_t *SpawnBadArea(vec3_t mins, vec3_t maxs, float lifespan, edict_t *owner);
edict_t *CheckForBadArea(edict_t *ent);
qboolean MarkTeslaArea(edict_t *self, edict_t *tesla);
void InitHintPaths(void);
void PredictAim(edict_t *target, vec3_t start, float bolt_speed, qboolean eye_height,
		float offset, vec3_t aimdir, vec3_t aimpoint);
qboolean below(edict_t *self, edict_t *other);
void drawbbox(edict_t *self);
void M_MonsterDodge(edict_t *self, edict_t *attacker, float eta, trace_t *tr);
void monster_duck_down(edict_t *self);
void monster_duck_hold(edict_t *self);
void monster_duck_up(edict_t *self);
qboolean has_valid_enemy(edict_t *self);
void TargetTesla(edict_t *self, edict_t *tesla);
void hintpath_stop(edict_t *self);
edict_t *PickCoopTarget(edict_t *self);
int CountPlayers(void);
void monster_jump_start(edict_t *self);
qboolean monster_jump_finished(edict_t *self);
qboolean face_wall(edict_t *self);

/* g_sphere.c */
void Defender_Launch(edict_t *self);
void Vengeance_Launch(edict_t *self);
void Hunter_Launch(edict_t *self);

/* g_newdm.c */
void InitGameRules(void);
edict_t *DoRandomRespawn(edict_t *ent);
void PrecacheForRandomRespawn(void);
qboolean Tag_PickupToken(edict_t *ent, edict_t *other);
void Tag_DropToken(edict_t *ent, gitem_t *item);
void Tag_PlayerDeath(edict_t *targ, edict_t *inflictor, edict_t *attacker);
void fire_doppleganger(edict_t *ent, vec3_t start, vec3_t aimdir);

/* g_spawn.c */
void ED_CallSpawn(edict_t *ent);
void DynamicResetSpawnModels(edict_t *self);
char *ED_NewString(const char *string, qboolean raw);
void SpawnInit(void);
void SpawnFree(void);
void P_ToggleFlashlight(edict_t *ent, qboolean state);
edict_t *CreateFlyMonster(vec3_t origin, vec3_t angles, vec3_t mins,
		vec3_t maxs, char *classname);
edict_t *CreateGroundMonster(vec3_t origin, vec3_t angles, vec3_t mins,
		vec3_t maxs, char *classname, int height);
qboolean FindSpawnPoint(vec3_t startpoint, vec3_t mins, vec3_t maxs,
		vec3_t spawnpoint, float maxMoveUp);
qboolean CheckSpawnPoint(vec3_t origin, vec3_t mins, vec3_t maxs);
qboolean CheckGroundSpawnPoint(vec3_t origin, vec3_t entMins, vec3_t entMaxs,
		float height, float gravity);
void SpawnGrow_Spawn(vec3_t startpos, int size);
void Widowlegs_Spawn(vec3_t startpos, vec3_t angles);
void ThrowSmallStuff(edict_t *self, vec3_t point);
void ThrowWidowGibSized(edict_t *self, char *gibname, int damage, int type,
		vec3_t startpos, int hitsound, qboolean fade);
void spawngrow_think(edict_t *self);

/* p_client.c */
void RemoveAttackingPainDaemons(edict_t *self);

/* g_resourcemanagers.c */
void G_InitResourceManagers();

/* g_breakable.c */
void KillBrush(edict_t *targ,edict_t *inflictor,edict_t *attacker,int damage);

/* g_obj.c */
void ObjectInit(edict_t *self,int health,int mass, int materialtype,int solid);

/* ============================================================================ */

#include "ai.h"//JABot

/* client_t->anim_priority */
#define ANIM_BASIC 0            /* stand / run */
#define ANIM_WAVE 1
#define ANIM_JUMP 2
#define ANIM_PAIN 3
#define ANIM_ATTACK 4
#define ANIM_DEATH 5
#define ANIM_REVERSE 6

/* client data that stays across multiple level loads */
typedef struct
{
	char userinfo[MAX_INFO_STRING];
	char netname[16];
	int hand;

	qboolean connected;             /* a loadgame will leave valid entities that
	                                   just don't have a connection yet */

	/* values saved and restored from
	   edicts when changing levels */
	int health;
	int max_health;
	int savedFlags;

	int selected_item;
	int inventory[MAX_ITEMS];

	/* ammo capacities */
	int max_bullets;
	int max_shells;
	int max_rockets;
	int max_grenades;
	int max_cells;
	int max_slugs;
	int max_magslug;
	int max_trap;

	gitem_t *weapon;
	gitem_t *lastweapon;

	int power_cubes;            /* used for tracking the cubes in coop games */
	int score;                  /* for calculating total unit score in coop games */

	int game_helpchanged;
	int helpchanged;

	qboolean spectator;         /* client is a spectator */
	int chasetoggle;       /* Chasetoggle */

	int max_tesla;
	int max_prox;
	int max_mines;
	int max_flechettes;
	int max_rounds;

	// ********************************************************************************************
	// User info.
	// ********************************************************************************************

	char		sounddir[MAX_QPATH];
	int			autoweapon;

	// ********************************************************************************************
	// Values that are saved from and restored to 'edict_t's when changing levels.
	// ********************************************************************************************

	short		mission_num1;
	short		mission_num2;

	// Visible model attributes.

	int			weaponready;
	byte		armortype;			// Current armour Corvus is wearing.
	byte		bowtype;			// Current bow and what kind (when it is on Corvus' back too).
	byte		stafflevel;			// Current powerup level for the staff.
	byte		helltype;			// Current skin on the hellstaff.
	byte		handfxtype;			// Current spell effect Corvus has attached to his refpoints.
	float		armor_count; 		// Not used on client.
	short		skintype;			// Skin index that reflects plague stages and alternate skins
	unsigned int altparts;			// Missing hands, heads etc.

	// Inventory.

	int old_inventory[MAX_ITEMS];

	// Ammo capacities.

	int			max_offmana;
	int			max_defmana;
	int			max_redarrow;
	int			max_phoenarr;
	int			max_hellstaff;

	// Offenses and defenses.

	gitem_t *defence, *lastdefence,
				*newweapon;
} client_persistant_t;

// ************************************************************************************************
// playerinfo_t
// ------------
// This is the information needed by the player animation system on both the client and server.
// ************************************************************************************************

typedef struct playerinfo_s
{
	// ********************************************************************************************
	// Inputs only.
	// ********************************************************************************************

	// Client side function callbacks (approximating functionality of server function callbacks).

	void (*CL_Sound)(byte EventId,vec3_t origin,int channel,char *soundname,float fvol,int attenuation,float timeofs);
	trace_t (*CL_Trace)(vec3_t start,vec3_t mins,vec3_t maxs,vec3_t end,int brushmask,int flags);
	int (*CL_CreateEffect)(byte EventId,void *owner,unsigned short type,int flags,vec3_t position,char *format,...);
	void (*CL_RemoveEffects)(byte EventId,void *owner,int fx);

	// Server (game) function callbacks (approximating functionality of client-side function callbacks).

	void (*G_L_Sound)(edict_t *entity,int sound_num);
	void (*G_Sound)(byte EventId,float leveltime, edict_t *entity,int channel,int sound_num,float volume,float attenuation,float timeofs);
	trace_t (*G_Trace)(const vec3_t start, const vec3_t mins, const vec3_t maxs,
			const vec3_t end, const edict_t *passent, int contentmask);
	void (*G_CreateEffect)(byte EventId, edict_t *state, int type, int flags, vec3_t origin, char *format,...);
	void (*G_RemoveEffects)(byte Eventid, edict_t *state, int type);

	// Server (game) function callbacks that have no client side equivalent.

	int (*G_SoundIndex)(const char *name);
	void (*G_SoundRemove)(char *name);
	void (*G_UseTargets)(edict_t *ent,edict_t *activator);
	entity_state_t *(*G_GetEntityStatePtr)(edict_t *entity);
	int (*G_BranchLwrClimbing)(playerinfo_t *playerinfo);
	qboolean (*G_PlayerActionCheckRopeGrab)(playerinfo_t *playerinfo, float stomp_org);
	void (*G_PlayerClimbingMoveFunc)(playerinfo_t *playerinfo, float height, float var2, float var3);
	qboolean (*G_PlayerActionCheckPuzzleGrab)(playerinfo_t *playerinfo);
	void (*G_PlayerActionTakePuzzle)(playerinfo_t *playerinfo);
	qboolean (*G_PlayerActionUsePuzzle)(playerinfo_t *playerinfo);
	qboolean (*G_PlayerActionCheckPushPull_Ent)(edict_t *ent);
	void (*G_PlayerActionMoveItem)(playerinfo_t *playerinfo,float distance);
	qboolean (*G_PlayerActionCheckPushButton)(playerinfo_t *playerinfo);
	void (*G_PlayerActionPushButton)(playerinfo_t *playerinfo);
	qboolean (*G_PlayerActionCheckPushLever)(playerinfo_t *playerinfo);
	void (*G_PlayerActionPushLever)(playerinfo_t *playerinfo);
	qboolean (*G_HandleTeleport)(playerinfo_t *playerinfo);
	void (*G_PlayerActionShrineEffect)(playerinfo_t *playerinfo);
	void (*G_PlayerActionChickenBite)(playerinfo_t *playerinfo);
	void (*G_PlayerFallingDamage)(playerinfo_t *playerinfo,float delta);
	void (*G_PlayerSpellShieldAttack)(playerinfo_t *playerinfo);
	void (*G_PlayerSpellStopShieldAttack)(playerinfo_t *playerinfo);
	void (*G_PlayerVaultKick)(playerinfo_t *playerinfo);
	void (*G_PlayerActionCheckRopeMove)(playerinfo_t *playerinfo);
	void (*G_cprintf)(edict_t *ent, int printlevel, short stringid);
	void (*G_WeapNext)(edict_t *ent);
	void (*G_UseItem)(edict_t *ent);

	// Common client & server (game) function callbacks.

	int (*PointContents)(vec3_t point);
	void (*SetJointAngles)(playerinfo_t *playerinfo);
	void (*ResetJointAngles)(playerinfo_t *playerinfo);
	void (*PlayerActionSwordAttack)(playerinfo_t *playerinfo,int value);
	void (*PlayerActionSpellFireball)(playerinfo_t *playerinfo);
	void (*PlayerActionSpellBlast)(playerinfo_t *playerinfo);
	void (*PlayerActionSpellArray)(playerinfo_t *playerinfo,int value);
	void (*PlayerActionSpellSphereCreate)(playerinfo_t *playerinfo,qboolean *Charging);
	void (*PlayerActionSpellFirewall)(playerinfo_t *playerinfo);
	void (*PlayerActionSpellBigBall)(playerinfo_t *playerinfo);
	void (*PlayerActionRedRainBowAttack)(playerinfo_t *playerinfo);
	void (*PlayerActionPhoenixBowAttack)(playerinfo_t *playerinfo);
	void (*PlayerActionHellstaffAttack)(playerinfo_t *playerinfo);
	void (*PlayerActionSpellDefensive)(playerinfo_t *playerinfo);
	qboolean (*G_EntIsAButton)(edict_t *ent);
	int (*irand)(playerinfo_t *playerinfo,int mn,int mx);

	// Indicates whether this playerinfo_t is held on the client or server.

	qboolean			isclient;

	// This is client only and records the highest level time the anim has run... we use this to
	// prevent multiple sounds etc. Logic is basically if(!ishistory) playsound...
	qboolean			ishistory;

	// Pointer to the associated player's edict_t.

	edict_t				*self;

	// Game .dll variables.

	float				leveltime;
	float				quickturnEndTime;

	// Server variables.

	float				sv_gravity;
	float				sv_cinematicfreeze;		// Not used on client.
	float				sv_jumpcinematic;		// Jumping through cinematic. Not used on client.

	// From edict_t.

	float				ideal_yaw;
	void				*groundentity;

	// Pointer to entity_state_t of player's enemy edict.

	entity_state_t		*enemystate;

	// Spell / weapon aiming direction (from g_client_t).

	vec3_t				aimangles;

	// Deathmatch flags.

	int	dmflags;

	// ********************************************************************************************
	// Inputs & outputs.
	// ********************************************************************************************

	// Data that must be maintatined over the duration of a level.

	client_persistant_t	pers;

	// Last usercmd_t.

	usercmd_t			pcmd;

	// Status of controller buttons.

	int					buttons;
	int					oldbuttons;
	int					latched_buttons;
	int					remember_buttons;

	// Weapons & defenses.

	qboolean			autoaim;				// Set on client from a flag.
	int					switchtoweapon;
	int					weap_ammo_index;
	int					def_ammo_index;			// Not used on client.
	int					weaponcharge;
	float				defensive_debounce;		// Used on client? Defensive spell delay.
	byte				meteor_count;

	// Visible model attributes.
	byte				plaguelevel;			// Current plague level: 0=none, 2=max.

	// Shrine stuff. Used by the player to determine the time for the torch to be lit, reflection
	// to work and invisibilty to work (xxx_timer).
	float				light_timer;			// Not used on client.
	float				reflect_timer;			// FIXME not transmitted yet.
	float				ghost_timer;			// FIXME not transmitted yet.
	float				powerup_timer;
	float				lungs_timer;			// Not used on client.
	float				shield_timer;			// FIXME not transmitted yet.
	float				speed_timer;			// FIXME not transmitted yet.
	float				block_timer;			// FIXME not transmitted yet.

	float				cinematic_starttime;	// Not used on client. Time cinematic started.
	float				cin_shield_timer;		// What the shield timer was set at the beginning of the cinematic
	int					c_mode;					// Show cinematics is on

	// Movement & animation.
	int					flags;
	float				fwdvel, sidevel, upvel;
	float				turncmd;
	float				waterheight;
	vec3_t				LastWatersplashPos;		// Not used on client.
	vec3_t				oldvelocity;
	qboolean			chargingspell;
	float				quickturn_rate;

	// From edict_t.
	vec3_t				origin;
	vec3_t				angles;
	vec3_t				velocity;
	vec3_t				mins,maxs;
	void				*enemy;					// Not used on client.
	void				*target;				// Not used on client.
	void				*target_ent;			// Not used on client.
	void				*targetEnt;				// FIXME - always 0 on client, but checked by client.
	float				nextthink;				// Not used on client.
	float				viewheight;
	float				knockbacktime;			// FIXME Used on client, but not transmitted yet?  --Pat
	int					watertype;
	int					waterlevel;
	int					deadflag;
	int					movetype;
	int					edictflags;

	// From entity_state_t.
	int					frame,swapFrame;
	int					effects;
	int					renderfx;
	int					skinnum;
	int					clientnum;
	fmnodeinfo_t		fmnodeinfo[MAX_FM_MESH_NODES];

	// From pmove_state_t.

	int					pm_flags,pm_w_flags;

	// ********************************************************************************************
	// Outputs only.
	// ********************************************************************************************

	// From playerstate_t.

	vec3_t				offsetangles;

	qboolean			advancedstaff;

	// Torso angle twisting stuff which is derived entirely from various inputs to the animation
	// system.

	qboolean			headjointonly;
	vec3_t				targetjointangles;
	qboolean			showscores;				// Set layout stat.
	qboolean			showpuzzleinventory;	// Set layout stat.

	// ********************************************************************************************
	// Internal state info.
	// ********************************************************************************************

	int					seqcmd[20];
	panimmove_t			*uppermove,*lowermove;
	int					uppermove_index,lowermove_index;
	panimframe_t		*upperframeptr,*lowerframeptr;
	int					upperframe,lowerframe;
	qboolean			upperidle,loweridle;
	int					upperseq,lowerseq;
	float				idletime;
	vec3_t				grabloc;
	float				grabangle;

	short camera_vieworigin[3];
	short camera_viewangles[3];

} playerinfo_t;

typedef struct
{
	void (*Init)(void);
	void (*Shutdown)(void);

	void (*PlayerReleaseRope)(playerinfo_t *playerinfo);
	void (*KnockDownPlayer)(playerinfo_t *playerinfo);
	void (*PlayFly)(playerinfo_t *playerinfo, float dist);
	void (*PlaySlap)(playerinfo_t *playerinfo, float dist);
	void (*PlayScratch)(playerinfo_t *playerinfo, float dist);
	void (*PlaySigh)(playerinfo_t *playerinfo, float dist);
	void (*SpawnDustPuff)(playerinfo_t *playerinfo, float dist);
	void (*PlayerInterruptAction)(playerinfo_t *playerinfo);

	qboolean (*BranchCheckDismemberAction)(playerinfo_t *playerinfo, int weapon);

	void (*TurnOffPlayerEffects)(playerinfo_t *playerinfo);
	void (*AnimUpdateFrame)(playerinfo_t *playerinfo);
	void (*PlayerFallingDamage)(playerinfo_t *playerinfo);

	void (*PlayerBasicAnimReset)(playerinfo_t *playerinfo);
	void (*PlayerAnimReset)(playerinfo_t *playerinfo);
	void (*PlayerAnimSetLowerSeq)(playerinfo_t *playerinfo, int seq);
	void (*PlayerAnimSetUpperSeq)(playerinfo_t *playerinfo, int seq);
	void (*PlayerAnimUpperIdle)(playerinfo_t *playerinfo);
	void (*PlayerAnimLowerIdle)(playerinfo_t *playerinfo);
	void (*PlayerAnimUpperUpdate)(playerinfo_t *playerinfo);
	void (*PlayerAnimLowerUpdate)(playerinfo_t *playerinfo);
	void (*PlayerAnimSetVault)(playerinfo_t *playerinfo, int seq);
	void (*PlayerPlayPain)(playerinfo_t *playerinfo, int type);

	void (*PlayerIntLand)(playerinfo_t *playerinfo_t, float landspeed);

	void (*PlayerInit)(playerinfo_t *playerinfo, int complete_reset);
	void (*PlayerClearEffects)(playerinfo_t *playerinfo);
	void (*PlayerUpdate)(playerinfo_t *playerinfo);
	void (*PlayerUpdateCmdFlags)(playerinfo_t *playerinfo);
	void (*PlayerUpdateModelAttributes)(playerinfo_t *playerinfo);
} player_export_t;

typedef struct
{
	void (*dprintf)(const char *fmt, ...);
	gitem_t *(*FindItem)(const char *pickup_name);
	void (*Weapon_EquipSpell)(struct edict_s *ent, gitem_t *Weapon);
	void (*Weapon_Ready)(playerinfo_t *playerinfo, gitem_t *Weapon);
	int (*Weapon_CurrentShotsLeft)(playerinfo_t *playerinfo);
	int (*Defence_CurrentShotsLeft)(playerinfo_t *playerinfo, int intent);
} player_import_t;

extern player_import_t pi;

/* client data that stays across deathmatch respawns */
typedef struct
{
	client_persistant_t coop_respawn;   /* what to set client->pers to on a respawn */
	int enterframe;                 /* level.framenum the client entered the game */
	int score;                      /* frags, etc */
	int ctf_team;                   /* CTF team */
	int ctf_state;
	float ctf_lasthurtcarrier;
	float ctf_lastreturnedflag;
	float ctf_flagsince;
	float ctf_lastfraggedcarrier;
	qboolean id_state;
	float lastidtime;
	qboolean voted;    /* for elections */
	qboolean ready;
	qboolean admin;
	struct ghost_s *ghost; /* for ghost codes */
	vec3_t cmd_angles;              /* angles sent over in the last command */
	int game_helpchanged;
	int helpchanged;
	qboolean spectator;             /* client is a spectator */
} client_respawn_t;

/*
 * CTF menu
 */
typedef struct pmenuhnd_s
{
	struct pmenu_s *entries;
	int cur;
	int num;
	void *arg;
} pmenuhnd_t;

/* this structure is cleared on each
   PutClientInServer(), except for 'client->pers' */
struct gclient_s
{
	/* known to server */
	player_state_t ps;              /* communicated by server to clients */
	int ping;

	/* private to game */
	client_persistant_t pers;
	client_respawn_t resp;
	pmove_state_t old_pmove;        /* for detecting out-of-pmove changes */

	qboolean showscores;            /* set layout stat */
	qboolean inmenu;                /* in menu */
	pmenuhnd_t *menu;               /* current menu */
	qboolean showinventory;         /* set layout stat */
	qboolean showhelp;
	qboolean showhelpicon;

	int ammo_index;

	int buttons;
	int oldbuttons;
	int latched_buttons;

	qboolean weapon_thunk;

	gitem_t *newweapon;

	/* sum up damage over an entire frame, so
	   shotgun blasts give a single big kick */
	int damage_armor;               /* damage absorbed by armor */
	int damage_parmor;              /* damage absorbed by power armor */
	int damage_blood;               /* damage taken out of health */
	int damage_knockback;           /* impact damage */
	vec3_t damage_from;             /* origin for vector calculation */

	float killer_yaw;               /* when dead, look at killer */

	weaponstate_t weaponstate;
	vec3_t kick_angles;				/* weapon kicks */
	vec3_t kick_origin;
	float v_dmg_roll, v_dmg_pitch, v_dmg_time;          /* damage kicks */
	float fall_time, fall_value;    /* for view drop on fall */
	float damage_alpha;
	float bonus_alpha;
	vec3_t damage_blend;
	vec3_t v_angle;                 /* aiming direction */
	float bobtime;                  /* so off-ground doesn't change it */
	vec3_t oldviewangles;
	vec3_t oldvelocity;

	float next_drown_time;
	int old_waterlevel;
	int breather_sound;

	int machinegun_shots;           /* for weapon raising */

	/* animation vars */
	int anim_end;
	int anim_priority;
	qboolean anim_duck;
	qboolean anim_run;

	/* powerup timers */
	float quad_framenum;
	float invincible_framenum;
	float invisible_framenum;
	float breather_framenum;
	float enviro_framenum;

	qboolean grenade_blew_up;
	float grenade_time;
	float quadfire_framenum;
	qboolean trap_blew_up;
	float trap_time;

	int silencer_shots;
	int weapon_sound;

	float pickup_msg_time;

	float flood_locktill;           /* locked from talking */
	float flood_when[10];           /* when messages were said */
	int flood_whenhead;             /* head pointer for when said */

	float respawn_time;             /* can respawn when time > this */

	edict_t *chase_target;          /* player we are chasing */
	qboolean update_chase;          /* need to update chase info? */

	void *ctf_grapple;              /* entity of grapple */
	int ctf_grapplestate;               /* true if pulling */
	float ctf_grapplereleasetime;       /* time of grapple release */
	float ctf_regentime;            /* regen tech */
	float ctf_techsndtime;
	float ctf_lasttechmsg;
	float menutime;                 /* time to update menu */
	qboolean menudirty;

	float double_framenum;
	float ir_framenum;
	float nuke_framenum;
	float tracker_pain_framenum;

	edict_t *owned_sphere;          /* this points to the player's sphere */

	/* Third person view */
	int chasetoggle;
	edict_t *chasecam;
	edict_t *oldplayer;
	int use;
	int zoom;
	int delayedstart;

	int complete_reset;
	qboolean damage_gas;            /* Did damage come from plague mist? */
	// Damage stuff. Sum up damage over an entire frame.



	//

	usercmd_t			pcmd;
	short				oldcmdangles[3];
	vec3_t				aimangles;				// Spell / weapon aiming direction.

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

	// Shrine stuff.

	float				shrine_framenum;

	// Data for the player obituaries

	MOD_t				meansofdeath;

	// Anti flooding vars

	float				flood_nextnamechange;	// next time for valid nick change
	float				flood_nextkill;			// next time for suicide

	playerinfo_t		playerinfo;

};

#include "../common/message.h"
#include "g_classstatics.h"

#define MAX_BUOY_BRANCHES 3

struct edict_s
{
	entity_state_t s;
	struct gclient_s *client;       /* NULL if not a player the server expects the first part
	                                   of gclient_s to be a player_state_t but the rest of it is
									   opaque */

	qboolean inuse;
	int linkcount;

	link_t area;                    /* linked to a division node or leaf */

	int num_clusters;               /* if -1, use headnode instead */
	int clusternums[MAX_ENT_CLUSTERS];
	int headnode;                   /* unused if num_clusters != -1 */
	int areanum, areanum2;

	/* ================================ */

	int svflags;
	vec3_t mins, maxs;
	vec3_t absmin, absmax, size;
	solid_t solid;
	int clipmask;
	edict_t *owner;

	/* Additional state from ReRelease */
	entity_rrstate_t rrs;

	/* DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER */
	/* EXPECTS THE FIELDS IN THAT ORDER! */

	/* ================================ */

	int movetype;
	int flags;

	char *model;
	float freetime;                 /* sv.time when the object was freed */

	/* only used locally in game, not by server */
	char *message;
	char *classname;
	int spawnflags;

	float timestamp;

	float angle;					/* set in qe3, -1 = up, -2 = down */
	char *target;
	char *targetname;
	char *killtarget;
	char *team;
	char *pathtarget;
	char *deathtarget;
	char *combattarget;
	edict_t *target_ent;

	float speed, accel, decel;
	vec3_t movedir;
	vec3_t pos1, pos2;

	vec3_t velocity;
	vec3_t avelocity;
	int mass;
	float air_finished;
	float gravity;              /* per entity gravity multiplier (1.0 is normal) */
	                            /* use for lowgrav artifact, flares */

	edict_t *goalentity;
	edict_t *movetarget;
	float yaw_speed;
	float ideal_yaw;

	float nextthink;
	void (*prethink)(edict_t *ent);
	void (*think)(edict_t *self);
	void (*blocked)(edict_t *self, edict_t *other);         /* move to moveinfo? */
	void (*touch)(edict_t *self, edict_t *other, cplane_t *plane,
			csurface_t *surf);
	void (*use)(edict_t *self, edict_t *other, edict_t *activator);
	void (*pain)(edict_t *self, edict_t *other, float kick, int damage);
	void (*die)(edict_t *self, edict_t *inflictor, edict_t *attacker,
			int damage, vec3_t point);

	float touch_debounce_time;		/* now also used by fixbots for timeouts when getting stuck */
	float pain_debounce_time;
	float damage_debounce_time;
	float fly_sound_debounce_time;	/* now also used by insane marines to store pain sound timeout */
									/* and by fixbots for storing object_repair timeout when getting stuck */
	float last_move_time;

	int health;
	int max_health;
	int gib_health;
	int deadflag;

	float show_hostile;
	float powerarmor_time;

	char *map;                  /* target_changelevel */

	int viewheight;             /* height above origin where eyesight is determined */
	int takedamage;
	int dmg;
	int radius_dmg;
	float dmg_radius;
	int sounds;                 /* now also used for player death sound aggregation */
	int count;

	edict_t *chain;
	edict_t *enemy;
	edict_t *oldenemy;
	edict_t *activator;
	edict_t *groundentity;
	int groundentity_linkcount;
	edict_t *teamchain;
	edict_t *teammaster;

	edict_t *mynoise;           /* can go in client only */
	edict_t *mynoise2;

	int noise_index;
	int noise_index2;
	float volume;
	float attenuation;

	/* timing variables */
	float wait;
	float delay;                /* before firing targets */
	float random;

	float last_sound_time;

	int watertype;
	int waterlevel;

	vec3_t move_origin;
	vec3_t move_angles;

	/* move this to clientinfo? */
	int light_level;

	int style;                  /* also used as areaportal number */

	gitem_t *item;              /* for bonus items */

	/* common data blocks */
	moveinfo_t moveinfo;
	monsterinfo_t monsterinfo;

	int orders;

	int plat2flags;
	vec3_t offset;
	vec3_t gravityVector;
	edict_t *bad_area;
	edict_t *hint_chain;
	edict_t *monster_hint_chain;
	edict_t *target_hint_chain;
	int hint_chain_id;
	float lastMoveTime;

	/* Third person view */
	int chasedist1;
	int chasedist2;

	ai_handle_t *ai;       /* jabot */
	qboolean		is_swim;	//AI_CategorizePosition
	qboolean		is_step;
	qboolean		is_ladder;
	qboolean		was_swim;
	qboolean		was_step;

	float				ideal_pitch;	// Used by monsters and player.
	float				yawOffset;		// Used in CreateMove_Step

	char *text_msg;
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

	// Used to indicate teams, (a way to group things).

	// Fields used by only one class of game entity (monster, player, poly, trigger, etc).

		// Text printed to con for door, polys, triggers, etc.

	// These really all could be changed to ints or hashed or something (currently, we do a search
	// on all the active edicts using strcmps). We should be able to assign indexes in the BSP, by
	// doing the string strcmps at BSP time. The player seem to use any of the target stuff.

	char				*scripttarget;

	int bloodType;		// type of stuff to spawn off when hit

	void				(*TriggerActivated)(edict_t *self, edict_t *activator);
										// used by anything which can "see", player and monsters
	float				reflected_time;	// used by objects to tell if they've been repulsed by something..

	int					targeted;		// used by Ogle to denote a targeted action queued up
	int					lastbuoy;		// used to save a buoy in checking
										// polys.
	// Used by player only.

	edict_t				*last_buoyed_enemy;		// used by monsters.

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

	vec3_t				knockbackvel;

	// Used by animating entities.

	int					curAnimID;
	int					lastAnimID;

	// used by the Morph Ovum

	void				(*oldthink)(edict_t *self);

	// Used by monsters and player.

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
};

#define SPHERE_DEFENDER 0x0001
#define SPHERE_HUNTER 0x0002
#define SPHERE_VENGEANCE 0x0004
#define SPHERE_DOPPLEGANGER 0x0100

#define SPHERE_TYPE 0x00FF
#define SPHERE_FLAGS 0xFF00

/* deathmatch games */
#define     RDM_TAG 2
#define     RDM_DEATHBALL 3

typedef struct dm_game_rs
{
	void (*GameInit)(void);
	void (*PostInitSetup)(void);
	void (*ClientBegin)(edict_t *ent);
	void (*SelectSpawnPoint)(edict_t *ent, vec3_t origin, vec3_t angles);
	void (*PlayerDeath)(edict_t *targ, edict_t *inflictor, edict_t *attacker);
	void (*Score)(edict_t *attacker, edict_t *victim, int scoreChange);
	void (*PlayerEffects)(edict_t *ent);
	void (*DogTag)(edict_t *ent, edict_t *killer, char **pic);
	void (*PlayerDisconnect)(edict_t *ent);
	int (*ChangeDamage)(edict_t *targ, edict_t *attacker, int damage, int mod);
	int (*ChangeKnockback)(edict_t *targ, edict_t *attacker, int knockback, int mod);
	int (*CheckDMRules)(void);
} dm_game_rt;

extern dm_game_rt DMGame;

void Tag_GameInit(void);
void Tag_PostInitSetup(void);
void Tag_PlayerDeath(edict_t *targ, edict_t *inflictor, edict_t *attacker);
void Tag_Score(edict_t *attacker, edict_t *victim, int scoreChange);
void Tag_PlayerEffects(edict_t *ent);
void Tag_DogTag(edict_t *ent, edict_t *killer, char **pic);
void Tag_PlayerDisconnect(edict_t *ent);
int Tag_ChangeDamage(edict_t *targ, edict_t *attacker, int damage, int mod);

void DBall_GameInit(void);
void DBall_ClientBegin(edict_t *ent);
void DBall_SelectSpawnPoint(edict_t *ent, vec3_t origin, vec3_t angles);
int DBall_ChangeKnockback(edict_t *targ, edict_t *attacker, int knockback, int mod);
int DBall_ChangeDamage(edict_t *targ, edict_t *attacker, int damage, int mod);
void DBall_PostInitSetup(void);
int DBall_CheckDMRules(void);

/*
 * CTF Menu
 */
enum
{
	PMENU_ALIGN_LEFT,
	PMENU_ALIGN_CENTER,
	PMENU_ALIGN_RIGHT
};

typedef void (*SelectFunc_t)(edict_t *ent, pmenuhnd_t *hnd);

typedef struct pmenu_s
{
	char *text;
	int align;
	SelectFunc_t SelectFunc;
} pmenu_t;

pmenuhnd_t *PMenu_Open(edict_t *ent,
		pmenu_t *entries,
		int cur,
		int num,
		void *arg);
void PMenu_Close(edict_t *ent);
void PMenu_UpdateEntry(pmenu_t *entry,
		const char *text,
		int align,
		SelectFunc_t SelectFunc);
void PMenu_Do_Update(edict_t *ent);
void PMenu_Update(edict_t *ent);
void PMenu_Next(edict_t *ent);
void PMenu_Prev(edict_t *ent);
void PMenu_Select(edict_t *ent);

/*
 * CTF specific stuff.
 */

#define CTF_VERSION 1.52
#define CTF_VSTRING2(x) # x
#define CTF_VSTRING(x) CTF_VSTRING2(x)
#define CTF_STRING_VERSION CTF_VSTRING(CTF_VERSION)

#define STAT_CTF_TEAM1_PIC 18
#define STAT_CTF_TEAM1_CAPS 19
#define STAT_CTF_TEAM2_PIC 20
#define STAT_CTF_TEAM2_CAPS 21
#define STAT_CTF_FLAG_PIC 22
#define STAT_CTF_JOINED_TEAM1_PIC 23
#define STAT_CTF_JOINED_TEAM2_PIC 24
#define STAT_CTF_TEAM1_HEADER 25
#define STAT_CTF_TEAM2_HEADER 26
#define STAT_CTF_TECH 27
#define STAT_CTF_ID_VIEW 28
#define STAT_CTF_MATCH 29
#define STAT_CTF_ID_VIEW_COLOR 30
#define STAT_CTF_TEAMINFO 31

#define CONFIG_CTF_MATCH (CS_AIRACCEL - 1)
#define CONFIG_CTF_TEAMINFO (CS_AIRACCEL - 2)

typedef enum
{
	CTF_NOTEAM,
	CTF_TEAM1,
	CTF_TEAM2
} ctfteam_t;

typedef enum
{
	CTF_GRAPPLE_STATE_FLY,
	CTF_GRAPPLE_STATE_PULL,
	CTF_GRAPPLE_STATE_HANG
} ctfgrapplestate_t;

typedef struct ghost_s
{
	char netname[16];
	int number;

	/* stats */
	int deaths;
	int kills;
	int caps;
	int basedef;
	int carrierdef;

	int code; /* ghost code */
	int team; /* team */
	int score; /* frags at time of disconnect */
	edict_t *ent;
} ghost_t;

extern cvar_t *ctf;
extern char *ctf_statusbar;

#define CTF_TEAM1_SKIN "ctf_r"
#define CTF_TEAM2_SKIN "ctf_b"

#define DF_CTF_FORCEJOIN 131072
#define DF_ARMOR_PROTECT 262144
#define DF_CTF_NO_TECH 524288

#define CTF_CAPTURE_BONUS 15        /* what you get for capture */
#define CTF_TEAM_BONUS 10           /* what your team gets for capture */
#define CTF_RECOVERY_BONUS 1        /* what you get for recovery */
#define CTF_FLAG_BONUS 0            /* what you get for picking up enemy flag */
#define CTF_FRAG_CARRIER_BONUS 2    /* what you get for fragging enemy flag carrier */
#define CTF_FLAG_RETURN_TIME 40     /* seconds until auto return */

#define CTF_CARRIER_DANGER_PROTECT_BONUS 2      /* bonus for fraggin someone who has recently hurt your flag carrier */
#define CTF_CARRIER_PROTECT_BONUS 1             /* bonus for fraggin someone while either you or your target are near your flag carrier */
#define CTF_FLAG_DEFENSE_BONUS 1                /* bonus for fraggin someone while either you or your target are near your flag */
#define CTF_RETURN_FLAG_ASSIST_BONUS 1          /* awarded for returning a flag that causes a capture to happen almost immediately */
#define CTF_FRAG_CARRIER_ASSIST_BONUS 2         /* award for fragging a flag carrier if a capture happens almost immediately */

#define CTF_TARGET_PROTECT_RADIUS 400           /* the radius around an object being defended where a target will be worth extra frags */
#define CTF_ATTACKER_PROTECT_RADIUS 400         /* the radius around an object being defended where an attacker will get extra frags when making kills */

#define CTF_CARRIER_DANGER_PROTECT_TIMEOUT 8
#define CTF_FRAG_CARRIER_ASSIST_TIMEOUT 10
#define CTF_RETURN_FLAG_ASSIST_TIMEOUT 10

#define CTF_AUTO_FLAG_RETURN_TIMEOUT 30         /* number of seconds before dropped flag auto-returns */

#define CTF_TECH_TIMEOUT 60                     /* seconds before techs spawn again */

#define CTF_GRAPPLE_SPEED 650                   /* speed of grapple in flight */
#define CTF_GRAPPLE_PULL_SPEED 650              /* speed player is pulled at */

void CTFInit(void);
void CTFSpawn(void);
void CTFPrecache(void);

void SP_info_player_team1(edict_t *self);
void SP_info_player_team2(edict_t *self);

char *CTFTeamName(int team);
char *CTFOtherTeamName(int team);
void CTFAssignSkin(edict_t *ent, char *s);
void CTFAssignTeam(gclient_t *who);
edict_t *SelectCTFSpawnPoint(edict_t *ent);
qboolean CTFPickup_Flag(edict_t *ent, edict_t *other);
void CTFDrop_Flag(edict_t *ent, gitem_t *item);
void CTFEffects(edict_t *player);
void CTFCalcScores(void);
void SetCTFStats(edict_t *ent);
void CTFDeadDropFlag(edict_t *self);
void CTFScoreboardMessage(edict_t *ent, edict_t *killer);
void CTFTeam_f(edict_t *ent);
void CTFID_f(edict_t *ent);
void CTFSay_Team(edict_t *who, char *msg);
void CTFFlagSetup(edict_t *ent);
void CTFResetFlag(int ctf_team);
void CTFFragBonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker);
void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker);

/* GRAPPLE */
void CTFWeapon_Grapple(edict_t *ent);
void CTFPlayerResetGrapple(edict_t *ent);
void CTFGrapplePull(edict_t *self);
void CTFResetGrapple(edict_t *self);

/* TECH */
gitem_t *CTFWhat_Tech(edict_t *ent);
qboolean CTFPickup_Tech(edict_t *ent, edict_t *other);
void CTFDrop_Tech(edict_t *ent, gitem_t *item);
void CTFDeadDropTech(edict_t *ent);
void CTFSetupTechSpawn(void);
int CTFApplyResistance(edict_t *ent, int dmg);
int CTFApplyStrength(edict_t *ent, int dmg);
qboolean CTFApplyStrengthSound(edict_t *ent);
qboolean CTFApplyHaste(edict_t *ent);
void CTFApplyHasteSound(edict_t *ent);
void CTFApplyRegeneration(edict_t *ent);
qboolean CTFHasRegeneration(edict_t *ent);
void CTFRespawnTech(edict_t *ent);
void CTFResetTech(void);

void CTFOpenJoinMenu(edict_t *ent);
qboolean CTFStartClient(edict_t *ent);
void CTFVoteYes(edict_t *ent);
void CTFVoteNo(edict_t *ent);
void CTFReady(edict_t *ent);
void CTFNotReady(edict_t *ent);
qboolean CTFNextMap(void);
qboolean CTFMatchSetup(void);
qboolean CTFMatchOn(void);
void CTFGhost(edict_t *ent);
void CTFAdmin(edict_t *ent);
qboolean CTFInMatch(void);
void CTFStats(edict_t *ent);
void CTFWarp(edict_t *ent);
void CTFBoot(edict_t *ent);
void CTFPlayerList(edict_t *ent);

qboolean CTFCheckRules(void);

void SP_misc_ctf_banner(edict_t *ent);
void SP_misc_ctf_small_banner(edict_t *ent);

void Cmd_Chasecam_Toggle(edict_t *ent);
void ChasecamStart(edict_t *ent);
void ChasecamRemove(edict_t *ent);
void CheckChasecam_Viewent(edict_t *ent);
void ChasecamTrack(edict_t *ent);

void CTFObserver(edict_t *ent);

void SP_trigger_teleport(edict_t *ent);
void SP_info_teleport_destination(edict_t *ent);

void CTFSetPowerUpEffect(edict_t *ent, int def);

qboolean Pickup_Adrenaline(edict_t * ent, edict_t * other);
qboolean Pickup_Ammo(edict_t * ent , edict_t * other);
qboolean Pickup_AncientHead(edict_t * ent, edict_t * other);
qboolean Pickup_Armor(edict_t * ent, edict_t * other);
qboolean Pickup_Bandolier(edict_t * ent, edict_t * other);
qboolean Pickup_Doppleganger(edict_t * ent, edict_t * other);
qboolean Pickup_Health(edict_t * ent, edict_t * other);
qboolean Pickup_Key(edict_t * ent, edict_t * other);
qboolean Pickup_Nuke(edict_t * ent, edict_t * other);
qboolean Pickup_Pack(edict_t * ent, edict_t * other);
qboolean Pickup_PowerArmor(edict_t * ent, edict_t * other);
qboolean Pickup_Powerup(edict_t * ent, edict_t * other);
qboolean Pickup_Sphere(edict_t * ent, edict_t * other);

void CopyToBodyQue(edict_t *ent);
void Use_Plat(edict_t *ent, edict_t *other, edict_t *activator);
void SelectSpawnPoint(edict_t *ent, vec3_t origin, vec3_t angles);

/* platforms states */
#define STATE_TOP 0
#define STATE_BOTTOM 1
#define STATE_UP 2
#define STATE_DOWN 3

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

/* Custom heretic 2 IT_* flags */
#define IT_PUZZLE 0x00001000
#define IT_DEFENSE 0x00002000
#define IT_OFFENSE 0x00004000

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


void SpawnItemEffect(edict_t *ent, gitem_t *item);

edict_t *newfindradius(edict_t *from, vec3_t org, float rad);
edict_t *findinblocking(edict_t *from, edict_t *checkent);
edict_t *findinbounds(edict_t *from, vec3_t min, vec3_t max);
edict_t *oldfindinbounds(edict_t *from, vec3_t min, vec3_t max);
edict_t *finddistance(edict_t *from, vec3_t org, float mindist, float maxdist);
edict_t *findonpath(edict_t *startent, vec3_t startpos, vec3_t endpos, vec3_t mins, vec3_t maxs, vec3_t *resultpos);

//commonly used functions
int range(edict_t *self, edict_t *other);
qboolean clear_visible(edict_t *self, edict_t *other);
qboolean visible(edict_t *self, edict_t *other);
qboolean visible_pos(edict_t *self, vec3_t spot2);
qboolean infront(edict_t *self, edict_t *other);
qboolean infront_pos(edict_t *self, vec3_t pos);
qboolean ahead(edict_t *self, edict_t *other);

void G_SetToFree(edict_t *);
void G_LinkMissile(edict_t *ent);

qboolean CanDamageFromLoc(edict_t *targ, edict_t *inflictor, vec3_t origin);
void T_DamageRadius(edict_t *inflictor, edict_t *attacker,
		edict_t *ignore, float radius,
		float maxdamage, float mindamage, int dflags,int MeansOfDeath);
void T_DamageRadiusFromLoc(vec3_t origin, edict_t *inflictor, edict_t *attacker, edict_t *ignore, float radius,
							float maxdamage, float mindamage, int dflags,int MeansOfDeath);
void PauseTime(edict_t *self, float time);

qboolean FindTarget(edict_t *self);
void MG_PostDeathThink(edict_t *self);
qboolean movable (edict_t *ent);
qboolean EntReflecting(edict_t *ent, qboolean checkmonster, qboolean checkplayer);
void SkyFly (edict_t *self);
void Use_Multi(edict_t *self, edict_t *other, edict_t *activator);
void c_swapplayer(edict_t *Self,edict_t *Cinematic);
void remove_non_cinematic_entites(edict_t *owner);
void reinstate_non_cinematic_entites(edict_t *owner);

void ProcessScripts(void);
void ShutdownScripts(qboolean Complete);

extern player_export_t *playerExport;	// interface to player library.
extern player_import_t playerImport;	// interface to player library.

#define AI_EATING				0x00002000
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

// NOTE: 1 means that the last entity was a wall...
#define WALL_ENTITY (struct edict_s *)1
#define AVG_VEC3T(scale) (((scale)[0] + (scale)[1] + (scale)[2]) / 3)

trace_t MG_WalkMove(edict_t *self, float yaw, float dist, qboolean *trace_succeeded);
trace_t MG_MoveStep(edict_t *self, vec3_t move, qboolean relink, qboolean *trace_succeeded);
unsigned GenNoDrawInfo(fmnodeinfo_t *fmnodeinfo);
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

int Defence_CurrentShotsLeft(playerinfo_t *playerinfo, int intent);
int Weapon_CurrentShotsLeft(playerinfo_t *playerinfo);
void Weapon_Ready(playerinfo_t *playerinfo, gitem_t *Weapon);
void Weapon_EquipSpell(struct edict_s *ent, gitem_t *Weapon);
void Weapon_EquipSwordStaff(struct edict_s *ent, gitem_t *Weapon);
void Weapon_EquipHellStaff(struct edict_s *ent, gitem_t *Weapon);
void Weapon_EquipBow(struct edict_s *ent, gitem_t *Weapon);
qboolean IsDecalApplicable(edict_t *owner, edict_t *target, vec3_t origin, csurface_t *surface,cplane_t *plane, vec3_t planeDir);

void Use_Defence(struct edict_s *ent, gitem_t *defence);
void DefenceThink_Powerup(edict_t *Caster, char *Format, ...);
void DefenceThink_RingOfRepulsion(edict_t *Caster, char *Format, ...);
void DefenceThink_MeteorBarrier(edict_t *Caster, char *Format, ...);
void DefenceThink_Teleport(edict_t *Caster, char *Format, ...);
void DefenceThink_Morph(edict_t *Caster, char *Format, ...);
void DefenceThink_Shield(edict_t *Caster, char *Format, ...);
void DefenceThink_Tornado(edict_t *Caster, char *Format, ...);

#define	SVF_INUSE				0x00000008	// Used to replace the inuse field.
#define SVF_ALWAYS_SEND			0x00000010	// Always send the ent to all the clients, regardless of
											// of PVS or view culling
#define SVF_NO_AUTOTARGET		0x00000020	// This entity will not be chosen by FindNearestVisibleActorInFrustum
#define SVF_REFLECT				0x00000040	// Reflect shots
#define SVF_TAKE_NO_IMPACT_DMG	0x00000080	// Don't apply impact damage to this entity
#define SVF_BOSS				0x00000100	// Immunity to a number of things
#define SVF_TOUCHED_BEAST		0x00000200	// Used for beast faked physics hack
#define SVF_DO_NO_IMPACT_DMG	0x00000400	// This entity Doesn't do impact damage to others
#define SVF_NO_PLAYER_DAMAGE	0x00000800	// This entity Doesn't take damage from players
#define SVF_PARTS_GIBBED		0x00001000	// Used to delay gibbing so monsters can throw body parts
#define SVF_WAIT_NOTSOLID		0x00002000	// Hacky flag to postpone dead monsters from turning notsolid
#define SVF_ONFIRE				0x00004000	// He likes it Hot! Hot! Hot!
#define SVF_SHOW_START_BUOY		0x00008000	// just puts an effect on a buoy for showbuoy debug mode
#define SVF_SHOW_END_BUOY		0x00010000	// just puts an effect on a buoy for showbuoy debug mode
#define SVF_FLOAT				0x00020000	// Allows walkmonsters to walk off ledges, assumes a low gravity
#define SVF_ALLOW_AUTO_TARGET	0x00040000	// Used to allow player to autotarget non-monsters
#define SVF_ALERT_NO_SHADE		0x00080000	// only used by alert_entity to make monsters check the alert as a sound alert

#define Clamp(v, v_min, v_max) Q_min(Q_max((v), (v_min)), (v_max));

#endif /* GAME_LOCAL_H */
