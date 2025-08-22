//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef GENERICUNIONS_H
#define GENERICUNIONS_H

typedef struct paletteRGBA_s
{
	union
	{
		struct
		{
			byte r, g, b, a;
		};
		unsigned c;
		byte c_array[4];
	};
} paletteRGBA_t;

// don't add anything to this union which is greater than 4 bytes in size
// TODO and Rewrite: Neet total rewrite pointer is not 4 bytes at all
typedef union GenericUnion4_u
{
	byte t_byte;
	short t_short;
	int t_int;
	unsigned int t_uint;
	float t_float;
	float *t_float_p;
	struct edict_s *t_edict_p;
	void *t_void_p;
	paletteRGBA_t t_RGBA;
} GenericUnion4_t;

#endif
