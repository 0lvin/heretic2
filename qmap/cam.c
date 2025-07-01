/* cam.c
 *   Copyright 2018 a dinosaur (0BSD)
 */

#include <SDL.h>

#include "mode.h"
#include "clock.h"
#include "cam.h"

#define ANG_MXVL 4.0
#define ANG_ACCL 30.0
#define ANG_FRCT 20.0

#define VEL_MXVL 420.0f
#define VEL_ACCL 2400.0f
#define VEL_FRCT (VEL_ACCL / 2.0f)

#define MOUSE_SENS (1.0 / 1600.0)

extern void cam_init(camera *cam)
{
   cam->loc    = (vector){0.0f, 0.0f, 0.0f};
   cam->vel    = (vector){0.0f, 0.0f, 0.0f};
   cam->ang    = (angvec){0, 0, 0};
   cam->angvel = (angvec){0, 0, 0};
}

extern void cam_update(camera *cam)
{
   int mx, my;
   vector temp;

   // CONTROLS

   if (get_key(SDL_SCANCODE_UP))
      cam->angvel.tx += ANG_ACCL * delta;
   if (get_key(SDL_SCANCODE_DOWN))
      cam->angvel.tx -= ANG_ACCL * delta;
   if (get_key(SDL_SCANCODE_Q))
      cam->angvel.ty += ANG_ACCL * delta;
   if (get_key(SDL_SCANCODE_E))
      cam->angvel.ty -= ANG_ACCL * delta;
   if (get_key(SDL_SCANCODE_RIGHT))
      cam->angvel.tz += ANG_ACCL * delta;
   if (get_key(SDL_SCANCODE_LEFT))
      cam->angvel.tz -= ANG_ACCL * delta;

   if (get_key(SDL_SCANCODE_D))
      cam->vel.x += VEL_ACCL * delta;
   if (get_key(SDL_SCANCODE_A))
      cam->vel.x -= VEL_ACCL * delta;
   if (get_key(SDL_SCANCODE_LSHIFT))
      cam->vel.z -= VEL_ACCL * delta;
   if (get_key(SDL_SCANCODE_SPACE))
      cam->vel.z += VEL_ACCL * delta;
   if (get_key(SDL_SCANCODE_S))
      cam->vel.y -= VEL_ACCL * delta;
   if (get_key(SDL_SCANCODE_W))
      cam->vel.y += VEL_ACCL * delta;

   // apply mouse movement
   if (get_mmove(&mx, &my)) {
      cam->ang.tz += (double)mx * MOUSE_SENS;
      cam->ang.tx += (double)my * MOUSE_SENS;
   }


   // "PHYSICS"

   // clamp velocity
   cam->vel.x = CLAMP(cam->vel.x, -VEL_MXVL, VEL_MXVL);
   cam->vel.y = CLAMP(cam->vel.y, -VEL_MXVL, VEL_MXVL);
   cam->vel.z = CLAMP(cam->vel.z, -VEL_MXVL, VEL_MXVL);

   // clamp rotational velocity
   cam->angvel.tx = CLAMP(cam->angvel.tx, -ANG_MXVL, ANG_MXVL);
   cam->angvel.ty = CLAMP(cam->angvel.ty, -ANG_MXVL, ANG_MXVL);
   cam->angvel.tz = CLAMP(cam->angvel.tz, -ANG_MXVL, ANG_MXVL);

   // weight roll back to centre
   if (fabs(cam->ang.ty) > 0.000001) {
      double x = fabs(cam->ang.ty);
      double x1 = x + 1.0;
      double curve = (1.0 - x1 / (x1 * x1)) * 2.0 + x * 0.125;
      cam->ang.ty -= copysign(curve, cam->ang.ty) * 2.0 * delta;
   } else {
      cam->ang.ty = 0.0;
   }

   // apply rotational velocity
   cam->ang.tx += cam->angvel.tx * delta;
   cam->ang.ty += cam->angvel.ty * delta;
   cam->ang.tz += cam->angvel.tz * delta;

   // keep angles within reasonable ranges
   if (cam->ang.tx < -M_PI * 0.5) {
      cam->ang.tx = -M_PI * 0.5;
      if (cam->angvel.tx < 0.0)
         cam->angvel.tx = 0.0;
   } else if (cam->ang.tx > M_PI * 0.5) {
      cam->ang.tx = M_PI * 0.5;
      if (cam->angvel.tx > 0.0)
         cam->angvel.tx = 0.0;
   }
   if (cam->ang.ty < -M_PI) {
      cam->ang.ty += M_PI * 2.0;
   } else if (cam->ang.ty > M_PI) {
      cam->ang.ty -= M_PI * 2.0;
   }
   if (cam->ang.tz < 0.0) {
      cam->ang.tz += M_PI * 2.0;
   } else if (cam->ang.tz >= M_PI * 2.0) {
      cam->ang.tz -= M_PI * 2.0;
   }


   // apply translational velocity (transformed by rotation)
   temp.x = cam->vel.x * delta;
   temp.y = 0;
   temp.z = 0;
   rotate_vec(&temp);
   cam->loc.x += temp.x;
   cam->loc.y += temp.y;
   cam->loc.z += temp.z;

   temp.x = 0;
   temp.y = cam->vel.y * delta;
   temp.z = 0;
   rotate_vec(&temp);
   cam->loc.x += temp.x;
   cam->loc.y += temp.y;
   cam->loc.z += temp.z;

   temp.x = 0;
   temp.y = 0;
   temp.z = cam->vel.z * delta;
   rotate_vec(&temp);
   cam->loc.x += temp.x;
   cam->loc.y += temp.y;
   cam->loc.z += temp.z;

   // apply translational friction
   if (cam->vel.x > 0.0f) {
      cam->vel.x -= VEL_FRCT * delta;
      if (cam->vel.x < 0.0f)
         cam->vel.x = 0.0f;
   } else if (cam->vel.x < 0.0f) {
      cam->vel.x += VEL_FRCT * delta;
      if (cam->vel.x > 0.0f)
         cam->vel.x = 0.0f;
   }
   if (cam->vel.y > 0.0f) {
      cam->vel.y -= VEL_FRCT * delta;
      if (cam->vel.y < 0.0f)
         cam->vel.y = 0.0f;
   } else if (cam->vel.y < 0.0f) {
      cam->vel.y += VEL_FRCT * delta;
      if (cam->vel.y > 0.0f)
         cam->vel.y = 0.0f;
   }
   if (cam->vel.z > 0.0f) {
      cam->vel.z -= VEL_FRCT * delta;
      if (cam->vel.z < 0.0f)
         cam->vel.z = 0.0f;
   } else if (cam->vel.z < 0.0f) {
      cam->vel.z += VEL_FRCT * delta;
      if (cam->vel.z > 0.0f)
         cam->vel.z = 0.0f;
   }

   // apply rotational friction
   if (cam->angvel.tx > 0.0) {
      cam->angvel.tx -= ANG_FRCT * delta;
      if (cam->angvel.tx < 0.0)
         cam->angvel.tx = 0.0;
   } else if (cam->angvel.tx < 0.0) {
      cam->angvel.tx += ANG_FRCT * delta;
      if (cam->angvel.tx > 0.0)
         cam->angvel.tx = 0.0;
   }
   if (cam->angvel.ty > 0.0) {
      cam->angvel.ty -= ANG_FRCT * delta;
      if (cam->angvel.ty < 0.0)
         cam->angvel.ty = 0.0;
   } else if (cam->angvel.ty < 0.0) {
      cam->angvel.ty += ANG_FRCT * delta;
      if (cam->angvel.ty > 0.0)
         cam->angvel.ty = 0.0;
   }
   if (cam->angvel.tz > 0.0) {
      cam->angvel.tz -= ANG_FRCT * delta;
      if (cam->angvel.tz < 0.0)
         cam->angvel.tz = 0.0;
   } else if (cam->angvel.tz < 0.0) {
      cam->angvel.tz += ANG_FRCT * delta;
      if (cam->angvel.tz > 0.0)
         cam->angvel.tz = 0.0;
   }
}
