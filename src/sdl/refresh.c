#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <SDL.h>
#include <GL/gl.h>

#include "../refresh/header/local.h"
#include "../unix/header/glwindow.h"

/* The window icon */
#include "icon/q2icon.xbm"

SDL_Surface		*surface;
glwstate_t		glw_state;
static cvar_t	*use_stencil;
qboolean		have_stencil = false;

/*
 * Initialzes the SDL OpenGL context
 */
int GLimp_Init(void)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
    {
		char driverName[ 64 ];

        if (SDL_Init(SDL_INIT_VIDEO) == -1)
        {
			ri.Con_Printf( PRINT_ALL, "\nSDL_Init( SDL_INIT_VIDEO ) FAILED (%s)\n", SDL_GetError());
            return false;
        }
		SDL_VideoDriverName( driverName, sizeof( driverName ) - 1 );
        ri.Con_Printf( PRINT_ALL, "\nSDL using driver \"%s\"\n", driverName );
	}

	return true;
}

static void SetSDLIcon()
{
	SDL_Surface *icon;
	SDL_Color	color;
	Uint8		*ptr;
	int			i;
	int			mask;

	icon = SDL_CreateRGBSurface(SDL_SWSURFACE, q2icon_width, q2icon_height, 8, 0, 0, 0, 0);

	if (icon == NULL)
	{
		return; 
	}
	
	SDL_SetColorKey(icon, SDL_SRCCOLORKEY, 0);

	color.r = 255;
	color.g = 255;
	color.b = 255;
	
	SDL_SetColors(icon, &color, 0, 1);
	
	color.r = 0;
	color.g = 16;
	color.b = 0;
	
	SDL_SetColors(icon, &color, 1, 1);

	ptr = (Uint8 *)icon->pixels;
	
	for (i = 0; i < sizeof(q2icon_bits); i++) 
	{
		for (mask = 1; mask != 0x100; mask <<= 1) {
			*ptr = (q2icon_bits[i] & mask) ? 1 : 0;
			ptr++;
		}		
	}

	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);
}

void
UpdateHardwareGamma(void)
{
	float gamma;

	gamma = (1.3 - vid_gamma->value + 1);

	if (gamma < 1)
	{
		gamma = 1;
	}
	
	SDL_SetGamma(gamma, gamma, gamma );
}

static qboolean GLimp_InitGraphics( qboolean fullscreen )
{
	int flags;
	int stencil_bits;
	
	if (surface && (surface->w == vid.width) && (surface->h == vid.height)) 
	{
		/* Are we running fullscreen? */
		int isfullscreen = (surface->flags & SDL_FULLSCREEN) ? 1 : 0;

		/* We should, but we don't */
		if (fullscreen != isfullscreen)
		{
			SDL_WM_ToggleFullScreen(surface);
		}

		/* Do we now? */
		isfullscreen = (surface->flags & SDL_FULLSCREEN) ? 1 : 0;

		if (fullscreen == isfullscreen)
		{
			return true;
		}
	}
	
	/* Is the surface used? */
	if (surface)
	{
		SDL_FreeSurface(surface);
	}

	/* Create the window */
	ri.Vid_NewWindow (vid.width, vid.height);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	
	/* Initiate the flags */
	flags = SDL_OPENGL;

	if (fullscreen)
	{
		flags |= SDL_FULLSCREEN;
	}
	
	/* Set the icon */
	SetSDLIcon();
	
	if ((surface = SDL_SetVideoMode(vid.width, vid.height, 0, flags)) == NULL) 
	{
		Sys_Error("(SDLGL) SDL SetVideoMode failed: %s\n", SDL_GetError());
		return false;
	}

	/* Initialize the stencil buffer */
	if (!SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stencil_bits)) 
	{
		ri.Con_Printf(PRINT_ALL, "I: got %d bits of stencil\n", stencil_bits);
		
		if (stencil_bits >= 1) 
		{
			have_stencil = true;
		}
	}

	/* Initialize hardware gamma */
	gl_state.hwgamma = true;
	vid_gamma->modified = true;
	ri.Con_Printf(PRINT_ALL, "Using hardware gamma\n");
	ri.Con_Printf(PRINT_ALL, "If this doesn't work your X11 driver is broken!\n");

	/* Window title */
	SDL_WM_SetCaption("Yamagi Quake II", "Yamagi Quake II");

	/* No cursor */
	SDL_ShowCursor(0);

	return true;
}

void GLimp_EndFrame (void)
{
	SDL_GL_SwapBuffers();
}


int GLimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen )
{
	ri.Con_Printf (PRINT_ALL, "setting mode %d:", mode );
	
	/* mode -1 is not in the vid mode table - so we keep the values in pwidth 
	   and pheight and don't even try to look up the mode info */
	if ( mode != -1 && !ri.Vid_GetModeInfo( pwidth, pheight, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	ri.Con_Printf( PRINT_ALL, " %d %d\n", *pwidth, *pheight);

	if ( !GLimp_InitGraphics( fullscreen ) ) 
	{
		return rserr_invalid_mode;
	}

	return rserr_ok;
}

void GLimp_Shutdown( void )
{
	if (surface)
	{
		SDL_FreeSurface(surface);
	}

	surface = NULL;
	
	if (SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_VIDEO)
	{
		SDL_Quit();
	}
	else
	{
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}
	
	gl_state.hwgamma = false;
}

