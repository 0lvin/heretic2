//
// Heretic II
// Copyright 1998 Raven Software
//
typedef enum AnimID_e
{
	ANIM_BITE,
	ANIM_MELEE,
	ANIM_RUN1,
	ANIM_RUN2,
	ANIM_RUN3,
	ANIM_WALK1,
	ANIM_WALK2,
	ANIM_WALK3,
	ANIM_STAND1,
	ANIM_PAIN1,
	ANIM_DEATH1,
	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_DIE,
	SND_GIB,
	SND_BITEHIT1,
	SND_BITEHIT2,
	SND_BITEMISS1,
	SND_BITEMISS2,
	SND_GROWL1,
	SND_GROWL2,
	SND_GROWL3,
	SND_SPLASH,
	SND_SLOW_SWIM1,
	SND_SLOW_SWIM2,
	SND_FAST_SWIM1,
	SND_FAST_SWIM2,
	NUM_SOUNDS
} SoundID_t;


extern mmove_t fish_move_bite;
extern mmove_t fish_move_melee;
extern mmove_t fish_move_run1;
extern mmove_t fish_move_run2;
extern mmove_t fish_move_run3;
extern mmove_t fish_move_walk1;
extern mmove_t fish_move_walk2;
extern mmove_t fish_move_walk3;
extern mmove_t fish_move_stand1;
extern mmove_t fish_move_pain1;
extern mmove_t fish_move_death;

void fish_pain(edict_t *self, G_Message_t *msg);
void fish_death(edict_t *self, G_Message_t *msg);

void fish_dead(edict_t *self);
void fishbite (edict_t *self);
void fish_idle(edict_t *self);
void finished_swim(edict_t *self);
void finished_runswim(edict_t *self);
void fish_walk(edict_t *self);
void fish_run(edict_t *self);
void finished_fish_pain(edict_t *self);
void fish_update_yaw(edict_t *self);
void fish_pause(edict_t *self);
void fish_chase(edict_t *self);
void fish_target(edict_t *self);
void fish_swim_sound (edict_t *self, float fast);
void fish_under_water_wake (edict_t *self);
