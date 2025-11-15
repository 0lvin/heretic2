//==============================================================================
//
// m_bee.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================

#include "../../header/local.h"
#include "../../header/g_defaultmessagehandler.h"
#include "../../header/g_monster.h"
#include "../../common/h2rand.h"
#include "../../header/g_hitlocation.h"
#include "../stats/stats.h"

static ClassResourceInfo_t resInfo;

typedef enum SoundID_e
{
	SND_BUZZ1,
	SND_BUZZ2,
	SND_STING,
	SND_GIB,
	NUM_SOUNDS
} SoundID_t;

static int sounds[NUM_SOUNDS];
/*
==========================================================

	Bee Spawn functions

==========================================================
*/

void BeeStaticsInit()
{
	resInfo.modelIndex = gi.modelindex("models/monsters/bee/tris.fm");

	sounds[SND_BUZZ1] = gi.soundindex("monsters/bee/buzz1.wav");
	sounds[SND_BUZZ2] = gi.soundindex("monsters/bee/buzz2.wav");
	sounds[SND_STING] = gi.soundindex("monsters/bee/sting.wav");
	sounds[SND_GIB] = gi.soundindex("monsters/bee/gib.wav");

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

}

/*QUAKED monster_bee (1 .5 0) (-16 -16 -24) (16 16 16)
The bee
*/
void SP_monster_bee(edict_t *self)
{
	ObjectInit(self, MAT_WOOD);
}
