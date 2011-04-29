/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 *
 *	[!] This is auto generated file by Makevfta.rb for gogo-no-coda. [!]
 */

#include "cpu.h"

#if   defined(CPU_PPC)
	extern void setup_fft( int unit );
	extern void setup_pow075( int unit );
#elif defined(CPU_I386)
	extern void setup_shiftoutpfb( int unit );
	extern void setup_dist_mfbuf( int unit );
	extern void setup_convolute_energy( int unit );
	extern void setup_choose_table( int unit );
	extern void setup_inner_psy_ch4( int unit );
	extern void setup_fft( int unit );
	extern void setup_mdct( int unit );
	extern void setup_quantize_xrpow_ISO( int unit );
	extern void setup_pow075( int unit );
	extern void setup_calc_noise_long( int unit );
	extern void setup_ms_convert( int unit );
#endif

void
setupUNIT(int unit)
{
#if   defined(CPU_PPC)
	setup_fft( unit );
	setup_pow075( unit );
#elif defined(CPU_I386)
	setup_shiftoutpfb( unit );
	setup_dist_mfbuf( unit );
	setup_convolute_energy( unit );
	setup_choose_table( unit );
	setup_inner_psy_ch4( unit );
	setup_fft( unit );
	setup_mdct( unit );
	setup_quantize_xrpow_ISO( unit );
	setup_pow075( unit );
	setup_calc_noise_long( unit );
	setup_ms_convert( unit );
#endif
} /* setupUNIT */
