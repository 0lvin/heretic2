//
// Heretic II
// Copyright 1998 Raven Software
//
// H2Rand.c
//

#include "../../common/header/shared.h"

#include <stdio.h>

#define QRAND_MAX 32768
static unsigned int	holdrand = 0x89abcdef; // 64 bit support for iojamp

float flrand(float min, float max)
{
	float	result;

	holdrand = (holdrand * 214013L) + 2531011L;
	result = (float)(holdrand >> 17);						// 0 - 32767 range
	result = ((result * (max - min)) / (float)QRAND_MAX) + min;

	return(result);
}

int irand(int min, int max)
{
	holdrand = 214013 * holdrand + 2531011;
	return min + ((signed int)(((unsigned int)holdrand >> 17) * (max - min + 1)) >> 15);
}
