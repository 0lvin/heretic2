//
// p_animactor.h
//
// Heretic II
// Copyright 1998 Raven Software
//

#ifndef _P_ANIMACTOR_H_
#define _P_ANIMACTOR_H_

#include "p_types.h"

extern void TurnOffPlayerEffects(playerinfo_t *playerinfo);
extern void AnimUpdateFrame(playerinfo_t *playerinfo);
extern void PlayerFallingDamage(playerinfo_t *playerinfo);

#endif // _P_ANIMACTOR_H_
