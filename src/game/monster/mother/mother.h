//
// Heretic II
// Copyright 1998 Raven Software
//
typedef enum AnimID_e
{
	MOTHER_ANIM_PAIN,
	MOTHER_ANIM_STAND,
	MOTHER_NUM_ANIMS
} AnimID_t;

extern mmove_t mother_move_pain;
extern mmove_t mother_move_stand;

void mother_pause (edict_t *self);
