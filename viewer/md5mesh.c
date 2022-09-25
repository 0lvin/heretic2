/*
 * md5mesh.c -- md5mesh model loader + animation
 * last modification: aug. 14, 2007
 *
 * Doom3's md5mesh viewer with animation.  Mesh portion.
 * Dependences: md5model.h, md5anim.c.
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

#include "md5model.h"


struct md5_model_t md5file;
struct md5_anim_t md5anim;

int animated = 0;

struct md5_joint_t *skeleton = NULL;
struct anim_info_t animInfo;

/* vertex array related stuff */
int max_verts = 0;
int max_tris = 0;

vec3_t *vertexArray = NULL;
GLuint *vertexIndices = NULL;


/**
 * Basic quaternion operations.
 */

void
Quat_computeW (quat4_t q)
{
  float t = 1.0f - (q[X] * q[X]) - (q[Y] * q[Y]) - (q[Z] * q[Z]);

  if (t < 0.0f)
    q[W] = 0.0f;
  else
    q[W] = -sqrt (t);
}

void
Quat_normalize (quat4_t q)
{
  /* compute magnitude of the quaternion */
  float mag = sqrt ((q[X] * q[X]) + (q[Y] * q[Y])
		    + (q[Z] * q[Z]) + (q[W] * q[W]));

  /* check for bogus length, to protect against divide by zero */
  if (mag > 0.0f)
    {
      /* normalize it */
      float oneOverMag = 1.0f / mag;

      q[X] *= oneOverMag;
      q[Y] *= oneOverMag;
      q[Z] *= oneOverMag;
      q[W] *= oneOverMag;
    }
}

void
Quat_multQuat (const quat4_t qa, const quat4_t qb, quat4_t out)
{
  out[W] = (qa[W] * qb[W]) - (qa[X] * qb[X]) - (qa[Y] * qb[Y]) - (qa[Z] * qb[Z]);
  out[X] = (qa[X] * qb[W]) + (qa[W] * qb[X]) + (qa[Y] * qb[Z]) - (qa[Z] * qb[Y]);
  out[Y] = (qa[Y] * qb[W]) + (qa[W] * qb[Y]) + (qa[Z] * qb[X]) - (qa[X] * qb[Z]);
  out[Z] = (qa[Z] * qb[W]) + (qa[W] * qb[Z]) + (qa[X] * qb[Y]) - (qa[Y] * qb[X]);
}

void
Quat_multVec (const quat4_t q, const vec3_t v, quat4_t out)
{
  out[W] = - (q[X] * v[X]) - (q[Y] * v[Y]) - (q[Z] * v[Z]);
  out[X] =   (q[W] * v[X]) + (q[Y] * v[Z]) - (q[Z] * v[Y]);
  out[Y] =   (q[W] * v[Y]) + (q[Z] * v[X]) - (q[X] * v[Z]);
  out[Z] =   (q[W] * v[Z]) + (q[X] * v[Y]) - (q[Y] * v[X]);
}

void
Quat_rotatePoint (const quat4_t q, const vec3_t in, vec3_t out)
{
  quat4_t tmp, inv, final;

  inv[X] = -q[X]; inv[Y] = -q[Y];
  inv[Z] = -q[Z]; inv[W] =  q[W];

  Quat_normalize (inv);

  Quat_multVec (q, in, tmp);
  Quat_multQuat (tmp, inv, final);

  out[X] = final[X];
  out[Y] = final[Y];
  out[Z] = final[Z];
}

/**
 * Load an MD5 model from file.
 */
int
ReadMD5Model (const char *filename, struct md5_model_t *mdl)
{
  FILE *fp;
  char buff[512];
  int version;
  int curr_mesh = 0;
  int i;

  fp = fopen (filename, "rb");
  if (!fp)
    {
      fprintf (stderr, "Error: couldn't open \"%s\"!\n", filename);
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
	      fprintf (stderr, "Error: bad model version\n");
	      fclose (fp);
	      return 0;
	    }
	}
      else if (sscanf (buff, " numJoints %d", &mdl->num_joints) == 1)
	{
	  if (mdl->num_joints > 0)
	    {
	      /* Allocate memory for base skeleton joints */
	      mdl->baseSkel = (struct md5_joint_t *)
		calloc (mdl->num_joints, sizeof (struct md5_joint_t));
	    }
	}
      else if (sscanf (buff, " numMeshes %d", &mdl->num_meshes) == 1)
	{
	  if (mdl->num_meshes > 0)
	    {
	      /* Allocate memory for meshes */
	      mdl->meshes = (struct md5_mesh_t *)
		calloc (mdl->num_meshes, sizeof (struct md5_mesh_t));
	    }
	}
      else if (strncmp (buff, "joints {", 8) == 0)
	{
	  /* Read each joint */
	  for (i = 0; i < mdl->num_joints; ++i)
	    {
	      struct md5_joint_t *joint = &mdl->baseSkel[i];

	      /* Read whole line */
	      fgets (buff, sizeof (buff), fp);

	      if (sscanf (buff, "%s %d ( %f %f %f ) ( %f %f %f )",
			  joint->name, &joint->parent, &joint->pos[0],
			  &joint->pos[1], &joint->pos[2], &joint->orient[0],
			  &joint->orient[1], &joint->orient[2]) == 8)
		{
		  /* Compute the w component */
		  Quat_computeW (joint->orient);
		}
	    }
	}
      else if (strncmp (buff, "mesh {", 6) == 0)
	{
	  struct md5_mesh_t *mesh = &mdl->meshes[curr_mesh];
	  int vert_index = 0;
	  int tri_index = 0;
	  int weight_index = 0;
	  float fdata[4];
	  int idata[3];

	  while ((buff[0] != '}') && !feof (fp))
	    {
	      /* Read whole line */
	      fgets (buff, sizeof (buff), fp);

	      if (strstr (buff, "shader "))
		{
		  int quote = 0, j = 0;

		  /* Copy the shader name whithout the quote marks */
		  for (i = 0; i < sizeof (buff) && (quote < 2); ++i)
		    {
		      if (buff[i] == '\"')
			quote++;

		      if ((quote == 1) && (buff[i] != '\"'))
			{
			  mesh->shader[j] = buff[i];
			  j++;
			}
		    }
		}
	      else if (sscanf (buff, " numverts %d", &mesh->num_verts) == 1)
		{
		  if (mesh->num_verts > 0)
		    {
		      /* Allocate memory for vertices */
		      mesh->vertices = (struct md5_vertex_t *)
			malloc (sizeof (struct md5_vertex_t) * mesh->num_verts);
		    }

		  if (mesh->num_verts > max_verts)
		    max_verts = mesh->num_verts;
		}
	      else if (sscanf (buff, " numtris %d", &mesh->num_tris) == 1)
		{
		  if (mesh->num_tris > 0)
		    {
		      /* Allocate memory for triangles */
		      mesh->triangles = (struct md5_triangle_t *)
			malloc (sizeof (struct md5_triangle_t) * mesh->num_tris);
		    }

		  if (mesh->num_tris > max_tris)
		    max_tris = mesh->num_tris;
		}
	      else if (sscanf (buff, " numweights %d", &mesh->num_weights) == 1)
		{
		  if (mesh->num_weights > 0)
		    {
		      /* Allocate memory for vertex weights */
		      mesh->weights = (struct md5_weight_t *)
			malloc (sizeof (struct md5_weight_t) * mesh->num_weights);
		    }
		}
	      else if (sscanf (buff, " vert %d ( %f %f ) %d %d", &vert_index,
			       &fdata[0], &fdata[1], &idata[0], &idata[1]) == 5)
		{
		  /* Copy vertex data */
		  mesh->vertices[vert_index].st[0] = fdata[0];
		  mesh->vertices[vert_index].st[1] = fdata[1];
		  mesh->vertices[vert_index].start = idata[0];
		  mesh->vertices[vert_index].count = idata[1];
		}
	      else if (sscanf (buff, " tri %d %d %d %d", &tri_index,
			       &idata[0], &idata[1], &idata[2]) == 4)
		{
		  /* Copy triangle data */
		  mesh->triangles[tri_index ].index[0] = idata[0];
		  mesh->triangles[tri_index ].index[1] = idata[1];
		  mesh->triangles[tri_index ].index[2] = idata[2];
		}
	      else if (sscanf (buff, " weight %d %d %f ( %f %f %f )",
			       &weight_index, &idata[0], &fdata[3],
			       &fdata[0], &fdata[1], &fdata[2]) == 6)
		{
		  /* Copy vertex data */
		  mesh->weights[weight_index].joint  = idata[0];
		  mesh->weights[weight_index].bias   = fdata[3];
		  mesh->weights[weight_index].pos[0] = fdata[0];
		  mesh->weights[weight_index].pos[1] = fdata[1];
		  mesh->weights[weight_index].pos[2] = fdata[2];
		}
	    }

	  curr_mesh++;
	}
    }

  fclose (fp);

  return 1;
}

/**
 * Free resources allocated for the model.
 */
void
FreeModel (struct md5_model_t *mdl)
{
  int i;

  if (mdl->baseSkel)
    {
      free (mdl->baseSkel);
      mdl->baseSkel = NULL;
    }

  if (mdl->meshes)
    {
      /* Free mesh data */
      for (i = 0; i < mdl->num_meshes; ++i)
	{
	  if (mdl->meshes[i].vertices)
	    {
	      free (mdl->meshes[i].vertices);
	      mdl->meshes[i].vertices = NULL;
	    }

	  if (mdl->meshes[i].triangles)
	    {
	      free (mdl->meshes[i].triangles);
	      mdl->meshes[i].triangles = NULL;
	    }

	  if (mdl->meshes[i].weights)
	    {
	      free (mdl->meshes[i].weights);
	      mdl->meshes[i].weights = NULL;
	    }
	}

      free (mdl->meshes);
      mdl->meshes = NULL;
    }
}

/**
 * Prepare a mesh for drawing.  Compute mesh's final vertex positions
 * given a skeleton.  Put the vertices in vertex arrays.
 */
void
PrepareMesh (const struct md5_mesh_t *mesh,
	     const struct md5_joint_t *skeleton)

{
  int i, j, k;

  /* Setup vertex indices */
  for (k = 0, i = 0; i < mesh->num_tris; ++i)
    {
      for (j = 0; j < 3; ++j, ++k)
	vertexIndices[k] = mesh->triangles[i].index[j];
    }

  /* Setup vertices */
  for (i = 0; i < mesh->num_verts; ++i)
    {
      vec3_t finalVertex = { 0.0f, 0.0f, 0.0f };

      /* Calculate final vertex to draw with weights */
      for (j = 0; j < mesh->vertices[i].count; ++j)
	{
	  const struct md5_weight_t *weight
	    = &mesh->weights[mesh->vertices[i].start + j];
	  const struct md5_joint_t *joint
	    = &skeleton[weight->joint];

	  /* Calculate transformed vertex for this weight */
	  vec3_t wv;
	  Quat_rotatePoint (joint->orient, weight->pos, wv);

	  /* The sum of all weight->bias should be 1.0 */
	  finalVertex[0] += (joint->pos[0] + wv[0]) * weight->bias;
	  finalVertex[1] += (joint->pos[1] + wv[1]) * weight->bias;
	  finalVertex[2] += (joint->pos[2] + wv[2]) * weight->bias;
	}

      vertexArray[i][0] = finalVertex[0];
      vertexArray[i][1] = finalVertex[1];
      vertexArray[i][2] = finalVertex[2];
    }
}

void
AllocVertexArrays ()
{
  vertexArray = (vec3_t *)malloc (sizeof (vec3_t) * max_verts);
  vertexIndices = (GLuint *)malloc (sizeof (GLuint) * max_tris * 3);
}

void
FreeVertexArrays ()
{
  if (vertexArray)
    {
      free (vertexArray);
      vertexArray = NULL;
    }

  if (vertexIndices)
    {
      free (vertexIndices);
      vertexIndices = NULL;
    }
}

/**
 * Draw the skeleton as lines and points (for joints).
 */
void
DrawSkeleton (const struct md5_joint_t *skeleton, int num_joints)
{
  int i;

  /* Draw each joint */
  glPointSize (5.0f);
  glColor3f (1.0f, 0.0f, 0.0f);
  glBegin (GL_POINTS);
    for (i = 0; i < num_joints; ++i)
      glVertex3fv (skeleton[i].pos);
  glEnd ();
  glPointSize (1.0f);

  /* Draw each bone */
  glColor3f (0.0f, 1.0f, 0.0f);
  glBegin (GL_LINES);
    for (i = 0; i < num_joints; ++i)
      {
	if (skeleton[i].parent != -1)
	  {
	    glVertex3fv (skeleton[skeleton[i].parent].pos);
	    glVertex3fv (skeleton[i].pos);
	  }
      }
  glEnd();
}

void
init (const char *filename, const char *animfile)
{
  /* Initialize OpenGL context */
  glClearColor (0.5f, 0.5f, 0.5f, 1.0f);
  glShadeModel (GL_SMOOTH);

  glEnable (GL_DEPTH_TEST);

  /* Load MD5 model file */
  if (!ReadMD5Model (filename, &md5file))
    exit (EXIT_FAILURE);

  AllocVertexArrays ();

  /* Load MD5 animation file */
  if (animfile)
    {
      if (!ReadMD5Anim (animfile, &md5anim))
	{
	  FreeAnim (&md5anim);
	}
      else
	{
	  animInfo.curr_frame = 0;
	  animInfo.next_frame = 1;

	  animInfo.last_time = 0;
	  animInfo.max_time = 1.0 / md5anim.frameRate;

	  /* Allocate memory for animated skeleton */
	  skeleton = (struct md5_joint_t *)
	    malloc (sizeof (struct md5_joint_t) * md5anim.num_joints);

	  animated = 1;
	}
    }

  if (!animated)
    printf ("init: no animation loaded.\n");
}

void
cleanup ()
{
  FreeModel (&md5file);
  FreeAnim (&md5anim);

  if (animated && skeleton)
    {
      free (skeleton);
      skeleton = NULL;
    }

  FreeVertexArrays ();
}

void
reshape (int w, int h)
{
  if (h == 0)
    h = 1;

  glViewport (0, 0, (GLsizei)w, (GLsizei)h);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (45.0, w/(GLdouble)h, 0.1, 1000.0);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
}

void
display ()
{
  int i;
  static float angle = 0;
  static double curent_time = 0;
  static double last_time = 0;

  last_time = curent_time;
  curent_time = (double)glutGet (GLUT_ELAPSED_TIME) / 1000.0;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity ();

  glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

  glTranslatef (0.0f, -35.0f, -150.0f);
  glRotatef (-90.0f, 1.0, 0.0, 0.0);
  glRotatef (angle, 0.0, 0.0, 1.0);

  angle += 25 * (curent_time - last_time);

  if (angle > 360.0f)
    angle -= 360.0f;

  if (animated)
    {
      /* Calculate current and next frames */
      Animate (&md5anim, &animInfo, curent_time - last_time);

      /* Interpolate skeletons between two frames */
      InterpolateSkeletons (md5anim.skelFrames[animInfo.curr_frame],
			    md5anim.skelFrames[animInfo.next_frame],
			    md5anim.num_joints,
			    animInfo.last_time * md5anim.frameRate,
			    skeleton);
    }
  else
    {
      /* No animation, use bind-pose skeleton */
      skeleton = md5file.baseSkel;
    }

  /* Draw skeleton */
  DrawSkeleton (skeleton, md5file.num_joints);

  glColor3f (1.0f, 1.0f, 1.0f);

  glEnableClientState (GL_VERTEX_ARRAY);

  /* Draw each mesh of the model */
  for (i = 0; i < md5file.num_meshes; ++i)
    {
      PrepareMesh (&md5file.meshes[i], skeleton);

      glVertexPointer (3, GL_FLOAT, 0, vertexArray);

      glDrawElements (GL_TRIANGLES, md5file.meshes[i].num_tris * 3,
		      GL_UNSIGNED_INT, vertexIndices);
    }

  glDisableClientState (GL_VERTEX_ARRAY);

  glutSwapBuffers ();
  glutPostRedisplay ();
}

void
keyboard (unsigned char key, int x, int y)
{
  /* Escape */
  if (key == 27)
    exit (0);
}

int
main (int argc, char *argv[])
{
  if (argc < 2)
    {
      fprintf (stderr, "usage: %s <filename.md5mesh> "
	       "[<filename.md5anim>]\n", argv[0]);
      return 0;
    }

  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize (640, 480);
  glutCreateWindow ("MD5 Model");

  atexit (cleanup);
  init (argv[1], (argc > 2) ? argv[2] : NULL);

  glutReshapeFunc (reshape);
  glutDisplayFunc (display);
  glutKeyboardFunc (keyboard);

  glutMainLoop ();

  return 0;
}
