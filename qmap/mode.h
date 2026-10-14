#ifndef INC_MODE_H
#define INC_MODE_H

#include "s.h"

extern void blit(char *src);
extern void present();
extern void set_pal(uchar *pal);
extern void setup_sdl(void);
extern void close_sdl(void);

extern void poll_events(bool* running);
extern bool get_key(int scancode);
extern bool get_mmove(int *outx, int *outy);

#define SCREENW 800
#define SCREENH 600

#endif
