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
 * Player weapons.
 *
 * =======================================================================
 */

#include "../header/local.h"
#include "../monster/misc/player.h"
#include <limits.h>
#include "../header/g_items.h"
#include "../header/g_skeletons.h"
#include "../monster/beast/beast.h"
#include "../player/library/p_main.h"
#include "../player/library/p_anims.h"
#include "../common/cl_strings.h"
#include "../common/matrix.h"
#include "../common/reference.h"
#include "../common/h2rand.h"

extern void SpellCastFlyingFist(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
extern void SpellCastMagicMissile(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir);
extern void SpellCastMagicMissileSpread(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,
										float NoOfMissiles,float Separation);
extern void SpellCastSphereOfAnnihilation(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,
										 float Value, float value);
extern void SpellCastMaceball(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
extern void SpellCastWall(edict_t *caster, vec3_t startpos, vec3_t aimangles, vec3_t AimDir, float Value);
extern void SpellCastRipper(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir);
extern void SpellCastBlast(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir);
extern void SpellCastRedRain(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
extern void SpellCastPhoenix(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value);
extern void SpellCastHellstaff(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir);

static void Weapon_CalcStartPos(vec3_t OriginToLowerJoint,vec3_t OriginToUpperJoint,
								vec3_t DefaultStartPos,vec3_t ActualStartPos, edict_t *caster);

#define PLAYER_NOISE_SELF 0
#define PLAYER_NOISE_IMPACT 1

#define FRAME_FIRE_FIRST (FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST (FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST (FRAME_IDLE_LAST + 1)

#define GRENADE_TIMER 3.0
#define GRENADE_MINSPEED 400
#define GRENADE_MAXSPEED 800

#define CHAINFIST_REACH 64

#define HEATBEAM_DM_DMG 15
#define HEATBEAM_SP_DMG 15

#define TRAP_TIMER 5.0
#define TRAP_MINSPEED 300
#define TRAP_MAXSPEED 700

static qboolean is_quad;
static qboolean is_quadfire;
static byte damage_multiplier;
static byte is_silenced;

void weapon_grenade_fire(edict_t *ent, qboolean held);
void weapon_trap_fire(edict_t *ent, qboolean held);

byte
P_DamageModifier(edict_t *ent)
{
	is_quad = 0;
	damage_multiplier = 1;

	if (!ent)
	{
		return 0;
	}

	if (ent->client->quad_framenum > level.framenum)
	{
		damage_multiplier *= 4;
		is_quad = 1;

		/* if we're quad and DF_NO_STACK_DOUBLE is on, return now. */
		if (((int)(dmflags->value) & DF_NO_STACK_DOUBLE))
		{
			return damage_multiplier;
		}
	}

	if (ent->client->double_framenum > level.framenum)
	{
		if ((deathmatch->value) || (damage_multiplier == 1))
		{
			damage_multiplier *= 2;
			is_quad = 1;
		}
	}

	if (ent->client->quadfire_framenum > level.framenum)
	{
		if ((deathmatch->value) || (damage_multiplier == 1))
		{
			damage_multiplier *= 2;
			is_quadfire = 1;
		}
	}

	return damage_multiplier;
}

void
P_ProjectSource(const edict_t *ent, const vec3_t distance,
		vec3_t forward, const vec3_t right, vec3_t result)
{
	gclient_t *client = ent->client;
	const float *point  = ent->s.origin;
	vec3_t     _distance;

	if (!client)
	{
		return;
	}

	VectorCopy(distance, _distance);

	if (client->pers.hand == LEFT_HANDED)
	{
		_distance[1] *= -1;
	}
	else if (client->pers.hand == CENTER_HANDED)
	{
		_distance[1] = 0;
	}

	G_ProjectSource(point, _distance, forward, right, result);

	// Berserker: fix - now the projectile hits exactly where the scope is pointing.
	if (aimfix->value)
	{
		vec3_t start, end;
		VectorSet(start, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] + ent->viewheight);
		VectorMA(start, 8192, forward, end);

		trace_t	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
		if (tr.fraction < 1)
		{
			VectorSubtract(tr.endpos, result, forward);
			VectorNormalize(forward);
		}
	}
}

static void
P_ProjectSource2(const edict_t *ent, const vec3_t point, const vec3_t distance,
		vec3_t forward, const vec3_t right, const vec3_t up, vec3_t result)
{
	gclient_t *client = ent->client;
	vec3_t     _distance;

	if (!client)
	{
		return;
	}

	VectorCopy(distance, _distance);

	if (client->pers.hand == LEFT_HANDED)
	{
		_distance[1] *= -1;
	}
	else if (client->pers.hand == CENTER_HANDED)
	{
		_distance[1] = 0;
	}

	G_ProjectSource2(point, _distance, forward, right, up, result);

	// Berserker: fix - now the projectile hits exactly where the scope is pointing.
	if (aimfix->value)
	{
		vec3_t start, end;
		VectorSet(start, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] + ent->viewheight);
		VectorMA(start, 8192, forward, end);

		trace_t	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
		if (tr.fraction < 1)
		{
			VectorSubtract(tr.endpos, result, forward);
			VectorNormalize(forward);
		}
	}
}

/*
 * Each player can have two noise objects associated with it:
 * a personal noise (jumping, pain, weapon firing), and a weapon
 * target noise (bullet wall impacts)
 *
 * Monsters that don't directly see the player can move
 * to a noise in hopes of seeing the player from there.
 */
static edict_t *
PlayerNoise_Spawn(edict_t *who, int type)
{
	edict_t *noise;

	if (!who)
	{
		return NULL;
	}

	noise = G_SpawnOptional();
	if (!noise)
	{
		return NULL;
	}

	noise->classname = "player_noise";
	noise->spawnflags = type;
	VectorSet(noise->mins, -8, -8, -8);
	VectorSet(noise->maxs, 8, 8, 8);
	noise->owner = who;
	noise->svflags = SVF_NOCLIENT;

	return noise;
}

static void
PlayerNoise_Verify(edict_t *who)
{
	edict_t *e;
	edict_t *n1;
	edict_t *n2;

	if (!who)
	{
		return;
	}

	n1 = who->mynoise;
	n2 = who->mynoise2;

	if (n1 && !n1->inuse)
	{
		n1 = NULL;
	}

	if (n2 && !n2->inuse)
	{
		n2 = NULL;
	}

	if (n1 && n2)
	{
		return;
	}

	for (e = g_edicts + 1 + game.maxclients; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (!e->inuse || strcmp(e->classname, "player_noise") != 0)
		{
			continue;
		}

		if (e->owner && e->owner != who)
		{
			continue;
		}

		e->owner = who;

		if (!n2 && (e->spawnflags == PLAYER_NOISE_IMPACT || n1))
		{
			n2 = e;
		}
		else
		{
			n1 = e;
		}

		if (n1 && n2)
		{
			break;
		}
	}

	if (!n1)
	{
		n1 = PlayerNoise_Spawn(who, PLAYER_NOISE_SELF);
	}

	if (!n2)
	{
		n2 = PlayerNoise_Spawn(who, PLAYER_NOISE_IMPACT);
	}

	who->mynoise = n1;
	who->mynoise2 = n2;
}

void
PlayerNoise(edict_t *who, vec3_t where, int type)
{
	edict_t *noise;

	if (!who || !who->client)
	{
		return;
	}

	if (type == PNOISE_WEAPON)
	{
		if (who->client->silencer_shots)
		{
			who->client->silencer_shots--;
			return;
		}
	}

	if (deathmatch->value)
	{
		return;
	}

	if (who->flags & FL_NOTARGET)
	{
		return;
	}

	if (who->flags & FL_DISGUISED)
	{
		if (type == PNOISE_WEAPON)
		{
			level.disguise_violator = who;
			level.disguise_violation_framenum = level.framenum + 5;
		}
		else
		{
			return;
		}
	}

	PlayerNoise_Verify(who);

	if ((type == PNOISE_SELF) || (type == PNOISE_WEAPON))
	{
		if (level.framenum <= (level.sound_entity_framenum + 3))
		{
			return;
		}

		if (!who->mynoise)
		{
			return;
		}

		noise = who->mynoise;
		level.sound_entity = noise;
		level.sound_entity_framenum = level.framenum;
	}
	else
	{
		if (level.framenum <= (level.sound2_entity_framenum + 3))
		{
			return;
		}

		if (!who->mynoise2)
		{
			return;
		}

		noise = who->mynoise2;
		level.sound2_entity = noise;
		level.sound2_entity_framenum = level.framenum;
	}

	VectorCopy(where, noise->s.origin);
	VectorSubtract(where, noise->maxs, noise->absmin);
	VectorAdd(where, noise->maxs, noise->absmax);
	noise->last_sound_time = level.time;
	gi.linkentity(noise);
}

qboolean
AddWeaponToInventory(gitem_t *item, edict_t *player)
{
	gitem_t	*newitem;
	int		count;
	int index;

	index = ITEM_INDEX(item);

	// Do we already have this weapon?

	if (!player->client->pers.inventory[index])
	{
		// We don't already have it, so get the weapon and some ammo.

		if (item->tag == ITEM_WEAPON_SWORDSTAFF)
			count= 0;
		else if (item->tag == ITEM_WEAPON_HELLSTAFF)
			count = AMMO_COUNT_HELLSTAFF;
		else if (item->tag == ITEM_WEAPON_REDRAINBOW)
		{
			// give us the bowtype
			player->client->pers.bowtype = BOW_TYPE_REDRAIN;
			count = AMMO_COUNT_REDRAINBOW;
		}
		else if (item->tag == ITEM_WEAPON_PHOENIXBOW)
		{
			// give us the bowtype
			player->client->pers.bowtype = BOW_TYPE_PHOENIX;
			count = AMMO_COUNT_PHOENIXBOW;
		}
		else
			count = AMMO_COUNT_MOST;

		player->client->pers.inventory[index] = 1;

		if (count)
		{
			newitem = FindItem(item->ammo);
			Add_Ammo(player, newitem,count);
		}

		// Now decide if we want to swap weapons or not.

		if (player->client->pers.autoweapon)
		{
			// If this new weapon is a higher value than the one we currently have, swap the current
			// weapon for the new one.

			if (ITEM_INDEX(item) > ITEM_INDEX(player->client->playerinfo.pers.weapon))
			{
				item->use(player, item);
			}
		}

		return true;
	}
	else
	{
		// We already have it...

		if (!((deathmatch->value&&((int)dmflags->value&DF_WEAPONS_STAY))||coop->value))
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

			if (Add_Ammo(player, newitem,count))
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

qboolean
Pickup_Weapon(edict_t *ent, edict_t *other)
{
	if (other->flags & FL_CHICKEN)
	{
		return false;
	}

	if (AddWeaponToInventory(ent->item,other))
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

/*
 * The old weapon has been dropped all
 * the way, so make the new one current
 */
void
ChangeWeapon(edict_t *ent)
{
	int i;

	if (!ent)
	{
		return;
	}

	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire(ent, false);
		ent->client->grenade_time = 0;
	}

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	/* set visible model */
	if (ent->s.modelindex == CUSTOM_PLAYER_MODEL)
	{
		if (ent->client->pers.weapon)
		{
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		}
		else
		{
			i = 0;
		}

		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
	{
		ent->client->ammo_index =
			ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	}
	else
	{
		ent->client->ammo_index = 0;
	}

	if (!ent->client->pers.weapon)
	{
		/* dead */
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
	/* Don't display weapon if chasetoggle is on */
	if (ent->client->chasetoggle)
	{
		ent->client->ps.gunindex = 0;
	}
	else
	{
		ent->client->ps.gunindex = gi.modelindex(
				ent->client->pers.weapon->view_model);
	}

	ent->client->anim_priority = ANIM_PAIN;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		P_SetAnimGroup(ent, "crpain", FRAME_crpain1, FRAME_crpain4, 0);
	}
	else
	{
		P_SetAnimGroup(ent, "pain", FRAME_pain301, FRAME_pain304, 3);
	}
}

void
NoAmmoWeaponChange(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] &&
		ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))])
	{
		ent->client->newweapon = FindItem("railgun");
		return;
	}

	if ((ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] >= 2) &&
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Plasma Beam"))])
	{
		ent->client->newweapon = FindItem("Plasma Beam");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("flechettes"))] &&
		ent->client->pers.inventory[ITEM_INDEX(FindItem("etf rifle"))])
	{
		ent->client->newweapon = FindItem("etf rifle");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] > 1 &&
		ent->client->pers.inventory[ITEM_INDEX(FindItem("ionripper"))])
	{
		ent->client->newweapon = FindItem("ionripper");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] &&
		ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))])
	{
		ent->client->newweapon = FindItem("hyperblaster");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] &&
		ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))])
	{
		ent->client->newweapon = FindItem("chaingun");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] &&
		ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))])
	{
		ent->client->newweapon = FindItem("machinegun");
		return;
	}

	if ((ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] > 1) &&
		ent->client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))])
	{
		ent->client->newweapon = FindItem("super shotgun");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] &&
		ent->client->pers.inventory[ITEM_INDEX(FindItem("shotgun"))])
	{
		ent->client->newweapon = FindItem("shotgun");
		return;
	}

	ent->client->newweapon = FindItem("blaster");
}

/*
 * Called by ClientBeginServerFrame and ClientThink
 */
void
Think_Weapon(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	/* if just died, put the weapon away */
	if (ent->health < 1)
	{
		ent->client->newweapon = NULL;
		ChangeWeapon(ent);
	}

	/* call active weapon think routine */
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		P_DamageModifier(ent);

		if (ent->client->silencer_shots)
		{
			is_silenced = MZ_SILENCED;
		}
		else
		{
			is_silenced = 0;
		}

		ent->client->pers.weapon->weaponthink(ent);
	}
}

/*
 * Make the weapon ready if there is ammo
 */
void
Use_Weapon(edict_t *ent, gitem_t *item)
{
	int ammo_index;
	gitem_t *ammo_item;

	if (!ent || !item)
	{
		return;
	}

	/* see if we're already using it */
	if (item == ent->client->pers.weapon)
	{
		return;
	}

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
			gi.cprintf(ent, PRINT_HIGH, "No %s for %s.\n",
					ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			gi.cprintf(ent, PRINT_HIGH, "Not enough %s for %s.\n",
					ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	/* change to this weapon when down */
	ent->client->newweapon = item;
}

void
Use_Weapon2(edict_t *ent, gitem_t *item)
{
	int ammo_index;
	gitem_t *ammo_item;
	gitem_t *nextitem;
	int index;

	if (!ent || !item)
	{
		return;
	}

	if (strcmp(item->pickup_name, "HyperBlaster") == 0)
	{
		if (item == ent->client->pers.weapon)
		{
			item = FindItem("Ionripper");
			index = ITEM_INDEX(item);

			if (!ent->client->pers.inventory[index])
			{
				item = FindItem("HyperBlaster");
			}
		}
	}

	else if (strcmp(item->pickup_name, "Railgun") == 0)
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
			nextitem = FindItem("Phalanx");
			ammo_item = FindItem(nextitem->ammo);
			ammo_index = ITEM_INDEX(ammo_item);

			if (ent->client->pers.inventory[ammo_index])
			{
				item = FindItem("Phalanx");
				index = ITEM_INDEX(item);

				if (!ent->client->pers.inventory[index])
				{
					item = FindItem("Railgun");
				}
			}
		}
		else if (item == ent->client->pers.weapon)
		{
			item = FindItem("Phalanx");
			index = ITEM_INDEX(item);

			if (!ent->client->pers.inventory[index])
			{
				item = FindItem("Railgun");
			}
		}
	}

	/* see if we're already using it */
	if (item == ent->client->pers.weapon)
	{
		return;
	}

	if (item->ammo)
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index] && !g_select_empty->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "No %s for %s.\n",
					ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	/* change to this weapon when down */
	ent->client->newweapon = item;
}

void
Drop_Weapon(edict_t *ent, gitem_t *item)
{
	int index;

	if (!ent || !item)
	{
		return;
	}

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
	{
		return;
	}

	index = ITEM_INDEX(item);

	/* see if we're already using it */
	if (((item == ent->client->pers.weapon) ||
		 (item == ent->client->newweapon)) &&
		(ent->client->pers.inventory[index] == 1))
	{
		gi.cprintf(ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	Drop_Item(ent, item);
	ent->client->pers.inventory[index]--;
}

/*
 * Client (player) animation for changing weapon
 */
static void
Change_Weap_Animation(edict_t *ent)
{
	int firstframe, lastframe, select;
	const char *animname;

	if (!ent)
	{
		return;
	}

	ent->client->anim_priority = ANIM_REVERSE;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		lastframe = FRAME_crpain4;
		firstframe = FRAME_crpain1;
		animname = "crpain";
		select = 0;
	}
	else
	{
		lastframe = FRAME_pain304;
		firstframe = FRAME_pain301;
		animname = "pain";
		select = 3;
	}

	lastframe -= firstframe - 1;
	M_SetAnimGroupFrameValues(ent, animname, &firstframe, &lastframe, select);
	lastframe += firstframe - 1;

	ent->s.frame = lastframe + 1;
	ent->client->anim_end = firstframe;
}

static void
PlayerApplyAttack(edict_t *ent)
{
	/* start the animation */
	ent->client->anim_priority = ANIM_ATTACK;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		P_SetAnimGroup(ent, "crattak", FRAME_crattak1, FRAME_crattak9, 0);
	}
	else
	{
		P_SetAnimGroup(ent, "attack", FRAME_attack1, FRAME_attack8, 0);
	}
	ent->s.frame --;
}

/*
 * A generic function to handle
 * the basics of weapon thinking
 */
static void
Weapon_Generic2(edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST,
		int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames,
		int *fire_frames, void (*fire)(edict_t *ent))
{
	int n;
	const unsigned short int change_speed = (g_swap_speed->value > 1)?
		(g_swap_speed->value < USHRT_MAX)? (unsigned short int)g_swap_speed->value : 1
		: 1;

	if (!ent || !fire_frames || !fire)
	{
		return;
	}

	if (ent->deadflag || (ent->s.modelindex != CUSTOM_PLAYER_MODEL)) /* VWep animations screw up corpses */
	{
		return;
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe >= FRAME_DEACTIVATE_LAST - change_speed + 1)
		{
			ChangeWeapon(ent);
			return;
		}
		else if ( (FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) >= (4 * change_speed) )
		{
			unsigned short int remainder = FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe;
			// "if (remainder == 4)" at change_speed == 1
			if ( ( remainder <= (4 * change_speed) )
				&& ( remainder > (3 * change_speed) ) )
			{
				Change_Weap_Animation(ent);
			}
		}

		ent->client->ps.gunframe += change_speed;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe >= FRAME_ACTIVATE_LAST - change_speed + 1)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe += change_speed;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		if ( (FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < (4 * change_speed) )
		{
			Change_Weap_Animation(ent);
		}

		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if (((ent->client->latched_buttons |
			  ent->client->buttons) & BUTTON_ATTACK))
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;

			if ((!ent->client->ammo_index) ||
				(ent->client->pers.inventory[ent->client->ammo_index] >=
				 ent->client->pers.weapon->quantity))
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				PlayerApplyAttack(ent);
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex(
								"weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}

				NoAmmoWeaponChange(ent);
			}
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (randk() & 15)
						{
							return;
						}
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (!CTFApplyStrengthSound(ent))
				{
					if (ent->client->quad_framenum > level.framenum)
					{
						gi.sound(ent, CHAN_ITEM, gi.soundindex(
									"items/damage3.wav"), 1, ATTN_NORM, 0);
					}
					else if (ent->client->double_framenum > level.framenum)
					{
						gi.sound(ent, CHAN_ITEM, gi.soundindex(
									"misc/ddamage3.wav"), 1, ATTN_NORM, 0);
					}
				}

				CTFApplyHasteSound(ent);

				fire(ent);
				break;
			}
		}

		if (!fire_frames[n])
		{
			ent->client->ps.gunframe++;
		}

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST + 1)
		{
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

void
Weapon_Generic(edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST,
		int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames,
		int *fire_frames, void (*fire)(edict_t *ent))
{
	int oldstate = ent->client->weaponstate;

	Weapon_Generic2(ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST,
			FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames,
			fire_frames, fire);

	/* run the weapon frame again if hasted */
	if ((Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0) &&
		(ent->client->weaponstate == WEAPON_FIRING))
	{
		return;
	}

	if ((CTFApplyHaste(ent) ||
		 ((Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0) &&
		  (ent->client->weaponstate != WEAPON_FIRING))) &&
		(oldstate == ent->client->weaponstate))
	{
		Weapon_Generic2(ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST,
				FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames,
				fire_frames, fire);
	}
}

/*
 * ======================================================================
 *
 * GRENADE
 *
 * ======================================================================
 */

void
weapon_grenade_fire(edict_t *ent, qboolean held)
{
	vec3_t offset;
	vec3_t forward, right, up;
	vec3_t start;
	int damage = 125;
	float timer;
	int speed;
	float radius;

	if (!ent)
	{
		return;
	}

	radius = damage + 40;

	if (is_quad)
	{
		damage *= damage_multiplier;

		if (damage_multiplier >= 4)
		{
			gi.sound(ent, CHAN_ITEM, gi.soundindex(
					"items/damage3.wav"), 1, ATTN_NORM, 0);
		}
		else if (damage_multiplier == 2)
		{
			gi.sound(ent, CHAN_ITEM, gi.soundindex(
					"misc/ddamage3.wav"), 1, ATTN_NORM, 0);
		}
	}

	if ((ent->client->use) && (ent->client->oldplayer))
	{
		AngleVectors(ent->client->oldplayer->s.angles, forward, right, up);
	}
	else
	{
		AngleVectors(ent->client->v_angle, forward, right, up);
	}

	if (ent->client->pers.weapon->tag == AMMO_TESLA)
	{
		VectorSet(offset, 0, -4, ent->viewheight - 22);
	}
	else
	{
		VectorSet(offset, 2, 6, ent->viewheight - 14);
	}

	P_ProjectSource2(ent, ent->s.origin, offset,
			forward, right, up, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) *
		((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);

	if (speed > GRENADE_MAXSPEED)
	{
		speed = GRENADE_MAXSPEED;
	}

	switch (ent->client->pers.weapon->tag)
	{
		case AMMO_GRENADES:
			fire_grenade2(ent, start, forward, damage, speed,
				timer, radius, held);
			break;
		default:
			fire_tesla(ent, start, forward, damage_multiplier, speed);
			break;
	}

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
	}

	ent->client->grenade_time = level.time + 1.0;

	if (ent->deadflag || (ent->s.modelindex != CUSTOM_PLAYER_MODEL)) /* VWep animations screw up corpses */
	{
		return;
	}

	if (ent->health <= 0)
	{
		return;
	}

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		int firstframe, lastframe;

		firstframe = FRAME_crattak1;
		lastframe = FRAME_crattak9;

		lastframe -= firstframe;
		M_SetAnimGroupFrameValues(ent, "crattak", &firstframe, &lastframe, 0);
		lastframe += firstframe;

		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = firstframe - 1;
		ent->client->anim_end = Q_min(firstframe + 2, lastframe);
	}
	else
	{
		int firstframe, lastframe;

		firstframe = FRAME_wave01;
		lastframe = FRAME_wave11;

		lastframe -= firstframe;
		M_SetAnimGroupFrameValues(ent, "wave", &firstframe, &lastframe, 0);
		lastframe += firstframe;

		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = Q_min(firstframe + 7, lastframe);
		ent->client->anim_end = firstframe;
	}
}

void
Throw_Generic(edict_t *ent, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_THROW_SOUND,
		int FRAME_THROW_HOLD, int FRAME_THROW_FIRE, int *pause_frames, int EXPLODE,
		void (*fire)(edict_t *ent, qboolean held))
{
	int n;

	if (!ent || !pause_frames || !fire)
	{
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon(ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = FRAME_IDLE_FIRST;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if (((ent->client->latched_buttons |
			  ent->client->buttons) & BUTTON_ATTACK))
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;

			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex(
								"weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}

				NoAmmoWeaponChange(ent);
			}

			return;
		}

		if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
		{
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		if (pause_frames)
		{
			for (n = 0; pause_frames[n]; n++)
			{
				if (ent->client->ps.gunframe == pause_frames[n])
				{
					if (randk() & 15)
					{
						return;
					}
				}
			}
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == FRAME_THROW_SOUND)
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex(
							"weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);
		}

		if (ent->client->ps.gunframe == FRAME_THROW_HOLD)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;

				switch (ent->client->pers.weapon->tag)
				{
					case AMMO_GRENADES:
						ent->client->weapon_sound = gi.soundindex(
							"weapons/hgrenc1b.wav");
						break;
				}
			}

			/* they waited too long, detonate it in their hand */
			if (EXPLODE && !ent->client->grenade_blew_up &&
				(level.time >= ent->client->grenade_time))
			{
				ent->client->weapon_sound = 0;
				fire(ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
			{
				return;
			}

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = FRAME_FIRE_LAST;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == FRAME_THROW_FIRE)
		{
			ent->client->weapon_sound = 0;
			fire(ent, true);
		}

		if ((ent->client->ps.gunframe == FRAME_FIRE_LAST) &&
			(level.time < ent->client->grenade_time))
		{
			return;
		}

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

void
Weapon_Grenade(edict_t *ent)
{
	static int pause_frames[] = {29, 34, 39, 48, 0};

	if (!ent)
	{
		return;
	}

	Throw_Generic(ent, 15, 48, 5, 11, 12, pause_frames,
			GRENADE_TIMER, weapon_grenade_fire);
}

void
Weapon_Tesla(edict_t *ent)
{
	static int pause_frames[] = {21, 0};

	if (!ent)
	{
		return;
	}

	if ((ent->client->ps.gunframe > 1) && (ent->client->ps.gunframe < 9))
	{
		ent->client->ps.gunindex = gi.modelindex("models/weapons/v_tesla2/tris.md2");
	}
	else
	{
		ent->client->ps.gunindex = gi.modelindex("models/weapons/v_tesla/tris.md2");
	}

	Throw_Generic(ent, 8, 32, 99, 1, 2, pause_frames, 0, weapon_grenade_fire);
}

/*
 * ======================================================================
 *
 * GRENADE LAUNCHER
 *
 * ======================================================================
 */

void
weapon_grenadelauncher_fire(edict_t *ent)
{
	vec3_t offset;
	vec3_t forward, right;
	vec3_t start;
	int damage;
	float radius;

	if (!ent)
	{
		return;
	}

	switch (ent->client->pers.weapon->tag)
	{
		case AMMO_PROX:
			damage = 90;
			break;
		default:
			damage = 120;
			break;
	}

	radius = damage + 40;

	if (is_quad)
	{
		damage *= damage_multiplier;
	}

	VectorSet(offset, 8, 8, ent->viewheight - 8);

	if ((ent->client->use) && (ent->client->oldplayer))
	{
		AngleVectors(ent->client->oldplayer->s.angles, forward, right, NULL);
	}
	else
	{
		AngleVectors(ent->client->v_angle, forward, right, NULL);
	}

	P_ProjectSource(ent, offset, forward, right, start);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	switch (ent->client->pers.weapon->tag)
	{
		case AMMO_PROX:
			fire_prox(ent, start, forward, damage_multiplier, 600);
			break;
		default:
			fire_grenade(ent, start, forward, damage, 600, 2.5, radius);
			break;
	}

	gi.WriteByte(svc_muzzleflash);

	if (ent->client->oldplayer)
	{
		gi.WriteShort(ent->client->oldplayer - g_edicts);
	}
	else
	{
		gi.WriteShort(ent - g_edicts);
	}

	if (ent->client->pers.weapon->tag == AMMO_PROX)
	{
		gi.WriteByte(MZ_PROX | is_silenced);
	}
	else
	{
		gi.WriteByte(MZ_GRENADE | is_silenced);
	}

	if (ent->client->oldplayer)
	{
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
	}
}

void
Weapon_GrenadeLauncher(edict_t *ent)
{
	static int pause_frames[] = {34, 51, 59, 0};
	static int fire_frames[] = {6, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 5, 16, 59, 64, pause_frames,
			fire_frames, weapon_grenadelauncher_fire);

	if (is_quadfire)
	{
		Weapon_Generic(ent, 5, 16, 59, 64, pause_frames,
				fire_frames, weapon_grenadelauncher_fire);
	}
}

void
Weapon_ProxLauncher(edict_t *ent)
{
	static int pause_frames[] = {34, 51, 59, 0};
	static int fire_frames[] = {6, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 5, 16, 59, 64, pause_frames,
			fire_frames, weapon_grenadelauncher_fire);
}

/*
 * ======================================================================
 *
 * ROCKET
 *
 * ======================================================================
 */

void
Weapon_RocketLauncher_Fire(edict_t *ent)
{
	vec3_t offset, start;
	vec3_t forward, right;
	int damage;
	float damage_radius;
	int radius_damage;

	if (!ent)
	{
		return;
	}

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;

	if (is_quad)
	{
		damage *= damage_multiplier;
		radius_damage *= damage_multiplier;
	}

	if ((ent->client->use) && (ent->client->oldplayer))
	{
		AngleVectors(ent->client->oldplayer->s.angles, forward, right, NULL);
	}
	else
	{
		AngleVectors(ent->client->v_angle, forward, right, NULL);
	}

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight - 8);
	P_ProjectSource(ent, offset, forward, right, start);
	fire_rocket(ent, start, forward, damage, 650, damage_radius, radius_damage);

	/* send muzzle flash */
	gi.WriteByte(svc_muzzleflash);

	if (ent->client->oldplayer)
	{
		gi.WriteShort(ent->client->oldplayer - g_edicts);
	}
	else
	{
		gi.WriteShort(ent - g_edicts);
	}

	gi.WriteByte(MZ_ROCKET | is_silenced);

	if (ent->client->oldplayer)
	{
		gi.multicast(ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
	}
}

void
Weapon_RocketLauncher(edict_t *ent)
{
	static int pause_frames[] = {25, 33, 42, 50, 0};
	static int fire_frames[] = {5, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 4, 12, 50, 54, pause_frames,
			fire_frames, Weapon_RocketLauncher_Fire);

	if (is_quadfire)
	{
		Weapon_Generic(ent, 4, 12, 50, 54, pause_frames,
				fire_frames, Weapon_RocketLauncher_Fire);
	}
}

/*
 * ======================================================================
 *
 * BLASTER / HYPERBLASTER
 *
 * ======================================================================
 */

void
Blaster_Fire(edict_t *ent, vec3_t g_offset, int damage,
		qboolean hyper, int effect)
{
	vec3_t forward, right;
	vec3_t start;
	vec3_t offset;

	if (!ent)
	{
		return;
	}

	if (is_quad)
	{
		damage *= damage_multiplier;
	}

	if ((ent->client->use) && (ent->client->oldplayer))
	{
		AngleVectors(ent->client->oldplayer->s.angles, forward, right, NULL);
	}
	else
	{
		AngleVectors(ent->client->v_angle, forward, right, NULL);
	}

	VectorSet(offset, 24, 8, ent->viewheight - 8);
	VectorAdd(offset, g_offset, offset);
	P_ProjectSource(ent, offset, forward, right, start);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster(ent, start, forward, damage, 1000, effect, hyper);

	/* send muzzle flash */
	gi.WriteByte(svc_muzzleflash);

	if (ent->client->oldplayer)
	{
		gi.WriteShort(ent->client->oldplayer - g_edicts);
	}
	else
	{
		gi.WriteShort(ent - g_edicts);
	}

	if (hyper)
	{
		gi.WriteByte(MZ_HYPERBLASTER | is_silenced);
	}
	else
	{
		gi.WriteByte(MZ_BLASTER | is_silenced);
	}

	if (ent->client->oldplayer)
	{
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void
Weapon_Blaster_Fire(edict_t *ent)
{
	int damage;

	if (!ent)
	{
		return;
	}

	if (deathmatch->value)
	{
		damage = 15;
	}
	else
	{
		damage = 10;
	}

	Blaster_Fire(ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void
Weapon_Blaster(edict_t *ent)
{
	static int pause_frames[] = {19, 32, 0};
	static int fire_frames[] = {5, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 4, 8, 52, 55, pause_frames,
			fire_frames, Weapon_Blaster_Fire);

	if (is_quadfire)
	{
		Weapon_Generic(ent, 4, 8, 52, 55, pause_frames,
				fire_frames, Weapon_Blaster_Fire);
	}
}

void
Weapon_HyperBlaster_Fire(edict_t *ent)
{
	float rotation;
	vec3_t offset;
	int effect;
	int damage;

	if (!ent)
	{
		return;
	}

	ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (!ent->client->pers.inventory[ent->client->ammo_index])
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex(
							"weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}

			NoAmmoWeaponChange(ent);
		}
		else
		{
			rotation = (ent->client->ps.gunframe - 5) * 2 * M_PI / 6;
			offset[0] = -4 * sin(rotation);
			offset[1] = 0;
			offset[2] = 4 * cos(rotation);

			if ((ent->client->ps.gunframe == 6) ||
				(ent->client->ps.gunframe == 9))
			{
				effect = EF_HYPERBLASTER;
			}
			else
			{
				effect = 0;
			}

			if (deathmatch->value)
			{
				damage = 15;
			}
			else
			{
				damage = 20;
			}

			Blaster_Fire(ent, offset, damage, true, effect);

			if (!((int)dmflags->value & DF_INFINITE_AMMO))
			{
				ent->client->pers.inventory[ent->client->ammo_index]--;
			}

			PlayerApplyAttack(ent);
		}

		ent->client->ps.gunframe++;

		if ((ent->client->ps.gunframe == 12) &&
			ent->client->pers.inventory[ent->client->ammo_index])
		{
			ent->client->ps.gunframe = 6;
		}
	}

	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex(
						"weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}
}

void
Weapon_HyperBlaster(edict_t *ent)
{
	static int pause_frames[] = {0};
	static int fire_frames[] = {6, 7, 8, 9, 10, 11, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 5, 20, 49, 53, pause_frames,
			fire_frames, Weapon_HyperBlaster_Fire);

	if (is_quadfire)
	{
		Weapon_Generic(ent, 5, 20, 49, 53, pause_frames,
				fire_frames, Weapon_HyperBlaster_Fire);
	}
}

/*
 * ======================================================================
 *
 * MACHINEGUN / CHAINGUN
 *
 * ======================================================================
 */

void
Machinegun_Fire(edict_t *ent)
{
	int i;
	vec3_t start;
	vec3_t forward, right;
	vec3_t angles;
	int damage = 8;
	int kick = 2;
	vec3_t offset;

	if (!ent)
	{
		return;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 5)
	{
		ent->client->ps.gunframe = 4;
	}
	else
	{
		ent->client->ps.gunframe = 5;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;

		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex(
						"weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}

		NoAmmoWeaponChange(ent);
		return;
	}

	if (is_quad)
	{
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	for (i = 1; i < 3; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	/* raise the gun as it is firing */
	if (!(deathmatch->value || g_machinegun_norecoil->value))
	{
		ent->client->machinegun_shots++;

		if (ent->client->machinegun_shots > 9)
		{
			ent->client->machinegun_shots = 9;
		}
	}

	/* get start / end positions */
	if ((ent->client->use) && (ent->client->oldplayer))
	{
		VectorAdd (ent->client->oldplayer->s.angles, ent->client->kick_angles, angles);
	}
	else
	{
		VectorAdd(ent->client->v_angle, ent->client->kick_angles, angles);
	}

	AngleVectors(angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight - 8);
	P_ProjectSource(ent, offset, forward, right, start);
	fire_bullet(ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD,
			DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

	gi.WriteByte(svc_muzzleflash);

	if (ent->client->oldplayer)
	{
		gi.WriteShort(ent->client->oldplayer - g_edicts);
	}
	else
	{
		gi.WriteShort(ent - g_edicts);
	}

	gi.WriteByte(MZ_MACHINEGUN | is_silenced);

	if (ent->client->oldplayer)
	{
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
	}

	PlayerApplyAttack(ent);

	ent->s.frame += 1 - (int)(random() + 0.25);
}

void
Weapon_Machinegun(edict_t *ent)
{
	static int pause_frames[] = {23, 45, 0};
	static int fire_frames[] = {4, 5, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 3, 5, 45, 49, pause_frames,
			fire_frames, Machinegun_Fire);

	if (is_quadfire)
	{
		Weapon_Generic(ent, 3, 5, 45, 49, pause_frames,
				fire_frames, Machinegun_Fire);
	}
}

void
Chaingun_Fire(edict_t *ent)
{
	int i;
	int shots;
	vec3_t start;
	vec3_t forward, right, up;
	float r, u;
	vec3_t offset;
	int damage;
	int kick = 2;

	if (!ent)
	{
		return;
	}

	if (deathmatch->value)
	{
		damage = 6;
	}
	else
	{
		damage = 8;
	}

	if (ent->client->ps.gunframe == 5)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex(
					"weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);
	}

	if ((ent->client->ps.gunframe == 14) &&
		!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) &&
			 (ent->client->buttons & BUTTON_ATTACK) &&
			 ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex(
					"weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	PlayerApplyAttack(ent);
	ent->s.frame += 1 - (ent->client->ps.gunframe & 1);

	if (ent->client->ps.gunframe <= 9)
	{
		shots = 1;
	}
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTON_ATTACK)
		{
			shots = 2;
		}
		else
		{
			shots = 1;
		}
	}
	else
	{
		shots = 3;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
	{
		shots = ent->client->pers.inventory[ent->client->ammo_index];
	}

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex(
							"weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}

		NoAmmoWeaponChange(ent);
		return;
	}

	if (is_quad)
	{
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	for (i = 0; i < 3; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i = 0; i < shots; i++)
	{
		/* get start / end positions */
		if ((ent->client->use) && (ent->client->oldplayer))
		{
			AngleVectors(ent->client->oldplayer->s.angles, forward, right, NULL);
		}
		else
		{
			AngleVectors(ent->client->v_angle, forward, right, up);
		}

		r = 7 + crandom() * 4;
		u = crandom() * 4;
		VectorSet(offset, 0, r, u + ent->viewheight - 8);
		P_ProjectSource(ent, offset,
				forward, right, start);

		fire_bullet(ent, start, forward, damage, kick,
				DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD,
				MOD_CHAINGUN);
	}

	/* send muzzle flash */
	gi.WriteByte(svc_muzzleflash);

	if (ent->client->oldplayer)
	{
		gi.WriteShort(ent->client->oldplayer - g_edicts);
	}
	else
	{
		gi.WriteShort(ent - g_edicts);
	}

	gi.WriteByte((MZ_CHAINGUN1 + shots - 1) | is_silenced);

	if (ent->client->oldplayer)
	{
		gi.multicast(ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
	}
}

void
Weapon_Chaingun(edict_t *ent)
{
	static int pause_frames[] = {38, 43, 51, 61, 0};
	static int fire_frames[] = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);

	if (is_quadfire)
	{
		Weapon_Generic(ent, 4, 31, 61, 64, pause_frames,
				fire_frames, Chaingun_Fire);
	}
}

/*
 * ======================================================================
 *
 * SHOTGUN / SUPERSHOTGUN
 *
 * ======================================================================
 */

void
weapon_shotgun_fire(edict_t *ent)
{
	vec3_t start;
	vec3_t forward, right;
	vec3_t offset;
	int damage = 4;
	int kick = 8;

	if (!ent)
	{
		return;
	}

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->use) && (ent->client->oldplayer))
	{
		AngleVectors(ent->client->oldplayer->s.angles, forward, right, NULL);
	}
	else
	{
		AngleVectors(ent->client->v_angle, forward, right, NULL);
	}

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8, ent->viewheight - 8);
	P_ProjectSource(ent, offset, forward, right, start);

	if (is_quad)
	{
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	if (deathmatch->value)
	{
		fire_shotgun(ent, start, forward, damage, kick,
				500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT,
				MOD_SHOTGUN);
	}
	else
	{
		fire_shotgun(ent, start, forward, damage, kick,
				500, 500, DEFAULT_SHOTGUN_COUNT,
				MOD_SHOTGUN);
	}

	/* send muzzle flash */
	gi.WriteByte(svc_muzzleflash);

	if (ent->client->oldplayer)
	{
		gi.WriteShort(ent->client->oldplayer - g_edicts);
	}
	else
	{
		gi.WriteShort(ent - g_edicts);
	}

	gi.WriteByte(MZ_SHOTGUN | is_silenced);

	if (ent->client->oldplayer)
	{
		gi.multicast(ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
	}
}

void
Weapon_Shotgun(edict_t *ent)
{
	static int pause_frames[] = {22, 28, 34, 0};
	static int fire_frames[] = {8, 9, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 7, 18, 36, 39, pause_frames,
			fire_frames, weapon_shotgun_fire);

	if (is_quadfire)
	{
		Weapon_Generic(ent, 7, 18, 36, 39, pause_frames,
				fire_frames, weapon_shotgun_fire);
	}
}

void
weapon_supershotgun_fire(edict_t *ent)
{
	vec3_t start;
	vec3_t forward, right;
	vec3_t offset;
	vec3_t v;
	int damage = 6;
	int kick = 12;

	if (!ent)
	{
		return;
	}

	if ((ent->client->use) && (ent->client->oldplayer))
	{
		AngleVectors(ent->client->oldplayer->s.angles, forward, right, NULL);
	}
	else
	{
		AngleVectors(ent->client->v_angle, forward, right, NULL);
	}

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8, ent->viewheight - 8);
	P_ProjectSource(ent, offset, forward, right, start);

	if (is_quad)
	{
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	if ((ent->client->use) && (ent->client->oldplayer))
	{
		v[PITCH] = ent->client->oldplayer->s.angles[PITCH];
		v[YAW] = ent->client->oldplayer->s.angles[YAW] - 5;
		v[ROLL] = ent->client->oldplayer->s.angles[ROLL];
	}
	else
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW] = ent->client->v_angle[YAW] - 5;
		v[ROLL] = ent->client->v_angle[ROLL];
	}

	AngleVectors(v, forward, NULL, NULL);

	if (aimfix->value)
	{
		AngleVectors(v, forward, right, NULL);

		VectorScale(forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -2;

		VectorSet(offset, 0, 8, ent->viewheight - 8);
		P_ProjectSource(ent, offset, forward, right, start);
	}

	fire_shotgun(ent, start, forward, damage, kick,
			DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD,
			DEFAULT_SSHOTGUN_COUNT / 2, MOD_SSHOTGUN);

	if ((ent->client->use) && (ent->client->oldplayer))
	{
		v[YAW] = ent->client->oldplayer->s.angles[YAW] + 5;
	}
	else
	{
		v[YAW] = ent->client->v_angle[YAW] + 5;
	}

	AngleVectors(v, forward, NULL, NULL);

	if (aimfix->value)
	{
		AngleVectors(v, forward, right, NULL);

		VectorScale(forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -2;

		VectorSet(offset, 0, 8, ent->viewheight - 8);
		P_ProjectSource(ent, offset, forward, right, start);
	}

	fire_shotgun(ent, start, forward, damage, kick,
			DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD,
			DEFAULT_SSHOTGUN_COUNT / 2, MOD_SSHOTGUN);

	/* send muzzle flash */
	gi.WriteByte(svc_muzzleflash);

	if (ent->client->oldplayer)
	{
		gi.WriteShort(ent->client->oldplayer - g_edicts);
	}
	else
	{
		gi.WriteShort(ent - g_edicts);
	}

	gi.WriteByte(MZ_SSHOTGUN | is_silenced);

	if (ent->client->oldplayer)
	{
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
	}
}

void
Weapon_SuperShotgun(edict_t *ent)
{
	static int pause_frames[] = {29, 42, 57, 0};
	static int fire_frames[] = {7, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 6, 17, 57, 61, pause_frames,
			fire_frames, weapon_supershotgun_fire);

	if (is_quadfire)
	{
		Weapon_Generic(ent, 6, 17, 57, 61, pause_frames,
				fire_frames, weapon_supershotgun_fire);
	}
}

/*
 * ======================================================================
 *
 * RAILGUN
 *
 * ======================================================================
 */

void
weapon_railgun_fire(edict_t *ent)
{
	vec3_t start;
	vec3_t forward, right;
	vec3_t offset;
	int damage;
	int kick;

	if (!ent)
	{
		return;
	}

	if (deathmatch->value)
	{
		/* normal damage is too extreme in dm */
		damage = 100;
		kick = 200;
	}
	else
	{
		damage = 150;
		kick = 250;
	}

	if (is_quad)
	{
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	if ((ent->client->use) && (ent->client->oldplayer))
	{
		AngleVectors(ent->client->oldplayer->s.angles, forward, right, NULL);
	}
	else
	{
		AngleVectors(ent->client->v_angle, forward, right, NULL);
	}

	VectorScale(forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7, ent->viewheight - 8);
	P_ProjectSource(ent, offset, forward, right, start);
	fire_rail(ent, start, forward, damage, kick);

	/* send muzzle flash */
	gi.WriteByte(svc_muzzleflash);

	if (ent->client->oldplayer)
	{
		gi.WriteShort(ent->client->oldplayer - g_edicts);
	}
	else
	{
		gi.WriteShort(ent - g_edicts);
	}

	gi.WriteByte(MZ_RAILGUN | is_silenced);

	if (ent->client->oldplayer)
	{
		gi.multicast(ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		ent->client->pers.inventory[ent->client->ammo_index]--;
	}
}

void
Weapon_Railgun(edict_t *ent)
{
	static int pause_frames[] = {56, 0};
	static int fire_frames[] = {4, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 3, 18, 56, 61, pause_frames,
			fire_frames, weapon_railgun_fire);

	if (is_quadfire)
	{
		Weapon_Generic(ent, 3, 18, 56, 61, pause_frames,
				fire_frames, weapon_railgun_fire);
	}
}

/*
 * ======================================================================
 *
 * BFG10K
 *
 * ======================================================================
 */

void
weapon_bfg_fire(edict_t *ent)
{
	vec3_t offset, start, forward, right;
	float damage_radius = 1000;
	int damage;

	if (!ent)
	{
		return;
	}

	if (deathmatch->value)
	{
		damage = 200;
	}
	else
	{
		damage = 500;
	}

	if (ent->client->ps.gunframe == 9)
	{
		if ((ent->client->use) && (ent->client->oldplayer))
		{
			AngleVectors(ent->client->oldplayer->s.angles, forward, right, NULL);
		}
		else
		{
			AngleVectors(ent->client->v_angle, forward, right, NULL);
		}

		VectorSet(offset, 8, 8, ent->viewheight - 8);
		P_ProjectSource(ent, offset, forward, right, start);

		/* send muzzle flash */
		gi.WriteByte(svc_muzzleflash);

		if (ent->client->oldplayer)
		{
			gi.WriteShort(ent->client->oldplayer - g_edicts);
		}
		else
		{
			gi.WriteShort(ent - g_edicts);
		}

		gi.WriteByte(MZ_BFG | is_silenced);

		if (ent->client->oldplayer)
		{
			gi.multicast(ent->client->oldplayer->s.origin, MULTICAST_PVS);
		}
		else
		{
			gi.multicast(ent->s.origin, MULTICAST_PVS);
		}

		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}

	/* cells can go down during windup (from power armor hits), so
	   check again and abort firing if we don't have enough now */
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad)
	{
		damage *= damage_multiplier;
	}

	if ((ent->client->use) && (ent->client->oldplayer))
	{
		AngleVectors(ent->client->oldplayer->s.angles, forward, right, NULL);
	}
	else
	{
		AngleVectors(ent->client->v_angle, forward, right, NULL);
	}

	VectorScale(forward, -2, ent->client->kick_origin);

	/* make a big pitch kick with an inverse fall */
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom() * 8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight - 8);
	P_ProjectSource(ent, offset, forward, right, start);
	fire_bfg(ent, start, forward, damage, 400, damage_radius);

	ent->client->ps.gunframe++;

	/* send muzzle flash */
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_BFG2 | is_silenced);

	if (ent->client->oldplayer)
	{
		gi.multicast(ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
	}
}

void
Weapon_BFG(edict_t *ent)
{
	static int pause_frames[] = {39, 45, 50, 55, 0};
	static int fire_frames[] = {9, 17, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 8, 32, 55, 58, pause_frames,
			fire_frames, weapon_bfg_fire);

	if (is_quadfire)
	{
		Weapon_Generic(ent, 8, 32, 55, 58, pause_frames,
				fire_frames, weapon_bfg_fire);
	}
}

/* CHAINFIST */

void
weapon_chainfist_fire(edict_t *ent)
{
	vec3_t offset;
	vec3_t forward, right, up;
	vec3_t start;
	int damage;

	if (!ent)
	{
		return;
	}

	damage = 15;

	if (deathmatch->value)
	{
		damage = 30;
	}

	if (is_quad)
	{
		damage *= damage_multiplier;
	}

	AngleVectors(ent->client->v_angle, forward, right, up);

	/* kick back */
	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	/* set start point */
	VectorSet(offset, 0, 8, ent->viewheight - 4);
	P_ProjectSource(ent, offset, forward, right, start);

	fire_player_melee(ent, start, forward, CHAINFIST_REACH, damage,
			100, 1, MOD_CHAINFIST);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->client->ps.gunframe++;
	ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}

/*
 * this spits out some smoke from the motor. it's a two-stroke, you know.
 */
void
chainfist_smoke(edict_t *ent)
{
	vec3_t tempVec, forward, right, up;
	vec3_t offset;

	if (!ent)
	{
		return;
	}

	AngleVectors(ent->client->v_angle, forward, right, up);
	VectorSet(offset, 8, 8, ent->viewheight - 4);
	P_ProjectSource(ent, offset, forward, right, tempVec);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_CHAINFIST_SMOKE);
	gi.WritePosition(tempVec);
	gi.unicast(ent, 0);
}

void
Weapon_ChainFist(edict_t *ent)
{
	static int pause_frames[] = {0};
	static int fire_frames[] = {8, 9, 16, 17, 18, 30, 31, 0};

	/* these are caches for the sound index. there's probably a better way to do this. */
	float chance;
	int last_sequence;

	last_sequence = 0;

	if ((ent->client->ps.gunframe == 13) ||
		(ent->client->ps.gunframe == 23)) /* end of attack, go idle */
	{
		ent->client->ps.gunframe = 32;
	}

	/* holds for idle sequence */
	else if ((ent->client->ps.gunframe == 42) && (rand() & 7))
	{
		if ((ent->client->pers.hand != CENTER_HANDED) && (random() < 0.4))
		{
			chainfist_smoke(ent);
		}
	}
	else if ((ent->client->ps.gunframe == 51) && (rand() & 7))
	{
		if ((ent->client->pers.hand != CENTER_HANDED) && (random() < 0.4))
		{
			chainfist_smoke(ent);
		}
	}

	/* set the appropriate weapon sound. */
	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		ent->client->weapon_sound = gi.soundindex("weapons/sawhit.wav");
	}
	else if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		ent->client->weapon_sound = 0;
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/sawidle.wav");
	}

	Weapon_Generic(ent, 4, 32, 57, 60, pause_frames,
			fire_frames, weapon_chainfist_fire);

	if ((ent->client->buttons) & BUTTON_ATTACK)
	{
		if ((ent->client->ps.gunframe == 13) ||
			(ent->client->ps.gunframe == 23) ||
			(ent->client->ps.gunframe == 32))
		{
			last_sequence = ent->client->ps.gunframe;
			ent->client->ps.gunframe = 6;
		}
	}

	if (ent->client->ps.gunframe == 6)
	{
		chance = random();

		if (last_sequence == 13) /* if we just did sequence 1, do 2 or 3. */
		{
			chance -= 0.34;
		}
		else if (last_sequence == 23) /* if we just did sequence 2, do 1 or 3 */
		{
			chance += 0.33;
		}
		else if (last_sequence == 32) /* if we just did sequence 3, do 1 or 2 */
		{
			if (chance >= 0.33)
			{
				chance += 0.34;
			}
		}

		if (chance < 0.33)
		{
			ent->client->ps.gunframe = 14;
		}
		else if (chance < 0.66)
		{
			ent->client->ps.gunframe = 24;
		}
	}
}

/* Disintegrator */

void
weapon_tracker_fire(edict_t *self)
{
	vec3_t forward, right;
	vec3_t start;
	vec3_t end;
	vec3_t offset;
	edict_t *enemy;
	trace_t tr;
	int damage;
	vec3_t mins, maxs;

	if (!self)
	{
		return;
	}

	if (deathmatch->value)
	{
		damage = 30;
	}
	else
	{
		damage = 45;
	}

	if (is_quad)
	{
		damage *= damage_multiplier;
	}

	VectorSet(mins, -16, -16, -16);
	VectorSet(maxs, 16, 16, 16);
	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, self->viewheight - 8);
	P_ProjectSource(self, offset, forward, right, start);

	VectorMA(start, 8192, forward, end);
	enemy = NULL;
	tr = gi.trace(start, vec3_origin, vec3_origin, end, self, MASK_SHOT);

	if (tr.ent != world)
	{
		if (tr.ent->svflags & SVF_MONSTER || tr.ent->client || tr.ent->svflags & SVF_DAMAGEABLE)
		{
			if (tr.ent->health > 0)
			{
				enemy = tr.ent;
			}
		}
	}
	else
	{
		tr = gi.trace(start, mins, maxs, end, self, MASK_SHOT);

		if (tr.ent != world)
		{
			if (tr.ent->svflags & SVF_MONSTER || tr.ent->client ||
				tr.ent->svflags & SVF_DAMAGEABLE)
			{
				if (tr.ent->health > 0)
				{
					enemy = tr.ent;
				}
			}
		}
	}

	VectorScale(forward, -2, self->client->kick_origin);
	self->client->kick_angles[0] = -1;

	fire_tracker(self, start, forward, damage, 1000, enemy);

	/* send muzzle flash */
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(MZ_TRACKER);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	PlayerNoise(self, start, PNOISE_WEAPON);

	self->client->ps.gunframe++;
	self->client->pers.inventory[self->client->ammo_index] -= self->client->pers.weapon->quantity;
}

void
Weapon_Disintegrator(edict_t *ent)
{
	static int pause_frames[] = {14, 19, 23, 0};
	static int fire_frames[] = {5, 0};

	Weapon_Generic(ent, 4, 9, 29, 34, pause_frames,
			fire_frames, weapon_tracker_fire);
}

/*
 * ======================================================================
 *
 * ETF RIFLE
 *
 * ======================================================================
 */
void
weapon_etf_rifle_fire(edict_t *ent)
{
	vec3_t forward, right, up;
	vec3_t start, tempPt;
	int damage = 10;
	int kick = 3;
	int i;
	vec3_t offset;

	if (!ent)
	{
		return;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < ent->client->pers.weapon->quantity)
	{
		VectorClear(ent->client->kick_origin);
		VectorClear(ent->client->kick_angles);
		ent->client->ps.gunframe = 8;

		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}

		NoAmmoWeaponChange(ent);
		return;
	}

	if (is_quad)
	{
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	for (i = 0; i < 3; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.85;
		ent->client->kick_angles[i] = crandom() * 0.85;
	}

	/* get start / end positions */
	AngleVectors(ent->client->v_angle, forward, right, up);

	if (ent->client->ps.gunframe == 6) /* right barrel */
	{
		VectorSet(offset, 15, 8, -8);
	}
	else /* left barrel */
	{
		VectorSet(offset, 15, 6, -8);
	}

	VectorCopy(ent->s.origin, tempPt);
	tempPt[2] += ent->viewheight;
	P_ProjectSource2(ent, tempPt, offset, forward, right, up, start);
	fire_flechette(ent, start, forward, damage, 750, kick);

	/* send muzzle flash */
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_ETF_RIFLE);
	gi.multicast(ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->client->ps.gunframe++;

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
	}

	PlayerApplyAttack(ent);
}

void
Weapon_ETF_Rifle(edict_t *ent)
{
	static int pause_frames[] = {18, 28, 0};
	static int fire_frames[] = {6, 7, 0};

	if (!ent)
	{
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->pers.inventory[ent->client->ammo_index] <= 0)
		{
			ent->client->ps.gunframe = 8;
		}
	}

	Weapon_Generic(ent, 4, 7, 37, 41, pause_frames,
			fire_frames, weapon_etf_rifle_fire);

	if ((ent->client->ps.gunframe == 8) &&
		(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 6;
	}
}

void
Heatbeam_Fire(edict_t *ent)
{
	vec3_t start;
	vec3_t forward, right, up;
	vec3_t offset;
	int damage;
	int kick;

	if (!ent)
	{
		return;
	}

	if (deathmatch->value)
	{
		damage = HEATBEAM_DM_DMG;
		kick = 75;	/* really knock 'em around in deathmatch */
	}
	else
	{
		damage = HEATBEAM_SP_DMG;
		kick = 30;
	}

	ent->client->ps.gunframe++;
	ent->client->ps.gunindex = gi.modelindex("models/weapons/v_beamer2/tris.md2");

	if (is_quad)
	{
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	VectorClear(ent->client->kick_origin);
	VectorClear(ent->client->kick_angles);

	/* get start / end positions */
	AngleVectors(ent->client->v_angle, forward, right, up);

	/* This offset is the "view" offset for the beam start (used by trace) */
	VectorSet(offset, 7, 2, ent->viewheight - 3);
	P_ProjectSource(ent, offset, forward, right, start);

	/* This offset is the entity offset */
	VectorSet(offset, 2, 7, -3);

	fire_heatbeam(ent, start, forward, offset, damage, kick, false);

	/* send muzzle flash */
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_HEATBEAM | is_silenced);
	gi.multicast(ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
	}

	PlayerApplyAttack(ent);
}

void
Weapon_Heatbeam(edict_t *ent)
{
	static int pause_frames[] = {35, 0};
	static int fire_frames[] = {9, 10, 11, 12, 0};

	if (!ent)
	{
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		ent->client->weapon_sound = gi.soundindex("weapons/bfg__l1a.wav");

		if ((ent->client->pers.inventory[ent->client->ammo_index] >= 2) &&
			((ent->client->buttons) & BUTTON_ATTACK))
		{
			if (ent->client->ps.gunframe >= 13)
			{
				ent->client->ps.gunframe = 9;
				ent->client->ps.gunindex = gi.modelindex("models/weapons/v_beamer2/tris.md2");
			}
			else
			{
				ent->client->ps.gunindex = gi.modelindex("models/weapons/v_beamer2/tris.md2");
			}
		}
		else
		{
			ent->client->ps.gunframe = 13;
			ent->client->ps.gunindex = gi.modelindex("models/weapons/v_beamer/tris.md2");
		}
	}
	else
	{
		ent->client->ps.gunindex = gi.modelindex("models/weapons/v_beamer/tris.md2");
		ent->client->weapon_sound = 0;
	}

	Weapon_Generic(ent, 8, 12, 39, 44, pause_frames, fire_frames, Heatbeam_Fire);
}

/* ====================================================================== */

/* RipperGun */

void
weapon_ionripper_fire(edict_t *ent)
{
	vec3_t start;
	vec3_t forward, right;
	vec3_t offset;
	vec3_t tempang;
	int damage;

	if (!ent)
	{
		return;
	}

	if (deathmatch->value)
	{
		/* tone down for deathmatch */
		damage = 30;
	}
	else
	{
		damage = 50;
	}

	if (is_quad)
	{
		damage *= 4;
	}

	VectorCopy(ent->client->v_angle, tempang);
	tempang[YAW] += crandom();

	AngleVectors(tempang, forward, right, NULL);

	VectorScale(forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 16, 7, ent->viewheight - 8);

	P_ProjectSource(ent, offset, forward, right, start);

	fire_ionripper(ent, start, forward, damage, 500, EF_IONRIPPER);

	/* send muzzle flash */
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_IONRIPPER | is_silenced);
	gi.multicast(ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		ent->client->pers.inventory[ent->client->ammo_index] -=
			ent->client->pers.weapon->quantity;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 0)
	{
		ent->client->pers.inventory[ent->client->ammo_index] = 0;
	}
}

void
Weapon_Ionripper(edict_t *ent)
{
	static int pause_frames[] = {36, 0};
	static int fire_frames[] = {5, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 4, 6, 36, 39, pause_frames,
			fire_frames, weapon_ionripper_fire);

	if (is_quadfire)
	{
		Weapon_Generic(ent, 4, 6, 36, 39, pause_frames,
				fire_frames, weapon_ionripper_fire);
	}
}

/*	Phalanx */

void
weapon_phalanx_fire(edict_t *ent)
{
	vec3_t start;
	vec3_t forward, right, up;
	vec3_t offset;
	vec3_t v;
	int damage;
	float damage_radius;
	int radius_damage;

	if (!ent)
	{
		return;
	}

	damage = 70 + (int)(random() * 10.0);
	radius_damage = 120;
	damage_radius = 120;

	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors(ent->client->v_angle, forward, right, NULL);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8, ent->viewheight - 8);
	P_ProjectSource(ent, offset, forward, right, start);

	if (ent->client->ps.gunframe == 8)
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW] = ent->client->v_angle[YAW] - 1.5;
		v[ROLL] = ent->client->v_angle[ROLL];
		AngleVectors(v, forward, right, up);

		radius_damage = 30;
		damage_radius = 120;

		fire_plasma(ent, start, forward, damage, 725,
				damage_radius, radius_damage);

		/* send muzzle flash */
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent - g_edicts);
		gi.WriteByte(MZ_PHALANX2 | is_silenced);
		gi.multicast(ent->s.origin, MULTICAST_PVS);

		if (!((int)dmflags->value & DF_INFINITE_AMMO))
		{
			ent->client->pers.inventory[ent->client->ammo_index]--;
		}
	}
	else
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW] = ent->client->v_angle[YAW] + 1.5;
		v[ROLL] = ent->client->v_angle[ROLL];
		AngleVectors(v, forward, right, up);
		fire_plasma(ent, start, forward, damage, 725,
				damage_radius, radius_damage);

		/* send muzzle flash */
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent - g_edicts);
		gi.WriteByte(MZ_PHALANX | is_silenced);
		gi.multicast(ent->s.origin, MULTICAST_PVS);

		PlayerNoise(ent, start, PNOISE_WEAPON);
	}

	ent->client->ps.gunframe++;
}

void
Weapon_Phalanx(edict_t *ent)
{
	static int pause_frames[] = {29, 42, 55, 0};
	static int fire_frames[] = {7, 8, 0};

	if (!ent)
	{
		return;
	}

	Weapon_Generic(ent, 5, 20, 58, 63, pause_frames,
			fire_frames, weapon_phalanx_fire);

	if (is_quadfire)
	{
		Weapon_Generic(ent, 5, 20, 58, 63, pause_frames,
				fire_frames, weapon_phalanx_fire);
	}
}

/* TRAP */

void
weapon_trap_fire(edict_t *ent, qboolean held)
{
	vec3_t offset;
	vec3_t forward, right;
	vec3_t start;
	int damage = 125;
	float timer;
	int speed;
	float radius;

	if (!ent)
	{
		return;
	}

	radius = damage + 40;

	if (is_quad)
	{
		damage *= 4;
	}

	VectorSet(offset, 8, 8, ent->viewheight - 8);
	AngleVectors(ent->client->v_angle, forward, right, NULL);
	P_ProjectSource(ent, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) *
		((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_trap(ent, start, forward, damage, speed, timer, radius, held);

	ent->client->pers.inventory[ent->client->ammo_index]--;
	ent->client->grenade_time = level.time + 1.0;
}

void
Weapon_Trap(edict_t *ent)
{
	if (!ent)
	{
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon(ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if (((ent->client->latched_buttons |
			  ent->client->buttons) & BUTTON_ATTACK))
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;

			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),
							1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}

				NoAmmoWeaponChange(ent);
			}

			return;
		}

		if ((ent->client->ps.gunframe == 29) ||
			(ent->client->ps.gunframe == 34) ||
			(ent->client->ps.gunframe == 39) ||
			(ent->client->ps.gunframe == 48))
		{
			if (rand() & 15)
			{
				return;
			}
		}

		if (++ent->client->ps.gunframe > 48)
		{
			ent->client->ps.gunframe = 16;
		}

		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/trapcock.wav"),
					1, ATTN_NORM, 0);
		}

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/traploop.wav");
			}

			/* they waited too long, detonate it in their hand */
			if (!ent->client->grenade_blew_up &&
				(level.time >= ent->client->grenade_time))
			{
				ent->client->weapon_sound = 0;
				weapon_trap_fire(ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
			{
				return;
			}

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_trap_fire(ent, false);

			if (ent->client->pers.inventory[ent->client->ammo_index] == 0)
			{
				NoAmmoWeaponChange(ent);
			}
		}

		if ((ent->client->ps.gunframe == 15) &&
			(level.time < ent->client->grenade_time))
		{
			return;
		}

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

// ************************************************************************************************
// Weapon_CalcStartPos
// -------------------
// Sets up a spell missile's starting position, correct with respect to the player model's angles
// and posture. We want the start position of the missile to be the caster's hand, so we must take
// into account the caster's joint orientations and global model orientation. This is a big-arsed
// hack. We assume that if the caster is standing fully upstraight, the missile will be launched
// from default co-ordinates (relative to the caster's origin) specified in DefaultStartPos. The
// other two inputs, LowerJoint & UpperJoint specify Corvus's bone joint positions (relative to his
// model's origin) for the animation frame in which the weapon is launched.
// *************************************************************************************************

static void
Weapon_CalcStartPos(vec3_t OriginToLowerJoint,vec3_t OriginToUpperJoint,
								vec3_t DefaultStartPos,vec3_t ActualStartPos, edict_t *caster)
{
	matrix3_t	LowerRotationMatrix,UpperRotationMatrix;
	vec3_t		LowerbackJointAngles,UpperbackJointAngles,
				UpperJoint,LowerJointToUpperJoint,
				Forward,Right,Up,
				StartPos;

	// Get matrices corresponding to the current angles of the upper and lower back joints.

	LowerbackJointAngles[PITCH]=GetJointAngle(caster->s.rootJoint+CORVUS_LOWERBACK,PITCH);
	LowerbackJointAngles[YAW]=GetJointAngle(caster->s.rootJoint+CORVUS_LOWERBACK,YAW);
	LowerbackJointAngles[ROLL]=GetJointAngle(caster->s.rootJoint+CORVUS_LOWERBACK,ROLL);
	Matrix3FromAngles(LowerbackJointAngles,LowerRotationMatrix);

	UpperbackJointAngles[PITCH]=GetJointAngle(caster->s.rootJoint+CORVUS_UPPERBACK,PITCH);
	UpperbackJointAngles[YAW]=GetJointAngle(caster->s.rootJoint+CORVUS_UPPERBACK,YAW);
	UpperbackJointAngles[ROLL]=GetJointAngle(caster->s.rootJoint+CORVUS_UPPERBACK,ROLL);
	Matrix3FromAngles(UpperbackJointAngles,UpperRotationMatrix);

	// Get vector from player model's origin to upper joint.

	VectorAdd(caster->s.origin,OriginToUpperJoint,UpperJoint);

	// Get vector from lower joint to upper joint.

	VectorSubtract(OriginToUpperJoint,OriginToLowerJoint,LowerJointToUpperJoint);

	// Get vector from upper joint to the default flying-fist's start position.

	AngleVectors(caster->s.angles,Forward,Right,Up);
	VectorMA(caster->s.origin,DefaultStartPos[0],Right,StartPos);
	VectorMA(StartPos,DefaultStartPos[1],Forward,StartPos);
	VectorMA(StartPos,DefaultStartPos[2],Up,StartPos);
	VectorSubtract(StartPos,UpperJoint,StartPos);

	// Add in the contribution from the 'bone' from the lower joint to upper joint.

	Matrix3MultByVec3(UpperRotationMatrix,StartPos,StartPos);
	VectorAdd(StartPos,LowerJointToUpperJoint,StartPos);

	// Add in the contribution from the model's origin to the lower joint.

	Matrix3MultByVec3(LowerRotationMatrix,StartPos,StartPos);
	VectorAdd(OriginToLowerJoint,StartPos,StartPos);

	// Finally, add on the model's origin to give the correct start position for the flying-fist.

	VectorAdd(StartPos,caster->s.origin,StartPos);

	VectorCopy(StartPos,ActualStartPos);
}

// ************************************************************************************************
// WeaponThink_SwordStaff
// ----------------------
// ************************************************************************************************

enum swordpos_e
{
	SWORD_ATK_L,
	SWORD_ATK_R,
	SWORD_SPINATK_L,
	SWORD_SPINATK_R,
	SWORD_ATK_B,
	SWORD_ATK_STAB
};

vec3_t swordpositions[23] =
{
	{	0,		0,		0	},	// 0
	{	-20,	-20,	26	},	// 1	swipeA4
	{	4,		-34,	22	},	// 2	swipeA5
	{	43,		-16,	-10	},	// 3	swipeA6
	{	33,		20,		-32	},	// 4	swipeA7

	{	-16,	12,		20	},	// 5	swipeB4
	{	8,		34,		16	},	// 6	swipeB5
	{	40,		-16,	-10	},	// 7	swipeB6
	{	-8,		-24,	-22	},	// 8	swipeB7

	{	-32,	0,		32	},	// 9	newspin5
	{	24,		-36,	8	},	// 10	newspin6
	{	44,		20,		-20	},	// 11	newspin7
	{	0,		0,		0	},	// 12

	{	-24,	0,		20	},	// 13	spining4
	{	24,		36,		16	},	// 14	spining5
	{	36,		-36,	-20	},	// 15	spining6
	{	0,		0,		0	},	// 16

	{	-12,	-12, 	-12	},	// 17	roundbck2
	{	-20,	28,		-4	},	// 18	roundbck3
	{	16,		36,		0	},	// 19	roundbck4
	{	0,		0,		0	},	// 20

	{	12,		0,		-12	},	// 21	spikedown7
	{	20,		0,		-48	},	// 22	spikedown8
};

int sworddamage[STAFF_LEVEL_MAX][2] =
{	//	MIN		MAX
	{	0,						0						},		// STAFF_LEVEL_NONE
	{	SWORD_DMG_MIN,			SWORD_DMG_MAX			},		// STAFF_LEVEL_BASIC
	{	SWORD_POWER1_DMG_MIN,	SWORD_POWER1_DMG_MAX	},		// STAFF_LEVEL_POWER1
	{	SWORD_POWER2_DMG_MIN,	SWORD_POWER2_DMG_MAX	},		// STAFF_LEVEL_POWER2
};

void
WeaponThink_SwordStaffEx(edict_t *caster, int locid)
{
	vec3_t fwd, right, up;
	vec3_t atkpos, startpos, endpos, hitdir, hitangles, diffangles;
	vec3_t mins = { -12, -12, -12};
	vec3_t maxs = { 12, 12, 12};
	int damage, powerlevel, dflags;
	playerinfo_t *playerinfo;

	trace_t trace;

	assert(caster->client);
	playerinfo = &caster->client->playerinfo;
	assert(playerinfo);

	powerlevel = caster->client->pers.stafflevel;
	if (playerinfo->powerup_timer > level.time)
	{
		powerlevel++;							// powerups now power up your staff, too.
	}

	if (powerlevel <= STAFF_LEVEL_NONE)
	{
		return;
	}
	else if (powerlevel >= STAFF_LEVEL_MAX)
	{
		powerlevel = STAFF_LEVEL_MAX-1;
	}

	AngleVectors(caster->client->ps.viewangles, fwd, right, up);

	// Set up the area to check.
	VectorCopy(swordpositions[locid], atkpos);
	VectorMA(caster->s.origin, atkpos[0], fwd, endpos);
	VectorMA(endpos, -atkpos[1], right, endpos);
	VectorMA(endpos, atkpos[2], up, endpos);

	// Now if we are the first attack of this sweep (1, 5, 9, 13), starting in solid means a hit.  If not, then we must avoid startsolid entities.
	if ((locid & 0x03) == 0x01)
	{	// First check of the swing.
		caster->client->lastentityhit = NULL;
		VectorCopy(endpos, startpos);
	}
	else
	{
		VectorCopy(swordpositions[locid-1], atkpos);
		VectorMA(caster->s.origin, atkpos[0], fwd, startpos);
		VectorMA(startpos, -atkpos[1], right, startpos);
		VectorMA(startpos, atkpos[2], up, startpos);
	}
	startpos[2] += caster->viewheight;
	endpos[2] += caster->viewheight;

	VectorCopy(endpos, caster->client->laststaffpos);
	caster->client->laststaffuse = level.time;

	trace = gi.trace(startpos, mins, maxs, endpos, caster, MASK_PLAYERSOLID|CONTENTS_DEADMONSTER);
	if (level.fighting_beast)
	{
		edict_t *ent;

		if ((ent = check_hit_beast(startpos, trace.endpos)))
			trace.ent = ent;
	}

	if (trace.ent && trace.ent->takedamage)
	{
		if (!trace.startsolid || trace.ent != caster->client->lastentityhit)
		{
			if (playerinfo->advancedstaff && trace.ent->client && trace.ent->client->playerinfo.block_timer >= level.time)
			{	// Crimminy, what if they're blocking?
				// Check angle
				VectorSubtract(caster->s.origin, trace.ent->s.origin, hitdir);
				VectorNormalize(hitdir);
				VectoAngles(hitdir, hitangles);
				diffangles[YAW] = hitangles[YAW] - trace.ent->client->ps.viewangles[YAW];
				if (diffangles[YAW] > 180.0)
					diffangles[YAW] -= 360.0;
				else if (diffangles[YAW] < -180.0)
					diffangles[YAW] += 360.0;
				diffangles[PITCH] = hitangles[PITCH] - trace.ent->client->ps.viewangles[PITCH];

				if (diffangles[YAW] > -60.0 &&
						diffangles[YAW] < 60.0 &&
						diffangles[PITCH] > -45.0 &&
						diffangles[PITCH] < 75.0)
				{	// The opponent is indeed facing you...
					if (powerlevel >= STAFF_LEVEL_POWER2)
					{
						gi.CreateEffect(NULL,
										FX_BLOCK_SPARKS,
										CEF_FLAG7,
										trace.endpos,
										"d",
										hitdir);
					}
					else
					{
						gi.CreateEffect(NULL,
										FX_BLOCK_SPARKS,
										0,
										trace.endpos,
										"d",
										hitdir);
					}

					AlertMonsters(caster, caster, 1, true);
					switch (irand(1,3))
					{
					case 1:
						gi.sound(caster,  CHAN_AUTO, gi.soundindex("weapons/ArmorRic1.wav"), 1, ATTN_NORM, 0);
						break;
					case 2:
						gi.sound(caster,  CHAN_AUTO, gi.soundindex("weapons/ArmorRic2.wav"), 1, ATTN_NORM, 0);
						break;
					case 3:
						gi.sound(caster,  CHAN_AUTO, gi.soundindex("weapons/ArmorRic3.wav"), 1, ATTN_NORM, 0);
						break;
					}
					caster->client->lastentityhit = trace.ent;

					// Now we're in trouble, go into the attack recoil...
					switch ((locid-1)>>2)
					{
					case SWORD_ATK_L:
						playerExport->PlayerAnimSetUpperSeq(playerinfo, ASEQ_WSWORD_BLOCKED_L);
						// And of course the blocker must react too.
						playerExport->PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_L);
						return;
						break;
					case SWORD_ATK_R:
						playerExport->PlayerAnimSetUpperSeq(playerinfo, ASEQ_WSWORD_BLOCKED_R);
						// And of course the blocker must react too.
						playerExport->PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_R);
						return;
						break;
					case SWORD_SPINATK_L:
						playerExport->PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPINBLOCKED);
						// And of course the blocker must react too.
						playerExport->PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_L);
						return;
						break;
					case SWORD_SPINATK_R:
						playerExport->PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPINBLOCKED2);
						// And of course the blocker must react too.
						playerExport->PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_R);
						return;
						break;
					case SWORD_ATK_B:
						playerExport->PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_R);
						return;
						break;
					case SWORD_ATK_STAB:
						playerExport->PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCK_L);
						return;
						break;
					}
				}
			}

			if (playerinfo->advancedstaff && trace.ent->client && trace.ent->client->laststaffuse+0.1 >= level.time)
			{	// Check if the staffs collided.
				VectorSubtract(trace.endpos, trace.ent->client->laststaffpos, hitdir);

				if (VectorLength(hitdir) < 48.0)
				{	// Let's make these two staffs collide.
					if (powerlevel >= STAFF_LEVEL_POWER2)
					{
						gi.CreateEffect(NULL,
										FX_BLOCK_SPARKS,
										CEF_FLAG7,
										trace.endpos,
										"d",
										hitdir);
					}
					else
					{
						gi.CreateEffect(NULL,
										FX_BLOCK_SPARKS,
										0,
										trace.endpos,
										"d",
										hitdir);
					}

					AlertMonsters (caster, caster, 1, true);
					switch (irand(1,3))
					{
					case 1:
						gi.sound(caster,  CHAN_AUTO, gi.soundindex("weapons/ArmorRic1.wav"), 1, ATTN_NORM, 0);
						break;
					case 2:
						gi.sound(caster,  CHAN_AUTO, gi.soundindex("weapons/ArmorRic2.wav"), 1, ATTN_NORM, 0);
						break;
					case 3:
						gi.sound(caster,  CHAN_AUTO, gi.soundindex("weapons/ArmorRic3.wav"), 1, ATTN_NORM, 0);
						break;
					}
					caster->client->lastentityhit = trace.ent;
					trace.ent->client->lastentityhit = caster;

					// Now we're in trouble, go into the attack recoil...
					switch ((locid-1)>>2)
					{
					case SWORD_ATK_L:
						playerExport->PlayerAnimSetUpperSeq(playerinfo, ASEQ_WSWORD_BLOCKED_L);
						// And of course the blocker must react too.
						playerExport->PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_L);
						return;
						break;
					case SWORD_ATK_R:
						playerExport->PlayerAnimSetUpperSeq(playerinfo, ASEQ_WSWORD_BLOCKED_R);
						// And of course the blocker must react too.
						playerExport->PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_R);
						return;
						break;
					case SWORD_SPINATK_L:
						playerExport->PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPINBLOCKED);
						// And of course the blocker must react too.
						playerExport->PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_L);
						return;
						break;
					case SWORD_SPINATK_R:
						playerExport->PlayerAnimSetLowerSeq(playerinfo, ASEQ_WSWORD_SPINBLOCKED2);
						// And of course the blocker must react too.
						playerExport->PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_R);
						return;
						break;
					case SWORD_ATK_B:
						playerExport->PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_R);
						return;
						break;
					case SWORD_ATK_STAB:
						playerExport->PlayerAnimSetUpperSeq(&trace.ent->client->playerinfo, ASEQ_WSWORD_BLOCKED_L);
						return;
						break;
					}
				}
			}

			if (CanDamage (trace.ent, caster))
			{
				VectorSubtract(endpos, startpos, hitdir);
				VectorNormalize2(hitdir, hitdir);

				if (powerlevel > STAFF_LEVEL_POWER2)
					powerlevel = STAFF_LEVEL_POWER2;

				damage = irand(sworddamage[powerlevel][0], sworddamage[powerlevel][1]);
				// Spin attacks should double damage
				switch ((locid-1)>>2)
				{
//				case SWORD_ATK_L:
//					break;
//				case SWORD_ATK_R:
//					break;
				case SWORD_SPINATK_L:
				case SWORD_SPINATK_R:
					damage *= SWORD_SPIN_DMG_MOD;		// 50% more damage from spins.
					break;
				case SWORD_ATK_B:
					damage *= SWORD_BACK_DMG_MOD;		// Half damage from behind.
					break;
				case SWORD_ATK_STAB:
					damage *= SWORD_STAB_DMG_MOD;		// Double damage from stab.
					break;
				}

				if (caster->client)
				{
					if (playerinfo->flags & PLAYER_FLAG_NO_LARM)
					{
						damage = ceil(damage/3);//only one arm 1/3 the damage
					}
				}

				switch (powerlevel)
				{
				case STAFF_LEVEL_BASIC:
					dflags = DAMAGE_EXTRA_KNOCKBACK|DAMAGE_DISMEMBER;
					break;

				case STAFF_LEVEL_POWER1:
					dflags = DAMAGE_EXTRA_KNOCKBACK|DAMAGE_DISMEMBER|DAMAGE_DOUBLE_DISMEMBER;
					break;

				case STAFF_LEVEL_POWER2:
					dflags = DAMAGE_EXTRA_KNOCKBACK|DAMAGE_DISMEMBER|DAMAGE_FIRE;
					break;
				}

				T_Damage (trace.ent, caster, caster, fwd, trace.endpos, hitdir, damage, damage*4, dflags,MOD_STAFF);

				// If we hit a monster, stick a trail of blood on the staff...
				if (trace.ent->svflags & SVF_MONSTER)
				{
					if (trace.ent->gib == GIB_INSECT)//yellow blood
						gi.CreateEffect(caster, FX_LINKEDBLOOD, CEF_FLAG8|CEF_OWNERS_ORIGIN, NULL, "bb", 30, CORVUS_BLADE);
					else
						gi.CreateEffect(caster, FX_LINKEDBLOOD, CEF_OWNERS_ORIGIN, NULL, "bb", 30, CORVUS_BLADE);
				}

				if (trace.ent->svflags & SVF_MONSTER || trace.ent->client)
				{
					caster->s.effects |= EF_BLOOD_ENABLED;
					playerinfo->effects |= EF_BLOOD_ENABLED;
				}

				//Use special hit puff
				switch (powerlevel)
				{
				case STAFF_LEVEL_BASIC:
					gi.sound(caster, CHAN_AUTO, gi.soundindex("weapons/staffhit.wav"), 1, ATTN_NORM, 0);
					break;

				case STAFF_LEVEL_POWER1:
					gi.CreateEffect(NULL,
									FX_WEAPON_STAFF_STRIKE,
									0,
									trace.endpos,
									"db",
									trace.plane.normal,
									powerlevel);

					gi.sound(caster, CHAN_AUTO, gi.soundindex("weapons/staffhit_2.wav"), 1, ATTN_NORM, 0);
					break;

				case STAFF_LEVEL_POWER2:
					gi.CreateEffect(NULL,
									FX_WEAPON_STAFF_STRIKE,
									0,
									trace.endpos,
									"db",
									trace.plane.normal,
									powerlevel);

					gi.sound(caster, CHAN_AUTO, gi.soundindex("weapons/staffhit_3.wav"), 1, ATTN_NORM, 0);
					break;
				}

				caster->client->lastentityhit = trace.ent;
			}
		}
	}
	else if (trace.fraction < 1.0 || trace.startsolid)
	{	// Hit a wall or such...
		if (caster->client->lastentityhit == NULL && Vec3NotZero(trace.plane.normal))
		{	// Don't do sparks if already hit something
			VectoAngles(trace.plane.normal, hitangles);

			if (powerlevel >= STAFF_LEVEL_POWER2)
			{
				gi.CreateEffect(NULL,
								FX_SPARKS,
								CEF_FLAG7,
								trace.endpos,
								"d",
								hitdir);
			}
			else
			{
				gi.CreateEffect(NULL,
								FX_SPARKS,
								0,
								trace.endpos,
								"d",
								hitdir);
			}

			AlertMonsters (caster, caster, 1, true);
			gi.sound(caster, CHAN_AUTO, gi.soundindex("weapons/staffhitwall.wav"), 1, ATTN_NORM, 0);

			caster->client->lastentityhit = WALL_ENTITY;
		}
	}
}

void
WeaponThink_SwordStaff(edict_t *caster)
{
	WeaponThink_SwordStaffEx(caster, 0);
}

// ************************************************************************************************
// WeaponThink_FlyingFist
// ----------------------
// ************************************************************************************************
void
WeaponThink_FlyingFist(edict_t *caster)
{
	vec3_t	OriginToLowerJoint={0.945585,2.26076,0.571354},
			OriginToUpperJoint={1.80845,2.98912,3.27800},
			DefaultStartPos={18.0,10.0,15.0},
			StartPos,
			Forward;

	// Set up the Magic-missile's starting position and aiming angles then cast the spell.

	Weapon_CalcStartPos(OriginToLowerJoint, OriginToUpperJoint, DefaultStartPos, StartPos, caster);

	AngleVectors(caster->client->ps.viewangles, Forward, NULL, NULL);

	StartPos[2] += caster->viewheight - 14.0;
	SpellCastFlyingFist(caster, StartPos, caster->client->ps.viewangles, Forward,0.0);

	// Take off mana, but if there is none, then fire a wimpy fizzle-weapon.
	if (caster->client->pers.inventory[caster->client->playerinfo.weap_ammo_index] > 0)
	{
		if (!(deathmatch->value && ((int)dmflags->value & DF_INFINITE_MANA)))
				caster->client->pers.inventory[caster->client->playerinfo.weap_ammo_index] -=
						caster->client->playerinfo.pers.weapon->quantity;
	}
}

// ************************************************************************************************
// WeaponThink_Maceballs
// ----------------------
// ************************************************************************************************

void
WeaponThink_Maceballs(edict_t *caster)
{
	vec3_t	OriginToLowerJoint={0.945585,2.26076,0.571354},
			OriginToUpperJoint={1.80845,2.98912,3.27800},
			defaultstartpos={-4.0,15.0,15.0},		// Ripper start position
			defaultstartpos2={13.0,15.0,-15.0},		// Maceball start position
			startpos,
			fwd;

	assert(caster->client);

	if (caster->client->playerinfo.powerup_timer > level.time)
	{
		// Set up the ball's starting position and aiming angles then cast the spell.
		Weapon_CalcStartPos(OriginToLowerJoint, OriginToUpperJoint, defaultstartpos2, startpos, caster);

		AngleVectors(caster->client->ps.viewangles, fwd, NULL, NULL);
		startpos[2] += caster->viewheight - 14.0;

		SpellCastMaceball(caster, startpos, caster->client->ps.viewangles, NULL, 0.0);
		// Giant iron dooms require lotsa mana, but yer average ripper needs far less.
		if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
			caster->client->pers.inventory[caster->client->playerinfo.weap_ammo_index] -=
					caster->client->playerinfo.pers.weapon->quantity * 2.0;
	}
	else
	{
		// Set up the ball's starting position and aiming angles then cast the spell.
		Weapon_CalcStartPos(OriginToLowerJoint, OriginToUpperJoint,defaultstartpos,startpos,caster);

		AngleVectors(caster->client->ps.viewangles, fwd, NULL, NULL);
		startpos[2] += caster->viewheight - 14.0;

		SpellCastRipper(caster, startpos, caster->client->ps.viewangles, NULL);
		if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
			caster->client->pers.inventory[caster->client->playerinfo.weap_ammo_index] -=
					caster->client->playerinfo.pers.weapon->quantity;		// Un-powered
	}
}

// ************************************************************************************************
// WeaponThink_MagicMissileSpread
// ------------------------------
// ************************************************************************************************
#define MISSILE_YAW 7.0
#define MISSILE_PITCH 2.0
#define MISSILE_SEP	4.0

void
WeaponThink_MagicMissileSpreadEx(edict_t *caster, int missilepos)
{
	vec3_t	OriginToLowerJoint={0.945585,2.26076,0.571354},
			OriginToUpperJoint={1.80845,2.98912,3.27800},
			DefaultStartPos={8.0,0.0,5.0},
			StartPos;
	vec3_t	fireangles, fwd;

	// Set up the Magic-missile's starting position and aiming angles then cast the spell.

	// Push the start position forward for earlier shots
	DefaultStartPos[0] -= MISSILE_SEP*missilepos;
	DefaultStartPos[1] += MISSILE_SEP*missilepos;
	Weapon_CalcStartPos(OriginToLowerJoint,OriginToUpperJoint,DefaultStartPos,StartPos,caster);
	StartPos[2] += caster->viewheight - 14.0;

	VectorCopy(caster->client->ps.viewangles, fireangles);
	fireangles[YAW] += missilepos*MISSILE_YAW;
	fireangles[PITCH] += missilepos*MISSILE_PITCH;
	AngleVectors(fireangles, fwd, NULL, NULL);
	SpellCastMagicMissile(caster, StartPos, fireangles, fwd);

	if (missilepos == -1.0)
		gi.sound(caster,CHAN_WEAPON, gi.soundindex("weapons/MagicMissileSpreadFire.wav"), 1, ATTN_NORM, 0);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->pers.inventory[caster->client->playerinfo.weap_ammo_index]--;
}

void
WeaponThink_MagicMissileSpread(edict_t *caster)
{
	WeaponThink_MagicMissileSpreadEx(caster, 1);
}

// ************************************************************************************************
// WeaponThink_SphereOfAnnihilation
// -------------------------------
// ************************************************************************************************

void
WeaponThink_SphereOfAnnihilationEx(edict_t *caster, float value)
{
	vec3_t		Forward;

	// Set up the Sphere-of-annihilation's aiming angles then cast the spell.
	AngleVectors(caster->client->ps.viewangles,Forward, NULL, NULL);

	SpellCastSphereOfAnnihilation(caster,
								 NULL,
								 caster->client->ps.viewangles,		//v_angle,
								 Forward,
								 0.0,
								 value);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->pers.inventory[caster->client->playerinfo.weap_ammo_index]-= caster->client->playerinfo.pers.weapon->quantity;
}

void
WeaponThink_SphereOfAnnihilation(edict_t *caster)
{
	WeaponThink_SphereOfAnnihilationEx(caster, 1.0);
}

// ************************************************************************************************
// WeaponThink_Firewall
// -------------------------------
// ************************************************************************************************

void
WeaponThink_Firewall(edict_t *caster)
{
	SpellCastWall(caster, caster->s.origin, caster->client->ps.viewangles, NULL, 0.0);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->pers.inventory[caster->client->playerinfo.weap_ammo_index] -= caster->client->playerinfo.pers.weapon->quantity;
}

// ************************************************************************************************
// WeaponThink_RedRainBow
// ----------------------
// ************************************************************************************************

void
WeaponThink_RedRainBow(edict_t *caster)
{
	vec3_t	StartPos, Forward, Right;

	AngleVectors(caster->client->ps.viewangles, Forward, Right, NULL);
	VectorMA(caster->s.origin, 25.0F, Forward, StartPos);
	VectorMA(StartPos, 6.0F, Right, StartPos);
	StartPos[2] += caster->viewheight + 4.0;

	SpellCastRedRain(caster, StartPos, caster->client->ps.viewangles, NULL, 0.0F);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->pers.inventory[caster->client->playerinfo.weap_ammo_index] -= caster->client->playerinfo.pers.weapon->quantity;
}

// ************************************************************************************************
// WeaponThink_PhoenixBow
// ----------------------
// ************************************************************************************************

void
WeaponThink_PhoenixBow(edict_t *caster)
{
	vec3_t	StartPos, Forward, Right;

	AngleVectors(caster->client->ps.viewangles, Forward, Right, NULL);
	VectorMA(caster->s.origin, 25.0F, Forward, StartPos);
	VectorMA(StartPos, 6.0F, Right, StartPos);
	StartPos[2] += caster->viewheight + 4.0;

	SpellCastPhoenix(caster, StartPos, caster->client->ps.viewangles, Forward, 0.0F);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->pers.inventory[caster->client->playerinfo.weap_ammo_index] -= caster->client->playerinfo.pers.weapon->quantity;
}

// ************************************************************************************************
// WeaponThink_HellStaff
// ---------------------
// ************************************************************************************************

void WeaponThink_HellStaff(edict_t *caster)
{
	vec3_t	StartPos;	//, off;
	vec3_t	fwd, right;
//	vec3_t	startangle;

	// Set up the Hellstaff's starting position and aiming angles then cast the spell.
//	VectorSet(off, 34.0, -6.0, 0.0);
//	VectorGetOffsetOrigin(off, caster->s.origin, caster->client->ps.viewangles[YAW], StartPos);

	// Two-thirds of the player angle is torso movement.
/*	startangle[PITCH] = (caster->client->ps.viewangles[PITCH] - caster->s.angles[PITCH]) * 2.0 / 3.0;
	startangle[YAW] = caster->client->ps.viewangles[YAW] - caster->s.angles[YAW];
	if (startangle[YAW] > 180.0)
		startangle[YAW] -= 360.0;
	else if (startangle[YAW] < -180.0)
		startangle[YAW] += 360;
	startangle[YAW] *= 2.0/3.0;
	startangle[ROLL] = 0.0;
*/
//	VectorAdd(startangle, caster->s.angles, startangle);
//	AngleVectors(startangle, fwd, right, NULL);
	AngleVectors(caster->client->ps.viewangles, fwd, right, NULL);
	VectorMA(caster->s.origin,30,fwd,StartPos);
	VectorMA(StartPos,10,right,StartPos);
	StartPos[2] += caster->viewheight - 14.0;

	SpellCastHellstaff(caster, StartPos, caster->client->ps.viewangles, NULL);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->pers.inventory[caster->client->playerinfo.weap_ammo_index] -= caster->client->playerinfo.pers.weapon->quantity;
}

// ************************************************************************************************
// WeaponThink_Blast
// ---------------------
// ************************************************************************************************

void
WeaponThink_Blast(edict_t *caster)
{
	vec3_t	startpos;
	vec3_t	fwd, right;
	vec3_t	mins={-3.0, -3.0, -3.0}, maxs={3.0, 3.0, 3.0};
	trace_t	trace;

	assert(caster->client);

	// Find the firing position first.
	AngleVectors(caster->client->ps.viewangles, fwd, right, NULL);
	VectorMA(caster->s.origin,10,fwd,startpos);
	VectorMA(startpos, -4.0F, right, startpos);
	startpos[2] += caster->viewheight;

	// Trace from the player's origin to the casting location to assure not spawning in a wall.
	trace = gi.trace(caster->s.origin, mins, maxs, startpos, caster, MASK_SHOT);
	if (trace.startsolid || trace.allsolid)
	{	// No way to avoid spawning in a wall.
		return;
	}

	if (trace.fraction < 1.0)
	{
		VectorCopy(trace.endpos, startpos);
	}

	// This weapon does not autotarget
	SpellCastBlast(caster, startpos, caster->client->ps.viewangles, NULL);

	if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
		caster->client->pers.inventory[caster->client->playerinfo.weap_ammo_index] -= caster->client->playerinfo.pers.weapon->quantity;

	gi.sound(caster,CHAN_WEAPON, gi.soundindex("weapons/BlastFire.wav"), 1, ATTN_NORM, 0);
}

// ************************************************************************************************
// Weapon_Ready
// ------------
// Make the specified weapon ready if the owner has enough ammo for it. Assumes that the owner does
// actually have the weapon. Called by Cmd_InvUse_f() and other functions which do check the
// availability first anyhow.
// ************************************************************************************************

void
Weapon_Ready(gclient_t *client, gitem_t *weapon)
{
	playerinfo_t *playerinfo;

	playerinfo = &client->playerinfo;

	assert(weapon);

	// See if we're already using the weapon.
	if (weapon == playerinfo->pers.weapon)
		return;

	//Make sure we have an arm to do it
	if (!playerExport->BranchCheckDismemberAction(playerinfo, weapon->tag))
		return;

	// Change to this weapon and set the weapon owner's ammo_index to reflect this.
	client->pers.lastweapon = playerinfo->pers.weapon;
	playerinfo->pers.weapon = weapon;

	if (playerinfo->pers.weapon && playerinfo->pers.weapon->ammo)
		playerinfo->weap_ammo_index = ITEM_INDEX(FindItem(playerinfo->pers.weapon->ammo));
	else
		playerinfo->weap_ammo_index = 0;
}

// ************************************************************************************************
// Weapon_EquipSwordStaff
// ----------------------
// ************************************************************************************************

void Weapon_EquipSwordStaff(struct edict_s *ent, gitem_t *Weapon)
{
	playerinfo_t *playerinfo = &ent->client->playerinfo;
	assert(playerinfo);

	// See if we're already using the sword-staff.
	if (Weapon == playerinfo->pers.weapon)
		return;

	// See if we're already switching...
	if (ent->client->newweapon != NULL)
		return;

	//Make sure we have an arm to do it
	if (!playerExport->BranchCheckDismemberAction(playerinfo, Weapon->tag))
		return;

	if (playerinfo->pm_w_flags & WF_SURFACE || playerinfo->waterlevel >= 2)
		return;

	ent->client->newweapon = Weapon;
	playerinfo->switchtoweapon = WEAPON_READY_SWORDSTAFF;
}

// ***********************************************************************************************
// Weapon_EquipSpell
// -----------------
// ************************************************************************************************

void
Weapon_EquipSpell(struct edict_s *ent, gitem_t *Weapon)
{
	playerinfo_t *playerinfo = &ent->client->playerinfo;
	assert(playerinfo);

	// See if we're already using this particular spell.
	if (Weapon == playerinfo->pers.weapon)
		return;

	// See if we're already switching...
	if (ent->client->newweapon != NULL)
	{
		if (playerinfo->switchtoweapon != WEAPON_READY_HANDS)
			return;
	}

	// Make sure we have an arm to do it.

	if (!playerExport->BranchCheckDismemberAction(playerinfo, Weapon->tag))
		return;

	// In blade only DM, don't put away the staff and change weapons.

	if (playerinfo->dmflags&DF_NO_OFFENSIVE_SPELL)
		if (playerinfo->pm_w_flags & WF_SURFACE || playerinfo->waterlevel >= 2)
			return;

	// if its anything other than the flying fist, see if we have mana for it.
	if (Weapon->tag != ITEM_WEAPON_FLYINGFIST)
	{
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem(Weapon->ammo))] < Weapon->quantity)
		{
			G_CPrintf(ent, PRINT_HIGH, GM_NOMANA);
			return;
		}
	}

	ent->client->newweapon = Weapon;
	playerinfo->switchtoweapon = WEAPON_READY_HANDS;
}

// ************************************************************************************************
// Weapon_EquipHellStaff
// ---------------------
// ************************************************************************************************

void Weapon_EquipHellStaff(struct edict_s *ent, gitem_t *Weapon)
{
	playerinfo_t *playerinfo = &ent->client->playerinfo;
	gitem_t	*AmmoItem;
	int		AmmoIndex;

	assert(playerinfo);

	// See if we're already using the hell-staff.
	if (Weapon == playerinfo->pers.weapon)
		return;

	// See if we're already switching...
	if (ent->client->newweapon != NULL)
		return;

	//Make sure we have an arm to do it
	if (!playerExport->BranchCheckDismemberAction(playerinfo, Weapon->tag))
	{
		return;
	}

	if (playerinfo->pm_w_flags & WF_SURFACE || playerinfo->waterlevel >= 2)
	{
		return;
	}

	// see if we actually have any ammo for it
	AmmoItem = FindItem(Weapon->ammo);
	AmmoIndex = ITEM_INDEX(AmmoItem);

    if (!ent->client->pers.inventory[AmmoIndex])
	{
		G_CPrintf(playerinfo->self, PRINT_HIGH, GM_NOAMMO);
		return;
	}

	ent->client->newweapon = Weapon;
	playerinfo->switchtoweapon = WEAPON_READY_HELLSTAFF;
}

// ************************************************************************************************
// Weapon_EquipBow
// ---------------
// ************************************************************************************************

void Weapon_EquipBow(struct edict_s *ent, gitem_t *Weapon)
{
	playerinfo_t *playerinfo = &ent->client->playerinfo;
	gitem_t	*AmmoItem;
	int		AmmoIndex;

	assert(playerinfo);

	// See if we're already using the bow.
	if (Weapon == playerinfo->pers.weapon)
		return;

	// See if we're already switching...
	if (ent->client->newweapon != NULL)
	{
		if (playerinfo->switchtoweapon != WEAPON_READY_BOW)
			return;
	}

	//Make sure we have an arm to do it
	if (!playerExport->BranchCheckDismemberAction(playerinfo, Weapon->tag))
		return;

	if (playerinfo->pm_w_flags & WF_SURFACE || playerinfo->waterlevel >= 2)
		return;

	// see if we actually have any ammo for it
	AmmoItem = FindItem(Weapon->ammo);
	AmmoIndex = ITEM_INDEX(AmmoItem);

    if (!ent->client->pers.inventory[AmmoIndex])
	{
		G_CPrintf(playerinfo->self, PRINT_HIGH, GM_NOAMMO);
		return;
	}

	ent->client->newweapon = Weapon;
	playerinfo->switchtoweapon = WEAPON_READY_BOW;
}

// ************************************************************************************************
// Weapon_CurrentShotsLeft
// -----------------------
// Returns the number of shots that a weapon owner could make with the currently selected weapon,
// in respect to the amount of ammo for that weapon that the player has in their inventory.
// ************************************************************************************************

int Weapon_CurrentShotsLeft(playerinfo_t *playerinfo)
{
	gitem_t	*Weapon, *AmmoItem;
	int		AmmoIndex;
	gclient_t *client;

	client = playerinfo->self->client;
	Weapon = playerinfo->pers.weapon;

	// If the weapon uses ammo, return the number of shots left, else return -1 (e.g. Sword-staff).

	if (Weapon->ammo && (Weapon->quantity))
	{
		AmmoItem = FindItem(Weapon->ammo);
		AmmoIndex = ITEM_INDEX(AmmoItem);

		if (playerinfo->pers.weapon->tag == ITEM_WEAPON_MACEBALLS &&
			playerinfo->powerup_timer > playerinfo->leveltime)
			return(client->pers.inventory[AmmoIndex]/(Weapon->quantity*2.0));		// Double consumption for mace.
		else
			return(client->pers.inventory[AmmoIndex]/Weapon->quantity);
	}
	else
		return(0);
}

// ************************************************************************************************
// Defence_CurrentShotsLeft
// -----------------------
// Returns the number of shots that a weapon owner could make with the currently selected weapon,
// in respect to the amount of ammo for that weapon that the player has in their inventory.
// ************************************************************************************************

int Defence_CurrentShotsLeft(playerinfo_t *playerinfo, int intent)
{
	gitem_t	*Defence,
			*ManaItem;
	int		ManaIndex;
	gclient_t *client;

	client = playerinfo->self->client;
	Defence = client->pers.defence;

	// If the weapon uses ammo, return the number of shots left, else return -1 (e.g. Sword-staff).

	if (Defence->ammo && Defence->quantity)
	{
		ManaItem = FindItem(Defence->ammo);
		ManaIndex = ITEM_INDEX(ManaItem);

		return(client->pers.inventory[ManaIndex] / Defence->quantity);
	}
	else
		return(0);
}
