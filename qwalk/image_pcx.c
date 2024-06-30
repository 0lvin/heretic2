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

#include <string.h>

#include "global.h"
#include "image.h"

typedef struct pcx_header_s
{
	char manufacturer;
	char version;
	char encoding;
	char bits_per_pixel;
	short xmin;
	short ymin;
	short xmax;
	short ymax;
	short hres;
	short vres;
	unsigned char palette[48];
	char reserved;
	char color_planes;
	short bytes_per_line;
	short palette_type;
	char filler[58];
} pcx_header_t;

/* FIXME - the +768 on the bad filesize error should be moved till after the header is checked.
 * otherwise very small 24-bit images will get a confusing error */

static bool_t image_pcx_load_header(pcx_header_t *header, void *filedata, size_t filesize, char **out_error)
{
	unsigned char *f = (unsigned char*)filedata;

	if (filesize < 128 + /* 1 + */ 768)
		return (void)(out_error && (*out_error = msprintf("pcx: file is too small too be a pcx"))), false;

/* load header byte by byte because we could be loading the pcx from in the
 * middle of an unaligned stream */
	header->manufacturer   = f[0];
	header->version        = f[1];
	header->encoding       = f[2];
	header->bits_per_pixel = f[3];
	header->xmin           = f[4] + (f[5] << 8);
	header->ymin           = f[6] + (f[7] << 8);
	header->xmax           = f[8] + (f[9] << 8);
	header->ymax           = f[10] + (f[11] << 8);
	header->hres           = f[12] + (f[13] << 8);
	header->vres           = f[14] + (f[15] << 8);
	memcpy(header->palette, f + 16, 48);
	header->reserved       = f[64];
	header->color_planes   = f[65];
	header->bytes_per_line = f[66] + (f[67] << 8);
	header->palette_type   = f[68] + (f[69] << 8);
	memcpy(header->filler, f + 70, 58);

	if (header->manufacturer != 0x0a)
		return (void)(out_error && (*out_error = msprintf("pcx: bad manufacturer"))), false;
	if (header->version != 5)
		return (void)(out_error && (*out_error = msprintf("pcx: bad version"))), false;
	if (header->encoding != 1)
		return (void)(out_error && (*out_error = msprintf("pcx: bad encoding"))), false;
	if (header->bits_per_pixel != 8)
		return (void)(out_error && (*out_error = msprintf("pcx: bad bits_per_pixel"))), false;
	if (header->xmax - header->xmin + 1 < 1 || header->xmax - header->xmin + 1 > 4096)
		return (void)(out_error && (*out_error = msprintf("pcx: bad xmax"))), false;
	if (header->ymax - header->ymin + 1 < 1 || header->ymax - header->ymin + 1 > 4096)
		return (void)(out_error && (*out_error = msprintf("pcx: bad ymax"))), false;
	if (header->color_planes != 1)
		return (void)(out_error && (*out_error = msprintf("pcx: bad color_planes"))), false;

/* header.palette_type should be 1, but some pcxes use 0 or 2 for some reason, so ignore it */

	return true;
}

image_paletted_t *image_pcx_load_paletted(mem_pool_t *pool, void *filedata, size_t filesize, char **out_error)
{
	image_paletted_t *image;
	unsigned char *f = (unsigned char*)filedata;
	pcx_header_t header;
	int x, y;

	if (!image_pcx_load_header(&header, f, filesize, out_error))
		return NULL;

	f += 128;

	image = image_paletted_alloc(pool, header.xmax - header.xmin + 1, header.ymax - header.ymin + 1);
	if (!image)
		return (void)(out_error && (*out_error = msprintf("pcx: out of memory"))), NULL;

	for (y = 0; y < header.ymax - header.ymin + 1; y++)
	{
		unsigned char *pix = image->pixels + y * image->width;

		for (x = 0; x < header.bytes_per_line; )
		{
			unsigned char databyte = *f++;
			int runlen;

			if ((databyte & 0xc0) == 0xc0)
			{
				runlen = databyte & 0x3f;
				databyte = *f++;
			}
			else
				runlen = 1;

			while (runlen-- > 0)
			{
				if (x < image->width)
					*pix++ = databyte;
				x++;
			}
		}
	}

/* grab the palette from the end of the file */
	memcpy(image->palette.rgb, (unsigned char*)filedata + filesize - 768, 768);
	memset(image->palette.fullbright_flags, 0, sizeof(image->palette.fullbright_flags));

	return image;
}

image_rgba_t *image_pcx_load(mem_pool_t *pool, void *filedata, size_t filesize, char **out_error)
{
	image_rgba_t *image;
	unsigned char *f = (unsigned char*)filedata;
	/*unsigned char *endf = f + filesize;*/
	pcx_header_t header;
	const unsigned char *palette768;
	int x, y;

	if (!image_pcx_load_header(&header, f, filesize, out_error))
		return NULL;

	f += 128;

/* grab the palette from the end of the file */
	palette768 = (unsigned char*)filedata + filesize - 768;

/* there is supposed to be an extra byte, 0x0c, before the palette. but, qME
 * forgets this when exporting pcxes, so we'll have to let it slide */
/*	if (palette768[-1] != 0x0c)
		return (out_error && (*out_error = msprintf("pcx: bad palette format"))), NULL;*/

	image = image_alloc(pool, header.xmax - header.xmin + 1, header.ymax - header.ymin + 1);
	if (!image)
		return (void)(out_error && (*out_error = msprintf("pcx: out of memory"))), NULL;

	for (y = 0; y < header.ymax - header.ymin + 1; y++)
	{
		unsigned char *pix = image->pixels + y * image->width * 4;

		for (x = 0; x < header.bytes_per_line; )
		{
			unsigned char databyte = *f++;
			int runlen;

			if ((databyte & 0xc0) == 0xc0)
			{
				runlen = databyte & 0x3f;
				databyte = *f++;
			}
			else
				runlen = 1;

			while (runlen-- > 0)
			{
				if (x < image->width)
				{
					pix[0] = palette768[databyte*3+0];
					pix[1] = palette768[databyte*3+1];
					pix[2] = palette768[databyte*3+2];
					pix[3] = 0xff;
					pix += 4;
				}
				x++;
			}
		}
	}

	return image;
}

bool_t image_pcx_save(const image_paletted_t *image, xbuf_t *xbuf, char **out_error)
{
	pcx_header_t header;
	int bytes_per_line, x, y;

/* PCX requires scanline length to be a multiple of 2, so round up */
	bytes_per_line = (image->width + 1) & ~1;

/* write header */
	header.manufacturer = 0x0a;
	header.version = 5;
	header.encoding = 1;
	header.bits_per_pixel = 8;
	header.xmin = LittleShort(0);
	header.ymin = LittleShort(0);
	header.xmax = LittleShort(image->width - 1);
	header.ymax = LittleShort(image->height - 1);
	header.hres = LittleShort(72); /* hres and vres are dots-per-inch values, unused */
	header.vres = LittleShort(72);
	memcpy(header.palette, image->palette.rgb, 48); /* not useful */
	header.reserved = 0;
	header.color_planes = 1;
	header.bytes_per_line = LittleShort(bytes_per_line);
	header.palette_type = LittleShort(1); /* according to pcx specs the accepted values are 1 (colour/bw) and 2 (greyscale) */
	memset(header.filler, 0, 58);

	xbuf_write_data(xbuf, sizeof(pcx_header_t), &header);

/* write image */
	for (y = 0; y < image->height; y++)
	{
		const unsigned char *pix = image->pixels + y * image->width;

		for (x = 0; x < bytes_per_line; )
		{
			unsigned char pix_x = (x < image->width) ? pix[x] : pix[image->width - 1];
			int runlen;

			for (runlen = 1; runlen < 63 && x + runlen < bytes_per_line; runlen++)
				if (pix_x != (((x + runlen) < image->width) ? pix[x + runlen] : pix[image->width - 1]))
					break;

			if (runlen == 1)
			{
				if ((pix_x & 0xc0) == 0xc0)
					xbuf_write_byte(xbuf, 0xc1);
				xbuf_write_byte(xbuf, pix_x);
			}
			else
			{
				xbuf_write_byte(xbuf, 0xc0 | runlen);
				xbuf_write_byte(xbuf, pix_x);
			}

			x += runlen;
		}
	}

/* write palette */
	xbuf_write_byte(xbuf, 0x0c);
	xbuf_write_data(xbuf, 768, image->palette.rgb);

/* done */
	return true;
}
