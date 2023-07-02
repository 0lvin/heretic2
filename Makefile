# ------------------------------------------------------ #
# Makefile for the "Yamagi Quake 2 Client"               #
#                                                        #
# Just type "make" to compile the                        #
#  - Client (quake2)                                     #
#  - Server (q2ded)                                      #
#  - Quake II Game (baseq2)                              #
#  - Renderer libraries (gl1, gl3, soft)                 #
#                                                        #
# Base dependencies:                                     #
#  - SDL 2.0                                             #
#  - libGL                                               #
#  - Vulkan headers                                      #
#                                                        #
# Optional dependencies:                                 #
#  - CURL                                                #
#  - OpenAL                                              #
#                                                        #
# Platforms:                                             #
#  - FreeBSD                                             #
#  - Linux                                               #
#  - NetBSD                                              #
#  - OpenBSD                                             #
#  - OS X                                                #
#  - Windows (MinGW)                                     #
# ------------------------------------------------------ #

# Variables
# ---------

# - ASAN: Builds with address sanitizer, includes DEBUG.
# - DEBUG: Builds a debug build, forces -O0 and adds debug symbols.
# - VERBOSE: Prints full compile, linker and misc commands.
# - UBSAN: Builds with undefined behavior sanitizer, includes DEBUG.

# ----------

# User configurable options
# -------------------------

# Enables HTTP support through cURL. Used for
# HTTP download.
WITH_CURL:=yes

# Enables the optional OpenAL sound system.
# To use it your system needs libopenal.so.1
# or openal32.dll (we recommend openal-soft)
# installed
WITH_OPENAL:=yes

# Sets an RPATH to $ORIGIN/lib. It can be used to
# inject custom libraries, e.g. a patches libSDL.so
# or libopenal.so. Not supported on Windows.
WITH_RPATH:=yes

# Enable systemwide installation of game assets.
WITH_SYSTEMWIDE:=no

# This will set the default SYSTEMDIR, a non-empty string
# would actually be used. On Windows normals slashes (/)
# instead of backslashed (\) should be used! The string
# MUST NOT be surrounded by quotation marks!
WITH_SYSTEMDIR:=""

# This will set the build options to create an MacOS .app-bundle.
# The app-bundle itself will not be created, but the runtime paths
# will be set to expect the game-data in *.app/
# Contents/Resources
OSX_APP:=yes

# This is an optional configuration file, it'll be used in
# case of presence.
CONFIG_FILE:=config.mk

# ----------

# In case a of a configuration file being present, we'll just use it
ifeq ($(wildcard $(CONFIG_FILE)), $(CONFIG_FILE))
include $(CONFIG_FILE)
endif

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

# Detect the compiler
ifeq ($(shell $(CC) -v 2>&1 | grep -c "clang version"), 1)
COMPILER := clang
COMPILERVER := $(shell $(CC)  -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/')
else ifeq ($(shell $(CC) -v 2>&1 | grep -c -E "(gcc version|gcc-Version)"), 1)
COMPILER := gcc
COMPILERVER := $(shell $(CC)  -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/')
else
COMPILER := unknown
endif

# ASAN includes DEBUG
ifdef ASAN
DEBUG=1
endif

# UBSAN includes DEBUG
ifdef UBSAN
DEBUG=1
endif

# ----------

# Base CFLAGS. These may be overridden by the environment.
# Highest supported optimizations are -O2, higher levels
# will likely break this crappy code.
ifdef DEBUG
CFLAGS ?= -O0 -g -Wall -pipe
ifdef ASAN
override CFLAGS += -fsanitize=address -DUSE_SANITIZER
endif
ifdef UBSAN
override CFLAGS += -fsanitize=undefined -DUSE_SANITIZER
endif
else
CFLAGS ?= -O2 -Wall -pipe -fomit-frame-pointer
endif

# Always needed are:
#  -fno-strict-aliasing since the source doesn't comply
#   with strict aliasing rules and it's next to impossible
#   to get it there...
#  -fwrapv for defined integer wrapping. MSVC6 did this
#   and the game code requires it.
#  -fvisibility=hidden to keep symbols hidden. This is
#   mostly best practice and not really necessary.
override CFLAGS += -fno-strict-aliasing -fwrapv -fvisibility=hidden

# -MMD to generate header dependencies. Unsupported by
#  the Clang shipped with OS X.
ifneq ($(YQ2_OSTYPE), Darwin)
override CFLAGS += -MMD
endif

# OS X architecture.
ifeq ($(YQ2_OSTYPE), Darwin)
override CFLAGS += -arch $(YQ2_ARCH)
endif

# ----------

# ARM needs a sane minimum architecture. We need the `yield`
# opcode, arm6k is the first iteration that supports it. arm6k
# is also the first Raspberry PI generation and older hardware
# is likely too slow to run the game. We're not enforcing the
# minimum architecture, but if you're build for something older
# like arm5 the `yield` opcode isn't compiled in and the game
# (especially q2ded) will consume more CPU time than necessary.
ifeq ($(YQ2_ARCH), arm)
CFLAGS += -march=armv6k
endif

# ----------

# Switch of some annoying warnings.
ifeq ($(COMPILER), clang)
	# -Wno-missing-braces because otherwise clang complains
	#  about totally valid 'vec3_t bla = {0}' constructs.
	override CFLAGS += -Wno-missing-braces
else ifeq ($(COMPILER), gcc)
	# GCC 8.0 or higher.
	ifeq ($(shell test $(COMPILERVER) -ge 80000; echo $$?),0)
	    # -Wno-format-truncation and -Wno-format-overflow
		# because GCC spams about 50 false positives.
		override CFLAGS += -Wno-format-truncation -Wno-format-overflow
	endif
endif

# ----------

# Defines the operating system and architecture
override CFLAGS += -DYQ2OSTYPE=\"$(YQ2_OSTYPE)\" -DYQ2ARCH=\"$(YQ2_ARCH)\"

# ----------

# For reproduceable builds, look here for details:
# https://reproducible-builds.org/specs/source-date-epoch/
ifdef SOURCE_DATE_EPOCH
override CFLAGS += -DBUILD_DATE=\"$(shell date --utc --date="@${SOURCE_DATE_EPOCH}" +"%b %_d %Y" | sed -e 's/ /\\ /g')\"
endif

# ----------

# Using the default x87 float math on 32bit x86 causes rounding trouble
# -ffloat-store could work around that, but the better solution is to
# just enforce SSE - every x86 CPU since Pentium3 supports that
# and this should even improve the performance on old CPUs
ifeq ($(YQ2_ARCH), i386)
override CFLAGS += -msse -mfpmath=sse
endif

# Force SSE math on x86_64. All sane compilers should do this
# anyway, just to protect us from broken Linux distros.
ifeq ($(YQ2_ARCH), x86_64)
override CFLAGS += -mfpmath=sse
endif

# Disable floating-point expression contraction. While this shouldn't be
# a problem for C (only for C++) better be safe than sorry. See
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100839 for details.
ifeq ($(COMPILER), gcc)
override CFLAGS += -ffp-contract=off
endif

# ----------

# Systemwide installation.
ifeq ($(WITH_SYSTEMWIDE),yes)
override CFLAGS += -DSYSTEMWIDE
ifneq ($(WITH_SYSTEMDIR),"")
override CFLAGS += -DSYSTEMDIR=\"$(WITH_SYSTEMDIR)\"
endif
endif

# ----------

# We don't support encrypted ZIP files.
ZIPCFLAGS := -DNOUNCRYPT

# Just set IOAPI_NO_64 on everything that's not Linux or Windows,
# otherwise minizip will use fopen64(), fseek64() and friends that
# may be unavailable. This is - of course - not really correct, in
# a better world we would set -DIOAPI_NO_64 to force everything to
# fopen(), fseek() and so on and -D_FILE_OFFSET_BITS=64 to let the
# libc headers do their work. Currently we can't do that because
# Quake II uses nearly everywere int instead of off_t...
#
# This may have the side effect that ZIP files larger than 2GB are
# unsupported. But I doubt that anyone has such large files, they
# would likely hit other internal limits.
ifneq ($(YQ2_OSTYPE),Windows)
ifneq ($(YQ2_OSTYPE),Linux)
ZIPCFLAGS += -DIOAPI_NO_64
endif
endif

# ----------

# Extra CFLAGS for SDL.
SDLCFLAGS := $(shell sdl2-config --cflags)

# ----------

# Base include path.
ifeq ($(YQ2_OSTYPE),Linux)
INCLUDE ?= -I/usr/include
else ifeq ($(YQ2_OSTYPE),FreeBSD)
INCLUDE ?= -I/usr/local/include
else ifeq ($(YQ2_OSTYPE),NetBSD)
INCLUDE ?= -I/usr/X11R7/include -I/usr/pkg/include
else ifeq ($(YQ2_OSTYPE),OpenBSD)
INCLUDE ?= -I/usr/local/include
else ifeq ($(YQ2_OSTYPE),Windows)
INCLUDE ?= -I/usr/include
endif

# ----------

# Base LDFLAGS. This is just the library path.
ifeq ($(YQ2_OSTYPE),Linux)
LDFLAGS ?= -L/usr/lib
else ifeq ($(YQ2_OSTYPE),FreeBSD)
LDFLAGS ?= -L/usr/local/lib
else ifeq ($(YQ2_OSTYPE),NetBSD)
LDFLAGS ?= -L/usr/X11R7/lib -Wl,-R/usr/X11R7/lib -L/usr/pkg/lib -Wl,-R/usr/pkg/lib
else ifeq ($(YQ2_OSTYPE),OpenBSD)
LDFLAGS ?= -L/usr/local/lib
else ifeq ($(YQ2_OSTYPE),Windows)
LDFLAGS ?= -L/usr/lib
endif

# Link address sanitizer if requested.
ifdef ASAN
override LDFLAGS += -fsanitize=address
endif

# Link undefined behavior sanitizer if requested.
ifdef UBSAN
override LDFLAGS += -fsanitize=undefined
endif

# Required libraries.
ifeq ($(YQ2_OSTYPE),Linux)
LDLIBS ?= -lm -ldl -rdynamic
else ifeq ($(YQ2_OSTYPE),FreeBSD)
LDLIBS ?= -lm
else ifeq ($(YQ2_OSTYPE),NetBSD)
LDLIBS ?= -lm
else ifeq ($(YQ2_OSTYPE),OpenBSD)
LDLIBS ?= -lm
else ifeq ($(YQ2_OSTYPE),Windows)
LDLIBS ?= -lws2_32 -lwinmm -static-libgcc
else ifeq ($(YQ2_OSTYPE), Darwin)
LDLIBS ?= -arch $(YQ2_ARCH)
else ifeq ($(YQ2_OSTYPE), Haiku)
LDLIBS ?= -lm -lnetwork
else ifeq ($(YQ2_OSTYPE), SunOS)
LDLIBS ?= -lm -lsocket -lnsl
endif

# ASAN and UBSAN must not be linked
# with --no-undefined. OSX and OpenBSD
# don't support it at all.
ifndef ASAN
ifndef UBSAN
ifneq ($(YQ2_OSTYPE), Darwin)
ifneq ($(YQ2_OSTYPE), OpenBSD)
override LDFLAGS += -Wl,--no-undefined
endif
endif
endif
endif

# ----------

# Extra LDFLAGS for SDL
ifeq ($(YQ2_OSTYPE), Darwin)
SDLLDFLAGS := -lSDL2
else # not Darwin
SDLLDFLAGS := $(shell sdl2-config --libs)
endif # Darwin

# The renderer libs don't need libSDL2main, libmingw32 or -mwindows.
ifeq ($(YQ2_OSTYPE), Windows)
DLL_SDLLDFLAGS = $(subst -mwindows,,$(subst -lmingw32,,$(subst -lSDL2main,,$(SDLLDFLAGS))))
endif

# ----------

# When make is invoked by "make VERBOSE=1" print
# the compiler and linker commands.
ifdef VERBOSE
Q :=
else
Q := @
endif

BUILD_DEBUG_DIR=build

CC ?= gcc
CXX ?= g++
BASE_CFLAGS=-Dstricmp=strcasecmp $(shell sdl2-config --cflags)

DEBUG_CFLAGS=$(BASE_CFLAGS) -O0 -g -Wall -pipe -fsanitize=address -fsanitize=undefined -fstack-protector-all
# -flto=auto
LDFLAGS=-ldl -lm

SHLIBEXT=so

SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_SHLIB_CXX=$(CXX) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_GL_SHLIB_CC=$(CC) $(CFLAGS) $(GLCFLAGS) -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

TARGETS= \
	$(BUILDDIR)/heretic2 \
	$(BUILDDIR)/ref_gl1.$(SHLIBEXT)

all:
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

targets: $(TARGETS)

HEADERS = \
	src/common/header/shared.h

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
	$(BUILDDIR)/h2common/arrayed_list.o \
	$(BUILDDIR)/h2common/h2matrix.o \
	$(BUILDDIR)/h2common/h2physics.o \
	$(BUILDDIR)/h2common/h2rand.o \
	$(BUILDDIR)/h2common/h2singlylinkedlist.o \
	$(BUILDDIR)/h2common/h2surfaces.o \
	$(BUILDDIR)/h2common/h2vector.o \
	$(BUILDDIR)/h2common/message.o \
	$(BUILDDIR)/h2common/netmsg_read.o \
	$(BUILDDIR)/h2common/reference.o \
	$(BUILDDIR)/h2common/resource_manager.o \
	$(BUILDDIR)/h2common/skeletons.o \
	$(BUILDDIR)/src/backends/generic/misc.o \
	$(BUILDDIR)/src/backends/unix/main.o \
	$(BUILDDIR)/src/backends/unix/network.o \
	$(BUILDDIR)/src/backends/unix/shared/hunk.o \
	$(BUILDDIR)/src/backends/unix/signalhandler.o \
	$(BUILDDIR)/src/backends/unix/system.o \
	$(BUILDDIR)/src/client/cl_cin.o \
	$(BUILDDIR)/src/client/cl_console.o \
	$(BUILDDIR)/src/client/cl_download.o \
	$(BUILDDIR)/src/client/cl_effects.o \
	$(BUILDDIR)/src/client/cl_entities.o \
	$(BUILDDIR)/src/client/cl_input.o \
	$(BUILDDIR)/src/client/cl_inventory.o \
	$(BUILDDIR)/src/client/cl_keyboard.o \
	$(BUILDDIR)/src/client/cl_library.o \
	$(BUILDDIR)/src/client/cl_lights.o \
	$(BUILDDIR)/src/client/cl_main.o \
	$(BUILDDIR)/src/client/cl_network.o \
	$(BUILDDIR)/src/client/cl_parse.o \
	$(BUILDDIR)/src/client/cl_particles.o \
	$(BUILDDIR)/src/client/cl_prediction.o \
	$(BUILDDIR)/src/client/cl_screen.o \
	$(BUILDDIR)/src/client/cl_string.o \
	$(BUILDDIR)/src/client/cl_tempentities.o \
	$(BUILDDIR)/src/client/cl_view.o \
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
	$(BUILDDIR)/src/client_effects/main.o \
	$(BUILDDIR)/src/client_effects/motion.o \
	$(BUILDDIR)/src/client_effects/particle.o \
	$(BUILDDIR)/src/client_effects/player_effects.o \
	$(BUILDDIR)/src/client_effects/test_effect.o \
	$(BUILDDIR)/src/client_effects/utilities.o \
	$(BUILDDIR)/src/client/input/sdl.o \
	$(BUILDDIR)/src/client/libsmacker/smacker.o \
	$(BUILDDIR)/src/client/menu/menu.o \
	$(BUILDDIR)/src/client/menu/qmenu.o \
	$(BUILDDIR)/src/client/menu/videomenu.o \
	$(BUILDDIR)/src/client/sound/ogg.o \
	$(BUILDDIR)/src/client/sound/openal.o \
	$(BUILDDIR)/src/client/sound/qal.o \
	$(BUILDDIR)/src/client/sound/sdl.o \
	$(BUILDDIR)/src/client/sound/sound.o \
	$(BUILDDIR)/src/client/sound/wave.o \
	$(BUILDDIR)/src/client/vid/glimp_sdl.o \
	$(BUILDDIR)/src/client/vid/vid.o \
	$(BUILDDIR)/src/common/argproc.o \
	$(BUILDDIR)/src/common/clientserver.o \
	$(BUILDDIR)/src/common/cmdparser.o \
	$(BUILDDIR)/src/common/collision.o \
	$(BUILDDIR)/src/common/crc.o \
	$(BUILDDIR)/src/common/cvar.o \
	$(BUILDDIR)/src/common/filesystem.o \
	$(BUILDDIR)/src/common/frame.o \
	$(BUILDDIR)/src/common/glob.o \
	$(BUILDDIR)/src/common/md4.o \
	$(BUILDDIR)/src/common/movemsg.o \
	$(BUILDDIR)/src/common/netchan.o \
	$(BUILDDIR)/src/common/pmove.o \
	$(BUILDDIR)/src/common/shared/flash.o \
	$(BUILDDIR)/src/common/shared/rand.o \
	$(BUILDDIR)/src/common/shared/shared.o \
	$(BUILDDIR)/src/common/szone.o \
	$(BUILDDIR)/src/common/unzip/ioapi.o \
	$(BUILDDIR)/src/common/unzip/miniz.o \
	$(BUILDDIR)/src/common/unzip/unzip.o \
	$(BUILDDIR)/src/common/zone.o \
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
	$(BUILDDIR)/src/game/player/client.o \
	$(BUILDDIR)/src/game/player/funcs.o \
	$(BUILDDIR)/src/game/player/hud.o \
	$(BUILDDIR)/src/game/player/item.o \
	$(BUILDDIR)/src/game/player/view.o \
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
	$(BUILDDIR)/src/player/p_library.o \
	$(BUILDDIR)/src/player/p_main.o \
	$(BUILDDIR)/src/player/p_weapon.o \
	$(BUILDDIR)/src/server/sv_cmd.o \
	$(BUILDDIR)/src/server/sv_conless.o \
	$(BUILDDIR)/src/server/sv_entities.o \
	$(BUILDDIR)/src/server/sv_game.o \
	$(BUILDDIR)/src/server/sv_init.o \
	$(BUILDDIR)/src/server/sv_main.o \
	$(BUILDDIR)/src/server/sv_save.o \
	$(BUILDDIR)/src/server/sv_send.o \
	$(BUILDDIR)/src/server/sv_user.o \
	$(BUILDDIR)/src/server/sv_world.o

$(BUILDDIR)/heretic2 : $(HERETIC2_OBJS) ${HEADERS}
	@echo "===> CXX $<"
	${Q}mkdir -p $(@D)
	${Q}$(CXX) $(CFLAGS) -o $@ $(HERETIC2_OBJS) $(LDFLAGS) $(SDLCFLAGS) $(SDLLDFLAGS)

#############################################################################
# REF_GL
#############################################################################

REF_GL_OBJS = \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_draw.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_image.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_light.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_mesh.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_model.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_main.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_misc.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_surf.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_sdl.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_scrap.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_lightmap.o \
	$(BUILDDIR)/src/client/refresh/gl1/gl1_warp.o \
	$(BUILDDIR)/src/client/refresh/gl1/qgl.o \
	$(BUILDDIR)/src/client/refresh/files/models.o \
	$(BUILDDIR)/src/client/refresh/files/pcx.o \
	$(BUILDDIR)/src/client/refresh/files/pvs.o \
	$(BUILDDIR)/src/client/refresh/files/stb.o \
	$(BUILDDIR)/src/client/refresh/files/surf.o \
	$(BUILDDIR)/src/client/refresh/files/wal.o \
	$(BUILDDIR)/src/common/shared/rand.o \
	$(BUILDDIR)/src/common/shared/shared.o \
	$(BUILDDIR)/src/backends/unix/shared/hunk.o \
	$(BUILDDIR)/src/common/glob.o

$(BUILDDIR)/ref_gl1.$(SHLIBEXT) : $(REF_GL_OBJS) ${HEADERS}
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(REF_GL_OBJS) $(SDLCFLAGS)

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
	$(REF_GL_OBJS)
