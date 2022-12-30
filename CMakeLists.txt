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
    "client_effects/Ambient_Effects.cpp"
    "client_effects/ambient_effects.h"
    "client_effects/ce_DefaultMessageHandler.cpp"
    "client_effects/ce_DefaultMessageHandler.h"
    "client_effects/ce_DLight.cpp"
    "client_effects/ce_DLight.h"
    "client_effects/ce_Message.cpp"
    "client_effects/ce_Message.h"
    "client_effects/Client_Effects.cpp"
    "client_effects/Client_Effects.h"
    "client_effects/Client_Entities.cpp"
    "client_effects/Client_Entities.h"
    "client_effects/Font1.cpp"
    "client_effects/Font2.cpp"
    "client_effects/fx_AmmoPickup.cpp"
    "client_effects/fx_animate.cpp"
    "client_effects/fx_assassin.cpp"
    "client_effects/fx_blood.cpp"
    "client_effects/fx_BlueRing.cpp"
    "client_effects/fx_bubbler.cpp"
    "client_effects/fx_crosshair.cpp"
    "client_effects/fx_cwatcher.cpp"
    "client_effects/fx_debris.cpp"
    "client_effects/fx_debris.h"
    "client_effects/fx_DefensePickup.cpp"
    "client_effects/fx_Dripper.cpp"
    "client_effects/fx_dust.cpp"
    "client_effects/fx_dustpuff.cpp"
    "client_effects/fx_dustpuff.h"
    "client_effects/fx_fire.cpp"
    "client_effects/fx_firehands.cpp"
    "client_effects/fx_flamethrow.cpp"
    "client_effects/fx_flyingfist.cpp"
    "client_effects/fx_Fountain.cpp"
    "client_effects/fx_halo.cpp"
    "client_effects/fx_HealthPickup.cpp"
    "client_effects/fx_HellStaff.cpp"
    "client_effects/fx_hitpuff.cpp"
    "client_effects/fx_hpproj.cpp"
    "client_effects/fx_insectstaff.cpp"
    "client_effects/fx_lensflare.cpp"
    "client_effects/fx_lightning.cpp"
    "client_effects/fx_Maceball.cpp"
    "client_effects/fx_magicmissile.cpp"
    "client_effects/fx_meteorbarrier.cpp"
    "client_effects/fx_mist.cpp"
    "client_effects/fx_mork.cpp"
    "client_effects/fx_Morph.cpp"
    "client_effects/fx_objects.cpp"
    "client_effects/fx_pespell.cpp"
    "client_effects/fx_Phoenix.cpp"
    "client_effects/fx_pickup.cpp"
    "client_effects/fx_pickuppuzzle.cpp"
    "client_effects/fx_PlagueMist.cpp"
    "client_effects/fx_PlagueMistExplode.cpp"
    "client_effects/fx_portal.cpp"
    "client_effects/fx_quake.cpp"
    "client_effects/fx_RedRain.cpp"
    "client_effects/fx_redrainglow.cpp"
    "client_effects/fx_remotecamera.cpp"
    "client_effects/fx_Ripples.cpp"
    "client_effects/fx_rope.cpp"
    "client_effects/fx_scorchmark.cpp"
    "client_effects/fx_shadow.cpp"
    "client_effects/fx_Shield.cpp"
    "client_effects/fx_shrine.cpp"
    "client_effects/fx_smoke.cpp"
    "client_effects/fx_sound.cpp"
    "client_effects/fx_sparks.cpp"
    "client_effects/fx_spellchange.cpp"
    "client_effects/fx_spellhands.cpp"
    "client_effects/fx_sphereofannihlation.cpp"
    "client_effects/fx_spoo.cpp"
    "client_effects/fx_ssarrow.cpp"
    "client_effects/fx_ssithra.cpp"
    "client_effects/fx_staff.cpp"
    "client_effects/fx_tbeast.cpp"
    "client_effects/fx_teleport.cpp"
    "client_effects/fx_tome.cpp"
    "client_effects/fx_tornado.cpp"
    "client_effects/fx_Wall.cpp"
    "client_effects/fx_waterentrysplash.cpp"
    "client_effects/fx_waterwake.cpp"
    "client_effects/fx_WeaponPickup.cpp"
    "client_effects/Generic_Character_Effects.cpp"
    "client_effects/Generic_Weapon_Effects.cpp"
    "client_effects/Item_Effects.cpp"
    "client_effects/LevelMaps.cpp"
    "client_effects/LightStyles.cpp"
    "client_effects/Main.cpp"
    "client_effects/Monster_Weapon_Effects.cpp"
    "client_effects/Particle.cpp"
    "client_effects/Particle.h"
    "client_effects/Player_Effects.cpp"
    "client_effects/TestEffect.cpp"
    "client_effects/Utilities.cpp"
    "client_effects/Utilities.h"
)
source_group("Client Effects" FILES ${Client_Effects})

set(Game
    "game/buoy.cpp"
    "game/buoy.h"
    "game/c_ai.cpp"
    "game/c_ai.h"
    "game/c_corvus1.cpp"
    "game/c_corvus1.h"
    "game/c_corvus1_anim.cpp"
    "game/c_corvus1_anim.h"
    "game/c_corvus2.cpp"
    "game/c_corvus2.h"
    "game/c_corvus2_anim.cpp"
    "game/c_corvus2_anim.h"
    "game/c_corvus3.cpp"
    "game/c_corvus3.h"
    "game/c_corvus3_anim.cpp"
    "game/c_corvus3_anim.h"
    "game/c_corvus4.cpp"
    "game/c_corvus4.h"
    "game/c_corvus4_anim.cpp"
    "game/c_corvus4_anim.h"
    "game/c_corvus5.cpp"
    "game/c_corvus5.h"
    "game/c_corvus5_anim.cpp"
    "game/c_corvus5_anim.h"
    "game/c_corvus6.cpp"
    "game/c_corvus6.h"
    "game/c_corvus6_anim.cpp"
    "game/c_corvus6_anim.h"
    "game/c_corvus7.cpp"
    "game/c_corvus7.h"
    "game/c_corvus7_anim.cpp"
    "game/c_corvus7_anim.h"
    "game/c_corvus8.cpp"
    "game/c_corvus8.h"
    "game/c_corvus8_anim.cpp"
    "game/c_corvus8_anim.h"
    "game/c_corvus9.cpp"
    "game/c_corvus9.h"
    "game/c_corvus9_anim.cpp"
    "game/c_corvus9_anim.h"
    "game/c_dranor.cpp"
    "game/c_Dranor.h"
    "game/c_dranor_anim.cpp"
    "game/c_dranor_anim.h"
    "game/c_elflord.cpp"
    "game/c_elflord.h"
    "game/c_elflord_anim.cpp"
    "game/c_elflord_anim.h"
    "game/c_morcalavin.cpp"
    "game/c_morcalavin.h"
    "game/c_morcalavin_anim.cpp"
    "game/c_morcalavin_anim.h"
    "game/c_priestess.cpp"
    "game/c_priestess.h"
    "game/c_priestess2.cpp"
    "game/c_priestess2.h"
    "game/c_priestess2_anim.cpp"
    "game/c_priestess2_anim.h"
    "game/c_priestess_anim.cpp"
    "game/c_priestess_anim.h"
    "game/c_siernan1.cpp"
    "game/c_siernan1.h"
    "game/c_siernan1_anim.cpp"
    "game/c_siernan1_anim.h"
    "game/c_siernan2.cpp"
    "game/c_siernan2.h"
    "game/c_siernan2_anim.cpp"
    "game/c_siernan2_anim.h"
    "game/c_ssithrascout.cpp"
    "game/c_ssithrascout.h"
    "game/c_ssithrascout_anim.cpp"
    "game/c_ssithrascout_anim.h"
    "game/c_tome.cpp"
    "game/c_tome.h"
    "game/c_tome_anim.cpp"
    "game/c_tome_anim.h"
    "game/c_victimSsithra.cpp"
    "game/c_victimSsithra.h"
    "game/c_victimSsithra_anim.cpp"
    "game/c_victimSsithra_anim.h"
    "game/decals.cpp"
    "game/decals.h"
    "game/ds.cpp"
    "game/ds.h"
    "game/g_ai.cpp"
    "game/g_BoundingForm.h"
    "game/g_breakable.cpp"
    "game/g_ClassStatics.cpp"
    "game/g_ClassStatics.h"
    "game/g_cmds.cpp"
    "game/g_combat.cpp"
    "game/g_DefaultMessageHandler.cpp"
    "game/g_DefaultMessageHandler.h"
    "game/g_Edict.h"
    "game/g_env.cpp"
    "game/g_field.cpp"
    "game/g_flamethrow.cpp"
    "game/g_func.cpp"
    "game/g_HitLocation.cpp"
    "game/g_HitLocation.h"
    "game/g_items.cpp"
    "game/g_items.h"
    "game/g_items_pickups.cpp"
    "game/g_itemstats.h"
    "game/g_light.cpp"
    "game/g_local.h"
    "game/g_main.cpp"
    "game/g_Message.cpp"
    "game/g_Message.h"
    "game/g_misc.cpp"
    "game/g_misc.h"
    "game/g_monster.cpp"
    "game/g_monster.h"
    "game/g_Monster_State.h"
    "game/g_MoveInfo.cpp"
    "game/g_MoveInfo.h"
    "game/g_NewSystem.h"
    "game/g_obj.cpp"
    "game/g_phys.cpp"
    "game/g_Physics.cpp"
    "game/g_Physics.h"
    "game/g_PhysicsInfo.cpp"
    "game/g_PhysicsInfo.h"
    "game/g_playstats.h"
    "game/g_ResourceManagers.cpp"
    "game/g_rope.cpp"
    "game/g_save.cpp"
    "game/g_shrine.cpp"
    "game/g_Skeleton.h"
    "game/g_Skeletons.cpp"
    "game/g_Skeletons.h"
    "game/g_spawn.cpp"
    "game/g_spawnf.cpp"
    "game/g_StateInfo.cpp"
    "game/g_StateInfo.h"
    "game/g_svcmds.cpp"
    "game/g_target.cpp"
    "game/g_teleport.h"
    "game/g_trigger.cpp"
    "game/g_Typedef.h"
    "game/g_utils.cpp"
    "game/g_volume_effect.h"
    "game/g_waterfx.cpp"
    "game/g_weapon.cpp"
    "game/g_weapon.h"
    "game/game.h"
    "game/game_utilities.cpp"
    "game/GameEntity.cpp"
    "game/GameEntity.h"
    "game/ICScript.h"
    "game/list.h"
    "game/m_actor.h"
    "game/m_assassin.cpp"
    "game/m_assassin.h"
    "game/m_assassin_anim.cpp"
    "game/m_assassin_anim.h"
    "game/m_beast.cpp"
    "game/m_beast.h"
    "game/m_beast_anim.cpp"
    "game/m_beast_anim.h"
    "game/m_bee.cpp"
    "game/m_chicken.cpp"
    "game/m_chicken.h"
    "game/m_chicken_anim.cpp"
    "game/m_chicken_anim.h"
    "game/m_elf.h"
    "game/m_elflord.cpp"
    "game/m_elflord.h"
    "game/m_elflord_anims.cpp"
    "game/m_elflord_anims.h"
    "game/m_fish.cpp"
    "game/m_fish.h"
    "game/m_fish_anim.cpp"
    "game/m_fish_anim.h"
    "game/m_FMtest.cpp"
    "game/m_gkrokon.cpp"
    "game/m_gkrokon.h"
    "game/m_gkrokon_anim.cpp"
    "game/m_gkrokon_anim.h"
    "game/m_gorgon.cpp"
    "game/m_gorgon.h"
    "game/m_gorgon_anim.cpp"
    "game/m_gorgon_anim.h"
    "game/m_harpy.cpp"
    "game/m_harpy.h"
    "game/m_harpy_anim.cpp"
    "game/m_harpy_anim.h"
    "game/m_imp.cpp"
    "game/m_imp.h"
    "game/m_imp_anim.cpp"
    "game/m_imp_anim.h"
    "game/m_morcalavin.cpp"
    "game/m_morcalavin.h"
    "game/m_morcalavin_anim.cpp"
    "game/m_morcalavin_anim.h"
    "game/m_mother.cpp"
    "game/m_mother.h"
    "game/m_mother_anim.cpp"
    "game/m_mother_anim.h"
    "game/m_move.cpp"
    "game/m_mssithra.cpp"
    "game/m_mssithra.h"
    "game/m_mssithra_anim.cpp"
    "game/m_mssithra_anim.h"
    "game/m_ogle.cpp"
    "game/m_ogle.h"
    "game/m_ogle_anim.cpp"
    "game/m_ogle_anim.h"
    "game/m_plagueElf.cpp"
    "game/m_plagueElf.h"
    "game/m_plagueElf_anim.cpp"
    "game/m_plagueElf_anim.h"
    "game/m_plagueSsithra.cpp"
    "game/m_plagueSsithra.h"
    "game/m_plagueSsithra_anim.cpp"
    "game/m_plagueSsithra_anim.h"
    "game/m_player.h"
    "game/m_priestess.cpp"
    "game/m_priestess.h"
    "game/m_priestess_anim.cpp"
    "game/m_priestess_anim.h"
    "game/m_rat.cpp"
    "game/m_rat.h"
    "game/m_rat_anim.cpp"
    "game/m_rat_anim.h"
    "game/m_seraph.cpp"
    "game/m_seraph.h"
    "game/m_seraph_anim.cpp"
    "game/m_seraph_anim.h"
    "game/m_seraph_guard.cpp"
    "game/m_seraph_guard.h"
    "game/m_seraph_guard_anim.cpp"
    "game/m_seraph_guard_anim.h"
    "game/m_spreader.cpp"
    "game/m_spreader.h"
    "game/m_spreader_anim.cpp"
    "game/m_spreader_anim.h"
    "game/m_spreaderbomb_anim.h"
    "game/m_spreadermist.cpp"
    "game/m_spreadermist.h"
    "game/m_stats.cpp"
    "game/m_stats.h"
    "game/m_tcheckrik.cpp"
    "game/m_tcheckrik.h"
    "game/m_tcheckrik_anim.cpp"
    "game/m_tcheckrik_anim.h"
    "game/m_tcheckrik_spells.cpp"
    "game/mg_ai.cpp"
    "game/mg_guide.cpp"
    "game/mg_guide.h"
    "game/p_client.cpp"
    "game/p_funcs.cpp"
    "game/p_funcs.h"
    "game/p_hud.cpp"
    "game/p_item.cpp"
    "game/p_item.h"
    "game/p_view.cpp"
    "game/pcode.h"
    "game/q_shared.cpp"
    "game/q_shared.h"
    "game/Script.cpp"
    "game/spl_blast.cpp"
    "game/spl_BlueRing.cpp"
    "game/spl_flyingfist.cpp"
    "game/spl_HellStaff.cpp"
    "game/spl_maceballs.cpp"
    "game/spl_magicmissile.cpp"
    "game/spl_meteorbarrier.cpp"
    "game/spl_morph.cpp"
    "game/spl_Phoenix.cpp"
    "game/spl_powerup.cpp"
    "game/spl_RedRain.cpp"
    "game/spl_ripper.cpp"
    "game/spl_shield.cpp"
    "game/spl_sphereofannihlation.cpp"
    "game/spl_teleport.cpp"
    "game/spl_tornado.cpp"
    "game/spl_wall.cpp"
    "game/Utilities.h"
    "qcommon/Message.cpp"
    "qcommon/p_dll.cpp"
    "qcommon/Skeletons.cpp"
)
source_group("Game" FILES ${Game})

set(Player
    "Player/p_actions.cpp"
    "Player/p_actions.h"
    "Player/p_anim_branch.cpp"
    "Player/p_anim_branch.h"
    "Player/p_anim_data.cpp"
    "Player/p_anim_data.h"
    "Player/p_animactor.cpp"
    "Player/p_animactor.h"
    "Player/p_anims.cpp"
    "Player/p_anims.h"
    "Player/p_chicken.cpp"
    "Player/p_chicken.h"
    "Player/p_chicken_anim.cpp"
    "Player/p_ctrl.cpp"
    "Player/p_ctrl.h"
    "Player/p_items.cpp"
    "Player/p_items.h"
    "Player/p_main.cpp"
    "Player/p_main.h"
    "Player/p_newmove.h"
    "Player/p_types.h"
    "Player/p_weapon.cpp"
    "Player/p_weapon.h"
    "Player/Player.h"
    "Player/player_main.cpp"
)
source_group("Player" FILES ${Player})

set(WinSnd
    "WinSnd/s_eax.cpp"
    "WinSnd/s_main.cpp"
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
    "client/cl_cin.cpp"
    "client/cl_ents.cpp"
    "client/cl_fx.cpp"
    "client/cl_input.cpp"
    "client/cl_inv.cpp"
    "client/cl_main.cpp"
    "client/cl_parse.cpp"
    "client/cl_pred.cpp"
    "client/cl_scrn.cpp"
    "client/cl_string.cpp"
    "client/cl_view.cpp"
    "client/client.h"
    "client/console.cpp"
    "client/console.h"
    "client/input.h"
    "client/keys.cpp"
    "client/keys.h"
    "client/menu.cpp"
    "client/qmenu.cpp"
    "client/qmenu.h"
    "client/ref.h"
    "client/screen.h"
    "client/vid.h"
    "client/x86.cpp"
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
    "h2common/H2Console.cpp"
    "h2common/H2Math.cpp"
    "h2common/H2Matrix.cpp"
    "h2common/H2Motion.cpp"
    "h2common/H2Palette.cpp"
    "h2common/H2Physics.cpp"
    "h2common/H2Rand.cpp"
    "h2common/H2SinglyLinkedList.cpp"
    "h2common/H2Surfaces.cpp"
    "h2common/H2Vector.cpp"
    "qcommon/ResourceManager.cpp"
)
source_group("h2common" FILES ${h2common})

set(h2common__Math
    "h2common/Math/Angles.h"
    "h2common/Math/Martix.cpp"
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
    "qcommon/cmd.cpp"
    "qcommon/cmodel.cpp"
    "qcommon/cmodel.h"
    "qcommon/cmodel_private.h"
    "qcommon/common.cpp"
    "qcommon/compfmod.h"
    "qcommon/crc.cpp"
    "qcommon/crc.h"
    "qcommon/cvar.cpp"
    "qcommon/EffectFlags.h"
    "qcommon/fc.h"
    "qcommon/files.cpp"
    "qcommon/flex.h"
    "qcommon/FX.h"
    "qcommon/GenericUnions.h"
    "qcommon/H2Common.h"
    "qcommon/items.h"
    "qcommon/LevelMaps.h"
    "qcommon/Matrix.h"
    "qcommon/md4.cpp"
    "qcommon/Message.h"
    "qcommon/ModelJoints.h"
    "qcommon/motion.h"
    "qcommon/net_chan.cpp"
    "qcommon/NetMsg_Read.cpp"
    "qcommon/Placement.h"
    "qcommon/playeranim.h"
    "qcommon/pmove.cpp"
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
    "qcommon/Reference.cpp"
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
    "ref_common/r_Skeletons.cpp"
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
    "server/sv_ccmds.cpp"
    "server/sv_ents.cpp"
    "server/sv_game.cpp"
    "server/sv_init.cpp"
    "server/sv_main.cpp"
    "server/sv_send.cpp"
    "server/sv_user.cpp"
    "server/sv_world.cpp"
)
source_group("server" FILES ${server})

set(win32
    "win32/cd_win.cpp"
    "win32/conproc.cpp"
    "win32/conproc.h"
    "win32/in_win.cpp"
    "win32/net_wins.cpp"
    "win32/resource.h"
    "win32/sys_win.cpp"
    "win32/vid_dll.cpp"
    "win32/vid_menu.cpp"
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

