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
GAME_DIR=$(MOUNT_DIR)/game
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
XLDFLAGS=-L/usr/X11R6/lib -lX11 -lXext -lXxf86dga
XCFLAGS=

GLLDFLAGS=-L/usr/X11R6/lib -L/usr/local/lib -lX11 -lXext -lvga -lm
GLXLDFLAGS=-L/usr/X11R6/lib -L/usr/local/lib -lX11 -lXext -lXxf86dga -lXxf86vm -lm
GLCFLAGS=-I/usr/include/glide

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

TARGETS=$(BUILDDIR)/quake2 \
	$(BUILDDIR)/game/game.$(SHLIBEXT) \
	$(BUILDDIR)/ref_glx.$(SHLIBEXT) \
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
		$(BUILD_DEBUG_DIR)/game \
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
		$(BUILD_RELEASE_DIR)/game \
		$(BUILD_RELEASE_DIR)/ctf
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

all: build_debug build_release

targets: $(TARGETS)

#############################################################################
# COMPILE
#############################################################################

$(BUILDDIR)/client/%.o :          $(CLIENT_DIR)/%.c
	$(DO_CC)

$(BUILDDIR)/game/%.o :            $(GAME_DIR)/%.c
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
	$(BUILDDIR)/game/m_flash.o \
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
	$(BUILDDIR)/game/q_shared.o \
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
	$(BUILDDIR)/game/q_shared.o \
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

GAME_OBJS = \
	$(BUILDDIR)/game/q_shared.o \
	$(BUILDDIR)/game/g_ai.o \
	$(BUILDDIR)/game/p_client.o \
	$(BUILDDIR)/game/g_cmds.o \
	$(BUILDDIR)/game/g_svcmds.o \
	$(BUILDDIR)/game/g_chase.o \
	$(BUILDDIR)/game/g_combat.o \
	$(BUILDDIR)/game/g_func.o \
	$(BUILDDIR)/game/g_items.o \
	$(BUILDDIR)/game/g_main.o \
	$(BUILDDIR)/game/g_misc.o \
	$(BUILDDIR)/game/g_monster.o \
	$(BUILDDIR)/game/g_phys.o \
	$(BUILDDIR)/game/g_save.o \
	$(BUILDDIR)/game/g_spawn.o \
	$(BUILDDIR)/game/g_target.o \
	$(BUILDDIR)/game/g_trigger.o \
	$(BUILDDIR)/game/g_turret.o \
	$(BUILDDIR)/game/g_utils.o \
	$(BUILDDIR)/game/g_weapon.o \
	$(BUILDDIR)/game/m_actor.o \
	$(BUILDDIR)/game/m_berserk.o \
	$(BUILDDIR)/game/m_boss2.o \
	$(BUILDDIR)/game/m_boss3.o \
	$(BUILDDIR)/game/m_boss31.o \
	$(BUILDDIR)/game/m_boss32.o \
	$(BUILDDIR)/game/m_brain.o \
	$(BUILDDIR)/game/m_chick.o \
	$(BUILDDIR)/game/m_flipper.o \
	$(BUILDDIR)/game/m_float.o \
	$(BUILDDIR)/game/m_flyer.o \
	$(BUILDDIR)/game/m_gladiator.o \
	$(BUILDDIR)/game/m_gunner.o \
	$(BUILDDIR)/game/m_hover.o \
	$(BUILDDIR)/game/m_infantry.o \
	$(BUILDDIR)/game/m_insane.o \
	$(BUILDDIR)/game/m_medic.o \
	$(BUILDDIR)/game/m_move.o \
	$(BUILDDIR)/game/m_mutant.o \
	$(BUILDDIR)/game/m_parasite.o \
	$(BUILDDIR)/game/m_soldier.o \
	$(BUILDDIR)/game/m_supertank.o \
	$(BUILDDIR)/game/m_tank.o \
	$(BUILDDIR)/game/p_hud.o \
	$(BUILDDIR)/game/p_trail.o \
	$(BUILDDIR)/game/p_view.o \
	$(BUILDDIR)/game/p_weapon.o \
	$(BUILDDIR)/game/m_flash.o

$(BUILDDIR)/game/game.$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS)

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
	$(BUILDDIR)/ref_soft/r_aclipa.o \
	$(BUILDDIR)/ref_soft/r_draw16.o \
	$(BUILDDIR)/ref_soft/r_drawa.o \
	$(BUILDDIR)/ref_soft/r_edgea.o \
	$(BUILDDIR)/ref_soft/r_scana.o \
	$(BUILDDIR)/ref_soft/r_spr8.o \
	$(BUILDDIR)/ref_soft/r_surf8.o \
	$(BUILDDIR)/ref_soft/math.o \
	$(BUILDDIR)/ref_soft/d_polysa.o \
	$(BUILDDIR)/ref_soft/r_varsa.o \
	$(BUILDDIR)/ref_soft/sys_dosa.o \
	\
	$(BUILDDIR)/game/q_shared.o \
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
	$(BUILDDIR)/game/q_shared.o \
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
# RPM
#############################################################################

# Make RPMs.  You need to be root to make this work
RPMROOT=/usr/src/redhat
RPM = rpm
RPMFLAGS = -bb
INSTALLDIR = /usr/local/games/quake2
TMPDIR = /var/tmp

ifeq ($(ARCH),axp)

RPMDIR = $(TMPDIR)/q2ded-$(VERSION_FN)
DESTDIR= $(RPMDIR)/$(INSTALLDIR)

rpm: quake2.axp.spec
	touch $(RPMROOT)/SOURCES/q2ded-$(VERSION_FN).tar.gz
	if [ ! -d archives ];then mkdir archives;fi
	$(MAKE) copyfiles COPYDIR=$(DESTDIR)
	cp $(MOUNT_DIR)/linux/quake2.gif $(RPMROOT)/SOURCES/.
	cp quake2.axp.spec $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) quake2.axp.spec
	rm -rf $(RPMDIR)
	mv $(RPMROOT)/RPMS/$(ARCH)/q2ded-$(VERSION_FN)-$(RPM_RELEASE).$(ARCH).rpm archives/.

tar:
	if [ ! -d archives ];then mkdir archives;fi
	$(MAKE) copyfiles COPYDIR=$(RPMDIR)
	cd $(RPMDIR); tar czvf q2ded-$(VERSION_FN)-$(ARCH)-unknown-linux2.0.tar.gz *
	mv $(RPMDIR)/*.tar.gz archives/.
	rm -rf $(RPMDIR)

copyfiles:
	-mkdirhier $(COPYDIR)
	cp $(BUILD_RELEASE_DIR)/q2ded $(COPYDIR)
	strip $(COPYDIR)/q2ded
	chmod 755 $(COPYDIR)/q2ded
	-mkdir $(COPYDIR)/baseq2
	chmod 755 $(COPYDIR)/baseq2
	cp $(BUILD_RELEASE_DIR)/game/game.$(SHLIBEXT) $(COPYDIR)/baseq2
	chmod 755 $(COPYDIR)/baseq2/game/game.$(SHLIBEXT)
	-mkdir $(COPYDIR)/ctf
	chmod 755 $(COPYDIR)/ctf
	cp $(BUILD_RELEASE_DIR)/ctf/game.$(SHLIBEXT) $(COPYDIR)/ctf
	chmod 755 $(COPYDIR)/ctf/game.$(SHLIBEXT)
	cp $(MOUNT_DIR)/linux/readme.axp $(COPYDIR)/README
	cp $(MOUNT_DIR)/readme.txt $(COPYDIR)
	cp $(MOUNT_DIR)/$(VERSION)_Changes.txt $(COPYDIR)


quake2.axp.spec : $(MOUNT_DIR)/linux/quake2.axp.spec.sh Makefile
	sh $< $(VERSION_FN) $(RPM_RELEASE) $(ARCH) $(INSTALLDIR) > $@

else

# rpm-quake2-patch
rpm:   rpm-quake2 rpm-quake2-data rpm-quake2-video \
		rpm-quake2-ctf

PATCHDIR = $(TMPDIR)/quake2-patch-$(VERSION_FN)
rpm-quake2-patch: quake2-patch.spec
	touch $(RPMROOT)/SOURCES/quake2-patch-$(VERSION_FN).tar.gz
	if [ ! -d archives ];then mkdir archives;fi
	$(MAKE) copyfiles COPYDIR=$(PATCHDIR)/$(INSTALLDIR)
	-mkdir $(RPMDIR)/etc
	cp $(MOUNT_DIR)/linux/quake2.conf $(PATCHDIR)/etc/quake2.conf
	cp $(MOUNT_DIR)/linux/quake2.gif $(RPMROOT)/SOURCES/.
	cp $< $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) $<
	rm -rf $(PATCHDIR)
	mv $(RPMROOT)/RPMS/$(ARCH)/quake2-$(VERSION_FN)-$(RPM_RELEASE).$(ARCH).rpm archives/.

QUAKE2DIR = $(TMPDIR)/quake2-$(VERSION_FN)
QUAKE2INSTDIR = $(TMPDIR)/quake2-$(VERSION_FN)/$(INSTALLDIR)
rpm-quake2: quake2.spec
	touch $(RPMROOT)/SOURCES/quake2-$(VERSION_FN).tar.gz
	if [ ! -d archives ];then mkdir archives;fi
	-mkdirhier $(QUAKE2INSTDIR)
	cp $(BUILD_RELEASE_DIR)/quake2 $(QUAKE2INSTDIR)/quake2
	strip $(QUAKE2INSTDIR)/quake2
	-mkdir $(QUAKE2INSTDIR)/baseq2
	chmod 1777 $(QUAKE2INSTDIR)/baseq2
	cp $(BUILD_RELEASE_DIR)/game/game.$(SHLIBEXT) $(QUAKE2INSTDIR)/baseq2
	cp $(BUILD_RELEASE_DIR)/ref_soft.$(SHLIBEXT)  $(QUAKE2INSTDIR)
	cp $(BUILD_RELEASE_DIR)/ref_softx.$(SHLIBEXT) $(QUAKE2INSTDIR)
	cp $(BUILD_RELEASE_DIR)/ref_gl.$(SHLIBEXT) $(QUAKE2INSTDIR)
	cp $(BUILD_RELEASE_DIR)/ref_glx.$(SHLIBEXT) $(QUAKE2INSTDIR)
	-mkdir $(QUAKE2INSTDIR)/ctf
	chmod 1777 $(QUAKE2INSTDIR)/ctf
	cp $(BUILD_RELEASE_DIR)/ctf/game.$(SHLIBEXT) $(QUAKE2INSTDIR)/ctf
	-mkdirhier $(QUAKE2DIR)/usr/lib
	cp $(MOUNT_DIR)/linux/README $(QUAKE2INSTDIR)
	cp $(MOUNT_DIR)/readme.txt $(QUAKE2INSTDIR)
	-mkdir $(QUAKE2DIR)/etc
	cp $(MOUNT_DIR)/linux/quake2.conf $(QUAKE2DIR)/etc/quake2.conf
	cp $(MOUNT_DIR)/linux/quake2.gif $(RPMROOT)/SOURCES/.
	cp $< $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) $<
	rm -rf $(QUAKE2DIR)
	mv $(RPMROOT)/RPMS/$(ARCH)/quake2-$(VERSION_FN)-$(RPM_RELEASE).$(ARCH).rpm archives/.

# rpm-quake2-patch: quake2-patch.spec

QUAKE2DATADIR = $(TMPDIR)/quake2-data-$(VERSION)/$(INSTALLDIR)
rpm-quake2-data: quake2-data.spec
	touch $(RPMROOT)/SOURCES/quake2-data-$(VERSION).tar.gz
	if [ ! -d archives ];then mkdir archives;fi
	-mkdirhier $(QUAKE2DATADIR)/baseq2
	-mkdirhier $(QUAKE2DATADIR)/baseq2/players
	-mkdirhier $(QUAKE2DATADIR)/baseq2/players/cyborg
	-mkdirhier $(QUAKE2DATADIR)/baseq2/players/female
	-mkdirhier $(QUAKE2DATADIR)/baseq2/players/male
	cp $(QUAKE2_DIR)/baseq2/pak0.pak							 	 $(QUAKE2DATADIR)/baseq2/pak0.pak
	cp $(QUAKE2_DIR)/baseq2/pak1.pak                                 $(QUAKE2DATADIR)/baseq2/pak1.pak
	cp $(QUAKE2_DIR)/baseq2/pak2.pak                                 $(QUAKE2DATADIR)/baseq2/pak2.pak
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/a_grenades.md2            $(QUAKE2DATADIR)/baseq2/players/cyborg/a_grenades.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_bfg.md2                 $(QUAKE2DATADIR)/baseq2/players/cyborg/w_bfg.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_blaster.md2             $(QUAKE2DATADIR)/baseq2/players/cyborg/w_blaster.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_chainfist.md2           $(QUAKE2DATADIR)/baseq2/players/cyborg/w_chainfist.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_chaingun.md2            $(QUAKE2DATADIR)/baseq2/players/cyborg/w_chaingun.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_disrupt.md2             $(QUAKE2DATADIR)/baseq2/players/cyborg/w_disrupt.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_etfrifle.md2            $(QUAKE2DATADIR)/baseq2/players/cyborg/w_etfrifle.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_glauncher.md2           $(QUAKE2DATADIR)/baseq2/players/cyborg/w_glauncher.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_grapple.md2             $(QUAKE2DATADIR)/baseq2/players/cyborg/w_grapple.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_hyperblaster.md2        $(QUAKE2DATADIR)/baseq2/players/cyborg/w_hyperblaster.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_machinegun.md2          $(QUAKE2DATADIR)/baseq2/players/cyborg/w_machinegun.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_phalanx.md2             $(QUAKE2DATADIR)/baseq2/players/cyborg/w_phalanx.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_plasma.md2              $(QUAKE2DATADIR)/baseq2/players/cyborg/w_plasma.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_plauncher.md2           $(QUAKE2DATADIR)/baseq2/players/cyborg/w_plauncher.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_railgun.md2             $(QUAKE2DATADIR)/baseq2/players/cyborg/w_railgun.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_ripper.md2              $(QUAKE2DATADIR)/baseq2/players/cyborg/w_ripper.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_rlauncher.md2           $(QUAKE2DATADIR)/baseq2/players/cyborg/w_rlauncher.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_shotgun.md2             $(QUAKE2DATADIR)/baseq2/players/cyborg/w_shotgun.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/w_sshotgun.md2            $(QUAKE2DATADIR)/baseq2/players/cyborg/w_sshotgun.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/a_grenades.md2            $(QUAKE2DATADIR)/baseq2/players/female/a_grenades.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_bfg.md2                 $(QUAKE2DATADIR)/baseq2/players/female/w_bfg.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_blaster.md2             $(QUAKE2DATADIR)/baseq2/players/female/w_blaster.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_chainfist.md2           $(QUAKE2DATADIR)/baseq2/players/female/w_chainfist.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_chaingun.md2            $(QUAKE2DATADIR)/baseq2/players/female/w_chaingun.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_disrupt.md2             $(QUAKE2DATADIR)/baseq2/players/female/w_disrupt.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_etfrifle.md2            $(QUAKE2DATADIR)/baseq2/players/female/w_etfrifle.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_glauncher.md2           $(QUAKE2DATADIR)/baseq2/players/female/w_glauncher.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_grapple.md2             $(QUAKE2DATADIR)/baseq2/players/female/w_grapple.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_hyperblaster.md2        $(QUAKE2DATADIR)/baseq2/players/female/w_hyperblaster.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_machinegun.md2          $(QUAKE2DATADIR)/baseq2/players/female/w_machinegun.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_phalanx.md2             $(QUAKE2DATADIR)/baseq2/players/female/w_phalanx.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_plasma.md2              $(QUAKE2DATADIR)/baseq2/players/female/w_plasma.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_plauncher.md2           $(QUAKE2DATADIR)/baseq2/players/female/w_plauncher.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_railgun.md2             $(QUAKE2DATADIR)/baseq2/players/female/w_railgun.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_ripper.md2              $(QUAKE2DATADIR)/baseq2/players/female/w_ripper.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_rlauncher.md2           $(QUAKE2DATADIR)/baseq2/players/female/w_rlauncher.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_shotgun.md2             $(QUAKE2DATADIR)/baseq2/players/female/w_shotgun.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/w_sshotgun.md2            $(QUAKE2DATADIR)/baseq2/players/female/w_sshotgun.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/a_grenades.md2              $(QUAKE2DATADIR)/baseq2/players/male/a_grenades.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_bfg.md2                   $(QUAKE2DATADIR)/baseq2/players/male/w_bfg.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_blaster.md2               $(QUAKE2DATADIR)/baseq2/players/male/w_blaster.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_chainfist.md2             $(QUAKE2DATADIR)/baseq2/players/male/w_chainfist.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_chaingun.md2              $(QUAKE2DATADIR)/baseq2/players/male/w_chaingun.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_disrupt.md2               $(QUAKE2DATADIR)/baseq2/players/male/w_disrupt.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_etfrifle.md2              $(QUAKE2DATADIR)/baseq2/players/male/w_etfrifle.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_glauncher.md2             $(QUAKE2DATADIR)/baseq2/players/male/w_glauncher.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_grapple.md2               $(QUAKE2DATADIR)/baseq2/players/male/w_grapple.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_hyperblaster.md2          $(QUAKE2DATADIR)/baseq2/players/male/w_hyperblaster.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_machinegun.md2            $(QUAKE2DATADIR)/baseq2/players/male/w_machinegun.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_phalanx.md2               $(QUAKE2DATADIR)/baseq2/players/male/w_phalanx.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_plasma.md2                $(QUAKE2DATADIR)/baseq2/players/male/w_plasma.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_plauncher.md2             $(QUAKE2DATADIR)/baseq2/players/male/w_plauncher.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_railgun.md2               $(QUAKE2DATADIR)/baseq2/players/male/w_railgun.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_ripper.md2                $(QUAKE2DATADIR)/baseq2/players/male/w_ripper.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_rlauncher.md2             $(QUAKE2DATADIR)/baseq2/players/male/w_rlauncher.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_shotgun.md2               $(QUAKE2DATADIR)/baseq2/players/male/w_shotgun.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/w_sshotgun.md2              $(QUAKE2DATADIR)/baseq2/players/male/w_sshotgun.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/athena.pcx                $(QUAKE2DATADIR)/baseq2/players/female/athena.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/athena_i.pcx              $(QUAKE2DATADIR)/baseq2/players/female/athena_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/brianna.pcx               $(QUAKE2DATADIR)/baseq2/players/female/brianna.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/brianna_i.pcx             $(QUAKE2DATADIR)/baseq2/players/female/brianna_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/cobalt.pcx                $(QUAKE2DATADIR)/baseq2/players/female/cobalt.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/cobalt_i.pcx              $(QUAKE2DATADIR)/baseq2/players/female/cobalt_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/ensign.pcx                $(QUAKE2DATADIR)/baseq2/players/female/ensign.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/ensign_i.pcx              $(QUAKE2DATADIR)/baseq2/players/female/ensign_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/jezebel.pcx               $(QUAKE2DATADIR)/baseq2/players/female/jezebel.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/jezebel_i.pcx             $(QUAKE2DATADIR)/baseq2/players/female/jezebel_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/jungle.pcx                $(QUAKE2DATADIR)/baseq2/players/female/jungle.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/jungle_i.pcx              $(QUAKE2DATADIR)/baseq2/players/female/jungle_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/lotus.pcx                 $(QUAKE2DATADIR)/baseq2/players/female/lotus.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/lotus_i.pcx               $(QUAKE2DATADIR)/baseq2/players/female/lotus_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/stiletto.pcx              $(QUAKE2DATADIR)/baseq2/players/female/stiletto.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/stiletto_i.pcx            $(QUAKE2DATADIR)/baseq2/players/female/stiletto_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/tris.md2                  $(QUAKE2DATADIR)/baseq2/players/female/tris.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/venus.pcx                 $(QUAKE2DATADIR)/baseq2/players/female/venus.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/venus_i.pcx               $(QUAKE2DATADIR)/baseq2/players/female/venus_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/voodoo.pcx                $(QUAKE2DATADIR)/baseq2/players/female/voodoo.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/voodoo_i.pcx              $(QUAKE2DATADIR)/baseq2/players/female/voodoo_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/female/weapon.md2                $(QUAKE2DATADIR)/baseq2/players/female/weapon.md2
	cp $(QUAKE2_DIR)/baseq2/players/female/weapon.pcx                $(QUAKE2DATADIR)/baseq2/players/female/weapon.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/cipher.pcx                  $(QUAKE2DATADIR)/baseq2/players/male/cipher.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/cipher_i.pcx                $(QUAKE2DATADIR)/baseq2/players/male/cipher_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/claymore.pcx                $(QUAKE2DATADIR)/baseq2/players/male/claymore.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/claymore_i.pcx              $(QUAKE2DATADIR)/baseq2/players/male/claymore_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/flak.pcx                    $(QUAKE2DATADIR)/baseq2/players/male/flak.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/flak_i.pcx                  $(QUAKE2DATADIR)/baseq2/players/male/flak_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/grunt.pcx                   $(QUAKE2DATADIR)/baseq2/players/male/grunt.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/grunt_i.pcx                 $(QUAKE2DATADIR)/baseq2/players/male/grunt_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/howitzer.pcx                $(QUAKE2DATADIR)/baseq2/players/male/howitzer.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/howitzer_i.pcx              $(QUAKE2DATADIR)/baseq2/players/male/howitzer_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/major.pcx                   $(QUAKE2DATADIR)/baseq2/players/male/major.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/major_i.pcx                 $(QUAKE2DATADIR)/baseq2/players/male/major_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/nightops.pcx                $(QUAKE2DATADIR)/baseq2/players/male/nightops.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/nightops_i.pcx              $(QUAKE2DATADIR)/baseq2/players/male/nightops_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/pointman.pcx                $(QUAKE2DATADIR)/baseq2/players/male/pointman.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/pointman_i.pcx              $(QUAKE2DATADIR)/baseq2/players/male/pointman_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/psycho.pcx                  $(QUAKE2DATADIR)/baseq2/players/male/psycho.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/psycho_i.pcx                $(QUAKE2DATADIR)/baseq2/players/male/psycho_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/rampage.pcx                 $(QUAKE2DATADIR)/baseq2/players/male/rampage.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/rampage_i.pcx               $(QUAKE2DATADIR)/baseq2/players/male/rampage_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/razor.pcx                   $(QUAKE2DATADIR)/baseq2/players/male/razor.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/razor_i.pcx                 $(QUAKE2DATADIR)/baseq2/players/male/razor_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/recon.pcx                   $(QUAKE2DATADIR)/baseq2/players/male/recon.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/recon_i.pcx                 $(QUAKE2DATADIR)/baseq2/players/male/recon_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/scout.pcx                   $(QUAKE2DATADIR)/baseq2/players/male/scout.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/scout_i.pcx                 $(QUAKE2DATADIR)/baseq2/players/male/scout_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/sniper.pcx                  $(QUAKE2DATADIR)/baseq2/players/male/sniper.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/sniper_i.pcx                $(QUAKE2DATADIR)/baseq2/players/male/sniper_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/tris.md2                    $(QUAKE2DATADIR)/baseq2/players/male/tris.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/viper.pcx                   $(QUAKE2DATADIR)/baseq2/players/male/viper.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/viper_i.pcx                 $(QUAKE2DATADIR)/baseq2/players/male/viper_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/male/weapon.md2                  $(QUAKE2DATADIR)/baseq2/players/male/weapon.md2
	cp $(QUAKE2_DIR)/baseq2/players/male/weapon.pcx                  $(QUAKE2DATADIR)/baseq2/players/male/weapon.pcx
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/bump1.wav                 $(QUAKE2DATADIR)/baseq2/players/cyborg/bump1.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/death1.wav                $(QUAKE2DATADIR)/baseq2/players/cyborg/death1.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/death2.wav                $(QUAKE2DATADIR)/baseq2/players/cyborg/death2.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/death3.wav                $(QUAKE2DATADIR)/baseq2/players/cyborg/death3.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/death4.wav                $(QUAKE2DATADIR)/baseq2/players/cyborg/death4.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/drown1.wav                $(QUAKE2DATADIR)/baseq2/players/cyborg/drown1.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/fall1.wav                 $(QUAKE2DATADIR)/baseq2/players/cyborg/fall1.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/fall2.wav                 $(QUAKE2DATADIR)/baseq2/players/cyborg/fall2.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/gurp1.wav                 $(QUAKE2DATADIR)/baseq2/players/cyborg/gurp1.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/gurp2.wav                 $(QUAKE2DATADIR)/baseq2/players/cyborg/gurp2.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/jump1.wav                 $(QUAKE2DATADIR)/baseq2/players/cyborg/jump1.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/oni911.pcx                $(QUAKE2DATADIR)/baseq2/players/cyborg/oni911.pcx
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/oni911_i.pcx              $(QUAKE2DATADIR)/baseq2/players/cyborg/oni911_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/pain100_1.wav             $(QUAKE2DATADIR)/baseq2/players/cyborg/pain100_1.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/pain100_2.wav             $(QUAKE2DATADIR)/baseq2/players/cyborg/pain100_2.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/pain25_1.wav              $(QUAKE2DATADIR)/baseq2/players/cyborg/pain25_1.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/pain25_2.wav              $(QUAKE2DATADIR)/baseq2/players/cyborg/pain25_2.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/pain50_1.wav              $(QUAKE2DATADIR)/baseq2/players/cyborg/pain50_1.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/pain50_2.wav              $(QUAKE2DATADIR)/baseq2/players/cyborg/pain50_2.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/pain75_1.wav              $(QUAKE2DATADIR)/baseq2/players/cyborg/pain75_1.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/pain75_2.wav              $(QUAKE2DATADIR)/baseq2/players/cyborg/pain75_2.wav
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/ps9000.pcx                $(QUAKE2DATADIR)/baseq2/players/cyborg/ps9000.pcx
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/ps9000_i.pcx              $(QUAKE2DATADIR)/baseq2/players/cyborg/ps9000_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/tris.md2                  $(QUAKE2DATADIR)/baseq2/players/cyborg/tris.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/tyr574.pcx                $(QUAKE2DATADIR)/baseq2/players/cyborg/tyr574.pcx
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/tyr574_i.pcx              $(QUAKE2DATADIR)/baseq2/players/cyborg/tyr574_i.pcx
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/weapon.md2                $(QUAKE2DATADIR)/baseq2/players/cyborg/weapon.md2
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/weapon.pcx                $(QUAKE2DATADIR)/baseq2/players/cyborg/weapon.pcx
	cp $(QUAKE2_DIR)/baseq2/players/cyborg/weapon.pcx.pcx            $(QUAKE2DATADIR)/baseq2/players/cyborg/weapon.pcx.pcx
	cp -r $(QUAKE2_DIR)/docs $(QUAKE2DATADIR)/docs
	cp $< $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) $<
	rm -rf $(TMPDIR)/quake2-data-$(VERSION)
	mv $(RPMROOT)/RPMS/$(NOARCH)/quake2-data-$(VERSION)-$(RPM_RELEASE).$(NOARCH).rpm archives/.

QUAKE2VIDEODIR = $(TMPDIR)/quake2-video-$(VERSION)/$(INSTALLDIR)
rpm-quake2-video: quake2-video.spec
	touch $(RPMROOT)/SOURCES/quake2-video-$(VERSION).tar.gz
	if [ ! -d archives ];then mkdir archives;fi
	-mkdirhier $(QUAKE2VIDEODIR)/baseq2
	-mkdirhier $(QUAKE2VIDEODIR)/baseq2/video
	cp $(QUAKE2_DIR)/baseq2/video/end.cin		$(QUAKE2VIDEODIR)/baseq2/video/.
	cp $(QUAKE2_DIR)/baseq2/video/eou1_.cin		$(QUAKE2VIDEODIR)/baseq2/video/.
	cp $(QUAKE2_DIR)/baseq2/video/eou2_.cin		$(QUAKE2VIDEODIR)/baseq2/video/.
	cp $(QUAKE2_DIR)/baseq2/video/eou3_.cin		$(QUAKE2VIDEODIR)/baseq2/video/.
	cp $(QUAKE2_DIR)/baseq2/video/eou4_.cin		$(QUAKE2VIDEODIR)/baseq2/video/.
	cp $(QUAKE2_DIR)/baseq2/video/eou5_.cin		$(QUAKE2VIDEODIR)/baseq2/video/.
	cp $(QUAKE2_DIR)/baseq2/video/eou6_.cin		$(QUAKE2VIDEODIR)/baseq2/video/.
	cp $(QUAKE2_DIR)/baseq2/video/eou7_.cin		$(QUAKE2VIDEODIR)/baseq2/video/.
	cp $(QUAKE2_DIR)/baseq2/video/eou8_.cin		$(QUAKE2VIDEODIR)/baseq2/video/.
	cp $(QUAKE2_DIR)/baseq2/video/idlog.cin		$(QUAKE2VIDEODIR)/baseq2/video/.
	cp $(QUAKE2_DIR)/baseq2/video/ntro.cin		$(QUAKE2VIDEODIR)/baseq2/video/.
	cp $< $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) $<
	rm -rf $(TMPDIR)/quake2-video-$(VERSION)
	mv $(RPMROOT)/RPMS/$(NOARCH)/quake2-video-$(VERSION)-$(RPM_RELEASE).$(NOARCH).rpm archives/.

QUAKE2CTFDIR = $(TMPDIR)/quake2-ctf-$(VERSION)/$(INSTALLDIR)
rpm-quake2-ctf: quake2-ctf.spec
	touch $(RPMROOT)/SOURCES/quake2-ctf-$(VERSION).tar.gz
	if [ ! -d archives ];then mkdir archives;fi
	-mkdirhier $(QUAKE2CTFDIR)/ctf/docs
	cp $(QUAKE2_DIR)/ctf/pak0.pak	$(QUAKE2CTFDIR)/ctf/.
	cp $(QUAKE2_DIR)/ctf/pak1.pak	$(QUAKE2CTFDIR)/ctf/.
	cp $(QUAKE2_DIR)/ctf/ctf2.ico	$(QUAKE2CTFDIR)/ctf/.
	cp $(QUAKE2_DIR)/ctf/readme.txt	$(QUAKE2CTFDIR)/ctf/.
	cp $(QUAKE2_DIR)/ctf/server.cfg	$(QUAKE2CTFDIR)/ctf/.
	cp $< $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) $<
	rm -rf $(TMPDIR)/quake2-ctf-$(VERSION)
	mv $(RPMROOT)/RPMS/$(NOARCH)/quake2-ctf-$(VERSION)-$(RPM_RELEASE).$(NOARCH).rpm archives/.

TARDIR=$(TMPDIR)/tar
tar:
	if [ ! -d archives ];then mkdir archives;fi
	$(MAKE) copyfiles COPYDIR=$(TARDIR)
	cp $(MOUNT_DIR)/linux/quake2.conf $(TARDIR)/quake2.conf
	cd $(TARDIR); sh $(MOUNT_DIR)/linux/fixperms.sh
	cd $(TARDIR); tar czvf quake2-$(VERSION_FN)-$(ARCH)-unknown-linux2.0.tar.gz *
	mv $(TARDIR)/*.tar.gz archives/.
	rm -rf $(TARDIR)


copyfiles:
	-mkdirhier $(COPYDIR)
	cp $(BUILD_RELEASE_DIR)/quake2 $(COPYDIR)/quake2
	strip $(COPYDIR)/quake2
	-mkdir $(COPYDIR)/baseq2
#	for x in `find $(COPYDIR)/baseq2 -type d`; do chmod 755 $$x;done
#	for x in `find $(COPYDIR)/baseq2 -type f`; do chmod 644 $$x;done
	chmod 1777 $(COPYDIR)/baseq2
	cp $(BUILD_RELEASE_DIR)/game/game.$(SHLIBEXT) $(COPYDIR)/baseq2
	cp $(BUILD_RELEASE_DIR)/ref_soft.$(SHLIBEXT)  $(COPYDIR)
	cp $(BUILD_RELEASE_DIR)/ref_softx.$(SHLIBEXT) $(COPYDIR)
	cp $(BUILD_RELEASE_DIR)/ref_gl.$(SHLIBEXT) $(COPYDIR)
	cp $(BUILD_RELEASE_DIR)/ref_glx.$(SHLIBEXT) $(COPYDIR)
#	-mkdir $(COPYDIR)/ctf
#	chmod 1777 $(COPYDIR)/ctf
#	cp $(BUILD_RELEASE_DIR)/ctf/game.$(SHLIBEXT) $(COPYDIR)/ctf
	cd $(COPYDIR); ln -sf libMesaGL.so.2.6 libMesaGL.so.2
	cd $(COPYDIR); ln -sf libMesaGL.so.2 libMesaGL.so
	cd $(COPYDIR); ln -sf libMesaGL.so libGL.so
	cp $(MOUNT_DIR)/linux/README $(COPYDIR)
	cp $(MOUNT_DIR)/readme.txt $(COPYDIR)
	cp $(MOUNT_DIR)/$(VERSION)_Changes.txt $(COPYDIR)

quake2.spec: $(MOUNT_DIR)/linux/quake2.spec.sh
	sh $< $(VERSION_FN) $(RPM_RELEASE) $(ARCH) $(INSTALLDIR) > $@

quake2-patch.spec: $(MOUNT_DIR)/linux/quake2-patch.spec.sh
	sh $< $(VERSION_FN) $(RPM_RELEASE) $(ARCH) $(INSTALLDIR) > $@

quake2-data.spec: $(MOUNT_DIR)/linux/quake2-data.spec.sh
	sh $< $(VERSION) $(RPM_RELEASE) $(NOARCH) $(INSTALLDIR) > $@

quake2-video.spec: $(MOUNT_DIR)/linux/quake2-video.spec.sh
	sh $< $(VERSION) $(RPM_RELEASE) $(NOARCH) $(INSTALLDIR) > $@

quake2-ctf.spec: $(MOUNT_DIR)/linux/quake2-ctf.spec.sh
	sh $< $(VERSION) $(RPM_RELEASE) $(NOARCH) $(INSTALLDIR) > $@


endif


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
	$(GAME_OBJS) \
	$(CTF_OBJS) \
	$(REF_SOFT_OBJS) \
	$(REF_SOFT_X11_OBJS) \
	$(REF_GL_OBJS) \
	$(REF_GL_GLX_OBJS)
#	$(REF_GL_FXMESA_OBJS) \
#	$(REF_SOFT_SVGA_OBJS) \
