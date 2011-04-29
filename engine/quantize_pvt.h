/*
 *	quantize_pvt include file
 *
 *	Copyright (c) 1999 Takehiro TOMINAGA
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

#ifndef LAME_QUANTIZE_PVT_H
#define LAME_QUANTIZE_PVT_H

extern const int nr_of_sfb_block[6][3][4];
extern const char pretab[SBMAX_l+2];
extern const char slen1_tab[16];
extern const char slen2_tab[16];

extern const scalefac_struct sfBandIndex[6];

#define CALC_NOISE_WITHOUT_LOG  
/**
 *  calc_noise_result
 */
typedef struct calc_noise_result_t {
	int over_count;		/* number of quantization noise > masking */
	/* このメンバ(tot_noise, over_noise)の順序を仮定する */
	/* アラインは気にしない */
#ifdef CALC_NOISE_WITHOUT_LOG
	unsigned int tot_noise;		/* sum of all quantization noise */
	unsigned int over_noise;		/* sum of quantization noise > masking */
#else
	float	tot_noise;		/* sum of all quantization noise */
	float	over_noise;		/* sum of quantization noise > masking */
#endif
} calc_noise_result;

int     on_pe (FLOAT8 pe[2][2], gr_info (*l3_side_tt)[2], int targ_bits[2], int mean_bits, int gr);

void    reduce_side (int targ_bits[2], FLOAT8 ms_ener_ratio, int mean_bits,
                     int max_bits);

void    iteration_init (void);


int calc_xmin_short( const FLOAT8                xr [576],
        const III_psy_ratio * const ratio,
              III_psy_xmin  * const l3_xmin,
		float masking_lower );

int calc_xmin_long( const FLOAT8                xr [576],
        const III_psy_ratio * const ratio,
              III_psy_xmin  * const l3_xmin,
		float masking_lower );

int  calc_noise( const FLOAT8                    xr [576],
        const int                       ix [576],
        const gr_info           * const cod_info,
        const III_psy_xmin      * const l3_xmin, 
        const III_scalefac_t    * const scalefac,
              III_psy_xmin      * xfsf,
              calc_noise_result * const res );


/* takehiro.c */
int count_bits_long(const int ix[576], gr_info * const gi);

void best_huffman_divide( gr_info * const gi, int *const ix );

void best_scalefac_store( const int gr, const int ch,
		  int			 l3_enc[2][2][576],
		  III_side_info_t * const l3_side,
		  III_scalefac_t		  scalefac[2][2] );

int scale_bitcount( III_scalefac_t * const scalefac, gr_info * const cod_info);
int scale_bitcount_lsf( III_scalefac_t * const scalefac, gr_info * const cod_info);

void huffman_init(void);

#define LARGE_BITS 100000

#define CALC_NOISE_TYPE const int max_index, const FLOAT8 xr [576], const int ix [576], const gr_info *const cod_info, const III_psy_xmin *const l3_xmin,  const III_scalefac_t *const scalefac, III_psy_xmin *xfsf, calc_noise_result *const res

#endif /* LAME_QUANTIZE_PVT_H */
