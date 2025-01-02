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

typedef enum SoundID_e
{
	SND_GROWL1,
	SND_GROWL2,
	SND_PAIN,
	SND_GIB,

	NUM_SOUNDS
} SoundID_t;

extern mmove_t mother_move_pain;
extern mmove_t mother_move_stand;

void mother_pause (edict_t *self);
