/*
 * mdx.c -- mdx model loader
 * last modification: aug. 14, 2007
 *
 * Copyright (c) 2005-2007 David HENRY
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <SDL.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "rendergl.h"

SDL_Window* sdl_window = NULL;
SDL_GLContext gl_context;

static void
initGL(int w, int h)
{
	GLfloat lightpos[] = { 5.0f, 10.0f, 0.0f, 1.0f };

	glViewport (0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	//gluPerspective (45.0, w/(GLdouble)h, 0.1, 1000.0);

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	/* Initialize OpenGL context */
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
}

static void
view_render()
{
	static int n = 0; /* The current frame */
	static float interp = 0.0;
	static double curent_time = 0;
	static double last_time = 0;

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity ();

	last_time = curent_time;
	curent_time = (double)SDL_GetTicks() / 100.0;

	/* Animate model from frames 0 to num_frames-1 */
	interp += 10 * (curent_time - last_time);
	Animate(0, mem_mod->num_frames - 1, &n, &interp);

	//Clear color buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glLoadIdentity ();

	glTranslatef(
		0.0f, /* x */
		0.0f, /* y */
		0.0f /* z */
	);

	glRotatef (xrotate, 1.0, 0.0, 0.0);
	glRotatef (yrotate, 0.0, 1.0, 0.0);
	glRotatef (zrotate, 0.0, 0.0, 1.0);
/*
	glRotatef(0.4f,0.0f,1.0f,0.0f);	// Rotate The cube around the Y axis
	glRotatef(0.2f,1.0f,1.0f,1.0f);
	glColor3f(0.0f,1.0f,0.0f);
*/
	/* Draw the model */
	//RenderFrameItp(n, interp, mem_mod);

#if 0
	glTranslatef(
		0.0f, /* x */
		0.0f, /* y */
		zdist /* z */
	);
	glRotatef (xrotate, 1.0, 0.0, 0.0);
	glRotatef (yrotate, 0.0, 1.0, 0.0);
	glRotatef (zrotate, 0.0, 0.0, 1.0);

	/* Draw the model */
	RenderFrameItp(n, interp, mem_mod);
#else
	glBegin(GL_QUADS);
	glVertex3f(-0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glEnd();
#endif
}

static void
view_close()
{
	//Disable text input
	SDL_StopTextInput();

	//Destroy window
	SDL_DestroyWindow(sdl_window);
	sdl_window = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

static void
view_init()
{
	GLboolean success = GL_TRUE;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	else
	{
		sdl_window = SDL_CreateWindow(
			"MD2/MDL/MD5 Model",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			800, 600,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
		if( sdl_window == NULL )
		{
			printf("Window could not be created! SDL Error: %s\n",
				SDL_GetError());
			exit(EXIT_FAILURE);
		}
		else
		{
			gl_context = SDL_GL_CreateContext( sdl_window );
			if( gl_context == NULL )
			{
				printf("OpenGL context could not be created! SDL Error: %s\n",
					SDL_GetError());
				exit(EXIT_FAILURE);
			}
			else
			{
				if(SDL_GL_SetSwapInterval( 1 ) < 0)
				{
					printf("Warning: Unable to set VSync! SDL Error: %s\n",
						SDL_GetError());
				}

				initGL(800, 600);
			}
		}
	}
}

int
main(int argc, char *argv[])
{
	SDL_Event e;

	if (argc < 2)
	{
		R_Printf(PRINT_ALL, "usage: %s <filename.md2> [<texture.png>]\n", argv[0]);
		return 0;
	}

	// quake swap prepere
	Swap_Init();

	view_init();
	SDL_StartTextInput();

	model_load(argv[1]);

	if (argc > 2)
	{
		int w, h, b;
		byte *data = image_load(argv[2], &w, &h, &b);
		if (!data)
		{
			R_Printf(PRINT_ALL, "%s: failed loading\n", argv[2]);
		}
		else
		{
			R_Printf(PRINT_ALL, "Checks %dx%d %d\n", w, h, b);
			upload_texture(data, w, h);
			free(data);
		}
	}

	//While application is running
	while(1)
	{
		//Handle events on queue
		while(SDL_PollEvent(&e) != 0)
		{
			/* User requests quit */
			if(e.type == SDL_QUIT)
			{
				view_close();
			}

			/* Handle keypress with current mouse position */
			else if(e.type == SDL_TEXTINPUT)
			{
				int x = 0, y = 0;
				SDL_GetMouseState(&x, &y);
				keyboard(e.text.text[ 0 ], x, y);
			}
		}

		//Render quad
		view_render();

		//Update screen
		SDL_GL_SwapWindow(sdl_window);
	}

	return 0;
}
