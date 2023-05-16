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
 * Platform independent initialization, main loop and frame handling.
 *
 * =======================================================================
 */

#include "header/common.h"
#include "header/zone.h"
#include <setjmp.h>

cvar_t *developer;
cvar_t *modder;
cvar_t *timescale;

#include "../client/header/screen.h"
#include "../client/header/keyboard.h"
#include "../client/header/console.h"
#include "../server/header/server.h"

#define	MAXPRINTMSG	4096

#define MAX_NUM_ARGVS	50


int		com_argc;
char	*com_argv[MAX_NUM_ARGVS+1];

int		realtime;

extern jmp_buf abortframe;		// an ERR_DROP occured, exit the entire frame
extern zhead_t z_chain;

FILE	*log_stats_file;

cvar_t	*host_speeds;
cvar_t	*log_stats;
cvar_t	*fixedtime;
cvar_t	*showtrace;
cvar_t	*dedicated;

/* host_speeds times */
int time_before_game;
int time_after_game;
int time_before_ref;
int time_after_ref;

// Is the game portable?
qboolean is_portable;

// Game given by user
char userGivenGame[MAX_QPATH];

// Game should quit next frame.
// Hack for the signal handlers.
qboolean quitnextframe;

/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

vec3_t	bytedirs[NUMVERTEXNORMALS] =
{
#include "../client/refresh/constants/anorms.h"
};

//
// writing functions
//

void MSG_WriteData(sizebuf_t* sb, byte* data, int len)
{
	byte* buf;
	buf = (byte *)SZ_GetSpace(sb, len);
	memcpy(buf, data, len);
}

void MSG_WriteChar (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < -128 || c > 127)
		Com_Error (ERR_FATAL, "MSG_WriteChar: range error");
#endif

	buf = (byte*)SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteByte (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < 0 || c > 255)
		Com_Error (ERR_FATAL, "MSG_WriteByte: range error");
#endif

	buf = (byte*)SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteShort (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Com_Error (ERR_FATAL, "MSG_WriteShort: range error");
#endif

	buf = (byte*)SZ_GetSpace (sb, 2);
	buf[0] = c&0xff;
	buf[1] = c>>8;
}

void MSG_WriteLong (sizebuf_t *sb, int c)
{
	byte	*buf;

	buf = (byte*)SZ_GetSpace (sb, 4);
	buf[0] = c&0xff;
	buf[1] = (c>>8)&0xff;
	buf[2] = (c>>16)&0xff;
	buf[3] = c>>24;
}

void MSG_WriteFloat (sizebuf_t *sb, float f)
{
	union
	{
		float	f;
		int	l;
	} dat;


	dat.f = f;
	dat.l = LittleLong (dat.l);

	SZ_Write (sb, &dat.l, 4);
}

void MSG_WriteString (sizebuf_t *sb, char *s)
{
	if (!s)
		SZ_Write (sb, "", 1);
	else
		SZ_Write (sb, s, strlen(s)+1);
}

void MSG_WriteCoord (sizebuf_t *sb, float f)
{
	MSG_WriteShort (sb, (int)(f*8));
}

void MSG_WritePos (sizebuf_t *sb, vec3_t pos)
{
	MSG_WriteShort (sb, (int)(pos[0]*8));
	MSG_WriteShort (sb, (int)(pos[1]*8));
	MSG_WriteShort (sb, (int)(pos[2]*8));
}

void MSG_WriteAngle (sizebuf_t *sb, float f)
{
	MSG_WriteByte (sb, (int)(f*256/360) & 255);
}

void MSG_WriteAngle16 (sizebuf_t *sb, float f)
{
	MSG_WriteShort (sb, ANGLE2SHORT(f));
}


void MSG_WriteDeltaUsercmd (sizebuf_t *buf, usercmd_t *from, usercmd_t *cmd)
{
	int		bits;

//
// send the movement message
//
	bits = 0;
	if (cmd->angles[0] != from->angles[0])
		bits |= CM_ANGLE1;
	if (cmd->angles[1] != from->angles[1])
		bits |= CM_ANGLE2;
	if (cmd->angles[2] != from->angles[2])
		bits |= CM_ANGLE3;
	if (cmd->forwardmove != from->forwardmove)
		bits |= CM_FORWARD;
	if (cmd->sidemove != from->sidemove)
		bits |= CM_SIDE;
	if (cmd->upmove != from->upmove)
		bits |= CM_UP;
	if (cmd->buttons != from->buttons)
		bits |= CM_BUTTONS;
	//if (cmd->impulse != from->impulse)
	//	bits |= CM_IMPULSE;

    MSG_WriteLong (buf, bits);

	if (bits & CM_ANGLE1)
		MSG_WriteShort (buf, cmd->angles[0]);
	if (bits & CM_ANGLE2)
		MSG_WriteShort (buf, cmd->angles[1]);
	if (bits & CM_ANGLE3)
		MSG_WriteShort (buf, cmd->angles[2]);

	if (bits & CM_FORWARD)
		MSG_WriteShort (buf, cmd->forwardmove);
	if (bits & CM_SIDE)
	  	MSG_WriteShort (buf, cmd->sidemove);
	if (bits & CM_UP)
		MSG_WriteShort (buf, cmd->upmove);

 	if (bits & CM_BUTTONS)
	  	MSG_WriteByte (buf, cmd->buttons);
 	//if (bits & CM_IMPULSE)
	//    MSG_WriteByte (buf, cmd->impulse);

    MSG_WriteByte (buf, cmd->msec);
	MSG_WriteByte (buf, cmd->lightlevel);
}

void Qcommon_ExecConfigs(qboolean gameStartUp)
{
	Cbuf_AddText("exec default.cfg\n");
	Cbuf_AddText("exec yq2.cfg\n");
	Cbuf_AddText("exec config.cfg\n");
	Cbuf_AddText("exec autoexec.cfg\n");

	if (gameStartUp)
	{
		/* Process cmd arguments only startup. */
		Cbuf_AddEarlyCommands(true);
	}

	Cbuf_Execute();
}

void MSG_WriteDir (sizebuf_t *sb, vec3_t dir)
{
	int		i, best;
	float	d, bestd;

	if (!dir)
	{
		MSG_WriteByte (sb, 0);
		return;
	}

	bestd = 0;
	best = 0;
	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		d = DotProduct (dir, bytedirs[i]);
		if (d > bestd)
		{
			bestd = d;
			best = i;
		}
	}
	MSG_WriteByte (sb, best);
}


void MSG_ReadDir (sizebuf_t *sb, vec3_t dir)
{
	int		b;

	b = MSG_ReadByte (sb);
	if (b >= NUMVERTEXNORMALS)
		Com_Error (ERR_DROP, "MSF_ReadDir: out of range");
	VectorCopy (bytedirs[b], dir);
}


/*
==================
MSG_WriteDeltaEntity

Writes part of a packetentities message.
Can delta from either a baseline or a previous packet_entity
==================
*/
void MSG_WriteDeltaEntity (entity_state_t *from, entity_state_t *to, sizebuf_t *msg, qboolean force, qboolean newentity)
{
	int		bits;

	if (!to->number)
		Com_Error (ERR_FATAL, "Unset entity number");
	//if (to->number >= MAX_EDICTS)
	//	Com_Error (ERR_FATAL, "Entity number >= MAX_EDICTS");

// send an update
	bits = 0;

	if (to->number >= 256)
		bits |= U_NUMBER16;		// number8 is implicit otherwise

	if (to->origin[0] != from->origin[0])
		bits |= U_ORIGIN12;
	if (to->origin[1] != from->origin[1])
		bits |= U_ORIGIN12;
	if (to->origin[2] != from->origin[2])
		bits |= U_ORIGIN3;

	if ( to->angles[0] != from->angles[0] )
		bits |= U_ANGLE1;
	if ( to->angles[1] != from->angles[1] )
		bits |= U_ANGLE2;
	if ( to->angles[2] != from->angles[2] )
		bits |= U_ANGLE3;

	if (to->clientEffects.numEffects != from->clientEffects.numEffects || to->clientEffects.isPersistant)
		bits |= U_CLIENT_EFFECTS;

	if ( to->skinnum != from->skinnum )
	{
		if ((unsigned)to->skinnum < 256)
			bits |= U_SKIN8;
		else if ((unsigned)to->skinnum < 0x10000)
			bits |= U_SKIN16;
		else
			bits |= (U_SKIN8|U_SKIN16);
	}

	if ( to->frame != from->frame )
	{
		if (to->frame < 256)
			bits |= U_FRAME8;
		else
			bits |= U_FRAME16;
	}

	if ( to->effects != from->effects )
	{
		if (to->effects < 256)
			bits |= U_EFFECTS8;
		else if (to->effects < 0x8000)
			bits |= U_EFFECTS16;
		else
			bits |= U_EFFECTS8|U_EFFECTS16;
	}

	if ( to->renderfx != from->renderfx )
	{
		if (to->renderfx < 256)
			bits |= U_RENDERFX8;
		else if (to->renderfx < 0x8000)
			bits |= U_RENDERFX16;
		else
			bits |= U_RENDERFX8|U_RENDERFX16;
	}

	if ( to->solid != from->solid )
		bits |= U_SOLID;

	// event is not delta compressed, just 0 compressed
	//if ( to->event  )
	//	bits |= U_EVENT;

	if ( to->modelindex != from->modelindex )
		bits |= U_MODEL;
	//if ( to->modelindex2 != from->modelindex2 )
	//	bits |= U_MODEL2;
	//if ( to->modelindex3 != from->modelindex3 )
	//	bits |= U_MODEL3;
	//if ( to->modelindex4 != from->modelindex4 )
	//	bits |= U_MODEL4;

	if ( to->sound != from->sound )
		bits |= U_SOUND;

	for (int i = 0; i < MAX_FM_MESH_NODES; i++)
	{
		if (to->fmnodeinfo[i].frame != from->fmnodeinfo[i].frame)
			bits |= U_FM_FRAME;

		if (to->fmnodeinfo[i].flags != from->fmnodeinfo[i].flags)
			bits |= U_FM_FLAGS;
	}

	//if (newentity || (to->renderfx & RF_BEAM))
	//	bits |= U_OLDORIGIN;

	//
	// write the message
	//
	if (!bits && !force)
		return;		// nothing to send!

	//----------

	//if (bits & 0xff000000)
	//	bits |= U_MOREBITS3 | U_MOREBITS2 | U_MOREBITS1;
	//else if (bits & 0x00ff0000)
	//	bits |= U_MOREBITS2 | U_MOREBITS1;
	//else if (bits & 0x0000ff00)
	//	bits |= U_MOREBITS1;

	MSG_WriteLong (msg,	bits );

	//if (bits & 0xff000000)
	//{
	//	MSG_WriteByte (msg,	(bits>>8)&255 );
	//	MSG_WriteByte (msg,	(bits>>16)&255 );
	//	MSG_WriteByte (msg,	(bits>>24)&255 );
	//}
	//else if (bits & 0x00ff0000)
	//{
	//	MSG_WriteByte (msg,	(bits>>8)&255 );
	//	MSG_WriteByte (msg,	(bits>>16)&255 );
	//}
	//else if (bits & 0x0000ff00)
	//{
	//	MSG_WriteByte (msg,	(bits>>8)&255 );
	//}

	//----------

	MSG_WriteShort(msg, to->number);

	if (bits & U_CLIENT_EFFECTS)
	{
		if (to->clientEffects.buf == NULL || to->clientEffects.numEffects <= 0)
		{
			MSG_WriteShort(msg, -1);
		}
		else
		{
			MSG_WriteShort(msg, to->clientEffects.numEffects);
			MSG_WriteShort(msg, to->clientEffects.bufSize);
			MSG_WriteData(msg, to->clientEffects.buf, to->clientEffects.bufSize);
		}
	}

	if (bits & U_FM_FRAME)
	{
		for (int i = 0; i < MAX_FM_MESH_NODES; i++)
		{
			MSG_WriteShort(msg, to->fmnodeinfo[i].frame);
		}
	}

	if (bits & U_FM_FLAGS)
	{
		for (int i = 0; i < MAX_FM_MESH_NODES; i++)
		{
			MSG_WriteShort(msg, to->fmnodeinfo[i].flags);
		}
	}

	if (bits & U_MODEL)
		MSG_WriteByte (msg,	to->modelindex);
	//if (bits & U_MODEL2)
	//	MSG_WriteByte (msg,	to->modelindex2);
	//if (bits & U_MODEL3)
	//	MSG_WriteByte (msg,	to->modelindex3);
	//if (bits & U_MODEL4)
	//	MSG_WriteByte (msg,	to->modelindex4);

	if (bits & U_FRAME8)
		MSG_WriteByte (msg, to->frame);
	if (bits & U_FRAME16)
		MSG_WriteShort (msg, to->frame);

	if ((bits & U_SKIN8) && (bits & U_SKIN16))		//used for laser colors
		MSG_WriteLong (msg, to->skinnum);
	else if (bits & U_SKIN8)
		MSG_WriteByte (msg, to->skinnum);
	else if (bits & U_SKIN16)
		MSG_WriteShort (msg, to->skinnum);


	if ( (bits & (U_EFFECTS8|U_EFFECTS16)) == (U_EFFECTS8|U_EFFECTS16) )
		MSG_WriteLong (msg, to->effects);
	else if (bits & U_EFFECTS8)
		MSG_WriteByte (msg, to->effects);
	else if (bits & U_EFFECTS16)
		MSG_WriteShort (msg, to->effects);

	if ( (bits & (U_RENDERFX8|U_RENDERFX16)) == (U_RENDERFX8|U_RENDERFX16) )
		MSG_WriteLong (msg, to->renderfx);
	else if (bits & U_RENDERFX8)
		MSG_WriteByte (msg, to->renderfx);
	else if (bits & U_RENDERFX16)
		MSG_WriteShort (msg, to->renderfx);

	if (bits & U_ORIGIN12) {
		MSG_WriteCoord(msg, 333.0f); // jmarshall: hack! padding, for some reason without this origin[0] can be nan
		MSG_WriteCoord(msg, to->origin[0]);
	}
	if (bits & U_ORIGIN12)
		MSG_WriteCoord (msg, to->origin[1]);
	if (bits & U_ORIGIN3)
		MSG_WriteCoord (msg, to->origin[2]);

	if (bits & U_ANGLE1) {
		MSG_WriteCoord(msg, 333.0f); // jmarshall: hack! padding, for some reason without this origin[0] can be nan
		MSG_WriteAngle(msg, to->angles[0]);
	}
	if (bits & U_ANGLE2) {
		MSG_WriteCoord(msg, 333.0f); // jmarshall: hack! padding, for some reason without this origin[0] can be nan
		MSG_WriteAngle(msg, to->angles[1]);
	}
	if (bits & U_ANGLE3)
		MSG_WriteAngle(msg, to->angles[2]);

	if (bits & U_OLDORIGIN)
	{
		MSG_WriteCoord (msg, to->old_origin[0]);
		MSG_WriteCoord (msg, to->old_origin[1]);
		MSG_WriteCoord (msg, to->old_origin[2]);
	}

	if (bits & U_SOUND)
		MSG_WriteByte (msg, to->sound);
	//if (bits & U_EVENT)
	//	MSG_WriteByte (msg, to->event);
	if (bits & U_SOLID)
		MSG_WriteShort (msg, to->solid);
}


//============================================================

//
// reading functions
//

void MSG_BeginReading (sizebuf_t *msg)
{
	msg->readcount = 0;
}

// returns -1 if no more characters are available
int MSG_ReadChar (sizebuf_t *msg_read)
{
	int	c;

	if (msg_read->readcount+1 > msg_read->cursize)
		c = -1;
	else
		c = (signed char)msg_read->data[msg_read->readcount];
	msg_read->readcount++;

	return c;
}

int MSG_ReadByte (sizebuf_t *msg_read)
{
	int	c;

	if (msg_read->readcount+1 > msg_read->cursize)
		c = -1;
	else
		c = (unsigned char)msg_read->data[msg_read->readcount];
	msg_read->readcount++;

	return c;
}

int MSG_ReadShort (sizebuf_t *msg_read)
{
	int	c;

	if (msg_read->readcount+2 > msg_read->cursize)
		c = -1;
	else
		c = (short)(msg_read->data[msg_read->readcount]
		+ (msg_read->data[msg_read->readcount+1]<<8));

	msg_read->readcount += 2;

	return c;
}

int MSG_ReadLong (sizebuf_t *msg_read)
{
	int	c;

	if (msg_read->readcount+4 > msg_read->cursize)
		c = -1;
	else
		c = msg_read->data[msg_read->readcount]
		+ (msg_read->data[msg_read->readcount+1]<<8)
		+ (msg_read->data[msg_read->readcount+2]<<16)
		+ (msg_read->data[msg_read->readcount+3]<<24);

	msg_read->readcount += 4;

	return c;
}

float MSG_ReadFloat (sizebuf_t *msg_read)
{
	union
	{
		byte	b[4];
		float	f;
		int	l;
	} dat;

	if (msg_read->readcount+4 > msg_read->cursize)
		dat.f = -1;
	else
	{
		dat.b[0] =	msg_read->data[msg_read->readcount];
		dat.b[1] =	msg_read->data[msg_read->readcount+1];
		dat.b[2] =	msg_read->data[msg_read->readcount+2];
		dat.b[3] =	msg_read->data[msg_read->readcount+3];
	}
	msg_read->readcount += 4;

	dat.l = LittleLong (dat.l);

	return dat.f;
}

char *MSG_ReadString (sizebuf_t *msg_read)
{
	static char	string[2048];
	int		l,c;

	l = 0;
	do
	{
		c = MSG_ReadChar (msg_read);
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);

	string[l] = 0;

	return string;
}

char *MSG_ReadStringLine (sizebuf_t *msg_read)
{
	static char	string[2048];
	int		l,c;

	l = 0;
	do
	{
		c = MSG_ReadChar (msg_read);
		if (c == -1 || c == 0 || c == '\n')
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);

	string[l] = 0;

	return string;
}

float MSG_ReadCoord (sizebuf_t *msg_read)
{
	return MSG_ReadShort(msg_read) * (1.0/8);
}

void MSG_ReadPos (sizebuf_t *msg_read, vec3_t pos)
{
	pos[0] = MSG_ReadShort(msg_read) * (1.0/8);
	pos[1] = MSG_ReadShort(msg_read) * (1.0/8);
	pos[2] = MSG_ReadShort(msg_read) * (1.0/8);
}

float MSG_ReadAngle (sizebuf_t *msg_read)
{
	return MSG_ReadChar(msg_read) * (360.0/256);
}

float MSG_ReadAngle16 (sizebuf_t *msg_read)
{
	return SHORT2ANGLE(MSG_ReadShort(msg_read));
}

void MSG_ReadDeltaUsercmd (sizebuf_t *msg_read, usercmd_t *from, usercmd_t *move)
{
	int bits;

	memcpy (move, from, sizeof(*move));

	bits = MSG_ReadLong (msg_read);

// read current angles
	if (bits & CM_ANGLE1)
		move->angles[0] = MSG_ReadShort (msg_read);
	if (bits & CM_ANGLE2)
		move->angles[1] = MSG_ReadShort (msg_read);
	if (bits & CM_ANGLE3)
		move->angles[2] = MSG_ReadShort (msg_read);

// read movement
	if (bits & CM_FORWARD)
		move->forwardmove = MSG_ReadShort (msg_read);
	if (bits & CM_SIDE)
		move->sidemove = MSG_ReadShort (msg_read);
	if (bits & CM_UP)
		move->upmove = MSG_ReadShort (msg_read);

// read buttons
	if (bits & CM_BUTTONS)
		move->buttons = MSG_ReadByte (msg_read);

	//if (bits & CM_IMPULSE)
	//	move->impulse = MSG_ReadByte (msg_read);

// read time to run command
	move->msec = MSG_ReadByte (msg_read);

// read the light level
	move->lightlevel = MSG_ReadByte (msg_read);
}


void MSG_ReadData (sizebuf_t *msg_read, void *data, int len)
{
	int		i;

	for (i=0 ; i<len ; i++)
		((byte *)data)[i] = MSG_ReadByte (msg_read);
}

//============================================================================


/*
================
COM_CheckParm

Returns the position (1 to argc-1) in the program's argument list
where the given parameter apears, or 0 if not present
================
*/
int COM_CheckParm (char *parm)
{
	int		i;

	for (i=1 ; i<com_argc ; i++)
	{
		if (!strcmp (parm,com_argv[i]))
			return i;
	}

	return 0;
}

int COM_Argc (void)
{
	return com_argc;
}

char *COM_Argv (int arg)
{
	if (arg < 0 || arg >= com_argc || !com_argv[arg])
		return "";
	return com_argv[arg];
}

void COM_ClearArgv (int arg)
{
	if (arg < 0 || arg >= com_argc || !com_argv[arg])
		return;
	com_argv[arg] = "";
}


/*
================
COM_InitArgv
================
*/
void COM_InitArgv (int argc, char **argv)
{
	int		i;

	if (argc > MAX_NUM_ARGVS)
		Com_Error (ERR_FATAL, "argc > MAX_NUM_ARGVS");
	com_argc = argc;
	for (i=0 ; i<argc ; i++)
	{
		if (!argv[i] || strlen(argv[i]) >= MAX_TOKEN_CHARS )
			com_argv[i] = "";
		else
			com_argv[i] = argv[i];
	}
}

/*
================
COM_AddParm

Adds the given string at the end of the current argument list
================
*/
void COM_AddParm (char *parm)
{
	if (com_argc == MAX_NUM_ARGVS)
		Com_Error (ERR_FATAL, "COM_AddParm: MAX_NUM)ARGS");
	com_argv[com_argc++] = parm;
}


char *CopyString (char *in)
{
	char	*out;

	out = (char *)Z_Malloc (strlen(in)+1);
	strcpy (out, in);
	return out;
}


void Info_Print (char *s)
{
	char	key[512];
	char	value[512];
	char	*o;
	int		l;

	if (*s == '\\')
		s++;
	while (*s)
	{
		o = key;
		while (*s && *s != '\\')
			*o++ = *s++;

		l = o - key;
		if (l < 20)
		{
			memset (o, ' ', 20-l);
			key[20] = 0;
		}
		else
			*o = 0;
		Com_Printf ("%s", key);

		if (!*s)
		{
			Com_Printf ("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (*s)
			s++;
		Com_Printf ("%s\n", value);
	}
}

//============================================================================

/*
=============
Com_Error_f

Just throw a fatal error to
test error shutdown procedures
=============
*/
static void Com_Error_f (void)
{
	Com_Error (ERR_FATAL, "%s", Cmd_Argv(1));
}

extern cvar_t *logfile_active;

void Qcommon_Frame (int msec);

void
Qcommon_Init(int argc, char **argv)
{
	char	*s;

	if (setjmp (abortframe) )
		Sys_Error ("Error during initialization");

	z_chain.next = z_chain.prev = &z_chain;

	// prepare enough of the subsystems to handle
	// cvar and command buffer management
	COM_InitArgv (argc, argv);

	Swap_Init ();
	Cbuf_Init ();

	Cmd_Init ();
	Cvar_Init ();

	Key_Init ();

	// we need to add the early commands twice, because
	// a basedir or cddir needs to be set before execing
	// config files, but we want other parms to override
	// the settings of the config files
	Cbuf_AddEarlyCommands (false);
	Cbuf_Execute ();

	FS_InitFilesystem ();

	Cbuf_AddText ("exec default.cfg\n");
	Cbuf_AddText ("exec config.cfg\n");

	Cbuf_AddEarlyCommands (true);
	Cbuf_Execute ();

	// remember the initial game name that might have been set on commandline
	{
		cvar_t* gameCvar = Cvar_Get("game", "", CVAR_LATCH | CVAR_SERVERINFO);
		const char* game = "";

		if(gameCvar->string && gameCvar->string[0])
		{
			game = gameCvar->string;
		}

		Q_strlcpy(userGivenGame, game, sizeof(userGivenGame));
	}

	//
	// init commands and vars
	//
	Cmd_AddCommand ("z_stats", Z_Stats_f);
	Cmd_AddCommand ("error", Com_Error_f);

	host_speeds = Cvar_Get ("host_speeds", "0", 0);
	log_stats = Cvar_Get ("log_stats", "0", 0);
	developer = Cvar_Get ("developer", "0", 0);
	modder = Cvar_Get ("modder", "0", 0);
	timescale = Cvar_Get ("timescale", "1", 0);
	fixedtime = Cvar_Get ("fixedtime", "0", 0);
	logfile_active = Cvar_Get ("logfile", "0", 0);
	showtrace = Cvar_Get ("showtrace", "0", 0);
#ifdef DEDICATED_ONLY
	dedicated = Cvar_Get ("dedicated", "1", CVAR_NOSET);
#else
	dedicated = Cvar_Get ("dedicated", "0", CVAR_NOSET);
#endif

	s = va("Heretic II %s", YQ2VERSION);
	Cvar_Get ("version", s, CVAR_SERVERINFO|CVAR_NOSET);


	if (dedicated->value)
		Cmd_AddCommand ("quit", Com_Quit);

	Sys_Init ();

	NET_Init ();
	Netchan_Init ();

	SV_Init ();
	CL_Init ();

	// add + commands from command line
	if (!Cbuf_AddLateCommands ())
	{	// if the user didn't give any commands, run default action
		if (!dedicated->value)
			Cbuf_AddText("demo_loop\n");
		else
			Cbuf_AddText ("dedicated_start\n");
		Cbuf_Execute ();
	}
	else
	{	// the user asked for something explicit
		// so drop the loading plaque
		SCR_EndLoadingPlaque ();
	}

	Com_Printf ("====== Heretic II fully initialized ======\n\n");

	int 	time, oldtime, newtime;

	oldtime = Sys_Milliseconds ();
	while (1)
	{
		// find time spent rendering last frame
		do {
			newtime = Sys_Milliseconds ();
			time = newtime - oldtime;
		} while (time < 1);
		Qcommon_Frame (time);
		oldtime = newtime;
	}
}

extern int	c_pointcontents;
extern int	c_traces, c_brush_traces;

/*
=================
Qcommon_Frame
=================
*/
void Qcommon_Frame (int msec)
{
	char	*s;
	int		time_before, time_between, time_after;

	if (setjmp (abortframe) )
		return;			// an ERR_DROP was thrown

	if ( log_stats->modified )
	{
		log_stats->modified = false;
		if ( log_stats->value )
		{
			if ( log_stats_file )
			{
				fclose( log_stats_file );
				log_stats_file = 0;
			}
			log_stats_file = fopen( "stats.log", "w" );
			if ( log_stats_file )
				fprintf( log_stats_file, "entities,dlights,parts,frame time\n" );
		}
		else
		{
			if ( log_stats_file )
			{
				fclose( log_stats_file );
				log_stats_file = 0;
			}
		}
	}

	if (fixedtime->value)
		msec = fixedtime->value;
	else if (timescale->value)
	{
		msec *= timescale->value;
		if (msec < 1)
			msec = 1;
	}

	if (showtrace->value)
	{
		Com_Printf ("%4i traces  %4i points\n", c_traces, c_pointcontents);
		c_traces = 0;
		c_brush_traces = 0;
		c_pointcontents = 0;
	}

	do
	{
		s = Sys_ConsoleInput ();
		if (s)
			Cbuf_AddText (va("%s\n",s));
	} while (s);
	Cbuf_Execute ();

	if (host_speeds->value)
		time_before = Sys_Milliseconds ();

	SV_Frame (msec);

	if (host_speeds->value)
		time_between = Sys_Milliseconds ();

	CL_Frame (msec);

	if (host_speeds->value)
		time_after = Sys_Milliseconds ();


	if (host_speeds->value)
	{
		int			all, sv, gm, cl, rf;

		all = time_after - time_before;
		sv = time_between - time_before;
		cl = time_after - time_between;
		gm = time_after_game - time_before_game;
		rf = time_after_ref - time_before_ref;
		sv -= gm;
		cl -= rf;
		Com_Printf ("all:%3i sv:%3i gm:%3i cl:%3i rf:%3i\n",
			all, sv, gm, cl, rf);
	}
}

void
Qcommon_Shutdown(void)
{
	FS_ShutdownFilesystem();
}
