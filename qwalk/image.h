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

#ifndef IMAGE_H
#define IMAGE_H

typedef struct palette_s
{
	unsigned char rgb[768];
	unsigned int fullbright_flags[8];
} palette_t;

/* 32-bit image */
typedef struct image_rgba_s
{
	int width, height;
	unsigned char *pixels;
} image_rgba_t;

/* 8-bit (256 colour) paletted image */
typedef struct image_paletted_s
{
	palette_t palette;

	int width, height;
	unsigned char *pixels;
} image_paletted_t;

image_rgba_t *image_load(mem_pool_t *pool, const char *filename, void *filedata, size_t filesize, char **out_error);
image_rgba_t *image_load_from_file(mem_pool_t *pool, const char *filename, char **out_error);

bool_t image_save(const char *filename, const image_rgba_t *image, char **out_error);
bool_t image_paletted_save(const char *filename, const image_paletted_t *image, char **out_error);

image_rgba_t *image_alloc(mem_pool_t *pool, int width, int height);
void image_free(image_rgba_t **image);

image_paletted_t *image_paletted_alloc(mem_pool_t *pool, int width, int height);
void image_paletted_free(image_paletted_t **image);

image_rgba_t *image_createfill(mem_pool_t *pool, int width, int height, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
image_rgba_t *image_clone(mem_pool_t *pool, const image_rgba_t *source);

image_paletted_t *image_pcx_load_paletted(mem_pool_t *pool, void *filedata, size_t filesize, char **out_error);
image_rgba_t *image_pcx_load(mem_pool_t *pool, void *filedata, size_t filesize, char **out_error);

bool_t image_pcx_save(const image_paletted_t *image, xbuf_t *xbuf, char **out_error);

image_paletted_t *image_palettize(mem_pool_t *pool, const palette_t *palette, const image_rgba_t *source_diffuse, const image_rgba_t *source_fullbright);
image_rgba_t *image_pad(mem_pool_t *pool, const image_rgba_t *source, int width, int height);
image_rgba_t *image_resize(mem_pool_t *pool, const image_rgba_t *source, int width, int height);

void image_drawpixel(image_rgba_t *image, int x, int y, unsigned char r, unsigned char g, unsigned char b);
void image_drawline(image_rgba_t *image, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b);

#endif
