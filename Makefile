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
#  - SDL 2 or SDL 3                                      #
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

# Use avcodec for decode ogv videos
WITH_AVCODEC:=no

# Sets an RPATH to $ORIGIN/lib. It can be used to
# inject custom libraries, e.g. a patches libSDL.so
# or libopenal.so. Not supported on Windows.
WITH_RPATH:=yes

# Builds with SDL 3 instead of SDL 2.
WITH_SDL3:=no

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
CFLAGS ?= -O0 -g -Wall -Wpointer-arith -pipe -Werror=format-security -DDEBUG
ifdef ASAN
override CFLAGS += -fsanitize=address -DUSE_SANITIZER
endif
ifdef UBSAN
override CFLAGS += -fsanitize=undefined -DUSE_SANITIZER
endif
else
CFLAGS ?= -O2 -Wall -Wpointer-arith -pipe -fomit-frame-pointer
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
ifeq ($(WITH_SDL3),yes)
SDLCFLAGS := $(shell pkgconf --cflags sdl3)
SDLCFLAGS += -DUSE_SDL3
else
SDLCFLAGS := $(shell sdl2-config --cflags)
endif

ifdef NO_SDL_GYRO
SDLCFLAGS += -DNO_SDL_GYRO
endif

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
else ifeq ($(YQ2_OSTYPE),Darwin)
MOLTENVK_PATH ?= $(shell brew --prefix molten-vk)
INCLUDE ?= -I/usr/local/include -I/opt/homebrew/include -I$(MOLTENVK_PATH)/libexec/include
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
else ifeq ($(YQ2_OSTYPE),Darwin)
LDFLAGS ?= -L/usr/local/lib -L/opt/homebrew/lib
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
ifeq ($(WITH_SDL3),yes)
ifeq ($(YQ2_OSTYPE), Darwin)
SDLLDFLAGS := -lSDL3
else
SDLLDFLAGS := $(shell pkgconf --libs sdl3)
endif
else
ifeq ($(YQ2_OSTYPE), Darwin)
SDLLDFLAGS := -lSDL2
else
SDLLDFLAGS := $(shell sdl2-config --libs)
endif
endif

ifeq ($(WITH_SDL3),yes)
# The renderer libs don't need libSDL3main, libmingw32 or -mwindows.
ifeq ($(YQ2_OSTYPE), Windows)
DLL_SDLLDFLAGS = $(subst -mwindows,,$(subst -lmingw32,,$(subst -lSDL3main,,$(SDLLDFLAGS))))
endif
else
# The renderer libs don't need libSDL2main, libmingw32 or -mwindows.
ifeq ($(YQ2_OSTYPE), Windows)
DLL_SDLLDFLAGS = $(subst -mwindows,,$(subst -lmingw32,,$(subst -lSDL2main,,$(SDLLDFLAGS))))
endif
endif

# ----------

# When make is invoked by "make VERBOSE=1" print
# the compiler and linker commands.
ifdef VERBOSE
Q :=
else
Q := @
endif

# ----------

# Phony targets
.PHONY : all client game icon server ref_gl1 ref_gl3 ref_gles1 ref_gles3 ref_soft ref_vk ref_gl4 player effects

# ----------

# Builds everything but the GLES1 renderer
all: config client server game ref_gl1 ref_gl3 ref_gles3 ref_soft ref_vk ref_gl4 player effects

# ----------

# Builds everything, including the GLES1 renderer
with_gles1: all ref_gles1

# ----------

# Print config values
config:
	@echo "Build configuration"
	@echo "============================"
	@echo "YQ2_ARCH = $(YQ2_ARCH) COMPILER = $(COMPILER)"
	@echo "WITH_CURL = $(WITH_CURL)"
	@echo "WITH_OPENAL = $(WITH_OPENAL)"
	@echo "WITH_AVCODEC = $(WITH_AVCODEC)"
	@echo "WITH_RPATH = $(WITH_RPATH)"
	@echo "WITH_SDL3 = $(WITH_SDL3)"
	@echo "WITH_SYSTEMWIDE = $(WITH_SYSTEMWIDE)"
	@echo "WITH_SYSTEMDIR = $(WITH_SYSTEMDIR)"
	@echo "============================"
	@echo ""

# ----------

# Special target to compile the icon on Windows
ifeq ($(YQ2_OSTYPE), Windows)
icon:
	@echo "===> WR build/icon/icon.res"
	${Q}mkdir -p build/icon
	${Q}windres src/backends/windows/icon.rc -O COFF -o build/icon/icon.res
endif

# ----------

# Cleanup
clean:
	@echo "===> CLEAN"
	${Q}rm -Rf build release/*

cleanall:
	@echo "===> CLEAN"
	${Q}rm -Rf build release

# ----------

# The client
ifeq ($(YQ2_OSTYPE), Windows)
client:
	@echo "===> Building yquake2.exe"
	${Q}mkdir -p release
	$(MAKE) release/yquake2.exe
	@echo "===> Building quake2.exe Wrapper"
	$(MAKE) release/quake2.exe

build/client/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(SDLCFLAGS) $(ZIPCFLAGS) $(INCLUDE) -o $@ $<

release/yquake2.exe : LDFLAGS += -mwindows

ifeq ($(WITH_CURL),yes)
release/yquake2.exe : CFLAGS += -DUSE_CURL
endif

ifeq ($(WITH_AVCODEC),yes)
release/yquake2.exe : CFLAGS += -DAVMEDIADECODE
release/yquake2.exe : LDLIBS += -lavformat -lavcodec -lswscale -lswresample -lavutil
endif

ifeq ($(WITH_OPENAL),yes)
release/yquake2.exe : CFLAGS += -DUSE_OPENAL -DDEFAULT_OPENAL_DRIVER='"openal32.dll"'
endif

else # not Windows

client:
	@echo "===> Building quake2"
	${Q}mkdir -p release
	$(MAKE) release/quake2

ifeq ($(YQ2_OSTYPE), Darwin)
build/client/%.o : %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -arch $(YQ2_ARCH) -x objective-c -c $(CFLAGS) $(SDLCFLAGS) $(ZIPCFLAGS) $(INCLUDE)  $< -o $@
else
build/client/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(SDLCFLAGS) $(ZIPCFLAGS) $(INCLUDE) -o $@ $<
endif

release/quake2 : CFLAGS += -Wno-unused-result

ifeq ($(WITH_CURL),yes)
release/quake2 : CFLAGS += -DUSE_CURL
endif

ifeq ($(WITH_AVCODEC),yes)
release/quake2 : CFLAGS += -DAVMEDIADECODE
release/quake2 : LDLIBS += -lavformat -lavcodec -lswscale -lswresample -lavutil
endif

ifeq ($(WITH_OPENAL),yes)
ifeq ($(YQ2_OSTYPE), OpenBSD)
release/quake2 : CFLAGS += -DUSE_OPENAL -DDEFAULT_OPENAL_DRIVER='"libopenal.so"'
else ifeq ($(YQ2_OSTYPE), Darwin)
OPENAL_PATH ?= $(shell brew --prefix openal-soft)
release/quake2 : CFLAGS += -DUSE_OPENAL -DDEFAULT_OPENAL_DRIVER='"libopenal.dylib"' -I$(OPENAL_PATH)/include
release/quake2 : LDFLAGS += -L$(OPENAL_PATH)/lib -rpath $(OPENAL_PATH)/lib
else
release/quake2 : CFLAGS += -DUSE_OPENAL -DDEFAULT_OPENAL_DRIVER='"libopenal.so.1"'
endif
endif

ifeq ($(YQ2_OSTYPE), Linux)
release/quake2 : CFLAGS += -DHAVE_EXECINFO
endif

ifeq ($(YQ2_OSTYPE), Darwin)
release/quake2 : CFLAGS += -DHAVE_EXECINFO
endif

ifeq ($(YQ2_OSTYPE), SunOS)
release/quake2 : CFLAGS += -DHAVE_EXECINFO
endif

ifeq ($(YQ2_OSTYPE), FreeBSD)
release/quake2 : CFLAGS += -DHAVE_EXECINFO
release/quake2 : LDLIBS += -lexecinfo
endif

ifeq ($(YQ2_OSTYPE), NetBSD)
release/quake2 : CFLAGS += -DHAVE_EXECINFO
release/quake2 : LDLIBS += -lexecinfo
endif

ifeq ($(YQ2_OSTYPE), OpenBSD)
release/quake2 : CFLAGS += -DHAVE_EXECINFO
release/quake2 : LDLIBS += -lexecinfo
endif

ifeq ($(YQ2_OSTYPE), Haiku)
release/quake2 : CFLAGS += -DHAVE_EXECINFO
release/quake2 : LDLIBS += -lexecinfo
endif

ifeq ($(WITH_RPATH),yes)
ifeq ($(YQ2_OSTYPE), Darwin)
release/quake2 : LDFLAGS += -Wl,-rpath,'@executable_path/lib'
else
release/quake2 : LDFLAGS += -Wl,-z,origin,-rpath='$$ORIGIN/lib'
endif
endif
endif

# ----------

# The server
ifeq ($(YQ2_OSTYPE), Windows)
server:
	@echo "===> Building q2ded"
	${Q}mkdir -p release
	$(MAKE) release/q2ded.exe

build/server/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(ZIPCFLAGS) $(INCLUDE) -o $@ $<

release/q2ded.exe : CFLAGS += -DDEDICATED_ONLY

else # not Windows

server:
	@echo "===> Building q2ded"
	${Q}mkdir -p release
	$(MAKE) release/q2ded

build/server/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(ZIPCFLAGS) $(INCLUDE) -o $@ $<

release/q2ded : CFLAGS += -DDEDICATED_ONLY -Wno-unused-result

ifeq ($(YQ2_OSTYPE), FreeBSD)
release/q2ded : LDLIBS += -lexecinfo
endif
endif

# ----------

# The OpenGL 1.x renderer lib

ifeq ($(YQ2_OSTYPE), Windows)

ref_gl1:
	@echo "===> Building ref_gl1.dll"
	$(MAKE) release/ref_gl1.dll

release/ref_gl1.dll : LDFLAGS += -shared
release/ref_gl1.dll : LDLIBS += -lopengl32

else ifeq ($(YQ2_OSTYPE), Darwin)

ref_gl1:
	@echo "===> Building ref_gl1.dylib"
	$(MAKE) release/ref_gl1.dylib


release/ref_gl1.dylib : LDFLAGS += -shared -framework OpenGL

else # not Windows or Darwin

ref_gl1:
	@echo "===> Building ref_gl1.so"
	$(MAKE) release/ref_gl1.so


release/ref_gl1.so : CFLAGS += -fPIC
release/ref_gl1.so : LDFLAGS += -shared
release/ref_gl1.so : LDLIBS += -lGL

endif # OS specific ref_gl1 stuff

build/ref_gl1/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(SDLCFLAGS) $(INCLUDE) -o $@ $<

# ----------

# The OpenGL ES 1.0 renderer lib

ifeq ($(YQ2_OSTYPE), Windows)

ref_gles1:
	@echo "===> Building ref_gles1.dll"
	$(MAKE) release/ref_gles1.dll

release/ref_gles1.dll : GLAD_INCLUDE = -Isrc/client/refresh/gl1/glad-gles1/include
release/ref_gles1.dll : CFLAGS += -DYQ2_GL1_GLES
release/ref_gles1.dll : LDFLAGS += -shared

else ifeq ($(YQ2_OSTYPE), Darwin)

ref_gles1:
	@echo "===> Building ref_gles1.dylib"
	$(MAKE) release/ref_gles1.dylib

release/ref_gles1.dylib : GLAD_INCLUDE = -Isrc/client/refresh/gl1/glad-gles1/include
release/ref_gles1.dylib : CFLAGS += -DYQ2_GL1_GLES
release/ref_gles1.dylib : LDFLAGS += -shared

else # not Windows or Darwin

ref_gles1:
	@echo "===> Building ref_gles1.so"
	$(MAKE) release/ref_gles1.so

release/ref_gles1.so : GLAD_INCLUDE = -Isrc/client/refresh/gl1/glad-gles1/include
release/ref_gles1.so : CFLAGS += -DYQ2_GL1_GLES -fPIC
release/ref_gles1.so : LDFLAGS += -shared

endif # OS specific ref_gles1 stuff

build/ref_gles1/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(SDLCFLAGS) $(INCLUDE) $(GLAD_INCLUDE) -o $@ $<

# ----------

# The OpenGL 3.x renderer lib

ifeq ($(YQ2_OSTYPE), Windows)

ref_gl3:
	@echo "===> Building ref_gl3.dll"
	$(MAKE) release/ref_gl3.dll

release/ref_gl3.dll : GLAD_INCLUDE = -Isrc/client/refresh/gl3/glad/include
release/ref_gl3.dll : LDFLAGS += -shared

else ifeq ($(YQ2_OSTYPE), Darwin)

ref_gl3:
	@echo "===> Building ref_gl3.dylib"
	$(MAKE) release/ref_gl3.dylib

release/ref_gl3.dylib : GLAD_INCLUDE = -Isrc/client/refresh/gl3/glad/include
release/ref_gl3.dylib : LDFLAGS += -shared

else # not Windows or Darwin

ref_gl3:
	@echo "===> Building ref_gl3.so"
	$(MAKE) release/ref_gl3.so

release/ref_gl3.so : GLAD_INCLUDE = -Isrc/client/refresh/gl3/glad/include
release/ref_gl3.so : CFLAGS += -fPIC
release/ref_gl3.so : LDFLAGS += -shared

endif # OS specific ref_gl3 stuff

build/ref_gl3/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(SDLCFLAGS) $(INCLUDE) $(GLAD_INCLUDE) -o $@ $<

# ----------

# The OpenGL ES 3.0 renderer lib

ifeq ($(YQ2_OSTYPE), Windows)

ref_gles3:
	@echo "===> Building ref_gles3.dll"
	$(MAKE) release/ref_gles3.dll

release/ref_gles3.dll : GLAD_INCLUDE = -Isrc/client/refresh/gl3/glad-gles3/include

# YQ2_GL3_GLES3 is for GLES3, DYQ2_GL3_GLES is for things that are identical
# in both GLES3 and GLES2 (in case we ever support that)
release/ref_gles3.dll : CFLAGS += -DYQ2_GL3_GLES3 -DYQ2_GL3_GLES

release/ref_gles3.dll : LDFLAGS += -shared

else ifeq ($(YQ2_OSTYPE), Darwin)

ref_gles3:
	@echo "===> Building ref_gles3.dylib"
	$(MAKE) release/ref_gles3.dylib

release/ref_gles3.dylib : GLAD_INCLUDE = -Isrc/client/refresh/gl3/glad-gles3/include

# YQ2_GL3_GLES3 is for GLES3, DYQ2_GL3_GLES is for things that are identical
# in both GLES3 and GLES2 (in case we ever support that)
release/ref_gles3.dylib : CFLAGS += -DYQ2_GL3_GLES3 -DYQ2_GL3_GLES

release/ref_gles3.dylib : LDFLAGS += -shared

else # not Windows or Darwin

ref_gles3:
	@echo "===> Building ref_gles3.so"
	$(MAKE) release/ref_gles3.so

release/ref_gles3.so : GLAD_INCLUDE = -Isrc/client/refresh/gl3/glad-gles3/include

# YQ2_GL3_GLES3 is for GLES3, DYQ2_GL3_GLES is for things that are identical
# in both GLES3 and GLES2 (in case we ever support that)
release/ref_gles3.so : CFLAGS += -DYQ2_GL3_GLES3 -DYQ2_GL3_GLES -fPIC

release/ref_gles3.so : LDFLAGS += -shared

GLAD_INCLUDE = -Isrc/client/refresh/gl3/glad-gles3/include

endif # OS specific ref_gl3 stuff

build/ref_gles3/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(SDLCFLAGS) $(INCLUDE) $(GLAD_INCLUDE) -o $@ $<

# ----------

# The OpenGL 4.6 renderer lib

ifeq ($(YQ2_OSTYPE), Windows)

ref_gl4:
	@echo "===> Building ref_gl4.dll"
	${Q}mkdir -p release
	$(MAKE) release/ref_gl4.dll

release/ref_gl4.dll : GLAD_INCLUDE = -Isrc/client/refresh/gl4/glad/include
release/ref_gl4.dll : LDFLAGS += -shared

else ifeq ($(YQ2_OSTYPE), Darwin)

ref_gl4:
	@echo "===> Building ref_gl4.dylib"
	$(MAKE) release/ref_gl4.dylib

release/ref_gl4.dylib : GLAD_INCLUDE = -Isrc/client/refresh/gl4/glad/include
release/ref_gl4.dylib : LDFLAGS += -shared

else # not Windows or Darwin - macOS doesn't support OpenGL 4.6

ref_gl4:
	@echo "===> Building ref_gl4.so"
	${Q}mkdir -p release
	$(MAKE) release/ref_gl4.so

release/ref_gl4.so : GLAD_INCLUDE = -Isrc/client/refresh/gl4/glad/include
release/ref_gl4.so : CFLAGS += -fPIC
release/ref_gl4.so : LDFLAGS += -shared

endif # OS specific ref_gl4 stuff

build/ref_gl4/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(SDLCFLAGS) $(INCLUDE) $(GLAD_INCLUDE) -o $@ $<

# ----------

# The soft renderer lib

ifeq ($(YQ2_OSTYPE), Windows)

ref_soft:
	@echo "===> Building ref_soft.dll"
	$(MAKE) release/ref_soft.dll

release/ref_soft.dll : LDFLAGS += -shared

else ifeq ($(YQ2_OSTYPE), Darwin)

ref_soft:
	@echo "===> Building ref_soft.dylib"
	$(MAKE) release/ref_soft.dylib

release/ref_soft.dylib : LDFLAGS += -shared

else # not Windows or Darwin

ref_soft:
	@echo "===> Building ref_soft.so"
	$(MAKE) release/ref_soft.so

release/ref_soft.so : CFLAGS += -fPIC
release/ref_soft.so : LDFLAGS += -shared

endif # OS specific ref_soft stuff

build/ref_soft/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(SDLCFLAGS) $(INCLUDE) -o $@ $<

# ----------

# The vk renderer lib

ifeq ($(YQ2_OSTYPE), Windows)

ref_vk:
	@echo "===> Building ref_vk.dll"
	$(MAKE) release/ref_vk.dll

release/ref_vk.dll : LDFLAGS += -shared

else ifeq ($(YQ2_OSTYPE), Darwin)

ref_vk:
	@echo "===> Building ref_vk.dylib"
	$(MAKE) release/ref_vk.dylib

release/ref_vk.dylib : LDFLAGS += -shared

else # not Windows or Darwin

ref_vk:
	@echo "===> Building ref_vk.so"
	$(MAKE) release/ref_vk.so

release/ref_vk.so : CFLAGS += -fPIC
release/ref_vk.so : LDFLAGS += -shared

endif # OS specific ref_vk stuff

build/ref_vk/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(SDLCFLAGS) $(INCLUDE) -o $@ $<

# ----------

# The baseq2 game
ifeq ($(YQ2_OSTYPE), Windows)
game:
	@echo "===> Building baseq2/game.dll"
	${Q}mkdir -p release/baseq2
	$(MAKE) release/baseq2/game.dll

build/baseq2/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

build/baseq2/%.o: %.cpp
	@echo "===> CXX $<"
	${Q}mkdir -p $(@D)
	${Q}$(CXX) -c $(CFLAGS) $(SDLCFLAGS) $(ZIPCFLAGS) $(INCLUDE) -o $@ $<

release/baseq2/game.dll : LDFLAGS += -shared

else ifeq ($(YQ2_OSTYPE), Darwin)

game:
	@echo "===> Building baseq2/game.dylib"
	${Q}mkdir -p release/baseq2
	$(MAKE) release/baseq2/game.dylib

build/baseq2/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

build/baseq2/%.o: %.cpp
	@echo "===> CXX $<"
	${Q}mkdir -p $(@D)
	${Q}$(CXX) -c $(CFLAGS) $(SDLCFLAGS) $(ZIPCFLAGS) $(INCLUDE) -o $@ $<

release/baseq2/game.dylib : CFLAGS += -fPIC
release/baseq2/game.dylib : LDFLAGS += -shared

else # not Windows or Darwin

game:
	@echo "===> Building baseq2/game.so"
	${Q}mkdir -p release/baseq2
	$(MAKE) release/baseq2/game.so

build/baseq2/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

build/baseq2/%.o: %.cpp
	@echo "===> CXX $<"
	${Q}mkdir -p $(@D)
	${Q}$(CXX) -c $(CFLAGS) $(SDLCFLAGS) $(ZIPCFLAGS) $(INCLUDE) -o $@ $<

release/baseq2/game.so : CFLAGS += -fPIC -Wno-unused-result
release/baseq2/game.so : LDFLAGS += -shared
endif

# ----------

# Used by the game
GAME_OBJS_ = \
	src/common/shared/flash.o \
	src/common/shared/rand.o \
	src/common/shared/shared.o \
	src/game/common/h2vector.o \
	src/game/common/h2rand.o \
	src/game/common/h2singlylinkedlist.o \
	src/game/common/resource_manager.o \
	src/game/common/h2matrix.o \
	src/game/common/arrayed_list.o \
	src/game/common/skeletons.o \
	src/game/common/h2physics.o \
	src/game/common/message.o \
	src/game/buoy.o \
	src/game/character/ai.o \
	src/game/character/corvus/corvus1_anim.o \
	src/game/character/corvus/corvus1.o \
	src/game/character/corvus/corvus2_anim.o \
	src/game/character/corvus/corvus2.o \
	src/game/character/corvus/corvus3_anim.o \
	src/game/character/corvus/corvus3.o \
	src/game/character/corvus/corvus4_anim.o \
	src/game/character/corvus/corvus4.o \
	src/game/character/corvus/corvus5_anim.o \
	src/game/character/corvus/corvus5.o \
	src/game/character/corvus/corvus6_anim.o \
	src/game/character/corvus/corvus6.o \
	src/game/character/corvus/corvus7_anim.o \
	src/game/character/corvus/corvus7.o \
	src/game/character/corvus/corvus8_anim.o \
	src/game/character/corvus/corvus8.o \
	src/game/character/corvus/corvus9_anim.o \
	src/game/character/corvus/corvus9.o \
	src/game/character/dranor/dranor_anim.o \
	src/game/character/dranor/dranor.o \
	src/game/character/elflord/elflord_anim.o \
	src/game/character/elflord/elflord.o \
	src/game/character/morcalavin/morcalavin_anim.o \
	src/game/character/morcalavin/morcalavin.o \
	src/game/character/priestess/priestess2_anim.o \
	src/game/character/priestess/priestess2.o \
	src/game/character/priestess/priestess_anim.o \
	src/game/character/priestess/priestess.o \
	src/game/character/siernan/siernan1_anim.o \
	src/game/character/siernan/siernan1.o \
	src/game/character/siernan/siernan2_anim.o \
	src/game/character/siernan/siernan2.o \
	src/game/character/ssithrascout/ssithrascout_anim.o \
	src/game/character/ssithrascout/ssithrascout.o \
	src/game/character/tome/tome_anim.o \
	src/game/character/tome/tome.o \
	src/game/character/victimssithra/victimssithra_anim.o \
	src/game/character/victimssithra/victimssithra.o \
	src/game/decals.o \
	src/game/ds.o \
	src/game/g_ai.o \
	src/game/game_utilities.o \
	src/game/g_breakable.o \
	src/game/g_classstatics.o \
	src/game/g_chase.o \
	src/game/g_cmds.o \
	src/game/g_ctf.o \
	src/game/g_combat.o \
	src/game/g_defaultmessagehandler.o \
	src/game/g_env.o \
	src/game/g_field.o \
	src/game/g_flamethrow.o \
	src/game/g_func.o \
	src/game/g_hitlocation.o \
	src/game/g_items.o \
	src/game/g_light.o \
	src/game/g_main.o \
	src/game/g_message.o \
	src/game/g_misc.o \
	src/game/g_monster.o \
	src/game/g_moveinfo.o \
	src/game/g_newai.o \
	src/game/g_newweap.o \
	src/game/g_obj.o \
	src/game/g_phys.o \
	src/game/g_resourcemanagers.o \
	src/game/g_rope.o \
	src/game/g_shrine.o \
	src/game/g_skeletons.o \
	src/game/g_spawnf.o \
	src/game/g_spawn.o \
	src/game/g_stateinfo.o \
	src/game/g_svcmds.o \
	src/game/g_target.o \
	src/game/g_translate.o \
	src/game/g_trigger.o \
	src/game/g_utils.o \
	src/game/g_weapon.o \
	src/game/g_waterfx.o \
	src/game/mg_guide.o \
	src/game/monster/assassin/assassin_anim.o \
	src/game/monster/assassin/assassin.o \
	src/game/monster/beast/beast_anim.o \
	src/game/monster/beast/beast.o \
	src/game/monster/bee/bee.o \
	src/game/monster/chicken/chicken_anim.o \
	src/game/monster/chicken/chicken.o \
	src/game/monster/elflord/elflord_anims.o \
	src/game/monster/elflord/elflord.o \
	src/game/monster/fish/fish_anim.o \
	src/game/monster/fish/fish.o \
	src/game/monster/gkrokon/gkrokon_anim.o \
	src/game/monster/gkrokon/gkrokon.o \
	src/game/monster/gorgon/gorgon_anim.o \
	src/game/monster/gorgon/gorgon.o \
	src/game/monster/harpy/harpy_anim.o \
	src/game/monster/harpy/harpy.o \
	src/game/monster/imp/imp_anim.o \
	src/game/monster/imp/imp.o \
	src/game/monster/misc/fmtest.o \
	src/game/monster/misc/move.o \
	src/game/monster/morcalavin/morcalavin_anim.o \
	src/game/monster/morcalavin/morcalavin.o \
	src/game/monster/mother/mother_anim.o \
	src/game/monster/mother/mother.o \
	src/game/monster/mssithra/mssithra_anim.o \
	src/game/monster/mssithra/mssithra.o \
	src/game/monster/ogle/ogle_anim.o \
	src/game/monster/ogle/ogle.o \
	src/game/monster/plagueelf/plagueelf_anim.o \
	src/game/monster/plagueelf/plagueelf.o \
	src/game/monster/plaguessithra/plaguessithra_anim.o \
	src/game/monster/plaguessithra/plaguessithra.o \
	src/game/monster/priestess/priestess_anim.o \
	src/game/monster/priestess/priestess.o \
	src/game/monster/rat/rat_anim.o \
	src/game/monster/rat/rat.o \
	src/game/monster/seraph/seraph_anim.o \
	src/game/monster/seraph/seraph.o \
	src/game/monster/seraph_guard/seraph_guard_anim.o \
	src/game/monster/seraph_guard/seraph_guard.o \
	src/game/monster/spreader/spreader_anim.o \
	src/game/monster/spreader/spreadermist.o \
	src/game/monster/spreader/spreader.o \
	src/game/monster/stats/stats.o \
	src/game/monster/tcheckrik/tcheckrik_anim.o \
	src/game/monster/tcheckrik/tcheckrik.o \
	src/game/monster/tcheckrik/tcheckrik_spells.o \
	src/game/player/chase.o \
	src/game/player/client.o \
	src/game/player/funcs.o \
	src/game/player/hud.o \
	src/game/player/library.o \
	src/game/player/item.o \
	src/game/player/view.o \
	src/game/player/weapon.o \
	src/game/savegame/savegame.o \
	src/game/spell/blast.o \
	src/game/spell/bluering.o \
	src/game/spell/flyingfist.o \
	src/game/spell/hellstaff.o \
	src/game/spell/maceballs.o \
	src/game/spell/magicmissile.o \
	src/game/spell/meteorbarrier.o \
	src/game/spell/morph.o \
	src/game/spell/phoenix.o \
	src/game/spell/powerup.o \
	src/game/spell/redrain.o \
	src/game/spell/ripper.o \
	src/game/spell/shield.o \
	src/game/spell/sphereofannihlation.o \
	src/game/spell/teleport.o \
	src/game/spell/tornado.o \
	src/game/spell/wall.o

# ----------

# The base player
ifeq ($(YQ2_OSTYPE), Windows)
player:
	@echo "===> Building baseq2/player.dll"
	${Q}mkdir -p release/baseq2
	$(MAKE) release/baseq2/player.dll

build/player/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/baseq2/player.dll : LDFLAGS += -shared

else ifeq ($(YQ2_OSTYPE), Darwin)

player:
	@echo "===> Building baseq2/player.dylib"
	${Q}mkdir -p release/baseq2
	$(MAKE) release/baseq2/player.dylib

build/player/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/baseq2/player.dylib : CFLAGS += -fPIC
release/baseq2/player.dylib : LDFLAGS += -shared

else # not Windows or Darwin

player:
	@echo "===> Building baseq2/player.so"
	${Q}mkdir -p release/baseq2
	$(MAKE) release/baseq2/player.so

build/player/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/baseq2/player.so : CFLAGS += -fPIC -Wno-unused-result
release/baseq2/player.so : LDFLAGS += -shared
endif

# ----------

# Used by the player
PLAYER_OBJS_ = \
	src/game/common/h2matrix.o \
	src/game/common/h2vector.o \
	src/game/common/h2rand.o \
	src/common/shared/shared.o \
	src/game/player/library/p_actions.o \
	src/game/player/library/p_animactor.o \
	src/game/player/library/p_anim_branch.o \
	src/game/player/library/p_anim_data.o \
	src/game/player/library/p_anims.o \
	src/game/player/library/p_chicken_anim.o \
	src/game/player/library/p_chicken.o \
	src/game/player/library/p_ctrl.o \
	src/game/player/library/p_items.o \
	src/game/player/library/player_main.o \
	src/game/player/library/p_main.o \
	src/game/player/library/p_weapon.o

# ----------

# The base effects
ifeq ($(YQ2_OSTYPE), Windows)
effects:
	@echo "===> Building baseq2/effects.dll"
	${Q}mkdir -p release/baseq2
	$(MAKE) release/baseq2/effects.dll

build/effects/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/baseq2/effects.dll : LDFLAGS += -shared

else ifeq ($(YQ2_OSTYPE), Darwin)

effects:
	@echo "===> Building baseq2/effects.dylib"
	${Q}mkdir -p release/baseq2
	$(MAKE) release/baseq2/effects.dylib

build/effects/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/baseq2/effects.dylib : CFLAGS += -fPIC
release/baseq2/effects.dylib : LDFLAGS += -shared

else # not Windows or Darwin

effects:
	@echo "===> Building baseq2/effects.so"
	${Q}mkdir -p release/baseq2
	$(MAKE) release/baseq2/effects.so

build/effects/%.o: %.c
	@echo "===> CC $<"
	${Q}mkdir -p $(@D)
	${Q}$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/baseq2/effects.so : CFLAGS += -fPIC -Wno-unused-result
release/baseq2/effects.so : LDFLAGS += -shared
endif

# Used by the effects
EFFECTS_OBJS_ = \
	src/common/shared/rand.o \
	src/common/shared/shared.o \
	src/game/common/h2matrix.o \
	src/game/common/h2physics.o \
	src/game/common/h2rand.o \
	src/game/common/h2singlylinkedlist.o \
	src/game/common/h2vector.o \
	src/game/common/message.o \
	src/game/common/resource_manager.o \
	src/game/effects/ambient_effects.o \
	src/game/effects/ce_default_message_handler.o \
	src/game/effects/ce_dlight.o \
	src/game/effects/ce_message.o \
	src/game/effects/client_effects.o \
	src/game/effects/client_entities.o \
	src/game/effects/fx_ammo_pickup.o \
	src/game/effects/fx_animate.o \
	src/game/effects/fx_assassin.o \
	src/game/effects/fx_blood.o \
	src/game/effects/fx_blue_ring.o \
	src/game/effects/fx_bubbler.o \
	src/game/effects/fx_crosshair.o \
	src/game/effects/fx_cwatcher.o \
	src/game/effects/fx_debris.o \
	src/game/effects/fx_defense_pickup.o \
	src/game/effects/fx_dripper.o \
	src/game/effects/fx_dust.o \
	src/game/effects/fx_dustpuff.o \
	src/game/effects/fx_firehands.o \
	src/game/effects/fx_fire.o \
	src/game/effects/fx_flamethrow.o \
	src/game/effects/fx_flyingfist.o \
	src/game/effects/fx_fountain.o \
	src/game/effects/fx_halo.o \
	src/game/effects/fx_health_pickup.o \
	src/game/effects/fx_hell_staff.o \
	src/game/effects/fx_hitpuff.o \
	src/game/effects/fx_hpproj.o \
	src/game/effects/fx_insectstaff.o \
	src/game/effects/fx_lensflare.o \
	src/game/effects/fx_lightning.o \
	src/game/effects/fx_maceball.o \
	src/game/effects/fx_magicmissile.o \
	src/game/effects/fx_meteorbarrier.o \
	src/game/effects/fx_mist.o \
	src/game/effects/fx_mork.o \
	src/game/effects/fx_morph.o \
	src/game/effects/fx_objects.o \
	src/game/effects/fx_pespell.o \
	src/game/effects/fx_phoenix.o \
	src/game/effects/fx_pickup.o \
	src/game/effects/fx_pickuppuzzle.o \
	src/game/effects/fx_plaguemistexplode.o \
	src/game/effects/fx_plaguemist.o \
	src/game/effects/fx_portal.o \
	src/game/effects/fx_quake.o \
	src/game/effects/fx_redrainglow.o \
	src/game/effects/fx_redrain.o \
	src/game/effects/fx_remotecamera.o \
	src/game/effects/fx_ripples.o \
	src/game/effects/fx_rope.o \
	src/game/effects/fx_scorchmark.o \
	src/game/effects/fx_shadow.o \
	src/game/effects/fx_shield.o \
	src/game/effects/fx_shrine.o \
	src/game/effects/fx_smoke.o \
	src/game/effects/fx_sound.o \
	src/game/effects/fx_sparks.o \
	src/game/effects/fx_spellchange.o \
	src/game/effects/fx_spellhands.o \
	src/game/effects/fx_sphereofannihlation.o \
	src/game/effects/fx_spoo.o \
	src/game/effects/fx_ssarrow.o \
	src/game/effects/fx_ssithra.o \
	src/game/effects/fx_staff.o \
	src/game/effects/fx_tbeast.o \
	src/game/effects/fx_teleport.o \
	src/game/effects/fx_tome.o \
	src/game/effects/fx_tornado.o \
	src/game/effects/fx_wall.o \
	src/game/effects/fx_waterentrysplash.o \
	src/game/effects/fx_waterwake.o \
	src/game/effects/fx_weaponpickup.o \
	src/game/effects/generic_character_effects.o \
	src/game/effects/generic_weapon_effects.o \
	src/game/effects/item_effects.o \
	src/game/effects/main.o \
	src/game/effects/motion.o \
	src/game/effects/netmsg_read.o \
	src/game/effects/particle.o \
	src/game/effects/player_effects.o \
	src/game/effects/test_effect.o \
	src/game/effects/utilities.o

# ----------

# Used by the client
CLIENT_OBJS_ := \
	src/backends/generic/misc.o \
	src/client/cl_cin.o \
	src/client/cl_image.o \
	src/client/cl_console.o \
	src/client/cl_download.o \
	src/client/cl_effects.o \
	src/client/cl_entities.o \
	src/client/cl_input.o \
	src/client/cl_inventory.o \
	src/client/cl_keyboard.o \
	src/game/common/h2singlylinkedlist.o \
	src/game/common/resource_manager.o \
	src/game/client_effects.o \
	src/client/cl_lights.o \
	src/client/cl_main.o \
	src/client/cl_network.o \
	src/client/cl_parse.o \
	src/client/cl_particles.o \
	src/client/cl_prediction.o \
	src/client/cl_screen.o \
	src/client/cl_tempentities.o \
	src/client/cl_view.o \
	src/client/curl/download.o \
	src/client/curl/qcurl.o \
	src/client/input/gyro.o \
	src/client/menu/menu.o \
	src/client/menu/qmenu.o \
	src/client/menu/videomenu.o \
	src/client/sound/ogg.o \
	src/client/sound/openal.o \
	src/client/sound/mp3.o \
	src/client/sound/qal.o \
	src/client/sound/sdl.o \
	src/client/sound/sound.o \
	src/client/sound/wave.o \
	src/client/vid/vid.o \
	src/common/argproc.o \
	src/common/clientserver.o \
	src/common/collision.o \
	src/common/cmodels.o \
	src/common/crc.o \
	src/common/cmdparser.o \
	src/common/cvar.o \
	src/common/filesystem.o \
	src/common/glob.o \
	src/common/md4.o \
	src/common/maps.o \
	src/common/models/image.o \
	src/common/models/loadfile.o \
	src/common/models/models.o \
	src/common/models/models_hlmdl.o \
	src/common/models/models_image.o \
	src/common/models/models_md5.o \
	src/common/models/models_mda.o \
	src/common/models/models_mdl.o \
	src/common/models/models_mdr.o \
	src/common/models/models_sdef.o \
	src/common/models/models_utils.o \
	src/common/models/sprites.o \
	src/common/movemsg.o \
	src/common/frame.o \
	src/common/netchan.o \
	src/common/pmove.o \
	src/common/protocol.o \
	src/common/szone.o \
	src/common/zone.o \
	src/common/shared/flash.o \
	src/common/shared/rand.o \
	src/common/shared/shared.o \
	src/common/shared/utils.o \
	src/common/unzip/ioapi.o \
	src/common/unzip/unzip.o \
	src/common/unzip/miniz/miniz.o \
	src/common/unzip/miniz/miniz_tdef.o \
	src/common/unzip/miniz/miniz_tinfl.o \
	src/server/sv_cmd.o \
	src/server/sv_conless.o \
	src/server/sv_entities.o \
	src/server/sv_game.o \
	src/server/sv_init.o \
	src/server/sv_main.o \
	src/server/sv_save.o \
	src/server/sv_send.o \
	src/server/sv_user.o \
	src/server/sv_world.o

ifeq ($(WITH_SDL3),yes)
CLIENT_OBJS_ += \
	src/client/input/sdl3.o \
	src/client/vid/glimp_sdl3.o
else
CLIENT_OBJS_ += \
	src/client/input/sdl2.o \
	src/client/vid/glimp_sdl2.o
endif

ifeq ($(YQ2_OSTYPE), Windows)
CLIENT_OBJS_ += \
	src/backends/windows/main.o \
	src/backends/windows/network.o \
	src/backends/windows/system.o \
	src/backends/windows/shared/hunk.o
else
CLIENT_OBJS_ += \
	src/backends/unix/main.o \
	src/backends/unix/network.o \
	src/backends/unix/signalhandler.o \
	src/backends/unix/system.o \
	src/backends/unix/shared/hunk.o
endif

# ----------

REFGL1_OBJS_ := \
	src/client/refresh/gl1/qgl.o \
	src/client/refresh/gl1/gl1_draw.o \
	src/client/refresh/gl1/gl1_image.o \
	src/client/refresh/gl1/gl1_light.o \
	src/client/refresh/gl1/gl1_lightmap.o \
	src/client/refresh/gl1/gl1_main.o \
	src/client/refresh/gl1/gl1_mesh.o \
	src/client/refresh/gl1/gl1_misc.o \
	src/client/refresh/gl1/gl1_model.o \
	src/client/refresh/gl1/gl1_scrap.o \
	src/client/refresh/gl1/gl1_surf.o \
	src/client/refresh/gl1/gl1_warp.o \
	src/client/refresh/gl1/gl1_sdl.o \
	src/client/refresh/files/mesh.o \
	src/client/refresh/files/light.o \
	src/client/refresh/gl1/gl1_buffer.o \
	src/client/refresh/files/common.o \
	src/client/refresh/files/surf.o \
	src/client/refresh/files/maps.o \
	src/client/refresh/files/models.o \
	src/client/refresh/files/stb.o \
	src/client/refresh/files/wal.o \
	src/client/refresh/files/warp.o \
	src/common/shared/shared.o \
	src/common/shared/utils.o \
	src/common/cmodels.o \
	src/common/md4.o

REFGL1_OBJS_GLADEES_ := \
	src/client/refresh/gl1/glad-gles1/src/glad.o

ifeq ($(YQ2_OSTYPE), Windows)
REFGL1_OBJS_ += \
	src/backends/windows/shared/hunk.o
else # not Windows
REFGL1_OBJS_ += \
	src/backends/unix/shared/hunk.o
endif

# ----------

REFGL3_OBJS_ := \
	src/client/refresh/gl3/gl3_draw.o \
	src/client/refresh/gl3/gl3_image.o \
	src/client/refresh/gl3/gl3_light.o \
	src/client/refresh/gl3/gl3_lightmap.o \
	src/client/refresh/gl3/gl3_main.o \
	src/client/refresh/gl3/gl3_mesh.o \
	src/client/refresh/gl3/gl3_misc.o \
	src/client/refresh/gl3/gl3_model.o \
	src/client/refresh/gl3/gl3_sdl.o \
	src/client/refresh/gl3/gl3_surf.o \
	src/client/refresh/gl3/gl3_warp.o \
	src/client/refresh/gl3/gl3_shaders.o \
	src/client/refresh/files/common.o \
	src/client/refresh/files/glshaders.o \
	src/client/refresh/files/mesh.o \
	src/client/refresh/files/light.o \
	src/client/refresh/files/surf.o \
	src/client/refresh/files/maps.o \
	src/client/refresh/files/models.o \
	src/client/refresh/files/stb.o \
	src/client/refresh/files/wal.o \
	src/client/refresh/files/warp.o \
	src/common/shared/shared.o \
	src/common/shared/utils.o \
	src/common/cmodels.o \
	src/common/md4.o

REFGL3_OBJS_GLADE_ := \
	src/client/refresh/gl3/glad/src/glad.o

REFGL3_OBJS_GLADEES_ := \
	src/client/refresh/gl3/glad-gles3/src/glad.o

ifeq ($(YQ2_OSTYPE), Windows)
REFGL3_OBJS_ += \
	src/backends/windows/shared/hunk.o
else # not Windows
REFGL3_OBJS_ += \
	src/backends/unix/shared/hunk.o
endif

# ----------

REFGL4_OBJS_ := \
	src/client/refresh/gl4/gl4_draw.o \
	src/client/refresh/gl4/gl4_image.o \
	src/client/refresh/gl4/gl4_light.o \
	src/client/refresh/gl4/gl4_lightmap.o \
	src/client/refresh/gl4/gl4_main.o \
	src/client/refresh/gl4/gl4_mesh.o \
	src/client/refresh/gl4/gl4_misc.o \
	src/client/refresh/gl4/gl4_model.o \
	src/client/refresh/gl4/gl4_sdl.o \
	src/client/refresh/gl4/gl4_surf.o \
	src/client/refresh/gl4/gl4_warp.o \
	src/client/refresh/gl4/gl4_shaders.o \
	src/client/refresh/files/common.o \
	src/client/refresh/files/glshaders.o \
	src/client/refresh/files/mesh.o \
	src/client/refresh/files/light.o \
	src/client/refresh/files/surf.o \
	src/client/refresh/files/maps.o \
	src/client/refresh/files/models.o \
	src/client/refresh/files/stb.o \
	src/client/refresh/files/wal.o \
	src/client/refresh/files/warp.o \
	src/common/shared/shared.o \
	src/common/cmodels.o \
	src/common/md4.o

REFGL4_OBJS_GLADE_ := \
	src/client/refresh/gl4/glad/src/glad.o

ifeq ($(YQ2_OSTYPE), Windows)
REFGL4_OBJS_ += \
	src/backends/windows/shared/hunk.o
else # not Windows
REFGL4_OBJS_ += \
	src/backends/unix/shared/hunk.o
endif

# ----------

REFSOFT_OBJS_ := \
	src/client/refresh/soft/sw_aclip.o \
	src/client/refresh/soft/sw_alias.o \
	src/client/refresh/soft/sw_bsp.o \
	src/client/refresh/soft/sw_draw.o \
	src/client/refresh/soft/sw_edge.o \
	src/client/refresh/soft/sw_image.o \
	src/client/refresh/soft/sw_light.o \
	src/client/refresh/soft/sw_main.o \
	src/client/refresh/soft/sw_misc.o \
	src/client/refresh/soft/sw_model.o \
	src/client/refresh/soft/sw_part.o \
	src/client/refresh/soft/sw_poly.o \
	src/client/refresh/soft/sw_polyset.o \
	src/client/refresh/soft/sw_rast.o \
	src/client/refresh/soft/sw_scan.o \
	src/client/refresh/soft/sw_sprite.o \
	src/client/refresh/soft/sw_surf.o \
	src/client/refresh/soft/sw_warp.o \
	src/client/refresh/files/common.o \
	src/client/refresh/files/mesh.o \
	src/client/refresh/files/light.o \
	src/client/refresh/files/surf.o \
	src/client/refresh/files/maps.o \
	src/client/refresh/files/models.o \
	src/client/refresh/files/stb.o \
	src/client/refresh/files/wal.o \
	src/client/refresh/files/warp.o \
	src/common/shared/shared.o \
	src/common/shared/utils.o \
	src/common/cmodels.o \
	src/common/md4.o

ifeq ($(YQ2_OSTYPE), Windows)
REFSOFT_OBJS_ += \
	src/backends/windows/shared/hunk.o
else # not Windows
REFSOFT_OBJS_ += \
	src/backends/unix/shared/hunk.o
endif

# ----------

REFVK_OBJS_ := \
	src/client/refresh/vk/vk_buffer.o \
	src/client/refresh/vk/vk_cmd.o \
	src/client/refresh/vk/vk_common.o \
	src/client/refresh/vk/vk_device.o \
	src/client/refresh/vk/vk_draw.o \
	src/client/refresh/vk/vk_image.o \
	src/client/refresh/vk/vk_light.o \
	src/client/refresh/vk/vk_lightmap.o \
	src/client/refresh/vk/vk_mesh.o \
	src/client/refresh/vk/vk_model.o \
	src/client/refresh/vk/vk_pipeline.o \
	src/client/refresh/vk/vk_main.o \
	src/client/refresh/vk/vk_misc.o \
	src/client/refresh/vk/vk_surf.o \
	src/client/refresh/vk/vk_shaders.o \
	src/client/refresh/vk/vk_swapchain.o \
	src/client/refresh/vk/vk_validation.o \
	src/client/refresh/vk/vk_warp.o \
	src/client/refresh/vk/vk_util.o \
	src/client/refresh/vk/volk/volk.o \
	src/client/refresh/files/common.o \
	src/client/refresh/files/mesh.o \
	src/client/refresh/files/light.o \
	src/client/refresh/files/surf.o \
	src/client/refresh/files/maps.o \
	src/client/refresh/files/models.o \
	src/client/refresh/files/stb.o \
	src/client/refresh/files/wal.o \
	src/client/refresh/files/warp.o \
	src/common/shared/shared.o \
	src/common/shared/utils.o \
	src/common/cmodels.o \
	src/common/md4.o

ifeq ($(YQ2_OSTYPE), Windows)
REFVK_OBJS_ += \
	src/backends/windows/shared/hunk.o
else # not Windows
REFVK_OBJS_ += \
	src/backends/unix/shared/hunk.o
endif

# ----------

# Used by the server
SERVER_OBJS_ := \
	src/backends/generic/misc.o \
	src/common/argproc.o \
	src/common/clientserver.o \
	src/common/collision.o \
	src/common/cmodels.o \
	src/common/crc.o \
	src/common/cmdparser.o \
	src/common/cvar.o \
	src/common/filesystem.o \
	src/common/glob.o \
	src/common/md4.o \
	src/common/frame.o \
	src/common/maps.o \
	src/common/models/image.o \
	src/common/models/loadfile.o \
	src/common/models/models.o \
	src/common/models/models_hlmdl.o \
	src/common/models/models_image.o \
	src/common/models/models_md5.o \
	src/common/models/models_mda.o \
	src/common/models/models_mdl.o \
	src/common/models/models_mdr.o \
	src/common/models/models_sdef.o \
	src/common/models/models_utils.o \
	src/common/models/sprites.o \
	src/common/movemsg.o \
	src/common/netchan.o \
	src/common/pmove.o \
	src/common/protocol.o \
	src/common/szone.o \
	src/common/zone.o \
	src/common/shared/rand.o \
	src/common/shared/shared.o \
	src/common/shared/utils.o \
	src/common/unzip/ioapi.o \
	src/common/unzip/unzip.o \
	src/common/unzip/miniz/miniz.o \
	src/common/unzip/miniz/miniz_tdef.o \
	src/common/unzip/miniz/miniz_tinfl.o \
	src/game/common/h2singlylinkedlist.o \
	src/game/common/resource_manager.o \
	src/server/sv_cmd.o \
	src/server/sv_conless.o \
	src/server/sv_entities.o \
	src/server/sv_game.o \
	src/server/sv_init.o \
	src/server/sv_main.o \
	src/server/sv_save.o \
	src/server/sv_send.o \
	src/server/sv_user.o \
	src/server/sv_world.o

ifeq ($(YQ2_OSTYPE), Windows)
SERVER_OBJS_ += \
	src/backends/windows/main.o \
	src/backends/windows/network.o \
	src/backends/windows/system.o \
	src/backends/windows/shared/hunk.o
else # not Windows
SERVER_OBJS_ += \
	src/backends/unix/main.o \
	src/backends/unix/network.o \
	src/backends/unix/signalhandler.o \
	src/backends/unix/system.o \
	src/backends/unix/shared/hunk.o
endif

# ----------

# Rewrite paths to our object directory.
CLIENT_OBJS = $(patsubst %,build/client/%,$(CLIENT_OBJS_))
REFGL1_OBJS = $(patsubst %,build/ref_gl1/%,$(REFGL1_OBJS_))
REFGLES1_OBJS = $(patsubst %,build/ref_gles1/%,$(REFGL1_OBJS_))
REFGLES1_OBJS += $(patsubst %,build/ref_gles1/%,$(REFGL1_OBJS_GLADEES_))
REFGL3_OBJS = $(patsubst %,build/ref_gl3/%,$(REFGL3_OBJS_))
REFGL3_OBJS += $(patsubst %,build/ref_gl3/%,$(REFGL3_OBJS_GLADE_))
REFGLES3_OBJS = $(patsubst %,build/ref_gles3/%,$(REFGL3_OBJS_))
REFGLES3_OBJS += $(patsubst %,build/ref_gles3/%,$(REFGL3_OBJS_GLADEES_))
REFGL4_OBJS = $(patsubst %,build/ref_gl4/%,$(REFGL4_OBJS_))
REFGL4_OBJS += $(patsubst %,build/ref_gl4/%,$(REFGL4_OBJS_GLADE_))
REFSOFT_OBJS = $(patsubst %,build/ref_soft/%,$(REFSOFT_OBJS_))
REFVK_OBJS = $(patsubst %,build/ref_vk/%,$(REFVK_OBJS_))
SERVER_OBJS = $(patsubst %,build/server/%,$(SERVER_OBJS_))
GAME_OBJS = $(patsubst %,build/baseq2/%,$(GAME_OBJS_))
PLAYER_OBJS = $(patsubst %,build/player/%,$(PLAYER_OBJS_))
EFFECTS_OBJS = $(patsubst %,build/effects/%,$(EFFECTS_OBJS_))

# ----------

# Generate header dependencies.
CLIENT_DEPS= $(CLIENT_OBJS:.o=.d)
GAME_DEPS= $(GAME_OBJS:.o=.d)
PLAYER_DEPS= $(PLAYER_OBJS:.o=.d)
EFFECTS_DEPS= $(EFFECTS_OBJS:.o=.d)
REFGL1_DEPS= $(REFGL1_OBJS:.o=.d)
REFGLES1_DEPS= $(REFGLES1_OBJS:.o=.d)
REFGL3_DEPS= $(REFGL3_OBJS:.o=.d)
REFGLES3_DEPS= $(REFGLES3_OBJS:.o=.d)
REFGL4_DEPS= $(REFGL4_OBJS:.o=.d)
REFSOFT_DEPS= $(REFSOFT_OBJS:.o=.d)
REFVK_DEPS= $(REFVK_OBJS:.o=.d)
SERVER_DEPS= $(SERVER_OBJS:.o=.d)

# Suck header dependencies in.
-include $(CLIENT_DEPS)
-include $(GAME_DEPS)
-include $(REFGL1_DEPS)
-include $(REFGLES1_DEPS)
-include $(REFGL3_DEPS)
-include $(REFGLES3_DEPS)
-include $(REFGL4_DEPS)
-include $(REFVK_DEPS)
-include $(SERVER_DEPS)

# ----------

# release/quake2
ifeq ($(YQ2_OSTYPE), Windows)
release/yquake2.exe : $(CLIENT_OBJS) icon
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) build/icon/icon.res $(CLIENT_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
	$(Q)strip $@
release/quake2.exe : src/win-wrapper/wrapper.c icon
	$(Q)$(CC) -Wall -mwindows build/icon/icon.res src/win-wrapper/wrapper.c -o $@
	$(Q)strip $@
else
release/quake2 : $(CLIENT_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(CLIENT_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
endif

# release/q2ded
ifeq ($(YQ2_OSTYPE), Windows)
release/q2ded.exe : $(SERVER_OBJS) icon
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) build/icon/icon.res $(SERVER_OBJS) $(LDLIBS) -o $@
	$(Q)strip $@
else
release/q2ded : $(SERVER_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(SERVER_OBJS) $(LDLIBS) -o $@
endif

# release/ref_gl1.so
ifeq ($(YQ2_OSTYPE), Windows)
release/ref_gl1.dll : $(REFGL1_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGL1_OBJS) $(LDLIBS) $(DLL_SDLLDFLAGS) -o $@
	$(Q)strip $@
else ifeq ($(YQ2_OSTYPE), Darwin)
release/ref_gl1.dylib : $(REFGL1_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGL1_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
else
release/ref_gl1.so : $(REFGL1_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGL1_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
endif

# release/ref_gles1.so
ifeq ($(YQ2_OSTYPE), Windows)
release/ref_gles1.dll : $(REFGLES1_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGLES1_OBJS) $(LDLIBS) $(DLL_SDLLDFLAGS) -o $@
	$(Q)strip $@
else ifeq ($(YQ2_OSTYPE), Darwin)
release/ref_gles1.dylib : $(REFGLES1_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGLES1_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
else
release/ref_gles1.so : $(REFGLES1_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGLES1_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
endif

# release/ref_gl3.so
ifeq ($(YQ2_OSTYPE), Windows)
release/ref_gl3.dll : $(REFGL3_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGL3_OBJS) $(LDLIBS) $(DLL_SDLLDFLAGS) -o $@
	$(Q)strip $@
else ifeq ($(YQ2_OSTYPE), Darwin)
release/ref_gl3.dylib : $(REFGL3_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGL3_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
else
release/ref_gl3.so : $(REFGL3_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGL3_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
endif

# release/ref_gles3.so
ifeq ($(YQ2_OSTYPE), Windows)
release/ref_gles3.dll : $(REFGLES3_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGLES3_OBJS) $(LDLIBS) $(DLL_SDLLDFLAGS) -o $@
	$(Q)strip $@
else ifeq ($(YQ2_OSTYPE), Darwin)
release/ref_gles3.dylib : $(REFGLES3_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGLES3_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
else
release/ref_gles3.so : $(REFGLES3_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGLES3_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
endif

# release/ref_gl4.so
ifeq ($(YQ2_OSTYPE), Windows)
release/ref_gl4.dll : $(REFGL4_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGL4_OBJS) $(LDLIBS) $(DLL_SDLLDFLAGS) -o $@
	$(Q)strip $@
else ifeq ($(YQ2_OSTYPE), Darwin)
release/ref_gl4.dylib : $(REFGL4_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGL4_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
else
release/ref_gl4.so : $(REFGL4_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFGL4_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
endif

# release/ref_soft.so
ifeq ($(YQ2_OSTYPE), Windows)
release/ref_soft.dll : $(REFSOFT_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFSOFT_OBJS) $(LDLIBS) $(DLL_SDLLDFLAGS) -o $@
	$(Q)strip $@
else ifeq ($(YQ2_OSTYPE), Darwin)
release/ref_soft.dylib : $(REFSOFT_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFSOFT_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
else
release/ref_soft.so : $(REFSOFT_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFSOFT_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
endif

# release/ref_vk.so
ifeq ($(YQ2_OSTYPE), Windows)
release/ref_vk.dll : $(REFVK_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFVK_OBJS) $(LDLIBS) $(DLL_SDLLDFLAGS) -o $@
else ifeq ($(YQ2_OSTYPE), Darwin)
release/ref_vk.dylib : $(REFVK_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFVK_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
else
release/ref_vk.so : $(REFVK_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(REFVK_OBJS) $(LDLIBS) $(SDLLDFLAGS) -o $@
endif

# release/baseq2/game.so
ifeq ($(YQ2_OSTYPE), Windows)
release/baseq2/game.dll : $(GAME_OBJS)
	@echo "===> LD $@"
	${Q}$(CXX) $(LDFLAGS) $(GAME_OBJS) $(LDLIBS) -o $@
	$(Q)strip $@
else ifeq ($(YQ2_OSTYPE), Darwin)
release/baseq2/game.dylib : $(GAME_OBJS)
	@echo "===> LD $@"
	${Q}$(CXX) $(LDFLAGS) $(GAME_OBJS) $(LDLIBS) -o $@
else
release/baseq2/game.so : $(GAME_OBJS)
	@echo "===> LD $@"
	${Q}$(CXX) $(LDFLAGS) $(GAME_OBJS) $(LDLIBS) -o $@
endif

# release/baseq2/player.so
ifeq ($(YQ2_OSTYPE), Windows)
release/baseq2/player.dll : $(PLAYER_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(PLAYER_OBJS) $(LDLIBS) -o $@
	$(Q)strip $@
else ifeq ($(YQ2_OSTYPE), Darwin)
release/baseq2/player.dylib : $(PLAYER_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(PLAYER_OBJS) $(LDLIBS) -o $@
else
release/baseq2/player.so : $(PLAYER_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(PLAYER_OBJS) $(LDLIBS) -o $@
endif

# release/baseq2/effects.so
ifeq ($(YQ2_OSTYPE), Windows)
release/baseq2/effects.dll : $(EFFECTS_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(EFFECTS_OBJS) $(LDLIBS) -o $@
	$(Q)strip $@
else ifeq ($(YQ2_OSTYPE), Darwin)
release/baseq2/effects.dylib : $(EFFECTS_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(EFFECTS_OBJS) $(LDLIBS) -o $@
else
release/baseq2/effects.so : $(EFFECTS_OBJS)
	@echo "===> LD $@"
	${Q}$(CC) $(LDFLAGS) $(EFFECTS_OBJS) $(LDLIBS) -o $@
endif

# ----------
