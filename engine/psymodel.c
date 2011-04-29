/*
 *	psymodel.c
 *
 *	Copyright (c) 1999 Mark Taylor
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


/*
PSYCHO ACOUSTICS


This routine computes the psycho acoustics, delayed by
one granule.  

Input: buffer of PCM data (1024 samples).  

This window should be centered over the 576 sample granule window.
The routine will compute the psycho acoustics for
this granule, but return the psycho acoustics computed
for the *previous* granule.  This is because the block
type of the previous granule can only be determined
after we have computed the psycho acoustics for the following
granule.  

Output:  maskings and energies for each scalefactor band.
block type, PE, and some correlation measures.  
The PE is used by CBR modes to determine if extra bits
from the bit reservoir should be used.  The correlation
measures are used to determine mid/side or regular stereo.


Notation:

barks:  a non-linear frequency scale.  Mapping from frequency to
        barks is given by freq2bark()

scalefactor bands: The spectrum (frequencies) are broken into 
                   SBMAX "scalefactor bands".  Thes bands
                   are determined by the MPEG ISO spec.  In
                   the noise shaping/quantization code, we allocate
                   bits among the partition bands to achieve the
                   best possible quality

partition bands:   The spectrum is also broken into about
                   64 "partition bands".  Each partition 
                   band is about .34 barks wide.  There are about 2-5
                   partition bands for each scalefactor band.

LAME computes all psycho acoustic information for each partition
band.  Then at the end of the computations, this information
is mapped to scalefactor bands.  The energy in each scalefactor
band is taken as the sum of the energy in all partition bands
which overlap the scalefactor band.  The maskings can be computed
in the same way (and thus represent the average masking in that band)
or by taking the minmum value multiplied by the number of
partition bands used (which represents a minimum masking in that band).


The general outline is as follows:


1. compute the energy in each partition band
2. compute the tonality in each partition band
3. compute the strength of each partion band "masker"
4. compute the masking (via the spreading function applied to each masker)
5. Modifications for mid/side masking.  

Each partition band is considiered a "masker".  The strength
of the i'th masker in band j is given by:

    s3(bark(i)-bark(j))*strength(i)

The strength of the masker is a function of the energy and tonality.
The more tonal, the less masking.  LAME uses a simple linear formula
(controlled by NMT and TMN) which says the strength is given by the
energy divided by a linear function of the tonality.


s3() is the "spreading function".  It is given by a formula
determined via listening tests.  

The total masking in the j'th partition band is the sum over
all maskings i.  It is thus given by the convolution of
the strength with s3(), the "spreading function."

masking(j) = sum_over_i  s3(i-j)*strength(i)  = s3 o strength

where "o" = convolution operator.  s3 is given by a formula determined
via listening tests.  It is normalized so that s3 o 1 = 1.

Note: instead of a simple convolution, LAME also has the
option of using "additive masking"

The most critical part is step 2, computing the tonality of each
partition band.  LAME has two tonality estimators.  The first
is based on the ISO spec, and measures how predictiable the
signal is over time.  The more predictable, the more tonal.
The second measure is based on looking at the spectrum of
a single granule.  The more peaky the spectrum, the more
tonal.  By most indications, the latter approach is better.

Finally, in step 5, the maskings for the mid and side
channel are possibly increased.  Under certain circumstances,
noise in the mid & side channels is assumed to also
be masked by strong maskers in the L or R channels.


Other data computed by the psy-model:

ms_ratio        side-channel / mid-channel masking ratio (for previous granule)
ms_ratio_next   side-channel / mid-channel masking ratio for this granule

percep_entropy[2]     L and R values (prev granule) of PE - A measure of how 
                      much pre-echo is in the previous granule
percep_entropy_MS[2]  mid and side channel values (prev granule) of percep_entropy
energy[4]             L,R,M,S energy in each channel, prev granule
blocktype_d[2]        block type to use for previous granule

*/

#include <assert.h>

#include "config.h"
#include "global.h"

#include "util.h"
#include "psymodel.h"
#include "tables.h"
#include "lame.h"
#include "vfta.h"

/* size of each partition band, in barks: */
#define DELBARK .34

    /* AAC values, results in more masking over MP3 values */
# define TMN 18
# define NMT 6

#define NBPSY_l  (SBMAX_l)
#define NBPSY_s  (SBMAX_s)


#ifdef M_LN10
#define		LN_TO_LOG10		(M_LN10/10)
#else
#define LN_TO_LOG10 0.2302585093
#endif
#define LOG10 2.30258509299404568402

#define tonalityTblNum 256	/* 128でも問題ない */
float tonalityTbl[(tonalityTblNum+1) * 2];

/* 14Kclk@PIII */
void convolute_energy_C( const int typeFlag, float *eb, float *cb, float *thr, float *nb_12 )
{
	int b;
	for( b = 0;b < RO.npart_l; b++ ){
		int k;
		float tbb, ecb, ctb;
		ecb = 0;
		ctb = 0;
		for( k = RO.s3ind[b][0]; k <= RO.s3ind[b][1]; k++ ){	/* 6..13 loop */
			ecb += RO.s3_l[b][k] * eb[k];	/* sprdngf for Layer III */
			ctb += RO.s3_l[b][k] * cb[k];
		}

	/* calculate the tonality of each threshold calculation partition 
	 * calculate the SNR in each threshold calculation partition 
	 * tonality = -0.299 - .43*log(ctb/ecb);
	 * tonality = 0:           use NMT   (lots of masking)
	 * tonality = 1:           use TMN   (little masking)
	 */

	/* ISO values */
#define CONV1 (-.299)
#define CONV2 (-.43)

#define LIMIT_L 0.048755842987912974		/* exp((1-CONV1)/CONV2) */
#define LIMIT_U 0.498900382609471840		/* exp((0-CONV1)/CONV2)) */
#define LIMIT_L_VAL 0.015848931924441274	/* exp(-LN_TO_LOG10 * TMN) */
#define LIMIT_U_VAL 0.251188643150060620	/* exp(-LN_TO_LOG10 * NMT) */
#define COEFF1 0.573852132673794870
#define COEFF2 1.188133907988000000
		tbb = ecb;
		if( tbb ){
			tbb = ctb / tbb;
			if( tbb <= LIMIT_L ){
				tbb = LIMIT_L_VAL;
			}else if( tbb >= LIMIT_U ){	/* 70%がこちら */
				tbb = LIMIT_U_VAL;
			}else{
#if 1
				int idx;
				tbb = (tbb - LIMIT_L) * (tonalityTblNum / (LIMIT_U - LIMIT_L));
				idx = (int)tbb;
				tbb = tonalityTbl[idx*2] + tonalityTbl[idx*2+1] * (tbb - idx) ;
#else
				tbb = COEFF1 * pow( tbb, COEFF2 );
#endif
			}
		}

	/* at this point, tbb represents the amount the spreading function
	 * will be reduced.  The smaller the value, the less masking.
	 * minval[] = 1 (0db)     says just use tbb.
	 * minval[]= .01 (-20db)  says reduce spreading function by 
	 *                        at least 20db.  
	 */

		tbb = Min(RO.minval[b], tbb);
		ecb *= tbb;

	/* long block pre-echo control.   */
	/* rpelev=2.0, rpelev2=16.0 */
	/* note: all surges in PE are because of this pre-echo formula
	 * for thr[b].  If it this is not used, PE is always around 600
	 */
	/* dont use long block pre-echo control if previous granule was 
	 * a short block.  This is to avoid the situation:   
	 * frame0:  quiet (very low masking)  
	 * frame1:  surge  (triggers short blocks)
	 * frame2:  regular frame.  looks like pre-echo when compared to 
	 *          frame0, but all pre-echo was in frame1.
	 */
	/* chn=0,1   L and R channels
	   chn=2,3   S and M channels.  
	*/
        
		if( typeFlag ){
			thr[b] = ecb;
		}else{
			const float rpelev = 2.0F, rpelev2 = 16.0F;
			thr[b] = Min(ecb, Min(rpelev*nb_12[b*2],rpelev2*nb_12[b*2+1]) );
		}
		nb_12[b*2+1] = nb_12[b*2];
		nb_12[b*2] = ecb;
	}
} /* convolute_energy */


#ifdef CPU_I386
void convolute_energy_SSE( const int typeFlag, float *eb, float *cb, float *thr, float *nb_12 );

void setup_convolute_energy( int unit )
{
	if( unit & MU_tSSE ){
		convolute_energy = convolute_energy_SSE;
	}else
	{
		convolute_energy = convolute_energy_C;
	}
}
#endif

static const int cw_lower_index = 6;

//	21.5Kclk@K7-500
void inner_psy_sub1_C( gogo_thread_data *tl ){
	int j, chn;
	float *wsamp_l;
	float tot0, tot1, tot2, tot3;
	wsamp_l = tl->psywork;
	/* sum total energy at nearly no extra cost */
	tot0 = tl->energy[0] = wsamp_l[0] * wsamp_l[0];
	tot1 = tl->energy[1] = wsamp_l[1] * wsamp_l[1];
	tot2 = tl->energy[2] = wsamp_l[2] * wsamp_l[2];
	tot3 = tl->energy[3] = wsamp_l[3] * wsamp_l[3];
	for( j = 1; j <= BLKSIZE/2; j++ ){
		for( chn = 0; chn < 4; chn++ ){
			float re, im;
			re = wsamp_l[j*4+chn];
			im = wsamp_l[(BLKSIZE - j)*4+chn];
			tl->energy[j*4+chn] = (re * re + im * im) * 0.5F;
		}
		if( j > 10 ){
			tot0 += tl->energy[j*4+0];
			tot1 += tl->energy[j*4+1];
			tot2 += tl->energy[j*4+2];
			tot3 += tl->energy[j*4+3];
		}
	}
	RW.tot_ener[0] = tot0;
	RW.tot_ener[1] = tot1;
	RW.tot_ener[2] = tot2;
	RW.tot_ener[3] = tot3;
}


//	12.7Kclk@K7-500
void inner_psy_sub2_C( gogo_thread_data *tl )
{
	const int numchn = 4;
	int j, b, chn;
	float (*wsamp_s)[BLKSIZE_s*4] = (float (*)[BLKSIZE_s*4])tl->psywork;
	for( b = 0; b < 3; b++ ){
		tl->energy_s[b][0*4+0] = wsamp_s[b][0*4+0] * wsamp_s[b][0*4+0];
		tl->energy_s[b][0*4+1] = wsamp_s[b][0*4+1] * wsamp_s[b][0*4+1];
		tl->energy_s[b][0*4+2] = wsamp_s[b][0*4+2] * wsamp_s[b][0*4+2];
		tl->energy_s[b][0*4+3] = wsamp_s[b][0*4+3] * wsamp_s[b][0*4+3];
		for( j = 1; j <= BLKSIZE_s/2; j++ ){
			for( chn = 0; chn < numchn; chn++ ){
				float re, im;
				re = wsamp_s[b][j*4+chn];
				im = wsamp_s[b][(BLKSIZE_s - j)*4+chn];
				tl->energy_s[b][j*4+chn] = (re * re + im * im) * 0.5F;
			}
		}
	} /* j */
}

INLINE static float logC( float x )
{
	float a, z, zz;
	unsigned int *p;
	int b;
	const float A = 1.0 + 3.104605e-7;		/* A*2 = 2.0000006209 */
	const float B = 1.0/3 - 9.440747e-5;	/* B*2 = 0.6664778517 */
	const float C = 1.0/5 + 6.987293e-3;	/* C*2 = 0.4139745860 */
	p = (unsigned int *)&x;
	b = (*p >> 23) - 127;	/* suppose x_org > 0 */
	*p= ( *p & 0x007FFFFF ) | 0x3F800000;
	a = *(float *)p;
	z = (a - SQRT2) / (a + SQRT2); 
	zz = z * z;
	z = (b+0.5) * LOG_E_2 + z * ((A*2) + zz * ((B*2) + zz * (C*2)));
	return z;
}

/* convolute_energy_C の 4並列版 */
// 60Klck@K7-500
void inner_psy_sub3_C( float *eb, float *cb, float *thr )
{
	int b;
	float *nb_12 = RW.nb_12[0];
	assert(0 < RO.npart_l); // inner_psy_sub3_3DN で仮定
	for( b = 0; b < RO.npart_l; b++ ){
		int k, idx;
		float tbb0, tbb1, tbb2, tbb3;
		float ecb0, ecb1, ecb2, ecb3;

		ecb0 = ecb1 = ecb2 = ecb3 = 0;
		tbb0 = tbb1 = tbb2 = tbb3 = 0;
		assert(RO.s3ind[b][0] <= RO.s3ind[b][1]); // inner_psy_sub3_3DN で仮定
		for( k = RO.s3ind[b][0]; k <= RO.s3ind[b][1]; k++ ){	/* 6..13 loop */
			float s3;
			s3 = RO.s3_l[b][k];
				/* sprdngf for Layer III */
			ecb0 += s3 * eb[k*4+0];
			tbb0 += s3 * cb[k*4+0];
			ecb1 += s3 * eb[k*4+1];
			tbb1 += s3 * cb[k*4+1];
			ecb2 += s3 * eb[k*4+2];
			tbb2 += s3 * cb[k*4+2];
			ecb3 += s3 * eb[k*4+3];
			tbb3 += s3 * cb[k*4+3];
		}

		tbb0 = (ecb0) ? tbb0 / ecb0 : 0;
		tbb1 = (ecb1) ? tbb1 / ecb1 : 0;
		tbb2 = (ecb2) ? tbb2 / ecb2 : 0;
		tbb3 = (ecb3) ? tbb3 / ecb3 : 0;
		tbb0 = Min(Max(tbb0, LIMIT_L), LIMIT_U);
		tbb1 = Min(Max(tbb1, LIMIT_L), LIMIT_U);
		tbb2 = Min(Max(tbb2, LIMIT_L), LIMIT_U);
		tbb3 = Min(Max(tbb3, LIMIT_L), LIMIT_U);

		tbb0 = (tbb0 - LIMIT_L) * (tonalityTblNum / (LIMIT_U - LIMIT_L));
		tbb1 = (tbb1 - LIMIT_L) * (tonalityTblNum / (LIMIT_U - LIMIT_L));
		tbb2 = (tbb2 - LIMIT_L) * (tonalityTblNum / (LIMIT_U - LIMIT_L));
		tbb3 = (tbb3 - LIMIT_L) * (tonalityTblNum / (LIMIT_U - LIMIT_L));

		idx = (int)tbb0;
		tbb0 = tonalityTbl[idx*2] + tonalityTbl[idx*2+1] * (tbb0 - idx) ;

		idx = (int)tbb1;
		tbb1 = tonalityTbl[idx*2] + tonalityTbl[idx*2+1] * (tbb1 - idx) ;

		idx = (int)tbb2;
		tbb2 = tonalityTbl[idx*2] + tonalityTbl[idx*2+1] * (tbb2 - idx) ;

		idx = (int)tbb3;
		tbb3 = tonalityTbl[idx*2] + tonalityTbl[idx*2+1] * (tbb3 - idx) ;

		tbb0 = Min(RO.minval[b], tbb0);
		tbb1 = Min(RO.minval[b], tbb1);
		tbb2 = Min(RO.minval[b], tbb2);
		tbb3 = Min(RO.minval[b], tbb3);
		ecb0 *= tbb0;
		ecb1 *= tbb1;
		ecb2 *= tbb2;
		ecb3 *= tbb3;
		/* RW.nb_12はこの中で完結しているのでどう使おうと問題ない */
		if( RW.blocktype_old[0] == SHORT_TYPE ){
			thr[b*4+0] = ecb0;
			thr[b*4+2] = ecb2;
		}else{
			const float rpelev = 2.0F, rpelev2 = 16.0F;
			thr[b*4+0] = Min(ecb0, Min(rpelev*nb_12[(b*2)*4+0],rpelev2*nb_12[(b*2+1)*4+0]) );
			thr[b*4+2] = Min(ecb2, Min(rpelev*nb_12[(b*2)*4+2],rpelev2*nb_12[(b*2+1)*4+2]) );
		}
		if( RW.blocktype_old[1] == SHORT_TYPE ){
			thr[b*4+1] = ecb1;
			thr[b*4+3] = ecb3;
		}else{
			const float rpelev = 2.0F, rpelev2 = 16.0F;
			thr[b*4+1] = Min(ecb1, Min(rpelev*nb_12[(b*2)*4+1],rpelev2*nb_12[(b*2+1)*4+1]) );
			thr[b*4+3] = Min(ecb3, Min(rpelev*nb_12[(b*2)*4+3],rpelev2*nb_12[(b*2+1)*4+3]) );
		}

		nb_12[(b*2+1)*4+0] = nb_12[(b*2)*4+0];
		nb_12[(b*2+1)*4+1] = nb_12[(b*2)*4+1];
		nb_12[(b*2+1)*4+2] = nb_12[(b*2)*4+2];
		nb_12[(b*2+1)*4+3] = nb_12[(b*2)*4+3];
		nb_12[(b*2)*4+0] = ecb0;
		nb_12[(b*2)*4+1] = ecb1;
		nb_12[(b*2)*4+2] = ecb2;
		nb_12[(b*2)*4+3] = ecb3;
	}
	RW.pe[0] = RW.pe[1] = RW.pe[2] = RW.pe[3] = 0;

	/* log部分統合 */
	/* 33Kclk =>26Kclk */
#define log logC
	for( b = 0;b < RO.npart_l; b++ ){
		float th0, th1, th2, th3;
		float ath, num;
		ath = RW.ATH.cb[b];
		num = RO.numlines_l[b];
		th0 = Max(thr[b*4+0], ath);
		th1 = Max(thr[b*4+1], ath);
		th2 = Max(thr[b*4+2], ath);
		th3 = Max(thr[b*4+3], ath);
		th0 = (eb[b*4+0]) ? th0 /= eb[b*4+0] : 1.0;
		th1 = (eb[b*4+1]) ? th1 /= eb[b*4+1] : 1.0;
		th2 = (eb[b*4+2]) ? th2 /= eb[b*4+2] : 1.0;
		th3 = (eb[b*4+3]) ? th3 /= eb[b*4+3] : 1.0;
		th0 = Min(th0, 1.0);
		th1 = Min(th1, 1.0);
		th2 = Min(th2, 1.0);
		th3 = Min(th3, 1.0);
		RW.pe[0] -= num * log( th0 );
		RW.pe[1] -= num * log( th1 );
		RW.pe[2] -= num * log( th2 );
		RW.pe[3] -= num * log( th3 );
	}
#undef log
} /* inner_psy_sub3_C */

// 37Klck@K7-500
void inner_psy_sub4_C( gogo_thread_data *tl )
{
	int j, k, chn;
	const int numchn = 4;
	float (*wsamp_s)[BLKSIZE_s*4];
	wsamp_s = (float (*)[BLKSIZE_s*4])tl->psywork;

	assert(cw_lower_index < RO.cw_upper_index); // inner_psy_sub4_3DN で仮定
	for( j = cw_lower_index; j < RO.cw_upper_index; j += 4 ){
		k = (j+2) / 4;	/* 2 <= k <= ?? */
		for( chn = 0; chn < numchn; chn++ ){
			float R, I, r, E1, E2, E3, a, b, den, tmp;
			E1 = tl->energy_s[0][k*4+chn];
			a = wsamp_s[0][k*4+chn];
			b = wsamp_s[0][(BLKSIZE_s-k)*4+chn];
			r = sqrt(E1);
			if( E1 == 0 ){
				E1 = a = b = 1;
			}
			R = a*b;
			I = (a*a - b*b) * 0.5F;
			den = E1;
			E2 = tl->energy_s[2][k*4+chn];
			a = wsamp_s[2][k*4+chn];
			b = wsamp_s[2][(BLKSIZE_s-k)*4+chn];
			tmp = sqrt(E2);
			if( E2 == 0 ){
				a = b = 1;
			}
			r = r*2 - tmp;
			if( E2 ) den *= tmp;
			if( !E2 ) E2 = 1;
			tmp = a*I + b*R;
			R = a*R - b*I;
			I = tmp;
			E3 = tl->energy_s[1][k*4+chn];
			tmp = sqrt(E3) + fabs(r);
			a = wsamp_s[1][k*4+chn];
			b = wsamp_s[1][(BLKSIZE_s-k)*4+chn];
			r = r / den;
			E3 += (r*E1) * (r*E1) * E2 - r * (a*I + b*R);
			if( E3 < 0 ) tmp = 0;	/* 数式上は不要だけど計算誤差のため必要 */
			if( tmp ) tmp = sqrt(E3) / tmp;
			RW.cw[j*4+chn] = RW.cw[(j+1)*4+chn] = RW.cw[(j+2)*4+chn] = RW.cw[(j+3)*4+chn] = tmp;
		} /* chn */
	} /* j */
} /* inner_psy_sub4_C */

/* 27Kclk@K7-500 */
void inner_psy_sub5_C( gogo_thread_data *tl )
{
	int k, b;
	float *eb  = &tl->psywork[0];
	float *thr = &tl->psywork[CBANDS*8];
	for( k = 0; k < 3; k++ ){
		float ecb0, ecb1, ecb2, ecb3;
		int i, j;
		j = 0;
		for( b = 0; b < RO.npart_s_orig; b++ ){
			ecb0 = ecb1 = ecb2 = ecb3 = 0;
			// inner_psy_sub5_3DN で仮定
			assert( 0 < RO.numlines_s[b] && RO.numlines_s[b] < 256 ); 
			for( i = 0; i < RO.numlines_s[b]; i++ ){
				ecb0 += tl->energy_s[k][j*4+0];
				ecb1 += tl->energy_s[k][j*4+1];
				ecb2 += tl->energy_s[k][j*4+2];
				ecb3 += tl->energy_s[k][j*4+3];
				j++;
			}
			eb[b*4+0] = ecb0;
			eb[b*4+1] = ecb1;
			eb[b*4+2] = ecb2;
			eb[b*4+3] = ecb3;
		}

		for ( b = 0; b < RO.npart_s; b++ ){
			float tmp;
			ecb0 = ecb1 = ecb2 = ecb3 = 0;
			// inner_psy_sub5_3DN で仮定
			assert( 0 <= RO.s3ind_s[b][0] && RO.s3ind_s[b][0] < 256 ); 
			assert( 0 <= RO.s3ind_s[b][1] && RO.s3ind_s[b][1] < 256 ); 
			assert( RO.s3ind_s[b][0] <= RO.s3ind_s[b][1] ); 
			for( i = RO.s3ind_s[b][0]; i <= RO.s3ind_s[b][1]; i++ ){
				tmp = RO.s3_s[b][i];
				ecb0 += tmp * eb[i*4+0];
				ecb1 += tmp * eb[i*4+1];
				ecb2 += tmp * eb[i*4+2];
				ecb3 += tmp * eb[i*4+3];
			}
			tmp = RO.SNR_s[b];
			ecb0 *= tmp;
			ecb1 *= tmp;
			ecb2 *= tmp;
			ecb3 *= tmp;
			thr[b*4+0] = Max(1e-6, ecb0);
			thr[b*4+1] = Max(1e-6, ecb1);
			thr[b*4+2] = Max(1e-6, ecb2);
			thr[b*4+3] = Max(1e-6, ecb3);
		}

		for( b = 0; b < NBPSY_s; b++ ){
			float enn0, enn1, enn2, enn3;
			float thm0, thm1, thm2, thm3;
			int bu, bo;
			bu = RO.bu_s[b]*4;
			bo = RO.bo_s[b]*4;
			enn0 = 0.5 * ( eb[bu+0] +  eb[bo+0]);
			thm0 = 0.5 * (thr[bu+0] + thr[bo+0]);
			enn1 = 0.5 * ( eb[bu+1] +  eb[bo+1]);
			thm1 = 0.5 * (thr[bu+1] + thr[bo+1]);
			enn2 = 0.5 * ( eb[bu+2] +  eb[bo+2]);
			thm2 = 0.5 * (thr[bu+2] + thr[bo+2]);
			enn3 = 0.5 * ( eb[bu+3] +  eb[bo+3]);
			thm3 = 0.5 * (thr[bu+3] + thr[bo+3]);
			for( i = RO.bu_s[b]+1; i < RO.bo_s[b]; i++ ){
				enn0 +=  eb[i*4+0];
				thm0 += thr[i*4+0];
				enn1 +=  eb[i*4+1];
				thm1 += thr[i*4+1];
				enn2 +=  eb[i*4+2];
				thm2 += thr[i*4+2];
				enn3 +=  eb[i*4+3];
				thm3 += thr[i*4+3];
			}
			RW.en [0].s[b][k] = enn0;
			RW.thm[0].s[b][k] = thm0;
			RW.en [1].s[b][k] = enn1;
			RW.thm[1].s[b][k] = thm1;
			RW.en [2].s[b][k] = enn2;
			RW.thm[2].s[b][k] = thm2;
			RW.en [3].s[b][k] = enn3;
			RW.thm[3].s[b][k] = thm3;
		}
	}
} /* inner_psy_sub5_C */

/* 7.4Kclk@K7-500 */
void inner_psy_sub6_C( gogo_thread_data *tl)
{
	int b, j;
	float *eb = &tl->psywork[0];
	float *cb = &tl->psywork[CBANDS*4];
	b = j = 0;
	/* 7Kclk */
	while( b < RO.npart_l_pre_max ){
		float ebb0, ebb1, ebb2, ebb3;
		float cbb0, cbb1, cbb2, cbb3;
		int i;
		ebb0 = ebb1 = ebb2 = ebb3 = 0;
		cbb0 = cbb1 = cbb2 = cbb3 = 0;
		assert( RO.numlines_l[b] );
		for( i = 0; i < RO.numlines_l[b]; i++ ){
			ebb0 += tl->energy[j*4+0];
			cbb0 += tl->energy[j*4+0] * RW.cw[j*4+0];
			ebb1 += tl->energy[j*4+1];
			cbb1 += tl->energy[j*4+1] * RW.cw[j*4+1];
			ebb2 += tl->energy[j*4+2];
			cbb2 += tl->energy[j*4+2] * RW.cw[j*4+2];
			ebb3 += tl->energy[j*4+3];
			cbb3 += tl->energy[j*4+3] * RW.cw[j*4+3];
			j++;
		}
		eb[b*4+0] = ebb0;
		cb[b*4+0] = cbb0;
		eb[b*4+1] = ebb1;
		cb[b*4+1] = cbb1;
		eb[b*4+2] = ebb2;
		cb[b*4+2] = cbb2;
		eb[b*4+3] = ebb3;
		cb[b*4+3] = cbb3;
		b++;
	}

	/* 2.5Kclk */
	for( ; b < RO.npart_l_orig; b++ ){
		float ebb0, ebb1, ebb2, ebb3;
		int i;
		assert( RO.numlines_l[b] );
		ebb0 = ebb1 = ebb2 = ebb3 = 0;
		for( i = 0; i < RO.numlines_l[b]; i++ ){
			ebb0 += tl->energy[j*4+0];
			ebb1 += tl->energy[j*4+1];
			ebb2 += tl->energy[j*4+2];
			ebb3 += tl->energy[j*4+3];
			j++;
		}
		eb[b*4+0] = ebb0;
		cb[b*4+0] = ebb0 * 0.4;
		eb[b*4+1] = ebb1;
		cb[b*4+1] = ebb1 * 0.4;
		eb[b*4+2] = ebb2;
		cb[b*4+2] = ebb2 * 0.4;
		eb[b*4+3] = ebb3;
		cb[b*4+3] = ebb3 * 0.4;
	}
} /* inner_psy_sub6_C */

#ifdef CPU_I386
void inner_psy_sub1_SSE( gogo_thread_data *tl );
void inner_psy_sub2_SSE( gogo_thread_data *tl );
void inner_psy_sub3_SSE( float *eb, float *cb, float *thr );
void inner_psy_sub3_SSE2( float *eb, float *cb, float *thr );
void inner_psy_sub3_SSE2_P4( float *eb, float *cb, float *thr );
void inner_psy_sub4_SSE( gogo_thread_data *tl );
void inner_psy_sub4_SSE_P4( gogo_thread_data *tl );
void inner_psy_sub5_SSE( gogo_thread_data *tl );
void inner_psy_sub5_SSE_P7( gogo_thread_data *tl );
void inner_psy_sub6_SSE( gogo_thread_data *tl );
void inner_psy_sub1_3DN( gogo_thread_data *tl );
void inner_psy_sub2_3DN( gogo_thread_data *tl );
void inner_psy_sub3_3DN_HI( float *eb, float *cb, float *thr );
void inner_psy_sub3_3DN_FAST( float *eb, float *cb, float *thr );
void inner_psy_sub4_E3DN_HI( gogo_thread_data *tl );
void inner_psy_sub4_E3DN_FAST( gogo_thread_data *tl );
void inner_psy_sub4_3DN( gogo_thread_data *tl );
void inner_psy_sub5_3DN( gogo_thread_data *tl );
void inner_psy_sub6_3DN( gogo_thread_data *tl );

void setup_inner_psy_ch4( int unit )
{
	if( (unit & MU_tSSE) && (unit & MU_tMMX) && (unit & MU_t3DN) && (unit & MU_tE3DN)) {
		// for Athlon-XP
		inner_psy_sub1 = inner_psy_sub1_3DN;     // _SSE 5.65kclk _3DN 5.21kclk
		inner_psy_sub2 = inner_psy_sub2_3DN;     // _SSE 3.82kclk _3DN 3.32kclk
		if (USE_LOW_PRECISIOIN) {                // _SSE 23.0kclk _3DN_HI 19.3kclk
			inner_psy_sub3 = inner_psy_sub3_3DN_FAST;
		} else 
		{
			inner_psy_sub3 = inner_psy_sub3_3DN_HI;
		}
		if (USE_LOW_PRECISIOIN) {                // _SSE 9.22kclk _E3DN_HI 9.61kclk _E3DN_FAST 7.20kclk
			inner_psy_sub4 = inner_psy_sub4_E3DN_FAST;
		} else 
		{
			inner_psy_sub4 = inner_psy_sub4_SSE;
		}
		inner_psy_sub5 = inner_psy_sub5_SSE;     // _SSE 9.51kclk _3DN 13.1kclk
		inner_psy_sub6 = inner_psy_sub6_3DN;     // _SSE 3.70kclk _3DN 3.12kclk
	}else
	if( unit & MU_tSSE2 ){
		inner_psy_sub1 = inner_psy_sub1_SSE;
		inner_psy_sub2 = inner_psy_sub2_SSE;
		inner_psy_sub3 = inner_psy_sub3_SSE2_P4;
		inner_psy_sub4 = inner_psy_sub4_SSE_P4;
		inner_psy_sub5 = inner_psy_sub5_SSE_P7;
		inner_psy_sub6 = inner_psy_sub6_SSE;
	}else
	if( unit & MU_tSSE ){
		inner_psy_sub1 = inner_psy_sub1_SSE;
		inner_psy_sub2 = inner_psy_sub2_SSE;
		inner_psy_sub3 = inner_psy_sub3_SSE;
		inner_psy_sub4 = inner_psy_sub4_SSE;
		inner_psy_sub5 = inner_psy_sub5_SSE;
		inner_psy_sub6 = inner_psy_sub6_SSE;
	}else
	if( (unit & MU_tMMX) && (unit & MU_t3DN) ){
		inner_psy_sub1 = inner_psy_sub1_3DN;
		inner_psy_sub2 = inner_psy_sub2_3DN;
		if (USE_LOW_PRECISIOIN) {
			inner_psy_sub3 = inner_psy_sub3_3DN_FAST;
		} else 
		{
			inner_psy_sub3 = inner_psy_sub3_3DN_HI;
		}
		if (unit & MU_tE3DN) {
			if (USE_LOW_PRECISIOIN) {
				inner_psy_sub4 = inner_psy_sub4_E3DN_FAST;
			} else 
			{
				inner_psy_sub4 = inner_psy_sub4_E3DN_HI;
			}
		}
		else {
			inner_psy_sub4 = inner_psy_sub4_3DN;
		}
		inner_psy_sub5 = inner_psy_sub5_3DN;
		inner_psy_sub6 = inner_psy_sub6_3DN;
	}else
	{
		inner_psy_sub1 = inner_psy_sub1_C;
		inner_psy_sub2 = inner_psy_sub2_C;
		inner_psy_sub3 = inner_psy_sub3_C;
		inner_psy_sub4 = inner_psy_sub4_C;
		inner_psy_sub5 = inner_psy_sub5_C;
		inner_psy_sub6 = inner_psy_sub6_C;
	}
}
#endif

/* 各ブロックはSIMD化できる */
static void inner_psy_chn4( gogo_thread_data *tl, int gr, int uselongblock[2] )
{
	const int numchn = 4;
	int chn;
	int j, b;
	const float rsq2 = SQRT2 * 0.5;
	/* convolution   */
	float *eb = &tl->psywork[0];
	float *cb = &tl->psywork[CBANDS*4];
	float *thr= &tl->psywork[CBANDS*8];

	/* wsamp_lのインタリーブ */
	/* 13Kclk */

#if defined(USE_VECTOR4_FHT)
	if( fht_vector4 ){
#if 1
		/* 11.8Kclk => 7Kclk */
		psy_prepare(tl->psywork, &tl->wsamp_L[0][0][gr*2], BLKSIZE);
#else
		float *src, *dest, L, R;
		src = &tl->wsamp_L[0][0][gr*2];
		dest = tl->psywork;
		for( j = 0; j < BLKSIZE; j++ ){
			L = src[0];
			R = src[1];
			dest[0] = L;
			dest[1] = R;
			dest[2] = (L + R) * rsq2;
			dest[3] = (L - R) * rsq2;
			src  += 4;
			dest += 4;
		}
#endif
	}else
#endif
	{
		float *src0, *src1, *dest, L, R;
		src0 = tl->wsamp_L[gr][0];
		src1 = tl->wsamp_L[gr][1];
		dest = tl->psywork;
		for( j = 0; j < BLKSIZE; j++ ){
			L = *src0++;
			R = *src1++;
			dest[0] = L;
			dest[1] = R;
			dest[2] = (L + R) * rsq2;
			dest[3] = (L - R) * rsq2;
			dest += 4;
		}
	}

	/* 32Kclk => 6Kclk */
	inner_psy_sub1( tl );

	/* 6Kclk */
	for( j = 0; j < cw_lower_index; j++ ){
		float *wsamp_l;
		wsamp_l = tl->psywork;
		for( chn = 0; chn < numchn; chn++ ){
		/**********************************************************************
		 *    compute unpredicatability of first six spectral lines            * 
		 **********************************************************************/
			/* calculate unpredictability measure cw */
			Abr_t *sn, *s1, s2;
			float numre, numim, den;

			s2 = RW.sav[1][j][chn];
			RW.sav[1][j][chn] = RW.sav[0][j][chn];
			s1 = &RW.sav[1][j][chn];
			RW.sav[0][j][chn].a = wsamp_l[j*4+chn];
			RW.sav[0][j][chn].b = j==0 ? wsamp_l[0*4+chn] : wsamp_l[(BLKSIZE-j)*4+chn];  
			RW.sav[0][j][chn].r = sqrt(tl->energy[j*4+chn]);
			sn = &RW.sav[0][j][chn];

		/* square (x1,y1) */
			if( s1->r ){
				numre = ( s1->a * s1->b );
				numim = (s1->a * s1->a- s1->b * s1->b)*(FLOAT)0.5;
				den = s1->r*s1->r;
			}else{
				numre = 1;
				numim = 0;
				den = 1;
			}
		/* multiply by (x2,-y2) */
			if( s2.r ){
				float tmp1, tmp2;
				tmp2 = (numim+numre)*(s2.a+s2.b)*(FLOAT)0.5;
				tmp1 = -s2.a*numre+tmp2;
				numre = -s2.b*numim+tmp2;
				numim = tmp1;
				den *= s2.r;
			}

			{ /* r-prime factor */
				float tmp = (2*s1->r-s2.r)/den;
				numre *= tmp;
				numim *= tmp;
			}
			den = sn->r + fabs(2*s1->r-s2.r);
			if( den ){
				numre = (sn->a+sn->b)*(FLOAT)0.5-numre;
				numim = (sn->a-sn->b)*(FLOAT)0.5-numim;
				den = sqrt(numre*numre+numim*numim)/den;
			}
			RW.cw[j*4+chn] = den;
		} /* chn */
	} /* j */

	/* wsamp_sのインタリーブ */
	/* 10Kclk => 5Kclk */
#if defined(USE_VECTOR4_FHT)
	if( fht_vector4 ){
#if 1
		psy_prepare(tl->psywork, &tl->wsamp_S[0][0][0][gr*2], BLKSIZE_s * 3);
#else
		float *src, *dest, L, R;
		src = &tl->wsamp_S[0][0][0][gr*2];
		dest = tl->psywork;
		for( j = 0; j < BLKSIZE_s * 3; j++ ){
			L = src[0];
			R = src[1];
			dest[0] = L;
			dest[1] = R;
			dest[2] = (L + R) * rsq2;
			dest[3] = (L - R) * rsq2;
			src  += 4;
			dest += 4;
		}
#endif
	}else
#endif
	{
		float *src0, *src1, *dest, L, R;
		src0 = tl->wsamp_S[gr][0][0];
		src1 = tl->wsamp_S[gr][1][0];
		dest = tl->psywork;
		for( j = 0; j < BLKSIZE_s * 3; j++ ){
			L = *src0++;
			R = *src1++;
			dest[0] = L;
			dest[1] = R;
			dest[2] = (L + R) * rsq2;
			dest[3] = (L - R) * rsq2;
			dest += 4;
		}
	}

	/**********************************************************************
	 *  compute energies
	 **********************************************************************/

	/* 18Kclk => 5Kclk */
	inner_psy_sub2( tl );

	/* 76Kclk => 71Kclk =>16Kclk */
	inner_psy_sub4( tl );

	/* これ以降 tl->psywork の値は自由に使えるので eb, cb, thr に割り当てる */
    /**********************************************************************
     *    Calculate the energy and the unpredictability in the threshold   *
     *    calculation partitions                                           *
     **********************************************************************/

	/* 7K + 2.5Kclk => 4.5Kclk */
	inner_psy_sub6( tl );

	/**********************************************************************
	 *      convolve the partitioned energy and unpredictability           *
	 *      with the spreading function, s3_l[b][k]                        *
	 ******************************************************************** */

			/*  calculate percetual entropy */

	/* 60K + 32K clk => 25Kclk */
	inner_psy_sub3( eb, cb, thr );

    /*************************************************************** 
     * determine the block type (window type) based on L & R channels
     * 
     ***************************************************************/

	/* 2Kclk */
	{
		/* compute PE for all 4 channels */
		float tmp;
		float m0, m1, m2, m3;
		float min0, min1, min2, min3;
		float max0, max1, max2, max3;

		min0 = min1 = min2 = min3 = 0;
		/* old mina */
		for( j = HBLKSIZE_s/2; j < HBLKSIZE_s; j++ ){
			min0 += tl->energy_s[0][j*4+0];
			min1 += tl->energy_s[0][j*4+1];
			min2 += tl->energy_s[0][j*4+2];
			min3 += tl->energy_s[0][j*4+3];
		}
		max0 = min0;
		max1 = min1;
		max2 = min2;
		max3 = min3;

		/* old mb */
		m0 = m1 = m2 = m3 = 0;
		for( j = HBLKSIZE_s/2; j < HBLKSIZE_s; j++ ){
			m0 += tl->energy_s[1][j*4+0];
			m1 += tl->energy_s[1][j*4+1];
			m2 += tl->energy_s[1][j*4+2];
			m3 += tl->energy_s[1][j*4+3];
		}
		min0 = Min(min0, m0); max0 = Max(max0, m0);
		min1 = Min(min1, m1); max1 = Max(max1, m1);
		min2 = Min(min2, m2); max2 = Max(max2, m2);
		min3 = Min(min3, m3); max3 = Max(max3, m3);

		/* old mc */
		m0 = m1 = m2 = m3 = 0;
		for( j = HBLKSIZE_s/2; j < HBLKSIZE_s; j++ ){
			m0 += tl->energy_s[2][j*4+0];
			m1 += tl->energy_s[2][j*4+1];
			m2 += tl->energy_s[2][j*4+2];
			m3 += tl->energy_s[2][j*4+3];
		}
		min0 = Min(min0, m0); max0 = Max(max0, m0);
		min1 = Min(min1, m1); max1 = Max(max1, m1);
		min2 = Min(min2, m2); max2 = Max(max2, m2);
		min3 = Min(min3, m3); max3 = Max(max3, m3);

		/* bit allocation is based on pe.  */
		/* max = min = 0のときがあるので注意 */
		/* max の代わりに max + 1e-12 でもいい */
#define adj	1e-12
		tmp = 400 * log((max0+adj) / (min0+adj));
		if( tmp > RW.pe[0] ) RW.pe[0] = tmp;
		tmp = 400 * log((max1+adj) / (min1+adj));
		if( tmp > RW.pe[1] ) RW.pe[1] = tmp;
		tmp = 400 * log((max2+adj) / (min2+adj));
		if( tmp > RW.pe[2] ) RW.pe[2] = tmp;
		tmp = 400 * log((max3+adj) / (min3+adj));
		if( tmp > RW.pe[3] ) RW.pe[3] = tmp;
#undef adj

		/* block type is based just on L or R channel */      
		uselongblock[0] = 1;
		uselongblock[1] = 1;

		if( (RW.pe[0] > 3000) || (max0 > 30 * min0) || (max0 > 10 * min0 && RW.pe[0] > 1000) ){
			uselongblock[0] = 0;
		}
		if( (RW.pe[1] > 3000) || (max1 > 30 * min1) || (max1 > 10 * min1 && RW.pe[1] > 1000) ){
			uselongblock[1] = 0;
		}
	}

    /*************************************************************** 
     * compute masking thresholds for both short and long blocks
     ***************************************************************/
    /* longblock threshold calculation (part 2) */

	/* 2Kclk */
	for( b = 0; b < NBPSY_l; b++ ){
		float enn0, enn1, enn2, enn3;
		float thm0, thm1, thm2, thm3;
		int bu, bo;
		bu = RO.bu_l[b] * 4;
		bo = RO.bo_l[b] * 4;
		enn0 = 0.5 * ( eb[bu+0] +  eb[bo+0]);
		thm0 = 0.5 * (thr[bu+0] + thr[bo+0]);
		enn1 = 0.5 * ( eb[bu+1] +  eb[bo+1]);
		thm1 = 0.5 * (thr[bu+1] + thr[bo+1]);
		enn2 = 0.5 * ( eb[bu+2] +  eb[bo+2]);
		thm2 = 0.5 * (thr[bu+2] + thr[bo+2]);
		enn3 = 0.5 * ( eb[bu+3] +  eb[bo+3]);
		thm3 = 0.5 * (thr[bu+3] + thr[bo+3]);
		for( j = RO.bu_l[b]+1; j < RO.bo_l[b]; j++ ){
			enn0 += eb[j*4+0];
			thm0 += thr[j*4+0];
			enn1 += eb[j*4+1];
			thm1 += thr[j*4+1];
			enn2 += eb[j*4+2];
			thm2 += thr[j*4+2];
			enn3 += eb[j*4+3];
			thm3 += thr[j*4+3];
		}
		RW.en [0].l[b] = enn0;
		RW.thm[0].l[b] = thm0;
		RW.en [1].l[b] = enn1;
		RW.thm[1].l[b] = thm1;
		RW.en [2].l[b] = enn2;
		RW.thm[2].l[b] = thm2;
		RW.en [3].l[b] = enn3;
		RW.thm[3].l[b] = thm3;
	}

    /* threshold calculation for short blocks */
	/* 34Kclk */
	inner_psy_sub5( tl );
} /* inner_psy_chn4 */

static void inner_psy_general( gogo_thread_data *tl, int uselongblock[2], int numchn, int gr_out );

/*
   L3psycho_anal.  Compute psycho acoustics.

   Data returned to the calling program must be delayed by one 
   granule. 

   This is done in two places.  
   If we do not need to know the blocktype, the copying
   can be done here at the top of the program: we copy the data for
   the last granule (computed during the last call) before it is
   overwritten with the new data.  It looks like this:
  
   0. static psymodel_data 
   1. calling_program_data = psymodel_data
   2. compute psymodel_data
    
   For data which needs to know the blocktype, the copying must be
   done at the end of this loop, and the old values must be saved:
   
   0. static psymodel_data_old 
   1. compute psymodel_data
   2. compute possible block type of this granule
   3. compute final block type of previous granule based on #2.
   4. calling_program_data = psymodel_data_old
   5. psymodel_data_old = psymodel_data
*/
void L3psycho_anal(gogo_thread_data *tl,
                    /* const sample_t *buffer[2], */ int gr_out, 
                    FLOAT8 *ms_ratio,
                    FLOAT8 *ms_ratio_next,
		    III_psy_ratio masking_ratio[2],
		    III_psy_ratio masking_MS_ratio[2],
		    FLOAT8 percep_entropy[2],FLOAT8 percep_MS_entropy[2], 
                    FLOAT8 energy[4],
                    int blocktype_d[2])
{
/* to get a good cache performance, one has to think about
 * the sequence, in which the variables are used.  
 * (Note: these static variables have been moved to the RW. struct,
 * and their order in memory is layed out in util.h)
 */

  /* ratios    */
  FLOAT8 ms_ratio_l=0,ms_ratio_s=0;

	/* block type  */
	int uselongblock[2];
  
  /* usual variables like loop indices, etc..    */
  int numchn, chn;
  int	sb,sblock;

  numchn = RO.channels_out;
  /* chn=2 and 3 = Mid and Side channels */
  if (RO.mode == JOINT_STEREO) numchn=4;

	for( chn = 0; chn < numchn; chn++ ){
		energy[chn] = RW.tot_ener[chn];
      /* there is a one granule delay.  Copy maskings computed last call
       * into masking_ratio to return to calling program.
       */
		if( chn < 2 ){
			/* LR maskings  */
			percep_entropy[chn]    = RW.pe [chn]; 
			masking_ratio[chn].en  = RW.en [chn];
			masking_ratio[chn].thm = RW.thm[chn];
		}else{
			/* MS maskings  */
			percep_MS_entropy[chn-2]     = RW.pe [chn]; 
			masking_MS_ratio [chn-2].en  = RW.en [chn];
			masking_MS_ratio [chn-2].thm = RW.thm[chn];
		}
	} /* chn */

	if( numchn == 4 ){
		inner_psy_chn4( tl, gr_out, uselongblock );
	}else
	{	/* numchn < 4 のとき */
		inner_psy_general( tl, uselongblock, numchn, gr_out );
	}

  /* compute M/S thresholds from Johnston & Ferreira 1992 ICASSP paper */

	/* 5Kclk */
  if( RO.mode == JOINT_STEREO ){
    FLOAT8 rside,rmid,mld;
    int chmid=2,chside=3; 
    
    for ( sb = 0; sb < NBPSY_l; sb++ ) {
      /* use this fix if L & R masking differs by 2db or less */
      /* if db = 10*log10(x2/x1) < 2 */
      /* if (x2 < 1.58*x1)  */
      if (RW.thm[0].l[sb] <= 1.58*RW.thm[1].l[sb]
	  && RW.thm[1].l[sb] <= 1.58*RW.thm[0].l[sb]) {

	mld = RO.mld_l[sb]*RW.en[chside].l[sb];
	rmid = Max(RW.thm[chmid].l[sb], Min(RW.thm[chside].l[sb],mld));

	mld = RO.mld_l[sb]*RW.en[chmid].l[sb];
	rside = Max(RW.thm[chside].l[sb],Min(RW.thm[chmid].l[sb],mld));

	RW.thm[chmid].l[sb]=rmid;
	RW.thm[chside].l[sb]=rside;
      }
    }
    for ( sb = 0; sb < NBPSY_s; sb++ ) {
      for ( sblock = 0; sblock < 3; sblock++ ) {
	if (RW.thm[0].s[sb][sblock] <= 1.58*RW.thm[1].s[sb][sblock]
	    && RW.thm[1].s[sb][sblock] <= 1.58*RW.thm[0].s[sb][sblock]) {

	  mld = RO.mld_s[sb]*RW.en[chside].s[sb][sblock];
	  rmid = Max(RW.thm[chmid].s[sb][sblock],Min(RW.thm[chside].s[sb][sblock],mld));

	  mld = RO.mld_s[sb]*RW.en[chmid].s[sb][sblock];
	  rside = Max(RW.thm[chside].s[sb][sblock],Min(RW.thm[chmid].s[sb][sblock],mld));

	  RW.thm[chmid].s[sb][sblock]=rmid;
	  RW.thm[chside].s[sb][sblock]=rside;
	}
      }
    }

		{
			/* determin ms_ratio from masking thresholds*/
			/* use ms_stereo (ms_ratio < .35) if average thresh. diff < 5 db */
			double x1, x2, sidetot;
			sidetot = 1;
			/* 30回程度のループなのでoverflowは起こらない */
			for( sb= NBPSY_l/4 ; sb < NBPSY_l; sb++ ){
				x1 = Min( RW.thm[0].l[sb], RW.thm[1].l[sb] );
				x2 = Max( RW.thm[0].l[sb], RW.thm[1].l[sb] );
				if( x2 >= x1 * 1000 ){
					x2 = 1000;
				}else{
					x2 /= x1;
				}
				sidetot *= x2;
			}
			sidetot = log10( sidetot );	/* db */
			ms_ratio_l= sidetot * ( 0.7 / (NBPSY_l - NBPSY_l/4) );	/* was .35*(sidetot/tot)/5.0*10 */
			ms_ratio_l = Min( ms_ratio_l, 0.5 );

			sidetot = 1;// tot=0;
			for( sb = NBPSY_s/4; sb < NBPSY_s; sb++ ){
				for( sblock = 0; sblock < 3; sblock++ ){
					x1 = Min( RW.thm[0].s[sb][sblock], RW.thm[1].s[sb][sblock] );
					x2 = Max( RW.thm[0].s[sb][sblock], RW.thm[1].s[sb][sblock] );
					if( x2 >= x1 * 1000 ){
						x2 = 1000;
					}else{
						x2 /= x1;
					}
					sidetot *= x2;
				}
			}
			sidetot = log10( sidetot ); /* db */
			ms_ratio_s = sidetot * ( 0.7 / (3 * (NBPSY_s - NBPSY_s/4)) ); /* was .35*(sidetot/tot)/5.0*10 */
			ms_ratio_s = Min( ms_ratio_s, 0.5 );
		}

		/* force both channels to use the same block type */
		/* this is necessary if the frame is to be encoded in ms_stereo.  */
		/* But even without ms_stereo, FhG  does this */
		if( !uselongblock[0] || !uselongblock[1] ){
			uselongblock[0] = 0;
			uselongblock[1] = 0;
		}
	} /* if( RO.mode == JOINT_STEREO ) */

	/* update the blocktype of the previous granule, since it depends on what
	 * happend in this granule */

	for( chn = 0; chn < RO.channels_out; chn++ ){
		if( uselongblock[chn] ){
			int blocktype;
			/* no attack : use long blocks */
			assert( RW.blocktype_old[chn] != START_TYPE );
			blocktype = ( RW.blocktype_old[chn] == SHORT_TYPE ) ? STOP_TYPE : NORM_TYPE;
			blocktype_d[chn] = RW.blocktype_old[chn];  /* value returned to calling program */
			RW.blocktype_old[chn] = blocktype;    /* save for next call to l3psy_anal */
		}else{
			/* attack : use short blocks */
			if( RW.blocktype_old[chn] == NORM_TYPE ){
				RW.blocktype_old[chn] = START_TYPE;
			}else
			if( RW.blocktype_old[chn] == STOP_TYPE ){
				RW.blocktype_old[chn] = SHORT_TYPE ;
			}
			blocktype_d[chn] = RW.blocktype_old[chn];  /* value returned to calling program */
			RW.blocktype_old[chn] = SHORT_TYPE;    /* save for next call to l3psy_anal */
		}
	}
	*ms_ratio = ( blocktype_d[0] == 2 ) ? RW.ms_ratio_s_old : RW.ms_ratio_l_old;

	RW.ms_ratio_s_old = ms_ratio_s;
	RW.ms_ratio_l_old = ms_ratio_l;

	/* we dont know the block type of this frame yet - assume long */
	*ms_ratio_next = ms_ratio_l;
} /* L3psycho_anal */

static void inner_psy_general( gogo_thread_data *tl, int uselongblock[2], int numchn, int gr_out )
{
	int chn, j, b, k, i, sb, sblock;
	for( chn = 0; chn < numchn; chn++ ){
		/* convolution   */
		float eb[CBANDS];
		float cb[CBANDS];
		float thr[CBANDS];
  
		/* fft and energy calculation   */
		float *wsamp_l;
		float (*wsamp_s)[BLKSIZE_s];

    /* FFT data for mid and side channel is derived from L & R */
	    if( chn == 2 ){
//			ms_convert_long (tl->wsamp_L[gr_out]);
//			ms_convert_short(tl->wsamp_S[gr_out]);
			ms_convert(tl->wsamp_L[gr_out][0], tl->wsamp_L[gr_out][1], 1024);
			ms_convert(tl->wsamp_S[gr_out][0][0], tl->wsamp_S[gr_out][1][0],  768);
		}

		/**********************************************************************
		 *  compute energies
		 **********************************************************************/

		wsamp_l = tl->wsamp_L[gr_out][chn & 1];
		wsamp_s = tl->wsamp_S[gr_out][chn & 1];

		tl->energy[0] = wsamp_l[0] * wsamp_l[0];

		RW.tot_ener[chn] = tl->energy[0]; /* sum total energy at nearly no extra cost */
    
		for( j = 1; j <= BLKSIZE/2; j++ ){
			float re, im;
			re = wsamp_l[j];
			im = wsamp_l[BLKSIZE - j];
			tl->energy[j] = (re * re + im * im) * 0.5F;
			if( j > 10 ) RW.tot_ener[chn] += tl->energy[j];
		}

		for( b = 0; b < 3; b++ ){
			tl->energy_s[b][0] = wsamp_s[b][0] * wsamp_s[b][0];
			for( j = 1; j <= BLKSIZE_s/2; j++ ){
				float re, im;
				re = wsamp_s[b][j];
				im = wsamp_s[b][BLKSIZE_s - j];
				tl->energy_s[b][j] = (re * re + im * im) * 0.5F;
			}
		}

		/**********************************************************************
		 *    compute unpredicatability of first six spectral lines            * 
		 **********************************************************************/
		for( j = 0; j < cw_lower_index; j++ ){
			/* calculate unpredictability measure cw */
			Abr_t *sn, *s1, s2;
			float numre, numim, den;

			s2 = RW.sav[1][j][chn];
			RW.sav[1][j][chn] = RW.sav[0][j][chn];
			s1 = &RW.sav[1][j][chn];
			RW.sav[0][j][chn].a = wsamp_l[j];
			RW.sav[0][j][chn].b = j==0 ? wsamp_l[0] : wsamp_l[BLKSIZE-j];  
			RW.sav[0][j][chn].r = sqrt(tl->energy[j]);
			sn = &RW.sav[0][j][chn];

		/* square (x1,y1) */
			if( s1->r ){
				numre = ( s1->a * s1->b );
				numim = (s1->a * s1->a- s1->b * s1->b)*(FLOAT)0.5;
				den = s1->r*s1->r;
			}else{
				numre = 1;
				numim = 0;
				den = 1;
			}
		/* multiply by (x2,-y2) */
			if( s2.r ){
				float tmp1, tmp2;
				tmp2 = (numim+numre)*(s2.a+s2.b)*(FLOAT)0.5;
				tmp1 = -s2.a*numre+tmp2;
				numre = -s2.b*numim+tmp2;
				numim = tmp1;
				den *= s2.r;
			}

			{ /* r-prime factor */
				float tmp = (2*s1->r-s2.r)/den;
				numre *= tmp;
				numim *= tmp;
			}
			den = sn->r + fabs(2*s1->r-s2.r);
			if( den ){
				numre = (sn->a+sn->b)*(FLOAT)0.5-numre;
				numim = (sn->a-sn->b)*(FLOAT)0.5-numim;
				den = sqrt(numre*numre+numim*numim)/den;
			}
			RW.cw[j] = den;
		} /* j */

		/**********************************************************************
		 *     compute unpredicatibility of next 200 spectral lines            *
		 **********************************************************************/ 
		for( j = cw_lower_index; j < RO.cw_upper_index; j += 4 ){
		/* calculate unpredictability measure cw */
			FLOAT rn, r1, r2;
			FLOAT numre, numim, den;
	
			k = (j+2) / 4; 
			/* square (x1,y1) */
			r1 = tl->energy_s[0][k];
			if( r1 ){
				float a1, b1;
				a1 = wsamp_s[0][k]; 
				b1 = wsamp_s[0][BLKSIZE_s-k]; /* k is never 0 */
				numre = (a1*b1);
				numim = (a1*a1-b1*b1)*(FLOAT)0.5;
				den = r1;
				r1 = sqrt(r1);
			}else{
				numre = 1;
				numim = 0;
				den = 1;
			}

			/* multiply by (x2,-y2) */
			r2 = tl->energy_s[2][k];
			if( r2 ){
				float a2, b2, tmp1, tmp2;
				a2 = wsamp_s[2][k]; 
				b2 = wsamp_s[2][BLKSIZE_s-k];
				tmp2 = (numim+numre)*(a2+b2)*(FLOAT)0.5;
				tmp1 = -a2*numre+tmp2;
				numre = -b2*numim+tmp2;
				numim = tmp1;
				r2 = sqrt(r2);
				den *= r2;
			}

			{ /* r-prime factor */
				float tmp = (2*r1-r2)/den;
				numre *= tmp;
				numim *= tmp;
			}
			rn = sqrt(tl->energy_s[1][k]);
			den = rn + fabs(2*r1-r2);
			if( den ){
				float an, bn;
				an = wsamp_s[1][k]; 
				bn = wsamp_s[1][BLKSIZE_s-k];
				numre = (an+bn)*(FLOAT)0.5-numre;
				numim = (an-bn)*(FLOAT)0.5-numim;
				den = sqrt(numre*numre+numim*numim)/den;
			}
			RW.cw[j] = RW.cw[j+1] = RW.cw[j+2] = RW.cw[j+3] = den;
		} /* j */
    
    /**********************************************************************
     *    Calculate the energy and the unpredictability in the threshold   *
     *    calculation partitions                                           *
     **********************************************************************/

		b = j = 0;
											/* この条件を消せそうな気が */
		while( j < RO.cw_upper_index && b < RO.npart_l_orig ){
			double ebb, cbb;
			ebb = cbb = 0;
			if( !RO.numlines_l[b] ) break;
			for( i = 0; i < RO.numlines_l[b]; i++ ){
				ebb += tl->energy[j];
				cbb += tl->energy[j] * RW.cw[j];
				j++;
			}
			eb[b] = ebb;
			cb[b] = cbb;
			b++;
		}

		for( ; b < RO.npart_l_orig; b++ ){
			double ebb;
			assert(RO.numlines_l[b]);
			ebb = 0;
			for( i = 0; i < RO.numlines_l[b]; i++ ){
			    ebb += tl->energy[j++];
			}
			eb[b] = ebb;
			cb[b] = ebb * 0.4;
		}

		/**********************************************************************
		 *      convolve the partitioned energy and unpredictability           *
		 *      with the spreading function, s3_l[b][k]                        *
		 ******************************************************************** */

		RW.pe[chn] = 0;		/*  calculate percetual entropy */
		convolute_energy( (RW.blocktype_old[ chn & 1] == SHORT_TYPE), eb, cb, thr, RW.nb_12[chn] );

		/* vector log 化のためloop分離(asm化後に上にくっつける予定) */
		for( b = 0;b < RO.npart_l; b++ ){
			float thrpe;
			thrpe = Max(thr[b],RW.ATH.cb[b]);
			if( thrpe < eb[b] ){	/* 90%以上 */
				RW.pe[chn] -= RO.numlines_l[b] * log(thrpe / eb[b]);
			}
		}

    /*************************************************************** 
     * determine the block type (window type) based on L & R channels
     * 
     ***************************************************************/
		{  /* compute PE for all 4 channels */
			float mn, mx, ma, mb, mc;
			ma = mb = mc = 0;
			for( j = HBLKSIZE_s/2; j < HBLKSIZE_s; j++ ){
				ma += tl->energy_s[0][j];
				mb += tl->energy_s[1][j];
				mc += tl->energy_s[2][j];
			}
			mn = Min(ma,mb);
			mn = Min(mn,mc);
			mx = Max(ma,mb);
			mx = Max(mx,mc);

			/* bit allocation is based on pe.  */
			if( mx > mn ){
				float tmp = 400*log(mx/(1e-12+mn));
				if( tmp > RW.pe[chn] ) RW.pe[chn] = tmp;
			}

			/* block type is based just on L or R channel */      
			if( chn < 2 ){
				uselongblock[chn] = 1;
				/* tuned for t1.wav.  doesnt effect most other samples */
				if( RW.pe[chn] > 3000 ) uselongblock[chn] = 0;
				if( mx > 30*mn ){
					/* big surge of energy - always use short blocks */
					uselongblock[chn] = 0;
				}else if( mx > 10*mn && RW.pe[chn] > 1000 ){
					/* medium surge, medium pe - use short blocks */
					uselongblock[chn] = 0;
				}
				/* disable short blocks */
			}
		}

    /*************************************************************** 
     * compute masking thresholds for both short and long blocks
     ***************************************************************/
    /* longblock threshold calculation (part 2) */
		for( sb = 0; sb < NBPSY_l; sb++ ){
			double enn, thmm;
			enn  = 0.5 * (  eb[RO.bu_l[sb]] +  eb[RO.bo_l[sb]] );
			thmm = 0.5 * ( thr[RO.bu_l[sb]] + thr[RO.bo_l[sb]] );

			for( b = RO.bu_l[sb]+1; b < RO.bo_l[sb]; b++ ){
				enn  += eb[b];
				thmm += thr[b];
			}
			RW.en [chn].l[sb] = enn;
			RW.thm[chn].l[sb] = thmm;
		}

    /* threshold calculation for short blocks */
		for( sblock = 0; sblock < 3; sblock++ ){
			j = 0;
			for( b = 0; b < RO.npart_s_orig; b++ ){
				FLOAT ecb = tl->energy_s[sblock][j++];
				for( i = 1 ; i < RO.numlines_s[b]; i++ ){
					ecb += tl->energy_s[sblock][j++];
				}
				eb[b] = ecb;
			}

			for ( b = 0; b < RO.npart_s; b++ ){
				FLOAT8 ecb = 0;
				for( k = RO.s3ind_s[b][0]; k <= RO.s3ind_s[b][1]; k++ ){
					ecb += RO.s3_s[b][k] * eb[k];
				}
				ecb *= RO.SNR_s[b];
				thr[b] = Max (1e-6, ecb);
			}

			for( sb = 0; sb < NBPSY_s; sb++ ){
				double enn, thmm;
				enn  = 0.5 * ( eb[RO.bu_s[sb]] +  eb[RO.bo_s[sb]] );
				thmm = 0.5 * (thr[RO.bu_s[sb]] + thr[RO.bo_s[sb]] );
				for( b = RO.bu_s[sb]+1; b < RO.bo_s[sb]; b++ ){
					enn  += eb[b];
					thmm += thr[b];
				}
				RW.en [chn].s[sb][sblock] = enn;
				RW.thm[chn].s[sb][sblock] = thmm;
			}
		}
	} /* end loop over chn */
} /* inner_psy_general */

/* 
 *   The spreading function.  Values returned in units of energy
 */
FLOAT8 s3_func(FLOAT8 bark) {
    
    FLOAT8 tempx,x,tempy,temp;
    tempx = bark;
    if (tempx>=0) tempx *= 3;
    else tempx *=1.5; 
    
    if(tempx>=0.5 && tempx<=2.5)
	{
	    temp = tempx - 0.5;
	    x = 8.0 * (temp*temp - 2.0 * temp);
	}
    else x = 0.0;
    tempx += 0.474;
    tempy = 15.811389 + 7.5*tempx - 17.5*sqrt(1.0+tempx*tempx);
    
    if (tempy <= -60.0) return  0.0;

    tempx = exp( (x + tempy)*LN_TO_LOG10 ); 

    /* Normalization.  The spreading function should be normalized so that:
         +inf
           /
           |  s3 [ bark ]  d(bark)   =  1
           /
         -inf
    */
    tempx /= .6609193;
    return tempx;
    
}

static int
L3para_read(FLOAT8 sfreq, int *numlines_l,int *numlines_s, 
	FLOAT8 *minval,
	FLOAT8 s3_l[CBANDS][CBANDS], FLOAT8 s3_s[CBANDS][CBANDS],
	FLOAT8 *SNR, 
	int *bu_l, int *bo_l, /*FLOAT8 *w1_l, FLOAT8 *w2_l, */
	int *bu_s, int *bo_s, /*FLOAT8 *w1_s, FLOAT8 *w2_s,*/
	int *npart_l_orig,int *npart_l,int *npart_s_orig,int *npart_s)
{
  FLOAT8 bval_l[CBANDS], bval_s[CBANDS];
  FLOAT8 bval_l_width[CBANDS], bval_s_width[CBANDS];
/*  int   cbmax=0; */
  int  i,j;
/*  int freq_scale=1; */
  int partition[HBLKSIZE]; 



  /* compute numlines, the number of spectral lines in each partition band */
  /* each partition band should be about DELBARK wide. */
  j=0;
  for(i=0;i<CBANDS;i++)
    {
      FLOAT8 ji, bark1,bark2;
      int k,j2;

      j2 = j;
      j2 = Min(j2,BLKSIZE/2);
      
      do {
	/* find smallest j2 >= j so that  (bark - bark_l[i-1]) < DELBARK */
	ji = j;
	bark1 = freq2bark(sfreq*ji/BLKSIZE);
	
	++j2;
	ji = j2;
	bark2  = freq2bark(sfreq*ji/BLKSIZE);
      } while ((bark2 - bark1) < DELBARK  && j2<=BLKSIZE/2);

      for (k=j; k<j2; ++k)
	partition[k]=i;
      numlines_l[i]=(j2-j);
      j = j2;
      if (j > BLKSIZE/2) break;
    }
  *npart_l_orig = i+1;
  assert(*npart_l_orig <= CBANDS);

  /* compute which partition bands are in which scalefactor bands */
  { int i1,i2,sfb,start,end;
    FLOAT8 freq1,freq2;
    for ( sfb = 0; sfb < SBMAX_l; sfb++ ) {
      start = RO.scalefac_band.l[ sfb ];
      end   = RO.scalefac_band.l[ sfb+1 ];
      freq1 = sfreq*(start-.5)/(2*576);
      freq2 = sfreq*(end-1+.5)/(2*576);
	  /* 次のstartは前回のendなのでfreq1と1つ前のfreq2は同じ */
	  /* したがってbo_l[sfb] = bu_l[sfb+1]である */
      i1 = floor(.5 + BLKSIZE*freq1/sfreq);
      if (i1<0) i1=0;
      i2 = floor(.5 + BLKSIZE*freq2/sfreq);
      if (i2>BLKSIZE/2) i2=BLKSIZE/2;

      //      RO.printf("longblock:  old: (%i,%i)  new: (%i,%i) %i %i \n",bu_l[sfb],bo_l[sfb],partition[i1],partition[i2],i1,i2);

//      w1_l[sfb]=.5;
//      w2_l[sfb]=.5;
	  assert( i1 < HBLKSIZE );
	  assert( i2 < HBLKSIZE );
      bu_l[sfb]=partition[i1];
      bo_l[sfb]=partition[i2];

    }
  }


  /* compute bark value and ATH of each critical band */
  j = 0;
  for ( i = 0; i < *npart_l_orig; i++ ) {
      int     k;
      FLOAT8  bark1,bark2;
      /* FLOAT8 mval,freq; */

      // Calculating the medium bark scaled frequency of the spectral lines
      // from j ... j + numlines[i]-1  (=spectral lines in parition band i)

      k         = numlines_l[i] - 1;
      bark1 = freq2bark(sfreq*(j+0)/BLKSIZE);
      bark2 = freq2bark(sfreq*(j+k)/BLKSIZE);
      bval_l[i] = .5*(bark1+bark2);

      bark1 = freq2bark(sfreq*(j+0-.5)/BLKSIZE);
      bark2 = freq2bark(sfreq*(j+k+.5)/BLKSIZE);
      bval_l_width[i] = bark2-bark1;

      RW.ATH.cb [i] = 1.e37; // preinit for minimum search
      for (k=0; k < numlines_l[i]; k++, j++) {
	FLOAT8  freq = sfreq*j/(1000.0*BLKSIZE);
	FLOAT8  level;
	assert( freq <= 24 );              // or only '<'
	//	freq = Min(.1,freq);       // ATH below 100 Hz constant, not further climbing
	level  = ATHformula (freq*1000) - 20;   // scale to FFT units; returned value is in dB
	level  = pow ( 10., 0.1*level );   // convert from dB -> energy
	level *= numlines_l [i];
	if ( level < RW.ATH.cb [i] )
	    RW.ATH.cb [i] = level;
      }


    }

  /* MINVAL.  For low freq, the strength of the masking is limited by minval
   * this is an ISO MPEG1 thing, dont know if it is really needed */
  for(i=0;i<*npart_l_orig;i++){
    double x = (-20+bval_l[i]*20.0/10.0);
    if (bval_l[i]>10) x = 0;
    minval[i]=pow(10.0,x/10);
  }







  /************************************************************************/
  /* SHORT BLOCKS */
  /************************************************************************/

  /* compute numlines */
  j=0;
  for(i=0;i<CBANDS;i++)
    {
      FLOAT8 ji, bark1,bark2;
      int k,j2;

      j2 = j;
      j2 = Min(j2,BLKSIZE_s/2);
      
      do {
	/* find smallest j2 >= j so that  (bark - bark_s[i-1]) < DELBARK */
	ji = j;
	bark1  = freq2bark(sfreq*ji/BLKSIZE_s);
	
	++j2;
	ji = j2;
	bark2  = freq2bark(sfreq*ji/BLKSIZE_s);

      } while ((bark2 - bark1) < DELBARK  && j2<=BLKSIZE_s/2);

      for (k=j; k<j2; ++k)
	partition[k]=i;
      numlines_s[i]=(j2-j);
      j = j2;
      if (j > BLKSIZE_s/2) break;
    }
  *npart_s_orig = i+1;
  assert(*npart_s_orig <= CBANDS);

  /* compute which partition bands are in which scalefactor bands */
  { int i1,i2,sfb,start,end;
    FLOAT8 freq1,freq2;
    for ( sfb = 0; sfb < SBMAX_s; sfb++ ) {
      start = RO.scalefac_band.s[ sfb ];
      end   = RO.scalefac_band.s[ sfb+1 ];
      freq1 = sfreq*(start-.5)/(2*192);
      freq2 = sfreq*(end-1+.5)/(2*192);
		     
      i1 = floor(.5 + BLKSIZE_s*freq1/sfreq);
      if (i1<0) i1=0;
      i2 = floor(.5 + BLKSIZE_s*freq2/sfreq);
      if (i2>BLKSIZE_s/2) i2=BLKSIZE_s/2;

      //RO.printf("shortblock: old: (%i,%i)  new: (%i,%i) %i %i \n",bu_s[sfb],bo_s[sfb], partition[i1],partition[i2],i1,i2);

//      w1_s[sfb]=.5;
//      w2_s[sfb]=.5;
      bu_s[sfb]=partition[i1];
      bo_s[sfb]=partition[i2];

    }
  }





  /* compute bark values of each critical band */
  j = 0;
  for(i=0;i<*npart_s_orig;i++)
    {
      int     k;
      FLOAT8  bark1,bark2,snr;
      k    = numlines_s[i] - 1;

      bark1 = freq2bark (sfreq*(j+0)/BLKSIZE_s);
      bark2 = freq2bark (sfreq*(j+k)/BLKSIZE_s); 
      bval_s[i] = .5*(bark1+bark2);

      bark1 = freq2bark (sfreq*(j+0-.5)/BLKSIZE_s);
      bark2 = freq2bark (sfreq*(j+k+.5)/BLKSIZE_s); 
      bval_s_width[i] = bark2-bark1;
      j        += k+1;
      
      /* SNR formula */
      if (bval_s[i]<13)
          snr=-8.25;
      else 
	  snr  = -4.5 * (bval_s[i]-13)/(24.0-13.0)  + 
	      -8.25*(bval_s[i]-24)/(13.0-24.0);

      SNR[i]=pow(10.0,snr/10.0);
    }






  /************************************************************************
   * Now compute the spreading function, s[j][i], the value of the spread-*
   * ing function, centered at band j, for band i, store for later use    *
   ************************************************************************/
  /* i.e.: sum over j to spread into signal barkval=i  
     NOTE: i and j are used opposite as in the ISO docs */
  for(i=0;i<*npart_l_orig;i++)    {
      for(j=0;j<*npart_l_orig;j++) 	{
  	  s3_l[i][j]=s3_func(bval_l[i]-bval_l[j])*bval_l_width[j];
      }
  }
  for(i=0;i<*npart_s_orig;i++)     {
      for(j=0;j<*npart_s_orig;j++) 	{
  	  s3_s[i][j]=s3_func(bval_s[i]-bval_s[j])*bval_s_width[j];
      }
  }
  



  /* compute: */
  /* npart_l_orig   = number of partition bands before convolution */
  /* npart_l  = number of partition bands after convolution */
  
  *npart_l=bo_l[NBPSY_l-1]+1;
  *npart_s=bo_s[NBPSY_s-1]+1;
  
  assert(*npart_l <= *npart_l_orig);
  assert(*npart_s <= *npart_s_orig);


    /* setup stereo demasking thresholds */
    /* formula reverse enginerred from plot in paper */
    for ( i = 0; i < NBPSY_s; i++ ) {
      FLOAT8 arg,mld;
      arg = freq2bark(sfreq*RO.scalefac_band.s[i]/(2*192));
      arg = (Min(arg, 15.5)/15.5);

      mld = 1.25*(1-cos(PI*arg))-2.5;
      RO.mld_s[i] = pow(10.0,mld);
    }
    for ( i = 0; i < NBPSY_l; i++ ) {
      FLOAT8 arg,mld;
      arg = freq2bark(sfreq*RO.scalefac_band.l[i]/(2*576));
      arg = (Min(arg, 15.5)/15.5);

      mld = 1.25*(1-cos(PI*arg))-2.5;
      RO.mld_l[i] = pow(10.0,mld);
    }

#define temporalmask_sustain_sec 0.01

    /* setup temporal masking */
    RO.decay = exp(-1.0*LOG10/(temporalmask_sustain_sec*sfreq/192.0));

    return 0;
}

int
psymodel_init(void)
{
    int i,j,sb,samplerate, b;
    FLOAT cwlimit;

    samplerate = RO.out_samplerate;
    RW.blocktype_old[0]=STOP_TYPE;
    RW.blocktype_old[1]=STOP_TYPE;
    RW.blocktype_old[0]=SHORT_TYPE;
    RW.blocktype_old[1]=SHORT_TYPE;

    for (i=0; i<4; ++i) {
      for (j=0; j<CBANDS * 2; ++j) {
	RW.nb_12[i][j]=1e20;
      }
      for ( sb = 0; sb < NBPSY_l; sb++ ) {
	RW.en[i].l[sb] = 1e20;
	RW.thm[i].l[sb] = 1e20;
      }
      for (j=0; j<3; ++j) {
	for ( sb = 0; sb < NBPSY_s; sb++ ) {
	  RW.en[i].s[sb][j] = 1e20;
	  RW.thm[i].s[sb][j] = 1e20;
	}
      }
    }

    cwlimit=(FLOAT)8871.7;
    RO.cw_upper_index = cwlimit*1024.0/RO.out_samplerate;
    RO.cw_upper_index=Min(HBLKSIZE-4,RO.cw_upper_index);      /* j+3 < HBLKSIZE-1 */
    RO.cw_upper_index=Max(6,RO.cw_upper_index);

    for ( j = 0; j < HBLKSIZE*4; j++ )
      RW.cw[j] = 0.4f;

    i = L3para_read((FLOAT8) samplerate,RO.numlines_l,RO.numlines_s,
          RO.minval,RO.s3_l,RO.s3_s,RO.SNR_s,RO.bu_l,
          RO.bo_l,/*RO.w1_l,RO.w2_l,*/ RO.bu_s,RO.bo_s,
          /*RO.w1_s,RO.w2_s,*/ &RO.npart_l_orig,&RO.npart_l,
          &RO.npart_s_orig,&RO.npart_s );
    if (i!=0) return -1;

    /* npart_l_orig   = number of partition bands before convolution */
    /* npart_l  = number of partition bands after convolution */
    
    for (i=0; i<RO.npart_l; i++) {
      for (j = 0; j < RO.npart_l_orig; j++) {
	if (RO.s3_l[i][j] != 0.0)
	  break;
      }
      RO.s3ind[i][0] = j;
      
      for (j = RO.npart_l_orig - 1; j > 0; j--) {
	if (RO.s3_l[i][j] != 0.0)
	  break;
      }
      RO.s3ind[i][1] = j;
    }


    for (i=0; i<RO.npart_s; i++) {
      for (j = 0; j < RO.npart_s_orig; j++) {
	if (RO.s3_s[i][j] != 0.0)
	  break;
      }
      RO.s3ind_s[i][0] = j;
      
      for (j = RO.npart_s_orig - 1; j > 0; j--) {
	if (RO.s3_s[i][j] != 0.0)
	  break;
      }
      RO.s3ind_s[i][1] = j;
    }

	for( i = 0; i <= tonalityTblNum; i++ ){	/* 右端も念のため計算 */
		double x, factor;
		factor = (LIMIT_U - LIMIT_L) / tonalityTblNum;
		x = i * factor + LIMIT_L;
		tonalityTbl[i*2+0] = COEFF1 * pow( x, COEFF2 );	/* 0次 */
		tonalityTbl[i*2+1] = COEFF1 * COEFF2 * factor * pow( x, COEFF2 - 1 ); /* 1次 */
	}

	b = j = 0;
	while( j < RO.cw_upper_index && b < RO.npart_l_orig ){
		int i;
		if( !RO.numlines_l[b] ) break;
		for( i = 0; i < RO.numlines_l[b]; i++ ){
			j++;
		}
		b++;
	}
	RO.npart_l_pre_max = b;
	return 0;
}
