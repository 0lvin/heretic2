//
// Main.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include "../../common/header/common.h"
#include "client_effects.h"
#include "../common/fx.h"
#include "client_entities.h"
#include "particle.h"
#include "ce_dlight.h"
#include "../common/skeletons.h"
#include "../header/g_playstats.h"
#include "../header/game.h"

client_fx_import_t fxi;
static client_fx_export_t effectsExport;

cvar_t	*cl_camera_under_surface;
cvar_t	*r_farclipdist;
cvar_t	*r_nearclipdist;
cvar_t	*r_detail;
cvar_t	*fx_numinview;
cvar_t	*fx_numactive;
cvar_t	*clfx_gravity;
cvar_t	*fxTest1;
cvar_t	*fxTest2;
cvar_t	*fxTest3;
cvar_t	*fxTest4;
cvar_t	*cl_lerpdist2;
cvar_t	*cl_timedemo;
cvar_t	*crosshair;
cvar_t	*compass;

int	numprocessedparticles;
int	numrenderedparticles;

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

static float EffectEventIdTimeArray[1000];
static predictinfo_t predictInfo;
static float PlayerAlpha = 1.0f;

static void
RemoveEffectsFromCent(centity_t *cent)
{
	if (cent->effects)
	{
		RemoveOwnedEffectList(cent);
	}
}

static void
Clear(void)
{
	centity_t *owner;
	int i;

	if (clientEnts)
	{
		RemoveEffectList(&clientEnts);
	}

	for(i = 0, owner = fxi.server_entities; i < MAX_EDICTS; ++i, ++owner)
	{
		if (owner->effects)
		{
			RemoveOwnedEffectList(owner);
		}

		if (owner->current.clientEffects.buf)
		{
			fxi.TagFree(owner->current.clientEffects.buf);
			owner->current.clientEffects.buf = NULL;
		}
	}

	fxi.CL_ClearLightStyles();

	memset(&CircularList[0],0,sizeof(CircularList));
	if (r_detail && r_detail->value == DETAIL_LOW)
	{
		total_circle_entries = 30;
	}
	else if (r_detail && r_detail->value == DETAIL_NORMAL)
	{
		total_circle_entries = 50;
	}
	else
	{
		total_circle_entries = MAX_ENTRIES_IN_CIRCLE_LIST;
	}
}

static void
Init(void)
{
	int i;

	InitResourceManager();
	InitParticleMngrMngr();
	InitFMNodeInfoMngr();
	InitEntityMngr();
	InitMsgMngr();
	InitDLightMngr();

	for(i = 0; i < CE_NUM_CLASSIDS; ++i)
	{
		cg_classStaticsInits[i]();
	}

	clientEnts = NULL;

	cl_camera_under_surface = fxi.Cvar_Get( "cl_camera_under_surface", "0", 0 );
	r_farclipdist = fxi.Cvar_Get("r_farclipdist", FAR_CLIP_DIST, 0);
	r_nearclipdist = fxi.Cvar_Get("r_nearclipdist", NEAR_CLIP_DIST, 0);
	r_detail = fxi.Cvar_Get("r_detail", DETAIL_DEFAULT, CVAR_ARCHIVE);
	fx_numinview = fxi.Cvar_Get("fx_numinview", "0", 0);
	fx_numactive = fxi.Cvar_Get("fx_numactive", "0", 0);
	clfx_gravity = fxi.Cvar_Get("clfx_gravity", "675.0", 0);
	cl_timedemo = fxi.Cvar_Get("timedemo","0",0);
	compass = fxi.Cvar_Get("compass", "0", CVAR_ARCHIVE);

	fxTest1 = fxi.Cvar_Get("fxTest1", "0", 0);
	fxTest2 = fxi.Cvar_Get("fxTest2", "0", 0);
	fxTest3 = fxi.Cvar_Get("fxTest3", "0", 0);
	fxTest4 = fxi.Cvar_Get("fxTest4", "0", 0);

	cl_lerpdist2 = fxi.Cvar_Get("cl_lerpdist2", "10000", 0);
	crosshair = fxi.Cvar_Get("crosshair", "0", CVAR_ARCHIVE);

	Clear();
}

static void
ShutDown(void)
{
	Clear();

	ReleaseParticleMngrMngr();
	ReleaseEntityMngr();
	ReleaseFMNodeInfoMngr();
	ReleaseDLightMngr();
	ReleaseMsgMngr();
	ShutdownResourceManager();
}

/*
==============
AddEffects

==============
*/
static int	num_owned_inview;

void AddEffects(void)
{
	int i;
	centity_t *owner;
	int	num_free_inview = 0;

	if (clientEnts)
	{
		num_free_inview += AddEffectsToView(&clientEnts, NULL);
	}

	// Add all effects which are attatched to entities, that have no model.

	for(i = 0, owner = fxi.server_entities; i < MAX_EDICTS; ++i, ++owner)
	{
		// gak, think something else need to be done... maybe a list of centities with effects.

		if (owner->effects && (owner->current.effects & EF_ALWAYS_ADD_EFFECTS))
		{
			num_owned_inview += AddEffectsToView(&owner->effects, owner);
		}
	}

	if (fx_numinview->value)
	{
		fxi.Com_Printf("Active CE : free %d, owned %d. Particles : processed %d, rendered %d\n",
					num_free_inview, num_owned_inview, numprocessedparticles, numrenderedparticles);
	}
}

/*
==============
PostRenderUpdate

==============
*/
static void
PostRenderUpdate(void)
{
	int i;
	centity_t *owner;
	int	num_free_active = 0;
	int	num_owned_active = 0;

	numprocessedparticles = 0;
	numrenderedparticles = 0;

	if (clientEnts)
	{
		num_free_active += UpdateEffects(&clientEnts, NULL);
	}

	for(i = 0, owner = fxi.server_entities; i < MAX_EDICTS; ++i, ++owner)	// gak, think something else need to be done
	{											// maybe a list of centities with effects. . .
		if (owner->effects)
		{
			num_owned_active += UpdateEffects(&owner->effects, owner);
		}
	}

	fxi.CL_RunLightStyles();

	if (fx_numactive->value)
	{
		fxi.Com_Printf("Active CE : free %d, owned %d\n", num_free_active, num_owned_active);
	}
}

/*
==============
ParseClientEffects

==============
*/

static void
ParseEffects(sizebuf_t *msg_read, int num)
{
	int				i;
	int				flags = 0;
	unsigned		short effect;
	vec3_t			position;
	centity_t		*tempOwner = NULL;
	int				last_effect = -1;
	int				eventId = 0;
	qboolean		EffectIsFromServer;

	assert(num >= 0);

	if (num < 0)
	{
		fxi.Com_Error(ERR_DROP, "ParseClientEffects: number of effects < 0");
		return;
	}

	for(i = 0; i < num; ++i)
	{
		int index = 0;

		EffectIsFromServer = false;

		effect = fxi.MSG_ReadShort(msg_read);

		flags = fxi.MSG_ReadShort(msg_read);

		if (flags & (CEF_BROADCAST | CEF_MULTICAST))
		{
			index = fxi.MSG_ReadShort(msg_read);

			if (index) // 0 should indicate the world
			{
				if (index < 0)
				{
					fxi.Com_Error(ERR_DROP, "%s: unexpected message end for %d",
						__func__, effect);
					return;
				}

				if (index >= MAX_EDICTS)
				{
					fxi.Com_Error(ERR_DROP, "%s: unexpected entity index %d for %d\n",
						__func__, index, effect);
					return;
				}

				tempOwner = fxi.server_entities + index;
			}
		}

		if (flags & CEF_OWNERS_ORIGIN)
		{
			if (tempOwner)
			{
				VectorCopy(tempOwner->origin, position);
			}
			else
			{
				position[0] = position[1] = position[2] = 0;
			}
		}
		else
		{
			fxi.MSG_ReadPos(msg_read, position);

			if (tempOwner && !(flags & CEF_BROADCAST))
			{
				position[0] += tempOwner->origin[0];
				position[1] += tempOwner->origin[1];
				position[2] += tempOwner->origin[2];
			}
		}

		if (fxi.MSG_ReadByte(msg_read) != 0x3a)
		{
			fxi.Com_Error(ERR_DROP, "Invalid effect header\n");
			return;
		}

		assert(effect < NUM_FX);

		if (!(effect >= 0 && effect < NUM_FX))
		{
			fxi.Com_Error(ERR_DROP, "ParseClientEffects: bad effect %d last effect %d", effect, last_effect);
			return;
		}

		// Start the client-effect.

		clientEffectSpawners[effect].SpawnCFX(tempOwner, effect, flags, position);

		if ((EffectIsFromServer)&&(EffectEventIdTimeArray[eventId]<=*fxi.leveltime))
			EffectEventIdTimeArray[eventId]=0.0;

		if (flags & (CEF_BROADCAST|CEF_MULTICAST))
		{
			tempOwner = NULL;
		}

		last_effect = effect;
	}
}

int
FXGetEffect(centity_t* ent, int flags, char* format, ...)
{
	sizebuf_t* msg;
	va_list args;

	msg = fxi.net_message;

	va_start(args, format);

	int len = strlen(format);
	for (int i = 0; i < len; i++)
	{
		switch (format[i])
		{
		case 'b':
		{
			byte* b = va_arg(args, byte*);
			*b = fxi.MSG_ReadByte(msg);
		}
			break;
		case 'd':
			fxi.MSG_ReadDir(msg, va_arg(args, float*));
			break;
		case 'f':
		{
			float* f = va_arg(args, float*);
			*f = fxi.MSG_ReadFloat(msg);
		}
			break;
		case 'i':
		{
			long* l = va_arg(args, long*);
			*l = fxi.MSG_ReadLong(msg);
		}
			break;
		case 'p':
		case 'v':
			fxi.MSG_ReadPos(msg, va_arg(args, float*));
			break;
		case 's':
		{
			short* s = va_arg(args, short*);
			*s = fxi.MSG_ReadShort(msg);
		}
			break;
		case 't':
		case 'u':
		case 'x':
			fxi.MSG_ReadPos(msg, va_arg(args, float*));
			break;
		default:
			break;
		}
	}

	return len;
}

static entity_t		sv_ents[MAX_ENTITIES];

static void
AddServerEntities(frame_t *frame)
{
	entity_t			*ent;
	float				autorotate, macerotate;
	int					i;
	int					pnum;
	centity_t			*cent;
	int					autoanim;
	int					effects, renderfx;
	int					numEntsToAdd;
	vec3_t				dist;
	clientinfo_t		*ci;
	int					clientnum;
	qboolean			isPredictedPlayer;

	PrepAddEffectsToView();
	num_owned_inview = 0;

	// Bonus items rotate at a fixed rate.

	autorotate = anglemod(fxi.cl->time / 10.0);
	macerotate = anglemod(fxi.cl->time / 700.0);

	// Brush models can auto animate their frames.

	autoanim = 2 * fxi.cl->time/1000;

	memset (sv_ents, 0, sizeof(sv_ents));

	numEntsToAdd = frame->num_entities;

	if (numEntsToAdd > MAX_ENTITIES)
	{
		fxi.Com_Printf("Overflow:  Too many (%d : %d) server entities to add to view\n", numEntsToAdd, MAX_ENTITIES);
		numEntsToAdd = MAX_ENTITIES;
	}

	for(pnum = 0, ent = sv_ents; pnum<numEntsToAdd; ++pnum)
	{
		entity_xstate_t *s1;

		s1 = &fxi.parse_entities[(frame->parse_entities +
				pnum) & (MAX_PARSE_ENTITIES - 1)];

		cent = fxi.server_entities + s1->number;

		if ((fxi.cl_predict->value) && (s1->number == fxi.cl->playernum+1))
		{
			// We are dealing with the client's model under prediction.
			isPredictedPlayer=true;
		}
		else
		{
			// We are dealing with a non predicted model (i.e. everything except the client's model).

			isPredictedPlayer=false;
		}

		// Setup effects, renderfx, skinnum and clientnum stuff.

		if (isPredictedPlayer)
		{
			cent->current.effects = effects = predictInfo.effects;
			cent->current.renderfx = renderfx = predictInfo.renderfx;
			ent->skinnum = predictInfo.skinnum;
			clientnum = predictInfo.clientnum;
		}
		else
		{
			effects = s1->effects;
			renderfx = s1->renderfx;
			ent->skinnum = s1->skinnum;
			clientnum = s1->clientnum;
		}

		/* Set frame. */
		if (effects & EF_ANIM_ALL)
		{
			ent->frame = autoanim;
		}
		else if (effects & EF_ANIM_ALLFAST)
		{
			ent->frame = fxi.cl->time / 100;
		}
		else
		{
			ent->frame = s1->frame;
		}

		// Handle flex-model nodes.
		ent->rr_mesh = s1->rr_mesh;

		// What's going on here?
// jmarshall - removed legacy prediction.
		//if(isPredictedPlayer)
		//{
		//	// Deal with predicted origin.
		//
		//	ent->backlerp = 1.0 - predictinfo.playerLerp;
		//	VectorCopy(cent->origin, ent->origin);
		//}
		//else
		{
			ent->oldframe = cent->prev.frame;
			ent->backlerp = 1.0 - fxi.cl->lerpfrac;

			// Interpolate origin.

			VectorSubtract(cent->current.origin, cent->prev.origin, dist);

			if (DotProduct(dist, dist) <= cl_lerpdist2->value)
				VectorMA(cent->prev.origin, 1.0f - ent->backlerp, dist, ent->origin);
			else
				VectorCopy(cent->current.origin, ent->origin);

			VectorCopy(ent->origin, cent->origin);
		}
// jmarshall end

		VectorCopy(ent->origin, ent->oldorigin);
		VectorCopy(cent->origin, cent->lerp_origin);

		// Set model.

		if (s1->modelindex == CUSTOM_PLAYER_MODEL)
		{
			// Use custom model and skin for player.

			ci = &fxi.cl->clientinfo[clientnum];

			ent->model = ci->model;
			/*
			 * TODO: Rewrite implementation skin change on damage: ent->skinnum
			 * and separate skins for different body parts
			 */
			// To allow for mutliple skins on various parts, I'm going to send across a pointer to the whole skin array.
			ent->skin = ci->skin;

			if (!ent->skin || !ent->model)
			{
				ent->model = fxi.cl->baseclientinfo.model;
				/* TODO: Rewrite implement skin change on damage: ent->skinnum */
				ent->skin = fxi.cl->baseclientinfo.skin;
			}
		}
		else
		{
			ent->model = fxi.cl->model_draw[s1->modelindex];
			ent->skin = NULL;			// No custom skin.
		}

		VectorCopy(s1->scale, ent->scale);

		if (s1->color[0] ||
		   s1->color[1] ||
		   s1->color[2] ||
		   s1->color[3])
		{
			ent->color.r = s1->color[0];
			ent->color.g = s1->color[1];
			ent->color.b = s1->color[2];
			ent->color.a = s1->color[3];
		}
		else
		{
			ent->color.c = 0xFFFFFFFF;
		}

		// Set render effects (fullbright, translucent, etc).

		ent->flags = renderfx;

		// Calculate angles.

		if (effects & EF_MACE_ROTATE)
		{
			// Some bonus items auto-rotate.

			ent->angles[0] = macerotate * 2;
			ent->angles[1] = macerotate;
			ent->angles[2] = 0;
		}
		else if (effects & EF_ROTATE)
		{
			// Some bonus items auto-rotate.

			ent->angles[0] = 0;
			ent->angles[1] = autorotate;
			ent->angles[2] = 0;
		}
		else
		{
			// Interpolate angles.

			float	a1, a2;
// jmarshall
			//if(isPredictedPlayer)
			//{
			//	// The corect angle values have already been generated by prediction and written
			//	// into the client's predictinfo_t structure.
			//
			//	for (i=0 ; i<3 ; i++)
			//	{
			//		cent->current.angles[i] = predictinfo.currAngles[i];
			//		cent->prev.angles[i] = predictinfo.prevAngles[i];
			//
			//		a1 = cent->current.angles[i];
			//		a2 = cent->prev.angles[i];
			//		ent->angles[i] = LerpAngle (a2, a1, predictinfo.playerLerp);
			//	}
			//}
			//else
// jmarshall end
			{
				// Get the angle values from the usual source, for all entities, including the player.

				for (i=0 ; i<3 ; i++)
				{
					a1 = cent->current.angles[i];
					a2 = cent->prev.angles[i];
					ent->angles[i] = LerpAngle (a2, a1, fxi.cl->lerpfrac);
				}
			}

			VectorCopy(ent->angles, cent->lerp_angles);
		}

		if (isPredictedPlayer)
		{
			// The corect frame and swapframe values have already been generated by prediction
			// and written into the client's predictinfo_t structure.
// jmarshall - Legacy Client Prediction
			//cent->prev.frame = predictinfo.prevFrame;
			//cent->current.frame = predictinfo.currFrame;
			//cent->prev.swapFrame = predictinfo.prevSwapFrame;
			//cent->current.swapFrame = predictinfo.currSwapFrame;
			//
			//ent->oldframe = cent->prev.frame;
			//ent->frame = cent->current.frame;

			ent->oldframe = cent->prev.frame;
			cent->prev.frame = ent->frame;
			cent->current.frame = ent->frame;
// jmarshall end
			if ((effects & EF_SWAPFRAME)&&(cent->current.swapFrame!=cent->current.frame))
			{
				ent->swapFrame=cent->current.swapFrame;
				ent->oldSwapFrame=cent->prev.swapFrame;

				// Yuck... but need to stop crashes for the demo.

				if (ent->oldSwapFrame==NO_SWAP_FRAME)
					ent->oldSwapFrame=ent->oldframe;
			}
			else
			{
				ent->swapFrame=cent->prev.swapFrame=NO_SWAP_FRAME;
			}
		}
		else
		{
			// Always get the frame and swapframe values from the usual source, for all entities,
			// including the player.

			if ((effects & EF_SWAPFRAME) && (s1->swapFrame != s1->frame))
			{
				ent->swapFrame = s1->swapFrame;
				ent->oldSwapFrame = cent->prev.swapFrame;

				// Yuck... but need to stop crashes for the demo.

				if (ent->oldSwapFrame==NO_SWAP_FRAME)
					ent->oldSwapFrame=ent->oldframe;
			}
			else
			{
				ent->swapFrame = cent->prev.swapFrame = NO_SWAP_FRAME;
			}
		}

		// Add player's packet_entity_t to refresh list of entity_t's and save the entity_t pointer
		// in PlayerEntPtr.

		if (s1->number == fxi.cl->playernum + 1)
		{
			// This is the player.

			if (PlayerAlpha < 1.0)
			{
				ent->color.a = (byte)(PlayerAlpha * (float)(ent->color.a));
				ent->flags |= RF_TRANSLUCENT;
			}
			else
			{
				// Color has already been copied from s1
//				ent->color.a = 255;
				ent->flags &= ~RF_TRANSLUCENT;
			}

			if (s1->modelindex)
			{
				AddEntityToView(ent);
			}

			// So client effects can play with owners entity.

			cent->entity = ent;

			if (cent->effects && !(effects & (EF_DISABLE_ALL_CFX | EF_ALWAYS_ADD_EFFECTS)))
			{
				num_owned_inview += AddEffectsToView(&cent->effects, cent);
			}

			++ent;
			continue;
		}

		// Cull (any elegible) entire models before they get rendered
		// Don't ask me--I just commented what _this does - MW).

		if (s1->modelindex)
		{
			vec3_t dir;

			VectorSubtract(ent->origin, fxi.cl->refdef.vieworg, dir);

			ent->depth = VectorNormalize(dir);

			AddEntityToView(ent);
			cent->entity = ent;					// So client effects can play with owners entity

			++ent;
		}

		if (cent->effects && !(effects & (EF_DISABLE_ALL_CFX | EF_ALWAYS_ADD_EFFECTS)))
		{
			num_owned_inview += AddEffectsToView(&cent->effects, cent);
		}
	}
}

// end

/*
==============
GetRefAPI

==============
*/
Q2_DLL_EXPORTED client_fx_export_t *
GetFXAPI(client_fx_import_t import)
{
	fxi = import;
	effectsExport.api_version = GAME_API_VERSION;
	effectsExport.Init = Init;
	effectsExport.ShutDown = ShutDown;
	effectsExport.Clear = Clear;
	effectsExport.RegisterModels = RegisterModels;

	// In the client code in the executable the following functions are called first.
	effectsExport.AddPacketEntities = AddServerEntities;

	// Secondly....
	effectsExport.AddEffects = AddEffects;

	// Thirdly (if any independent effects exist).
	effectsExport.ParseClientEffects = ParseEffects;

	// Lastly.
	effectsExport.UpdateEffects = PostRenderUpdate;

	effectsExport.RemoveClientEffects = RemoveEffectsFromCent;

	return &effectsExport;
}

/*
 * this is only here so the functions
 * in shared source files can link
 */
void
Sys_Error(const char *error, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, error);
	vsnprintf(text, sizeof(text), error, argptr);
	va_end(argptr);

	fxi.Com_Error(ERR_DROP, "Game Error: %s", text);
}

void
Com_Printf(const char *msg, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, msg);
	vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

	fxi.Com_Printf("%s", text);
}
