#
# Quake2 Makefile for Linux 2.0
#
# Nov '97 by Zoid <zoid@idsoftware.com>
#
# ELF only
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

BUILD_DEBUG_DIR=build

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

# Detect the OS
ifdef SystemRoot
YQ2_OSTYPE ?= Windows
else
YQ2_OSTYPE ?= $(shell uname -s)
endif

# Special case for MinGW
ifneq (,$(findstring MINGW,$(YQ2_OSTYPE)))
YQ2_OSTYPE := Windows
endif

# Detect the architecture
ifeq ($(YQ2_OSTYPE), Windows)
ifdef MINGW_CHOST
ifeq ($(MINGW_CHOST), x86_64-w64-mingw32)
YQ2_ARCH ?= x86_64
else # i686-w64-mingw32
YQ2_ARCH ?= i386
endif
else # windows, but MINGW_CHOST not defined
ifdef PROCESSOR_ARCHITEW6432
# 64 bit Windows
YQ2_ARCH ?= $(PROCESSOR_ARCHITEW6432)
else
# 32 bit Windows
YQ2_ARCH ?= $(PROCESSOR_ARCHITECTURE)
endif
endif # windows but MINGW_CHOST not defined
else
ifneq ($(YQ2_OSTYPE), Darwin)
# Normalize some abiguous YQ2_ARCH strings
YQ2_ARCH ?= $(shell uname -m | sed -e 's/i.86/i386/' -e 's/amd64/x86_64/' -e 's/arm64/aarch64/' -e 's/^arm.*/arm/')
else
YQ2_ARCH ?= $(shell uname -m)
endif
endif

# Defines the operating system and architecture
override CFLAGS += -DYQ2OSTYPE=\"$(YQ2_OSTYPE)\" -DYQ2ARCH=\"$(YQ2_ARCH)\"

DO_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_SHLIB_CXX=$(CXX) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_GL_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) $(GLCFLAGS) -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

TARGETS= \
	$(BUILDDIR)/heretic2 \
	$(BUILDDIR)/ref_gl.$(SHLIBEXT)
#	$(BUILDDIR)/ref_softx.$(SHLIBEXT)

all:
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

targets: $(TARGETS)

HEADERS = \
	src/common/header/shared.h

# When make is invoked by "make VERBOSE=1" print
# the compiler and linker commands.
ifdef VERBOSE
Q :=
else
Q := @
endif

#############################################################################
# COMPILE
#############################################################################

$(BUILDDIR)/%.o :                 %.cpp ${HEADERS}
	@echo "===> CXX $<"
	${Q}mkdir -p $(@D)
	${Q}$(DO_SHLIB_CXX)

$(BUILDDIR)/%.o :                 %.c ${HEADERS}
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(DO_CC)


#############################################################################
# CLIENT/SERVER
#############################################################################

HERETIC2_OBJS = \
	$(BUILDDIR)/src/client/cl_cin.o \
	$(BUILDDIR)/src/client/cl_ents.o \
	$(BUILDDIR)/src/client/cl_fx.o \
	$(BUILDDIR)/src/client/cl_input.o \
	$(BUILDDIR)/src/client/cl_inv.o \
	$(BUILDDIR)/src/client/cl_main.o \
	$(BUILDDIR)/src/client/cl_parse.o \
	$(BUILDDIR)/src/client/cl_pred.o \
	$(BUILDDIR)/src/client/cl_scrn.o \
	$(BUILDDIR)/src/client/cl_string.o \
	$(BUILDDIR)/src/client/cl_view.o \
	$(BUILDDIR)/src/client/snd_mem.o \
	$(BUILDDIR)/src/client/snd_mix.o \
	$(BUILDDIR)/src/client/snd_dma.o \
	$(BUILDDIR)/src/client/cl_console.o \
	$(BUILDDIR)/src/client_effects/ambient_effects.o \
	$(BUILDDIR)/src/client_effects/ce_default_message_handler.o \
	$(BUILDDIR)/src/client_effects/ce_dlight.o \
	$(BUILDDIR)/src/client_effects/ce_message.o \
	$(BUILDDIR)/src/client_effects/client_effects.o \
	$(BUILDDIR)/src/client_effects/client_entities.o \
	$(BUILDDIR)/src/client_effects/font1.o \
	$(BUILDDIR)/src/client_effects/font2.o \
	$(BUILDDIR)/src/client_effects/fx_ammo_pickup.o \
	$(BUILDDIR)/src/client_effects/fx_animate.o \
	$(BUILDDIR)/src/client_effects/fx_assassin.o \
	$(BUILDDIR)/src/client_effects/fx_blood.o \
	$(BUILDDIR)/src/client_effects/fx_blue_ring.o \
	$(BUILDDIR)/src/client_effects/fx_bubbler.o \
	$(BUILDDIR)/src/client_effects/fx_crosshair.o \
	$(BUILDDIR)/src/client_effects/fx_cwatcher.o \
	$(BUILDDIR)/src/client_effects/fx_debris.o \
	$(BUILDDIR)/src/client_effects/fx_defense_pickup.o \
	$(BUILDDIR)/src/client_effects/fx_dripper.o \
	$(BUILDDIR)/src/client_effects/fx_dust.o \
	$(BUILDDIR)/src/client_effects/fx_dustpuff.o \
	$(BUILDDIR)/src/client_effects/fx_firehands.o \
	$(BUILDDIR)/src/client_effects/fx_fire.o \
	$(BUILDDIR)/src/client_effects/fx_flamethrow.o \
	$(BUILDDIR)/src/client_effects/fx_flyingfist.o \
	$(BUILDDIR)/src/client_effects/fx_fountain.o \
	$(BUILDDIR)/src/client_effects/fx_halo.o \
	$(BUILDDIR)/src/client_effects/fx_health_pickup.o \
	$(BUILDDIR)/src/client_effects/fx_hell_staff.o \
	$(BUILDDIR)/src/client_effects/fx_hitpuff.o \
	$(BUILDDIR)/src/client_effects/fx_hpproj.o \
	$(BUILDDIR)/src/client_effects/fx_insectstaff.o \
	$(BUILDDIR)/src/client_effects/fx_lensflare.o \
	$(BUILDDIR)/src/client_effects/fx_lightning.o \
	$(BUILDDIR)/src/client_effects/fx_maceball.o \
	$(BUILDDIR)/src/client_effects/fx_magicmissile.o \
	$(BUILDDIR)/src/client_effects/fx_meteorbarrier.o \
	$(BUILDDIR)/src/client_effects/fx_mist.o \
	$(BUILDDIR)/src/client_effects/fx_mork.o \
	$(BUILDDIR)/src/client_effects/fx_morph.o \
	$(BUILDDIR)/src/client_effects/fx_objects.o \
	$(BUILDDIR)/src/client_effects/fx_pespell.o \
	$(BUILDDIR)/src/client_effects/fx_phoenix.o \
	$(BUILDDIR)/src/client_effects/fx_pickup.o \
	$(BUILDDIR)/src/client_effects/fx_pickuppuzzle.o \
	$(BUILDDIR)/src/client_effects/fx_plaguemistexplode.o \
	$(BUILDDIR)/src/client_effects/fx_plaguemist.o \
	$(BUILDDIR)/src/client_effects/fx_portal.o \
	$(BUILDDIR)/src/client_effects/fx_quake.o \
	$(BUILDDIR)/src/client_effects/fx_redrainglow.o \
	$(BUILDDIR)/src/client_effects/fx_redrain.o \
	$(BUILDDIR)/src/client_effects/fx_remotecamera.o \
	$(BUILDDIR)/src/client_effects/fx_ripples.o \
	$(BUILDDIR)/src/client_effects/fx_rope.o \
	$(BUILDDIR)/src/client_effects/fx_scorchmark.o \
	$(BUILDDIR)/src/client_effects/fx_shadow.o \
	$(BUILDDIR)/src/client_effects/fx_shield.o \
	$(BUILDDIR)/src/client_effects/fx_shrine.o \
	$(BUILDDIR)/src/client_effects/fx_smoke.o \
	$(BUILDDIR)/src/client_effects/fx_sound.o \
	$(BUILDDIR)/src/client_effects/fx_sparks.o \
	$(BUILDDIR)/src/client_effects/fx_spellchange.o \
	$(BUILDDIR)/src/client_effects/fx_spellhands.o \
	$(BUILDDIR)/src/client_effects/fx_sphereofannihlation.o \
	$(BUILDDIR)/src/client_effects/fx_spoo.o \
	$(BUILDDIR)/src/client_effects/fx_ssarrow.o \
	$(BUILDDIR)/src/client_effects/fx_ssithra.o \
	$(BUILDDIR)/src/client_effects/fx_staff.o \
	$(BUILDDIR)/src/client_effects/fx_tbeast.o \
	$(BUILDDIR)/src/client_effects/fx_teleport.o \
	$(BUILDDIR)/src/client_effects/fx_tome.o \
	$(BUILDDIR)/src/client_effects/fx_tornado.o \
	$(BUILDDIR)/src/client_effects/fx_wall.o \
	$(BUILDDIR)/src/client_effects/fx_waterentrysplash.o \
	$(BUILDDIR)/src/client_effects/fx_waterwake.o \
	$(BUILDDIR)/src/client_effects/fx_weaponpickup.o \
	$(BUILDDIR)/src/client_effects/generic_character_effects.o \
	$(BUILDDIR)/src/client_effects/generic_weapon_effects.o \
	$(BUILDDIR)/src/client_effects/item_effects.o \
	$(BUILDDIR)/src/client_effects/level_maps.o \
	$(BUILDDIR)/src/client_effects/light_styles.o \
	$(BUILDDIR)/src/client_effects/main.o \
	$(BUILDDIR)/src/client_effects/particle.o \
	$(BUILDDIR)/src/client_effects/player_effects.o \
	$(BUILDDIR)/src/client_effects/test_effect.o \
	$(BUILDDIR)/src/client_effects/utilities.o \
	$(BUILDDIR)/src/client/cl_keyboard.o \
	$(BUILDDIR)/src/client/menu/menu.o \
	$(BUILDDIR)/src/client/menu/qmenu.o \
	$(BUILDDIR)/external/smacker.o \
	$(BUILDDIR)/external/smk_bitstream.o \
	$(BUILDDIR)/external/smk_hufftree.o \
	$(BUILDDIR)/src/game/buoy.o \
	$(BUILDDIR)/src/game/c_ai.o \
	$(BUILDDIR)/src/game/c_corvus1_anim.o \
	$(BUILDDIR)/src/game/c_corvus1.o \
	$(BUILDDIR)/src/game/c_corvus2_anim.o \
	$(BUILDDIR)/src/game/c_corvus2.o \
	$(BUILDDIR)/src/game/c_corvus3_anim.o \
	$(BUILDDIR)/src/game/c_corvus3.o \
	$(BUILDDIR)/src/game/c_corvus4_anim.o \
	$(BUILDDIR)/src/game/c_corvus4.o \
	$(BUILDDIR)/src/game/c_corvus5_anim.o \
	$(BUILDDIR)/src/game/c_corvus5.o \
	$(BUILDDIR)/src/game/c_corvus6_anim.o \
	$(BUILDDIR)/src/game/c_corvus6.o \
	$(BUILDDIR)/src/game/c_corvus7_anim.o \
	$(BUILDDIR)/src/game/c_corvus7.o \
	$(BUILDDIR)/src/game/c_corvus8_anim.o \
	$(BUILDDIR)/src/game/c_corvus8.o \
	$(BUILDDIR)/src/game/c_corvus9_anim.o \
	$(BUILDDIR)/src/game/c_corvus9.o \
	$(BUILDDIR)/src/game/c_dranor_anim.o \
	$(BUILDDIR)/src/game/c_dranor.o \
	$(BUILDDIR)/src/game/c_elflord_anim.o \
	$(BUILDDIR)/src/game/c_elflord.o \
	$(BUILDDIR)/src/game/c_morcalavin_anim.o \
	$(BUILDDIR)/src/game/c_morcalavin.o \
	$(BUILDDIR)/src/game/c_priestess2_anim.o \
	$(BUILDDIR)/src/game/c_priestess2.o \
	$(BUILDDIR)/src/game/c_priestess_anim.o \
	$(BUILDDIR)/src/game/c_priestess.o \
	$(BUILDDIR)/src/game/c_siernan1_anim.o \
	$(BUILDDIR)/src/game/c_siernan1.o \
	$(BUILDDIR)/src/game/c_siernan2_anim.o \
	$(BUILDDIR)/src/game/c_siernan2.o \
	$(BUILDDIR)/src/game/c_ssithrascout_anim.o \
	$(BUILDDIR)/src/game/c_ssithrascout.o \
	$(BUILDDIR)/src/game/c_tome_anim.o \
	$(BUILDDIR)/src/game/c_tome.o \
	$(BUILDDIR)/src/game/c_victimssithra_anim.o \
	$(BUILDDIR)/src/game/c_victimssithra.o \
	$(BUILDDIR)/src/game/decals.o \
	$(BUILDDIR)/src/game/ds.o \
	$(BUILDDIR)/src/game/g_ai.o \
	$(BUILDDIR)/src/game/game_utilities.o \
	$(BUILDDIR)/src/game/g_breakable.o \
	$(BUILDDIR)/src/game/g_classstatics.o \
	$(BUILDDIR)/src/game/g_cmds.o \
	$(BUILDDIR)/src/game/g_combat.o \
	$(BUILDDIR)/src/game/g_defaultmessagehandler.o \
	$(BUILDDIR)/src/game/g_env.o \
	$(BUILDDIR)/src/game/g_field.o \
	$(BUILDDIR)/src/game/g_flamethrow.o \
	$(BUILDDIR)/src/game/g_func.o \
	$(BUILDDIR)/src/game/g_hitlocation.o \
	$(BUILDDIR)/src/game/g_items.o \
	$(BUILDDIR)/src/game/g_items_pickups.o \
	$(BUILDDIR)/src/game/g_light.o \
	$(BUILDDIR)/src/game/g_main.o \
	$(BUILDDIR)/src/game/g_message.o \
	$(BUILDDIR)/src/game/g_misc.o \
	$(BUILDDIR)/src/game/g_monster.o \
	$(BUILDDIR)/src/game/g_moveinfo.o \
	$(BUILDDIR)/src/game/g_obj.o \
	$(BUILDDIR)/src/game/g_physics.o \
	$(BUILDDIR)/src/game/g_phys.o \
	$(BUILDDIR)/src/game/g_resourcemanagers.o \
	$(BUILDDIR)/src/game/g_rope.o \
	$(BUILDDIR)/src/game/g_save.o \
	$(BUILDDIR)/src/game/g_shrine.o \
	$(BUILDDIR)/src/game/g_skeletons.o \
	$(BUILDDIR)/src/game/g_spawnf.o \
	$(BUILDDIR)/src/game/g_spawn.o \
	$(BUILDDIR)/src/game/g_stateinfo.o \
	$(BUILDDIR)/src/game/g_svcmds.o \
	$(BUILDDIR)/src/game/g_target.o \
	$(BUILDDIR)/src/game/g_trigger.o \
	$(BUILDDIR)/src/game/g_utils.o \
	$(BUILDDIR)/src/game/g_waterfx.o \
	$(BUILDDIR)/src/game/g_weapon.o \
	$(BUILDDIR)/src/game/m_assassin_anim.o \
	$(BUILDDIR)/src/game/m_assassin.o \
	$(BUILDDIR)/src/game/m_beast_anim.o \
	$(BUILDDIR)/src/game/m_beast.o \
	$(BUILDDIR)/src/game/m_bee.o \
	$(BUILDDIR)/src/game/m_chicken_anim.o \
	$(BUILDDIR)/src/game/m_chicken.o \
	$(BUILDDIR)/src/game/m_elflord_anims.o \
	$(BUILDDIR)/src/game/m_elflord.o \
	$(BUILDDIR)/src/game/m_fish_anim.o \
	$(BUILDDIR)/src/game/m_fish.o \
	$(BUILDDIR)/src/game/m_fmtest.o \
	$(BUILDDIR)/src/game/mg_ai.o \
	$(BUILDDIR)/src/game/mg_guide.o \
	$(BUILDDIR)/src/game/m_gkrokon_anim.o \
	$(BUILDDIR)/src/game/m_gkrokon.o \
	$(BUILDDIR)/src/game/m_gorgon_anim.o \
	$(BUILDDIR)/src/game/m_gorgon.o \
	$(BUILDDIR)/src/game/m_harpy_anim.o \
	$(BUILDDIR)/src/game/m_harpy.o \
	$(BUILDDIR)/src/game/m_imp_anim.o \
	$(BUILDDIR)/src/game/m_imp.o \
	$(BUILDDIR)/src/game/m_morcalavin_anim.o \
	$(BUILDDIR)/src/game/m_morcalavin.o \
	$(BUILDDIR)/src/game/m_mother_anim.o \
	$(BUILDDIR)/src/game/m_mother.o \
	$(BUILDDIR)/src/game/m_move.o \
	$(BUILDDIR)/src/game/m_mssithra_anim.o \
	$(BUILDDIR)/src/game/m_mssithra.o \
	$(BUILDDIR)/src/game/m_ogle_anim.o \
	$(BUILDDIR)/src/game/m_ogle.o \
	$(BUILDDIR)/src/game/m_plagueelf_anim.o \
	$(BUILDDIR)/src/game/m_plagueelf.o \
	$(BUILDDIR)/src/game/m_plaguessithra_anim.o \
	$(BUILDDIR)/src/game/m_plaguessithra.o \
	$(BUILDDIR)/src/game/m_priestess_anim.o \
	$(BUILDDIR)/src/game/m_priestess.o \
	$(BUILDDIR)/src/game/m_rat_anim.o \
	$(BUILDDIR)/src/game/m_rat.o \
	$(BUILDDIR)/src/game/m_seraph_anim.o \
	$(BUILDDIR)/src/game/m_seraph_guard_anim.o \
	$(BUILDDIR)/src/game/m_seraph_guard.o \
	$(BUILDDIR)/src/game/m_seraph.o \
	$(BUILDDIR)/src/game/m_spreader_anim.o \
	$(BUILDDIR)/src/game/m_spreadermist.o \
	$(BUILDDIR)/src/game/m_spreader.o \
	$(BUILDDIR)/src/game/m_stats.o \
	$(BUILDDIR)/src/game/m_tcheckrik_anim.o \
	$(BUILDDIR)/src/game/m_tcheckrik.o \
	$(BUILDDIR)/src/game/m_tcheckrik_spells.o \
	$(BUILDDIR)/src/game/p_client.o \
	$(BUILDDIR)/src/game/p_funcs.o \
	$(BUILDDIR)/src/game/p_hud.o \
	$(BUILDDIR)/src/game/p_item.o \
	$(BUILDDIR)/src/game/p_view.o \
	$(BUILDDIR)/src/game/script.o \
	$(BUILDDIR)/src/game/spl_blast.o \
	$(BUILDDIR)/src/game/spl_bluering.o \
	$(BUILDDIR)/src/game/spl_flyingfist.o \
	$(BUILDDIR)/src/game/spl_hellstaff.o \
	$(BUILDDIR)/src/game/spl_maceballs.o \
	$(BUILDDIR)/src/game/spl_magicmissile.o \
	$(BUILDDIR)/src/game/spl_meteorbarrier.o \
	$(BUILDDIR)/src/game/spl_morph.o \
	$(BUILDDIR)/src/game/spl_phoenix.o \
	$(BUILDDIR)/src/game/spl_powerup.o \
	$(BUILDDIR)/src/game/spl_redrain.o \
	$(BUILDDIR)/src/game/spl_ripper.o \
	$(BUILDDIR)/src/game/spl_shield.o \
	$(BUILDDIR)/src/game/spl_sphereofannihlation.o \
	$(BUILDDIR)/src/game/spl_teleport.o \
	$(BUILDDIR)/src/game/spl_tornado.o \
	$(BUILDDIR)/src/game/spl_wall.o \
	$(BUILDDIR)/h2common/h2matrix.o \
	$(BUILDDIR)/h2common/h2motion.o \
	$(BUILDDIR)/h2common/h2palette.o \
	$(BUILDDIR)/h2common/h2physics.o \
	$(BUILDDIR)/h2common/h2singlylinkedlist.o \
	$(BUILDDIR)/h2common/h2surfaces.o \
	$(BUILDDIR)/h2common/h2vector.o \
	$(BUILDDIR)/src/common/glob.o \
	$(BUILDDIR)/linux/net_udp.o \
	$(BUILDDIR)/linux/p_dll.o \
	$(BUILDDIR)/src/backends/unix/system.o \
	$(BUILDDIR)/src/backends/unix/shared/hunk.o \
	$(BUILDDIR)/src/common/clientserver.o \
	$(BUILDDIR)/linux/snd_linux.o \
	$(BUILDDIR)/linux/sys_linux.o \
	$(BUILDDIR)/src/client/menu/videomenu.o \
	$(BUILDDIR)/src/client/vid/vid.o \
	$(BUILDDIR)/src/player/p_actions.o \
	$(BUILDDIR)/src/player/p_animactor.o \
	$(BUILDDIR)/src/player/p_anim_branch.o \
	$(BUILDDIR)/src/player/p_anim_data.o \
	$(BUILDDIR)/src/player/p_anims.o \
	$(BUILDDIR)/src/player/p_chicken_anim.o \
	$(BUILDDIR)/src/player/p_chicken.o \
	$(BUILDDIR)/src/player/p_ctrl.o \
	$(BUILDDIR)/src/player/p_items.o \
	$(BUILDDIR)/src/player/player_main.o \
	$(BUILDDIR)/src/player/p_main.o \
	$(BUILDDIR)/src/player/p_weapon.o \
	$(BUILDDIR)/src/common/cmdparser.o \
	$(BUILDDIR)/src/common/collision.o \
	$(BUILDDIR)/src/common/frame.o \
	$(BUILDDIR)/src/common/zone.o \
	$(BUILDDIR)/src/common/crc.o \
	$(BUILDDIR)/src/common/cvar.o \
	$(BUILDDIR)/src/common/filesystem.o \
	$(BUILDDIR)/src/common/md4.o \
	$(BUILDDIR)/h2common/message.o \
	$(BUILDDIR)/src/common/netchan.o \
	$(BUILDDIR)/h2common/netmsg_read.o \
	$(BUILDDIR)/src/common/pmove.o \
	$(BUILDDIR)/h2common/h2rand.o \
	$(BUILDDIR)/src/common/shared/rand.o \
	$(BUILDDIR)/src/common/shared/shared.o \
	$(BUILDDIR)/h2common/reference.o \
	$(BUILDDIR)/h2common/resource_manager.o \
	$(BUILDDIR)/h2common/arrayed_list.o \
	$(BUILDDIR)/h2common/skeletons.o \
	$(BUILDDIR)/src/server/sv_ccmds.o \
	$(BUILDDIR)/src/server/sv_ents.o \
	$(BUILDDIR)/src/server/sv_game.o \
	$(BUILDDIR)/src/server/sv_init.o \
	$(BUILDDIR)/src/server/sv_main.o \
	$(BUILDDIR)/src/server/sv_send.o \
	$(BUILDDIR)/src/server/sv_user.o \
	$(BUILDDIR)/src/server/sv_world.o


$(BUILDDIR)/heretic2 : $(HERETIC2_OBJS) ${HEADERS}
	@echo "===> CXX $<"
	${Q}mkdir -p $(@D)
	${Q}$(CXX) $(CFLAGS) -o $@ $(HERETIC2_OBJS) $(LDFLAGS) $(GLXLDFLAGS)

#############################################################################
# REF_SOFT
#############################################################################

REF_SOFT_OBJS = \
	$(BUILDDIR)/src/client/refresh/soft/sw_aclip.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_alias.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_bsp.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_draw.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_edge.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_image.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_light.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_main.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_misc.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_model.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_part.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_poly.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_polyse.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_rast.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_scan.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_sprite.o \
	$(BUILDDIR)/src/client/refresh/soft/sw_surf.o \
	\
	$(BUILDDIR)/src/common/shared/rand.o \
	$(BUILDDIR)/src/common/shared/shared.o \
	$(BUILDDIR)/src/backends/unix/shared/hunk.o \
	$(BUILDDIR)/src/common/glob.o

REF_SOFT_X11_OBJS = \
	$(BUILDDIR)/linux/rw_x11.o

$(BUILDDIR)/ref_softx.$(SHLIBEXT) : $(REF_SOFT_OBJS) $(REF_SOFT_X11_OBJS) ${HEADERS}
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(REF_SOFT_OBJS) \
		$(REF_SOFT_X11_OBJS) $(XLDFLAGS)

#############################################################################
# REF_GL
#############################################################################

REF_GL_OBJS = \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_book.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_draw.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_fmodel.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_image.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_light.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_mesh.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_model.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_movie.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_rmain.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_rmisc.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_rsurf.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_warp.o \
	$(BUILDDIR)/h2common/r_skeletons.o \
	$(BUILDDIR)/h2common/arrayed_list.o \
	$(BUILDDIR)/h2common/skeletons.o \
	$(BUILDDIR)/linux/qgl_linux.o \
	$(BUILDDIR)/src/common/shared/rand.o \
	$(BUILDDIR)/src/common/shared/shared.o \
	$(BUILDDIR)/src/backends/unix/shared/hunk.o \
	$(BUILDDIR)/src/common/glob.o

REF_GL_GLX_OBJS = \
	$(BUILDDIR)/linux/gl_glx.o

$(BUILDDIR)/ref_gl.$(SHLIBEXT) : $(REF_GL_OBJS) $(REF_GL_GLX_OBJS) ${HEADERS}
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(REF_GL_OBJS) $(REF_GL_GLX_OBJS) $(GLXLDFLAGS)

##########################################################################
# MISC
#############################################################################

clean:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f \
	$(HERETIC2_OBJS) \
	$(REF_SOFT_OBJS) \
	$(REF_SOFT_X11_OBJS) \
	$(REF_GL_OBJS) \
	$(REF_GL_GLX_OBJS)
