//==============================================================================
//
// m_gkrokon_anim.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================

#include "../../header/local.h"
#include "gkrokon_anim.h"
#include "gkrokon.h"
#include "../../common/h2rand.h"

// ****************************************************************************
// Stand1 - Laid down, resting, still on the floor.
// ****************************************************************************

static mframe_t GkrokonFramesStand1[]=
{
	{FRAME_bwait1,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait2,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait3,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait4,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait5,	NULL, 0, 0, 0, beetle_ai_stand,	0,	beetle_idle_sound},
	{FRAME_bwait6,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait7,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait8,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait9,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait10,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait11,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait12,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait13,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait14,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
};
mmove_t GkrokonMoveStand1 = {FRAME_bwait1, FRAME_bwait14, GkrokonFramesStand1, GkrokonPause};//GkrokonOrderStand};

// ****************************************************************************
// Stand2 - Getting up off the floor.
// ****************************************************************************

static mframe_t GkrokonFramesStand2[]=
{
	{FRAME_birth1,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth2,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth3,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth4,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth5,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth6,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth7,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth8,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth9,	NULL, 0, 0, 0, beetle_ai_stand,	0,	beetle_idle_sound},
	{FRAME_birth10,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth11,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth12,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
};
mmove_t GkrokonMoveStand2 = {FRAME_birth1, FRAME_birth12, GkrokonFramesStand2, GkrokonPause};//GkrokonOrderStand};

// ****************************************************************************
// Stand3 - Standing fairly still, waiting.
// ****************************************************************************

static mframe_t GkrokonFramesStand3[]=
{
	{FRAME_wait1,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_wait2,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_wait3,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_wait4,	NULL, 0, 0, 0, beetle_ai_stand,	0,	beetle_idle_sound},
	{FRAME_wait5,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_wait6,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_wait7,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_wait8,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
};
mmove_t GkrokonMoveStand3 = {FRAME_wait1, FRAME_wait8, GkrokonFramesStand3, GkrokonPause};//GkrokonOrderStand};

// ****************************************************************************
// Stand4 - Settling down onto the floor.
// ****************************************************************************

static mframe_t GkrokonFramesStand4[]=
{
	{FRAME_birth5,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth4,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth3,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_birth2,	NULL, 0, 0, 0, beetle_ai_stand,	0,	beetle_idle_sound},
	{FRAME_birth1,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
};
mmove_t GkrokonMoveStand4 = {FRAME_birth5, FRAME_birth5 + 4, GkrokonFramesStand4, GkrokonPause};//GkrokonOrderStand};

// ****************************************************************************
// Crouch1 - Crouched down on the floor (stalking enemy).
// ****************************************************************************

static mframe_t GkrokonFramesCrouch1[]=
{
	{FRAME_bwait1,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait2,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait3,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait4,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait5,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait6,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait7,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait8,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait9,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait10,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait11,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait12,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait13,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
	{FRAME_bwait14,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL},
};
mmove_t GkrokonMoveCrouch1 = {FRAME_bwait1, FRAME_bwait14, GkrokonFramesCrouch1, GkrokonPause};//GkrokonOrderCrouch};

// ****************************************************************************
// Crouch2 - Getting up off the floor from crouching (stalking enemy).
// ****************************************************************************

static mframe_t GkrokonFramesCrouch2[]=
{
	{FRAME_birth1,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth2,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth3,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth4,	NULL, 0, 0, 0, 	NULL,	0,	NULL},
};
mmove_t GkrokonMoveCrouch2 = {FRAME_birth1, FRAME_birth4, GkrokonFramesCrouch2, beetle_to_stand};//GkrokonOrderCrouch};

// ****************************************************************************
// Crouch3 - Settling down into crouching position (stalking enemy).
// ****************************************************************************

static mframe_t GkrokonFramesCrouch3[]=
{
	{FRAME_birth4,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth3,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth2,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth1,	NULL, 0, 0, 0, NULL,	0,	NULL},
};
mmove_t GkrokonMoveCrouch3 = {FRAME_birth1, FRAME_birth4, GkrokonFramesCrouch3, beetle_to_crouch};//GkrokonOrderCrouch};

// ****************************************************************************
// Walk1 - A leisurely ambling gait.
// ****************************************************************************

static mframe_t GkrokonFramesWalk1[]=
{
	{FRAME_walkB1,	NULL, 0, 0, 0, beetle_ai_stand,	6,	NULL},
	{FRAME_walkB2,	NULL, 0, 0, 0, beetle_ai_stand,	7,	NULL},
	{FRAME_walkB3,	NULL, 0, 0, 0, beetle_ai_stand,	5,	NULL},
	{FRAME_walkB4,	NULL, 0, 0, 0, beetle_ai_stand,	8,	NULL},
	{FRAME_walkB5,	NULL, 0, 0, 0, beetle_ai_stand,	7,	NULL},
	{FRAME_walkB6,	NULL, 0, 0, 0, beetle_ai_stand,	6,	gkrokonRandomWalkSound},
	{FRAME_walkB7,	NULL, 0, 0, 0, beetle_ai_stand,	5,	NULL},
	{FRAME_walkB8,	NULL, 0, 0, 0, beetle_ai_stand,	8,	NULL},
};
mmove_t GkrokonMoveWalk1 = {FRAME_walkB1, FRAME_walkB8, GkrokonFramesWalk1, GkrokonPause};

// ****************************************************************************
// Run1 - A galloping run.
// ****************************************************************************

static mframe_t GkrokonFramesRun1[]=
{
	{FRAME_gallop1,	NULL, 0, 0, 0, ai_run,	16,	NULL},
	{FRAME_gallop2,	NULL, 0, 0, 0, ai_run,	24,	NULL},
	{FRAME_gallop3,	NULL, 0, 0, 0, ai_run,	22,	NULL},
	{FRAME_gallop4,	NULL, 0, 0, 0, ai_run,	18,	gkrokonRandomWalkSound},
	{FRAME_gallop5,	NULL, 0, 0, 0, ai_run,	16,	NULL},
	{FRAME_gallop6,	NULL, 0, 0, 0, ai_run,	24,	NULL},
};
mmove_t GkrokonMoveRun1 = {FRAME_gallop1, FRAME_gallop6, GkrokonFramesRun1, GkrokonPause};//GkrokonOrderRun};

// ****************************************************************************
// Run2 - A skittering, insectlike run.
// ****************************************************************************

static mframe_t GkrokonFramesRun2[]=
{
	{FRAME_skittr1,	NULL, 0, 0, 0, ai_run,	12,	NULL},
	{FRAME_skittr2,	NULL, 0, 0, 0, ai_run,	12,	NULL},
	{FRAME_skittr3,	NULL, 0, 0, 0, ai_run,	12,	gkrokonRandomWalkSound},
	{FRAME_skittr4,	NULL, 0, 0, 0, ai_run,	12,	NULL},
};
mmove_t GkrokonMoveRun2 = {FRAME_skittr1, FRAME_skittr4, GkrokonFramesRun2, GkrokonPause};//GkrokonOrderRun};

static mframe_t GkrokonFramesRunAway[]=
{
	{FRAME_skittr4,	gkrokonSound, CHAN_VOICE, SND_FLEE, ATTN_NORM, ai_run,	-14,	NULL},
	{FRAME_skittr3,	NULL, 0, 0, 0, ai_run,	-16,	NULL},
	{FRAME_skittr2,	NULL, 0, 0, 0, ai_run,	-14,	gkrokonRandomWalkSound},
	{FRAME_skittr1,	NULL, 0, 0, 0, ai_run,	-12,	NULL},
	{FRAME_skittr4,	NULL, 0, 0, 0, ai_run,	-14,	NULL},
	{FRAME_skittr3,	NULL, 0, 0, 0, ai_run,	-16,	NULL},
	{FRAME_skittr2,	NULL, 0, 0, 0, ai_run,	-14,	gkrokonRandomWalkSound},
	{FRAME_skittr1,	NULL, 0, 0, 0, ai_run,	-12,	NULL},
};
mmove_t GkrokonMoveRunAway = {FRAME_skittr1, FRAME_skittr1 + 7, GkrokonFramesRunAway, GkrokonPause};//GkrokonOrderRun};

// ****************************************************************************
// Jump1 - Jumping.
// ****************************************************************************

static mframe_t GkrokonFramesJump1[]=
{
	{FRAME_jump1,	gkrokonSound, CHAN_VOICE, SND_ANGRY, ATTN_NORM, NULL,	0,	NULL},
	{FRAME_jump2,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump4,	NULL, 0, 0, 0, NULL,	0,	/*GkrokonJump*/NULL},
	{FRAME_jump6,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump10,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump12,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump14,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump16,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump18,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump20,	NULL, 0, 0, 0, NULL,	0,	/*GkrokonBite*/NULL},
	{FRAME_jump22,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump23,	NULL, 0, 0, 0, NULL,	0,	NULL},
};
mmove_t GkrokonMoveJump1 = {FRAME_jump1, FRAME_jump1 + 11, GkrokonFramesJump1, GkrokonPause}; //GkrokonOrderRun};

static mframe_t GkrokonFramesForcedJump[]=
{
	{FRAME_jump1,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump2,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump4,	NULL, 0, 0, 0, NULL,	0,	/*GkrokonJump*/NULL},
	{FRAME_jump6,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump10,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump12,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump14,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump16,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump18,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump20,	NULL, 0, 0, 0, NULL,	0,	/*GkrokonBite*/NULL},
	{FRAME_jump22,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_jump23,	NULL, 0, 0, 0, NULL,	0,	NULL},
};
mmove_t GkrokonMoveForcedJump = {FRAME_jump1, FRAME_jump1 + 11, GkrokonFramesForcedJump, GkrokonPause}; //GkrokonOrderRun};

// ****************************************************************************
// GkrokonFramesMeleeAttack1 - A bite attack on my enemy.
// ****************************************************************************
static mframe_t GkrokonFramesMeleeAttack1[]=
{
	{FRAME_latack1,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_latack2,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_latack3,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_latack4,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_latack5,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_latack6,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_latack7,	NULL, 0, 0, 0, GkrokonBite,	0, NULL},
	{FRAME_latack8,	NULL, 0, 0, 0, NULL,	0,	NULL},
};
mmove_t GkrokonMoveMeleeAttack1 = {FRAME_latack1, FRAME_latack8, GkrokonFramesMeleeAttack1, GkrokonPause};//GkrokonOrderStand};

static mframe_t GkrokonFramesMeleeAttack2[]=
{
	{FRAME_ratack1,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_ratack2,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_ratack3,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_ratack4,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_ratack5,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_ratack6,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_ratack7,	NULL, 0, 0, 0, GkrokonBite,	1, NULL},
	{FRAME_ratack8,	NULL, 0, 0, 0, NULL,	0,	NULL},
};
mmove_t GkrokonMoveMeleeAttack2 = {FRAME_ratack1, FRAME_ratack8, GkrokonFramesMeleeAttack2, GkrokonPause};//GkrokonOrderStand};

// ****************************************************************************
// GkrokonFramesMissileAttack1 - Firing spoo-goo from spoo launcher.
// ****************************************************************************

static mframe_t GkrokonFramesMissileAttack1[]=
{
	{FRAME_spoo1,	NULL, 0, 0, 0, ai_charge,	0,	NULL},
	{FRAME_spoo2,	NULL, 0, 0, 0, ai_charge,	0,	NULL},
	{FRAME_spoo3,	NULL, 0, 0, 0, ai_charge,	0,	NULL},
	{FRAME_spoo4,	NULL, 0, 0, 0, ai_charge,	0,	NULL},
	{FRAME_spoo5,	NULL, 0, 0, 0, ai_charge,	0,	GkrokonSpoo},
};
mmove_t GkrokonMoveMissileAttack1 = {FRAME_spoo1, FRAME_spoo5, GkrokonFramesMissileAttack1, GkrokonPause};//GkrokonOrderRun};

static mframe_t GkrokonFramesMissileAttack2[]=
{
	{FRAME_spoo1,	NULL, 0, 0, 0, ai_charge,	0,	NULL},
	{FRAME_spoo2,	NULL, 0, 0, 0, ai_charge,	0,	NULL},
	{FRAME_spoo3,	NULL, 0, 0, 0, ai_charge,	0,	NULL},
	{FRAME_spoo4,	NULL, 0, 0, 0, ai_charge,	0,	NULL},
	{FRAME_spoo5,	NULL, 0, 0, 0, ai_charge,	0,	NULL},
};
mmove_t GkrokonMoveMissileAttack2 = {FRAME_spoo1, FRAME_spoo5, GkrokonFramesMissileAttack2, GkrokonPause};//GkrokonOrderRun};

// ****************************************************************************
// GkrokonFramesEat1 - Going from ready to eating.
// ****************************************************************************

static mframe_t GkrokonFramesEat1[]=
{
	{FRAME_eat1,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
	{FRAME_eat2,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
	{FRAME_eat3,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
	{FRAME_eat4,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
};
mmove_t GkrokonMoveEat1 = {FRAME_eat1, FRAME_eat4, GkrokonFramesEat1, GkrokonPause};//GkrokonOrderEat};

// ****************************************************************************
// GkrokonFramesEat2 - The eat cycle.
// ****************************************************************************

static mframe_t GkrokonFramesEat2[]=
{
	{FRAME_eat5,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
	{FRAME_eat6,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
	{FRAME_eat7,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
	{FRAME_eat8,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
	{FRAME_eat9,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
};
mmove_t GkrokonMoveEat2 = {FRAME_eat5, FRAME_eat9, GkrokonFramesEat2, GkrokonPause};//GkrokonOrderEat};

// ****************************************************************************
// GkrokonFramesEat3 - Going from eating to ready.
// ****************************************************************************

static mframe_t GkrokonFramesEat3[]=
{
	{FRAME_EATTRANS1,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
	{FRAME_EATTRANS2,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
	{FRAME_EATTRANS3,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
	{FRAME_EATTRANS4,	NULL, 0, 0, 0, ai_eat,	0,	NULL},
};
mmove_t GkrokonMoveEat3 = {FRAME_EATTRANS1, FRAME_EATTRANS4, GkrokonFramesEat3, GkrokonPause};//GkrokonOrderEat};

// ****************************************************************************
// Pain1 -
// ****************************************************************************

static mframe_t GkrokonFramesPain1[]=
{
	{FRAME_pain1,	NULL, 0, 0, 0, ai_charge,	-1,	NULL},
	{FRAME_pain2,	NULL, 0, 0, 0, ai_charge,	-1,	NULL},
	{FRAME_pain3,	NULL, 0, 0, 0, ai_charge,	-1,	NULL},
	{FRAME_pain4,	NULL, 0, 0, 0, ai_charge,	-1,	NULL},
	{FRAME_pain5,	NULL, 0, 0, 0, ai_charge,	-1,	NULL},
	{FRAME_pain6,	NULL, 0, 0, 0, ai_charge,	-1,	NULL},
	{FRAME_pain7,	NULL, 0, 0, 0, ai_charge,	-1,	NULL},
	{FRAME_pain8,	NULL, 0, 0, 0, ai_charge,	-1,	NULL},
};
mmove_t GkrokonMovePain1 = {FRAME_pain1, FRAME_pain8, GkrokonFramesPain1, GkrokonPause};//GkrokonOrderRun};

// ****************************************************************************
// Death1 -
// ****************************************************************************

mframe_t GkrokonFramesDeath1[]=
{
	{FRAME_death1,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death2,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death3,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death4,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death5,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death6,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death7,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death8,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death9,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death10,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death11,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death12,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death13,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death14,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death15,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_death16,	NULL, 0, 0, 0, ai_move,	2,	NULL},
	{FRAME_death17,	NULL, 0, 0, 0, ai_move,	2,	NULL},
	{FRAME_death18,	NULL, 0, 0, 0, ai_move,	2,	NULL},
	{FRAME_death19,	NULL, 0, 0, 0, ai_move,	2,	NULL},
	{FRAME_death20,	NULL, 0, 0, 0, ai_move,	2,	NULL},
	{FRAME_death21,	NULL, 0, 0, 0, ai_move,	2,	NULL},
	{FRAME_death22,	NULL, 0, 0, 0, ai_move,	2,	NULL},
	{FRAME_death23,	NULL, 0, 0, 0, ai_move,	2,	NULL},
	{FRAME_death24,	NULL, 0, 0, 0, ai_move,	2,	NULL},
	{FRAME_death25,	NULL, 0, 0, 0, ai_move,	2,	NULL},
	{FRAME_death26,	NULL, 0, 0, 0, ai_move,	2,	NULL},
};
mmove_t GkrokonMoveDeath1 = {FRAME_death1, FRAME_death26, GkrokonFramesDeath1, GkrokonDead};

static mframe_t GkrokonStartHop[]=
{
	{FRAME_birth5,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth4,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth5,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth6,	NULL, 0, 0, 0, NULL,	0,	/*gkrokon_hopdown*/NULL},
	{FRAME_birth7,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth8,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth9,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth10,	NULL, 0, 0, 0, NULL,	0,	gkrokonRandomWalkSound},
	{FRAME_birth11,	NULL, 0, 0, 0, NULL,	0,	NULL},
	{FRAME_birth12,	NULL, 0, 0, 0, NULL,	0,	/*gkrokon_donehop*/NULL},
};
mmove_t GkrokonMoveHop1 = {FRAME_birth5, FRAME_birth12, GkrokonStartHop, GkrokonPause};

static mframe_t GkrokonFramesDelay[]=
{
	{FRAME_bwait1,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
	{FRAME_bwait2,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
	{FRAME_bwait3,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
	{FRAME_bwait4,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
	{FRAME_bwait5,	gkrokonSound, CHAN_VOICE, SND_IDLE1, ATTN_NORM, NULL,	0,	GkrokonPause},
	{FRAME_bwait6,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
	{FRAME_bwait7,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
	{FRAME_bwait8,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
	{FRAME_bwait9,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
	{FRAME_bwait10,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
	{FRAME_bwait11,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
	{FRAME_bwait12,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
	{FRAME_bwait13,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
	{FRAME_bwait14,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause},
};
mmove_t GkrokonMoveDelay = {FRAME_bwait1, FRAME_bwait14, GkrokonFramesDelay, GkrokonPause};
