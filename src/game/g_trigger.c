/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (c) ZeniMax Media Inc.
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
 * Trigger.
 *
 * =======================================================================
 */

#include "header/local.h"
#include "header/g_defaultmessagehandler.h"
#include "header/g_playstats.h"
#include "common/cl_strings.h"
#include "player/library/p_main.h"

#define TRIGGER_MONSTER 0x01
#define TRIGGER_NOT_PLAYER 0x02
#define TRIGGER_TRIGGERED 0x04
#define TRIGGER_TOGGLE 0x08

#define PUSH_ONCE 0x01
#define PUSH_START_OFF 0x02
#define PUSH_SILENT 0x04

void trigger_push_active(edict_t *self);
void hurt_touch(edict_t *self, edict_t *other, cplane_t *plane /* unused */,
		csurface_t *surf /* unused */);

#define PUZZLE_SHOWNO_INVENTORY 16
#define PUZZLE_DONT_REMOVE		32

void Trigger_Deactivate(edict_t *self, G_Message_t *msg);
void Trigger_Activate(edict_t *self, G_Message_t *msg);

void trigger_enable(edict_t *self, edict_t *other, edict_t *activator);
void Use_Multi(edict_t *self, edict_t *other, edict_t *activator);
void InitField(edict_t *self);

void TriggerStaticsInit()
{
	classStatics[CID_TRIGGER].msgReceivers[G_MSG_SUSPEND] = Trigger_Deactivate;
	classStatics[CID_TRIGGER].msgReceivers[G_MSG_UNSUSPEND] = Trigger_Activate;
}

/*
 * The wait time has passed, so
 * set back up for another activation
 */
void
multi_wait(edict_t *self)
{
	self->think = NULL;
	if (self->activator)
	{
		self->activator->target_ent = NULL;
	}
}

// the trigger was just activated
// self->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void TriggerActivated(edict_t *self)
{
	if (self->think)
	{
		return;		// already been triggered
	}

	assert(self->TriggerActivated);

	self->TriggerActivated(self, self->activator);

	if (self->wait > 0)
	{
		self->think = multi_wait;
		self->nextthink = level.time + self->wait;
	}
	else
	{
		self->touch = NULL;
		self->nextthink = level.time + FRAMETIME;
		self->think = G_FreeEdict;
	}
}

void Touch_Multi(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// Monsters or players can trigger it
	if ((self->spawnflags & TRIGGER_TOGGLE) && ((strcmp(other->classname, "player") == 0) ||
		(other->svflags & SVF_MONSTER)))
		;
	// Player cannot trigger it
	else if(strcmp(other->classname, "player") == 0)
	{
		if (self->spawnflags & TRIGGER_NOT_PLAYER)
		{
			return;
		}
	}
	// Just monster will trigger it
	else if(other->svflags & SVF_MONSTER)
	{
		if (!(self->spawnflags & TRIGGER_MONSTER))
		{
			return;
		}
	}
	else
	{
		return;
	}

	if (!Vec3IsZero(self->movedir))
	{
		vec3_t forward;

		AngleVectors(other->s.angles, forward, NULL, NULL);

		if (DotProduct(forward, self->movedir) < 0)
		{
			return;
		}
	}

	self->activator = other;

	TriggerActivated(self);
}

void InitTrigger(edict_t *self)
{
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_TRIGGER;

	if (!self->wait)
	{
		self->wait = 0.2;
	}

	// Triggers still use the touch function even with the new physics
	self->touch = Touch_Multi;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	if (self->spawnflags & TRIGGER_TRIGGERED)
	{
		self->solid = SOLID_NOT;
		self->use = trigger_enable;
	}
	else
	{
		self->solid = SOLID_TRIGGER;
		self->use = Use_Multi;
	}

	if (!Vec3IsZero(self->s.angles))
	{
		G_SetMovedir(self->s.angles, self->movedir);
	}

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}

void Trigger_Deactivate(edict_t *self, G_Message_t *msg)
{
	self->solid = SOLID_NOT;
	self->use = NULL;
}

void Trigger_Activate(edict_t *self, G_Message_t *msg)
{
	self->solid = SOLID_TRIGGER;
	self->use = Use_Multi;
	gi.linkentity(self);
}

void Trigger_Sounds(edict_t *self)
{
	if (self->sounds == 1)
		self->noise_index = gi.soundindex("misc/secret.wav");
	else if (self->sounds == 3)
		self->noise_index = gi.soundindex("misc/talk.wav");
	else
		self->noise_index = 0;
}

//----------------------------------------------------------------------
// One Time Trigger
//----------------------------------------------------------------------

/*QUAKED trigger_multiple (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY
Variable sized repeatable trigger.  Must be targeted at one or more entities.
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
--------KEYS---------
delay   - Time to wait after activating before firing.
message - text string to display when activated
wait    - Seconds between triggerings. (.2 default)
sounds  - sound made when activating
1)	secret
2)	none
3)	large switch
*/
void SP_trigger_multiple(edict_t *self)
{
	InitTrigger(self);

	self->TriggerActivated = G_UseTargets;

	Trigger_Sounds(self);

}

void trigger_enable(edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_TRIGGER;
	self->use = Use_Multi;
	gi.linkentity(self);
}

void Use_Multi(edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;
	TriggerActivated(self);
}

//----------------------------------------------------------------------
// One Time Trigger
//----------------------------------------------------------------------

/*QUAKED trigger_once (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY
Triggers once, then removes itself.
You must set the key "target" to the name of another object in the level that has a matching "targetname".
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it

sounds
 1)	secret
 2)	no sound
 3)	large switch

"message"	string to be displayed when triggered
*/

void SP_trigger_once(edict_t *self)
{
	InitTrigger(self);

	self->TriggerActivated = G_UseTargets;

	self->wait = -1;

	Trigger_Sounds(self);

}

//----------------------------------------------------------------------
// Relay Trigger
//----------------------------------------------------------------------

void trigger_relay_use(edict_t *self, edict_t *other, edict_t *activator);

/*QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
This fixed size trigger cannot be touched, it can only be fired by other events.
*/
void
SP_trigger_relay(edict_t *self)
{
	self->use = trigger_relay_use;
}

void
trigger_relay_use(edict_t *self, edict_t *other, edict_t *activator)
{
	G_UseTargets(self, activator);
}

//----------------------------------------------------------------------
// Key Trigger
//----------------------------------------------------------------------

void trigger_key_use(edict_t *self, edict_t *other, edict_t *activator);

/*QUAKED trigger_puzzle (.5 .5 .5) (-8 -8 -8) (8 8 8)  NO_TEXT  NO_TAKE
A relay trigger that only fires it's targets if player has the proper puzzle item.
------KEYS--------------
NO_TEXT - won't generate the "You need..." text when triggered
NO_TAKE - don't take puzzle item from player inventory
------FIELDS------------
Use "item" to specify the required puzzle item, for example "key_data_cd"
*/
void SP_trigger_puzzle(edict_t *self)
{
	self->classID = CID_TRIGGER;

	if (!st.item)
	{
		gi.dprintf("no key item for trigger_key at %s\n", vtos(self->s.origin));
		return;
	}
	self->item = FindItemByClassname(st.item);

	if (!self->item)
	{
		gi.dprintf("item %s not found for trigger_key at %s\n", st.item, vtos(self->s.origin));
		return;
	}

	if (!self->target)
	{
		gi.dprintf("%s at %s has no target\n", self->classname, vtos(self->s.origin));
		return;
	}

	self->use = trigger_key_use;
}

void
trigger_key_use(edict_t *self, edict_t *other, edict_t *activator)
{
	int	index;
	edict_t *puzzle;

	if (!self->item)
		return;
	if (!activator->client)
		return;

	index = ITEM_INDEX(self->item);

	if (!activator->client->playerinfo.pers.inventory[index])
	{
		if (level.time < self->touch_debounce_time)
			return;
		self->touch_debounce_time = level.time + 5.0;
		if (!(self->spawnflags & 1))
			G_CPrintf(activator, PRINT_HIGH, self->item->msg_nouse);

		return;
	}

	// Clear out the puzzle piece from all clients.

	if (!(self->spawnflags & 2))
	{
		int		i;
		edict_t	*ent;

		if (coop->value)	// If COOP remove model from world if puzzle item is used.
		{
			puzzle = NULL;

			puzzle = G_Find(puzzle, FOFS(classname), (char *) self->item->classname);

			if (puzzle)
			{
				gi.sound(puzzle, CHAN_ITEM, gi.soundindex(self->item->pickup_sound), 1, ATTN_NORM, 0);

				gi.CreateEffect(NULL, FX_PICKUP, 0, puzzle->s.origin, "");

				puzzle->solid = SOLID_NOT;

				// Once picked up, the item is gone forever, so remove it's client effect(s).
				G_RemoveEffects(puzzle, FX_REMOVE_EFFECTS);

				// The persistent part is removed from the server here.
				G_SetToFree(puzzle);
			}
		}


		for (i=0 ; i<maxclients->value ; i++)
		{
			ent = g_edicts + 1 + i;

			if (!ent->inuse)
				continue;

			ent->client->playerinfo.pers.inventory[index]=0;
		}
	}

	gi.sound(self, CHAN_AUTO, gi.soundindex("player/useobject.wav"), 1, ATTN_NORM, 0);

	G_UseTargets (self, activator);

	self->use = NULL;

	if (!(other->spawnflags & PUZZLE_DONT_REMOVE))	// Get rid of it.
	{
		G_SetToFree(other);
// jmarshall - cleaned this up.
		activator->target_ent = NULL;
		activator->client->playerinfo.target_ent = NULL;
// jmarshall end
	}
}

//----------------------------------------------------------------------
// Counter Trigger
//----------------------------------------------------------------------

void trigger_counter_use(edict_t *self, edict_t *other, edict_t *activator);

#define TRIGGER_COUNTER_NOMESSAGE	1
/*QUAKED trigger_counter (.5 .5 .5) ? NOMESSAGE
Acts as an intermediary for an action that takes multiple inputs.

If NOMESSAGE is not set, t will print "1 more.. " etc when triggered and "sequence complete" when finished.

After the counter has been triggered "count" times (default 2), it will fire all of it's targets and remove itself.
*/
void SP_trigger_counter(edict_t *self)
{
	self->classID = CID_TRIGGER;

	self->wait = -1;

	if (!self->count)
	{
		self->count = 2;
	}

	self->use = trigger_counter_use;

	self->TriggerActivated = G_UseTargets;
}

void trigger_counter_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->count == 0)
	{
		return;
	}

	self->count--;

	if (self->count)
	{
		if (! (self->spawnflags & TRIGGER_COUNTER_NOMESSAGE))
		{
			G_CPrintf(activator, PRINT_HIGH, (short)(self->count + GM_SEQCOMPLETE));
//			gi.sound(activator, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	if (! (self->spawnflags & TRIGGER_COUNTER_NOMESSAGE))
	{
		G_CPrintf(activator, PRINT_HIGH, GM_SEQCOMPLETE);
//		gi.sound(activator, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}

	self->activator = activator;

	TriggerActivated(self);
}

//----------------------------------------------------------------------
// Always Trigger
//----------------------------------------------------------------------

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_always(edict_t *self)
{
	self->classID = CID_TRIGGER;

	// we must have some delay to make sure our use targets are present
	if (self->delay < 0.2)
	{
		self->delay = 0.2;
	}

	G_UseTargets(self, self);
}

//----------------------------------------------------------------------
// Player Use Item
//----------------------------------------------------------------------

void trigger_playerusepuzzle(edict_t *self, edict_t *activator)
{
	if (!(self->spawnflags & PUZZLE_SHOWNO_INVENTORY))
	{
		if (!strcmp(activator->classname, "player"))
		{
			activator->target_ent = self;
			self->activator = activator;
		}
	}
	else
		G_UseTargets(self,activator);

}

/*QUAKED trigger_playerusepuzzle (.5 .5 .5) ?  MONSTER NOT_PLAYER TRIGGERED ANY NO_INVENTORY DONT_REMOVE
Player can 'use' puzzle items within this entity.  Will remove itself after one use.
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
NO_INVENTORY - don't show inventory bar, don't take puzzle piece
DONT_REMOVE - entity won't remove itself after one use
*/

void SP_trigger_PlayerUsePuzzle(edict_t *self)
{
	InitTrigger(self);

	self->wait = 1.0;
	self->TriggerActivated = trigger_playerusepuzzle;

	gi.setmodel (self, self->model);
	gi.linkentity(self);
}

//----------------------------------------------------------------------
// Player Push Button Trigger
//----------------------------------------------------------------------

void trigger_playerpushbutton(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	if (!strcmp(other->classname, "player"))
	{
		other->target = self->target;
	}
}

/*QUAKED trigger_playerpushbutton (.5 .5 .5) ?
Triggers player to know he is near a button.
*/
void SP_trigger_PlayerPushButton(edict_t *self)
{
	self->classID = CID_TRIGGER;

	self->wait = FRAMETIME;
	self->touch = trigger_playerpushbutton;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_TRIGGER;

	gi.setmodel (self, self->model);
	gi.linkentity(self);
}


//----------------------------------------------------------------------
// Suspend Trigger
//----------------------------------------------------------------------

void SuspendTrigger_Activated(edict_t *self, edict_t *activator);

/*QUAKED trigger_Deactivate (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY
Variable sized repeatable trigger, which posts a SUSPEND message to its target.
Must be targeted at one or more entities.
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
--------KEYS-----------
delay - If set, the trigger waits this amount after activating before firing.
wait  - Seconds between triggerings. (.2 default)
message - text string displayed when triggered
*/
void SP_trigger_Deactivate(edict_t *self)
{
	InitTrigger(self);

	self->TriggerActivated = SuspendTrigger_Activated;
}

void SuspendTrigger_Activated(edict_t *self, edict_t *activator)
{
	edict_t		*t;

	assert(self->target);

//
// DeActivate all targets
//
	t = NULL;
	while ((t = G_Find (t, FOFS(targetname), self->target)))
	{
		if (t->msgHandler)
			G_QPostMessage(t, G_MSG_SUSPEND, PRI_ORDER, "f", self->time);
	}
}

//----------------------------------------------------------------------
// Unsuspend Trigger
//----------------------------------------------------------------------

void ActivateTrigger_Activated(edict_t *self, edict_t *activator);

/*QUAKED trigger_Activate (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED ANY
Variable sized repeatable trigger, which posts a UNSUSPEND message to its target.
Must be targeted at one or more entities.
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
------KEYS-----------
delay - If set, the trigger waits this amount after activating before firing.
wait  - Seconds between triggerings. (.2 default)
message - text string displayed when triggered
*/
void SP_trigger_Activate(edict_t *self)
{
	InitTrigger(self);

	self->TriggerActivated = ActivateTrigger_Activated;
}

void ActivateTrigger_Activated(edict_t *self, edict_t *activator)
{
	edict_t		*t;

	assert(self->target);

//
// Activate all targets
//
	t = NULL;
	while ((t = G_Find (t, FOFS(targetname), self->target)))
	{
		if (t->msgHandler)
			G_QPostMessage(t, G_MSG_UNSUSPEND, PRI_ORDER, "f", self->time);
	}
}

void trigger_quit_to_menu_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;

	gi.AddCommandString("menu_main\n");
}

void trigger_quit_to_menu_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!activator->client)
		return;

	gi.AddCommandString("menu_main\n");
}

/*QUAKED trigger_quit_to_menu (.5 .5 .5) ?
Player only, quits to menu
*/

void SP_trigger_quit_to_menu(edict_t *self)
{
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_TRIGGER;

	self->touch = trigger_quit_to_menu_touch;
	self->use = trigger_quit_to_menu_use;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	self->solid = SOLID_TRIGGER;

	if (!Vec3IsZero(self->s.angles))
	{
		G_SetMovedir(self->s.angles, self->movedir);
	}

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}

void mappercentage_use (edict_t *self, edict_t *other)
{
	if (!other->client)	// Only players use these
		return;

	other->client->ps.map_percentage = (byte) self->count;

	G_UseTargets(self, self);

	gi.dprintf("Map percentage updated to %d\n", (byte) self->count);
}

/*QUAKED trigger_mappercentage (0.3 0.1 0.6) ?  MONSTER NOT_PLAYER TRIGGERED ANY
When triggered it updates Player with the percentage of the level completed.
--------FLAGS----------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
----------KEYS---------------
count - amount of level completed
*/
void SP_trigger_mappercentage (edict_t *self)
{
	InitTrigger(self);

	self->TriggerActivated = mappercentage_use;

	if (self->count > 100)
		self->count = 100;
}


void lightning_use (edict_t *self, edict_t *other)
{
	edict_t *target=NULL;
	byte	width, duration;

	width=self->style;
	if (width<1) width=6;
	duration=(byte)(self->delay*10);

	G_UseTargets(self, self);

	// Find the entities targeted by this entity.
	while ((target = G_Find (target, FOFS(targetname), self->target)) != NULL)
	{
		if (target->classname)
		{
			if (strcmp(target->classname, "info_notnull") == 0)
			{
				// Found another with this target.
				if (self->materialtype)	// Red lightning
					gi.CreateEffect(NULL, FX_LIGHTNING, CEF_FLAG6, self->s.origin, "vbb", target->s.origin, width, duration);
				else
					gi.CreateEffect(NULL, FX_LIGHTNING, 0, self->s.origin, "vbb", target->s.origin, width, duration);
			}
		}
	}

	if (self->pain_debounce_time < level.time)
	{
		self->pain_debounce_time = level.time + 2;
		gi.sound(self, CHAN_AUTO, gi.soundindex("world/lightningloop.wav"), 1, ATTN_NORM, 0);
	}
}
void lightning_go (edict_t *self, edict_t *other, edict_t *activator)
{
	lightning_use (self,other);
}

/*QUAKED trigger_lightning (0.3 0.1 0.6) ? MONSTER NOT_PLAYER TRIGGERED ANY
Triggers a lightning bolt
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
-------KEYS--------------------
origin-- Starting point.
target-- Ending point entity.
	 There may be more than one with a given targetname.
delay-- (0-25.5) Sec. duration of lightning.
	 Leave this at zero for a normal strike
materialtype-- 0=blue, 1=red
style-- Width of bolt.  Red rain uses 6.
wait - amount of time until it will become active again (default 10).
*/
void SP_trigger_lightning (edict_t *self)
{
	InitTrigger(self);

	if (!self->wait)
		self->wait = 10;

	self->TriggerActivated = lightning_use;
	self->use = lightning_go;	// This is so a trigger_relay can use it.
}

void quake_quiet(edict_t *self)
{
	gi.sound(self, CHAN_NO_PHS_ADD+CHAN_VOICE,self->moveinfo.sound_end, 1, ATTN_NORM, 0);
	self->nextthink = level.time + FRAMETIME;
	self->think = G_FreeEdict;
}

void quake_use (edict_t *self, edict_t *other)
{
	edict_t *killsound;
	int count,time;

	if (self->touch_debounce_time > level.time)
		return;

	self->touch_debounce_time = level.time + self->wait;

	count = (byte)self->count;
	time = (byte) self->time * 10;

	gi.CreateEffect(self, FX_QUAKE, CEF_BROADCAST, self->s.origin,"bbb",count,time,self->style);

	G_UseTargets(self, self);

	if (self->wait==-1)
	{
		self->touch = NULL;
		self->nextthink = level.time + FRAMETIME;
		self->think = G_FreeEdict;
	}

	// Because nextthink is multi_use for a trigger I have to create a new entity with the sound
	// so I can then kill the sound at the right time
	killsound = G_Spawn();

	gi.sound(killsound, CHAN_NO_PHS_ADD+CHAN_VOICE,self->moveinfo.sound_middle, 1, ATTN_NORM, 0);
	VectorCopy(self->s.origin,killsound->s.origin);
	killsound->moveinfo.sound_end = self->moveinfo.sound_end;
	killsound->nextthink = level.time + self->time;
	killsound->think = quake_quiet;
}

/*QUAKED trigger_quake (0.3 0.1 0.6) ? MONSTER NOT_PLAYER TRIGGERED ANY
Triggers an earth quake
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
--------KEYS---------------------
wait - amount of time until it will become active again (default 10). -1 makes it go away for ever.
count - max number of pixels to shake screen (default 20)
time - duration to the tenth of a second  (range 0 - 12.8) (default 2)
style - direction of shake
1 - SHAKE_LATERAL
2 - SHAKE_VERTICAL
4 - SHAKE_DEPTH
7 - SHAKE_ALL_DIR  (default)
*/
void SP_trigger_quake (edict_t *self)
{
	if (!self->wait)
		self->wait = 10;

	self->moveinfo.sound_middle = gi.soundindex("world/quake.wav");
	self->moveinfo.sound_end = gi.soundindex("world/quakend.wav");

	InitTrigger(self);

	if (!self->count)	// Amount of shake
		self->count = 20;

	if (!self->time)	// Duration
		self->time = 2.0;

	if (!self->style)
		self->style = SHAKE_ALL_DIR;

	self->TriggerActivated = quake_use;
}

void mission_give_use (edict_t *self, edict_t *other)
{
	int				num, i;
	player_state_t	*ps;

	gi.dprintf("TODO: mission changed %s\n", self->message);

	num = atoi(self->message);
	for (i = 1; i <= game.maxclients; i++)
	{
		other = &g_edicts[i];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;

		ps = &other->client->ps;
		if ((ps->mission_num1 != num) && (ps->mission_num2 != num))
		{
			if (!ps->mission_num1)
			{
				ps->mission_num1 = num;
			}
			else
			{
				ps->mission_num2 = num;
			}
			G_CPrintf(other, PRINT_HIGH, GM_NEWOBJ);
		}
	}

	G_UseTargets(self, self);
}

/*QUAKED trigger_mission_give (0.3 0.1 0.6) ? MONSTER NOT_PLAYER TRIGGERED ANY
Gives player(s) the current mission objectives
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
-------KEYS--------------------
message - number of line from strings.txt, put in objectives
wait - amount of time until it will become active again (default 10).
*/
void SP_trigger_mission_give (edict_t *self)
{
	InitTrigger(self);

	if (!self->wait)
		self->wait = 10;

	self->TriggerActivated = mission_give_use;
}

#define MISSION_TAKE1 16
#define MISSION_TAKE2 32

void
mission_take_use(edict_t *self, edict_t *other)
{
	player_state_t		*ps;
	int					i;

	for (i = 1; i <= game.maxclients; i++)
	{
		other = &g_edicts[i];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;

		ps = &other->client->ps;

		if (self->spawnflags & MISSION_TAKE1)
			ps->mission_num1 = 0;

		if (self->spawnflags & MISSION_TAKE2)
			ps->mission_num2 = 0;
	}


	G_UseTargets(self, self);
}

/*QUAKED trigger_mission_take (0.3 0.1 0.6) ? MONSTER NOT_PLAYER TRIGGERED ANY TAKE1  TAKE2
Removes player(s) the current mission objectives
-------SPAWN FLAGS-------------
MONSTER - only a monster will trigger it
NOT_PLAYER -  can't be triggered by player
TRIGGERED - starts trigger deactivated
ANY - anything can activate it
TAKE1 mission statement 1
TAKE2 mission statement 2
-------KEYS--------------------
wait - amount of time until it will become active again (default 10).
*/
void SP_trigger_mission_take (edict_t *self)
{
	InitTrigger(self);

	if (!self->wait)
		self->wait = 10;

	self->TriggerActivated = mission_take_use;
}

void ClipDistance_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	char temp[10];
	cvar_t *r_farclipdist;
	r_farclipdist = gi.cvar("r_farclipdist", FAR_CLIP_DIST, 0);

	// if we aren't a player, forget it
	if (!other->client)
		return;

	if (self->pain_debounce_time < level.time)
	{
		if (r_farclipdist->value == FAR_CLIP_DIST_VAL)
		{
			sprintf(temp, "%f", AVG_VEC3T(self->rrs.scale));
			gi.cvar_set("r_farclipdist", temp);

		}
		else
		{
			gi.cvar_set("r_farclipdist", FAR_CLIP_DIST);
		}
		self->pain_debounce_time = level.time + 0.5;
	}
}

/*QUAKED trigger_farclip (0.5 0.5 0.5) ?
Allows the console var Farclip to be reset - this is a toggle function - if triggered
and far-clip is set to the default, it will be reset to the value passed in. If its the
value passed in, its reset to the default. Be aware that there must be no teleport
destinations within the area that has a reset far-clip.
-------SPAWN FLAGS-------------
-------KEYS--------------------
scale - distance to set far clip to. Default of farclip is 4096.0
*/
void SP_trigger_farclip (edict_t *self)
{
	InitTrigger(self);

	self->touch = ClipDistance_touch;
	self->solid = SOLID_TRIGGER;

}

void trigger_endgame_think(edict_t *self)
{
	gi.AddCommandString("newcoopgame\n");

	G_SetToFree(self);
}

void Touch_endgame(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	cvar_t *sv_loopcoop;

	sv_loopcoop = gi.cvar("sv_loopcoop", "0", 0);

	if (self->count)
		return;

	self->count++;

	// If we aren't a player, forget it.

	if (!other->client)
		return;

	// Not valid on DM play.

	if (deathmatch->value)
		return;

	// Single player - just end, coop - restart if sv_loopcoop is set.

	if (sv_loopcoop->value && coop->value )
	{
		int		i;
		edict_t	*ent;

		for(i=0;i<maxclients->value;i++)
		{
			if ((ent=(&g_edicts[i+1]))->inuse)
				G_CPrintf(ent, PRINT_HIGH, GM_COOP_RESTARTING);
		}

		self->think=trigger_endgame_think;
		self->nextthink=level.time+1.0;
	}
	else
	{
		gi.AddCommandString("endgame\n");

		G_SetToFree(self);
	}
}

void Use_endgame (edict_t *self, edict_t *other, edict_t *activator)
{
	cvar_t *sv_loopcoop;

	sv_loopcoop = gi.cvar("sv_loopcoop", "0", 0);

	if (self->count)
		return;

	self->count++;

	// Not valid on DM play.

	if (deathmatch->value)
		return;

	// Single player - just end, coop - restart if sv_loopcoop is set.

	if (sv_loopcoop->value && coop->value)
	{
		int		i;
		edict_t *ent;

		for(i=0;i<maxclients->value;i++)
		{
			if ((ent=(&g_edicts[i+1]))->inuse)
				G_CPrintf(ent, PRINT_HIGH, GM_COOP_RESTARTING);
		}

		self->think=trigger_endgame_think;
		self->nextthink=level.time+1.0;
	}
	else
	{
		gi.AddCommandString("endgame\n");

		G_SetToFree(self);
	}

}


/*QUAKED trigger_endgame (.5 .5 .5) ?
End game trigger. once used, game over
*/
void
SP_trigger_endgame(edict_t *self)
{
	InitTrigger(self);
	self->touch = Touch_endgame;
	self->solid = SOLID_TRIGGER;
	self->use = Use_endgame;
	self->count=0;
}

//----------------------------------------------------------------------
// Player Push Lever Trigger
//----------------------------------------------------------------------

//void trigger_playerpushlever(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
void trigger_playerpushlever(edict_t *self, edict_t *other)
{
	if (!strcmp(other->classname, "player"))
	{
		other->target = self->target;
	}
}

/*QUAKED trigger_playerpushlever (.5 .5 .5) ?  x1 x2 TRIGGERED
Triggers player to know he is near a lever.
*/
void SP_trigger_PlayerPushLever(edict_t *self)
{
	InitTrigger(self);

	self->TriggerActivated = trigger_playerpushlever;

}

//----------------------------------------------------------------------
// Force Field
//----------------------------------------------------------------------

#define FIELD_FORCE_ONCE		1


void
trigger_push_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t forward,up;

	if (other->health > 0)
	{
		if (other->client)	// A player???
		{
			// don't take falling damage immediately from this
			VectorCopy(other->velocity, other->client->playerinfo.oldvelocity);
			other->client->playerinfo.flags |= PLAYER_FLAG_USE_ENT_POS;
			other->groundentity = NULL;
		}

		AngleVectors(self->s.angles,forward,NULL,up);

		VectorMA(other->velocity,self->speed,forward,other->velocity);
		VectorMA(other->velocity,self->speed,up,other->velocity);

	}

	G_UseTargets(self, self);

	if (self->spawnflags & FIELD_FORCE_ONCE)
	{
		G_FreeEdict(self);
	}
}

void
push_touch_trigger(edict_t *self, edict_t *activator)
{
	trigger_push_touch(self, activator, NULL, NULL);
}

void
TrigPush_Deactivate(edict_t *self, G_Message_t *msg)
{
	self->solid = SOLID_NOT;
	self->touch = NULL;
}

void
TrigPush_Activate(edict_t *self, G_Message_t *msg)
{
	self->solid = SOLID_TRIGGER;
	self->touch = trigger_push_touch;
	gi.linkentity(self);
}

void
TrigPushStaticsInit()
{
	classStatics[CID_TRIG_PUSH].msgReceivers[G_MSG_SUSPEND] = TrigPush_Deactivate;
	classStatics[CID_TRIG_PUSH].msgReceivers[G_MSG_UNSUSPEND] = TrigPush_Activate;
}

/*
 * QUAKED trigger_push (.5 .5 .5) ? FORCE_ONCE
 * Pushes the player
 * "speed"		defaults to 1000
 * angle - the angle to push the player along the X,Y
 * zangle - the up direction to push the player (0 is straight up, 180 is straight down)
 *
 * If targeted, it will toggle on and off when used.
 *
 * FORCE_ONCE - pushes once and then goes away
 */
void
SP_trigger_push(edict_t *self)
{
	if (!self)
	{
		return;
	}

	InitTrigger(self);
	self->solid = SOLID_TRIGGER;
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_TRIG_PUSH;

	if (!self->speed)
	{
		self->speed = 500;
	}

	self->s.angles[2] = st.zangle;

	// Can't really use the normal trigger setup cause it doesn't update velocity often enough
	self->touch = trigger_push_touch;
	self->TriggerActivated = push_touch_trigger;
}

/*
 * ==============================================================================
 *
 * trigger_hurt
 *
 * ==============================================================================
 */

/*
 * QUAKED trigger_hurt (.5 .5 .5) ? START_OFF TOGGLE SILENT NO_PROTECTION SLOW
 *
 * Any entity that touches this will be hurt.
 *
 * It does dmg points of damage each server frame
 *
 * SILENT			supresses playing the sound
 * SLOW			changes the damage rate to once per second
 * NO_PROTECTION	*nothing* stops the damage
 *
 * "dmg"			default 5 (whole numbers only)
 *
 */
void
hurt_use(edict_t *self, edict_t *other /* unused */,
		edict_t *activator /* unused */)
{
	if (!self)
	{
		return;
	}

	if (self->solid == SOLID_NOT)
	{
		int	i, num;
		edict_t	*touch[MAX_EDICTS], *hurtme;

		self->solid = SOLID_TRIGGER;
		num = gi.BoxEdicts(self->absmin, self->absmax,
				touch, MAX_EDICTS, AREA_SOLID);

		/* Check for idle monsters in
		   trigger hurt */
		for (i = 0 ; i < num ; i++)
		{
			hurtme = touch[i];
			hurt_touch (self, hurtme, NULL, NULL);
		}
	}
	else
	{
		self->solid = SOLID_NOT;
	}

	gi.linkentity(self);

	if (!(self->spawnflags & 2))
	{
		self->use = NULL;
	}
}

void
hurt_touch(edict_t *self, edict_t *other, cplane_t *plane /* unused */,
		csurface_t *surf /* unused */)
{
	int dflags;

	if (!self || !other)
	{
		return;
	}

	if (!other->takedamage)
	{
		return;
	}

	if (self->timestamp > level.time)
	{
		return;
	}

	if (self->spawnflags & 16)
	{
		self->timestamp = level.time + 1;
	}
	else
	{
		self->timestamp = level.time + FRAMETIME;
	}

	if (!(self->spawnflags & 4))
	{
		if ((level.framenum % 10) == 0)
		{
			gi.sound(other, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
		}
	}

	if (self->spawnflags & 8)
	{
		dflags = DAMAGE_NO_PROTECTION;
	}
	else
	{
		dflags = 0;
	}

	T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin,
			self->dmg, self->dmg, dflags, MOD_TRIGGER_HURT);
}

void
SP_trigger_hurt(edict_t *self)
{
	if (!self)
	{
		return;
	}

	InitTrigger(self);

	self->noise_index = gi.soundindex("world/electro.wav");
	self->touch = hurt_touch;

	if (!self->dmg)
	{
		self->dmg = 5;
	}

	if (self->spawnflags & 1)
	{
		self->solid = SOLID_NOT;
	}
	else
	{
		self->solid = SOLID_TRIGGER;
	}

	if (self->spawnflags & 2)
	{
		self->use = hurt_use;
	}

	gi.linkentity(self);
}

/*
 * ==============================================================================
 *
 * trigger_gravity
 *
 * ==============================================================================
 */

void
trigger_gravity_use(edict_t *self, edict_t *other /* unused */, edict_t *activator /* unused */)
{
	if (!self)
	{
		return;
	}

	if (self->solid == SOLID_NOT)
	{
		self->solid = SOLID_TRIGGER;
	}
	else
	{
		self->solid = SOLID_NOT;
	}

	gi.linkentity(self);
}

/*
 * QUAKED trigger_gravity (.5 .5 .5) ?
 * Changes the touching entites gravity to
 * the value of "gravity".  1.0 is standard
 * gravity for the level.
 */
void
trigger_gravity_touch(edict_t *self, edict_t *other, cplane_t *plane /* unused */,
		csurface_t *surf /* unused */)
{
	if (!self || !other)
	{
		return;
	}

	other->gravity = self->gravity;
	G_UseTargets(self, self);

}

/*
 * QUAKED trigger_gravity (.5 .5 .5) ? TOGGLE START_OFF
 * Changes the touching entites gravity to
 * the value of "gravity".  1.0 is standard
 * gravity for the level.
 *
 * TOGGLE - trigger_gravity can be turned on and off
 * START_OFF - trigger_gravity starts turned off (implies TOGGLE)
 */
void
SP_trigger_gravity(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (st.gravity == 0)
	{
		gi.dprintf("trigger_gravity without gravity set at %s\n",
				vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	InitField(self);

	self->gravity = (int)strtol(st.gravity, (char **)NULL, 10);

	self->touch = trigger_gravity_touch;
}

/*
 * ==============================================================================
 *
 * trigger_monsterjump
 *
 * ==============================================================================
 */

/*
 * QUAKED trigger_monsterjump (.5 .5 .5) ?
 * Walking monsters that touch this will jump in the direction of the trigger's angle
 *
 * "speed"  default to 200, the speed thrown forward
 * "height" default to 200, the speed thrown upwards
 */

void
trigger_monsterjump_touch(edict_t *self, edict_t *other, cplane_t *plane /* unused */,
		csurface_t *surf /* unused */)
{
	if (!self || !other)
	{
		return;
	}

	if (other->flags & (FL_FLY | FL_SWIM))
	{
		return;
	}

	if (other->svflags & SVF_DEADMONSTER)
	{
		return;
	}

	if (!(other->svflags & SVF_MONSTER))
	{
		return;
	}

	VectorMA(other->velocity, self->speed, self->movedir, other->velocity);
	other->velocity[2] += self->accel;

	other->groundentity = NULL;
}

void
SP_trigger_monsterjump(edict_t *self)
{
	if (!self)
	{
		return;
	}

	if (self->s.angles[YAW] == 0)
	{
		self->s.angles[YAW] = 360;
	}

	InitField(self);

	if (!self->speed)
	{
		self->speed = 200;
	}

	if (!st.height)
	{
		self->accel = 200;
	}
	else
		self->accel = st.height;

	self->touch = trigger_monsterjump_touch;
}

/* QUAKED trigger_flashlight (.5 .5 .5) ?
 * Players moving against this trigger will have their flashlight turned on or off.
 * "style" default to 0, set to 1 to always turn flashlight on, 2 to always turn off,
 *      otherwise "angles" are used to control on/off state
 */

#define SPAWNFLAG_FLASHLIGHT_CLIPPED 1

void
trigger_flashlight_touch(edict_t *self, edict_t *other, cplane_t *plane /* unused */,
		csurface_t *surf /* unused */)
{
	if (!other->client)
	{
		return;
	}

	if (self->style == 1)
	{
		P_ToggleFlashlight(other, true);
	}
	else if (self->style == 2)
	{
		P_ToggleFlashlight(other, false);
	}
	else if (VectorLength(other->velocity) > 6)
	{
		vec3_t forward;

		VectorNormalize2(other->velocity, forward);

		P_ToggleFlashlight(other, _DotProduct(forward, self->movedir) > 0);
	}
}

void
SP_trigger_flashlight(edict_t *self)
{
	if (self->s.angles[YAW] == 0)
	{
		self->s.angles[YAW] = 360;
	}

	InitTrigger(self);
	self->touch = trigger_flashlight_touch;
	self->movedir[2] = (float) st.height;

	gi.linkentity(self);
}

/*
 * QUAKED trigger_fog (.5 .5 .5) ? AFFECT_FOG AFFECT_HEIGHTFOG INSTANTANEOUS FORCE BLEND
 *
 * Players moving against this trigger will have their fog settings changed.
 * Fog/heightfog will be adjusted if the spawnflags are set. Instantaneous
 * ignores any delays. Force causes it to ignore movement dir and always use
 * the "on" values. Blend causes it to change towards how far you are into the trigger
 * with respect to angles.
 * "target" can target an info_notnull to pull the keys below from.
 * "delay" default to 0.5; time in seconds a change in fog will occur over
 * "wait" default to 0.0; time in seconds before a re-trigger can be executed
 *
 * "fog_density"; density value of fog, 0-1
 * "fog_color"; color value of fog, 3d vector with values between 0-1 (r g b)
 * "fog_density_off"; transition density value of fog, 0-1
 * "fog_color_off"; transition color value of fog, 3d vector with values between 0-1 (r g b)
 * "fog_sky_factor"; sky factor value of fog, 0-1
 * "fog_sky_factor_off"; transition sky factor value of fog, 0-1
 *
 * "heightfog_falloff"; falloff value of heightfog, 0-1
 * "heightfog_density"; density value of heightfog, 0-1
 * "heightfog_start_color"; the start color for the fog (r g b, 0-1)
 * "heightfog_start_dist"; the start distance for the fog (units)
 * "heightfog_end_color"; the start color for the fog (r g b, 0-1)
 * "heightfog_end_dist"; the end distance for the fog (units)
 *
 * "heightfog_falloff_off"; transition falloff value of heightfog, 0-1
 * "heightfog_density_off"; transition density value of heightfog, 0-1
 * "heightfog_start_color_off"; transition the start color for the fog (r g b, 0-1)
 * "heightfog_start_dist_off"; transition the start distance for the fog (units)
 * "heightfog_end_color_off"; transition the start color for the fog (r g b, 0-1)
 * "heightfog_end_dist_off"; transition the end distance for the fog (units)
 */

#define SPAWNFLAG_FOG_AFFECT_FOG 1
#define SPAWNFLAG_FOG_AFFECT_HEIGHTFOG 2
#define SPAWNFLAG_FOG_INSTANTANEOUS 4
#define SPAWNFLAG_FOG_FORCE 8
#define SPAWNFLAG_FOG_BLEND 16

static vec_t
lerp(vec_t from, vec_t to, float t)
{
	return (to * t) + (from * (1.f - t));
}

void
trigger_fog_touch(edict_t *self, edict_t *other, cplane_t *plane /* unused */, csurface_t *surf /* unused */)
{
	edict_t *fog_value_storage;

	if (!other->client)
	{
		return;
	}

	if (self->timestamp > level.time)
	{
		return;
	}

	self->timestamp = level.time + self->wait;

	fog_value_storage = self;

	if (self->movetarget)
	{
		fog_value_storage = self->movetarget;
	}

	if (self->spawnflags & SPAWNFLAG_FOG_INSTANTANEOUS)
	{
		other->client->pers.fog_transition_time = 0;
	}
	else
	{
		other->client->pers.fog_transition_time = fog_value_storage->delay;
	}

	if (self->spawnflags & SPAWNFLAG_FOG_BLEND)
	{
		vec3_t center, half_size, start, end, player_dist,
			player_dist_diff, start_end;
		float dist;
		int i;

		VectorAdd(self->absmin, self->absmax, center);
		VectorScale(center, 0.5, center);
		VectorAdd(self->size, other->size, half_size);
		VectorScale(half_size, 0.5, half_size);

		VectorScale(self->movedir, -1.0, start);
		VectorScale(self->movedir, 1.0, end);
		for (i = 0; i < 3; i++)
		{
			start[i] *= half_size[i];
			end[i] *= half_size[i];
		}

		VectorSubtract(other->s.origin, center, player_dist);
		for (i = 0; i < 3; i++)
		{
			player_dist[i] *= fabs(self->movedir[i]);
		}

		VectorSubtract(player_dist, start, player_dist_diff);
		dist = VectorLength(player_dist_diff);
		VectorSubtract(start, end, start_end);
		dist /= VectorLength(start_end);
		dist = Q_clamp(dist, 0.f, 1.f);

		if (self->spawnflags & SPAWNFLAG_FOG_AFFECT_FOG)
		{
			other->client->pers.wanted_fog[0] = lerp(
				fog_value_storage->fog.density_off, fog_value_storage->fog.density, dist);
			other->client->pers.wanted_fog[1] = lerp(
				fog_value_storage->fog.color_off[0], fog_value_storage->fog.color[0], dist);
			other->client->pers.wanted_fog[2] = lerp(
				fog_value_storage->fog.color_off[1], fog_value_storage->fog.color[1], dist);
			other->client->pers.wanted_fog[3] = lerp(
				fog_value_storage->fog.color_off[2], fog_value_storage->fog.color[2], dist);
			other->client->pers.wanted_fog[4] = lerp(
				fog_value_storage->fog.sky_factor_off, fog_value_storage->fog.sky_factor, dist);
		}

		if (self->spawnflags & SPAWNFLAG_FOG_AFFECT_HEIGHTFOG)
		{
			VectorLerp(
				fog_value_storage->heightfog.start_color_off,
				fog_value_storage->heightfog.start_color,
				dist,
				other->client->pers.wanted_heightfog.start
			);

			VectorLerp(
				fog_value_storage->heightfog.end_color_off,
				fog_value_storage->heightfog.end_color,
				dist,
				other->client->pers.wanted_heightfog.end
			);

			other->client->pers.wanted_heightfog.start[3] = lerp(
				fog_value_storage->heightfog.start_dist_off,
				fog_value_storage->heightfog.start_dist, dist);
			other->client->pers.wanted_heightfog.end[3] = lerp(
				fog_value_storage->heightfog.end_dist_off,
				fog_value_storage->heightfog.end_dist, dist);
			other->client->pers.wanted_heightfog.falloff = lerp(
				fog_value_storage->heightfog.falloff_off,
				fog_value_storage->heightfog.falloff, dist);
			other->client->pers.wanted_heightfog.density = lerp(
				fog_value_storage->heightfog.density_off,
				fog_value_storage->heightfog.density, dist);
		}

		return;
	}

	bool use_on = true;

	if (!(self->spawnflags & SPAWNFLAG_FOG_FORCE))
	{
		float len;
		vec3_t forward;

		VectorCopy(other->velocity, forward);
		len = VectorNormalize(forward);

		// not moving enough to trip; this is so we don't trip
		// the wrong direction when on an elevator, etc.
		if (len <= 0.0001f)
		{
			return;
		}

		use_on = _DotProduct(forward, self->movedir) > 0;
	}

	if (self->spawnflags & SPAWNFLAG_FOG_AFFECT_FOG)
	{
		if (use_on)
		{
			int i;
			other->client->pers.wanted_fog[0] = fog_value_storage->fog.density;
			other->client->pers.wanted_fog[4] = fog_value_storage->fog.sky_factor;
			for (i = 0; i < 3; i++)
			{
				other->client->pers.wanted_fog[i + 1] = fog_value_storage->fog.color[i];
			}
		}
		else
		{
			int i;

			other->client->pers.wanted_fog[0] = fog_value_storage->fog.density_off;
			other->client->pers.wanted_fog[4] = fog_value_storage->fog.sky_factor_off;
			for (i = 0; i < 3; i++)
			{
				other->client->pers.wanted_fog[i + 1] = fog_value_storage->fog.color_off[i];
			}
		}
	}

	if (self->spawnflags & SPAWNFLAG_FOG_AFFECT_HEIGHTFOG)
	{
		if (use_on)
		{
			/* start */
			VectorCopy(
				fog_value_storage->heightfog.start_color,
				other->client->pers.wanted_heightfog.start
			);
			other->client->pers.wanted_heightfog.start[3] =
				fog_value_storage->heightfog.start_dist;
			/* end */
			VectorCopy(
				fog_value_storage->heightfog.end_color,
				other->client->pers.wanted_heightfog.end
			);
			other->client->pers.wanted_heightfog.end[3] =
				fog_value_storage->heightfog.end_dist;

			/* falloff, density */
			other->client->pers.wanted_heightfog.falloff =
				fog_value_storage->heightfog.falloff;
			other->client->pers.wanted_heightfog.density =
				fog_value_storage->heightfog.density;
		}
		else
		{
			/* start */
			VectorCopy(
				fog_value_storage->heightfog.start_color_off,
				other->client->pers.wanted_heightfog.start
			);
			other->client->pers.wanted_heightfog.start[3] =
				fog_value_storage->heightfog.start_dist_off;
			/* end */
			VectorCopy(
				fog_value_storage->heightfog.end_color_off,
				other->client->pers.wanted_heightfog.end
			);
			other->client->pers.wanted_heightfog.end[3] =
				fog_value_storage->heightfog.end_dist_off;

			/* falloff, density */
			other->client->pers.wanted_heightfog.falloff =
				fog_value_storage->heightfog.falloff_off;
			other->client->pers.wanted_heightfog.density =
				fog_value_storage->heightfog.density_off;
		}
	}
}

void
SP_trigger_fog(edict_t *self)
{
	if (self->s.angles[YAW] == 0)
	{
		self->s.angles[YAW] = 360;
	}

	InitTrigger(self);

	if (!(self->spawnflags & (SPAWNFLAG_FOG_AFFECT_FOG | SPAWNFLAG_FOG_AFFECT_HEIGHTFOG)))
	{
		Com_Printf("WARNING: %s with no fog spawnflags set\n", self->classname);
	}

	if (self->target)
	{
		self->movetarget = G_PickTarget(self->target);

		if (self->movetarget)
		{
			if (!self->movetarget->delay)
			{
				self->movetarget->delay = 0.5f;
			}
		}
	}

	if (!self->delay)
	{
		self->delay = 0.5f;
	}

	self->touch = trigger_fog_touch;
}

/*
 * QUAKED choose_cdtrack (.5 .5 .5) ?
 * Heretic 2: Sets CD track
 *
 * Variable sized repeatable trigger which chooses a CD track.
 * ------KEYS-----------
 * style: CD Track Id
 * NO_LOOP - allows you to set the track to play not to loop
 */
void
choose_cdtrack_touch(edict_t *self, edict_t *other, cplane_t *plane /* unused */,
		csurface_t *surf /* unused */)
{
	if (!self)
	{
		return;
	}

	// if we aren't a player, forget it
	if (!other->client)
	{
		return;
	}

	gi.configstring(CS_CDTRACK, va("%i", self->style));

	/* kill this trigger */
	G_SetToFree(self);
}

void
choose_cdtrack_use(edict_t *self, edict_t *other, edict_t *activator)
{
	choose_cdtrack_touch(self, other, NULL, NULL);
}

void
SP_choose_cdtrack(edict_t *self)
{
	if (!self)
	{
		return;
	}

	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_TRIGGER;

	self->use = choose_cdtrack_use;

	if (self->spawnflags & 1)
		self->spawnflags = false;
	else
		self->spawnflags = true;

	if (!self->wait)
	{
		self->wait = 0.2;
	}

	/* Triggers still use the touch function even with the new physics */
	self->touch = choose_cdtrack_touch;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_TRIGGER;

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}
