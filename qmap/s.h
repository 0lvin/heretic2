#ifndef INC_S_H
#define INC_S_H

#include "fix.h"

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef int bool;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef int int32;

#define FALSE 0
#define TRUE  1

#define MAX(a, b)      (((a) > (b)) ? (a) : (b))
#define MIN(a, b)      (((a) < (b)) ? (a) : (b))
#define CLAMP(x, a, b) (MIN((b), MAX((a), (x))))

// convert from 64-bit IEEE little-endian to 32-bit int, faster
// than built-in Intel ops, but doesn't obey rounding rule and
// doesn't deal well with overflow

#if 1

#define BIG_NUM ((float) (1 << 26) * (1 << 26) * 1.5)

static inline int FLOAT_TO_INT(double x)
{
   double temp = x + BIG_NUM;
   return *(int*)&temp;
}
static inline fix FLOAT_TO_FIX(double x)
{
   double temp = x + BIG_NUM / 65536.0;
   return *(int*)&temp;
}

#else

static inline int FLOAT_TO_INT(double x)
{
   return (int)x;
}

static inline fix FLOAT_TO_FIX(double x)
{
   return (fix)(x * 65536.0);
}

#endif

extern void fatal_error(char *message, char *file, int line);
#define fatal(s)   fatal_error(s, __FILE__, __LINE__)

#endif
