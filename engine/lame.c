/* -*- mode: C; mode: fold -*- */
/*
 *	GOGO / LAME MP3 encoding engine
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

#include <assert.h>

#include "config.h"
#include "global.h"

#include "lame.h"
#include "util.h"
#include "bitstream.h"
#include "tables.h"
#include "quantize_pvt.h"
#include "vbrtag.h"
#include "psymodel.h"
#include "gogo.h"

#if defined(__FreeBSD__) && !defined(__alpha__)
#include <floatingpoint.h>
#endif

static void
lame_init_params_ppflt_lowpass(FLOAT8 amp_lowpass[32], FLOAT lowpass1,
	   FLOAT lowpass2, int *lowpass_band, int *minband, int *maxband)
{
	int	 band;
	FLOAT8  freq;

	for (band = 0; band <= 31; band++) {
		freq = band / 31.0;
		amp_lowpass[band] = 1;
		/* this band and above will be zeroed: */
		if (freq >= lowpass2) {
			*lowpass_band = Min(*lowpass_band, band);
			amp_lowpass[band] = 0;
		}
		if (lowpass1 < freq && freq < lowpass2) {
			*minband = Min(*minband, band);
			*maxband = Max(*maxband, band);
			amp_lowpass[band] = cos((PI / 2) *
									(lowpass1 - freq) / (lowpass2 - lowpass1));
		}
		/*
		 * RO.printf("lowpass band=%i  amp=%f \n",
		 *	  band, RO.amp_lowpass[band]);
		 */
	}
}

static void
lame_init_params_ppflt(void)
{
  /***************************************************************/
	/* compute info needed for polyphase filter (filter type==0, default) */
  /***************************************************************/

	int	 band, maxband, minband;
	FLOAT8  freq;

	if (BE.lowpass1 > 0) {
		minband = 999;
		maxband = -1;
		lame_init_params_ppflt_lowpass(RO.amp_lowpass,
			   BE.lowpass1, BE.lowpass2, &RO.lowpass_band, &minband, &maxband);
		/* compute the *actual* transition band implemented by
		 * the polyphase filter */
		if (minband == 999) {
			BE.lowpass1 = (RO.lowpass_band - .75) / 31.0;
		}
		else {
			BE.lowpass1 = (minband - .75) / 31.0;
		}
		BE.lowpass2 = RO.lowpass_band / 31.0;

		RO.lowpass_start_band = minband;
		RO.lowpass_end_band = maxband;

		/* as the lowpass may have changed above
		 * calculate the amplification here again
		 */
		for (band = minband; band <= maxband; band++) {
			freq = band / 31.0;
			RO.amp_lowpass[band] =
				cos((PI / 2) * (BE.lowpass1 - freq) /
					(BE.lowpass2 - BE.lowpass1));
		}
	}
	else {
		RO.lowpass_start_band = 0;
		RO.lowpass_end_band = -1; /* do not to run into for-loops */
	}

	/* make sure highpass filter is within 90% of what the effective
	 * highpass frequency will be */
	if (BE.highpass2 > 0) {
		if (BE.highpass2 < .9 * (.75 / 31.0)) {
			BE.highpass1 = 0;
			BE.highpass2 = 0;
			RO.printf("Warning: highpass filter disabled.  "
				 "highpass frequency too small\n");
		}
	}

	if (BE.highpass2 > 0) {
		minband = 999;
		maxband = -1;
		for (band = 0; band <= 31; band++) {
			freq = band / 31.0;
			RO.amp_highpass[band] = 1;
			/* this band and below will be zereod */
			if (freq <= BE.highpass1) {
				RO.highpass_band = Max(RO.highpass_band, band);
				RO.amp_highpass[band] = 0;
			}
			if (BE.highpass1 < freq && freq < BE.highpass2) {
				minband = Min(minband, band);
				maxband = Max(maxband, band);
				RO.amp_highpass[band] =
					cos((PI / 2) *
						(BE.highpass2 - freq) /
						(BE.highpass2 - BE.highpass1));
			}
			/*
			   RO.printf("highpass band=%i  amp=%f \n",
			   band, RO.amp_highpass[band]);
			 */
		}
		/* compute the *actual* transition band implemented by
		 * the polyphase filter */
		BE.highpass1 = RO.highpass_band / 31.0;
		if (maxband == -1) {
			BE.highpass2 = (RO.highpass_band + .75) / 31.0;
		}
		else {
			BE.highpass2 = (maxband + .75) / 31.0;
		}

		RO.highpass_start_band = minband;
		RO.highpass_end_band = maxband;

		/* as the highpass may have changed above
		 * calculate the amplification here again
		 */
		for (band = minband; band <= maxband; band++) {
			freq = band / 31.0;
			RO.amp_highpass[band] =
				cos((PI / 2) * (BE.highpass2 - freq) /
					(BE.highpass2 - BE.highpass1));
		}
	}
	else {
		RO.highpass_start_band = 0;
		RO.highpass_end_band = -1; /* do not to run into for-loops */
	}

	RO.use_filtering = 0 <= RO.lowpass_end_band || 0 <= RO.highpass_end_band;
	/*
	   RO.printf("lowpass band with amp=0:  %i \n",RO.lowpass_band);
	   RO.printf("highpass band with amp=0:  %i \n",RO.highpass_band);
	   RO.printf("lowpass band start:  %i \n",RO.lowpass_start_band);
	   RO.printf("lowpass band end:	%i \n",RO.lowpass_end_band);
	   RO.printf("highpass band start:  %i \n",RO.highpass_start_band);
	   RO.printf("highpass band end:	%i \n",RO.highpass_end_band);
	 */
	if (RO.lowpass_band != -1) {
		/* RO.ixend は8の倍数を仮定する */
		/* 18samples per band, extra 8samples for anti-aliasing in mdct */
		RO.ixend = RO.lowpass_band * 18 + 8;
		if (RO.ixend > 576) {
			RO.ixend = 576;
		}

		for (band = 0; band < SBMAX_s; band++) {
			int start = RO.scalefac_band.s[band];
			int end   = RO.scalefac_band.s[band + 1];
			if (start*3 <= RO.ixend && RO.ixend <= end*3) {
				RO.ixend = end*3;
				break;
			}
		}

		/* must be 8samples aligned */
		RO.ixend = (RO.ixend + 7) & ~7;
	} else {
		RO.ixend = 576;
	}
	assert (RO.ixend <= 576);
}

static void
optimum_bandwidth(double *const lowerlimit,
				  double *const upperlimit,
				  const unsigned bitrate,
				  const int samplefreq,
				  const double channels)
{
/* 
 *  Input:
 *	  bitrate	 total bitrate in bps
 *	  samplefreq  output sampling frequency in Hz
 *	  channels	1 for mono, 2+epsilon for MS stereo, 3 for LR stereo
 *				  epsilon is the percentage of LR frames for typical audio
 *				  (I use 'Fade to Gray' by Metallica)
 *
 *   Output:
 *	  lowerlimit: best lowpass frequency limit for input filter in Hz
 *	  upperlimit: best highpass frequency limit for input filter in Hz
 */
	double  f_low;
	double  f_high;
	double  br;

	assert(bitrate >= 8000 && bitrate <= 320000);
	assert(samplefreq >= 16000 && samplefreq <= 48000);
	assert(channels == 1 || (channels >= 2 && channels <= 3));

	if (samplefreq >= 32000)
		br =
			bitrate - (channels ==
					   1 ? (17 + 4) * 8 : (32 + 4) * 8) * samplefreq / 1152;
	else
		br =
			bitrate - (channels ==
					   1 ? (9 + 4) * 8 : (17 + 4) * 8) * samplefreq / 576;

	if (channels >= 2.)
		br /= 1.75 + 0.25 * (channels - 2.); // MS needs 1.75x mono, LR needs 2.00x mono (experimental data of a lot of albums)

	br *= 0.5;		  // the sine and cosine term must share the bitrate

/* 
 *  So, now we have the bitrate for every spectral line.
 *  Let's look at the current settings:
 *
 *	Bitrate   limit	bits/line
 *	 8 kbps   0.34 kHz  4.76
 *	16 kbps   1.9 kHz   2.06
 *	24 kbps   2.8 kHz   2.21
 *	32 kbps   3.85 kHz  2.14
 *	40 kbps   5.1 kHz   2.06
 *	48 kbps   5.6 kHz   2.21
 *	56 kbps   7.0 kHz   2.10
 *	64 kbps   7.7 kHz   2.14
 *	80 kbps  10.1 kHz   2.08
 *	96 kbps  11.2 kHz   2.24
 *   112 kbps  14.0 kHz   2.12
 *   128 kbps  15.4 kHz   2.17
 *   160 kbps  18.2 kHz   2.05
 *   192 kbps  21.1 kHz   2.14
 *   224 kbps  22.0 kHz   2.41
 *   256 kbps  22.0 kHz   2.78
 *
 *   What can we see?
 *	   Value for 8 kbps is nonsense (although 8 kbps and stereo is nonsense)
 *	   Values are between 2.05 and 2.24 for 16...192 kbps
 *	   Some bitrate lack the following bitrates have: 16, 40, 80, 160 kbps
 *	   A lot of bits per spectral line have: 24, 48, 96 kbps
 *
 *   What I propose?
 *	   A slightly with the bitrate increasing bits/line function. It is
 *	   better to decrease NMR for low bitrates to get a little bit more
 *	   bandwidth. So we have a better trade off between twickling and
 *	   muffled sound.
 */

	f_low = br / log10(br * 4.425e-3); // Tests with 8, 16, 32, 64, 112 and 160 kbps



/*
 *  Now we try to choose a good high pass filtering frequency.
 *  This value is currently not used.
 *	For fu < 16 kHz:  sqrt(fu*fl) = 560 Hz
 *	For fu = 18 kHz:  no high pass filtering
 *  This gives:
 *
 *   2 kHz => 160 Hz
 *   3 kHz => 107 Hz
 *   4 kHz =>  80 Hz
 *   8 kHz =>  40 Hz
 *  16 kHz =>  20 Hz
 *  17 kHz =>  10 Hz
 *  18 kHz =>   0 Hz
 *
 *  These are ad hoc values and these can be optimized if a high pass is available.
 */
	if (f_low <= 16000)
		f_high = 16000. * 20. / f_low;
	else if (f_low <= 18000)
		f_high = 180. - 0.01 * f_low;
	else
		f_high = 0.;

	/*  
	 *  When we sometimes have a good highpass filter, we can add the highpass
	 *  frequency to the lowpass frequency
	 */

	if (lowerlimit != NULL)
		*lowerlimit = f_low /* + f_high */ ;
	if (upperlimit != NULL)
		*upperlimit = f_high;
/*
 * Now the weak points:
 *
 *   - the formula f_low=br/log10(br*4.425e-3) is an ad hoc formula
 *	 (but has a physical background and is easy to tune)
 *   - the switch to the ATH based bandwidth selecting is the ad hoc
 *	 value of 128 kbps
 */
}

/* set internal feature flags.  USER should not access these since
 * some combinations will produce strange results */
void
lame_init_qval(void)
{
	switch (BE.quality) {
	case 9:
		RO.noise_shaping = 0;
		RO.noise_shaping_amp = 0;
		RO.noise_shaping_stop = 0;
		RO.use_best_huffman = 0;
		RO.use_psy = 0;
		break;
	case 8:
	case 7:			/* use psymodel (for short block and m/s switching), but no noise shapping */
		RO.noise_shaping = 0;
		RO.noise_shaping_amp = 0;
		RO.noise_shaping_stop = 0;
		RO.use_best_huffman = 0;
		RO.use_psy = 1;
		break;

	default:
		assert(0);
	case 6:
	case 5:			/* the default */
		RO.noise_shaping = 1;
		RO.noise_shaping_amp = 0;
		RO.noise_shaping_stop = 0;
		RO.use_best_huffman = 0;
		RO.use_psy = 1;
		break;

	case 4:
	case 3:
		RO.noise_shaping = 1;
		RO.noise_shaping_amp = 0;
		RO.noise_shaping_stop = 0;
		RO.use_best_huffman = 1;
		RO.use_psy = 1;
		break;

	case 2:
	case 1:
		RO.noise_shaping = 1;
		RO.noise_shaping_amp = 1;
		RO.noise_shaping_stop = 1;
		RO.use_best_huffman = 1;
		RO.use_psy = 1;
		break;

	case 0:
		RO.noise_shaping = 1; /* 2=usually lowers quality */
		RO.noise_shaping_amp = 2;
		RO.noise_shaping_stop = 1;
		RO.use_best_huffman = 1; /* 2 not yet coded */
		RO.use_psy = 1;
		break;
	}
}

/********************************************************************
 *   initialize internal params based on data in gf
 *   (globalflags struct filled in by calling program)
 *
 *  OUTLINE:
 *
 * We first have some complex code to determine bitrate, 
 * output samplerate and mode.  It is complicated by the fact
 * that we allow the user to set some or all of these parameters,
 * and need to determine best possible values for the rest of them:
 *
 *  1. set some CPU related flags
 *  2. check if we are mono->mono, stereo->mono or stereo->stereo
 *  3.  compute bitrate and output samplerate:
 *		  user may have set compression ratio
 *		  user may have set a bitrate  
 *		  user may have set a output samplerate
 *  4. set some options which depend on output samplerate
 *  5. compute the actual compression ratio
 *  6. set mode based on compression ratio
 *
 *  The remaining code is much simpler - it just sets options
 *  based on the mode & compression ratio: 
 *   
 *   select lowpass filter based on compression ratio & mode
 *   set the bitrate index, and min/max bitrates for VBR modes
 *   disable VBR tag if it is not appropriate
 *   initialize the bitstream
 *   initialize scalefac_band data
 *   set sideinfo_len (based on channels, CRC, out_samplerate)
 *   write an id3v2 tag into the bitstream
 *   write VBR tag into the bitstream
 *   set mpeg1/2 flag
 *   estimate the number of frames (based on a lot of data)
 *		 
 *   now we set more flags:
 *   nspsytune:
 *	  see code
 *   VBR modes
 *	  see code	  
 *   CBR/ABR
 *	  see code   
 *
 *  Finally, we set the algorithm flags based on the gfp->quality value
 *  lame_init_qval(gfp);
 *
 ********************************************************************/
MERET
lame_init_params(void)
{

	int	 i;
	int	 j;
	float compression_ratio = 0;  /* 適当に消して... sizeof(wav file)/sizeof(mp3 file) */

	if (RO.nChannel == 1)
		RO.mode = MONO;
	RO.channels_out = (RO.mode == MONO) ? 1 : 2;
	if( RO.bWriteVbrTag == -1 ){
		if( RO.bWriteLameTag ){
			RO.bWriteVbrTag = TRUE;
		} else {
			if( RO.VBR == vbr_off )
				RO.bWriteVbrTag = FALSE;
			else
				RO.bWriteVbrTag = TRUE;
		}
	}

	RO.mode_gr = RO.out_samplerate <= 24000 ? 1 : 2; // Number of granules per frame
	RO.framesize = 576 * RO.mode_gr;

	/* 
	 *  sample freq	   bitrate	 compression ratio
	 *	 [kHz]	  [kbps/channel]   for 16 bit input
	 *	 44.1			56			   12.6
	 *	 44.1			64			   11.025
	 *	 44.1			80				8.82
	 *	 22.05		   24			   14.7
	 *	 22.05		   32			   11.025
	 *	 22.05		   40				8.82
	 *	 16			  16			   16.0
	 *	 16			  24			   10.667
	 *
	 */
	/* 
	 *  For VBR, take a guess at the compression_ratio. 
	 *  For example:
	 *
	 *	VBR_q	compression	 like
	 *	 -		4.4		 320 kbps/44 kHz
	 *   0...1	  5.5		 256 kbps/44 kHz
	 *	 2		7.3		 192 kbps/44 kHz
	 *	 4		8.8		 160 kbps/44 kHz
	 *	 6	   11		   128 kbps/44 kHz
	 *	 9	   14.7		  96 kbps
	 *
	 *  for lower bitrates, downsample with --resample
	 */

    switch ( RO.VBR ) {
    case vbr_rh:
        {
            FLOAT8  cmp[] = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
            compression_ratio = cmp[BE.VBR_q];
        }
        break;
    case vbr_abr:
		compression_ratio =
			RO.out_samplerate * 16 * RO.channels_out / (1.e3 * RO.brate);
        break;
	default:
		assert( 0 );
    case vbr_off:
		compression_ratio =
			RO.out_samplerate * 16 * RO.channels_out / (1.e3 * RO.brate);
    }



	/* mode = -1 (not set by user) or 
	 * mode = MONO (because of only 1 input channel).  
	 * If mode has been set, then select between STEREO or J-STEREO
	 * At higher quality (lower compression) use STEREO instead of J-STEREO.
	 * (unless the user explicitly specified a mode)
	 *
	 * The threshold to switch to STEREO is:
	 *	48 kHz:   171 kbps (used at 192+)
	 *	44.1 kHz: 160 kbps (used at 160+)
	 *	32 kHz:   119 kbps (used at 128+)
	 *
	 *   Note, that for 32 kHz/128 kbps J-STEREO FM recordings sound much
	 *   better than STEREO, so I'm not so very happy with that. 
	 *   fs < 32 kHz I have not tested.
	 */
	if (RO.mode == NOT_SET) {
		if (compression_ratio < 8)
			RO.mode = STEREO;
		else
			RO.mode = JOINT_STEREO;
	}

  /****************************************************************/
  /* if a filter has not been enabled, see if we should add one: */
  /****************************************************************/
	if (BE.lowpassfreq == 0) {
		double  lowpass;
		double  highpass;
		double  channels;

		switch (RO.mode) {
		case MONO:
			channels = 1.;
			break;
		case JOINT_STEREO:
			channels = 2. + 0.00;
			break;
		case STEREO:
			channels = 3.;
			break;
		default:	
			channels = 1.;  // just to make data flow analysis happy :-)
			assert(0);
			break;
		}

		optimum_bandwidth(&lowpass, &highpass,
				  RO.out_samplerate * 16 * RO.channels_out /
				  compression_ratio, RO.out_samplerate, channels);

		if (lowpass < 0.5 * RO.out_samplerate) {
			BE.lowpass1 = BE.lowpass2 =
				lowpass / (0.5 * RO.out_samplerate);
		}

		// アップサンプリング時、元データのサンプリング周波数の1/2が現状のLPF設定より下なら
		// 元データのサンプリング周波数の1/2をLPFに設定する
		if (BE.inpFreqHz < RO.out_samplerate && 
			1.0 * BE.inpFreqHz / RO.out_samplerate < BE.lowpass1) {
			BE.lowpass1 = BE.lowpass2 = 
				1.0 * BE.inpFreqHz / RO.out_samplerate;
		}

		fflush(stderr);
	}

	/* apply user driven high pass filter */
	if (BE.highpassfreq > 0) {
		BE.highpass1 = 2. * BE.highpassfreq / RO.out_samplerate; /* will always be >=0 */
		if (BE.highpasswidth >= 0)
			BE.highpass2 =
				2. * (BE.highpassfreq +
					  BE.highpasswidth) / RO.out_samplerate;
		else			/* 0% above on default */
			BE.highpass2 =
				(1 + 0.00) * 2. * BE.highpassfreq / RO.out_samplerate;
	}

	/* apply user driven low pass filter */
	if (BE.lowpassfreq > 0) {
		BE.lowpass2 = 2. * BE.lowpassfreq / RO.out_samplerate; /* will always be >=0 */
		if (BE.lowpasswidth >= 0) {
			BE.lowpass1 =
				2. * (BE.lowpassfreq -
					  BE.lowpasswidth) / RO.out_samplerate;
			if (BE.lowpass1 < 0) /* has to be >= 0 */
				BE.lowpass1 = 0;
		}
		else {		  /* 0% below on default */
			BE.lowpass1 =
				(1 - 0.00) * 2. * BE.lowpassfreq / RO.out_samplerate;
		}
	}

  /*******************************************************
   * samplerate and bitrate index
   *******************************************************/
	RO.samplerate_index = SmpFrqIndex(RO.out_samplerate, &RO.version);
	if (RO.samplerate_index < 0)
		return ME_FREQERROR;

	if (RO.VBR  == vbr_off ) {
		BE.CBR_bitrate = BitrateIndex(RO.brate, RO.version);
		if (BE.CBR_bitrate < 0) return ME_BITRATEERROR;
	} else {			  /* choose a min/max bitrate for VBR */
		/* if the user didn't specify VBR_max_bitrate: */
		RO.VBR_min_bitrate = 1; /* default: allow   8 kbps (MPEG-2) or  32 kbps (MPEG-1) */
		RO.VBR_max_bitrate = 14; /* default: allow 160 kbps (MPEG-2) or 320 kbps (MPEG-1) */

		if (BE.VBR_min_bitrate_kbps)
			if (
				(RO.VBR_min_bitrate =
				 BitrateIndex(BE.VBR_min_bitrate_kbps, RO.version)) < 0) return ME_BITRATEERROR;
		if (BE.VBR_max_bitrate_kbps)
			if (
				(RO.VBR_max_bitrate =
				 BitrateIndex(BE.VBR_max_bitrate_kbps, RO.version)) < 0) return ME_BITRATEERROR;

		BE.VBR_min_bitrate_kbps =
			bitrate_table[RO.version][RO.VBR_min_bitrate];
		BE.VBR_max_bitrate_kbps =
			bitrate_table[RO.version][RO.VBR_max_bitrate];
	}

////// initWrite()へ移動
 //	init_bit_stream_w();
	
	j =
		RO.samplerate_index + (3 * RO.version) + 6 * (RO.out_samplerate <
														  16000);
	for (i = 0; i < SBMAX_l + 1; i++)
		RO.scalefac_band.l[i] = sfBandIndex[j].l[i];
	for (i = 0; i < SBMAX_s + 1; i++)
		RO.scalefac_band.s[i] = sfBandIndex[j].s[i];

	/* determine the mean bitrate for main data */
	if (RO.version == 1) /* MPEG 1 */
		RO.sideinfo_len = (RO.channels_out == 1) ? 4 + 17 : 4 + 32;
	else				/* MPEG 2 */
		RO.sideinfo_len = (RO.channels_out == 1) ? 4 + 9 : 4 + 17;

  /**********************************************************************/
  /* compute info needed for polyphase filter (filter type==0, default) */
  /**********************************************************************/
	lame_init_params_ppflt();

	/* 
	 *  Write id3v2 tag into the bitstream.
	 *  This tag must be before the Xing VBR header.
	 */
//	id3tag_write_v2();


	/* Write initial VBR Header to bitstream */
//	if (RO.VBR != vbr_off ) InitVbrTag();
	if (RO.bWriteVbrTag) InitVbrTag();

	/* estimate total frames and samples of last frame.  */
	if ( 0 < RO.nSample ) {
		RW.totalframes = (ENCDELAY - MDCTDELAY + RO.nSample + ENCDELAY + 288 + (RO.framesize-1)) / RO.framesize;
	}
	else {
		// can't estimate
		RW.totalframes = 1;
	}

	if(RO.VBR == vbr_rh ) {
		extern const FLOAT8 dbQ[10];
		/*  automatic ATH adjustment on, VBR modes need it
		 */
		RW.ATH.use_adjust = 1;

		/*  sfb21 extra only with MPEG-1 at higher sampling rates
		 */
		RW.sfb21_extra = (RO.out_samplerate > 44000);

		/*  VBR needs at least the output of GPSYCHO,
		 *  so we have to garantee that by setting a minimum 
		 *  quality level, actually level 5 does it.
		 *  the -v and -V x settings switch the quality to level 2
		 *  you would have to add a -q 5 to reduce the quality
		 *  down to level 5
		 */
		if(BE.quality > 5) BE.quality = 5;

		RO.VBR_dbQ = dbQ[BE.VBR_q];
	}else{
		/*  automatic ATH adjustment off, not so important for CBR code
		 */
		RW.ATH.use_adjust = 0;

		/*  no sfb21 extra with CBR code
		 */
		RW.sfb21_extra = 0;
	}

	/* initialize internal qval settings */
	lame_init_qval();

	psymodel_init();
	iteration_init();
	return ME_NOERR;
}

void
lame_init( void )
{
	memset( &RW, 0, sizeof( RW ) );

	/* Global flags.  set defaults here for non-zero values */
	/* see lame.h for description */
	/* set integer values to -1 to mean that LAME will compute the
	 * best value, UNLESS the calling program as set it
	 * (and the value is no longer -1)
	 */

	BE.lowpassfreq = 0;
	BE.highpassfreq = 0;
	BE.lowpasswidth = -1;
	BE.highpasswidth = -1;

	RO.VBR = vbr_off;
	RO.bWriteVbrTag = -1;             /* 後で設定を上書きします */
	RO.bWriteLameTag = FALSE;
	BE.VBR_q = 4;
	BE.VBR_min_bitrate_kbps = 0;
	BE.VBR_max_bitrate_kbps = 0;

	RO.lowpass_band = 32;
	RO.highpass_band = -1;
	RO.VBR_min_bitrate = 1; /* not  0 ????? */
	RO.VBR_max_bitrate = 13; /* not 14 ????? */

	RW.OldValue[0] = 180;
	RW.OldValue[1] = 180;
	RW.CurrentStep = 4;
}

