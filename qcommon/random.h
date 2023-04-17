#ifndef RANDOM_H
#define RANDOM_H

#ifdef __cplusplus
extern "C" {
#endif
// Required protos for random functions
QUAKE2_API float flrand(float, float);
QUAKE2_API int irand(int, int);
#ifdef __cplusplus
} //end extern "C"
#endif

#endif
