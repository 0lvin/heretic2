#ifndef __GEOM_H__
#define __GEOM_H__

struct Vec4;

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
	explicit Vec3(const Vec4 &v);

	float &operator[](int i) { return v[i]; }

	Vec3 operator*(float k) const { return Vec3(x*k, y*k, z*k); }

	Vec3 &operator*=(const Vec3 &o) { x *= o.x; y *= o.y; z *= o.z; return *this; }
	Vec3 &operator/=(float k) { x /= k; y /= k; z /= k; return *this; }

	float dot(const Vec3 &o) const { return x*o.x + y*o.y + z*o.z; }
	float squaredlen() const { return dot(*this); }
	Vec3 cross(const Vec3 &o) const { return Vec3(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }
};

struct Vec4
{
	union
	{
		struct { float x, y, z, w; };
		float v[4];
	};

	Vec4() {}
	Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	explicit Vec4(const Vec3 &p, float w = 0) : x(p.x), y(p.y), z(p.z), w(w) {}
	explicit Vec4(const float *v) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

	Vec4 operator+(const Vec4 &o) const { return Vec4(x+o.x, y+o.y, z+o.z, w+o.w); }
	Vec4 operator*(float k) const { return Vec4(x*k, y*k, z*k, w*k); }
	Vec4 addw(float f) const { return Vec4(x, y, z, w + f); }

	float dot(const Vec3 &o) const  { return x*o.x + y*o.y + z*o.z + w; }
	Vec3 cross3(const Vec4 &o) const { return Vec3(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }
};

inline Vec3::Vec3(const Vec4 &v) : x(v.x), y(v.y), z(v.z) {}

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
	invrot.a /= invrot.a.squaredlen();
	invrot.b /= invrot.b.squaredlen();
	invrot.c /= invrot.c.squaredlen();
	Vec3 trans(o.a.w, o.b.w, o.c.w);
	out->a = Vec4(invrot.a, -invrot.a.dot(trans));
	out->b = Vec4(invrot.b, -invrot.b.dot(trans));
	out->c = Vec4(invrot.c, -invrot.c.dot(trans));
}

void Matrix3x3_mul_float(const Vec4 q, const Vec3 scale, Matrix3x3 *out)
{
	float x = q.x, y = q.y, z = q.z, w = q.w,
		  tx = 2*x, ty = 2*y, tz = 2*z,
		  txx = tx*x, tyy = ty*y, tzz = tz*z,
		  txy = tx*y, txz = tx*z, tyz = ty*z,
		  twx = w*tx, twy = w*ty, twz = w*tz;
	(*out).a = Vec3(1 - (tyy + tzz), txy - twz, txz + twy);
	(*out).b = Vec3(txy + twz, 1 - (txx + tzz), tyz - twx);
	(*out).c = Vec3(txz - twy, tyz + twx, 1 - (txx + tyy));

	(*out).a *= scale;
	(*out).b *= scale;
	(*out).c *= scale;
}

void Matrix3x4_mul(const Matrix3x4 in1, const Matrix3x4 in2, Matrix3x4 *out)
{
	(*out).a = Vec4(in2.a*in1.a.x + in2.b*in1.a.y + in2.c*in1.a.z).addw(in1.a.w);
	(*out).b = Vec4(in2.a*in1.b.x + in2.b*in1.b.y + in2.c*in1.b.z).addw(in1.b.w);
	(*out).c = Vec4(in2.a*in1.c.x + in2.b*in1.c.y + in2.c*in1.c.z).addw(in1.c.w);
}

void Matrix3x4_mul_float(const Matrix3x4 in1, float in2, Matrix3x4 *out)
{
	out->a = in1.a * in2;
	out->b = in1.b * in2;
	out->c = in1.c * in2;
}

#endif
