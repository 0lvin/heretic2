/*  SinView - Sin Pak File Viewer
    Copyright (C) 1998

    Trey Harrison trey@u.washington.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <memory.h>
#include <glide.h>
#include "error.h"
#include "clip_delta.h"
#include "render_window.h"
#include "engine_input.h"
#include "sin_model.h"
#include "v_normals.h"

const float VTX_SNAP = (float)(1L<<19);

//fog table
static GrFog_t fogtable[GR_FOG_TABLE_SIZE];

float r_width_o2;
float r_height_o2;

engine_render_window_class::engine_render_window_class(w32 _width,
                                                       w32 _height,
                                                       w32 _hwnd)
{
  memset(&camera,0,sizeof(vec3));
  memset(&camera_orientation,0,sizeof(vec3));
  memset(&camera_transform,0,sizeof(matrix_class));

  hwnd = _hwnd;
  glide_open = FALSE;

  if (_width>640 || _height>480)
  {
    engine_error("Cant initialize 3dfx resolution","render_window_class()");
  }

  width  = _width;
  height = _height;

  r_width_o2  = width_o2  = (float)width * 0.5f;
  r_height_o2 = height_o2 = (float)height * 0.5f;

  grGlideInit();

  GrHwConfiguration config;
  grSstQueryHardware(&config);

  if (config.num_sst<=0)
  {
    engine_error("Couldnt find your 3dfx board","engine_render_window_class::engine_render_window_class()");
    grGlideShutdown();
    return;
  }

  grSstSelect(0);

  if (!grSstWinOpen(_hwnd,
       GR_RESOLUTION_640x480,
       GR_REFRESH_60Hz,
       GR_COLORFORMAT_ARGB,
       GR_ORIGIN_UPPER_LEFT,
       2, 1 ))
  {
		grGlideShutdown();
		engine_error("Unable to init 3dfx","3DFX Init");
		return;
	}

  grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);

  grDepthBufferFunction(GR_CMP_LEQUAL);
  grDepthMask(FXTRUE);

  //guColorCombineFunction(GR_COLORCOMBINE_ITRGB);
  guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB);
  //guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_ADD_ITRGB);

  grTexCombineFunction(GR_TMU0, GR_TEXTURECOMBINE_DECAL);

  grAlphaCombine(GR_COMBINE_FUNCTION_BLEND_OTHER,GR_COMBINE_FACTOR_ONE,
                 GR_COMBINE_LOCAL_NONE,GR_COMBINE_OTHER_CONSTANT,0);

  grAlphaBlendFunction(GR_BLEND_ONE,GR_BLEND_ZERO,GR_BLEND_ZERO,GR_BLEND_ZERO);

  grTexFilterMode(GR_TMU0,GR_TEXTUREFILTER_BILINEAR,GR_TEXTUREFILTER_BILINEAR);

  //grTexClampMode(GR_TMU0,GR_TEXTURECLAMP_WRAP,GR_TEXTURECLAMP_WRAP);
  grTexClampMode(GR_TMU0,GR_TEXTURECLAMP_CLAMP,GR_TEXTURECLAMP_CLAMP);

  //grFogMode(GR_FOG_WITH_TABLE);
	//grFogColorValue(0x00000000);
	//guFogGenerateExp(fogtable,0.001f);
	//grFogTable(fogtable);

  grCullMode(GR_CULL_NEGATIVE);

  grBufferClear(0x00000000,0,GR_WDEPTHVALUE_FARTHEST);

  glide_open = TRUE;

  //initial light direction
  light_dir.x = 1.f;
  light_dir.y = 1.f;
  light_dir.z = 1.f;
  vec3_normalize(&light_dir, &light_dir);
  l_intensity[0] = 255.f; //r
  l_intensity[1] = 255.f; //g
  l_intensity[2] = 255.f; //b
}

engine_render_window_class::~engine_render_window_class()
{
  if (glide_open)
  {
    glide_open = FALSE;
    grSstWinClose();
    grGlideShutdown();
  }
}

void engine_render_window_class::activate()
{
  grSstControl(GR_CONTROL_ACTIVATE);
}

void engine_render_window_class::deactivate()
{
  grSstControl(GR_CONTROL_DEACTIVATE);
}

void engine_render_window_class::grab_framebuffer(w16 *dst_bitmap)
{
  grSstIdle();
  grLfbReadRegion(GR_BUFFER_BACKBUFFER,
                  0,0,width,height,width*2,
                  dst_bitmap);
}

void engine_render_window_class::flush(bool clear)
{
  while (grBufferNumPending());
	  grBufferSwap(1);

  if (clear)
    grBufferClear(0x00000000,0,GR_WDEPTHVALUE_FARTHEST);
}

const float near_clip_z = 0.001f;

//the flags are kept to avoid recalculating outcodes for a shared vertex
__inline w8 calc_outcode(r_vert *v)
{
  if (v->flags & R_VERT_OUTCODE_CALCULATED) return v->outcode;

  w8 res = 0;

	     if ((v->x)>(v->z))  res |=  1;
	else if ((v->x)<-(v->z)) res |=  2;
	     if ((v->y)>(v->z))  res |=  4;
	else if ((v->y)<-(v->z)) res |=  8;
	     if ((v->z)<near_clip_z)     res |= 16;

  v->outcode = res;
  v->flags   |= R_VERT_OUTCODE_CALCULATED;

  return res;
}

void engine_render_window_class::draw_object(sin_model_class *o)
{
  if (!o->vertices)
    return;

  matrix_class temp,model_transform,w_transform;

  //calculate the model->worldspace transform
  w_transform.identity();
  temp.translate(o->origin.x, o->origin.y, o->origin.z);
  w_transform.multiply(temp);
  temp.rotate_z(o->orientation.z * 3.1415927f/180.f);
  w_transform.multiply(temp);

  //calculate the model->viewspace transform
  //(concatenate the w_transform on the camera_transform)
  model_transform.multiply(camera_transform, w_transform);

  //find the light vector in object space
  vec3 object_light_dir;
  w_transform.inverse_transform_3x3(light_dir, object_light_dir);

  SetIntelPrecision(); //this could probably go higher in the pipeline
                       //but something seems to be reseting the fpu status word

  //keep the frame value valid
	sw32 frame      = o->frame % o->num_frames;
	sw32 next_frame = (frame+1) % o->num_frames;

  vec3_t *frame1_verts = o->vertices[frame];
  vec3_t *frame2_verts = o->vertices[next_frame];

  sw32 i;
  for (i=0; i<o->num_verts; i++)
  {
    //interpolate
    vec3 diff_vec,p;
    vec3_sub(&frame2_verts[i], &frame1_verts[i], &diff_vec);
    vec3_scale(&diff_vec, &o->ratio, &diff_vec);

    vec3_add(&frame1_verts[i], &diff_vec, &p);

    //transform each interpolated point
    model_transform.transform(p, *(vec3 *)&o->t_vertices[i]);
    //o->t_vertices[i].x = -o->t_vertices[i].x;
    o->t_vertices[i].flags = 0;

    //calculate the lighting values for each vertex
    vec3 *v1_norm,*v2_norm;
    scalar l,l1,l2;

    v1_norm = (vec3 *)&v_normals[o->normal_indices[frame][i]];
    v2_norm = (vec3 *)&v_normals[o->normal_indices[next_frame][i]];

    l1 = dot_product(v1_norm, &object_light_dir);
    l2 = dot_product(v2_norm, &object_light_dir);

    //lerp the lighting
    l = 0.6f + (l1 + (l2-l1)*o->ratio)*0.4f;
    if (l<0.f) l=0.f;
    else
    if (l>1.f) l=1.f;

    o->t_vertices[i].r = l * l_intensity[0];
    o->t_vertices[i].g = l * l_intensity[1];
    o->t_vertices[i].b = l * l_intensity[2];
  }

  //draw each tri group
  for (i=0; i<o->num_tri_groups; i++)
  {
    //pointer to triangles in group i
    sin_triangle_t *tri = o->triangles[i];

    sw32 j;
    for (j=0; j<o->num_triangles[i]; j++,tri++)
    {
      vertex_info v_info[3];

      sw32 k;
      for (k=0; k<3; k++)
      {
        //get the light and st values
        v_info[k].r = o->t_vertices[tri->index_xyz[k]].r;
        v_info[k].g = o->t_vertices[tri->index_xyz[k]].g;
        v_info[k].b = o->t_vertices[tri->index_xyz[k]].b;

        v_info[k].s = o->st_verts[tri->index_st[k]].s * s_scale;
        v_info[k].t = o->st_verts[tri->index_st[k]].t * t_scale;
      }

      clip_and_draw_tri(tri, o->t_vertices, v_info);
    }
  }
}

r_vert clip_buf_1[64]; //temporary storage for clipped vertices
r_vert clip_buf_2[64]; //temporary storage for clipped vertices

void engine_render_window_class::clip_and_draw_tri(sin_triangle_t *face,
                                                   r_vert *t_vertices,
                                                   vertex_info *v_info)
{
  w8 ORCODE  = 0;
  w8 ANDCODE = 0xFF;

  sw32 i,j;

  //setup clipping delta info as well as aliases
  clip_delta_class clip_deltas;
  scalar &dx = clip_deltas.dx;
  scalar &dy = clip_deltas.dy;
  scalar &dz = clip_deltas.dz;
  scalar &dr = clip_deltas.dr;
  scalar &dg = clip_deltas.dg;
  scalar &db = clip_deltas.db;
  scalar &da = clip_deltas.da;
  scalar &ds = clip_deltas.ds;
  scalar &dt = clip_deltas.dt;

  r_vert *vertices     = clip_buf_1;
  sw32    num_vertices = 3;

  r_vert *clip_vertices     = clip_buf_2;
  sw32    num_clip_vertices = 0;

  r_vert  clip_point;

  for (i=0; i<num_vertices; i++)
  {
    r_vert       *v     = &t_vertices[face->index_xyz[i]];

    w8 outcode = calc_outcode(v); //this outcode is stored in the
                                  //object vertex list so that if
                                  //vertices are shared, outcodes
                                  //arent re-calculated
    vertices[i].outcode = outcode;

    vertices[i].x = v->x;
    vertices[i].y = v->y;
    vertices[i].z = v->z;
    vertices[i].r = v_info[i].r;
    vertices[i].g = v_info[i].g;
    vertices[i].b = v_info[i].b;
    vertices[i].a = v_info[i].a;
    vertices[i].s = v_info[i].s;
    vertices[i].t = v_info[i].t;

    ORCODE  |= outcode;
    ANDCODE &= outcode;
  }

  if (ANDCODE) //nothing to draw
    return;

  if (!ORCODE) //nothing to clip
  {
    draw_poly(num_vertices,vertices);
    return;
  }

  w8 c0,c1   = 0;
  w8 bitmask = 0;
  scalar t;

  for (bitmask=16;bitmask;bitmask=bitmask>>1)
  {
    if ((bitmask&ORCODE)==0) continue;

    for (i=0;i<num_vertices;i++)
    {
      if (i==num_vertices-1)
        j = 0;
      else
        j = i+1;

      c0 = bitmask & vertices[i].outcode;
      c1 = bitmask & vertices[j].outcode;

      //if c0 is not outside of this plane,
      //add it
      if (c0==0) {
        clip_vertices[num_clip_vertices] = vertices[i];
        num_clip_vertices++;
      }

      //if they are on the same
      //side, move to the next vert
      if (c0==c1) continue;

      //otherwise, generate a clipped
      //point

      clip_deltas.calculate(&vertices[j],&vertices[i]);

      switch (bitmask) {
        case 1:  t = (-vertices[i].x + vertices[i].z) / (dx - dz);
                 clip_point.y = vertices[i].y + (t * dy);
                 clip_point.z = vertices[i].z + (t * dz);

                 clip_point.x = clip_point.z;
                 break;

        case 2:  t = ( vertices[i].x + vertices[i].z) / (-dx - dz);
                 clip_point.y = vertices[i].y + (t * dy);
                 clip_point.z = vertices[i].z + (t * dz);

                 clip_point.x = -clip_point.z;
                 break;

        case 4:  t = (-vertices[i].y + vertices[i].z) / ( dy - dz);
                 clip_point.x = vertices[i].x + (t * dx);
                 clip_point.z = vertices[i].z + (t * dz);

                 clip_point.y = clip_point.z;
                 break;

        case 8:  t = ( vertices[i].y + vertices[i].z) / (-dy - dz);
                 clip_point.x = vertices[i].x + (t * dx);
                 clip_point.z = vertices[i].z + (t * dz);

                 clip_point.y = -clip_point.z;
                 break;

        case 16: t = (near_clip_z - vertices[i].z) / (dz);
                 clip_point.x = vertices[i].x + (t * dx);
                 clip_point.y = vertices[i].y + (t * dy);

                 clip_point.z = near_clip_z;
                 break;
      }

      clip_deltas.generate_point_extras(&clip_point,&vertices[i],&t);

      ORCODE |= calc_outcode(&clip_point);

      clip_vertices[num_clip_vertices] = clip_point;
      num_clip_vertices++;
    }

    num_vertices      = num_clip_vertices;
    num_clip_vertices = 0;

    if (vertices==clip_buf_1)
    {
      vertices      = clip_buf_2;
      clip_vertices = clip_buf_1;
    }
    else
    {
      vertices      = clip_buf_1;
      clip_vertices = clip_buf_2;
    }
  }
  if (num_vertices>=3)
    draw_poly(num_vertices,vertices);
}

GrVertex temp_grvertex_buf[64]; //temporary storage for rendering vertices

//macro for making a GrVertex from a vec3 and a r_vert (r_vert used for argbst)
__inline void build_GrVertex(vec3 *a, GrVertex *b, r_vert *r)
{
  float oozBG;// = 1.f / a->z;
  _asm
  {
    mov  eax,dword ptr [a]
    fld1
    fld dword ptr [eax+8] //load z
    fdiv
  }

  b->r = r->r;
  b->g = r->g;
  b->b = r->b;
  b->a = r->a;

  _asm //finish the divide
  {
   fstp dword ptr [oozBG]
  }

  float pxBG = a->x * oozBG * r_width_o2 + r_width_o2;
  float pyBG = a->y * oozBG * r_height_o2 + r_height_o2;
  pxBG += VTX_SNAP; pxBG -= VTX_SNAP;
  pyBG += VTX_SNAP; pyBG -= VTX_SNAP;
  b->x = pxBG;
  b->y = pyBG;

  //hardcoded scale of oow to fit more within allowed range
  b->oow = oozBG*0.03f;
  b->tmuvtx[0].sow = r->s * b->oow;
  b->tmuvtx[0].tow = r->t * b->oow;
}

void engine_render_window_class::draw_poly(sw32 num_vertices,
                                           r_vert *r_list)
{
  //this can be called with a regular list of vertices
  //or with an indexed list + vertex info for each vertex

  sw32   i;

  r_vert *r = r_list;
  for (i=0; i<num_vertices; i++,r++)
  {
    build_GrVertex((vec3 *)r, &temp_grvertex_buf[i], r);
  }

  if (main_input && main_input->key_down('W'))
  {
    sw32 j;
    for (i=0; i<num_vertices; i++)
    {
      if (i==num_vertices-1)
        j=0;
      else
        j=i+1;

      grDrawLine(temp_grvertex_buf+i, temp_grvertex_buf+j);
    }
  }
  else
  {
    grDrawPolygonVertexList(num_vertices,temp_grvertex_buf);
  }
}

void engine_render_window_class::update_camera_transform()
{
  matrix_class temp_matrix,temp_cam;

  temp_cam.identity();

  temp_matrix.rotate_x(camera_orientation.x);
  temp_cam.multiply(temp_matrix);
  temp_matrix.rotate_z(camera_orientation.z);
  temp_cam.multiply(temp_matrix);

  temp_matrix.translate(-camera.x, -camera.y, -camera.z);
  temp_cam.multiply(temp_matrix);

  //put it into the correct view system
  //(z is forward/back, y up/down, x left/right)
  camera_transform.x.x = temp_cam.x.x;
  camera_transform.y.x = temp_cam.y.x;
  camera_transform.z.x = temp_cam.z.x;
  camera_transform.t.x = temp_cam.t.x;

  camera_transform.x.y = -temp_cam.x.z;
  camera_transform.y.y = -temp_cam.y.z;
  camera_transform.z.y = -temp_cam.z.z;
  camera_transform.t.y = -temp_cam.t.z;

  camera_transform.x.z = temp_cam.x.y;
  camera_transform.y.z = temp_cam.y.y;
  camera_transform.z.z = temp_cam.z.y;
  camera_transform.t.z = temp_cam.t.y;
}
