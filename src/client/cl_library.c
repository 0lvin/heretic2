//
// Heretic II
// Copyright 1998 Raven Software
//
#include "header/client.h"
#include "../../h2common/resourcemanager.h"

client_fx_export_t fxe;

float EffectEventIdTimeArray[1000];
int cl_effectpredict[1000];

predictinfo_t predictInfo;
EffectsBuffer_t clientPredEffects;
float PlayerAlpha = 1.0f;

client_fx_export_t GetfxAPI(client_fx_import_t import);

ResourceManager_t FXBufMgnr;

extern sizebuf_t* fxMsgBuf;

int CL_GetEffect(centity_t* ent, int flags, char* format, ...) {
	sizebuf_t* msg;
	va_list args;
	EffectsBuffer_t* clientEffects;

	if (!ent)
	{
		msg = &net_message;
	}
	else
	{
		//if (cl_effectpredict)
		//{
		//	clientEffects = (EffectsBuffer_t*)&clientPredEffects;
		//	clientEffects = (EffectsBuffer_t*)&clientPredEffects;
		//}
		//else
		{
			clientEffects = &ent->current.clientEffects;
			clientEffects = &ent->current.clientEffects;
		}
		msg = fxMsgBuf;
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
			MSG_ReadPos(msg, va_arg(args, float*));
			break;
		case 's':
		{
			short* s = va_arg(args, short*);
			*s = MSG_ReadShort(msg);
		}
			break;
		case 't':
			MSG_ReadPos(msg, va_arg(args, float*));
			break;
		case 'u':
			MSG_ReadPos(msg, va_arg(args, float*));
			break;
		case 'x':
			MSG_ReadPos(msg, va_arg(args, float*));
			break;
		default:
			break;
		}
	}

	return len;
}

void CL_ShutdownClientEffects()
{
	fxe.ShutDown();
}

qboolean
InCameraPVS(vec3_t point)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
	return true;
}

// Screen flash set
void
Activate_Screen_Flash(int color)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
}

// Screen flash set
void
Activate_Screen_Shake(float intensity, float duration, float current_time, int flags)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
}

// Screen flash unset
void
Deactivate_Screen_Flash(void)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
}

void
Deactivate_Screen_Shake(void)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
}

qboolean
Get_Crosshair(vec3_t origin, byte* type)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
	return true;
}

trace_t CL_PMTrace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end);

trace_t
CL_NewTrace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int brushmask, int flags)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}

	return CL_PMTrace(start, mins, maxs, end); // jmarshall: incomplete
}

void CL_Sys_Error(int errLevel, char* fmt, ...)
{
	va_list		argptr;
	char		msg[4096];

	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	Sys_Error(msg); // TODO vargs
}

void CL_Printf(int errLevel, char* fmt, ...) {
	va_list		argptr;
	char		msg[4096];

	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	Com_Printf(msg);
}

extern int r_numentities;
extern entity_t *r_entities[MAX_ENTITIES];

extern int r_num_alpha_entities;
extern entity_t *r_alpha_entities[MAX_ALPHA_ENTITIES];

extern int r_numdlights;
extern dlight_t r_dlights[MAX_DLIGHTS];

extern lightstyle_t r_lightstyles[MAX_LIGHTSTYLES];

extern int r_numparticles;
extern particle_t r_particles[MAX_PARTICLES];

extern int r_anumparticles;
extern particle_t r_aparticles[MAX_PARTICLES];

static client_fx_import_t cl_game_import;

int
CL_InitClientEffects(const char* name)
{
	int result;

	Com_Printf("------ Loading %s ------\n", name);

	cl_game_import.cl_predict = cl_predict;
	cl_game_import.cl = &cl;
	cl_game_import.cls = &cls;

	cl_game_import.r_numentities = &r_numentities;
	cl_game_import.r_entities = r_entities;

	cl_game_import.r_num_alpha_entities = &r_num_alpha_entities;
	cl_game_import.r_alpha_entities = r_alpha_entities;

	cl_game_import.r_numdlights = &r_numdlights;
	cl_game_import.r_dlights = r_dlights;

	cl_game_import.r_numparticles = &r_numparticles;
	cl_game_import.r_particles = r_particles;

	cl_game_import.r_anumparticles = &r_anumparticles;
	cl_game_import.r_aparticles = r_aparticles;

	cl_game_import.server_entities = &cl_entities[0];
	cl_game_import.parse_entities = cl_parse_entities;
	cl_game_import.EffectEventIdTimeArray = EffectEventIdTimeArray;
	cl_game_import.leveltime = (float*)&cl.time;
	cl_game_import.clientPredEffects = &clientPredEffects;
	cl_game_import.net_message = &net_message;
	cl_game_import.PlayerEntPtr = (entity_t**)&cl_entities[0];
	cl_game_import.PlayerAlpha = (float*)&PlayerAlpha;
	cl_game_import.predictinfo = &predictInfo;
	cl_game_import.FXBufMngr = &FXBufMgnr;
	cl_game_import.cl_effectpredict = &cl_effectpredict[0];
	cl_game_import.Sys_Error = CL_Sys_Error;
	cl_game_import.Com_Error = Com_Error;
	cl_game_import.Con_Printf = CL_Printf;
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
	cl_game_import.Trace = CL_NewTrace;
	cl_game_import.InCameraPVS = InCameraPVS;

	fxe = GetfxAPI(cl_game_import);
	if (fxe.api_version != 3)
	{
		CL_ShutdownClientEffects();
		Com_Error(0, "%s has incompatible api_version", name);
	}
	ResMngr_Con(&FXBufMgnr, 192, 256);
	Com_Printf("------------------------------------");
	result = 1;

	fxe.Init();

	return result;
}
