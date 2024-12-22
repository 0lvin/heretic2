// cmdlib.h

#ifndef __CMDLIB__
#define __CMDLIB__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
typedef enum {false, true} qboolean;
typedef unsigned char byte;
#endif


int     filelength (FILE *f);

void	Error (char *error, ...);

FILE	*SafeOpenRead (char *filename);
void	SafeRead (FILE *f, void *buffer, int count);

int		LoadFile (char *filename, void **bufferptr);

short	BigShort (short l);
short	LittleShort (short l);
int		BigLong (int l);
int		LittleLong (int l);
float	BigFloat (float l);
float	LittleFloat (float l);

#endif
