//==============================================================================
//
// p_chicken.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//
//	AI :
//
//	STAND1		: Looking straight ahead
//
//  Ported from m_chicken.c by Marcus Whitlock
//
//==============================================================================

#include "../../header/g_teleport.h"
#include "../../header/g_skeletons.h"
#include "../../header/local.h"
#include "p_anims.h"
#include "p_chicken.h"
#include "../../common/fx.h"
#include "../../common/h2rand.h"
#include "../../header/utilities.h"
#include "p_main.h"

#define CHICKEN_GLIDE			150
#define CHICKEN_GLIDE_FORWARD	200

void ChickenStepSound(playerinfo_t *playerinfo, float value)
{
	char *name;

	if (playerinfo->self->flags & FL_SUPER_CHICKEN)
	{
		name = (irand(0,1)) ? "monsters/tbeast/step1.wav" : "monsters/tbeast/step2.wav";

		pi.G_Sound(playerinfo->self,
							CHAN_WEAPON,
							pi.G_SoundIndex(name),
							1.0,
							ATTN_NORM,
							0);
	}
}

void ChickenAssert(playerinfo_t *playerinfo)
{
	//This should never be called, if it is, a sequence has been selected that cannot be addressed by the chicken
	PlayerAnimSetLowerSeq(playerinfo, ASEQ_STAND);
	PlayerAnimSetUpperSeq(playerinfo, ASEQ_NONE);

	assert(0);
}

// ***********************************************************************************************
// PlayerChickenBite
// -----------------
// ************************************************************************************************

void PlayerChickenBite(playerinfo_t *playerinfo)
{
	pi.G_PlayerActionChickenBite(playerinfo);
}

// ***********************************************************************************************
// PlayerChickenSqueal
// -------------------
// ************************************************************************************************

void PlayerChickenSqueal(playerinfo_t *playerinfo)
{
	pi.G_Sound(playerinfo->self,
						CHAN_WEAPON,
						pi.G_SoundIndex(""),
						1.0,
						ATTN_NORM,
						0);
}

// ***********************************************************************************************
// PlayerChickenNoise
// -------------------
// ************************************************************************************************

void PlayerChickenCluck(playerinfo_t *playerinfo, float force)
{
	char *soundname;

	assert(playerinfo);

	if ( (!force) && (irand(0,10)) )
		return;

	if (playerinfo->self->flags & FL_SUPER_CHICKEN)
		soundname = (irand(0,1)) ? "monsters/superchicken/cluck1.wav" : "monsters/superchicken/cluck2.wav";
	else
		soundname = (irand(0,1)) ? "monsters/chicken/cluck1.wav" : "monsters/chicken/cluck2.wav";

	pi.G_Sound(playerinfo->self, CHAN_WEAPON, pi.G_SoundIndex(soundname), 1.0, ATTN_NORM, 0);
}

// ***********************************************************************************************
// PlayerChickenJump
// -----------------
// ************************************************************************************************

int PlayerChickenJump(playerinfo_t *playerinfo)
{
	trace_t		trace;
	vec3_t		endpos;
	char		*soundname;
	int			id;

	VectorCopy(playerinfo->origin, endpos);
	endpos[2] += (playerinfo->mins[2] - 2.0);

	trace = pi.G_Trace(playerinfo->origin,
							  playerinfo->mins,
							  playerinfo->maxs,
							  endpos,
							  playerinfo->self,
							  MASK_PLAYERSOLID);

	if ((playerinfo->groundentity || trace.fraction < 0.2) && playerinfo->waterlevel<2)
		playerinfo->upvel=200;

	PlayerAnimSetLowerSeq(playerinfo,ASEQ_FALL);

	id = irand(0,6);

	if (playerinfo->self->flags & FL_SUPER_CHICKEN)
	{
		switch ( id )
		{
		case 0:
			soundname = "monsters/superchicken/jump1.wav";
			break;

		case 1:
			soundname = "monsters/superchicken/jump2.wav";
			break;

		case 2:
			soundname = "monsters/superchicken/jump3.wav";
			break;

		default:
			return ASEQ_FALL;
			break;
		}
	}
	else
	{
		switch ( id )
		{
		case 0:
			soundname = "monsters/chicken/jump1.wav";
			break;

		case 1:
			soundname = "monsters/chicken/jump2.wav";
			break;

		case 2:
			soundname = "monsters/chicken/jump3.wav";
			break;

		default:
			return ASEQ_FALL;
			break;
		}
	}

	pi.G_Sound(playerinfo->self, CHAN_WEAPON, pi.G_SoundIndex(soundname), 1.0, ATTN_NORM, 0);

	return ASEQ_FALL;
}

void PlayerChickenCheckFlap (playerinfo_t *playerinfo)
{
	if (playerinfo->seqcmd[ACMDL_JUMP])
	{
		edict_t *self;
		vec3_t	vf;

		self = playerinfo->self;

		playerinfo->flags |= PLAYER_FLAG_USE_ENT_POS;

		AngleVectors(self->s.angles, vf, NULL, NULL);
		vf[2] = 0;

		VectorScale(vf, CHICKEN_GLIDE_FORWARD, self->velocity);

		self->velocity[2] += CHICKEN_GLIDE;

		pi.G_CreateEffect(self,
								   FX_CHICKEN_EXPLODE,
								   CEF_OWNERS_ORIGIN | CEF_FLAG6,
								   NULL,
								   "");

		PlayerAnimSetLowerSeq(playerinfo, ASEQ_JUMPFWD);
	}
}

void
PlayerChickenFlap(playerinfo_t *playerinfo)
{
	edict_t *self;
	vec3_t	vf;

	self = playerinfo->self;
	playerinfo->flags |= PLAYER_FLAG_USE_ENT_POS;

	AngleVectors(self->s.angles, vf, NULL, NULL);
	vf[2] = 0;

	VectorScale(vf, CHICKEN_GLIDE_FORWARD, self->velocity);

	self->velocity[2] += CHICKEN_GLIDE;

	pi.G_CreateEffect(self, // jmarshall: believe this is right.
							   FX_CHICKEN_EXPLODE,
							   CEF_OWNERS_ORIGIN | CEF_FLAG6,
							   NULL,
							   "");
}
