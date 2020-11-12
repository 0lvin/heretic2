/*
 * Copyright (C) 2016,2017 Edd Biddulph
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * Refresher setup and main part of the frame generation
 *
 * =======================================================================
 */

#include "header/local.h"

#define NUM_BEAM_SEGS 6

viddef_t vid;
model_t *r_worldmodel;

float gldepthmin, gldepthmax;

glconfig_t gl_config;
glstate_t gl_state;

image_t *r_notexture; /* use for bad textures */
image_t *r_particletexture; /* little dot for particles */

entity_t *currententity;
model_t *currentmodel;

cplane_t frustum[4];

int r_visframecount; /* bumped when going to a new PVS */
int r_framecount; /* used for dlight push checking */

int c_brush_polys, c_alias_polys;

float v_blend[4]; /* final blending color */

void R_Strings(void);

/* view origin */
vec3_t vup;
vec3_t vpn;
vec3_t vright;
vec3_t r_origin;

float r_world_matrix[16];
float r_base_world_matrix[16];

/* screen size info */
refdef_t r_newrefdef;

int r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;
extern qboolean have_stencil;
unsigned r_rawpalette[256];

cvar_t *gl_norefresh;
cvar_t *gl_drawentities;
cvar_t *gl_drawworld;
cvar_t *gl_speeds;
cvar_t *gl_fullbright;
cvar_t *gl_novis;
cvar_t *gl_lerpmodels;
cvar_t *gl_lefthand;
cvar_t *gl_farsee;

cvar_t *gl_lightlevel;
cvar_t *gl_overbrightbits;

cvar_t *gl_particle_min_size;
cvar_t *gl_particle_max_size;
cvar_t *gl_particle_size;
cvar_t *gl_particle_att_a;
cvar_t *gl_particle_att_b;
cvar_t *gl_particle_att_c;

cvar_t *gl_palettedtexture;
cvar_t *gl_multitexture;
cvar_t *gl_pointparameters;
cvar_t *gl_mtexcombine;

cvar_t *gl_drawbuffer;
cvar_t *gl_lightmap;
cvar_t *gl_shadows;
cvar_t *gl_stencilshadow;
cvar_t *gl_mode;

cvar_t *gl_customwidth;
cvar_t *gl_customheight;

cvar_t *gl_retexturing;

cvar_t *gl_dynamic;
cvar_t *gl_modulate;
cvar_t *gl_nobind;
cvar_t *gl_round_down;
cvar_t *gl_picmip;
cvar_t *gl_showtris;
cvar_t *gl_ztrick;
cvar_t *gl_zfix;
cvar_t *gl_finish;
cvar_t *gl_clear;
cvar_t *gl_cull;
cvar_t *gl_polyblend;
cvar_t *gl_flashblend;
cvar_t *gl_saturatelighting;
cvar_t *gl_swapinterval;
cvar_t *gl_texturemode;
cvar_t *gl_texturealphamode;
cvar_t *gl_texturesolidmode;
cvar_t *gl_anisotropic;
cvar_t *gl_lockpvs;
cvar_t *gl_msaa_samples;

//cvar_t *vid_fullscreen;
//cvar_t *vid_gamma;

static cvar_t *gl_stereo;
static cvar_t *gl_stereo_separation;
cvar_t *gl_stereo_anaglyph_colors;
static cvar_t *gl_stereo_convergence;

/*
 * Returns true if the box is completely outside the frustom
 */
qboolean
R_CullBox(vec3_t mins, vec3_t maxs)
{
	int i;

	if (!gl_cull->value)
	{
		return false;
	}

	for (i = 0; i < 4; i++)
	{
		if (BOX_ON_PLANE_SIDE(mins, maxs, &frustum[i]) == 2)
		{
			return true;
		}
	}

	return false;
}

void
R_RotateForEntity(entity_t *e)
{
	glTranslatef(e->origin[0], e->origin[1], e->origin[2]);

	glRotatef(e->angles[1], 0, 0, 1);
	glRotatef(-e->angles[0], 0, 1, 0);
	glRotatef(-e->angles[2], 1, 0, 0);
}

void
R_DrawSpriteModel(entity_t *e)
{
	float alpha = 1.0F;
    vec3_t point[4];
	dsprframe_t *frame;
	float *up, *right;
	dsprite_t *psprite;

	/* don't even bother culling, because it's just
	   a single polygon without a surface cache */
	psprite = (dsprite_t *)currentmodel->extradata;

	e->frame %= psprite->numframes;
	frame = &psprite->frames[e->frame];

	/* normal sprite */
	up = vup;
	right = vright;

	if (e->flags & RF_TRANSLUCENT)
	{
		alpha = e->alpha;
	}

	if (alpha != 1.0F)
	{
		glEnable(GL_BLEND);
	}

	glColor4f(1, 1, 1, alpha);

	R_Bind(currentmodel->skins[e->frame]->texnum);

	R_TexEnv(GL_MODULATE);

	if (alpha == 1.0)
	{
		glEnable(GL_ALPHA_TEST);
	}
	else
	{
		glDisable(GL_ALPHA_TEST);
	}

	GLfloat tex[] = {
		0, 1,
		0, 0,
		1, 0,
		1, 1
	};

	VectorMA( e->origin, -frame->origin_y, up, point[0] );
	VectorMA( point[0], -frame->origin_x, right, point[0] );

	VectorMA( e->origin, frame->height - frame->origin_y, up, point[1] );
	VectorMA( point[1], -frame->origin_x, right, point[1] );

	VectorMA( e->origin, frame->height - frame->origin_y, up, point[2] );
	VectorMA( point[2], frame->width - frame->origin_x, right, point[2] );

	VectorMA( e->origin, -frame->origin_y, up, point[3] );
	VectorMA( point[3], frame->width - frame->origin_x, right, point[3] );

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, point );
	glTexCoordPointer( 2, GL_FLOAT, 0, tex );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	glDisable(GL_ALPHA_TEST);
	R_TexEnv(GL_REPLACE);

	if (alpha != 1.0F)
	{
		glDisable(GL_BLEND);
	}

	glColor4f(1, 1, 1, 1);
}

void
R_DrawNullModel(void)
{
	vec3_t shadelight;

	if (currententity->flags & RF_FULLBRIGHT)
	{
		shadelight[0] = shadelight[1] = shadelight[2] = 1.0F;
	}
	else
	{
		R_LightPoint(currententity->origin, shadelight);
	}

	glPushMatrix();
	R_RotateForEntity(currententity);

	glDisable(GL_TEXTURE_2D);
	glColor4f( shadelight[0], shadelight[1], shadelight[2], 1 );

    GLfloat vtxA[] = {
        0, 0, -16,
        16 * cos( 0 * M_PI / 2 ), 16 * sin( 0 * M_PI / 2 ), 0,
        16 * cos( 1 * M_PI / 2 ), 16 * sin( 1 * M_PI / 2 ), 0,
        16 * cos( 2 * M_PI / 2 ), 16 * sin( 2 * M_PI / 2 ), 0,
        16 * cos( 3 * M_PI / 2 ), 16 * sin( 3 * M_PI / 2 ), 0,
        16 * cos( 4 * M_PI / 2 ), 16 * sin( 4 * M_PI / 2 ), 0
    };

    glEnableClientState( GL_VERTEX_ARRAY );

    glVertexPointer( 3, GL_FLOAT, 0, vtxA );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 6 );

    glDisableClientState( GL_VERTEX_ARRAY );

	GLfloat vtxB[] = {
		0, 0, 16,
		16 * cos( 4 * M_PI / 2 ), 16 * sin( 4 * M_PI / 2 ), 0,
		16 * cos( 3 * M_PI / 2 ), 16 * sin( 3 * M_PI / 2 ), 0,
		16 * cos( 2 * M_PI / 2 ), 16 * sin( 2 * M_PI / 2 ), 0,
		16 * cos( 1 * M_PI / 2 ), 16 * sin( 1 * M_PI / 2 ), 0,
		16 * cos( 0 * M_PI / 2 ), 16 * sin( 0 * M_PI / 2 ), 0
	};

	glEnableClientState( GL_VERTEX_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, vtxB );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 6 );

	glDisableClientState( GL_VERTEX_ARRAY );

	glColor4f(1, 1, 1, 1);
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

void
R_DrawEntitiesOnList(void)
{
	int i;

	if (!gl_drawentities->value)
	{
		return;
	}

	/* draw non-transparent first */
	for (i = 0; i < r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];

		if (currententity->flags & RF_TRANSLUCENT)
		{
			continue; /* solid */
		}

		if (currententity->flags & RF_BEAM)
		{
			R_DrawBeam(currententity);
		}
		else
		{
			currentmodel = currententity->model;

			if (!currentmodel)
			{
				R_DrawNullModel();
				continue;
			}

			switch (currentmodel->type)
			{
				case mod_alias:
					R_DrawAliasModel(currententity);
					break;
				case mod_brush:
					R_DrawBrushModel(currententity);
					break;
				case mod_sprite:
					R_DrawSpriteModel(currententity);
					break;
				default:
					VID_Error(ERR_DROP, "Bad modeltype");
					break;
			}
		}
	}

	if (gl_pt_enable->value)
	{
		R_CaptureWorldForTAA();
	}

	/* draw transparent entities
	   we could sort these if it ever
	   becomes a problem... */
	glDepthMask(0);

	for (i = 0; i < r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];

		if (!(currententity->flags & RF_TRANSLUCENT))
		{
			continue; /* solid */
		}

		if (currententity->flags & RF_BEAM)
		{
			R_DrawBeam(currententity);
		}
		else
		{
			currentmodel = currententity->model;

			if (!currentmodel)
			{
				R_DrawNullModel();
				continue;
			}

			switch (currentmodel->type)
			{
				case mod_alias:
					R_DrawAliasModel(currententity);
					break;
				case mod_brush:
					R_DrawBrushModel(currententity);
					break;
				case mod_sprite:
					R_DrawSpriteModel(currententity);
					break;
				default:
					VID_Error(ERR_DROP, "Bad modeltype");
					break;
			}
		}
	}

	glDepthMask(1); /* back to writing */
}

void
R_DrawParticles2(int num_particles, const particle_t particles[],
		const unsigned colortable[768])
{
	const particle_t *p;
	int i;
	vec3_t up, right;
	float scale;
	byte color[4];
 
	GLfloat vtx[3*num_particles*3];
	GLfloat tex[2*num_particles*3];
	GLfloat clr[4*num_particles*3];
	unsigned int index_vtx = 0;
	unsigned int index_tex = 0;
	unsigned int index_clr = 0;
	unsigned int j;
 
	R_Bind(r_particletexture->texnum);
	glDepthMask(GL_FALSE); /* no z buffering */
	glEnable(GL_BLEND);
	R_TexEnv(GL_MODULATE);

	VectorScale( vup, 1.5, up );
	VectorScale( vright, 1.5, right );

	for ( p = particles, i = 0; i < num_particles; i++, p++ )
	{
		/* hack a scale up to keep particles from disapearing */
		scale = ( p->origin [ 0 ] - r_origin [ 0 ] ) * vpn [ 0 ] +
			( p->origin [ 1 ] - r_origin [ 1 ] ) * vpn [ 1 ] +
			( p->origin [ 2 ] - r_origin [ 2 ] ) * vpn [ 2 ];

		if ( scale < 20 )
		{
			scale = 1;
		}
		else
		{
			scale = 1 + scale * 0.004;
		}

		*(int *) color = colortable [ p->color ];

		for (j=0; j<3; j++) // Copy the color for each point
		{
			clr[index_clr++] = color[0]/255.0f;
			clr[index_clr++] = color[1]/255.0f;
			clr[index_clr++] = color[2]/255.0f;
			clr[index_clr++] = p->alpha;
		}

		// point 0
		tex[index_tex++] = 0.0625f;
		tex[index_tex++] = 0.0625f;

		vtx[index_vtx++] = p->origin[0];
		vtx[index_vtx++] = p->origin[1];
		vtx[index_vtx++] = p->origin[2];

		// point 1
		tex[index_tex++] = 1.0625f;
		tex[index_tex++] = 0.0625f;

		vtx[index_vtx++] = p->origin [ 0 ] + up [ 0 ] * scale;
		vtx[index_vtx++] = p->origin [ 1 ] + up [ 1 ] * scale;
		vtx[index_vtx++] = p->origin [ 2 ] + up [ 2 ] * scale;

		// point 2
		tex[index_tex++] = 0.0625f;
		tex[index_tex++] = 1.0625f;

		vtx[index_vtx++] = p->origin [ 0 ] + right [ 0 ] * scale;
		vtx[index_vtx++] = p->origin [ 1 ] + right [ 1 ] * scale;
		vtx[index_vtx++] = p->origin [ 2 ] + right [ 2 ] * scale;
	}

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, vtx );
	glTexCoordPointer( 2, GL_FLOAT, 0, tex );
	glColorPointer( 4, GL_FLOAT, 0, clr );
	glDrawArrays( GL_TRIANGLES, 0, num_particles*3 );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
 
	glDisable(GL_BLEND);
	glColor4f(1, 1, 1, 1);
	glDepthMask(1); /* back to normal Z buffering */
	R_TexEnv(GL_REPLACE);
}

void
R_DrawParticles(void)
{
	qboolean stereo_split_tb = ((gl_state.stereo_mode == STEREO_SPLIT_VERTICAL) && gl_state.camera_separation);
	qboolean stereo_split_lr = ((gl_state.stereo_mode == STEREO_SPLIT_HORIZONTAL) && gl_state.camera_separation);

	if (gl_config.pointparameters && !(stereo_split_tb || stereo_split_lr))
	{
		int i;
		unsigned char color[4];
		const particle_t *p;
 
		GLfloat vtx[3*r_newrefdef.num_particles];
		GLfloat clr[4*r_newrefdef.num_particles];
		unsigned int index_vtx = 0;
		unsigned int index_clr = 0;
  
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);

		glPointSize(LittleFloat(gl_particle_size->value));

		for ( i = 0, p = r_newrefdef.particles; i < r_newrefdef.num_particles; i++, p++ )
		{
			*(int *) color = d_8to24table [ p->color & 0xFF ];
			clr[index_clr++] = color[0]/255.0f;
			clr[index_clr++] = color[1]/255.0f;
			clr[index_clr++] = color[2]/255.0f;
			clr[index_clr++] = p->alpha;

			vtx[index_vtx++] = p->origin[0];
			vtx[index_vtx++] = p->origin[1];
			vtx[index_vtx++] = p->origin[2];
		}

		glEnableClientState( GL_VERTEX_ARRAY );
		glEnableClientState( GL_COLOR_ARRAY );

		glVertexPointer( 3, GL_FLOAT, 0, vtx );
		glColorPointer( 4, GL_FLOAT, 0, clr );
		glDrawArrays( GL_POINTS, 0, r_newrefdef.num_particles );

		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_COLOR_ARRAY );

		glDisable(GL_BLEND);
		glColor4f( 1, 1, 1, 1 );
		glDepthMask(GL_TRUE);
		glEnable(GL_TEXTURE_2D);
	}
	else
	{
		R_DrawParticles2(r_newrefdef.num_particles,
				r_newrefdef.particles, d_8to24table);
	}
}

void
R_PolyBlend(void)
{
	if (!gl_polyblend->value)
	{
		return;
	}

	if (!v_blend[3])
	{
		return;
	}

	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	glLoadIdentity();

	glRotatef(-90, 1, 0, 0); /* put Z going up */
	glRotatef(90, 0, 0, 1); /* put Z going up */

	glColor4f( v_blend[0], v_blend[1], v_blend[2], v_blend[3] );

	GLfloat vtx[] = {
		10, 100, 100,
		10, -100, 100,
		10, -100, -100,
		10, 100, -100
	};

	glEnableClientState( GL_VERTEX_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, vtx );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

	glDisableClientState( GL_VERTEX_ARRAY );

	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);

	glColor4f(1, 1, 1, 1);
}

int
R_SignbitsForPlane(cplane_t *out)
{
	int bits, j;

	/* for fast box on planeside test */
	bits = 0;

	for (j = 0; j < 3; j++)
	{
		if (out->normal[j] < 0)
		{
			bits |= 1 << j;
		}
	}

	return bits;
}

void
R_SetFrustum(void)
{
	int i;

	/* rotate VPN right by FOV_X/2 degrees */
	RotatePointAroundVector(frustum[0].normal, vup, vpn,
			-(90 - r_newrefdef.fov_x / 2));
	/* rotate VPN left by FOV_X/2 degrees */
	RotatePointAroundVector(frustum[1].normal,
			vup, vpn, 90 - r_newrefdef.fov_x / 2);
	/* rotate VPN up by FOV_X/2 degrees */
	RotatePointAroundVector(frustum[2].normal,
			vright, vpn, 90 - r_newrefdef.fov_y / 2);
	/* rotate VPN down by FOV_X/2 degrees */
	RotatePointAroundVector(frustum[3].normal, vright, vpn,
			-(90 - r_newrefdef.fov_y / 2));

	for (i = 0; i < 4; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct(r_origin, frustum[i].normal);
		frustum[i].signbits = R_SignbitsForPlane(&frustum[i]);
	}
}

void
R_SetupFrame(void)
{
	int i;
	mleaf_t *leaf;

	r_framecount++;

	/* build the transformation matrix for the given view angles */
	VectorCopy(r_newrefdef.vieworg, r_origin);

	AngleVectors(r_newrefdef.viewangles, vpn, vright, vup);

	/* current viewcluster */
	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
	{
		r_oldviewcluster = r_viewcluster;
		r_oldviewcluster2 = r_viewcluster2;
		leaf = Mod_PointInLeaf(r_origin, r_worldmodel);
		r_viewcluster = r_viewcluster2 = leaf->cluster;

		/* check above and below so crossing solid water doesn't draw wrong */
		if (!leaf->contents)
		{
			/* look down a bit */
			vec3_t temp;

			VectorCopy(r_origin, temp);
			temp[2] -= 16;
			leaf = Mod_PointInLeaf(temp, r_worldmodel);

			if (!(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2))
			{
				r_viewcluster2 = leaf->cluster;
			}
		}
		else
		{
			/* look up a bit */
			vec3_t temp;

			VectorCopy(r_origin, temp);
			temp[2] += 16;
			leaf = Mod_PointInLeaf(temp, r_worldmodel);

			if (!(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2))
			{
				r_viewcluster2 = leaf->cluster;
			}
		}
	}

	for (i = 0; i < 4; i++)
	{
		v_blend[i] = r_newrefdef.blend[i];
	}

	c_brush_polys = 0;
	c_alias_polys = 0;

	/* clear out the portion of the screen that the NOWORLDMODEL defines */
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
	{
		glEnable(GL_SCISSOR_TEST);
		glClearColor(0.3, 0.3, 0.3, 1);
		glScissor(r_newrefdef.x,
				vid.height - r_newrefdef.height - r_newrefdef.y,
				r_newrefdef.width, r_newrefdef.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1, 0, 0.5, 0.5);
		glDisable(GL_SCISSOR_TEST);
	}
}

void
R_MYgluPerspective(GLdouble fovy, GLdouble aspect,
		GLdouble zNear, GLdouble zFar)
{
	GLdouble xmin, xmax, ymin, ymax;

	ymax = zNear * tan(fovy * M_PI / 360.0);
	ymin = -ymax;

	xmin = ymin * aspect;
	xmax = ymax * aspect;

	xmin += - gl_stereo_convergence->value * (2 * gl_state.camera_separation) / zNear;
	xmax += - gl_stereo_convergence->value * (2 * gl_state.camera_separation) / zNear;

	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

void
R_SetupGL(void)
{
	float screenaspect;
	int x, x2, y2, y, w, h;

	/* set up viewport */
	x = floor(r_newrefdef.x * vid.width / vid.width);
	x2 = ceil((r_newrefdef.x + r_newrefdef.width) * vid.width / vid.width);
	y = floor(vid.height - r_newrefdef.y * vid.height / vid.height);
	y2 = ceil(vid.height -
			  (r_newrefdef.y + r_newrefdef.height) * vid.height / vid.height);

	w = x2 - x;
	h = y - y2;

	qboolean drawing_left_eye = gl_state.camera_separation < 0;
	qboolean stereo_split_tb = ((gl_state.stereo_mode == STEREO_SPLIT_VERTICAL) && gl_state.camera_separation);
	qboolean stereo_split_lr = ((gl_state.stereo_mode == STEREO_SPLIT_HORIZONTAL) && gl_state.camera_separation);

	if(stereo_split_lr) {
		w = w / 2;
		x = drawing_left_eye ? (x / 2) : (x + vid.width) / 2;
	}

	if(stereo_split_tb) {
		h = h / 2;
		y2 = drawing_left_eye ? (y2 + vid.height) / 2 : (y2 / 2);
	}

	glViewport(x, y2, w, h);

	/* set up projection matrix */
	screenaspect = (float)r_newrefdef.width / r_newrefdef.height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (gl_farsee->value == 0)
	{
		R_MYgluPerspective(r_newrefdef.fov_y, screenaspect, 4, 4096);
	}
	else
	{
		R_MYgluPerspective(r_newrefdef.fov_y, screenaspect, 4, 8192);
	}

	glCullFace(GL_FRONT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1, 0, 0); /* put Z going up */
	glRotatef(90, 0, 0, 1); /* put Z going up */
	glRotatef(-r_newrefdef.viewangles[2], 1, 0, 0);
	glRotatef(-r_newrefdef.viewangles[0], 0, 1, 0);
	glRotatef(-r_newrefdef.viewangles[1], 0, 0, 1);
	glTranslatef(-r_newrefdef.vieworg[0], -r_newrefdef.vieworg[1],
			-r_newrefdef.vieworg[2]);

	glGetFloatv(GL_MODELVIEW_MATRIX, r_world_matrix);

	/* set drawing parms */
	if (gl_cull->value)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_DEPTH_TEST);
}

void
R_Clear(void)
{
	// Check whether the stencil buffer needs clearing, and do so if need be.
	GLbitfield stencilFlags = 0;
	if (gl_state.stereo_mode >= STEREO_MODE_ROW_INTERLEAVED && gl_state.stereo_mode <= STEREO_MODE_PIXEL_INTERLEAVED) {
		glClearStencil(0);
		stencilFlags |= GL_STENCIL_BUFFER_BIT;
	}

	if (gl_ztrick->value)
	{
		static int trickframe;

		if (gl_clear->value)
		{
			glClear(GL_COLOR_BUFFER_BIT | stencilFlags);
		}

		trickframe++;

		if (trickframe & 1)
		{
			gldepthmin = 0;
			gldepthmax = 0.49999;
			glDepthFunc(GL_LEQUAL);
		}
		else
		{
			gldepthmin = 1;
			gldepthmax = 0.5;
			glDepthFunc(GL_GEQUAL);
		}
	}
	else
	{
		if (gl_clear->value)
		{
			glClear(GL_COLOR_BUFFER_BIT | stencilFlags | GL_DEPTH_BUFFER_BIT);
		}
		else
		{
			glClear(GL_DEPTH_BUFFER_BIT | stencilFlags);
		}

		gldepthmin = 0;
		gldepthmax = 1;
		glDepthFunc(GL_LEQUAL);
	}

	glDepthRange(gldepthmin, gldepthmax);

	if (gl_zfix->value)
	{
		if (gldepthmax > gldepthmin)
		{
			glPolygonOffset(0.05, 1);
		}
		else
		{
			glPolygonOffset(-0.05, -1);
		}
	}

	/* stencilbuffer shadows */
	if (gl_shadows->value && have_stencil && gl_stencilshadow->value)
	{
		glClearStencil(1);
		glClear(GL_STENCIL_BUFFER_BIT);
	}
}

void
R_Flash(void)
{
	R_PolyBlend();
}

void
R_SetGL2D(void)
{
	int x, w, y, h;
	/* set 2D virtual screen size */
	qboolean drawing_left_eye = gl_state.camera_separation < 0;
	qboolean stereo_split_tb = ((gl_state.stereo_mode == STEREO_SPLIT_VERTICAL) && gl_state.camera_separation);
	qboolean stereo_split_lr = ((gl_state.stereo_mode == STEREO_SPLIT_HORIZONTAL) && gl_state.camera_separation);

	x = 0;
	w = vid.width;
	y = 0;
	h = vid.height;

	if(stereo_split_lr) {
		w =  w / 2;
		x = drawing_left_eye ? 0 : w;
	}

	if(stereo_split_tb) {
		h =  h / 2;
		y = drawing_left_eye ? h : 0;
	}

	glViewport(x, y, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, vid.width, vid.height, 0, -99999, 99999);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glColor4f(1, 1, 1, 1);
}

/*
 * r_newrefdef must be set before the first call
 */
void
R_RenderView(refdef_t *fd)
{
	if ((gl_state.stereo_mode != STEREO_MODE_NONE) && gl_state.camera_separation) {

		qboolean drawing_left_eye = gl_state.camera_separation < 0;
		switch (gl_state.stereo_mode) {
			case STEREO_MODE_ANAGLYPH:
				{

					// Work out the colour for each eye.
					int anaglyph_colours[] = { 0x4, 0x3 }; // Left = red, right = cyan.

					if (strlen(gl_stereo_anaglyph_colors->string) == 2) {
						int eye, colour, missing_bits;
						// Decode the colour name from its character.
						for (eye = 0; eye < 2; ++eye) {
							colour = 0;
							switch (toupper(gl_stereo_anaglyph_colors->string[eye])) {
								case 'B': ++colour; // 001 Blue
								case 'G': ++colour; // 010 Green
								case 'C': ++colour; // 011 Cyan
								case 'R': ++colour; // 100 Red
								case 'M': ++colour; // 101 Magenta
								case 'Y': ++colour; // 110 Yellow
									anaglyph_colours[eye] = colour;
									break;
							}
						}
						// Fill in any missing bits.
						missing_bits = ~(anaglyph_colours[0] | anaglyph_colours[1]) & 0x3;
						for (eye = 0; eye < 2; ++eye) {
							anaglyph_colours[eye] |= missing_bits;
						}
					}

					// Set the current colour.
					glColorMask(
						!!(anaglyph_colours[drawing_left_eye] & 0x4),
						!!(anaglyph_colours[drawing_left_eye] & 0x2),
						!!(anaglyph_colours[drawing_left_eye] & 0x1),
						GL_TRUE
					);
				}
				break;
			case STEREO_MODE_ROW_INTERLEAVED:
			case STEREO_MODE_COLUMN_INTERLEAVED:
			case STEREO_MODE_PIXEL_INTERLEAVED:
				{
					qboolean flip_eyes = true;
					int client_x, client_y;

					//GLimp_GetClientAreaOffset(&client_x, &client_y);
					client_x = 0;
					client_y = 0;

					R_SetGL2D();

					glEnable(GL_STENCIL_TEST);
					glStencilMask(GL_TRUE);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

					glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
					glStencilFunc(GL_NEVER, 0, 1);

					glBegin(GL_QUADS);
					{
						glVertex2i(0, 0);
						glVertex2i(vid.width, 0);
						glVertex2i(vid.width, vid.height);
						glVertex2i(0, vid.height);
					}
					glEnd();

					glStencilOp(GL_INVERT, GL_KEEP, GL_KEEP);
					glStencilFunc(GL_NEVER, 1, 1);

					glBegin(GL_LINES);
					{
						if (gl_state.stereo_mode == STEREO_MODE_ROW_INTERLEAVED || gl_state.stereo_mode == STEREO_MODE_PIXEL_INTERLEAVED) {
							int y;
							for (y = 0; y <= vid.height; y += 2) {
								glVertex2f(0, y - 0.5f);
								glVertex2f(vid.width, y - 0.5f);
							}
							flip_eyes ^= (client_y & 1);
						}

						if (gl_state.stereo_mode == STEREO_MODE_COLUMN_INTERLEAVED || gl_state.stereo_mode == STEREO_MODE_PIXEL_INTERLEAVED) {
							int x;
							for (x = 0; x <= vid.width; x += 2) {
								glVertex2f(x - 0.5f, 0);
								glVertex2f(x - 0.5f, vid.height);
							}
							flip_eyes ^= (client_x & 1);
						}
					}
					glEnd();

					glStencilMask(GL_FALSE);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

					glStencilFunc(GL_EQUAL, drawing_left_eye ^ flip_eyes, 1);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				}
				break;
			default:
				break;
		}
	}


	if (gl_norefresh->value)
	{
		return;
	}

	r_newrefdef = *fd;

	if (!r_worldmodel && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
	{
		VID_Error(ERR_DROP, "R_RenderView: NULL worldmodel");
	}

	if (gl_speeds->value)
	{
		c_brush_polys = 0;
		c_alias_polys = 0;
	}

	R_PushDlights();

	if (gl_finish->value)
	{
		glFinish();
	}

	R_SetupFrame();

	R_SetFrustum();

	R_SetupGL();

	R_MarkLeaves(); /* done here so we know if we're in water */

	if (gl_pt_enable->value)
	{
		R_UpdatePathtracerForCurrentFrame();
		R_DrawPathtracerDepthPrePass();
	}

	R_DrawWorld();

	R_DrawEntitiesOnList();

	R_RenderDlights();

	R_DrawParticles();

	R_DrawAlphaSurfaces();

	R_Flash();

	if (gl_speeds->value)
	{
		VID_Printf(PRINT_ALL, "%4i wpoly %4i epoly %i tex %i lmaps\n",
				c_brush_polys, c_alias_polys, c_visible_textures,
				c_visible_lightmaps);
	}

	switch (gl_state.stereo_mode) {
		case STEREO_MODE_ANAGLYPH:
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			break;
		case STEREO_MODE_ROW_INTERLEAVED:
		case STEREO_MODE_COLUMN_INTERLEAVED:
		case STEREO_MODE_PIXEL_INTERLEAVED:
			glDisable(GL_STENCIL_TEST);
			break;
		default:
			break;
	}
}

enum opengl_special_buffer_modes GL_GetSpecialBufferModeForStereoMode(enum stereo_modes stereo_mode) {
	switch (stereo_mode) {
		case STEREO_MODE_NONE:
		case STEREO_SPLIT_HORIZONTAL:
		case STEREO_SPLIT_VERTICAL:
		case STEREO_MODE_ANAGLYPH:
			return OPENGL_SPECIAL_BUFFER_MODE_NONE;
		case STEREO_MODE_OPENGL:
			return OPENGL_SPECIAL_BUFFER_MODE_STEREO;
		case STEREO_MODE_ROW_INTERLEAVED:
		case STEREO_MODE_COLUMN_INTERLEAVED:
		case STEREO_MODE_PIXEL_INTERLEAVED:
			return OPENGL_SPECIAL_BUFFER_MODE_STENCIL;
	}
	return OPENGL_SPECIAL_BUFFER_MODE_NONE;
}

void
R_SetLightLevel(void)
{
	vec3_t shadelight;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
	{
		return;
	}

	/* save off light value for server to look at */
	R_LightPoint(r_newrefdef.vieworg, shadelight);

	/* pick the greatest component, which should be the
	 * same as the mono value returned by software */
	if (shadelight[0] > shadelight[1])
	{
		if (shadelight[0] > shadelight[2])
		{
			gl_lightlevel->value = 150 * shadelight[0];
		}
		else
		{
			gl_lightlevel->value = 150 * shadelight[2];
		}
	}
	else
	{
		if (shadelight[1] > shadelight[2])
		{
			gl_lightlevel->value = 150 * shadelight[1];
		}
		else
		{
			gl_lightlevel->value = 150 * shadelight[2];
		}
	}
}

void
R_RenderFrame(refdef_t *fd)
{
	R_RenderView(fd);
	R_SetLightLevel();
	R_SetGL2D();
}

void
R_Register(void)
{
	gl_lefthand = Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	gl_farsee = Cvar_Get("gl_farsee", "0", CVAR_LATCH | CVAR_ARCHIVE);
	gl_norefresh = Cvar_Get("gl_norefresh", "0", 0);
	gl_fullbright = Cvar_Get("gl_fullbright", "0", 0);
	gl_drawentities = Cvar_Get("gl_drawentities", "1", 0);
	gl_drawworld = Cvar_Get("gl_drawworld", "1", 0);
	gl_novis = Cvar_Get("gl_novis", "0", 0);
	gl_lerpmodels = Cvar_Get("gl_lerpmodels", "1", 0);
	gl_speeds = Cvar_Get("gl_speeds", "0", 0);

	gl_lightlevel = Cvar_Get("gl_lightlevel", "0", 0);
	gl_overbrightbits = Cvar_Get("gl_overbrightbits", "0", CVAR_ARCHIVE);

	gl_particle_min_size = Cvar_Get("gl_particle_min_size", "2", CVAR_ARCHIVE);
	gl_particle_max_size = Cvar_Get("gl_particle_max_size", "40", CVAR_ARCHIVE);
	gl_particle_size = Cvar_Get("gl_particle_size", "40", CVAR_ARCHIVE);
	gl_particle_att_a = Cvar_Get("gl_particle_att_a", "0.01", CVAR_ARCHIVE);
	gl_particle_att_b = Cvar_Get("gl_particle_att_b", "0.0", CVAR_ARCHIVE);
	gl_particle_att_c = Cvar_Get("gl_particle_att_c", "0.01", CVAR_ARCHIVE);

	gl_modulate = Cvar_Get("gl_modulate", "1", CVAR_ARCHIVE);
	gl_mode = Cvar_Get("gl_mode", "4", CVAR_ARCHIVE);
	gl_lightmap = Cvar_Get("gl_lightmap", "0", 0);
	gl_shadows = Cvar_Get("gl_shadows", "0", CVAR_ARCHIVE);
	gl_stencilshadow = Cvar_Get("gl_stencilshadow", "0", CVAR_ARCHIVE);
	gl_dynamic = Cvar_Get("gl_dynamic", "1", 0);
	gl_nobind = Cvar_Get("gl_nobind", "0", 0);
	gl_round_down = Cvar_Get("gl_round_down", "1", 0);
	gl_picmip = Cvar_Get("gl_picmip", "0", 0);
	gl_showtris = Cvar_Get("gl_showtris", "0", 0);
	gl_ztrick = Cvar_Get("gl_ztrick", "0", 0);
	gl_zfix = Cvar_Get("gl_zfix", "0", 0);
	gl_finish = Cvar_Get("gl_finish", "0", CVAR_ARCHIVE);
	gl_clear = Cvar_Get("gl_clear", "0", 0);
	gl_cull = Cvar_Get("gl_cull", "1", 0);
	gl_polyblend = Cvar_Get("gl_polyblend", "1", 0);
	gl_flashblend = Cvar_Get("gl_flashblend", "0", 0);

	gl_texturemode = Cvar_Get("gl_texturemode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE);
	gl_texturealphamode = Cvar_Get("gl_texturealphamode", "default", CVAR_ARCHIVE);
	gl_texturesolidmode = Cvar_Get("gl_texturesolidmode", "default", CVAR_ARCHIVE);
	gl_anisotropic = Cvar_Get("gl_anisotropic", "0", CVAR_ARCHIVE);
	gl_lockpvs = Cvar_Get("gl_lockpvs", "0", 0);

	gl_palettedtexture = Cvar_Get("gl_palettedtexture", "0", CVAR_ARCHIVE);
	gl_multitexture = Cvar_Get("gl_multitexture", "0", CVAR_ARCHIVE);
	gl_pointparameters = Cvar_Get("gl_pointparameters", "1", CVAR_ARCHIVE);
	gl_mtexcombine = Cvar_Get("gl_mtexcombine", "1", CVAR_ARCHIVE);

	gl_drawbuffer = Cvar_Get("gl_drawbuffer", "GL_BACK", 0);
	gl_swapinterval = Cvar_Get("gl_swapinterval", "1", CVAR_ARCHIVE);

	gl_saturatelighting = Cvar_Get("gl_saturatelighting", "0", 0);

	vid_fullscreen = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = Cvar_Get("vid_gamma", "1.0", CVAR_ARCHIVE);

	gl_customwidth = Cvar_Get("gl_customwidth", "1024", CVAR_ARCHIVE);
	gl_customheight = Cvar_Get("gl_customheight", "768", CVAR_ARCHIVE);
	gl_msaa_samples = Cvar_Get ( "gl_msaa_samples", "0", CVAR_ARCHIVE );

	gl_retexturing = Cvar_Get("gl_retexturing", "1", CVAR_ARCHIVE);


	gl_stereo = Cvar_Get( "gl_stereo", "0", CVAR_ARCHIVE );
	gl_stereo_separation = Cvar_Get( "gl_stereo_separation", "-0.4", CVAR_ARCHIVE );
	gl_stereo_anaglyph_colors = Cvar_Get( "gl_stereo_anaglyph_colors", "rc", CVAR_ARCHIVE );
	gl_stereo_convergence = Cvar_Get( "gl_stereo_convergence", "1", CVAR_ARCHIVE );

	Cmd_AddCommand("imagelist", R_ImageList_f);
	Cmd_AddCommand("screenshot", R_ScreenShot);
	Cmd_AddCommand("modellist", Mod_Modellist_f);
	Cmd_AddCommand("gl_strings", R_Strings);
}

qboolean
R_SetMode(void)
{
	rserr_t err;
	qboolean fullscreen;

	fullscreen = vid_fullscreen->value;

	vid_fullscreen->modified = false;
	gl_mode->modified = false;

	/* a bit hackish approach to enable custom resolutions:
	   Glimp_SetMode needs these values set for mode -1 */
	vid.width = gl_customwidth->value;
	vid.height = gl_customheight->value;

	if ((err = GLimp_SetMode(&vid.width, &vid.height, gl_mode->value,
					 fullscreen)) == rserr_ok)
	{
		if (gl_mode->value == -1)
		{
			gl_state.prev_mode = 4; /* safe default for custom mode */
		}
		else
		{
			gl_state.prev_mode = gl_mode->value;
		}
	}
	else
	{
		if (err == rserr_invalid_fullscreen)
		{
			Cvar_SetValue("vid_fullscreen", 0);
			vid_fullscreen->modified = false;
			VID_Printf(PRINT_ALL, "ref_gl::R_SetMode() - fullscreen unavailable in this mode\n");

			if ((err = GLimp_SetMode(&vid.width, &vid.height, gl_mode->value, false)) == rserr_ok)
			{
				return true;
			}
		}
		else if (err == rserr_invalid_mode)
		{
			Cvar_SetValue("gl_mode", gl_state.prev_mode);
			gl_mode->modified = false;
			VID_Printf(PRINT_ALL, "ref_gl::R_SetMode() - invalid mode\n");
		}

		/* try setting it back to something safe */
		if ((err = GLimp_SetMode(&vid.width, &vid.height, gl_state.prev_mode, false)) != rserr_ok)
		{
			VID_Printf(PRINT_ALL, "ref_gl::R_SetMode() - could not revert to safe mode\n");
			return false;
		}
	}

	return true;
}

qboolean
R_VersionOfGLIsGreaterThanOrEqualTo(int major, int minor)
{
	return gl_config.version_major > major || (gl_config.version_major == major && gl_config.version_minor >= minor);
}

int
R_Init(void *hinstance, void *hWnd)
{
	int j;
	int err;
	extern float r_turbsin[256];

	Swap_Init();

	for (j = 0; j < 256; j++)
	{
		r_turbsin[j] *= 0.5;
	}

	/* Options */
	VID_Printf(PRINT_ALL, "Refresher build options:\n");

	VID_Printf(PRINT_ALL, " + Retexturing support\n");

#ifdef X11GAMMA
	VID_Printf(PRINT_ALL, " + Gamma via X11\n");
#else
	VID_Printf(PRINT_ALL, " - Gamma via X11\n");
#endif

	VID_Printf(PRINT_ALL, "Refresh: " REF_VERSION "\n");

	Draw_GetPalette();

	R_Register();

	/* initialize our QGL dynamic bindings */
	QGL_Init();

	/* initialize OS-specific parts of OpenGL */
	if (!GLimp_Init())
	{
		QGL_Shutdown();
		return -1;
	}

	/* set our "safe" mode */
	gl_state.prev_mode = 4;
	gl_state.stereo_mode = gl_stereo->value;

	/* create the window and set up the context */
	if (!R_SetMode())
	{
		QGL_Shutdown();
		VID_Printf(PRINT_ALL, "ref_gl::R_Init() - could not R_SetMode()\n");
		return -1;
	}

	VID_MenuInit();

	// --------

	/* get our various GL strings */
	VID_Printf(PRINT_ALL, "\nOpenGL setting:\n", gl_config.vendor_string);

	gl_config.vendor_string = (char *)glGetString(GL_VENDOR);
	VID_Printf(PRINT_ALL, "GL_VENDOR: %s\n", gl_config.vendor_string);

	gl_config.renderer_string = (char *)glGetString(GL_RENDERER);
	VID_Printf(PRINT_ALL, "GL_RENDERER: %s\n", gl_config.renderer_string);

	gl_config.version_string = (char *)glGetString(GL_VERSION);
	VID_Printf(PRINT_ALL, "GL_VERSION: %s\n", gl_config.version_string);

	gl_config.extensions_string = (char *)glGetString(GL_EXTENSIONS);
	VID_Printf(PRINT_ALL, "GL_EXTENSIONS: %s\n", gl_config.extensions_string);

	sscanf(gl_config.version_string, "%d.%d", &gl_config.major_version, &gl_config.minor_version);

	if (gl_config.major_version == 1)
	{
		if (gl_config.minor_version < 4)
		{
			QGL_Shutdown();
			VID_Printf(PRINT_ALL, "Support for OpenGL 1.4 is not available\n");

			return -1;
		}
	}

	/* Get the major and minor version numbers of the context. For a context with version
		less than 3.0 this will fail, so that case needs to be checked for. */

	/* First deal with any error which already occured. */
	err = glGetError();

	if (err != GL_NO_ERROR)
	{
		VID_Printf(PRINT_ALL, "Entering R_Init: glGetError() = 0x%x\n", err);
	}

	gl_config.version_major = 0;
	gl_config.version_minor = 0;

	glGetIntegerv(GL_MAJOR_VERSION, &gl_config.version_major);

	err = glGetError();

	if (err != GL_NO_ERROR)
	{
		gl_config.version_major = 0;
		gl_config.version_minor = 0;
		VID_Printf(PRINT_ALL, "\n\nglGetIntegerv(GL_MAJOR_VERSION) failed with glGetError() = 0x%x, GL version is assumed to be less than 3.0", err);
	}
	else
	{
		glGetIntegerv(GL_MINOR_VERSION, &gl_config.version_minor);
		VID_Printf(PRINT_ALL, "\n\nGL version is %d.%d", gl_config.version_major, gl_config.version_minor);
	}

	VID_Printf(PRINT_ALL, "\n\nProbing for OpenGL extensions:\n");

	// ----

	/* Point parameters */
	VID_Printf(PRINT_ALL, " - Point parameters: ");

	if (strstr(gl_config.extensions_string, "GL_ARB_point_parameters"))
	{
			qglPointParameterfARB = (void (APIENTRY *)(GLenum, GLfloat))GLimp_GetProcAddress ( "glPointParameterfARB" );
			qglPointParameterfvARB = (void (APIENTRY *)(GLenum, const GLfloat *))GLimp_GetProcAddress ( "glPointParameterfvARB" );
	}

	gl_config.pointparameters = false;

	if (gl_pointparameters->value)
	{
		if (qglPointParameterfARB && qglPointParameterfvARB)
		{
			gl_config.pointparameters = true;
			VID_Printf(PRINT_ALL, "Okay\n");
		}
		else
		{
			VID_Printf(PRINT_ALL, "Failed\n");
		}
	}
	else
	{
		VID_Printf(PRINT_ALL, "Disabled\n");
	}

	// ----

	/* Paletted texture */
	VID_Printf(PRINT_ALL, " - Paletted texture: ");

	if (strstr(gl_config.extensions_string, "GL_EXT_paletted_texture") &&
		strstr(gl_config.extensions_string, "GL_EXT_shared_texture_palette"))
	{
			qglColorTableEXT = (void (APIENTRY *)(GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid * ))
					GLimp_GetProcAddress ("glColorTableEXT");
	}

	gl_config.palettedtexture = false;

	if (gl_palettedtexture->value)
	{
		if (qglColorTableEXT)
		{
			gl_config.palettedtexture = true;
			VID_Printf(PRINT_ALL, "Okay\n");
		}
		else
		{
			VID_Printf(PRINT_ALL, "Failed\n");
		}
	}
	else
	{
		VID_Printf(PRINT_ALL, "Disabled\n");
	}

	// ----

	/* Multitexturing */
	VID_Printf(PRINT_ALL, " - Multitexturing: ");

	if (strstr(gl_config.extensions_string, "GL_ARB_multitexture"))
	{
		qglMultiTexCoord2fARB = (void *)GLimp_GetProcAddress("glMultiTexCoord2fARB");
		qglMultiTexCoord2fvARB = (void *)GLimp_GetProcAddress("glMultiTexCoord2fvARB");
		qglMultiTexCoord3fARB = (void *)GLimp_GetProcAddress("glMultiTexCoord3fARB");
		qglMultiTexCoord4fARB = (void *)GLimp_GetProcAddress("glMultiTexCoord4fARB");
		qglActiveTextureARB = (void *)GLimp_GetProcAddress("glActiveTextureARB");
		qglClientActiveTextureARB = (void *)GLimp_GetProcAddress("glClientActiveTextureARB");
	}

	gl_config.multitexture = false;

	if (gl_multitexture->value)
	{
		if (qglMultiTexCoord2fARB && qglMultiTexCoord2fvARB && qglActiveTextureARB && qglClientActiveTextureARB)
		{
			gl_config.multitexture = true;
			VID_Printf(PRINT_ALL, "Okay\n");
		}
		else
		{
			VID_Printf(PRINT_ALL, "Failed\n");
		}
	}
	else
	{
		VID_Printf(PRINT_ALL, "Disabled\n");
	}

	// ----

	/* Multi texturing combine */
	VID_Printf(PRINT_ALL, " - Multi texturing combine: ");

	if (strstr(gl_config.extensions_string, "GL_ARB_texture_env_combine") && gl_config.multitexture)
	{
		if (gl_mtexcombine->value)
		{
			gl_config.mtexcombine = true;
			VID_Printf(PRINT_ALL, "Okay\n");
		}
		else
		{
			VID_Printf(PRINT_ALL, "Disabled\n");
		}
	}
	else
	{
		VID_Printf(PRINT_ALL, "Failed\n");
	}

	// --------

	/* Anisotropic */
	VID_Printf(PRINT_ALL, " - Anisotropic: ");

	if (strstr(gl_config.extensions_string, "GL_EXT_texture_filter_anisotropic"))
	{
		gl_config.anisotropic = true;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_config.max_anisotropy);

		VID_Printf(PRINT_ALL, "%ux\n", (int)gl_config.max_anisotropy);
	}
	else
	{
		gl_config.anisotropic = false;
		gl_config.max_anisotropy = 0.0;

		VID_Printf(PRINT_ALL, "Failed\n");
	}

	// ----

	/* Non power of two textures */
	VID_Printf(PRINT_ALL, " - Non power of two textures: ");

	if (strstr(gl_config.extensions_string, "GL_ARB_texture_non_power_of_two"))
	{
		gl_config.npottextures = true;
		VID_Printf(PRINT_ALL, "Okay\n");
	}
	else
	{
		gl_config.npottextures = false;
		VID_Printf(PRINT_ALL, "Failed\n");
	}

	// ----

	/* ------------------------- GL_ARB_shader_objects ------------------------- */

#define GET_PROC_ADDRESS(x) q##x = ( void * ) GLimp_GetProcAddress ( #x ); if (!q##x) { VID_Printf(PRINT_ALL, #x " was not found!\n"); return -1; }

	gl_config.shaders = false;

	if (strstr(gl_config.extensions_string, "GL_ARB_shader_objects"))
	{
		VID_Printf(PRINT_ALL, "...using GL_ARB_shader_objects\n");

		gl_config.shaders = true;

		GET_PROC_ADDRESS(glAttachObjectARB);
		GET_PROC_ADDRESS(glCompileShaderARB);
		GET_PROC_ADDRESS(glCreateProgramObjectARB);
		GET_PROC_ADDRESS(glCreateShaderObjectARB);
		GET_PROC_ADDRESS(glDeleteObjectARB);
		GET_PROC_ADDRESS(glDetachObjectARB);
		GET_PROC_ADDRESS(glGetActiveUniformARB);
		GET_PROC_ADDRESS(glGetAttachedObjectsARB);
		GET_PROC_ADDRESS(glGetHandleARB);
		GET_PROC_ADDRESS(glGetInfoLogARB);
		GET_PROC_ADDRESS(glGetObjectParameterfvARB);
		GET_PROC_ADDRESS(glGetObjectParameterivARB);
		GET_PROC_ADDRESS(glGetShaderSourceARB);
		GET_PROC_ADDRESS(glGetUniformLocationARB);
		GET_PROC_ADDRESS(glGetUniformfvARB);
		GET_PROC_ADDRESS(glGetUniformivARB);
		GET_PROC_ADDRESS(glLinkProgramARB);
		GET_PROC_ADDRESS(glShaderSourceARB);
		GET_PROC_ADDRESS(glUniform1fARB);
		GET_PROC_ADDRESS(glUniform1fvARB);
		GET_PROC_ADDRESS(glUniform1iARB);
		GET_PROC_ADDRESS(glUniform1ivARB);
		GET_PROC_ADDRESS(glUniform2fARB);
		GET_PROC_ADDRESS(glUniform2fvARB);
		GET_PROC_ADDRESS(glUniform2iARB);
		GET_PROC_ADDRESS(glUniform2ivARB);
		GET_PROC_ADDRESS(glUniform3fARB);
		GET_PROC_ADDRESS(glUniform3fvARB);
		GET_PROC_ADDRESS(glUniform3iARB);
		GET_PROC_ADDRESS(glUniform3ivARB);
		GET_PROC_ADDRESS(glUniform4fARB);
		GET_PROC_ADDRESS(glUniform4fvARB);
		GET_PROC_ADDRESS(glUniform4iARB);
		GET_PROC_ADDRESS(glUniform4ivARB);
		GET_PROC_ADDRESS(glUniformMatrix2fvARB);
		GET_PROC_ADDRESS(glUniformMatrix3fvARB);
		GET_PROC_ADDRESS(glUniformMatrix4fvARB);
		GET_PROC_ADDRESS(glUseProgramObjectARB);
		GET_PROC_ADDRESS(glValidateProgramARB);

	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_ARB_shader_objects not found\n");
		gl_config.shaders = false;
	}

	CHECK_GL_ERROR();

	/* -------------------------- GL_ARB_vertex_shader ------------------------- */

	gl_config.vertex_shaders = false;

	if (strstr(gl_config.extensions_string, "GL_ARB_vertex_shader"))
	{
		VID_Printf(PRINT_ALL, "...using GL_ARB_vertex_shader\n");

		gl_config.vertex_shaders = true;

		GET_PROC_ADDRESS(glBindAttribLocationARB);
		GET_PROC_ADDRESS(glGetActiveAttribARB);
		GET_PROC_ADDRESS(glGetAttribLocationARB);

	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_ARB_vertex_shader not found\n");
		gl_config.vertex_shaders = false;
	}

	/* -------------------------- GL_ARB_fragment_shader ------------------------- */

	gl_config.fragment_shaders = false;

	if (strstr(gl_config.extensions_string, "GL_ARB_fragment_shader"))
	{
		VID_Printf(PRINT_ALL, "...using GL_ARB_fragment_shader\n");
		gl_config.fragment_shaders = true;
	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_ARB_fragment_shader not found\n");
		gl_config.fragment_shaders = false;
	}

	/* -------------------------- GL_ARB_texture_float ------------------------- */

	gl_config.float_textures = false;

	if (strstr(gl_config.extensions_string, "GL_ARB_texture_float"))
	{
		VID_Printf(PRINT_ALL, "...using GL_ARB_texture_float\n");
		gl_config.float_textures = true;
	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_ARB_texture_float not found\n");
		gl_config.float_textures = false;
	}

	/* -------------------------- GL_ATI_texture_float ------------------------- */

	if (!gl_config.float_textures)
	{
		if (strstr(gl_config.extensions_string, "GL_ATI_texture_float"))
		{
			VID_Printf(PRINT_ALL, "...using GL_ATI_texture_float\n");
			gl_config.float_textures = true;
		}
		else
		{
			VID_Printf(PRINT_ALL, "...GL_ATI_texture_float not found\n");
		}
	}

	/* -------------------------- GL_ARB_texture_buffer_object ------------------------- */

	gl_config.texture_buffer_objects = false;

	if (strstr(gl_config.extensions_string, "GL_ARB_texture_buffer_object"))
	{
		VID_Printf(PRINT_ALL, "...using GL_ARB_texture_buffer_object\n");

		gl_config.texture_buffer_objects = true;

		GET_PROC_ADDRESS(glTexBufferARB);

	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_ARB_texture_buffer_object not found\n");
		gl_config.texture_buffer_objects = false;
	}

	/* -------------------------- GL_EXT_texture_buffer_object ------------------------- */

	if (!gl_config.texture_buffer_objects)
	{
		if (strstr(gl_config.extensions_string, "GL_EXT_texture_buffer_object"))
		{
			VID_Printf(PRINT_ALL, "...using GL_EXT_texture_buffer_object\n");

			gl_config.texture_buffer_objects = true;

			GET_PROC_ADDRESS(glTexBufferEXT);

		}
		else
		{
			VID_Printf(PRINT_ALL, "...GL_EXT_texture_buffer_object not found\n");
			gl_config.texture_buffer_objects = false;
		}
	}

	/* -------------------------- GL_ARB_texture_buffer_object_rgb32 ------------------------- */

	gl_config.texture_buffer_objects_rgb = false;

	if (strstr(gl_config.extensions_string, "GL_ARB_texture_buffer_object_rgb32"))
	{
		VID_Printf(PRINT_ALL, "...using GL_ARB_texture_buffer_object_rgb32\n");
		gl_config.texture_buffer_objects_rgb = true;
	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_ARB_texture_buffer_object_rgb32 not found\n");
		gl_config.texture_buffer_objects_rgb = false;
	}

	/* ----------------------------- GL_VERSION_3_1 ---------------------------- */

	if (R_VersionOfGLIsGreaterThanOrEqualTo(3, 1))
	{
		VID_Printf(PRINT_ALL, "...using OpenGL 3.1 features\n");

		gl_config.texture_buffer_objects = true;

		GET_PROC_ADDRESS(glDrawArraysInstanced);
		GET_PROC_ADDRESS(glDrawElementsInstanced);
		GET_PROC_ADDRESS(glPrimitiveRestartIndex);
		GET_PROC_ADDRESS(glTexBuffer);
	}

	/* ---------------------- GL_ARB_vertex_buffer_object ---------------------- */

	gl_config.vertex_buffer_objects = false;

	if (strstr(gl_config.extensions_string, "GL_ARB_vertex_buffer_object"))
	{
		VID_Printf(PRINT_ALL, "...using GL_ARB_vertex_buffer_object\n");

		gl_config.vertex_buffer_objects = true;

		GET_PROC_ADDRESS(glBindBufferARB);
		GET_PROC_ADDRESS(glBufferDataARB);
		GET_PROC_ADDRESS(glBufferSubDataARB);
		GET_PROC_ADDRESS(glDeleteBuffersARB);
		GET_PROC_ADDRESS(glGenBuffersARB);
		GET_PROC_ADDRESS(glGetBufferParameterivARB);
		GET_PROC_ADDRESS(glGetBufferPointervARB);
		GET_PROC_ADDRESS(glGetBufferSubDataARB);
		GET_PROC_ADDRESS(glIsBufferARB);
		GET_PROC_ADDRESS(glMapBufferARB);
		GET_PROC_ADDRESS(glUnmapBufferARB);

	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_ARB_vertex_buffer_object not found\n");
		gl_config.vertex_buffer_objects = false;
	}

	/* ----------------------------- GL_VERSION_1_5 ---------------------------- */

	if (R_VersionOfGLIsGreaterThanOrEqualTo(1, 5))
	{
		VID_Printf(PRINT_ALL, "...using OpenGL 1.5 features\n");

		gl_config.vertex_buffer_objects = true;

		GET_PROC_ADDRESS(glBeginQuery);
		GET_PROC_ADDRESS(glBindBuffer);
		GET_PROC_ADDRESS(glBufferData);
		GET_PROC_ADDRESS(glBufferSubData);
		GET_PROC_ADDRESS(glDeleteBuffers);
		GET_PROC_ADDRESS(glDeleteQueries);
		GET_PROC_ADDRESS(glEndQuery);
		GET_PROC_ADDRESS(glGenBuffers);
		GET_PROC_ADDRESS(glGenQueries);
		GET_PROC_ADDRESS(glGetBufferParameteriv);
		GET_PROC_ADDRESS(glGetBufferPointerv);
		GET_PROC_ADDRESS(glGetBufferSubData);
		GET_PROC_ADDRESS(glGetQueryObjectiv);
		GET_PROC_ADDRESS(glGetQueryObjectuiv);
		GET_PROC_ADDRESS(glGetQueryiv);
		GET_PROC_ADDRESS(glIsBuffer);
		GET_PROC_ADDRESS(glIsQuery);
		GET_PROC_ADDRESS(glMapBuffer);
		GET_PROC_ADDRESS(glUnmapBuffer);

	}

	/* -------------------------- GL_ARB_texture_rg ------------------------- */

	gl_config.texture_rg = false;

	if (strstr(gl_config.extensions_string, "GL_ARB_texture_rg"))
	{
		VID_Printf(PRINT_ALL, "...using GL_ARB_texture_rg\n");
		gl_config.texture_rg = true;
	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_ARB_texture_rg not found\n");
		gl_config.texture_rg = false;
	}

	/* -------------------------- NV_multisample_filter_hint ------------------------- */

	gl_config.multisample_filter_hint = false;

	if (strstr(gl_config.extensions_string, "NV_multisample_filter_hint"))
	{
		VID_Printf(PRINT_ALL, "...using NV_multisample_filter_hint\n");
		gl_config.multisample_filter_hint = true;
	}
	else
	{
		VID_Printf(PRINT_ALL, "...NV_multisample_filter_hint not found\n");
		gl_config.multisample_filter_hint = false;
	}

	/* ---------------------- GL_ARB_map_buffer_range ---------------------- */

	gl_config.map_buffer_range = false;

	if (strstr(gl_config.extensions_string, "GL_ARB_map_buffer_range"))
	{
		VID_Printf(PRINT_ALL, "...using GL_ARB_map_buffer_range\n");

		gl_config.map_buffer_range = true;

		GET_PROC_ADDRESS(glFlushMappedBufferRange);
		GET_PROC_ADDRESS(glMapBufferRange);

	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_ARB_map_buffer_range not found\n");
		gl_config.map_buffer_range = false;
	}


	/* ----------------------------- GL_VERSION_1_2 ---------------------------- */

	if (R_VersionOfGLIsGreaterThanOrEqualTo(1, 2))
	{
		VID_Printf(PRINT_ALL, "...using OpenGL 1.2 features\n");

		GET_PROC_ADDRESS(glCopyTexSubImage3D);
		GET_PROC_ADDRESS(glDrawRangeElements);
		GET_PROC_ADDRESS(glTexImage3D);
		GET_PROC_ADDRESS(glTexSubImage3D);
	}
#undef GET_PROC_ADDRESS

	CHECK_GL_ERROR();

	R_SetDefaultState();

	R_InitImages();
	Mod_Init();
	R_InitParticleTexture();
	Draw_InitLocal();

	R_InitPathtracing();

	return true;
}

void
R_Shutdown(void)
{
	Cmd_RemoveCommand("modellist");
	Cmd_RemoveCommand("screenshot");
	Cmd_RemoveCommand("imagelist");
	Cmd_RemoveCommand("gl_strings");

	Mod_FreeAll();

	R_ShutdownImages();

	R_ShutdownPathtracing();

	/* shutdown OS specific OpenGL stuff like contexts, etc.  */
	GLimp_Shutdown();

	/* shutdown our QGL subsystem */
	QGL_Shutdown();
}

extern void UpdateHardwareGamma();

void
R_BeginFrame(float camera_separation)
{
	gl_state.camera_separation = camera_separation;

	/* change modes if necessary */
	if (gl_mode->modified)
	{
		vid_fullscreen->modified = true;
	}

	// force a vid_restart if gl_stereo has been modified.
	if ( gl_state.stereo_mode != gl_stereo->value ) {
		// If we've gone from one mode to another with the same special buffer requirements there's no need to restart.
		if ( GL_GetSpecialBufferModeForStereoMode( gl_state.stereo_mode ) == GL_GetSpecialBufferModeForStereoMode( gl_stereo->value )  ) {
			gl_state.stereo_mode = gl_stereo->value;
		}
		else
		{
			VID_Printf(PRINT_ALL, "stereo supermode changed, restarting video!\n");
			cvar_t	*ref;
			ref = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
			ref->modified = true;
		}
	}

	if (vid_gamma->modified)
	{
		vid_gamma->modified = false;

		if (gl_state.hwgamma)
		{
			UpdateHardwareGamma();
		}
	}

	// Clamp overbrightbits
	if (gl_overbrightbits->modified)
	{
		if (gl_overbrightbits->value > 2 && gl_overbrightbits->value < 4)
		{
			Cvar_Set("gl_overbrightbits", "2");
		}
		else if (gl_overbrightbits->value > 4)
		{
			Cvar_Set("gl_overbrightbits", "4");
		}

		gl_overbrightbits->modified = false;
	}

	/* go into 2D mode */

	int x, w, y, h;
	qboolean drawing_left_eye = gl_state.camera_separation < 0;
	qboolean stereo_split_tb = ((gl_state.stereo_mode == STEREO_SPLIT_VERTICAL) && gl_state.camera_separation);
	qboolean stereo_split_lr = ((gl_state.stereo_mode == STEREO_SPLIT_HORIZONTAL) && gl_state.camera_separation);

	x = 0;
	w = vid.width;
	y = 0;
	h = vid.height;

	if(stereo_split_lr) {
		w =  w / 2;
		x = drawing_left_eye ? 0 : w;
	}

	if(stereo_split_tb) {
		h =  h / 2;
		y = drawing_left_eye ? h : 0;
	}

	glViewport(x, y, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, vid.width, vid.height, 0, -99999, 99999);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glColor4f(1, 1, 1, 1);

	/* draw buffer stuff */
	if (gl_drawbuffer->modified)
	{
		gl_drawbuffer->modified = false;

		if ((gl_state.camera_separation == 0) || gl_state.stereo_mode != STEREO_MODE_OPENGL)
		{
			if (Q_stricmp(gl_drawbuffer->string, "GL_FRONT") == 0)
			{
				glDrawBuffer(GL_FRONT);
			}
			else
			{
				glDrawBuffer(GL_BACK);
			}
		}
	}

	/* texturemode stuff */
	if (gl_texturemode->modified || (gl_config.anisotropic && gl_anisotropic->modified))
	{
		R_TextureMode(gl_texturemode->string);
		gl_texturemode->modified = false;
		gl_anisotropic->modified = false;
	}

	if (gl_texturealphamode->modified)
	{
		R_TextureAlphaMode(gl_texturealphamode->string);
		gl_texturealphamode->modified = false;
	}

	if (gl_texturesolidmode->modified)
	{
		R_TextureSolidMode(gl_texturesolidmode->string);
		gl_texturesolidmode->modified = false;
	}

	/* clear screen if desired */
	R_Clear();
}

void
R_SetPalette(const unsigned char *palette)
{
	int i;

	byte *rp = (byte *)r_rawpalette;

	if (palette)
	{
		for (i = 0; i < 256; i++)
		{
			rp[i * 4 + 0] = palette[i * 3 + 0];
			rp[i * 4 + 1] = palette[i * 3 + 1];
			rp[i * 4 + 2] = palette[i * 3 + 2];
			rp[i * 4 + 3] = 0xff;
		}
	}
	else
	{
		for (i = 0; i < 256; i++)
		{
			rp[i * 4 + 0] = LittleLong(d_8to24table[i]) & 0xff;
			rp[i * 4 + 1] = (LittleLong(d_8to24table[i]) >> 8) & 0xff;
			rp[i * 4 + 2] = (LittleLong(d_8to24table[i]) >> 16) & 0xff;
			rp[i * 4 + 3] = 0xff;
		}
	}

	R_SetTexturePalette(r_rawpalette);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(1, 0, 0.5, 0.5);
}

/* R_DrawBeam */
void
R_DrawBeam(entity_t *e)
{
	int i;
	float r, g, b;

	vec3_t perpvec;
	vec3_t direction, normalized_direction;
	vec3_t start_points[NUM_BEAM_SEGS], end_points[NUM_BEAM_SEGS];
	vec3_t oldorigin, origin;
 
	GLfloat vtx[3*NUM_BEAM_SEGS*4];
	unsigned int index_vtx = 0;
	unsigned int pointb;
 
	oldorigin[0] = e->oldorigin[0];
	oldorigin[1] = e->oldorigin[1];
	oldorigin[2] = e->oldorigin[2];

	origin[0] = e->origin[0];
	origin[1] = e->origin[1];
	origin[2] = e->origin[2];

	normalized_direction[0] = direction[0] = oldorigin[0] - origin[0];
	normalized_direction[1] = direction[1] = oldorigin[1] - origin[1];
	normalized_direction[2] = direction[2] = oldorigin[2] - origin[2];

	if (VectorNormalize(normalized_direction) == 0)
	{
		return;
	}

	PerpendicularVector(perpvec, normalized_direction);
	VectorScale(perpvec, e->frame / 2, perpvec);

	for (i = 0; i < 6; i++)
	{
		RotatePointAroundVector(start_points[i], normalized_direction, perpvec,
				(360.0 / NUM_BEAM_SEGS) * i);
		VectorAdd(start_points[i], origin, start_points[i]);
		VectorAdd(start_points[i], direction, end_points[i]);
	}

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	r = (LittleLong(d_8to24table[e->skinnum & 0xFF])) & 0xFF;
	g = (LittleLong(d_8to24table[e->skinnum & 0xFF]) >> 8) & 0xFF;
	b = (LittleLong(d_8to24table[e->skinnum & 0xFF]) >> 16) & 0xFF;

	r *= 1 / 255.0F;
	g *= 1 / 255.0F;
	b *= 1 / 255.0F;

	glColor4f(r, g, b, e->alpha);

	for ( i = 0; i < NUM_BEAM_SEGS; i++ )
	{
		vtx[index_vtx++] = start_points [ i ][ 0 ];
		vtx[index_vtx++] = start_points [ i ][ 1 ];
		vtx[index_vtx++] = start_points [ i ][ 2 ];

		vtx[index_vtx++] = end_points [ i ][ 0 ];
		vtx[index_vtx++] = end_points [ i ][ 1 ];
		vtx[index_vtx++] = end_points [ i ][ 2 ];

		pointb = ( i + 1 ) % NUM_BEAM_SEGS;
		vtx[index_vtx++] = start_points [ pointb ][ 0 ];
		vtx[index_vtx++] = start_points [ pointb ][ 1 ];
		vtx[index_vtx++] = start_points [ pointb ][ 2 ];

		vtx[index_vtx++] = end_points [ pointb ][ 0 ];
		vtx[index_vtx++] = end_points [ pointb ][ 1 ];
		vtx[index_vtx++] = end_points [ pointb ][ 2 ];
	}

	glEnableClientState( GL_VERTEX_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, vtx );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, NUM_BEAM_SEGS*4 );

	glDisableClientState( GL_VERTEX_ARRAY );

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}
