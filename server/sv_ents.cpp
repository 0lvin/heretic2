/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "server.h"

void SV_WriteClientEffectsToClient(client_frame_t* from, client_frame_t* to, sizebuf_t* msg);

/*
=============================================================================

Encode a client frame onto the network channel

=============================================================================
*/

#if 0

// because there can be a lot of projectiles, there is a special
// network protocol for them
#define	MAX_PROJECTILES		64
edict_t	*projectiles[MAX_PROJECTILES];
int		numprojs;
cvar_t  *sv_projectiles;

qboolean SV_AddProjectileUpdate (edict_t *ent)
{
	if (!sv_projectiles)
		sv_projectiles = Cvar_Get("sv_projectiles", "1", 0);

	if (!sv_projectiles->value)
		return false;

	if (!(ent->svflags & SVF_PROJECTILE))
		return false;
	if (numprojs == MAX_PROJECTILES)
		return true;

	projectiles[numprojs++] = ent;
	return true;
}

void SV_EmitProjectileUpdate (sizebuf_t *msg)
{
	byte	bits[16];	// [modelindex] [48 bits] xyz p y 12 12 12 8 8 [entitynum] [e2]
	int		n, i;
	edict_t	*ent;
	int		x, y, z, p, yaw;
	int len;

	if (!numprojs)
		return;

	MSG_WriteByte (msg, numprojs);

	for (n=0 ; n<numprojs ; n++)
	{
		ent = projectiles[n];
		x = (int)(ent->s.origin[0]+4096)>>1;
		y = (int)(ent->s.origin[1]+4096)>>1;
		z = (int)(ent->s.origin[2]+4096)>>1;
		p = (int)(256*ent->s.angles[0]/360)&255;
		yaw = (int)(256*ent->s.angles[1]/360)&255;

		len = 0;
		bits[len++] = x;
		bits[len++] = (x>>8) | (y<<4);
		bits[len++] = (y>>4);
		bits[len++] = z;
		bits[len++] = (z>>8);
		if (ent->s.effects & EF_BLASTER)
			bits[len-1] |= 64;

		if (ent->s.old_origin[0] != ent->s.origin[0] ||
			ent->s.old_origin[1] != ent->s.origin[1] ||
			ent->s.old_origin[2] != ent->s.origin[2]) {
			bits[len-1] |= 128;
			x = (int)(ent->s.old_origin[0]+4096)>>1;
			y = (int)(ent->s.old_origin[1]+4096)>>1;
			z = (int)(ent->s.old_origin[2]+4096)>>1;
			bits[len++] = x;
			bits[len++] = (x>>8) | (y<<4);
			bits[len++] = (y>>4);
			bits[len++] = z;
			bits[len++] = (z>>8);
		}

		bits[len++] = p;
		bits[len++] = yaw;
		bits[len++] = ent->s.modelindex;

		bits[len++] = (ent->s.number & 0x7f);
		if (ent->s.number > 255) {
			bits[len-1] |= 128;
			bits[len++] = (ent->s.number >> 7);
		}

		for (i=0 ; i<len ; i++)
			MSG_WriteByte (msg, bits[i]);
	}
}
#endif

/*
=============
SV_EmitPacketEntities

Writes a delta update of an entity_state_t list to the message.
=============
*/
void SV_EmitPacketEntities (client_frame_t *from, client_frame_t *to, sizebuf_t *msg)
{
	entity_state_t	*oldent = NULL, *newent = NULL;
	int		oldindex, newindex;
	int		oldnum, newnum;
	int		from_num_entities;
	int		bits;

#if 0
	if (numprojs)
		MSG_WriteByte (msg, svc_packetentities2);
	else
#endif
		MSG_WriteByte (msg, svc_packetentities);

	if (!from)
		from_num_entities = 0;
	else
		from_num_entities = from->num_entities;

	newindex = 0;
	oldindex = 0;
	while (newindex < to->num_entities || oldindex < from_num_entities)
	{
		if (newindex >= to->num_entities)
			newnum = 9999;
		else
		{
			newent = &svs.client_entities[(to->first_entity+newindex)%svs.num_client_entities];
			newnum = newent->number;
		}

		if (oldindex >= from_num_entities)
			oldnum = 9999;
		else
		{
			oldent = &svs.client_entities[(from->first_entity+oldindex)%svs.num_client_entities];
			oldnum = oldent->number;
		}

		if (newnum == oldnum)
		{	// delta update from old position
			// because the force parm is false, this will not result
			// in any bytes being emited if the entity has not changed at all
			// note that players are always 'newentities', this updates their oldorigin always
			// and prevents warping
			MSG_WriteDeltaEntity (oldent, newent, msg, false, newent->number <= maxclients->value);
			oldindex++;
			newindex++;
			continue;
		}

		if (newnum < oldnum)
		{	// this is a new entity, send it from the baseline
			MSG_WriteDeltaEntity (&sv.baselines[newnum], newent, msg, true, true);
			newindex++;
			continue;
		}

		if (newnum > oldnum)
		{	// the old entity isn't present in the new message
			bits = U_REMOVE;
			if (oldnum >= 256)
				bits |= U_NUMBER16;

			MSG_WriteLong (msg,	bits );
			MSG_WriteShort (msg, oldnum);			

			oldindex++;
			continue;
		}
	}

	MSG_WriteLong(msg, 0);
	MSG_WriteShort (msg, 0);	// end of packetentities

#if 0
	if (numprojs)
		SV_EmitProjectileUpdate(msg);
#endif
}



/*
=============
SV_WritePlayerstateToClient

=============
*/
void SV_WritePlayerstateToClient (client_frame_t *from, client_frame_t *to, sizebuf_t *msg)
{
	int				i;
	int				pflags;
	player_state_t	*ps, *ops;
	player_state_t	dummy;
	int				statbits;

	ps = &to->ps;
	if (!from)
	{
		memset (&dummy, 0, sizeof(dummy));
		ops = &dummy;
	}
	else
		ops = &from->ps;

	//
	// determine what needs to be sent
	//
	pflags = 0;

	if (ps->pmove.pm_type != ops->pmove.pm_type)
		pflags |= PS_M_TYPE;

	if (ps->pmove.origin[0] != ops->pmove.origin[0]
		|| ps->pmove.origin[1] != ops->pmove.origin[1]
		|| ps->pmove.origin[2] != ops->pmove.origin[2] ) {
		pflags |= PS_M_ORIGIN_XY;
		pflags |= PS_M_ORIGIN_Z;
	}

	if (ps->pmove.velocity[0] != ops->pmove.velocity[0]
		|| ps->pmove.velocity[1] != ops->pmove.velocity[1]
		|| ps->pmove.velocity[2] != ops->pmove.velocity[2])
	{
		pflags |= PS_M_VELOCITY_XY;
		pflags |= PS_M_VELOCITY_Z;
	}

	if (ps->pmove.pm_time != ops->pmove.pm_time)
		pflags |= PS_M_TIME;

	if (ps->pmove.pm_flags != ops->pmove.pm_flags)
		pflags |= PS_M_FLAGS;

	if (ps->viewheight != ops->viewheight)
		pflags |= PS_VIEWHEIGHT;

	if (ps->pmove.gravity != ops->pmove.gravity)
		pflags |= PS_M_GRAVITY;

	if (ps->pmove.delta_angles[0] != ops->pmove.delta_angles[0]
		|| ps->pmove.delta_angles[1] != ops->pmove.delta_angles[1]
		|| ps->pmove.delta_angles[2] != ops->pmove.delta_angles[2] )
		pflags |= PS_M_DELTA_ANGLES;


	if (ps->viewangles[0] != ops->viewangles[0]
		|| ps->viewangles[1] != ops->viewangles[1]
		|| ps->viewangles[2] != ops->viewangles[2] )
		pflags |= PS_VIEWANGLES;

	if (ps->fov != ops->fov)
		pflags |= PS_FOV;

	if (ps->rdflags != ops->rdflags)
		pflags |= PS_RDFLAGS;

	if (ps->remote_vieworigin[0] != ops->remote_vieworigin[0] ||
		ps->remote_vieworigin[1] != ops->remote_vieworigin[1] ||
		ps->remote_vieworigin[2] != ops->remote_vieworigin[2]) {

		pflags |= PS_REMOTE_VIEWORIGIN;
	}

	if (ps->remote_viewangles[0] != ops->remote_viewangles[0] ||
		ps->remote_viewangles[1] != ops->remote_viewangles[1] ||
		ps->remote_viewangles[2] != ops->remote_viewangles[2]) {

		pflags |= PS_REMOTE_VIEWANGLES;
	}

	if (ps->mins[0] != ops->mins[0] || ps->mins[1] != ops->mins[1] || ps->mins[2] != ops->mins[2])
		pflags |= PS_MINSMAXS;

	if (ps->maxs[0] != ops->maxs[0] || ps->maxs[1] != ops->maxs[1] || ps->maxs[2] != ops->maxs[2])
		pflags |= PS_MINSMAXS;

	if (ps->remote_id != ops->remote_id)
	{
		pflags |= PS_REMOTE_ID;
	}

	//
	// write it
	//
	MSG_WriteByte (msg, svc_playerinfo);
	MSG_WriteLong (msg, pflags);

	MSG_WriteData(msg, (byte *)&ps->stats[0], sizeof(ps->stats));

	if (pflags & PS_MINSMAXS) {
		MSG_WriteFloat(msg, ps->mins[0]);
		MSG_WriteFloat(msg, ps->mins[1]);
		MSG_WriteFloat(msg, ps->mins[2]);

		MSG_WriteFloat(msg, ps->maxs[0]);
		MSG_WriteFloat(msg, ps->maxs[1]);
		MSG_WriteFloat(msg, ps->maxs[2]);
	}

	//
	// write the pmove_state_t
	//
	if (pflags & PS_M_TYPE)
		MSG_WriteByte (msg, ps->pmove.pm_type);

	if (pflags & PS_REMOTE_ID)
	{
		MSG_WriteShort(msg, ps->remote_id);
	}

	if (pflags & PS_REMOTE_VIEWORIGIN)
	{
		MSG_WriteShort(msg, ps->remote_vieworigin[0]);
		MSG_WriteShort(msg, ps->remote_vieworigin[1]);
		MSG_WriteShort(msg, ps->remote_vieworigin[2]);
	}

	if (pflags & PS_REMOTE_VIEWANGLES)
	{
		MSG_WriteShort(msg, ps->remote_viewangles[0]);
		MSG_WriteShort(msg, ps->remote_viewangles[1]);
		MSG_WriteShort(msg, ps->remote_viewangles[2]);
	}

	if (pflags & PS_M_ORIGIN_XY)
	{
		MSG_WriteShort (msg, ps->pmove.origin[0]);
		MSG_WriteShort (msg, ps->pmove.origin[1]);		
	}

	if (pflags & PS_VIEWHEIGHT)
	{
		MSG_WriteShort(msg, ps->viewheight);
	}

	if (pflags & PS_M_ORIGIN_Z)
	{
		MSG_WriteShort(msg, ps->pmove.origin[2]);
	}

	if (pflags & PS_M_VELOCITY_XY)
	{
		MSG_WriteShort (msg, ps->pmove.velocity[0]);
		MSG_WriteShort (msg, ps->pmove.velocity[1]);		
	}

	if (pflags & PS_M_VELOCITY_Z)
	{
		MSG_WriteShort(msg, ps->pmove.velocity[2]);
	}

	if (pflags & PS_M_TIME)
		MSG_WriteByte (msg, ps->pmove.pm_time);

	if (pflags & PS_M_FLAGS)
		MSG_WriteByte (msg, ps->pmove.pm_flags);

	if (pflags & PS_M_GRAVITY)
		MSG_WriteShort (msg, ps->pmove.gravity);

	if (pflags & PS_M_DELTA_ANGLES)
	{
		MSG_WriteShort (msg, ps->pmove.delta_angles[0]);
		MSG_WriteShort (msg, ps->pmove.delta_angles[1]);
		MSG_WriteShort (msg, ps->pmove.delta_angles[2]);
	}

	//
	// write the rest of the player_state_t
	//
	if (pflags & PS_VIEWANGLES)
	{
		MSG_WriteAngle16 (msg, ps->viewangles[0]);
		MSG_WriteAngle16 (msg, ps->viewangles[1]);
		MSG_WriteAngle16 (msg, ps->viewangles[2]);
	}

	if (pflags & PS_FOV)
		MSG_WriteByte (msg, ps->fov);
	if (pflags & PS_RDFLAGS)
		MSG_WriteByte (msg, ps->rdflags);

	// send stats
	//statbits = 0;
	//for (i=0 ; i<MAX_STATS ; i++)
	//	if (ps->stats[i] != ops->stats[i])
	//		statbits |= 1<<i;
	//MSG_WriteLong (msg, statbits);
	//for (i=0 ; i<MAX_STATS ; i++)
	//	if (statbits & (1<<i) )
	//		MSG_WriteShort (msg, ps->stats[i]);
}


/*
==================
SV_WriteFrameToClient
==================
*/
void SV_WriteFrameToClient (client_t *client, sizebuf_t *msg)
{
	client_frame_t		*frame, *oldframe;
	int					lastframe;

//Com_Printf ("%i -> %i\n", client->lastframe, sv.framenum);
	// this is the frame we are creating
	frame = &client->frames[sv.framenum & UPDATE_MASK];

	if (client->lastframe <= 0)
	{	// client is asking for a retransmit
		oldframe = NULL;
		lastframe = -1;
	}
	else if (sv.framenum - client->lastframe >= (UPDATE_BACKUP - 3) )
	{	// client hasn't gotten a good message through in a long time
//		Com_Printf ("%s: Delta request from out-of-date packet.\n", client->name);
		oldframe = NULL;
		lastframe = -1;
	}
	else
	{	// we have a valid message to delta from
		oldframe = &client->frames[client->lastframe & UPDATE_MASK];
		lastframe = client->lastframe;
	}

	MSG_WriteByte (msg, svc_frame);
	MSG_WriteLong (msg, sv.framenum);
	MSG_WriteLong (msg, lastframe);	// what we are delta'ing from
	//MSG_WriteByte (msg, client->surpressCount);	// rate dropped packets
	client->surpressCount = 0;

	// send over the areabits
	MSG_WriteByte (msg, frame->areabytes);
	SZ_Write (msg, frame->areabits, frame->areabytes);

	// delta encode the playerstate
	SV_WritePlayerstateToClient (oldframe, frame, msg);

	// delta encode the entities
	SV_EmitPacketEntities (oldframe, frame, msg);

	// Write our global effects to the client.
	SV_WriteClientEffectsToClient(oldframe, frame, msg);
}


/*
=============================================================================

Build a client frame structure

=============================================================================
*/

byte		fatpvs[65536/8];	// 32767 is MAX_MAP_LEAFS

/*
============
SV_FatPVS

The client will interpolate the view position,
so we can't use a single PVS point
===========
*/
void SV_FatPVS (vec3_t org)
{
	int		leafs[64];
	int		i, j, count;
	int		longs;
	byte	*src;
	vec3_t	mins, maxs;

	for (i=0 ; i<3 ; i++)
	{
		mins[i] = org[i] - 8;
		maxs[i] = org[i] + 8;
	}

	count = CM_BoxLeafnums (mins, maxs, leafs, 64, NULL);
	if (count < 1)
		Com_Error (ERR_FATAL, "SV_FatPVS: count < 1");
	longs = (CM_NumClusters()+31)>>5;

	// convert leafs to clusters
	for (i=0 ; i<count ; i++)
		leafs[i] = CM_LeafCluster(leafs[i]);

	memcpy (fatpvs, CM_ClusterPVS(leafs[0]), longs<<2);
	// or in all the other leaf bits
	for (i=1 ; i<count ; i++)
	{
		for (j=0 ; j<i ; j++)
			if (leafs[i] == leafs[j])
				break;
		if (j != i)
			continue;		// already have the cluster we want
		src = CM_ClusterPVS(leafs[i]);
		for (j=0 ; j<longs ; j++)
			((long *)fatpvs)[j] |= ((long *)src)[j];
	}
}


/*
=============
SV_BuildClientFrame

Decides which entities are going to be visible to the client, and
copies off the playerstat and areabits.
=============
*/
void SV_BuildClientFrame (client_t *client)
{
	int		e, i;
	vec3_t	org;
	edict_t	*ent;
	edict_t	*clent;
	client_frame_t	*frame;
	entity_state_t	*state;
	int		l;
	int		clientarea, clientcluster;
	int		leafnum;
	int		c_fullsend;
	byte	*clientphs;
	byte	*bitvector;

	clent = client->edict;
	if (!clent->client)
		return;		// not in game yet

#if 0
	numprojs = 0; // no projectiles yet
#endif

	// this is the frame we are creating
	frame = &client->frames[sv.framenum & UPDATE_MASK];

	frame->senttime = svs.realtime; // save it for ping calc later

	// find the client's PVS	
	for (i = 0; i < 3; i++) {
		if (clent->client->ps.remote_id == -1)
		{
			org[i] = clent->client->ps.pmove.origin[i] * 0.125;
		}
		else
		{
			org[i] = clent->client->ps.remote_vieworigin[i];
		}
	}

	leafnum = CM_PointLeafnum (org);
	clientarea = CM_LeafArea (leafnum);
	clientcluster = CM_LeafCluster (leafnum);

	// calculate the visible areas
	frame->areabytes = CM_WriteAreaBits (frame->areabits, clientarea);

	// grab the current player_state_t
	frame->ps = clent->client->ps;


	SV_FatPVS (org);
	clientphs = CM_ClusterPHS (clientcluster);

	// build up the list of visible entities
	frame->num_entities = 0;
	frame->first_entity = svs.next_client_entities;

	c_fullsend = 0;

	for (e=1 ; e<ge->num_edicts ; e++)
	{
		ent = EDICT_NUM(e);

		// ignore ents without visible models
		if (ent->svflags & SVF_NOCLIENT)
			continue;

		// ignore ents without visible models unless they have an effect
		if (!ent->s.modelindex && !ent->s.effects && !ent->s.sound)
			continue;

		// ignore if not touching a PV leaf
		if (ent != clent)
		{
			// check area
			if (!CM_AreasConnected (clientarea, ent->areanum))
			{	// doors can legally straddle two areas, so
				// we may need to check another one
				if (!ent->areanum2
					|| !CM_AreasConnected (clientarea, ent->areanum2))
					continue;		// blocked by a door
			}

			{
				// FIXME: if an ent has a model and a sound, but isn't
				// in the PVS, only the PHS, clear the model
				if (ent->s.sound)
				{
					bitvector = fatpvs;	//clientphs;
				}
				else
					bitvector = fatpvs;

				if (ent->num_clusters == -1)
				{	// too many leafs for individual check, go by headnode
					if (!CM_HeadnodeVisible (ent->headnode, bitvector))
						continue;
					c_fullsend++;
				}
				else
				{	// check individual leafs
					for (i=0 ; i < ent->num_clusters ; i++)
					{
						l = ent->clusternums[i];
						if (bitvector[l >> 3] & (1 << (l&7) ))
							break;
					}
					if (i == ent->num_clusters)
						continue;		// not visible
				}

				if (!ent->s.modelindex)
				{	// don't send sounds if they will be attenuated away
					vec3_t	delta;
					float	len;

					VectorSubtract (org, ent->s.origin, delta);
					len = VectorLength (delta);
					if (len > 400)
						continue;
				}
			}
		}

#if 0
		if (SV_AddProjectileUpdate(ent))
			continue; // added as a special projectile
#endif

		// add it to the circular client_entities array
		state = &svs.client_entities[svs.next_client_entities%svs.num_client_entities];
		if (ent->s.number != e)
		{
			Com_DPrintf ("FIXING ENT->S.NUMBER!!!\n");
			ent->s.number = e;
		}
		*state = ent->s;

		// don't mark players missiles as solid
		if (ent->owner == client->edict)
			state->solid = 0;

		svs.next_client_entities++;
		frame->num_entities++;
	}
}


/*
==================
SV_RecordDemoMessage

Save everything in the world out without deltas.
Used for recording footage for merged or assembled demos
==================
*/
void SV_RecordDemoMessage (void)
{
	int			e;
	edict_t		*ent;
	entity_state_t	nostate;
	sizebuf_t	buf;
	byte		buf_data[32768];
	int			len;

	if (!svs.demofile)
		return;

	memset (&nostate, 0, sizeof(nostate));
	SZ_Init (&buf, buf_data, sizeof(buf_data));

	// write a frame message that doesn't contain a player_state_t
	MSG_WriteByte (&buf, svc_frame);
	MSG_WriteLong (&buf, sv.framenum);

	MSG_WriteByte (&buf, svc_packetentities);

	e = 1;
	ent = EDICT_NUM(e);
	while (e < ge->num_edicts) 
	{
		// ignore ents without visible models unless they have an effect
		if (ent->inuse &&
			ent->s.number && 
			(ent->s.modelindex || ent->s.effects || ent->s.sound) && 
			!(ent->svflags & SVF_NOCLIENT))
			MSG_WriteDeltaEntity (&nostate, &ent->s, &buf, false, true);

		e++;
		ent = EDICT_NUM(e);
	}

	MSG_WriteShort (&buf, 0);		// end of packetentities

	// now add the accumulated multicast information
	SZ_Write (&buf, svs.demo_multicast.data, svs.demo_multicast.cursize);
	SZ_Clear (&svs.demo_multicast);

	// now write the entire message to the file, prefixed by the length
	len = LittleLong (buf.cursize);
	fwrite (&len, 4, 1, svs.demofile);
	fwrite (buf.data, buf.cursize, 1, svs.demofile);
}

