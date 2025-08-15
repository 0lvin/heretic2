//
// g_weapon.h
//
// Heretic II
// Copyright 1998 Raven Software
//

#ifndef P_WEAPON_H
#define P_WEAPON_H

// ************************************************************************************************
// WeaponThink_XXX
// ---------------
// Player-weapon think functions, each corresponding to one weapon type. Made visible here so that
// they can be referenced from 'g_items.c'.
// ************************************************************************************************

void WeaponThink_SwordStaff(edict_t *Caster);
void WeaponThink_SwordStaffEx(edict_t *Caster,char *Format,...);
void WeaponThink_FlyingFist(edict_t *Caster);
void WeaponThink_MagicMissileSpread(edict_t *Caster);
void WeaponThink_MagicMissileSpreadEx(edict_t *Caster,char *Format,...);
void WeaponThink_SphereOfAnnihilation(edict_t *Caster);
void WeaponThink_SphereOfAnnihilationEx(edict_t *Caster, char *Format,...);
void WeaponThink_Maceballs(edict_t *Caster);
void WeaponThink_Firewall(edict_t *Caster);
void WeaponThink_Blast(edict_t *Caster);
void WeaponThink_RedRainBow(edict_t *Caster);
void WeaponThink_PhoenixBow(edict_t *Caster);
void WeaponThink_HellStaff(edict_t *Caster);
extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);

#endif // P_WEAPON_H
