cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(Heretic2 C CXX)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)

################################################################################
# Global compiler options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_C_FLAGS "")
    set(CMAKE_C_FLAGS_DEBUG "")
    set(CMAKE_C_FLAGS_RELEASE "")
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
endif()

################################################################################
# Global linker options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
endif()

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################

set(PROJECT_NAME Heretic2)

################################################################################
# Source groups
################################################################################
set(Client_Effects
    "client_effects/Ambient_Effects.c"
    "client_effects/ambient_effects.h"
    "client_effects/ce_DefaultMessageHandler.c"
    "client_effects/ce_DefaultMessageHandler.h"
    "client_effects/ce_DLight.c"
    "client_effects/ce_DLight.h"
    "client_effects/ce_Message.c"
    "client_effects/ce_Message.h"
    "client_effects/Client_Effects.c"
    "client_effects/Client_Effects.h"
    "client_effects/Client_Entities.c"
    "client_effects/Client_Entities.h"
    "client_effects/Font1.c"
    "client_effects/Font2.c"
    "client_effects/fx_AmmoPickup.c"
    "client_effects/fx_animate.c"
    "client_effects/fx_assassin.c"
    "client_effects/fx_blood.c"
    "client_effects/fx_BlueRing.c"
    "client_effects/fx_bubbler.c"
    "client_effects/fx_crosshair.c"
    "client_effects/fx_cwatcher.c"
    "client_effects/fx_debris.c"
    "client_effects/fx_debris.h"
    "client_effects/fx_DefensePickup.c"
    "client_effects/fx_Dripper.c"
    "client_effects/fx_dust.c"
    "client_effects/fx_dustpuff.c"
    "client_effects/fx_dustpuff.h"
    "client_effects/fx_fire.c"
    "client_effects/fx_firehands.c"
    "client_effects/fx_flamethrow.c"
    "client_effects/fx_flyingfist.c"
    "client_effects/fx_Fountain.c"
    "client_effects/fx_halo.c"
    "client_effects/fx_HealthPickup.c"
    "client_effects/fx_HellStaff.c"
    "client_effects/fx_hitpuff.c"
    "client_effects/fx_hpproj.c"
    "client_effects/fx_insectstaff.c"
    "client_effects/fx_lensflare.c"
    "client_effects/fx_lightning.c"
    "client_effects/fx_Maceball.c"
    "client_effects/fx_magicmissile.c"
    "client_effects/fx_meteorbarrier.c"
    "client_effects/fx_mist.c"
    "client_effects/fx_mork.c"
    "client_effects/fx_Morph.c"
    "client_effects/fx_objects.c"
    "client_effects/fx_pespell.c"
    "client_effects/fx_Phoenix.c"
    "client_effects/fx_pickup.c"
    "client_effects/fx_pickuppuzzle.c"
    "client_effects/fx_PlagueMist.c"
    "client_effects/fx_PlagueMistExplode.c"
    "client_effects/fx_portal.c"
    "client_effects/fx_quake.c"
    "client_effects/fx_RedRain.c"
    "client_effects/fx_redrainglow.c"
    "client_effects/fx_remotecamera.c"
    "client_effects/fx_Ripples.c"
    "client_effects/fx_rope.c"
    "client_effects/fx_scorchmark.c"
    "client_effects/fx_shadow.c"
    "client_effects/fx_Shield.c"
    "client_effects/fx_shrine.c"
    "client_effects/fx_smoke.c"
    "client_effects/fx_sound.c"
    "client_effects/fx_sparks.c"
    "client_effects/fx_spellchange.c"
    "client_effects/fx_spellhands.c"
    "client_effects/fx_sphereofannihlation.c"
    "client_effects/fx_spoo.c"
    "client_effects/fx_ssarrow.c"
    "client_effects/fx_ssithra.c"
    "client_effects/fx_staff.c"
    "client_effects/fx_tbeast.c"
    "client_effects/fx_teleport.c"
    "client_effects/fx_tome.c"
    "client_effects/fx_tornado.c"
    "client_effects/fx_Wall.c"
    "client_effects/fx_waterentrysplash.c"
    "client_effects/fx_waterwake.c"
    "client_effects/fx_WeaponPickup.c"
    "client_effects/Generic_Character_Effects.c"
    "client_effects/Generic_Weapon_Effects.c"
    "client_effects/Item_Effects.c"
    "client_effects/LevelMaps.c"
    "client_effects/LightStyles.c"
    "client_effects/Main.c"
    "client_effects/Monster_Weapon_Effects.c"
    "client_effects/Particle.c"
    "client_effects/Particle.h"
    "client_effects/Player_Effects.c"
    "client_effects/TestEffect.c"
    "client_effects/Utilities.c"
    "client_effects/Utilities.h"
)
source_group("Client Effects" FILES ${Client_Effects})

set(Game
    "game/buoy.c"
    "game/buoy.h"
    "game/c_ai.c"
    "game/c_ai.h"
    "game/c_corvus1.c"
    "game/c_corvus1.h"
    "game/c_corvus1_anim.c"
    "game/c_corvus1_anim.h"
    "game/c_corvus2.c"
    "game/c_corvus2.h"
    "game/c_corvus2_anim.c"
    "game/c_corvus2_anim.h"
    "game/c_corvus3.c"
    "game/c_corvus3.h"
    "game/c_corvus3_anim.c"
    "game/c_corvus3_anim.h"
    "game/c_corvus4.c"
    "game/c_corvus4.h"
    "game/c_corvus4_anim.c"
    "game/c_corvus4_anim.h"
    "game/c_corvus5.c"
    "game/c_corvus5.h"
    "game/c_corvus5_anim.c"
    "game/c_corvus5_anim.h"
    "game/c_corvus6.c"
    "game/c_corvus6.h"
    "game/c_corvus6_anim.c"
    "game/c_corvus6_anim.h"
    "game/c_corvus7.c"
    "game/c_corvus7.h"
    "game/c_corvus7_anim.c"
    "game/c_corvus7_anim.h"
    "game/c_corvus8.c"
    "game/c_corvus8.h"
    "game/c_corvus8_anim.c"
    "game/c_corvus8_anim.h"
    "game/c_corvus9.c"
    "game/c_corvus9.h"
    "game/c_corvus9_anim.c"
    "game/c_corvus9_anim.h"
    "game/c_dranor.c"
    "game/c_Dranor.h"
    "game/c_dranor_anim.c"
    "game/c_dranor_anim.h"
    "game/c_elflord.c"
    "game/c_elflord.h"
    "game/c_elflord_anim.c"
    "game/c_elflord_anim.h"
    "game/c_morcalavin.c"
    "game/c_morcalavin.h"
    "game/c_morcalavin_anim.c"
    "game/c_morcalavin_anim.h"
    "game/c_priestess.c"
    "game/c_priestess.h"
    "game/c_priestess2.c"
    "game/c_priestess2.h"
    "game/c_priestess2_anim.c"
    "game/c_priestess2_anim.h"
    "game/c_priestess_anim.c"
    "game/c_priestess_anim.h"
    "game/c_siernan1.c"
    "game/c_siernan1.h"
    "game/c_siernan1_anim.c"
    "game/c_siernan1_anim.h"
    "game/c_siernan2.c"
    "game/c_siernan2.h"
    "game/c_siernan2_anim.c"
    "game/c_siernan2_anim.h"
    "game/c_ssithrascout.c"
    "game/c_ssithrascout.h"
    "game/c_ssithrascout_anim.c"
    "game/c_ssithrascout_anim.h"
    "game/c_tome.c"
    "game/c_tome.h"
    "game/c_tome_anim.c"
    "game/c_tome_anim.h"
    "game/c_victimSsithra.c"
    "game/c_victimSsithra.h"
    "game/c_victimSsithra_anim.c"
    "game/c_victimSsithra_anim.h"
    "game/decals.c"
    "game/decals.h"
    "game/ds.cpp"
    "game/ds.h"
    "game/g_ai.c"
    "game/g_BoundingForm.h"
    "game/g_breakable.c"
    "game/g_ClassStatics.c"
    "game/g_ClassStatics.h"
    "game/g_cmds.c"
    "game/g_combat.c"
    "game/g_DefaultMessageHandler.c"
    "game/g_DefaultMessageHandler.h"
    "game/g_Edict.h"
    "game/g_env.c"
    "game/g_field.c"
    "game/g_flamethrow.c"
    "game/g_func.c"
    "game/g_HitLocation.c"
    "game/g_HitLocation.h"
    "game/g_items.c"
    "game/g_items.h"
    "game/g_items_pickups.c"
    "game/g_itemstats.h"
    "game/g_light.c"
    "game/g_local.h"
    "game/g_main.c"
    "game/g_Message.c"
    "game/g_Message.h"
    "game/g_misc.c"
    "game/g_misc.h"
    "game/g_monster.c"
    "game/g_monster.h"
    "game/g_Monster_State.h"
    "game/g_MoveInfo.c"
    "game/g_MoveInfo.h"
    "game/g_NewSystem.h"
    "game/g_obj.c"
    "game/g_phys.c"
    "game/g_Physics.c"
    "game/g_Physics.h"
    "game/g_PhysicsInfo.c"
    "game/g_PhysicsInfo.h"
    "game/g_playstats.h"
    "game/g_ResourceManagers.c"
    "game/g_rope.c"
    "game/g_save.c"
    "game/g_shrine.c"
    "game/g_Skeleton.h"
    "game/g_Skeletons.c"
    "game/g_Skeletons.h"
    "game/g_spawn.c"
    "game/g_spawnf.c"
    "game/g_StateInfo.c"
    "game/g_StateInfo.h"
    "game/g_svcmds.c"
    "game/g_target.c"
    "game/g_teleport.h"
    "game/g_trigger.c"
    "game/g_Typedef.h"
    "game/g_utils.c"
    "game/g_volume_effect.h"
    "game/g_waterfx.c"
    "game/g_weapon.c"
    "game/g_weapon.h"
    "game/game.h"
    "game/game_utilities.c"
    "game/GameEntity.c"
    "game/GameEntity.h"
    "game/ICScript.h"
    "game/list.h"
    "game/m_actor.h"
    "game/m_assassin.c"
    "game/m_assassin.h"
    "game/m_assassin_anim.c"
    "game/m_assassin_anim.h"
    "game/m_beast.c"
    "game/m_beast.h"
    "game/m_beast_anim.c"
    "game/m_beast_anim.h"
    "game/m_bee.c"
    "game/m_chicken.c"
    "game/m_chicken.h"
    "game/m_chicken_anim.c"
    "game/m_chicken_anim.h"
    "game/m_elf.h"
    "game/m_elflord.c"
    "game/m_elflord.h"
    "game/m_elflord_anims.c"
    "game/m_elflord_anims.h"
    "game/m_fish.c"
    "game/m_fish.h"
    "game/m_fish_anim.c"
    "game/m_fish_anim.h"
    "game/m_FMtest.c"
    "game/m_gkrokon.c"
    "game/m_gkrokon.h"
    "game/m_gkrokon_anim.c"
    "game/m_gkrokon_anim.h"
    "game/m_gorgon.c"
    "game/m_gorgon.h"
    "game/m_gorgon_anim.c"
    "game/m_gorgon_anim.h"
    "game/m_harpy.c"
    "game/m_harpy.h"
    "game/m_harpy_anim.c"
    "game/m_harpy_anim.h"
    "game/m_imp.c"
    "game/m_imp.h"
    "game/m_imp_anim.c"
    "game/m_imp_anim.h"
    "game/m_morcalavin.c"
    "game/m_morcalavin.h"
    "game/m_morcalavin_anim.c"
    "game/m_morcalavin_anim.h"
    "game/m_mother.c"
    "game/m_mother.h"
    "game/m_mother_anim.c"
    "game/m_mother_anim.h"
    "game/m_move.c"
    "game/m_mssithra.c"
    "game/m_mssithra.h"
    "game/m_mssithra_anim.c"
    "game/m_mssithra_anim.h"
    "game/m_ogle.c"
    "game/m_ogle.h"
    "game/m_ogle_anim.c"
    "game/m_ogle_anim.h"
    "game/m_plagueElf.c"
    "game/m_plagueElf.h"
    "game/m_plagueElf_anim.c"
    "game/m_plagueElf_anim.h"
    "game/m_plagueSsithra.c"
    "game/m_plagueSsithra.h"
    "game/m_plagueSsithra_anim.c"
    "game/m_plagueSsithra_anim.h"
    "game/m_player.h"
    "game/m_priestess.c"
    "game/m_priestess.h"
    "game/m_priestess_anim.c"
    "game/m_priestess_anim.h"
    "game/m_rat.c"
    "game/m_rat.h"
    "game/m_rat_anim.c"
    "game/m_rat_anim.h"
    "game/m_seraph.c"
    "game/m_seraph.h"
    "game/m_seraph_anim.c"
    "game/m_seraph_anim.h"
    "game/m_seraph_guard.c"
    "game/m_seraph_guard.h"
    "game/m_seraph_guard_anim.c"
    "game/m_seraph_guard_anim.h"
    "game/m_spreader.c"
    "game/m_spreader.h"
    "game/m_spreader_anim.c"
    "game/m_spreader_anim.h"
    "game/m_spreaderbomb_anim.h"
    "game/m_spreadermist.c"
    "game/m_spreadermist.h"
    "game/m_stats.c"
    "game/m_stats.h"
    "game/m_tcheckrik.c"
    "game/m_tcheckrik.h"
    "game/m_tcheckrik_anim.c"
    "game/m_tcheckrik_anim.h"
    "game/m_tcheckrik_spells.c"
    "game/mg_ai.c"
    "game/mg_guide.c"
    "game/mg_guide.h"
    "game/p_client.c"
    "game/p_funcs.c"
    "game/p_funcs.h"
    "game/p_hud.c"
    "game/p_item.c"
    "game/p_item.h"
    "game/p_view.c"
    "game/pcode.h"
    "game/q_shared.c"
    "game/q_shared.h"
    "game/Script.c"
    "game/spl_blast.c"
    "game/spl_BlueRing.c"
    "game/spl_flyingfist.c"
    "game/spl_HellStaff.c"
    "game/spl_maceballs.c"
    "game/spl_magicmissile.c"
    "game/spl_meteorbarrier.c"
    "game/spl_morph.c"
    "game/spl_Phoenix.c"
    "game/spl_powerup.c"
    "game/spl_RedRain.c"
    "game/spl_ripper.c"
    "game/spl_shield.c"
    "game/spl_sphereofannihlation.c"
    "game/spl_teleport.c"
    "game/spl_tornado.c"
    "game/spl_wall.c"
    "game/Utilities.h"
    "qcommon/Message.c"
    "qcommon/p_dll.c"
    "qcommon/Skeletons.c"
)
source_group("Game" FILES ${Game})

set(Player
    "Player/p_actions.c"
    "Player/p_actions.h"
    "Player/p_anim_branch.c"
    "Player/p_anim_branch.h"
    "Player/p_anim_data.c"
    "Player/p_anim_data.h"
    "Player/p_animactor.c"
    "Player/p_animactor.h"
    "Player/p_anims.c"
    "Player/p_anims.h"
    "Player/p_chicken.c"
    "Player/p_chicken.h"
    "Player/p_chicken_anim.c"
    "Player/p_ctrl.c"
    "Player/p_ctrl.h"
    "Player/p_items.c"
    "Player/p_items.h"
    "Player/p_main.c"
    "Player/p_main.h"
    "Player/p_newmove.h"
    "Player/p_types.h"
    "Player/p_weapon.c"
    "Player/p_weapon.h"
    "Player/Player.h"
    "Player/player_main.c"
)
source_group("Player" FILES ${Player})

set(WinSnd
    "WinSnd/s_eax.c"
    "WinSnd/s_main.c"
)
source_group("WinSnd" FILES ${WinSnd})

set(WinSnd__ALut
    "WinSnd/ALut/alutBufferData.c"
    "WinSnd/ALut/alutCodec.c"
    "WinSnd/ALut/alutError.c"
    "WinSnd/ALut/alutInit.c"
    "WinSnd/ALut/alutInputStream.c"
    "WinSnd/ALut/alutInternal.h"
    "WinSnd/ALut/alutLoader.c"
    "WinSnd/ALut/alutOutputStream.c"
    "WinSnd/ALut/alutUtil.c"
    "WinSnd/ALut/alutVersion.c"
    "WinSnd/ALut/alutWaveform.c"
)
source_group("WinSnd\\ALut" FILES ${WinSnd__ALut})

set(client
    "client/adivtab.h"
    "client/anorms.h"
    "client/asm_i386.h"
    "client/block16.h"
    "client/block8.h"
    "client/cdaudio.h"
    "client/cl_cin.c"
    "client/cl_ents.c"
    "client/cl_fx.c"
    "client/cl_input.c"
    "client/cl_inv.c"
    "client/cl_main.c"
    "client/cl_parse.c"
    "client/cl_pred.c"
    "client/cl_scrn.c"
    "client/cl_string.c"
    "client/cl_view.c"
    "client/client.h"
    "client/console.c"
    "client/console.h"
    "client/input.h"
    "client/keys.c"
    "client/keys.h"
    "client/menu.c"
    "client/qmenu.c"
    "client/qmenu.h"
    "client/ref.h"
    "client/screen.h"
    "client/vid.h"
    "client/x86.c"
)
source_group("client" FILES ${client})

set(external__libsmacker
    "external/libsmacker/smacker.c"
    "external/libsmacker/smacker.h"
    "external/libsmacker/smk_bitstream.c"
    "external/libsmacker/smk_bitstream.h"
    "external/libsmacker/smk_hufftree.c"
    "external/libsmacker/smk_hufftree.h"
    "external/libsmacker/smk_malloc.h"
)
source_group("external\\libsmacker" FILES ${external__libsmacker})

set(h2common
    "h2common/H2Console.c"
    "h2common/H2Math.c"
    "h2common/H2Matrix.c"
    "h2common/H2Motion.c"
    "h2common/H2Palette.c"
    "h2common/H2Physics.c"
    "h2common/H2Rand.c"
    "h2common/H2SinglyLinkedList.c"
    "h2common/H2Surfaces.c"
    "h2common/H2Vector.c"
    "qcommon/ResourceManager.c"
)
source_group("h2common" FILES ${h2common})

set(h2common__Math
    "h2common/Math/Angles.h"
    "h2common/Math/Martix.c"
    "h2common/Math/Matrix.h"
    "h2common/Math/Vector.h"
)
source_group("h2common\\Math" FILES ${h2common__Math})

set(qcommon
    "qcommon/Angles.h"
    "qcommon/anorms.h"
    "qcommon/ArrayedList.h"
    "qcommon/asm.h"
    "qcommon/cdaudio.h"
    "qcommon/cl_strings.h"
    "qcommon/cmd.c"
    "qcommon/cmodel.c"
    "qcommon/cmodel.h"
    "qcommon/cmodel_private.h"
    "qcommon/common.c"
    "qcommon/compfmod.h"
    "qcommon/crc.c"
    "qcommon/crc.h"
    "qcommon/cvar.c"
    "qcommon/EffectFlags.h"
    "qcommon/fc.h"
    "qcommon/files.c"
    "qcommon/flex.h"
    "qcommon/FX.h"
    "qcommon/GenericUnions.h"
    "qcommon/H2Common.h"
    "qcommon/items.h"
    "qcommon/LevelMaps.h"
    "qcommon/Matrix.h"
    "qcommon/md4.c"
    "qcommon/Message.h"
    "qcommon/ModelJoints.h"
    "qcommon/motion.h"
    "qcommon/net_chan.c"
    "qcommon/NetMsg_Read.c"
    "qcommon/Placement.h"
    "qcommon/playeranim.h"
    "qcommon/pmove.c"
    "qcommon/PrimitiveDisplayHack.h"
    "qcommon/q_ClientServer.h"
    "qcommon/q_math.h"
    "qcommon/q_Physics.h"
    "qcommon/q_Sprite.h"
    "qcommon/q_Surface.h"
    "qcommon/q_Typedef.h"
    "qcommon/qcommon.h"
    "qcommon/qfiles.h"
    "qcommon/random.h"
    "qcommon/Reference.c"
    "qcommon/Reference.h"
    "qcommon/ResourceManager.h"
    "qcommon/SinglyLinkedList.h"
    "qcommon/Skeletons.h"
    "qcommon/surfaceprops.h"
    "qcommon/timing.h"
    "qcommon/tokens.h"
    "qcommon/Vector.h"
    "qcommon/VerifyA3D.h"
)
source_group("qcommon" FILES ${qcommon})

set(ref_common
    "ref_common/fmodel.h"
    "ref_common/m_Reference.h"
    "ref_common/m_SkeletalCluster.h"
    "ref_common/m_Skeleton.h"
    "ref_common/part_uvs.h"
    "ref_common/q_fmodel.h"
    "ref_common/qd_skeletons.h"
    "ref_common/r_Skeletons.c"
    "ref_common/r_Skeletons.h"
)
source_group("ref_common" FILES ${ref_common})

set(ref_gl
    "ref_gl/anorms.h"
    "ref_gl/anormtab.h"
    "ref_gl/gl_book.c"
    "ref_gl/gl_draw.c"
    "ref_gl/gl_fmodel.c"
    "ref_gl/gl_image.c"
    "ref_gl/gl_light.c"
    "ref_gl/gl_local.h"
    "ref_gl/gl_math.c"
    "ref_gl/gl_mesh.c"
    "ref_gl/gl_model.c"
    "ref_gl/gl_model.h"
    "ref_gl/gl_movie.c"
    "ref_gl/gl_rmain.c"
    "ref_gl/gl_rmisc.c"
    "ref_gl/gl_rsurf.c"
    "ref_gl/gl_warp.c"
    "ref_gl/glew.c"
    "ref_gl/qgl.h"
    "ref_gl/warpsin.h"
    "win32/glw_imp.c"
    "win32/q_shwin.c"
    "win32/qgl_win.c"
)
source_group("ref_gl" FILES ${ref_gl})

set(ref_gl__GL
    "ref_gl/GL/eglew.h"
    "ref_gl/GL/glew.h"
    "ref_gl/GL/glxew.h"
    "ref_gl/GL/wglew.h"
)
source_group("ref_gl\\GL" FILES ${ref_gl__GL})

set(server
    "server/server.h"
    "server/sv_ccmds.c"
    "server/sv_ents.c"
    "server/sv_game.c"
    "server/sv_init.c"
    "server/sv_main.c"
    "server/sv_send.c"
    "server/sv_user.c"
    "server/sv_world.c"
)
source_group("server" FILES ${server})

set(win32
    "win32/cd_win.c"
    "win32/conproc.c"
    "win32/conproc.h"
    "win32/in_win.c"
    "win32/net_wins.c"
    "win32/resource.h"
    "win32/sys_win.c"
    "win32/vid_dll.c"
    "win32/vid_menu.c"
)
source_group("win32" FILES ${win32})

set(ALL_FILES
    ${Client_Effects}
    ${Game}
    ${Player}
    ${WinSnd}
    ${WinSnd__ALut}
    ${client}
    ${external__libsmacker}
    ${h2common}
    ${h2common__Math}
    ${qcommon}
    ${ref_common}
    ${ref_gl}
    ${ref_gl__GL}
    ${server}
    ${win32}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE Heretic2)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Target name
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    TARGET_NAME_DEBUG   "Heretic2x64"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/../../.."
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)
################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/client;"
    "${CMAKE_CURRENT_SOURCE_DIR}/game;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Player;"
    "${CMAKE_CURRENT_SOURCE_DIR}/qcommon;"
    "${CMAKE_CURRENT_SOURCE_DIR}/WinSnd/OpenAL/include"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "_HERETIC2_;"
    "GLEW_STATIC;"
    "_WINDOWS;"
    "_MBCS"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        /wd4996;
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_chicken.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_chicken_anim.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_elflord.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_elflord_anims.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_fish.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_fish_anim.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_gkrokon.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_gkrokon_anim.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_gorgon.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_gorgon_anim.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_harpy.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_harpy_anim.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_plagueElf.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_plagueElf_anim.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_plagueSsithra.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_plagueSsithra_anim.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_rat.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_rat_anim.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_spreader.cpp ${FILE_CL_OPTIONS})
    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )
    source_file_compile_options(game/m_spreader_anim.cpp ${FILE_CL_OPTIONS})
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /INCREMENTAL
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO
        >
        /DEBUG;
        /SUBSYSTEM:WINDOWS;
        /force
    )
endif()

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "./WinSnd/OpenAL/x64/OpenAL64;"
    "Ws2_32;"
    "winmm;"
    "opengl32;"
    "kernel32;"
    "user32;"
    "gdi32;"
    "winspool;"
    "comdlg32;"
    "advapi32;"
    "shell32;"
    "ole32;"
    "oleaut32;"
    "uuid;"
    "odbc32;"
    "odbccp32"
)
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

