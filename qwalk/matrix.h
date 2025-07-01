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

#ifndef MATRIX_H
#define MATRIX_H

typedef struct { float m[4][4]; } mat4x4f_t;

extern const mat4x4f_t mat4x4f_identity;

void mat4x4f_create_identity(mat4x4f_t *out);
void mat4x4f_create_translate(mat4x4f_t *out, float x, float y, float z);
void mat4x4f_create_rotate(mat4x4f_t *out, float angle, float x, float y, float z);

void mat4x4f_concat(mat4x4f_t *out, const mat4x4f_t *in1, const mat4x4f_t *in2);
void mat4x4f_concat_with(mat4x4f_t *out, const mat4x4f_t *in);
void mat4x4f_concat_translate(mat4x4f_t *out, float x, float y, float z);
void mat4x4f_concat_rotate(mat4x4f_t *out, float angle, float x, float y, float z);

void mat4x4f_blend(mat4x4f_t *out, const mat4x4f_t *in1, const mat4x4f_t *in2, float blend);
void mat4x4f_transform(const mat4x4f_t *in, const float v[3], float out[3]);
void mat4x4f_transform_3x3(const mat4x4f_t *in, const float v[3], float out[3]);

void mat4x4f_transpose(mat4x4f_t *out);
void mat4x4f_invert_simple(mat4x4f_t *out, const mat4x4f_t *in1);

#endif
