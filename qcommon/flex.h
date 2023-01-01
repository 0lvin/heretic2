//
// Copyright 1998 Raven Software
//
// Heretic II
//
// Generic flexible format

#define RAVENFMHEADER		(('d'<<24)+('a'<<16)+('e'<<8)+'h')

typedef struct
{
	char	ident[32];
	int		version;
	int		size;
} header_t;

void WriteHeader(FILE *, char *, int, int, void *);

// end
