//
// Heretic II
// Copyright 1998 Raven Software
//
#include "../../header/local.h"
#include "../../common/h2rand.h"

static void
MakeSolidObject(edict_t *ent, char *Model, float MinX, float MinY, float MinZ,
					 float MaxX, float MaxY, float MaxZ)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet(ent->mins, MinX, MinY, MinZ);
	VectorSet(ent->maxs, MaxX, MaxY, MaxZ);
	gi.setmodel(ent, Model);

	gi.linkentity(ent);
}

static void
flag_think (edict_t *self)
{
	self->s.frame++;
	if (self->s.frame > 10)
		self->s.frame = 0;

	self->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_flag (1 .5 0) (-10 -10 0) (10 10 80)
*/
void SP_misc_flag (edict_t *ent)
{
	MakeSolidObject(ent, "models/rj5/tris.fm", -10, -10, 0, 10, 10, 80);

	ent->think = flag_think;
	ent->nextthink = level.time + flrand(0.0F, 1.0F);
}
