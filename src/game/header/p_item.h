//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef P_ITEM
#define P_ITEM 1

#include "local.h"
#include "../player/library/p_types.h"

void Use_Defence(playerinfo_t *playerinfo, gitem_t *defence);
void DefenceThink_Powerup(edict_t *Caster, char *Format, ...);
void DefenceThink_RingOfRepulsion(edict_t *Caster, char *Format, ...);
void DefenceThink_MeteorBarrier(edict_t *Caster, char *Format, ...);
void DefenceThink_Teleport(edict_t *Caster, char *Format, ...);
void DefenceThink_Morph(edict_t *Caster, char *Format, ...);
void DefenceThink_Shield(edict_t *Caster, char *Format, ...);
void DefenceThink_Tornado(edict_t *Caster, char *Format, ...);

#endif
