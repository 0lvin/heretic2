//
// Heretic II
// Copyright 1998 Raven Software
//

typedef enum AnimID_e
{
	SERAPH_GUARD_ANIM_STAND,
	SERAPH_GUARD_ANIM_RUN,
	SERAPH_GUARD_ANIM_FJUMP,
	SERAPH_GUARD_ANIM_RUN_MELEE,
	SERAPH_GUARD_ANIM_WALK,
	SERAPH_GUARD_ANIM_PAIN,
	SERAPH_GUARD_ANIM_MELEE1,
	SERAPH_GUARD_ANIM_MELEE2,
	SERAPH_GUARD_ANIM_MELEE3,
	SERAPH_GUARD_ANIM_DEATH1,
	SERAPH_GUARD_ANIM_DEATH2_GO,
	SERAPH_GUARD_ANIM_DEATH2_LOOP,
	SERAPH_GUARD_ANIM_DEATH2_END,
	SERAPH_GUARD_ANIM_BACKUP,
	SERAPH_GUARD_ANIM_MISSILE,
	SERAPH_GUARD_ANIM_DELAY,
	SERAPH_GUARD_NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_ATTACK,
	SND_ATTACK_MISS,
	SND_PAIN1,
	SND_PAIN2,
	SND_PAIN3,
	SND_PAIN4,
	SND_DEATH1,
	SND_DEATH2,
	SND_DEATH3,
	SND_DEATH4,
	SND_HIT_WALL,
	SND_MISSILE,
	SND_MISSHIT,
	SND_FIST_HIT_WALL,
	SND_SIGHT1,
	SND_SIGHT2,
	SND_SIGHT3,
	SND_SIGHT4,
	NUM_SOUNDS
} SoundID_t;

extern mmove_t	seraph_guard_move_stand,
					seraph_guard_move_run,
					seraph_guard_move_fjump,
					seraph_guard_move_runmelee,
					seraph_guard_move_walk,
					seraph_guard_move_pain,
					seraph_guard_move_melee,
					seraph_guard_move_melee2,
					seraph_guard_move_melee3,
					seraph_guard_move_death1,
					seraph_guard_move_death2_go,
					seraph_guard_move_death2_loop,
					seraph_guard_move_death2_end,
					seraph_guard_move_backup,
					seraph_guard_move_missile,
					seraph_guard_move_delay;

void SP_monster_seraph_guard( edict_t* self );

void seraph_guard_death_loop ( edict_t *self );
void seraph_guard_check_land ( edict_t *self );
void seraph_guard_dead ( edict_t *self );
void seraph_guard_checkpoke ( edict_t *self );

void seraph_guard_pause( edict_t *self );
void seraph_guard_strike( edict_t *self, float damage, float forward, float up );
void seraphGuardApplyJump (edict_t *self);
void seraph_guard_back (edict_t *self, float dist);
void seraph_guard_fire (edict_t *self);

qboolean MG_BoolWalkMove (edict_t *self, float yaw, float dist);


#define BIT_BASEBIN		0
#define BIT_PITHEAD		1
#define BIT_SHOULDPAD	2
#define BIT_GUARDHEAD	4
#define BIT_LHANDGRD	8
#define BIT_LHANDBOSS	16
#define BIT_RHAND		32
#define BIT_FRTORSO		64
#define BIT_ARMSPIKES	128
#define BIT_LFTUPARM	256
#define BIT_RTLEG		512
#define BIT_RTARM		1024
#define BIT_LFTLEG		2048
#define BIT_BKTORSO		4096
#define BIT_AXE			8192
#define BIT_WHIP		16384
