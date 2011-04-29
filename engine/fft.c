/*
   ** FFT and FHT routines
   **  Copyright 1988, 1993; Ron Mayer
   **  Copyright (c) 2001,2002,2003 gogo-developer
   **  
   **  fht(fz,n);
   **      Does a hartley transform of "n" points in the array "fz".
   **      
   ** NOTE: This routine uses at least 2 patented algorithms, and may be
   **       under the restrictions of a bunch of different organizations.
   **       Although I wrote it completely myself; it is kind of a derivative
   **       of a routine I once authored and released under the GPL, so it
   **       may fall under the free software foundation's restrictions;
   **       it was worked on as a Stanford Univ project, so they claim
   **       some rights to it; it was further optimized at work here, so
   **       I think this company claims parts of it.  The patents are
   **       held by R. Bracewell (the FHT algorithm) and O. Buneman (the
   **       trig generator), both at Stanford Univ.
   **       If it were up to me, I'd say go do whatever you want with it;
   **       but it would be polite to give credit to the following people
   **       if you use this anywhere:
   **           Euler     - probable inventor of the fourier transform.
   **           Gauss     - probable inventor of the FFT.
   **           Hartley   - probable inventor of the hartley transform.
   **           Buneman   - for a really cool trig generator
   **           Mayer(me) - for authoring this particular version and
   **                       including all the optimizations in one package.
   **       Thanks,
   **       Ron Mayer; mayer@acuson.com
   ** and added some optimization by
   **           Mather    - idea of using lookup table
   **           Takehiro  - some dirty hack for speed up
 */

#include <math.h>

#include "config.h"
#include "global.h"

#include "vfta.h"

#if defined(CPU_I386)
void fht_C(float * fz, int n);
void fft_short_C(float (*wsamp)[3][BLKSIZE_s], float (*mfbuf)[4][576]);
void fft_long_C(float (*wsamp)[BLKSIZE], float (*mfbuf)[4][576]);

# ifdef USE_VECTOR4_FHT
void fht_vector4_C(float * fz, int n);
void fht_vector4_SSE(float * fz, int n);
void fft_short_vector4_C( float *wsamp, float *mfbuf );
void fft_short_vector4_SSE( float *wsamp, float *mfbuf );
void fft_long_vector4_C( float *wsamp, float *mfbuf );
void fft_long_vector4_SSE( float *wsamp, float *mfbuf );
void fft_prepare_SSE(float *dest, const float *src, int count);
void psy_prepare_SSE(float *dest, const float *src, int count);
#  endif // USE_VECTOR4_FHT

#  ifdef USE_GOGO2_FHT
void gogo2_fht_SSE(float * fz, int n);
void gogo2_fht_3DN(float * fz, int n);
void gogo2_fht_C(float * fz, int n);
void gogo2_fft_short_C(float (*wsamp)[3][BLKSIZE_s], float (*mfbuf)[4][576]);
void gogo2_fft_long_C(float (*wsamp)[BLKSIZE], float (*mfbuf)[4][576]);
#  endif // USE_GOGO2_FHT
#endif

#define TRI_SIZE (5-1)		/* 1024 =  4**5 */

static const float
costab[TRI_SIZE * 2] =
{
	9.238795325112867e-01F, 3.826834323650898e-01F,
	9.951847266721969e-01F, 9.801714032956060e-02F,
	9.996988186962042e-01F, 2.454122852291229e-02F,
	9.999811752826011e-01F, 6.135884649154475e-03F
};

static const unsigned
char rv_tbl[] =
{
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe
};

#define ch01(index)  (buffer[index])

#define ml00(f)	(RO.window[i        ] * f(i))
#define ml10(f)	(RO.window[i + 0x200] * f(i + 0x200))
#define ml20(f)	(RO.window[i + 0x100] * f(i + 0x100))
#define ml30(f)	(RO.window[i + 0x300] * f(i + 0x300))

#define ml01(f)	(RO.window[i + 0x001] * f(i + 0x001))
#define ml11(f)	(RO.window[i + 0x201] * f(i + 0x201))
#define ml21(f)	(RO.window[i + 0x101] * f(i + 0x101))
#define ml31(f)	(RO.window[i + 0x301] * f(i + 0x301))

#define ms00(f)	(RO.window_s[i       ] * f(i + k))
#define ms10(f)	(RO.window_s[0x7f - i] * f(i + k + 0x80))
#define ms20(f)	(RO.window_s[i + 0x40] * f(i + k + 0x40))
#define ms30(f)	(RO.window_s[0x3f - i] * f(i + k + 0xc0))

#define ms01(f)	(RO.window_s[i + 0x01] * f(i + k + 0x01))
#define ms11(f)	(RO.window_s[0x7e - i] * f(i + k + 0x81))
#define ms21(f)	(RO.window_s[i + 0x41] * f(i + k + 0x41))
#define ms31(f)	(RO.window_s[0x3e - i] * f(i + k + 0xc1))

void
fht_C(float * fz, int n)
{
	const float *tri = costab;
	int k4;
	float *fi, *fn, *gi;

	fn = fz + n;
	k4 = 4;
	do {
		float s1, c1;
		int i, k1, k2, k3, kx;
		kx = k4 >> 1;
		k1 = k4;
		k2 = k4 << 1;
		k3 = k2 + k1;
		k4 = k2 << 1;
		fi = fz;
		gi = fi + kx;
		do {
			float f0, f1, f2, f3;
			f1 = fi[0] - fi[k1];
			f0 = fi[0] + fi[k1];
			f3 = fi[k2] - fi[k3];
			f2 = fi[k2] + fi[k3];
			fi[k2] = f0 - f2;
			fi[0] = f0 + f2;
			fi[k3] = f1 - f3;
			fi[k1] = f1 + f3;
			f1 = gi[0] - gi[k1];
			f0 = gi[0] + gi[k1];
			f3 = SQRT2 * gi[k3];
			f2 = SQRT2 * gi[k2];
			gi[k2] = f0 - f2;
			gi[0] = f0 + f2;
			gi[k3] = f1 - f3;
			gi[k1] = f1 + f3;
			gi += k4;
			fi += k4;
		} while (fi < fn);
		c1 = tri[0];
		s1 = tri[1];
		for (i = 1; i < kx; i++) {
			float c2, s2;
			c2 = 1 - (2 * s1) * s1;
			s2 = (2 * s1) * c1;
			fi = fz + i;
			gi = fz + k1 - i;
			do {
				float a, b, g0, f0, f1, g1, f2, g2,
				 f3, g3;
				b = s2 * fi[k1] - c2 * gi[k1];
				a = c2 * fi[k1] + s2 * gi[k1];
				f1 = fi[0] - a;
				f0 = fi[0] + a;
				g1 = gi[0] - b;
				g0 = gi[0] + b;
				b = s2 * fi[k3] - c2 * gi[k3];
				a = c2 * fi[k3] + s2 * gi[k3];
				f3 = fi[k2] - a;
				f2 = fi[k2] + a;
				g3 = gi[k2] - b;
				g2 = gi[k2] + b;
				b = s1 * f2 - c1 * g3;
				a = c1 * f2 + s1 * g3;
				fi[k2] = f0 - a;
				fi[0] = f0 + a;
				gi[k3] = g1 - b;
				gi[k1] = g1 + b;
				b = c1 * g2 - s1 * f3;
				a = s1 * g2 + c1 * f3;
				gi[k2] = g0 - a;
				gi[0] = g0 + a;
				fi[k3] = f1 - b;
				fi[k1] = f1 + b;
				gi += k4;
				fi += k4;
			} while (fi < fn);
			c2 = c1;
			c1 = c2 * tri[0] - s1 * tri[1];
			s1 = c2 * tri[1] + s1 * tri[0];
		}
		tri += 2;
	} while (k4 < n);
}

void
fft_short_C(float (*wsamp)[3][BLKSIZE_s], float (*mfbuf)[4][576])
{
	int	i, j, k, b, ch;
	float	*buffer, *x;

	for(ch = 0; ch < RO.channels_out; ch++) {
		buffer = &mfbuf[ch][0][576 - FFTOFFSET];
		for (b = 0; b < 3; b++) {
			x = &wsamp[ch][b][BLKSIZE_s / 2];

			k = (576 / 3) * (b + 1);
			j = BLKSIZE_s / 8 - 1;
			do {
				float f0, f1, f2, f3, w;

				i = rv_tbl[j << 2];

				f0 = ms00(ch01);
				w = ms10(ch01);
				f1 = f0 - w;
				f0 = f0 + w;
				f2 = ms20(ch01);
				w = ms30(ch01);
				f3 = f2 - w;
				f2 = f2 + w;

				x -= 4;
				x[0] = f0 + f2;
				x[2] = f0 - f2;
				x[1] = f1 + f3;
				x[3] = f1 - f3;

				f0 = ms01(ch01);
				w = ms11(ch01);
				f1 = f0 - w;
				f0 = f0 + w;
				f2 = ms21(ch01);
				w = ms31(ch01);
				f3 = f2 - w;
				f2 = f2 + w;

				x[BLKSIZE_s / 2 + 0] = f0 + f2;
				x[BLKSIZE_s / 2 + 2] = f0 - f2;
				x[BLKSIZE_s / 2 + 1] = f1 + f3;
				x[BLKSIZE_s / 2 + 3] = f1 - f3;
			} while (--j >= 0);
	
			fht(x, BLKSIZE_s);
		}
	}
}

void
fft_long_C(float (*wsamp)[BLKSIZE], float (*mfbuf)[4][576])
{
	int	i, ch, jj;
	float	*buffer, *x;

	for(ch = 0; ch < RO.channels_out; ch++) {
		buffer = &mfbuf[ch][0][576 - FFTOFFSET];
		x = wsamp[ch] + BLKSIZE/2;
		jj = BLKSIZE/8 - 1;

		do {
			float f0, f1, f2, f3, w;

			i = rv_tbl[jj];
			f0 = ml00(ch01);
			w = ml10(ch01);
			f1 = f0 - w;
			f0 = f0 + w;
			f2 = ml20(ch01);
			w = ml30(ch01);
			f3 = f2 - w;
			f2 = f2 + w;

			x -= 4;
			x[0] = f0 + f2;
			x[2] = f0 - f2;
			x[1] = f1 + f3;
			x[3] = f1 - f3;

			f0 = ml01(ch01);
			w = ml11(ch01);
			f1 = f0 - w;
			f0 = f0 + w;
			f2 = ml21(ch01);
			w = ml31(ch01);
			f3 = f2 - w;
			f2 = f2 + w;

			x[BLKSIZE / 2 + 0] = f0 + f2;
			x[BLKSIZE / 2 + 2] = f0 - f2;
			x[BLKSIZE / 2 + 1] = f1 + f3;
			x[BLKSIZE / 2 + 3] = f1 - f3;
		} while (--jj >= 0);

		fht(x, BLKSIZE);
	}
}

#ifdef USE_VECTOR4_FHT

#define CHN 4
void
fht_vector4_C(float * fz, int n)
{
	int k4;
	float *fi, *fn, *gi;
	const float *tri = costab;

	n *= 4;
	fn = &fz[n];
	for( fi = fz; fi < fn; fi += 4*4 ){
		int chn;
		for( chn = 0; chn < CHN; chn++ ){
			float f0, f1, f2, f3;
			f1 = fi[0*4+chn] - fi[1*4+chn];
			f0 = fi[0*4+chn] + fi[1*4+chn];
			f3 = fi[2*4+chn] - fi[3*4+chn];
			f2 = fi[2*4+chn] + fi[3*4+chn];
			fi[2*4+chn] = f0 - f2;
			fi[0*4+chn] = f0 + f2;
			fi[3*4+chn] = f1 - f3;
			fi[1*4+chn] = f1 + f3;
		}
	}

	k4 = 4 * 4;
	do{
		float s1, c1;
		int i, k1, k2, k3, kx;
		kx = k4 >> 1;
		k1 = k4;
		k2 = k4 << 1;
		k3 = k2 + k1;
		k4 = k2 << 1;
		fi = fz;
		gi = &fi[kx];
		do{
			int chn;
			for( chn = 0; chn < CHN; chn ++ ){
				float f0, f1, f2, f3;
				f1 = fi[ 0+chn] - fi[k1+chn];
				f0 = fi[ 0+chn] + fi[k1+chn];
				f3 = fi[k2+chn] - fi[k3+chn];
				f2 = fi[k2+chn] + fi[k3+chn];
				fi[k2+chn] = f0 - f2;
				fi[ 0+chn] = f0 + f2;
				fi[k3+chn] = f1 - f3;
				fi[k1+chn] = f1 + f3;
				f1 = gi[0+chn] - gi[k1+chn];
				f0 = gi[0+chn] + gi[k1+chn];
				f3 = SQRT2 * gi[k3+chn];
				f2 = SQRT2 * gi[k2+chn];
				gi[k2+chn] = f0 - f2;
				gi[ 0+chn] = f0 + f2;
				gi[k3+chn] = f1 - f3;
				gi[k1+chn] = f1 + f3;
			}
			gi += k4;
			fi += k4;
		}while(fi < fn);
		c1 = tri[0];
		s1 = tri[1];

		for( i = 4; i < kx; i += 4 ){
			float c2, s2;
//printf("i=%d k4=%d: %d(%d)\n", i/4, k4/4, (1024/(k4/4))*(i/4),tblIdx );
			c2 = 1 - (2 * s1) * s1;
			s2 = (2 * s1) * c1;

			fi = &fz[i];
			gi = &fz[(k1-i)];

			do{
				float a, b, g0, f0, f1, g1, f2, g2, f3, g3;
				int chn;
				for( chn = 0; chn < CHN; chn++ ){
					b = s2 * fi[k1+chn] - c2 * gi[k1+chn];
					a = c2 * fi[k1+chn] + s2 * gi[k1+chn];
					f1 = fi[0+chn] - a;
					f0 = fi[0+chn] + a;
					g1 = gi[0+chn] - b;
					g0 = gi[0+chn] + b;
					b = s2 * fi[k3+chn] - c2 * gi[k3+chn];
					a = c2 * fi[k3+chn] + s2 * gi[k3+chn];
					f3 = fi[k2+chn] - a;
					f2 = fi[k2+chn] + a;
					g3 = gi[k2+chn] - b;
					g2 = gi[k2+chn] + b;
					b = s1 * f2 - c1 * g3;
					a = c1 * f2 + s1 * g3;
					fi[k2+chn] = f0 - a;
					fi[ 0+chn] = f0 + a;
					gi[k3+chn] = g1 - b;
					gi[k1+chn] = g1 + b;
					b = c1 * g2 - s1 * f3;
					a = s1 * g2 + c1 * f3;
					gi[k2+chn] = g0 - a;
					gi[ 0+chn] = g0 + a;
					fi[k3+chn] = f1 - b;
					fi[k1+chn] = f1 + b;
				}
				gi += k4;
				fi += k4;
			}while( fi < fn );
			c2 = c1;
			c1 = c2 * tri[0] - s1 * tri[1];
			s1 = c2 * tri[1] + s1 * tri[0];
		}
		tri += 2;
	}while( k4 < n );
}

/* 13Kclk@PIII */
/* BLKSZIE_s = 256 */

void
fft_short_vector4_C( float *wsamp, float *mfbuf )
{
	int	i, j, b;
	float *buffer, *x, *win1, *win2;

	x = &wsamp[(BLKSIZE_s/2) * 4];
	buffer = &mfbuf[(576 - FFTOFFSET + 576/3)*4];

	for( b = 0; b < 3; b++ ){
		win1 = &RO.window_s[0];
		win2 = &RO.window_s[128-4];

		for( j = BLKSIZE_s/8 - 1; j >= 0; j-- ){
			int chn;

			i = rv_tbl[j << 2];
			for( chn = 0; chn < CHN; chn++ ){
				x[(0-4)*4+chn] = win1[0] * buffer[(i+0x00)*4+chn];
				x[(BLKSIZE_s/2+0-4)*4+chn] = win2[3] * buffer[(i+0x01)*4+chn];

				x[(1-4)*4+chn] = win1[1] * buffer[(i+0x80)*4+chn];
				x[(BLKSIZE_s/2+1-4)*4+chn] = win2[2] * buffer[(i+0x81)*4+chn];

				x[(2-4)*4+chn] = win1[2] * buffer[(i+0x40)*4+chn];
				x[(BLKSIZE_s/2+2-4)*4+chn] = win2[1] * buffer[(i+0x41)*4+chn];

				x[(3-4)*4+chn] = win1[3] * buffer[(i+0xc0)*4+chn];
				x[(BLKSIZE_s/2+3-4)*4+chn] = win2[0] * buffer[(i+0xc1)*4+chn];
			}
			x -= 4 * 4;
			win1 += 4;
			win2 -= 4;
		}
		fht_vector4(x, BLKSIZE_s);
		buffer += 576 / 3 * 4;
		x += BLKSIZE_s * 4 + 4*4 * BLKSIZE_s/8;
	}
}

void
fft_long_vector4_C( float *wsamp, float *mfbuf )
{
	int	i, j;
	float *buffer, *x, *win;

	buffer = &mfbuf[(576 - FFTOFFSET)*4];
	x = &wsamp[(BLKSIZE/2)*4];
	win = RO.window;

	for( j = BLKSIZE/8 - 1; j >= 0; j-- ){
		int chn;

		i = rv_tbl[j];
		for( chn = 0; chn < CHN; chn++ ){
			x[(0-4)*4+chn] = win[0] * buffer[(i+0x000)*4+chn];
			x[(BLKSIZE/2+0-4)*4+chn] = win[1] * buffer[(i+0x001)*4+chn];

			x[(1-4)*4+chn] = win[2] * buffer[(i+0x200)*4+chn];
			x[(BLKSIZE/2+1-4)*4+chn] = win[3] * buffer[(i+0x201)*4+chn];

			x[(2-4)*4+chn] = win[4] * buffer[(i+0x100)*4+chn];
			x[(BLKSIZE/2+2-4)*4+chn] = win[5] * buffer[(i+0x101)*4+chn];

			x[(3-4)*4+chn] = win[6] * buffer[(i+0x300)*4+chn];
			x[(BLKSIZE/2+3-4)*4+chn] = win[7] * buffer[(i+0x301)*4+chn];
		}
		win += 8;
		x -= 4 * 4;
	}
	fht_vector4(x, BLKSIZE);
}
#endif // USE_VECTOR4_FHT

#ifdef USE_GOGO2_FHT
void
gogo2_fht_C(float * fz, int n)
{
	const float *tri = costab;
	int k4;
	float *fi, *fn, *gi;

	fn = fz + n;
	for (fi=fz;fi<fn;fi+=4)
	{
      float f0,f1,f2,f3;
      f1     = fi[0 ]-fi[1 ];
      f0     = fi[0 ]+fi[1 ];
      f3     = fi[2 ]-fi[3 ];
      f2     = fi[2 ]+fi[3 ];
      fi[2 ] = (f0-f2);	
      fi[0 ] = (f0+f2);
      fi[3 ] = (f1-f3);	
      fi[1 ] = (f1+f3);
	}
	k4 = 4;
	do {
		float s1, c1;
		int i, k1, k2, k3, kx;
		kx = k4 >> 1;
		k1 = k4;
		k2 = k4 << 1;
		k3 = k2 + k1;
		k4 = k2 << 1;
		fi = fz;
		gi = fi + kx;
		do {
			float f0, f1, f2, f3;
			f1 = fi[0] - fi[k1];
			f0 = fi[0] + fi[k1];
			f3 = fi[k2] - fi[k3];
			f2 = fi[k2] + fi[k3];
			fi[k2] = f0 - f2;
			fi[0] = f0 + f2;
			fi[k3] = f1 - f3;
			fi[k1] = f1 + f3;
			f1 = gi[0] - gi[k1];
			f0 = gi[0] + gi[k1];
			f3 = SQRT2 * gi[k3];
			f2 = SQRT2 * gi[k2];
			gi[k2] = f0 - f2;
			gi[0] = f0 + f2;
			gi[k3] = f1 - f3;
			gi[k1] = f1 + f3;
			gi += k4;
			fi += k4;
		} while (fi < fn);
		c1 = tri[0];
		s1 = tri[1];
		for (i = 1; i < kx; i++) {
			float c2, s2;
			c2 = 1 - (2 * s1) * s1;
			s2 = (2 * s1) * c1;
			fi = fz + i;
			gi = fz + k1 - i;
			do {
				float a, b, g0, f0, f1, g1, f2, g2,
				 f3, g3;
				b = s2 * fi[k1] - c2 * gi[k1];
				a = c2 * fi[k1] + s2 * gi[k1];
				f1 = fi[0] - a;
				f0 = fi[0] + a;
				g1 = gi[0] - b;
				g0 = gi[0] + b;
				b = s2 * fi[k3] - c2 * gi[k3];
				a = c2 * fi[k3] + s2 * gi[k3];
				f3 = fi[k2] - a;
				f2 = fi[k2] + a;
				g3 = gi[k2] - b;
				g2 = gi[k2] + b;
				b = s1 * f2 - c1 * g3;
				a = c1 * f2 + s1 * g3;
				fi[k2] = f0 - a;
				fi[0] = f0 + a;
				gi[k3] = g1 - b;
				gi[k1] = g1 + b;
				b = c1 * g2 - s1 * f3;
				a = s1 * g2 + c1 * f3;
				gi[k2] = g0 - a;
				gi[0] = g0 + a;
				fi[k3] = f1 - b;
				fi[k1] = f1 + b;
				gi += k4;
				fi += k4;
			} while (fi < fn);
			c2 = c1;
			c1 = c2 * tri[0] - s1 * tri[1];
			s1 = c2 * tri[1] + s1 * tri[0];
		}
		tri += 2;
	} while (k4 < n);
}

void
gogo2_fft_short_C(float (*wsamp)[3][BLKSIZE_s], float (*mfbuf)[4][576])
{
	int	i, j, k, b, ch;
	float	*buffer, *x;

	for(ch = 0; ch < RO.channels_out; ch++) {
		buffer = &mfbuf[ch][0][576 - FFTOFFSET];
		for (b = 0; b < 3; b++) {
			x = &wsamp[ch][b][BLKSIZE_s / 2];

			k = (576 / 3) * (b + 1);
			j = BLKSIZE_s / 8 - 1;
			do {
				i = rv_tbl[j << 2];

				x -= 4;
				x[0] = ms00(ch01);
				x[1] = ms10(ch01);
				x[2] = ms20(ch01);
				x[3] = ms30(ch01);

				x[BLKSIZE_s / 2 + 0] = ms01(ch01);
				x[BLKSIZE_s / 2 + 1] = ms11(ch01);
				x[BLKSIZE_s / 2 + 2] = ms21(ch01);
				x[BLKSIZE_s / 2 + 3] = ms31(ch01);
			} while (--j >= 0);
	
			fht(x, BLKSIZE_s);
		}
	}
}

void
gogo2_fft_long_C(float (*wsamp)[BLKSIZE], float (*mfbuf)[4][576])
{
	int	i, ch, jj;
	float	*buffer, *x;

	for(ch = 0; ch < RO.channels_out; ch++) {
		buffer = &mfbuf[ch][0][576 - FFTOFFSET];
		x = wsamp[ch] + BLKSIZE/2;
		jj = BLKSIZE/8 - 1;

		do {
			i = rv_tbl[jj];

			x -= 4;
			x[0] = ml00(ch01);
			x[1] = ml10(ch01);
			x[2] = ml20(ch01);
			x[3] = ml30(ch01);

			x[BLKSIZE / 2 + 0] = ml01(ch01);
			x[BLKSIZE / 2 + 1] = ml11(ch01);
			x[BLKSIZE / 2 + 2] = ml21(ch01);
			x[BLKSIZE / 2 + 3] = ml31(ch01);
		} while (--jj >= 0);

		fht(x, BLKSIZE);
	}
}
#endif // USE_GOGO2_FHT

void
setup_fft(int unit)
{
	float *window = &RO.window[0];
	float *window_s = &RO.window_s[0];
	int i;

#if defined(CPU_I386)
	fht			= fht_C;
	fft_short	= fft_short_C;
	fft_long	= fft_long_C;

#ifdef USE_VECTOR4_FHT

	fht_vector4	= NULL;
	fft_prepare = NULL;
	psy_prepare = NULL;
	if( RO.mode_gr == 2 && RO.channels_out == 2 ){
		if( unit & MU_tSSE ){
			fht_vector4			= fht_vector4_SSE;
			fft_short_vector4	= fft_short_vector4_SSE;
			fft_long_vector4	= fft_long_vector4_SSE;
			fft_prepare			= fft_prepare_SSE;
			psy_prepare			= psy_prepare_SSE;
		}
	}

#endif // USE_VECTOR4_FHT

#ifdef USE_GOGO2_FHT

	fht			= gogo2_fht_C;
	fft_short	= gogo2_fft_short_C;
	fft_long	= gogo2_fft_long_C;

	if( unit & MU_tSSE ){
		fht		= gogo2_fht_SSE;
	} else
	if( (unit & MU_tMMX) && (unit & MU_t3DN) ){
		fht		= gogo2_fht_3DN;
	}
#endif // USE_GOGO2_FHT

#ifdef USE_VECTOR4_FHT
	if( fht_vector4 ){
		int j;
		float tmp[BLKSIZE];
		for (i = 0; i < BLKSIZE; i++){
			/* blackman window */
			tmp[i] = 0.42 - 0.5 * cos(2 * PI * (i + .5) / BLKSIZE) +
			    0.08 * cos(4 * PI * (i + .5) / BLKSIZE);
		}
	/* tmp[i] = tmp[BLKSIZE-1-i] */
		for( i = 0; i < BLKSIZE/8; i++ ){
			j = rv_tbl[BLKSIZE/8 - 1 - i];
			window[i*8+0] = tmp[j+0x000];
			window[i*8+1] = tmp[j+0x001];
			window[i*8+2] = tmp[j+0x200];
			window[i*8+3] = tmp[j+0x201];
			window[i*8+4] = tmp[j+0x100];
			window[i*8+5] = tmp[j+0x101];
			window[i*8+6] = tmp[j+0x300];
			window[i*8+7] = tmp[j+0x301];
		}
		for( i = 0; i < BLKSIZE_s / 2; i++ ){
			tmp[i] = 0.5 * (1.0 - cos(2.0 * PI * (i + 0.5) / BLKSIZE_s));
		}
		for( j = 0; j < BLKSIZE_s / 8; j++ ){
			i = rv_tbl[(BLKSIZE_s /8 - 1 -j) << 2];
			window_s[j*4+0] = tmp[i + 0x00];
			window_s[j*4+1] = tmp[0x7F - i];
			window_s[j*4+2] = tmp[i + 0x40];
			window_s[j*4+3] = tmp[0x3F - i];
		}
	}else
#endif
#endif
	{
		for (i = 0; i < BLKSIZE; i++){
			/* blackman window */
			window[i] = 0.42 - 0.5 * cos(2 * PI * (i + .5) / BLKSIZE) +
			    0.08 * cos(4 * PI * (i + .5) / BLKSIZE);
		}
		for (i = 0; i < BLKSIZE_s / 2; i++){
			window_s[i] = 0.5 * (1.0 - cos(2.0 * PI * (i + 0.5) / BLKSIZE_s));
		}
	}
}
