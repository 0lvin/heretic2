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
#include "player/library/player.h"
#include "player/library/p_weapon.h"
#include "header/g_weapon.h"
#include "player/library/p_anims.h"
#include "player/library/p_anim_data.h"
#include "common/fx.h"
#include "common/h2rand.h"
#include "header/g_itemstats.h"

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
	gitem_t* p_itemlist = playerExport->GetPlayerItems();

	if ((index == 0) || (index >= game.num_items))
	{
		return NULL;
	}

	return &p_itemlist[index];
}

gitem_t *
FindItemByClassname(const char *classname)
{
	int i;
	gitem_t *it;
	gitem_t* p_itemlist = playerExport->GetPlayerItems();

	if (!classname)
	{
		return NULL;
	}

	it = p_itemlist;

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
	gitem_t* p_itemlist = playerExport->GetPlayerItems();

	if (!pickup_name)
	{
		return NULL;
	}

	it = p_itemlist;

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
	if(ent->team)
	{
		edict_t	*Master;
		int		Count;
		int		Choice;

		Master=ent->teammaster;

		for(Count=0,ent=Master;ent;ent=ent->chain,Count++)
			;

		Choice=irand(0, Count - 1);

		for(Count=0,ent=Master;Count<Choice;ent=ent->chain,Count++)
			;
	}

	ent->solid=SOLID_TRIGGER;

	gi.linkentity(ent);

	// Create a respawn client-effect (this isn't currenlty doing anything on the client).

	//gi.CreateEffect(ent,FX_ITEM_RESPAWN,CEF_OWNERS_ORIGIN,ent->s.origin,NULL);

	// So it'll get sent to the client again.

//	ent->svflags &= ~SVF_NOCLIENT;

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
SetRespawn(edict_t *ent)
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

	item = playerExport->FindItemByClassname(ent->classname);

	if (!other->client->playerinfo.pers.inventory.Items[playerExport->GetItemIndex(ent->item)])
	{
		other->client->playerinfo.pers.inventory.Items[playerExport->GetItemIndex(ent->item)] = 1;

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

	if(!player->client->playerinfo.pers.inventory.Items[playerExport->GetItemIndex(item)])
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

		player->client->playerinfo.pers.inventory.Items[playerExport->GetItemIndex(item)] = 1;

		if(count)
		{
			newitem = playerExport->FindItem(item->ammo);
			Add_Ammo(player, newitem,count);
		}

		// Now decide if we want to swap weapons or not.

		if (player->client->playerinfo.pers.autoweapon)
		{
			// If this new weapon is a higher value than the one we currently have, swap the current
			// weapon for the new one.

			if (playerExport->GetItemIndex(item) > playerExport->GetItemIndex(player->client->playerinfo.pers.weapon))
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
				newitem = playerExport->FindItemByClassname("item_ammo_hellstaff");
				count = AMMO_COUNT_HELLSTAFF;
			}
			else if (item->tag == ITEM_WEAPON_REDRAINBOW)
			{
				newitem = playerExport->FindItemByClassname("item_ammo_redrain");
				count = AMMO_COUNT_REDRAINBOW;
			}
			else if (item->tag == ITEM_WEAPON_PHOENIXBOW)
			{
				newitem = playerExport->FindItemByClassname("item_ammo_phoenix");
				count = AMMO_COUNT_PHOENIXBOW;
			}
			else
			{
				newitem = playerExport->FindItemByClassname("item_mana_offensive_half");
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
	if(!player->client->playerinfo.pers.inventory.Items[playerExport->GetItemIndex(item)])
	{
		player->client->playerinfo.pers.inventory.Items[playerExport->GetItemIndex(item)]=1;

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

	index = playerExport->GetItemIndex(item);

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

	if (!ent || !item)
	{
		return false;
	}

	if (!ent->client)
	{
		return false;
	}

	if ((item->tag == ITEM_AMMO_MANA_OFFENSIVE_HALF) || (item->tag == ITEM_AMMO_MANA_OFFENSIVE_FULL))
	{
		item = playerExport->FindItemByClassname("item_mana_offensive_half");
		max = ent->client->playerinfo.pers.max_offmana;
		return(Add_AmmoToInventory (ent,item,count,max));
	}
	else if ((item->tag == ITEM_AMMO_MANA_DEFENSIVE_HALF) || (item->tag == ITEM_AMMO_MANA_DEFENSIVE_FULL))
	{
		item = playerExport->FindItemByClassname("item_mana_defensive_half");
		max = ent->client->playerinfo.pers.max_defmana;
		return(Add_AmmoToInventory (ent,item,count,max));
	}
	else if ((item->tag == ITEM_AMMO_MANA_COMBO_QUARTER) || (item->tag == ITEM_AMMO_MANA_COMBO_HALF))
	{
		item = playerExport->FindItemByClassname("item_mana_offensive_half");
		max = ent->client->playerinfo.pers.max_offmana;

		bo = Add_AmmoToInventory (ent,item,count,max);

		item = playerExport->FindItemByClassname("item_mana_defensive_half");
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

	index = playerExport->GetItemIndex(item);
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

/*
===============
Pickup_Health
===============
*/

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

	if(ent->flags&FL_RESPAWN)
	{
		// The item should respawn.

		SetRespawn(ent);
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

//	ent->svflags &= ~SVF_NOCLIENT;
	ent->use = NULL;

	if (ent->spawnflags & 2)	// NO_TOUCH
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

static void
FixObjectPosition(edict_t *ent)
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
	trace_t tr;
	vec3_t dest;
	float *v;

	if (!ent)
	{
		return;
	}

	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_STATIC;
	ent->touch = Touch_Item;
	ent->think = NULL;

	if (!(ent->spawnflags & ITEM_NO_DROP))
	{
		v = tv(0, 0, -128);
		VectorAdd(ent->s.origin, v, dest);

		tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);

		if (tr.startsolid)
		{
			FixObjectPosition(ent);

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
		ammo = playerExport->FindItem (it->ammo);
		if (ammo != it)
			PrecacheItem (ammo);
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

// ************************************************************************************************
// IsItem
// ------
// ************************************************************************************************

gitem_t	*IsItem(edict_t *ent)
{
	gitem_t	*item;
	int i;

	if(!ent->classname)
	{
		return NULL;
	}

	for(i = 0, item = playerExport->GetPlayerItems(); i < game.num_items; ++i, ++item)
	{
		if(!item->classname)
		{
			continue;
		}

		if(!strcmp(item->classname, ent->classname))
		{
			// Found it.

			return item;
		}
	}

	return NULL;
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


// ************************************************************************************************
// InitItems
// -----------
// ************************************************************************************************

void InitItems(void)
{
	gitem_t* p_itemlist = playerExport->GetPlayerItems();

	// ********************************************************************************************
	// Setup item function pointers which yield pick-up, use, drop and weaponthink functionality.
	// ********************************************************************************************

	// Leave index 0 empty.

	// weapon_swordstaff
	// This can't be placed in the editor

	p_itemlist[1].pickup=Pickup_Weapon;
	p_itemlist[1].use=playerExport->Weapon_EquipSwordStaff;
	p_itemlist[1].weaponthink=WeaponThink_SwordStaff;

	// weapon_flyingfist
	// This can't be placed in the editor

	p_itemlist[2].pickup=Pickup_Weapon;
	p_itemlist[2].use=playerExport->Weapon_EquipSpell;
	p_itemlist[2].weaponthink=WeaponThink_FlyingFist;

	// item_weapon_hellstaff
/*
 * QUAKED item_weapon_hellstaff (.3 .3 1) (-16 -16 -16) (16 16 16) COOP_ONLY
Pickup for the hellstaff weapon.
*/

	p_itemlist[3].pickup=Pickup_Weapon;
	p_itemlist[3].use=playerExport->Weapon_EquipHellStaff;
	p_itemlist[3].weaponthink=WeaponThink_HellStaff;

	// item_weapon_magicmissile
/*
 * QUAKED item_weapon_magicmissile (.3 .3 1) (-16 -16 -16) (16 16 16) COOP_ONLY
Pickup for the Magic Missile weapon.
*/

	p_itemlist[4].pickup=Pickup_Weapon;
	p_itemlist[4].use=playerExport->Weapon_EquipSpell;
	p_itemlist[4].weaponthink=WeaponThink_MagicMissileSpread;

	// item_weapon_redrain_bow
/*
 * QUAKED item_weapon_redrain_bow (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
Pickup for the Red Rain Bow weapon.
*/

	p_itemlist[5].pickup=Pickup_Weapon;
	p_itemlist[5].use=playerExport->Weapon_EquipBow;
	p_itemlist[5].weaponthink=WeaponThink_RedRainBow;

	// item_weapon_firewall
/*
 * QUAKED item_weapon_firewall (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Fire Wall weapon.
*/

	p_itemlist[6].pickup=Pickup_Weapon;
	p_itemlist[6].use=playerExport->Weapon_EquipSpell;
	p_itemlist[6].weaponthink=WeaponThink_Firewall;

	// item_weapon_phoenixbow
/*
 * QUAKED item_weapon_phoenixbow (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
Pickup for the Phoenix Bow weapon.
*/

	p_itemlist[7].pickup=Pickup_Weapon;
	p_itemlist[7].use=playerExport->Weapon_EquipBow;
	p_itemlist[7].weaponthink=WeaponThink_PhoenixBow;

	// item_weapon_sphereofannihilation
/*
 * QUAKED item_weapon_sphereofannihilation (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
Pickup for the Sphere Annihilation weapon.
*/

	p_itemlist[8].pickup=Pickup_Weapon;
	p_itemlist[8].use=playerExport->Weapon_EquipSpell;
	p_itemlist[8].weaponthink=WeaponThink_SphereOfAnnihilation;

	// item_weapon_maceballs
/*
 * QUAKED item_weapon_maceballs (.3 .3 1) (-16 -16 -16) (16 16 16)  COOP_ONLY
Pickup for the Mace Balls weapon.
*/

	p_itemlist[9].pickup=Pickup_Weapon;
	p_itemlist[9].use=playerExport->Weapon_EquipSpell;
	p_itemlist[9].weaponthink=WeaponThink_Maceballs;

	// item_defense_powerup
	// This can't be placed in the editor

	p_itemlist[10].pickup=Pickup_Defense;
	p_itemlist[10].use=Use_Defence;
	p_itemlist[10].weaponthink=DefenceThink_Powerup;

	// item_defense_ringofrepulsion
/*
 * QUAKED item_defense_ringofrepulsion (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Ring of Repulsion defensive spell.
*/

	p_itemlist[11].pickup=Pickup_Defense;
	p_itemlist[11].use=Use_Defence;
	p_itemlist[11].weaponthink=DefenceThink_RingOfRepulsion;

	// item_defense_shield
/*
 * QUAKED item_defense_shield (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Shield defensive spell.
*/

	p_itemlist[12].pickup=Pickup_Defense;
	p_itemlist[12].use=Use_Defence;
	p_itemlist[12].weaponthink=DefenceThink_Shield;

	// item_defense_teleport
/*
 * QUAKED item_defense_teleport (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Teleport defensive spell.
*/

	p_itemlist[13].pickup=Pickup_Defense;
	p_itemlist[13].use=Use_Defence;
	p_itemlist[13].weaponthink=DefenceThink_Teleport;

	// item_defense_polymorph
/*
 * QUAKED item_defense_polymorph (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Polymorph Barrier defensive spell.
*/

	p_itemlist[14].pickup=Pickup_Defense;
	p_itemlist[14].use=Use_Defence;
	p_itemlist[14].weaponthink=DefenceThink_Morph;

	// item_defense_meteorbarrier
/*
 * QUAKED item_defense_meteorbarrier (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Meteor Barrier defensive spell.
*/

	p_itemlist[15].pickup=Pickup_Defense;
	p_itemlist[15].use=Use_Defence;
	p_itemlist[15].weaponthink=DefenceThink_MeteorBarrier;

	// item_mana_offensive_half
/*
 * QUAKED item_mana_offensive_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the offensive mana (50 points).
*/
	p_itemlist[16].pickup=Pickup_Mana;

	// item_mana_offensive_full
/*
 * QUAKED item_mana_offensive_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the offensive mana (100 points).
*/

	p_itemlist[17].pickup=Pickup_Mana;

	// item_mana_defensive_half
/*
 * QUAKED item_mana_defensive_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the defensive mana (50 points).
*/

	p_itemlist[18].pickup=Pickup_Mana;

	// item_mana_defensive_full
/*
 * QUAKED item_mana_defensive_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the defensive mana (100 points).
*/

	p_itemlist[19].pickup=Pickup_Mana;

	// item_mana_combo_quarter
/*
 * QUAKED item_mana_combo_quarter (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for both defensive & offensive mana (25 points).
*/

	p_itemlist[20].pickup=Pickup_Mana;

	// item_mana_combo_half
/*
 * QUAKED item_mana_combo_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for both defensive & offensive mana (50 points).
*/

	p_itemlist[21].pickup=Pickup_Mana;

	// item_ammo_redrain
/*
 * QUAKED item_ammo_redrain (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup ammo for the Red Rain Bow
*/

	p_itemlist[22].pickup=Pickup_Ammo;

	// item_ammo_phoenix
/*
 * QUAKED item_ammo_phoenix (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup ammo for the Phoenix Bow
*/

	p_itemlist[23].pickup=Pickup_Ammo;

	// item_ammo_hellstaff
/*
 * QUAKED item_ammo_hellstaff (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup ammo for the Hellstaff
*/

	p_itemlist[24].pickup=Pickup_Ammo;

	// item_health_half
/*
 * QUAKED item_health_half (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup health (10 points)
*/

	p_itemlist[25].pickup=Pickup_Health;

	// item_health_full
/*
 * QUAKED item_health_full (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup health (30 points)
*/

	p_itemlist[26].pickup=Pickup_Health;

/*
 * QUAKED item_puzzle_townkey (.3 .3 1) (-8 -8 -4) (8 8 4)  x NO_DROP
Key puzzle piece
Town Level
NO_DROP - won't drop to ground

*/
	p_itemlist[27].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_cog (.3 .3 1) (-10 -10 -24) (10 10 20)  x  NO_DROP
Cog puzzle piece
Palace level
NO_DROP - won't drop to ground
*/
	p_itemlist[28].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_shield (.3 .3 1) (-2 -6 -12) (2 6 12)  x  NO_DROP
Sithra Shield puzzle item
Healer Level
NO_DROP - won't drop to ground
*/
	p_itemlist[29].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_potion (.3 .3 1) (-3 -3 -10) (3 3 10)  x  NO_DROP
Potion puzzle item
Healer Level
NO_DROP - won't drop to ground
*/
	p_itemlist[30].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_plazacontainer (.3 .3 1) (-6 -6 -8) (6 6 6)  x  NO_DROP
Container puzzle item
Plaza Level
NO_DROP - won't drop to ground
*/
	p_itemlist[31].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_slumcontainer (.3 .3 1) (-6 -6 -8) (6 6 6)  x  NO_DROP
Full Container puzzle item
Slum Level
NO_DROP - won't drop to ground
*/
	p_itemlist[32].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_crystal (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Crystal puzzle item
Academic Level
NO_DROP - won't drop to ground
*/
	p_itemlist[33].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_canyonkey (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Key puzzle item
Canyon Level
NO_DROP - won't drop to ground
*/
	p_itemlist[34].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_hive2amulet (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Amulet puzzle item
Hive 2 Level
NO_DROP - won't drop to ground
*/
	p_itemlist[35].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_hive2spear (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Spear puzzle item
Hive 2 Level
NO_DROP - won't drop to ground
*/
	p_itemlist[36].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_hive2gem (.3 .3 1) (-16 -16 -16) (16 16 16)  x  NO_DROP
Gem puzzle item
Hive 2 Level
NO_DROP - won't drop to ground
*/
	p_itemlist[37].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_minecartwheel (.3 .3 1) (-1 -6 -6) (1 6 6)  x  NO_DROP
Mine Cart Wheel puzzle item
Mine 1 Level
NO_DROP - won't drop to ground
*/
	p_itemlist[38].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_ore (.3 .3 1) (-10 -10 -8) (10 10 8)  x  NO_DROP
Unrefined Ore puzzle item
Mine 2 Level
NO_DROP - won't drop to ground
*/
	p_itemlist[39].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_refinedore (.3 .3 1) (-3 -12 -2) (3 12 2) x   NO_DROP
Refined Ore puzzle item
Mine 2 Level
NO_DROP - won't drop to ground
*/
	p_itemlist[40].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_dungeonkey (.3 .3 1) (-1 -18 -9) (1 18 9)  x  NO_DROP
Amulet puzzle item
Dungeon Level
NO_DROP - won't drop to ground
*/
	p_itemlist[41].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_cloudkey (.3 .3 1) (-8 -8 -3) (8 8 6)  x  NO_DROP
Key puzzle item
Cloud Quarters 2 Level
NO_DROP - won't drop to ground
*/
	p_itemlist[42].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_highpriestesskey (.3 .3 1) (-12 -12 -6) (12 12 6) x   NO_DROP
Key puzzle item
High Priestess Level
NO_DROP - won't drop to ground
*/
	p_itemlist[43].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_highpriestesssymbol (.3 .3 1) (-12 -12 -4) (12 12 4) x   NO_DROP
Key puzzle item
High Priestess Level
NO_DROP - won't drop to ground
*/
	p_itemlist[44].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_tome (.3 .3 1) (-12 -12 -4) (12 12 4)  x  NO_DROP
Tome puzzle piece
2 Cloud Levels
NO_DROP - won't drop to ground
*/
	p_itemlist[45].pickup = Pickup_Puzzle;

/*
 * QUAKED item_puzzle_tavernkey (.3 .3 1) (-8 -8 -4) (8 8 4)    x   NO_DROP
Key puzzle piece
Ssdocks Level
NO_DROP - won't drop to ground
*/
	p_itemlist[46].pickup = Pickup_Puzzle;

	// item_defense_tornado
/*
 * QUAKED item_defense_tornado (.3 .3 1) (-16 -16 -16) (16 16 16)   COOP_ONLY
Pickup for the Tornado defensive spell.
*/
	p_itemlist[47].pickup=Pickup_Defense;
	p_itemlist[47].use=Use_Defence;
	p_itemlist[47].weaponthink=DefenceThink_Tornado;

	// ********************************************************************************************
	// Initialise game variables.
	// ********************************************************************************************

	game.num_items = playerExport->GetPlayerItemsCount();
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
		it = playerExport->GetPlayerItems() + i;
		gi.configstring(CS_ITEMS + i, it->pickup_name);
	}
}
