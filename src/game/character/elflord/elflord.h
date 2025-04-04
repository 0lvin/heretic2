//
// Heretic II
// Copyright 1998 Raven Software
//
//c_elflord.h


typedef enum AnimID_e
{
	ANIM_C_ACTION1,
	ANIM_C_ACTION2,
	ANIM_C_DEATH1,
	ANIM_C_DEATH2,
	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	ANIM_C_GIB1,
	NUM_ANIMS
} AnimID_t;

#define MODEL_SCALE		1.000000

extern mmove_t Elflord_move_c_action1;
extern mmove_t Elflord_move_c_action2;
extern mmove_t Elflord_move_c_death1;
extern mmove_t Elflord_move_c_death2;
extern mmove_t Elflord_move_c_idle1;
extern mmove_t Elflord_move_c_idle2;
extern void Elflord_c_boom(edict_t *self);
