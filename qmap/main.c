/* main.c - General setup control, main "sim" loop
 *   Copyright 1997 Sean Barett (Public domain)
 */

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <SDL_scancode.h>
#include <stdio.h>
#include "bspfile.h"
#include "mode.h"
#include "3d.h"
#include "fix.h"
#include "scr.h"
#include "tm.h"
#include "render.h"
#include "bsp.h"
#include "surface.h"
#include "poly.h"
#include "clock.h"
#include "text.h"
#include "cam.h"

char *scr_buf;
int   scr_row;

uchar colormap[64][256];

bool running;
camera cam;

void sim_loop(void)
{
   char text[256];

   // RENDER

   set_view_info(&cam.loc, &cam.ang);
   render_world(&cam.loc);
   blit(scr_buf);

   // UI

   snprintf(text, sizeof(text), "FPS: %.2f", fps);
   draw_text(8, 6, text);

   present();

   // INPUT

   poll_events(&running);
   if (get_key(SDL_SCANCODE_ESCAPE))
      running = false;

   // LOGIC

   clock_tick();
   cam_update(&cam);
}

void run_sim(void)
{
   running = TRUE;

   scr_buf = malloc(SCREENW * SCREENH);
   memset(scr_buf, 0, SCREENW * SCREENH);
   scr_row = SCREENW;
   qmap_set_output(scr_buf, scr_row);

   cam_init(&cam);
   cam.loc.x = 500;
   cam.loc.y = 240;
   cam.loc.z = 100;

#ifdef EMSCRIPTEN
   emscripten_set_main_loop(sim_loop, 0, 1);
#else
   while (running) sim_loop();
#endif

   free(scr_buf);
}

void load_graphics(void)
{
   char pal[768];
   FILE *f;
   if ((f = fopen("palette.lmp", "rb")) == NULL)
      fatal("Couldn't read palette.lmp\n");
   fread(pal, 1, 768, f);
   fclose(f);
   set_pal((uchar*)pal);
   if ((f = fopen("colormap.lmp", "rb")) == NULL)
      fatal("Couldn't read colormap.lmp\n");
   fread(colormap, 256, 64, f);
   fclose(f);
}

int main(int argc, char **argv)
{
#ifdef EMSCRIPTEN
   {
      LoadBSPFile("e1m1.bsp");
#else
   if (argc != 2) {
      printf("Usage: qmap <bspfile>\n");
   } else {
      LoadBSPFile(argv[1]);
#endif
      setup_sdl();
      load_graphics();
      init_cache();
      setup_default_point_list();
      clock_init();

      run_sim();
      close_sdl();
   }
   return 0;
}
