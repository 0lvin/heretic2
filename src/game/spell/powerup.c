//
// spl_powerup.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "../header/local.h"
#include "../header/g_itemstats.h"


void
SpellCastPowerup(edict_t *caster, vec3_t StartPos, vec3_t AimAngles, vec3_t AimDir, float value)
{
	assert(caster->client);

	// Kill any tomes that may already be out there for this player.

	G_RemoveEffects(caster, FX_TOME_OF_POWER);

	// if we are a chicken, lets make us a player again
	if (caster->flags & FL_CHICKEN)
	{
		caster->morph_timer = level.time - 0.1;
	}
	else
	{
		// add some time in on the timer for the powerup
		caster->client->playerinfo.powerup_timer = level.time + POWERUP_DURATION;

		// turn on the light at the client effect end through client flags that are passed down
		caster->s.effects |= EF_POWERUP_ENABLED;
		caster->client->playerinfo.effects |= EF_POWERUP_ENABLED;

		// create the tome of power
		gi.CreateEffect(caster, FX_TOME_OF_POWER, CEF_OWNERS_ORIGIN, NULL, "");
	}

	// start up the shrine powerup effect
//	gi.CreateEffect(caster, FX_SHRINE_POWERUP, CEF_OWNERS_ORIGIN, NULL, "");

	// do the SHRINE sound
	gi.sound(caster, CHAN_ITEM, gi.soundindex("items/shrine5.wav"), 1, ATTN_NORM, 0);
}

// end
