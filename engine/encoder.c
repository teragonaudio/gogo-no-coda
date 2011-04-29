/*
 *	LAME MP3 encoding engine
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

void
#if defined(MT_ENCODER)
encodethread_init(gogo_thread_data *tl)
#else
encodeframe_init(gogo_thread_data *tl)
#endif
{
	FLOAT8 frame_duration = 576. * RO.mode_gr / RO.out_samplerate;

	RO.frac_SpF = ((RO.version+1)*72000L*RO.brate) % RO.out_samplerate;
	RW.slot_lag = RO.frac_SpF;

	/* 前のフレームのsb_sampleを作る */
#if !defined(MT_ENCODER)
	pfb(tl->sb[RO.mode_gr], (float (*)[4][576])tl->mfbuf[0][RO.mode_gr-1]);
#else
	pfb(RW.subband_buf, (float (*)[4][576])tl->mfbuf[0][RO.mode_gr-1]);
#endif

	/*  prepare for ATH auto adjustment:
	 *  we want to decrease the ATH by 12 dB per second */
	RW.ATH.decay = pow(10., -12./10. * frame_duration);
	RW.ATH.adjust = 1.0;
	RW.ATH.adjust_limit = 0.01;
}

/************************************************************************
*
* encode a single frame
*
************************************************************************

                       gr 0            gr 1
mfbuf:           |--------------|---------------|-------------|-------------|
MDCT output:  |--------------|---------------|-------------|

FFT's                    <---------1024---------->
                                         <---------1024-------->

    mfbuf = buffer of PCM data size=MP3 framesize
    encoder acts on mfbuf[ch][0], but output is delayed by MDCTDELAY
    so the MDCT coefficints are from mfbuf[ch][-MDCTDELAY]

    psy-model FFT has a 1 granule delay, so we feed it data for the 
    next granule.
    FFT is centered over granule:  224+576+224
    So FFT starts at:   576-224-MDCTDELAY

    MPEG2:  FFT ends at:  BLKSIZE+576-224-MDCTDELAY
    MPEG1:  FFT ends at:  BLKSIZE+2*576-224-MDCTDELAY    (1904)

    FFT starts at 576-224-MDCTDELAY (304)  = 576-FFTOFFSET

*/

int
#if defined(MT_ENCODER)
lame_encode_mp3_frame_multi(gogo_thread_data *tl)
#else
lame_encode_mp3_frame(gogo_thread_data *tl)
#endif
{
	III_psy_ratio masking_LR[2][2], masking_MS[2][2];    /* masking & energy for LR/MS */
	III_psy_ratio (*masking)[2];  /*pointer to selected maskings*/

	FLOAT8 tot_ener[2][4];   
	int	blocktype[2][2];
	FLOAT8 ms_ener_ratio[2]={.5,.5};
	FLOAT8 pe_LR[2][2],pe_MS[2][2];
	FLOAT8 (*pe)[2];  /*pointer to selected pe*/

	int ch,gr,mean_bits;
	int bitsPerFrame;

	int check_ms_stereo;
	FLOAT8 ms_ratio_next = 0.;
	FLOAT8 ms_ratio_prev = 0.;

	tl->mode_ext = MPG_MD_LR_LR;
	memset((char *) masking_LR, 0, sizeof(masking_LR));
	memset((char *) masking_MS, 0, sizeof(masking_MS));

	check_ms_stereo =  (RO.mode == JOINT_STEREO);

	/********************** padding *****************************/
	if (RO.VBR) {
		tl->padding=0;
	} else {
		/* padding method as described in 
		 * "MPEG-Layer3 / Bitstream Syntax and Decoding"
		 * by Martin Sieler, Ralph Sperschneider
		 *
		 * note: there is no padding for the very first frame
		 *
		 * Robert.Hegemann@gmx.de 2000-06-22
		 */

		RW.slot_lag -= RO.frac_SpF;
		if (RW.slot_lag < 0) {
			RW.slot_lag += RO.out_samplerate;
			tl->padding = 1;
		} else {
			tl->padding = 0;
		}
	} /* reservoir enabled */

#if !defined(MT_ENCODER)
	/* shift out pfb result (for st_encode) */
	tl->sb[3] = tl->sb[RO.mode_gr];
	tl->sb[RO.mode_gr] = tl->sb[0];
	tl->sb[0] = tl->sb[3];
#endif

	/* polyphase filter bank */
	/* 200Kclk@PIII-700 */
	pfb(tl->sb[1], (float (*)[4][576])tl->mfbuf[0][0]);
	if(RO.mode_gr == 2) pfb(tl->sb[2], (float (*)[4][576])tl->mfbuf[0][1]);

	if (RO.use_psy) {
#if defined(MT_ENCODER)
		gogo_lock_mutex(tl->critical_region + 1);
		gogo_unlock_mutex(tl->critical_region++);
#endif

	/* FFT */
//clkbegin();
	/* 384Kclk 並列化前のCでは334Kclk gogo2_FFTでは174clk 果たしてgogo2を超えられるか */
	/* 2001/10/17 Cレベルで340Kclkまでいった。 */
	/* 2001/10/28 155Kclk(まだまだだけど一応速くなったということで) */
	/* mfbuf => wsamp_L/S */
#if defined(USE_VECTOR4_FHT)
		if( fht_vector4 ){
			float *work = &tl->psywork[0];			/* float [576*4*4] */
			float *dest = &tl->psywork[576*4*4];	/* float [576*4*4] */
			float *ptr;
			int i;
#if 1
			assert(((1152+BLKSIZE-224-MDCTDELAY) % 4) == 0);
			fft_prepare(work, (float *)tl->mfbuf[0][0], 1152+BLKSIZE-224-MDCTDELAY);
#else
			float *gr0L, *gr0R, *gr1L, *gr1R;
			gr0L = (float *)tl->mfbuf[0][0];
			gr0R = (float *)tl->mfbuf[1][0];
			gr1L = (float *)tl->mfbuf[0][1];
			gr1R = (float *)tl->mfbuf[1][1];
			for( i = 0; i < 1152+BLKSIZE-224-MDCTDELAY; i++ ){
				work[i*4+0] = *gr0L++;
				work[i*4+1] = *gr0R++;
				work[i*4+2] = *gr1L++;
				work[i*4+3] = *gr1R++;
			}
#endif

			if( RO.mode == JOINT_STEREO ){
				/* インタリーブのままpsymodelに渡す */
				fft_long_vector4( tl->wsamp_L[0][0], work );
				/* 48Kclk */
				fft_short_vector4( tl->wsamp_S[0][0][0], work );
			}else{
				fft_long_vector4( dest, work );
				ptr = tl->wsamp_L[0][0];
				for( i = 0; i < BLKSIZE; i++ ){
					ptr[i+BLKSIZE*0] = dest[i*4+0];		// gr=0 L
					ptr[i+BLKSIZE*1] = dest[i*4+1];		// gr=0 R
					ptr[i+BLKSIZE*2] = dest[i*4+2];		// gr=1 L
					ptr[i+BLKSIZE*3] = dest[i*4+3];		// gr=1 R
				}
				fft_short_vector4( dest, work );
				ptr = tl->wsamp_S[0][0][0];
				for( i = 0; i < BLKSIZE_s*3; i++ ){
					ptr[i+BLKSIZE_s*3*0] = dest[i*4+0];
					ptr[i+BLKSIZE_s*3*1] = dest[i*4+1];
					ptr[i+BLKSIZE_s*3*2] = dest[i*4+2];
					ptr[i+BLKSIZE_s*3*3] = dest[i*4+3];
				}
			}
		}else
#endif
		{
			fft_long (tl->wsamp_L[0], (float (*)[4][576])tl->mfbuf[0][0]);
			fft_short(tl->wsamp_S[0], (float (*)[4][576])tl->mfbuf[0][0]);
			if(RO.mode_gr == 2){
				fft_long (tl->wsamp_L[1], (float (*)[4][576])tl->mfbuf[0][1]);
				fft_short(tl->wsamp_S[1], (float (*)[4][576])tl->mfbuf[0][1]);
			}
		}
	}
//clkend();

#if defined(MT_ENCODER)
	gogo_lock_mutex(tl->critical_region + 1);
	gogo_unlock_mutex(tl->critical_region++);

	/* shift out pfb result (for mt_encode) */
	shiftoutpfb(tl->sb[0], tl->sb[RO.mode_gr]);

	gogo_lock_mutex(tl->critical_region + 1);
	gogo_unlock_mutex(tl->critical_region++);
#endif

	/* psychoacoustic model
	 * psy model has a 1 granule (576) delay that we must compensate for(mt 6/99).  */
	if (RO.use_psy) {
		ms_ratio_prev = RW.ms_ratio[RO.mode_gr-1];
		for (gr=0; gr < RO.mode_gr ; gr++) {
			L3psycho_anal( tl, gr, &RW.ms_ratio[gr], &ms_ratio_next,
			 masking_LR[gr], masking_MS[gr], pe_LR[gr],pe_MS[gr],tot_ener[gr],blocktype[gr]);
			if (check_ms_stereo) {
				ms_ener_ratio[gr] = tot_ener[gr][2]+tot_ener[gr][3];
				if (ms_ener_ratio[gr]>0){
					ms_ener_ratio[gr] = tot_ener[gr][3]/ms_ener_ratio[gr];
				}
			}
		}
	} else {
		for (gr=0; gr < RO.mode_gr ; gr++) {
			for ( ch = 0; ch < RO.channels_out; ch++ ) {
				blocktype[gr][ch] = NORM_TYPE;
				pe_MS[gr][ch]=pe_LR[gr][ch]=700;
 			}
 		}
 	}

	if (check_ms_stereo) {
		/* ms_ratio = is scaled, for historical reasons, to look like
		a ratio of side_channel / total.  
		 0 = signal is 100% mono
		 .5 = L & R uncorrelated
		*/

		/* [0] and [1] are the results for the two granules in MPEG-1,
		* in MPEG-2 it's only a faked averaging of the same value
		* _prev is the value of the last granule of the previous frame
		* _next is the value of the first granule of the next frame
		*/
		FLOAT8  ms_ratio_ave1;
		FLOAT8  ms_ratio_ave2;
		FLOAT8  threshold1    = 0.35;
		FLOAT8  threshold2    = 0.45;

		/* take an average */
		if (RO.mode_gr==1) {
			/* MPEG2 - no second granule */
			ms_ratio_ave1 = 0.33 * ( RW.ms_ratio[0] + ms_ratio_prev + ms_ratio_next );
			ms_ratio_ave2 = RW.ms_ratio[0];
			if ((ms_ratio_ave1 < threshold1)  &&  (ms_ratio_ave2 < threshold2)) {
				int  sum_pe_MS = pe_MS[0][0] + pe_MS[0][1];
				int  sum_pe_LR = pe_LR[0][0] + pe_LR[0][1];

				/* based on PE: M/S coding would not use much more bits than L/R coding */

				if (sum_pe_MS <= 1.07 * sum_pe_LR ) tl->mode_ext = MPG_MD_MS_LR;
			}
		}else{
			ms_ratio_ave1 = 0.25 * ( RW.ms_ratio[0] + RW.ms_ratio[1] + ms_ratio_prev + ms_ratio_next );
			ms_ratio_ave2 = 0.50 * ( RW.ms_ratio[0] + RW.ms_ratio[1] );
			if ((ms_ratio_ave1 < threshold1)  &&  (ms_ratio_ave2 < threshold2)) {
				int  sum_pe_MS = pe_MS[0][0] + pe_MS[0][1] + pe_MS[1][0] + pe_MS[1][1];
				int  sum_pe_LR = pe_LR[0][0] + pe_LR[0][1] + pe_LR[1][0] + pe_LR[1][1];

				/* based on PE: M/S coding would not use much more bits than L/R coding */

				if (sum_pe_MS <= 1.07 * sum_pe_LR ) tl->mode_ext = MPG_MD_MS_LR;
			}
		}
	}

#if defined(MT_ENCODER)
	gogo_lock_mutex(tl->critical_region + 1);
	gogo_unlock_mutex(tl->critical_region++);
#endif

	/* make sure block type is the same in each channel */
	/* mdct */
	/* sb => xr */
	mdct(tl->xr[0], (float (*)[576])tl->sb[0], (float (*)[576])tl->sb[1], blocktype[0]);
	if(RO.mode_gr == 2){
		mdct(tl->xr[1], (float (*)[576])tl->sb[1], (float (*)[576])tl->sb[2], blocktype[1]);
	}
	/* ここから後ろではxrはixendまでしかない */

	/* bit and noise allocation */
	if (MPG_MD_MS_LR == tl->mode_ext) {
		masking = masking_MS;    /* use MS masking */
		pe = pe_MS;
		for( gr = 0; gr < RO.mode_gr; gr++ ) {
			ms_convert(tl->xr[gr][0], tl->xr[gr][1], RO.ixend);
		}
	} else {
		masking = masking_LR;    /* use LR masking */
		pe = pe_LR;
	}

#if defined(MT_ENCODER)
	gogo_lock_mutex(tl->critical_region + 1);
	gogo_unlock_mutex(tl->critical_region++);
#endif

	/* auto-adjust of ATH, useful for low volume */
	if(RW.ATH.use_adjust){
		adjust_ATH(tot_ener);
	}

	assert( BE.VBR_q <= 9 );
	assert( BE.VBR_q >= 0 );

	for( gr = 0; gr < RO.mode_gr; gr++ ) {
		for ( ch = 0; ch < RO.channels_out; ch++ ) {
			/* block type flags */
			tl->l3_side.tt[gr][ch].block_type = blocktype[gr][ch];
			pow075(tl->xr[gr][ch], tl->xrpow[gr][ch],
				&tl->xrpow_sum[gr][ch], &tl->xrpow_max[gr][ch], tl->xr_sign[gr][ch]);

			if(RO.VBR == vbr_default){
				FLOAT8   masking_lower, adjust;

				if (blocktype[gr][ch] == SHORT_TYPE) {
					adjust = 5/(1+exp(3.5-pe[gr][ch]/300.))-0.14;
					masking_lower = pow (10.0, (RO.VBR_dbQ - adjust) * 0.1);
					tl->ath_over[gr][ch] =
						calc_xmin_short(tl->xr[gr][ch], &masking[gr][ch], &tl->l3_xmin[gr][ch], masking_lower);
				} else {
					adjust = 2/(1+exp(3.5-pe[gr][ch]/300.))-0.05;
					masking_lower = pow (10.0, (RO.VBR_dbQ - adjust) * 0.1);
					tl->ath_over[gr][ch] =
						calc_xmin_long(tl->xr[gr][ch], &masking[gr][ch], &tl->l3_xmin[gr][ch], masking_lower);
				}
			}else if(tl->xrpow_sum[gr][ch] > (float)1E-20){
				if (blocktype[gr][ch] == SHORT_TYPE) {
					tl->ath_over[gr][ch] =
						calc_xmin_short(tl->xr[gr][ch], &masking[gr][ch], &tl->l3_xmin[gr][ch], 1.0);
				 }else{
					tl->ath_over[gr][ch] =
						calc_xmin_long(tl->xr[gr][ch], &masking[gr][ch], &tl->l3_xmin[gr][ch], 1.0);
				}
			}
		}
	}

#if defined(MT_ENCODER)
	gogo_lock_mutex(tl->critical_region + 1);
	gogo_unlock_mutex(tl->critical_region++);
#endif

	/* quantization */
	assert(RO.iteration_loop);
	RO.iteration_loop(tl, pe, ms_ener_ratio, &mean_bits);
	if (RO.iteration_finish) {
		RO.iteration_finish(tl->l3_enc, &tl->l3_side, tl->scalefac, mean_bits);
	}
	if( RO.bWriteVbrTag ) AddVbrFrame(tl->bitrate_index);

#if defined(MT_ENCODER)
	gogo_lock_mutex(tl->critical_region + 1);
#endif
	ResvFrameEnd (&tl->l3_side, mean_bits);	/* RW.main_data_begin 対策で移動 */
	tl->ResvSize = RW.ResvSize;
#if defined(MT_ENCODER)
	gogo_unlock_mutex(tl->critical_region++);
#endif
	RO.getframebits(tl, &bitsPerFrame, &mean_bits);

	/*  write the frame to the bitstream  */
	format_bitstream(tl, bitsPerFrame);

	return 0;
}

