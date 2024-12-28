//
// Heretic II
// Copyright 1998 Raven Software
//
#include "header/local.h"
#include "header/g_skeletons.h"
#include "common/arrayed_list.h"
#include "header/g_physics.h"

void
ConstructEntities(void)
{
	edict_t *ent;
	int		i;

	gi.dprintf("%s: Init started %d\n", __func__, game.entitiesSpawned);
	if (game.entitiesSpawned)
	{
		return;
	}

	// Create message queues for entites.
	for(i = 0, ent = g_edicts; i < maxentities->value ; i++, ent++)
	{
		SLList_DefaultCon(&ent->msgQ.msgs);

		ent->s.skeletalType = SKEL_NULL;
	}

	// Allocate skeletons for clients only.

	for (i=0 ; i<game.maxclients ; i++)
	{
		edict_t *ent;
		ent = &globals.edicts[1+i];
		ent->s.skeletalType = SKEL_CORVUS;
		ent->s.rootJoint = CreateSkeleton(ent->s.skeletalType);
	}

	game.entitiesSpawned = true;
	gi.dprintf("%s: Inited\n", __func__);
}

// end
