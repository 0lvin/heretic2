//
// Heretic II
// Copyright 1998 Raven Software
//
typedef enum AnimID_e
{
	ANIM_C_ACTION1,
	ANIM_C_ACTION2,
	ANIM_C_ACTION3,
	ANIM_C_ACTION4,
	ANIM_C_ACTION5,
	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	ANIM_C_WALKSTART,
	ANIM_C_WALK1,
	ANIM_C_WALK2,
	ANIM_C_WALKSTOP1,
	ANIM_C_WALKSTOP2,
	ANIM_C_PIVOTLEFTGO,
	ANIM_C_PIVOTLEFT,
	ANIM_C_PIVOTLEFTSTOP,
	ANIM_C_PIVOTRIGHTGO,
	ANIM_C_PIVOTRIGHT,
	ANIM_C_PIVOTRIGHTSTOP,
	NUM_ANIMS
} AnimID_t;

void MG_InitMoods(edict_t *self);


extern mmove_t corvus4_move_c_action1;
extern mmove_t corvus4_move_c_action2;
extern mmove_t corvus4_move_c_action3;
extern mmove_t corvus4_move_c_action4;
extern mmove_t corvus4_move_c_action5;
extern mmove_t corvus4_move_c_idle1;
extern mmove_t corvus4_move_c_idle2;
extern mmove_t corvus4_move_c_walkstart;
extern mmove_t corvus4_move_c_walk1;
extern mmove_t corvus4_move_c_walk2;
extern mmove_t corvus4_move_c_walkstop1;
extern mmove_t corvus4_move_c_walkstop2;
extern mmove_t corvus4_move_c_pivotleftgo;
extern mmove_t corvus4_move_c_pivotleft;
extern mmove_t corvus4_move_c_pivotleftstop;
extern mmove_t corvus4_move_c_pivotrightgo;
extern mmove_t corvus4_move_c_pivotright;
extern mmove_t corvus4_move_c_pivotrightstop;
