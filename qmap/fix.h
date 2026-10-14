#ifndef INC_FIX_H
#define INC_FIX_H

typedef int fix;

#define fix_int(x)        ((x) >> 16)
#define float_to_fix(x)   ((fix) ((x) * 65536))

#define fix_cint(x)       (((x)+65535) >> 16)

#define fix_floor(x)      ((x) & 0xffff0000)
#define fix_ceil(x)       fix_floor((x)+0xffff)

#define fix_make(a,b)     (((a) << 16) + (b))

#endif
