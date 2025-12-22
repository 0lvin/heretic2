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

static gitem_t *
FindItemInList(const char *classname, gitem_t *list, int count)
{
	int i;
	gitem_t *it;

	if (!classname)
	{
		return NULL;
	}

	it = list;

	for (i = 0; i < count; i++, it++)
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
FindItemByClassname(const char *classname)
{
	return FindItemInList(classname, itemlist, game.num_items);
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

	if (!ent)
	{
		return;
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	gi.linkentity(ent);

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

	if (deathmatch->value && game.num_clients > 8)
	{
		// No less than 1/4th the delay.

		ent->nextthink = level.time + (ent->delay) / 4;
	}
	if (deathmatch->value && game.num_clients > 2)
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
	if (other->flags & FL_CHICKEN)
	{
		return false;
	}

	if (!other->client->pers.inventory[ITEM_INDEX(ent->item)])
	{
		other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;

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
	if (!player->client->pers.inventory[ITEM_INDEX(item)])
	{
		player->client->pers.inventory[ITEM_INDEX(item)]=1;

		// Now decide if we want to swap defenses or not.

		if (player->client->pers.autoweapon )
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

	if (AddDefenseToInventory(ent->item,other))
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

	if (ent->client->pers.inventory[index] == max)
		return false;

	ent->client->pers.inventory[index] += count;

	if (ent->client->pers.inventory[index] > max)
		ent->client->pers.inventory[index] = max;

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
		max = ent->client->pers.max_offmana;
		return Add_AmmoToInventory(ent, item, count, max);
	}
	else if ((item->tag == AMMO_MANA_DEFENSIVE_HALF) || (item->tag == AMMO_MANA_DEFENSIVE_FULL))
	{
		item = FindItemByClassname("item_mana_defensive_half");
		max = ent->client->pers.max_defmana;
		return Add_AmmoToInventory(ent, item, count, max);
	}
	else if ((item->tag == AMMO_MANA_COMBO_QUARTER) || (item->tag == AMMO_MANA_COMBO_HALF))
	{
		qboolean bo;

		item = FindItemByClassname("item_mana_offensive_half");
		max = ent->client->pers.max_offmana;

		bo = Add_AmmoToInventory (ent,item,count,max);

		item = FindItemByClassname("item_mana_defensive_half");
		max = ent->client->pers.max_defmana;
		bo |= Add_AmmoToInventory(ent, item, count, max);

		return bo;
	}
	else if (item->tag == AMMO_REDRAIN)
	{
		max = ent->client->pers.max_redarrow;
		return Add_AmmoToInventory(ent, item, count, max);
	}
	else if (item->tag == AMMO_PHOENIX)
	{
		max = ent->client->pers.max_phoenarr;
		return Add_AmmoToInventory(ent, item, count, max);
	}
	else if (item->tag == AMMO_HELLSTAFF)
	{
		max = ent->client->pers.max_hellstaff;
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

	if (ent->client->pers.inventory[index] >= item->quantity)
	{
		dropped->count = item->quantity;
	}
	else
	{
		dropped->count = ent->client->pers.inventory[index];
	}

	ent->client->pers.inventory[index] -= dropped->count;

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

	if (other->fire_damage_time>level.time)
	{
		other->fire_damage_time -= ent->item->quantity/10;
		if (other->fire_damage_time<=0)
		{
			other->fire_damage_time = 0;
//			G_RemoveEffects(other, FX_FIRE_ON_ENTITY);//turn off CFX too
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

	if (other->client)
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
	else if (!old_armor_index) /* if player has no armor, just use it */
	{
		other->client->pers.inventory[ITEM_INDEX(ent->item)] =
			newinfo->base_count;
	}
	else /* use the better armor */
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
		else
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

	if (strcmp(other->classname,"player"))
	{
		// Only players can touch items.

		return;
	}

	if (other->health < 1)
	{
		// Dead players can't pickup.

		return;
	}

	if (!ent->item->pickup)
	{
		// Not a grabbable item.

		return;
	}

	assert(ent->item->pickup);

	if ((other->flags & FL_CHICKEN) && (ent->item->pickup == Pickup_Health))
	{
		// chickens can't pickup health

		return;
	}

		if (!ent->item->pickup(ent, other))
	{
		// Player can't hold it.

		return;
	}

	gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);

	gi.CreateEffect(NULL, FX_PICKUP, 0, ent->s.origin, "");

	G_UseTargets(ent, other);

	// Handle respawn / removal of the item.

	if (((ent->item->pickup==Pickup_Weapon)||(ent->item->pickup==Pickup_Defense)||(ent->item->pickup==Pickup_Puzzle))&&
	   ((deathmatch->value&&((int)dmflags->value&DF_WEAPONS_STAY))||coop->value))
	{
		// The item is a weapon or a defence or a puzzle piece AND (deathmatch rule DF_WEAPONS_STAY
		// is on OR we are playing coop), so just return right now, as we don't care about respawn
		// or removal.

		return;
	}

	if (ent->flags & FL_RESPAWN)
	{
		// The item should respawn.

		SetRespawn(ent, ent->item->quantity);
	}
	else
	{
		// Going away for good, so make it noclipping.

		ent->solid = SOLID_NOT;

		// Once picked up, the item is gone forever, so remove it's client effect(s).

		G_RemoveEffects(ent, FX_REMOVE_EFFECTS);

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
		dropped->s.angles[YAW] += frandk()*45;
	}
	else
	{
		dropped->s.angles[YAW] -= frandk()*45;
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
			vec3_t pos, diff;
			trace_t tr_pos;
			int k;

			VectorCopy(ent->s.origin, pos);

			VectorSubtract(ent->maxs, ent->mins, diff);

			/* move by up */
			for (k = 0; k < i + 1; k++)
			{
				int v;

				v = (j + k) % 3;
				pos[v] = ent->s.origin[v] + diff[v];
			}

			tr_pos = gi.trace(pos, ent->mins, ent->maxs, ent->s.origin, ent, MASK_SOLID);
			if (!tr_pos.startsolid)
			{
				VectorCopy(tr_pos.endpos, ent->s.origin);
				return;
			}

			/* move by down */
			for (k = 0; k < i + 1; k++)
			{
				int v;

				v = (j + k) % 3;
				pos[v] = ent->s.origin[v] - diff[v];
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

	if (deathmatch->value)
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

	if (!ValidItem(item))
	{
		G_FreeEdict(ent);
		return;
	}

	assert(!ent->PersistantCFX);

	if ((ent->spawnflags & ITEM_COOP_ONLY) && (!coop->value))
	{
		return;
	}

	if (ent->item->tag == AMMO_HELLSTAFF ||
		ent->item->tag == AMMO_REDRAIN ||
		ent->item->tag == AMMO_PHOENIX
	)
	{
		return;
	}

	if (ent->item->flags & IT_WEAPON)
	{
		ent->PersistantCFX = gi.CreatePersistantEffect(ent, FX_PICKUP_WEAPON, CEF_BROADCAST, ent->s.origin, "b", ent->item->tag);
	}
	else if (ent->item->flags & IT_AMMO)
	{
		ent->PersistantCFX = gi.CreatePersistantEffect(ent, FX_PICKUP_AMMO, CEF_BROADCAST, ent->s.origin, "b", ent->item->tag);
	}
	else if (ent->item->flags & IT_DEFENSE)
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
			return;
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

	if (g_itemsbobeffect->value && (item->world_model_flags & EF_ROTATE))
	{
		item->world_model_flags |= EF_BOB;
	}

	if (!g_disruptor->value)
	{
		if ((!strcmp(ent->classname, "ammo_disruptor")) ||
				(!strcmp(ent->classname, "weapon_disintegrator")))
		{
			G_FreeEdict(ent);
			return;
		}
	}

	if (ent->spawnflags > 1)
	{
		if (strcmp(ent->classname, "key_power_cube") != 0)
		{
			ent->spawnflags = 0;
			gi.dprintf("%s at %s has invalid spawnflags set\n",
					ent->classname, vtos(ent->s.origin));
		}
	}

	/* some items will be prevented in deathmatch */
	if (deathmatch->value)
	{
		if ((int)dmflags->value & DF_NO_ARMOR)
		{
			if ((item->pickup == Pickup_Armor) ||
				(item->pickup == Pickup_PowerArmor))
			{
				G_FreeEdict(ent);
				return;
			}
		}

		if ((int)dmflags->value & DF_NO_ITEMS)
		{
			if (item->pickup == Pickup_Powerup)
			{
				G_FreeEdict(ent);
				return;
			}

			if (item->pickup == Pickup_Sphere)
			{
				G_FreeEdict(ent);
				return;
			}

			if (item->pickup == Pickup_Doppleganger)
			{
				G_FreeEdict(ent);
				return;
			}
		}

		if ((int)dmflags->value & DF_NO_HEALTH)
		{
			if ((item->pickup == Pickup_Health) ||
				(item->pickup == Pickup_Adrenaline) ||
				(item->pickup == Pickup_AncientHead))
			{
				G_FreeEdict(ent);
				return;
			}
		}

		if ((int)dmflags->value & DF_INFINITE_AMMO)
		{
			if ((item->flags == IT_AMMO) ||
				(strcmp(ent->classname, "weapon_bfg") == 0))
			{
				G_FreeEdict(ent);
				return;
			}
		}

		if ((int)dmflags->value & DF_NO_MINES)
		{
			if (!strcmp(ent->classname, "ammo_prox") ||
				!strcmp(ent->classname, "ammo_tesla"))
			{
				G_FreeEdict(ent);
				return;
			}
		}

		if ((int)dmflags->value & DF_NO_NUKES)
		{
			if (!strcmp(ent->classname, "ammo_nuke"))
			{
				G_FreeEdict(ent);
				return;
			}
		}

		if ((int)dmflags->value & DF_NO_SPHERES)
		{
			if (item->pickup == Pickup_Sphere)
			{
				G_FreeEdict(ent);
				return;
			}
		}
	}

	/* DM only items */
	if (!deathmatch->value)
	{
		if ((item->pickup == Pickup_Doppleganger) ||
			(item->pickup == Pickup_Nuke))
		{
			G_FreeEdict(ent);
			return;
		}

		if ((item->use == Use_Vengeance) || (item->use == Use_Hunter))
		{
			G_FreeEdict(ent);
			return;
		}
	}

	if ((ent->spawnflags & ITEM_COOP_ONLY) && (!coop->value))
	{
		return;
	}

	PrecacheItem(item);

	if (coop->value && !(ent->spawnflags & ITEM_NO_TOUCH) && (strcmp(ent->classname, "key_power_cube") == 0))
	{
		ent->spawnflags |= (1 << (8 + level.power_cubes));
		level.power_cubes++;
	}

	/* don't let them drop items that stay in a coop game */
	if ((coop->value) && (item->flags & IT_STAY_COOP))
	{
		item->drop = NULL;
	}

	/* Don't spawn the flags unless enabled */
	if (!ctf->value && ((strcmp(ent->classname, "item_flag_team1") == 0) ||
				(strcmp(ent->classname, "item_flag_team2") == 0)))
	{
		G_FreeEdict(ent);
		return;
	}

	if (!ValidItem(item))
	{
		G_FreeEdict(ent);
		return;
	}

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME; /* items start after other solids */
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW | RF_TRANSLUCENT;
	ent->s.effects |= EF_ALWAYS_ADD_EFFECTS;

	if (item->tag == AMMO_MANA_DEFENSIVE_HALF ||
		item->tag == AMMO_MANA_DEFENSIVE_FULL ||
		item->tag == ITEM_TAVERNKEY ||
		item->tag == ITEM_CANKEY ||
		item->tag == ITEM_WEAPON_PHOENIXBOW)
	{
		ent->s.skinnum = 1;
	}

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
	if (deathmatch->value)
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
		Pickup_Weapon,							// Pickup (f)
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
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_SWORDSTAFF,					// tag ?
		NULL,									// precaches
		0,										// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WSWORD_STD1,						// Player animation sequence to engage when used
		ASEQ_WSWORD_STD1,						// Player animation sequence to engage when powered
	},

	/*
	 * QUAKED weapon_flyingfist
	 * This can't be placed in the editor
	 */
	{
		"Weapon_FlyingFist",					// Spawnname
		Pickup_Weapon,							// Pickup (f)
		Weapon_EquipSpell,						// Use (f)
		NULL,									// Drop	(f)
		WeaponThink_FlyingFist,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		NULL,									// view model
		"icons/i_fball.m8",						// Icon name (char *)
		"fball",								// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_USE_FIREBALL,						// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_FLYINGFIST,					// tag ?
		NULL,									// precaches
		0,										// pickup message
		GM_NOFLYINGFIST,						// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WFIREBALL,							// Player animation sequence to engage when used
		ASEQ_WFIREBALL,							// Player animation sequence to engage when powered
	},

	/*
	 * QUAKED item_weapon_hellstaff (.3 .3 1) (-16 -16 -16) (16 16 16) COOP_ONLY
	 * Pickup for the hellstaff weapon.
	 */
	{
		"item_weapon_hellstaff",				// Spawnname
		Pickup_Weapon,							// Pickup (f)
		Weapon_EquipHellStaff,					// Use (f)
		Drop_Weapon,							// Drop	(f)
		WeaponThink_HellStaff,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/i_hell.m8",						// Icon name (char *)
		"hell",									// Pickup name (char *)
		2,										// Number of digits to display
		AMMO_USE_HELLSTAFF,						// Ammo/ammo use per shot
		"Hell-staff-ammo",						// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_HELLSTAFF,					// tag ?
		NULL,									// precaches
		GM_HELLSTAFF,							// pickup message
		GM_NOHELLORBS,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WHELL_GO,							// Player animation sequence to engage when used
		ASEQ_WHELL_GO,							// Player animation sequence to engage when powered
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
		EF_ROTATE | EF_BOB,						// world model flags
		NULL,									// view model
		"icons/i_array.m8",						// Icon name (char *)
		"array",								// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_USE_MAGICMISSILE,					// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_MAGICMISSILE,				// tag ?
		NULL,									// precaches
		GM_FORCEBLAST,							// pickup message
		GM_NOFORCE,								// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WBLAST,							// Player animation sequence to engage when used
		ASEQ_WARRAY,							// Player animation sequence to engage when powered
	},

	/*
	 * QUAKED item_weapon_redrain_bow (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
	 * Pickup for the Red Rain Bow weapon.
	 */
	{
		"item_weapon_redrain_bow",				// Spawnname
		Pickup_Weapon,							// Pickup (f)
		Weapon_EquipBow,						// Use (f)
		Drop_Weapon,							// Drop	(f)
		WeaponThink_RedRainBow,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/i_rain.m8",						// Icon name (char *)
		"rain",									// Pickup name (char *)
		2,										// Number of digits to display
		AMMO_USE_REDRAIN,						// Ammo/ammo use per shot
		"Red-Rain-Arrows",						// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_REDRAINBOW,					// tag ?
		NULL,									// precaches
		GM_STORMBOW,							// pickup message
		GM_NOSTORMBOW,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WRRBOW_GO,							// Player animation sequence to engage when used
		ASEQ_WRRBOW_GO,							// Player animation sequence to engage when powered
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
		EF_ROTATE | EF_BOB,						// world model flags
		NULL,									// view model
		"icons/i_fwall.m8",						// Icon name (char *)
		"fwall",								// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_USE_FIREWALL,						// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_FIREWALL,					// tag ?
		NULL,									// precaches
		GM_FIREWALL,							// pickup message
		GM_NOFIREWALL,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WFIREWALL,							// Player animation sequence to engage when used
		ASEQ_WFIREWALL,							// Player animation sequence to engage when powered
	},

	/*
	 * QUAKED item_weapon_phoenixbow (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
	 * Pickup for the Phoenix Bow weapon.
	 */
	{
		"item_weapon_phoenixbow",				// Spawnname
		Pickup_Weapon,							// Pickup (f)
		Weapon_EquipBow,						// Use (f)
		Drop_Weapon,							// Drop	(f)
		WeaponThink_PhoenixBow,					// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/i_phoen.m8",						// Icon name (char *)
		"phoen",								// Pickup name (char *)
		2,										// Number of digits to display
		AMMO_USE_PHOENIX,						// Ammo/ammo use per shot
		"Phoenix-Arrows",						// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_PHOENIXBOW,					// tag ?
		NULL,									// precaches
		GM_PHOENIX,								// pickup message
		GM_NOPHOENIX,							// can`t use message
		{0, 0, 0},								// Bounding box mins
		{0, 0, 0},								// Bounding box maxs
		ASEQ_WPHBOW_GO,							// Player animation sequence to engage when used
		ASEQ_WPHBOW_GO,							// Player animation sequence to engage when powered
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
		EF_ROTATE | EF_BOB,						// world model flags
		NULL,									// view model
		"icons/i_sphere.m8",					// Icon name (char *)
		"sphere",								// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_USE_SPHERE,						// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_SPHEREOFANNIHILATION,		// tag ?
		NULL,									// precaches
		GM_SPHERE,								// pickup message
		GM_NOSPHERE,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WSPHERE_GO,						// Player animation sequence to engage when used
		ASEQ_WSPHERE_GO,						// Player animation sequence to engage when powered
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
		EF_ROTATE | EF_BOB,						// world model flags
		NULL,									// view model
		"icons/i_mace.m8",						// Icon name (char *)
		"mace",									// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_USE_MACEBALL,						// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_WEAPON_MACEBALLS,					// tag ?
		NULL,									// precaches
		GM_IRONDOOM,							// pickup message
		GM_NOIRONDOOM,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WRIPPER,							// Player animation sequence to engage when used
		ASEQ_WBIGBALL,							// Player animation sequence to engage when powered
	},

	/*
	 * QUAKED item_defense_powerup (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
	 * This can't be placed in the editor, default defense on client startup
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
		"icons/i_tome.m8",						// Icon name (char *)
		"powerup",								// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_POWERUP,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_POWERUP,					// tag ?
		NULL,									// precaches
		GM_TOME,								// pickup message
		GM_NOTOME,								// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
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
		EF_ROTATE | EF_BOB,						// world model flags
		NULL,									// view model
		"icons/i_ring.m8",						// Icon name (char *)
		"ring",									// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_RING,							// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_REPULSION,					// tag ?
		NULL,									// precaches
		GM_RING,								// pickup message
		GM_NORING,								// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
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
		EF_ROTATE | EF_BOB,						// world model flags
		NULL,									// view model
		"icons/i_shield.m8",					// Icon name (char *)
		"lshield",								// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_SHIELD,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_SHIELD,					// tag ?
		NULL,									// precaches
		GM_SHIELD,								// pickup message
		GM_NOSHIELD,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
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
		EF_ROTATE | EF_BOB,						// world model flags
		NULL,									// view model
		"icons/i_tele.m8",						// Icon name (char *)
		"tele",									// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_TELEPORT,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_TELEPORT,					// tag ?
		NULL,									// precaches
		GM_TELEPORT,							// pickup message
		GM_NOTELEPORT,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
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
		EF_ROTATE | EF_BOB,						// world model flags
		NULL,									// view model
		"icons/i_morph.m8",						// Icon name (char *)
		"morph",								// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_POLYMORPH,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_POLYMORPH,					// tag ?
		NULL,									// precaches
		GM_MORPH,								// pickup message
		GM_NOMORPH,								// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
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
		EF_ROTATE | EF_BOB,						// world model flags
		NULL,									// view model
		"icons/i_meteor.m8",					// Icon name (char *)
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
	},

	// =============================================================================================

	// Ammo items.

	/*
	 * QUAKED item_mana_offensive_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the offensive mana (50 points).
	 */
	{
		"item_mana_offensive_half",				// Spawnname
		Pickup_Ammo,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		NULL,									// Icon name (char *)
		"Off-mana",								// Pickup name (char *)
		0,										// Number of digits to display
		HALF_OFF_MANA,							// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		AMMO_MANA_OFFENSIVE_HALF,				// tag ?
		NULL,									// precaches
		GM_OFFMANAS,							// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_mana_offensive_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the offensive mana (100 points).
	 */
	{
		"item_mana_offensive_full",				// Spawnname
		Pickup_Ammo,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		NULL,									// Icon name (char *)
		"Off-mana",								// Pickup name (char *)
		0,										// Number of digits to display
		FULL_OFF_MANA,							// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		AMMO_MANA_OFFENSIVE_FULL,				// tag ?
		NULL,									// precaches
		GM_OFFMANAB,							// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_mana_defensive_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the defensive mana (50 points).
	 */
	{
		"item_mana_defensive_half",				// Spawnname
		Pickup_Ammo,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		NULL,									// Icon name (char *)
		"Def-mana",								// Pickup name (char *)
		0,										// Number of digits to display
		HALF_DEF_MANA,							// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_DEFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		AMMO_MANA_DEFENSIVE_HALF,				// tag ?
		NULL,									// precaches
		GM_DEFMANAS,							// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_mana_defensive_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for the defensive mana (100 points).
	 */
	{
		"item_mana_defensive_full",				// Spawnname
		Pickup_Ammo,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		NULL,									// Icon name (char *)
		"Def-mana",								// Pickup name (char *)
		0,										// Number of digits to display
		FULL_DEF_MANA,							// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_DEFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		AMMO_MANA_DEFENSIVE_FULL,				// tag ?
		NULL,									// precaches
		GM_DEFMANAB,							// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_mana_combo_quarter (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for both defensive & offensive mana (25 points).
	 */
	{
		"item_mana_combo_quarter",				// Spawnname
		Pickup_Ammo,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		NULL,									// Icon name (char *)
		"Def-mana",								// Pickup name (char *)
		0,										// Number of digits to display
		HALF_COMBO_MANA,						// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		AMMO_MANA_COMBO_QUARTER,				// tag ?
		NULL,									// precaches
		GM_COMBMANAS,							// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_mana_combo_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup for both defensive & offensive mana (50 points).
	 */
	{
		"item_mana_combo_half",					// Spawnname
		Pickup_Ammo,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		NULL,									// Icon name (char *)
		"Def-mana",								// Pickup name (char *)
		0,										// Number of digits to display
		FULL_COMBO_MANA,						// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		AMMO_MANA_COMBO_HALF,					// tag ?
		NULL,									// precaches
		GM_COMBMANAB,							// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_ammo_redrain (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup ammo for the Red Rain Bow
	 */
	{
		"item_ammo_redrain",					// Spawnname
		Pickup_Ammo,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/i_ammo-redrain.m8",				// Icon name (char *)
		"Red-Rain-Arrows",						// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_COUNT_REDRAINBOW,					// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		AMMO_REDRAIN,							// tag ?
		NULL,									// precaches
		GM_STORMARROWS,							// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_ammo_phoenix (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup ammo for the Phoenix Bow
	 */
	{
		"item_ammo_phoenix",					// Spawnname
		Pickup_Ammo,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/i_ammo-phoen.m8",				// Icon name (char *)
		"Phoenix-Arrows",						// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_COUNT_PHOENIXBOW,					// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		AMMO_PHOENIX,							// tag ?
		NULL,									// precaches
		GM_PHOENARROWS,							// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_ammo_hellstaff (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup ammo for the Hellstaff
	 */
	{
		"item_ammo_hellstaff",					// Spawnname
		Pickup_Ammo,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/i_ammo-hellstaff.m8",			// Icon name (char *)
		"Hell-staff-ammo",						// Pickup name (char *)
		0,										// Number of digits to display
		AMMO_COUNT_HELLSTAFF,					// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_OFFENSE,					// flags
		0,										// weapon model index
		NULL,									// void * ?
		AMMO_HELLSTAFF,							// tag ?
		NULL,									// precaches
		GM_HELLORB,								// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	// ============================================================================================

	// Other items.

	/*
	 * QUAKED item_health_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup health (10 points)
	 */
	{
		"item_health_half",						// Spawnname
		Pickup_Health,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"*gethealth.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		NULL,									// Icon name (char *)
		"Minor health",							// Pickup name (char *)
		0,										// Number of digits to display
		10,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_HEALTH | EF_ALWAYS_ADD_EFFECTS,		// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_HEALTH1,							// tag ?
		NULL,									// precaches
		GM_HEALTHVIAL,							// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_health_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
	 * Pickup health (30 points)
	 */
	{
		"item_health_full",						// Spawnname
		Pickup_Health,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"*gethealth.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		NULL,									// Icon name (char *)
		"Major health",							// Pickup name (char *)
		0,										// Number of digits to display
		30,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_HEALTH | EF_ALWAYS_ADD_EFFECTS,		// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_HEALTH2,							// tag ?
		NULL,									// precaches
		GM_HEALTHPOTION,						// pickup message
		0,										// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
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
		"item_puzzle_townkey",					// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_townkey.m8",					// Icon name (char *)
		"Town Key",								// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_TOWNKEY,							// tag ?
		NULL,									// precaches
		GM_F_TOWNKEY,							// pickup message
		GM_NEED_TOWNKEY,						// can`t use message
		{-8, -8, -4},							// Bounding box mins
		{8,  8,  4},							// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_cog (.3 .3 1) (-10 -10 -24) (10 10 20)  x  NO_DROP
	 * Cog puzzle piece
	 * Palace level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_cog",						// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_cog.m8",						// Icon name (char *)
		"Cog",									// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_COG,								// tag ?
		NULL,									// precaches
		GM_F_COG,								// pickup message
		GM_NEED_COG,							// can`t use message
		{-10, -10, -24},						// Bounding box mins
		{10,  10,  20},							// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_shield (.3 .3 1) (-2 -6 -12) (2 6 12)  x  NO_DROP
	 * Sithra Shield puzzle item
	 * Healer Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_shield",					// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_shield.m8",					// Icon name (char *)
		"Defensive Shield",						// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_SHIELD,							// tag ?
		NULL,									// precaches
		GM_F_SHIELD,							// pickup message
		GM_NEED_SHIELD,							// can`t use message
		{-2, -6, -12},							// Bounding box mins
		{2,  6,  12},							// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_potion (.3 .3 1) (-3 -3 -10) (3 3 10)  x  NO_DROP
	 * Potion puzzle item
	 * Healer Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_potion",					// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_potion.m8",					// Icon name (char *)
		"Potion",								// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_POTION,							// tag ?
		NULL,									// precaches
		GM_F_POTION,							// pickup message
		GM_NEED_POTION,							// can`t use message
		{-3, -3, -10},							// Bounding box mins
		{3,  3,  10},							// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_plazacontainer (.3 .3 1) (-6 -6 -8) (6 6 6)  x  NO_DROP
	 * Container puzzle item
	 * Plaza Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_plazacontainer",			// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_plazajug.m8",					// Icon name (char *)
		"Container",							// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_CONT,								// tag ?
		NULL,									// precaches
		GM_F_CONT,								// pickup message
		GM_NEED_CONT,							// can`t use message
		{-6, -6, -8},							// Bounding box mins
		{6,  6,  6},							// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_slumcontainer (.3 .3 1) (-6 -6 -8) (6 6 6)  x  NO_DROP
	 * Full Container puzzle item
	 * Slum Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_slumcontainer",			// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_jugfill.m8",					// Icon name (char *)
		"Full Container",						// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_SLUMCONT,							// tag ?
		NULL,									// precaches
		GM_F_CONTFULL,							// pickup message
		GM_NEED_CONTFULL,						// can`t use message
		{-6, -6, -8},							// Bounding box mins
		{6,  6,  6},							// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_crystal (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
	 * Crystal puzzle item
	 * Academic Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_crystal",					// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_crystal.m8",					// Icon name (char *)
		"Crystal",								// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_CRYSTAL,							// tag ?
		NULL,									// precaches
		GM_F_CRYSTAL,							// pickup message
		GM_NEED_CRYSTAL,						// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_canyonkey (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
	 * Key puzzle item
	 * Canyon Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_canyonkey",				// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_canyonkey.m8",					// Icon name (char *)
		"Canyon Key",							// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_CANKEY,							// tag ?
		NULL,									// precaches
		GM_F_CANYONKEY,							// pickup message
		GM_NEED_CANYONKEY,						// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_hive2amulet (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
	 * Amulet puzzle item
	 * Hive 2 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_hive2amulet",				// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_tcheckrikbust.m8",				// Icon name (char *)
		"Hive 2 Amulet",						// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_AMULET,							// tag ?
		NULL,									// precaches
		GM_F_AMULET,							// pickup message
		GM_NEED_AMULET,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_hive2spear (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
	 * Spear puzzle item
	 * Hive 2 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_hive2spear",				// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_spear.m8",						// Icon name (char *)
		"Hive 2 Spear",							// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_SPEAR,								// tag ?
		NULL,									// precaches
		GM_F_SPEAR,								// pickup message
		GM_NEED_SPEAR,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_hive2gem (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
	 * Gem puzzle item
	 * Hive 2 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_hive2gem",					// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_tcheckrikgem.m8",				// Icon name (char *)
		"Hive 2 Gem",							// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_GEM,								// tag ?
		NULL,									// precaches
		GM_F_GEM,								// pickup message
		GM_NEED_GEM,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_minecartwheel (.3 .3 1) (-1 -6 -6) (1 6 6)  x  NO_DROP
	 * Mine Cart Wheel puzzle item
	 * Mine 1 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_minecartwheel",			// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_wheel.m8",						// Icon name (char *)
		"Minecart Wheel",						// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_WHEEL,								// tag ?
		NULL,									// precaches
		GM_F_CARTWHEEL,							// pickup message
		GM_NEED_CARTWHEEL,						// can`t use message
		{-1,-6,-6},								// Bounding box mins
		{1, 6, 6},								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_ore (.3 .3 1) (-10 -10 -8) (10 10 8)  x  NO_DROP
	 * Unrefined Ore puzzle item
	 * Mine 2 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_ore",						// Spawnname
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_oreunrefined.m8",				// Icon name (char *)
		"Ore",									// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_ORE,								// tag ?
		NULL,									// precaches
		GM_F_UNREFORE,							// pickup message
		GM_NEED_UNREFORE,						// can`t use message
		{-10,-10,-8},							// Bounding box mins
		{10, 10, 8},							// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_refinedore (.3 .3 1) (-3 -12 -2) (3 12 2) x   NO_DROP
	 * Refined Ore puzzle item
	 * Mine 2 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_refinedore",				// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_orerefined.m8",				// Icon name (char *)
		"Refined Ore",							// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_REF_ORE,							// tag ?
		NULL,									// precaches
		GM_F_REFORE,							// pickup message
		GM_NEED_REFORE,							// can`t use message
		{-3,-12,-2},							// Bounding box mins
		{3, 12, 2},								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_dungeonkey (.3 .3 1) (-1 -18 -9) (1 18 9)  x  NO_DROP
	 * Amulet puzzle item
	 * Dungeon Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_dungeonkey",				// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_dungeonkey.m8",				// Icon name (char *)
		"Dungeon Key",							// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_DUNKEY,							// tag ?
		NULL,									// precaches
		GM_F_DUNGEONKEY,						// pickup message
		GM_NEED_DUNGEONKEY,						// can`t use message
		{-1,-18,-9},							// Bounding box mins
		{1, 18, 9},								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_cloudkey (.3 .3 1) (-8 -8 -3) (8 8 6)  x  NO_DROP
	 * Key puzzle item
	 * Cloud Quarters 2 Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_cloudkey",					// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_cloudkey.m8",					// Icon name (char *)
		"Cloud Key",							// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_CLOUDKEY,							// tag ?
		NULL,									// precaches
		GM_F_CLOUDKEY,							// pickup message
		GM_NEED_CLOUDKEY,						// can`t use message
		{-8, -8, -3},							// Bounding box mins
		{8,  8,  3},							// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_highpriestesskey (.3 .3 1) (-12 -12 -6) (12 12 6) x   NO_DROP
	 * Key puzzle item
	 * High Priestess Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_highpriestesskey",			// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_hivekey.m8",					// Icon name (char *)
		"Key",									// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_HIVEKEY,							// tag ?
		NULL,									// precaches
		GM_F_HIGHKEY,							// pickup message
		GM_NEED_HIGHKEY,						// can`t use message
		{-12,-12, -6},							// Bounding box mins
		{12, 12,  6},							// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_highpriestesssymbol (.3 .3 1) (-12 -12 -4) (12 12 4) x   NO_DROP
	 * Key puzzle item
	 * High Priestess Level
	 * NO_DROP - won't drop to ground
	*/
	{
		"item_puzzle_highpriestesssymbol",		// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_queenkey.m8",					// Icon name (char *)
		"Symbol",								// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_HPSYM,								// tag ?
		NULL,									// precaches
		GM_F_SYMBOL,							// pickup message
		GM_NEED_SYMBOL,							// can`t use message
		{-12,-12,-4},							// Bounding box mins
		{12, 12, 4},							// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_tome (.3 .3 1) (-12 -12 -4) (12 12 4)  x  NO_DROP
	 * Tome puzzle piece
	 * 2 Cloud Levels
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_tome",						// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_tomepower.m8",					// Icon name (char *)
		"Tome",									// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_TOME,								// tag ?
		NULL,									// precaches
		GM_F_TOME,								// pickup message
		GM_NEED_TOME,							// can`t use message
		{-12,-12,-4},							// Bounding box mins
		{12, 12, 4},							// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
	},

	/*
	 * QUAKED item_puzzle_tavernkey (.3 .3 1) (-8 -8 -4) (8 8 4)    x   NO_DROP
	 * Key puzzle piece
	 * Ssdocks Level
	 * NO_DROP - won't drop to ground
	 */
	{
		"item_puzzle_tavernkey",				// Spawnname
		Pickup_Puzzle,							// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_BOB,									// world model flags
		NULL,									// view model
		"icons/p_tavernkey.m8",					// Icon name (char *)
		"Tavern Key",							// Pickup name (char *)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_PUZZLE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_TAVERNKEY,							// tag ?
		NULL,									// precaches
		GM_F_TAVERNKEY,							// pickup message
		GM_NEED_TAVERNKEY,						// can`t use message
		{-12,-12,-4},							// Bounding box mins
		{12, 12, 4},							// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
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
		EF_ROTATE | EF_BOB,						// world model flags
		NULL,									// view model
		"icons/i_tornado.m8",					// Icon name (char *)
		"tornado",								// Pickup name (char *)
		0,										// Number of digits to display
		MANA_USE_TORNADO,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		0,										// weapon model index
		NULL,									// void * ?
		ITEM_DEFENSE_TORNADO,					// tag ?
		NULL,									// precaches
		GM_TORNADO,								// pickup message
		GM_NOTORNADO,							// can`t use message
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
	},

	/*
	 * QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_armor_body",
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/body/tris.md2", EF_ROTATE,
		NULL,
		"i_bodyarmor",
		"Body Armor",
		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&bodyarmor_info,
		ARMOR_BODY,
		""
	},

	/*
	 * QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_armor_combat",
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/combat/tris.md2", EF_ROTATE,
		NULL,
		"i_combatarmor",
		"Combat Armor",
		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&combatarmor_info,
		ARMOR_COMBAT,
		""
	},

	/*
	 * QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_armor_jacket",
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/jacket/tris.md2", EF_ROTATE,
		NULL,
		"i_jacketarmor",
		"Jacket Armor",
		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&jacketarmor_info,
		ARMOR_JACKET,
		""
	},

	/*
	 * QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_armor_shard",
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar2_pkup.wav",
		"models/items/armor/shard/tris.md2", EF_ROTATE,
		NULL,
		"i_jacketarmor",
		"Armor Shard",
		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		ARMOR_SHARD,
		""
	},

	/*
	 * QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_power_screen",
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/screen/tris.md2", EF_ROTATE,
		NULL,
		"i_powerscreen",
		"Power Screen",
		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_power_shield",
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/shield/tris.md2", EF_ROTATE,
		NULL,
		"i_powershield",
		"Power Shield",
		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
		"misc/power2.wav misc/power1.wav"
	},

	/*
	 * weapon_grapple (.3 .3 1) (-16 -16 -16) (16 16 16)
	 * always owned, never in the world
	 */
	{
		"weapon_grapple",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		CTFWeapon_Grapple,
		"misc/w_pkup.wav",
		"models/weapons/grapple/hook/tris.md2", EF_ROTATE,
		"models/weapons/grapple/tris.md2",
		"w_grapple",
		"Grapple",
		0,
		0,
		NULL,
		IT_WEAPON,
		WEAP_GRAPPLE,
		NULL,
		0,

		"weapons/grapple/grfire.wav weapons/grapple/grpull.wav weapons/grapple/grhang.wav weapons/grapple/grreset.wav weapons/grapple/grhit.wav"
	},

	/*
	 * weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
	 * always owned, never in the world
	 */
	{
		"weapon_blaster",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Blaster,
		"misc/w_pkup.wav",
		"models/weapons/g_blast/tris.md2", EF_ROTATE,
		"models/weapons/v_blast/tris.md2",
		"w_blaster",
		"Blaster",
		0,
		0,
		NULL,
		IT_WEAPON | IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
		"weapons/blastf1a.wav misc/lasfly.wav"
	},

	/*
	 * QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_shotgun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Shotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
		"w_shotgun",
		"Shotgun",
		0,
		1,
		"Shells",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_SHOTGUN,
		NULL,
		0,
		"weapons/shotgf1b.wav weapons/shotgr1b.wav"
	},

	/*
	 * QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_supershotgun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_SuperShotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
		"w_sshotgun",
		"Super Shotgun",
		0,
		2,
		"Shells",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
		"weapons/sshotf1b.wav"
	},

	/*
	 * QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_machinegun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Machinegun,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_machn/tris.md2",
		"w_machinegun",
		"Machinegun",
		0,
		1,
		"Bullets",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
		"weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
	},

	/*
	 * QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_chaingun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Chaingun,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
		"w_chaingun",
		"Chaingun",
		0,
		1,
		"Bullets",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
		"weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav"
	},

	/*
	 * QUAKED weapon_etf_rifle (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_etf_rifle",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ETF_Rifle,
		"misc/w_pkup.wav",
		"models/weapons/g_etf_rifle/tris.md2", EF_ROTATE,
		"models/weapons/v_etf_rifle/tris.md2",
		"w_etf_rifle",
		"ETF Rifle",
		0,
		1,
		"Flechettes",
		IT_WEAPON,
		WEAP_ETFRIFLE,
		NULL,
		0,
		"weapons/nail1.wav models/proj/flechette/tris.md2",
	},

	/*
	 * QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"ammo_grenades",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
		"a_grenades",
		"Grenades",
		3,
		5,
		"grenades",
		IT_AMMO | IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		AMMO_GRENADES,
		"weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

	/*
	 * QUAKED ammo_trap (.3 .3 1) (-16 -16 -16) (16 16 16)
	 */
	{
		"ammo_trap",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Trap,
		"misc/am_pkup.wav",
		"models/weapons/g_trap/tris.md2", EF_ROTATE,
		"models/weapons/v_trap/tris.md2",
		"a_trap",
		"Trap",
		3,
		1,
		"trap",
		IT_AMMO | IT_WEAPON,
		0,
		NULL,
		AMMO_TRAP,
		"weapons/trapcock.wav weapons/traploop.wav weapons/trapsuck.wav weapons/trapdown.wav"
	},

	/*
	 * QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_GrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
		"w_glauncher",
		"Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
		"models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

	/*
	 * QUAKED weapon_proxlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_proxlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ProxLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_plaunch/tris.md2", EF_ROTATE,
		"models/weapons/v_plaunch/tris.md2",
		"w_proxlaunch",
		"Prox Launcher",
		0,
		1,
		"Prox",
		IT_WEAPON,
		WEAP_PROXLAUNCH,
		NULL,
		AMMO_PROX,
		"weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav weapons/proxwarn.wav weapons/proxopen.wav",
	},

	/*
	 * QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
		"w_rlauncher",
		"Rocket Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
		"models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

	/*
	 * QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_hyperblaster",
		Pickup_Weapon,
		Use_Weapon2,
		Drop_Weapon,
		Weapon_HyperBlaster,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
		"w_hyperblaster",
		"HyperBlaster",
		0,
		1,
		"Cells",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
		"weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"
	},

	/*
	 * QUAKED weapon_plasmabeam (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_plasmabeam",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Heatbeam,
		"misc/w_pkup.wav",
		"models/weapons/g_beamer/tris.md2", EF_ROTATE,
		"models/weapons/v_beamer/tris.md2",
		"w_heatbeam",
		"Plasma Beam",
		0,
		2,
		"Cells",
		IT_WEAPON,
		WEAP_PLASMA,
		NULL,
		0,
		"models/weapons/v_beamer2/tris.md2 weapons/bfg__l1a.wav",
	},

	/*
	 * QUAKED weapon_boomer (.3 .3 1) (-16 -16 -16) (16 16 16)
	 */
	{
		"weapon_boomer",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Ionripper,
		"misc/w_pkup.wav",
		"models/weapons/g_boom/tris.md2", EF_ROTATE,
		"models/weapons/v_boomer/tris.md2",
		"w_ripper",
		"Ionripper",
		0,
		2,
		"Cells",
		IT_WEAPON,
		WEAP_BOOMER,
		NULL,
		0,
		"weapons/rg_hum.wav weapons/rippfire.wav"
	},

	/*
	 * QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_railgun",
		Pickup_Weapon,
		Use_Weapon2,
		Drop_Weapon,
		Weapon_Railgun,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
		"w_railgun",
		"Railgun",
		0,
		1,
		"Slugs",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
		"weapons/rg_hum.wav"
	},

	/*
	 * QUAKED weapon_phalanx (.3 .3 1) (-16 -16 -16) (16 16 16)
	 */
	{
		"weapon_phalanx",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Phalanx,
		"misc/w_pkup.wav",
		"models/weapons/g_shotx/tris.md2", EF_ROTATE,
		"models/weapons/v_shotx/tris.md2",
		"w_phallanx",
		"Phalanx",
		0,
		1,
		"Mag Slug",
		IT_WEAPON,
		WEAP_PHALANX,
		NULL,
		0,
		"weapons/plasshot.wav"
	},

	/*
	 * QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BFG,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
		"w_bfg",
		"BFG10K",
		0,
		50,
		"Cells",
		IT_WEAPON | IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
		"sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav"
	},

	/*
	 * QUAKED weapon_chainfist (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_chainfist",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ChainFist,
		"misc/w_pkup.wav",
		"models/weapons/g_chainf/tris.md2", EF_ROTATE,
		"models/weapons/v_chainf/tris.md2",
		"w_chainfist",
		"Chainfist",
		0,
		0,
		NULL,
		IT_WEAPON | IT_MELEE,
		WEAP_CHAINFIST,
		NULL,
		1,
		"weapons/sawidle.wav weapons/sawhit.wav",
	},

	/*
	 * QUAKED weapon_disintegrator (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"weapon_disintegrator",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Disintegrator,
		"misc/w_pkup.wav",
		"models/weapons/g_dist/tris.md2", EF_ROTATE,
		"models/weapons/v_dist/tris.md2",
		"w_disintegrator",
		"Disruptor",
		0,
		1,
		"Rounds",
		IT_WEAPON,
		WEAP_DISRUPTOR,
		NULL,
		1,
		"models/items/spawngro/tris.md2 models/proj/disintegrator/tris.md2 weapons/disrupt.wav weapons/disint2.wav weapons/disrupthit.wav",
	},

	/*
	 * QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"ammo_shells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/shells/medium/tris.md2", 0,
		NULL,
		"a_shells",
		"Shells",
		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SHELLS,
		""
	},

	/*
	 * QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"ammo_bullets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/bullets/medium/tris.md2", 0,
		NULL,
		"a_bullets",
		"Bullets",
		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BULLETS,
		""
	},

	/*
	 * QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"ammo_cells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/cells/medium/tris.md2", 0,
		NULL,
		"a_cells",
		"Cells",
		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_CELLS,
		""
	},

	/*
	 * QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"ammo_rockets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/medium/tris.md2", 0,
		NULL,
		"a_rockets",
		"Rockets",
		3,
		5,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_ROCKETS,
		""
	},

	/*
	 * QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"ammo_slugs",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/slugs/medium/tris.md2", 0,
		NULL,
		"a_slugs",
		"Slugs",
		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SLUGS,
		""
	},

	/*
	 * QUAKED ammo_flechettes (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"ammo_flechettes",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/ammo/am_flechette/tris.md2", 0,
		NULL,
		"a_flechettes",
		"Flechettes",
		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_FLECHETTES
	},

	/*
	 * QUAKED ammo_prox (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"ammo_prox",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/ammo/am_prox/tris.md2", 0,
		NULL,
		"a_prox",
		"Prox",
		3,
		5,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_PROX,
		"models/weapons/g_prox/tris.md2 weapons/proxwarn.wav"
	},

	/*
	 * QUAKED ammo_tesla (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"ammo_tesla",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Tesla,
		"misc/am_pkup.wav",
		"models/ammo/am_tesl/tris.md2", 0,
		"models/weapons/v_tesla/tris.md2",
		"a_tesla",
		"Tesla",
		3,
		5,
		"Tesla",
		IT_AMMO | IT_WEAPON,
		0,
		NULL,
		AMMO_TESLA,
		"models/weapons/v_tesla2/tris.md2 weapons/teslaopen.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav models/weapons/g_tesla/tris.md2"
	},

	/*
	 * QUAKED ammo_nuke (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"ammo_nuke",
		Pickup_Nuke,
		Use_Nuke,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/g_nuke/tris.md2", EF_ROTATE,
		NULL,
		"p_nuke",
		"A-M Bomb",
		3,
		300,
		"A-M Bomb",
		IT_POWERUP,
		0,
		NULL,
		0,
		"weapons/nukewarn2.wav world/rumble.wav"
	},

	/*
	 * QUAKED ammo_disruptor (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"ammo_disruptor",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/ammo/am_disr/tris.md2", 0,
		NULL,
		"a_disruptor",
		"Rounds",
		3,
		15,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_DISRUPTOR
	},

	/*
	 * QUAKED ammo_magslug (.3 .3 1) (-16 -16 -16) (16 16 16)
	 */
	{
		"ammo_magslug",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/objects/ammo/tris.md2", 0,
		NULL,
		"a_mslugs",
		"Mag Slug",
		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_MAGSLUG,
		""
	},

	/*
	 * QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_quad",
		Pickup_Powerup,
		Use_Quad,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quaddama/tris.md2", EF_ROTATE,
		NULL,
		"p_quad",
		"Quad Damage",
		2,
		60,
		NULL,
		IT_POWERUP | IT_INSTANT_USE,
		0,
		NULL,
		0,
		"items/damage.wav items/damage2.wav items/damage3.wav"
	},

	/*
	 * QUAKED item_quadfire (.3 .3 1) (-16 -16 -16) (16 16 16)
	 */
	{
		"item_quadfire",
		Pickup_Powerup,
		Use_QuadFire,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quadfire/tris.md2", EF_ROTATE,
		NULL,
		"p_quadfire",

		"DualFire Damage",
		2,
		60,
		NULL,
		IT_POWERUP | IT_INSTANT_USE,
		0,
		NULL,
		0,
		"items/quadfire1.wav items/quadfire2.wav items/quadfire3.wav"
	},

	/*
	 * QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_invulnerability",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/invulner/tris.md2", EF_ROTATE,
		NULL,
		"p_invulnerability",
		"Invulnerability",
		2,
		300,
		NULL,
		IT_POWERUP | IT_INSTANT_USE,
		0,
		NULL,
		0,
		"items/protect.wav items/protect2.wav items/protect4.wav"
	},

	/*
	 * QUAKED item_invisibility (.3 .3 1) (-16 -16 -16) (16 16 16)
	 */
	{
		"item_invisibility",
		Pickup_Powerup,
		Use_Invisibility,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/cloaker/tris.md2",
		EF_ROTATE,
		NULL,
		"p_cloaker",
		"Invisibility",
		2,
		300,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
		NULL,
	},

	/*
	 * QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_silencer",
		Pickup_Powerup,
		Use_Silencer,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/silencer/tris.md2", EF_ROTATE,
		NULL,
		"p_silencer",
		"Silencer",
		2,
		60,
		NULL,
		IT_POWERUP | IT_INSTANT_USE,
		0,
		NULL,
		0,
		""
	},

	/*
	 * QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_breather",
		Pickup_Powerup,
		Use_Breather,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/breather/tris.md2", EF_ROTATE,
		NULL,
		"p_rebreather",
		"Rebreather",
		2,
		60,
		NULL,
		IT_STAY_COOP | IT_POWERUP | IT_INSTANT_USE,
		0,
		NULL,
		0,
		"items/airout.wav"
	},

	/*
	 * QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_enviro",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/enviro/tris.md2", EF_ROTATE,
		NULL,
		"p_envirosuit",
		"Environment Suit",
		2,
		60,
		NULL,
		IT_STAY_COOP | IT_POWERUP | IT_INSTANT_USE,
		0,
		NULL,
		0,
		"items/airout.wav"
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

	/*
	 * QUAKED item_ir_goggles (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_ir_goggles",
		Pickup_Powerup,
		Use_IR,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/goggles/tris.md2", EF_ROTATE,
		NULL,
		"p_ir",
		"IR Goggles",
		2,
		60,
		NULL,
		IT_POWERUP | IT_INSTANT_USE,
		0,
		NULL,
		0,
		"misc/ir_start.wav"
	},

	/*
	 * QUAKED item_double (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_double",
		Pickup_Powerup,
		Use_Double,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/ddamage/tris.md2", EF_ROTATE,
		NULL,
		"p_double",
		"Double Damage",
		2,
		60,
		NULL,
		IT_POWERUP | IT_INSTANT_USE,
		0,
		NULL,
		0,
		"misc/ddamage1.wav misc/ddamage2.wav misc/ddamage3.wav"
	},

	/*
	 * QUAKED item_compass (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_compass",
		Pickup_Powerup,
		Use_Compass,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/objects/fire/tris.md2", EF_ROTATE,
		NULL,
		"p_compass",
		"compass",
		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
	},

	/*
	 * QUAKED item_sphere_vengeance (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_sphere_vengeance",
		Pickup_Sphere,
		Use_Vengeance,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/vengnce/tris.md2", EF_ROTATE,
		NULL,
		"p_vengeance",
		"vengeance sphere",
		2,
		60,
		NULL,
		IT_POWERUP | IT_INSTANT_USE,
		0,
		NULL,
		0,
		"spheres/v_idle.wav"
	},

	/*
	 * QUAKED item_sphere_hunter (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_sphere_hunter",
		Pickup_Sphere,
		Use_Hunter,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/hunter/tris.md2", EF_ROTATE,
		NULL,
		"p_hunter",
		"hunter sphere",
		2,
		120,
		NULL,
		IT_POWERUP | IT_INSTANT_USE,
		0,
		NULL,
		0,
		"spheres/h_idle.wav spheres/h_active.wav spheres/h_lurk.wav"
	},

	/*
	 * QUAKED item_sphere_defender (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_sphere_defender",
		Pickup_Sphere,
		Use_Defender,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/defender/tris.md2", EF_ROTATE,
		NULL,
		"p_defender",
		"defender sphere",
		2,
		60,
		NULL,
		IT_POWERUP | IT_INSTANT_USE,
		0,
		NULL,
		0,
		"models/proj/laser2/tris.md2 models/items/shell/tris.md2 spheres/d_idle.wav"
	},

	/*
	 * QUAKED item_doppleganger (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
	 */
	{
		"item_doppleganger",
		Pickup_Doppleganger,
		Use_Doppleganger,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/dopple/tris.md2",
		EF_ROTATE,
		NULL,
		"p_doppleganger",
		"Doppleganger",
		0,
		90,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
		"models/objects/dopplebase/tris.md2 models/items/spawngro2/tris.md2 models/items/hunter/tris.md2 models/items/vengnce/tris.md2",
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
		Drop_General,
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

/*
 * QUAKED item_health_half (.3 .3 1) (-10 -10 -10) (10 10 10) TRIGGER_SPAWN
 *
 * Heretic 2: health full pack (30)
 */
void
SP_item_health_half(edict_t *self)
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

	self->count = 10;
	SpawnItem(self, FindItem("Health"));
	self->s.effects |= EF_ROTATE;
	gi.soundindex("items/l_health.wav");
}

/*
 * QUAKED item_health_full (.3 .3 1) (-10 -10 -10) (10 10 10) TRIGGER_SPAWN
 *
 * Heretic 2: health full pack (30)
 */
void
SP_item_health_full(edict_t *self)
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

	self->count = 30;
	SpawnItem(self, FindItem("Health"));
	self->s.effects |= EF_ROTATE;
	gi.soundindex("items/l_health.wav");
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
	gitem_t *dyn_items;
	int dyn_count, num_items;

	if (sizeof(gameitemlist) > sizeof(itemlist))
	{
		gi.error("Defined items more than %d\n", MAX_ITEMS);
		return;
	}

	memset(itemlist, 0, sizeof(itemlist));
	memcpy(itemlist, gameitemlist, sizeof(gameitemlist));
	num_items = ARRLEN(gameitemlist) - 1;

	dyn_items = GetDynamicItems(&dyn_count);
	if (dyn_items)
	{
		if (dyn_count > 0)
		{
			size_t i;

			for (i = 0; i < dyn_count; i ++)
			{
				gitem_t *it;

				it = FindItemInList(dyn_items[i].classname, itemlist, num_items);
				if (!it)
				{
					memcpy(itemlist + num_items, dyn_items + i, sizeof(gitem_t));
					/* Add callbacks */
					if (!strncmp(itemlist[num_items].classname, "weapon_", 7))
					{
						itemlist[num_items].pickup = Pickup_Weapon;
						itemlist[num_items].use = Use_Weapon;
						itemlist[num_items].drop = Drop_Weapon;
						itemlist[num_items].world_model_flags = EF_ROTATE;
						itemlist[num_items].flags = IT_WEAPON;
					}
					else if (!strncmp(itemlist[num_items].classname, "item_", 5))
					{
						itemlist[num_items].pickup = Pickup_General;
						itemlist[num_items].drop = Drop_General;
						itemlist[num_items].world_model_flags = EF_ROTATE;
					}
					else if (!strncmp(itemlist[num_items].classname, "key_", 4))
					{
						itemlist[num_items].pickup = Pickup_Key;
						itemlist[num_items].drop = Drop_General;
						itemlist[num_items].world_model_flags = EF_ROTATE;
						itemlist[num_items].flags = IT_KEY;
					}
					else if (!strncmp(itemlist[num_items].classname, "ammo_", 5))
					{
						itemlist[num_items].pickup = Pickup_Ammo;
						itemlist[num_items].use = Use_Weapon;
						itemlist[num_items].drop = Drop_Ammo;
						itemlist[num_items].flags = IT_AMMO;
					}

					num_items ++;

					if (num_items >= MAX_ITEMS)
					{
						gi.dprintf("No space for additional items\n");
						break;
					}
				}
			}
		}

		free(dyn_items);
	}

	game.num_items = num_items;
}

qboolean
ItemHasValidModel(gitem_t *item)
{
	const dmdxframegroup_t * frames;
	int num, modelindex;

	modelindex = gi.modelindex(item->world_model);
	frames = gi.GetModelInfo(modelindex, &num, NULL, NULL);

	return frames && (num > 0);
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
	char *spawnClass = NULL;
	gitem_t *item;

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

	item = FindItemInList(spawnClass, itemlist, game.num_items);
	if (item)
	{
		/* found it */
		SpawnItem(self, item);
		return;
	}
}
