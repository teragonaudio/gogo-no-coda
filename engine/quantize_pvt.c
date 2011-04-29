/*
 *	quantize_pvt source file
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
#include <float.h>	/* for FLT_MAX */

#include "config.h"
#include "global.h"

#include "util.h"
#include "tables.h"
#include "reservoir.h"
#include "quantize_pvt.h"
#include "vfta.h"

//#define NSATHSCALE 100 // Assuming dynamic range=96dB, this value should be 92

const char  slen1_tab [16] = { 0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4 };
const char  slen2_tab [16] = { 0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3 };

/*
  The following table is used to implement the scalefactor
  partitioning for MPEG2 as described in section
  2.4.3.2 of the IS. The indexing corresponds to the
  way the tables are presented in the IS:

  [table_number][row_in_table][column of nr_of_sfb]
*/
const int  nr_of_sfb_block [6] [3] [4] =
{
  {
    {6, 5, 5, 5},
    {9, 9, 9, 9},
    {6, 9, 9, 9}
  },
  {
    {6, 5, 7, 3},
    {9, 9, 12, 6},
    {6, 9, 12, 6}
  },
  {
    {11, 10, 0, 0},
    {18, 18, 0, 0},
    {15,18,0,0}
  },
  {
    {7, 7, 7, 0},
    {12, 12, 12, 0},
    {6, 15, 12, 0}
  },
  {
    {6, 6, 6, 3},
    {12, 9, 9, 6},
    {6, 12, 9, 6}
  },
  {
    {8, 8, 5, 0},
    {15,12,9,0},
    {6,18,9,0}
  }
};


/* Table B.6: layer3 preemphasis */
const char  pretab [SBMAX_l+2] =	// original was pretab [SBMAX_l]
									// added 2 for SIMD code
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 2, 2, 3, 3, 3, 2, 0, 0, 0
};

/*
  Here are MPEG1 Table B.8 and MPEG2 Table B.1
  -- Layer III scalefactor bands. 
  Index into this using a method such as:
    idx  = fr_ps->header->sampling_frequency
           + (fr_ps->header->version * 3)
*/


const scalefac_struct sfBandIndex[6] =
{
  { /* Table B.2.b: 22.05 kHz */
    {0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576},
    {0,4,8,12,18,24,32,42,56,74,100,132,174,192}
  },
  { /* Table B.2.c: 24 kHz */                 /* docs: 332. mpg123(broken): 330 */
    {0,6,12,18,24,30,36,44,54,66,80,96,114,136,162,194,232,278, 332, 394,464,540,576},
    {0,4,8,12,18,26,36,48,62,80,104,136,180,192}
  },
  { /* Table B.2.a: 16 kHz */
    {0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576},
    {0,4,8,12,18,26,36,48,62,80,104,134,174,192}
  },
  { /* Table B.8.b: 44.1 kHz */
    {0,4,8,12,16,20,24,30,36,44,52,62,74,90,110,134,162,196,238,288,342,418,576},
    {0,4,8,12,16,22,30,40,52,66,84,106,136,192}
  },
  { /* Table B.8.c: 48 kHz */
    {0,4,8,12,16,20,24,30,36,42,50,60,72,88,106,128,156,190,230,276,330,384,576},
    {0,4,8,12,16,22,28,38,50,64,80,100,126,192}
  },
  { /* Table B.8.a: 32 kHz */
    {0,4,8,12,16,20,24,30,36,44,54,66,82,102,126,156,194,240,296,364,448,550,576},
    {0,4,8,12,16,22,30,42,58,78,104,138,180,192}
  }
};



static FLOAT8
ATHmdct(FLOAT8 f)
{
	FLOAT8 ath;
  
	ath = ATHformula(f);
        ath -= 114;
    
	/* convert to energy */
	ath = pow( 10.0, ath/10.0 );
	return ath;
}
 
static void
compute_ath(FLOAT8 ATH_l[], FLOAT8 ATH_s[] )
{
    int sfb, i, start, end;
    FLOAT8 ATH_f;
    FLOAT8 samp_freq = RO.out_samplerate;

    for (sfb = 0; sfb < SBMAX_l; sfb++) {
        start = RO.scalefac_band.l[ sfb ];
        end   = RO.scalefac_band.l[ sfb+1 ];
        ATH_l[sfb]= FLT_MAX;
        for (i = start ; i < end; i++) {
            FLOAT8 freq = i*samp_freq/(2*576);
            ATH_f = ATHmdct(freq);  /* freq in kHz */
            ATH_l[sfb] = Min( ATH_l[sfb], ATH_f );
        }

    }

    for (sfb = 0; sfb < SBMAX_s; sfb++){
        start = RO.scalefac_band.s[ sfb ];
        end   = RO.scalefac_band.s[ sfb+1 ];
        ATH_s[sfb] = FLT_MAX;
        for (i = start ; i < end; i++) {
            FLOAT8 freq = i*samp_freq/(2*192);
            ATH_f = ATHmdct(freq);    /* freq in kHz */
            ATH_s[sfb] = Min( ATH_s[sfb], ATH_f );
        }
    }
}

/************************************************************************/
/*  initialization for iteration_loop */
/************************************************************************/
void
iteration_init(void)
{
  int i;

    RW.main_data_begin = 0;
    compute_ath(RW.ATH.l,RW.ATH.s);

    RO.pow43[0] = 0.0;
    for(i=1;i<PRECALC_SIZE;i++)
        RO.pow43[i] = pow((FLOAT8)i, 4.0/3.0);

    for (i = 0; i < Q_MAX; i++) {
	RO.ipow20[i] = pow(2.0, (double)(i - 210) * -0.1875);
	RO.pow20[i] = pow(2.0, (double)(i - 210) * 0.25);
    }
    huffman_init();
}





/* 
compute the ATH for each scalefactor band 
cd range:  0..96db

Input:  3.3kHz signal  32767 amplitude  (3.3kHz is where ATH is smallest = -5db)
longblocks:  sfb=12   en0/bw=-11db    max_en0 = 1.3db
shortblocks: sfb=5           -9db              0db

Input:  1 1 1 1 1 1 1 -1 -1 -1 -1 -1 -1 -1 (repeated)
longblocks:  amp=1      sfb=12   en0/bw=-103 db      max_en0 = -92db
            amp=32767   sfb=12           -12 db                 -1.4db 

Input:  1 1 1 1 1 1 1 -1 -1 -1 -1 -1 -1 -1 (repeated)
shortblocks: amp=1      sfb=5   en0/bw= -99                    -86 
            amp=32767   sfb=5           -9  db                  4db 


MAX energy of largest wave at 3.3kHz = 1db
AVE energy of largest wave at 3.3kHz = -11db
Let's take AVE:  -11db = maximum signal in sfb=12.  
Dynamic range of CD: 96db.  Therefor energy of smallest audible wave 
in sfb=12  = -11  - 96 = -107db = ATH at 3.3kHz.  

ATH formula for this wave: -5db.  To adjust to LAME scaling, we need
ATH = ATH_formula  - 103  (db)
ATH = ATH * 2.5e-10      (ener)

*/

/************************************************************************
 * allocate bits among 2 channels based on PE
 * mt 6/99
 ************************************************************************/
int
on_pe(FLOAT8 pe[2][2], gr_info (*l3_side_tt)[2], int targ_bits[2],int mean_bits, int gr)
{
  gr_info *cod_info;
  int extra_bits,tbits,bits;
  int add_bits[2]; 
  int ch;
  int max_bits;  /* maximum allowed bits for this granule */

  /* allocate targ_bits for granule */
  ResvMaxBits(mean_bits, &tbits, &extra_bits);
  max_bits=tbits+extra_bits;
  if (max_bits > MAX_BITS) /* hard limit per granule */
    max_bits = MAX_BITS;

  bits=0;

  for (ch=0 ; ch < RO.channels_out ; ch ++) {
    /******************************************************************
     * allocate bits for each channel 
     ******************************************************************/
    cod_info = &l3_side_tt[gr][ch];
    
    targ_bits[ch]=Min(MAX_BITS, tbits/RO.channels_out);
    
      add_bits[ch]=(pe[gr][ch]-750)/1.4;
      /* short blocks us a little extra, no matter what the pe */
      if (cod_info->block_type==SHORT_TYPE) {
	if (add_bits[ch]<mean_bits/4) add_bits[ch]=mean_bits/4;
      }

      /* at most increase bits by 1.5*average */
      if (add_bits[ch] > .75*mean_bits) add_bits[ch]=mean_bits*.75;
      if (add_bits[ch] < 0) add_bits[ch]=0;

      if ((targ_bits[ch]+add_bits[ch]) > MAX_BITS) 
	add_bits[ch]=Max(0, MAX_BITS-targ_bits[ch]);

    bits += add_bits[ch];
  }
  if (bits > extra_bits)
    for (ch=0 ; ch < RO.channels_out ; ch ++) {
      add_bits[ch] = (extra_bits*add_bits[ch])/bits;
    }

  for (ch=0 ; ch < RO.channels_out ; ch ++) {
    targ_bits[ch] = targ_bits[ch] + add_bits[ch];
    extra_bits -= add_bits[ch];
  }
  return max_bits;
}




void reduce_side(int targ_bits[2],FLOAT8 ms_ener_ratio,int mean_bits,int max_bits)
{
  int move_bits;
  FLOAT fac;


  /*  ms_ener_ratio = 0:  allocate 66/33  mid/side  fac=.33  
   *  ms_ener_ratio =.5:  allocate 50/50 mid/side   fac= 0 */
  /* 75/25 split is fac=.5 */
  /* float fac = .50*(.5-ms_ener_ratio[gr])/.5;*/
  fac = .33*(.5-ms_ener_ratio)/.5;
  if (fac<0) fac=0;
  if (fac>.5) fac=.5;
  
    /* number of bits to move from side channel to mid channel */
    /*    move_bits = fac*targ_bits[1];  */
    move_bits = fac*.5*(targ_bits[0]+targ_bits[1]);  

    if (move_bits > MAX_BITS - targ_bits[0]) {
        move_bits = MAX_BITS - targ_bits[0];
    }
    if (move_bits<0) move_bits=0;
    
    if (targ_bits[1] >= 125) {
      /* dont reduce side channel below 125 bits */
      if (targ_bits[1]-move_bits > 125) {

	/* if mid channel already has 2x more than average, dont bother */
	/* mean_bits = bits per granule (for both channels) */
	if (targ_bits[0] < mean_bits)
	  targ_bits[0] += move_bits;
	targ_bits[1] -= move_bits;
      } else {
	targ_bits[0] += targ_bits[1] - 125;
	targ_bits[1] = 125;
      }
    }
    
    move_bits=targ_bits[0]+targ_bits[1];
    if (move_bits > max_bits) {
      targ_bits[0]=(max_bits*targ_bits[0])/move_bits;
      targ_bits[1]=(max_bits*targ_bits[1])/move_bits;
    }
}

/*************************************************************************/
/*            calc_xmin                                                  */
/*************************************************************************/

/*
  Calculate the allowed distortion for each scalefactor band,
  as determined by the psychoacoustic model.
  xmin(sb) = ratio(sb) * en(sb) / bw(sb)

  returns number of sfb's with energy > ATH
*/
/* こっちは殆どこないのでいじらない */
int
calc_xmin_short(const FLOAT8 xr[576], const III_psy_ratio * const ratio,
              III_psy_xmin  * const l3_xmin, float masking_lower) 
{
	int sfb,j,start, end, bw,l, b, ath_over=0;
	FLOAT8 en0, xmin, ener;
	
	for ( j=0, sfb = 0; sfb < SBMAX_s; sfb++ ) {
		start = RO.scalefac_band.s[ sfb ];
		end   = RO.scalefac_band.s[ sfb + 1 ];
		bw = end - start;
		for ( b = 0; b < 3; b++ ) {
			for (en0 = 0.0, l = start; l < end && j < RO.ixend; l++) {
				ener = xr[j++];
				ener = ener * ener;
				en0 += ener;
			}
			en0 /= bw;
			
			xmin = ratio->en.s[sfb][b];
			if (xmin > 0.0)
				xmin = en0 * ratio->thm.s[sfb][b] * masking_lower / xmin;
			xmin = Max(RW.ATH.adjust * RW.ATH.s[sfb], xmin);
			l3_xmin->s[sfb][b] = xmin * bw;
			
			if (en0 > RW.ATH.adjust * RW.ATH.s[sfb]) ath_over++;
		}
	}
	
    for (sfb = 0; sfb < SBMAX_s; sfb++ ) {
		for ( b = 1; b < 3; b++ ) {
			xmin = l3_xmin->s[sfb][b] * (1.0 - RO.decay)
				+  l3_xmin->s[sfb][b-1] * RO.decay;
			if (l3_xmin->s[sfb][b] < xmin)
				l3_xmin->s[sfb][b] = xmin;
		}
    }
	return ath_over;
}

int
calc_xmin_long(const FLOAT8 xr[576], const III_psy_ratio * const ratio,
              III_psy_xmin  * const l3_xmin, float masking_lower ) 
{
	int sfb, start, end, bw, i, ath_over = 0;
	float en0, xmin, ener;
	for ( sfb = 0; sfb < SBMAX_l; sfb++ ){
		start = RO.scalefac_band.l[ sfb ];
		end   = RO.scalefac_band.l[ sfb+1 ];
		bw = end - start;
		
		if (end > RO.ixend) {
			end = RO.ixend;
		}
		en0 = 0;
		for (i = start; i < end; i++ ) {
			ener = xr[i] * xr[i];
			en0 += ener;
		}
		en0 /= bw;
		
		xmin = ratio->en.l[sfb];
		if (xmin > 0.0){
			xmin = en0 * ratio->thm.l[sfb] * masking_lower / xmin;
			xmin = Max(RW.ATH.adjust * RW.ATH.l[sfb], xmin);
		}else{
			xmin = RW.ATH.adjust * RW.ATH.l[sfb];
		}
		l3_xmin->l[sfb] = xmin * bw;
		if (en0 > RW.ATH.adjust * RW.ATH.l[sfb]) ath_over++;
	}
	return ath_over;
}

/*
 *	org 11.9Kclk@PIII-700
 *	2001/08/01 6500clk@PIII-700?
 *             4100clk@K7-500
 *	現時点でtot_count, max_noiseは参照されていないので削除
 *
 */
#define SUPPORSE_IEE754

#ifndef SUPPORSE_IEE754
#error "can't run"
#endif

typedef struct{
	int exponent;
	float mantissa;
} qfloat;

void calc_noise_long_C( CALC_NOISE_TYPE );

#ifdef CPU_I386

void calc_noise_long_SSE( CALC_NOISE_TYPE );
void calc_noise_long_3DN( CALC_NOISE_TYPE );

void setup_calc_noise_long( int unit )
{
	if( unit & MU_tSSE ){
		calc_noise_long = calc_noise_long_SSE;
	}else
	if( (unit & MU_tMMX) && (unit & MU_t3DN) ){
		calc_noise_long = calc_noise_long_3DN;
	}else
	{
		calc_noise_long = calc_noise_long_C;
	}

	/* asm has shortblock prob. */
	calc_noise_long = calc_noise_long_C;
}
#endif

// CALC_NOISE_WITHOUT_LOG  にする前は 4.5Kclk@K7-500
// CALC_NOISE_WITHOUT_LOG  にして     4.0Kclk@K7-500

void calc_noise_long_C(
	const int max_index,
	const FLOAT8 *xr,
	const int *ix,
	const gr_info *const cod_info,
	const III_psy_xmin *const l3_xmin, 
	const III_scalefac_t *const scalefac,
	III_psy_xmin *xfsf,
	calc_noise_result *const res
){
	qfloat qtot_noise = {0, 1}, qover_noise = {0, 1};
	int sfb, over = 0;
	const int *band_l = RO.scalefac_band.l;
	qfloat qnoise;
//clkbegin(); /* 3800clk@PIII */
	assert( max_index == 21 || max_index == 22 );

	for( sfb = 0; sfb < max_index; sfb++ ){
		int start, end, l, s;
		unsigned int itmp;
		float step, sum, noise;
		start = RO.scalefac_band.l[ sfb ];
		if (start >= RO.ixend)
			break;
		end   = RO.scalefac_band.l[ sfb+1 ];
		if (end > RO.ixend)
			end = RO.ixend;

		s = scalefac->l[sfb];
		if( cod_info->preflag ) s += pretab[sfb];
		assert( -8192 < s && s < 8192 );  // calc_noise_long_E3DN で仮定
		assert( 0 <= cod_info->scalefac_scale && cod_info->scalefac_scale <= 1 );
		s = cod_info->global_gain - (s << (cod_info->scalefac_scale + 1));
		step = POW20(s);
		sum = 0;

		for (l=start; l<end; l++) {
			float temp;
			temp = *xr++ - RO.pow43[*ix++] * step;
			sum += temp * temp;
		}

		noise = xfsf->l[sfb] = sum / l3_xmin->l[sfb];
		itmp = *((unsigned int *)&noise);
		qnoise.exponent = (itmp>>23);	/*  - 127; あとでまとめて引く */
		itmp = (itmp & ((1<<23)-1)) | (127<<23);
		qnoise.mantissa = *((float *)&itmp);
		qtot_noise.exponent += qnoise.exponent;
		qtot_noise.mantissa *= qnoise.mantissa;
		if( noise > 1 ){
			over++;
			qover_noise.exponent += qnoise.exponent;
			qover_noise.mantissa *= qnoise.mantissa;
		}
	} /* for sfb */
	res->over_count = over;

#ifdef CALC_NOISE_WITHOUT_LOG
	/*
		2^a*b (a:expoent, b=mantissa) の形の大小比較をしたいのだがそのままでは
		overflowする。従ってオリジナルではlogをとっていた。
		しかしa, bを別々に計算すればlogをとる必要はない。
		aは 255(最大) * max_index で 13bitで十分なので bの精度を19bitにすれば
		a|bの形(unsigned int)で比較可能。
	 */
	{
		unsigned int imantissa;

		imantissa = *(unsigned int *)&qtot_noise.mantissa;
		res->tot_noise = ((qtot_noise.exponent + (imantissa>>23)) << 19)
						| ((imantissa & ((1<<23)-1)) >> 4);

		imantissa = *(unsigned int *)&qover_noise.mantissa;
		res->over_noise = ((qover_noise.exponent + (imantissa>>23)) << 19)
						| ((imantissa & ((1<<23)-1)) >> 4);
	}
#else
	/* tot_noise, over_noiseがfloatのときのコード */
//#define X(x) ((x.exponent * log(2) + log(x.mantissa))/log(10))*10 /* original */
/* 大小比較のみに使われるので底はなんでもいい */
/* asm化するときにlogじゃなくてlog_2を使えば1回掛け算が減る(意味無し) */
#define X(x) (x.exponent * LOG_E_2 + log(x.mantissa))
	assert (qtot_noise.mantissa >= 1);
	assert (qover_noise.mantissa >= 1);
	qtot_noise.exponent -= 127 * sfb;
	qover_noise.exponent -= 127 * over;
	res->tot_noise = X(qtot_noise);
	res->over_noise = X(qover_noise);
#undef X
#endif
	for (; sfb < max_index; sfb++)
		xfsf->l[sfb] = 0;
//clkend();
} /* calc_noise_long_C */

void calc_noise_short(
	const int max_index,
	const FLOAT8 xr [576],
	const int ix [576],
	const gr_info *const cod_info,
	const III_psy_xmin *const l3_xmin, 
	const III_scalefac_t *const scalefac,
	III_psy_xmin *xfsf,
	calc_noise_result *const res
){
	int sfb, start, end, j, l, i, over = 0;
	FLOAT8 sum;
  
	float noise;
	qfloat qnoise, qtot_noise = {0, 1}, qover_noise = {0, 1};
	unsigned int itmp;
	for( j = 0, sfb = 0; sfb < max_index; sfb++ ){
		start = RO.scalefac_band.s[ sfb ];
		if (start*3 >= RO.ixend)
			break;
		end   = RO.scalefac_band.s[ sfb+1 ];
		for( i = 0; i < 3; i++ ){
			FLOAT8 step;
			int s;
			s = (scalefac->s[sfb][i] << (cod_info->scalefac_scale + 1))
				+ cod_info->subblock_gain[i] * 8;
			s = cod_info->global_gain - s;
			
			assert( s < Q_MAX && s>= 0 );
			step = POW20(s);
			sum = 0;
			for (l = start; l < end && j < RO.ixend; l++) {
				FLOAT8 temp;
				temp = RO.pow43[ix[j]] * step - xr[j];
				j++;
				sum += temp * temp;
			};
			noise = xfsf->s[sfb][i]  = sum / l3_xmin->s[sfb][i];
			itmp = *((unsigned int *)&noise);
			qnoise.exponent = (itmp>>23);	/*  - 127; あとでまとめて引く */
			itmp = (itmp & ((1<<23)-1)) | (127<<23);
			qnoise.mantissa = *((float *)&itmp);
			qtot_noise.exponent += qnoise.exponent;
			qtot_noise.mantissa *= qnoise.mantissa;
			if( noise > 1 ){
				over++;
				qover_noise.exponent += qnoise.exponent;
				qover_noise.mantissa *= qnoise.mantissa;
			}
		} /* for i */
	}
	res->over_count = over;

#ifdef CALC_NOISE_WITHOUT_LOG
	assert( qtot_noise.exponent < (1<<13) && qover_noise.exponent < (1<<13) );
	/* max_idx *3 = 36 or 39 なので1bit削ったほうがいいかもしれないけどまあ大丈夫でしょう */
	/* 心配なら下の方法でもいいけど */
	{
		unsigned int imantissa;

		imantissa = *(unsigned int *)&qtot_noise.mantissa;
		res->tot_noise = ((qtot_noise.exponent + (imantissa>>23)) << 19)
						| ((imantissa & ((1<<23)-1)) >> 4);

		imantissa = *(unsigned int *)&qover_noise.mantissa;
		res->over_noise = ((qover_noise.exponent + (imantissa>>23)) << 19)
						| ((imantissa & ((1<<23)-1)) >> 4);
	}
#else
//#define X(x) ((x.exponent * log(2) + log(x.mantissa))/log(10))*10 /* original */
/* 大小比較のみに使われるので底はなんでもいい */
#define X(x) (x.exponent * LOG_E_2 + log(x.mantissa))
	assert (qtot_noise.mantissa >= 1);
	assert (qover_noise.mantissa >= 1);
	qtot_noise.exponent -= 127 * 3 * sfb;
	qover_noise.exponent -= 127 * over;
	res->tot_noise = X(qtot_noise);
	res->over_noise = X(qover_noise);
#undef X
#endif
	for (; sfb < max_index; sfb++)
		for (i = 0; i < 3; i++)
			xfsf->s[sfb][i] = 0;
} /* calc_noise_short */

#if 0
/* 注意:ショートブロック修正以前版 { */
void calc_noise_long_C(
	const int max_index,
	const FLOAT8 *xr,
	const int *ix,
	const gr_info *const cod_info,
	const III_psy_xmin *const l3_xmin, 
	const III_scalefac_t *const scalefac,
	III_psy_xmin *xfsf,
	calc_noise_result *const res
){
	qfloat qtot_noise = {0, 1}, qover_noise = {0, 1};
	int sfb, over = 0;
	const int *band_l = RO.scalefac_band.l;
	qfloat qnoise;
//clkbegin(); /* 3800clk@PIII */
	assert( max_index == 21 || max_index == 22 );

	for( sfb = 0; sfb < max_index; sfb++ ){
		float step, sum, noise, ftmp;
		int n, *itmp;
		int s = scalefac->l[sfb];
		if( cod_info->preflag ) s += pretab[sfb];
		assert( -8192 < s && s < 8192 );  // calc_noise_long_E3DN で仮定
		assert( 0 <= cod_info->scalefac_scale && cod_info->scalefac_scale <= 1 );
		s = cod_info->global_gain - (s << (cod_info->scalefac_scale + 1));
		step = POW20(s);
		sum = 0;

		n = band_l[sfb+1] - band_l[sfb];
		assert( (n & 1) == 0 );	/* n is even. */
		assert( 4 <= n );  // calc_noise_long_E3DN で仮定
		do{
			float temp;
			temp = *xr++ - RO.pow43[*ix++] * step;
			sum += temp * temp;
			n--;
		}while( n );

		noise = xfsf->l[sfb] = sum / l3_xmin->l[sfb];
		ftmp = noise;
		itmp = (int *)&ftmp;
		qnoise.exponent = (*itmp>>23);	/* - 127 気持ちの問題だけどね */
		*itmp = (*itmp & ((1<<23)-1)) | (127<<23);
		qnoise.mantissa = ftmp;
		qtot_noise.exponent += qnoise.exponent;
		qtot_noise.mantissa *= qnoise.mantissa;
		if( noise > 1 ){
			over++;
			qover_noise.exponent += qnoise.exponent;
			qover_noise.mantissa *= qnoise.mantissa;
		}
	} /* for sfb */
	res->over_count = over;

#ifdef CALC_NOISE_WITHOUT_LOG
	/*
		2^a*b (a:expoent, b=mantissa) の形の大小比較をしたいのだがそのままでは
		overflowする。従ってオリジナルではlogをとっていた。
		しかしa, bを別々に計算すればlogをとる必要はない。
		aは 255(最大) * max_index で 13bitで十分なので bの精度を19bitにすれば
		a|bの形(unsigned int)で比較可能。
	 */
	{
		unsigned int imantissa;

		imantissa = *(unsigned int *)&qtot_noise.mantissa;
		res->tot_noise = ((qtot_noise.exponent + (imantissa>>23)) << 19)
						| ((imantissa & ((1<<23)-1)) >> 4);

		imantissa = *(unsigned int *)&qover_noise.mantissa;
		res->over_noise = ((qover_noise.exponent + (imantissa>>23)) << 19)
						| ((imantissa & ((1<<23)-1)) >> 4);
	}
#else
	/* tot_noise, over_noiseがfloatのときのコード */
//#define X(x) ((x.exponent * log(2) + log(x.mantissa))/log(10))*10 /* original */
/* 大小比較のみに使われるので底はなんでもいい */
/* asm化するときにlogじゃなくてlog_2を使えば1回掛け算が減る(意味無し) */
#define X(x) (x.exponent * LOG_E_2 + log(x.mantissa))
	qtot_noise.exponent -= 127 * max_index;
	qover_noise.exponent -= 127 * over;
	res->tot_noise = X(qtot_noise);
	res->over_noise = X(qover_noise);
#undef X
#endif
//clkend();
} /* calc_noise_long_C */

void calc_noise_short(
	const int max_index,
	const FLOAT8 xr [576],
	const int ix [576],
	const gr_info *const cod_info,
	const III_psy_xmin *const l3_xmin, 
	const III_scalefac_t *const scalefac,
	III_psy_xmin *xfsf,
	calc_noise_result *const res
){
	int sfb, start, end, j, l, i, over = 0;
	FLOAT8 sum;
  
	float noise;
	qfloat qnoise, qtot_noise = {0, 1}, qover_noise = {0, 1};
	int *itmp;
	float ftmp;
	for( j = 0, sfb = 0; sfb < max_index; sfb++ ){
		start = RO.scalefac_band.s[ sfb ];
		end   = RO.scalefac_band.s[ sfb+1 ];
		if (end > RO.ixend) {
			end = RO.ixend;
		}
		for( i = 0; i < 3; i++ ){
			FLOAT8 step;
			int s;
			s = (scalefac->s[sfb][i] << (cod_info->scalefac_scale + 1))
				+ cod_info->subblock_gain[i] * 8;
			s = cod_info->global_gain - s;
			
			assert( s < Q_MAX && s>= 0 );
			step = POW20(s);
			sum = 0;
			l = start;
			do{
				FLOAT8 temp;
				temp = RO.pow43[ix[j]] * step - xr[j];
				j++;
				sum += temp * temp;
				l++;
			}while( l < end );
			noise = xfsf->s[sfb][i]  = sum / l3_xmin->s[sfb][i];
			ftmp = noise;
			itmp = (int *)&ftmp;
			qnoise.exponent = (*itmp>>23);	/*  - 127; あとでまとめて引く */
			*itmp = (*itmp & ((1<<23)-1)) | (127<<23);
			qnoise.mantissa = ftmp;
			qtot_noise.exponent += qnoise.exponent;
			qtot_noise.mantissa *= qnoise.mantissa;
			if( noise > 1 ){
				over++;
				qover_noise.exponent += qnoise.exponent;
				qover_noise.mantissa *= qnoise.mantissa;
			}
		} /* for i */
	}
	res->over_count = over;

#ifdef CALC_NOISE_WITHOUT_LOG
	assert( qtot_noise.exponent < (1<<13) && qover_noise.exponent < (1<<13) );
	/* max_idx *3 = 36 or 39 なので1bit削ったほうがいいかもしれないけどまあ大丈夫でしょう */
	/* 心配なら下の方法でもいいけど */
	{
		unsigned int imantissa;

		imantissa = *(unsigned int *)&qtot_noise.mantissa;
		res->tot_noise = ((qtot_noise.exponent + (imantissa>>23)) << 19)
						| ((imantissa & ((1<<23)-1)) >> 4);

		imantissa = *(unsigned int *)&qover_noise.mantissa;
		res->over_noise = ((qover_noise.exponent + (imantissa>>23)) << 19)
						| ((imantissa & ((1<<23)-1)) >> 4);
	}
#else
//#define X(x) ((x.exponent * log(2) + log(x.mantissa))/log(10))*10 /* original */
/* 大小比較のみに使われるので底はなんでもいい */
#define X(x) (x.exponent * LOG_E_2 + log(x.mantissa))
	qtot_noise.exponent -= 127 * 3 * max_index;
	qover_noise.exponent -= 127 * over;
	res->tot_noise = X(qtot_noise);
	res->over_noise = X(qover_noise);
#undef X
#endif
} /* calc_noise_short */
/* } 注意:ショートブロック修正以前版 */
#endif

#if 0
/* 注意:動かない */
int calc_noise(
	const FLOAT8 xr [576],
	const int ix [576],
	const gr_info *const cod_info,
	const III_psy_xmin *const l3_xmin, 
	const III_scalefac_t *const scalefac,
	III_psy_xmin *xfsf,
	calc_noise_result *const res
){
	int sfb, start, end, j, l, i, over = 0;
	FLOAT8 sum;
  
	float noise;
	float noise_db, over_noise_db = 0;
	float tot_noise_db = 0;     /*    0 dB relative to masking */
	if( cod_info->block_type == SHORT_TYPE ){
		int max_index = RW.sfb21_extra ? SBMAX_s : SBPSY_s;

		for( j = 0, sfb = 0; sfb < max_index; sfb++ ){
			start = RO.scalefac_band.s[ sfb ];
			end   = RO.scalefac_band.s[ sfb+1 ];
			for( i = 0; i < 3; i++ ){
				FLOAT8 step;
				int s;
				s = (scalefac->s[sfb][i] << (cod_info->scalefac_scale + 1))
					+ cod_info->subblock_gain[i] * 8;
				s = cod_info->global_gain - s;

				assert( s < Q_MAX && s>= 0 );
				step = POW20(s);
				sum = 0;
				l = start;
				do{
					FLOAT8 temp;
					temp = RO.pow43[ix[j]] * step - xr[j];
					j++;
					sum += temp * temp;
					l++;
				}while( l < end );
				noise = xfsf->s[sfb][i]  = sum / l3_xmin->s[sfb][i];
			    noise_db = 10*log10(Max(noise,1E-20));
				tot_noise_db += noise_db;
				if( noise > 1 ){
					over++;
					over_noise_db += noise_db;
				}
			} /* for i */
		}
		res->over_count = over;
	}else{ /* cod_info->block_type == SHORT_TYPE */
		/* ほとんどこちら */
		int max_index = RW.sfb21_extra ? SBMAX_l : SBPSY_l;	/* 22 or 21 */
		for( sfb = 0; sfb < max_index; sfb++ ){
			FLOAT8 step;
			int s = scalefac->l[sfb];
			if( cod_info->preflag ) s += pretab[sfb];

			s = cod_info->global_gain - (s << (cod_info->scalefac_scale + 1));
			assert( s < Q_MAX && s>= 0);
			step = POW20(s);
			start = RO.scalefac_band.l[ sfb ];
			end   = RO.scalefac_band.l[ sfb+1 ];

			for( sum = 0, l = start; l < end; l++ ){
				FLOAT8 temp;
				temp = xr[l] - RO.pow43[ix[l]] * step;
				sum += temp * temp;
			}
			noise = xfsf->l[sfb] = sum / l3_xmin->l[sfb];
			noise_db=10*log10(Max(noise,1E-20));
			tot_noise_db += noise_db;

			if( noise > 1 ){
				over++;
				over_noise_db += noise_db;
			}
		} /* for sfb */
		res->over_count = over;
	} /* cod_info->block_type == SHORT_TYPE */

  /* convert to db. DO NOT CHANGE THESE */
  /* tot_noise = is really the average over each sfb of: 
     [noise(db) - allowed_noise(db)]

     and over_noise is the same average, only over only the
     bands with noise > allowed_noise.  

  */
	res->tot_noise   = tot_noise_db;
	res->over_noise  = over_noise_db;
	return res->over_count;
}
#endif /* #if 0 */
