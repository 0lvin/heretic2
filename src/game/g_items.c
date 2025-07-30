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
#include "header/p_item.h"
#include "header/g_items.h"
#include "header/g_itemstats.h"
#include "header/g_weapon.h"
#include "player/library/player.h"
#include "player/library/p_weapon.h"
#include "player/library/p_anims.h"
#include "player/library/p_anim_data.h"
#include "player/library/p_items.h"
#include "common/fx.h"
#include "common/h2rand.h"
#include "common/cl_strings.h"

#define ITEM_COOP_ONLY		1
#define ITEM_NO_DROP		2

#define HEALTH_IGNORE_MAX 1
#define HEALTH_TIMED 2

qboolean Pickup_Weapon(edict_t *ent, edict_t *other);
void Use_Weapon(edict_t *ent, gitem_t *inv);
void Use_Weapon2(edict_t *ent, gitem_t *inv);
void Drop_Weapon(edict_t *ent, gitem_t *inv);

void Weapon_Blaster(edict_t *ent);
void Weapon_Shotgun(edict_t *ent);
void Weapon_SuperShotgun(edict_t *ent);
void Weapon_Machinegun(edict_t *ent);
void Weapon_Chaingun(edict_t *ent);
void Weapon_HyperBlaster(edict_t *ent);
void Weapon_RocketLauncher(edict_t *ent);
void Weapon_Grenade(edict_t *ent);
void Weapon_GrenadeLauncher(edict_t *ent);
void Weapon_Railgun(edict_t *ent);
void Weapon_BFG(edict_t *ent);
void Weapon_ChainFist(edict_t *ent);
void Weapon_Disintegrator(edict_t *ent);
void Weapon_ETF_Rifle(edict_t *ent);
void Weapon_Heatbeam(edict_t *ent);
void Weapon_Prox(edict_t *ent);
void Weapon_Tesla(edict_t *ent);
void Weapon_ProxLauncher(edict_t *ent);

void Weapon_Ionripper(edict_t *ent);
void Weapon_Phalanx(edict_t *ent);
void Weapon_Trap(edict_t *ent);

gitem_armor_t jacketarmor_info = {25, 50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info = {50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info = {100, 200, .80, .60, ARMOR_BODY};

static int jacket_armor_index;
static int combat_armor_index;
static int body_armor_index;
static int power_screen_index;
static int power_shield_index;

void Use_Quad(edict_t *ent, gitem_t *item);
void Use_QuadFire(edict_t *ent, gitem_t *item);

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

#define ITEM_COOP_ONLY		1
#define ITEM_NO_DROP		2

// ************************************************************************************************
// RespawnedThink
// --------------
// ************************************************************************************************

void
RespawnedThink(edict_t *ent)
{
	ent->think = NULL;
//	ent->svflags |= SVF_NOCLIENT;
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
//	ent->svflags |= SVF_NOCLIENT;
}

// ************************************************************************************************
// SetRespawn
// ----------
// ************************************************************************************************

void
SetRespawn(edict_t *ent, float delay)
{
	// So it'll get sent to the client again.

//	ent->svflags &= ~SVF_NOCLIENT;

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

	if (!other->client->playerinfo.pers.inventory.Items[ITEM_INDEX(ent->item)])
	{
		other->client->playerinfo.pers.inventory.Items[ITEM_INDEX(ent->item)] = 1;

		G_CPrintf(other, PRINT_HIGH, ent->item->msg_pickup);

		return true;
	}
	else
	{
		return false;
	}
}

// ************************************************************************************************
// AddWeaponToInventory
// --------------------
// ************************************************************************************************

qboolean AddWeaponToInventory(gitem_t *item,edict_t *player)
{
	gitem_t	*newitem;
	int		count;

	// Do we already have this weapon?

	if(!player->client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)])
	{
		// We don't already have it, so get the weapon and some ammo.

		if (item->tag == ITEM_WEAPON_SWORDSTAFF)
			count= 0;
		else if (item->tag == ITEM_WEAPON_HELLSTAFF)
			count = AMMO_COUNT_HELLSTAFF;
		else if (item->tag == ITEM_WEAPON_REDRAINBOW)
		{
			// give us the bowtype
			player->client->playerinfo.pers.bowtype = BOW_TYPE_REDRAIN;
			count = AMMO_COUNT_REDRAINBOW;
		}
		else if (item->tag == ITEM_WEAPON_PHOENIXBOW)
		{
			// give us the bowtype
			player->client->playerinfo.pers.bowtype = BOW_TYPE_PHOENIX;
			count = AMMO_COUNT_PHOENIXBOW;
		}
		else
			count = AMMO_COUNT_MOST;

		player->client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)] = 1;

		if(count)
		{
			newitem = FindItem(item->ammo);
			Add_Ammo(player, newitem,count);
		}

		// Now decide if we want to swap weapons or not.

		if (player->client->playerinfo.pers.autoweapon)
		{
			// If this new weapon is a higher value than the one we currently have, swap the current
			// weapon for the new one.

			if (ITEM_INDEX(item) > ITEM_INDEX(player->client->playerinfo.pers.weapon))
			{
				item->use(&player->client->playerinfo,item);
			}
		}

		return true;
	}
	else
	{
		// We already have it...

		if(!((deathmatch->value&&((int)dmflags->value&DF_WEAPONS_STAY))||coop->value))
		{
			// ...and DF_WEPONS_STAY is off and we're not in coop, so just try to up the ammo counts.

			if (item->tag == ITEM_WEAPON_HELLSTAFF)
			{
				newitem = FindItemByClassname("item_ammo_hellstaff");
				count = AMMO_COUNT_HELLSTAFF;
			}
			else if (item->tag == ITEM_WEAPON_REDRAINBOW)
			{
				newitem = FindItemByClassname("item_ammo_redrain");
				count = AMMO_COUNT_REDRAINBOW;
			}
			else if (item->tag == ITEM_WEAPON_PHOENIXBOW)
			{
				newitem = FindItemByClassname("item_ammo_phoenix");
				count = AMMO_COUNT_PHOENIXBOW;
			}
			else
			{
				newitem = FindItemByClassname("item_mana_offensive_half");
				count = AMMO_COUNT_MOST;
			}

			if(Add_Ammo(player, newitem,count))
			{
				// Have space in our inventory, so add ammo.

				return true;
			}
			else
			{
				// No space in inventory to add the ammo.

				return false;
			}
		}
		else
		{
			// ...but we're not able to pick it up.

			return false;
		}
	}
}

// ************************************************************************************************
// Pickup_Weapon
// -------------
// ************************************************************************************************

qboolean
Pickup_Weapon(edict_t *ent,edict_t *other)
{
	if (other->flags & FL_CHICKEN)
	{
		return false;
	}

	if(AddWeaponToInventory(ent->item,other))
	{
		G_CPrintf(other, PRINT_HIGH, ent->item->msg_pickup);

		return true;
	}
	else
	{
		// We already have it.

		return false;
	}
}

// ************************************************************************************************
// AddDefenseToInventory
// ---------------------
// ************************************************************************************************

qboolean AddDefenseToInventory(gitem_t *item,edict_t *player)
{
	if(!player->client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)])
	{
		player->client->playerinfo.pers.inventory.Items[ITEM_INDEX(item)]=1;

		// Now decide if we want to swap defenses or not.

//		if(player->client->playerinfo.pers.autoweapon || !player->client->playerinfo.pers.defence)
		if(player->client->playerinfo.pers.autoweapon )
		{
			item->use(&player->client->playerinfo,item);
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

	if (ent->client->playerinfo.pers.inventory.Items[index] == max)
		return false;

	ent->client->playerinfo.pers.inventory.Items[index] += count;

	if (ent->client->playerinfo.pers.inventory.Items[index] > max)
		ent->client->playerinfo.pers.inventory.Items[index] = max;

	return true;
}

/* ====================================================================== */

qboolean
Add_Ammo(edict_t *ent, gitem_t *item, int count)
{
	int bo;
	int max;

	if (!ent || !item || !ent->client)
	{
		return false;
	}

	if ((item->tag == ITEM_AMMO_MANA_OFFENSIVE_HALF) || (item->tag == ITEM_AMMO_MANA_OFFENSIVE_FULL))
	{
		item = FindItemByClassname("item_mana_offensive_half");
		max = ent->client->playerinfo.pers.max_offmana;
		return(Add_AmmoToInventory (ent,item,count,max));
	}
	else if ((item->tag == ITEM_AMMO_MANA_DEFENSIVE_HALF) || (item->tag == ITEM_AMMO_MANA_DEFENSIVE_FULL))
	{
		item = FindItemByClassname("item_mana_defensive_half");
		max = ent->client->playerinfo.pers.max_defmana;
		return(Add_AmmoToInventory (ent,item,count,max));
	}
	else if ((item->tag == ITEM_AMMO_MANA_COMBO_QUARTER) || (item->tag == ITEM_AMMO_MANA_COMBO_HALF))
	{
		item = FindItemByClassname("item_mana_offensive_half");
		max = ent->client->playerinfo.pers.max_offmana;

		bo = Add_AmmoToInventory (ent,item,count,max);

		item = FindItemByClassname("item_mana_defensive_half");
		max = ent->client->playerinfo.pers.max_defmana;
		bo |= Add_AmmoToInventory (ent,item,count,max);

		return(bo);
	}
	else if (item->tag == ITEM_AMMO_REDRAIN)
	{
		max = ent->client->playerinfo.pers.max_redarrow;
		return(Add_AmmoToInventory (ent,item,count,max));
	}
	else if (item->tag == ITEM_AMMO_PHOENIX)
	{
		max = ent->client->playerinfo.pers.max_phoenarr;
		return(Add_AmmoToInventory (ent,item,count,max));
	}
	else if (item->tag == ITEM_AMMO_HELLSTAFF)
	{
		max = ent->client->playerinfo.pers.max_hellstaff;
		return(Add_AmmoToInventory (ent,item,count,max));
	}
	else
	{
		gi.dprintf("undefined ammo type\n");
		return false;
	}
}

qboolean
Pickup_Ammo(edict_t *ent, edict_t *other)
{
	int count;

	if (other->flags & FL_CHICKEN)
	{
		return false;
	}

	if (ent->count)
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

	if (ent->client->playerinfo.pers.inventory.Items[index] >= item->quantity)
	{
		dropped->count = item->quantity;
	}
	else
	{
		dropped->count = ent->client->playerinfo.pers.inventory.Items[index];
	}

	ent->client->playerinfo.pers.inventory.Items[index] -= dropped->count;

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

/*
===============
Touch_Item
===============
*/

void
Touch_Item(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(strcmp(other->classname,"player"))
	{
		// Only players can touch items.

		return;
	}

	if(other->health <= 0)
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
===============
PrecacheItem

Precaches all data needed for a given item. This will be called for each item
spawned in a level, and for each item in each client's inventory.
===============
*/

void
PrecacheItem(gitem_t *it)
{
	gitem_t	*ammo;

	if (!it)
		return;

	if (it->pickup_sound)
		gi.soundindex(it->pickup_sound);
	if (it->world_model)
		gi.modelindex (it->world_model);
	if (it->icon)
		gi.imageindex (it->icon);

	// parse everything for its ammo
	if (it->ammo && it->ammo[0])
	{
		ammo = FindItem(it->ammo);
		if (ammo != it)
		{
			PrecacheItem(ammo);
		}
	}
}

// ************************************************************************************************
// SpawnItem
// ---------
// Sets the clipping size and plants the object on the floor. Items can't be immediately dropped to
// the floor because they might be on an entity that hasn't spawned yet.
// ************************************************************************************************

void
SpawnItem(edict_t *ent, gitem_t *item)
{
	if ((ent->spawnflags & ITEM_COOP_ONLY) && (!coop->value))
		return;

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
		if (item->tag == ITEM_AMMO_REDRAIN || item->tag == ITEM_AMMO_PHOENIX)
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

	// =============================================================================================

	// Weapon items.

	{
		"Weapon_SwordStaff",					// Spawnname (char *)
		"staff",								// Pickup name (char *)
		0,										// pickup message
		0,										// can`t use message
		NULL,	 								// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",					// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WSWORD_STD1,						// Player animation sequence to engage when used
		ASEQ_WSWORD_STD1,						// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		0,										// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_WEAPON,								// flags
		NULL,									// void * ?
		ITEM_WEAPON_SWORDSTAFF,					// tag ?
		"icons/i_staff.m8",				// Icon name (char *)
	},

	{
		"Weapon_FlyingFist", 					// Spawnname
		"fball",								// Pickup name (char *)
		0,										// pickup message
		GM_NOFLYINGFIST,						// can`t use message
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WFIREBALL,							// Player animation sequence to engage when used
		ASEQ_WFIREBALL,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		AMMO_USE_FIREBALL,						// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		NULL,									// void * ?
		ITEM_WEAPON_FLYINGFIST,					// tag ?
 		"icons/i_fball.m8",   			// Icon name (char *)
	},

	{
		"item_weapon_hellstaff",				// Spawnname
		"hell",									// Pickup name (char *)
		GM_HELLSTAFF,							// pickup message
		GM_NOHELLORBS,										// can`t use message
		NULL,	 								// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WHELL_GO,							// Player animation sequence to engage when used
		ASEQ_WHELL_GO,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = infinite)
		2,										// Number of digits to display
		AMMO_USE_HELLSTAFF,						// Ammo/ammo use per shot
		"Hell-staff-ammo",						// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		NULL,									// void * ?
		ITEM_WEAPON_HELLSTAFF,					// tag ?
		"icons/i_hell.m8",				// Icon name (char *)
	},

	{
		"item_weapon_magicmissile",				// Spawnname
		"array",								// Pickup name (char *)
		GM_FORCEBLAST, 							// pickup message
		GM_NOFORCE,										// can`t use message
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WBLAST,							// Player animation sequence to engage when used
		ASEQ_WARRAY,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		AMMO_USE_MAGICMISSILE,					// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		NULL,									// void * ?
		ITEM_WEAPON_MAGICMISSILE,				// tag ?
		"icons/i_array.m8",				// Icon name (char *)
	},

	{
		"item_weapon_redrain_bow", 				// Spawnname
		"rain",									// Pickup name (char *)
		GM_STORMBOW,										// pickup message
		GM_NOSTORMBOW,										// can`t use message
		NULL,	 								// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WRRBOW_GO,							// Player animation sequence to engage when used
		ASEQ_WRRBOW_GO,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
		2,										// Number of digits to display
		AMMO_USE_REDRAIN,						// Ammo/ammo use per shot
		"Red-Rain-Arrows",						// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		NULL,									// void * ?
		ITEM_WEAPON_REDRAINBOW,					// tag ?
		"icons/i_rain.m8",				// Icon name (char *)
	},

	{
		"item_weapon_firewall",					// Spawnname
		"fwall",								// Pickup name (char *)
		GM_FIREWALL,										// pickup message
		GM_NOFIREWALL,										// can`t use message
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		EF_ROTATE,								// world model flags
		ASEQ_WFIREWALL,							// Player animation sequence to engage when used
		ASEQ_WFIREWALL,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		AMMO_USE_FIREWALL,						// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		NULL,									// void * ?
		ITEM_WEAPON_FIREWALL,					// tag ?
 		"icons/i_fwall.m8",					// Icon name (char *)
	},

	{
		"item_weapon_phoenixbow", 				// Spawnname
		"phoen",							// Pickup name (char *)
		GM_PHOENIX,										// pickup message
		GM_NOPHOENIX,										// can`t use message
		NULL,	 								// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		0, 0, 0,								// Bounding box mins
		0, 0, 0,								// Bounding box maxs
		ASEQ_WPHBOW_GO,							// Player animation sequence to engage when used
		ASEQ_WPHBOW_GO,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
		2,										// Number of digits to display
		AMMO_USE_PHOENIX,						// Ammo/ammo use per shot
		"Phoenix-Arrows",						// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		NULL,									// void * ?
		ITEM_WEAPON_PHOENIXBOW,					// tag ?
		"icons/i_phoen.m8", 				// Icon name (char *)
	},

	{
		"item_weapon_sphereofannihilation",		// Spawnname
		"sphere",								// Pickup name (char *)
		GM_SPHERE,										// pickup message
		GM_NOSPHERE,										// can`t use message
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WSPHERE_GO,						// Player animation sequence to engage when used
		ASEQ_WSPHERE_GO,						// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		AMMO_USE_SPHERE,						// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		NULL,									// void * ?
		ITEM_WEAPON_SPHEREOFANNIHILATION,		// tag ?
 		"icons/i_sphere.m8",			// Icon name (char *)
	},

	{
		"item_weapon_maceballs",				// Spawnname
		"mace",									// Pickup name (char *)
		GM_IRONDOOM,										// pickup message
		GM_NOIRONDOOM,										// can`t use message
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_WRIPPER,							// Player animation sequence to engage when used
		ASEQ_WBIGBALL,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		AMMO_USE_MACEBALL,						// Ammo/ammo use per shot
		"Off-mana",								// Ammo (char *)
		IT_WEAPON | IT_OFFENSE,					// flags
		NULL,									// void * ?
		ITEM_WEAPON_MACEBALLS,					// tag ?
 		"icons/i_mace.m8",				// Icon name (char *)
	},

	{
		"item_defense_powerup",					// Spawnname
		"powerup",								// Pickup name (char *)
		GM_TOME, 								// pickup message
		GM_NOTOME,										// can`t use message
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		MANA_USE_POWERUP,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		NULL,									// void * ?
		ITEM_DEFENSE_POWERUP,					// tag ?
 		"icons/i_tome.m8",				// Icon name (char *)
	},

	{
		"item_defense_ringofrepulsion",			// Spawnname
		"ring",									// Pickup name (char *)
		GM_RING,										// pickup message
		GM_NORING,										// can`t use message
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		MANA_USE_RING,							// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		NULL,									// void * ?
		ITEM_DEFENSE_REPULSION,					// tag ?
 		"icons/i_ring.m8",			// Icon name (char *)
	},

	{
		"item_defense_shield",					// Spawnname
		"lshield",								// Pickup name (char *)
		GM_SHIELD,										// pickup message
		GM_NOSHIELD,										// can`t use message
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,	 								// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		MANA_USE_SHIELD,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		NULL,									// void * ?
		ITEM_DEFENSE_SHIELD,					// tag ?
 		"icons/i_shield.m8",			// Icon name (char *)
	},

	{
		"item_defense_teleport",				// Spawnname
		"tele",									// Pickup name (char *)
		GM_TELEPORT,										// pickup message
		GM_NOTELEPORT,										// can`t use message
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,  									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		MANA_USE_TELEPORT,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		NULL,									// void * ?
		ITEM_DEFENSE_TELEPORT,					// tag ?
 		"icons/i_tele.m8",					// Icon name (char *)
	},

	{
		"item_defense_polymorph",			    // Spawnname
		"morph",						    // Pickup name (char *)
		GM_MORPH,										// pickup message
		GM_NOMORPH,										// can`t use message
		NULL,								    // Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		MANA_USE_POLYMORPH,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		NULL,									// void * ?
		ITEM_DEFENSE_POLYMORPH,					// tag ?
 		"icons/i_morph.m8",					// Icon name (char *)
	},

	{
		"item_defense_meteorbarrier",			// Spawnname
		"meteor",								// Pickup name (char *)
		GM_METEOR,										// pickup message
		GM_NOMETEOR,										// can`t use message
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		MANA_USE_METEORS,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		NULL,									// void * ?
		ITEM_DEFENSE_METEORBARRIER,				// tag ?
 		"icons/i_meteor.m8",			// Icon name (char *)
	},


	// =============================================================================================

	// Ammo items.

	{
		"item_mana_offensive_half",				// Spawnname
		"Off-mana",								// Pickup name (char *)
		GM_OFFMANAS,										// pickup message
		0,
		NULL, 									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		HALF_OFF_MANA,							// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_OFFENSE,					// flags
		NULL,									// void * ?
		ITEM_AMMO_MANA_OFFENSIVE_HALF,			// tag ?
		NULL,									// Icon name (char *)
	},

	{
		"item_mana_offensive_full",				// Spawnname
		"Off-mana",								// Pickup name (char *)
		GM_OFFMANAB,										// pickup message
		0,
		NULL, 									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		FULL_OFF_MANA,							// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_OFFENSE,					// flags
		NULL,									// void * ?
		ITEM_AMMO_MANA_OFFENSIVE_FULL,			// tag ?
		NULL,									// Icon name (char *)
	},

	{
		"item_mana_defensive_half",				// Spawnname
		"Def-mana",								// Pickup name (char *)
		GM_DEFMANAS,							// pickup message
		0,
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		HALF_DEF_MANA,							// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_DEFENSE,					// flags
		NULL,									// void * ?
		ITEM_AMMO_MANA_DEFENSIVE_HALF,			// tag ?
		NULL,									// Icon name (char *)
	},

	{
		"item_mana_defensive_full",				// Spawnname
		"Def-mana",								// Pickup name (char *)
		GM_DEFMANAB,							// pickup message
		0,
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/picup.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		0,										// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_NONE,								// Player animation sequence to engage when used
		ASEQ_NONE,								// Alternate player animation sequence to engage when used
		-1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		FULL_DEF_MANA,							// Ammo/ammo use per shot
		NULL,									// Ammo (char *)
		IT_AMMO | IT_DEFENSE,					// flags
		NULL,									// void * ?
		ITEM_AMMO_MANA_DEFENSIVE_FULL,			// tag ?
		NULL,									// Icon name (char *)
	},


	{
		"item_mana_combo_quarter",					  // Spawnname
		"Def-mana",									  // Pickup name (char *)
		GM_COMBMANAS,										// pickup message
		0,
		NULL,								  // Pickup (f)
		NULL,										  // Use (f)
		NULL,										  // Drop	(f)
		NULL,										  // Think (f)
		"player/picup.wav",							  // Pickup sound (char *)
		NULL,										  // world model (char *)
		0,											  // world model flags
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)
		0,											  // Number of digits to display
		HALF_COMBO_MANA,							  // Ammo/ammo use per shot
		NULL,										  // Ammo (char *)
		IT_AMMO,									  // flags
		NULL,										  // void * ?
		ITEM_AMMO_MANA_COMBO_QUARTER,				  // tag ?
		NULL,										  // Icon name (char *)
	},

	{
		"item_mana_combo_half",					  // Spawnname
		"Def-mana",									  // Pickup name (char *)
		GM_COMBMANAB,										// pickup message
		0,
		NULL,								  // Pickup (f)
		NULL,										  // Use (f)
		NULL,										  // Drop	(f)
		NULL,										  // Think (f)
		"player/picup.wav",							  // Pickup sound (char *)
		NULL,										  // world model (char *)
		0,											  // world model flags
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)
		0,											  // Number of digits to display
		FULL_COMBO_MANA,							  // Ammo/ammo use per shot
		NULL,										  // Ammo (char *)
		IT_AMMO,									  // flags
		NULL,										  // void * ?
		ITEM_AMMO_MANA_COMBO_HALF,					  // tag ?
		NULL,										// Icon name (char *)
	},

	{
		"item_ammo_redrain",						  // Spawnname
		"Red-Rain-Arrows",							  // Pickup name (char *)
		GM_STORMARROWS,										// pickup message
		0,
		NULL,								  // Pickup (f)
		NULL,										  // Use (f)
		NULL,										  // Drop	(f)
		NULL,										  // Think (f)
		"player/picup.wav",							  // Pickup sound (char *)
		NULL,										  // world model (char *)
		0,											  // world model flags
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)
		0,											  // Number of digits to display
		AMMO_COUNT_REDRAINBOW,						  // Ammo/ammo use per shot
		NULL,										  // Ammo (char *)
		IT_AMMO | IT_OFFENSE,						  // flags
		NULL,										  // void * ?
		ITEM_AMMO_REDRAIN,								  // tag ?
		"icons/i_ammo-redrain.m8",								  // Icon name (char *)
	},

	{
		"item_ammo_phoenix",							  // Spawnname
		"Phoenix-Arrows",							  // Pickup name (char *)
		GM_PHOENARROWS,										// pickup message
		0,
		NULL,										  // Pickup (f)
		NULL,										  // Use (f)
		NULL,										  // Drop	(f)
		NULL,										  // Think (f)
		"player/picup.wav",							  // Pickup sound (char *)
		NULL,										  // world model (char *)
		0,											  // world model flags
		PICKUP_MIN,									// Bounding box mins
		PICKUP_MAX,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,										// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)
		0,											  // Number of digits to display
		AMMO_COUNT_PHOENIXBOW,						  // Ammo/ammo use per shot
		NULL,										  // Ammo (char *)
		IT_AMMO | IT_OFFENSE,						  // flags
		NULL,										// void * ?
		ITEM_AMMO_PHOENIX,							  // tag ?
		"icons/i_ammo-phoen.m8",								  // Icon name (char *)
	},

	{
		"item_ammo_hellstaff",						  // Spawnname
		"Hell-staff-ammo",							  // Pickup name (char *)
		GM_HELLORB,										// pickup message
		0,
		NULL,										  // Pickup (f)
		NULL,										  // Use (f)
		NULL,										  // Drop	(f)
		NULL,										  // Think (f)
		"player/picup.wav",							  // Pickup sound (char *)
		NULL,										  // world model (char *)
		0,											  // world model flags
		PICKUP_MIN,									// Bounding box mins
		PICKUP_MAX,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											  // Max uses (-1 = inifinite)
		0,											  // Number of digits to display
		AMMO_COUNT_HELLSTAFF,						  // Ammo/ammo use per shot
		NULL,										  // Ammo (char *)
		IT_AMMO | IT_OFFENSE,						  // flags
		NULL,										  // void * ?
		ITEM_AMMO_HELLSTAFF,						  // tag ?
		"icons/i_ammo-hellstaff.m8",				  // Icon name (char *)
	},

	// ============================================================================================

	// Other items.

	{
		"item_health_half",							// Spawnname
		"Minor health",								// Pickup name (char *)
		GM_HEALTHVIAL,										// pickup message
		0,
		NULL, 								// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"*gethealth.wav",							// Pickup sound (char *)
		"models/items/health/healthsmall/tris.fm",	// world model (char *)
		0,											// world model flags
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		-1,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		10,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_HEALTH | EF_ALWAYS_ADD_EFFECTS, 			// flags
		NULL,										// void * ?
		ITEM_HEALTH1,								// tag ?
		NULL,										// Icon name (char *)
	},

	{
		"item_health_full",							// Spawnname
		"Major health",								// Pickup name (char *)
		GM_HEALTHPOTION,										// pickup message
		0,
		NULL, 								// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"*gethealth.wav",							// Pickup sound (char *)
		"models/items/health/healthbig/tris.fm",	// world model (char *)
		0,											// world model flags
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		30,										// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_HEALTH | EF_ALWAYS_ADD_EFFECTS, 			// flags
		NULL,										// void * ?
		ITEM_HEALTH2,								// tag ?
		NULL,								// Icon name (char *)
	},

	// ============================================================================================

	// Puzzle Pieces

	{
		"item_puzzle_townkey",						// Spawnname
		"Town Key",									// Pickup name (char *)
		GM_F_TOWNKEY,										// pickup message
		GM_NEED_TOWNKEY,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,		// world model (char *)
		0,											// world model flags
		-8, -8, -4,									// Bounding box mins
		 8,  8,  4,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_TOWNKEY,							 				// tag ?
		"icons/p_townkey.m8",					// Icon name (char *)
	},

	{
		"item_puzzle_cog",							// Spawnname
		"Cog",										// Pickup name (char *)
		GM_F_COG,										// pickup message
		GM_NEED_COG,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,			// world model (char *)
		0,											// world model flags
		-10, -10, -24,								// Bounding box mins
		 10,  10,  20,								// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_COG,							 				// tag ?
		"icons/p_cog.m8",							// Icon name (char *)
	},

	{
		"item_puzzle_shield",					// Spawnname
		"Defensive Shield",						// Pickup name (char *)
		GM_F_SHIELD,										// pickup message
		GM_NEED_SHIELD,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		-2, -6, -12,							// Bounding box mins
		 2,  6,  12,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_SHIELD,				 							// tag ?
		"icons/p_shield.m8",						// Icon name (char *)
	},

	{
		"item_puzzle_potion",						// Spawnname
		"Potion",									// Pickup name (char *)
		GM_F_POTION,										// pickup message
		GM_NEED_POTION,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,		// world model (char *)
		0,											// world model flags
		-3, -3, -10,									// Bounding box mins
		 3,  3,  10,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_POTION,				 							// tag ?
		"icons/p_potion.m8",						// Icon name (char *)
	},


	{
		"item_puzzle_plazacontainer",				// Spawnname
		"Container",								// Pickup name (char *)
		GM_F_CONT,										// pickup message
		GM_NEED_CONT,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		-6, -6, -8,									// Bounding box mins
		 6,  6,  6,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_CONT,				 							// tag ?
		"icons/p_plazajug.m8",						// Icon name (char *)
	},


	{
		"item_puzzle_slumcontainer",				// Spawnname
		"Full Container",							// Pickup name (char *)
		GM_F_CONTFULL,										// pickup message
		GM_NEED_CONTFULL,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		-6, -6, -8,							// Bounding box mins
		 6,  6,  6,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_SLUMCONT,								// tag ?
		"icons/p_jugfill.m8",						// Icon name (char *)
	},

	{
		"item_puzzle_crystal",						// Spawnname
		"Crystal",									// Pickup name (char *)
		GM_F_CRYSTAL,										// pickup message
		GM_NEED_CRYSTAL,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_CRYSTAL,				 							// tag ?
		"icons/p_crystal.m8",						// Icon name (char *)
	},

	{
		"item_puzzle_canyonkey",					// Spawnname
		"Canyon Key",								// Pickup name (char *)
		GM_F_CANYONKEY,										// pickup message
		GM_NEED_CANYONKEY,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_CANKEY,				 							// tag ?
		"icons/p_canyonkey.m8",						// Icon name (char *)
	},

	{
		"item_puzzle_hive2amulet",					// Spawnname
		"Hive 2 Amulet",							// Pickup name (char *)
		GM_F_AMULET,										// pickup message
		GM_NEED_AMULET,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_AMULET,				 							// tag ?
		"icons/p_tcheckrikbust.m8",					// Icon name (char *)
	},

	{
		"item_puzzle_hive2spear",					// Spawnname
		"Hive 2 Spear",								// Pickup name (char *)
		GM_F_SPEAR,										// pickup message
		GM_NEED_SPEAR,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_SPEAR,				 							// tag ?
		"icons/p_spear.m8",						// Icon name (char *)
	},

	{
		"item_puzzle_hive2gem",						// Spawnname
		"Hive 2 Gem",								// Pickup name (char *)
		GM_F_GEM,										// pickup message
		GM_NEED_GEM,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		PICKUP_MIN,							// Bounding box mins
		PICKUP_MAX,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_GEM,				 							// tag ?
		"icons/p_tcheckrikgem.m8",						// Icon name (char *)
	},

	{
		"item_puzzle_minecartwheel",				// Spawnname
		"Minecart Wheel",							// Pickup name (char *)
		GM_F_CARTWHEEL,										// pickup message
		GM_NEED_CARTWHEEL,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,		// world model (char *)
		0,											// world model flags
		-1,-6,-6,							// Bounding box mins
		 1, 6, 6,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_WHEEL,				 							// tag ?
		"icons/p_wheel.m8",							// Icon name (char *)
	},

	{
		"item_puzzle_ore",							// Spawnname
		"Ore",										// Pickup name (char *)
		GM_F_UNREFORE,										// pickup message
		GM_NEED_UNREFORE,										// can`t use message
		NULL,		 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		-10,-10,-8,							// Bounding box mins
		 10, 10, 8,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_ORE	,				 							// tag ?
		"icons/p_oreunrefined.m8",						// Icon name (char *)
	},

	{
		"item_puzzle_refinedore",					// Spawnname
		"Refined Ore",								// Pickup name (char *)
		GM_F_REFORE,										// pickup message
		GM_NEED_REFORE,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		-3,-12,-2,							// Bounding box mins
		 3, 12, 2,							// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_REF_ORE,	 							// tag ?
		"icons/p_orerefined.m8",						// Icon name (char *)
	},

	{
		"item_puzzle_dungeonkey",					// Spawnname
		"Dungeon Key",								// Pickup name (char *)
		GM_F_DUNGEONKEY,										// pickup message
		GM_NEED_DUNGEONKEY,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		-1,-18,-9,									// Bounding box mins
		 1, 18, 9,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_DUNKEY,	 							// tag ?
		"icons/p_dungeonkey.m8",						// Icon name (char *)
	},

	{
		"item_puzzle_cloudkey",						// Spawnname
		"Cloud Key",								// Pickup name (char *)
		GM_F_CLOUDKEY,										// pickup message
		GM_NEED_CLOUDKEY,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		-8, -8, -3,									// Bounding box mins
		 8,  8,  3,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_CLOUDKEY,				 							// tag ?
		"icons/p_cloudkey.m8",						// Icon name (char *)
	},


	{
		"item_puzzle_highpriestesskey",				// Spawnname
		"Key",										// Pickup name (char *)
		GM_F_HIGHKEY,										// pickup message
		GM_NEED_HIGHKEY,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		-12,-12, -6,								// Bounding box mins
		 12, 12,  6,								// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_HIVEKEY,				 							// tag ?
		"icons/p_hivekey.m8",						// Icon name (char *)
	},

	{
		"item_puzzle_highpriestesssymbol",			// Spawnname
		"Symbol",									// Pickup name (char *)
		GM_F_SYMBOL,										// pickup message
		GM_NEED_SYMBOL,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		-12,-12,-4,									// Bounding box mins
		 12, 12, 4,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_HPSYM,				 							// tag ?
		"icons/p_queenkey.m8",						// Icon name (char *)
	},

	{
		"item_puzzle_tome",							// Spawnname
		"Tome",										// Pickup name (char *)
		GM_F_TOME,										// pickup message
		GM_NEED_TOME,										// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		-12,-12,-4,									// Bounding box mins
		 12, 12, 4,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_TOME,				 							// tag ?
		"icons/p_tomepower.m8",						// Icon name (char *)
	},
	{
		"item_puzzle_tavernkey",					// Spawnname
		"Tavern Key",								// Pickup name (char *)
		GM_F_TAVERNKEY,								// pickup message
		GM_NEED_TAVERNKEY,							// can`t use message
		NULL,				 						// Pickup (f)
		NULL,										// Use (f)
		NULL,										// Drop	(f)
		NULL,										// Think (f)
		"player/picup.wav",							// Pickup sound (char *)
		NULL,	// world model (char *)
		0,											// world model flags
		-12,-12,-4,									// Bounding box mins
		 12, 12, 4,									// Bounding box maxs
		ASEQ_NONE,									// Player animation sequence to engage when used
		ASEQ_NONE,									// Alternate player animation sequence to engage when used
		0,											// Max uses (-1 = inifinite)
		0,											// Number of digits to display
		0,											// Ammo/ammo use per shot
		NULL,										// Ammo (char *)
		IT_PUZZLE,									// flags
		NULL,										// void * ?
		ITEM_TAVERNKEY,				 				// tag ?
		"icons/p_tavernkey.m8",					// Icon name (char *)
	},

	{
		"item_defense_tornado",					// Spawnname
		"tornado",								// Pickup name (char *)
		GM_TORNADO,										// pickup message
		GM_NOTORNADO,										// can`t use message
		NULL,									// Pickup (f)
		NULL,									// Use (f)
		NULL,									// Drop	(f)
		NULL,									// Think (f)
		"player/getweapon.wav",						// Pickup sound (char *)
		NULL,									// world model (char *)
		EF_ROTATE,								// world model flags
		PICKUP_MIN,								// Bounding box mins
		PICKUP_MAX,								// Bounding box maxs
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when used
		ASEQ_SPELL_DEF,							// Player animation sequence to engage when powered
		1,										// Max uses (-1 = inifinite)
		0,										// Number of digits to display
		MANA_USE_TORNADO,						// Ammo/ammo use per shot
		"Def-mana",								// Ammo (char *)
		IT_DEFENSE,								// flags
		NULL,									// void * ?
		ITEM_DEFENSE_TORNADO,					// tag ?
 		"icons/i_tornado.m8",					// Icon name (char *)
	},


	// End of list marker.

	{NULL}
};

gitem_t itemlist[MAX_ITEMS];

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
	memset(itemlist, 0, sizeof(itemlist));
	memcpy(itemlist, gameitemlist, sizeof(gameitemlist));
	game.num_items = sizeof(gameitemlist) / sizeof(gameitemlist[0]) - 1;

	// ********************************************************************************************
	// Setup item function pointers which yield pick-up, use, drop and weaponthink functionality.
	// ********************************************************************************************

	// Leave index 0 empty.

	// weapon_swordstaff
	// This can't be placed in the editor

	itemlist[1].pickup = Pickup_Weapon;
	itemlist[1].use=playerExport->Weapon_EquipSwordStaff;
	itemlist[1].weaponthink=WeaponThink_SwordStaff;

	// weapon_flyingfist
	// This can't be placed in the editor

	itemlist[2].pickup = Pickup_Weapon;
	itemlist[2].use=playerExport->Weapon_EquipSpell;
	itemlist[2].weaponthink=WeaponThink_FlyingFist;

	// item_weapon_hellstaff
/*
 * QUAKED item_weapon_hellstaff (.3 .3 1) (-16 -16 -16) (16 16 16) COOP_ONLY
Pickup for the hellstaff weapon.
*/

	itemlist[3].pickup = Pickup_Weapon;
	itemlist[3].use=playerExport->Weapon_EquipHellStaff;
	itemlist[3].weaponthink=WeaponThink_HellStaff;

	// item_weapon_magicmissile
/*
 * QUAKED item_weapon_magicmissile (.3 .3 1) (-16 -16 -16) (16 16 16) COOP_ONLY
Pickup for the Magic Missile weapon.
*/

	itemlist[4].pickup = Pickup_Weapon;
	itemlist[4].use=playerExport->Weapon_EquipSpell;
	itemlist[4].weaponthink=WeaponThink_MagicMissileSpread;

	// item_weapon_redrain_bow
/*
 * QUAKED item_weapon_redrain_bow (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
Pickup for the Red Rain Bow weapon.
*/

	itemlist[5].pickup = Pickup_Weapon;
	itemlist[5].use=playerExport->Weapon_EquipBow;
	itemlist[5].weaponthink=WeaponThink_RedRainBow;

	// item_weapon_firewall
/*
 * QUAKED item_weapon_firewall (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Fire Wall weapon.
*/

	itemlist[6].pickup = Pickup_Weapon;
	itemlist[6].use=playerExport->Weapon_EquipSpell;
	itemlist[6].weaponthink=WeaponThink_Firewall;

	// item_weapon_phoenixbow
/*
 * QUAKED item_weapon_phoenixbow (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
Pickup for the Phoenix Bow weapon.
*/

	itemlist[7].pickup = Pickup_Weapon;
	itemlist[7].use=playerExport->Weapon_EquipBow;
	itemlist[7].weaponthink=WeaponThink_PhoenixBow;

	// item_weapon_sphereofannihilation
/*
 * QUAKED item_weapon_sphereofannihilation (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
Pickup for the Sphere Annihilation weapon.
*/

	itemlist[8].pickup = Pickup_Weapon;
	itemlist[8].use=playerExport->Weapon_EquipSpell;
	itemlist[8].weaponthink=WeaponThink_SphereOfAnnihilation;

	// item_weapon_maceballs
/*
 * QUAKED item_weapon_maceballs (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
Pickup for the Mace Balls weapon.
*/

	itemlist[9].pickup = Pickup_Weapon;
	itemlist[9].use=playerExport->Weapon_EquipSpell;
	itemlist[9].weaponthink=WeaponThink_Maceballs;

	// item_defense_powerup
	// This can't be placed in the editor

	itemlist[10].pickup = Pickup_Defense;
	itemlist[10].use = Use_Defence;
	itemlist[10].weaponthink = DefenceThink_Powerup;

	// item_defense_ringofrepulsion
/*
 * QUAKED item_defense_ringofrepulsion (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Ring of Repulsion defensive spell.
*/

	itemlist[11].pickup = Pickup_Defense;
	itemlist[11].use = Use_Defence;
	itemlist[11].weaponthink = DefenceThink_RingOfRepulsion;

	// item_defense_shield
/*
 * QUAKED item_defense_shield (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Shield defensive spell.
*/

	itemlist[12].pickup = Pickup_Defense;
	itemlist[12].use = Use_Defence;
	itemlist[12].weaponthink = DefenceThink_Shield;

	// item_defense_teleport
/*
 * QUAKED item_defense_teleport (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Teleport defensive spell.
*/

	itemlist[13].pickup = Pickup_Defense;
	itemlist[13].use = Use_Defence;
	itemlist[13].weaponthink = DefenceThink_Teleport;

	// item_defense_polymorph
/*
 * QUAKED item_defense_polymorph (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Polymorph Barrier defensive spell.
*/

	itemlist[14].pickup = Pickup_Defense;
	itemlist[14].use = Use_Defence;
	itemlist[14].weaponthink = DefenceThink_Morph;

	// item_defense_meteorbarrier
/*
 * QUAKED item_defense_meteorbarrier (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Meteor Barrier defensive spell.
*/

	itemlist[15].pickup = Pickup_Defense;
	itemlist[15].use = Use_Defence;
	itemlist[15].weaponthink = DefenceThink_MeteorBarrier;

	// item_mana_offensive_half
/*
 * QUAKED item_mana_offensive_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the offensive mana (50 points).
*/
	itemlist[16].pickup = Pickup_Mana;

	// item_mana_offensive_full
/*
 * QUAKED item_mana_offensive_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the offensive mana (100 points).
*/

	itemlist[17].pickup = Pickup_Mana;

	// item_mana_defensive_half
/*
 * QUAKED item_mana_defensive_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the defensive mana (50 points).
*/

	itemlist[18].pickup = Pickup_Mana;

	// item_mana_defensive_full
/*
 * QUAKED item_mana_defensive_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the defensive mana (100 points).
*/

	itemlist[19].pickup = Pickup_Mana;

	// item_mana_combo_quarter
/*
 * QUAKED item_mana_combo_quarter (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for both defensive & offensive mana (25 points).
*/

	itemlist[20].pickup = Pickup_Mana;

	// item_mana_combo_half
/*
 * QUAKED item_mana_combo_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for both defensive & offensive mana (50 points).
*/

	itemlist[21].pickup = Pickup_Mana;

	// item_ammo_redrain
/*
 * QUAKED item_ammo_redrain (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup ammo for the Red Rain Bow
*/

	itemlist[22].pickup = Pickup_Ammo;

	// item_ammo_phoenix
/*
 * QUAKED item_ammo_phoenix (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup ammo for the Phoenix Bow
*/

	itemlist[23].pickup = Pickup_Ammo;

	// item_ammo_hellstaff
/*
 * QUAKED item_ammo_hellstaff (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup ammo for the Hellstaff
*/

	itemlist[24].pickup = Pickup_Ammo;

	// item_health_half
/*
 * QUAKED item_health_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup health (10 points)
*/

	itemlist[25].pickup = Pickup_Health;

	// item_health_full
/*
 * QUAKED item_health_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup health (30 points)
*/

	itemlist[26].pickup = Pickup_Health;

/*
 * QUAKED item_puzzle_townkey (.3 .3 1) (-8 -8 -4) (8 8 4)  x NO_DROP
Key puzzle piece
Town Level
NO_DROP - won't drop to ground

*/
	itemlist[27].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_cog (.3 .3 1) (-10 -10 -24) (10 10 20)  x  NO_DROP
Cog puzzle piece
Palace level
NO_DROP - won't drop to ground
*/
	itemlist[28].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_shield (.3 .3 1) (-2 -6 -12) (2 6 12)  x  NO_DROP
Sithra Shield puzzle item
Healer Level
NO_DROP - won't drop to ground
*/
	itemlist[29].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_potion (.3 .3 1) (-3 -3 -10) (3 3 10)  x  NO_DROP
Potion puzzle item
Healer Level
NO_DROP - won't drop to ground
*/
	itemlist[30].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_plazacontainer (.3 .3 1) (-6 -6 -8) (6 6 6)  x  NO_DROP
Container puzzle item
Plaza Level
NO_DROP - won't drop to ground
*/
	itemlist[31].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_slumcontainer (.3 .3 1) (-6 -6 -8) (6 6 6)  x  NO_DROP
Full Container puzzle item
Slum Level
NO_DROP - won't drop to ground
*/
	itemlist[32].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_crystal (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Crystal puzzle item
Academic Level
NO_DROP - won't drop to ground
*/
	itemlist[33].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_canyonkey (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Key puzzle item
Canyon Level
NO_DROP - won't drop to ground
*/
	itemlist[34].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_hive2amulet (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Amulet puzzle item
Hive 2 Level
NO_DROP - won't drop to ground
*/
	itemlist[35].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_hive2spear (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Spear puzzle item
Hive 2 Level
NO_DROP - won't drop to ground
*/
	itemlist[36].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_hive2gem (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Gem puzzle item
Hive 2 Level
NO_DROP - won't drop to ground
*/
	itemlist[37].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_minecartwheel (.3 .3 1) (-1 -6 -6) (1 6 6)  x  NO_DROP
Mine Cart Wheel puzzle item
Mine 1 Level
NO_DROP - won't drop to ground
*/
	itemlist[38].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_ore (.3 .3 1) (-10 -10 -8) (10 10 8)  x  NO_DROP
Unrefined Ore puzzle item
Mine 2 Level
NO_DROP - won't drop to ground
*/
	itemlist[39].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_refinedore (.3 .3 1) (-3 -12 -2) (3 12 2) x   NO_DROP
Refined Ore puzzle item
Mine 2 Level
NO_DROP - won't drop to ground
*/
	itemlist[40].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_dungeonkey (.3 .3 1) (-1 -18 -9) (1 18 9)  x  NO_DROP
Amulet puzzle item
Dungeon Level
NO_DROP - won't drop to ground
*/
	itemlist[41].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_cloudkey (.3 .3 1) (-8 -8 -3) (8 8 6)  x  NO_DROP
Key puzzle item
Cloud Quarters 2 Level
NO_DROP - won't drop to ground
*/
	itemlist[42].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_highpriestesskey (.3 .3 1) (-12 -12 -6) (12 12 6) x   NO_DROP
Key puzzle item
High Priestess Level
NO_DROP - won't drop to ground
*/
	itemlist[43].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_highpriestesssymbol (.3 .3 1) (-12 -12 -4) (12 12 4) x   NO_DROP
Key puzzle item
High Priestess Level
NO_DROP - won't drop to ground
*/
	itemlist[44].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_tome (.3 .3 1) (-12 -12 -4) (12 12 4)  x  NO_DROP
Tome puzzle piece
2 Cloud Levels
NO_DROP - won't drop to ground
*/
	itemlist[45].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_tavernkey (.3 .3 1) (-8 -8 -4) (8 8 4)    x   NO_DROP
Key puzzle piece
Ssdocks Level
NO_DROP - won't drop to ground
*/
	itemlist[46].pickup = Pickup_Puzzle;

	// item_defense_tornado
/*
 * QUAKED item_defense_tornado (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Tornado defensive spell.
*/
	itemlist[47].pickup = Pickup_Defense;
	itemlist[47].use = Use_Defence;
	itemlist[47].weaponthink = DefenceThink_Tornado;
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
