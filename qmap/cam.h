#ifndef INC_CAM_H
#define INC_CAM_H

#include "3d.h"

typedef struct {
   vector loc, vel;
   angvec ang, angvel;
} camera;

extern void cam_init(camera *cam);
extern void cam_update(camera *cam);

#endif//INC_CAM_H
