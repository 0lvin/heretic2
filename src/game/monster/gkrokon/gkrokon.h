//==============================================================================
//
// m_gkrokon.h
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================

#define GKROKON_JUMP_VELOCITY	300.0
#define GKROKON_HOP_VELOCITY	128.0

#define GKROKON_SPOO_SPEED 450.0
#define GKROKON_SPOO_ARC 150.0

// ****************************************************************************
// The Gkrokon's animations.
// ****************************************************************************

typedef enum AnimID_e
{
	GKROKON_ANIM_STAND1,
	GKROKON_ANIM_STAND2,
	GKROKON_ANIM_STAND3,
	GKROKON_ANIM_STAND4,
	GKROKON_ANIM_CROUCH1,
	GKROKON_ANIM_CROUCH2,
	GKROKON_ANIM_CROUCH3,
	GKROKON_ANIM_WALK1,
	GKROKON_ANIM_RUN1,
	GKROKON_ANIM_RUN2,
	GKROKON_ANIM_JUMP1,
	GKROKON_ANIM_FJUMP,
	GKROKON_ANIM_MELEE1,
	GKROKON_ANIM_MELEE2,
	GKROKON_ANIM_MISSILE1,
	GKROKON_ANIM_EAT1,
	GKROKON_ANIM_EAT2,
	GKROKON_ANIM_EAT3,
	GKROKON_ANIM_PAIN1,
	GKROKON_ANIM_DIE1,
	GKROKON_ANIM_HOP,
	GKROKON_ANIM_RUNAWAY,
	GKROKON_ANIM_SNEEZE,
	GKROKON_ANIM_DELAY,
	GKROKON_NUM_ANIMS
} AnimID_t;

// ****************************************************************************
// The Gkrokon's sounds.
// ****************************************************************************

typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_DIE,
	SND_GIB,
	SND_SPOO,
	SND_IDLE1,
	SND_IDLE2,
	SND_SIGHT,
	SND_WALK1,
	SND_WALK2,
	SND_FLEE,
	SND_ANGRY,
	SND_EATING,
	SND_BITEHIT1,
	SND_BITEHIT2,
	SND_BITEMISS1,
	SND_BITEMISS2,
	NUM_SOUNDS
} SoundID_t;

extern mmove_t GkrokonMoveStand1;
extern mmove_t GkrokonMoveStand2;
extern mmove_t GkrokonMoveStand3;
extern mmove_t GkrokonMoveStand4;
extern mmove_t GkrokonMoveCrouch1;
extern mmove_t GkrokonMoveCrouch2;
extern mmove_t GkrokonMoveCrouch3;
extern mmove_t GkrokonMoveWalk1;
extern mmove_t GkrokonMoveRun1;
extern mmove_t GkrokonMoveRun2;
extern mmove_t GkrokonMoveRunAway;
extern mmove_t GkrokonMoveJump1;
extern mmove_t GkrokonMoveForcedJump;
extern mmove_t GkrokonMoveMeleeAttack1;
extern mmove_t GkrokonMoveMeleeAttack2;
extern mmove_t GkrokonMoveMissileAttack1;
extern mmove_t GkrokonMoveMissileAttack2;
extern mmove_t GkrokonMoveEat1;
extern mmove_t GkrokonMoveEat2;
extern mmove_t GkrokonMoveEat3;
extern mmove_t GkrokonMovePain1;
extern mmove_t GkrokonMoveDeath1;
//extern mmove_t GkrokonMoveDeath_hold;
extern mmove_t GkrokonMoveHop1;
extern mmove_t GkrokonMoveDelay;

void GkrokonSpooTouch2(edict_t *self,trace_t *trace);

void GkrokonPause(edict_t *self);
void gkrokonSound(edict_t *self, float channel, float sndindex, float atten);
void gkrokonRandomWalkSound (edict_t *self);
void GkrokonSpoo(edict_t *self);
void GkrokonDead(edict_t *self);

void extrapolateFiredir (edict_t *self,vec3_t p1,float pspeed,edict_t *targ,float accept,vec3_t vec2);

void create_gkrokon_spoo(edict_t *Spoo);
void BecomeDebris(edict_t *self);

void GkrokonSpooThink(edict_t *self);
void GkrokonSpooTouch(edict_t *self,edict_t *Other,cplane_t *Plane,csurface_t *Surface);

trace_t trace_dir(edict_t *self, vec3_t source, vec3_t angles, float dist);

void GkrokonStaticsInit(void);
void GkrokonInit(void);

void beetle_ai_stand(edict_t *self, float dist);
float MG_FaceGoal (edict_t *self, qboolean doturn);
void beetle_idle_sound(edict_t *self);
void beetle_to_stand (edict_t *self);
void beetle_to_crouch (edict_t *self);
void GkrokonBite(edict_t *self, float value);

#define BIT_WAIT1			0
#define BIT_SHELLA_P1		1
#define BIT_SPIKE_P1		2
#define BIT_HEAD_P1			4
#define BIT_RPINCHERA_P1	8
#define BIT_RPINCHERB_P1	16
#define BIT_LPINCHERA_P1	32
#define BIT_LPINCHERB_P1	64
#define BIT_LARM_P1			128
#define BIT_RARM_P1			256
#define BIT_ABDOMEN_P1		512
#define BIT_LTHIGH_P1		1024
#define BIT_RTHIGH_P1		2048
#define BIT_SHELLB_P1		4096
