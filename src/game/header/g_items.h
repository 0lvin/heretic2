//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef G_ITEMS_H
#define G_ITEMS_H

// ************************************************************************************************
// itemhealth_t
// ------
// ************************************************************************************************
typedef enum
{
	ITEM_HEALTH1,
	ITEM_HEALTH2
} itemhealth_t;

// ************************************************************************************************
// itemweapon_t
// ------
// ************************************************************************************************
typedef enum
{
	ITEM_WEAPON_SWORDSTAFF,
	ITEM_WEAPON_FLYINGFIST,
	ITEM_WEAPON_HELLSTAFF,
	ITEM_WEAPON_MAGICMISSILE,
	ITEM_WEAPON_REDRAINBOW,
	ITEM_WEAPON_SPHEREOFANNIHILATION,
	ITEM_WEAPON_PHOENIXBOW,
	ITEM_WEAPON_MACEBALLS,
	ITEM_WEAPON_FIREWALL,
} itemweapon_t;

// ************************************************************************************************
// itemdefense_t
// ------
// ************************************************************************************************
typedef enum
{
	ITEM_DEFENSE_REPULSION,
	ITEM_DEFENSE_METEORBARRIER,
	ITEM_DEFENSE_POLYMORPH,
	ITEM_DEFENSE_TELEPORT,
	ITEM_DEFENSE_SHIELD,
	ITEM_DEFENSE_TORNADO,
	ITEM_DEFENSE_POWERUP,
} itemdefense_t;

#endif
