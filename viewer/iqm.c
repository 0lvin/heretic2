#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

#include "rendergl.h"

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef signed long long int llong;
typedef unsigned long long int ullong;

/* Vectors */
typedef float vec2_t[2];
typedef float vec3_t[3];

/* Quaternion (x, y, z, w) */
typedef float vec4_t[4];

typedef vec3_t vec3x3_t[3];
typedef vec4_t quat4x3_t[3];

/**
 * Basic quaternion operations.
 */

static void
Quat_normalize (vec4_t q)
{
	/* compute magnitude of the quaternion */
	float mag = sqrt ((q[0] * q[0]) + (q[1] * q[1])
		+ (q[2] * q[2]) + (q[3] * q[3]));

	/* check for bogus length, to protect against divide by zero */
	if (mag > 0.0f)
	{
		/* normalize it */
		float oneOverMag = 1.0f / mag;

		q[0] *= oneOverMag;
		q[1] *= oneOverMag;
		q[2] *= oneOverMag;
		q[3] *= oneOverMag;
	}
}

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef clamp
#define clamp(val, minval, maxval) max(minval, min(val, maxval))
#endif

static void
Vec3_cross(const vec3_t in1, const vec3_t in2, vec3_t out)
{
	out[0] = in1[1] * in2[2] - in1[2] * in2[1];
	out[1] = in1[2] * in2[0] - in1[0] * in2[2];
	out[2] = in1[0] * in2[1] - in1[1] * in2[0];
}

static void
Vec3_mul(const vec3_t in1, const vec3_t in2, vec3_t out)
{
	int i;

	for(i=0; i<3; i++)
	{
		out[i] = in1[i] * in2[i];
	}
}

static void
Vec3_mul_float(const vec3_t in1, const float in2, vec3_t out)
{
	int i;

	for(i=0; i<3; i++)
	{
		out[i] = in1[i] * in2;
	}
}

static void
Vec3_div_float(const vec3_t in1, const float in2, vec3_t out)
{
	int i;

	for(i=0; i<3; i++)
	{
		out[i] = in1[i] / in2;
	}
}

static float
Vec3_dot(const vec3_t in1, const vec3_t in2)
{
	float dot = 0;
	int i;

	for(i=0; i<3; i++)
	{
		dot += in1[i] * in2[i];
	}
	return dot;
}

static void
Quat_add(const vec4_t in1, const vec4_t in2, vec4_t out)
{
	int i;

	for(i=0; i<4; i++)
	{
		out[i] = in1[i] + in2[i];
	}
}

static void
Quat_mul_float(const vec4_t in1, float in2, vec4_t out)
{
	int i;

	for(i=0; i<4; i++)
	{
		out[i] = in1[i] * in2;
	}
}

static void
Quat_addw(const vec4_t in1, float in2, vec4_t out)
{
	int i;

	for(i=0; i<4; i++)
	{
		out[i] = in1[i];
	}

	out[3] += in2;
}

static float
Quat_dot(const vec4_t in1, const vec3_t in2)
{
	float dot = 0;
	int i;

	for(i=0; i<3; i++)
	{
		dot += in1[i] * in2[i];
	}

	dot += in1[3];

	return dot;
}

static void
Quat_cross3(const vec4_t in1, const vec4_t in2, vec3_t out)
{
	out[0] = in1[1] * in2[2] - in1[2] * in2[1];
	out[1] = in1[2] * in2[0] - in1[0] * in2[2];
	out[2] = in1[0] * in2[1] - in1[1] * in2[0];
};

static void
Matrix3x4_plus(const quat4x3_t in1, const quat4x3_t in2, quat4x3_t out)
{
	int i;

	for(i=0; i<3; i++)
	{
		Quat_add(in1[i], in2[i], out[i]);
	}
}

static void
Matrix3x4_invert(quat4x3_t o, quat4x3_t out)
{
	vec3x3_t invrot;
	vec3_t trans;
	int i;

	for(i=0; i<3; i++)
	{
		int j;

		for(j=0; j<3; j++)
		{
			invrot[i][j] = o[j][i];
		}
	}

	for(i=0; i<3; i++)
	{
		float dot;

		dot = Vec3_dot(invrot[i], invrot[i]);
		Vec3_div_float(invrot[i], dot, invrot[i]);
	}

	for(i=0; i<3; i++)
	{
		trans[i] = o[i][3];
	}

	for(i=0; i<3; i++)
	{
		int j;

		for(j=0; j<3; j++)
		{
			out[i][j] = invrot[i][j];
		}

		out[i][3] = -Vec3_dot(invrot[i], trans);
	}
}

static void
Matrix3x3_mul(const vec3_t q, const vec3_t scale, vec3x3_t out)
{
	int i;
	float x = q[0], y = q[1], z = q[2], w = q[3],
		  tx = 2*x, ty = 2*y, tz = 2*z,
		  txx = tx*x, tyy = ty*y, tzz = tz*z,
		  txy = tx*y, txz = tx*z, tyz = ty*z,
		  twx = tx*w, twy = ty*w, twz = tz*w;

	out[0][0] = 1 - (tyy + tzz);
	out[0][1] = txy - twz;
	out[0][2] = txz + twy;
	out[1][0] = txy + twz;
	out[1][1] = 1 - (txx + tzz);
	out[1][2] = tyz - twx;
	out[2][0] = txz - twy;
	out[2][1] = tyz + twx;
	out[2][2] = 1 - (txx + tyy);

	for(i=0; i<3; i++)
	{
		Vec3_mul(out[i], scale, out[i]);
	}
}

static void
Matrix3x4_mul(const quat4x3_t in1, const quat4x3_t in2, quat4x3_t out)
{
	int j;
	for(j=0; j<3; j++)
	{
		vec4_t sum = {0};
		int i;

		for(i=0; i<3; i++)
		{
			vec4_t tmp;

			Quat_mul_float(in2[i], in1[j][i], tmp);
			Quat_add(sum, tmp, sum);
		}
		Quat_addw(sum, in1[j][3], out[j]);
	}
}

static void
Matrix3x4_mul_float(const quat4x3_t in1, float in2, quat4x3_t out)
{
	int i;

	for(i=0; i<3; i++)
	{
		Quat_mul_float(in1[i], in2, out[i]);
	}
}

#include "iqm.h"

// include resize implementation
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../src/client/refresh/files/stb_image_resize.h"

static void
resizetexture(int w, int h, qboolean mipmap, int *tw, int *th)
{
	GLint sizelimit = 4096;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &sizelimit);
	w = min(w, sizelimit);
	h = min(h, sizelimit);
	if (mipmap || w & (w-1) || h & (h-1))
	{
		*tw = *th = 1;

		while(*tw < w)
			*tw *= 2;
		while(*th < h)
			*th *= 2;
		if (w < *tw - *tw/4)
			*tw /= 2;
		if (h < *th - *th/4)
			*th /= 2;
	}
	else
	{
		*tw = w;
		*th = h;
	}
}

static void
uploadtexture(int tw, int th, void *pixels, int pw, int ph, qboolean mipmap)
{
	int bpp = 4;
	byte *buf = NULL;

	if (pw!=tw || ph!=th)
	{
		buf = (byte*)malloc(sizeof(byte[tw*th*bpp]));
		stbir_resize_uint8((byte *)pixels, pw, ph, 0,
				buf, tw, th, 0, 4);
	}

	for(int level = 0;; level++)
	{
		byte *src = buf ? buf : (byte *)pixels;

		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, src);

		if (!mipmap || max(tw, th) <= 1)
			break;

		int srcw = tw, srch = th;
		if (tw > 1) tw /= 2;
		if (th > 1) th /= 2;
		if (!buf)
			buf = (byte*)malloc(sizeof(byte[tw*th*bpp]));

		stbir_resize_uint8(src, srcw, srch, 0,
				buf, tw, th, 0, 4);
	}

	if (buf)
		free(buf);
}

static void
createtexture(int tnum, int w, int h, void *pixels, int clamp, int filter)
{
	int pw, ph;

	glBindTexture(GL_TEXTURE_2D, tnum);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		clamp&1 ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		clamp&2 ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		filter ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		filter > 1 ? GL_LINEAR_MIPMAP_LINEAR : (filter ? GL_LINEAR : GL_NEAREST));

	pw = w;
	ph = h;

	int tw = w, th = h;
	qboolean mipmap = filter > 1;
	if (pixels)
		resizetexture(w, h, mipmap, &tw, &th);
	uploadtexture(tw, th, pixels, pw, ph, mipmap && pixels);
}

static GLuint
loadtexture(const char *name, int clamp)
{
	int w, h, b;
	byte *data = image_load(name, &w, &h, &b);
	if (!data) {
		printf("%s: failed loading\n", name);
		return 0;
	}

	printf("Checks %dx%d %d\n", w, h, b);

	GLuint tex;
	glGenTextures(1, &tex);
	createtexture(tex, w, h, data, clamp, 2);

	free(data);
	return tex;
}

// Note that while this demo stores pointer directly into mesh data in a buffer
// of the entire IQM file's data, it is recommended that you copy the data and
// convert it into a more suitable internal representation for whichever 3D
// engine you use.
byte *meshdata = NULL, *animdata = NULL;
float *inposition = NULL, *innormal = NULL, *intangent = NULL, *intexcoord = NULL;
byte *inblendindex = NULL, *inblendweight = NULL, *incolor = NULL;
float *outposition = NULL, *outnormal = NULL, *outtangent = NULL, *outbitangent = NULL;
int nummeshes = 0, numtris = 0, numverts = 0, numjoints = 0, numframes = 0, numanims = 0;
struct iqmtriangle *tris = NULL, *adjacency = NULL;
struct iqmmesh *meshes = NULL;
GLuint *textures = NULL;
struct iqmjoint *joints = NULL;
struct iqmpose *poses = NULL;
struct iqmanim *anims = NULL;
struct iqmbounds *bounds = NULL;
quat4x3_t *baseframe = NULL, *inversebaseframe = NULL, *outframe = NULL, *frames = NULL;

static void
cleanupiqm()
{
	if (textures)
	{
		glDeleteTextures(nummeshes, textures);
		free(textures);
	}
	free(outposition);
	free(outnormal);
	free(outtangent);
	free(outbitangent);
	free(baseframe);
	free(inversebaseframe);
	free(outframe);
	free(frames);
}

static void
lilswap_uint(uint *buf, int len)
{
/*
	int i;

	for(i=0; i < len; i++)
	{
		buf[i] = LittleLong(buf[i]);
	}
*/
}

static void
lilswap_float(float *buf, int len)
{
/*
	int i;

	for(i=0; i < len; i++)
	{
		buf[i] = LittleFloat(buf[i]);
	}
*/
}

static void
lilswap_short(ushort *buf, int len)
{
/*
	int i;

	for(i=0; i < len; i++)
	{
		buf[i] = LittleShort(buf[i]);
	}
*/
}

static qboolean
loadiqmmeshes(const char *filename, const struct iqmheader *hdr, byte *buf)
{
	if (meshdata)
		return false;

	lilswap_uint((uint *)&buf[hdr->ofs_vertexarrays], hdr->num_vertexarrays*sizeof(struct iqmvertexarray)/sizeof(uint));
	lilswap_uint((uint *)&buf[hdr->ofs_triangles], hdr->num_triangles*sizeof(struct iqmtriangle)/sizeof(uint));
	lilswap_uint((uint *)&buf[hdr->ofs_meshes], hdr->num_meshes*sizeof(struct iqmmesh)/sizeof(uint));
	lilswap_uint((uint *)&buf[hdr->ofs_joints], hdr->num_joints*sizeof(struct iqmjoint)/sizeof(uint));
	if (hdr->ofs_adjacency)
		lilswap_uint((uint *)&buf[hdr->ofs_adjacency], hdr->num_triangles*sizeof(struct iqmtriangle)/sizeof(uint));

	meshdata = buf;
	nummeshes = hdr->num_meshes;
	numtris = hdr->num_triangles;
	numverts = hdr->num_vertexes;
	numjoints = hdr->num_joints;
	outposition = (float*)malloc(sizeof(float[3*numverts]));
	outnormal = (float*)malloc(sizeof(float[3*numverts]));
	outtangent = (float*)malloc(sizeof(float[3*numverts]));
	outbitangent = (float*)malloc(sizeof(float[3*numverts]));
	outframe = (quat4x3_t*)malloc(sizeof(quat4x3_t[hdr->num_joints]));
	textures = (GLuint*)malloc(sizeof(GLuint[nummeshes]));
	memset(textures, 0, nummeshes*sizeof(GLuint));

	printf("%s: load: %d vertex arrays \n", __func__, hdr->num_vertexarrays);
	printf("%s: load: %d vertex ofs \n", __func__, hdr->ofs_vertexarrays);
	printf("%s: load: %d text ofs \n", __func__, hdr->ofs_text);

	const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
	struct iqmvertexarray *vas = (struct iqmvertexarray *)&buf[hdr->ofs_vertexarrays];

	for(int i = 0; i < (int)hdr->num_vertexarrays; i++)
	{
		struct iqmvertexarray *va = &vas[i];

		printf("%s: load: %d type vertex\n", __func__, va->type);

		switch(va->type)
		{
			case IQM_POSITION:
				if (va->format != IQM_FLOAT || va->size != 3)
					return false;
				inposition = (float *)&buf[va->offset];
				lilswap_float(inposition, 3*hdr->num_vertexes);
				break;
			case IQM_NORMAL:
				if (va->format != IQM_FLOAT || va->size != 3)
					return false;
				innormal = (float *)&buf[va->offset];
				lilswap_float(innormal, 3*hdr->num_vertexes);
				break;
			case IQM_TANGENT:
				if (va->format != IQM_FLOAT || va->size != 4)
					return false;
				intangent = (float *)&buf[va->offset];
				lilswap_float(intangent, 4*hdr->num_vertexes);
				break;
			case IQM_TEXCOORD:
				if (va->format != IQM_FLOAT || va->size != 2)
					return false;
				intexcoord = (float *)&buf[va->offset];
				lilswap_float(intexcoord, 2*hdr->num_vertexes);
				break;
			case IQM_BLENDINDEXES:
				if (va->format != IQM_UBYTE || va->size != 4)
					return false;
				inblendindex = (byte *)&buf[va->offset];
				break;
			case IQM_BLENDWEIGHTS:
				if (va->format != IQM_UBYTE || va->size != 4)
					return false;
				inblendweight = (byte *)&buf[va->offset];
				break;
			case IQM_COLOR:
				if (va->format != IQM_UBYTE || va->size != 4)
					return false;
				incolor = (byte *)&buf[va->offset];
				break;
		}
	}

	tris = (struct iqmtriangle *)&buf[hdr->ofs_triangles];
	meshes = (struct iqmmesh *)&buf[hdr->ofs_meshes];
	joints = (struct iqmjoint *)&buf[hdr->ofs_joints];
	if (hdr->ofs_adjacency)
	{
		adjacency = (struct iqmtriangle *)&buf[hdr->ofs_adjacency];
	}

	baseframe = (quat4x3_t*)malloc(sizeof(quat4x3_t[hdr->num_joints]));
	inversebaseframe = (quat4x3_t*)malloc(sizeof(quat4x3_t[hdr->num_joints]));
	for(int i = 0; i < (int)hdr->num_joints; i++)
	{
		struct iqmjoint *j = &joints[i];
		vec4_t q;
		memcpy(&q, &joints[i].rotate, sizeof(vec4_t));
		Quat_normalize(q);

		vec3x3_t rot;
		Matrix3x3_mul(q, j->scale, rot);

		int k;
		for(k=0; k < 3; k++)
		{
			memcpy(&baseframe[i][k][0], &rot[k][0], sizeof(vec3_t));
			baseframe[i][k][3] = j->translate[k];
		}

		Matrix3x4_invert(baseframe[i], inversebaseframe[i]);
		if (j->parent >= 0)
		{
			quat4x3_t tmp;

			memcpy(&tmp, &baseframe[i], sizeof(quat4x3_t));
			Matrix3x4_mul(baseframe[j->parent], tmp, baseframe[i]);

			memcpy(&tmp, &inversebaseframe[i], sizeof(quat4x3_t));
			Matrix3x4_mul(tmp, inversebaseframe[j->parent], inversebaseframe[i]);
		}
	}

	for(int i = 0; i < (int)hdr->num_meshes; i++)
	{
		struct iqmmesh *m = &meshes[i];
		printf("%s: loaded mesh: %s\n", filename, &str[m->name]);
		textures[i] = loadtexture(&str[m->material], 0);
		if (textures[i])
			printf("%s: loaded material: %s\n", filename, &str[m->material]);
	}

	return true;
}

static qboolean
loadiqmanims(const char *filename, const struct iqmheader *hdr, byte *buf)
{
	if ((int)hdr->num_poses != numjoints)
		return false;

	if (animdata)
	{
		if (animdata != meshdata)
		{
			free(animdata);
		}
		free(frames);
		animdata = NULL;
		anims = NULL;
		frames = 0;
		numframes = 0;
		numanims = 0;
	}

	lilswap_uint((uint *)&buf[hdr->ofs_poses], hdr->num_poses*sizeof(struct iqmpose)/sizeof(uint));
	lilswap_uint((uint *)&buf[hdr->ofs_anims], hdr->num_anims*sizeof(struct iqmanim)/sizeof(uint));
	lilswap_short((ushort *)&buf[hdr->ofs_frames], hdr->num_frames*hdr->num_framechannels);
	if (hdr->ofs_bounds)
		lilswap_uint((uint *)&buf[hdr->ofs_bounds], hdr->num_frames*sizeof(struct iqmbounds)/sizeof(uint));

	animdata = buf;
	numanims = hdr->num_anims;
	numframes = hdr->num_frames;

	const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
	anims = (struct iqmanim *)&buf[hdr->ofs_anims];
	poses = (struct iqmpose *)&buf[hdr->ofs_poses];
	frames = (quat4x3_t*)malloc(sizeof(quat4x3_t[hdr->num_frames * hdr->num_poses]));
	ushort *framedata = (ushort *)&buf[hdr->ofs_frames];
	if (hdr->ofs_bounds)
		bounds = (struct iqmbounds *)&buf[hdr->ofs_bounds];

	for(int i = 0; i < (int)hdr->num_frames; i++)
	{
		for(int j = 0; j < (int)hdr->num_poses; j++)
		{
			struct iqmpose *p = &poses[j];
			vec4_t rotate;
			vec3_t translate, scale;
			int k;

			translate[0] = p->channeloffset[0];
			if (p->mask&0x01)
				translate[0] += *framedata++ * p->channelscale[0];
			translate[1] = p->channeloffset[1];
			if (p->mask&0x02)
				translate[1] += *framedata++ * p->channelscale[1];
			translate[2] = p->channeloffset[2];
			if (p->mask&0x04)
				translate[2] += *framedata++ * p->channelscale[2];
			rotate[0] = p->channeloffset[3];
			if (p->mask&0x08)
				rotate[0] += *framedata++ * p->channelscale[3];
			rotate[1] = p->channeloffset[4];
			if (p->mask&0x10)
				rotate[1] += *framedata++ * p->channelscale[4];
			rotate[2] = p->channeloffset[5];
			if (p->mask&0x20)
				rotate[2] += *framedata++ * p->channelscale[5];
			rotate[3] = p->channeloffset[6];
			if (p->mask&0x40)
				rotate[3] += *framedata++ * p->channelscale[6];
			scale[0] = p->channeloffset[7];
			if (p->mask&0x80)
				scale[0] += *framedata++ * p->channelscale[7];
			scale[1] = p->channeloffset[8];
			if (p->mask&0x100)
				scale[1] += *framedata++ * p->channelscale[8];
			scale[2] = p->channeloffset[9];
			if (p->mask&0x200)
				scale[2] += *framedata++ * p->channelscale[9];

			// Concatenate each pose with the inverse base pose to avoid doing this at animation time.
			// If the joint has a parent, then it needs to be pre-concatenated with its parent's base pose.
			// Thus it all negates at animation time like so:
			//	 (parentPose * parentInverseBasePose) * (parentBasePose * childPose * childInverseBasePose) =>
			//	 parentPose * (parentInverseBasePose * parentBasePose) * childPose * childInverseBasePose =>
			//	 parentPose * childPose * childInverseBasePose

			Quat_normalize(rotate);

			vec3x3_t rot;
			quat4x3_t m;

			Matrix3x3_mul(rotate, scale, rot);

			for(k=0; k<3; k++)
			{
				memcpy(&m[k][0], &rot[k][0], sizeof(vec3_t));
				m[k][3] = translate[k];
			}

			if (p->parent >= 0) {
				quat4x3_t tmp;

				Matrix3x4_mul(baseframe[p->parent], m, tmp);
				Matrix3x4_mul(tmp, inversebaseframe[j], frames[i*hdr->num_poses + j]);
			}
			else
			{
				Matrix3x4_mul(m, inversebaseframe[j], frames[i*hdr->num_poses + j]);
			}
		}
	}

	for(int i = 0; i < (int)hdr->num_anims; i++)
	{
		printf("%s: loaded anim: %s\n", filename, &str[anims[i].name]);
	}

	return true;
}

static qboolean
loadiqm(const char *filename)
{
	FILE *f = fopen(filename, "rb");
	if (!f)
		return false;

	byte *buf = NULL;
	struct iqmheader hdr;
	if (fread(&hdr, 1, sizeof(hdr), f) != sizeof(hdr) || memcmp(hdr.magic, IQM_MAGIC, sizeof(hdr.magic)))
		goto error;
	lilswap_uint(&hdr.version, (sizeof(hdr) - sizeof(hdr.magic))/sizeof(uint));
	if (hdr.version != IQM_VERSION)
		goto error;
	if (hdr.filesize > (16<<20))
		goto error; // sanity check... don't load files bigger than 16 MB
	buf = (byte*)malloc(sizeof(byte[hdr.filesize]));
	if (fread(buf + sizeof(hdr), 1, hdr.filesize - sizeof(hdr), f) != hdr.filesize - sizeof(hdr))
		goto error;

	if (hdr.num_meshes > 0 && !loadiqmmeshes(filename, &hdr, buf))
		goto error;
	if (hdr.num_anims > 0 && !loadiqmanims(filename, &hdr, buf))
		goto error;

	fclose(f);
	return true;

error:
	printf("%s: error while loading\n", filename);
	if (buf != meshdata && buf != animdata)
		free(buf);
	fclose(f);
	return false;
}

// Note that this animates all attributes (position, normal, tangent, bitangent)
// for expository purposes, even though this demo does not use all of them for rendering.
static void
animateiqm(float curframe)
{
	if (!numframes)
		return;

	int frame1 = (int)floor(curframe);
	frame1 %= numframes;
	quat4x3_t *mat1 = &frames[frame1 * numjoints];
	// Interpolate matrixes between the two closest frames and concatenate with parent matrix if necessary.
	// Concatenate the result with the inverse of the base pose.
	// You would normally do animation blending and inter-frame blending here in a 3D engine.
	for(int i = 0; i < numjoints; i++)
	{
		if (joints[i].parent >= 0)
		{
			Matrix3x4_mul(outframe[joints[i].parent], mat1[i], outframe[i]);
		}
		else
		{
			memcpy(&outframe[i], &mat1[i], sizeof(quat4x3_t));
		}
	}
	// The actual vertex generation based on the matrixes follows...
	const vec3_t *srcpos = (const vec3_t *)inposition,
		*srcnorm = (const vec3_t *)innormal;
	const vec4_t *srctan = (const vec4_t *)intangent;
	vec3_t *dstpos = (vec3_t *)outposition,
		*dstnorm = (vec3_t *)outnormal,
		*dsttan = (vec3_t *)outtangent,
		*dstbitan = (vec3_t *)outbitangent;
	const byte *index = inblendindex, *weight = inblendweight;
	for(int i = 0; i < numverts; i++)
	{
		int k;
		// Blend matrixes for this vertex according to its blend weights.
		// the first index/weight is always present, and the weights are
		// guaranteed to add up to 255. So if only the first weight is
		// presented, you could optimize this case by skipping any weight
		// multiplies and intermediate storage of a blended matrix.
		// There are only at most 4 weights per vertex, and they are in
		// sorted order from highest weight to lowest weight. Weights with
		// 0 values, which are always at the end, are unused.
		quat4x3_t mat;

		Matrix3x4_mul_float(outframe[index[0]], weight[0]/255.0f, mat);
		for(int j = 1; j < 4 && weight[j]; j++)
		{
			quat4x3_t tmp;

			Matrix3x4_mul_float(outframe[index[j]], weight[j]/255.0f, tmp);
			Matrix3x4_plus(tmp, mat, mat);
		}

		// Transform attributes by the blended matrix.
		// Position uses the full 3x4 transformation matrix.
		// Normals and tangents only use the 3x3 rotation part
		// of the transformation matrix.
		for(k=0; k<3; k++)
		{
			(*dstpos)[k] = Quat_dot(mat[k], *srcpos);
		}

		// Note that if the matrix includes non-uniform scaling, normal vectors
		// must be transformed by the inverse-transpose of the matrix to have the
		// correct relative scale. Note that invert(mat) = adjoint(mat)/determinant(mat),
		// and since the absolute scale is not important for a vector that will later
		// be renormalized, the adjoint-transpose matrix will work fine, which can be
		// cheaply generated by 3 cross-products.
		//
		// If you don't need to use joint scaling in your models, you can simply use the
		// upper 3x3 part of the position matrix instead of the adjoint-transpose shown
		// here.
		vec3x3_t matnorm;
		Quat_cross3(mat[1], mat[2], matnorm[0]);
		Quat_cross3(mat[2], mat[0], matnorm[1]);
		Quat_cross3(mat[0], mat[1], matnorm[2]);

		for(k=0; k<3; k++)
		{
			(*dstnorm)[k] = Vec3_dot(matnorm[k], *srcnorm);
		}
		// Note that input tangent data has 4 coordinates,
		// so only transform the first 3 as the tangent vector.
		for(k=0; k<3; k++)
		{
			(*dsttan)[k] = Vec3_dot(matnorm[k], *srctan);
		}
		// Note that bitangent = cross(normal, tangent) * sign,
		// where the sign is stored in the 4th coordinate of the input tangent data.
		Vec3_cross(*dstnorm, *dsttan, *dstbitan);
		Vec3_mul_float(*dstbitan, (*srctan)[3], *dstbitan);

		srcpos++;
		srcnorm++;
		srctan++;
		dstpos++;
		dstnorm++;
		dsttan++;
		dstbitan++;

		index += 4;
		weight += 4;
	}
}

static void
renderiqm()
{
	static const GLfloat zero[4] = { 0, 0, 0, 0 },
						 one[4] = { 1, 1, 1, 1 },
						 ambientcol[4] = { 0.5f, 0.5f, 0.5f, 1 },
						 diffusecol[4] = { 0.5f, 0.5f, 0.5f, 1 },
						 lightdir[4] = { cosf(( -60 * M_PI ) / 180), 0, sinf(( -60 * M_PI ) / 180), 0 };

	glPushMatrix();

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, zero);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zero);
	glMaterialfv(GL_FRONT, GL_EMISSION, zero);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, one);
	glLightfv(GL_LIGHT0, GL_SPECULAR, zero);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientcol);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffusecol);
	glLightfv(GL_LIGHT0, GL_POSITION, lightdir);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);

	glColor3f(1, 1, 1);
	glVertexPointer(3, GL_FLOAT, 0, numframes > 0 ? outposition : inposition);
	glNormalPointer(GL_FLOAT, 0, numframes > 0 ? outnormal : innormal);
	glTexCoordPointer(2, GL_FLOAT, 0, intexcoord);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	if (incolor)
	{
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, incolor);

		glEnableClientState(GL_COLOR_ARRAY);
	}

	glEnable(GL_TEXTURE_2D);

	for(int i = 0; i < nummeshes; i++)
	{
		struct iqmmesh *m = &meshes[i];
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glDrawElements(GL_TRIANGLES, 3 * m->num_triangles, GL_UNSIGNED_INT, &tris[m->first_triangle]);
	}

	glDisable(GL_TEXTURE_2D);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if (incolor) glDisableClientState(GL_COLOR_ARRAY);

	glDisable(GL_NORMALIZE);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);

	glPopMatrix();
}

static void
initgl()
{
	glClearColor(0, 0, 0, 0);
	glClearDepth(1);
	glDisable(GL_FOG);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

int scrw = 0, scrh = 0;

static void
reshapefunc(int w, int h)
{
	scrw = w;
	scrh = h;
	glViewport(0, 0, w, h);
}

vec3_t campos = {20, 0, 5};

static void
setupcamera()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLdouble aspect = (double)scrw / scrh,
			 fov = (90 * M_PI) / 180,
			 fovy = 2 * atan2(tan(fov / 2), aspect),
			 nearplane = 1e-2f, farplane = 1000,
			 ydist = nearplane * tan(fovy / 2), xdist = ydist * aspect;
	glFrustum(-xdist, xdist, -ydist, ydist, nearplane, farplane);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef (xrotate, 1.0, 0.0, 0.0);
	glRotatef (yrotate, 0.0, 1.0, 0.0);
	glRotatef (zrotate, 0.0, 0.0, 1.0);

	glScalef(1, -1, 1);
	glTranslatef(-campos[0], -campos[1], -campos[2]);
}

float animate = 0;

static void
timerfunc(int val)
{
	animate += 10 * val / 1000.0f;
	glutPostRedisplay();
	glutTimerFunc(35, timerfunc, 35);
}

static void
displayfunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setupcamera();

	animateiqm(animate);

	renderiqm();

	glutSwapBuffers();
}

int main(int argc, char **argv)
{
	glutInitWindowSize(640, 480);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutCreateWindow("IQM Demo");

	atexit(cleanupiqm);
	for(int i = 1; i < argc; i++)
	{
		if (!loadiqm(argv[i]))
			return EXIT_FAILURE;
	}
	if (!meshdata && !loadiqm("mrfixit.iqm")) return EXIT_FAILURE;

	initgl();

	glutTimerFunc(35, timerfunc, 35);
	glutReshapeFunc(reshapefunc);
	glutDisplayFunc(displayfunc);
	glutKeyboardFunc (keyboard);
	glutMainLoop();

	return EXIT_SUCCESS;
}
