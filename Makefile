#
# Quake2 Makefile for Linux 2.0
#
# Nov '97 by Zoid <zoid@idsoftware.com>
#
# Elf only
#

# Detect the OS
ifdef SystemRoot
OSTYPE ?= Windows
else
OSTYPE ?= $(shell uname -s)
endif

# Special case for MinGW
ifneq (,$(findstring MINGW,$(OSTYPE)))
OSTYPE := Windows
endif

# Detect the architecture
ifeq ($(OSTYPE), Windows)
ifdef MINGW_CHOST
ifeq ($(MINGW_CHOST), x86_64-w64-mingw32)
ARCH ?= x86_64
else # i686-w64-mingw32
ARCH ?= i386
endif
else # windows, but MINGW_CHOST not defined
ifdef PROCESSOR_ARCHITEW6432
# 64 bit Windows
ARCH ?= $(PROCESSOR_ARCHITEW6432)
else
# 32 bit Windows
ARCH ?= $(PROCESSOR_ARCHITECTURE)
endif
endif # windows but MINGW_CHOST not defined
else
ifneq ($(OSTYPE), Darwin)
# Normalize some abiguous ARCH strings
ARCH ?= $(shell uname -m | sed -e 's/i.86/i386/' -e 's/amd64/x86_64/' -e 's/arm64/aarch64/' -e 's/^arm.*/arm/')
else
ARCH ?= $(shell uname -m)
endif
endif

NOARCH=noarch

QUAKE2_DIR=/grog/Projects/Quake2Master

BUILD_DEBUG_DIR=Debug

CC ?= gcc
CXX ?= g++
BASE_CFLAGS=-Dstricmp=strcasecmp

DEBUG_CFLAGS=$(BASE_CFLAGS) -g -fsanitize=address
LDFLAGS=-ldl -lm
SVGALDFLAGS=-lvga -lm
XLDFLAGS=-lX11 -lXext -lXxf86dga
XCFLAGS=

GLLDFLAGS=-lGL -lX11 -lXext -lvga -lm
GLXLDFLAGS=-lGL -lX11 -lXext -lXxf86dga -lXxf86vm -lm
GLCFLAGS=

SHLIBEXT=so

SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<
DO_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_SHLIB_CXX=$(CXX) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_GL_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) $(GLCFLAGS) -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

TARGETS= \
	$(BUILDDIR)/heretic2

all:
	-mkdir -p $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/client \
		$(BUILD_DEBUG_DIR)/client_effects \
		$(BUILD_DEBUG_DIR)/external \
		$(BUILD_DEBUG_DIR)/game \
		$(BUILD_DEBUG_DIR)/h2common \
		$(BUILD_DEBUG_DIR)/linux \
		$(BUILD_DEBUG_DIR)/player \
		$(BUILD_DEBUG_DIR)/qcommon \
		$(BUILD_DEBUG_DIR)/ref_common \
		$(BUILD_DEBUG_DIR)/ref_gl \
		$(BUILD_DEBUG_DIR)/server
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

targets: $(TARGETS)

HEADERS = \
	qcommon/q_shared.h

#############################################################################
# COMPILE
#############################################################################

$(BUILDDIR)/client/%.o :          client/%.c ${HEADERS}
	$(DO_CC)

$(BUILDDIR)/client_effects/%.o :  client_effects/%.c ${HEADERS}
	$(DO_SHLIB_CC)

$(BUILDDIR)/external/%.o :        external/%.c ${HEADERS}
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/%.o :            game/%.c ${HEADERS}
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/%.o :            game/%.cpp ${HEADERS}
	$(DO_SHLIB_CXX)

$(BUILDDIR)/h2common/%.o :        h2common/%.c ${HEADERS}
	$(DO_SHLIB_CC)

$(BUILDDIR)/linux/%.o :           linux/%.c ${HEADERS}
	$(DO_SHLIB_CC)

$(BUILDDIR)/player/%.o :          player/%.c ${HEADERS}
	$(DO_SHLIB_CC)

$(BUILDDIR)/qcommon/%.o :         qcommon/%.c ${HEADERS}
	$(DO_SHLIB_CC)

$(BUILDDIR)/ref_common/%.o :      ref_common/%.c ${HEADERS}
	$(DO_SHLIB_CC)

$(BUILDDIR)/ref_gl/%.o :          ref_gl/%.c ${HEADERS}
	$(DO_GL_SHLIB_CC)


$(BUILDDIR)/server/%.o :          server/%.c ${HEADERS}
	$(DO_CC)


#############################################################################
# CLIENT/SERVER
#############################################################################

HERETIC2_OBJS = \
	$(BUILDDIR)/client/cl_cin.o \
	$(BUILDDIR)/client/cl_ents.o \
	$(BUILDDIR)/client/cl_fx.o \
	$(BUILDDIR)/client/cl_input.o \
	$(BUILDDIR)/client/cl_inv.o \
	$(BUILDDIR)/client/cl_main.o \
	$(BUILDDIR)/client/cl_parse.o \
	$(BUILDDIR)/client/cl_pred.o \
	$(BUILDDIR)/client/cl_scrn.o \
	$(BUILDDIR)/client/cl_string.o \
	$(BUILDDIR)/client/cl_view.o \
	$(BUILDDIR)/client/console.o \
	$(BUILDDIR)/client_effects/ambient_effects.o \
	$(BUILDDIR)/client_effects/ce_default_message_handler.o \
	$(BUILDDIR)/client_effects/ce_dlight.o \
	$(BUILDDIR)/client_effects/ce_message.o \
	$(BUILDDIR)/client_effects/client_effects.o \
	$(BUILDDIR)/client_effects/client_entities.o \
	$(BUILDDIR)/client_effects/font1.o \
	$(BUILDDIR)/client_effects/font2.o \
	$(BUILDDIR)/client_effects/fx_ammo_pickup.o \
	$(BUILDDIR)/client_effects/fx_animate.o \
	$(BUILDDIR)/client_effects/fx_assassin.o \
	$(BUILDDIR)/client_effects/fx_blood.o \
	$(BUILDDIR)/client_effects/fx_blue_ring.o \
	$(BUILDDIR)/client_effects/fx_bubbler.o \
	$(BUILDDIR)/client_effects/fx_crosshair.o \
	$(BUILDDIR)/client_effects/fx_cwatcher.o \
	$(BUILDDIR)/client_effects/fx_debris.o \
	$(BUILDDIR)/client_effects/fx_defense_pickup.o \
	$(BUILDDIR)/client_effects/fx_dripper.o \
	$(BUILDDIR)/client_effects/fx_dust.o \
	$(BUILDDIR)/client_effects/fx_dustpuff.o \
	$(BUILDDIR)/client_effects/fx_firehands.o \
	$(BUILDDIR)/client_effects/fx_fire.o \
	$(BUILDDIR)/client_effects/fx_flamethrow.o \
	$(BUILDDIR)/client_effects/fx_flyingfist.o \
	$(BUILDDIR)/client_effects/fx_fountain.o \
	$(BUILDDIR)/client_effects/fx_halo.o \
	$(BUILDDIR)/client_effects/fx_health_pickup.o \
	$(BUILDDIR)/client_effects/fx_hell_staff.o \
	$(BUILDDIR)/client_effects/fx_hitpuff.o \
	$(BUILDDIR)/client_effects/fx_hpproj.o \
	$(BUILDDIR)/client_effects/fx_insectstaff.o \
	$(BUILDDIR)/client_effects/fx_lensflare.o \
	$(BUILDDIR)/client_effects/fx_lightning.o \
	$(BUILDDIR)/client_effects/fx_maceball.o \
	$(BUILDDIR)/client_effects/fx_magicmissile.o \
	$(BUILDDIR)/client_effects/fx_meteorbarrier.o \
	$(BUILDDIR)/client_effects/fx_mist.o \
	$(BUILDDIR)/client_effects/fx_mork.o \
	$(BUILDDIR)/client_effects/fx_morph.o \
	$(BUILDDIR)/client_effects/fx_objects.o \
	$(BUILDDIR)/client_effects/fx_pespell.o \
	$(BUILDDIR)/client_effects/fx_phoenix.o \
	$(BUILDDIR)/client_effects/fx_pickup.o \
	$(BUILDDIR)/client_effects/fx_pickuppuzzle.o \
	$(BUILDDIR)/client_effects/fx_plaguemistexplode.o \
	$(BUILDDIR)/client_effects/fx_plaguemist.o \
	$(BUILDDIR)/client_effects/fx_portal.o \
	$(BUILDDIR)/client_effects/fx_quake.o \
	$(BUILDDIR)/client_effects/fx_redrainglow.o \
	$(BUILDDIR)/client_effects/fx_redrain.o \
	$(BUILDDIR)/client_effects/fx_remotecamera.o \
	$(BUILDDIR)/client_effects/fx_ripples.o \
	$(BUILDDIR)/client_effects/fx_rope.o \
	$(BUILDDIR)/client_effects/fx_scorchmark.o \
	$(BUILDDIR)/client_effects/fx_shadow.o \
	$(BUILDDIR)/client_effects/fx_shield.o \
	$(BUILDDIR)/client_effects/fx_shrine.o \
	$(BUILDDIR)/client_effects/fx_smoke.o \
	$(BUILDDIR)/client_effects/fx_sound.o \
	$(BUILDDIR)/client_effects/fx_sparks.o \
	$(BUILDDIR)/client_effects/fx_spellchange.o \
	$(BUILDDIR)/client_effects/fx_spellhands.o \
	$(BUILDDIR)/client_effects/fx_sphereofannihlation.o \
	$(BUILDDIR)/client_effects/fx_spoo.o \
	$(BUILDDIR)/client_effects/fx_ssarrow.o \
	$(BUILDDIR)/client_effects/fx_ssithra.o \
	$(BUILDDIR)/client_effects/fx_staff.o \
	$(BUILDDIR)/client_effects/fx_tbeast.o \
	$(BUILDDIR)/client_effects/fx_teleport.o \
	$(BUILDDIR)/client_effects/fx_tome.o \
	$(BUILDDIR)/client_effects/fx_tornado.o \
	$(BUILDDIR)/client_effects/fx_wall.o \
	$(BUILDDIR)/client_effects/fx_waterentrysplash.o \
	$(BUILDDIR)/client_effects/fx_waterwake.o \
	$(BUILDDIR)/client_effects/fx_weaponpickup.o \
	$(BUILDDIR)/client_effects/generic_character_effects.o \
	$(BUILDDIR)/client_effects/generic_weapon_effects.o \
	$(BUILDDIR)/client_effects/item_effects.o \
	$(BUILDDIR)/client_effects/level_maps.o \
	$(BUILDDIR)/client_effects/light_styles.o \
	$(BUILDDIR)/client_effects/main.o \
	$(BUILDDIR)/client_effects/particle.o \
	$(BUILDDIR)/client_effects/player_effects.o \
	$(BUILDDIR)/client_effects/test_effect.o \
	$(BUILDDIR)/client_effects/utilities.o \
	$(BUILDDIR)/client/keys.o \
	$(BUILDDIR)/client/menu.o \
	$(BUILDDIR)/client/qmenu.o \
	$(BUILDDIR)/external/smacker.o \
	$(BUILDDIR)/external/smk_bitstream.o \
	$(BUILDDIR)/external/smk_hufftree.o \
	$(BUILDDIR)/game/buoy.o \
	$(BUILDDIR)/game/c_ai.o \
	$(BUILDDIR)/game/c_corvus1_anim.o \
	$(BUILDDIR)/game/c_corvus1.o \
	$(BUILDDIR)/game/c_corvus2_anim.o \
	$(BUILDDIR)/game/c_corvus2.o \
	$(BUILDDIR)/game/c_corvus3_anim.o \
	$(BUILDDIR)/game/c_corvus3.o \
	$(BUILDDIR)/game/c_corvus4_anim.o \
	$(BUILDDIR)/game/c_corvus4.o \
	$(BUILDDIR)/game/c_corvus5_anim.o \
	$(BUILDDIR)/game/c_corvus5.o \
	$(BUILDDIR)/game/c_corvus6_anim.o \
	$(BUILDDIR)/game/c_corvus6.o \
	$(BUILDDIR)/game/c_corvus7_anim.o \
	$(BUILDDIR)/game/c_corvus7.o \
	$(BUILDDIR)/game/c_corvus8_anim.o \
	$(BUILDDIR)/game/c_corvus8.o \
	$(BUILDDIR)/game/c_corvus9_anim.o \
	$(BUILDDIR)/game/c_corvus9.o \
	$(BUILDDIR)/game/c_dranor_anim.o \
	$(BUILDDIR)/game/c_dranor.o \
	$(BUILDDIR)/game/c_elflord_anim.o \
	$(BUILDDIR)/game/c_elflord.o \
	$(BUILDDIR)/game/c_morcalavin_anim.o \
	$(BUILDDIR)/game/c_morcalavin.o \
	$(BUILDDIR)/game/c_priestess2_anim.o \
	$(BUILDDIR)/game/c_priestess2.o \
	$(BUILDDIR)/game/c_priestess_anim.o \
	$(BUILDDIR)/game/c_priestess.o \
	$(BUILDDIR)/game/c_siernan1_anim.o \
	$(BUILDDIR)/game/c_siernan1.o \
	$(BUILDDIR)/game/c_siernan2_anim.o \
	$(BUILDDIR)/game/c_siernan2.o \
	$(BUILDDIR)/game/c_ssithrascout_anim.o \
	$(BUILDDIR)/game/c_ssithrascout.o \
	$(BUILDDIR)/game/c_tome_anim.o \
	$(BUILDDIR)/game/c_tome.o \
	$(BUILDDIR)/game/c_victimssithra_anim.o \
	$(BUILDDIR)/game/c_victimssithra.o \
	$(BUILDDIR)/game/decals.o \
	$(BUILDDIR)/game/ds.o \
	$(BUILDDIR)/game/g_ai.o \
	$(BUILDDIR)/game/game_utilities.o \
	$(BUILDDIR)/game/g_breakable.o \
	$(BUILDDIR)/game/g_classstatics.o \
	$(BUILDDIR)/game/g_cmds.o \
	$(BUILDDIR)/game/g_combat.o \
	$(BUILDDIR)/game/g_defaultmessagehandler.o \
	$(BUILDDIR)/game/g_env.o \
	$(BUILDDIR)/game/g_field.o \
	$(BUILDDIR)/game/g_flamethrow.o \
	$(BUILDDIR)/game/g_func.o \
	$(BUILDDIR)/game/g_hitlocation.o \
	$(BUILDDIR)/game/g_items.o \
	$(BUILDDIR)/game/g_items_pickups.o \
	$(BUILDDIR)/game/g_light.o \
	$(BUILDDIR)/game/g_main.o \
	$(BUILDDIR)/game/g_message.o \
	$(BUILDDIR)/game/g_misc.o \
	$(BUILDDIR)/game/g_monster.o \
	$(BUILDDIR)/game/g_moveinfo.o \
	$(BUILDDIR)/game/g_obj.o \
	$(BUILDDIR)/game/g_physics.o \
	$(BUILDDIR)/game/g_phys.o \
	$(BUILDDIR)/game/g_resourcemanagers.o \
	$(BUILDDIR)/game/g_rope.o \
	$(BUILDDIR)/game/g_save.o \
	$(BUILDDIR)/game/g_shrine.o \
	$(BUILDDIR)/game/g_skeletons.o \
	$(BUILDDIR)/game/g_spawnf.o \
	$(BUILDDIR)/game/g_spawn.o \
	$(BUILDDIR)/game/g_stateinfo.o \
	$(BUILDDIR)/game/g_svcmds.o \
	$(BUILDDIR)/game/g_target.o \
	$(BUILDDIR)/game/g_trigger.o \
	$(BUILDDIR)/game/g_utils.o \
	$(BUILDDIR)/game/g_waterfx.o \
	$(BUILDDIR)/game/g_weapon.o \
	$(BUILDDIR)/game/m_assassin_anim.o \
	$(BUILDDIR)/game/m_assassin.o \
	$(BUILDDIR)/game/m_beast_anim.o \
	$(BUILDDIR)/game/m_beast.o \
	$(BUILDDIR)/game/m_bee.o \
	$(BUILDDIR)/game/m_chicken_anim.o \
	$(BUILDDIR)/game/m_chicken.o \
	$(BUILDDIR)/game/m_elflord_anims.o \
	$(BUILDDIR)/game/m_elflord.o \
	$(BUILDDIR)/game/m_fish_anim.o \
	$(BUILDDIR)/game/m_fish.o \
	$(BUILDDIR)/game/m_fmtest.o \
	$(BUILDDIR)/game/mg_ai.o \
	$(BUILDDIR)/game/mg_guide.o \
	$(BUILDDIR)/game/m_gkrokon_anim.o \
	$(BUILDDIR)/game/m_gkrokon.o \
	$(BUILDDIR)/game/m_gorgon_anim.o \
	$(BUILDDIR)/game/m_gorgon.o \
	$(BUILDDIR)/game/m_harpy_anim.o \
	$(BUILDDIR)/game/m_harpy.o \
	$(BUILDDIR)/game/m_imp_anim.o \
	$(BUILDDIR)/game/m_imp.o \
	$(BUILDDIR)/game/m_morcalavin_anim.o \
	$(BUILDDIR)/game/m_morcalavin.o \
	$(BUILDDIR)/game/m_mother_anim.o \
	$(BUILDDIR)/game/m_mother.o \
	$(BUILDDIR)/game/m_move.o \
	$(BUILDDIR)/game/m_mssithra_anim.o \
	$(BUILDDIR)/game/m_mssithra.o \
	$(BUILDDIR)/game/m_ogle_anim.o \
	$(BUILDDIR)/game/m_ogle.o \
	$(BUILDDIR)/game/m_plagueelf_anim.o \
	$(BUILDDIR)/game/m_plagueelf.o \
	$(BUILDDIR)/game/m_plaguessithra_anim.o \
	$(BUILDDIR)/game/m_plaguessithra.o \
	$(BUILDDIR)/game/m_priestess_anim.o \
	$(BUILDDIR)/game/m_priestess.o \
	$(BUILDDIR)/game/m_rat_anim.o \
	$(BUILDDIR)/game/m_rat.o \
	$(BUILDDIR)/game/m_seraph_anim.o \
	$(BUILDDIR)/game/m_seraph_guard_anim.o \
	$(BUILDDIR)/game/m_seraph_guard.o \
	$(BUILDDIR)/game/m_seraph.o \
	$(BUILDDIR)/game/m_spreader_anim.o \
	$(BUILDDIR)/game/m_spreadermist.o \
	$(BUILDDIR)/game/m_spreader.o \
	$(BUILDDIR)/game/m_stats.o \
	$(BUILDDIR)/game/m_tcheckrik_anim.o \
	$(BUILDDIR)/game/m_tcheckrik.o \
	$(BUILDDIR)/game/m_tcheckrik_spells.o \
	$(BUILDDIR)/game/p_client.o \
	$(BUILDDIR)/game/p_funcs.o \
	$(BUILDDIR)/game/p_hud.o \
	$(BUILDDIR)/game/p_item.o \
	$(BUILDDIR)/game/p_view.o \
	$(BUILDDIR)/game/script.o \
	$(BUILDDIR)/game/spl_blast.o \
	$(BUILDDIR)/game/spl_bluering.o \
	$(BUILDDIR)/game/spl_flyingfist.o \
	$(BUILDDIR)/game/spl_hellstaff.o \
	$(BUILDDIR)/game/spl_maceballs.o \
	$(BUILDDIR)/game/spl_magicmissile.o \
	$(BUILDDIR)/game/spl_meteorbarrier.o \
	$(BUILDDIR)/game/spl_morph.o \
	$(BUILDDIR)/game/spl_phoenix.o \
	$(BUILDDIR)/game/spl_powerup.o \
	$(BUILDDIR)/game/spl_redrain.o \
	$(BUILDDIR)/game/spl_ripper.o \
	$(BUILDDIR)/game/spl_shield.o \
	$(BUILDDIR)/game/spl_sphereofannihlation.o \
	$(BUILDDIR)/game/spl_teleport.o \
	$(BUILDDIR)/game/spl_tornado.o \
	$(BUILDDIR)/game/spl_wall.o \
	$(BUILDDIR)/h2common/h2math.o \
	$(BUILDDIR)/h2common/h2matrix.o \
	$(BUILDDIR)/h2common/h2motion.o \
	$(BUILDDIR)/h2common/h2palette.o \
	$(BUILDDIR)/h2common/h2physics.o \
	$(BUILDDIR)/h2common/h2singlylinkedlist.o \
	$(BUILDDIR)/h2common/h2surfaces.o \
	$(BUILDDIR)/h2common/h2vector.o \
	$(BUILDDIR)/linux/gl_glx.o \
	$(BUILDDIR)/linux/glob.o \
	$(BUILDDIR)/linux/in_linux.o \
	$(BUILDDIR)/linux/net_udp.o \
	$(BUILDDIR)/linux/p_dll.o \
	$(BUILDDIR)/linux/qgl_linux.o \
	$(BUILDDIR)/linux/q_shlinux.o \
	$(BUILDDIR)/linux/rw_x11.o \
	$(BUILDDIR)/linux/snd_linux.o \
	$(BUILDDIR)/linux/sys_linux.o \
	$(BUILDDIR)/linux/vid_menu.o \
	$(BUILDDIR)/linux/vid_so.o \
	$(BUILDDIR)/player/p_actions.o \
	$(BUILDDIR)/player/p_animactor.o \
	$(BUILDDIR)/player/p_anim_branch.o \
	$(BUILDDIR)/player/p_anim_data.o \
	$(BUILDDIR)/player/p_anims.o \
	$(BUILDDIR)/player/p_chicken_anim.o \
	$(BUILDDIR)/player/p_chicken.o \
	$(BUILDDIR)/player/p_ctrl.o \
	$(BUILDDIR)/player/p_items.o \
	$(BUILDDIR)/player/player_main.o \
	$(BUILDDIR)/player/p_main.o \
	$(BUILDDIR)/player/p_weapon.o \
	$(BUILDDIR)/qcommon/cmd.o \
	$(BUILDDIR)/qcommon/cmodel.o \
	$(BUILDDIR)/qcommon/common.o \
	$(BUILDDIR)/qcommon/crc.o \
	$(BUILDDIR)/qcommon/cvar.o \
	$(BUILDDIR)/qcommon/files.o \
	$(BUILDDIR)/qcommon/md4.o \
	$(BUILDDIR)/qcommon/message.o \
	$(BUILDDIR)/qcommon/net_chan.o \
	$(BUILDDIR)/qcommon/netmsg_read.o \
	$(BUILDDIR)/qcommon/p_dll.o \
	$(BUILDDIR)/qcommon/pmove.o \
	$(BUILDDIR)/qcommon/q_shared.o \
	$(BUILDDIR)/qcommon/rand.o \
	$(BUILDDIR)/qcommon/reference.o \
	$(BUILDDIR)/qcommon/resource_manager.o \
	$(BUILDDIR)/qcommon/skeletons.o \
	$(BUILDDIR)/ref_common/font1.o \
	$(BUILDDIR)/ref_common/r_skeletons.o \
	$(BUILDDIR)/ref_gl/gl_book.o \
	$(BUILDDIR)/ref_gl/gl_draw.o \
	$(BUILDDIR)/ref_gl/glew.o \
	$(BUILDDIR)/ref_gl/gl_fmodel.o \
	$(BUILDDIR)/ref_gl/gl_image.o \
	$(BUILDDIR)/ref_gl/gl_light.o \
	$(BUILDDIR)/ref_gl/gl_math.o \
	$(BUILDDIR)/ref_gl/gl_mesh.o \
	$(BUILDDIR)/ref_gl/gl_model.o \
	$(BUILDDIR)/ref_gl/gl_movie.o \
	$(BUILDDIR)/ref_gl/gl_rmain.o \
	$(BUILDDIR)/ref_gl/gl_rmisc.o \
	$(BUILDDIR)/ref_gl/gl_rsurf.o \
	$(BUILDDIR)/ref_gl/gl_warp.o \
	$(BUILDDIR)/server/sv_ccmds.o \
	$(BUILDDIR)/server/sv_ents.o \
	$(BUILDDIR)/server/sv_game.o \
	$(BUILDDIR)/server/sv_init.o \
	$(BUILDDIR)/server/sv_main.o \
	$(BUILDDIR)/server/sv_null.o \
	$(BUILDDIR)/server/sv_send.o \
	$(BUILDDIR)/server/sv_user.o \
	$(BUILDDIR)/server/sv_world.o

#	$(BUILDDIR)/ref_soft/r_aclip.o \
#	$(BUILDDIR)/ref_soft/r_alias.o \
#	$(BUILDDIR)/ref_soft/r_bsp.o \
#	$(BUILDDIR)/ref_soft/r_draw.o \
#	$(BUILDDIR)/ref_soft/r_edge.o \
#	$(BUILDDIR)/ref_soft/r_image.o \
#	$(BUILDDIR)/ref_soft/r_light.o \
#	$(BUILDDIR)/ref_soft/r_main.o \
#	$(BUILDDIR)/ref_soft/r_misc.o \
#	$(BUILDDIR)/ref_soft/r_model.o \
#	$(BUILDDIR)/ref_soft/r_part.o \
#	$(BUILDDIR)/ref_soft/r_poly.o \
#	$(BUILDDIR)/ref_soft/r_polyse.o \
#	$(BUILDDIR)/ref_soft/r_rast.o \
#	$(BUILDDIR)/ref_soft/r_scan.o \
#	$(BUILDDIR)/ref_soft/r_sprite.o \
#	$(BUILDDIR)/ref_soft/r_surf.o \

$(BUILDDIR)/heretic2 : $(HERETIC2_OBJS) ${HEADERS}
	$(CC) $(CFLAGS) -o $@ $(HERETIC2_OBJS) $(LDFLAGS) $(GLXLDFLAGS)

##########################################################################
# MISC
#############################################################################

clean:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f \
	$(HERETIC2_OBJS)
