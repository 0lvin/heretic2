//
// Heretic II
// Copyright 1998 Raven Software
//
//Elf Lord header

typedef enum AnimID_e
{
	ELFLORD_ANIM_HOVER,				//hover
	ELFLORD_ANIM_FLOAT_FORWARD,		//float forward
	ELFLORD_ANIM_CHARGE,			//charge forward
	ELFLORD_ANIM_CHARGE_BTRANS,		//transition to charge
	ELFLORD_ANIM_FLOAT_BACK,		//float backwards (w / attack)
	ELFLORD_ANIM_DODGE_RIGHT,		//dodge right
	ELFLORD_ANIM_DODGE_LEFT,		//dodge	left
	ELFLORD_ANIM_ATTACK_SOA_BTRANS,	//attack 1 (lightning sphere) beginning
	ELFLORD_ANIM_ATTACK_SOA_LOOP,	//attack 1 (lightning sphere) loop
	ELFLORD_ANIM_ATTACK_SOA_END,	//attack 1 (lightning sphere) ending
	ELFLORD_ANIM_ATTACK_LS,			//attack 2 (light surge)
	ELFLORD_ANIM_PAIN1,				//pain
	ELFLORD_ANIM_DIE_BTRANS,		//death beginning
	ELFLORD_ANIM_DIE_LOOP,			//death loop
	ELFLORD_ANIM_SHIELD,			//shield
	ELFLORD_ANIM_ATTACK,
	ELFLORD_ANIM_MOVE,
	ELFLORD_ANIM_WAIT,
	ELFLORD_ANIM_COME_TO_LIFE,
	ELFLORD_NUM_ANIMS
} AnimID_t;


typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_DIE,

	SND_SACHARGE,
	SND_SAFIRE,
	SND_SAHIT,

	SND_PROJ1,
	SND_BEAM,

	NUM_SOUNDS
} SoundID_t;

extern mmove_t elflord_move_idle;
extern mmove_t elflord_move_run;
extern mmove_t elflord_move_charge;
extern mmove_t elflord_move_charge_trans;
extern mmove_t elflord_move_floatback;
extern mmove_t elflord_move_dodgeright;
extern mmove_t elflord_move_dodgeleft;
extern mmove_t elflord_move_soa_begin;
extern mmove_t elflord_move_soa_loop;
extern mmove_t elflord_move_soa_end;
extern mmove_t elflord_move_ls;
extern mmove_t elflord_move_pain;
extern mmove_t elflord_move_death_btrans;
extern mmove_t elflord_move_death_loop;
extern mmove_t elflord_move_shield;
extern mmove_t elflord_move_attack;
extern mmove_t elflord_move_move;
extern mmove_t elflord_move_wait;
extern mmove_t elflord_move_come_to_life;

qboolean elfLordCheckAttack (edict_t *self);
void elflord_Attack(edict_t *self);

void elflord_decell(edict_t *self, float value);
void elflord_decide_movement (edict_t *self);
void elflord_ai_stand (edict_t *self, float dist);
void elflord_stand(edict_t *self, G_Message_t *msg);
void elflord_run(edict_t *self, G_Message_t *msg);
void elflord_death_start(edict_t *self, G_Message_t *msg);
void elflord_soa_start(edict_t *self, G_Message_t *msg);
void elflordRandomRushSound(edict_t *self);
void elflordSound(edict_t *self, float channel, float sndindex, float atten);
void elflord_flymove (edict_t *self, float dist);
void elfLordPause(edict_t *self);
void elfLordGoCharge(edict_t *self);
void elflord_soa_loop(edict_t *self);
void elflord_soa_end(edict_t *self);

void elflord_StartBeam(edict_t *self);
void elflord_EndBeam(edict_t *self);
void elford_Attack( edict_t *self );

void elflord_face(edict_t *self);
void elflord_track(edict_t *self);
void elflord_SlideMeter( edict_t *self );
void elflord_soa_go(edict_t *self);
void elflord_soa_charge(edict_t *self);
void elflord_FixAngles(edict_t *self);

void ai_charge2(edict_t *self, float dist);
float ai_face_goal(edict_t *self);
void MG_CheckEvade (edict_t *self);
