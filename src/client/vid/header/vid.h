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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 * =======================================================================
 *
 * API between client and renderer.
 *
 * =======================================================================
 */

#ifndef CLIENT_VID_H
#define CLIENT_VID_H
// vid.h -- video driver defs

#define	MIN_GAMMA	0.1			// These also need to be defined in gl_local.h
#define MAX_GAMMA	4.0

#define DEF_WIDTH	640
#define DEF_HEIGHT	480

typedef struct vrect_s
{
	int				x,y,width,height;
} vrect_t;

typedef struct
{
	int			 	width;
	int				height;
	byte			*pixels;
} viddef_t;

extern	viddef_t	viddef;				// global video state

// Video module initialisation etc
void	VID_Init (void);
void	VID_Shutdown (void);
void	VID_CheckChanges (void);

void	VID_MenuInit( void );
void	VID_PreMenuInit( void );
void	 VID_MenuDraw();
const char	* VID_MenuKey( int );

// end
#endif
