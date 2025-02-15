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

CFLAGS = -g -Wall -lm -lGL -lGLU -lglut
COMMON_FILES = \
	../src/client/cl_image.c \
	../src/common/md4.c \
	../src/common/shared/shared.c \
	../src/backends/unix/shared/hunk.c \
	../src/client/refresh/files/mesh.c \
	../src/client/refresh/files/models.c \
	../src/client/refresh/files/models_md5.c \
	../src/client/refresh/files/models_mdr.c \
	render.c

COMMON_HEADERS =  \
	../src/common/header/files.h \
	../src/common/header/shared.h \
	../src/client/refresh/files/stb_image.h \
	../src/client/refresh/files/stb_truetype.h \
	../src/client/refresh/files/stb_image_resize.h \
	iqm.h \
	render.h

all: view-quake view-iqm sdlview
clean:
	rm -f *.o view-quake view-iqm sdlview

# -fsanitize=address
check: check.c ${COMMON_FILES} ${COMMON_HEADERS}
	gcc -g -DYQ2OSTYPE=\"$(YQ2_OSTYPE)\" -DYQ2ARCH=\"$(YQ2_ARCH)\" ${COMMON_FILES} check.c -lm -o check

view-quake: mdlx.c ${COMMON_FILES} ${COMMON_HEADERS} rendergl.c
	gcc -g -DYQ2OSTYPE=\"$(YQ2_OSTYPE)\" -DYQ2ARCH=\"$(YQ2_ARCH)\" ${COMMON_FILES} rendergl.c mdlx.c ${CFLAGS} -o view-quake

sdlview: sdlview.c ${COMMON_FILES} ${COMMON_HEADERS}
	gcc -g -DYQ2OSTYPE=\"$(YQ2_OSTYPE)\" -DYQ2ARCH=\"$(YQ2_ARCH)\" ${COMMON_FILES} rendergl.c sdlview.c -lm -I/usr/include/SDL2 -D_REENTRANT -lSDL2  -lGL -o sdlview

view-iqm: iqm.c ${COMMON_FILES} ${COMMON_HEADERS}
	gcc -DYQ2OSTYPE=\"$(YQ2_OSTYPE)\" -DYQ2ARCH=\"$(YQ2_ARCH)\" ${COMMON_FILES} rendergl.c iqm.c ${CFLAGS} -o view-iqm
