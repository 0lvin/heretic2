COMMON_HEADERS = \
	../src/common/header/files.h \
	../src/client/refresh/constants/anorms.h \
	anorms.h \
	global.h \
	image.h \
	matrix.h \
	model.h \
	model_md2.h \
	palettes.h \
	util.h \
	v_font.h

COMMON_FILES = \
	image_pcx.c \
	palettes.c \
	anorms.c \
	model_md2.c \
	model_md3.c \
	model_dkm.c \
	model_mdl.c \
	model_mdo.c \
	model_fm.c \
	util.c \
	image.c \
	matrix.c \
	model.c

all: viewer modelconv

viewer: viewer.c v_font.c ${COMMON_FILES} ${COMMON_HEADERS}
	gcc -g -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT ${COMMON_FILES} v_font.c viewer.c -lSDL -lGL -lm -o viewer

modelconv: modelconv.c ${COMMON_FILES} ${COMMON_HEADERS}
	gcc -g ${COMMON_FILES} modelconv.c -lm -o modelconv
