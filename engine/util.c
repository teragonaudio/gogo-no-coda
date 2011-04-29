/*
 *	lame utility library source file
 *
 *	Copyright (c) 1999 Albert L Faber
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

#include <ctype.h>
#include <assert.h>
#include <stdarg.h>

#include "config.h"
#include "global.h"

#define PRECOMPUTE

#include "lame.h"
#include "util.h"

#if defined(__FreeBSD__) && !defined(__alpha__)
# include <machine/floatingpoint.h>
#endif

/***********************************************************************
*
*  Global Function Definitions
*
***********************************************************************/

/*
 * auto-adjust of ATH, useful for low volume
 * Gabriel Bouvigne 3 feb 2001
 *
 * modifies some values in RO.ATH
 */
void
adjust_ATH(FLOAT8 tot_ener[2][4])
{
        int     gr, channel;
        FLOAT8  max_val = 0, max_val_n, adj_lim_new;

        for ( gr = 0; gr < RO.mode_gr; ++gr ) {
                for ( channel = 0; channel < RO.channels_out; ++channel ) {
                        max_val = Max( max_val, tot_ener[gr][channel] );
                }
        }
        /* scale to 0..1, and then rescale to 0..32767 */
        max_val *= 32767/1e13;

        /*  adjust ATH depending on range of maximum value
         */

        /* continuous curves based on approximation to GB's original values */
          max_val_n = max_val / 32768;
                                /* For an increase in approximate loudness, */
                                /* set ATH adjust to adjust_limit immediately*/
                                /* after a delay of one frame. */
                                /* For a loudness decrease, reduce ATH adjust*/
                                /* towards adjust_limit gradually. */
          if( max_val_n > 0.25) { /* sqrt((1 - 0.01) / 15.84) from curve below*/
            if( RW.ATH.adjust >= 1.0) {
              RW.ATH.adjust = 1.0;
            } else {            /* preceding frame has lower ATH adjust; */
                                /* ascend only to the preceding adjust_limit */
                                /* in case there is leading low volume */
              if( RW.ATH.adjust < RW.ATH.adjust_limit) {
                RW.ATH.adjust = RW.ATH.adjust_limit;
              }
            }
            RW.ATH.adjust_limit = 1.0;
          } else {              /* adjustment curve (parabolic) */
            adj_lim_new = 15.84 * (max_val_n * max_val_n) + 0.01;
            if( RW.ATH.adjust >= adj_lim_new) { /* descend gradually */
              RW.ATH.adjust *= adj_lim_new * 0.075 + 0.925;
              if( RW.ATH.adjust < adj_lim_new) { /* stop descent */
                RW.ATH.adjust = adj_lim_new;
              }
            } else {            /* ascend */
              if( RW.ATH.adjust_limit >= adj_lim_new) {
                RW.ATH.adjust = adj_lim_new;
              } else {          /* preceding frame has lower ATH adjust; */
                                /* ascend only to the preceding adjust_limit */
                if( RW.ATH.adjust < RW.ATH.adjust_limit) {
                  RW.ATH.adjust = RW.ATH.adjust_limit;
                }
              }
            }
            RW.ATH.adjust_limit = adj_lim_new;
        }
}

FLOAT8 ATHformula_GBtweak(FLOAT8 f)
{
  FLOAT8 ath;
  f /= 1000;  // convert to khz
  f  = Max(0.01, f);
  f  = Min(18.0, f);

  /* from Painter & Spanias, 1997 */
  /* modified by Gabriel Bouvigne to better fit to the reality */
  ath =    3.640 * pow(f,-0.8)
         - 6.800 * exp(-0.6*pow(f-3.4,2.0))
         + 6.000 * exp(-0.15*pow(f-8.7,2.0))
         + 0.57* 0.001 * pow(f,4.0) //0.57 to maximize HF importance
         + 6; //std --athlower -6 for
  return ath;
}


/* 
 *  Klemm 1994 and 1997. Experimental data. Sorry, data looks a little bit
 *  dodderly. Data below 30 Hz is extrapolated from other material, above 18
 *  kHz the ATH is limited due to the original purpose (too much noise at
 *  ATH is not good even if it's theoretically inaudible).
 */

FLOAT8  ATHformula_Frank( FLOAT8 freq )
{
    /*
     * one value per 100 cent = 1
     * semitone = 1/4
     * third = 1/12
     * octave = 1/40 decade
     * rest is linear interpolated, values are currently in decibel rel. 20 ¥ªPa
     */
#ifdef _MSC_VER
	#pragma warning( disable : 4305 )	/* convert const double->float */
#endif
    static FLOAT tab [] = {
        /*    10.0 */  96.69, 96.69, 96.26, 95.12,
        /*    12.6 */  93.53, 91.13, 88.82, 86.76,
        /*    15.8 */  84.69, 82.43, 79.97, 77.48,
        /*    20.0 */  74.92, 72.39, 70.00, 67.62,
        /*    25.1 */  65.29, 63.02, 60.84, 59.00,
        /*    31.6 */  57.17, 55.34, 53.51, 51.67,
        /*    39.8 */  50.04, 48.12, 46.38, 44.66,
        /*    50.1 */  43.10, 41.73, 40.50, 39.22,
        /*    63.1 */  37.23, 35.77, 34.51, 32.81,
        /*    79.4 */  31.32, 30.36, 29.02, 27.60,
        /*   100.0 */  26.58, 25.91, 24.41, 23.01,
        /*   125.9 */  22.12, 21.25, 20.18, 19.00,
        /*   158.5 */  17.70, 16.82, 15.94, 15.12,
        /*   199.5 */  14.30, 13.41, 12.60, 11.98,
        /*   251.2 */  11.36, 10.57,  9.98,  9.43,
        /*   316.2 */   8.87,  8.46,  7.44,  7.12,
        /*   398.1 */   6.93,  6.68,  6.37,  6.06,
        /*   501.2 */   5.80,  5.55,  5.29,  5.02,
        /*   631.0 */   4.75,  4.48,  4.22,  3.98,
        /*   794.3 */   3.75,  3.51,  3.27,  3.22,
        /*  1000.0 */   3.12,  3.01,  2.91,  2.68,
        /*  1258.9 */   2.46,  2.15,  1.82,  1.46,
        /*  1584.9 */   1.07,  0.61,  0.13, -0.35,
        /*  1995.3 */  -0.96, -1.56, -1.79, -2.35,
        /*  2511.9 */  -2.95, -3.50, -4.01, -4.21,
        /*  3162.3 */  -4.46, -4.99, -5.32, -5.35,
        /*  3981.1 */  -5.13, -4.76, -4.31, -3.13,
        /*  5011.9 */  -1.79,  0.08,  2.03,  4.03,
        /*  6309.6 */   5.80,  7.36,  8.81, 10.22,
        /*  7943.3 */  11.54, 12.51, 13.48, 14.21,
        /* 10000.0 */  14.79, 13.99, 12.85, 11.93,
        /* 12589.3 */  12.87, 15.19, 19.14, 23.69,
        /* 15848.9 */  33.52, 48.65, 59.42, 61.77,
        /* 19952.6 */  63.85, 66.04, 68.33, 70.09,
        /* 25118.9 */  70.66, 71.27, 71.91, 72.60,
    };
    FLOAT8    freq_log;
    unsigned  index;
    
    if ( freq <    10. ) freq =    10.;
    if ( freq > 29853. ) freq = 29853.;
    
    freq_log = 40. * log10 (0.1 * freq);   /* 4 steps per third, starting at 10 Hz */
    index    = (unsigned) freq_log;
    assert ( index < sizeof(tab)/sizeof(*tab) );
    return tab [index] * (1 + index - freq_log) + tab [index+1] * (freq_log - index);
}

FLOAT8 ATHformula(FLOAT8 f)
{
	if (RO.VBR != vbr_rh) {
		return ATHformula_Frank(f);
	} else {
		return ATHformula_GBtweak(f);
	}
}

/* see for example "Zwicker: Psychoakustik, 1982; ISBN 3-540-11401-7 */
FLOAT8 freq2bark(FLOAT8 freq)
{
  /* input: freq in hz  output: barks */
    if (freq<0) freq=0;
    freq = freq * 0.001;
    return 13.0*atan(.76*freq) + 3.5*atan(freq*freq/(7.5*7.5));
}


/***********************************************************************
 * compute bitsperframe and mean_bits for a layer III frame 
 **********************************************************************/
void
CBR_getframebits(gogo_thread_data *tl, int *bitsPerFrame, int *mean_bits) 
{
	assert ( RO.brate <= 550 );

	/* main encoding routine toggles padding on and off */
	/* one Layer3 Slot consists of 8 bits */
	*bitsPerFrame = RO.bitsPerFrame + 8 * tl->padding;

	// sideinfo_len
	*mean_bits = (*bitsPerFrame - 8*RO.sideinfo_len) / RO.mode_gr;
}

void
VBRABR_getframebits(gogo_thread_data *tl, int *bitsPerFrame, int *mean_bits) 
{
  int  whole_SpF;  /* integral number of Slots per Frame without padding */
  int  bit_rate;
  
	if (tl->bitrate_index == 1) {
		*bitsPerFrame = RO.bitsPerFrame;
	}
	else {
    bit_rate = bitrate_table[RO.version][tl->bitrate_index];
  assert ( bit_rate <= 550 );
  
  whole_SpF = (RO.version+1)*72000*bit_rate / RO.out_samplerate;
		*bitsPerFrame = 8 * whole_SpF;
	}
  
  // sideinfo_len
  *mean_bits = (*bitsPerFrame - 8*RO.sideinfo_len) / RO.mode_gr;
}

int FindNearestBitrate(
int bRate,        /* legal rates from 32 to 448 */
int version)      /* MPEG-1 or MPEG-2 LSF */
{
    int  bitrate = bitrate_table[version][1];
    int  i;
  
    for ( i = 1; i <= 14; i++ )
        if ( abs(bitrate_table[version][i] - bRate) < abs(bitrate - bRate) )
            bitrate = bitrate_table [version] [i];
	    
    return bitrate;
}


/* map frequency to a valid MP3 sample frequency
 *
 * Robert.Hegemann@gmx.de 2000-07-01
 */
int map2MP3Frequency(int freq)
{
    if (freq <= 16000) return 16000;
    if (freq <= 22050) return 22050;
    if (freq <= 24000) return 24000;
    if (freq <= 32000) return 32000;
    if (freq <= 44100) return 44100;
    
    return 48000;
}

int BitrateIndex(
int bRate,        /* legal rates from 32 to 448 kbps */
int version)      /* MPEG-1 or MPEG-2/2.5 LSF */
{
    int  i;
#if		0
    for ( i = 0; i <= 14; i++)
        if ( bitrate_table [version] [i] == bRate )
            return i;
    return -1;
#else
	unsigned int dist = 0xFFFFFFFF;
	int				dist_i = 1;
    for ( i = 1; i <= 14; i++) {
        if ( (unsigned long)abs( bitrate_table [version] [i] - bRate ) <= dist ) {
            dist	=	abs( bitrate_table [version] [i] - bRate );
			dist_i	=	i;
		}
	}
	return	dist_i;
#endif
}

/* convert samp freq in Hz to index */

int SmpFrqIndex ( int sample_freq, unsigned char* const version )
{
    switch ( sample_freq ) {
    case 44100: *version = 1; return  0;
    case 48000: *version = 1; return  1;
    case 32000: *version = 1; return  2;
    case 22050: *version = 0; return  0;
    case 24000: *version = 0; return  1;
    case 16000: *version = 0; return  2;
//    case 11025: *version = 0; return  0;
//    case 12000: *version = 0; return  1;
//    case  8000: *version = 0; return  2;
    default:    *version = 0; return -1;
    }
}


/*****************************************************************************
*
*  End of bit_stream.c package
*
*****************************************************************************/

/*  caution: a[] will be resorted!!
 */
int select_kth_int(int a[], int N, int k)
{
    int i, j, l, r, v, w;
    
    l = 0;
    r = N-1;
    while (r > l) {
        v = a[r];
        i = l-1;
        j = r;
        for (;;) {
            while (a[++i] < v) /*empty*/;
            while (a[--j] > v) /*empty*/;
            if (i >= j) 
                break;
            /* swap i and j */
            w = a[i];
            a[i] = a[j];
            a[j] = w;
        }
        /* swap i and r */
        w = a[i];
        a[i] = a[r];
        a[r] = w;
        if (i >= k) 
            r = i-1;
        if (i <= k) 
            l = i+1;
    }
    return a[k];
}
