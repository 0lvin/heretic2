//
// fx_quake.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "../src/common/header/common.h"
#include "client_effects.h"
#include "client_entities.h"
#include "../qcommon/fx.h"



void FXQuake(centity_t *owner, int type, int flags, vec3_t origin)
{
	byte count,time,dir;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_QUAKE].formatString, &count,&time,&dir);

	fxi.Activate_Screen_Shake(count,(time * 100), fxi.cl->time, dir);

}
