/*
 *	Layer 3 side include file
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

#ifndef LAME_L3SIDE_H
#define LAME_L3SIDE_H

#include "encoder.h"
#include "machine.h"

/* Layer III side information. */

typedef FLOAT8	D576[576];
typedef int	I576[576];
typedef FLOAT8	D192_3[192][3];
typedef int	I192_3[192][3];


/** 
 * scalefac_struct
 */
typedef struct scalefac_struct_s
{
   int l[1+SBMAX_l];
   int s[1+SBMAX_s];
} scalefac_struct;


/** 
 * III_psy_xmin
 */
typedef struct III_psy_xmin_s {
	FLOAT8	l[SBMAX_l];
	int		dummy0[4-(SBMAX_l & 3)];	/* =2 for align */
	FLOAT8	s[SBMAX_s][3];
	int		dummy1[4-((SBMAX_s*3) & 3)];/* = 1 for align */
} III_psy_xmin;

/** 
 * III_psy_ratio
 */
typedef struct III_psy_ratio_s {
    III_psy_xmin thm;
    III_psy_xmin en;
} III_psy_ratio;

/** 
 * gr_info
 */
typedef struct gr_info_s {
	int part2_3_length;
	int big_values;
	int count1;
 	int global_gain;
	int scalefac_compress;
	int window_switching_flag;
	int block_type;	
	int table_select[3];
	 int subblock_gain[3];
	 int region0_count;
	 int region1_count;
	 int preflag;
	 int scalefac_scale;
	 int count1table_select;

	 int part2_length;
	 int sfb_lmax;
	 int sfb_smin;
	 int count1bits;
	/* added for LSF */
	 const int *sfb_partition_table;
	 int slen[4];
} gr_info;

/** 
 * III_side_info_t
 */
typedef struct III_side_info {
//	int main_data_begin; 	// shared, 初期化, ResvFrameEnd()とformat_bitstream()で参照＆変更
/*	int private_bits;	 0固定 */
	int resvDrain_pre;	// local, ResvFrameEnd()で変更、format_bitstream()で参照
	int resvDrain_post;	// local, ResvFrameEnd()で変更、format_bitstream()で参照
	int scfsi[2][4];	// local, best_scalefac_store()で変更, format_bitstream()で参照
	gr_info	tt[2][2];
} III_side_info_t;

/**
 * Layer III scale factors. 
 * note: there are only SBPSY_l=(SBMAX_l-1) and SBPSY_s=(SBMAX_s-1) scalefactors.
 * Dont know why these would be dimensioned SBMAX_l and SBMAX-s 
 */
typedef struct III_scalefac {
	int l[SBMAX_l];            /* [cb] */
	int	dummy0[4-(SBMAX_l & 3)];	/* =2 for align */
	int s[SBMAX_s][3];         /* [window][cb] */
	int	dummy1[4-((SBMAX_s*3) & 3)];/* = 1 for align */
} III_scalefac_t;  /* [gr][ch] */

#endif
