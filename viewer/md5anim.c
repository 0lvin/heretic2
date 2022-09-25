/*
 * md5anim.c -- md5mesh model loader + animation
 * last modification: aug. 14, 2007
 *
 * Doom3's md5mesh viewer with animation.  Animation portion.
 * Dependences: md5model.h, md5mesh.c.
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
 * gcc -Wall -ansi -lGL -lGLU -lglut md5anim.c md5anim.c -o md5model
 */

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "md5model.h"

/* Joint info */
struct joint_info_t
{
  char name[64];
  int parent;
  int flags;
  int startIndex;
};

/* Base frame joint */
struct baseframe_joint_t
{
  vec3_t pos;
  quat4_t orient;
};

/**
 * More quaternion operations for skeletal animation.
 */

float
Quat_dotProduct (const quat4_t qa, const quat4_t qb)
{
  return ((qa[X] * qb[X]) + (qa[Y] * qb[Y]) + (qa[Z] * qb[Z]) + (qa[W] * qb[W]));
}

void
Quat_slerp (const quat4_t qa, const quat4_t qb, float t, quat4_t out)
{
  /* Check for out-of range parameter and return edge points if so */
  if (t <= 0.0)
    {
      memcpy (out, qa, sizeof(quat4_t));
      return;
    }

  if (t >= 1.0)
    {
      memcpy (out, qb, sizeof (quat4_t));
      return;
    }

  /* Compute "cosine of angle between quaternions" using dot product */
  float cosOmega = Quat_dotProduct (qa, qb);

  /* If negative dot, use -q1.  Two quaternions q and -q
     represent the same rotation, but may produce
     different slerp.  We chose q or -q to rotate using
     the acute angle. */
  float q1w = qb[W];
  float q1x = qb[X];
  float q1y = qb[Y];
  float q1z = qb[Z];

  if (cosOmega < 0.0f)
    {
      q1w = -q1w;
      q1x = -q1x;
      q1y = -q1y;
      q1z = -q1z;
      cosOmega = -cosOmega;
    }

  /* We should have two unit quaternions, so dot should be <= 1.0 */
  assert (cosOmega < 1.1f);

  /* Compute interpolation fraction, checking for quaternions
     almost exactly the same */
  float k0, k1;

  if (cosOmega > 0.9999f)
    {
      /* Very close - just use linear interpolation,
	 which will protect againt a divide by zero */

      k0 = 1.0f - t;
      k1 = t;
    }
  else
    {
      /* Compute the sin of the angle using the
	 trig identity sin^2(omega) + cos^2(omega) = 1 */
      float sinOmega = sqrt (1.0f - (cosOmega * cosOmega));

      /* Compute the angle from its sin and cosine */
      float omega = atan2 (sinOmega, cosOmega);

      /* Compute inverse of denominator, so we only have
	 to divide once */
      float oneOverSinOmega = 1.0f / sinOmega;

      /* Compute interpolation parameters */
      k0 = sin ((1.0f - t) * omega) * oneOverSinOmega;
      k1 = sin (t * omega) * oneOverSinOmega;
    }

  /* Interpolate and return new quaternion */
  out[W] = (k0 * qa[3]) + (k1 * q1w);
  out[X] = (k0 * qa[0]) + (k1 * q1x);
  out[Y] = (k0 * qa[1]) + (k1 * q1y);
  out[Z] = (k0 * qa[2]) + (k1 * q1z);
}

/**
 * Check if an animation can be used for a given model.  Model's
 * skeleton and animation's skeleton must match.
 */
int
CheckAnimValidity (const struct md5_model_t *mdl,
		   const struct md5_anim_t *anim)
{
  int i;

  /* md5mesh and md5anim must have the same number of joints */
  if (mdl->num_joints != anim->num_joints)
    return 0;

  /* We just check with frame[0] */
  for (i = 0; i < mdl->num_joints; ++i)
    {
      /* Joints must have the same parent index */
      if (mdl->baseSkel[i].parent != anim->skelFrames[0][i].parent)
	return 0;

      /* Joints must have the same name */
      if (strcmp (mdl->baseSkel[i].name, anim->skelFrames[0][i].name) != 0)
	return 0;
    }

  return 1;
}

/**
 * Build skeleton for a given frame data.
 */
static void
BuildFrameSkeleton (const struct joint_info_t *jointInfos,
		    const struct baseframe_joint_t *baseFrame,
		    const float *animFrameData,
		    struct md5_joint_t *skelFrame,
		    int num_joints)
{
  int i;

  for (i = 0; i < num_joints; ++i)
    {
      const struct baseframe_joint_t *baseJoint = &baseFrame[i];
      vec3_t animatedPos;
      quat4_t animatedOrient;
      int j = 0;

      memcpy (animatedPos, baseJoint->pos, sizeof (vec3_t));
      memcpy (animatedOrient, baseJoint->orient, sizeof (quat4_t));

      if (jointInfos[i].flags & 1) /* Tx */
	{
	  animatedPos[0] = animFrameData[jointInfos[i].startIndex + j];
	  ++j;
	}

      if (jointInfos[i].flags & 2) /* Ty */
	{
	  animatedPos[1] = animFrameData[jointInfos[i].startIndex + j];
	  ++j;
	}

      if (jointInfos[i].flags & 4) /* Tz */
	{
	  animatedPos[2] = animFrameData[jointInfos[i].startIndex + j];
	  ++j;
	}

      if (jointInfos[i].flags & 8) /* Qx */
	{
	  animatedOrient[0] = animFrameData[jointInfos[i].startIndex + j];
	  ++j;
	}

      if (jointInfos[i].flags & 16) /* Qy */
	{
	  animatedOrient[1] = animFrameData[jointInfos[i].startIndex + j];
	  ++j;
	}

      if (jointInfos[i].flags & 32) /* Qz */
	{
	  animatedOrient[2] = animFrameData[jointInfos[i].startIndex + j];
	  ++j;
	}

      /* Compute orient quaternion's w value */
      Quat_computeW (animatedOrient);

      /* NOTE: we assume that this joint's parent has
	 already been calculated, i.e. joint's ID should
	 never be smaller than its parent ID. */
      struct md5_joint_t *thisJoint = &skelFrame[i];

      int parent = jointInfos[i].parent;
      thisJoint->parent = parent;
      strcpy (thisJoint->name, jointInfos[i].name);

      /* Has parent? */
      if (thisJoint->parent < 0)
	{
	  memcpy (thisJoint->pos, animatedPos, sizeof (vec3_t));
	  memcpy (thisJoint->orient, animatedOrient, sizeof (quat4_t));
	}
      else
	{
	  struct md5_joint_t *parentJoint = &skelFrame[parent];
	  vec3_t rpos; /* Rotated position */

	  /* Add positions */
	  Quat_rotatePoint (parentJoint->orient, animatedPos, rpos);
	  thisJoint->pos[0] = rpos[0] + parentJoint->pos[0];
	  thisJoint->pos[1] = rpos[1] + parentJoint->pos[1];
	  thisJoint->pos[2] = rpos[2] + parentJoint->pos[2];

	  /* Concatenate rotations */
	  Quat_multQuat (parentJoint->orient, animatedOrient, thisJoint->orient);
	  Quat_normalize (thisJoint->orient);
	}
    }
}

/**
 * Load an MD5 animation from file.
 */
int
ReadMD5Anim (const char *filename, struct md5_anim_t *anim)
{
  FILE *fp = NULL;
  char buff[512];
  struct joint_info_t *jointInfos = NULL;
  struct baseframe_joint_t *baseFrame = NULL;
  float *animFrameData = NULL;
  int version;
  int numAnimatedComponents;
  int frame_index;
  int i;

  fp = fopen (filename, "rb");
  if (!fp)
    {
      fprintf (stderr, "error: couldn't open \"%s\"!\n", filename);
      return 0;
    }

  while (!feof (fp))
    {
      /* Read whole line */
      fgets (buff, sizeof (buff), fp);

      if (sscanf (buff, " MD5Version %d", &version) == 1)
	{
	  if (version != 10)
	    {
	      /* Bad version */
	      fprintf (stderr, "Error: bad animation version\n");
	      fclose (fp);
	      return 0;
	    }
	}
      else if (sscanf (buff, " numFrames %d", &anim->num_frames) == 1)
	{
	  /* Allocate memory for skeleton frames and bounding boxes */
	  if (anim->num_frames > 0)
	    {
	      anim->skelFrames = (struct md5_joint_t **)
		malloc (sizeof (struct md5_joint_t*) * anim->num_frames);
	      anim->bboxes = (struct md5_bbox_t *)
		malloc (sizeof (struct md5_bbox_t) * anim->num_frames);
	    }
	}
      else if (sscanf (buff, " numJoints %d", &anim->num_joints) == 1)
	{
	  if (anim->num_joints > 0)
	    {
	      for (i = 0; i < anim->num_frames; ++i)
		{
		  /* Allocate memory for joints of each frame */
		  anim->skelFrames[i] = (struct md5_joint_t *)
		    malloc (sizeof (struct md5_joint_t) * anim->num_joints);
		}

	      /* Allocate temporary memory for building skeleton frames */
	      jointInfos = (struct joint_info_t *)
		malloc (sizeof (struct joint_info_t) * anim->num_joints);

	      baseFrame = (struct baseframe_joint_t *)
		malloc (sizeof (struct baseframe_joint_t) * anim->num_joints);
	    }
	}
      else if (sscanf (buff, " frameRate %d", &anim->frameRate) == 1)
	{
	  /*
	    printf ("md5anim: animation's frame rate is %d\n", anim->frameRate);
	  */
	}
      else if (sscanf (buff, " numAnimatedComponents %d", &numAnimatedComponents) == 1)
	{
	  if (numAnimatedComponents > 0)
	    {
	      /* Allocate memory for animation frame data */
	      animFrameData = (float *)malloc (sizeof (float) * numAnimatedComponents);
	    }
	}
      else if (strncmp (buff, "hierarchy {", 11) == 0)
	{
	  for (i = 0; i < anim->num_joints; ++i)
	    {
	      /* Read whole line */
	      fgets (buff, sizeof (buff), fp);

	      /* Read joint info */
	      sscanf (buff, " %s %d %d %d", jointInfos[i].name, &jointInfos[i].parent,
		      &jointInfos[i].flags, &jointInfos[i].startIndex);
	    }
	}
      else if (strncmp (buff, "bounds {", 8) == 0)
	{
	  for (i = 0; i < anim->num_frames; ++i)
	    {
	      /* Read whole line */
	      fgets (buff, sizeof (buff), fp);

	      /* Read bounding box */
	      sscanf (buff, " ( %f %f %f ) ( %f %f %f )",
		      &anim->bboxes[i].min[0], &anim->bboxes[i].min[1],
		      &anim->bboxes[i].min[2], &anim->bboxes[i].max[0],
		      &anim->bboxes[i].max[1], &anim->bboxes[i].max[2]);
	    }
	}
      else if (strncmp (buff, "baseframe {", 10) == 0)
	{
	  for (i = 0; i < anim->num_joints; ++i)
	    {
	      /* Read whole line */
	      fgets (buff, sizeof (buff), fp);

	      /* Read base frame joint */
	      if (sscanf (buff, " ( %f %f %f ) ( %f %f %f )",
			  &baseFrame[i].pos[0], &baseFrame[i].pos[1],
			  &baseFrame[i].pos[2], &baseFrame[i].orient[0],
			  &baseFrame[i].orient[1], &baseFrame[i].orient[2]) == 6)
		{
		  /* Compute the w component */
		  Quat_computeW (baseFrame[i].orient);
		}
	    }
	}
      else if (sscanf (buff, " frame %d", &frame_index) == 1)
	{
	  /* Read frame data */
	  for (i = 0; i < numAnimatedComponents; ++i)
	    fscanf (fp, "%f", &animFrameData[i]);

	  /* Build frame skeleton from the collected data */
	  BuildFrameSkeleton (jointInfos, baseFrame, animFrameData,
			      anim->skelFrames[frame_index], anim->num_joints);
	}
  }

  fclose (fp);

  /* Free temporary data allocated */
  if (animFrameData)
    free (animFrameData);

  if (baseFrame)
    free (baseFrame);

  if (jointInfos)
    free (jointInfos);

  return 1;
}

/**
 * Free resources allocated for the animation.
 */
void
FreeAnim (struct md5_anim_t *anim)
{
  int i;

  if (anim->skelFrames)
    {
      for (i = 0; i < anim->num_frames; ++i)
	{
	  if (anim->skelFrames[i])
	    {
	      free (anim->skelFrames[i]);
	      anim->skelFrames[i] = NULL;
	    }
	}

      free (anim->skelFrames);
      anim->skelFrames = NULL;
    }

  if (anim->bboxes)
    {
      free (anim->bboxes);
      anim->bboxes = NULL;
    }
}

/**
 * Smoothly interpolate two skeletons
 */
void
InterpolateSkeletons (const struct md5_joint_t *skelA,
		      const struct md5_joint_t *skelB,
		      int num_joints, float interp,
		      struct md5_joint_t *out)
{
  int i;

  for (i = 0; i < num_joints; ++i)
    {
      /* Copy parent index */
      out[i].parent = skelA[i].parent;

      /* Linear interpolation for position */
      out[i].pos[0] = skelA[i].pos[0] + interp * (skelB[i].pos[0] - skelA[i].pos[0]);
      out[i].pos[1] = skelA[i].pos[1] + interp * (skelB[i].pos[1] - skelA[i].pos[1]);
      out[i].pos[2] = skelA[i].pos[2] + interp * (skelB[i].pos[2] - skelA[i].pos[2]);

      /* Spherical linear interpolation for orientation */
      Quat_slerp (skelA[i].orient, skelB[i].orient, interp, out[i].orient);
    }
}

/**
 * Perform animation related computations.  Calculate the current and
 * next frames, given a delta time.
 */
void
Animate (const struct md5_anim_t *anim,
	 struct anim_info_t *animInfo, double dt)
{
  int maxFrames = anim->num_frames - 1;

  animInfo->last_time += dt;

  /* move to next frame */
  if (animInfo->last_time >= animInfo->max_time)
    {
      animInfo->curr_frame++;
      animInfo->next_frame++;
      animInfo->last_time = 0.0;

      if (animInfo->curr_frame > maxFrames)
	animInfo->curr_frame = 0;

      if (animInfo->next_frame > maxFrames)
	animInfo->next_frame = 0;
    }
}
