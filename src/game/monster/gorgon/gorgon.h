//
// Heretic II
// Copyright 1998 Raven Software
//
typedef enum AnimID_e
{
	GORGON_ANIM_STAND1,
	GORGON_ANIM_STAND2,
	GORGON_ANIM_STAND3,
	GORGON_ANIM_STAND4,
	GORGON_ANIM_WALK1,
	GORGON_ANIM_WALK2,
	GORGON_ANIM_WALK3,//10
	GORGON_ANIM_MELEE1,
	GORGON_ANIM_MELEE2,
	GORGON_ANIM_MELEE3,
	GORGON_ANIM_MELEE4,
	GORGON_ANIM_MELEE5,
	GORGON_ANIM_MELEE6,
	GORGON_ANIM_MELEE7,
	GORGON_ANIM_MELEE8,
	GORGON_ANIM_MELEE9,
	GORGON_ANIM_MELEE10,//20
	GORGON_ANIM_FJUMP,
	GORGON_ANIM_RUN1,
	GORGON_ANIM_RUN2,
	GORGON_ANIM_RUN3,
	GORGON_ANIM_PAIN1,
	GORGON_ANIM_PAIN2,
	GORGON_ANIM_PAIN3,
	GORGON_ANIM_DIE1,
	GORGON_ANIM_DIE2,
	GORGON_ANIM_SNATCH,//30
	GORGON_ANIM_CATCH,
	GORGON_ANIM_MISS,
	GORGON_ANIM_READY_CATCH,
	GORGON_ANIM_SNATCHHI,
	GORGON_ANIM_SNATCHLOW,
	GORGON_ANIM_SLIP,
	GORGON_ANIM_SLIP_PAIN,
	GORGON_ANIM_DELAY,
	GORGON_ANIM_ROAR,
	GORGON_ANIM_ROAR2,//40
	GORGON_ANIM_LAND2,
	GORGON_ANIM_LAND,
	GORGON_ANIM_INAIR,

	GORGON_ANIM_TO_SWIM,
	GORGON_ANIM_SWIM,
	GORGON_ANIM_SWIM_BITE_A,
	GORGON_ANIM_SWIM_BITE_B,
	GORGON_ANIM_OUT_WATER,

	GORGON_ANIM_EAT_DOWN,
	GORGON_ANIM_EAT_UP,//50
	GORGON_ANIM_EAT_LOOP,//51
	GORGON_ANIM_EAT_TEAR,//52
	GORGON_ANIM_EAT_PULLBACK,//53
	GORGON_ANIM_LOOK_AROUND,//54
	GORGON_ANIM_EAT_LEFT,//55
	GORGON_ANIM_EAT_RIGHT,//56
	GORGON_ANIM_EAT_SNAP,//57
	GORGON_ANIM_EAT_REACT,

	GORGON_NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_GURGLE,
	SND_DIE,
	SND_GIB,
	SND_MELEEHIT1,
	SND_MELEEHIT2,
	SND_MELEEMISS1,
	SND_MELEEMISS2,
	SND_STEP1,
	SND_STEP2,
	SND_STEP3,
	SND_STEP4,
	SND_GROWL1,
	SND_GROWL2,
	SND_GROWL3,
	SND_LAND,
	NUM_SOUNDS
} SoundID_t;

extern mmove_t gorgon_move_stand1;
extern mmove_t gorgon_move_stand2;
extern mmove_t gorgon_move_stand3;
extern mmove_t gorgon_move_stand4;
extern mmove_t gorgon_move_walk;
extern mmove_t gorgon_move_walk2;
extern mmove_t gorgon_move_walk3;
extern mmove_t gorgon_move_melee1;
extern mmove_t gorgon_move_melee2;
extern mmove_t gorgon_move_melee3;
extern mmove_t gorgon_move_melee4;
extern mmove_t gorgon_move_melee5;
extern mmove_t gorgon_move_melee6;
extern mmove_t gorgon_move_melee7;
extern mmove_t gorgon_move_melee8;
extern mmove_t gorgon_move_melee9;
extern mmove_t gorgon_move_melee10;
extern mmove_t gorgon_move_fjump;
extern mmove_t gorgon_move_run1;
extern mmove_t gorgon_move_run2;
extern mmove_t gorgon_move_run3;
extern mmove_t gorgon_move_pain1;
extern mmove_t gorgon_move_pain2;
extern mmove_t gorgon_move_pain3;
extern mmove_t gorgon_move_die1;
extern mmove_t gorgon_move_die2;
extern mmove_t gorgon_move_jump;
extern mmove_t gorgon_move_catch;
extern mmove_t gorgon_move_snatch;
extern mmove_t gorgon_move_miss;
extern mmove_t gorgon_move_readycatch;
extern mmove_t gorgon_move_snatchhi;
extern mmove_t gorgon_move_snatchlow;
extern mmove_t gorgon_move_slip;
extern mmove_t gorgon_move_slip_pain;
extern mmove_t gorgon_move_delay;
extern mmove_t gorgon_move_roar;
extern mmove_t gorgon_move_roar2;
extern mmove_t gorgon_move_land2;
extern mmove_t gorgon_move_land;
extern mmove_t gorgon_move_inair;

extern mmove_t gorgon_move_to_swim;
extern mmove_t gorgon_move_swim;
extern mmove_t gorgon_move_swim_bite_a;
extern mmove_t gorgon_move_swim_bite_a;
extern mmove_t gorgon_move_outwater;

extern mmove_t gorgon_move_eat_down;
extern mmove_t gorgon_move_eat_up;
extern mmove_t gorgon_move_eat_loop;
extern mmove_t gorgon_move_eat_tear;
extern mmove_t gorgon_move_eat_pullback;
extern mmove_t gorgon_move_look_around;
extern mmove_t gorgon_move_eat_left;
extern mmove_t gorgon_move_eat_right;
extern mmove_t gorgon_move_eat_snap;
extern mmove_t gorgon_move_eat_react;


void gorgon_eat(edict_t *self, G_Message_t *msg);
void gorgon_stand(edict_t *self, G_Message_t *msg);
void gorgon_walk(edict_t *self, G_Message_t *msg);
void gorgon_melee(edict_t *self, G_Message_t *msg);
void gorgon_run(edict_t *self, G_Message_t *msg);
void gorgon_pain(edict_t *self, G_Message_t *msg);
void gorgon_death(edict_t *self, G_Message_t *msg);
void gorgonbite (edict_t *self);
void gorgon_footstep (edict_t *self);
void gorgon_eatorder (edict_t *self);
void gorgon_dead(edict_t *self);
void gorgon_hop (edict_t *self);
void gorgon_growl (edict_t *self);
void gorgon_jump (edict_t *self);
void gorgon_ready_catch (edict_t *self);
void gorgon_throw_toy(edict_t *self);
void gorgon_toy_ofs(edict_t *self, float ofsf, float ofsr, float ofsu);
void gorgon_check_snatch(edict_t *self, float ofsf, float ofsr, float ofsu);
void gorgon_gore_toy(edict_t *self, float jumpht);
void gorgon_miss_sound (edict_t *self);
void gorgon_anger_sound (edict_t *self);
void gorgon_go_snatch (edict_t *self);
void gorgon_done_gore (edict_t *self);
void gorgon_blocked (edict_t *self, trace_t *trace);
void gorgonRoll (edict_t *self, float rollangle);
void gorgonLerpOff (edict_t *self);
void gorgonLerpOn (edict_t *self);
void gorgonCheckSlip (edict_t *self);
void gorgonSlide (edict_t *self, float force);
qboolean gorgonCheckMood (edict_t *self);
void gorgon_mood(edict_t *self);
void ai_goal_charge (edict_t *self, float dist);
void gorgonApplyJump (edict_t *self);
void gorgonRoar (edict_t *self);
void gorgon_roar_sound (edict_t *self);
void gorgon_go_inair (edict_t *self);
void gorgon_check_landed (edict_t *self);
void gorgonJumpOutWater (edict_t *self);
void gorgonGoSwim (edict_t *self);
void gorgonCheckInWater (edict_t *self);
void gorgon_ai_swim (edict_t *self, float dist);
void gorgonForward (edict_t *self, float dist);
void gorgonFixPitch (edict_t *self);
void gorgonZeroPitch (edict_t *self);
void gorgon_death2twitch (edict_t *self);
void gorgonChooseDeath (edict_t *self);
void gorgon_ai_eat(edict_t *self, float crap);

float MG_ChangePitchForZVel(edict_t *self, float speed, float cap_vel, float max_angle);
void MG_SetNormalizeVelToGoal(edict_t *self, vec3_t vec);
float MG_ChangeYaw (edict_t *self);
void MG_Pathfind(edict_t *self, qboolean check_clear_path);
float MG_ChangePitch(edict_t *self, float ideal, float speed);
void fish_under_water_wake (edict_t *self);
