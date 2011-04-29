/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#define _GOGO_C_
#include <stddef.h>
#include "../common.h"
#include "../global.h"
#include "../thread.h"
#include "../l3side.h"
#include "../quantize_pvt.h"
#include "../tables.h"
#include "../vbrtag.h"

RW_t RW;
RO_t RO;

#if (defined(WIN32) || defined(__linux__) || defined(__os2__)) &&  !defined(__BORLANDC__) &&  !defined(__MINGW32_VERSION)
	#define USE_BSS
#endif

static void putMember( const char *base, const char *name, int offset )
{
	printf( "%%define %s.%s\t\t(%s+%d)\n", base, name, base, offset );
}

static void putOffset( const char *base, const char *name, int offset )
{
	printf( "%%define %s.%s\t\t(%d)\n", base, name, offset );
}

#define OFFSET( base, x ) ( (int)&base.x - (int)&base )

#ifndef offsetof
  #define offsetof(s,m)   (size_t)&(((s *)0)->m)
#endif

#define putMem( base, x ) putMember( #base, #x, OFFSET( base, x ) )
#define putOff( base, x ) putOffset( #base, #x, offsetof( struct base, x ) )
#define putSize( base ) { printf( "%%define sizeof_%s\t(%d)\n", #base, sizeof( struct base ) ); }

int main( void )
{
	static const char nameTbl[][4] = { "RO", "RW", "" };
	static const int valSize[] = { sizeof(RO) / 4, (sizeof(RW)+127) / 4 };
	int i;

	puts( "\t%include \"nasm.cfg\"" );

	puts( "%ifdef DEF_GLOBAL_VARS" );

	/* global mode */
	printf( "\tglobaldef RO\n" );
	printf( "\tglobaldef RW\n" );
#ifdef USE_BSS
	printf( "\tsegment_bss\n" );
	for( i = 0; i < 2; i++ ){
		printf( "\talignb 32\n" );
		printf( "%s\tresd %d\n", nameTbl[i], valSize[i] );
	}
#else	/* USE_BSS */
	printf( "\tsegment_data\n" );
	for( i = 0; i < 2; i++ ){
		printf( "\talign 32\n" );
		printf( "%s\ttimes %d dd 0\n", nameTbl[i], valSize[i] );
	}
#endif	/* USE_BSS */

	puts( "%else  ; DEF_GLOBAL_VARS" );

	/* extern mode */
	puts( "\texterndef RO\n" );
	puts( "\texterndef RW\n" );
	
	putMem( RO, window );
	putMem( RO, window_s );
	putMem( RO, minval );
	putMem( RO, s3_s );
	putMem( RO, s3_l );
	putMem( RO, s3ind );
	putMem( RO, s3ind_s );
	putMem( RO, SNR_s );
	putMem( RO, numlines_s );
	putMem( RO, numlines_l );
	putMem( RO, mld_l );
	putMem( RO, mld_s );
	putMem( RO, bu_l );
	putMem( RO, bo_l );
	putMem( RO, bu_s );
	putMem( RO, bo_s );
	putMem( RO, iteration_loop );
	putMem( RO, iteration_finish );
	putMem( RO, getframebits );
	putMem( RO, bitsPerFrame );
	putMem( RO, nSample );
	putMem( RO, framesize );
	putMem( RO, nChannel );
	putMem( RO, out_samplerate );
	putMem( RO, channels_out );
	putMem( RO, mode_gr );
	putMem( RO, ixend );
	putMem( RO, nCPU );
	putMem( RO, nThread );
	putMem( RO, tl );
	putMem( RO, printf );
	putMem( RO, frac_SpF );
	putMem( RO, pow20 );
	putMem( RO, ipow20 );
	putMem( RO, pow43 );
	putMem( RO, decay );
	putMem( RO, sideinfo_len );
	putMem( RO, scalefac_band );
	putMem( RO, cw_upper_index );
	putMem( RO, npart_l );
	putMem( RO, npart_s );
	putMem( RO, npart_l_orig );
	putMem( RO, npart_s_orig );
	putMem( RO, npart_l_pre_max );
	putMem( RO, emphasis );
	putMem( RO, samplerate_index );
	putMem( RO, input_device_handle );
	putMem( RO, output_device_handle );
	putMem( RO, read_input_device );
	putMem( RO, write_output_device );
	putMem( RO, mode );
	putMem( RO, version );
	putMem( RO, lame_encode_init );
	putMem( RO, noise_shaping );
	putMem( RO, noise_shaping_amp );
	putMem( RO, noise_shaping_stop );
	putMem( RO, use_best_huffman );
	putMem( RO, use_filtering );
	putMem( RO, use_psy );
	putMem( RO, VBR );
	putMem( RO, bWriteVbrTag );
	putMem( RO, bWriteLameTag );
	putMem( RO, debug );
	putMem( RO, silent );
	putMem( RO, dummybyte1 );
	putMem( RO, dummybyte2 );
	putMem( RO, brate );
	putMem( RO, VBR_min_bitrate );
	putMem( RO, VBR_max_bitrate );
	putMem( RO, VBR_dbQ );
	putMem( RO, amp_lowpass );
	putMem( RO, amp_highpass );
	putMem( RO, lowpass_band );
	putMem( RO, highpass_band );
	putMem( RO, lowpass_start_band );
	putMem( RO, lowpass_end_band );
	putMem( RO, highpass_start_band );
	putMem( RO, highpass_end_band );
	putMem( RW, subband_buf );
	putMem( RW, sample_buf );
	putMem( RW, cw );
	putMem( RW, thm );
	putMem( RW, en );
	putMem( RW, tot_ener );
	putMem( RW, pe );
	putMem( RW, nb_12 );
	putMem( RW, fr0 );
	putMem( RW, fr1 );
	putMem( RW, fr2 );
	putMem( RW, fr3 );
	putMem( RW, num_samples_read );
	putMem( RW, num_samples_padding );
	putMem( RW, num_samples_to_encode );
	putMem( RW, frameNum );
	putMem( RW, totalframes );
	putMem( RW, ms_ratio );
	putMem( RW, slot_lag );
	putMem( RW, OldValue );
	putMem( RW, CurrentStep );
	putMem( RW, bv_scf );
	putMem( RW, sfb21_extra );
	putMem( RW, header );
	putMem( RW, h_ptr );
	putMem( RW, w_ptr );
	putMem( RW, ancillary_flag );
	putMem( RW, bs );
	putMem( RW, ResvSize );
	putMem( RW, ResvMax );
	putMem( RW, main_data_begin );
	putMem( RW, sav );
	putMem( RW, ms_ratio_s_old );
	putMem( RW, ms_ratio_l_old );
	putMem( RW, blocktype_old );
	putMem( RW, ATH );
	putMem( RW, OutputDoneSize );
	putMem( RW, VBR_seek_table );


	putOff( ath_t, use_adjust );
	putOff( ath_t, adjust );
	putOff( ath_t, adjust_limit );
	putOff( ath_t, decay );
	putOff( ath_t, l );
	putOff( ath_t, s );
	putOff( ath_t, cb );
	putOff( be_t, quality );
	putOff( be_t, inpFreqHz );
	putOff( be_t, rateKbps );
	putOff( be_t, CBR_bitrate );
	putOff( be_t, unit );
	putOff( be_t, VBR_q );
	putOff( be_t, VBR_min_bitrate_kbps );
	putOff( be_t, VBR_max_bitrate_kbps );
	putOff( be_t, inFileName );
	putOff( be_t, outFileName );
	putOff( be_t, nZeroStreamSize );
	putOff( be_t, TotalFrameSize );
	putOff( be_t, riffInfo );
	putOff( be_t, addtagInfo );
	putOff( be_t, lowpassfreq );
	putOff( be_t, highpassfreq );
	putOff( be_t, lowpasswidth );
	putOff( be_t, highpasswidth );
	putOff( be_t, lowpass1 );
	putOff( be_t, lowpass2 );
	putOff( be_t, highpass1 );
	putOff( be_t, highpass2 );
	putOff( be_t, open_input_device );
	putOff( be_t, close_input_device );
	putOff( be_t, open_input_format );
	putOff( be_t, open_input_filters );
	putOff( be_t, input_filter_count );
	putOff( be_t, open_output_device );
	putOff( be_t, seek_top_output_device );
	putOff( be_t, close_output_device );
	putOff( be_t, open_output_format );
	putOff( calc_noise_result_t, over_count );
	putOff( calc_noise_result_t, tot_noise );
	putOff( calc_noise_result_t, over_noise );
	putOff( gogo_thread_data_s, mfbuf );
	putOff( gogo_thread_data_s, sb );
	putOff( gogo_thread_data_s, sb_sample );
	putOff( gogo_thread_data_s, xr );
	putOff( gogo_thread_data_s, xr_sign );
	putOff( gogo_thread_data_s, xrpow );
	putOff( gogo_thread_data_s, work_xrpow );
	putOff( gogo_thread_data_s, xrpow_sum );
	putOff( gogo_thread_data_s, xrpow_max );
	putOff( gogo_thread_data_s, l3_enc );
	putOff( gogo_thread_data_s, scalefac );
	putOff( gogo_thread_data_s, ath_over );
	putOff( gogo_thread_data_s, l3_xmin );
	putOff( gogo_thread_data_s, wsamp_L );
	putOff( gogo_thread_data_s, wsamp_S );
	putOff( gogo_thread_data_s, psywork );
	putOff( gogo_thread_data_s, energy );
	putOff( gogo_thread_data_s, energy_s );
	putOff( gogo_thread_data_s, tot_ener );
	putOff( gogo_thread_data_s, l3_side );
	putOff( gogo_thread_data_s, next );
	putOff( gogo_thread_data_s, unaligned );
	putOff( gogo_thread_data_s, tid );
	putOff( gogo_thread_data_s, padding );
	putOff( gogo_thread_data_s, mode_ext );
	putOff( gogo_thread_data_s, bitrate_index );
	putOff( gogo_thread_data_s, ResvSize );
	putOff( gogo_thread_data_s, additional_part2_3_length );
	putOff( gr_info_s, part2_3_length );
	putOff( gr_info_s, big_values );
	putOff( gr_info_s, count1 );
	putOff( gr_info_s, global_gain );
	putOff( gr_info_s, scalefac_compress );
	putOff( gr_info_s, window_switching_flag );
	putOff( gr_info_s, block_type );
	putOff( gr_info_s, table_select );
	putOff( gr_info_s, subblock_gain );
	putOff( gr_info_s, region0_count );
	putOff( gr_info_s, region1_count );
	putOff( gr_info_s, preflag );
	putOff( gr_info_s, scalefac_scale );
	putOff( gr_info_s, count1table_select );
	putOff( gr_info_s, part2_length );
	putOff( gr_info_s, sfb_lmax );
	putOff( gr_info_s, sfb_smin );
	putOff( gr_info_s, count1bits );
	putOff( gr_info_s, sfb_partition_table );
	putOff( gr_info_s, slen );
	putOff( header_buf_t, write_period );
	putOff( header_buf_t, buf );
	putOff( header_buf_t, next );
	putOff( huffcodetab, xlen );
	putOff( huffcodetab, linmax );
	putOff( huffcodetab, table );
	putOff( huffcodetab, hlen );
	putOff( III_psy_ratio_s, thm );
	putOff( III_psy_ratio_s, en );
	putOff( III_psy_xmin_s, l );
	putOff( III_psy_xmin_s, dummy0 );
	putOff( III_psy_xmin_s, s );
	putOff( III_psy_xmin_s, dummy1 );
	putOff( III_scalefac, l );
	putOff( III_scalefac, dummy0 );
	putOff( III_scalefac, s );
	putOff( III_scalefac, dummy1 );
	putOff( III_side_info, resvDrain_pre );
	putOff( III_side_info, resvDrain_post );
	putOff( III_side_info, scfsi );
	putOff( III_side_info, tt );
	putOff( riffInfo_t, riffInfosLen );
	putOff( riffInfo_t, pRiffInfos );
	putOff( ro_t, window );
	putOff( ro_t, window_s );
	putOff( ro_t, minval );
	putOff( ro_t, s3_s );
	putOff( ro_t, s3_l );
	putOff( ro_t, s3ind );
	putOff( ro_t, s3ind_s );
	putOff( ro_t, SNR_s );
	putOff( ro_t, numlines_s );
	putOff( ro_t, numlines_l );
	putOff( ro_t, mld_l );
	putOff( ro_t, mld_s );
	putOff( ro_t, bu_l );
	putOff( ro_t, bo_l );
	putOff( ro_t, bu_s );
	putOff( ro_t, bo_s );
	putOff( ro_t, iteration_loop );
	putOff( ro_t, iteration_finish );
	putOff( ro_t, getframebits );
	putOff( ro_t, bitsPerFrame );
	putOff( ro_t, nSample );
	putOff( ro_t, framesize );
	putOff( ro_t, nChannel );
	putOff( ro_t, out_samplerate );
	putOff( ro_t, channels_out );
	putOff( ro_t, mode_gr );
	putOff( ro_t, ixend );
	putOff( ro_t, nCPU );
	putOff( ro_t, nThread );
	putOff( ro_t, tl );
	putOff( ro_t, printf );
	putOff( ro_t, frac_SpF );
	putOff( ro_t, pow20 );
	putOff( ro_t, ipow20 );
	putOff( ro_t, pow43 );
	putOff( ro_t, decay );
	putOff( ro_t, sideinfo_len );
	putOff( ro_t, scalefac_band );
	putOff( ro_t, cw_upper_index );
	putOff( ro_t, npart_l );
	putOff( ro_t, npart_s );
	putOff( ro_t, npart_l_orig );
	putOff( ro_t, npart_s_orig );
	putOff( ro_t, npart_l_pre_max );
	putOff( ro_t, emphasis );
	putOff( ro_t, samplerate_index );
	putOff( ro_t, input_device_handle );
	putOff( ro_t, output_device_handle );
	putOff( ro_t, read_input_device );
	putOff( ro_t, write_output_device );
	putOff( ro_t, mode );
	putOff( ro_t, version );
	putOff( ro_t, lame_encode_init );
	putOff( ro_t, noise_shaping );
	putOff( ro_t, noise_shaping_amp );
	putOff( ro_t, noise_shaping_stop );
	putOff( ro_t, use_best_huffman );
	putOff( ro_t, use_filtering );
	putOff( ro_t, use_psy );
	putOff( ro_t, VBR );
	putOff( ro_t, bWriteVbrTag );
	putOff( ro_t, bWriteLameTag );
	putOff( ro_t, debug );
	putOff( ro_t, silent );
	putOff( ro_t, dummybyte1 );
	putOff( ro_t, dummybyte2 );
	putOff( ro_t, brate );
	putOff( ro_t, VBR_min_bitrate );
	putOff( ro_t, VBR_max_bitrate );
	putOff( ro_t, VBR_dbQ );
	putOff( ro_t, amp_lowpass );
	putOff( ro_t, amp_highpass );
	putOff( ro_t, lowpass_band );
	putOff( ro_t, highpass_band );
	putOff( ro_t, lowpass_start_band );
	putOff( ro_t, lowpass_end_band );
	putOff( ro_t, highpass_start_band );
	putOff( ro_t, highpass_end_band );
	putOff( rw_t, subband_buf );
	putOff( rw_t, sample_buf );
	putOff( rw_t, cw );
	putOff( rw_t, thm );
	putOff( rw_t, en );
	putOff( rw_t, tot_ener );
	putOff( rw_t, pe );
	putOff( rw_t, nb_12 );
	putOff( rw_t, fr0 );
	putOff( rw_t, fr1 );
	putOff( rw_t, fr2 );
	putOff( rw_t, fr3 );
	putOff( rw_t, num_samples_read );
	putOff( rw_t, num_samples_padding );
	putOff( rw_t, num_samples_to_encode );
	putOff( rw_t, frameNum );
	putOff( rw_t, totalframes );
	putOff( rw_t, ms_ratio );
	putOff( rw_t, slot_lag );
	putOff( rw_t, OldValue );
	putOff( rw_t, CurrentStep );
	putOff( rw_t, bv_scf );
	putOff( rw_t, sfb21_extra );
	putOff( rw_t, header );
	putOff( rw_t, h_ptr );
	putOff( rw_t, w_ptr );
	putOff( rw_t, ancillary_flag );
	putOff( rw_t, bs );
	putOff( rw_t, ResvSize );
	putOff( rw_t, ResvMax );
	putOff( rw_t, main_data_begin );
	putOff( rw_t, sav );
	putOff( rw_t, ms_ratio_s_old );
	putOff( rw_t, ms_ratio_l_old );
	putOff( rw_t, blocktype_old );
	putOff( rw_t, ATH );
	putOff( rw_t, OutputDoneSize );
	putOff( rw_t, VBR_seek_table );
	putOff( scalefac_struct_s, l );
	putOff( scalefac_struct_s, s );
	putOff( vbr_seek_info_t, sum );
	putOff( vbr_seek_info_t, seen );
	putOff( vbr_seek_info_t, want );
	putOff( vbr_seek_info_t, pos );
	putOff( vbr_seek_info_t, size );
	putOff( vbr_seek_info_t, bag );
	putOff( VBRTAGDATA_s, h_id );
	putOff( VBRTAGDATA_s, samprate );
	putOff( VBRTAGDATA_s, flags );
	putOff( VBRTAGDATA_s, frames );
	putOff( VBRTAGDATA_s, bytes );
	putOff( VBRTAGDATA_s, vbr_scale );
	putOff( VBRTAGDATA_s, toc );
	putOff( VBRTAGDATA_s, headersize );
	putOff( VBRTAGDATA_s, enc_delay );
	putOff( VBRTAGDATA_s, enc_padding );


	putSize( ath_t );
	putSize( be_t );
	putSize( calc_noise_result_t );
	putSize( gogo_thread_data_s );
	putSize( gr_info_s );
	putSize( header_buf_t );
	putSize( huffcodetab );
	putSize( III_psy_ratio_s );
	putSize( III_psy_xmin_s );
	putSize( III_scalefac );
	putSize( III_side_info );
	putSize( riffInfo_t );
	putSize( ro_t );
	putSize( rw_t );
	putSize( scalefac_struct_s );
	putSize( vbr_seek_info_t );
	putSize( VBRTAGDATA_s );


	puts( "%endif ; DEF_GLOBAL_VARS" );
	return 0;
}
