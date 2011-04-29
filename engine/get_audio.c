/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif

#include "config.h"
#include "global.h"

#include "get_audio.h"
#include "tool.h"
#include "gogo_io.h"
#include "vfta.h"

/* NEED asm */
void
dist_mfbuf_C(float (*mfbuf)[4][576])
{
	int	i;
	short	*fr0, *fr1, *fr2;
	float	*buf_L, *buf_R;

	buf_L = mfbuf[0][0];
	buf_R = mfbuf[1][0];
	fr0 = RW.fr0;
	fr1 = RW.fr1;
	fr2 = RW.fr2;

	if(RO.mode_gr == 2){
		/* 1904(1152+BLKSIZE-224-MDCTDELAY) samples for MPEG-1 */
		if(RO.nChannel == 1){
			for(i = 0; i < 1152; i++){
				*buf_L++ = (float)(*fr0++);
			}
			for(i = 0; i < (BLKSIZE-224-MDCTDELAY)/* 752 */; i++){
				*buf_L++ = (float)(*fr1++);
			}
		}else if(RO.channels_out == 2){
			// 20.6klck@K7-500
			for(i = 0; i < 1152; i++){
				*buf_L++ = (float)(*fr0++);
				*buf_R++ = (float)(*fr0++);
			}
			for(i = 0; i < (BLKSIZE-224-MDCTDELAY)/* 752 */; i++){
				*buf_L++ = (float)(*fr1++);
				*buf_R++ = (float)(*fr1++);
			}
		}else{
			for(i = 0; i < 1152; i++){
				*buf_L++ = 0.5*((float)(*fr0++) + (float)(*fr0++));
			}
			for(i = 0; i < (BLKSIZE-224-MDCTDELAY)/* 752 */; i++){
				*buf_L++ = 0.5*((float)(*fr1++) + (float)(*fr1++));
			}
		}
	}else{
		/* 1328(576+BLKSIZE-224-MDCTDELAY) samples for MPEG-2 */
		if(RO.nChannel == 1){
			for(i = 0; i < 576; i++){
				*buf_L++ = (float)(*fr0++);
			}
			for(i = 0; i < 576; i++){
				*buf_L++ = (float)(*fr1++);
			}
			for(i = 0; i < (BLKSIZE-224-MDCTDELAY-576)/* 176 */; i++){
				*buf_L++ = (float)(*fr2++);
			}
		}else if(RO.channels_out == 2){
			for(i = 0; i < 576; i++){
				*buf_L++ = (float)(*fr0++);
				*buf_R++ = (float)(*fr0++);
			}
			for(i = 0; i < 576; i++){
				*buf_L++ = (float)(*fr1++);
				*buf_R++ = (float)(*fr1++);
			}
			for(i = 0; i < (BLKSIZE-224-MDCTDELAY-576)/* 176 */; i++){
				*buf_L++ = (float)(*fr2++);
				*buf_R++ = (float)(*fr2++);
			}
		}else{
			for(i = 0; i < 576; i++){
				*buf_L++ = 0.5*((float)(*fr0++) + (float)(*fr0++));
			}
			for(i = 0; i < 576; i++){
				*buf_L++ = 0.5*((float)(*fr1++) + (float)(*fr1++));
			}
			for(i = 0; i < (BLKSIZE-224-MDCTDELAY-576)/* 176 */; i++){
				*buf_L++ = 0.5*((float)(*fr2++) + (float)(*fr2++));
			}
		}
	}
}

#ifdef CPU_I386
void dist_mfbuf_mpeg1stereo_SSE(float (*mfbuf)[4][576]);
void dist_mfbuf_mpeg1stereo_E3DN(float (*mfbuf)[4][576]);
void setup_dist_mfbuf(int unit)
{
	dist_mfbuf = dist_mfbuf_C;
	if(RO.mode_gr == 2 && RO.channels_out == 2){
		if( unit & MU_tSSE ){
			assert(BLKSIZE-224-MDCTDELAY == 752);
			dist_mfbuf = dist_mfbuf_mpeg1stereo_SSE;
		}else
		if( (unit & MU_tMMX) && (unit & MU_t3DN) && (unit & MU_tE3DN) ){
			assert((BLKSIZE-224-MDCTDELAY) % 8 == 0);
			dist_mfbuf = dist_mfbuf_mpeg1stereo_E3DN;
		}
	}
}
#endif

/* The reason for
 *         int mf_samples_to_encode = ENCDELAY + 288;
 * ENCDELAY = internal encoder delay.  And then we have to add 288
 * because of the 50% MDCT overlap.  A 576 MDCT granule decodes to
 * 1152 samples.  To synthesize the 576 samples centered under this granule
 * we need the previous granule for the first 288 samples (no problem), and
 * the next granule for the next 288 samples (not possible if this is last
 * granule).  So we need to pad with 288 samples to make sure we can
 * encode the 576 samples we are interested in.
 */

void get_audio_init(void)
{
	int	to_read, iread, can_read;

	RW.num_samples_read = 0;
	RW.num_samples_padding = - MDCTDELAY - RO.framesize;
	RW.num_samples_to_encode = ENCDELAY + 288 + 1152 /* for FFT */;
	// FFTのためにget_audio_init()で1152サンプル先行して読むので、最後に1152の0を吐く

	memset(RW.sample_buf, 0, 2*2*1152*sizeof(short));
	can_read = to_read = RO.nChannel*(1152 - (ENCDELAY - MDCTDELAY));
	if (0 < RO.nSample && RO.nChannel*RO.nSample < can_read ){
		can_read = RO.nChannel*RO.nSample;
	}
	RW.fr0 = &RW.sample_buf[                         0];
	RW.fr1 = &RW.sample_buf[RO.nChannel*RO.framesize*1];
	RW.fr2 = &RW.sample_buf[RO.nChannel*RO.framesize*2];
	RW.fr3 = NULL;
	iread = readData((unsigned char *)(RW.fr1+RO.nChannel*(ENCDELAY - MDCTDELAY)), can_read*sizeof(short));
	if(iread < 0) return;/*iread;*/

	iread >>= 1;	/* iread /= sizeof(short) */
	if(RO.nChannel == 2){
		iread >>= 1;
		to_read >>=1 ;
	}
	/* subtract size of zero-filled region */
	RW.num_samples_to_encode -= (to_read-iread);

	RW.num_samples_read += iread;
}

/* input : readData
 * output: RW.fr? */
int get_pcm(void)
{
	int	to_read, iread, can_read;

	if(RW.num_samples_to_encode <= 0) return 0;

	can_read = to_read = RO.nChannel*RO.framesize;
	if (0 < RO.nSample ) {
		int		leftsample = RO.nChannel*(RO.nSample - RW.num_samples_read);
		if( leftsample < can_read ){
			/* データ末端の処理 */
			assert( leftsample >= 0 );
			if( leftsample < 0 ) {
				can_read = 0;
			} else  {
				assert( (can_read - leftsample) % RO.nChannel == 0 );
				can_read = leftsample;
			}
		}
	}
	if( can_read > 0 )
		iread = readData((unsigned char *)RW.fr0, can_read*sizeof(short));
	else
		iread = 0;
	if(iread < 0) return iread;	/* error */

	iread >>= 1;	/* iread /= sizeof(short) */
	RW.num_samples_padding += (to_read - iread) / RO.nChannel;
	if(iread != to_read) memset(RW.fr0+iread, 0, (to_read-iread)*sizeof(short));

	if(RO.mode_gr == 2){
		/* shift out */
		RW.fr2 = RW.fr0;
		RW.fr0 = RW.fr1;
		RW.fr1 = RW.fr2;
		/* シフトした結果 Fr0:Fr1 (Fr1=Fr2,Fr1には今回読みこんだデータ) */
	}else{
		/* shift out */
		RW.fr3 = RW.fr0;
		RW.fr0 = RW.fr1;
		RW.fr1 = RW.fr2;
		RW.fr2 = RW.fr3;
		/* シフトした結果 Fr0:Fr1:Fr2 (Fr2=Fr3,Fr2には今回読み込んだデータ) */
	}

	if(RO.nChannel == 2){
		iread >>= 1;
		to_read >>=1 ;
	}
	RW.num_samples_read += iread;

	/* subtract size of zero-filled region */
	RW.num_samples_to_encode -= (to_read-iread);
	if( RW.num_samples_to_encode < 0 ) {
		//RW.num_samples_padding += RW.num_samples_to_encode /*/ RO.nChannel*/;
		if( RW.num_samples_padding < RO.framesize )
			RW.num_samples_padding += RO.framesize;
		RW.num_samples_to_encode = 0;
	}

	return to_read;
}

