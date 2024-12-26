//
// Heretic II
// Copyright 1998 Raven Software
//
//c_Dranor.h


typedef enum AnimID_e
{
	ANIM_C_ACTION1,
	ANIM_C_ACTION2,
	ANIM_C_ACTION3,
	ANIM_C_ACTION4,
	ANIM_C_ACTION5,
	ANIM_C_ACTION6,
	ANIM_C_ACTION7,
	ANIM_C_ACTION8,
	ANIM_C_ACTION9,
	ANIM_C_ACTION10,
	ANIM_C_ACTION11,
	ANIM_C_ACTION12,
	ANIM_C_DEATH1,
	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	ANIM_C_IDLE3,
	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,
	NUM_SOUNDS
} SoundID_t;

#define MODEL_SCALE		1.000000

extern mmove_t dranor_move_c_action1;
extern mmove_t dranor_move_c_action2;
extern mmove_t dranor_move_c_action3;
extern mmove_t dranor_move_c_action4;
extern mmove_t dranor_move_c_action5;
extern mmove_t dranor_move_c_action6;
extern mmove_t dranor_move_c_action7;
extern mmove_t dranor_move_c_action8;
extern mmove_t dranor_move_c_action9;
extern mmove_t dranor_move_c_action10;
extern mmove_t dranor_move_c_action11;
extern mmove_t dranor_move_c_action12;
extern mmove_t dranor_move_c_death1;
extern mmove_t dranor_move_c_idle1;
extern mmove_t dranor_move_c_idle2;
extern mmove_t dranor_move_c_idle3;
