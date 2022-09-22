#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

// #include "../src/common/header/shared.h"
// #include "../src/common/header/files.h"

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef signed long long int llong;
typedef unsigned long long int ullong;

/* Vectors */
typedef float vec2_t[2];
typedef float vec3_t[3];

/* Quaternion (x, y, z, w) */
typedef float quat4_t[4];

/**
 * Basic quaternion operations.
 */

static void
Quat_computeW (quat4_t q)
{
	float t = 1.0f - (q[0] * q[0]) - (q[1] * q[1]) - (q[2] * q[2]);

	if (t < 0.0f)
		q[3] = 0.0f;
	else
		q[3] = -sqrt (t);
}

static void
Quat_normalize (quat4_t q)
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

static void
Quat_multQuat (const quat4_t qa, const quat4_t qb, quat4_t out)
{
	out[3] = (qa[3] * qb[3]) - (qa[0] * qb[0]) - (qa[1] * qb[1]) - (qa[2] * qb[2]);
	out[0] = (qa[0] * qb[3]) + (qa[3] * qb[0]) + (qa[1] * qb[2]) - (qa[2] * qb[1]);
	out[1] = (qa[1] * qb[3]) + (qa[3] * qb[1]) + (qa[2] * qb[0]) - (qa[0] * qb[2]);
	out[2] = (qa[2] * qb[3]) + (qa[3] * qb[2]) + (qa[0] * qb[1]) - (qa[1] * qb[0]);
}

static void
Quat_multVec (const quat4_t q, const vec3_t v, quat4_t out)
{
	out[3] = - (q[0] * v[0]) - (q[1] * v[1]) - (q[2] * v[2]);
	out[0] =   (q[3] * v[0]) + (q[1] * v[2]) - (q[2] * v[1]);
	out[1] =   (q[3] * v[1]) + (q[2] * v[0]) - (q[0] * v[2]);
	out[2] =   (q[3] * v[2]) + (q[0] * v[1]) - (q[1] * v[0]);
}

static void
Quat_rotatePoint (const quat4_t q, const vec3_t in, vec3_t out)
{
	quat4_t tmp, inv, final;

	inv[0] = -q[0];
	inv[1] = -q[1];
	inv[2] = -q[2];
	inv[3] =  q[3];

	Quat_normalize (inv);

	Quat_multVec (q, in, tmp);
	Quat_multQuat (tmp, inv, final);

	out[0] = final[0];
	out[1] = final[1];
	out[2] = final[2];
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

struct Vec3
{
	union
	{
		struct { float x, y, z; };
		float v[3];
	};

	Vec3() {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	explicit Vec3(const float *v) : x(v[0]), y(v[1]), z(v[2]) {}

	float &operator[](int i) { return v[i]; }

	Vec3 operator*(float k) const { return Vec3(x*k, y*k, z*k); }

	Vec3 &operator*=(const Vec3 &o) { x *= o.x; y *= o.y; z *= o.z; return *this; }
	Vec3 &operator/=(float k) { x /= k; y /= k; z /= k; return *this; }
	Vec3 cross(const Vec3 &o) const { return Vec3(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }
};

static float
Vec3_dot(const Vec3 in1, const Vec3 in2)
{
	return in1.x * in2.x + in1.y * in2.y + in1.z * in2.z;
}

struct Vec4
{
	struct { float x, y, z, w; };

	Vec4() {}
	Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	Vec4 operator+(const Vec4 &o) const { return Vec4(x+o.x, y+o.y, z+o.z, w+o.w); }
	Vec4 operator*(float k) const { return Vec4(x*k, y*k, z*k, w*k); }
};

void Vec4_addw(const Vec4 in1, float in2, Vec4 *out)
{
	out->x = in1.x;
	out->y = in1.y;
	out->z = in1.z;
	out->w = in1.w + in2;
}

float Vec4_dot(const Vec4 in1, const Vec3 in2)
{
	return in1.x * in2.x + in1.y * in2.y + in1.z * in2.z + in1.w;
}

void Vec4_cross3(const Vec4 in1, const Vec4 in2, Vec3 *out)
{
	out->x = in1.y * in2.z - in1.z * in2.y;
	out->y = in1.z * in2.x - in1.x * in2.z;
	out->z = in1.x * in2.y - in1.y * in2.x;
};

struct Matrix3x3
{
	Vec3 a, b, c;
};

struct Matrix3x4
{
	Vec4 a, b, c;
};

void Matrix3x4_plus(const Matrix3x4 in1, const Matrix3x4 in2, Matrix3x4 *out)
{
	out->a = in1.a + in2.a;
	out->b = in1.b + in2.b;
	out->c = in1.c + in2.c;
}

void Matrix3x4_invert(const Matrix3x4 o, Matrix3x4 *out)
{
	Matrix3x3 invrot;
	invrot.a = Vec3(o.a.x, o.b.x, o.c.x);
	invrot.b = Vec3(o.a.y, o.b.y, o.c.y);
	invrot.c = Vec3(o.a.z, o.b.z, o.c.z);
	invrot.a /= Vec3_dot(invrot.a, invrot.a);
	invrot.b /= Vec3_dot(invrot.b, invrot.b);
	invrot.c /= Vec3_dot(invrot.c, invrot.c);
	Vec3 trans(o.a.w, o.b.w, o.c.w);

	out->a.x = invrot.a.x;
	out->a.y = invrot.a.y;
	out->a.z = invrot.a.z;
	out->a.w = -Vec3_dot(invrot.a, trans);

	out->b.x = invrot.b.x;
	out->b.y = invrot.b.y;
	out->b.z = invrot.b.z;
	out->b.w = -Vec3_dot(invrot.b, trans);

	out->c.x = invrot.c.x;
	out->c.y = invrot.c.y;
	out->c.z = invrot.c.z;
	out->c.w = -Vec3_dot(invrot.c, trans);
}

void Matrix3x3_mul_float(const vec3_t q, const Vec3 scale, Matrix3x3 *out)
{
	float x = q[0], y = q[1], z = q[2], w = q[3],
		  tx = 2*x, ty = 2*y, tz = 2*z,
		  txx = tx*x, tyy = ty*y, tzz = tz*z,
		  txy = tx*y, txz = tx*z, tyz = ty*z,
		  twx = tx*w, twy = ty*w, twz = tz*w;
	(*out).a = Vec3(1 - (tyy + tzz), txy - twz, txz + twy);
	(*out).b = Vec3(txy + twz, 1 - (txx + tzz), tyz - twx);
	(*out).c = Vec3(txz - twy, tyz + twx, 1 - (txx + tyy));
	(*out).a *= scale;
	(*out).b *= scale;
	(*out).c *= scale;
}

void Matrix3x4_mul(const Matrix3x4 in1, const Matrix3x4 in2, Matrix3x4 *out)
{
	Vec4_addw(Vec4(in2.a*in1.a.x + in2.b*in1.a.y + in2.c*in1.a.z), in1.a.w, &(*out).a);
	Vec4_addw(Vec4(in2.a*in1.b.x + in2.b*in1.b.y + in2.c*in1.b.z), in1.b.w, &(*out).b);
	Vec4_addw(Vec4(in2.a*in1.c.x + in2.b*in1.c.y + in2.c*in1.c.z), in1.c.w, &(*out).c);
}

void Matrix3x4_mul_float(const Matrix3x4 in1, float in2, Matrix3x4 *out)
{
	out->a = in1.a * in2;
	out->b = in1.b * in2;
	out->c = in1.c * in2;
}

#include "iqm.h"

// don't need HDR stuff
#define STBI_NO_LINEAR
#define STBI_NO_HDR
// make sure STB_image uses standard malloc(), as we'll use standard free() to deallocate
#define STBI_MALLOC(sz)	malloc(sz)
#define STBI_REALLOC(p,sz)	realloc(p,sz)
#define STBI_FREE(p)	free(p)
// Switch of the thread local stuff. Breaks mingw under Windows.
#define STBI_NO_THREAD_LOCALS
// include implementation part of stb_image into this file
#define STB_IMAGE_IMPLEMENTATION
#include "../src/client/refresh/files/stb_image.h"

// include resize implementation
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../src/client/refresh/files/stb_image_resize.h"

static void
resizetexture(int w, int h, bool mipmap, int &tw, int &th)
{
	GLint sizelimit = 4096;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &sizelimit);
	w = min(w, sizelimit);
	h = min(h, sizelimit);
	if (mipmap || w & (w-1) || h & (h-1))
	{
		tw = th = 1;

		while(tw < w)
			tw *= 2;
		while(th < h)
			th *= 2;
		if (w < tw - tw/4)
			tw /= 2;
		if (h < th - th/4)
			th /= 2;
	}
	else
	{
		tw = w;
		th = h;
	}
}

static void
uploadtexture(int tw, int th, void *pixels, int pw, int ph, bool mipmap)
{
	int bpp = 4;
	byte *buf = NULL;

	if (pw!=tw || ph!=th)
	{
		buf = (byte*)malloc(sizeof(byte[tw*th*bpp]));
		stbir_resize_uint8((byte *)pixels, pw, ph, 0,
				buf, tw, th, 0, bpp);
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
				buf, tw, th, 0, bpp);
	}

	if (buf)
		free(buf);
}

static void
createtexture(int tnum, int w, int h, void *pixels, int clamp, int filter, int pw = 0, int ph = 0)
{
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

	if (!pw)
		pw = w;
	if (!ph)
		ph = h;
	int tw = w, th = h;
	bool mipmap = filter > 1;
	if (pixels)
		resizetexture(w, h, mipmap, tw, th);
	uploadtexture(tw, th, pixels, pw, ph, mipmap && pixels);
}

static GLuint
loadtexture(const char *name, int clamp)
{
	int w, h, b;
	byte *data = stbi_load(name, &w, &h, &b, STBI_rgb_alpha);
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
iqmtriangle *tris = NULL, *adjacency = NULL;
iqmmesh *meshes = NULL;
GLuint *textures = NULL;
iqmjoint *joints = NULL;
iqmpose *poses = NULL;
iqmanim *anims = NULL;
iqmbounds *bounds = NULL;
Matrix3x4 *baseframe = NULL, *inversebaseframe = NULL, *outframe = NULL, *frames = NULL;

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

static bool
loadiqmmeshes(const char *filename, const iqmheader &hdr, byte *buf)
{
	if (meshdata)
		return false;

	lilswap_uint((uint *)&buf[hdr.ofs_vertexarrays], hdr.num_vertexarrays*sizeof(iqmvertexarray)/sizeof(uint));
	lilswap_uint((uint *)&buf[hdr.ofs_triangles], hdr.num_triangles*sizeof(iqmtriangle)/sizeof(uint));
	lilswap_uint((uint *)&buf[hdr.ofs_meshes], hdr.num_meshes*sizeof(iqmmesh)/sizeof(uint));
	lilswap_uint((uint *)&buf[hdr.ofs_joints], hdr.num_joints*sizeof(iqmjoint)/sizeof(uint));
	if (hdr.ofs_adjacency)
		lilswap_uint((uint *)&buf[hdr.ofs_adjacency], hdr.num_triangles*sizeof(iqmtriangle)/sizeof(uint));

	meshdata = buf;
	nummeshes = hdr.num_meshes;
	numtris = hdr.num_triangles;
	numverts = hdr.num_vertexes;
	numjoints = hdr.num_joints;
	outposition = (float*)malloc(sizeof(float[3*numverts]));
	outnormal = (float*)malloc(sizeof(float[3*numverts]));
	outtangent = (float*)malloc(sizeof(float[3*numverts]));
	outbitangent = (float*)malloc(sizeof(float[3*numverts]));
	outframe = (Matrix3x4*)malloc(sizeof(Matrix3x4[hdr.num_joints]));
	textures = (GLuint*)malloc(sizeof(GLuint[nummeshes]));
	memset(textures, 0, nummeshes*sizeof(GLuint));

	printf("%s: load: %d vertex arrays \n", __func__, hdr.num_vertexarrays);
	printf("%s: load: %d vertex ofs \n", __func__, hdr.ofs_vertexarrays);
	printf("%s: load: %d text ofs \n", __func__, hdr.ofs_text);

	const char *str = hdr.ofs_text ? (char *)&buf[hdr.ofs_text] : "";
	iqmvertexarray *vas = (iqmvertexarray *)&buf[hdr.ofs_vertexarrays];

	for(int i = 0; i < (int)hdr.num_vertexarrays; i++)
	{
		iqmvertexarray &va = vas[i];

		printf("%s: load: %d type vertex\n", __func__, va.type);

		switch(va.type)
		{
			case IQM_POSITION:
				if (va.format != IQM_FLOAT || va.size != 3)
					return false;
				inposition = (float *)&buf[va.offset];
				lilswap_float(inposition, 3*hdr.num_vertexes);
				break;
			case IQM_NORMAL:
				if (va.format != IQM_FLOAT || va.size != 3)
					return false;
				innormal = (float *)&buf[va.offset];
				lilswap_float(innormal, 3*hdr.num_vertexes);
				break;
			case IQM_TANGENT:
				if (va.format != IQM_FLOAT || va.size != 4)
					return false;
				intangent = (float *)&buf[va.offset];
				lilswap_float(intangent, 4*hdr.num_vertexes);
				break;
			case IQM_TEXCOORD:
				if (va.format != IQM_FLOAT || va.size != 2)
					return false;
				intexcoord = (float *)&buf[va.offset];
				lilswap_float(intexcoord, 2*hdr.num_vertexes);
				break;
			case IQM_BLENDINDEXES:
				if (va.format != IQM_UBYTE || va.size != 4)
					return false;
				inblendindex = (byte *)&buf[va.offset];
				break;
			case IQM_BLENDWEIGHTS:
				if (va.format != IQM_UBYTE || va.size != 4)
					return false;
				inblendweight = (byte *)&buf[va.offset];
				break;
			case IQM_COLOR:
				if (va.format != IQM_UBYTE || va.size != 4)
					return false;
				incolor = (byte *)&buf[va.offset];
				break;
		}
	}

	tris = (iqmtriangle *)&buf[hdr.ofs_triangles];
	meshes = (iqmmesh *)&buf[hdr.ofs_meshes];
	joints = (iqmjoint *)&buf[hdr.ofs_joints];
	if (hdr.ofs_adjacency)
		adjacency = (iqmtriangle *)&buf[hdr.ofs_adjacency];

	baseframe = (Matrix3x4*)malloc(sizeof(Matrix3x4[hdr.num_joints]));
	inversebaseframe = (Matrix3x4*)malloc(sizeof(Matrix3x4[hdr.num_joints]));
	for(int i = 0; i < (int)hdr.num_joints; i++)
	{
		iqmjoint &j = joints[i];
		quat4_t q;
		q[0] = j.rotate[0];
		q[1] = j.rotate[1];
		q[2] = j.rotate[2];
		q[3] = j.rotate[3];

		Quat_normalize(q);

		Matrix3x3 rot;
		Matrix3x3_mul_float(q, Vec3(j.scale), &rot);

		baseframe[i].a.x = rot.a.x;
		baseframe[i].a.y = rot.a.y;
		baseframe[i].a.z = rot.a.z;
		baseframe[i].a.w = j.translate[0];

		baseframe[i].b.x = rot.b.x;
		baseframe[i].b.y = rot.b.y;
		baseframe[i].b.z = rot.b.z;
		baseframe[i].b.w = j.translate[1];

		baseframe[i].c.x = rot.c.x;
		baseframe[i].c.y = rot.c.y;
		baseframe[i].c.z = rot.c.z;
		baseframe[i].c.w = j.translate[2];

		Matrix3x4_invert(baseframe[i], &inversebaseframe[i]);
		if (j.parent >= 0)
		{
			Matrix3x4_mul(baseframe[j.parent], baseframe[i], &baseframe[i]);
			Matrix3x4_mul(inversebaseframe[i], inversebaseframe[j.parent], &inversebaseframe[i]);
		}
	}

	for(int i = 0; i < (int)hdr.num_meshes; i++)
	{
		iqmmesh &m = meshes[i];
		printf("%s: loaded mesh: %s\n", filename, &str[m.name]);
		textures[i] = loadtexture(&str[m.material], 0);
		if (textures[i])
			printf("%s: loaded material: %s\n", filename, &str[m.material]);
	}

	return true;
}

static bool
loadiqmanims(const char *filename, const iqmheader &hdr, byte *buf)
{
	if ((int)hdr.num_poses != numjoints)
		return false;

	if (animdata)
	{
		if (animdata != meshdata) free(animdata);
		free(frames);
		animdata = NULL;
		anims = NULL;
		frames = 0;
		numframes = 0;
		numanims = 0;
	}

	lilswap_uint((uint *)&buf[hdr.ofs_poses], hdr.num_poses*sizeof(iqmpose)/sizeof(uint));
	lilswap_uint((uint *)&buf[hdr.ofs_anims], hdr.num_anims*sizeof(iqmanim)/sizeof(uint));
	lilswap_short((ushort *)&buf[hdr.ofs_frames], hdr.num_frames*hdr.num_framechannels);
	if (hdr.ofs_bounds)
		lilswap_uint((uint *)&buf[hdr.ofs_bounds], hdr.num_frames*sizeof(iqmbounds)/sizeof(uint));

	animdata = buf;
	numanims = hdr.num_anims;
	numframes = hdr.num_frames;

	const char *str = hdr.ofs_text ? (char *)&buf[hdr.ofs_text] : "";
	anims = (iqmanim *)&buf[hdr.ofs_anims];
	poses = (iqmpose *)&buf[hdr.ofs_poses];
	frames = (Matrix3x4*)malloc(sizeof(Matrix3x4[hdr.num_frames * hdr.num_poses]));
	ushort *framedata = (ushort *)&buf[hdr.ofs_frames];
	if (hdr.ofs_bounds)
		bounds = (iqmbounds *)&buf[hdr.ofs_bounds];

	for(int i = 0; i < (int)hdr.num_frames; i++)
	{
		for(int j = 0; j < (int)hdr.num_poses; j++)
		{
			iqmpose &p = poses[j];
			quat4_t rotate;
			vec3_t translate, scale;

			translate[0] = p.channeloffset[0];
			if (p.mask&0x01)
				translate[0] += *framedata++ * p.channelscale[0];
			translate[1] = p.channeloffset[1];
			if (p.mask&0x02)
				translate[1] += *framedata++ * p.channelscale[1];
			translate[2] = p.channeloffset[2];
			if (p.mask&0x04)
				translate[2] += *framedata++ * p.channelscale[2];
			rotate[0] = p.channeloffset[3];
			if (p.mask&0x08)
				rotate[0] += *framedata++ * p.channelscale[3];
			rotate[1] = p.channeloffset[4];
			if (p.mask&0x10)
				rotate[1] += *framedata++ * p.channelscale[4];
			rotate[2] = p.channeloffset[5];
			if (p.mask&0x20)
				rotate[2] += *framedata++ * p.channelscale[5];
			rotate[3] = p.channeloffset[6];
			if (p.mask&0x40)
				rotate[3] += *framedata++ * p.channelscale[6];
			scale[0] = p.channeloffset[7];
			if (p.mask&0x80)
				scale[0] += *framedata++ * p.channelscale[7];
			scale[1] = p.channeloffset[8];
			if (p.mask&0x100)
				scale[1] += *framedata++ * p.channelscale[8];
			scale[2] = p.channeloffset[9];
			if (p.mask&0x200)
				scale[2] += *framedata++ * p.channelscale[9];

			// Concatenate each pose with the inverse base pose to avoid doing this at animation time.
			// If the joint has a parent, then it needs to be pre-concatenated with its parent's base pose.
			// Thus it all negates at animation time like so:
			//	 (parentPose * parentInverseBasePose) * (parentBasePose * childPose * childInverseBasePose) =>
			//	 parentPose * (parentInverseBasePose * parentBasePose) * childPose * childInverseBasePose =>
			//	 parentPose * childPose * childInverseBasePose

			Quat_normalize(rotate);

			Matrix3x3 rot;
			Matrix3x4 m;

			Matrix3x3_mul_float(rotate, Vec3(scale[0], scale[1], scale[2]), &rot);

			m.a.x = rot.a.x;
			m.a.y = rot.a.y;
			m.a.z = rot.a.z;
			m.a.w = translate[0];

			m.b.x = rot.b.x;
			m.b.y = rot.b.y;
			m.b.z = rot.b.z;
			m.b.w = translate[1];

			m.c.x = rot.c.x;
			m.c.y = rot.c.y;
			m.c.z = rot.c.z;
			m.c.w = translate[2];

			if (p.parent >= 0) {
				Matrix3x4 tmp;

				Matrix3x4_mul(baseframe[p.parent], m, &tmp);
				Matrix3x4_mul(tmp, inversebaseframe[j], &frames[i*hdr.num_poses + j]);
			}
			else
			{
				Matrix3x4_mul(m, inversebaseframe[j], &frames[i*hdr.num_poses + j]);
			}
		}
	}

	for(int i = 0; i < (int)hdr.num_anims; i++)
	{
		printf("%s: loaded anim: %s\n", filename, &str[anims[i].name]);
	}

	return true;
}

static bool
loadiqm(const char *filename)
{
	FILE *f = fopen(filename, "rb");
	if (!f)
		return false;

	byte *buf = NULL;
	iqmheader hdr;
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

	if (hdr.num_meshes > 0 && !loadiqmmeshes(filename, hdr, buf))
		goto error;
	if (hdr.num_anims > 0 && !loadiqmanims(filename, hdr, buf))
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
	Matrix3x4 *mat1 = &frames[frame1 * numjoints];
	// Interpolate matrixes between the two closest frames and concatenate with parent matrix if necessary.
	// Concatenate the result with the inverse of the base pose.
	// You would normally do animation blending and inter-frame blending here in a 3D engine.
	for(int i = 0; i < numjoints; i++)
	{
		if (joints[i].parent >= 0)
		{
			Matrix3x4_mul(outframe[joints[i].parent], mat1[i], &outframe[i]);
		}
		else
		{
			outframe[i] = mat1[i];
		}
	}
	// The actual vertex generation based on the matrixes follows...
	const Vec3 *srcpos = (const Vec3 *)inposition,
		*srcnorm = (const Vec3 *)innormal;
	const Vec4 *srctan = (const Vec4 *)intangent;
	Vec3 *dstpos = (Vec3 *)outposition,
		*dstnorm = (Vec3 *)outnormal,
		*dsttan = (Vec3 *)outtangent,
		*dstbitan = (Vec3 *)outbitangent;
	const byte *index = inblendindex, *weight = inblendweight;
	for(int i = 0; i < numverts; i++)
	{
		// Blend matrixes for this vertex according to its blend weights.
		// the first index/weight is always present, and the weights are
		// guaranteed to add up to 255. So if only the first weight is
		// presented, you could optimize this case by skipping any weight
		// multiplies and intermediate storage of a blended matrix.
		// There are only at most 4 weights per vertex, and they are in
		// sorted order from highest weight to lowest weight. Weights with
		// 0 values, which are always at the end, are unused.
		Matrix3x4 mat = outframe[index[0]];
		Matrix3x4_mul_float(mat, weight[0]/255.0f, &mat);
		for(int j = 1; j < 4 && weight[j]; j++)
		{
			Matrix3x4 tmp = outframe[index[j]];
			Matrix3x4_mul_float(tmp, weight[j]/255.0f, &tmp);
			Matrix3x4_plus(tmp, mat, &mat);
		}

		// Transform attributes by the blended matrix.
		// Position uses the full 3x4 transformation matrix.
		// Normals and tangents only use the 3x3 rotation part
		// of the transformation matrix.
		(*dstpos)[0] = Vec4_dot(mat.a, *srcpos);
		(*dstpos)[1] = Vec4_dot(mat.b, *srcpos);
		(*dstpos)[2] = Vec4_dot(mat.c, *srcpos);

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
		Matrix3x3 matnorm;
		Vec4_cross3(mat.b, mat.c, &matnorm.a);
		Vec4_cross3(mat.c, mat.a, &matnorm.b);
		Vec4_cross3(mat.a, mat.b, &matnorm.c);

		(*dstnorm)[0] = Vec3_dot(matnorm.a, *srcnorm);
		(*dstnorm)[1] = Vec3_dot(matnorm.b, *srcnorm);
		(*dstnorm)[2] = Vec3_dot(matnorm.c, *srcnorm);
		// Note that input tangent data has 4 coordinates,
		// so only transform the first 3 as the tangent vector.
		(*dsttan)[0] = Vec3_dot(matnorm.a, Vec3(srctan->x, srctan->y, srctan->z));
		(*dsttan)[1] = Vec3_dot(matnorm.b, Vec3(srctan->x, srctan->y, srctan->z));
		(*dsttan)[2] = Vec3_dot(matnorm.c, Vec3(srctan->x, srctan->y, srctan->z));
		// Note that bitangent = cross(normal, tangent) * sign,
		// where the sign is stored in the 4th coordinate of the input tangent data.
		*dstbitan = dstnorm->cross(*dsttan) * srctan->w;

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
		iqmmesh &m = meshes[i];
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glDrawElements(GL_TRIANGLES, 3*m.num_triangles, GL_UNSIGNED_INT, &tris[m.first_triangle]);
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

float camyaw = -90, campitch = 0, camroll = 0;
Vec3 campos(20, 0, 5);

static void
setupcamera()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLdouble aspect = double(scrw)/scrh,
			 fov = ( 90 * M_PI ) / 180,
			 fovy = 2*atan2(tan(fov/2), aspect),
			 nearplane = 1e-2f, farplane = 1000,
			 ydist = nearplane * tan(fovy/2), xdist = ydist * aspect;
	glFrustum(-xdist, xdist, -ydist, ydist, nearplane, farplane);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(camroll, 0, 0, 1);
	glRotatef(campitch, -1, 0, 0);
	glRotatef(camyaw, 0, 1, 0);
	glRotatef(-90, 1, 0, 0);
	glScalef(1, -1, 1);
	glTranslatef(-campos.x, -campos.y, -campos.z);
}

float animate = 0;

static void
timerfunc(int val)
{
	animate += 10*val/1000.0f;
	glutPostRedisplay();
	glutTimerFunc(35, timerfunc, 35);
}

static void
displayfunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setupcamera();

	animateiqm(animate);

	printf("%s: frame: %.2f\n", __func__, animate);

	renderiqm();

	glutSwapBuffers();
}

static void
keyboardfunc(byte c, int x, int y)
{
	switch(c)
	{
	case 27:
		exit(EXIT_SUCCESS);
		break;
	}
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
	glutKeyboardFunc(keyboardfunc);
	glutMainLoop();

	return EXIT_SUCCESS;
}
