//
// Heretic II
// Copyright 1998 Raven Software
//

#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "../client/header/client.h"
#include "header/game.h"
#include "header/client_effects.h"
#include "effects/client_effects.h"

// Structure containing functions and data pointers exported from the effects DLL.
client_fx_export_t *fxe;

// Handle to player DLL.

static void *effects_library = NULL;

float EffectEventIdTimeArray[1000];
int cl_effectpredict[1000];

predictinfo_t predictInfo;
EffectsBuffer_t clientPredEffects;
float PlayerAlpha = 1.0f;
void CL_ClearLightStyles(void);

static int
CL_GetEffect(centity_t* ent, int flags, char* format, ...)
{
	sizebuf_t* msg;
	va_list args;

	if (!ent)
	{
		msg = &net_message;
	}
	else
	{
		msg = fxe->fxMsgBuf;
	}

	va_start(args, format);

	int len = strlen(format);
	for (int i = 0; i < len; i++)
	{
		switch (format[i])
		{
		case 'b':
		{
			byte* b = va_arg(args, byte*);
			*b = MSG_ReadByte(msg);
		}
			break;
		case 'd':
			MSG_ReadDir(msg, va_arg(args, float*));
			break;
		case 'f':
		{
			float* f = va_arg(args, float*);
			*f = MSG_ReadFloat(msg);
		}
			break;
		case 'i':
		{
			long* l = va_arg(args, long*);
			*l = MSG_ReadLong(msg);
		}
			break;
		case 'p':
		case 'v':
			MSG_ReadPos(msg, va_arg(args, float*), cls.serverProtocol);
			break;
		case 's':
		{
			short* s = va_arg(args, short*);
			*s = MSG_ReadShort(msg);
		}
			break;
		case 't':
		case 'u':
		case 'x':
			MSG_ReadPos(msg, va_arg(args, float*), cls.serverProtocol);
			break;
		default:
			break;
		}
	}

	return len;
}

static qboolean
InCameraPVS(vec3_t point)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
	return true;
}

// Screen flash set
static void
Activate_Screen_Flash(int color)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
}

// Screen flash set
static void
Activate_Screen_Shake(float intensity, float duration, float current_time, int flags)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
}

static qboolean
Get_Crosshair(vec3_t origin, byte* type)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
	return true;
}

trace_t CL_PMTrace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end);

static trace_t
CL_NewTrace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int brushmask, int flags)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}

	return CL_PMTrace(start, mins, maxs, end); // jmarshall: incomplete
}

static void
CL_Sys_Error(int errLevel, const char* fmt, ...)
{
	va_list		argptr;
	char		msg[4096];

	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	Sys_Error(msg); // TODO vargs
}

static void
CL_Printf(int errLevel, const char* fmt, ...) {
	va_list		argptr;
	char		msg[4096];

	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	Com_Printf("%s", msg);
}

static void
CL_ReadPos(sizebuf_t *sb, vec3_t pos)
{
	MSG_ReadPos(sb, pos, cls.serverProtocol);
}

extern int r_numentities;
extern entity_t r_entities[MAX_ENTITIES];

extern int r_numdlights;
extern dlight_t r_dlights[MAX_DLIGHTS];

extern lightstyle_t r_lightstyles[MAX_LIGHTSTYLES];

extern int r_numparticles;
extern particle_t r_particles[MAX_PARTICLES];

static client_fx_import_t cl_game_import;

// ************************************************************************************************
// E_Freelib
// ---------
// ************************************************************************************************

void
E_Freelib()
{
	if(!effects_library)
	{
		return;
	}

	if (fxe)
	{
		fxe->ShutDown();
	}
	fxe = NULL;

#if _WIN32
	FreeLibrary(effects_library);
#else
	dlclose (effects_library);
#endif

	effects_library = NULL;
	Com_Printf("Shutting down Effect library.\n");
}

// ************************************************************************************************
// E_Load
// ------
// ************************************************************************************************

void *
E_Load(void)
{
	client_fx_export_t *(*P_GetFXAPI)(client_fx_import_t import);

	char name[MAX_OSPATH];
	const char *path;
#if defined(_WIN32)
	WCHAR wname[MAX_OSPATH];
	const char *effectsname = "effects.dll";
#elif defined(__APPLE__)
	const char *effectsname = "effects.dylib";
#else
	const char *effectsname = "effects.so";
#endif

	if (effects_library)
	{
		Com_Error(ERR_FATAL, "%s without E_Freelib", __func__);
	}

	Com_Printf("Loading library: %s\n", effectsname);

	/* now run through the search paths */
	path = NULL;

	while (1)
	{
		FILE *fp;

		path = FS_NextPath(path);

		if (!path)
		{
			return NULL;     /* couldn't find one anywhere */
		}

		snprintf(name, MAX_OSPATH, "%s/%s", path, effectsname);

		/* skip it if it just doesn't exist */
		fp = fopen(name, "rb");

		if (fp == NULL)
		{
			continue;
		}

		fclose(fp);

#ifdef _WIN32
		MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, MAX_OSPATH);
		effects_library = LoadLibraryW(wname);
#else

#ifdef USE_SANITIZER
		effects_library = dlopen(name, RTLD_NOW | RTLD_NODELETE);
#else
		effects_library = dlopen(name, RTLD_NOW);
#endif

#endif
		if (effects_library)
		{
			Com_Printf("Loading library: %s\n", name);
			break;
		}
#ifndef _WIN32
		else
		{
			const char *str_p;

			Com_Printf("Failed to load library: %s\n: ", name);

			path = (char *)dlerror();
			str_p = strchr(path, ':');   /* skip the path (already shown) */

			if (str_p == NULL)
			{
				str_p = path;
			}
			else
			{
				str_p++;
			}

			Com_Printf("%s\n", str_p);

			return NULL;
		}
#endif
	}

#ifdef _WIN32
	P_GetFXAPI = (void *)GetProcAddress(effects_library, "GetFXAPI");
#else
	P_GetFXAPI = (void *)dlsym(effects_library, "GetFXAPI");
#endif

	if (!P_GetFXAPI)
	{
		E_Freelib();
		return NULL;
	}

	cl_game_import.cl = &cl;
	cl_game_import.cls = &cls;

	cl_game_import.r_numentities = &r_numentities;
	cl_game_import.r_entities = r_entities;

	cl_game_import.r_numdlights = &r_numdlights;
	cl_game_import.r_dlights = r_dlights;

	cl_game_import.r_numparticles = &r_numparticles;
	cl_game_import.r_particles = r_particles;

	cl_game_import.server_entities = cl_entities;
	cl_game_import.parse_entities = cl_parse_entities;
	cl_game_import.EffectEventIdTimeArray = EffectEventIdTimeArray;
	cl_game_import.leveltime = (float*)&cl.time;
	cl_game_import.clientPredEffects = &clientPredEffects;
	cl_game_import.net_message = &net_message;
	cl_game_import.PlayerEntPtr = (entity_t**)&cl_entities[0];
	cl_game_import.PlayerAlpha = (float*)&PlayerAlpha;
	cl_game_import.predictinfo = &predictInfo;
	cl_game_import.cl_effectpredict = &cl_effectpredict[0];
	cl_game_import.Sys_Error = CL_Sys_Error;
	cl_game_import.Com_Error = Com_Error;
	cl_game_import.Con_Printf = CL_Printf;
	cl_game_import.Com_Printf = Com_Printf;
	cl_game_import.Cvar_Get = Cvar_Get;
	cl_game_import.Cvar_Set = Cvar_Set;
	cl_game_import.Cvar_SetValue = Cvar_SetValue;
	cl_game_import.Cvar_VariableValue = Cvar_VariableValue;
	cl_game_import.Cvar_VariableString = Cvar_VariableString;
	cl_game_import.Activate_Screen_Flash = Activate_Screen_Flash;
	cl_game_import.Activate_Screen_Shake = Activate_Screen_Shake;
	cl_game_import.Get_Crosshair = Get_Crosshair;
	cl_game_import.S_StartSound = S_StartSound;
	cl_game_import.S_RegisterSound = S_RegisterSound;
	cl_game_import.RegisterModel = re.RegisterModel;
	cl_game_import.GetEffect = CL_GetEffect;
	cl_game_import.TagMalloc = Z_TagMalloc;
	cl_game_import.TagFree = Z_Free;
	cl_game_import.FreeTags = Z_FreeTags;
	cl_game_import.cl_predict = cl_predict;
	cl_game_import.Trace = CL_NewTrace;
	cl_game_import.InCameraPVS = InCameraPVS;
	cl_game_import.MSG_ReadByte = MSG_ReadByte;
	cl_game_import.MSG_ReadShort = MSG_ReadShort;
	cl_game_import.MSG_ReadLong = MSG_ReadLong;
	cl_game_import.MSG_ReadFloat = MSG_ReadFloat;
	cl_game_import.MSG_ReadPos = CL_ReadPos;
	cl_game_import.MSG_ReadDir = MSG_ReadDir;
	cl_game_import.MSG_ReadData = MSG_ReadData;
	cl_game_import.CL_RunLightStyles = CL_RunLightStyles;
	cl_game_import.CL_ClearLightStyles = CL_ClearLightStyles;

	fxe = P_GetFXAPI(cl_game_import);
	if (fxe->api_version != GAME_API_VERSION)
	{
		Com_Error(0, "%s has incompatible api_version", name);
		E_Freelib();
		return NULL;
	}

	fxe->Init();

	Com_Printf("------------------------------------\n");

	return fxe;
}
