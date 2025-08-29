//==============================================================================
//
// m_elflord.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================
#include "../../header/local.h"
#include "../../header/g_defaultmessagehandler.h"
#include "../../header/g_monster.h"
#include "../../common/angles.h"
#include "../../common/h2rand.h"
#include "../../common/matrix.h"

#include "elflord_anim.h"
#include "elflord.h"
#include "../ai.h"

/*----------------------------------------------------------------------
  Elf Lord Base Info
-----------------------------------------------------------------------*/
static mmove_t *animations[ NUM_ANIMS] =
{
	&Elflord_move_c_action1,
	&Elflord_move_c_action2,
	&Elflord_move_c_death1,
	&Elflord_move_c_death2,
	&Elflord_move_c_idle1,
	&Elflord_move_c_idle2,
	NULL
};

static ClassResourceInfo_t resInfo;

float mist_yaw;
#define MIST_ADD 35

void Elflord_c_gib(edict_t *self, G_Message_t *msg)
{
	gi.CreateEffect(self, FX_WEAPON_SPHEREEXPLODE, CEF_OWNERS_ORIGIN, NULL,
					"db", self->movedir, (byte)(AVG_VEC3T(self->rrs.scale) * 7.5));

	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/SphereImpact.wav"), 1, ATTN_NORM, 0);

	self->think = BecomeDebris;
	self->nextthink = level.time + 0.1;
}

void Elflord_c_boom(edict_t *self)
{
	gi.CreateEffect(NULL, FX_WEAPON_FLYINGFISTEXPLODE, 0, self->s.origin, "d", self->movedir);
}

void Elflord_c_throwhead(edict_t *self)
{
	vec3_t		gore_spot;

	mist_yaw = 0;

	VectorClear(gore_spot);
	ThrowBodyPart(self, gore_spot, 8, 15, FRAME_idle1);
	gi.sound(self, CHAN_BODY, gi.soundindex("monsters/elflord/death1.wav"), 1, ATTN_NORM, 0);
}

/*-------------------------------------------------------------------------
	elflord_mist
-------------------------------------------------------------------------*/
void elflord_mist(edict_t *self, float x, float y, float z)
{
	vec3_t		offset;
	vec3_t		rotoffset;
	vec3_t		normalized;
	vec3_t		velocity;
	float		yaw;
	matrix3_t	mat;

	if (self->monsterinfo.aiflags & AI_NO_MELEE)
		return;//fixme: actually prevent these anims

	mist_yaw = mist_yaw + MIST_ADD;
	yaw = anglemod(mist_yaw);

	// Converts degrees to radians for use with trig and matrix functions
	yaw = yaw * ANGLE_TO_RAD;

	// Sets offset presuming yaw of zero
	VectorSet(offset, 0, 0, 0);

	// Creates a rotation matrix to rotate the point about the z axis
	CreateYawMatrix(mat, yaw);

	// Rotates point about local z axis
	Matrix3MultByVec3(mat, offset, rotoffset);

	// Get normalized offset
	VectorCopy(rotoffset, normalized);
	normalized[2] = 0.0F;
	VectorNormalize(normalized);

	// Add offset to owners origin
	Vec3AddAssign(self->s.origin, rotoffset);

	// Get direction vector scaled by speed
	VectorSet(velocity, cos(yaw) * 200.0F, sin(yaw) * 200.0F, -100);

	gi.CreateEffect(NULL, FX_PLAGUEMIST, 0, rotoffset, "vb", velocity, 2050 / 35);

}



/*-------------------------------------------------------------------------
	Elflord_c_anims
-------------------------------------------------------------------------*/
void Elflord_c_anims(edict_t *self, G_Message_t *msg)
{
	int int_msg;
	int curr_anim;

	ai_c_readmessage(self, msg);
	int_msg = (int) msg->ID;

	self->monsterinfo.c_anim_flag = 0;

	switch(int_msg)
	{
		case MSG_C_ACTION1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION1;
			break;
		case MSG_C_ACTION2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION2;
			break;
		case MSG_C_DEATH1:
			self->s.fmnodeinfo[MESH__LIGHT25].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HEAD25].flags |= FMNI_NO_DRAW;
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_DEATH1;
			break;
		case MSG_C_DEATH2:
			self->s.fmnodeinfo[MESH__LIGHT25].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HEAD25].flags |= FMNI_NO_DRAW;
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_DEATH2;
			break;
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;
		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE2;
			break;
		default:
			return;
	}

	self->rrs.mesh = GenNoDrawInfo(self->s.fmnodeinfo);

	SetAnim(self, curr_anim);
}

/*-------------------------------------------------------------------------
	ElfLordStaticsCinInit
-------------------------------------------------------------------------*/
void ElflordCinStaticsInit()
{
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_ACTION1] = Elflord_c_anims;
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_ACTION2] = Elflord_c_anims;
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_DEATH1] = Elflord_c_anims;
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_DEATH2] = Elflord_c_anims;
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_IDLE1] = Elflord_c_anims;
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_IDLE2] = Elflord_c_anims;
	classStatics[CID_C_ELFLORD].msgReceivers[MSG_C_GIB1] = Elflord_c_gib;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/elflord/cinematic/tris.fm");

	classStatics[CID_C_ELFLORD].resInfo = &resInfo;

}

/*QUAKED character_elflord (1 .5 0) (-24 -24 -78) (24 24 16) INVISIBLE
The Celestial Watcher who whispers when he talks
*/
void SP_character_elflord (edict_t *self)
{
	VectorSet(self->mins, -24, -24, -78);
	VectorSet(self->maxs, 24, 24, 16);

	c_character_init(self,CID_C_ELFLORD);

	self->monsterinfo.scale = 2.0;
	VectorSet(self->rrs.scale,
		self->monsterinfo.scale,
		self->monsterinfo.scale,
		self->monsterinfo.scale);
}
