//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef _M_SPREADERMIST_H_
#define _M_SPREADERMIST_H_

#include "../../header/g_volume_effect.h"

void spreader_grenade_die(edict_t *self);
void spreader_mist_fast(edict_t *self, float x, float y, float z);
void spreader_mist(edict_t *self, float x, float y, float z);
void spreader_toss_grenade(edict_t *self);
void mist_damage_think(edict_t *victim, volume_effect_t *vol);

#endif
