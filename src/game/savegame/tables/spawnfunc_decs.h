/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (C) 2011 Yamagi Burmeister
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
 * Prototypes for every spawn function in the game.so.
 *
 * =======================================================================
 */

extern void SP_info_player_start (edict_t *ent);
extern void SP_info_player_deathmatch (edict_t *ent);
extern void SP_info_player_coop (edict_t *ent);
extern void SP_info_player_intermission (edict_t *ent);
extern void SP_info_buoy (edict_t *ent);
extern void SP_func_plat (edict_t *ent);
extern void SP_func_rotating (edict_t *ent);
extern void SP_func_button (edict_t *ent);
extern void SP_func_door (edict_t *ent);
extern void SP_func_door_secret (edict_t *ent);
extern void SP_func_door_rotating (edict_t *ent);
extern void SP_func_water (edict_t *ent);
extern void SP_func_train (edict_t *ent);
extern void SP_func_wall (edict_t *self);
extern void SP_func_object (edict_t *self);
extern void SP_func_timer (edict_t *self);
extern void SP_func_areaportal (edict_t *ent);
extern void SP_func_monsterspawner (edict_t *ent);
extern void SP_trigger_Activate(edict_t *self);
extern void SP_trigger_Always (edict_t *ent);
extern void SP_trigger_Counter (edict_t *ent);
extern void SP_trigger_Damage(edict_t *self);
extern void SP_trigger_Deactivate(edict_t *self);
extern void SP_trigger_Elevator (edict_t *ent);
//void SP_trigger_flamethrower (edict_t *ent);
extern void SP_trigger_fogdensity(edict_t *self);
extern void SP_trigger_Gravity(edict_t *self);
extern void SP_trigger_mappercentage(edict_t *self);
extern void SP_trigger_quit_to_menu(edict_t *self);
extern void SP_trigger_mission_give(edict_t *self);
extern void SP_trigger_mission_take(edict_t *self);
extern void SP_trigger_MonsterJump(edict_t *self);
extern void SP_trigger_goto_buoy(edict_t *self);
extern void SP_trigger_Multiple (edict_t *ent);
extern void SP_trigger_Once (edict_t *ent);
extern void SP_trigger_PlayerPushButton (edict_t *ent);
extern void SP_trigger_PlayerPushLever (edict_t *ent);
extern void SP_trigger_PlayerUsePuzzle (edict_t *ent);
extern void SP_trigger_push(edict_t *self);
extern void SP_trigger_puzzle (edict_t *ent);
extern void SP_trigger_quake (edict_t *ent);
extern void SP_trigger_Relay (edict_t *ent);
extern void SP_trigger_lightning (edict_t *ent);
extern void SP_trigger_farclip (edict_t *ent);
extern void SP_trigger_endgame(edict_t *self);
extern void SP_choose_CDTrack(edict_t *self);
extern void SP_target_explosion (edict_t *ent);
extern void SP_target_changelevel (edict_t *ent);
extern void SP_target_crosslevel_trigger (edict_t *ent);
extern void SP_target_crosslevel_target (edict_t *ent);
extern void SP_target_lightramp (edict_t *self);
extern void SP_target_earthquake (edict_t *ent);
extern void SP_worldspawn (edict_t *ent);
extern void SP_light (edict_t *self);
extern void SP_info_null (edict_t *self);
extern void SP_info_notnull (edict_t *self);
extern void SP_path_corner (edict_t *self);
extern void SP_point_combat (edict_t *self);
extern void SP_misc_teleporter (edict_t *self);
extern void SP_misc_teleporter_dest (edict_t *self);
extern void misc_update_spawner (edict_t *self);
extern void SP_misc_remote_camera (edict_t *self);
extern void SP_misc_magic_portal (edict_t *self);
extern void SP_misc_fire_sparker (edict_t *ent);
extern void SP_Monster_Gkrokon(edict_t *Self);
extern void SP_misc_flag (edict_t *ent);
extern void SP_monster_gorgon (edict_t *self);
extern void SP_monster_gorgon_leader (edict_t *self);
extern void SP_monster_rat (edict_t *self);
extern void SP_monster_rat_giant (edict_t *self);
extern void SP_monster_chicken (edict_t *self);
extern void SP_monster_plagueElf(edict_t *self);
extern void SP_monster_palace_plague_guard(edict_t *self);
extern void SP_monster_palace_plague_guard_invisible(edict_t *self);
extern void SP_monster_fish (edict_t *self);
extern void SP_monster_harpy (edict_t *self);
extern void SP_monster_spreader (edict_t *self);
extern void SP_monster_elflord (edict_t *self);
extern void SP_monster_plague_ssithra (edict_t *self);
extern void SP_monster_mssithra (edict_t *self);
extern void SP_monster_chkroktk (edict_t *self);
extern void SP_monster_tcheckrik_male (edict_t *self);
extern void SP_monster_tcheckrik_female (edict_t *self);
extern void SP_monster_tcheckrik_mothers (edict_t *self);
extern void SP_monster_high_priestess (edict_t *self);
extern void SP_monster_ogle (edict_t *self);
extern void SP_monster_seraph_overlord (edict_t *self);
extern void SP_monster_seraph_guard (edict_t *self);
extern void SP_monster_assassin (edict_t *self);
extern void SP_monster_morcalavin (edict_t *self);
extern void SP_monster_trial_beast (edict_t *self);
extern void SP_monster_imp (edict_t *self);
extern void SP_monster_bee (edict_t *self);
extern void SP_character_corvus1 (edict_t *self);
extern void SP_character_corvus2 (edict_t *self);
extern void SP_character_corvus3 (edict_t *self);
extern void SP_character_corvus4 (edict_t *self);
extern void SP_character_corvus5 (edict_t *self);
extern void SP_character_corvus6 (edict_t *self);
extern void SP_character_corvus7 (edict_t *self);
extern void SP_character_corvus8 (edict_t *self);
extern void SP_character_corvus9 (edict_t *self);
extern void SP_character_dranor (edict_t *self);
extern void SP_character_elflord (edict_t *self);
extern void SP_character_highpriestess (edict_t *self);
extern void SP_character_highpriestess2 (edict_t *self);
extern void SP_character_morcalavin (edict_t *self);
extern void SP_character_sidhe_guard (edict_t *self);
extern void SP_character_siernan1 (edict_t *self);
extern void SP_character_siernan2 (edict_t *self);
extern void SP_character_ssithra_scout (edict_t *self);
extern void SP_character_ssithra_victim (edict_t *self);
extern void SP_character_tome (edict_t *self);
extern void SP_breakable_brush (edict_t *ent);
extern void SP_light_walltorch (edict_t *ent);
//void SP_light_flame (edict_t *ent);
extern void SP_light_floortorch (edict_t *ent);
extern void SP_light_torch1(edict_t *ent);
extern void SP_light_gem2(edict_t *ent);
extern void SP_light_chandelier1 (edict_t *ent);
extern void SP_light_chandelier2 (edict_t *ent);
extern void SP_light_chandelier3 (edict_t *ent);
extern void SP_light_lantern1 (edict_t *ent);
extern void SP_light_lantern2 (edict_t *ent);
extern void SP_light_lantern3 (edict_t *ent);
extern void SP_light_lantern4 (edict_t *ent);
extern void SP_light_lantern5 (edict_t *ent);
extern void SP_light_buglight (edict_t *ent);
extern void SP_env_fire (edict_t *self);
extern void SP_env_dust (edict_t *self);
extern void SP_env_smoke (edict_t *self);
extern void SP_env_mist(edict_t *self);
extern void SP_env_bubbler(edict_t *self);
extern void SP_env_water_drip(edict_t *self);
extern void SP_env_water_fountain(edict_t *self);
extern void SP_env_waterfall_base(edict_t *self);
extern void SP_env_sun1(edict_t *ent);
extern void SP_env_muck(edict_t *ent);
extern void SP_sound_ambient_silverspring (edict_t *ent);
extern void SP_sound_ambient_swampcanyon (edict_t *ent);
extern void SP_sound_ambient_andoria (edict_t *ent);
extern void SP_sound_ambient_hive (edict_t *ent);
extern void SP_sound_ambient_mine (edict_t *ent);
extern void SP_sound_ambient_cloudfortress (edict_t *ent);
// Object stuff
extern void SP_obj_andwallhanging(edict_t *ent);
extern void SP_obj_banner(edict_t *ent);
extern void SP_obj_banneronpole(edict_t *ent);
extern void SP_obj_barrel(edict_t *ent);
extern void SP_obj_barrel_explosive(edict_t *ent);
extern void SP_obj_barrel_metal(edict_t *ent);
extern void SP_obj_basket(edict_t *ent);
extern void SP_obj_bench(edict_t *ent);
extern void SP_obj_bigcrystal(edict_t *self);
extern void SP_obj_biotank (edict_t *self);
extern void SP_obj_bloodsplat(edict_t *ent);
extern void SP_obj_bookclosed(edict_t *ent);
extern void SP_obj_bookopen(edict_t *ent);
extern void SP_obj_bottle1(edict_t *ent);
extern void SP_obj_broom (edict_t *ent);
extern void SP_obj_bucket (edict_t *ent);
extern void SP_obj_bush1 (edict_t *ent);
extern void SP_obj_bush2 (edict_t *ent);
extern void SP_obj_cactus (edict_t *ent);
extern void SP_obj_cactus3 (edict_t *ent);
extern void SP_obj_cactus4 (edict_t *ent);
extern void SP_obj_cauldron(edict_t *ent);
extern void SP_obj_chair1(edict_t *ent);
extern void SP_obj_chair2(edict_t *ent);
extern void SP_obj_chair3(edict_t *ent);
extern void SP_obj_chest1(edict_t *ent);
extern void SP_obj_chest2(edict_t *ent);
extern void SP_obj_chest3(edict_t *ent);
extern void SP_obj_choppeddude(edict_t *ent);
extern void SP_obj_claybowl (edict_t *ent);
extern void SP_obj_clayjar (edict_t *ent);
extern void SP_obj_cocoon(edict_t *ent);
extern void SP_obj_cocoonopen(edict_t *ent);
extern void SP_obj_cog1(edict_t *ent);
extern void SP_obj_corpse1(edict_t *self);
extern void SP_obj_corpse2(edict_t *self);
extern void SP_obj_corpse_ogle(edict_t *self);
extern void SP_obj_corpse_ssithra(edict_t *self);
extern void SP_obj_dying_elf(edict_t *self);
extern void SP_obj_eggpan(edict_t *ent);
extern void SP_obj_eyeball_jar(edict_t *ent);
extern void SP_obj_firepot(edict_t *ent);
extern void SP_obj_fishhead1(edict_t *ent);
extern void SP_obj_fishhead2(edict_t *ent);
extern void SP_obj_fishtrap (edict_t *ent);
extern void SP_obj_flagonpole (edict_t *ent);
extern void SP_obj_floor_candelabrum(edict_t *ent);
extern void SP_obj_fountain_fish(edict_t *ent);
extern void SP_obj_frypan(edict_t *ent);
extern void SP_obj_gascan(edict_t *ent);
extern void SP_obj_gorgonbones(edict_t *ent);
extern void SP_obj_grass(edict_t *ent);
extern void SP_obj_hangingdude(edict_t *ent);
extern void SP_obj_hanging_ogle(edict_t *ent);
extern void SP_obj_hivepriestessssymbol (edict_t *self);
extern void SP_obj_jawbone(edict_t *ent);
extern void SP_obj_jug1(edict_t *ent);
extern void SP_obj_kettle(edict_t *ent);
extern void SP_obj_lab_parts_container(edict_t *ent);
extern void SP_obj_lab_tray(edict_t *ent);
extern void SP_obj_larva(edict_t *ent);
extern void SP_obj_larvabrokenegg(edict_t *ent);
extern void SP_obj_larvaegg(edict_t *ent);
extern void SP_obj_lever1 (edict_t *ent);
extern void SP_obj_lever2 (edict_t *ent);
extern void SP_obj_lever3 (edict_t *ent);
extern void SP_obj_metalchunk1(edict_t *ent);
extern void SP_obj_metalchunk2(edict_t *ent);
extern void SP_obj_metalchunk3(edict_t *ent);
extern void SP_obj_minecart(edict_t *ent);
extern void SP_obj_minecart2(edict_t *ent);
extern void SP_obj_minecart3(edict_t *ent);
extern void SP_obj_moss1(edict_t *self);
extern void SP_obj_moss2(edict_t *self);
extern void SP_obj_moss3(edict_t *self);
extern void SP_obj_moss4(edict_t *self);
extern void SP_obj_moss5(edict_t *self);
extern void SP_obj_nest(edict_t *self);
extern void SP_obj_pick(edict_t *ent);
extern void SP_obj_pipe1(edict_t *ent);
extern void SP_obj_pipe2(edict_t *ent);
extern void SP_obj_pipewheel(edict_t *ent);
extern void SP_obj_plant1(edict_t *ent);
extern void SP_obj_plant2(edict_t *ent);
extern void SP_obj_plant3(edict_t *ent);
extern void SP_obj_pot1(edict_t *ent);
extern void SP_obj_pot2(edict_t *ent);
extern void SP_obj_pottedplant(edict_t *ent);
extern void SP_obj_pushcart(edict_t *ent);
extern void SP_obj_queenchair(edict_t *ent);
extern void SP_obj_queenthrone(edict_t *ent);
extern void SP_obj_ring_plaque2 (edict_t *ent);
extern void SP_obj_rocks1(edict_t *ent);
extern void SP_obj_rocks2(edict_t *ent);
extern void SP_obj_rope(edict_t *ent);
extern void SP_obj_ropechain (edict_t *ent);
extern void SP_obj_scroll(edict_t *ent);
extern void SP_obj_seasonglobe(edict_t *ent);
extern void SP_obj_shovel(edict_t *ent);
extern void SP_obj_shrine(edict_t *ent);
extern void SP_obj_sign1(edict_t *ent);
extern void SP_obj_sign4(edict_t *ent);
extern void SP_obj_skullpole(edict_t *ent);
extern void SP_obj_spellbook(edict_t *ent);
extern void SP_obj_stalactite1(edict_t *ent);
extern void SP_obj_stalactite2(edict_t *ent);
extern void SP_obj_stalactite3(edict_t *ent);
extern void SP_obj_stalagmite1(edict_t *ent);
extern void SP_obj_stalagmite2(edict_t *ent);
extern void SP_obj_stalagmite3(edict_t *ent);
extern void SP_obj_statue_corvus (edict_t *ent);
extern void SP_obj_statue_boulderfish (edict_t *ent);
extern void SP_obj_statue_dolphin1(edict_t *ent);
extern void SP_obj_statue_dolphin2(edict_t *ent);
extern void SP_obj_statue_dolphin3(edict_t *ent);
extern void SP_obj_statue_dolphin4(edict_t *ent);
extern void SP_obj_statue_dragon (edict_t *ent);
extern void SP_obj_statue_dragonhead (edict_t *ent);
extern void SP_obj_statue_duckbill1(edict_t *ent);
extern void SP_obj_statue_duckbill2(edict_t *ent);
extern void SP_obj_statue_guardian(edict_t *ent);
extern void SP_obj_statue_saraphbust(edict_t *ent);
extern void SP_obj_statue_sariph(edict_t *ent);
extern void SP_obj_statue_sithraguard(edict_t *ent);
extern void SP_obj_statue_tchecktrik_bust (edict_t *self);
extern void SP_obj_statue_techeckrikleft (edict_t *self);
extern void SP_obj_statue_techeckrikright (edict_t *self);
extern void SP_obj_statue_techeckriktomb (edict_t *self);
extern void SP_obj_stein(edict_t *ent);
extern void SP_obj_swampflat_top(edict_t *ent);
extern void SP_obj_swampflat_bottom(edict_t *ent);
extern void SP_obj_table1(edict_t *ent);
extern void SP_obj_table2(edict_t *ent);
extern void SP_obj_tapper(edict_t *ent);
extern void SP_obj_throne (edict_t *ent);
extern void SP_obj_torture_bed (edict_t *ent);
extern void SP_obj_torture_ironmaiden (edict_t *ent);
extern void SP_obj_torture_rack (edict_t *ent);
extern void SP_obj_torture_table (edict_t *ent);
extern void SP_obj_torture_wallring (edict_t *ent);
extern void SP_obj_tree(edict_t *ent);
extern void SP_obj_tree2(edict_t *ent);
extern void SP_obj_tree3(edict_t *ent);
extern void SP_obj_treefallen(edict_t *ent);
extern void SP_obj_treestump(edict_t *ent);
extern void SP_obj_treetall(edict_t *ent);
extern void SP_obj_treetop(edict_t *ent);
extern void SP_obj_urn (edict_t *ent);
extern void SP_obj_venusflytrap(edict_t *ent);
extern void SP_obj_wallringplaque(edict_t *ent);
extern void SP_obj_web(edict_t *ent);
extern void SP_obj_wheelbarrow(edict_t *ent);
extern void SP_obj_wheelbarrowdamaged(edict_t *ent);
extern void SP_obj_woodpile(edict_t *ent);
extern void SP_obj_morcalavin_barrier(edict_t *ent);
extern void SP_flamethrower(edict_t *ent);
extern void SP_item_spitter(edict_t *ent);
// shrine stuff
extern void shrine_heal (edict_t *ent);
extern void shrine_armor (edict_t *ent);
extern void shrine_armor_gold (edict_t *ent);
extern void shrine_staff (edict_t *ent);
extern void shrine_lung (edict_t *ent);
extern void shrine_light (edict_t *ent);
extern void shrine_mana (edict_t *ent);
extern void shrine_ghost (edict_t *ent);
extern void shrine_reflect (edict_t *ent);
extern void shrine_powerup (edict_t *ent);
extern void shrine_random (edict_t *ent);
extern void shrine_speed (edict_t *ent);
extern void SP_script_runner (edict_t *ent);
