/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (c) ZeniMax Media Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * Item handling and item definitions.
 *
 * =======================================================================
 */

#include "header/local.h"
#include "header/g_items.h"
#include "header/g_itemstats.h"
#include "header/g_weapon.h"
#include "player/library/p_anims.h"
#include "player/library/p_anim_data.h"
#include "player/library/p_items.h"
#include "common/h2rand.h"
#include "common/cl_strings.h"

#define ITEM_COOP_ONLY		1
#define ITEM_NO_DROP		2

#define HEALTH_IGNORE_MAX 1
#define HEALTH_TIMED 2

gitem_armor_t jacketarmor_info = {25, 50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info = {50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info = {100, 200, .80, .60, ARMOR_BODY};

static int jacket_armor_index;
static int combat_armor_index;
static int body_armor_index;
static int power_screen_index;
static int power_shield_index;

static int quad_drop_timeout_hack;
static int quad_fire_drop_timeout_hack;

/* ====================================================================== */

gitem_t *
GetItemByIndex(int index)
{
	if ((index == 0) || (index >= game.num_items))
	{
		return NULL;
	}

	return &itemlist[index];
}

gitem_t *
FindItemByClassname(const char *classname)
{
	int i;
	gitem_t *it;

	if (!classname)
	{
		return NULL;
	}

	it = itemlist;

	for (i = 0; i < game.num_items; i++, it++)
	{
		if (!it->classname)
		{
			continue;
		}

		if (!Q_stricmp(it->classname, classname))
		{
			return it;
		}
	}

	return NULL;
}

gitem_t *
FindItem(const char *pickup_name)
{
	int i;
	gitem_t *it;

	if (!pickup_name)
	{
		return NULL;
	}

	it = itemlist;

	for (i = 0; i < game.num_items; i++, it++)
	{
		if (!it->pickup_name)
		{
			continue;
		}

		if (!Q_stricmp(it->pickup_name, pickup_name))
		{
			return it;
		}
	}

	return NULL;
}

// ************************************************************************************************
// RespawnedThink
// --------------
// ************************************************************************************************

void
RespawnedThink(edict_t *ent)
{
	ent->think = NULL;
}

/* ====================================================================== */

void
DoRespawn(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	if (ent->team)
	{
		edict_t *master;

		master = ent->teammaster;

		/* in ctf, when we are weapons stay, only the master
		   of a team of weapons is spawned */
		if (ctf->value &&
			((int)dmflags->value & DF_WEAPONS_STAY) &&
			master->item && (master->item->flags & IT_WEAPON))
		{
			ent = master;
		}
		else
		{
			int count, choice;

			for (count = 0, ent = master; ent; ent = ent->chain, count++)
			{
			}

			choice = count ? randk() % count : 0;

			for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
			{
			}
		}
	}

	if (randomrespawn && randomrespawn->value)
	{
		edict_t *newEnt;

		newEnt = DoRandomRespawn(ent);

		/* if we've changed entities, then do some sleight
		 * of hand. otherwise, the old entity will respawn */
		if (newEnt)
		{
			G_FreeEdict(ent);
			ent = newEnt;
		}
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	gi.linkentity(ent);

	// Create a respawn client-effect (this isn't currenlty doing anything on the client).

	//gi.CreateEffect(ent,FX_ITEM_RESPAWN,CEF_OWNERS_ORIGIN,ent->s.origin,NULL);

	// So it'll get sent to the client again.

	// Re-enable the persistent effect.

	ent->s.effects &= ~EF_DISABLE_ALL_CFX;

	// So it'll get displayed again.

	ent->s.effects |= EF_ALWAYS_ADD_EFFECTS;

	// And the rest.

	ent->think = RespawnedThink;
	ent->nextthink = level.time + FRAMETIME;
}

// ************************************************************************************************
// PreRespawnThink
// ---------------
// ************************************************************************************************

void
PreRespawnThink(edict_t *ent)
{
	int delay;
	float clients;

	// The equation for respawn:
	//		--The respawn times should be normal for 8 players.
	//		--For 32 players the respawn should be halved
	//		--For 2 players the respawn should be doubled.
	if (deathmatch->value)
	{
/*		// You know what?  I thought I was being clever here, but I just screwed up the eq and made the game not respawn enough.
		delay = ent->delay * sqrt((float)game.num_clients/8.0);		// This makes it a nice curve.  Clever, no?
		// Lemme see here:  sqrt(2/8) = sqrt(1/4) = 1/2
		//					sqrt(8/8) = sqrt(1) = 1
		//					sqrt(32/8) = sqrt(4) = 2
*/
		clients=(float)game.num_clients;
		if (clients<2.0)
			clients=2.0;
		delay = ent->delay * sqrt(2.0/clients);		// Spawn more frequently when more players.
		// Lemme see here:  sqrt(2/2) = sqrt(1) = 1
		//					sqrt(2/8) = sqrt(1/4) = 1/2
		//					sqrt(2/32) = sqrt(1/16) = 1/4
	}
	else
	{
		delay = ent->delay;
	}
	ent->nextthink = level.time + delay - FRAMETIME;
	ent->think = DoRespawn;
}

void
SetRespawn(edict_t *ent, float delay)
{
	if (!ent)
	{
		return;
	}

	// So it'll get sent to the client again.

	// Disables all the effects on the entity.

	ent->s.effects |= EF_DISABLE_ALL_CFX;

	// Take off the EF_ALWAYS_ADD_EFFECTS or EF_DISABLE_ALL_CFX wont have an effect.

	ent->s.effects &= ~EF_ALWAYS_ADD_EFFECTS;

	// And the rest.

	ent->solid = SOLID_NOT;

	if(deathmatch->value && game.num_clients > 8)
	{
		// No less than 1/4th the delay.

		ent->nextthink = level.time + (ent->delay) / 4;
	}
	if(deathmatch->value && game.num_clients > 2)
	{
		// So things respawn faster with a lot of players.

		ent->nextthink = level.time + (ent->delay) / (game.num_clients/2);
	}
	else
	{
		ent->nextthink = level.time + ent->delay;
	}

	ent->think = PreRespawnThink;
}

/* ====================================================================== */

qboolean
Pickup_Powerup(edict_t *ent, edict_t *other)
{
	int quantity;

	if (!ent || !other)
	{
		return false;
	}

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (((skill->value == SKILL_MEDIUM) &&
		 (quantity >= 2)) || ((skill->value >= SKILL_HARD) && (quantity >= 1)))
	{
		return false;
	}

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			SetRespawn(ent, ent->item->quantity);
		}

		if (((int)dmflags->value & DF_INSTANT_ITEMS) ||
			((ent->item->use == Use_Quad) &&
			 (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_Quad) &&
				(ent->spawnflags & DROPPED_PLAYER_ITEM))
			{
				quad_drop_timeout_hack =
					(ent->nextthink - level.time) / FRAMETIME;
			}

			ent->item->use(other, ent->item);
		}
	}

	return true;
}

qboolean
Pickup_General(edict_t *ent, edict_t *other)
{
	if (!ent || !other)
	{
		return false;
	}

	if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			SetRespawn(ent, ent->item->quantity);
		}
	}

	return true;
}

void
Drop_General(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	Drop_Item(ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);
}

/* ====================================================================== */

qboolean
Pickup_Adrenaline(edict_t *ent, edict_t *other)
{
	if (!ent || !other)
	{
		return false;
	}

	if (!deathmatch->value)
	{
		other->max_health += 1;
	}

	if (other->health < other->max_health)
	{
		other->health = other->max_health;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
	{
		SetRespawn(ent, ent->item->quantity);
	}

	return true;
}

qboolean
Pickup_AncientHead(edict_t *ent, edict_t *other)
{
	if (!ent || !other)
	{
		return false;
	}

	other->max_health += 2;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
	{
		SetRespawn(ent, ent->item->quantity);
	}

	return true;
}

qboolean
Pickup_Bandolier(edict_t *ent, edict_t *other)
{
	gitem_t *item;
	int index;

	if (!ent || !other)
	{
		return false;
	}

	if (other->client->pers.max_bullets < 250)
	{
		other->client->pers.max_bullets = 250;
	}

	if (other->client->pers.max_shells < 150)
	{
		other->client->pers.max_shells = 150;
	}

	if (other->client->pers.max_cells < 250)
	{
		other->client->pers.max_cells = 250;
	}

	if (other->client->pers.max_slugs < 75)
	{
		other->client->pers.max_slugs = 75;
	}

	if (other->client->pers.max_magslug < 75)
	{
		other->client->pers.max_magslug = 75;
	}

	if (other->client->pers.max_flechettes < 250)
	{
		other->client->pers.max_flechettes = 250;
	}

	if (g_disruptor->value)
	{
		if (other->client->pers.max_rounds < 150)
		{
			other->client->pers.max_rounds = 150;
		}
	}

	item = FindItem("Bullets");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_bullets)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_bullets;
		}
	}

	item = FindItem("Shells");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_shells)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_shells;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
	{
		SetRespawn(ent, ent->item->quantity);
	}

	return true;
}

qboolean
Pickup_Pack(edict_t *ent, edict_t *other)
{
	gitem_t *item;
	int index;

	if (!ent || !other)
	{
		return false;
	}

	if (other->client->pers.max_bullets < 300)
	{
		other->client->pers.max_bullets = 300;
	}

	if (other->client->pers.max_shells < 200)
	{
		other->client->pers.max_shells = 200;
	}

	if (other->client->pers.max_rockets < 100)
	{
		other->client->pers.max_rockets = 100;
	}

	if (other->client->pers.max_grenades < 100)
	{
		other->client->pers.max_grenades = 100;
	}

	if (other->client->pers.max_cells < 300)
	{
		other->client->pers.max_cells = 300;
	}

	if (other->client->pers.max_slugs < 100)
	{
		other->client->pers.max_slugs = 100;
	}

	if (other->client->pers.max_magslug < 100)
	{
		other->client->pers.max_magslug = 100;
	}

	if (other->client->pers.max_flechettes < 200)
	{
		other->client->pers.max_flechettes = 200;
	}

	if (g_disruptor->value)
	{
		if (other->client->pers.max_rounds < 200)
		{
			other->client->pers.max_rounds = 200;
		}
	}

	item = FindItem("Bullets");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_bullets)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_bullets;
		}
	}

	item = FindItem("Shells");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_shells)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_shells;
		}
	}

	item = FindItem("Cells");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_cells)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_cells;
		}
	}

	item = FindItem("Grenades");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_grenades)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_grenades;
		}
	}

	item = FindItem("Rockets");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_rockets)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_rockets;
		}
	}

	item = FindItem("Slugs");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_slugs)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_slugs;
		}
	}

	item = FindItem("Mag Slug");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_magslug)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_magslug;
		}
	}

	item = FindItem("Flechettes");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
			other->client->pers.max_flechettes)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_flechettes;
		}
	}

	item = FindItem("Rounds");

	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;

		if (other->client->pers.inventory[index] >
				other->client->pers.max_rounds)
		{
			other->client->pers.inventory[index] =
				other->client->pers.max_rounds;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
	{
		SetRespawn(ent, ent->item->quantity);
	}

	return true;
}

/* ====================================================================== */

qboolean
Pickup_Nuke(edict_t *ent, edict_t *other)
{
	int quantity;

	if (!ent || !other)
	{
		return false;
	}

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (quantity >= 1)
	{
		return false;
	}

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP))
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			SetRespawn(ent, ent->item->quantity);
		}
	}

	return true;
}

void
Use_IR(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->ir_framenum > level.framenum)
	{
		ent->client->ir_framenum += 600;
	}
	else
	{
		ent->client->ir_framenum = level.framenum + 600;
	}

	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/ir_start.wav"), 1, ATTN_NORM, 0);
}

void
Use_Double(edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->double_framenum > level.framenum)
	{
		ent->client->double_framenum += 300;
	}
	else
	{
		ent->client->double_framenum = level.framenum + 300;
	}

	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/ddamage1.wav"), 1, ATTN_NORM, 0);
}

void
Use_Compass(edict_t *ent, gitem_t *item)
{
	int ang;

	if (!ent || !item)
	{
		return;
	}

	ang = (int)(ent->client->v_angle[1]);

	if (ang < 0)
	{
		ang += 360;
	}

	gi.cprintf(ent, PRINT_HIGH, "Origin: %0.0f,%0.0f,%0.0f    Dir: %d\n",
			ent->s.origin[0], ent->s.origin[1], ent->s.origin[2], ang);
}

void
Use_Nuke(edict_t *ent, gitem_t *item)
{
	vec3_t forward, right, start;
	float speed;

	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	AngleVectors(ent->client->v_angle, forward, right, NULL);

	VectorCopy(ent->s.origin, start);
	speed = 100;
	fire_nuke(ent, start, forward, speed);
}

void
Use_Doppleganger(edict_t *ent, gitem_t *item)
{
	vec3_t forward, right;
	vec3_t createPt, spawnPt;
	vec3_t ang;

	if (!ent || !item)
	{
		return;
	}

	VectorClear(ang);
	ang[YAW] = ent->client->v_angle[YAW];
	AngleVectors(ang, forward, right, NULL);

	VectorMA(ent->s.origin, 48, forward, createPt);

	if (!FindSpawnPoint(createPt, ent->mins, ent->maxs, spawnPt, 32))
	{
		return;
	}

	if (!CheckGroundSpawnPoint(spawnPt, ent->mins, ent->maxs, 64, -1))
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	SpawnGrow_Spawn(spawnPt, 0);
	fire_doppleganger(ent, spawnPt, forward);
}

qboolean
Pickup_Doppleganger(edict_t *ent, edict_t *other)
{
	int quantity;

	if (!ent || !other)
	{
		return false;
	}

	if (!(deathmatch->value))
	{
		return false;
	}

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (quantity >= 1)
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (!(ent->spawnflags & DROPPED_ITEM))
	{
		SetRespawn(ent, ent->item->quantity);
	}

	return true;
}

qboolean
Pickup_Sphere(edict_t *ent, edict_t *other)
{
	int quantity = 0;

	if (!ent || !other || !other->client || other->client->owned_sphere)
	{
		return false;
	}

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (((skill->value == SKILL_MEDIUM) &&
		 (quantity >= 2)) || ((skill->value >= SKILL_HARD) && (quantity >= 1)))
	{
		return false;
	}

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			SetRespawn(ent, ent->item->quantity);
		}
	}

	return true;
}

void
Use_Defender(edict_t *ent, gitem_t *item)
{
	if (!ent || !item || !ent->client)
	{
		return;
	}

	if (ent->client->owned_sphere)
	{
		gi.cprintf(ent, PRINT_HIGH, "Only one sphere at a time!\n");
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	Defender_Launch(ent);
}

void
Use_Hunter(edict_t *ent, gitem_t *item)
{
	if (!ent || !item || !ent->client)
	{
		return;
	}

	if (ent->client->owned_sphere)
	{
		gi.cprintf(ent, PRINT_HIGH, "Only one sphere at a time!\n");
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	Hunter_Launch(ent);
}

void
Use_Vengeance(edict_t *ent, gitem_t *item)
{
	if (!ent || !item || !ent->client)
	{
		return;
	}

	if (ent->client->owned_sphere)
	{
		gi.cprintf(ent, PRINT_HIGH, "Only one sphere at a time!\n");
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	Vengeance_Launch(ent);
}

/* ====================================================================== */

void
Use_Quad(edict_t *ent, gitem_t *item)
{
	int timeout;

	if (!ent || !item || !ent->client)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (quad_drop_timeout_hack)
	{
		timeout = quad_drop_timeout_hack;
		quad_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->quad_framenum += timeout;
	}
	else
	{
		ent->client->quad_framenum = level.framenum + timeout;
	}

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM,
			0);
}

/* ===================================================================== */

void
Use_QuadFire(edict_t *ent, gitem_t *item)
{
	int timeout;

	if (!ent || !item || !ent->client)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (quad_fire_drop_timeout_hack)
	{
		timeout = quad_fire_drop_timeout_hack;
		quad_fire_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->client->quadfire_framenum > level.framenum)
	{
		ent->client->quadfire_framenum += timeout;
	}
	else
	{
		ent->client->quadfire_framenum = level.framenum + timeout;
	}

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/quadfire1.wav"), 1, ATTN_NORM, 0);
}

/* ====================================================================== */

void
Use_Breather(edict_t *ent, gitem_t *item)
{
	if (!ent || !item || !ent->client)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->breather_framenum += 300;
	}
	else
	{
		ent->client->breather_framenum = level.framenum + 300;
	}
}

/* ====================================================================== */

void
Use_Envirosuit(edict_t *ent, gitem_t *item)
{
	if (!ent || !item || !ent->client)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->enviro_framenum += 300;
	}
	else
	{
		ent->client->enviro_framenum = level.framenum + 300;
	}
}

/* ====================================================================== */

void
Use_Invulnerability(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->invincible_framenum += 300;
	}
	else
	{
		ent->client->invincible_framenum = level.framenum + 300;
	}

	gi.sound(ent, CHAN_ITEM, gi.soundindex(
					"items/protect.wav"), 1, ATTN_NORM, 0);
}

void
Use_Invisibility(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;

	if (ent->client->invisible_framenum > level.framenum)
	{
		ent->client->invisible_framenum += 300;
	}
	else
	{
		ent->client->invisible_framenum = level.framenum + 300;
	}

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}

/* ====================================================================== */

void
Use_Silencer(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);
	ent->client->silencer_shots += 30;
}

/* ====================================================================== */

qboolean
Pickup_Key(edict_t *ent, edict_t *other)
{
	if (!ent || !other)
	{
		return false;
	}

	if (coop->value)
	{
		if (strcmp(ent->classname, "key_power_cube") == 0)
		{
			if (other->client->pers.power_cubes &
				((ent->spawnflags & 0x0000ff00) >> 8))
			{
				return false;
			}

			other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
			other->client->pers.power_cubes |=
				((ent->spawnflags & 0x0000ff00) >> 8);
		}
		else
		{
			if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
			{
				return false;
			}

			other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
		}

		return true;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	return true;
}

// ************************************************************************************************
// Pickup_Puzzle
// -------------
// ************************************************************************************************

qboolean
Pickup_Puzzle(edict_t *ent, edict_t *other)
{
	gitem_t	*item;

	if (other->flags & FL_CHICKEN)
	{
		return false;
	}

	item = FindItemByClassname(ent->classname);

	if (!other->client->playerinfo.pers.inventory[ITEM_INDEX(ent->item)])
	{
		other->client->playerinfo.pers.inventory[ITEM_INDEX(ent->item)] = 1;

		G_CPrintf(other, PRINT_HIGH, ent->item->msg_pickup);

		return true;
	}
	else
	{
		return false;
	}
}

// ************************************************************************************************
// AddDefenseToInventory
// ---------------------
// ************************************************************************************************

qboolean AddDefenseToInventory(gitem_t *item,edict_t *player)
{
	if(!player->client->playerinfo.pers.inventory[ITEM_INDEX(item)])
	{
		player->client->playerinfo.pers.inventory[ITEM_INDEX(item)]=1;

		// Now decide if we want to swap defenses or not.

		if(player->client->playerinfo.pers.autoweapon )
		{
			item->use(player, item);
		}

		return true;
	}
	else
	{
		// We already have it...

		return false;
	}
}

// ************************************************************************************************
// Pickup_Defense
// --------------
// ************************************************************************************************

qboolean Pickup_Defense (edict_t *ent, edict_t *other)
{
	if (other->flags & FL_CHICKEN)
	{
		return false;
	}

	if(AddDefenseToInventory(ent->item,other))
	{
		G_CPrintf(other, PRINT_HIGH, ent->item->msg_pickup);

		return true;
	}
	else
	{
		return false;
	}
}

// ************************************************************************************************
// Add_AmmoToInventory
// -------------------
// ************************************************************************************************

qboolean Add_AmmoToInventory (edict_t *ent, gitem_t *item, int count,int max)
{
	int			index;

	index = ITEM_INDEX(item);

	if (ent->client->playerinfo.pers.inventory[index] == max)
		return false;

	ent->client->playerinfo.pers.inventory[index] += count;

	if (ent->client->playerinfo.pers.inventory[index] > max)
		ent->client->playerinfo.pers.inventory[index] = max;

	return true;
}

/* ====================================================================== */

qboolean
Add_Ammo(edict_t *ent, gitem_t *item, int count)
{
	int index;
	int max;

	if (!ent || !item || !ent->client)
	{
		return false;
	}

	if (item->tag == AMMO_BULLETS)
	{
		max = ent->client->pers.max_bullets;
	}
	else if (item->tag == AMMO_SHELLS)
	{
		max = ent->client->pers.max_shells;
	}
	else if (item->tag == AMMO_ROCKETS)
	{
		max = ent->client->pers.max_rockets;
	}
	else if (item->tag == AMMO_GRENADES)
	{
		max = ent->client->pers.max_grenades;
	}
	else if (item->tag == AMMO_CELLS)
	{
		max = ent->client->pers.max_cells;
	}
	else if (item->tag == AMMO_SLUGS)
	{
		max = ent->client->pers.max_slugs;
	}
	else if (item->tag == AMMO_MAGSLUG)
	{
		max = ent->client->pers.max_magslug;
	}
	else if (item->tag == AMMO_TRAP)
	{
		max = ent->client->pers.max_trap;
	}
	else if (item->tag == AMMO_FLECHETTES)
	{
		max = ent->client->pers.max_flechettes;
	}
	else if (item->tag == AMMO_PROX)
	{
		max = ent->client->pers.max_prox;
	}
	else if (item->tag == AMMO_TESLA)
	{
		max = ent->client->pers.max_tesla;
	}
	else if (item->tag == AMMO_DISRUPTOR)
	{
		max = ent->client->pers.max_rounds;
	}
	else if ((item->tag == AMMO_MANA_OFFENSIVE_HALF) || (item->tag == AMMO_MANA_OFFENSIVE_FULL))
	{
		item = FindItemByClassname("item_mana_offensive_half");
		max = ent->client->playerinfo.pers.max_offmana;
		return Add_AmmoToInventory(ent, item, count, max);
	}
	else if ((item->tag == AMMO_MANA_DEFENSIVE_HALF) || (item->tag == AMMO_MANA_DEFENSIVE_FULL))
	{
		item = FindItemByClassname("item_mana_defensive_half");
		max = ent->client->playerinfo.pers.max_defmana;
		return Add_AmmoToInventory(ent, item, count, max);
	}
	else if ((item->tag == AMMO_MANA_COMBO_QUARTER) || (item->tag == AMMO_MANA_COMBO_HALF))
	{
		qboolean bo;

		item = FindItemByClassname("item_mana_offensive_half");
		max = ent->client->playerinfo.pers.max_offmana;

		bo = Add_AmmoToInventory (ent,item,count,max);

		item = FindItemByClassname("item_mana_defensive_half");
		max = ent->client->playerinfo.pers.max_defmana;
		bo |= Add_AmmoToInventory(ent, item, count, max);

		return bo;
	}
	else if (item->tag == AMMO_REDRAIN)
	{
		max = ent->client->playerinfo.pers.max_redarrow;
		return Add_AmmoToInventory(ent, item, count, max);
	}
	else if (item->tag == AMMO_PHOENIX)
	{
		max = ent->client->playerinfo.pers.max_phoenarr;
		return Add_AmmoToInventory(ent, item, count, max);
	}
	else if (item->tag == AMMO_HELLSTAFF)
	{
		max = ent->client->playerinfo.pers.max_hellstaff;
		return Add_AmmoToInventory(ent, item, count, max);
	}
	else
	{
		gi.dprintf("undefined ammo type\n");
		return false;
	}

	index = ITEM_INDEX(item);

	if (ent->client->pers.inventory[index] == max)
	{
		return false;
	}

	ent->client->pers.inventory[index] += count;

	if (ent->client->pers.inventory[index] > max)
	{
		ent->client->pers.inventory[index] = max;
	}

	return true;
}

qboolean
Pickup_Ammo(edict_t *ent, edict_t *other)
{
	int count;
	qboolean weapon;

	if (!ent || !other)
	{
		return false;
	}

	if (other->flags & FL_CHICKEN)
	{
		return false;
	}

	weapon = (ent->item->flags & IT_WEAPON);

	if ((weapon) && ((int)dmflags->value & DF_INFINITE_AMMO))
	{
		count = 1000;
	}
	else if (ent->count)
	{
		count = ent->count;
	}
	else
	{
		count = ent->item->quantity;
	}

	if (!Add_Ammo(other, ent->item, count))
	{
		return false;
	}

	G_CPrintf(other, PRINT_HIGH, ent->item->msg_pickup);

	return true;
}

/*
===============
Pickup_Manna

Separate routine so we can distinguish between ammo and mana.
===============
*/

qboolean
Pickup_Mana(edict_t *ent, edict_t *other)
{
	return(Pickup_Ammo(ent, other));
}

void
Drop_Ammo(edict_t *ent, gitem_t *item)
{
	edict_t *dropped;
	int index;

	if (!ent || !item)
	{
		return;
	}

	index = ITEM_INDEX(item);
	dropped = Drop_Item(ent, item);

	if (ent->client->playerinfo.pers.inventory[index] >= item->quantity)
	{
		dropped->count = item->quantity;
	}
	else
	{
		dropped->count = ent->client->playerinfo.pers.inventory[index];
	}

	ent->client->playerinfo.pers.inventory[index] -= dropped->count;

	ValidateSelectedItem(ent);
}

/* ====================================================================== */

void
MegaHealth_think(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if ((self->owner->health > self->owner->max_health)
		&& !CTFHasRegeneration(self->owner))
	{
		self->nextthink = level.time + 1;
		self->owner->health -= 1;
		return;
	}

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value))
	{
		SetRespawn(self, 20);
	}
	else
	{
		G_FreeEdict(self);
	}
}

qboolean
Pickup_Health(edict_t *ent, edict_t *other)
{
	if (!ent || !other)
	{
		return false;
	}

	if (other->flags & FL_CHICKEN)
	{
		return false;
	}

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health >= other->max_health)
		{
			return false;
		}
	}

	other->health += ent->item->quantity;

	if(other->fire_damage_time>level.time)
	{
		other->fire_damage_time -= ent->item->quantity/10;
		if(other->fire_damage_time<=0)
		{
			other->fire_damage_time = 0;
//			gi.RemoveEffects(other, FX_FIRE_ON_ENTITY);//turn off CFX too
			other->s.effects |= EF_MARCUS_FLAG1;		// Notify the effect to turn itself off.
		}
	}

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
		{
			other->health = other->max_health;
		}
	}

	if(other->client)
		player_repair_skin(other);

	G_CPrintf(other, PRINT_HIGH, ent->item->msg_pickup);

	return true;
}

/* ====================================================================== */

int
ArmorIndex(edict_t *ent)
{
	if (!ent || !ent->client)
	{
		return 0;
	}

	if (ent->client->pers.inventory[jacket_armor_index] > 0)
	{
		return jacket_armor_index;
	}

	if (ent->client->pers.inventory[combat_armor_index] > 0)
	{
		return combat_armor_index;
	}

	if (ent->client->pers.inventory[body_armor_index] > 0)
	{
		return body_armor_index;
	}

	return 0;
}

qboolean
Pickup_Armor(edict_t *ent, edict_t *other)
{
	int old_armor_index;
	gitem_armor_t *oldinfo;
	gitem_armor_t *newinfo;
	int newcount;
	float salvage;
	int salvagecount;

	if (!ent || !other)
	{
		return false;
	}

	/* get info on new armor */
	newinfo = (gitem_armor_t *)ent->item->info;

	old_armor_index = ArmorIndex(other);

	/* handle armor shards specially */
	if (ent->item->tag == ARMOR_SHARD)
	{
		if (!old_armor_index)
		{
			other->client->pers.inventory[jacket_armor_index] = 2;
		}
		else
		{
			other->client->pers.inventory[old_armor_index] += 2;
		}
	}

	/* if player has no armor, just use it */
	else if (!old_armor_index)
	{
		other->client->pers.inventory[ITEM_INDEX(ent->item)] =
			newinfo->base_count;
	}

	/* use the better armor */
	else
	{
		/* get info on old armor */
		if (old_armor_index == jacket_armor_index)
		{
			oldinfo = &jacketarmor_info;
		}
		else if (old_armor_index == combat_armor_index)
		{
			oldinfo = &combatarmor_info;
		}
		else /* (old_armor_index == body_armor_index) */
		{
			oldinfo = &bodyarmor_info;
		}

		if (newinfo->normal_protection > oldinfo->normal_protection)
		{
			/* calc new armor values */
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount = salvage *
						   other->client->pers.inventory[old_armor_index];
			newcount = newinfo->base_count + salvagecount;

			if (newcount > newinfo->max_count)
			{
				newcount = newinfo->max_count;
			}

			/* zero count of old armor so it goes away */
			other->client->pers.inventory[old_armor_index] = 0;

			/* change armor to new item with computed value */
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;
		}
		else
		{
			/* calc new armor values */
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = salvage * newinfo->base_count;
			newcount = other->client->pers.inventory[old_armor_index] +
					   salvagecount;

			if (newcount > oldinfo->max_count)
			{
				newcount = oldinfo->max_count;
			}

			/* if we're already maxed out then we don't need the new armor */
			if (other->client->pers.inventory[old_armor_index] >= newcount)
			{
				return false;
			}

			/* update current armor value */
			other->client->pers.inventory[old_armor_index] = newcount;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
	{
		SetRespawn(ent, 20);
	}

	return true;
}

/* ====================================================================== */

int
PowerArmorType(edict_t *ent)
{
	if (!ent)
	{
		return POWER_ARMOR_NONE;
	}

	if (!ent->client)
	{
		return POWER_ARMOR_NONE;
	}

	if (!(ent->flags & FL_POWER_ARMOR))
	{
		return POWER_ARMOR_NONE;
	}

	if (ent->client->pers.inventory[power_shield_index] > 0)
	{
		return POWER_ARMOR_SHIELD;
	}

	if (ent->client->pers.inventory[power_screen_index] > 0)
	{
		return POWER_ARMOR_SCREEN;
	}

	return POWER_ARMOR_NONE;
}

void
Use_PowerArmor(edict_t *ent, gitem_t *item)
{
	int index;

	if (!ent || !item)
	{
		return;
	}

	if (ent->flags & FL_POWER_ARMOR)
	{
		ent->flags &= ~FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex(
						"misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		index = ITEM_INDEX(FindItem("cells"));

		if (!ent->client->pers.inventory[index])
		{
			gi.cprintf(ent, PRINT_HIGH, "No cells for power armor.\n");
			return;
		}

		ent->flags |= FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex(
						"misc/power1.wav"), 1, ATTN_NORM, 0);
	}
}

qboolean
Pickup_PowerArmor(edict_t *ent, edict_t *other)
{
	int quantity;

	if (!ent || !other)
	{
		return false;
	}

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			SetRespawn(ent, ent->item->quantity);
		}

		/* auto-use for DM only if we didn't already have one */
		if (!quantity)
		{
			ent->item->use(other, ent->item);
		}
	}

	return true;
}

void
Drop_PowerArmor(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	if ((ent->flags & FL_POWER_ARMOR) &&
		(ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
	{
		Use_PowerArmor(ent, item);
	}

	Drop_General(ent, item);
}

/* ====================================================================== */

void
Touch_Item(edict_t *ent, edict_t *other, cplane_t *plane /* unused */, csurface_t *surf /* unused */)
{
	if (!ent || !other)
	{
		return;
	}

	if (!other->client)
	{
		return;
	}

	if(strcmp(other->classname,"player"))
	{
		// Only players can touch items.

		return;
	}

	if (other->health < 1)
	{
		// Dead players can't pickup.

		return;
	}


	if(!ent->item->pickup)
	{
		// Not a grabbable item.

		return;
	}

	assert(ent->item->pickup);

	if((other->client->playerinfo.edictflags & FL_CHICKEN) && (ent->item->pickup == Pickup_Health))
	{
		// chickens can't pickup health

		return;
	}

		if(!ent->item->pickup(ent, other))
	{
		// Player can't hold it.

		return;
	}

	gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);

	gi.CreateEffect(NULL, FX_PICKUP, 0, ent->s.origin, "");

	G_UseTargets(ent, other);

	// Handle respawn / removal of the item.

	if(((ent->item->pickup==Pickup_Weapon)||(ent->item->pickup==Pickup_Defense)||(ent->item->pickup==Pickup_Puzzle))&&
	   ((deathmatch->value&&((int)dmflags->value&DF_WEAPONS_STAY))||coop->value))
	{
		// The item is a weapon or a defence or a puzzle piece AND (deathmatch rule DF_WEAPONS_STAY
		// is on OR we are playing coop), so just return right now, as we don't care about respawn
		// or removal.

		return;
	}

	if(ent->flags & FL_RESPAWN)
	{
		// The item should respawn.

		SetRespawn(ent, ent->item->quantity);
	}
	else
	{
		// Going away for good, so make it noclipping.

		ent->solid = SOLID_NOT;

		// Once picked up, the item is gone forever, so remove it's client effect(s).

		gi.RemoveEffects(ent,0);

		// The persistent part is removed from the server here.

		G_SetToFree(ent);
	}
}

/* ====================================================================== */

void
drop_temp_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!ent || !other)
	{
		return;
	}

	if (other == ent->owner)
	{
		return;
	}

	/* plane and surf are unused in Touch_Item
	   but since the function is part of the
	   game <-> client interface dropping
	   them is too much pain. */
	Touch_Item(ent, other, plane, surf);
}

void
drop_make_touchable(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	ent->touch = Touch_Item;

	if (deathmatch->value)
	{
		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
}

edict_t *
Drop_Item(edict_t *ent, gitem_t *item)
{
	edict_t *dropped;
	vec3_t forward, right;
	vec3_t offset;

	if (!ent || !item)
	{
		return NULL;
	}

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;

	if (frandk() > 0.5)
	{
		dropped->s.angles[1] += frandk()*45;
	}
	else
	{
		dropped->s.angles[1] -= frandk()*45;
	}

	VectorSet(dropped->mins, -15, -15, -15);
	VectorSet(dropped->maxs, 15, 15, 15);
	gi.setmodel(dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_NONE;
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t trace;

		AngleVectors(ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource(ent->s.origin, offset, forward, right,
				dropped->s.origin);
		trace = gi.trace(ent->s.origin, dropped->mins, dropped->maxs,
				dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy(trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors(ent->s.angles, forward, right, NULL);
		VectorCopy(ent->s.origin, dropped->s.origin);
	}

	VectorScale(forward, 100, dropped->velocity);
	dropped->velocity[2] = 300;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity(dropped);

	return dropped;
}

void
Use_Item(edict_t *ent, edict_t *other /* unused */, edict_t *activator /* unused */)
{
	if (!ent)
	{
		return;
	}

	ent->use = NULL;

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->touch = Touch_Item;
	}

	gi.linkentity(ent);
}

/* ====================================================================== */

void
FixEntityPosition(edict_t *ent)
{
	int i;

	for (i = 0; i < 3; i++)
	{
		int j;

		for (j = 0; j < 3; j++)
		{
			vec3_t pos;
			trace_t tr_pos;
			int k;

			VectorCopy(ent->s.origin, pos);

			/* move by min */
			for (k = 0; k < i + 1; k++)
			{
				int v;

				v = (j + k) % 3;
				pos[v] = ent->s.origin[v] - ent->mins[v];
			}

			tr_pos = gi.trace(pos, ent->mins, ent->maxs, ent->s.origin, ent, MASK_SOLID);
			if (!tr_pos.startsolid)
			{
				VectorCopy(tr_pos.endpos, ent->s.origin);
				return;
			}

			/* move by max */
			for (k = 0; k < i + 1; k++)
			{
				int v;

				v = (j + k) % 3;
				pos[v] = ent->s.origin[v] - ent->maxs[v];
			}
			tr_pos = gi.trace(pos, ent->mins, ent->maxs, ent->s.origin, ent, MASK_SOLID);
			if (!tr_pos.startsolid)
			{
				VectorCopy(tr_pos.endpos, ent->s.origin);
				return;
			}
		}
	}
}

void
droptofloor(edict_t *ent)
{
	vec3_t mins, maxs, dest;
	trace_t tr;
	float *v;
	int i;

	if (!ent)
	{
		return;
	}

	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_STATIC;
	ent->touch = Touch_Item;
	ent->think = NULL;

	/* set real size of item model except height to items fly hack */
	VectorCopy(ent->mins, mins);
	VectorCopy(ent->maxs, maxs);
	gi.GetModelInfo(ent->s.modelindex, NULL, mins, maxs);

	for (i = 0; i < 2; i++)
	{
		ent->mins[i] = mins[i];
		ent->maxs[i] = maxs[i];
	}

	if (!(ent->spawnflags & ITEM_NO_DROP))
	{
		v = tv(0, 0, -128);
		VectorAdd(ent->s.origin, v, dest);

		tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);

		if (tr.startsolid)
		{
			FixEntityPosition(ent);

			tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
		}

		if (tr.startsolid)
		{
			if (strcmp(ent->classname, "foodcube") == 0)
			{
				VectorCopy(ent->s.origin, tr.endpos);
				ent->velocity[2] = 0;
			}
			else
			{
				gi.dprintf("%s: %s startsolid at %s\n",
						__func__,
						ent->classname,
						vtos(ent->s.origin));
				G_FreeEdict(ent);
				return;
			}
		}

		tr.endpos[2] += 24;
		VectorCopy(tr.endpos, ent->s.origin);
	}

	gi.linkentity(ent);

	// if we loading a saved game - the objects will already be out there
	if (!ent->PersistantCFX)
	{
		SpawnItemEffect(ent, ent->item);
	}
}

// ************************************************************************************************
// ValidItem
// ---------
// ************************************************************************************************

qboolean
ValidItem(gitem_t *item)
{
	// Some items will be prevented in deathmatch.

	if(deathmatch->value)
	{
		if ( (int)dmflags->value & DF_NO_DEFENSIVE_SPELL )
		{
			if (item->flags & IT_DEFENSE)
			{
				return false;
			}
		}
		if ( (int)dmflags->value & DF_NO_OFFENSIVE_SPELL )
		{
			if (item->flags & IT_OFFENSE)
			{
				return false;
			}
		}
		if ( (int)dmflags->value & DF_NO_HEALTH )
		{
			if (item->flags & IT_HEALTH)
			{
				return false;
			}
		}

		if ((item->flags & IT_DEFENSE) && (item->tag == ITEM_DEFENSE_TORNADO) && (no_tornado->value))
			return false;
		else if ((item->flags & IT_DEFENSE) && (item->tag == ITEM_DEFENSE_POLYMORPH) && (no_morph->value))
			return false;
		else if ((item->flags & IT_DEFENSE) && (item->tag == ITEM_DEFENSE_SHIELD) && (no_shield->value))
			return false;
		else if ((item->flags & IT_DEFENSE) && (item->tag == ITEM_DEFENSE_TELEPORT) && (no_teleport->value))
			return false;
		else if ((item->flags & IT_OFFENSE) && (item->tag == ITEM_WEAPON_PHOENIXBOW) && (no_phoenix->value))
			return false;
		else if ((item->flags & IT_OFFENSE) && (item->tag == ITEM_WEAPON_MACEBALLS) && (no_irondoom->value))
			return false;

	}

	return true;
}

// ************************************************************************************************
// SpawnItemEffect
// ---------------
// ************************************************************************************************

void
SpawnItemEffect(edict_t *ent, gitem_t *item)
{

	if(!ValidItem(item))
	{
		G_FreeEdict(ent);
		return;
	}

	assert(!ent->PersistantCFX);

	if ((ent->spawnflags & ITEM_COOP_ONLY) && (!coop->value))
		return;

	if(ent->item->flags & IT_PUZZLE)
	{
		ent->PersistantCFX = gi.CreatePersistantEffect(ent, FX_PICKUP_PUZZLE, CEF_BROADCAST, ent->s.origin, "bv", ent->item->tag,ent->s.angles);
	}
	else if(ent->item->flags & IT_WEAPON)
	{
		ent->PersistantCFX = gi.CreatePersistantEffect(ent, FX_PICKUP_WEAPON, CEF_BROADCAST, ent->s.origin, "b", ent->item->tag);
	}
	else if(ent->item->flags & IT_AMMO)
	{
		ent->PersistantCFX = gi.CreatePersistantEffect(ent, FX_PICKUP_AMMO, CEF_BROADCAST, ent->s.origin, "b", ent->item->tag);
	}
	else if(ent->item->flags & IT_DEFENSE)
	{
		ent->PersistantCFX = gi.CreatePersistantEffect(ent, FX_PICKUP_DEFENSE, CEF_BROADCAST, ent->s.origin, "b", ent->item->tag);
	}
	else
	{
		if (ent->item->tag)
			ent->PersistantCFX = gi.CreatePersistantEffect(ent, FX_PICKUP_HEALTH, CEF_FLAG6|CEF_BROADCAST, ent->s.origin, "");
		else
			ent->PersistantCFX = gi.CreatePersistantEffect(ent, FX_PICKUP_HEALTH, CEF_BROADCAST, ent->s.origin, "");
	}
}

/*
 * Precaches all data needed for a given item.
 * This will be called for each item spawned in a level,
 * and for each item in each client's inventory.
 */
void
PrecacheItem(gitem_t *it)
{
	char *s, *start;
	char data[MAX_QPATH];
	int len;
	gitem_t *ammo;

	if (!it)
	{
		return;
	}

	if (it->pickup_sound)
	{
		gi.soundindex(it->pickup_sound);
	}

	if (it->world_model)
	{
		gi.modelindex(it->world_model);
	}

	if (it->view_model)
	{
		gi.modelindex(it->view_model);
	}

	if (it->icon)
	{
		gi.imageindex(it->icon);
	}

	/* parse everything for its ammo */
	if (it->ammo && it->ammo[0])
	{
		ammo = FindItem(it->ammo);

		if (ammo != it)
		{
			PrecacheItem(ammo);
		}
	}

	/* parse the space seperated precache string for other items */
	s = it->precaches;

	if (!s || !s[0])
	{
		return;
	}

	while (*s)
	{
		start = s;

		while (*s && *s != ' ')
		{
			s++;
		}

		len = s - start;

		if ((len >= MAX_QPATH) || (len < 5))
		{
			gi.error("PrecacheItem: %s has bad precache string", it->classname);
		}

		memcpy(data, start, len);
		data[len] = 0;

		if (*s)
		{
			s++;
		}

		/* determine type based on extension */
		if (!strcmp(data + len - 3, "md2"))
		{
			gi.modelindex(data);
		}
		else if (!strcmp(data + len - 3, "sp2"))
		{
			gi.modelindex(data);
		}
		else if (!strcmp(data + len - 3, "wav"))
		{
			gi.soundindex(data);
		}

		if (!strcmp(data + len - 3, "pcx"))
		{
			gi.imageindex(data);
		}
	}
}

/*
 * Create the item marked for spawn creation
 */
void
Item_TriggeredSpawn(edict_t *self, edict_t *other /* unused */, edict_t *activator /* unused */)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;

	if (strcmp(self->classname, "key_power_cube"))
	{
		self->spawnflags = 0;
	}

	droptofloor(self);
}

/*
 * Set up an item to spawn in later.
 */
void
SetTriggeredSpawn(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	/* don't do anything on key_power_cubes. */
	if (!strcmp(ent->classname, "key_power_cube"))
	{
		return;
	}

	ent->think = NULL;
	ent->nextthink = 0;
	ent->use = Item_TriggeredSpawn;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
}

/*
 * ============
 * Sets the clipping size and
 * plants the object on the floor.
 *
 * Items can't be immediately dropped
 * to floor, because they might be on
 * an entity that hasn't spawned yet.
 * ============
 */
void
SpawnItem(edict_t *ent, gitem_t *item)
{
	if (!ent || !item)
	{
		return;
	}

	if ((ent->spawnflags & ITEM_COOP_ONLY) && (!coop->value))
	{
		return;
	}

	/* RREXTEND: Reset dynamic model assign */
	ent->s.modelindex = 0;

	PrecacheItem(item);

	if(!ValidItem(item))
	{
		G_FreeEdict(ent);
		return;
	}

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME; /* items start after other solids */
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	ent->s.effects |= EF_ALWAYS_ADD_EFFECTS;

	if (item->flags & IT_WEAPON)
	{
		if (item->tag == ITEM_WEAPON_MACEBALLS)
			ent->delay = RESPAWN_TIME_MACEBALL;		// Maceballs shouldn't come back as fast...
		else
			ent->delay = RESPAWN_TIME_WEAPON;
	}
	else if (item->flags & IT_DEFENSE)
	{
		if (item->tag == ITEM_DEFENSE_REPULSION)
			ent->delay = RESPAWN_TIME_RING;
		else if (item->tag == ITEM_DEFENSE_TELEPORT)
			ent->delay = RESPAWN_TIME_TELEPORT;
		else if (item->tag == ITEM_DEFENSE_POLYMORPH)
			ent->delay = RESPAWN_TIME_MORPH;
		else
			ent->delay = RESPAWN_TIME_DEFENSE;
	}
	else if (item->flags & IT_AMMO)
	{
		if (item->tag == AMMO_REDRAIN || item->tag == AMMO_PHOENIX)
			ent->delay = RESPAWN_TIME_ARROWS;
		else
			ent->delay = RESPAWN_TIME_AMMO;
	}
	else // Health and anything else
	{
		ent->delay = RESPAWN_TIME_MISC;
	}

	ent->flags = item->flags;
	ent->clipmask = MASK_MONSTERSOLID;

	if (item->flags == IT_PUZZLE)
	{
		VectorCopy(ent->item->mins, ent->mins);
		VectorCopy(ent->item->maxs, ent->maxs);
	}

	// FIXME: Until all objects have bounding boxes, default to these vals.
	if (Vec3IsZero(ent->mins))
	{
		VectorSet(ent->mins, -10.0, -10.0, -10.0);
		VectorSet(ent->maxs, 10.0, 10.0, 10.0);
	}

	ent->classname = item->classname;
	if(deathmatch->value)
	{
		ent->flags |= FL_RESPAWN;
	}
}


void
P_ToggleFlashlight(edict_t *ent, qboolean state)
{
	if (!!(ent->flags & FL_FLASHLIGHT) == state)
	{
		return;
	}

	ent->flags ^= FL_FLASHLIGHT;

	gi.sound(ent, CHAN_AUTO,
		gi.soundindex(ent->flags & FL_FLASHLIGHT ?
			"items/flashlight_on.wav" : "items/flashlight_off.wav"),
		1.f, ATTN_STATIC, 0);
}

void
Use_Flashlight(edict_t *ent, gitem_t *inv)
{
	P_ToggleFlashlight(ent, !(ent->flags & FL_FLASHLIGHT));
}

/* ====================================================================== */

static const gitem_t gameitemlist[] = {
	{
		NULL
	}, /* leave index 0 alone */


	/*
	 * QUAKED weapon_swordstaff
	 * This can't be placed in the editor
	 */
	{
		"Weapon_SwordStaff",					// Spawnname (char *)
		Pickup_Weapon,	 						// Pickup (f)
		Weapon_EquipSwordStaff,					// Use (f)
		NULL,									// Drop	(f)
		WeaponThink_SwordStaff,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		NULL,									// view model
		"icons/i_staff.m8",						// Icon name (char *)
		"staff",								// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_WEAPON,								// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_SWORDSTAFF,					// tag ?
		NULL,									// precaches
		0,										// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WSWORD_STD1,						// Player animation sequence to engage when used
		ASEQ_WSWORD_STD1,						// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED weapon_flyingfist
	 * This can't be placed in the editor
	 */
	{
		"Weapon_FlyingFist", 					// Spawnname
		Pickup_Weapon,							// Pickup (f)
		Weapon_EquipSpell,						// Use (f)
		NULL,									// Drop	(f)
		WeaponThink_FlyingFist,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		NULL,									// view model
		"icons/i_fball.m8",   			// Icon name (char *)
		"fball",								// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_USE_FIREBALL,						// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_FLYINGFIST,					// tag ?
		NULL,									// precaches
		0,										// pickup message
		GM_NOFLYINGFIST,						// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WFIREBALL,							// Player animation sequence to engage when used
		ASEQ_WFIREBALL,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_weapon_hellstaff (.3 .3 1) (-16 -16 -16) (16 16 16) COOP_ONLY
	 * Pickup for the hellstaff weapon.
	 */
	{
		"item_weapon_hellstaff",				// Spawnname
		Pickup_Weapon,	 						// Pickup (f)
		Weapon_EquipHellStaff,					// Use (f)
		Drop_Weapon,							// Drop	(f)
		WeaponThink_HellStaff,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		NULL,									// view model
		"icons/i_hell.m8",				// Icon name (char *)
		"hell",									// Pickup name (char *)
		2,										// Number of digits to display
		AMMO_USE_HELLSTAFF,						// Ammo/ammo use per shot
		"Hell-staff-ammo",						// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_HELLSTAFF,					// tag ?
		NULL,									// precaches
		GM_HELLSTAFF,							// pickup message
		GM_NOHELLORBS,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WHELL_GO,							// Player animation sequence to engage when used
		ASEQ_WHELL_GO,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = infinite)
	},

	/*
	 * QUAKED item_weapon_magicmissile (.3 .3 1) (-16 -16 -16) (16 16 16) COOP_ONLY
	 * Pickup for the Magic Missile weapon.
	 */
	{
		"item_weapon_magicmissile",				// Spawnname
		Pickup_Weapon,							// Pickup (f)
		Weapon_EquipSpell,						// Use (f)
		Drop_Weapon,							// Drop	(f)
		WeaponThink_MagicMissileSpread,			// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		NULL,									// view model
		"icons/i_array.m8",				// Icon name (char *)
		"array",								// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_USE_MAGICMISSILE,					// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_MAGICMISSILE,				// tag ?
		NULL,									// precaches
		GM_FORCEBLAST, 							// pickup message
		GM_NOFORCE,								// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WBLAST,							// Player animation sequence to engage when used
		ASEQ_WARRAY,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_weapon_redrain_bow (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
	 * Pickup for the Red Rain Bow weapon.
	 */
	{
		"item_weapon_redrain_bow", 				// Spawnname
		Pickup_Weapon,	 						// Pickup (f)
		Weapon_EquipBow,						// Use (f)
		Drop_Weapon,							// Drop	(f)
		WeaponThink_RedRainBow,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		NULL,									// view model
		"icons/i_rain.m8",				// Icon name (char *)
		"rain",									// Pickup name (char *)
		2,										// Number of digits to display
		AMMO_USE_REDRAIN,						// Ammo/ammo use per shot
		"Red-Rain-Arrows",						// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_REDRAINBOW,					// tag ?
		NULL,									// precaches
		GM_STORMBOW,							// pickup message
		GM_NOSTORMBOW,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WRRBOW_GO,							// Player animation sequence to engage when used
		ASEQ_WRRBOW_GO,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_weapon_firewall (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the Fire Wall weapon.
	 */
	{
		"item_weapon_firewall",					// Spawnname
		Pickup_Weapon,							// Pickup (f)
		Weapon_EquipSpell,						// Use (f)
		Drop_Weapon,							// Drop	(f)
		WeaponThink_Firewall,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		NULL,									// view model
		"icons/i_fwall.m8",						// Icon name (char *)
		"fwall",								// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_USE_FIREWALL,						// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_FIREWALL,					// tag ?
		NULL,									// precaches
		GM_FIREWALL,							// pickup message
		GM_NOFIREWALL,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WFIREWALL,							// Player animation sequence to engage when used
		ASEQ_WFIREWALL,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_weapon_phoenixbow (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
	 * Pickup for the Phoenix Bow weapon.
	 */
	{
		"item_weapon_phoenixbow", 				// Spawnname
		Pickup_Weapon,	 						// Pickup (f)
		Weapon_EquipBow,						// Use (f)
		Drop_Weapon,							// Drop	(f)
		WeaponThink_PhoenixBow,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		NULL,									// view model
		"icons/i_phoen.m8", 					// Icon name (char *)
		"phoen",								// Pickup name (char *)
		2,										// Number of digits to display
		AMMO_USE_PHOENIX,						// Ammo/ammo use per shot
		"Phoenix-Arrows",						// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_PHOENIXBOW,					// tag ?
		NULL,									// precaches
		GM_PHOENIX,								// pickup message
		GM_NOPHOENIX,							// can`t use message
		{0, 0, 0},								// Bounding box mins
		{0, 0, 0},								// Bounding box maxs
		ASEQ_WPHBOW_GO,							// Player animation sequence to engage when used
		ASEQ_WPHBOW_GO,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_weapon_sphereofannihilation (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
	 * Pickup for the Sphere Annihilation weapon.
	 */
	{
		"item_weapon_sphereofannihilation",		// Spawnname
		Pickup_Weapon,							// Pickup (f)
		Weapon_EquipSpell,						// Use (f)
		Drop_Weapon,							// Drop	(f)
		WeaponThink_SphereOfAnnihilation,		// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		NULL,									// view model
		"icons/i_sphere.m8",			// Icon name (char *)
		"sphere",								// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_USE_SPHERE,						// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_SPHEREOFANNIHILATION,		// tag ?
		NULL,									// precaches
		GM_SPHERE,								// pickup message
		GM_NOSPHERE,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WSPHERE_GO,						// Player animation sequence to engage when used
		ASEQ_WSPHERE_GO,						// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_weapon_maceballs (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
	 * Pickup for the Mace Balls weapon.
	 */
	{
		"item_weapon_maceballs",				// Spawnname
		Pickup_Weapon,							// Pickup (f)
		Weapon_EquipSpell,						// Use (f)
		Drop_Weapon,							// Drop	(f)
		WeaponThink_Maceballs,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		NULL,									// view model
		"icons/i_mace.m8",				// Icon name (char *)
		"mace",									// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_USE_MACEBALL,						// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_MACEBALLS,					// tag ?
		NULL,									// precaches
		GM_IRONDOOM,							// pickup message
		GM_NOIRONDOOM,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WRIPPER,							// Player animation sequence to engage when used
		ASEQ_WBIGBALL,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_defense_powerup (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
	 * This can't be placed in the editor
	 */
	{
		"item_defense_powerup",					// Spawnname
		Pickup_Defense,							// Pickup (f)
		Use_Defence,							// Use (f)
		NULL,									// Drop	(f)
		DefenceThink_Powerup,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		NULL,									// view model
		"icons/i_tome.m8",				// Icon name (char *)
		"powerup",								// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_POWERUP,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_POWERUP,					// tag ?
		NULL,									// precaches
		GM_TOME, 								// pickup message
		GM_NOTOME,								// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_defense_ringofrepulsion (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the Ring of Repulsion defensive spell.
	 */
	{
		"item_defense_ringofrepulsion",			// Spawnname
		Pickup_Defense,							// Pickup (f)
		Use_Defence,							// Use (f)
		NULL,									// Drop	(f)
		DefenceThink_RingOfRepulsion,			// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		NULL,									// view model
		"icons/i_ring.m8",			// Icon name (char *)
		"ring",									// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_RING,							// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_REPULSION,					// tag ?
		NULL,									// precaches
		GM_RING,								// pickup message
		GM_NORING,								// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_defense_shield (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the Shield defensive spell.
	 */
	{
		"item_defense_shield",					// Spawnname
		Pickup_Defense,							// Pickup (f)
		Use_Defence,							// Use (f)
		NULL,									// Drop	(f)
		DefenceThink_Shield,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		NULL,									// view model
		"icons/i_shield.m8",			// Icon name (char *)
		"lshield",								// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_SHIELD,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_SHIELD,					// tag ?
		NULL,									// precaches
		GM_SHIELD,								// pickup message
		GM_NOSHIELD,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_defense_teleport (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the Teleport defensive spell.
	 */
	{
		"item_defense_teleport",				// Spawnname
		Pickup_Defense,							// Pickup (f)
		Use_Defence,							// Use (f)
		NULL,									// Drop	(f)
		DefenceThink_Teleport,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		NULL,									// view model
		"icons/i_tele.m8",						// Icon name (char *)
		"tele",									// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_TELEPORT,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_TELEPORT,					// tag ?
		NULL,									// precaches
		GM_TELEPORT,							// pickup message
		GM_NOTELEPORT,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_defense_polymorph (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the Polymorph Barrier defensive spell.
	 */
	{
		"item_defense_polymorph",				// Spawnname
		Pickup_Defense,							// Pickup (f)
		Use_Defence,							// Use (f)
		NULL,									// Drop	(f)
		DefenceThink_Morph,						// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		NULL,									// view model
		"icons/i_morph.m8",					// Icon name (char *)
		"morph",								// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_POLYMORPH,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_POLYMORPH,					// tag ?
		NULL,									// precaches
		GM_MORPH,								// pickup message
		GM_NOMORPH,								// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_defense_meteorbarrier (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the Meteor Barrier defensive spell.
	 */
	{
		"item_defense_meteorbarrier",			// Spawnname
		Pickup_Defense,							// Pickup (f)
		Use_Defence,							// Use (f)
		NULL,									// Drop	(f)
		DefenceThink_MeteorBarrier,				// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		NULL,									// view model
		"icons/i_meteor.m8",			// Icon name (char *)
		"meteor",								// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_METEORS,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_METEORBARRIER,				// tag ?
		NULL,									// precaches
		GM_METEOR,								// pickup message
		GM_NOMETEOR,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)
	},


	// =============================================================================================

	// Ammo items.

	/*
	 * QUAKED item_mana_offensive_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the offensive mana (50 points).
	 */
	{
		"item_mana_offensive_half",				// Spawnname
		Pickup_Mana, 							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		NULL,									// view model
		NULL,									// Icon name (char *)
		"Off-mana",								// Pickup name (char *)
		0,										// Number of digits to display
		HALF_OFF_MANA,							// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		AMMO_MANA_OFFENSIVE_HALF,			// tag ?
		NULL,									// precaches
		GM_OFFMANAS,							// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_mana_offensive_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the offensive mana (100 points).
	 */
	{
		"item_mana_offensive_full",				// Spawnname
		Pickup_Mana, 									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		NULL,									// view model
		NULL,									// Icon name (char *)
		"Off-mana",								// Pickup name (char *)
		0,										// Number of digits to display
		FULL_OFF_MANA,							// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_OFFENSE,					// flags
		0,									// weapon model index
		NULL,									// void * ?
		AMMO_MANA_OFFENSIVE_FULL,			// tag ?
		NULL,									// precaches
		GM_OFFMANAB,										// pickup message
		0,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_mana_defensive_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the defensive mana (50 points).
	 */
	{
		"item_mana_defensive_half",				// Spawnname
		Pickup_Mana,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		NULL,									// view model
		NULL,									// Icon name (char *)
		"Def-mana",								// Pickup name (char *)
		0,										// Number of digits to display
		HALF_DEF_MANA,							// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_DEFENSE,					// flags
		0,									// weapon model index
		NULL,									// void * ?
		AMMO_MANA_DEFENSIVE_HALF,			// tag ?
		NULL,									// precaches
		GM_DEFMANAS,							// pickup message
		0,
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_mana_defensive_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the defensive mana (100 points).
	 */
	{
		"item_mana_defensive_full",				// Spawnname
		Pickup_Mana,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		NULL,									// view model
		NULL,									// Icon name (char *)
		"Def-mana",								// Pickup name (char *)
		0,										// Number of digits to display
		FULL_DEF_MANA,							// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_DEFENSE,					// flags
		0,									// weapon model index
		NULL,									// void * ?
		AMMO_MANA_DEFENSIVE_FULL,			// tag ?
		NULL,									// precaches
		GM_DEFMANAB,							// pickup message
		0,
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
		-1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_mana_combo_quarter (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for both defensive & offensive mana (25 points).
	 */
	{
		"item_mana_combo_quarter",					  // Spawnname
		Pickup_Mana,								  // Pickup (f)
		NULL,										  // Use (f)
		NULL,										  // Drop	(f)
		NULL,										  // Think (f)
		"player/picup.wav",							  // Pickup sound (char *)
		NULL,										  // world model (char *)
		0,											  // world model flags
		NULL,									// view model
		NULL,										  // Icon name (char *)
		"Def-mana",									  // Pickup name (char *)
		0,											  // Number of digits to display
		HALF_COMBO_MANA,							  // Ammo/ammo use per shot
		NULL,										  // Ammo (char *)
		IT_AMMO,									  // flags
		0,									// weapon model index
		NULL,										  // void * ?
		AMMO_MANA_COMBO_QUARTER,				  // tag ?
		NULL,									// precaches
		GM_COMBMANAS,										// pickup message
		0,
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_mana_combo_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for both defensive & offensive mana (50 points).
	 */
	{
		"item_mana_combo_half",					  // Spawnname
		Pickup_Mana,								  // Pickup (f)
		NULL,										  // Use (f)
		NULL,										  // Drop	(f)
		NULL,										  // Think (f)
		"player/picup.wav",							  // Pickup sound (char *)
		NULL,										  // world model (char *)
		0,											  // world model flags
		NULL,									// view model
		NULL,										// Icon name (char *)
		"Def-mana",									  // Pickup name (char *)
		0,											  // Number of digits to display
		FULL_COMBO_MANA,							  // Ammo/ammo use per shot
		NULL,										  // Ammo (char *)
		IT_AMMO,									  // flags
		0,									// weapon model index
		NULL,										  // void * ?
		AMMO_MANA_COMBO_HALF,					  // tag ?
		NULL,									// precaches
		GM_COMBMANAB,										// pickup message
		0,
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_ammo_redrain (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup ammo for the Red Rain Bow
	 */
	{
		"item_ammo_redrain",						  // Spawnname
		Pickup_Ammo,								  // Pickup (f)
		NULL,										  // Use (f)
		NULL,										  // Drop	(f)
		NULL,										  // Think (f)
		"player/picup.wav",							  // Pickup sound (char *)
		NULL,										  // world model (char *)
		0,											  // world model flags
		NULL,									// view model
		"icons/i_ammo-redrain.m8",								  // Icon name (char *)
		"Red-Rain-Arrows",							  // Pickup name (char *)
		0,											  // Number of digits to display
		AMMO_COUNT_REDRAINBOW,						  // Ammo/ammo use per shot
		NULL,										  // Ammo (char *)
		IT_AMMO | IT_OFFENSE,						  // flags
		0,									// weapon model index
		NULL,										  // void * ?
		AMMO_REDRAIN,								  // tag ?
		NULL,									// precaches
		GM_STORMARROWS,										// pickup message
		0,
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_ammo_phoenix (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup ammo for the Phoenix Bow
	 */
	{
		"item_ammo_phoenix",							  // Spawnname
		Pickup_Ammo,								  // Pickup (f)
		NULL,										  // Use (f)
		NULL,										  // Drop	(f)
		NULL,										  // Think (f)
		"player/picup.wav",							  // Pickup sound (char *)
		NULL,										  // world model (char *)
		0,											  // world model flags
		NULL,									// view model
		"icons/i_ammo-phoen.m8",								  // Icon name (char *)
		"Phoenix-Arrows",							  // Pickup name (char *)
		0,											  // Number of digits to display
		AMMO_COUNT_PHOENIXBOW,						  // Ammo/ammo use per shot
		NULL,										  // Ammo (char *)
		IT_AMMO | IT_OFFENSE,						  // flags
		0,									// weapon model index
		NULL,										// void * ?
		AMMO_PHOENIX,							  // tag ?
		NULL,									// precaches
		GM_PHOENARROWS,										// pickup message
		0,
		PICKUP_MIN,									// Bounding box mins
		PICKUP_MAX,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,										// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_ammo_hellstaff (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup ammo for the Hellstaff
	 */
	{
		"item_ammo_hellstaff",						  // Spawnname
		Pickup_Ammo,								  // Pickup (f)
		NULL,										  // Use (f)
		NULL,										  // Drop	(f)
		NULL,										  // Think (f)
		"player/picup.wav",							  // Pickup sound (char *)
		NULL,										  // world model (char *)
		0,											  // world model flags
		NULL,									// view model
		"icons/i_ammo-hellstaff.m8",				  // Icon name (char *)
		"Hell-staff-ammo",							  // Pickup name (char *)
		0,											  // Number of digits to display
		AMMO_COUNT_HELLSTAFF,						  // Ammo/ammo use per shot
		NULL,										  // Ammo (char *)
		IT_AMMO | IT_OFFENSE,						  // flags
		0,									// weapon model index
		NULL,										  // void * ?
		AMMO_HELLSTAFF,						  // tag ?
		NULL,									// precaches
		GM_HELLORB,										// pickup message
		0,
		PICKUP_MIN,									// Bounding box mins
		PICKUP_MAX,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)
	},

	// ============================================================================================

	// Other items.

	/*
	 * QUAKED item_health_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup health (10 points)
	 */
	{
		"item_health_half",							// Spawnname
		Pickup_Health, 								// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"*gethealth.wav",							// Pickup sound (char *)
		"models/items/health/healthsmall/tris.fm",	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		NULL,										// Icon name (char *)
		"Minor health",								// Pickup name (char *)
		0,											// Number of digits to display
		10,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_HEALTH | EF_ALWAYS_ADD_EFFECTS, 			// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_HEALTH1,								// tag ?
		NULL,									// precaches
		GM_HEALTHVIAL,										// pickup message
		0,
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_health_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup health (30 points)
	 */
	{
		"item_health_full",							// Spawnname
		Pickup_Health, 								// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"*gethealth.wav",							// Pickup sound (char *)
		"models/items/health/healthbig/tris.fm",	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		NULL,								// Icon name (char *)
		"Major health",								// Pickup name (char *)
		0,											// Number of digits to display
		30,										// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_HEALTH | EF_ALWAYS_ADD_EFFECTS, 			// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_HEALTH2,								// tag ?
		NULL,									// precaches
		GM_HEALTHPOTION,										// pickup message
		0,
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	// ============================================================================================

	// Puzzle Pieces

	/*
	 * QUAKED item_puzzle_townkey (.3 .3 1) (-8 -8 -4) (8 8 4)  x NO_DROP
	 * Key puzzle piece
	 * Town Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_townkey",						// Spawnname
		Pickup_Puzzle,		 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,		// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_townkey.m8",						// Icon name (char *)
		"Town Key",									// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_TOWNKEY,								// tag ?
		NULL,									// precaches
		GM_F_TOWNKEY,								// pickup message
		GM_NEED_TOWNKEY,							// can`t use message
		{-8, -8, -4},									// Bounding box mins
		{8,  8,  4},									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_cog (.3 .3 1) (-10 -10 -24) (10 10 20)  x  NO_DROP
	 * Cog puzzle piece
	 * Palace level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_cog",							// Spawnname
		Pickup_Puzzle,		 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,			// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_cog.m8",							// Icon name (char *)
		"Cog",										// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_COG,									// tag ?
		NULL,									// precaches
		GM_F_COG,									// pickup message
		GM_NEED_COG,								// can`t use message
		{-10, -10, -24},								// Bounding box mins
		{10,  10,  20},								// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},


	/*
	 * QUAKED item_puzzle_shield (.3 .3 1) (-2 -6 -12) (2 6 12)  x  NO_DROP
	 * Sithra Shield puzzle item
	 * Healer Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_shield",						// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,										// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_shield.m8",						// Icon name (char *)
		"Defensive Shield",							// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_SHIELD,				 							// tag ?
		NULL,									// precaches
		GM_F_SHIELD,								// pickup message
		GM_NEED_SHIELD,								// can`t use message
		{-2, -6, -12},								// Bounding box mins
		{2,  6,  12},								// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_potion (.3 .3 1) (-3 -3 -10) (3 3 10)  x  NO_DROP
	 * Potion puzzle item
	 * Healer Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_potion",						// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,		// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_potion.m8",						// Icon name (char *)
		"Potion",									// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_POTION,				 							// tag ?
		NULL,									// precaches
		GM_F_POTION,								// pickup message
		GM_NEED_POTION,								// can`t use message
		{-3, -3, -10},									// Bounding box mins
		{3,  3,  10},									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_plazacontainer (.3 .3 1) (-6 -6 -8) (6 6 6)  x  NO_DROP
	 * Container puzzle item
	 * Plaza Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_plazacontainer",				// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_plazajug.m8",						// Icon name (char *)
		"Container",								// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_CONT,				 							// tag ?
		NULL,									// precaches
		GM_F_CONT,										// pickup message
		GM_NEED_CONT,										// can`t use message
		{-6, -6, -8},									// Bounding box mins
		{6,  6,  6},									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_slumcontainer (.3 .3 1) (-6 -6 -8) (6 6 6)  x  NO_DROP
	 * Full Container puzzle item
	 * Slum Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_slumcontainer",				// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,										// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_jugfill.m8",						// Icon name (char *)
		"Full Container",							// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_SLUMCONT,								// tag ?
		NULL,									// precaches
		GM_F_CONTFULL,								// pickup message
		GM_NEED_CONTFULL,							// can`t use message
		{-6, -6, -8},								// Bounding box mins
		{6,  6,  6},								// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_crystal (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
	 * Crystal puzzle item
	 * Academic Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_crystal",						// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_crystal.m8",						// Icon name (char *)
		"Crystal",									// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_CRYSTAL,				 							// tag ?
		NULL,									// precaches
		GM_F_CRYSTAL,										// pickup message
		GM_NEED_CRYSTAL,										// can`t use message
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_canyonkey (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
	 * Key puzzle item
	 * Canyon Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_canyonkey",					// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_canyonkey.m8",						// Icon name (char *)
		"Canyon Key",								// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_CANKEY,				 							// tag ?
		NULL,									// precaches
		GM_F_CANYONKEY,										// pickup message
		GM_NEED_CANYONKEY,										// can`t use message
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_hive2amulet (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
	 * Amulet puzzle item
	 * Hive 2 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_hive2amulet",					// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_tcheckrikbust.m8",					// Icon name (char *)
		"Hive 2 Amulet",							// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_AMULET,				 							// tag ?
		NULL,									// precaches
		GM_F_AMULET,										// pickup message
		GM_NEED_AMULET,										// can`t use message
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_hive2spear (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
	 * Spear puzzle item
	 * Hive 2 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_hive2spear",					// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_spear.m8",						// Icon name (char *)
		"Hive 2 Spear",								// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_SPEAR,				 							// tag ?
		NULL,									// precaches
		GM_F_SPEAR,										// pickup message
		GM_NEED_SPEAR,										// can`t use message
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_hive2gem (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
	 * Gem puzzle item
	 * Hive 2 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_hive2gem",						// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,										// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_tcheckrikgem.m8",					// Icon name (char *)
		"Hive 2 Gem",								// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_GEM,				 							// tag ?
		NULL,									// precaches
		GM_F_GEM,										// pickup message
		GM_NEED_GEM,										// can`t use message
		PICKUP_MIN,									// Bounding box mins
		PICKUP_MAX,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_minecartwheel (.3 .3 1) (-1 -6 -6) (1 6 6)  x  NO_DROP
	 * Mine Cart Wheel puzzle item
	 * Mine 1 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_minecartwheel",				// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,		// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_wheel.m8",							// Icon name (char *)
		"Minecart Wheel",							// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_WHEEL,				 					// tag ?
		NULL,									// precaches
		GM_F_CARTWHEEL,										// pickup message
		GM_NEED_CARTWHEEL,										// can`t use message
		{-1,-6,-6},									// Bounding box mins
		{1, 6, 6},									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_ore (.3 .3 1) (-10 -10 -8) (10 10 8)  x  NO_DROP
	 * Unrefined Ore puzzle item
	 * Mine 2 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_ore",							// Spawnname
		NULL,		 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_oreunrefined.m8",						// Icon name (char *)
		"Ore",										// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_ORE	,				 							// tag ?
		NULL,									// precaches
		GM_F_UNREFORE,										// pickup message
		GM_NEED_UNREFORE,										// can`t use message
		{-10,-10,-8},							// Bounding box mins
		{10, 10, 8},							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_refinedore (.3 .3 1) (-3 -12 -2) (3 12 2) x   NO_DROP
	 * Refined Ore puzzle item
	 * Mine 2 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_refinedore",					// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_orerefined.m8",					// Icon name (char *)
		"Refined Ore",								// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_REF_ORE,	 							// tag ?
		NULL,									// precaches
		GM_F_REFORE,								// pickup message
		GM_NEED_REFORE,								// can`t use message
		{-3,-12,-2},								// Bounding box mins
		{3, 12, 2},									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_dungeonkey (.3 .3 1) (-1 -18 -9) (1 18 9)  x  NO_DROP
	 * Amulet puzzle item
	 * Dungeon Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_dungeonkey",					// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_dungeonkey.m8",					// Icon name (char *)
		"Dungeon Key",								// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_DUNKEY,	 							// tag ?
		NULL,									// precaches
		GM_F_DUNGEONKEY,										// pickup message
		GM_NEED_DUNGEONKEY,										// can`t use message
		{-1,-18,-9},								// Bounding box mins
		{1, 18, 9},									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_cloudkey (.3 .3 1) (-8 -8 -3) (8 8 6)  x  NO_DROP
	 * Key puzzle item
	 * Cloud Quarters 2 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_cloudkey",						// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_cloudkey.m8",						// Icon name (char *)
		"Cloud Key",								// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_CLOUDKEY,				 							// tag ?
		NULL,									// precaches
		GM_F_CLOUDKEY,										// pickup message
		GM_NEED_CLOUDKEY,										// can`t use message
		{-8, -8, -3},								// Bounding box mins
		{8,  8,  3},								// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_highpriestesskey (.3 .3 1) (-12 -12 -6) (12 12 6) x   NO_DROP
	 * Key puzzle item
	 * High Priestess Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_highpriestesskey",				// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_hivekey.m8",						// Icon name (char *)
		"Key",										// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_HIVEKEY,				 							// tag ?
		NULL,									// precaches
		GM_F_HIGHKEY,										// pickup message
		GM_NEED_HIGHKEY,										// can`t use message
		{-12,-12, -6},								// Bounding box mins
		{12, 12,  6},								// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_highpriestesssymbol (.3 .3 1) (-12 -12 -4) (12 12 4) x   NO_DROP
	 * Key puzzle item
	 * High Priestess Level
	 * NO_DROP - won't drop to ground
	*/
	{
		"item_puzzle_highpriestesssymbol",			// Spawnname
		Pickup_Puzzle,	 							// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_queenkey.m8",						// Icon name (char *)
		"Symbol",									// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_HPSYM,				 							// tag ?
		NULL,									// precaches
		GM_F_SYMBOL,								// pickup message
		GM_NEED_SYMBOL,								// can`t use message
		{-12,-12,-4},								// Bounding box mins
		{12, 12, 4},								// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_tome (.3 .3 1) (-12 -12 -4) (12 12 4)  x  NO_DROP
	 * Tome puzzle piece
	 * 2 Cloud Levels
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_tome",							// Spawnname
		Pickup_Puzzle,		 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		NULL,									// view model
		"icons/p_tomepower.m8",						// Icon name (char *)
		"Tome",										// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_TOME,		 							// tag ?
		NULL,									// precaches
		GM_F_TOME,									// pickup message
		GM_NEED_TOME,								// can`t use message
		{-12,-12,-4},									// Bounding box mins
		{12, 12, 4},									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_puzzle_tavernkey (.3 .3 1) (-8 -8 -4) (8 8 4)    x   NO_DROP
	 * Key puzzle piece
	 * Ssdocks Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_tavernkey",				// Spawnname
		Pickup_Puzzle,		 					// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		NULL,									// view model
		"icons/p_tavernkey.m8",					// Icon name (char *)
		"Tavern Key",							// Pickup name (char *)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		0,									// weapon model index
		NULL,										// void * ?
		ITEM_TAVERNKEY,				 				// tag ?
		NULL,									// precaches
		GM_F_TAVERNKEY,							// pickup message
		GM_NEED_TAVERNKEY,						// can`t use message
		{-12,-12,-4},							// Bounding box mins
		{12, 12, 4},								// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_defense_tornado (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the Tornado defensive spell.
	 */
	{
		"item_defense_tornado",					// Spawnname
		Pickup_Defense,							// Pickup (f)
		Use_Defence,							// Use (f)
		NULL,									// Drop	(f)
		DefenceThink_Tornado,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		NULL,									// view model
		"icons/i_tornado.m8",					// Icon name (char *)
		"tornado",								// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_TORNADO,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,									// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_TORNADO,					// tag ?
		NULL,									// precaches
		GM_TORNADO,								// pickup message
		GM_NOTORNADO,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)
	},

	/*
	 * QUAKED item_ancient_head (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 * Special item that gives +2 to maximum health
	 */
	{
		"item_ancient_head",
		Pickup_AncientHead,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/c_head/tris.md2", EF_ROTATE,
		NULL,
		"i_fixme",
		"Ancient Head",
		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 * gives +1 to maximum health
	 */
	{
		"item_adrenaline",
		Pickup_Adrenaline,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/adrenal/tris.md2", EF_ROTATE,
		NULL,
		"p_adrenaline",
		"Adrenaline",
		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_bandolier",
		Pickup_Bandolier,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/band/tris.md2", EF_ROTATE,
		NULL,
		"p_bandolier",
		"Bandolier",
		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_pack",
		Pickup_Pack,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/pack/tris.md2", EF_ROTATE,
		NULL,
		"i_pack",
		"Ammo Pack",
		2,
		180,
		NULL,
		0,
		0,
		NULL,
		0,
		""
	},

	{
		NULL,
		Tag_PickupToken,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/tagtoken/tris.md2",
		EF_ROTATE | EF_TAGTRAIL,
		NULL,
		"i_tagtoken",
		"Tag Token",
		0,
		0,
		NULL,
		IT_POWERUP | IT_NOT_GIVEABLE,
		0,
		NULL,
		1,
		NULL,
	},

	/*
	 * QUAKED key_data_cd (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 * key for computer centers
	 */
	{
		"key_data_cd",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/data_cd/tris.md2", EF_ROTATE,
		NULL,
		"k_datacd",
		"Data CD",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_power_cube (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN NO_TOUCH
	 * warehouse circuits
	 */
	{
		"key_power_cube",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/power/tris.md2", EF_ROTATE,
		NULL,
		"k_powercube",
		"Power Cube",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_explosive_charges (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN NO_TOUCH
	 * warehouse circuits, key for N64
	 */
	{
		"key_explosive_charges",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/n64/charge/tris.md2", EF_ROTATE,
		NULL,
		"n64/i_charges",
		"Explosive Charges",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_yellow_key (0 .5 .8) (-16 -16 -16) (16 16 16)
	 * normal door key - yellow, key for N64
	 */
	{
		"key_yellow_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/n64/yellow_key/tris.md2", EF_ROTATE,
		NULL,
		"n64/i_yellow_key",
		"Yellow Key",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_power_core (0 .5 .8) (-16 -16 -16) (16 16 16)
	 * key for N64
	 */
	{
		"key_power_core",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/n64/power_core/tris.md2", EF_ROTATE,
		NULL,
		"k_pyramid",
		"Power Core",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 * key for the entrance of jail3
	 */
	{
		"key_pyramid",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
		"k_pyramid",
		"Pyramid Key",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 * key for the city computer
	 */
	{
		"key_data_spinner",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/spinner/tris.md2", EF_ROTATE,
		NULL,
		"k_dataspin",
		"Data Spinner",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 * security pass for the security level
	 */
	{
		"key_pass",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"k_security",
		"Security Pass",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 * normal door key - blue
	 */
	{
		"key_blue_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key/tris.md2", EF_ROTATE,
		NULL,
		"k_bluekey",
		"Blue Key",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 * normal door key - red
	 */
	{
		"key_red_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/red_key/tris.md2", EF_ROTATE,
		NULL,
		"k_redkey",
		"Red Key",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_green_key (0 .5 .8) (-16 -16 -16) (16 16 16)
	 * normal door key - blue
	 */
	{
		"key_green_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/green_key/tris.md2", EF_ROTATE,
		NULL,
		"k_green",
		"Green Key",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 * tank commander's head
	 */
	{
		"key_commander_head",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/monsters/commandr/head/tris.md2", EF_GIB,
		NULL,
		"k_comhead",
		"Commander's Head",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 * tank commander's head
	 */
	{
		"key_airstrike_target",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/target/tris.md2", EF_ROTATE,
		NULL,
		"i_airstrike",
		"Airstrike Marker",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED key_nuke_container (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"key_nuke_container",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/weapons/g_nuke/tris.md2",
		EF_ROTATE,
		NULL,
		"i_contain",
		"Antimatter Pod",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		NULL,
	},

	/*
	 * QUAKED key_nuke (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"key_nuke",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/weapons/g_nuke/tris.md2",
		EF_ROTATE,
		NULL,
		"i_nuke",
		"Antimatter Bomb",
		2,
		0,
		NULL,
		IT_STAY_COOP | IT_KEY,
		0,
		NULL,
		0,
		NULL,
	},

	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0,
		NULL,
		"i_health",
		"Health",
		3,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
		"items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},

	/*
	 * QUAKED item_flag_team1 (1 0.2 0) (-16 -16 -24) (16 16 32)
	 */
	{
		"item_flag_team1",
		CTFPickup_Flag,
		NULL,
		CTFDrop_Flag,
		NULL,
		"ctf/flagtk.wav",
		"players/male/flag1.md2", EF_FLAG1,
		NULL,
		"i_ctf1",
		"Red Flag",
		2,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
		"ctf/flagcap.wav"
	},

	/*
	 * QUAKED item_flag_team2 (1 0.2 0) (-16 -16 -24) (16 16 32)
	 */
	{
		"item_flag_team2",
		CTFPickup_Flag,
		NULL,
		CTFDrop_Flag,
		NULL,
		"ctf/flagtk.wav",
		"players/male/flag2.md2", EF_FLAG2,
		NULL,
		"i_ctf2",
		"Blue Flag",
		2,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
		"ctf/flagcap.wav"
	},

	/* Resistance Tech */
	{
		"item_tech1",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech,
		NULL,
		"items/pkup.wav",
		"models/ctf/resistance/tris.md2", EF_ROTATE,
		NULL,
		"tech1",
		"Disruptor Shield",
		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
		"ctf/tech1.wav"
	},

	/* Strength Tech */
	{
		"item_tech2",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech,
		NULL,
		"items/pkup.wav",
		"models/ctf/strength/tris.md2", EF_ROTATE,
		NULL,
		"tech2",
		"Power Amplifier",
		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
		"ctf/tech2.wav ctf/tech2x.wav"
	},

	/* Haste Tech */
	{
		"item_tech3",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech,
		NULL,
		"items/pkup.wav",
		"models/ctf/haste/tris.md2", EF_ROTATE,
		NULL,
		"tech3",
		"Time Accel",
		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
		"ctf/tech3.wav"
	},

	/* Regeneration Tech */
	{
		"item_tech4",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech,
		NULL,
		"items/pkup.wav",
		"models/ctf/regeneration/tris.md2", EF_ROTATE,
		NULL,
		"tech4",
		"AutoDoc",
		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
		"ctf/tech4.wav"
	},

	{
		"item_flashlight",
		Pickup_General,
		Use_Flashlight,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/flashlight/tris.md2", EF_ROTATE,
		NULL,
		"p_torch",
		"Flashlight",
		2,
		0,
		NULL,
		IT_STAY_COOP,
		0,
		NULL,
		0,
		"items/flashlight_on.wav items/flashlight_off.wav",
	},

	/* end of list marker */
	{NULL}
};

gitem_t itemlist[MAX_ITEMS];

/*
 * QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
 */
void
SP_item_health(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH))
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem(self, FindItem("Health"));
	gi.soundindex("items/n_health.wav");
}

/*
 * QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
 */
void
SP_item_health_small(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH))
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/items/healing/stimpack/tris.md2";
	self->count = 2;
	SpawnItem(self, FindItem("Health"));
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex("items/s_health.wav");
}

/*
 * QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
 */
void
SP_item_health_large(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH))
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
	SpawnItem(self, FindItem("Health"));
	gi.soundindex("items/l_health.wav");
}

/*
 * QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
 */
void
SP_item_health_mega(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH))
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
	SpawnItem(self, FindItem("Health"));
	gi.soundindex("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX | HEALTH_TIMED;
}

void
SP_item_foodcube(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH))
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/objects/trapfx/tris.md2";
	SpawnItem(self, FindItem("Health"));
	self->spawnflags |= DROPPED_ITEM;
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex("items/s_health.wav");
	self->classname = "foodcube";
}

void
InitItems(void)
{
	if (sizeof(gameitemlist) > sizeof(itemlist))
	{
		gi.error("Defined items more than %d\n", MAX_ITEMS);
	}

	memset(itemlist, 0, sizeof(itemlist));
	memcpy(itemlist, gameitemlist, sizeof(gameitemlist));
	game.num_items = sizeof(gameitemlist) / sizeof(gameitemlist[0]) - 1;
}

/*
 * Called by worldspawn
 */
void
SetItemNames(void)
{
	int i;
	gitem_t *it;

	for (i = 0; i < game.num_items; i++)
	{
		it = &itemlist[i];
		gi.configstring(CS_ITEMS + i, it->pickup_name);
	}

	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
	body_armor_index = ITEM_INDEX(FindItem("Body Armor"));
	power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
	power_shield_index = ITEM_INDEX(FindItem("Power Shield"));
}

void
SP_xatrix_item(edict_t *self)
{
	gitem_t *item;
	int i;
	char *spawnClass = NULL;

	if (!self)
	{
		return;
	}

	if (!self->classname)
	{
		return;
	}

	if (!strcmp(self->classname, "ammo_magslug"))
	{
		spawnClass = "ammo_flechettes";
	}
	else if (!strcmp(self->classname, "ammo_trap"))
	{
		spawnClass = "weapon_proxlauncher";
	}
	else if (!strcmp(self->classname, "item_quadfire"))
	{
		float chance;

		chance = random();

		if (chance < 0.2)
		{
			spawnClass = "item_sphere_hunter";
		}
		else if (chance < 0.6)
		{
			spawnClass = "item_sphere_vengeance";
		}
		else
		{
			spawnClass = "item_sphere_defender";
		}
	}
	else if (!strcmp(self->classname, "weapon_boomer"))
	{
		spawnClass = "weapon_etf_rifle";
	}
	else if (!strcmp(self->classname, "weapon_phalanx"))
	{
		spawnClass = "weapon_plasmabeam";
	}

	if (!spawnClass)
	{
		return;
	}

	/* check item spawn functions */
	for (i = 0, item = itemlist; i < game.num_items; i++, item++)
	{
		if (!item->classname)
		{
			continue;
		}

		if (!strcmp(item->classname, spawnClass))
		{
			/* found it */
			SpawnItem(self, item);
			return;
		}
	}
}
