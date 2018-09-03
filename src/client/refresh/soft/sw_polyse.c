/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// d_polyset.c: routines for drawing sets of polygons sharing the same
// texture (used for Alias models)

#include "header/local.h"

#define MASK_1K	0x3FF

typedef struct {
	int	isflattop;
	int	numleftedges;
	int	*pleftedgevert0;
	int	*pleftedgevert1;
	int	*pleftedgevert2;
	int	numrightedges;
	int	*prightedgevert0;
	int	*prightedgevert1;
	int	*prightedgevert2;
} edgetable;

static int	ubasestep, errorterm, erroradjustup, erroradjustdown;

static int	r_p0[6], r_p1[6], r_p2[6];

static int	d_xdenom;

static edgetable *pedgetable;

static edgetable edgetables[12] = {
	{0, 1, r_p0, r_p2, NULL, 2, r_p0, r_p1, r_p2},
	{0, 2, r_p1, r_p0, r_p2, 1, r_p1, r_p2, NULL},
	{1, 1, r_p0, r_p2, NULL, 1, r_p1, r_p2, NULL},
	{0, 1, r_p1, r_p0, NULL, 2, r_p1, r_p2, r_p0},
	{0, 2, r_p0, r_p2, r_p1, 1, r_p0, r_p1, NULL},
	{0, 1, r_p2, r_p1, NULL, 1, r_p2, r_p0, NULL},
	{0, 1, r_p2, r_p1, NULL, 2, r_p2, r_p0, r_p1},
	{0, 2, r_p2, r_p1, r_p0, 1, r_p2, r_p0, NULL},
	{0, 1, r_p1, r_p0, NULL, 1, r_p1, r_p2, NULL},
	{1, 1, r_p2, r_p1, NULL, 1, r_p0, r_p1, NULL},
	{1, 1, r_p1, r_p0, NULL, 1, r_p2, r_p0, NULL},
	{0, 1, r_p0, r_p2, NULL, 1, r_p0, r_p1, NULL},
};

// FIXME: some of these can become statics
static int	a_sstepxfrac, a_tstepxfrac, r_lstepx, a_ststepxwhole;
static int	r_sstepx, r_tstepx, r_lstepy, r_sstepy, r_tstepy;
static zvalue_t	r_zistepx, r_zistepy;
static int	d_aspancount, d_countextrastep;

static spanpackage_t	*d_pedgespanpackage;

spanpackage_t	*triangle_spans, *triangles_max;

static int	ystart;
static pixel_t	*d_pdest, *d_ptex;
static zvalue_t	*d_pz;
static int	d_sfrac, d_tfrac, d_light;
static zvalue_t	d_zi;
static int	d_ptexextrastep, d_sfracextrastep;
static int	d_tfracextrastep, d_lightextrastep, d_pdestextrastep;
static int	d_lightbasestep, d_pdestbasestep, d_ptexbasestep;
static int	d_sfracbasestep, d_tfracbasestep;
static zvalue_t	d_ziextrastep, d_zibasestep;
static zvalue_t	d_pzextrastep, d_pzbasestep;

typedef struct {
	int		quotient;
	int		remainder;
} adivtab_t;

static adivtab_t	adivtab[32*32] = {
#include "../constants/adivtab.h"
};

static byte	*skintable[MAX_LBM_HEIGHT];
int		skinwidth;
static byte	*skinstart;

void	(*d_pdrawspans)(spanpackage_t *pspanpackage);

void R_PolysetDrawSpans8_33 (spanpackage_t *pspanpackage);
void R_PolysetDrawSpans8_66 (spanpackage_t *pspanpackage);
void R_PolysetDrawSpans8_Opaque (spanpackage_t *pspanpackage);

static void R_PolysetCalcGradients (int skinwidth);
static void R_PolysetSetEdgeTable (void);
static void R_RasterizeAliasPolySmooth (void);
static void R_PolysetScanLeftEdge_C(int height);

// ======================
// PGM
// 64 65 66 67 68 69 70 71   72 73 74 75 76 77 78 79
static const byte irtable[256] = {
	79, 78, 77, 76, 75, 74, 73, 72,		// black/white
	71, 70, 69, 68, 67, 66, 65, 64,
	64, 65, 66, 67, 68, 69, 70, 71,		// dark taupe
	72, 73, 74, 75, 76, 77, 78, 79,

	64, 65, 66, 67, 68, 69, 70, 71,		// slate grey
	72, 73, 74, 75, 76, 77, 78, 79,
	208, 208, 208, 208, 208, 208, 208, 208,	// unused?'
	64, 66, 68, 70, 72, 74, 76, 78,		// dark yellow

	64, 65, 66, 67, 68, 69, 70, 71,		// dark red
	72, 73, 74, 75, 76, 77, 78, 79,
	64, 65, 66, 67, 68, 69, 70, 71,		// grey/tan
	72, 73, 74, 75, 76, 77, 78, 79,

	64, 66, 68, 70, 72, 74, 76, 78,		// chocolate
	68, 67, 66, 65, 64, 65, 66, 67,		// mauve / teal
	68, 69, 70, 71, 72, 73, 74, 75,
	76, 76, 77, 77, 78, 78, 79, 79,

	64, 65, 66, 67, 68, 69, 70, 71,		// more mauve
	72, 73, 74, 75, 76, 77, 78, 79,
	64, 65, 66, 67, 68, 69, 70, 71,		// olive
	72, 73, 74, 75, 76, 77, 78, 79,

	64, 65, 66, 67, 68, 69, 70, 71,		// maroon
	72, 73, 74, 75, 76, 77, 78, 79,
	64, 65, 66, 67, 68, 69, 70, 71,		// sky blue
	72, 73, 74, 75, 76, 77, 78, 79,

	64, 65, 66, 67, 68, 69, 70, 71,		// olive again
	72, 73, 74, 75, 76, 77, 78, 79,
	64, 65, 66, 67, 68, 69, 70, 71,		// nuclear green
	64, 65, 66, 67, 68, 69, 70, 71,		// bright yellow

	64, 65, 66, 67, 68, 69, 70, 71,		// fire colors
	72, 73, 74, 75, 76, 77, 78, 79,
	208, 208, 64, 64, 70, 71, 72, 64,		// mishmash1
	66, 68, 70, 64, 65, 66, 67, 68};		// mishmash2

// PGM
// ======================

/*
================
R_PolysetUpdateTables
================
*/
void
R_PolysetUpdateTables (void)
{
	byte	*s;

	if (r_affinetridesc.skinwidth != skinwidth ||
		r_affinetridesc.pskin != skinstart)
	{
		int i;

		skinwidth = r_affinetridesc.skinwidth;
		skinstart = r_affinetridesc.pskin;
		s = skinstart;
		for (i=0 ; i<MAX_LBM_HEIGHT ; i++, s+=skinwidth)
			skintable[i] = s;
	}
}

/*
================
R_DrawTriangle
================
*/
void
R_DrawTriangle(const finalvert_t *a, const finalvert_t *b, const finalvert_t *c)
{
	int dv1_ab, dv0_ac;
	int dv0_ab, dv1_ac;

	/*
	d_xdenom = ( a->v[1] - b->v[1] ) * ( a->v[0] - c->v[0] ) -
			   ( a->v[0] - b->v[0] ) * ( a->v[1] - c->v[1] );
	*/

	dv0_ab = a->u - b->u;
	dv1_ab = a->v - b->v;

	if ( !( dv0_ab | dv1_ab ) )
		return;

	dv0_ac = a->u - c->u;
	dv1_ac = a->v - c->v;

	if ( !( dv0_ac | dv1_ac ) )
		return;

	d_xdenom = ( dv0_ac * dv1_ab ) - ( dv0_ab * dv1_ac );

	if ( d_xdenom < 0 )
	{
		r_p0[0] = a->u;	// u
		r_p0[1] = a->v;	// v
		r_p0[2] = a->s;	// s
		r_p0[3] = a->t;	// t
		r_p0[4] = a->l;	// light
		r_p0[5] = a->zi;	// iz

		r_p1[0] = b->u;
		r_p1[1] = b->v;
		r_p1[2] = b->s;
		r_p1[3] = b->t;
		r_p1[4] = b->l;
		r_p1[5] = b->zi;

		r_p2[0] = c->u;
		r_p2[1] = c->v;
		r_p2[2] = c->s;
		r_p2[3] = c->t;
		r_p2[4] = c->l;
		r_p2[5] = c->zi;

		R_PolysetSetEdgeTable ();
		R_RasterizeAliasPolySmooth ();
	}
}

static void
R_PushEdgesSpan()
{
	if (d_pedgespanpackage >= triangles_max)
	{
		// no space any more
		r_outoftriangles++;
		return;
	}

	d_pedgespanpackage->pdest = d_pdest;
	d_pedgespanpackage->pz = d_pz;
	d_pedgespanpackage->count = d_aspancount;
	d_pedgespanpackage->ptex = d_ptex;

	d_pedgespanpackage->sfrac = d_sfrac;
	d_pedgespanpackage->tfrac = d_tfrac;

	// FIXME: need to clamp l, s, t, at both ends?
	d_pedgespanpackage->light = d_light;
	d_pedgespanpackage->zi = d_zi;

	d_pedgespanpackage++;
}

/*
===================
R_PolysetScanLeftEdge_C
====================
*/
static void
R_PolysetScanLeftEdge_C(int height)
{
	do
	{
		R_PushEdgesSpan();

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_pdest += d_pdestextrastep;
			d_pz += d_pzextrastep;
			d_aspancount += d_countextrastep;
			d_ptex += d_ptexextrastep;
			d_sfrac += d_sfracextrastep;
			d_ptex += d_sfrac >> SHIFT16XYZ;

			d_sfrac &= 0xFFFF;
			d_tfrac += d_tfracextrastep;
			if (d_tfrac & 0x10000)
			{
				d_ptex += r_affinetridesc.skinwidth;
				d_tfrac &= 0xFFFF;
			}
			d_light += d_lightextrastep;
			d_zi += d_ziextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_pdest += d_pdestbasestep;
			d_pz += d_pzbasestep;
			d_aspancount += ubasestep;
			d_ptex += d_ptexbasestep;
			d_sfrac += d_sfracbasestep;
			d_ptex += d_sfrac >> SHIFT16XYZ;
			d_sfrac &= 0xFFFF;
			d_tfrac += d_tfracbasestep;
			if (d_tfrac & 0x10000)
			{
				d_ptex += r_affinetridesc.skinwidth;
				d_tfrac &= 0xFFFF;
			}
			d_light += d_lightbasestep;
			d_zi += d_zibasestep;
		}
	} while (--height);
}

/*
===================
FloorDivMod

Returns mathematically correct (floor-based) quotient and remainder for
numer and denom, both of which should contain no fractional part. The
quotient must fit in 32 bits.
FIXME: GET RID OF THIS! (FloorDivMod)
====================
*/
static void
FloorDivMod (float numer, float denom, int *quotient,
		int *rem)
{
	int		q, r;
	float	x;

	if (numer >= 0.0)
	{

		x = floor(numer / denom);
		q = (int)x;
		r = (int)floor(numer - (x * denom));
	}
	else
	{
		//
		// perform operations with positive values, and fix mod to make floor-based
		//
		x = floor(-numer / denom);
		q = -(int)x;
		r = (int)floor(-numer - (x * denom));
		if (r != 0)
		{
			q--;
			r = (int)denom - r;
		}
	}

	*quotient = q;
	*rem = r;
}


/*
===================
R_PolysetSetUpForLineScan
====================
*/
static void
R_PolysetSetUpForLineScan(fixed8_t startvertu, fixed8_t startvertv,
		fixed8_t endvertu, fixed8_t endvertv)
{
	int		tm, tn;
	adivtab_t	*ptemp;

	errorterm = -1;

	tm = endvertu - startvertu;
	tn = endvertv - startvertv;

	if (((tm <= 16) && (tm >= -15)) &&
		((tn <= 16) && (tn >= -15)))
	{
		ptemp = &adivtab[((tm+15) << 5) + (tn+15)];
		ubasestep = ptemp->quotient;
		erroradjustup = ptemp->remainder;
		erroradjustdown = tn;
	}
	else
	{
		float dm, dn;

		dm = tm;
		dn = tn;

		FloorDivMod (dm, dn, &ubasestep, &erroradjustup);

		erroradjustdown = dn;
	}
}



/*
================
R_PolysetCalcGradients
================
*/
static void
R_PolysetCalcGradients (int skinwidth)
{
	float	xstepdenominv, ystepdenominv, t0, t1;
	float	p01_minus_p21, p11_minus_p21, p00_minus_p20, p10_minus_p20;

	p00_minus_p20 = r_p0[0] - r_p2[0];
	p01_minus_p21 = r_p0[1] - r_p2[1];
	p10_minus_p20 = r_p1[0] - r_p2[0];
	p11_minus_p21 = r_p1[1] - r_p2[1];

	xstepdenominv = 1.0 / (float)d_xdenom;

	ystepdenominv = -xstepdenominv;

	// ceil () for light so positive steps are exaggerated, negative steps
	// diminished,  pushing us away from underflow toward overflow. Underflow is
	// very visible, overflow is very unlikely, because of ambient lighting
	t0 = r_p0[4] - r_p2[4];
	t1 = r_p1[4] - r_p2[4];
	r_lstepx = (int)
			ceil((t1 * p01_minus_p21 - t0 * p11_minus_p21) * xstepdenominv);
	r_lstepy = (int)
			ceil((t1 * p00_minus_p20 - t0 * p10_minus_p20) * ystepdenominv);

	t0 = r_p0[2] - r_p2[2];
	t1 = r_p1[2] - r_p2[2];
	r_sstepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) *
			xstepdenominv);
	r_sstepy = (int)((t1 * p00_minus_p20 - t0* p10_minus_p20) *
			ystepdenominv);

	t0 = r_p0[3] - r_p2[3];
	t1 = r_p1[3] - r_p2[3];
	r_tstepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) *
			xstepdenominv);
	r_tstepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) *
			ystepdenominv);

	t0 = r_p0[5] - r_p2[5];
	t1 = r_p1[5] - r_p2[5];
	r_zistepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) *
			xstepdenominv);
	r_zistepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) *
			ystepdenominv);

	{
		a_sstepxfrac = r_sstepx & 0xFFFF;
		a_tstepxfrac = r_tstepx & 0xFFFF;
	}

	a_ststepxwhole = skinwidth * (r_tstepx >> SHIFT16XYZ) + (r_sstepx >> SHIFT16XYZ);
}


/*
================
R_PolysetDrawSpans8
================
*/
void
R_PolysetDrawSpans8_33( spanpackage_t *pspanpackage)
{
	byte		*lpdest;
	byte		*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	zvalue_t	lzi;
	zvalue_t	*lpz;

	do
	{
		int lcount;

		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> SHIFT16XYZ) >= *lpz)
				{
					int temp = vid_colormap[*lptex + ( llight & 0xFF00 )];

					*lpdest = vid_alphamap[temp+ *lpdest*256];
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_lstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> SHIFT16XYZ;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}

void
R_PolysetDrawSpansConstant8_33( spanpackage_t *pspanpackage)
{
	pixel_t		*lpdest;
	int		lzi;
	zvalue_t	*lpz;

	do
	{
		int lcount;

		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = pspanpackage->pdest;
			lpz = pspanpackage->pz;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> SHIFT16XYZ) >= *lpz)
				{
					*lpdest = vid_alphamap[r_aliasblendcolor + *lpdest*256];
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}

void
R_PolysetDrawSpans8_66(spanpackage_t *pspanpackage)
{
	pixel_t		*lpdest;
	pixel_t		*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	zvalue_t	lzi;
	zvalue_t	*lpz;

	do
	{
		int lcount;

		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> SHIFT16XYZ) >= *lpz)
				{
					int temp = vid_colormap[*lptex + ( llight & 0xFF00 )];

					*lpdest = vid_alphamap[temp*256 + *lpdest];
					*lpz = lzi >> SHIFT16XYZ;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_lstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> SHIFT16XYZ;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}

void
R_PolysetDrawSpansConstant8_66( spanpackage_t *pspanpackage)
{
	pixel_t		*lpdest;
	zvalue_t	lzi;
	zvalue_t	*lpz;

	do
	{
		int lcount;

		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = pspanpackage->pdest;
			lpz = pspanpackage->pz;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> SHIFT16XYZ) >= *lpz)
				{
					*lpdest = vid_alphamap[r_aliasblendcolor*256 + *lpdest];
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}

void
R_PolysetDrawSpans8_Opaque (spanpackage_t *pspanpackage)
{
	do
	{
		int lcount;

		lcount = d_aspancount - pspanpackage->count;
		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			int		lsfrac, ltfrac;
			pixel_t		*lpdest;
			pixel_t		*lptex;
			int		llight;
			zvalue_t	lzi;
			zvalue_t	*lpz;

			lpdest = pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> SHIFT16XYZ) >= *lpz)
				{
					//PGM
					if(r_newrefdef.rdflags & RDF_IRGOGGLES && currententity->flags & RF_IR_VISIBLE)
						*lpdest = ((byte *)vid_colormap)[irtable[*lptex]];
					else
						*lpdest = ((byte *)vid_colormap)[*lptex + (llight & 0xFF00)];
					//PGM

					*lpz = lzi >> SHIFT16XYZ;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_lstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> SHIFT16XYZ;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}

/*
================
R_RasterizeAliasPolySmooth
================
*/
static void
R_RasterizeAliasPolySmooth (void)
{
	int	initialleftheight, initialrightheight;
	int	*plefttop, *prighttop, *pleftbottom, *prightbottom;
	int	working_lstepx, originalcount;

	plefttop = pedgetable->pleftedgevert0;
	prighttop = pedgetable->prightedgevert0;

	pleftbottom = pedgetable->pleftedgevert1;
	prightbottom = pedgetable->prightedgevert1;

	initialleftheight = pleftbottom[1] - plefttop[1];
	initialrightheight = prightbottom[1] - prighttop[1];

	//
	// set the s, t, and light gradients, which are consistent across the triangle
	// because being a triangle, things are affine
	//
	R_PolysetCalcGradients (r_affinetridesc.skinwidth);
	//
	// rasterize the polygon
	//

	//
	// scan out the top (and possibly only) part of the left edge
	//
	d_pedgespanpackage = triangle_spans;

	ystart = plefttop[1];
	d_aspancount = plefttop[0] - prighttop[0];

	d_ptex = (byte *)r_affinetridesc.pskin + (plefttop[2] >> SHIFT16XYZ) +
			(plefttop[3] >> SHIFT16XYZ) * r_affinetridesc.skinwidth;
	{
		d_sfrac = plefttop[2] & 0xFFFF;
		d_tfrac = plefttop[3] & 0xFFFF;
	}
	d_light = plefttop[4];
	d_zi = plefttop[5];

	d_pdest = d_viewbuffer + ystart * vid.width + plefttop[0];
	d_pz = d_pzbuffer + ystart * vid.width + plefttop[0];

	if (initialleftheight == 1)
	{
		R_PushEdgesSpan();
	}
	else
	{
		R_PolysetSetUpForLineScan(plefttop[0], plefttop[1],
					  pleftbottom[0], pleftbottom[1]);
		{
			d_pzbasestep = vid.width + ubasestep;
			d_pzextrastep = d_pzbasestep + 1;
		}

		d_pdestbasestep = vid.width + ubasestep;
		d_pdestextrastep = d_pdestbasestep + 1;

		// TODO: can reuse partial expressions here

		// for negative steps in x along left edge, bias toward overflow rather than
		// underflow (sort of turning the floor () we did in the gradient calcs into
		// ceil (), but plus a little bit)
		if (ubasestep < 0)
			working_lstepx = r_lstepx - 1;
		else
			working_lstepx = r_lstepx;

		d_countextrastep = ubasestep + 1;
		d_ptexbasestep = ((r_sstepy + r_sstepx * ubasestep) >> SHIFT16XYZ) +
				((r_tstepy + r_tstepx * ubasestep) >> SHIFT16XYZ) *
				r_affinetridesc.skinwidth;
		{
			d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) & 0xFFFF;
			d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) & 0xFFFF;
		}
		d_lightbasestep = r_lstepy + working_lstepx * ubasestep;
		d_zibasestep = r_zistepy + r_zistepx * ubasestep;

		d_ptexextrastep = ((r_sstepy + r_sstepx * d_countextrastep) >> SHIFT16XYZ) +
				((r_tstepy + r_tstepx * d_countextrastep) >> SHIFT16XYZ) *
				r_affinetridesc.skinwidth;
		{
			d_sfracextrastep = (r_sstepy + r_sstepx*d_countextrastep) & 0xFFFF;
			d_tfracextrastep = (r_tstepy + r_tstepx*d_countextrastep) & 0xFFFF;
		}
		d_lightextrastep = d_lightbasestep + working_lstepx;
		d_ziextrastep = d_zibasestep + r_zistepx;

		{
			R_PolysetScanLeftEdge_C(initialleftheight);
		}
	}

	//
	// scan out the bottom part of the left edge, if it exists
	//
	if (pedgetable->numleftedges == 2)
	{
		int	height;

		plefttop = pleftbottom;
		pleftbottom = pedgetable->pleftedgevert2;

		height = pleftbottom[1] - plefttop[1];

		// TODO: make this a function; modularize this function in general

		ystart = plefttop[1];
		d_aspancount = plefttop[0] - prighttop[0];
		d_ptex = (byte *)r_affinetridesc.pskin + (plefttop[2] >> SHIFT16XYZ) +
				(plefttop[3] >> SHIFT16XYZ) * r_affinetridesc.skinwidth;
		d_sfrac = 0;
		d_tfrac = 0;
		d_light = plefttop[4];
		d_zi = plefttop[5];

		d_pdest = d_viewbuffer + ystart * vid.width + plefttop[0];
		d_pz = d_pzbuffer + ystart * vid.width + plefttop[0];

		if (height == 1)
		{
			R_PushEdgesSpan();
		}
		else
		{
			R_PolysetSetUpForLineScan(plefttop[0], plefttop[1],
								  pleftbottom[0], pleftbottom[1]);

			d_pdestbasestep = vid.width + ubasestep;
			d_pdestextrastep = d_pdestbasestep + 1;

			{
				d_pzbasestep = vid.width + ubasestep;
				d_pzextrastep = d_pzbasestep + 1;
			}

			if (ubasestep < 0)
				working_lstepx = r_lstepx - 1;
			else
				working_lstepx = r_lstepx;

			d_countextrastep = ubasestep + 1;
			d_ptexbasestep = ((r_sstepy + r_sstepx * ubasestep) >> SHIFT16XYZ) +
					((r_tstepy + r_tstepx * ubasestep) >> SHIFT16XYZ) *
					r_affinetridesc.skinwidth;
			{
				d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) & 0xFFFF;
				d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) & 0xFFFF;
			}
			d_lightbasestep = r_lstepy + working_lstepx * ubasestep;
			d_zibasestep = r_zistepy + r_zistepx * ubasestep;

			d_ptexextrastep = ((r_sstepy + r_sstepx * d_countextrastep) >> SHIFT16XYZ) +
					((r_tstepy + r_tstepx * d_countextrastep) >> SHIFT16XYZ) *
					r_affinetridesc.skinwidth;
			{
				d_sfracextrastep = (r_sstepy+r_sstepx*d_countextrastep) & 0xFFFF;
				d_tfracextrastep = (r_tstepy+r_tstepx*d_countextrastep) & 0xFFFF;
			}
			d_lightextrastep = d_lightbasestep + working_lstepx;
			d_ziextrastep = d_zibasestep + r_zistepx;

			{
				R_PolysetScanLeftEdge_C(height);
			}
		}
	}

	// scan out the top (and possibly only) part of the right edge, updating the
	// count field
	d_pedgespanpackage = triangle_spans;

	R_PolysetSetUpForLineScan(prighttop[0], prighttop[1],
						  prightbottom[0], prightbottom[1]);
	d_aspancount = 0;
	d_countextrastep = ubasestep + 1;
	if ((triangle_spans + initialrightheight) >= triangles_max)
	{
		// we dont have enough triangles for save full height
		r_outoftriangles++;
		return;
	}
	originalcount = triangle_spans[initialrightheight].count;
	triangle_spans[initialrightheight].count = -999999; // mark end of the spanpackages
	(*d_pdrawspans) (triangle_spans);

	// scan out the bottom part of the right edge, if it exists
	if (pedgetable->numrightedges == 2)
	{
		int				height;
		spanpackage_t	*pstart;

		pstart = triangle_spans + initialrightheight;
		pstart->count = originalcount;

		d_aspancount = prightbottom[0] - prighttop[0];

		prighttop = prightbottom;
		prightbottom = pedgetable->prightedgevert2;

		height = prightbottom[1] - prighttop[1];

		R_PolysetSetUpForLineScan(prighttop[0], prighttop[1],
							  prightbottom[0], prightbottom[1]);

		d_countextrastep = ubasestep + 1;

		if ((triangle_spans + initialrightheight + height) >= triangles_max)
		{
			// we dont have enough triangles for save full height
			r_outoftriangles++;
			return;
		}
		triangle_spans[initialrightheight + height].count = -999999; // mark end of the spanpackages
		(*d_pdrawspans) (pstart);
	}
}


/*
================
R_PolysetSetEdgeTable
================
*/
static void
R_PolysetSetEdgeTable (void)
{
	int			edgetableindex;

	edgetableindex = 0;	// assume the vertices are already in
				//  top to bottom order

	//
	// determine which edges are right & left, and the order in which
	// to rasterize them
	//
	if (r_p0[1] >= r_p1[1])
	{
		if (r_p0[1] == r_p1[1])
		{
			if (r_p0[1] < r_p2[1])
				pedgetable = &edgetables[2];
			else
				pedgetable = &edgetables[5];

			return;
		}
		else
		{
			edgetableindex = 1;
		}
	}

	if (r_p0[1] == r_p2[1])
	{
		if (edgetableindex)
			pedgetable = &edgetables[8];
		else
			pedgetable = &edgetables[9];

		return;
	}
	else if (r_p1[1] == r_p2[1])
	{
		if (edgetableindex)
			pedgetable = &edgetables[10];
		else
			pedgetable = &edgetables[11];

		return;
	}

	if (r_p0[1] > r_p2[1])
		edgetableindex += 2;

	if (r_p1[1] > r_p2[1])
		edgetableindex += 4;

	pedgetable = &edgetables[edgetableindex];
}
