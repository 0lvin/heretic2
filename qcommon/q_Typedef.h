//
// Copyright 1998 Raven Software
//
// Heretic II
//
#ifndef QCOMMON_TYPEDEF_H
#define QCOMMON_TYPEDEF_H

#include "q_shared.h"

typedef vec_t vec2_t[2];
typedef double vec3d_t[3];

typedef float matrix3_t[3][3];
typedef float matrix3d_t[3][3];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

typedef unsigned char 		byte;
#ifndef __cplusplus
// typedef enum {false, true}	qboolean;
#else
typedef int qboolean;
#endif

typedef struct edict_s edict_t;

typedef struct paletteRGBA_s
{
	union
	{
		struct
		{
			byte r,g,b,a;
		};
		unsigned c;
		byte c_array[4];
	};
} paletteRGBA_t;

typedef struct
{
	int				frame;
	paletteRGBA_t	color;
	byte			flags;
	int				skin;
} fmnodeinfo_t;

// flags

#define FMNI_USE_FRAME		(1<<0)
#define FMNI_USE_COLOR		(1<<1)
#define FMNI_USE_SKIN		(1<<2)
#define FMNI_NO_LERP		(1<<3)
#define FMNI_NO_DRAW		(1<<4)
#define FMNI_USE_REFLECT	(1<<5)

//=============================================

#define MAX_COLORS	33

extern paletteRGBA_t TextPalette[MAX_COLORS];

typedef enum
{
	P_BLACK,
	P_RED,
	P_GREEN,
	P_YELLOW,
	P_BLUE,
	P_PURPLE,
	P_CYAN,
	P_WHITE,

	P_HBLACK,
	P_HRED,
	P_HGREEN,
	P_HYELLOW,
	P_HBLUE,
	P_HPURPLE,
	P_HCYAN,
	P_HWHITE,

	P_DESIGNER,
	P_PROGRAMMER,
	P_OBJ_NORMAL,
	P_OBJ_BOLD,
	P_OBIT,
	P_CAPTION,
	P_CHAT,
	P_TEAM,

	P_VERSION,
	P_FRAGS,
	P_ALTFRAGS,
	P_MENUFIELD,
	P_MSGBOX,
	P_HEADER,
	P_CRED_TITLE,
	P_CRED_CONTENT,
	P_FRAGNAME

} PalIdx_t;

void Com_ColourPrintf (PalIdx_t colour, char *msg, ...);

#endif
