/*
    QShed <http://www.icculus.org/qshed>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* some matrix functions hastily ripped from DarkPlaces */

#include <math.h>

#include "global.h"
#include "matrix.h"

const mat4x4f_t mat4x4f_identity = {{{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}}};

void mat4x4f_create_identity(mat4x4f_t *out)
{
	out->m[0][0] = 1;
	out->m[0][1] = 0;
	out->m[0][2] = 0;
	out->m[0][3] = 0;
	out->m[1][0] = 0;
	out->m[1][1] = 1;
	out->m[1][2] = 0;
	out->m[1][3] = 0;
	out->m[2][0] = 0;
	out->m[2][1] = 0;
	out->m[2][2] = 1;
	out->m[2][3] = 0;
	out->m[3][0] = 0;
	out->m[3][1] = 0;
	out->m[3][2] = 0;
	out->m[3][3] = 1;
}

void mat4x4f_create_translate(mat4x4f_t *out, float x, float y, float z)
{
	out->m[0][0] = 1;
	out->m[0][1] = 0;
	out->m[0][2] = 0;
	out->m[0][3] = x;
	out->m[1][0] = 0;
	out->m[1][1] = 1;
	out->m[1][2] = 0;
	out->m[1][3] = y;
	out->m[2][0] = 0;
	out->m[2][1] = 0;
	out->m[2][2] = 1;
	out->m[2][3] = z;
	out->m[3][0] = 0;
	out->m[3][1] = 0;
	out->m[3][2] = 0;
	out->m[3][3] = 1;
}

void mat4x4f_create_rotate(mat4x4f_t *out, float angle, float x, float y, float z)
{
	float len, c, s;

	len = x*x+y*y+z*z;
	if (len != 0)
		len = 1 / (float)sqrt(len);
	x *= len;
	y *= len;
	z *= len;

	angle *= (float)(-M_PI / 180.0);
	c = (float)cos(angle);
	s = (float)sin(angle);

	out->m[0][0] = x * x + c * (1 - x * x);
	out->m[0][1] = x * y * (1 - c) + z * s;
	out->m[0][2] = z * x * (1 - c) - y * s;
	out->m[0][3] = 0;
	out->m[1][0] = x * y * (1 - c) - z * s;
	out->m[1][1] = y * y + c * (1 - y * y);
	out->m[1][2] = y * z * (1 - c) + x * s;
	out->m[1][3] = 0;
	out->m[2][0] = z * x * (1 - c) + y * s;
	out->m[2][1] = y * z * (1 - c) - x * s;
	out->m[2][2] = z * z + c * (1 - z * z);
	out->m[2][3] = 0;
	out->m[3][0] = 0;
	out->m[3][1] = 0;
	out->m[3][2] = 0;
	out->m[3][3] = 1;
}

void mat4x4f_concat(mat4x4f_t *out, const mat4x4f_t *in1, const mat4x4f_t *in2)
{
	out->m[0][0] = in1->m[0][0] * in2->m[0][0] + in1->m[0][1] * in2->m[1][0] + in1->m[0][2] * in2->m[2][0] + in1->m[0][3] * in2->m[3][0];
	out->m[0][1] = in1->m[0][0] * in2->m[0][1] + in1->m[0][1] * in2->m[1][1] + in1->m[0][2] * in2->m[2][1] + in1->m[0][3] * in2->m[3][1];
	out->m[0][2] = in1->m[0][0] * in2->m[0][2] + in1->m[0][1] * in2->m[1][2] + in1->m[0][2] * in2->m[2][2] + in1->m[0][3] * in2->m[3][2];
	out->m[0][3] = in1->m[0][0] * in2->m[0][3] + in1->m[0][1] * in2->m[1][3] + in1->m[0][2] * in2->m[2][3] + in1->m[0][3] * in2->m[3][3];
	out->m[1][0] = in1->m[1][0] * in2->m[0][0] + in1->m[1][1] * in2->m[1][0] + in1->m[1][2] * in2->m[2][0] + in1->m[1][3] * in2->m[3][0];
	out->m[1][1] = in1->m[1][0] * in2->m[0][1] + in1->m[1][1] * in2->m[1][1] + in1->m[1][2] * in2->m[2][1] + in1->m[1][3] * in2->m[3][1];
	out->m[1][2] = in1->m[1][0] * in2->m[0][2] + in1->m[1][1] * in2->m[1][2] + in1->m[1][2] * in2->m[2][2] + in1->m[1][3] * in2->m[3][2];
	out->m[1][3] = in1->m[1][0] * in2->m[0][3] + in1->m[1][1] * in2->m[1][3] + in1->m[1][2] * in2->m[2][3] + in1->m[1][3] * in2->m[3][3];
	out->m[2][0] = in1->m[2][0] * in2->m[0][0] + in1->m[2][1] * in2->m[1][0] + in1->m[2][2] * in2->m[2][0] + in1->m[2][3] * in2->m[3][0];
	out->m[2][1] = in1->m[2][0] * in2->m[0][1] + in1->m[2][1] * in2->m[1][1] + in1->m[2][2] * in2->m[2][1] + in1->m[2][3] * in2->m[3][1];
	out->m[2][2] = in1->m[2][0] * in2->m[0][2] + in1->m[2][1] * in2->m[1][2] + in1->m[2][2] * in2->m[2][2] + in1->m[2][3] * in2->m[3][2];
	out->m[2][3] = in1->m[2][0] * in2->m[0][3] + in1->m[2][1] * in2->m[1][3] + in1->m[2][2] * in2->m[2][3] + in1->m[2][3] * in2->m[3][3];
	out->m[3][0] = in1->m[3][0] * in2->m[0][0] + in1->m[3][1] * in2->m[1][0] + in1->m[3][2] * in2->m[2][0] + in1->m[3][3] * in2->m[3][0];
	out->m[3][1] = in1->m[3][0] * in2->m[0][1] + in1->m[3][1] * in2->m[1][1] + in1->m[3][2] * in2->m[2][1] + in1->m[3][3] * in2->m[3][1];
	out->m[3][2] = in1->m[3][0] * in2->m[0][2] + in1->m[3][1] * in2->m[1][2] + in1->m[3][2] * in2->m[2][2] + in1->m[3][3] * in2->m[3][2];
	out->m[3][3] = in1->m[3][0] * in2->m[0][3] + in1->m[3][1] * in2->m[1][3] + in1->m[3][2] * in2->m[2][3] + in1->m[3][3] * in2->m[3][3];
}

void mat4x4f_concat_with(mat4x4f_t *out, const mat4x4f_t *in)
{
	mat4x4f_t outcopy = *out;
	mat4x4f_concat(out, &outcopy, in);
}

void mat4x4f_concat_translate(mat4x4f_t *out, float x, float y, float z)
{
	mat4x4f_t m, r;

	mat4x4f_create_translate(&m, x, y, z);
	mat4x4f_concat(&r, out, &m);

	*out = r;
}

void mat4x4f_concat_rotate(mat4x4f_t *out, float angle, float x, float y, float z)
{
	mat4x4f_t m, r;

	mat4x4f_create_rotate(&m, angle, x, y, z);
	mat4x4f_concat(&r, out, &m);

	*out = r;
}

/* blends two matrices together, at a given percentage (blend controls percentage of in2) */
void mat4x4f_blend(mat4x4f_t *out, const mat4x4f_t *in1, const mat4x4f_t *in2, float blend)
{
	float iblend = 1 - blend;
	out->m[0][0] = in1->m[0][0] * iblend + in2->m[0][0] * blend;
	out->m[0][1] = in1->m[0][1] * iblend + in2->m[0][1] * blend;
	out->m[0][2] = in1->m[0][2] * iblend + in2->m[0][2] * blend;
	out->m[0][3] = in1->m[0][3] * iblend + in2->m[0][3] * blend;
	out->m[1][0] = in1->m[1][0] * iblend + in2->m[1][0] * blend;
	out->m[1][1] = in1->m[1][1] * iblend + in2->m[1][1] * blend;
	out->m[1][2] = in1->m[1][2] * iblend + in2->m[1][2] * blend;
	out->m[1][3] = in1->m[1][3] * iblend + in2->m[1][3] * blend;
	out->m[2][0] = in1->m[2][0] * iblend + in2->m[2][0] * blend;
	out->m[2][1] = in1->m[2][1] * iblend + in2->m[2][1] * blend;
	out->m[2][2] = in1->m[2][2] * iblend + in2->m[2][2] * blend;
	out->m[2][3] = in1->m[2][3] * iblend + in2->m[2][3] * blend;
	out->m[3][0] = in1->m[3][0] * iblend + in2->m[3][0] * blend;
	out->m[3][1] = in1->m[3][1] * iblend + in2->m[3][1] * blend;
	out->m[3][2] = in1->m[3][2] * iblend + in2->m[3][2] * blend;
	out->m[3][3] = in1->m[3][3] * iblend + in2->m[3][3] * blend;
}

void mat4x4f_transform(const mat4x4f_t *in, const float v[3], float out[3])
{
	out[0] = v[0] * in->m[0][0] + v[1] * in->m[0][1] + v[2] * in->m[0][2] + in->m[0][3];
	out[1] = v[0] * in->m[1][0] + v[1] * in->m[1][1] + v[2] * in->m[1][2] + in->m[1][3];
	out[2] = v[0] * in->m[2][0] + v[1] * in->m[2][1] + v[2] * in->m[2][2] + in->m[2][3];
}

void mat4x4f_transform_3x3(const mat4x4f_t *in, const float v[3], float out[3])
{
	out[0] = v[0] * in->m[0][0] + v[1] * in->m[0][1] + v[2] * in->m[0][2];
	out[1] = v[0] * in->m[1][0] + v[1] * in->m[1][1] + v[2] * in->m[1][2];
	out[2] = v[0] * in->m[2][0] + v[1] * in->m[2][1] + v[2] * in->m[2][2];
}

void mat4x4f_transpose(mat4x4f_t *out)
{
	mat4x4f_t temp = *out;

	out->m[0][0] = temp.m[0][0];
	out->m[0][1] = temp.m[1][0];
	out->m[0][2] = temp.m[2][0];
	out->m[0][3] = temp.m[3][0];
	out->m[1][0] = temp.m[0][1];
	out->m[1][1] = temp.m[1][1];
	out->m[1][2] = temp.m[2][1];
	out->m[1][3] = temp.m[3][1];
	out->m[2][0] = temp.m[0][2];
	out->m[2][1] = temp.m[1][2];
	out->m[2][2] = temp.m[2][2];
	out->m[2][3] = temp.m[3][2];
	out->m[3][0] = temp.m[0][3];
	out->m[3][1] = temp.m[1][3];
	out->m[3][2] = temp.m[2][3];
	out->m[3][3] = temp.m[3][3];
}

void mat4x4f_invert_simple(mat4x4f_t *out, const mat4x4f_t *in1)
{
	/* we only support uniform scaling, so assume the first row is enough
	   (note the lack of sqrt here, because we're trying to undo the scaling,
	   this means multiplying by the inverse scale twice - squaring it, which
	   makes the sqrt a waste of time)
	*/
#if 1
	float scale = 1 / (in1->m[0][0] * in1->m[0][0] + in1->m[0][1] * in1->m[0][1] + in1->m[0][2] * in1->m[0][2]);
#else
	float scale = 3 / (float)sqrt
		 (in1->m[0][0] * in1->m[0][0] + in1->m[0][1] * in1->m[0][1] + in1->m[0][2] * in1->m[0][2]
		+ in1->m[1][0] * in1->m[1][0] + in1->m[1][1] * in1->m[1][1] + in1->m[1][2] * in1->m[1][2]
		+ in1->m[2][0] * in1->m[2][0] + in1->m[2][1] * in1->m[2][1] + in1->m[2][2] * in1->m[2][2]);
	scale *= scale;
#endif

	/* invert the rotation by transposing and multiplying by the squared
	   recipricol of the input matrix scale as described above
	*/
	out->m[0][0] = in1->m[0][0] * scale;
	out->m[0][1] = in1->m[1][0] * scale;
	out->m[0][2] = in1->m[2][0] * scale;
	out->m[1][0] = in1->m[0][1] * scale;
	out->m[1][1] = in1->m[1][1] * scale;
	out->m[1][2] = in1->m[2][1] * scale;
	out->m[2][0] = in1->m[0][2] * scale;
	out->m[2][1] = in1->m[1][2] * scale;
	out->m[2][2] = in1->m[2][2] * scale;

	/* invert the translate */
	out->m[0][3] = -(in1->m[0][3] * out->m[0][0] + in1->m[1][3] * out->m[0][1] + in1->m[2][3] * out->m[0][2]);
	out->m[1][3] = -(in1->m[0][3] * out->m[1][0] + in1->m[1][3] * out->m[1][1] + in1->m[2][3] * out->m[1][2]);
	out->m[2][3] = -(in1->m[0][3] * out->m[2][0] + in1->m[1][3] * out->m[2][1] + in1->m[2][3] * out->m[2][2]);

	/* don't know if there's anything worth doing here */
	out->m[3][0] = 0;
	out->m[3][1] = 0;
	out->m[3][2] = 0;
	out->m[3][3] = 1;
}
