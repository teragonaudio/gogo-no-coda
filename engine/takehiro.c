/*
 *	MP3 huffman table selecting and bit counting
 *
 *	Copyright (c) 1999 Takehiro TOMINAGA
 *	Copyright (c) 2001,2002,2003 gogo-developer
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <assert.h>

#include "config.h"
#include "global.h"

#include "tables.h"
#include "quantize_pvt.h"
#include "cpu.h"
#include "vfta.h"

static const struct
{
	const int region0_count;
	const int region1_count;
} subdv_table[ 23 ] =
{
{0, 0}, /* 0 bands */
{0, 0}, /* 1 bands */
{0, 0}, /* 2 bands */
{0, 0}, /* 3 bands */
{0, 0}, /* 4 bands */
{0, 1}, /* 5 bands */
{1, 1}, /* 6 bands */
{1, 1}, /* 7 bands */
{1, 2}, /* 8 bands */
{2, 2}, /* 9 bands */
{2, 3}, /* 10 bands */
{2, 3}, /* 11 bands */
{3, 4}, /* 12 bands */
{3, 4}, /* 13 bands */
{3, 4}, /* 14 bands */
{4, 5}, /* 15 bands */
{4, 5}, /* 16 bands */
{4, 6}, /* 17 bands */
{5, 6}, /* 18 bands */
{5, 6}, /* 19 bands */
{5, 7}, /* 20 bands */
{6, 7}, /* 21 bands */
{6, 7}, /* 22 bands */
};



#ifdef CPU_I386
int choose_table_C(const int *ix, const int *end, int *s);
int choose_table_MMX(const int *ix, const int *end, int *s);
void setup_choose_table( int unit )
{
	if( unit & MU_tMMX ){
		choose_table = choose_table_MMX;
	}else
	{
		choose_table = choose_table_C;
	}
}
#endif

/*************************************************************************/
/*		  ix_max							 */
/*************************************************************************/

int 
ix_max(const int *ix, const int *end)
{
	int max1 = 0, max2 = 0;

	do {
	int x1 = *ix++;
	int x2 = *ix++;
	if (max1 < x1) 
		max1 = x1;

	if (max2 < x2) 
		max2 = x2;
	} while (ix < end);
	if (max1 < max2) 
	max1 = max2;
	return max1;
}


int
count_bit_ESC( 
	const int *	   ix, 
	const int * const end, 
		  int		 t1,
	const int		 t2,
		  int * const s )
{
	/* ESC-table is used */
	int linbits = ht[t1].xlen * 65536 + ht[t2].xlen;
	int sum = 0, sum2;

	do {
	int x = *ix++;
	int y = *ix++;

	if (x != 0) {
		if (x > 14) {
		x = 15;
		sum += linbits;
		}
		x *= 16;
	}

	if (y != 0) {
		if (y > 14) {
		y = 15;
		sum += linbits;
		}
		x += y;
	}

	sum += largetbl[x];
	} while (ix < end);

	sum2 = sum & 0xffff;
	sum >>= 16;

	if (sum > sum2) {
	sum = sum2;
	t1 = t2;
	}

	*s += sum;
	return t1;
}


static int
count_bit_noESC(const int * ix, const int * const end, int * const s)
{
	/* No ESC-words */
	int	sum1 = 0;
	const char *hlen1 = ht[1].hlen;

	do {
	int x = ix[0] * 2 + ix[1];
	ix += 2;
	sum1 += hlen1[x];
	} while (ix < end);

	*s += sum1;
	return 1;
}



static int
count_bit_noESC_from2(
	const int *	   ix, 
	const int * const end,
		  int		 t1,
		  int * const s )
{
	/* No ESC-words */
	unsigned int sum = 0, sum2;
	const int xlen = ht[t1].xlen;
	const unsigned int *hlen;
	if (t1 == 2)
	hlen = table23;
	else
	hlen = table56;

	do {
	int x = ix[0] * xlen + ix[1];
	ix += 2;
	sum += hlen[x];
	} while (ix < end);

	sum2 = sum & 0xffff;
	sum >>= 16;

	if (sum > sum2) {
	sum = sum2;
	t1++;
	}

	*s += sum;
	return t1;
}


static int
count_bit_noESC_from3(
	const int *	   ix, 
	const int * const end,
		  int		 t1,
		  int * const s )
{
	/* No ESC-words */
	int	sum1 = 0;
	int	sum2 = 0;
	int	sum3 = 0;
	const int xlen = ht[t1].xlen;
	const char *hlen1 = ht[t1].hlen;
	const char *hlen2 = ht[t1+1].hlen;
	const char *hlen3 = ht[t1+2].hlen;
	int t;

	do {
	int x = ix[0] * xlen + ix[1];
	ix += 2;
	sum1 += hlen1[x];
	sum2 += hlen2[x];
	sum3 += hlen3[x];
	} while (ix < end);

	t = t1;
	if (sum1 > sum2) {
	sum1 = sum2;
	t++;
	}
	if (sum1 > sum3) {
	sum1 = sum3;
	t = t1+2;
	}
	*s += sum1;

	return t;
}


/*************************************************************************/
/*		  choose table						 */
/*************************************************************************/

/*
  Choose the Huffman table that will encode ix[begin..end] with
  the fewest bits.

  Note: This code contains knowledge about the sizes and characteristics
  of the Huffman tables as defined in the IS (Table B.7), and will not work
  with any arbitrary tables.
*/

int choose_table_C( const int *ix, const int *end, int *s )
{
	int max;
	int choice, choice2;
	static const int huf_tbl_noESC[] = {
	1, 2, 5, 7, 7,10,10,13,13,13,13,13,13,13,13 /* char not enough ? */
	};

	max = ix_max(ix, end);

	switch (max) {
	case 0:
	return max;

	case 1:
	return count_bit_noESC(ix, end, s);

	case 2:
	case 3:
	return count_bit_noESC_from2(ix, end, huf_tbl_noESC[max - 1], s);

	case 4: case 5: case 6:
	case 7: case 8: case 9:
	case 10: case 11: case 12:
	case 13: case 14: case 15:
	return count_bit_noESC_from3(ix, end, huf_tbl_noESC[max - 1], s);

	default:
	/* try tables with linbits */
	if (max > IXMAX_VAL) {
		*s = LARGE_BITS;
		return -1;
	}
	max -= 15;
	for (choice2 = 24; choice2 < 32; choice2++) {
		if (ht[choice2].linmax >= max) {
		break;
		}
	}

	for (choice = choice2 - 8; choice < 24; choice++) {
		if (ht[choice].linmax >= max) {
		break;
		}
	}
	return count_bit_ESC(ix, end, choice, choice2, s);
	}
}



/*************************************************************************/
/*		  count_bit							 */
/*************************************************************************/

/* Count the number of bits necessary to code the subregion.   */

/* NEED asm */
int
count_bits_long(const int ix[576], gr_info * const gi)
{
	int i, a1, a2;
	int bits = 0;
	
	i = RO.ixend;
	/* Determine count1 region */
	for (; i > 1; i -= 2) {
		if (ix[i - 1] | ix[i - 2]) break;
	}
	gi->count1 = i;
	
	/* Determines the number of bits to encode the quadruples. */
	a1 = a2 = 0;
	for (; i > 3; i -= 4) {
		int p;
		/* hack to check if all values <= 1 */
		if ((unsigned int)(ix[i-1] | ix[i-2] | ix[i-3] | ix[i-4]) > 1) break;
		
		p = ((ix[i-4] * 2 + ix[i-3]) * 2 + ix[i-2]) * 2 + ix[i-1];
		a1 += t32l[p];
		a2 += t33l[p];
	}
	
	bits = a1;
	gi->count1table_select = 0;
	if (a1 > a2) {
		bits = a2;
		gi->count1table_select = 1;
	}
	
	gi->count1bits = bits;
	gi->big_values = i;
	if (i == 0)
		return bits;
	
	if (gi->block_type == SHORT_TYPE) {
		a1=3*RO.scalefac_band.s[3];
		if (a1 > gi->big_values) a1 = gi->big_values;
		a2 = gi->big_values;
		
	}else if (gi->block_type == NORM_TYPE) {
		assert(i <= 576); /* bv_scf has 576 entries (0..575) */
		a1 = gi->region0_count = RW.bv_scf[i-2];
		a2 = gi->region1_count = RW.bv_scf[i-1];
		
		assert(a1+a2+2 < SBPSY_l);
		a2 = RO.scalefac_band.l[a1 + a2 + 2];
		a1 = RO.scalefac_band.l[a1 + 1];
		if (a2 < i)
			gi->table_select[2] = choose_table(ix + a2, ix + i, &bits);
		
	} else {
		gi->region0_count = 7;
		/*gi->region1_count = SBPSY_l - 7 - 1;*/
		gi->region1_count = SBMAX_l -1 - 7 - 1;
		a1 = RO.scalefac_band.l[7 + 1];
		a2 = i;
		if (a1 > a2) {
			a1 = a2;
		}
	}
	
	
	/* have to allow for the case when bigvalues < region0 < region1 */
	/* (and region0, region1 are ignored) */
	a1 = Min(a1,i);
	a2 = Min(a2,i);
	
	assert( a1 >= 0 );
	assert( a2 >= 0 );
	
	/* Count the number of bits necessary to code the bigvalues region. */
	//clkbegin();
	if (0 < a1)
		gi->table_select[0] = choose_table(ix, ix + a1, &bits);
	if (a1 < a2)
		gi->table_select[1] = choose_table(ix + a1, ix + a2, &bits);
	//clkend();
	return bits;
}

#ifdef CPU_I386
void quantize_xrpow_ISO_FPU( const FLOAT8 xr[576], int ix[576], float *istepPtr );
void quantize_xrpow_ISO_3DN( const FLOAT8 xr[576], int ix[576], float *istepPtr );
void quantize_xrpow_ISO_SSE( const FLOAT8 xr[576], int ix[576], float *istepPtr );
void quantize_xrpow_ISO_SSE2( const FLOAT8 xr[576], int ix[576], float *istepPtr );
void setup_quantize_xrpow_ISO( int unit )
{
	if( unit & MU_tSSE2 ){
		quantize_xrpow_ISO = quantize_xrpow_ISO_SSE2;
	} else
	if( (unit & MU_tSSE) && (unit & MU_tMMX) && (unit & MU_t3DN) ){
		// for Athlon-XP
		quantize_xrpow_ISO = quantize_xrpow_ISO_3DN; // _SSE 735clk _3DN 675clk
	} else
	if( unit & MU_tSSE ){
		quantize_xrpow_ISO = quantize_xrpow_ISO_SSE;
	} else
	if( (unit & MU_tMMX) && (unit & MU_t3DN) ){
		quantize_xrpow_ISO = quantize_xrpow_ISO_3DN;
	}else
	{
		quantize_xrpow_ISO = quantize_xrpow_ISO_FPU;
	}
}
#else
void quantize_xrpow_ISO_C( const FLOAT8 xr[576], int ix[576], float *istepPtr )
{
    /* quantize on xr^(3/4) instead of xr */
	const float istep = *istepPtr;
    const FLOAT8 compareval0 = (1.0 - 0.4054)/istep;
    int j;
	/* 17Kclk@PIII */
    for (j = 0; j < RO.ixend; j++) {
		if( compareval0 > *xr ){
			*ix++ = 0;
			xr++;
		}else{
			*ix++ = (int)(istep * *xr + 0.4054);
			xr++;
		}
	}
}
#endif

/***********************************************************************
  re-calculate the best scalefac_compress using scfsi
  the saved bits are kept in the bit reservoir.
 **********************************************************************/


static void
recalc_divide_init( gr_info		 cod_info,
		  int	 * const ix,
		  int			 r01_bits_cash[],
		  int			 r01_div [],
		  int			 r0_tbl  [],
		  int			 r1_tbl  [] )
{
	int r0, r1, bigv, r0t, r1t, bits;

	bigv = cod_info.big_values;

	for (r0 = 0; r0 <= 7 + 15; r0++) {
	r01_bits_cash[r0] = LARGE_BITS;
	}

	for (r0 = 0; r0 < 16; r0++) {
	int a1 = RO.scalefac_band.l[r0 + 1], r0bits;
	if (a1 >= bigv)
		break;
	r0bits = cod_info.part2_length;
	r0t = choose_table(ix, ix + a1, &r0bits);

	for (r1 = 0; r1 < 8; r1++) {
		int a2 = RO.scalefac_band.l[r0 + r1 + 2];
		if (a2 >= bigv)
		break;

		bits = r0bits;
		r1t = choose_table(ix + a1, ix + a2, &bits);
		if (r01_bits_cash[r0 + r1] > bits) {
		r01_bits_cash[r0 + r1] = bits;
		r01_div[r0 + r1] = r0;
		r0_tbl[r0 + r1] = r0t;
		r1_tbl[r0 + r1] = r1t;
		}
	}
	}
}

static void
recalc_divide_sub( const gr_info cod_info2, gr_info *const gi, const int *const ix,
	const int r01_bits_cash[], const int r01_div [],
	const int r0_tbl  [], const int r1_tbl  [])
{
	int bits, r2, a2, bigv, r2t;

	bigv = cod_info2.big_values;

	for (r2 = 2; r2 < SBMAX_l + 1; r2++) {
	a2 = RO.scalefac_band.l[r2];
	if (a2 >= bigv) 
		break;

	bits = r01_bits_cash[r2 - 2] + cod_info2.count1bits;
	if (gi->part2_3_length <= bits)
		break;

	r2t = choose_table(ix + a2, ix + bigv, &bits);
	if (gi->part2_3_length <= bits)
		continue;

	memcpy(gi, &cod_info2, sizeof(gr_info));
	gi->part2_3_length = bits;
	gi->region0_count = r01_div[r2 - 2];
	gi->region1_count = r2 - 2 - r01_div[r2 - 2];
	gi->table_select[0] = r0_tbl[r2 - 2];
	gi->table_select[1] = r1_tbl[r2 - 2];
	gi->table_select[2] = r2t;
	}
}

//#define BEST_HUFFMAN_DEVIDE_WITH_CASHING

#ifndef BEST_HUFFMAN_DEVIDE_WITH_CASHING
void
best_huffman_divide( gr_info *const gi, int *const ix)
{
	int i, a1, a2;
	gr_info cod_info2;

	int r01_bits[7 + 15 + 1];
	int r01_div[7 + 15 + 1];
	int r0_tbl[7 + 15 + 1];
	int r1_tbl[7 + 15 + 1];


	/* SHORT BLOCK stuff fails for MPEG2 */ 
	if (gi->block_type == SHORT_TYPE && RO.mode_gr==1) 
		  return;


	memcpy(&cod_info2, gi, sizeof(gr_info));
	if (gi->block_type == NORM_TYPE) {
	recalc_divide_init(cod_info2, ix, r01_bits,r01_div,r0_tbl,r1_tbl);
	recalc_divide_sub(cod_info2, gi, ix, r01_bits,r01_div,r0_tbl,r1_tbl);
	}

	i = cod_info2.big_values;
	if (i == 0 || (unsigned int)(ix[i-2] | ix[i-1]) > 1)
	return;

	i = gi->count1 + 2;
	if (i > 576)
	return;

	/* Determines the number of bits to encode the quadruples. */
	memcpy(&cod_info2, gi, sizeof(gr_info));
	cod_info2.count1 = i;
	a1 = a2 = 0;

	assert(i <= 576);
	
	for (; i > cod_info2.big_values; i -= 4) {
	int p = ((ix[i-4] * 2 + ix[i-3]) * 2 + ix[i-2]) * 2 + ix[i-1];
	a1 += t32l[p];
	a2 += t33l[p];
	}
	cod_info2.big_values = i;

	cod_info2.count1table_select = 0;
	if (a1 > a2) {
	a1 = a2;
	cod_info2.count1table_select = 1;
	}

	cod_info2.count1bits = a1;
	cod_info2.part2_3_length = a1 + cod_info2.part2_length;

	if (cod_info2.block_type == NORM_TYPE)
	recalc_divide_sub(cod_info2, gi, ix, r01_bits,r01_div,r0_tbl,r1_tbl);
	else {
	/* Count the number of bits necessary to code the bigvalues region. */
	a1 = RO.scalefac_band.l[7 + 1];
	if (a1 > i) {
		a1 = i;
	}
	if (a1 > 0)
	  cod_info2.table_select[0] =
		choose_table(ix, ix + a1, (int *)&cod_info2.part2_3_length);
	if (i > a1)
	  cod_info2.table_select[1] =
		choose_table(ix + a1, ix + i, (int *)&cod_info2.part2_3_length);
	if (gi->part2_3_length > cod_info2.part2_3_length)
		memcpy(gi, &cod_info2, sizeof(gr_info));
	}
}
#endif

static const int slen1_n[16] = { 1, 1, 1, 1, 8, 2, 2, 2, 4, 4, 4, 8, 8, 8,16,16 };
static const int slen2_n[16] = { 1, 2, 4, 8, 1, 2, 4, 8, 2, 4, 8, 2, 4, 8, 4, 8 };

static void
scfsi_calc(int ch, III_side_info_t *l3_side, III_scalefac_t scalefac[2][2])
{
	int i, s1, s2, c1, c2;
	int sfb;
	gr_info *gi = &l3_side->tt[1][ch];

	static const int scfsi_band[5] = { 0, 6, 11, 16, 21 };
#if 0
	static const int slen1_n[16] = { 0, 1, 1, 1, 8, 2, 2, 2, 4, 4, 4, 8, 8, 8,16,16 };
	static const int slen2_n[16] = { 0, 2, 4, 8, 1, 2, 4, 8, 2, 4, 8, 2, 4, 8, 4, 8 };
#endif

	for (i = 0; i < 4; i++) 
	l3_side->scfsi[ch][i] = 0;

	for (i = 0; i < (sizeof(scfsi_band) / sizeof(int)) - 1; i++) {
	for (sfb = scfsi_band[i]; sfb < scfsi_band[i + 1]; sfb++) {
		if (scalefac[0][ch].l[sfb] != scalefac[1][ch].l[sfb])
		break;
	}
	if (sfb == scfsi_band[i + 1]) {
		for (sfb = scfsi_band[i]; sfb < scfsi_band[i + 1]; sfb++) {
		scalefac[1][ch].l[sfb] = -1;
		}
		l3_side->scfsi[ch][i] = 1;
	}
	}

	s1 = c1 = 0;
	for (sfb = 0; sfb < 11; sfb++) {
	if (scalefac[1][ch].l[sfb] < 0)
		continue;
	c1++;
	if (s1 < scalefac[1][ch].l[sfb])
		s1 = scalefac[1][ch].l[sfb];
	}

	s2 = c2 = 0;
	for (; sfb < SBPSY_l; sfb++) {
	if (scalefac[1][ch].l[sfb] < 0)
		continue;
	c2++;
	if (s2 < scalefac[1][ch].l[sfb])
		s2 = scalefac[1][ch].l[sfb];
	}

	for (i = 0; i < 16; i++) {
	if (s1 < slen1_n[i] && s2 < slen2_n[i]) {
		int c = slen1_tab[i] * c1 + slen2_tab[i] * c2;
		if (gi->part2_length > c) {
		gi->part2_length = c;
		gi->scalefac_compress = i;
		}
	}
	}
}

/*
Find the optimal way to store the scalefactors.
Only call this routine after final scalefactors have been
chosen and the channel/granule will not be re-encoded.
 */
void
best_scalefac_store( const int gr, const int ch, int l3_enc[2][2][576],
	III_side_info_t *const l3_side, III_scalefac_t scalefac[2][2])
{
	
	/* use scalefac_scale if we can */
	gr_info *gi = &l3_side->tt[gr][ch];
	int sfb,i,j,j2,l,start,end;

	/* remove scalefacs from bands with ix=0.  This idea comes
	* from the AAC ISO docs.  added mt 3/00 */
	/* check if l3_enc=0 */

	for ( sfb = 0; sfb < gi->sfb_lmax; sfb++ ) {
		if (scalefac[gr][ch].l[sfb] == 0) continue;
		start = RO.scalefac_band.l[ sfb ];
		if (start >= RO.ixend) {
			scalefac[gr][ch].l[sfb] = 0;
			continue;
		}
		end   = RO.scalefac_band.l[ sfb+1 ];
		if (end > RO.ixend) end = RO.ixend;
		for (l = start; l < end; l++) {
			if (l3_enc[gr][ch][l]) break;
		}
		if (l == end) scalefac[gr][ch].l[sfb] = 0;
	}
	for ( j=0, sfb = gi->sfb_smin; sfb < SBPSY_s; sfb++ ) {
		start = RO.scalefac_band.s[ sfb ];
		end   = RO.scalefac_band.s[ sfb+1 ];
		if (start*3 >= RO.ixend) {
			scalefac[gr][ch].s[sfb][0] = 0;
			scalefac[gr][ch].s[sfb][1] = 0;
			scalefac[gr][ch].s[sfb][2] = 0;
			continue;
		}
		for ( i = 0; i < 3; i++ ) {
			if (scalefac[gr][ch].s[sfb][i] > 0) {
				j2 = j;
				for ( l = start; l < end && j2 < RO.ixend; l++ ) {
					if (l3_enc[gr][ch][j2++] != 0) break;
				}
				if (l==end) scalefac[gr][ch].s[sfb][i] = 0;
			}
			j += end-start;
		}
	}

	gi->part2_3_length -= gi->part2_length;
	if (!gi->scalefac_scale && !gi->preflag) {
		int b, s = 0;
		for (sfb = 0; sfb < gi->sfb_lmax; sfb++) {
			s |= scalefac[gr][ch].l[sfb];
		}
		
		for (sfb = gi->sfb_smin; sfb < SBPSY_s; sfb++) {
			for (b = 0; b < 3; b++) {
				s |= scalefac[gr][ch].s[sfb][b];
			}
		}
		
		if (!(s & 1) && s != 0) {
			for (sfb = 0; sfb < gi->sfb_lmax; sfb++) {
				scalefac[gr][ch].l[sfb] /= 2;
			}
			for (sfb = gi->sfb_smin; sfb < SBPSY_s; sfb++) {
				for (b = 0; b < 3; b++) {
					scalefac[gr][ch].s[sfb][b] /= 2;
				}
			}
			
			gi->scalefac_scale = 1;
			gi->part2_length = 99999999;
			if (RO.mode_gr == 2) {
				scale_bitcount(&scalefac[gr][ch], gi);
			} else {
				scale_bitcount_lsf(&scalefac[gr][ch], gi);
			}
		}
	}
	
	
	for ( i = 0; i < 4; i++ )
		l3_side->scfsi[ch][i] = 0;
	
	if (RO.mode_gr==2 && gr == 1
		&& l3_side->tt[0][ch].block_type != SHORT_TYPE
		&& l3_side->tt[1][ch].block_type != SHORT_TYPE) {
		scfsi_calc(ch, l3_side, scalefac);
	}
	gi->part2_3_length += gi->part2_length;
}


/* number of bits used to encode scalefacs */

/* 18*slen1_tab[i] + 18*slen2_tab[i] */
static const int scale_short[16] = {
	0, 18, 36, 54, 54, 36, 54, 72, 54, 72, 90, 72, 90, 108, 108, 126 };

/* 17*slen1_tab[i] + 18*slen2_tab[i] */
static const int scale_mixed[16] = {
	0, 18, 36, 54, 51, 35, 53, 71, 52, 70, 88, 69, 87, 105, 104, 122 };

/* 11*slen1_tab[i] + 10*slen2_tab[i] */
static const int scale_long[16] = {
	0, 10, 20, 30, 33, 21, 31, 41, 32, 42, 52, 43, 53, 63, 64, 74 };


/*************************************************************************/
/*			scale_bitcount											 */
/*************************************************************************/

/* Also calculates the number of bits necessary to code the scalefactors. */

int scale_bitcount( 
	III_scalefac_t * const scalefac, gr_info * const cod_info)
{
	int i, k, sfb, max_slen1 = 0, max_slen2 = 0, ep = 2;

	/* maximum values */
	const int *tab;


	if ( cod_info->block_type == SHORT_TYPE ) {
	tab = scale_short;

	for ( i = 0; i < 3; i++ ) {
		for ( sfb = cod_info->sfb_smin; sfb < 6; sfb++ )
		if (max_slen1 < scalefac->s[sfb][i])
			max_slen1 = scalefac->s[sfb][i];
		for (sfb = 6; sfb < SBPSY_s; sfb++ )
		if (max_slen2 < scalefac->s[sfb][i])
			max_slen2 = scalefac->s[sfb][i];
	}
	}
	else
	{ /* block_type == 1,2,or 3 */
		tab = scale_long;
		for ( sfb = 0; sfb < 11; sfb++ )
			if ( scalefac->l[sfb] > max_slen1 )
				max_slen1 = scalefac->l[sfb];

	if (!cod_info->preflag) {
		for ( sfb = 11; sfb < SBPSY_l; sfb++ )
		if (scalefac->l[sfb] < pretab[sfb])
			break;

		if (sfb == SBPSY_l) {
		cod_info->preflag = 1;
		for ( sfb = 11; sfb < SBPSY_l; sfb++ )
			scalefac->l[sfb] -= pretab[sfb];
		}
	}

		for ( sfb = 11; sfb < SBPSY_l; sfb++ )
			if ( scalefac->l[sfb] > max_slen2 )
				max_slen2 = scalefac->l[sfb];
	}


	/* from Takehiro TOMINAGA <tominaga@isoternet.org> 10/99
	 * loop over *all* posible values of scalefac_compress to find the
	 * one which uses the smallest number of bits.  ISO would stop
	 * at first valid index */
	cod_info->part2_length = LARGE_BITS;
	for ( k = 0; k < 16; k++ )
	{
		if ( (max_slen1 < slen1_n[k]) && (max_slen2 < slen2_n[k]) &&
			 (cod_info->part2_length > tab[k])) {
	  cod_info->part2_length=tab[k];
	  cod_info->scalefac_compress=k;
	  ep=0;  /* we found a suitable scalefac_compress */
	}
	}
	return ep;
}



/*
  table of largest scalefactor values for MPEG2
*/
static const int max_range_sfac_tab[6][4] =
{
 { 15, 15, 7,  7},
 { 15, 15, 7,  0},
 { 7,  3,  0,  0},
 { 15, 31, 31, 0},
 { 7,  7,  7,  0},
 { 3,  3,  0,  0}
};




/*************************************************************************/
/*			scale_bitcount_lsf										 */
/*************************************************************************/

/* Also counts the number of bits to encode the scalefacs but for MPEG 2 */ 
/* Lower sampling frequencies  (24, 22.05 and 16 kHz.)				   */
 
/*  This is reverse-engineered from section 2.4.3.2 of the MPEG2 IS,	 */
/* "Audio Decoding Layer III"											*/

int scale_bitcount_lsf( III_scalefac_t *const scalefac, gr_info * const cod_info)
{
	int table_number, row_in_table, partition, nr_sfb, window, over;
	int i, sfb, max_sfac[ 4 ];
	const int *partition_table;

	/*
	  Set partition table. Note that should try to use table one,
	  but do not yet...
	*/
	if ( cod_info->preflag )
	table_number = 2;
	else
	table_number = 0;

	for ( i = 0; i < 4; i++ )
	max_sfac[i] = 0;

	if ( cod_info->block_type == SHORT_TYPE )
	{
		row_in_table = 1;
		partition_table = &nr_of_sfb_block[table_number][row_in_table][0];
		for ( sfb = 0, partition = 0; partition < 4; partition++ )
		{
		nr_sfb = partition_table[ partition ] / 3;
		for ( i = 0; i < nr_sfb; i++, sfb++ )
			for ( window = 0; window < 3; window++ )
			if ( scalefac->s[sfb][window] > max_sfac[partition] )
				max_sfac[partition] = scalefac->s[sfb][window];
		}
	}
	else
	{
	row_in_table = 0;
	partition_table = &nr_of_sfb_block[table_number][row_in_table][0];
	for ( sfb = 0, partition = 0; partition < 4; partition++ )
	{
		nr_sfb = partition_table[ partition ];
		for ( i = 0; i < nr_sfb; i++, sfb++ )
		if ( scalefac->l[sfb] > max_sfac[partition] )
			max_sfac[partition] = scalefac->l[sfb];
	}
	}

	for ( over = 0, partition = 0; partition < 4; partition++ )
	{
	if ( max_sfac[partition] > max_range_sfac_tab[table_number][partition] )
		over++;
	}
	if ( !over )
	{
	/*
	  Since no bands have been over-amplified, we can set scalefac_compress
	  and slen[] for the formatter
	*/
	static const int log2tab[] = { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };

	int slen1, slen2, slen3, slen4;

		cod_info->sfb_partition_table = nr_of_sfb_block[table_number][row_in_table];
	for ( partition = 0; partition < 4; partition++ )
		cod_info->slen[partition] = log2tab[max_sfac[partition]];

	/* set scalefac_compress */
	slen1 = cod_info->slen[ 0 ];
	slen2 = cod_info->slen[ 1 ];
	slen3 = cod_info->slen[ 2 ];
	slen4 = cod_info->slen[ 3 ];

	switch ( table_number )
	{
	  case 0:
		cod_info->scalefac_compress = (((slen1 * 5) + slen2) << 4)
		+ (slen3 << 2)
		+ slen4;
		break;

	  case 1:
		cod_info->scalefac_compress = 400
		+ (((slen1 * 5) + slen2) << 2)
		+ slen3;
		break;

	  case 2:
		cod_info->scalefac_compress = 500 + (slen1 * 3) + slen2;
		break;

	  default:
		RO.printf("intensity stereo not implemented yet\n" );
		break;
	}
	}
#ifdef DEBUG
	if ( over ) 
		RO.printf("---WARNING !! Amplification of some bands over limits\n" );
#endif
	if (!over) {
	  assert( cod_info->sfb_partition_table );	 
	  cod_info->part2_length=0;
	  for ( partition = 0; partition < 4; partition++ )
	cod_info->part2_length += cod_info->slen[partition] * cod_info->sfb_partition_table[partition];
	}
	return over;
}

void
huffman_init(void)
{
	int i;

	for (i = 2; i <= 576; i += 2) {
	int scfb_anz = 0, index;
	while (RO.scalefac_band.l[++scfb_anz] < i)
		;

	index = subdv_table[scfb_anz].region0_count;
	while (RO.scalefac_band.l[index + 1] > i)
		index--;

	if (index < 0) {
	  /* this is an indication that everything is going to
		 be encoded as region0:  bigvalues < region0 < region1
		 so lets set region0, region1 to some value larger
		 than bigvalues */
	  index = subdv_table[scfb_anz].region0_count;
	}

	RW.bv_scf[i-2] = index;

	index = subdv_table[scfb_anz].region1_count;
	while (RO.scalefac_band.l[index + RW.bv_scf[i-2] + 2] > i)
		index--;

	if (index < 0) {
	  index = subdv_table[scfb_anz].region1_count;
	}

	RW.bv_scf[i-1] = index;
	}
}


#ifdef BEST_HUFFMAN_DEVIDE_WITH_CASHING

void 
ix_max_cashing_init(const int *ix, int ix_max_cash[18])
{
	int max1, max2;
	int i, j;
	for ( i = 0; i < 18; i++ ) {
		max1 = 0;
		max2 = 0;
		for ( j = 0; j < 16; j++ ) {
			int x1 = *ix++;
			int x2 = *ix++;
			if (max1 < x1) {
				max1 = x1;
			}
			if (max2 < x2) {
				max2 = x2;
			}
		}
		if (max1 < max2) {
			max1 = max2;
		}
		ix_max_cash[ i ] = max1;
	}
}

int 
ix_max_cashing(const int *ix, int start, int end, int* max_cash)
{
	int max1 = 0, max2 = 0;
	int i;
	for ( i = start; i < end && i % 32; i += 2 ) {
		int x1 = ix[i+0];
		int x2 = ix[i+1];
		if (max1 < x1) 
			max1 = x1;

		if (max2 < x2) 
			max2 = x2;
	}
	if (max1 < max2) 
		max1 = max2;

	for ( ; i+32 <= end; i += 32 ) {
		if (max1 < max_cash[i/32]) 
			max1 = max_cash[i/32];
	}

	for ( ; i < end; i += 2 ) {
		int x1 = ix[i+0];
		int x2 = ix[i+1];
		if (max1 < x1) 
			max1 = x1;

		if (max2 < x2) 
			max2 = x2;
	}
	if (max1 < max2) 
		max1 = max2;

	return max1;
}


int
count_bit_ESC_cashing( 
	const int *	   ix, 
		  int      start,
		  int	   end,
		  int		 t1,
	const int		 t2,
		  int * const s )
{
	/* ESC-table is used */
	int linbits = ht[t1].xlen * 65536 + ht[t2].xlen;
	int sum = 0, sum2;

	do {
		int x = ix[start++];
		int y = ix[start++];

		if (x != 0) {
			if (x > 14) {
			x = 15;
			sum += linbits;
			}
			x *= 16;
		}

		if (y != 0) {
			if (y > 14) {
			y = 15;
			sum += linbits;
			}
			x += y;
		}

		sum += largetbl[x];
	} while (start < end);

	sum2 = sum & 0xffff;
	sum >>= 16;

	if (sum > sum2) {
	sum = sum2;
	t1 = t2;
	}

	*s += sum;
	return t1;
}


static int
count_bit_noESC_cashing(const int * ix, 
						int      start,
						int	   end,
		  int * const s,
		  int*     bits
)
{
	/* No ESC-words */
	int	sum1 = 0;
	const char *hlen1 = ht[1].hlen;

//	return count_bit_noESC(ix+start, ix+end, s);

	for ( ; start < end && start % 32; start += 2 ) {
		int x = ix[start+0] * 2 + ix[start+1];
		sum1 += hlen1[x];
	}
	for ( ; start+32 < end; start += 32 ) {
		if ( bits[start/32] < 0 ) {
			bits[start/32] = (int)hlen1[ix[start+ 0] * 2 + ix[start+ 1]]
				           + (int)hlen1[ix[start+ 2] * 2 + ix[start+ 3]]
				           + (int)hlen1[ix[start+ 4] * 2 + ix[start+ 5]]
				           + (int)hlen1[ix[start+ 6] * 2 + ix[start+ 7]]
				           + (int)hlen1[ix[start+ 8] * 2 + ix[start+ 9]]
				           + (int)hlen1[ix[start+10] * 2 + ix[start+11]]
				           + (int)hlen1[ix[start+12] * 2 + ix[start+13]]
				           + (int)hlen1[ix[start+14] * 2 + ix[start+15]]
				           + (int)hlen1[ix[start+16] * 2 + ix[start+17]]
				           + (int)hlen1[ix[start+18] * 2 + ix[start+19]]
				           + (int)hlen1[ix[start+20] * 2 + ix[start+21]]
				           + (int)hlen1[ix[start+22] * 2 + ix[start+23]]
				           + (int)hlen1[ix[start+24] * 2 + ix[start+25]]
				           + (int)hlen1[ix[start+26] * 2 + ix[start+27]]
				           + (int)hlen1[ix[start+28] * 2 + ix[start+29]]
				           + (int)hlen1[ix[start+30] * 2 + ix[start+31]];
		}
		sum1 += bits[start/32];
	}
	for ( ; start < end; start += 2 ) {
		int x = ix[start+0] * 2 + ix[start+1];
		sum1 += hlen1[x];
	}

	*s += sum1;
	return 1;
}



static int
count_bit_noESC_from2_cashing(
	const int *	   ix, 
		  int      start,
		  int	   end,
		  int		 t1,
		  int * const s,
		  int*     bits
)
{
	/* No ESC-words */
	unsigned int sum = 0, sum2;
	const int xlen = ht[t1].xlen;
	const unsigned int *hlen;

//	return count_bit_noESC_from2(ix+start, ix+end, t1, s);

	if (t1 == 2)
	hlen = table23;
	else
	hlen = table56;

	for ( ; start < end && start % 32; start += 2 ) {
		int x = ix[start+0] * xlen + ix[start+1];
		sum += hlen[x];
	}
	for ( ; start+32 < end; start += 32 ) {
		if ( bits[start/32] < 0 ) {
			bits[start/32] = hlen[ix[start+ 0] * xlen + ix[start+ 1]]
				           + hlen[ix[start+ 2] * xlen + ix[start+ 3]]
				           + hlen[ix[start+ 4] * xlen + ix[start+ 5]]
				           + hlen[ix[start+ 6] * xlen + ix[start+ 7]]
				           + hlen[ix[start+ 8] * xlen + ix[start+ 9]]
				           + hlen[ix[start+10] * xlen + ix[start+11]]
				           + hlen[ix[start+12] * xlen + ix[start+13]]
				           + hlen[ix[start+14] * xlen + ix[start+15]]
				           + hlen[ix[start+16] * xlen + ix[start+17]]
				           + hlen[ix[start+18] * xlen + ix[start+19]]
				           + hlen[ix[start+20] * xlen + ix[start+21]]
				           + hlen[ix[start+22] * xlen + ix[start+23]]
				           + hlen[ix[start+24] * xlen + ix[start+25]]
				           + hlen[ix[start+26] * xlen + ix[start+27]]
				           + hlen[ix[start+28] * xlen + ix[start+29]]
				           + hlen[ix[start+30] * xlen + ix[start+31]];
		}
		sum += bits[start/32];
	}
	for ( ; start < end; start += 2 ) {
		int x = ix[start+0] * xlen + ix[start+1];
		sum += hlen[x];
	}

	sum2 = sum & 0xffff;
	sum >>= 16;

	if (sum > sum2) {
		sum = sum2;
		t1++;
	}

	*s += sum;
	return t1;
}


static int
count_bit_noESC_from3_cashing(
	const int *	   ix, 
		  int      start,
		  int	   end,
		  int		 t1,
		  int * const s,
		  int     bits[][18]
)
{
	/* No ESC-words */
	int	sum1 = 0;
	int	sum2 = 0;
	int	sum3 = 0;
	const int xlen = ht[t1].xlen;
	const char *hlen1 = ht[t1].hlen;
	const char *hlen2 = ht[t1+1].hlen;
	const char *hlen3 = ht[t1+2].hlen;
	int t;

//	return count_bit_noESC_from3(ix+start, ix+end, t1, s);

	for ( ; start < end && start % 32; start += 2 ) {
		int x = ix[start+0] * xlen + ix[start+1];
		sum1 += hlen1[x];
		sum2 += hlen2[x];
		sum3 += hlen3[x];
	}
	for ( ; start+32 < end; start += 32 ) {
		if ( bits[0][start/32] < 0 ) {
			bits[0][start/32] = (int)hlen1[ix[start+ 0] * xlen + ix[start+ 1]]
				              + (int)hlen1[ix[start+ 2] * xlen + ix[start+ 3]]
				              + (int)hlen1[ix[start+ 4] * xlen + ix[start+ 5]]
				              + (int)hlen1[ix[start+ 6] * xlen + ix[start+ 7]]
				              + (int)hlen1[ix[start+ 8] * xlen + ix[start+ 9]]
				              + (int)hlen1[ix[start+10] * xlen + ix[start+11]]
				              + (int)hlen1[ix[start+12] * xlen + ix[start+13]]
				              + (int)hlen1[ix[start+14] * xlen + ix[start+15]]
				              + (int)hlen1[ix[start+16] * xlen + ix[start+17]]
				              + (int)hlen1[ix[start+18] * xlen + ix[start+19]]
				              + (int)hlen1[ix[start+20] * xlen + ix[start+21]]
				              + (int)hlen1[ix[start+22] * xlen + ix[start+23]]
				              + (int)hlen1[ix[start+24] * xlen + ix[start+25]]
				              + (int)hlen1[ix[start+26] * xlen + ix[start+27]]
				              + (int)hlen1[ix[start+28] * xlen + ix[start+29]]
				              + (int)hlen1[ix[start+30] * xlen + ix[start+31]];
		}
		if ( bits[1][start/32] < 0 ) {
			bits[1][start/32] = (int)hlen2[ix[start+ 0] * xlen + ix[start+ 1]]
				              + (int)hlen2[ix[start+ 2] * xlen + ix[start+ 3]]
				              + (int)hlen2[ix[start+ 4] * xlen + ix[start+ 5]]
				              + (int)hlen2[ix[start+ 6] * xlen + ix[start+ 7]]
				              + (int)hlen2[ix[start+ 8] * xlen + ix[start+ 9]]
				              + (int)hlen2[ix[start+10] * xlen + ix[start+11]]
				              + (int)hlen2[ix[start+12] * xlen + ix[start+13]]
				              + (int)hlen2[ix[start+14] * xlen + ix[start+15]]
				              + (int)hlen2[ix[start+16] * xlen + ix[start+17]]
				              + (int)hlen2[ix[start+18] * xlen + ix[start+19]]
				              + (int)hlen2[ix[start+20] * xlen + ix[start+21]]
				              + (int)hlen2[ix[start+22] * xlen + ix[start+23]]
				              + (int)hlen2[ix[start+24] * xlen + ix[start+25]]
				              + (int)hlen2[ix[start+26] * xlen + ix[start+27]]
				              + (int)hlen2[ix[start+28] * xlen + ix[start+29]]
				              + (int)hlen2[ix[start+30] * xlen + ix[start+31]];
		}
		if ( bits[2][start/32] < 0 ) {
			bits[2][start/32] = (int)hlen3[ix[start+ 0] * xlen + ix[start+ 1]]
				              + (int)hlen3[ix[start+ 2] * xlen + ix[start+ 3]]
				              + (int)hlen3[ix[start+ 4] * xlen + ix[start+ 5]]
				              + (int)hlen3[ix[start+ 6] * xlen + ix[start+ 7]]
				              + (int)hlen3[ix[start+ 8] * xlen + ix[start+ 9]]
				              + (int)hlen3[ix[start+10] * xlen + ix[start+11]]
				              + (int)hlen3[ix[start+12] * xlen + ix[start+13]]
				              + (int)hlen3[ix[start+14] * xlen + ix[start+15]]
				              + (int)hlen3[ix[start+16] * xlen + ix[start+17]]
				              + (int)hlen3[ix[start+18] * xlen + ix[start+19]]
				              + (int)hlen3[ix[start+20] * xlen + ix[start+21]]
				              + (int)hlen3[ix[start+22] * xlen + ix[start+23]]
				              + (int)hlen3[ix[start+24] * xlen + ix[start+25]]
				              + (int)hlen3[ix[start+26] * xlen + ix[start+27]]
				              + (int)hlen3[ix[start+28] * xlen + ix[start+29]]
				              + (int)hlen3[ix[start+30] * xlen + ix[start+31]];
		}
		sum1 += bits[0][start/32];
		sum2 += bits[1][start/32];
		sum3 += bits[2][start/32];
	}
	for ( ; start < end; start += 2 ) {
		int x = ix[start+0] * xlen + ix[start+1];
		sum1 += hlen1[x];
		sum2 += hlen2[x];
		sum3 += hlen3[x];
	}

	t = t1;
	if (sum1 > sum2) {
		sum1 = sum2;
		t++;
	}
	if (sum1 > sum3) {
		sum1 = sum3;
		t = t1+2;
	}
	*s += sum1;

	return t;
}


/*************************************************************************/
/*		  choose table						 */
/*************************************************************************/

/*
  Choose the Huffman table that will encode ix[begin..end] with
  the fewest bits.

  Note: This code contains knowledge about the sizes and characteristics
  of the Huffman tables as defined in the IS (Table B.7), and will not work
  with any arbitrary tables.
*/

static int choose_table_C_cashing( const int *ix, int start, int end, int *s,
	int			 max_cash[],
	int			 bits_cash[][18]
)
{
	int max;
	int choice, choice2;
	static const int huf_tbl_noESC[] = {
		1, 2, 5, 7, 7,10,10,13,13,13,13,13,13,13,13 /* char not enough ? */
	};
	static const int bits_cash_tbl[] = {
		0, 1, 2, 3, 3, 6, 6, 9, 9, 9, 9, 9, 9, 9, 9
	};

	max = ix_max_cashing(ix, start, end, max_cash);
//	max = ix_max(ix+start, ix+end);

	switch (max) {
	case 0:
		return max;

	case 1:
		return count_bit_noESC_cashing(ix, start, end, s, bits_cash[bits_cash_tbl[max - 1]]);

	case 2:
	case 3:
		return count_bit_noESC_from2_cashing(ix, start, end, huf_tbl_noESC[max - 1], s, bits_cash[bits_cash_tbl[max - 1]]);

	case 4: case 5: case 6:
	case 7: case 8: case 9:
	case 10: case 11: case 12:
	case 13: case 14: case 15:
		return count_bit_noESC_from3_cashing(ix, start, end, huf_tbl_noESC[max - 1], s, &bits_cash[bits_cash_tbl[max - 1]]);

	default:
		/* try tables with linbits */
		if (max > IXMAX_VAL) {
			*s = LARGE_BITS;
			return -1;
		}
		max -= 15;
		for (choice2 = 24; choice2 < 32; choice2++) {
			if (ht[choice2].linmax >= max) {
				break;
			}
		}

		for (choice = choice2 - 8; choice < 24; choice++) {
			if (ht[choice].linmax >= max) {
				break;
			}
		}
		return count_bit_ESC_cashing(ix, start, end, choice, choice2, s);
	}
}

static void
recalc_divide_init_cashing( gr_info		 cod_info,
		  int	 * const ix,
		  int			 r01_bits[],
		  int			 r01_div [],
		  int			 r0_tbl  [],
		  int			 r1_tbl  [],
		  int			 max_cash[],
		  int			 bits_cash[][18]
)
{
	int r0, r1, bigv, r0t, r1t, bits;

	bigv = cod_info.big_values;

	for (r0 = 0; r0 <= 7 + 15; r0++) {
		r01_bits[r0] = LARGE_BITS;
	}

	for (r0 = 0; r0 < 16; r0++) {
		int a1 = RO.scalefac_band.l[r0 + 1], r0bits;
		if (a1 >= bigv)
			break;
		r0bits = cod_info.part2_length;
		r0t = choose_table_C_cashing(ix, 0, a1, &r0bits, max_cash, bits_cash);

		for (r1 = 0; r1 < 8; r1++) {
			int a2 = RO.scalefac_band.l[r0 + r1 + 2];
			if (a2 >= bigv)
				break;

			bits = r0bits;
			r1t = choose_table_C_cashing(ix, a1, a2, &bits, max_cash, bits_cash);
			if (r01_bits[r0 + r1] > bits) {
				r01_bits[r0 + r1] = bits;
				r01_div[r0 + r1] = r0;
				r0_tbl[r0 + r1] = r0t;
				r1_tbl[r0 + r1] = r1t;
			}
		}
	}
}

static void
recalc_divide_sub_cashing( const gr_info cod_info2, gr_info *const gi, const int *const ix,
	const int r01_bits_cash[], const int r01_div [],
	const int r0_tbl  [], const int r1_tbl  [],
		  int			 max_cash[],
		  int			 bits_cash[][18]
)
{
	int bits, r2, a2, bigv, r2t;

	bigv = cod_info2.big_values;

	for (r2 = 2; r2 < SBMAX_l + 1; r2++) {
	a2 = RO.scalefac_band.l[r2];
	if (a2 >= bigv) 
		break;

	bits = r01_bits_cash[r2 - 2] + cod_info2.count1bits;
	if (gi->part2_3_length <= bits)
		break;

	r2t = choose_table_C_cashing(ix, a2, bigv, &bits, max_cash, bits_cash);
	if (gi->part2_3_length <= bits)
		continue;

	memcpy(gi, &cod_info2, sizeof(gr_info));
	gi->part2_3_length = bits;
	gi->region0_count = r01_div[r2 - 2];
	gi->region1_count = r2 - 2 - r01_div[r2 - 2];
	gi->table_select[0] = r0_tbl[r2 - 2];
	gi->table_select[1] = r1_tbl[r2 - 2];
	gi->table_select[2] = r2t;
	}
}

void
best_huffman_divide( gr_info *const gi, int *const ix)
{
	int i, a1, a2;
	gr_info cod_info2;

	int r01_bits[7 + 15 + 1];
	int r01_div[7 + 15 + 1];
	int r0_tbl[7 + 15 + 1];
	int r1_tbl[7 + 15 + 1];
	int	max_cash[18];
	int bits_cash[12][18];

	/* SHORT BLOCK stuff fails for MPEG2 */ 
	if (gi->block_type == SHORT_TYPE && RO.mode_gr==1) 
		  return;

	ix_max_cashing_init(ix, max_cash);
	memset(bits_cash, -1, sizeof(bits_cash));
	memcpy(&cod_info2, gi, sizeof(gr_info));
	if (gi->block_type == NORM_TYPE) {
		recalc_divide_init_cashing(cod_info2, ix, r01_bits,r01_div,r0_tbl,r1_tbl, max_cash, bits_cash);
		recalc_divide_sub_cashing(cod_info2, gi, ix, r01_bits,r01_div,r0_tbl,r1_tbl, max_cash, bits_cash);
	}

	i = cod_info2.big_values;
	if (i == 0 || (unsigned int)(ix[i-2] | ix[i-1]) > 1)
		return;

	i = gi->count1 + 2;
	if (i > 576)
		return;

	/* Determines the number of bits to encode the quadruples. */
	memcpy(&cod_info2, gi, sizeof(gr_info));
	cod_info2.count1 = i;
	a1 = a2 = 0;

	assert(i <= 576);
	
	for (; i > cod_info2.big_values; i -= 4) {
		int p = ((ix[i-4] * 2 + ix[i-3]) * 2 + ix[i-2]) * 2 + ix[i-1];
		a1 += t32l[p];
		a2 += t33l[p];
	}
	cod_info2.big_values = i;

	cod_info2.count1table_select = 0;
	if (a1 > a2) {
		a1 = a2;
		cod_info2.count1table_select = 1;
	}

	cod_info2.count1bits = a1;
	cod_info2.part2_3_length = a1 + cod_info2.part2_length;

	if (cod_info2.block_type == NORM_TYPE) {
		recalc_divide_sub_cashing(cod_info2, gi, ix, r01_bits,r01_div,r0_tbl,r1_tbl, max_cash, bits_cash);
	} 
	else {
		/* Count the number of bits necessary to code the bigvalues region. */
		a1 = RO.scalefac_band.l[7 + 1];
		if (a1 > i) {
			a1 = i;
		}
		if (a1 > 0)
			cod_info2.table_select[0] =
				choose_table_C_cashing(ix, 0, a1, (int *)&cod_info2.part2_3_length, max_cash, bits_cash);
		if (i > a1)
			cod_info2.table_select[1] =
				choose_table_C_cashing(ix, a1, i, (int *)&cod_info2.part2_3_length, max_cash, bits_cash);
		if (gi->part2_3_length > cod_info2.part2_3_length)
			memcpy(gi, &cod_info2, sizeof(gr_info));
	}
}

#endif
