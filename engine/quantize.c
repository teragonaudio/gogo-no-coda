/*
 * MP3 quantization
 *
 * Copyright (c) 1999 Mark Taylor
 * Copyright (c) 2001,2002,2003 gogo-developer
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <math.h>
#include <assert.h>

#include "config.h"
#include "global.h"

#include "util.h"
#include "quantize.h"
#include "reservoir.h"
#include "quantize_pvt.h"
#include "lame.h"
#include "vfta.h"

/************************************************************************
 *
 *      init_outer_loop()
 *
 *  initializes cod_info and scalefac
 *
 ************************************************************************/
static void
init_outer_loop(gr_info *const cod_info, III_scalefac_t *const scalefac)
{
	int	blocktype = cod_info->block_type;

	memset(scalefac, 0, sizeof(III_scalefac_t));
	memset(cod_info, 0, sizeof(gr_info));
	if(blocktype != SHORT_TYPE){
        	cod_info->sfb_lmax        = SBPSY_l;
        	cod_info->sfb_smin        = SBPSY_s;
	}
	if(blocktype != NORM_TYPE){
		cod_info->window_switching_flag = 1;
	}
	cod_info->block_type          = blocktype;
	cod_info->global_gain         = 210;
	cod_info->sfb_partition_table = nr_of_sfb_block[0][0];
}

/************************************************************************
 *
 *      bin_search_StepSize()
 *
 *  author/date??
 *
 *  binary step size search
 *  used by outer_loop to get a quantizer step size to start with
 *
 ************************************************************************/

typedef enum {
    BINSEARCH_NONE,
    BINSEARCH_UP, 
    BINSEARCH_DOWN
} binsearchDirection_t;

static int
bin_search_StepSize(
	gr_info * const cod_info,
	const int desired_rate, 
	const int start, 
	float xrpow [576],
	float xrpow_max,
	int l3enc [576] ) 
{
	int nBits;
	int CurrentStep;
	int flag_GoneOver = 0;
	int StepSize	  = start;
	float	w;
	float *istepPtr;
	
	binsearchDirection_t Direction = BINSEARCH_NONE;
	assert(RW.CurrentStep);
	CurrentStep = RW.CurrentStep;
	
	for(;;) {
		cod_info->global_gain = StepSize;
		w = (IXMAX_VAL)/IPOW20(cod_info->global_gain);
		if(xrpow_max > w){
			nBits = LARGE_BITS;
		}else{
			istepPtr = &IPOW20(cod_info->global_gain);
			quantize_xrpow_ISO(xrpow, l3enc, istepPtr);
			nBits = count_bits_long(l3enc, cod_info);
		}
		
		if (CurrentStep == 1) break; /* nothing to adjust anymore */
		if (flag_GoneOver) CurrentStep /= 2;
		if (nBits > desired_rate) {  
			/* increase Quantize_StepSize */
			if (Direction == BINSEARCH_DOWN && !flag_GoneOver) {
				flag_GoneOver = 1;
				CurrentStep  /= 2; /* late adjust */
			}
			Direction = BINSEARCH_UP;
			StepSize += CurrentStep;
			if (StepSize > 255) break;
		}
		else if (nBits < desired_rate) {
			/* decrease Quantize_StepSize */
			if (Direction == BINSEARCH_UP && !flag_GoneOver) {
				flag_GoneOver = 1;
				CurrentStep  /= 2; /* late adjust */
			}
			Direction = BINSEARCH_DOWN;
			StepSize -= CurrentStep;
			if (StepSize < 0) break;
		}
		else break; /* nBits == desired_rate;; most unlikely to happen.*/
	}	/* For-ever, break is adjusted. */
	
	CurrentStep = start - StepSize;
	
	RW.CurrentStep = CurrentStep/4 != 0 ? 4 : 2;
	
	return nBits;
}

/*************************************************************************** 
 *
 *         inner_loop ()                                                     
 *
 *  The code selects the best global gain for a particular set of scalefacs 
 *
 ***************************************************************************/ 
static int 
inner_loop(gr_info * const cod_info, const int max_bits, float xrpow[576],
	float xrpow_max,
	int l3enc [576])
{
	int bits;
	float	w;
	float *istepPtr;
	
	assert(max_bits >= 0);
	
	/*  scalefactors may have changed, so count bits
	*/
	w = (IXMAX_VAL)/IPOW20(cod_info->global_gain);
	if (xrpow_max > w) {
		bits = LARGE_BITS;
	} else {
		istepPtr = &IPOW20( cod_info->global_gain );
		
		quantize_xrpow_ISO(xrpow, l3enc, istepPtr);
		bits = count_bits_long(l3enc, cod_info);
	}
	
	/*  increase quantizer stepsize until needed bits are below maximum
	*/
	while (bits > max_bits) {
		cod_info->global_gain++;
		w = (IXMAX_VAL)/IPOW20(cod_info->global_gain);
		if(xrpow_max > w){
			bits = LARGE_BITS;
		}else{
			istepPtr = &IPOW20( cod_info->global_gain );
			
			quantize_xrpow_ISO(xrpow, l3enc, istepPtr );
			
			bits = count_bits_long(l3enc, cod_info);
		}
	} 
	return bits;
}

/*************************************************************************
 *
 *      loop_break()                                               
 *
 *  Function: Returns zero if there is a scalefac which has not been
 *            amplified. Otherwise it returns one. 
 *
 *************************************************************************/

static int
loop_break(const gr_info *const cod_info, const III_scalefac_t *const scalefac)
{
	int i, sfb;

	for (sfb = 0; sfb < cod_info->sfb_lmax; sfb++){
		if (scalefac->l[sfb] == 0) return 0;
	}
	for (i = 0; i < 3; i++) {
		if (cod_info->subblock_gain[i]) continue;
		for (sfb = cod_info->sfb_smin; sfb < SBPSY_s; sfb++) {
			if (!scalefac->s[sfb][i]) return 0;
		}
	}
	return 1;
}

/*************************************************************************
 *
 *          amp_scalefac_bands() 
 *
 *  author/date??
 *        
 *  Amplify the scalefactor bands that violate the masking threshold.
 *  See ISO 11172-3 Section C.1.5.4.3.5
 * 
 *  distort[] = noise/masking
 *  distort[] > 1   ==> noise is not masked
 *  distort[] < 1   ==> noise is masked
 *  max_dist = maximum value of distort[]
 *  
 *  Three algorithms:
 *  noise_shaping_amp
 *        0             Amplify all bands with distort[]>1.
 *
 *        1             Amplify all bands with distort[] >= max_dist^(.5);
 *                     ( 50% in the db scale)
 *
 *        2             Amplify first band with distort[] >= max_dist;
 *                       
 *
 *  For algorithms 0 and 1, if max_dist < 1, then amplify all bands 
 *  with distort[] >= .95*max_dist.  This is to make sure we always
 *  amplify at least one band.  
 * 
 *
 *************************************************************************/
/*
 *	2001/11/01	2470clk@PIII
 *	by shigeo	2200clk@PIII
 */
static void 
amp_scalefac_bands( const gr_info  *const cod_info, III_scalefac_t *const scalefac,
    III_psy_xmin *distort, FLOAT8 xrpow[576], float *pxrpow_max
)
{
  int start, end, l,i,j,sfb;
  FLOAT8 ifqstep34, trigger;
	float xrpow_max = *pxrpow_max;
	int imax;

  if (cod_info->scalefac_scale == 0) {
    ifqstep34 = 1.29683955465100964055; /* 2**(.75*.5)*/
  } else {
    ifqstep34 = 1.68179283050742922612;  /* 2**(.75*1) */
  }

	/* compute maximum value of distort[]  */
	/* assume IEEE-754 */
	imax = 0;
	if (cod_info->block_type != SHORT_TYPE) {
		for (i = 0; i < SBPSY_l; i++) {
			int val, mask;
			assert(distort->l[i] >= 0);
			val = *(int *) &distort->l[i];
			imax -= val;
			mask = ~(imax >> 31);	/* mask = (imax < val) ? 0 : -1 */
			imax = (imax & mask) + val;	/* imax = (imax < val) ? val : imax; */
		}
	} else {
		for (i = 0; i < SBPSY_s; i++) {
			for (j = 0; j < 3; j++) {
				int val, mask;
				assert(distort->s[i][j] >= 0);
				val = *(int *) &distort->s[i][j];
				imax -= val;
				mask = ~(imax >> 31);	/* mask = (imax < val) ? 0 : -1 */
				imax = (imax & mask) + val;	/* imax = (imax < val) ? val : imax; */
			}
		}
	}
	trigger = *(float *) &imax;

	if (RO.noise_shaping_amp == 0) {
		/* ISO algorithm.  amplify all bands with distort>1 */
		trigger = (trigger > 1.0) ? 1.0 : trigger * 0.95;
	} else
	if (RO.noise_shaping_amp == 1) {
		/* amplify bands within 50% of max (on db scale) */
		trigger = (trigger > 1.0) ? sqrt(trigger) : trigger * 0.95;
	}

	if (cod_info->block_type != SHORT_TYPE) {
		for (sfb = 0; sfb < SBPSY_l; sfb++) {
			if (distort->l[sfb] >= trigger) {
				scalefac->l[sfb]++;
				start = RO.scalefac_band.l[sfb];
				end   = RO.scalefac_band.l[sfb+1];
				if (end > RO.ixend) end = RO.ixend;
				for (l = start; l < end; l++){
					xrpow[l] *= ifqstep34;
					if(xrpow[l] > xrpow_max) xrpow_max = xrpow[l];
				}
				if (RO.noise_shaping_amp==2) goto done;
			}
		}
	} else {
		for (j=0, sfb = 0; sfb < SBPSY_s; sfb++) {
			start = RO.scalefac_band.s[sfb];
			end   = RO.scalefac_band.s[sfb+1];
			for (i = 0; i < 3; i++) {
				int j2 = j;
				if (distort->s[sfb][i] >= trigger) {
					scalefac->s[sfb][i]++;
					for (l = start; l < end && j2 < RO.ixend; l++) {
						xrpow[j2] *= ifqstep34;
						if(xrpow[j2] > xrpow_max) xrpow_max = xrpow[j2];
						j2++;
					}
					if (RO.noise_shaping_amp==2) goto done;
				}
				j += end-start;
			}
		}
	}
done:
	*pxrpow_max = xrpow_max;
}

/********************************************************************
 *
 *      balance_noise()
 *
 *  Takehiro Tominaga /date??
 *  Robert Hegemann 2000-09-06: made a function of it
 *
 *  amplifies scalefactor bands, 
 *   - if all are already amplified returns 0
 *   - if some bands are amplified too much:
 *      * try to increase scalefac_scale
 *      * if already scalefac_scale was set
 *          try on short blocks to increase subblock gain
 *
 ********************************************************************/
INLINE static int 
balance_noise(gr_info *const cod_info, III_scalefac_t *const scalefac, 
    III_psy_xmin *distort, FLOAT8  xrpow[576], float *pxrpow_max
)
{
    int status;
//clkbegin();
    amp_scalefac_bands(cod_info, scalefac, distort, xrpow, pxrpow_max);
//clkend();
    
    /* check to make sure we have not amplified too much 
     * loop_break returns 0 if there is an unamplified scalefac
     * scale_bitcount returns 0 if no scalefactors are too large
     */
    
    status = loop_break (cod_info, scalefac);
    
    if (status) 
        return 0; /* all bands amplified */
    
    /* not all scalefactors have been amplified.  so these 
     * scalefacs are possibly valid.  encode them: 
     */
    if (IS_MPEG1)
        status = scale_bitcount (scalefac, cod_info);
    else 
        status = scale_bitcount_lsf (scalefac, cod_info);

    return !status;
}

/************************************************************************
 *
 *  outer_loop ()                                                       
 *
 *  Function: The outer iteration loop controls the masking conditions  
 *  of all scalefactorbands. It computes the best scalefac and          
 *  global gain. This module calls the inner iteration loop             
 * 
 *  mt 5/99 completely rewritten to allow for bit reservoir control,   
 *  mid/side channels with L/R or mid/side masking thresholds, 
 *  and chooses best quantization instead of last quantization when 
 *  no distortion free quantization can be found.  
 *  
 *  added VBR support mt 5/99
 *
 *  some code shuffle rh 9/00
 ************************************************************************/
extern void calc_noise_short( CALC_NOISE_TYPE );

static int 
outer_loop(
	gogo_thread_data *tl,
	gr_info * const cod_info,
    const FLOAT8                 xr[576],   /* magnitudes of spectral values */
    const III_psy_xmin   * const l3_xmin,   /* allowed distortion of the scalefactor */
          III_scalefac_t * const scalefac,  /* scalefactors */
          FLOAT8                 xrpow[576], /* coloured magnitudes of spectral values */
	float	*pxrpow_max,
          int                    l3enc[576], /* vector of quantized values ix(0..575) */
    const int                    ch, 
    const int                    targ_bits )  /* maximum allowed bits */
{
    III_scalefac_t save_scalefac;
    gr_info save_cod_info;
    FLOAT8 save_xrpow[576];
	int	save_l3enc[576];
	float	save_xrpow_max = 0;
	/* 用期間が重ならない psywork を共用する */
	III_psy_xmin *distort = (III_psy_xmin*)tl->psywork;
    calc_noise_result noise_info;
    calc_noise_result best_noise_info;
    int iteration = 0;
    int bits_found;
    int huff_bits;
    int better;
    int over;
	
    int notdone;
    int copy = 0;
    int age = 0;
	int max_index;
	const int ixend = RO.ixend;
	void (*calc_noise)( CALC_NOISE_TYPE );
	assert(sizeof(III_psy_xmin) <= sizeof(tl->psywork));
	
    noise_info.over_count = 100;
    noise_info.tot_noise  = 0;
    noise_info.over_noise = 0;
    
    best_noise_info.over_count = 100;
	
    bits_found = bin_search_StepSize (cod_info, targ_bits, 
		RW.OldValue[ch], xrpow, *pxrpow_max, l3enc);
    RW.OldValue[ch] = cod_info->global_gain;
	
	if( cod_info->block_type == SHORT_TYPE ){
		calc_noise = calc_noise_short;
		max_index = RW.sfb21_extra ? SBMAX_s : SBPSY_s;
	}else{
		calc_noise = calc_noise_long;
		max_index = RW.sfb21_extra ? SBMAX_l : SBPSY_l;	/* 22 or 21 */
	}
	
    /* BEGIN MAIN LOOP */
	cod_info->global_gain++;
    do {
        iteration ++;
		
        /* inner_loop starts with the initial quantization step computed above
		* and slowly increases until the bits < huff_bits.
		* Thus it is important not to start with too large of an inital
		* quantization step.  Too small is ok, but inner_loop will take longer 
		*/
        huff_bits = targ_bits - cod_info->part2_length;
        if (huff_bits < 0) {
       //     assert(iteration != 1);
            /*  scale factors too large, not enough bits. 
			*  use previous quantizaton */
            break;
        }
        /*  if this is the first iteration, 
		*  see if we can reuse the quantization computed in 
		*  bin_search_StepSize above */
		
		if (iteration == 1 && bits_found <= huff_bits) {
			cod_info->global_gain--;
			cod_info->part2_3_length = bits_found;
		} else {
			/* ここでglobal_gainが1増えるかもしれない */
			/* l3enc <-- xrpow */
			cod_info->part2_3_length = inner_loop(cod_info, huff_bits, xrpow, *pxrpow_max, l3enc);
		}
		
        /* compute the distortion in this quantization */
        if (RO.noise_shaping) {
            /* coefficients and thresholds both l/r (or both mid/side) */
			/* distort <-- 量子化ノイズ(|xr-l3enc|/l3_xmin) */
            calc_noise( max_index, xr, l3enc, cod_info, l3_xmin, 
				scalefac, distort, &noise_info);
			over = noise_info.over_count;
        } else {
            /* fast mode, no noise shaping, we are ready */
            best_noise_info = noise_info;
            over = 0;
            copy = 0;
            break;
        }
		
		
		/* distortがより小さければそれを選択 */
        /* check if this quantization is better
		* than our saved quantization */
		better = (iteration == 1) /* the first iteration is always better */
			|| (noise_info.over_count  < best_noise_info.over_count)
			|| (noise_info.over_count == best_noise_info.over_count &&
			noise_info.over_noise  < best_noise_info.over_noise)
			|| (noise_info.over_count == best_noise_info.over_count &&
			noise_info.over_noise == best_noise_info.over_noise &&
			noise_info.tot_noise   < best_noise_info.tot_noise); 
        
        /* save data so we can restore this quantization later */    
        if (better) {
            copy = 0;
            best_noise_info = noise_info;
            age = 0;
        }else{
            age ++;
		}
		
        /******************************************************************/
        /* stopping criterion */
        /******************************************************************/
        /* if no bands with distortion and -X0, we are done */
        if (0==RO.noise_shaping_stop && (over == 0 || best_noise_info.over_count == 0) )
            break;
			/* Otherwise, allow up to 3 unsuccesful tries in serial, then stop 
			* if our best quantization so far had no distorted bands. This
			* gives us more possibilities for different quant_compare modes.
			* Much more than 3 makes not a big difference, it is only slower.
		*/
        if (age > 3 && best_noise_info.over_count == 0) 
            break;
		
			/* Check if the last scalefactor band is distorted.
			* in VBR mode we can't get rid of the distortion, so quit now
			* and VBR mode will try again with more bits.  
			* (makes a 10% speed increase, the files I tested were
			* binary identical, 2000/05/20 Robert.Hegemann@gmx.de)
			* distort[] > 1 means noise > allowed noise
		*/
        if (RW.sfb21_extra) {
            if (cod_info->block_type == SHORT_TYPE) {
                if (distort->s[SBMAX_s-1][0] > 1 ||
                    distort->s[SBMAX_s-1][1] > 1 ||
                    distort->s[SBMAX_s-1][2] > 1) break;
            } else {
                if (distort->l[SBMAX_l-1] > 1) break;
            }
        }
		
        /* save data so we can restore this quantization later */    
        if (better) {
            copy = 1;
            save_scalefac = *scalefac;
            save_cod_info = *cod_info;
			memcpy(save_l3enc, l3enc, ixend * sizeof(int));
            if (RO.VBR == vbr_rh) {
                /* store for later reuse */
                memcpy(save_xrpow, xrpow, ixend * sizeof(float));
				save_xrpow_max = *pxrpow_max;
            }
        }
		
		/* distortの最大値をもとめてその付近のscalefacを1増やして再量子化の用意 */
        notdone = balance_noise (cod_info, scalefac, distort, xrpow, pxrpow_max);
	} while (notdone); /* main iteration loop, breaks adjusted */
   /*  finish up */
   if (copy) {
	   *cod_info = save_cod_info;
	   *scalefac = save_scalefac;
	   memcpy(l3enc, save_l3enc, ixend * sizeof(int));
	   if (RO.VBR == vbr_rh ){
		   /* restore for reuse on next try */
		   memcpy(xrpow, save_xrpow, ixend * sizeof(float));
		   *pxrpow_max = save_xrpow_max;
	   }
   }
   cod_info->part2_3_length += cod_info->part2_length;
   
   assert (cod_info->global_gain < 256);
   
   return best_noise_info.over_count;
}

/************************************************************************
 *
 *      iteration_finish()                                                    
 *
 *  Robert Hegemann 2000-09-06
 *
 *  update reservoir status after FINAL quantization/bitrate 
 *
 *  rh 2000-09-06: it will not work with CBR due to the bitstream formatter
 *            you will get "Error: MAX_HEADER_BUF too small in bitstream.c"
 *
 ************************************************************************/
void 
iteration_finish (
    int             l3_enc  [2][2][576],
    III_side_info_t *l3_side,
    III_scalefac_t  scalefac[2][2],
    const int       mean_bits )
{
    int gr, ch;
    
    for (gr = 0; gr < RO.mode_gr; gr++) {
        for (ch = 0; ch < RO.channels_out; ch++) {
            gr_info *cod_info = &l3_side->tt[gr][ch];

            /*  try some better scalefac storage */
            best_scalefac_store (gr, ch, l3_enc, l3_side, scalefac);
            
            /*  best huffman_divide may save some bits too */
            if (RO.use_best_huffman) 
                best_huffman_divide (cod_info, l3_enc[gr][ch]);
            
            /*  update reservoir status after FINAL quantization/bitrate */
            ResvAdjust(cod_info, mean_bits);
        } /* for ch */
    }    /* for gr */
}

/*********************************************************************
 *
 *      VBR_encode_granule()
 *
 *  2000-09-04 Robert Hegemann
 *
 *********************************************************************/
 
static void
VBR_encode_granule (
		  gogo_thread_data *     tl,
          gr_info        * const cod_info,
          FLOAT8                 xr[576],     /* magnitudes of spectral values */
    const III_psy_xmin   * const l3_xmin,     /* allowed distortion of the scalefactor */
          III_scalefac_t * const scalefac,    /* scalefactors */
          FLOAT8                 xrpow[576],  /* coloured magnitudes of spectral values */
	float	xrpow_max,
          int                    l3_enc[576], /* vector of quantized values ix(0..575) */
    const int                    ch, 
          int                    min_bits, 
          int                    max_bits )
{
    gr_info         bst_cod_info;
    III_scalefac_t  bst_scalefac;
    FLOAT8          bst_xrpow [576]; 
	float			bst_xrpow_max;
    int             bst_l3_enc[576];
    int Max_bits  = max_bits;
    int real_bits = max_bits+1;
    int this_bits = (max_bits+min_bits)/2;
    int dbits, over, found = 0;
    int sfb21_extra = RW.sfb21_extra;
	const int ixend = RO.ixend;

    assert(Max_bits <= MAX_BITS);

    /*  search within round about 40 bits of optimal
     */
    do {
        assert(this_bits >= min_bits);
        assert(this_bits <= max_bits);
        assert(min_bits <= max_bits);

        if (this_bits > Max_bits-42) 
            RW.sfb21_extra = 0;
        else
            RW.sfb21_extra = sfb21_extra;

        over = outer_loop (tl, cod_info, xr, l3_xmin, scalefac, xrpow,
				&xrpow_max,
				l3_enc, ch, this_bits );

        /*  is quantization as good as we are looking for ?
         *  in this case: is no scalefactor band distorted?
         */
        if (over <= 0) {
            found = 1;
            /*  now we know it can be done with "real_bits"
             *  and maybe we can skip some iterations
             */
            real_bits = cod_info->part2_3_length;

            /*  store best quantization so far
             */
            bst_cod_info = *cod_info;
            bst_scalefac = *scalefac;
			bst_xrpow_max = xrpow_max;
            memcpy(bst_xrpow, xrpow, ixend * sizeof(xrpow));
            memcpy(bst_l3_enc, l3_enc, ixend * sizeof(l3_enc[0]));

            /*  try with fewer bits
             */
            max_bits  = real_bits-32;
            dbits     = max_bits-min_bits;
            this_bits = (max_bits+min_bits)/2;
        } 
        else {
            /*  try with more bits
             */
            min_bits  = this_bits+32;
            dbits     = max_bits-min_bits;
            this_bits = (max_bits+min_bits)/2;
            
            if (found) {
                found = 2;
                /*  start again with best quantization so far
                 */
                *cod_info = bst_cod_info;
                *scalefac = bst_scalefac;
				xrpow_max = bst_xrpow_max;
                memcpy(xrpow, bst_xrpow, ixend * sizeof(xrpow[0]));
            }
        }
    } while (dbits>12);

    RW.sfb21_extra = sfb21_extra;

    /*  found=0 => nothing found, use last one
     *  found=1 => we just found the best and left the loop
     *  found=2 => we restored a good one and have now l3_enc to restore too
     */
    if (found==2) {
        memcpy(l3_enc, bst_l3_enc, ixend * sizeof(l3_enc[0]));
    }
    assert(cod_info->part2_3_length <= Max_bits);
}

/************************************************************************
 *
 *      VBR_get_framebits()   
 *
 *  Robert Hegemann 2000-09-05
 *
 *  calculates
 *  * how many bits are available for analog silent granules
 *  * how many bits to use for the lowest allowed bitrate
 *  * how many bits each bitrate would provide
 *
 ************************************************************************/
static void 
VBR_get_framebits (gogo_thread_data *tl,
	int     * const analog_mean_bits,
    int     * const min_mean_bits,
    int             frameBits[15] )
{
    int bitsPerFrame, mean_bits, i;
    
    /*  always use at least this many bits per granule per channel 
     *  unless we detect analog silence, see below 
     */
    tl->bitrate_index = RO.VBR_min_bitrate;
    VBRABR_getframebits(tl, &bitsPerFrame, &mean_bits);
    *min_mean_bits = mean_bits / RO.channels_out;

    /*  bits for analog silence */
    tl->bitrate_index = 1;
    VBRABR_getframebits(tl, &bitsPerFrame, &mean_bits);
    *analog_mean_bits = mean_bits / RO.channels_out;

    for (i = 1; i <= RO.VBR_max_bitrate; i++) {
		tl->bitrate_index = i;
        VBRABR_getframebits(tl, &bitsPerFrame, &mean_bits);
        frameBits[i] = ResvFrameBegin(mean_bits, bitsPerFrame);
    }
}

/************************************************************************
 *
 *      calc_min_bits()   
 *
 *  Robert Hegemann 2000-09-04
 *
 *  determine minimal bit skeleton
 *
 ************************************************************************/
INLINE static int
calc_min_bits ( const gr_info * const cod_info,
    const int             pe,
    const FLOAT8          ms_ener_ratio, 
    const int             bands,    
    const int             mch_bits,
    const int             analog_mean_bits,
    const int             min_mean_bits,
    const int             analog_silence,
    const int             ch,
	int	mode_ext )
{
    int min_bits, min_pe_bits;

    /*  base amount of minimum bits */
    min_bits = Max (125, min_mean_bits);

    if (mode_ext == MPG_MD_MS_LR && ch == 1)  
        min_bits = Max (min_bits, mch_bits/5);

    /*  bit skeleton based on PE */
    if (cod_info->block_type == SHORT_TYPE) 
        /*  if LAME switches to short blocks then pe is
         *  >= 1000 on medium surge
         *  >= 3000 on big surge
         */
        min_pe_bits = (pe-350) * bands/39;
    else 
        min_pe_bits = (pe-350) * bands/22;
    
    if (mode_ext == MPG_MD_MS_LR && ch == 1) {
        /*  side channel will use a lower bit skeleton based on PE */ 
        FLOAT8 fac  = .33 * (.5 - ms_ener_ratio) / .5;
        min_pe_bits = (int)(min_pe_bits * ((1-fac)/(1+fac)));
    }
    min_pe_bits = Min (min_pe_bits, (1820 * RO.out_samplerate / 44100));

    /*  determine final minimum bits */
    if (analog_silence )
        min_bits = analog_mean_bits;
    else 
        min_bits = Max (min_bits, min_pe_bits);
    
    return min_bits;
}

/************************************************************************
 *
 *      calc_max_bits()   
 *
 *  Robert Hegemann 2000-09-05
 *
 *  determine maximal bit skeleton
 *
 ************************************************************************/
INLINE static int 
calc_max_bits(const int frameBits[15], const int min_bits)
{
    int max_bits;
    
    max_bits  = frameBits[RO.VBR_max_bitrate];
    max_bits /= RO.channels_out * RO.mode_gr;
    max_bits  = Min (1200 + max_bits, MAX_BITS - 195 * (RO.channels_out - 1));
    max_bits  = Max (max_bits, min_bits);
    
    return max_bits;
}

/*********************************************************************
 *
 *      VBR_prepare()
 *
 *  2000-09-04 Robert Hegemann
 *
 *  * converts LR to MS coding when necessary 
 *  * calculates allowed/adjusted quantization noise amounts
 *  * detects analog silent frames
 *
 *  some remarks:
 *  - lower masking depending on Quality setting
 *  - quality control together with adjusted ATH MDCT scaling
 *    on lower quality setting allocate more noise from
 *    ATH masking, and on higher quality setting allocate
 *    less noise from ATH masking.
 *  - experiments show that going more than 2dB over GPSYCHO's
 *    limits ends up in very annoying artefacts
 *
 *********************************************************************/

/* RH: this one needs to be overhauled sometime */
 
const FLOAT8 dbQ[10]={ -2.0, -1.0, -0.66, -0.33, 0.0, 0.33, 0.66, 1.0, 1.33, 1.66};
    
static int 
VBR_prepare (gogo_thread_data *tl,
          FLOAT8          pe            [2][2],
          FLOAT8          ms_ener_ratio [2], 
          int             frameBits     [16],
          int            *analog_mean_bits,
          int            *min_mean_bits,
          int             min_bits      [2][2],
          int             max_bits      [2][2])
{
    int     gr, ch;
    int     analog_silence = 1;
    int     bpf, avg, mxb, bits = 0;
  
    tl->bitrate_index = RO.VBR_max_bitrate;
    VBRABR_getframebits (tl, &bpf, &avg);
    bpf = ResvFrameBegin ( avg, bpf );
	avg = bpf / RO.mode_gr; 
	assert(0 < avg);

    VBR_get_framebits (tl, analog_mean_bits, min_mean_bits, frameBits);
    
    for (gr = 0; gr < RO.mode_gr; gr++) {
        mxb = on_pe (pe, tl->l3_side.tt, max_bits[gr], avg, gr);
        if (tl->mode_ext == MPG_MD_MS_LR) {
            reduce_side (max_bits[gr], ms_ener_ratio[gr], avg, mxb);
        }
        for (ch = 0; ch < RO.channels_out; ++ch) {
            gr_info *cod_info = &tl->l3_side.tt[gr][ch];
      
            if (tl->ath_over[gr][ch]) 
                analog_silence = 0;

            min_bits[gr][ch] = calc_min_bits (cod_info, (int)pe[gr][ch],
                                      ms_ener_ratio[gr], tl->ath_over[gr][ch],
                                      0, *analog_mean_bits, 
                                      *min_mean_bits, analog_silence, ch, tl->mode_ext);
      
            bits += max_bits[gr][ch];
        }
    }
    for (gr = 0; gr < RO.mode_gr; gr++) {
        for (ch = 0; ch < RO.channels_out; ch++) {            
            if (bits > frameBits[RO.VBR_max_bitrate]) {
                max_bits[gr][ch] *= frameBits[RO.VBR_max_bitrate];
                max_bits[gr][ch] /= bits;
            }
            if (min_bits[gr][ch] > max_bits[gr][ch]) 
                min_bits[gr][ch] = max_bits[gr][ch];
            
        } /* for ch */
    }  /* for gr */
    
    *min_mean_bits = Max(*min_mean_bits, 126);

    return analog_silence;
}
 
/************************************************************************
 *
 *      VBR_iteration_loop()   
 *
 *  tries to find out how many bits are needed for each granule and channel
 *  to get an acceptable quantization. An appropriate bitrate will then be
 *  choosed for quantization.  rh 8/99                          
 *
 *  Robert Hegemann 2000-09-06 rewrite
 *
 ************************************************************************/
void
VBR_iteration_loop (gogo_thread_data *tl,
    FLOAT8             pe           [2][2],
    FLOAT8             ms_ener_ratio[2], 
	int *mean_bits)
{
    int       frameBits[15];
    int       bitsPerFrame;
    int       save_bits[2][2];
    int       used_bits, used_bits2;
    int       bits;
    int       min_bits[2][2], max_bits[2][2];
    int       analog_mean_bits, min_mean_bits;
    int       ch, gr, analog_silence;
	const int ixend = RO.ixend;
    gr_info             *cod_info;

    analog_silence = VBR_prepare (tl, pe, ms_ener_ratio,
                                  frameBits, &analog_mean_bits,
                                  &min_mean_bits, min_bits, max_bits);

    /*---------------------------------*/
    do {  
    
	    /*  quantize granules with lowest possible number of bits
		 */
    
	    used_bits = 0;
		used_bits2 = 0;
   
		for (gr = 0; gr < RO.mode_gr; gr++) {
			for (ch = 0; ch < RO.channels_out; ch++) {
	            cod_info = &tl->l3_side.tt[gr][ch];
      
				/*  init_outer_loop sets up cod_info and scalefac */
	            init_outer_loop(cod_info, &tl->scalefac[gr][ch]);

				if (tl->xrpow_sum[gr][ch] <= (FLOAT8)1E-20) {
					/*  xr contains no energy 
					 *  l3_enc, our encoding data, will be quantized to zero
					 */
					memset(tl->l3_enc[gr][ch], 0, ixend * sizeof(int));
					save_bits[gr][ch] = 0;
					continue; /* with next channel */
				}
				memcpy(tl->work_xrpow, tl->xrpow[gr][ch], ixend * sizeof(int));
      
				VBR_encode_granule (tl, cod_info, tl->xr[gr][ch], &tl->l3_xmin[gr][ch],
										&tl->scalefac[gr][ch], tl->work_xrpow,
					tl->xrpow_max[gr][ch],
					tl->l3_enc[gr][ch], ch, min_bits[gr][ch], max_bits[gr][ch] );

				used_bits += cod_info->part2_3_length;
				save_bits[gr][ch] = Min(MAX_BITS, cod_info->part2_3_length);
				used_bits2 += Min(MAX_BITS, cod_info->part2_3_length);
			} /* for ch */
		}    /* for gr */

		/*  find lowest bitrate able to hold used bits
		 */
		if (analog_silence) 
			tl->bitrate_index = 1;
		else
			tl->bitrate_index = RO.VBR_min_bitrate;
     
		for( ; tl->bitrate_index < RO.VBR_max_bitrate; tl->bitrate_index++) {
			if (used_bits <= frameBits[tl->bitrate_index]) break; 
		}

		VBRABR_getframebits (tl, &bitsPerFrame, /* & */mean_bits);
	    bits = ResvFrameBegin(*mean_bits, bitsPerFrame);
    
		if (used_bits > bits) {
			for (gr = 0; gr < RO.mode_gr; gr++) {
				for (ch = 0; ch < RO.channels_out; ch++) {
					int sfb;
					cod_info = &tl->l3_side.tt[gr][ch];
	                if (cod_info->block_type == SHORT_TYPE) {
						for (sfb = 0; sfb < SBMAX_s; sfb++) {
							tl->l3_xmin[gr][ch].s[sfb][0] *= 1.+.029*sfb*sfb/SBMAX_s/SBMAX_s;
							tl->l3_xmin[gr][ch].s[sfb][1] *= 1.+.029*sfb*sfb/SBMAX_s/SBMAX_s;
							tl->l3_xmin[gr][ch].s[sfb][2] *= 1.+.029*sfb*sfb/SBMAX_s/SBMAX_s;
						}
					}
					else {
						for (sfb = 0; sfb < SBMAX_l; sfb++) 
							tl->l3_xmin[gr][ch].l[sfb] *= 1.+.029*sfb*sfb/SBMAX_l/SBMAX_l;
					}
					max_bits[gr][ch] = Max(min_bits[gr][ch], 0.9*max_bits[gr][ch]);
				}
			}
		}
    } while (used_bits > bits);
}

/********************************************************************
 *
 *  ABR_calc_target_bits()
 *
 *  calculates target bits for ABR encoding
 *
 *  mt 2000/05/31
 *
 ********************************************************************/

static void 
ABR_calc_target_bits (gogo_thread_data *tl,
    FLOAT8               pe            [2][2],
    FLOAT8               ms_ener_ratio [2],
    int                  targ_bits     [2][2],
    int                 *analog_silence_bits,
    int                 *max_frame_bits)
{
    FLOAT8 res_factor;
    int gr, ch, totbits, mean_bits, bitsPerFrame;
	float	compression_ratio;

    
    tl->bitrate_index = RO.VBR_max_bitrate;
    VBRABR_getframebits(tl, &bitsPerFrame, &mean_bits);
    *max_frame_bits = ResvFrameBegin(mean_bits, bitsPerFrame);

    tl->bitrate_index = 1;
    VBRABR_getframebits(tl, &bitsPerFrame, &mean_bits);
    *analog_silence_bits = mean_bits / RO.channels_out;

    mean_bits  = RO.brate * RO.framesize * 1000;
    mean_bits /= RO.out_samplerate;
    mean_bits -= RO.sideinfo_len*8;
    mean_bits /= RO.mode_gr;

	compression_ratio = RO.out_samplerate * 16 * RO.channels_out / (1.e3 * RO.brate) ;
    res_factor = .90 + .10 * (11.0 - compression_ratio) / (11.0 - 5.5);
    if (res_factor <  .90)
        res_factor =  .90; 
    if (res_factor > 1.00) 
        res_factor = 1.00;

    for (gr = 0; gr < RO.mode_gr; gr++) {
        for (ch = 0; ch < RO.channels_out; ch++) {
            targ_bits[gr][ch] = res_factor * (mean_bits / RO.channels_out);
            
            if (pe[gr][ch] > 700) {
                int add_bits = (pe[gr][ch] - 700) / 1.4;
  
                gr_info *cod_info = &tl->l3_side.tt[gr][ch];
                targ_bits[gr][ch] = res_factor * (mean_bits / RO.channels_out);
 
                /* short blocks use a little extra, no matter what the pe */
                if (cod_info->block_type == SHORT_TYPE) {
                    if (add_bits < mean_bits/4) 
                        add_bits = mean_bits/4; 
                }
                /* at most increase bits by 1.5*average */
                if (add_bits > mean_bits*3/4)
                    add_bits = mean_bits*3/4;
                else
                if (add_bits < 0) 
                    add_bits = 0;

                targ_bits[gr][ch] += add_bits;
            }
        }/* for ch */
    }   /* for gr */
    
	if (tl->mode_ext == MPG_MD_MS_LR){
		for (gr = 0; gr < RO.mode_gr; gr++) {
			reduce_side (targ_bits[gr], ms_ener_ratio[gr], mean_bits, MAX_BITS);
		}
	}

    /*  sum target bits */
    totbits=0;
    for (gr = 0; gr < RO.mode_gr; gr++) {
        for (ch = 0; ch < RO.channels_out; ch++) {
            if (targ_bits[gr][ch] > MAX_BITS) 
                targ_bits[gr][ch] = MAX_BITS;
            totbits += targ_bits[gr][ch];
        }
    }

    /*  repartion target bits if needed */
    if (totbits > *max_frame_bits) {
        for(gr = 0; gr < RO.mode_gr; gr++) {
            for(ch = 0; ch < RO.channels_out; ch++) {
                targ_bits[gr][ch] *= *max_frame_bits; 
                targ_bits[gr][ch] /= totbits; 
            }
        }
    }
}

/********************************************************************
 *
 *  ABR_iteration_loop()
 *
 *  encode a frame with a disired average bitrate
 *
 *  mt 2000/05/31
 *
 ********************************************************************/
void 
ABR_iteration_loop(gogo_thread_data *tl,
    FLOAT8             pe           [2][2],
    FLOAT8             ms_ener_ratio[2], 
int *mean_bits)
{
    int       targ_bits[2][2];
    int       bitsPerFrame, /* mean_bits, */ totbits, max_frame_bits;
    int       ch, gr;
    int       analog_silence_bits;
	const int ixend = RO.ixend;
    gr_info             *cod_info;

    ABR_calc_target_bits (tl, pe, ms_ener_ratio, targ_bits, 
                      &analog_silence_bits, &max_frame_bits);
    
    /*  encode granules */
    totbits=0;
    for (gr = 0; gr < RO.mode_gr; gr++) {
        for (ch = 0; ch < RO.channels_out; ch++) {
            cod_info = &tl->l3_side.tt[gr][ch];

            /*  cod_info and scalefac  get initialized in init_outer_loop */
            init_outer_loop(cod_info, &tl->scalefac[gr][ch]);
            if (tl->xrpow_sum[gr][ch] <= (FLOAT8)1E-20) {
                /*  xr contains no energy 
                 *  l3_enc, our encoding data, will be quantized to zero
                 */
                memset(tl->l3_enc[gr][ch], 0, ixend * sizeof(int));
            } else {
                if (0 == tl->ath_over[gr][ch]) /* analog silence */
                    targ_bits[gr][ch] = analog_silence_bits;

                outer_loop (tl, cod_info, tl->xr[gr][ch], &tl->l3_xmin[gr][ch], &tl->scalefac[gr][ch], tl->xrpow[gr][ch],
				&tl->xrpow_max[gr][ch],
				tl->l3_enc[gr][ch], ch, targ_bits[gr][ch]);
            }

            totbits += cod_info->part2_3_length;
        } /* ch */
    }  /* gr */
  
    /*  find a bitrate which can handle totbits */
    for (tl->bitrate_index =  RO.VBR_min_bitrate ;
         tl->bitrate_index <= RO.VBR_max_bitrate;
         tl->bitrate_index++    ) {
        VBRABR_getframebits(tl, &bitsPerFrame, mean_bits);
        max_frame_bits = ResvFrameBegin(*mean_bits, bitsPerFrame);
        if (totbits <= max_frame_bits) break; 
    }
    assert (tl->bitrate_index <= RO.VBR_max_bitrate);
}

/************************************************************************
 *
 *      iteration_loop()                                                    
 *
 *  author/date??
 *
 *  encodes one frame of MP3 data with constant bitrate
 *
 ************************************************************************/
void 
CBR_iteration_loop(gogo_thread_data *tl,
    FLOAT8             pe           [2][2],
    FLOAT8             ms_ener_ratio[2],  
	int *mean_bits)
{
    int    targ_bits[2];
    int    bitsPerFrame;
    int    max_bits;
    int    gr, ch;
    gr_info             *cod_info;

    CBR_getframebits(tl, &bitsPerFrame, mean_bits);
    ResvFrameBegin(*mean_bits, bitsPerFrame );

    /* quantize! */
    for (gr = 0; gr < RO.mode_gr; gr++) {

        /*  calculate needed bits */
        max_bits = on_pe (pe, tl->l3_side.tt, targ_bits, *mean_bits, gr);
        
        if (tl->mode_ext == MPG_MD_MS_LR) {
            reduce_side (targ_bits, ms_ener_ratio[gr], *mean_bits, max_bits);
        }
        
        for (ch=0 ; ch < RO.channels_out ; ch ++) {
            cod_info = &tl->l3_side.tt[gr][ch]; 

            /*  init_outer_loop sets up cod_info and scalefac */
            init_outer_loop(cod_info, &tl->scalefac[gr][ch]);
            if (tl->xrpow_sum[gr][ch] <= (FLOAT8)1E-20) {
                /*  xr contains no energy, l3_enc will be quantized to zero */
                memset(tl->l3_enc[gr][ch], 0, RO.ixend * sizeof(int));
            } else {
                /*  xr contains energy we will have to encode 
                 *  calculate the masking abilities
                 *  find some good quantization in outer_loop 
                 */
                outer_loop (tl, cod_info, tl->xr[gr][ch], &tl->l3_xmin[gr][ch], 
                            &tl->scalefac[gr][ch], tl->xrpow[gr][ch],
				&tl->xrpow_max[gr][ch],
				tl->l3_enc[gr][ch], ch, targ_bits[ch]);
            }
            assert (cod_info->part2_3_length <= MAX_BITS);

            /*  try some better scalefac storage */
            best_scalefac_store (gr, ch, tl->l3_enc, &tl->l3_side, tl->scalefac);
            
            /*  best huffman_divide may save some bits too */
            if (RO.use_best_huffman) 
                best_huffman_divide (cod_info, tl->l3_enc[gr][ch]);
            
            /*  update reservoir status after FINAL quantization/bitrate */
#undef  NORES_TEST
#ifndef NORES_TEST
            ResvAdjust(cod_info, *mean_bits);
#endif      
            /*  set the sign of l3_enc from the sign of xr */
        } /* for ch */
    }    /* for gr */
    
#ifdef NORES_TEST
    /* replace ResvAdjust above with this code if you do not want
       the second granule to use bits saved by the first granule.
       Requires using the --nores.  This is useful for testing only */
    for (gr = 0; gr < RO.mode_gr; gr++) {
        for (ch =  0; ch < RO.channels_out; ch++) {
            cod_info = &l3_side->gr[gr].ch[ch].tt;
            ResvAdjust(cod_info, *mean_bits);
        }
    }
#endif
}
