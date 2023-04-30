//
// g_misc.h
//
// Heretic II
// Copyright 1998 Raven Software
//

#ifndef	G_MISC_H
#define G_MISC_H

#include "../src/common/header/common.h"

#ifdef __cplusplus
extern "C" {
#endif

void BecomeDebris(edict_t *Self);
void SprayDebris(edict_t *self, vec3_t spot, byte NoOfChunks, float damage);
void ThrowBodyPart(edict_t *self, vec3_t *spot, int BodyPart, float damage, int frame);
void ThrowWeapon(edict_t *self, vec3_t *spot, int BodyPart, float damage, int frame);
void DefaultObjectDieHandler(edict_t *self, struct G_Message_s *msg);
void BboxYawAndScale(edict_t *self);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif
