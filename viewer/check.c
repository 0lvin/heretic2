#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "render.h"

struct image_s *
insert_skin(const char *name, byte *data, int width, int realwidth,
	int height, int realheight, size_t size, imagetype_t type, int bits)
{
	return NULL;
}

struct image_s*
find_image(const char *name, imagetype_t type)
{
	return NULL;
}

int
main(int argc, char *argv[])
{
	if (argc < 2)
	{
		R_Printf(PRINT_ALL, "usage: %s <filename.md2> [<texture.png>]\n", argv[0]);
		return 0;
	}

	// quake swap prepere
	Swap_Init();

	model_load(argv[1]);
}
