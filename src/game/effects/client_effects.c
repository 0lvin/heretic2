//
// Heretic II
// Copyright 1998 Raven Software
//
#include "../../common/header/common.h"
#include "client_effects.h"
#include "client_entities.h"
#include "../common/fx.h"

// NB. The assassin tport go is not precached

ClientEffect_t clientEffectSpawners[NUM_FX] =
{
	// ***NOTE*** We currently have 113 client effects, and we don't want to exceed 32768!  Ha!

	{ RemoveEffects,					NULL,					"s"			}, // FX_REMOVE_EFFECTS
	{ GenericExplosion1,				NULL,					NULL		}, // FX_EXPLOSION1
	{ GenericExplosion2,				NULL,					NULL		}, // FX_EXPLOSION2
	{ WaterSplash,						PreCacheWaterSplash,	"b"			}, // FX_SPLASH
	{ GenericGibTrail,					NULL,					NULL		}, // FX_GIB_TRAIL
	{ FXBlood,							PreCacheSplat,			"ub"		}, // FX_BLOOD
	{ FXBloodTrail,						PreCacheSplat,			"d"			}, // FX_BLOOD_TRAIL
	{ FXLinkedBlood,					NULL,					"bb"		}, // FX_LINKEDBLOOD
	{ FXGenericSparks,					PreCacheSparks,			"d"			}, // FX_SPARKS
	{ PlayerTeleportin,					PreCacheTeleport,		NULL		}, // FX_PLAYER_TELEPORT_IN
	{ FXHealthPickup,					PreCacheHealth,			NULL		}, // FX_PICKUP_HEALTH
	{ FXWeaponPickup,					PreCacheItemWeapons,	"b"			}, // FX_PICKUP_WEAPON
	{ FXDefensePickup,					PreCacheItemDefense,	"b"			}, // FX_PICKUP_DEFENSE
	{ FXPuzzlePickup,					PreCachePuzzleItems,	"bv"		}, // FX_PICKUP_PUZZLE
	{ FXAmmoPickup,						PreCacheItemAmmo,		"b"			}, // FX_PICKUP_AMMO
	{ FXFlyingFist,						PreCacheFist,			"t"			}, // FX_WEAPON_FLYINGFIST
	{ FXFlyingFistExplode,				NULL,					"d"			}, // FX_WEAPON_FLYINGFISTEXPLODE
	{ FXBlueRing,						PreCacheBluering,		NULL		}, // FX_SPELL_BLUERING
	{ FXMeteorBarrier,					PreCacheMeteor,			NULL		}, // FX_SPELL_METEORBARRIER, see fx.h for an explanation of _this
	{ FXMeteorBarrier,					PreCacheMeteor,			NULL		}, // FX_SPELL_METEORBARRIER1
	{ FXMeteorBarrier,					PreCacheMeteor,			NULL		}, // FX_SPELL_METEORBARRIER2
	{ FXMeteorBarrier,					PreCacheMeteor,			NULL		}, // FX_SPELL_METEORBARRIER3
	{ FXMeteorBarrierTravel,			PreCacheMeteor,			NULL		}, // FX_SPELL_METEORBARRIER_TRAVEL
	{ FXMeteorBarrierExplode,			NULL,					"d"			}, // FX_SPELL_METEORBARRIEREXPLODE
	{ FXLightningShield,				PreCacheShield,			NULL		}, // FX_SPELL_LIGHTNINGSHIELD
	{ FXSphereOfAnnihilation,			PreCacheSphere,			"s"			}, // FX_WEAPON_SPHERE
	{ FXSphereOfAnnihilationGlowballs,	NULL,					"s"			}, // FX_WEAPON_SPHEREGLOWBALLS
	{ FXSphereOfAnnihilationExplode,	NULL,					"db"		}, // FX_WEAPON_SPHEREEXPLODE
	{ FXSphereOfAnnihilationPower,		NULL,					"xbb"		}, // FX_WEAPON_SPHEREPOWER
	{ FXSpherePlayerExplode,			NULL,					"db"		}, // FX_WEAPON_SPHEREPLAYEREXPLODE
	{ FXMagicMissile,					PreCacheArray,			"ss"		}, // FX_WEAPON_MAGICMISSILE
	{ FXMagicMissileExplode,			NULL,					"d"			}, // FX_WEAPON_MAGICMISSILEEXPLODE
	{ FXBlast,							NULL,					"sssssss"	}, // FX_WEAPON_BLAST
	{ FXRedRainMissile,					PreCacheRedrain,		"t"			}, // FX_WEAPON_REDRAINMISSILE
	{ FXRedRain,						NULL,					NULL		}, // FX_WEAPON_REDRAIN
	{ FXRedRainGlow,					NULL,					"b"			}, // FX_WEAPON_REDRAINGLOW
	{ FXMaceball,						PreCacheMaceball,		NULL		}, // FX_WEAPON_MACEBALL
	{ FXMaceballBounce,					NULL,					"d"			}, // FX_WEAPON_MACEBALLBOUNCE
	{ FXMaceballExplode,				NULL,					"d"			}, // FX_WEAPON_MACEBALLEXPLODE
	{ FXPhoenixMissile,					PreCachePhoenix,		"t"			}, // FX_WEAPON_PHOENIXMISSILE
	{ FXPhoenixExplode,					NULL,					"td"		}, // FX_WEAPON_PHOENIXEXPLODE
	{ FXMorphMissile,					PreCacheMorph,			"bb"		}, // FX_SPELL_MORPHMISSILE
	{ FXMorphMissile_initial,			NULL,					"bssssss"	}, // FX_SPELL_MORPHMISSILE_INITIAL
	{ FXMorphExplode,					NULL,					"d"			}, // FX_SPELL_MORPHEXPLODE
	{ FXFireWave,						PreCacheWall,			"ss"		}, // FX_WEAPON_FIREWAVE
	{ FXFireWaveWorm,					PreCacheWall,			"t"			}, // FX_WEAPON_FIREWAVEWORM
	{ FXFireBurst,						NULL,					"ss"		}, // FX_WEAPON_FIREBURST
	{ FXRipperExplode,					NULL,					"vbssssssss"}, // FX_WEAPON_RIPPEREXPLODE
	{ FXWaterEntrySplash,				NULL,					"bd"		}, // FX_WATER_ENTRYSPLASH
	{ FXWaterRipples,					PreCacheRipples,		NULL		}, // FX_WATER_RIPPLES
	{ FXWaterWake,						PreCacheWake,			"sbv"		}, // FX_WATER_WAKE
	{ FXBubbler,						PreCacheBubbler,		"b"			}, // FX_BUBBLER
	{ FXScorchmark,						PreCacheScorch,			"d"			}, // FX_SCORCHMARK
	{ FXDebris,							PreCacheDebris,			"bbdb"		}, // FX_DEBRIS
	{ FXFleshDebris,					PreCacheDebris,			"bdb"		}, // FX_FLESH_DEBRIS
	{ FXShadow,							PrecacheShadow,			"f"			}, // FX_SHADOW
	{ FXFountain,						NULL,					"vsb"		}, // FX_FOUNTAIN
	{ FXWaterfallBase,					NULL,					"bbb"		}, // FX_WATERFALLBASE
	{ FXDripper,						PreCacheDripper,		"bb"		}, // FX_DRIPPER
	{ FXMist,							PreCacheMist,			"b"			}, // FX_MIST
	{ FXPlagueMist,						NULL,					"vb"		}, // FX_PLAGUEMIST
	{ FXPlagueMistExplode,				NULL,					"b"			}, // FX_PLAGUEMISTEXPLODE
	{ FXSpellHands,						PreCacheHands,			"b"			}, // FX_SPELLHANDS
	{ FXLensFlare,						PreCacheFlare,			"bbbf"		}, // FX_LENSFLARE
	{ FXStaff,							PreCacheStaff,			"bb"		}, // FX_STAFF
	{ FXSpoo,							PreCacheSpoo,			NULL		}, // FX_SPOO
	{ FXHalo,							PreCacheHalos,			NULL		}, // FX_HALO
	{ FXRemoteCamera,					NULL,					"s"			}, // FX_REMOTE_CAMERA
	{ FXHellbolt,						PreCacheHellstaff,		"t"			}, // FX_WEAPON_HELLBOLT
	{ FXHellboltExplode,				NULL,					"d"			}, // FX_WEAPON_HELLBOLTEXPLODE
	{ FXHellstaffPower,					PreCacheHellstaff,		"tb"		}, // FX_WEAPON_HELLSTAFF_POWER
	{ FXHellstaffPowerBurn,				NULL,					"t"			}, // FX_WEAPON_HELLSTAFF_POWER_BURN
	{ FXSpellChange,					NULL,					"db"		}, // FX_SPELL_CHANGE
	{ FXStaffCreate,					NULL,					NULL		}, // FX_STAFF_CREATE
	{ FXStaffCreatePoof,				NULL,					NULL		}, // FX_STAFF_CREATEPOOF
	{ FXStaffRemove,					NULL,					NULL		}, // FX_STAFF_REMOVE
	{ FXDustPuffOnGround,				NULL,					NULL		}, // FX_DUST_PUFF
	{ FXFire,							NULL,					"b"			}, // FX_FIRE
	{ FXSound,							NULL,					"bbbb"		}, // FX_SOUND
	{ FXPickup,							PreCachePickup,			NULL		}, // FX_PICKUP
	{ FXGenericHitPuff,					NULL,					"db"		}, // FX_HITPUFF
	{ FXDust,							PreCacheRockchunks,		"bdb"		}, // FX_DUST
	{ FXEnvSmoke,						PreCacheSmoke,			"bdbbb"		}, // FX_ENVSMOKE
	{ FXSpooSplat,						NULL,					"d"			}, // FX_SPOO_SPLAT
	{ FXBodyPart,						NULL,					"ssbbb"		}, // FX_BODYPART
	{ PlayerTeleportout,				NULL,					NULL		}, // FX_PLAYER_TELEPORT_OUT
	{ FXPlayerPersistant,				NULL,					NULL		}, // FX_PLAYER_PERSISTANT
	{ FXplayertorch,					PreCacheTorch,			NULL		}, // FX_PLAYER_TORCH
	{ FXTomeOfPower,					NULL,					NULL		}, // FX_TOME_OF_POWER
	{ FXFireOnEntity,					NULL,					"bbb"		}, // FX_FIRE_ON_ENTITY
	{ FXFlareup,						PreCacheFlareup,		NULL		}, // FX_FLAREUP
	{ FXShrinePlayerEffect,				PreCacheShrine,			"b"			}, // FX_SHRINE_PLAYER
	{ FXShrineManaEffect,				NULL,					NULL		}, // FX_SHRINE_MANA
	{ FXShrineLungsEffect,				NULL,					NULL		}, // FX_SHRINE_LUNGS
	{ FXShrineLightEffect,				NULL,					NULL		}, // FX_SHRINE_LIGHT
	{ FXShrineReflectEffect,			NULL,					NULL		}, // FX_SHRINE_REFLECT
	{ FXShrineArmorEffect,				NULL,					NULL		}, // FX_SHRINE_ARMOR
	{ FXShrineHealthEffect,				NULL,					NULL		}, // FX_SHRINE_HEALTH
	{ FXShrineStaffEffect,				NULL,					NULL		}, // FX_SHRINE_STAFF
	{ FXShrineGhostEffect,				NULL,					NULL		}, // FX_SHRINE_GHOST
	{ FXShrineSpeedEffect,				NULL,					NULL		}, // FX_SHRINE_SPEED
	{ FXShrinePowerUpEffect,			NULL,					NULL		}, // FX_SHRINE_POWERUP
	{ FXRope,							PreCacheRope,			"ssbvvv"	}, // FX_ROPE
	{ FXFireHands,						NULL,					"b"			}, // FX_FIREHANDS
	{ FXShrineBall,						NULL,					"db"		}, // FX_SHRINE_BALL
	{ FXShrineBallExplode,				NULL,					"db"		}, // FX_SHRINE_BALL_EXPLODE
	{ FXOgleHitPuff,					PrecacheOgleHitPuff,	"v"			}, // FX_OGLE_HITPUFF
	{ FXHPMissile,						PreCacheHPMissile,		"vb"		}, // FX_HP_MISSILE
	{ FXIEffects,						PreCacheIEffects,		"bv"		}, // FX_I_EFFECTS
	{ FXChickenExplode,					NULL,					NULL		}, // FX_CHICKEN_EXPLODE
	{ FXFlamethrower,					NULL,					"df"		}, // FX_FLAMETHROWER
	{ FXTeleportPad,					NULL,					NULL		}, // FX_TELEPORT_PAD, 110 fx to here
	{ FXQuake,							NULL,					"bbb"		}, // FX_QUAKE
	{ FXLightning,						PreCacheLightning,		"vbb"		}, // FX_LIGHTNING
	{ FXPowerLightning,					PreCacheLightning,		"vb"		}, // FX_POWER_LIGHTNING
	{ FXBubble,							PreCacheBubbler,		NULL		}, // FX_BUBBLE
	{ FXTPortSmoke,						PreCacheTPortSmoke,		NULL		}, // FX_TPORTSMOKE
	{ FXWaterParticles,					PreCacheWaterParticles,	NULL		}, // FX_WATER_PARTICLES
	{ FXMEffects,						PreCacheMEffects,		"bv"		}, // FX_M_EFFECTS - all of Morcalavin's effects
	{ FXHPStaff,						PreCacheHPStaff,		"bs"		}, // FX_HP_STAFF - staff effects for the high priestess
	{ FXRandWaterBubble,				NULL,					NULL		}, // FX_WATER_BUBBLE
	{ FXMagicPortal,					PreCachePortal,			"vbb"		}, // FX_MAGIC_PORTAL
	{ FXTBEffects,						PreCacheTB,				"bv"		}, // FX_TB_EFFECTS
	{ FXBodyPart,						NULL,					"ssbbb"		}, // FX_THROWWEAPON - uses body part, which just detects type for certain things
	{ FXSsithraArrow,					PrecacheSsithraArrow,	"bv"		}, // FX_SSITHRA_ARROW
	{ FXPESpell,						PrecachePESpell,		"bv"		}, // FX_PE_SPELL
	{ FXLightningHit,					PreCacheHitPuff,		"t"			}, // FX_LIGHTNING_HIT
	{ FXStaffStrike,					PreCacheStaffHit,		"db"		}, // FX_WEAPON_STAFF_STRIKE
	{ FXCreateArmorHit,					PreCacheArmorHit,		"d"			}, // FX_ARMOR_HIT
	{ FXBarrelExplode,					PreCacheObjects,		NULL		}, // FX_BARREL_EXPLODE
	{ FXCWatcherEffects,				PreCacheCWModels,		"bv"		}, // FX_CWATCHER
	{ FXCorpseRemove,					PreCacheCrosshair,		NULL		}, // FX_CORPSE_REMOVE, naughty little hack here, crosshair has nothing to do with corpse removal
	{ FXLeader,							NULL,					NULL		}, // FX_SHOW_LEADER
	{ FXTornado,						PreCacheTorn,			NULL		}, // FX_TORNADO
	{ FXTornadoBall,					NULL,					NULL		}, // FX_TORNADO_BALL
	{ FXTornadoBallExplode,				NULL,					NULL		}, // FX_TORNADO_BALL_EXPLODE
	{ FXFeetTrail,						NULL,					NULL		}, // FX_FOOT_TRAIL
	{ FXGenericSparks,					PreCacheSparks,			"d"			}, // FX_BLOCK_SPARKS
	{ NULL,						 		NULL,					NULL		}, // FX_CROSSHAIR
};

CE_ClassStatics_t ce_classStatics[CE_NUM_CLASSIDS];

void (*cg_classStaticsInits[CE_NUM_CLASSIDS])()=
{
	InitDebrisStatics
};

void RemoveEffects(centity_t *owner, int type, int flags, vec3_t origin)
{
	FX_Type_t	fx;

	assert(owner);
//	assert(owner->effects);		// FIXME:  This assert fires, but it should not.  We shouldn't be here anyway.

	FXGetEffect(owner, flags, clientEffectSpawners[FX_REMOVE_EFFECTS].formatString, &fx);
	RemoveEffectTypeList(&owner->effects, fx, owner);
}

// ***************************************************************************************
// Client Models
// ***************************************************************************************

void RegisterModels()
{
	int		i;

	for(i = 0; i < NUM_FX; i++)
	{
		if (clientEffectSpawners[i].PrecacheCFX)
		{
			clientEffectSpawners[i].PrecacheCFX();
		}
	}
}
