/*
	This is auto generated file by Makevfta.rb for gogo-no-coda.
*/

#ifndef GOGO_VFTA_H
#define GOGO_VFTA_H

#include "quantize_pvt.h"

#if   defined(CPU_PPC)
	extern void ms_convert_C(FLOAT8* srcl, FLOAT8* srcr, int n);
	#define ms_convert ms_convert_C
	extern void pow075_C(float xr[576], float xrpow[576], float *psum, float *pmax, uint32 *xr_sign);
	#define pow075 pow075_C
	extern void shiftoutpfb_C(void* dest, void* src);
	#define shiftoutpfb shiftoutpfb_C
	extern void fht_C(float * fz, int n);
	#define fht fht_C
	extern void fft_short_C(float (*wsamp)[3][BLKSIZE_s], float (*mfbuf)[4][576]);
	#define fft_short fft_short_C
	extern void fft_long_C(float (*wsamp)[BLKSIZE], float (*mfbuf)[4][576]);
	#define fft_long fft_long_C
	extern void fht_vector4_C(float * fz, int n);
	#define fht_vector4 fht_vector4_C
	extern void fft_short_vector4_C(float *wsamp, float *mfbuf);
	#define fft_short_vector4 fft_short_vector4_C
	extern void fft_long_vector4_C(float *wsamp, float *mfbuf);
	#define fft_long_vector4 fft_long_vector4_C
	extern void fft_prepare_C(float *dest, const float *src, int count);
	#define fft_prepare fft_prepare_C
	extern void psy_prepare_C(float *dest, const float *src, int count);
	#define psy_prepare psy_prepare_C
	extern void dist_mfbuf_C(float (*mfbuf)[4][576]);
	#define dist_mfbuf dist_mfbuf_C
	extern void convolute_energy_C(const int typeFlag, float *eb, float *cb, float *thr, float *nb_12);
	#define convolute_energy convolute_energy_C
	extern int choose_table_C(const int *ix, const int *end, int *s);
	#define choose_table choose_table_C
	extern void inner_psy_sub1_C(gogo_thread_data *tl);
	#define inner_psy_sub1 inner_psy_sub1_C
	extern void inner_psy_sub2_C(gogo_thread_data *tl);
	#define inner_psy_sub2 inner_psy_sub2_C
	extern void inner_psy_sub3_C(float *eb, float *cb, float *thr);
	#define inner_psy_sub3 inner_psy_sub3_C
	extern void inner_psy_sub4_C(gogo_thread_data *tl);
	#define inner_psy_sub4 inner_psy_sub4_C
	extern void inner_psy_sub5_C(gogo_thread_data *tl);
	#define inner_psy_sub5 inner_psy_sub5_C
	extern void inner_psy_sub6_C(gogo_thread_data *tl);
	#define inner_psy_sub6 inner_psy_sub6_C
	extern void mdct_long_C(FLOAT8 * out, FLOAT8 * in);
	#define mdct_long mdct_long_C
	extern void window_subband_sub1_C(const float *x1, float *a);
	#define window_subband_sub1 window_subband_sub1_C
	extern void quantize_xrpow_ISO_C(const FLOAT8 xr[576], int ix[576], float *istepPtr);
	#define quantize_xrpow_ISO quantize_xrpow_ISO_C
	extern void calc_noise_long_C(CALC_NOISE_TYPE);
	#define calc_noise_long calc_noise_long_C
#elif defined(CPU_I386)
	EXT void (*ms_convert)(FLOAT8* srcl, FLOAT8* srcr, int n);
	EXT void (*pow075)(float xr[576], float xrpow[576], float *psum, float *pmax, uint32 *xr_sign);
	EXT void (*shiftoutpfb)(void* dest, void* src);
	EXT void (*fht)(float * fz, int n);
	EXT void (*fft_short)(float (*wsamp)[3][BLKSIZE_s], float (*mfbuf)[4][576]);
	EXT void (*fft_long)(float (*wsamp)[BLKSIZE], float (*mfbuf)[4][576]);
	EXT void (*fht_vector4)(float * fz, int n);
	EXT void (*fft_short_vector4)(float *wsamp, float *mfbuf);
	EXT void (*fft_long_vector4)(float *wsamp, float *mfbuf);
	EXT void (*fft_prepare)(float *dest, const float *src, int count);
	EXT void (*psy_prepare)(float *dest, const float *src, int count);
	EXT void (*dist_mfbuf)(float (*mfbuf)[4][576]);
	EXT void (*convolute_energy)(const int typeFlag, float *eb, float *cb, float *thr, float *nb_12);
	EXT int (*choose_table)(const int *ix, const int *end, int *s);
	EXT void (*inner_psy_sub1)(gogo_thread_data *tl);
	EXT void (*inner_psy_sub2)(gogo_thread_data *tl);
	EXT void (*inner_psy_sub3)(float *eb, float *cb, float *thr);
	EXT void (*inner_psy_sub4)(gogo_thread_data *tl);
	EXT void (*inner_psy_sub5)(gogo_thread_data *tl);
	EXT void (*inner_psy_sub6)(gogo_thread_data *tl);
	EXT void (*mdct_long)(FLOAT8 * out, FLOAT8 * in);
	EXT void (*window_subband_sub1)(const float *x1, float *a);
	EXT void (*quantize_xrpow_ISO)(const FLOAT8 xr[576], int ix[576], float *istepPtr);
	EXT void (*calc_noise_long)(CALC_NOISE_TYPE);
#endif

#endif // GOGO_VFTA_H
