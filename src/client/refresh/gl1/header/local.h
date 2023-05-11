/*
 * Copyright (C) 1997-2001 Id Software, Inc.
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
 * Local header for the refresher.
 *
 * =======================================================================
 */

#ifndef REF_LOCAL_H
#define REF_LOCAL_H

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "../../ref_shared.h"
#include "qgl.h"


#ifndef GL_COLOR_INDEX8_EXT
 #define GL_COLOR_INDEX8_EXT GL_COLOR_INDEX
#endif

#ifndef GL_EXT_texture_filter_anisotropic
 #define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
 #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#ifndef GL_VERSION_1_3
 #define GL_TEXTURE0 0x84C0
 #define GL_TEXTURE1 0x84C1
#endif

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

#ifndef GL_MULTISAMPLE_FILTER_HINT_NV
#define GL_MULTISAMPLE_FILTER_HINT_NV 0x8534
#endif

#define TEXNUM_LIGHTMAPS 1024
#define TEXNUM_SCRAPS 1152
#define TEXNUM_IMAGES 1153
#define MAX_GLTEXTURES 1024
#define MAX_SCRAPS 1
#define BLOCK_WIDTH 128
#define BLOCK_HEIGHT 128
#define REF_VERSION "Yamagi Quake II OpenGL Refresher"
#define BACKFACE_EPSILON 0.01
#define LIGHTMAP_BYTES 4
#define MAX_LIGHTMAPS 128
#define GL_LIGHTMAP_FORMAT GL_RGBA

/* up / down */
#define PITCH 0

/* left / right */
#define YAW 1

/* fall over */
#define ROLL 2

extern viddef_t vid;


enum stereo_modes {
	STEREO_MODE_NONE,
	STEREO_MODE_OPENGL,
	STEREO_MODE_ANAGLYPH,
	STEREO_MODE_ROW_INTERLEAVED,
	STEREO_MODE_COLUMN_INTERLEAVED,
	STEREO_MODE_PIXEL_INTERLEAVED,
	STEREO_SPLIT_HORIZONTAL,
	STEREO_SPLIT_VERTICAL,
};

enum opengl_special_buffer_modes {
	OPENGL_SPECIAL_BUFFER_MODE_NONE,
	OPENGL_SPECIAL_BUFFER_MODE_STEREO,
	OPENGL_SPECIAL_BUFFER_MODE_STENCIL,
};

typedef struct image_s
{
	char name[MAX_QPATH];               /* game path, including extension */
	imagetype_t type;
	int width, height;                  /* source image */
	int upload_width, upload_height;    /* after power of two and picmip */
	int registration_sequence;          /* 0 = free */
	struct msurface_s *texturechain;    /* for sort-by-texture world drawing */
	int texnum;                         /* gl texture binding */
	float sl, tl, sh, th;               /* 0,0 - 1,1 unless part of the scrap */
	qboolean scrap;
	qboolean has_alpha;

	qboolean paletted;
} image_t;

typedef enum
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

#include "model.h"

void GL_BeginRendering(int *x, int *y, int *width, int *height);
void GL_EndRendering(void);

void GL_SetDefaultState(void);
void GL_UpdateSwapInterval( void );

extern float gldepthmin, gldepthmax;

typedef struct
{
	float x, y, z;
	float s, t;
	float r, g, b;
} glvert_t;

#include "../../../../common/header/common.h"
#include "../../../../../h2common/vector.h"

typedef struct CL_SkeletalJoint_s
{
	int children;
	vec3_t angles;
} CL_SkeletalJoint_t;

#define	MAX_LBM_HEIGHT		480

#define BACKFACE_EPSILON	0.01

//====================================================

extern	image_t		gltextures[MAX_GLTEXTURES];
extern	int			numgltextures;

extern	image_t		*r_notexture;
extern	image_t		*r_particletexture;
extern	entity_t	*currententity;
extern	model_t		*currentmodel;
extern	int			r_visframecount;
extern	int			r_framecount;
extern	cplane_t	frustum[4];
extern	int			c_brush_polys, c_alias_polys;

extern	int			gl_filter_min, gl_filter_max;

//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_newrefdef;
extern	int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

extern	cvar_t	*r_norefresh;
extern	cvar_t	*r_lefthand;
extern	cvar_t	*r_drawentities;
extern	cvar_t	*r_drawworld;
extern	cvar_t	*r_speeds;
extern	cvar_t	*r_fullbright;
extern	cvar_t	*r_novis;
extern	cvar_t	*r_nocull;
extern	cvar_t	*r_lerpmodels;

extern	cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

extern cvar_t	*gl_vertex_arrays;

extern cvar_t	*gl_ext_swapinterval;
extern cvar_t	*gl_ext_palettedtexture;
extern cvar_t	*gl_ext_multitexture;
extern cvar_t	*gl_ext_pointparameters;
extern cvar_t	*gl_ext_compiled_vertex_array;

extern cvar_t	*gl1_particle_min_size;
extern cvar_t	*gl1_particle_max_size;
extern cvar_t	*gl1_particle_size;
extern cvar_t	*gl1_particle_att_a;
extern cvar_t	*gl1_particle_att_b;
extern cvar_t	*gl1_particle_att_c;

extern	cvar_t	*gl_nosubimage;
extern	cvar_t	*gl_bitdepth;
extern	cvar_t	*r_mode;
extern	cvar_t	*r_lightmap;
extern	cvar_t	*r_shadows;
extern	cvar_t	*gl1_dynamic;
extern  cvar_t  *gl_monolightmap;
extern	cvar_t	*gl_nobind;
extern	cvar_t	*gl1_round_down;
extern	cvar_t	*gl1_picmip;
extern	cvar_t	*gl_skymip;
extern	cvar_t	*gl_showtris;
extern	cvar_t	*gl_finish;
extern	cvar_t	*gl1_ztrick;
extern	cvar_t	*r_clear;
extern	cvar_t	*r_cull;
extern	cvar_t	*gl_poly;
extern	cvar_t	*gl_texsort;
extern	cvar_t	*gl_polyblend;
extern	cvar_t	*gl1_flashblend;
extern	cvar_t	*r_lightmaptype;
extern	cvar_t	*r_modulate;
extern	cvar_t	*gl_playermip;
extern	cvar_t	*gl_drawbuffer;
extern	cvar_t	*gl_3dlabs_broken;
extern  cvar_t  *gl_driver;
extern	cvar_t	*r_vsync;
extern	cvar_t	*gl_texturemode;
extern	cvar_t	*gl1_texturealphamode;
extern	cvar_t	*gl1_texturesolidmode;
extern  cvar_t  *gl_saturatelighting;
extern  cvar_t  *r_lockpvs;

extern	cvar_t	*vid_fullscreen;
extern	cvar_t	*vid_gamma;

extern	cvar_t		*intensity;

extern	int		r_lightmap_format;
extern	int		gl_solid_format;
extern	int		gl_alpha_format;
extern	int		gl_tex_solid_format;
extern	int		gl_tex_alpha_format;

extern	int		c_visible_lightmaps;
extern	int		c_visible_textures;

extern	float	r_world_matrix[16];

void R_TranslatePlayerSkin (int playernum);
void R_Bind (int texnum);
void GL_TexEnv( GLenum value );

void R_LightPoint (vec3_t p, vec3_t color);
void R_PushDlights (void);

//====================================================================

extern	model_t	*r_worldmodel;

extern	unsigned	d_8to24table[256];

extern	int		registration_sequence;

void V_AddBlend (float r, float g, float b, float a, float *v_blend);

int 	R_Init( void *hinstance, void *hWnd );
void	R_Shutdown( void );

void R_RenderView (refdef_t *fd);
void GL_ScreenShot_f (void);
void R_DrawAliasModel (entity_t *e);
void R_DrawBrushModel (entity_t *e);
void R_DrawSpriteModel (entity_t *e);
void R_DrawBeam( entity_t *e );
void R_DrawWorld (void);
void R_RenderDlights (void);
void R_DrawAlphaSurfaces (void);
void R_RenderBrushPoly (msurface_t *fa);
void R_InitParticleTexture (void);
void Draw_InitLocal (void);
void GL_SubdivideSurface (msurface_t *fa);
qboolean R_CullBox (vec3_t mins, vec3_t maxs);
void R_RotateForEntity (entity_t *e);
void R_MarkLeaves (void);

glpoly_t *WaterWarpPolyVerts (glpoly_t *p);
void EmitWaterPolys (msurface_t *fa);
void R_AddSkySurface (msurface_t *fa);
void R_ClearSkyBox (void);
void R_DrawSkyBox (void);
void R_MarkLights (dlight_t *light, int bit, mnode_t *node);

void COM_StripExtension (char *in, char *out);

void    Draw_Image(int x, int y, int w, int h, float alpha, qboolean scale, image_t* gl);
void	Draw_GetPicSize (int *w, int *h, char *name);
void	Draw_Pic (int x, int y, char *name);
void	Draw_StretchPic (int x, int y, int w, int h, char *pic, float alpha, qboolean scale);
void	Draw_Char (int x, int y, int c);
void	Draw_TileClear (int x, int y, int w, int h, char *name);
void	Draw_Fill (int x, int y, int w, int h, byte r, byte g, byte b);
void	Draw_FadeScreen (void);
void	Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data);

void	R_BeginFrame( float camera_separation );
void	R_SwapBuffers( int );
void	R_SetPalette ( const unsigned char *palette);

int		Draw_GetPalette (void);

void GL_ResampleTexture (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight);

struct image_s *R_RegisterSkin(char* name, qboolean* retval);

void LoadPCX (char *filename, byte **pic, byte **palette, int *width, int *height);
image_t *GL_LoadPic (char *name, byte *pic, int width, int height, imagetype_t type, int bits);
image_t	*GL_FindImage (char *name, imagetype_t type);
void	GL_TextureMode( char *string );
void	GL_ImageList_f (void);

void	GL_SetTexturePalette( unsigned palette[256] );

void	GL_InitImages (void);
void	GL_ShutdownImages (void);

void	GL_FreeUnusedImages (void);

void GL_TextureAlphaMode( char *string );
void GL_TextureSolidMode( char *string );

/*
** GL config stuff
*/
#define GL_RENDERER_VOODOO		0x00000001
#define GL_RENDERER_VOODOO2   	0x00000002
#define GL_RENDERER_VOODOO_RUSH	0x00000004
#define GL_RENDERER_BANSHEE		0x00000008
#define		GL_RENDERER_3DFX		0x0000000F

#define GL_RENDERER_PCX1		0x00000010
#define GL_RENDERER_PCX2		0x00000020
#define GL_RENDERER_PMX			0x00000040
#define		GL_RENDERER_POWERVR		0x00000070

#define GL_RENDERER_PERMEDIA2	0x00000100
#define GL_RENDERER_GLINT_MX	0x00000200
#define GL_RENDERER_GLINT_TX	0x00000400
#define GL_RENDERER_3DLABS_MISC	0x00000800
#define		GL_RENDERER_3DLABS	0x00000F00

#define GL_RENDERER_REALIZM		0x00001000
#define GL_RENDERER_REALIZM2	0x00002000
#define		GL_RENDERER_INTERGRAPH	0x00003000

#define GL_RENDERER_3DPRO		0x00004000
#define GL_RENDERER_REAL3D		0x00008000
#define GL_RENDERER_RIVA128		0x00010000
#define GL_RENDERER_DYPIC		0x00020000

#define GL_RENDERER_V1000		0x00040000
#define GL_RENDERER_V2100		0x00080000
#define GL_RENDERER_V2200		0x00100000
#define		GL_RENDERER_RENDITION	0x001C0000

#define GL_RENDERER_O2          0x00100000
#define GL_RENDERER_IMPACT      0x00200000
#define GL_RENDERER_RE			0x00400000
#define GL_RENDERER_IR			0x00800000
#define		GL_RENDERER_SGI			0x00F00000

#define GL_RENDERER_MCD			0x01000000
#define GL_RENDERER_OTHER		0x80000000

void R_TextureAlphaMode(char *string);
void R_TextureSolidMode(char *string);
int Scrap_AllocBlock(int w, int h, int *x, int *y);

/* GL extension emulation functions */
void R_DrawParticles2(int n,
		const particle_t particles[],
		const unsigned *colortable);

/*
 * GL config stuff
 */

typedef struct
{
	const char *renderer_string;
	const char *vendor_string;
	const char *version_string;
	const char *extensions_string;

	int major_version;
	int minor_version;

	// ----

	qboolean anisotropic;
	qboolean npottextures;
	qboolean palettedtexture;
	qboolean pointparameters;

	// ----

	float max_anisotropy;
} glconfig_t;

typedef struct
{
	float inverse_intensity;
	qboolean fullscreen;

	int prev_mode;

	unsigned char *d_16to8table;

	int lightmap_textures;

	int currenttextures[2];
	int currenttmu;
	GLenum currenttarget;

	float camera_separation;
	enum stereo_modes stereo_mode;

	qboolean stencil;
} glstate_t;

typedef struct
{
	int internal_format;
	int current_lightmap_texture;

	msurface_t *lightmap_surfaces[MAX_LIGHTMAPS];

	int allocated[BLOCK_WIDTH];

	/* the lightmap texture data needs to be kept in
	   main memory so texsubimage can update properly */
	byte lightmap_buffer[4 * BLOCK_WIDTH * BLOCK_HEIGHT];
} gllightmapstate_t;

extern glconfig_t gl_config;
extern glstate_t gl_state;

void R_DrawBigFont(int x, int y, char *text, float alpha);
int BF_Strlen(char *text);
void R_BookDrawPic(int w, int h, char *name, float scale);
void R_DrawInitCinematic(int w, int h, char *overlay, char *backdrop);
void R_DrawCloseCinematic();
void R_DrawCinematic(int cols, int rows, byte *data, paletteRGB_t *palette, float alpha);

/*
====================================================================

IMPORTED FUNCTIONS

====================================================================
*/
extern	refimport_t	ri;

/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void GLimp_BeginFrame( float camera_separation );
void GLimp_EndFrame( void );
int GLimp_Init( void *hinstance, void *hWnd );
void GLimp_Shutdown( void );
int GLimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen );
void GLimp_AppActivate( qboolean active );
void GLimp_LogNewFrame( void );

extern image_t* atlas_particle;
extern image_t* atlas_aparticle;

#endif
