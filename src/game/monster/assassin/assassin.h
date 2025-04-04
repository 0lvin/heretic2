//
// Heretic II
// Copyright 1998 Raven Software
//
typedef enum AnimID_e
{
	ASSASSIN_ANIM_DAGGERL,// = {14, assassin_frames_daggerl, assassin_pause},
	ASSASSIN_ANIM_DAGGERR,//= {15, assassin_frames_daggerr, assassin_pause},
	ASSASSIN_ANIM_DAGGERB,// = {15, assassin_frames_daggerb, assassin_pause},
	ASSASSIN_ANIM_DAGGERC,// = {11, assassin_frames_daggerc, assassin_pause},
	ASSASSIN_ANIM_NEWDAGGER,
	ASSASSIN_ANIM_NEWDAGGERB,
	ASSASSIN_ANIM_BACKFLIP,// = {16, assassin_frames_backflip, assassin_pause},
	ASSASSIN_ANIM_FRONTFLIP,// = {16, assassin_frames_frontflip, assassin_pause},
	ASSASSIN_ANIM_DODGE_RIGHT,// = {10, assassin_frames_dodge_right, assassin_pause},
	ASSASSIN_ANIM_DODGE_LEFT,// = {10, assassin_frames_dodge_left, assassin_pause},
	ASSASSIN_ANIM_DEATHA,// = {14, assassin_frames_deatha, assassin_dead},
	ASSASSIN_ANIM_DEATHB,// = {14, assassin_frames_deathb, assassin_dead},
	ASSASSIN_ANIM_JUMP,// = {17, assassin_frames_jump, assassin_pause},
	ASSASSIN_ANIM_RUN,// = {10, assassin_frames_run, assassin_pause},
	ASSASSIN_ANIM_PAIN1,// = {5, assassin_frames_pain1, assassin_pause},
	ASSASSIN_ANIM_PAIN2,// = {4, assassin_frames_pain2, assassin_pause},
	ASSASSIN_ANIM_DELAY,// = {1, assassin_frames_delay, assassin_pause},
	ASSASSIN_ANIM_STAND,// = {1, assassin_frames_delay, assassin_pause},
	ASSASSIN_ANIM_CROUCH,// = {1, assassin_frames_delay, assassin_pause},
	ASSASSIN_ANIM_UNCROUCH,// = {1, assassin_frames_delay, assassin_pause},
	ASSASSIN_ANIM_EVJUMP,// = {17, assassin_frames_jump, assassin_pause},
	ASSASSIN_ANIM_EVBACKFLIP,// = {16, assassin_frames_backflip, assassin_pause},
	ASSASSIN_ANIM_EVFRONTFLIP,// = {16, assassin_frames_frontflip, assassin_pause},
	ASSASSIN_ANIM_INAIR,
	ASSASSIN_ANIM_LAND,
	ASSASSIN_ANIM_FORCED_JUMP,
	ASSASSIN_ANIM_FJUMP,
	ASSASSIN_ANIM_BFINAIR,
	ASSASSIN_ANIM_BFLAND,
	ASSASSIN_ANIM_FFINAIR,
	ASSASSIN_ANIM_FFLAND,
	ASSASSIN_ANIM_EVINAIR,
	ASSASSIN_ANIM_TELEPORT,
	ASSASSIN_ANIM_CLOAK,
	ASSASSIN_ANIM_WALK,
	ASSASSIN_ANIM_WALK_LOOP,
	ASSASSIN_ANIM_BACKSPRING,
//crouches
	ASSASSIN_ANIM_CROUCH_TRANS,
	ASSASSIN_ANIM_CROUCH_IDLE,
	ASSASSIN_ANIM_CROUCH_LOOK_RIGHT,
	ASSASSIN_ANIM_CROUCH_LOOK_RIGHT_IDLE,
	ASSASSIN_ANIM_CROUCH_LOOK_L2R,
	ASSASSIN_ANIM_CROUCH_LOOK_LEFT,
	ASSASSIN_ANIM_CROUCH_LOOK_LEFT_IDLE,
	ASSASSIN_ANIM_CROUCH_LOOK_R2L,
	ASSASSIN_ANIM_CROUCH_LOOK_R2C,
	ASSASSIN_ANIM_CROUCH_LOOK_L2C,
	ASSASSIN_ANIM_CROUCH_POKE,
	ASSASSIN_ANIM_CROUCH_END,

	ASSASSIN_ANIM_C_IDLE1,
	ASSASSIN_ANIM_C_RUN1,
	ASSASSIN_ANIM_C_ATTACK1,
	ASSASSIN_ANIM_C_ATTACK2,

	NUM_ANIMS
} AnimID_t;


typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_DIE1,
	SND_GIB,
	SND_THROW1,
	SND_THROW2,
	SND_DAGHITF,
	SND_DAGHITW,
	SND_JUMP,
	SND_FLIP,
	SND_LAND,
	SND_LANDF,
	SND_SLIDE,
	SND_SLASH1,
	SND_SLASH2,
	SND_GROWL1,
	SND_GROWL2,
	SND_GROWL3,
	SND_CLOAK,
	SND_DECLOAK,
	NUM_SOUNDS
} SoundID_t;

extern mmove_t assassin_move_daggerl;// = {14, assassin_frames_daggerl, assassin_pause};
extern mmove_t assassin_move_daggerr;//= {15, assassin_frames_daggerr, assassin_pause};
extern mmove_t assassin_move_daggerb;// = {15, assassin_frames_daggerb, assassin_pause};
extern mmove_t assassin_move_daggerc;// = {11, assassin_frames_daggerc, assassin_pause};
extern mmove_t assassin_move_newdagger;
extern mmove_t assassin_move_newdaggerb;
extern mmove_t assassin_move_backflip;// = {16, assassin_frames_backflip, assassin_pause};
extern mmove_t assassin_move_frontflip;// = {16, assassin_frames_frontflip, assassin_pause};
extern mmove_t assassin_move_dodge_right;// = {10, assassin_frames_dodge_right, assassin_pause};
extern mmove_t assassin_move_dodge_left;// = {10, assassin_frames_dodge_left, assassin_pause};
extern mmove_t assassin_move_deatha;// = {14, assassin_frames_deatha, assassin_dead};
extern mmove_t assassin_move_deathb;// = {14, assassin_frames_deathb, assassin_dead};
extern mmove_t assassin_move_jump;// = {17, assassin_frames_jump, assassin_pause};
extern mmove_t assassin_move_run;// = {10, assassin_frames_run, assassin_pause};
extern mmove_t assassin_move_pain1;// = {5, assassin_frames_pain1, assassin_pause};
extern mmove_t assassin_move_pain2;// = {4, assassin_frames_pain2, assassin_pause};
extern mmove_t assassin_move_delay;// = {1, assassin_frames_delay, assassin_pause};
extern mmove_t assassin_move_stand;// = {1, assassin_frames_stand, assassin_pause};
extern mmove_t assassin_move_crouch;
extern mmove_t assassin_move_uncrouch;

extern mmove_t assassin_move_evade_jump;// = {17, assassin_frames_jump, assassin_pause};
extern mmove_t assassin_move_evade_backflip;// = {16, assassin_frames_backflip, assassin_pause};
extern mmove_t assassin_move_evade_frontflip;// = {16, assassin_frames_frontflip, assassin_pause};
extern mmove_t assassin_move_inair;
extern mmove_t assassin_move_evinair;
extern mmove_t assassin_move_land;
extern mmove_t assassin_move_forcedjump;
extern mmove_t assassin_move_fjump;
extern mmove_t assassin_move_ffinair;
extern mmove_t assassin_move_ffland;
extern mmove_t assassin_move_bfinair;
extern mmove_t assassin_move_bfland;
extern mmove_t assassin_move_teleport;
extern mmove_t assassin_move_cloak;
extern mmove_t assassin_move_walk;
extern mmove_t assassin_move_walk_loop;
extern mmove_t assassin_move_backspring;
//crouch
extern mmove_t assassin_move_crouch_trans;
extern mmove_t assassin_move_crouch_idle;
extern mmove_t assassin_move_crouch_look_right;
extern mmove_t assassin_move_crouch_look_right_idle;
extern mmove_t assassin_move_crouch_look_l2r;
extern mmove_t assassin_move_crouch_look_left;
extern mmove_t assassin_move_crouch_look_left_idle;
extern mmove_t assassin_move_crouch_look_r2l;
extern mmove_t assassin_move_crouch_look_r2c;
extern mmove_t assassin_move_crouch_look_l2c;
extern mmove_t assassin_move_crouch_poke;
extern mmove_t assassin_move_crouch_end;

extern mmove_t assassin_move_c_idle1;
extern mmove_t assassin_move_c_run1;
extern mmove_t assassin_move_c_attack1;
extern mmove_t assassin_move_c_attack2;



void MG_CheckLanded (edict_t *self, float next_anim);
void MG_InAirMove (edict_t *self, float fwdspd,float upspd,float rtspd);
void MG_ApplyJump (edict_t *self);
void MG_InitMoods(edict_t *self);



void assassin_blocked(edict_t *self, G_Message_t *msg);
void assassin_death(edict_t *self, G_Message_t *msg);
void assassin_run(edict_t *self, G_Message_t *msg);
void assassin_walk(edict_t *self, G_Message_t *msg);
void assassin_melee(edict_t *self, G_Message_t *msg);
void assassin_stand(edict_t *self, G_Message_t *msg);
void assassin_pain(edict_t *self, G_Message_t *msg);

void assassindagger(edict_t *self, float right_ofs);
void assassin_dead(edict_t *self);
void assassindeathsqueal(edict_t *self);
void assassinsqueal(edict_t *self);
void assassingrowl(edict_t *self);
void assassinbite(edict_t *self);
void assassin_think_pain(edict_t *self);
void assassin_pause (edict_t *self);
void assassin_dropweapon (edict_t *self, int whichknives);
void ai_charge2 (edict_t *self, float dist);
void assassinCrouchedCheckAttack (edict_t *self, float attack);
void assassinSetCrouched (edict_t *self);
void assassinUndoCrouched (edict_t *self);
void assassinGoJump (edict_t *self, float upspd,float fwdspd,float rtspd);
void assassinStop (edict_t *self);
void assassin_go_run(edict_t *self, float dist);
void mg_ai_charge (edict_t *self, float dist);
void assassinCheckLoop (edict_t *self, float frame);
void assassinCheckLanded (edict_t *self, float nextanim);
void assassin_go_inair(edict_t *self);
void assassin_go_ffinair(edict_t *self);
void assassin_go_bfinair(edict_t *self);
void assassinNodeOn (edict_t *self, float node);
void assassinApplyJump (edict_t *self);
void assassinInAirMove (edict_t *self, float fwdspd,float upspd,float rtspd);
void assassin_go_evinair(edict_t *self);
void assassin_sound(edict_t *self, float channel, float soundnum, float attn);
qboolean assassinCheckTeleport (edict_t *self, int type);
qboolean assassinChooseTeleportDestination(edict_t *self, int type, qboolean imperative, qboolean instant);
void assassinGone(edict_t *self);
void assassinSmoke(edict_t *self);
void SP_monster_assassin (edict_t *self);
void assassin_post_pain (edict_t *self);
void assassinCloak (edict_t *self);
void M_MoveFrame (edict_t *self);
void assassinInitCloak (edict_t *self);
void assassinDeCloak (edict_t *self);
void assassinReadyTeleport (edict_t *self);
void assassinUnCrouch (edict_t *self);
void assassinSkipFrameSkillCheck (edict_t *self);
void MG_CheckEvade (edict_t *self);
void FoundTarget (edict_t *self, qboolean setsightent);
void assasin_walk_loop_go (edict_t *self);
void assassin_ai_walk (edict_t *self, float dist);
void assassin_crouch_idle_decision (edict_t *self);


#define BIT_DADDYNULL	0
#define BIT_TORSOFT		1
#define BIT_TORSOBK		2
#define BIT_HEAD		4
#define BIT_LKNIFE		8
#define BIT_RKNIFE		16
#define BIT_R4ARM		32
#define BIT_L4ARM		64
#define BIT_HIPS		128
#define BIT_LCALF		256
#define BIT_RCALF		512
#define BIT_RTHIGH		1024
#define BIT_LTHIGH		2048
#define BIT_KNIFES		4096
#define BIT_LUPARM		8192
#define BIT_RUPARM		16384
