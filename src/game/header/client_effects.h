//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef GAME_CLIENT_EFFECTS_H
#define GAME_CLIENT_EFFECTS_H

//=============================================================================
#include <limits.h>

// ********************************************************************************************
// predictinfo_t
// -------------
// Repositiory for all elements of player rendering that need to be predicted. When prediction
// is active, the values below are written by CL_DoPrediction() and read by AddServerEntities()
// instead of using values derived from server sent data.
// ********************************************************************************************

typedef struct
{
	int				prevFrame,currFrame,
					prevSwapFrame,currSwapFrame;
	vec3_t			prevAngles,currAngles;
	float			playerLerp;
	int				effects,
					renderfx,
					skinnum,
					clientnum;
	fmnodeinfo_t	fmnodeinfo[MAX_FM_MESH_NODES];
} predictinfo_t;

//
// these are the data and functions exported by the client fx module
//
typedef struct
{
	// if api_version is different, the dll cannot be used
	int		api_version;

	void (*Init)();
	void (*ShutDown)();

	void (*Clear)();

	void (*RegisterSounds)();
	void (*RegisterModels)();

	void (*ParseClientEffects)(centity_t *cent);
	void (*RemoveClientEffects)(centity_t *cent);

	void (*AddPacketEntities)(frame_t *frame);
	void (*AddEffects)(qboolean freeze);
	void (*UpdateEffects)();

	sizebuf_t	*fxMsgBuf;
} client_fx_export_t;

extern client_fx_export_t *fxe;

//
// these are the data and functions imported by the client fx module
//
typedef struct
{
	client_state_t	*cl;
	client_static_t *cls;

	int *r_numentities;
	entity_t *r_entities;

	int *r_numdlights;
	dlight_t *r_dlights;

	int *r_numparticles;
	particle_t *r_particles;

	// Client versions of the game entities.

	centity_t		*server_entities;

	// Buffer into which net stuff is parsed.

	entity_xstate_t	*parse_entities;

	sizebuf_t		*net_message;
	float			*PlayerAlpha;
	entity_t		**PlayerEntPtr;

	// Client prediction stuff.

	cvar_t			*cl_predict;
	int				*cl_effectpredict;
	predictinfo_t	*predictinfo;
	float			*leveltime;
	float			*EffectEventIdTimeArray;
	EffectsBuffer_t *clientPredEffects;
	//

	void	(*Sys_Error) (int err_level, const char *str, ...);
	void	(*Com_Error) (int code, const char *fmt, ...);
	void	(*Con_Printf) (int print_level, const char *str, ...);
	void	(*Com_Printf) (const char *msg, ...);

	//

	cvar_t *(*Cvar_Get) (const char *name, const char *value, int flags);
	cvar_t *(*Cvar_Set)( const char *name, const char *value );
	void	(*Cvar_SetValue)( const char *name, float value );
	float	(*Cvar_VariableValue) (const char *var_name);
	const char	*(*Cvar_VariableString) (const char *var_name);

	// allow the screen flash to be controlled from within the client effect DLL rather than going through the server.
	// this means we get 60 hz (hopefully) screen flashing, rather than 10 hz
	void	(*Activate_Screen_Flash)(int color);

	// allow the client to call a screen shake - done within the camera code, so we can shake the screen at 60hz
	void 	(*Activate_Screen_Shake)(float intensity, float duration, float current_time, int flags);

	qboolean	(*Get_Crosshair)(vec3_t origin, byte *type);

	void	(*S_StartSound)(vec3_t origin, int entnum, int entchannel, sfx_t *sfx, float fvol, float attenuation, float timeofs);
	struct sfx_s	*(*S_RegisterSound)(const char *name);
	struct model_s *(*RegisterModel) (const char *name);

	int		(*GetEffect)(centity_t *ent, int flags, char *format, ...);

	void	*(*TagMalloc)(int size, int tag);
	void	(*TagFree)(void *block);
	void	(*FreeTags)(int tag);

	int	(*MSG_ReadByte)(sizebuf_t *sb);
	int	(*MSG_ReadShort)(sizebuf_t *sb);
	int	(*MSG_ReadLong)(sizebuf_t *sb);
	float	(*MSG_ReadFloat)(sizebuf_t *sb);
	void	(*MSG_ReadPos)(sizebuf_t *sb, vec3_t pos);
	void	(*MSG_ReadDir)(sizebuf_t *sb, vec3_t vector);
	void	(*MSG_ReadData)(sizebuf_t *sb, void *buffer, int size);
	void	(*CL_RunLightStyles)(void);
	void	(*CL_ClearLightStyles)(void);

	trace_t	(*Trace)(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int brushmask, int flags);
	qboolean (*InCameraPVS)(vec3_t point);
} client_fx_import_t;

// ********************************************************************************************
// CF_XXX
// ------
// Flags for the client side entity to know if its been deleted on the server, or is server
// culled.
// ********************************************************************************************

#define CF_INUSE			0x00000001
#define CF_SERVER_CULLED	0x00000002

/* This doesn't use the macro because of referencing weirdness. */
#define NUM_PARTICLE_TYPES	62

#endif

// end
