#
# Quake2 Makefile for Linux 2.0
#
# Nov '97 by Zoid <zoid@idsoftware.com>
#
# ELF only
#

VERSION=3.21
VERSION_FN=$(VERSION)$(GLIBC)
RPM_RELEASE=6

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

MOUNT_DIR=..
QUAKE2_DIR=/grog/Projects/Quake2Master

BUILD_DEBUG_DIR=Debug
BUILD_RELEASE_DIR=Release
CLIENT_DIR=$(MOUNT_DIR)/client
SERVER_DIR=$(MOUNT_DIR)/server
REF_SOFT_DIR=$(MOUNT_DIR)/ref_soft
REF_GL_DIR=$(MOUNT_DIR)/ref_gl
COMMON_DIR=$(MOUNT_DIR)/qcommon
LINUX_DIR=$(MOUNT_DIR)/linux
Q2GAME_DIR=$(MOUNT_DIR)/q2game
NULL_DIR=$(MOUNT_DIR)/null
CTF_DIR=$(MOUNT_DIR)/ctf

CC ?= gcc
BASE_CFLAGS=-Dstricmp=strcasecmp

ifeq ($(ARCH),axp)
RELEASE_CFLAGS=$(BASE_CFLAGS) -O6 -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations
else
RELEASE_CFLAGS=$(BASE_CFLAGS) -g -mpentiumpro -O6 -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations
endif

DEBUG_CFLAGS=$(BASE_CFLAGS) -g
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
DO_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) -o $@ -c $<
DO_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_SHLIB_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_GL_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) $(GLCFLAGS) -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

TARGETS= \
	$(BUILDDIR)/quake2 \
	$(BUILDDIR)/q2game/game.$(SHLIBEXT) \
	$(BUILDDIR)/ref_glx.$(SHLIBEXT) \
	$(BUILDDIR)/ref_softx.$(SHLIBEXT) \
	$(BUILDDIR)/q2ded \
	$(BUILDDIR)/ctf/game.$(SHLIBEXT)

build_debug:
	-mkdir -p $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/client \
		$(BUILD_DEBUG_DIR)/null \
		$(BUILD_DEBUG_DIR)/linux \
		$(BUILD_DEBUG_DIR)/common \
		$(BUILD_DEBUG_DIR)/server \
		$(BUILD_DEBUG_DIR)/ded \
		$(BUILD_DEBUG_DIR)/ref_soft \
		$(BUILD_DEBUG_DIR)/ref_gl \
		$(BUILD_DEBUG_DIR)/q2game \
		$(BUILD_DEBUG_DIR)/ctf
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

build_release:
	-mkdir -p $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/client \
		$(BUILD_RELEASE_DIR)/null \
		$(BUILD_RELEASE_DIR)/linux \
		$(BUILD_RELEASE_DIR)/common \
		$(BUILD_RELEASE_DIR)/server \
		$(BUILD_RELEASE_DIR)/ded \
		$(BUILD_RELEASE_DIR)/ref_soft \
		$(BUILD_RELEASE_DIR)/ref_gl \
		$(BUILD_RELEASE_DIR)/q2game \
		$(BUILD_RELEASE_DIR)/ctf
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

all: build_debug build_release

targets: $(TARGETS)

#############################################################################
# COMPILE
#############################################################################

$(BUILDDIR)/client/%.o :          $(CLIENT_DIR)/%.c
	$(DO_CC)

$(BUILDDIR)/q2game/%.o :            $(Q2GAME_DIR)/%.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/common/%.o :          $(COMMON_DIR)/%.c
	$(DO_CC)

$(BUILDDIR)/server/%.o :          $(SERVER_DIR)/%.c
	$(DO_CC)

$(BUILDDIR)/ctf/%.o :             $(CTF_DIR)/%.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/ref_soft/%.o :        $(REF_SOFT_DIR)/%.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/ref_gl/%.o :          $(REF_GL_DIR)/%.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/linux/%.o :           $(LINUX_DIR)/%.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/null/%.o :            $(NULL_DIR)/%.c
	$(DO_CC)

#############################################################################
# CLIENT/SERVER
#############################################################################

QUAKE2_OBJS = \
	$(BUILDDIR)/client/cl_cin.o \
	$(BUILDDIR)/client/cl_ents.o \
	$(BUILDDIR)/client/cl_fx.o \
	$(BUILDDIR)/client/cl_input.o \
	$(BUILDDIR)/client/cl_inv.o \
	$(BUILDDIR)/client/cl_main.o \
	$(BUILDDIR)/client/cl_newfx.o \
	$(BUILDDIR)/client/cl_parse.o \
	$(BUILDDIR)/client/cl_pred.o \
	$(BUILDDIR)/client/cl_tent.o \
	$(BUILDDIR)/client/cl_scrn.o \
	$(BUILDDIR)/client/cl_view.o \
	$(BUILDDIR)/client/console.o \
	$(BUILDDIR)/client/keys.o \
	$(BUILDDIR)/client/menu.o \
	$(BUILDDIR)/client/snd_dma.o \
	$(BUILDDIR)/client/snd_mem.o \
	$(BUILDDIR)/client/snd_mix.o \
	$(BUILDDIR)/client/qmenu.o \
	$(BUILDDIR)/q2game/m_flash.o \
	\
	$(BUILDDIR)/common/cmd.o \
	$(BUILDDIR)/common/cmodel.o \
	$(BUILDDIR)/common/common.o \
	$(BUILDDIR)/common/crc.o \
	$(BUILDDIR)/common/cvar.o \
	$(BUILDDIR)/common/files.o \
	$(BUILDDIR)/common/md4.o \
	$(BUILDDIR)/common/net_chan.o \
	\
	$(BUILDDIR)/server/sv_ccmds.o \
	$(BUILDDIR)/server/sv_ents.o \
	$(BUILDDIR)/server/sv_game.o \
	$(BUILDDIR)/server/sv_init.o \
	$(BUILDDIR)/server/sv_main.o \
	$(BUILDDIR)/server/sv_send.o \
	$(BUILDDIR)/server/sv_user.o \
	$(BUILDDIR)/server/sv_world.o \
	\
	$(BUILDDIR)/linux/cd_linux.o \
	$(BUILDDIR)/linux/q_shlinux.o \
	$(BUILDDIR)/linux/vid_menu.o \
	$(BUILDDIR)/linux/vid_so.o \
	$(BUILDDIR)/linux/snd_linux.o \
	$(BUILDDIR)/linux/sys_linux.o \
	$(BUILDDIR)/linux/glob.o \
	$(BUILDDIR)/linux/net_udp.o \
	\
	$(BUILDDIR)/q2game/q_shared.o \
	$(BUILDDIR)/common/pmove.o

$(BUILDDIR)/quake2 : $(QUAKE2_OBJS) $(QUAKE2_AS_OBJS)
	$(CC) $(CFLAGS) -o $@ $(QUAKE2_OBJS) $(LDFLAGS)

#############################################################################
# DEDICATED SERVER
#############################################################################

Q2DED_OBJS = \
	\
	$(BUILDDIR)/common/cmd.o \
	$(BUILDDIR)/common/cmodel.o \
	$(BUILDDIR)/ded/common.o \
	$(BUILDDIR)/common/crc.o \
	$(BUILDDIR)/common/cvar.o \
	$(BUILDDIR)/common/files.o \
	$(BUILDDIR)/common/md4.o \
	$(BUILDDIR)/common/net_chan.o \
	\
	$(BUILDDIR)/server/sv_ccmds.o \
	$(BUILDDIR)/server/sv_ents.o \
	$(BUILDDIR)/server/sv_game.o \
	$(BUILDDIR)/server/sv_init.o \
	$(BUILDDIR)/server/sv_main.o \
	$(BUILDDIR)/server/sv_send.o \
	$(BUILDDIR)/server/sv_user.o \
	$(BUILDDIR)/server/sv_world.o \
	\
	$(BUILDDIR)/linux/q_shlinux.o \
	$(BUILDDIR)/ded/sys_linux.o \
	$(BUILDDIR)/linux/glob.o \
	$(BUILDDIR)/linux/net_udp.o \
	\
	$(BUILDDIR)/q2game/q_shared.o \
	$(BUILDDIR)/common/pmove.o \
	\
	$(BUILDDIR)/null/cl_null.o \
	$(BUILDDIR)/null/cd_null.o

$(BUILDDIR)/q2ded : $(Q2DED_OBJS)
	$(CC) $(CFLAGS) -o $@ $(Q2DED_OBJS) $(LDFLAGS)

# dedicated
$(BUILDDIR)/ded/common.o :     $(COMMON_DIR)/common.c
	$(DO_CC) -DDEDICATED_ONLY

$(BUILDDIR)/ded/sys_linux.o :  $(LINUX_DIR)/sys_linux.c
	$(DO_CC) -DDEDICATED_ONLY

#############################################################################
# GAME
#############################################################################

Q2GAME_OBJS = \
	$(BUILDDIR)/q2game/q_shared.o \
	$(BUILDDIR)/q2game/g_ai.o \
	$(BUILDDIR)/q2game/p_client.o \
	$(BUILDDIR)/q2game/g_cmds.o \
	$(BUILDDIR)/q2game/g_svcmds.o \
	$(BUILDDIR)/q2game/g_chase.o \
	$(BUILDDIR)/q2game/g_combat.o \
	$(BUILDDIR)/q2game/g_func.o \
	$(BUILDDIR)/q2game/g_items.o \
	$(BUILDDIR)/q2game/g_main.o \
	$(BUILDDIR)/q2game/g_misc.o \
	$(BUILDDIR)/q2game/g_monster.o \
	$(BUILDDIR)/q2game/g_phys.o \
	$(BUILDDIR)/q2game/g_save.o \
	$(BUILDDIR)/q2game/g_spawn.o \
	$(BUILDDIR)/q2game/g_target.o \
	$(BUILDDIR)/q2game/g_trigger.o \
	$(BUILDDIR)/q2game/g_turret.o \
	$(BUILDDIR)/q2game/g_utils.o \
	$(BUILDDIR)/q2game/g_weapon.o \
	$(BUILDDIR)/q2game/m_actor.o \
	$(BUILDDIR)/q2game/m_berserk.o \
	$(BUILDDIR)/q2game/m_boss2.o \
	$(BUILDDIR)/q2game/m_boss3.o \
	$(BUILDDIR)/q2game/m_boss31.o \
	$(BUILDDIR)/q2game/m_boss32.o \
	$(BUILDDIR)/q2game/m_brain.o \
	$(BUILDDIR)/q2game/m_chick.o \
	$(BUILDDIR)/q2game/m_flipper.o \
	$(BUILDDIR)/q2game/m_float.o \
	$(BUILDDIR)/q2game/m_flyer.o \
	$(BUILDDIR)/q2game/m_gladiator.o \
	$(BUILDDIR)/q2game/m_gunner.o \
	$(BUILDDIR)/q2game/m_hover.o \
	$(BUILDDIR)/q2game/m_infantry.o \
	$(BUILDDIR)/q2game/m_insane.o \
	$(BUILDDIR)/q2game/m_medic.o \
	$(BUILDDIR)/q2game/m_move.o \
	$(BUILDDIR)/q2game/m_mutant.o \
	$(BUILDDIR)/q2game/m_parasite.o \
	$(BUILDDIR)/q2game/m_soldier.o \
	$(BUILDDIR)/q2game/m_supertank.o \
	$(BUILDDIR)/q2game/m_tank.o \
	$(BUILDDIR)/q2game/p_hud.o \
	$(BUILDDIR)/q2game/p_trail.o \
	$(BUILDDIR)/q2game/p_view.o \
	$(BUILDDIR)/q2game/p_weapon.o \
	$(BUILDDIR)/q2game/m_flash.o

$(BUILDDIR)/q2game/game.$(SHLIBEXT) : $(Q2GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(Q2GAME_OBJS)

#############################################################################
# CTF
#############################################################################

CTF_OBJS = \
	$(BUILDDIR)/ctf/g_ai.o \
	$(BUILDDIR)/ctf/g_chase.o \
	$(BUILDDIR)/ctf/g_cmds.o \
	$(BUILDDIR)/ctf/g_combat.o \
	$(BUILDDIR)/ctf/g_ctf.o \
	$(BUILDDIR)/ctf/g_func.o \
	$(BUILDDIR)/ctf/g_items.o \
	$(BUILDDIR)/ctf/g_main.o \
	$(BUILDDIR)/ctf/g_misc.o \
	$(BUILDDIR)/ctf/g_monster.o \
	$(BUILDDIR)/ctf/g_phys.o \
	$(BUILDDIR)/ctf/g_save.o \
	$(BUILDDIR)/ctf/g_spawn.o \
	$(BUILDDIR)/ctf/g_svcmds.o \
	$(BUILDDIR)/ctf/g_target.o \
	$(BUILDDIR)/ctf/g_trigger.o \
	$(BUILDDIR)/ctf/g_utils.o \
	$(BUILDDIR)/ctf/g_weapon.o \
	$(BUILDDIR)/ctf/m_move.o \
	$(BUILDDIR)/ctf/p_client.o \
	$(BUILDDIR)/ctf/p_hud.o \
	$(BUILDDIR)/ctf/p_menu.o \
	$(BUILDDIR)/ctf/p_trail.o \
	$(BUILDDIR)/ctf/p_view.o \
	$(BUILDDIR)/ctf/p_weapon.o \
	$(BUILDDIR)/ctf/q_shared.o

$(BUILDDIR)/ctf/game.$(SHLIBEXT) : $(CTF_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(CTF_OBJS)

#############################################################################
# REF_SOFT
#############################################################################

REF_SOFT_OBJS = \
	$(BUILDDIR)/ref_soft/r_aclip.o \
	$(BUILDDIR)/ref_soft/r_alias.o \
	$(BUILDDIR)/ref_soft/r_bsp.o \
	$(BUILDDIR)/ref_soft/r_draw.o \
	$(BUILDDIR)/ref_soft/r_edge.o \
	$(BUILDDIR)/ref_soft/r_image.o \
	$(BUILDDIR)/ref_soft/r_light.o \
	$(BUILDDIR)/ref_soft/r_main.o \
	$(BUILDDIR)/ref_soft/r_misc.o \
	$(BUILDDIR)/ref_soft/r_model.o \
	$(BUILDDIR)/ref_soft/r_part.o \
	$(BUILDDIR)/ref_soft/r_poly.o \
	$(BUILDDIR)/ref_soft/r_polyse.o \
	$(BUILDDIR)/ref_soft/r_rast.o \
	$(BUILDDIR)/ref_soft/r_scan.o \
	$(BUILDDIR)/ref_soft/r_sprite.o \
	$(BUILDDIR)/ref_soft/r_surf.o \
	\
	$(BUILDDIR)/q2game/q_shared.o \
	$(BUILDDIR)/linux/q_shlinux.o \
	$(BUILDDIR)/linux/glob.o

REF_SOFT_SVGA_OBJS = \
	$(BUILDDIR)/linux/rw_svgalib.o \
	$(BUILDDIR)/ref_soft/d_copy.o \
	$(BUILDDIR)/linux/rw_in_svgalib.o

REF_SOFT_X11_OBJS = \
	$(BUILDDIR)/linux/rw_x11.o

$(BUILDDIR)/ref_soft.$(SHLIBEXT) : $(REF_SOFT_OBJS) $(REF_SOFT_SVGA_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -Xlinker -Map -Xlinker map.out -o $@ $(REF_SOFT_OBJS) \
		$(REF_SOFT_SVGA_OBJS) $(SVGALDFLAGS)

$(BUILDDIR)/ref_softx.$(SHLIBEXT) : $(REF_SOFT_OBJS) $(REF_SOFT_X11_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(REF_SOFT_OBJS) \
		$(REF_SOFT_X11_OBJS) $(XLDFLAGS)

#############################################################################
# REF_GL
#############################################################################

REF_GL_OBJS = \
	$(BUILDDIR)/ref_gl/gl_draw.o \
	$(BUILDDIR)/ref_gl/gl_image.o \
	$(BUILDDIR)/ref_gl/gl_light.o \
	$(BUILDDIR)/ref_gl/gl_mesh.o \
	$(BUILDDIR)/ref_gl/gl_model.o \
	$(BUILDDIR)/ref_gl/gl_rmain.o \
	$(BUILDDIR)/ref_gl/gl_rmisc.o \
	$(BUILDDIR)/ref_gl/gl_rsurf.o \
	$(BUILDDIR)/ref_gl/gl_warp.o \
	\
	$(BUILDDIR)/linux/qgl_linux.o \
	$(BUILDDIR)/q2game/q_shared.o \
	$(BUILDDIR)/linux/q_shlinux.o \
	$(BUILDDIR)/linux/glob.o

# TTimo: taking out Mesa
REF_GL_FXMESA_OBJS = \
	$(BUILDDIR)/linux/rw_in_svgalib.o

#REF_GL_FXMESA_OBJS = \
#	$(BUILDDIR)/linux/gl_fxmesa.o \
#	$(BUILDDIR)/linux/rw_in_svgalib.o

REF_GL_GLX_OBJS = \
	$(BUILDDIR)/linux/gl_glx.o

$(BUILDDIR)/ref_gl.$(SHLIBEXT) : $(REF_GL_OBJS) $(REF_GL_FXMESA_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(REF_GL_OBJS) $(REF_GL_FXMESA_OBJS) $(GLLDFLAGS)

$(BUILDDIR)/ref_glx.$(SHLIBEXT) : $(REF_GL_OBJS) $(REF_GL_GLX_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(REF_GL_OBJS) $(REF_GL_GLX_OBJS) $(GLXLDFLAGS)

#############################################################################
# MISC
#############################################################################

clean: clean-debug clean-release

clean-debug:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f \
	$(QUAKE2_OBJS) \
	$(Q2DED_OBJS) \
	$(Q2GAME_OBJS) \
	$(CTF_OBJS) \
	$(REF_SOFT_OBJS) \
	$(REF_SOFT_X11_OBJS) \
	$(REF_GL_OBJS) \
	$(REF_GL_GLX_OBJS)
#	$(REF_GL_FXMESA_OBJS) \
#	$(REF_SOFT_SVGA_OBJS) \
