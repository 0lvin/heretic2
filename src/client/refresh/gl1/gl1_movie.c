//
// Heretic II
// Copyright 1998 Raven Software
//
// gl_movie.c
//

#include "header/local.h"

image_t* cinematicImage = NULL;

qboolean R_Upload32(unsigned* data, int width, int height, qboolean mipmap);

void R_DrawInitCinematic(int w, int h, char *overlay, char *backdrop)
{

}

void R_DrawCloseCinematic()
{

}

void R_DrawCinematic(int width, int height, byte *data, paletteRGB_t *palette, float alpha)
{
	static byte imageScratch[800 * 600 * 4];

	paletteRGBA_t* dest = (paletteRGBA_t*)&imageScratch[0];

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			paletteRGB_t* p = &palette[data[(y * width) + x]];
			dest->r = p->r;
			dest->g = p->g;
			dest->b = p->b;
			dest->a = 255;
			dest++;
		}
	}

	if (cinematicImage == NULL)
	{
		cinematicImage = R_LoadPic("_cinematic", &imageScratch[0],
			width, width, height, height, width * height, it_pic, 32);
	}
	R_Bind(cinematicImage->texnum);
	R_Upload32((unsigned int *)&imageScratch[0], width, height, it_pic);
	Draw_Image(0, 0, vid.width, vid.height, -1.0f, false, cinematicImage);
}
