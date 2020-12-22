/*
 * Copyright (C) 2016,2017 Edd Biddulph
 * Copyright (C) 2013 Alejandro Ricoveri
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
 * This file implements the operating system binding of GL to QGL function
 * pointers.  When doing a port of Quake2 you must implement the following
 * two functions:
 *
 * QGL_Init() - loads libraries, assigns function pointers, etc.
 * QGL_Shutdown() - unloads libraries, NULLs function pointers
 *
 * This implementation should work for Windows and unixoid platforms,
 * other platforms may need an own implementation.
 *
 * =======================================================================
 */

#include "header/local.h"

/*
 * GL extensions
 */
void (APIENTRY *qglPointParameterfARB)(GLenum param, GLfloat value);
void (APIENTRY *qglPointParameterfvARB)(GLenum param, const GLfloat *value);
void (APIENTRY *qglColorTableEXT)(GLenum, GLenum, GLsizei, GLenum, GLenum,
		const GLvoid *);

void ( APIENTRY *qglMultiTexCoord3fARB ) ( GLenum, GLfloat, GLfloat, GLfloat );
void ( APIENTRY *qglMultiTexCoord4fARB ) ( GLenum, GLfloat, GLfloat, GLfloat, GLfloat );
void ( APIENTRY *qglActiveTextureARB ) ( GLenum );

/* ------------------------- GL_ARB_shader_objects ------------------------- */

PFNGLATTACHOBJECTARBPROC qglAttachObjectARB;
PFNGLCOMPILESHADERARBPROC qglCompileShaderARB;
PFNGLCREATEPROGRAMOBJECTARBPROC qglCreateProgramObjectARB;
PFNGLCREATESHADEROBJECTARBPROC qglCreateShaderObjectARB;
PFNGLDELETEOBJECTARBPROC qglDeleteObjectARB;
PFNGLGETINFOLOGARBPROC qglGetInfoLogARB;
PFNGLGETOBJECTPARAMETERIVARBPROC qglGetObjectParameterivARB;
PFNGLGETUNIFORMLOCATIONARBPROC qglGetUniformLocationARB;
PFNGLLINKPROGRAMARBPROC qglLinkProgramARB;
PFNGLSHADERSOURCEARBPROC qglShaderSourceARB;
PFNGLUNIFORM1FARBPROC qglUniform1fARB;
PFNGLUNIFORM1IARBPROC qglUniform1iARB;
PFNGLUNIFORM3FARBPROC qglUniform3fARB;
PFNGLUNIFORM4FARBPROC qglUniform4fARB;
PFNGLUNIFORMMATRIX4FVARBPROC qglUniformMatrix4fvARB;
PFNGLUSEPROGRAMOBJECTARBPROC qglUseProgramObjectARB;

/* -------------------------- GL_ARB_vertex_shader ------------------------- */

PFNGLBINDATTRIBLOCATIONARBPROC qglBindAttribLocationARB;

/* ---------------------- GL_ARB_texture_buffer_object --------------------- */

PFNGLTEXBUFFERARBPROC qglTexBufferARB;

/* ---------------------- GL_EXT_texture_buffer_object --------------------- */

PFNGLTEXBUFFEREXTPROC qglTexBufferEXT;

/* ----------------------------- GL_VERSION_3_1 ---------------------------- */

PFNGLTEXBUFFERPROC qglTexBuffer;


/* ---------------------- GL_ARB_vertex_buffer_object ---------------------- */

PFNGLBINDBUFFERARBPROC qglBindBufferARB;
PFNGLBUFFERDATAARBPROC qglBufferDataARB;
PFNGLBUFFERSUBDATAARBPROC qglBufferSubDataARB;
PFNGLDELETEBUFFERSARBPROC qglDeleteBuffersARB;
PFNGLGENBUFFERSARBPROC qglGenBuffersARB;
PFNGLMAPBUFFERARBPROC qglMapBufferARB;
PFNGLUNMAPBUFFERARBPROC qglUnmapBufferARB;

/* ----------------------------- GL_VERSION_1_5 ---------------------------- */

PFNGLBINDBUFFERPROC qglBindBuffer;
PFNGLBUFFERDATAPROC qglBufferData;
PFNGLBUFFERSUBDATAPROC qglBufferSubData;
PFNGLDELETEBUFFERSPROC qglDeleteBuffers;
PFNGLGENBUFFERSPROC qglGenBuffers;
PFNGLMAPBUFFERPROC qglMapBuffer;
PFNGLUNMAPBUFFERPROC qglUnmapBuffer;

/* ------------------------ GL_ARB_map_buffer_range ------------------------ */

PFNGLMAPBUFFERRANGEPROC qglMapBufferRange;

/* ----------------------------- GL_VERSION_1_2 ---------------------------- */

PFNGLTEXIMAGE3DPROC qglTexImage3D;
PFNGLTEXSUBIMAGE3DPROC qglTexSubImage3D;

/* ========================================================================= */

void QGL_EXT_Reset ( void )
{
	qglPointParameterfARB     = NULL;
	qglPointParameterfvARB    = NULL;
	qglColorTableEXT          = NULL;
	qglMultiTexCoord3fARB     = NULL;
	qglMultiTexCoord4fARB     = NULL;
	qglActiveTextureARB       = NULL;

	/* ------------------------- GL_ARB_shader_objects ------------------------- */

	qglAttachObjectARB = NULL;
	qglCompileShaderARB = NULL;
	qglCreateProgramObjectARB = NULL;
	qglCreateShaderObjectARB = NULL;
	qglDeleteObjectARB = NULL;
	qglGetInfoLogARB = NULL;
	qglGetObjectParameterivARB = NULL;
	qglGetUniformLocationARB = NULL;
	qglLinkProgramARB = NULL;
	qglShaderSourceARB = NULL;
	qglUniform1fARB = NULL;
	qglUniform1iARB = NULL;
	qglUniform3fARB = NULL;
	qglUniformMatrix4fvARB = NULL;
	qglUseProgramObjectARB = NULL;

	/* -------------------------- GL_ARB_vertex_shader ------------------------- */

	qglBindAttribLocationARB = NULL;

	/* ---------------------- GL_ARB_texture_buffer_object --------------------- */

	qglTexBufferARB = NULL;

	/* ---------------------- GL_EXT_texture_buffer_object --------------------- */

	qglTexBufferEXT = NULL;

	/* ----------------------------- GL_VERSION_3_1 ---------------------------- */

	qglTexBuffer = NULL;

	/* ---------------------- GL_ARB_vertex_buffer_object ---------------------- */

	qglBindBufferARB = NULL;
	qglBufferDataARB = NULL;
	qglBufferSubDataARB = NULL;
	qglDeleteBuffersARB = NULL;
	qglGenBuffersARB = NULL;
	qglMapBufferARB = NULL;
	qglUnmapBufferARB = NULL;

	/* ----------------------------- GL_VERSION_1_5 ---------------------------- */

	qglBindBuffer = NULL;
	qglBufferData = NULL;
	qglBufferSubData = NULL;
	qglDeleteBuffers = NULL;
	qglGenBuffers = NULL;
	qglMapBuffer = NULL;
	qglUnmapBuffer = NULL;

	/* ------------------------ GL_ARB_map_buffer_range ------------------------ */

	qglMapBufferRange = NULL;

	/* ----------------------------- GL_VERSION_1_2 ---------------------------- */

	qglTexImage3D = NULL;
	qglTexSubImage3D = NULL;
}

/* ========================================================================= */

void
QGL_Shutdown ( void )
{
	// Reset GL extension pointers
	QGL_EXT_Reset();
}

/* ========================================================================= */

qboolean
QGL_Init (void)
{
	// Reset GL extension pointers
	QGL_EXT_Reset();
	return true;
}

