//
// Heretic II
// Copyright 1998 Raven Software
//

typedef enum AnimID_e
{
	SERAPH_ANIM_WALK1,
	SERAPH_ANIM_WALK2,
	SERAPH_ANIM_ATTACK1,
	SERAPH_ANIM_ATTACK1_LOOP,
	SERAPH_ANIM_ATTACK1_END,
	SERAPH_ANIM_STAND1,
	SERAPH_ANIM_STAND1_TR,
	SERAPH_ANIM_STAND1_R,
	SERAPH_ANIM_STAND1_TRC,
	SERAPH_ANIM_STAND1_TL,
	SERAPH_ANIM_STAND1_L,
	SERAPH_ANIM_STAND1_TLC,
	SERAPH_ANIM_POINT1,
	SERAPH_ANIM_RUN1,
	SERAPH_ANIM_FJUMP,
	SERAPH_ANIM_RUN1_WHIP,
	SERAPH_ANIM_PAIN,
	SERAPH_ANIM_SWIPE,
	SERAPH_ANIM_GET2WORK,
	SERAPH_ANIM_GET2WORK2,
	SERAPH_ANIM_STARTLE,
	SERAPH_ANIM_READY2IDLE,
	SERAPH_ANIM_BACKUP,
	SERAPH_ANIM_DEATH1,
	SERAPH_ANIM_DEATH2_GO,
	SERAPH_ANIM_DEATH2_LOOP,
	SERAPH_ANIM_DEATH2_END,
	SERAPH_ANIM_BACKUP2,

	SERAPH_NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_ATTACK,
	SND_SCOLD1,
	SND_SCOLD2,
	SND_SCOLD3,
	SND_STARTLE,
	SND_SLAP,
	SND_DEATH1,
	SND_DEATH2,
	SND_DEATH3,
	SND_DEATH4,
	SND_PAIN1,
	SND_PAIN2,
	SND_PAIN3,
	SND_PAIN4,
	SND_SCARE,
	SND_SIGHT1,
	SND_SIGHT2,
	SND_SIGHT3,
	NUM_SOUNDS
} SoundID_t;

extern mmove_t	seraph_move_walk1,
					seraph_move_walk2,
					seraph_move_whip1,
					seraph_move_whip1_loop,
					seraph_move_whip1_end,
					seraph_move_stand1,
					seraph_move_stand1_tr,
					seraph_move_stand1_r,
					seraph_move_stand1_trc,
					seraph_move_stand1_tl,
					seraph_move_stand1_l,
					seraph_move_stand1_tlc,
					seraph_move_point1,
					seraph_move_run1,
					seraph_move_fjump,
					seraph_move_run1_whip,
					seraph_move_pain,
					seraph_move_swipe,
					seraph_move_get2work,
					seraph_move_get2work2,
					seraph_move_startle,
					seraph_move_ready2idle,
					seraph_move_backup,
					seraph_move_death1,
					seraph_move_death2_go,
					seraph_move_death2_loop,
					seraph_move_death2_end,
					seraph_move_backup2;

void SP_monster_seraph_overlord(edict_t* self);

void seraph_done_startle(edict_t *self);
void seraph_done_get2work(edict_t *self);
void seraph_enforce_ogle(edict_t *self);
void seraph_ai_walk(edict_t *self, float dist);
void seraph_idle(edict_t *self);
void seraph_pause(edict_t *self);
void seraph_enforce(edict_t *self);
void seraph_strike(edict_t *self, float damage, float a, float b);

void seraph_dead ( edict_t *self );

void seraph_death_loop ( edict_t *self );
void seraph_check_land ( edict_t *self );

void seraph_sound_startle(edict_t *self);
void seraph_sound_slap(edict_t *self);
void seraph_sound_scold(edict_t *self);
void seraph_sound_yell(edict_t *self);
void seraph_sound_whip(edict_t *self);
void seraphApplyJump (edict_t *self);

void seraph_back (edict_t *self, float dist);
void seraph_sound_scold2(edict_t *self);
void seraph2idle (edict_t *self);

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
