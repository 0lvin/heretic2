/*
    QShed <http://www.icculus.org/qshed>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef unsigned char byte;
typedef enum { false = 0, true = 1 } bool_t;

/* FIXME - detect endianness? */
#define LittleShort(x) (x)
#define LittleLong(x)  (x)
#define LittleFloat(x) (x)
/*#define LittleShort(x) SwapShort(x)*/
/*#define LittleLong(x)  SwapLong(x)*/
/*#define LittleFloat(x) SwapFloat(x)*/

#ifndef M_PI
# define M_PI 3.14159265358979323846 /* matches value in gcc v2 math.h */
#endif

#ifndef min
# define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
# define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef bound
# define bound(xmin,x,xmax) (((x) < (xmin)) ? (xmin) : (((x) > (xmax)) ? (xmax) : (x)))
#endif

typedef float vec3f_t[3];

#define VectorClear(a) ((a)[0]=0,(a)[1]=0,(a)[2]=0)
#define VectorCopy(a,b) ((a)[0]=(b)[0],(a)[1]=(b)[1],(a)[2]=(b)[2])
#define DotProduct(a,b) ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VectorAdd(a,b,c) ((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorSubtract(a,b,c) ((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorScale(in, scale, out) ((out)[0] = (in)[0] * (scale),(out)[1] = (in)[1] * (scale),(out)[2] = (in)[2] * (scale))
#define CrossProduct(a,b,c) ((c)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1],(c)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2],(c)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0])
#define VectorNormalize(v) do{float ilength = (float)sqrt(DotProduct(v,v));if (ilength) {ilength = 1.0f / ilength;VectorScale(v, ilength, v);}}while(0)

#define IS_POWER_OF_TWO(x) ((x) && !(((x)-1) & (x)))

typedef struct mem_pool_s mem_pool_t;
extern mem_pool_t *mem_globalpool;
mem_pool_t *mem_create_pool_(const char *file, int line);
#define mem_create_pool() mem_create_pool_(__FILE__,__LINE__)
void mem_merge_pool(mem_pool_t *pool);
void mem_free_pool_(mem_pool_t *pool, bool_t complain);
#define mem_free_pool(pool) mem_free_pool_(pool,false)
void *mem_alloc_(mem_pool_t *pool, size_t numbytes, const char *file, int line);
#define mem_alloc(pool,numbytes) mem_alloc_(pool,numbytes,__FILE__,__LINE__)
void mem_free(void *mem);
void mem_init(void);
void mem_shutdown(void);
void *qmalloc_(size_t numbytes, const char *file, int line);
#define qmalloc(numbytes) qmalloc_(numbytes, __FILE__, __LINE__)
void qfree(void *mem);

char *mem_copystring(mem_pool_t *pool, const char *string);
char *copystring(const char *string);
char *mem_sprintf(mem_pool_t *pool, const char *format, ...);
char *msprintf(const char *format, ...);

#ifdef WIN32
# define strcasecmp _stricmp
# define strncasecmp _strnicmp
#endif

extern bool_t g_force_yes;

bool_t makepath(char *path, char **out_error);
bool_t loadfile(const char *filename, void **out_data, size_t *out_size, char **out_error);
bool_t writefile(const char *filename, const void *data, size_t size, char **out_error);

typedef void* dllhandle_t;
typedef struct dllfunction_s { const char *name; void **funcvariable; } dllfunction_t;

bool_t loadlibrary(const char *dllname, dllhandle_t *handle, const dllfunction_t *functions);
void unloadlibrary(dllhandle_t *handle);
void *getprocaddress(dllhandle_t handle, const char *name);

void add_atexit_event(void (*function)(void));
void set_atexit_final_event(void (*function)(void));
void call_atexit_events(void);

/* expandable buffers */

typedef struct xbuf_s xbuf_t;

xbuf_t *xbuf_create_memory(size_t block_size, char **out_error);
xbuf_t *xbuf_create_file(size_t block_size, const char *filename, char **out_error);

bool_t xbuf_free(xbuf_t *xbuf, char **out_error);
bool_t xbuf_finish_memory(xbuf_t *xbuf, void **out_data, size_t *out_length, char **out_error);
bool_t xbuf_finish_file(xbuf_t *xbuf, char **out_error);

bool_t xbuf_write_to_file(xbuf_t *xbuf, const char *filename, char **out_error);

void xbuf_write_data(xbuf_t *xbuf, size_t length, const void *data);
void xbuf_write_byte(xbuf_t *xbuf, unsigned char byte);
void *xbuf_reserve_data(xbuf_t *xbuf, size_t length);

int xbuf_get_bytes_written(const xbuf_t *xbuf);

#endif
