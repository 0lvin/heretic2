/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (C) 2011 Yamagi Burmeister
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
 * Game fields to be saved.
 *
 * =======================================================================
 */

{"classname", FOFS(classname), F_LRAWSTRING},
{"model", FOFS(model), F_LRAWSTRING},
{"spawnflags", FOFS(spawnflags), F_INT},
{"speed", FOFS(speed), F_FLOAT},
{"accel", FOFS(accel), F_FLOAT},
{"decel", FOFS(decel), F_FLOAT},
{"target", FOFS(target), F_LRAWSTRING},
{"targetname", FOFS(targetname), F_LRAWSTRING},
{"pathtarget", FOFS(pathtarget), F_LRAWSTRING},
{"deathtarget", FOFS(deathtarget), F_LRAWSTRING},
{"killtarget", FOFS(killtarget), F_LRAWSTRING},
{"combattarget", FOFS(combattarget), F_LRAWSTRING},
{"message", FOFS(message), F_LSTRING},
{"team", FOFS(team), F_LSTRING},
{"wait", FOFS(wait), F_FLOAT},
{"delay", FOFS(delay), F_FLOAT},
{"random", FOFS(random), F_FLOAT},
{"move_origin", FOFS(move_origin), F_VECTOR},
{"move_angles", FOFS(move_angles), F_VECTOR},
{"style", FOFS(style), F_INT},
{"count", FOFS(count), F_INT},
{"health", FOFS(health), F_INT},
{"sounds", FOFS(sounds), F_INT},
{"light", 0, F_IGNORE},
{"dmg", FOFS(dmg), F_INT},
{"mass", FOFS(mass), F_INT},
{"volume", FOFS(volume), F_FLOAT},
{"attenuation", FOFS(attenuation), F_FLOAT},
{"map", FOFS(map), F_LRAWSTRING},
{"origin", FOFS(s.origin), F_VECTOR},
{"angles", FOFS(s.angles), F_VECTOR},
{"angle", FOFS(s.angles), F_ANGLEHACK},
{"rgb", STOFS(rgba), F_RGBA, FFL_SPAWNTEMP},
{"rgba", STOFS(rgba), F_RGBA, FFL_SPAWNTEMP},
{"scale", STOFS(scale), F_VECTOR, FFL_SPAWNTEMP},
{"radius", STOFS(radius), F_FLOAT, FFL_SPAWNTEMP},
{"fade_start_dist", STOFS(fade_start_dist), F_FLOAT, FFL_SPAWNTEMP},
{"fade_end_dist", STOFS(fade_end_dist), F_FLOAT, FFL_SPAWNTEMP},
{"image", STOFS(image), F_LRAWSTRING, FFL_SPAWNTEMP},
{"goalentity", FOFS(goalentity), F_EDICT, FFL_NOSPAWN},
{"movetarget", FOFS(movetarget), F_EDICT, FFL_NOSPAWN},
{"enemy", FOFS(enemy), F_EDICT, FFL_NOSPAWN},
{"oldenemy", FOFS(oldenemy), F_EDICT, FFL_NOSPAWN},
{"activator", FOFS(activator), F_EDICT, FFL_NOSPAWN},
{"groundentity", FOFS(groundentity), F_EDICT, FFL_NOSPAWN},
{"teamchain", FOFS(teamchain), F_EDICT, FFL_NOSPAWN},
{"teammaster", FOFS(teammaster), F_EDICT, FFL_NOSPAWN},
{"owner", FOFS(owner), F_EDICT, FFL_NOSPAWN},
{"mynoise", FOFS(mynoise), F_EDICT, FFL_NOSPAWN},
{"mynoise2", FOFS(mynoise2), F_EDICT, FFL_NOSPAWN},
{"target_ent", FOFS(target_ent), F_EDICT, FFL_NOSPAWN},
{"chain", FOFS(chain), F_EDICT, FFL_NOSPAWN},
{"prethink", FOFS(prethink), F_FUNCTION, FFL_NOSPAWN},
{"think", FOFS(think), F_FUNCTION, FFL_NOSPAWN},
{"blocked", FOFS(blocked), F_FUNCTION, FFL_NOSPAWN},
{"touch", FOFS(touch), F_FUNCTION, FFL_NOSPAWN},
{"use", FOFS(use), F_FUNCTION, FFL_NOSPAWN},
{"pain", FOFS(pain), F_FUNCTION, FFL_NOSPAWN},
{"die", FOFS(die), F_FUNCTION, FFL_NOSPAWN},
{"stand", FOFS(monsterinfo.stand), F_FUNCTION, FFL_NOSPAWN},
{"idle", FOFS(monsterinfo.idle), F_FUNCTION, FFL_NOSPAWN},
{"search", FOFS(monsterinfo.search), F_FUNCTION, FFL_NOSPAWN},
{"walk", FOFS(monsterinfo.walk), F_FUNCTION, FFL_NOSPAWN},
{"run", FOFS(monsterinfo.run), F_FUNCTION, FFL_NOSPAWN},
{"dodge", FOFS(monsterinfo.dodge), F_FUNCTION, FFL_NOSPAWN},
{"attack", FOFS(monsterinfo.attack), F_FUNCTION, FFL_NOSPAWN},
{"melee", FOFS(monsterinfo.melee), F_FUNCTION, FFL_NOSPAWN},
{"sight", FOFS(monsterinfo.sight), F_FUNCTION, FFL_NOSPAWN},
{"checkattack", FOFS(monsterinfo.checkattack), F_FUNCTION, FFL_NOSPAWN},
{"currentmove", FOFS(monsterinfo.currentmove), F_MMOVE, FFL_NOSPAWN},
{"endfunc", FOFS(moveinfo.endfunc), F_FUNCTION, FFL_NOSPAWN},
{"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
{"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
{"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
{"noise", STOFS(noise), F_LRAWSTRING, FFL_SPAWNTEMP},
{"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
{"item", STOFS(item), F_LRAWSTRING, FFL_SPAWNTEMP},
{"item", FOFS(item), F_ITEM},
{"gravity", STOFS(gravity), F_LRAWSTRING, FFL_SPAWNTEMP},
{"sky", STOFS(sky), F_LRAWSTRING, FFL_SPAWNTEMP},
{"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
{"skyautorotate", STOFS(skyautorotate), F_INT, FFL_SPAWNTEMP},
{"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
{"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
{"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
{"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
{"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},
{"music", STOFS(music), F_LRAWSTRING, FFL_SPAWNTEMP},
{"nextmap", STOFS(nextmap), F_LRAWSTRING, FFL_SPAWNTEMP},
{"skinnum", FOFS(s.skinnum), F_INT},
{"time", FOFS(time), F_FLOAT},
{"text_msg", FOFS(text_msg), F_LSTRING},
{"jumptarget", FOFS(jumptarget), F_LSTRING},
{"scripttarget", FOFS(scripttarget), F_LSTRING},
{"materialtype", FOFS(materialtype), F_INT},
{"color", FOFS(s.color), F_RGBA},
{"frame", FOFS(s.frame), F_INT},
{"mintel", FOFS(mintel), F_INT},
{"melee_range", FOFS(melee_range), F_FLOAT},
{"missile_range", FOFS(missile_range), F_FLOAT},
{"min_missile_range", FOFS(min_missile_range), F_FLOAT},
{"bypass_missile_chance", FOFS(bypass_missile_chance), F_INT},
{"jump_chance", FOFS(jump_chance), F_INT},
{"wakeup_distance", FOFS(wakeup_distance), F_FLOAT},
{"c_mode", FOFS(monsterinfo.c_mode), F_INT, F_INT},
{"homebuoy", FOFS(homebuoy), F_LSTRING},
{"wakeup_target", FOFS(wakeup_target), F_LSTRING},
{"pain_target", FOFS(pain_target), F_LSTRING},

// temp spawn vars -- only valid when the spawn function is called

{"rotate", STOFS(rotate), F_INT, FFL_SPAWNTEMP},
{"target2", FOFS(target2), F_LSTRING},
{"pathtargetname",  FOFS(pathtargetname), F_LSTRING},
{"zangle", STOFS(zangle), F_FLOAT, FFL_SPAWNTEMP},
{"file", STOFS(file), F_LSTRING, FFL_SPAWNTEMP},
{"radius", STOFS(radius), F_INT, FFL_SPAWNTEMP},
{"offensive", STOFS(offensive), F_INT, FFL_SPAWNTEMP},
{"defensive", STOFS(defensive), F_INT, FFL_SPAWNTEMP},
{"spawnflags2", STOFS(spawnflags2), F_INT, FFL_SPAWNTEMP},
{"cooptimeout", STOFS(cooptimeout), F_INT, FFL_SPAWNTEMP},

{"script", STOFS(script), F_LSTRING, FFL_SPAWNTEMP},
{"parm1", STOFS(parms[0]), F_LSTRING, FFL_SPAWNTEMP},
{"parm2", STOFS(parms[1]), F_LSTRING, FFL_SPAWNTEMP},
{"parm3", STOFS(parms[2]), F_LSTRING, FFL_SPAWNTEMP},
{"parm4", STOFS(parms[3]), F_LSTRING, FFL_SPAWNTEMP},
{"parm5", STOFS(parms[4]), F_LSTRING, FFL_SPAWNTEMP},
{"parm6", STOFS(parms[5]), F_LSTRING, FFL_SPAWNTEMP},
{"parm7", STOFS(parms[6]), F_LSTRING, FFL_SPAWNTEMP},
{"parm8", STOFS(parms[7]), F_LSTRING, FFL_SPAWNTEMP},
{"parm9", STOFS(parms[8]), F_LSTRING, FFL_SPAWNTEMP},
{"parm10", STOFS(parms[9]), F_LSTRING, FFL_SPAWNTEMP},
{0, 0, 0, 0}
