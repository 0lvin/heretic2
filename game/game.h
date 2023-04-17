// game.h -- game dll information visible to server.

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>

#include "q_ClientServer.h"
#include "g_NewSystem.h"
#include "g_BoundingForm.h"

#define	GAME_API_VERSION	3

// ************************************************************************************************
// 'SVF_XXX'.
// ----------
// Held in 'edict_t'->svflags.
// ************************************************************************************************

#define	SVF_NOCLIENT			0x00000001	// Don't send entity to clients, even if it has effects.
											// This overides SVF_ALWAYS_SEND
#define	SVF_DEADMONSTER			0x00000002	// Treat as CONTENTS_DEADMONSTER for collision.
#define	SVF_MONSTER				0x00000004	// Treat as CONTENTS_MONSTER for collision.
#define	SVF_INUSE				0x00000008	// Used to replace the inuse field.
#define SVF_ALWAYS_SEND			0x00000010	// Always send the ent to all the clients, regardless of
											// of PVS or view culling
#define SVF_NO_AUTOTARGET		0x00000020	// This entity will not be chosen by FindNearestVisibleActorInFrustum
#define SVF_REFLECT				0x00000040	// Reflect shots
#define SVF_TAKE_NO_IMPACT_DMG	0x00000080	// Don't apply impact damage to this entity
#define SVF_BOSS				0x00000100	// Immunity to a number of things
#define SVF_TOUCHED_BEAST		0x00000200	// Used for beast faked physics hack
#define SVF_DO_NO_IMPACT_DMG	0x00000400	// This entity Doesn't do impact damage to others
#define SVF_NO_PLAYER_DAMAGE	0x00000800	// This entity Doesn't take damage from players
#define SVF_PARTS_GIBBED		0x00001000	// Used to delay gibbing so monsters can throw body parts
#define SVF_WAIT_NOTSOLID		0x00002000	// Hacky flag to postpone dead monsters from turning notsolid
#define SVF_ONFIRE				0x00004000	// He likes it Hot! Hot! Hot!
#define SVF_SHOW_START_BUOY		0x00008000	// just puts an effect on a buoy for showbuoy debug mode
#define SVF_SHOW_END_BUOY		0x00010000	// just puts an effect on a buoy for showbuoy debug mode
#define SVF_FLOAT				0x00020000	// Allows walkmonsters to walk off ledges, assumes a low gravity
#define SVF_ALLOW_AUTO_TARGET	0x00040000	// Used to allow player to autotarget non-monsters
#define SVF_ALERT_NO_SHADE		0x00080000	// only used by alert_entity to make monsters check the alert as a sound alert


// ************************************************************************************************
// 'solid_t'.
// ----------
// edict->solid values
// ************************************************************************************************

typedef enum
{
	SOLID_NOT,		// No interaction with other objects.
	SOLID_TRIGGER,	// Only touch when inside, after moving.
	SOLID_BBOX,		// Touch on edge.
	SOLID_BSP		// BSP clip, touch on edge.
} solid_t;

// ************************************************************************************************
// 'link_t'.
// ---------
// Only used for entity area links.
// ************************************************************************************************

typedef struct link_s
{
	struct link_s	*prev,*next;
} link_t;

#define	MAX_ENT_CLUSTERS	16

// Forward define these two as they are needed below.

typedef struct gclient_s gclient_t;

// Do not define the following short, server-visible 'gclient_t' and 'edict_t' structures because
// when we are being #inclued in g_local.h.

#ifndef GAME_INCLUDE

// ************************************************************************************************
// 'gclient_t'.
// ------------
// This structure is cleared on each PutClientInServer().
// ************************************************************************************************

typedef struct gclient_s
{
	// Following two fields are known to the server.

	player_state_t		ps;		// Communicated by server to clients.
	int					ping;

	// DO NOT MODIFY ANYTHING ABOVE THIS! THE SERVER EXPECTS THE FIELDS IN THAT ORDER! The game dll
	// can add anything it wants after this point.

	//=============================================================================================
} gclient_t;

// ************************************************************************************************
// 'edict_s'.
// ------------
// ************************************************************************************************

struct edict_s
{
	// This is sent to the server as part of each client frame.

	entity_state_t		s;

	// NULL if not a player. The server expects the first part of a 'gclient_s' to be a
	// 'player_state_t' but the rest of it is opaque.

	struct gclient_s	*client;

	// House keeping information not used by the game logic.

	qboolean			inuse;
	int					just_deleted;			// used to delete stuff entities properly on the client
	int					client_sent;			// used to delete stuff entities properly on the client
	int					linkcount;

	// FIXME: move these fields to a server private sv_entity_t

	link_t				area;				// Linked to a division node or leaf.
	int					num_clusters;		// If -1, use headnode instead.
	int					clusternums[MAX_ENT_CLUSTERS];
	int					headnode;			// Unused if num_clusters is -1.
	int					areanum,areanum2;

	int					svflags;

	edict_t				*groundentity;			// entity serving as ground
	int					groundentity_linkcount;	// if self and groundentity's don't match, groundentity should be cleared
	vec3_t				groundNormal;			// normal of the ground

	vec3_t				intentMins, intentMaxs;	// if PF_RESIZE is set, then physics will attempt to change
												// the ents bounding form to the new one indicated
												// If it was succesfully resized, the PF_RESIZE is turned off
												// otherwise it will remain on.

	solid_t				solid;
	int					clipmask;
	edict_t				*owner;

	vec3_t				mins,maxs;
	vec3_t				absmin,absmax,size;

	// called when self is the collidee in a collision, resulting in the impediment or bouncing of trace->ent
	void				(*isBlocking)(edict_t *self, struct trace_s *trace);

	// The game dll can add anything it wants after this point in the edict_t in g_Edict.h.

	//=============================================================================================
};

#endif // GAME_INCLUDE

// ************************************************************************************************
// 'game_import_t'.
// ----------------
// The game dll imports these functions which are provided by the main engine code.
// ************************************************************************************************

typedef struct
{
	// Special message printing routines.

	void	(*bprintf) (int printlevel, char *fmt, ...);
	void	(*bcaption) (int printlevel, short stringid);
	void	(*Obituary) (int printlevel, short stringid, short client1, short client2);
	void	(*dprintf) (char *fmt, ...);
	void	(*cprintf) (edict_t *ent, int printlevel, char *fmt, ...);
	void	(*clprintf) (edict_t *ent, edict_t *from, int color, char *fmt, ...);
	void	(*centerprintf) (edict_t *ent, char *fmt, ...);
	void	(*gamemsg_centerprintf) (edict_t *ent, short msg);
	void	(*levelmsg_centerprintf) (edict_t *ent, short msg);
	void	(*msgvar_centerprintf) (edict_t *ent, short msg, int vari);
	void	(*msgdual_centerprintf) (edict_t *ent, short msg1, short msg2);
	void	(*captionprintf) (edict_t *ent, short msg);

	// Sound playing routines.

	void	(*sound) (edict_t *ent, int channel, int soundindex, float volume, float attenuation, float timeofs);
	void	(*soundevent) (byte EventId,float leveltime,edict_t *ent, int channel, int soundindex, float volume, float attenuation, float timeofs);
	void	(*positioned_sound) (vec3_t origin, edict_t *ent, int channel, int soundinedex, float volume, float attenuation, float timeofs);

	// Config strings hold all the index strings, the lightstyles and misc data, like the sky
	// definition and cdtrack. All of the current configstrings are sent to clients when they
	// connect and changes are sent to all connected clients.

	void	(*configstring) (int num, char *string);

	//	Error, bailout routine.

	void	(*error) (char *fmt, ...);

	// routine that sends over new CD track
	void	(*changeCDtrack) (edict_t *ent, int track, int loop );

	// New names can only be added during spawning but existing names can be looked up at any time.

	void	(*cleanlevel) (void);
	int		(*modelindex) (char *name);
	void	(*modelremove) (char *name);
	int		(*soundindex) (char *name);
	void	(*soundremove) (char *name);
	int		(*imageindex) (char *name);

	void	(*setmodel) (edict_t *ent, char *name);

	// Collision detection.

	void (*trace) (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, edict_t *passent, int contentmask,trace_t *tr);
	int		(*pointcontents) (vec3_t point);
	
	// Potentially visible / invisible set routines.

	qboolean	(*inPVS) (vec3_t p1, vec3_t p2);
	qboolean	(*inPHS) (vec3_t p1, vec3_t p2);
	void		(*SetAreaPortalState) (int portalnum, qboolean open);
	qboolean	(*AreasConnected) (int area1, int area2);

	// An entity will never be sent to a client or used for collision if it is not passed to
	// linkentity.  If the size, position, or solidity changes, it must be relinked.

	void	(*linkentity) (edict_t *ent);
	void	(*unlinkentity) (edict_t *ent);	// Call before removing an interactive edict.
	int		(*BoxEdicts) (vec3_t mins, vec3_t maxs, edict_t **list,	int maxcount, int areatype);
	void	(*Pmove) (pmove_t *pmove, qboolean server);		// Player movement code, common with client prediction.

	// New_Physics

	int		(*FindEntitiesInBounds) (vec3_t mins, vec3_t maxs, struct SinglyLinkedList_s *list, int areatype);
	void	(*TraceBoundingForm) (struct FormMove_s *formMove);
	qboolean	(*ResizeBoundingForm) (edict_t *self, struct FormMove_s *formMove);
	int		(*GetContentsAtPoint) (vec3_t point);
	qboolean (*CheckDistances)(vec3_t origin, float dist);

	// End New_Physics

	// Network messaging services.

	void	(*multicast) (vec3_t origin, multicast_t to);
	void	(*unicast) (edict_t *ent, qboolean reliable);
	void	(*WriteChar) (int c);
	void	(*WriteByte) (int c);
	void	(*WriteShort) (int c);
	void	(*WriteLong) (int c);
	void	(*WriteFloat) (float f);
		void	(*WriteString) (char *s);
	void	(*WritePosition) (vec3_t pos);	// Some fractional bits.
	void	(*WriteDir) (vec3_t pos);		// Single byte encoded, very coarse.
	void	(*WriteAngle) (float f);
	void	(*CreateEffect) (entity_state_t *ent, int type, int flags, vec3_t origin, char *format, ...);
	void	(*RemoveEffects)(entity_state_t *ent, int type);
	void	(*CreateEffectEvent) (byte EventId,entity_state_t *ent, int type, int flags, vec3_t origin, char *format, ...);
	void	(*RemoveEffectsEvent)(byte EventId,entity_state_t *ent, int type);
	int		(*CreatePersistantEffect) (entity_state_t *ent, int type, int flags, vec3_t origin, char *format, ...);
	qboolean (*RemovePersistantEffect) (int toRemove, int call_from);	// removes the effect from the server's persistant effect list.
														// The effect is not removed on the client
														// This should be done by removing the effects from the owning entity or freeing


	// Managed memory allocation.

	void	*(*TagMalloc) (int size, int tag);
	void	(*TagFree) (void *block);
	void	(*FreeTags) (int tag);

	// Console variable interaction.

	cvar_t	*(*cvar) (char *var_name, char *value, int flags);
	cvar_t	*(*cvar_set) (char *var_name, char *value);
	cvar_t	*(*cvar_forceset) (char *var_name, char *value);
	float	(*cvar_variablevalue) (char *var_name);

	// ClientCommand and console command parameter checking.

	int		(*argc) (void);
	char	*(*argv) (int n);
	char	*(*args) (void);

	// Add commands to the server console as if they were typed in for map changing, etc.

	void	(*AddCommandString) (char *text);

	// Debugging aid.

	void	(*DebugGraph) (float value, byte r, byte g, byte b);

	// Files will be memory mapped read only. The returned buffer may be part of a larger '.pak'
	// file, or a discrete file from anywhere in the quake search path. A -1 return means the file
	// does not exist. NULL can be passed for buf to just determine existance.

	int		(*FS_LoadFile) (char *name, void **buf);
	void	(*FS_FreeFile) (void *buf);
	char	*(*FS_Userdir)  (void);
	void	(*FS_CreatePath) (char *path);


	void	(*Sys_LoadGameDll)(const char *name, HINSTANCE *hinst, DWORD *chkSum);
	void	(*Sys_UnloadGameDll)(const char *name, HINSTANCE *hinst);

	// pointer to the server side persistant effects arrary
	void	(*ClearPersistantEffects) (void);
	void	*Persistant_Effects_Array;

} game_import_t;

// ************************************************************************************************
// 'game_export_t'.
// ----------------
// The game dll exports these functions.
// ************************************************************************************************

typedef struct
{
	int			apiversion;

	int			numSkeletalJoints;
	struct G_SkeletalJoint_s *skeletalJoints;
	struct ArrayedListNode_s *jointNodes;

	// The init() function will only be called when a game starts, not each time a level is loaded.
	// Persistant data for clients and the server can be allocated in init().

	void		(*Init) (void);
	void		(*Shutdown) (void);

	// Each new level entered will cause a call to SpawnEntities().

	void		(*SpawnEntities) (char *mapname, char *entstring, char *spawnpoint, qboolean loadgame);
	void		(*ConstructEntities)(void);
	void		(*CheckCoopTimeout)(qboolean BeenHereBefore);

	// Read/Write Game is for storing persistant cross level information about the world state and
	// the clients. WriteGame is called every time a level is exited. ReadGame is called on a
	// loadgame.

	void		(*WriteGame) (char *filename, qboolean autosave);
	void		(*ReadGame) (char *filename);

	// ReadLevel is called after the default map information has been loaded with SpawnEntities, so
	// any stored client spawn spots will be used when the clients reconnect.

	void		(*WriteLevel) (char *filename);
	void		(*ReadLevel) (char *filename);

	// 

	qboolean	(*ClientConnect) (edict_t *ent, char *userinfo);
	void		(*ClientBegin) (edict_t *ent);
	void		(*ClientUserinfoChanged) (edict_t *ent, char *userinfo);
	void		(*ClientDisconnect) (edict_t *ent);
	void		(*ClientCommand) (edict_t *ent);
	void		(*ClientThink) (edict_t *ent, usercmd_t *cmd);

	//

	void		(*RunFrame) (void);

	// ServerCommand will be called when an "sv <command>" command is issued on the server console.
	// The game can issue gi.argc() / gi.argv() commands to get the rest of the parameters.
	
	void		(*ServerCommand) (void);

	// Global variables shared between game and server. The edict array is allocated in the game dll
	// so it can vary in size from one game to another. The size will be fixed when ge->Init() is
	// called.

	struct edict_s	*edicts;
	int			edict_size;
	int			num_edicts;	// Current number of edicts. Always <= max_edicts.
	int			max_edicts;
} game_export_t;

game_export_t *GetGameApi (game_import_t *import);
