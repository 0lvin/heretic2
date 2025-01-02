//
// Heretic II
// Copyright 1998 Raven Software
//

typedef enum AnimID_e
{
	PRIESTESS_ANIM_STAND1,
	PRIESTESS_ANIM_ATTACK1_GO,
	PRIESTESS_ANIM_ATTACK1_LOOP,
	PRIESTESS_ANIM_ATTACK1_END,
	PRIESTESS_ANIM_ATTACK2,
	PRIESTESS_ANIM_BACKUP,
	PRIESTESS_ANIM_DEATH,
	PRIESTESS_ANIM_IDLE,
	PRIESTESS_ANIM_JUMP,
	PRIESTESS_ANIM_PAIN,
	PRIESTESS_ANIM_IDLE_POSE,
	PRIESTESS_ANIM_POSE_TRANS,
	PRIESTESS_ANIM_SHIELD_GO,
	PRIESTESS_ANIM_SHIELD_END,
	PRIESTESS_ANIM_DODGE_LEFT,
	PRIESTESS_ANIM_DODGE_RIGHT,
	PRIESTESS_ANIM_WALK,
	PRIESTESS_ANIM_JUMP_FORWARD,
	PRIESTESS_ANIM_JUMP_BACK,
	PRIESTESS_ANIM_JUMP_RIGHT,
	PRIESTESS_ANIM_JUMP_LEFT,
	PRIESTESS_ANIM_JUMP_POUNCE,
	PRIESTESS_ANIM_POUNCE_ATTACK,
	PRIESTESS_ANIM_ATTACK3_GO,
	PRIESTESS_ANIM_ATTACK3_LOOP,
	PRIESTESS_ANIM_ATTACK3_END,
	PRIESTESS_ANIM_JUMP_ATTACK,
	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{	//shield being used at all?
	//reuse staff swing, hit, and landing sounds
//she takes pain
	SND_PAIN1,
	SND_PAIN2,
//you win
	SND_FALL,
//attack 1 - 3 balls spell
	SND_3BALLATK,
	SND_BALLHIT,
//attack 2 - whirling staff
	SND_WHIRL,
	SND_SWIPE,
	SND_SWIPEHIT,
	SND_SWIPEMISS,
	SND_SWIPEWALL,
//bug minefield
	SND_BUGS,//fire a bug out
	SND_BUGBUZZ,//bug buzzes in place
	SND_BUGHIT,//bug explodes
//ray of light
	SND_ZAP,//earthquake, lightning, rays of light
	SND_ZAPHIT,//earthquake, lightning, rays of light
//homing missiles
	SND_HOMINGATK,
	SND_HOMINGHIT,
//teleport in & out
	SND_TPORT_IN,
	SND_TPORT_OUT,
	NUM_SOUNDS
} SoundID_t;

extern mmove_t	priestess_move_stand1,
					priestess_move_attack1_go,
					priestess_move_attack1_loop,
					priestess_move_attack1_end,
					priestess_move_attack2,
					priestess_move_backup,
					priestess_move_death,
					priestess_move_idle,
					priestess_move_jump,
					priestess_move_pain,
					priestess_move_idle_pose,
					priestess_move_pose_trans,
					priestess_move_shield_go,
					priestess_move_shield_end,
					priestess_move_dodge_left,
					priestess_move_dodge_right,
					priestess_move_walk,
					priestess_move_jump_forward,
					priestess_move_jump_back,
					priestess_move_jump_right,
					priestess_move_jump_left,
					priestess_move_jump_pounce,
					priestess_move_pounce_attack,
					priestess_move_attack3_go,
					priestess_move_attack3_loop,
					priestess_move_attack3_end,
					priestess_move_jump_attack;

void priestess_fire1( edict_t *self, float pitch_ofs, float yaw_ofs, float roll_ofs );

void priestess_jump_forward( edict_t *self );
void priestess_jump_back( edict_t *self );
void priestess_jump_right( edict_t *self );
void priestess_jump_left( edict_t *self );

void priestess_strike( edict_t *self, float damage );

void priestess_pounce_attack( edict_t *self );
void priestess_idle( edict_t *self );
void priestess_pounce( edict_t *self );
void priestess_move( edict_t *self, float vf, float vr, float vu );
void priestess_pause( edict_t *self );

void priestess_attack3_loop( edict_t *self );
void priestess_attack3_loop_fire( edict_t *self );

void priestess_jump_attack( edict_t *self );

void priestess_teleport_go( edict_t *self );
void priestess_teleport_end( edict_t *self );
void priestess_teleport_move( edict_t *self );
void priestess_teleport_return( edict_t *self );
void priestess_teleport_self_effects( edict_t *self );

void priestess_attack1_pause( edict_t *self );
void priestess_dead( edict_t *self );
void priestess_stop_alpha( edict_t *self );

void priestess_delta_alpha( edict_t *self, float amount );
