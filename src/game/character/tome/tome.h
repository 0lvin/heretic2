//
// Heretic II
// Copyright 1998 Raven Software
//
typedef enum AnimID_e
{
	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	NUM_ANIMS
} AnimID_t;

void MG_InitMoods(edict_t *self);

extern mmove_t tome_move_c_idle1;
extern mmove_t tome_move_c_idle2;
