//
// p_dll.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include <stdio.h>
#include <stdlib.h>

#include "../header/local.h"
#include "../../common/header/shared.h"

// Structure containing functions and data pointers exported from the player library.


player_import_t	playerImport;

extern void Player_InitAPI(player_import_t *import);

// ** setup a looping sound on the client
static void
G_set_looping_sound(edict_t *self, int sound_num)
{
	self->s.sound = sound_num;
	self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;
}

// ************************************************************************************************
// P_Load
// ------
// ************************************************************************************************

void
P_Load(void)
{
	Com_Printf("Initializing static player library\n");

	playerImport.dprintf = gi.dprintf;
	playerImport.Weapon_EquipSpell = Weapon_EquipSpell;
	playerImport.Weapon_Ready = Weapon_Ready;
	playerImport.Weapon_CurrentShotsLeft = Weapon_CurrentShotsLeft;
	playerImport.Defence_CurrentShotsLeft = Defence_CurrentShotsLeft;
	playerImport.FindItem = FindItem;

	// Server (game) function callbacks (approximating functionality of client-side function callbacks).
	playerImport.G_L_Sound = G_set_looping_sound;
	playerImport.G_Sound = gi.sound;
	playerImport.G_Trace = gi.trace;
	playerImport.G_CreateEffect = gi.CreateEffect;
	playerImport.G_RemoveEffects = G_RemoveEffects;

	// Server (game) function callbacks that have no client side equivalent.
	playerImport.G_SoundIndex = gi.soundindex;
	playerImport.G_UseTargets = G_UseTargets;
	playerImport.G_GetEntityStatePtr = G_GetEntityStatePtr;
	playerImport.G_BranchLwrClimbing = G_BranchLwrClimbing;
	playerImport.G_PlayerActionCheckRopeGrab = G_PlayerActionCheckRopeGrab;
	playerImport.G_PlayerClimbingMoveFunc = G_PlayerClimbingMoveFunc;
	playerImport.G_PlayerActionUsePuzzle = G_PlayerActionUsePuzzle;
	playerImport.G_PlayerActionCheckPuzzleGrab = G_PlayerActionCheckPuzzleGrab;
	playerImport.G_PlayerActionTakePuzzle = G_PlayerActionTakePuzzle;
	playerImport.G_PlayerActionCheckPushPull_Ent = G_PlayerActionCheckPushPull_Ent;
	playerImport.G_PlayerActionMoveItem = G_PlayerActionMoveItem;
	playerImport.G_PlayerActionCheckPushButton = G_PlayerActionCheckPushButton;
	playerImport.G_PlayerActionPushButton = G_PlayerActionPushButton;
	playerImport.G_PlayerActionCheckPushLever = G_PlayerActionCheckPushLever;
	playerImport.G_PlayerActionPushLever = G_PlayerActionPushLever;
	playerImport.G_HandleTeleport = G_HandleTeleport;
	playerImport.G_PlayerActionShrineEffect = G_PlayerActionShrineEffect;
	playerImport.G_PlayerActionChickenBite = G_PlayerActionChickenBite;
	playerImport.G_PlayerFallingDamage = G_PlayerFallingDamage;
	playerImport.G_PlayerSpellShieldAttack = G_PlayerSpellShieldAttack;
	playerImport.G_PlayerSpellStopShieldAttack = G_PlayerSpellStopShieldAttack;
	playerImport.G_PlayerVaultKick = G_PlayerVaultKick;
	playerImport.G_PlayerActionCheckRopeMove = G_PlayerActionCheckRopeMove;
	playerImport.G_cprintf = G_CPrintf;
	playerImport.G_WeapNext = Cmd_WeapPrev_f;
	playerImport.G_UseItem = Cmd_Use_f;

	// Common client & server (game) function callbacks.

	playerImport.PointContents = gi.pointcontents;
	playerImport.SetJointAngles = G_SetJointAngles;
	playerImport.ResetJointAngles = G_ResetJointAngles;
	playerImport.PlayerActionSwordAttack = G_PlayerActionSwordAttack;
	playerImport.PlayerActionSpellFireball = WeaponThink_FlyingFist;
	playerImport.PlayerActionSpellBlast = WeaponThink_Blast;
	playerImport.PlayerActionSpellArray = G_PlayerActionSpellArray;
	playerImport.PlayerActionSpellSphereCreate = G_PlayerActionSpellSphereCreate;
	playerImport.PlayerActionSpellBigBall = WeaponThink_Maceballs;
	playerImport.PlayerActionSpellFirewall = WeaponThink_Firewall;
	playerImport.PlayerActionRedRainBowAttack = WeaponThink_RedRainBow;
	playerImport.PlayerActionPhoenixBowAttack = WeaponThink_PhoenixBow;
	playerImport.PlayerActionHellstaffAttack = WeaponThink_HellStaff;
	playerImport.PlayerActionSpellDefensive = G_PlayerActionSpellDefensive;
	playerImport.G_EntIsAButton = G_EntIsAButton;


	Player_InitAPI(&playerImport);

	Com_Printf("------------------------------------\n");
}
