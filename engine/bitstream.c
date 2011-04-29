/*
 *	MP3 bitstream Output interface for LAME / GOGO
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

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "config.h"
#include "global.h"
#include "lame.h"
#include "util.h"
#include "tables.h"
#include "bitstream.h"
#include "quantize.h"
#include "quantize_pvt.h"
#include "gogo_io.h"

/* This is the scfsi_band table from 2.4.2.7 of the IS */
const int scfsi_band[5] = { 0, 6, 11, 16, 21 };

/* unsigned int is at least this large:  */
/* we work with ints, so when doing bit manipulation, we limit
 * ourselves to MAX_LENGTH-2 just to be on the safe side */
#define MAX_LENGTH      32

#define putheader_bits() { \
	if (RW.w_ptr->write_period == 0) { \
		unsigned char	*src = RW.w_ptr->buf; \
		int		i = RO.sideinfo_len; \
		do { \
			*RW.bs.buf_byte_ptr++ = *src++; \
		} while(--i); \
		RW.w_ptr = RW.w_ptr->next; \
	} \
	RW.w_ptr->write_period--; \
}

/*
  Some combinations of bitrate, Fs, and stereo make it impossible to stuff
  out a frame using just main_data, due to the limited number of bits to
  indicate main_data_length. In these situations, we put stuffing bits into
  the ancillary data...
*/

static void drain_into_ancillary(int remainingBits)
{
	int	bit_buf = *RW.bs.buf_byte_ptr, bit_idx = RW.bs.bit_idx;
	assert(remainingBits >= 0);

	while (remainingBits >= 16) {
		putheader_bits();
		bit_buf <<= 8; bit_buf |= 0x47;
		*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);

		putheader_bits();
		bit_buf <<= 8; bit_buf |= 0x4F;
		*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);

		remainingBits -= 16;
	}
	for (; remainingBits >= 1; remainingBits -= 1) {
		bit_buf <<= 1; bit_buf |= RW.ancillary_flag; bit_idx++;
		RW.ancillary_flag ^= 1;
	}
	while(8 <= bit_idx){
		putheader_bits();
		bit_idx -= 8;
		*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);
	}
	assert(remainingBits == 0);
	assert(bit_idx < 8);
	*RW.bs.buf_byte_ptr = bit_buf;
	RW.bs.bit_idx = bit_idx;
}

static void drain_unused_part2_3(int remainingBits)
{
	int	bit_buf = *RW.bs.buf_byte_ptr, bit_idx = RW.bs.bit_idx;
	assert(remainingBits >= 0);

	while (remainingBits >= 16) {
		putheader_bits();
		bit_buf <<= 8; bit_buf |= 0xFF;
		*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);

		putheader_bits();
		bit_buf <<= 8; bit_buf |= 0xFF;
		*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);

		remainingBits -= 16;
	}
	for (; remainingBits >= 1; remainingBits -= 1) {
		bit_buf <<= 1; bit_buf |= 1; bit_idx++;
	}
	while(8 <= bit_idx){
		putheader_bits();
		bit_idx -= 8;
		*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);
	}
	assert(remainingBits == 0);
	assert(bit_idx < 8);
	*RW.bs.buf_byte_ptr = bit_buf;
	RW.bs.bit_idx = bit_idx;
}

/* 10Kclk@PIII */
/*  CRC部分削除 6Kclk */
/* まとめ書き 3.3Kclk */
/* 全面書き換え 2Kclk前後 */
static void encodeSideInfo2(gogo_thread_data * tl, int bitsPerFrame)
{
	III_side_info_t	*l3_side;
	int		gr, ch;
	unsigned char	*bp;
	int		data, bits;

	l3_side = &tl->l3_side;
	bp = RW.h_ptr->buf;

	*bp++ = 0xFF;
	data = (RO.out_samplerate < 16000) ? (0x0E << 1) : (0x0F << 1);
	            data |= RO.version;
	data <<= 2; data |= (4 - 3 /* layer3 */ );
	data <<= 1; data |= 1; /* !error_protection */
	*bp++ = data;
	data  = tl->bitrate_index;
	data <<= 2; data |= RO.samplerate_index;
	data <<= 1; data |= tl->padding;
	data <<= 1; /* 0 extension */
	*bp++ = data;
	data  = RO.mode;
	data <<= 2; data |= tl->mode_ext;
	data <<= 1; data |= COPYRIGHT;
	data <<= 1; data |= ORIGINAL;
	data <<= 2; data |= RO.emphasis;
	*bp++ = data;

	if (RO.version == 1) {
		/* MPEG1 */
		assert(RW.main_data_begin >= 0);
		data = RW.main_data_begin; bits = 9;
		bits -= 8; *bp++ = (data >> bits);

		if (RO.channels_out == 2) {
			data <<= 3; /* data |= l3_side->private_bits; */
			data <<= 1; data |= l3_side->scfsi[0][0];
			data <<= 1; data |= l3_side->scfsi[0][1];
			data <<= 1; data |= l3_side->scfsi[0][2];
			data <<= 1; data |= l3_side->scfsi[0][3];
			data <<= 1; data |= l3_side->scfsi[1][0];
			*bp++ = (data >> bits);

			data <<= 1; data |= l3_side->scfsi[1][1];
			data <<= 1; data |= l3_side->scfsi[1][2];
			data <<= 1; data |= l3_side->scfsi[1][3]; bits += 3;
		} else {
			data <<= 5; /* data |= l3_side->private_bits; */
			data <<= 1; data |= l3_side->scfsi[0][0];
			data <<= 1; data |= l3_side->scfsi[0][1];
			data <<= 1; data |= l3_side->scfsi[0][2];
			*bp++ = (data >> bits);

			data <<= 1; data |= l3_side->scfsi[0][3]; bits += 1;
		}

		for (gr = 0; gr < 2; gr++) {
			for (ch = 0; ch < RO.channels_out; ch++) {
				gr_info *gi = &tl->l3_side.tt[gr][ch];
				data <<= 12; data |= gi->part2_3_length + tl->additional_part2_3_length[gr][ch]; bits += 12;
				bits -= 8; *bp++ = (data >> bits);
				data <<=  9; data |= (gi->big_values/2); bits += 9;
				bits -= 8; *bp++ = (data >> bits);
				data <<=  8; data |= gi->global_gain;
				           *bp++ = (data >> bits);
				data <<=  4; data |= gi->scalefac_compress;
				data <<=  1; data |= gi->window_switching_flag; bits += 5;
				bits -= 8; *bp++ = (data >> bits);

				if (gi->table_select[0] == 14)
					gi->table_select[0] = 16;
				if (gi->table_select[1] == 14)
					gi->table_select[1] = 16;
				if (gi->window_switching_flag) {
					data <<= 2; data |= gi->block_type;
					data <<= 1;/* gi->mixed_block_flag = 0 */
					data <<= 5; data |= gi->table_select[0];
					           *bp++ = (data >> bits);
					data <<= 5; data |= gi->table_select[1];
					data <<= 3; data |= gi->subblock_gain[0];
					           *bp++ = (data >> bits);
					data <<= 3; data |= gi->subblock_gain[1];
					data <<= 3; data |= gi->subblock_gain[2]; bits += 6;
					bits -= 8; *bp++ = (data >> bits);
				} else {
					assert(gi->block_type == NORM_TYPE);
					assert(gi->region0_count < 16U && gi->region1_count < 8U);

					if (gi->table_select[2] == 14)
						gi->table_select[2] = 16;

					data <<= 5; data |= gi->table_select[0];
					data <<= 5; data |= gi->table_select[1]; bits += 10;
					bits -= 8; *bp++ = (data >> bits);
					data <<= 5; data |= gi->table_select[2];
					data <<= 4; data |= gi->region0_count; bits += 9;
					bits -= 8; *bp++ = (data >> bits);
					data <<= 3; data |= gi->region1_count; bits += 3;
					bits -= 8; *bp++ = (data >> bits);
				}
				data <<= 1; data |= gi->preflag;
				data <<= 1; data |= gi->scalefac_scale;
				data <<= 1; data |= gi->count1table_select; bits += 3;
			}
		}
	} else {
		/* MPEG2 */
		assert(RW.main_data_begin >= 0);
		*bp++ = RW.main_data_begin; 	/* writeheader(RW.main_data_begin, 8); */
		data = 0 /* l3_side->private_bits */; bits = RO.channels_out;

		gr = 0;
		for (ch = 0; ch < RO.channels_out; ch++) {
			gr_info *gi = &tl->l3_side.tt[gr][ch];

			data <<= 12; data |= gi->part2_3_length + tl->additional_part2_3_length[gr][ch]; bits += 12;
			bits -= 8; *bp++ = (data >> bits);
			data <<=  9; data |= (gi->big_values/2); bits += 9;
			bits -= 8; *bp++ = (data >> bits);
			data <<=  8; data |= gi->global_gain;
			           *bp++ = (data >> bits);
			data <<=  9; data |= gi->scalefac_compress; bits += 9;
			bits -= 8; *bp++ = (data >> bits);
			data <<=  1; data |= gi->window_switching_flag; bits += 1;
			bits -= 8; *bp++ = (data >> bits);

			if (gi->window_switching_flag) {
				data <<= 2; data |= gi->block_type;
				data <<= 1; data |= 0 /*gi->mixed_block_flag */;
				if (gi->table_select[0] == 14)
					gi->table_select[0] = 16;
				data <<= 5; data |= gi->table_select[0];
				           *bp++ = (data >> bits);

				if (gi->table_select[1] == 14)
					gi->table_select[1] = 16;
				data <<= 5; data |= gi->table_select[1];
				data <<= 3; data |= gi->subblock_gain[0];
				           *bp++ = (data >> bits);

				data <<= 3; data |= gi->subblock_gain[1];
				data <<= 3; data |= gi->subblock_gain[2]; bits += 6;
			} else {
				if (gi->table_select[0] == 14)
					gi->table_select[0] = 16;
				data <<= 5; data |= gi->table_select[0];
				if (gi->table_select[1] == 14)
					gi->table_select[1] = 16;
				data <<= 5; data |= gi->table_select[1]; bits += 10;
				bits -= 8; *bp++ = (data >> bits);

				if (gi->table_select[2] == 14)
					gi->table_select[2] = 16;
				data <<= 5; data |= gi->table_select[2];
				assert(gi->region0_count < 16U);
				assert(gi->region1_count < 8U);
				data <<= 4; data |= gi->region0_count; bits += 9;
				bits -= 8; *bp++ = (data >> bits);

				data <<= 3; data |= gi->region1_count; bits += 3;
			}
			data <<= 1; data |= gi->scalefac_scale;
			data <<= 1; data |= gi->count1table_select; bits += 2;
			bits -= 8; *bp++ = (data >> bits);
		}
	}
	assert((bits & 7) == 0);
	while(bits){
		bits -= 8;
		*bp++ = (data >> bits);
	}

	RW.h_ptr = RW.h_ptr->next;
	RW.h_ptr->write_period = bitsPerFrame/8 - RO.sideinfo_len;
	assert(RW.h_ptr != RW.w_ptr);	/* MAX_HEADER_BUF is too small. */
}								/* encodeSideInfo2 */

static int huffman_coder_count1(int *ix, gr_info * gi, uint32 * xr_sign)
{
	/* Write count1 area */
	const struct huffcodetab *h = &ht[gi->count1table_select + 32];
	int i, bits = 0;
	int	bit_buf = *RW.bs.buf_byte_ptr, bit_idx = RW.bs.bit_idx;

	ix += gi->big_values;
	xr_sign += gi->big_values;
	assert(gi->count1table_select < 2);

	for (i = (gi->count1 - gi->big_values) / 4; i > 0; --i) {
		int huffbits = 0;
		int p = 0;
		if (ix[0]) {
			p += 8;
			huffbits -= xr_sign[0];
		}
		if (ix[1]) {
			p += 4;
			huffbits = huffbits * 2 - xr_sign[1];
		}
		if (ix[2]) {
			p += 2;
			huffbits = huffbits * 2 - xr_sign[2];
		}
		if (ix[3]) {
			p += 1;
			huffbits = huffbits * 2 - xr_sign[3];
		}
		xr_sign += 4;
		ix += 4;

		bit_buf <<= h->hlen[p]; bit_buf |= (huffbits + h->table[p]);
		bit_idx += h->hlen[p];
		bits += h->hlen[p];
		while(8 <= bit_idx){
			putheader_bits();
			bit_idx -= 8;
			*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);
		}
	}
	assert(bit_idx < 8);
	*RW.bs.buf_byte_ptr = bit_buf;
	RW.bs.bit_idx = bit_idx;
	return bits;
}

/* Implements the pseudocode of page 98 of the IS */
/* NEED asm */
static void Huffmancodebits(int tableindex, int start, int end, int *ix, uint32 * xr_sign)
{
	const struct huffcodetab *h = ht + tableindex;
	int	i;
	/* int	bits = 0; */
	int	bit_buf = *RW.bs.buf_byte_ptr, bit_idx = RW.bs.bit_idx;

	assert(tableindex < 32);
	if (!tableindex)
		return /* 0 */;

/* 6100clk -> 3200clk*/
	if (tableindex > 15) {
		int	linbits = h->xlen;

		/* use ESC-words */
		for (i = start; i < end; i += 2) {
			int	x1 = ix[i], x2 = ix[i + 1];
			int	code = 0, cbits = 0;
			int	xbits1 = 0, ext1 = 0, xbits2 = 0, ext2 = 0;

			if (x1 != 0) {
				if (x1 > 14) {
					int linbits_x1 = x1 - 15;
					assert(linbits_x1 <= h->linmax);
					ext1 = linbits_x1;
					xbits1 = linbits;
					x1 = 15;
				}
				ext1 <<= 1;
				ext1 |= (xr_sign[i    ])? 1: 0;
				cbits--;
				xbits1++;
			}

			if (x2 != 0) {
				if (x2 > 14) {
					int linbits_x2 = x2 - 15;
					assert(linbits_x2 <= h->linmax);
					ext2 = linbits_x2;
					xbits2 = linbits;
					x2 = 15;
				}
				ext2 <<= 1;
				ext2 |= (xr_sign[i + 1])? 1: 0;
				cbits--;
				xbits2++;
			}

			assert((x1 | x2) < 16u);

			x1 = x1 * 16 + x2;

			code = h->table[x1];
			cbits += h->hlen[x1];

			/* bits += (cbits + xbits1 + xbits2) */;
				/* 最大でも 19(-2) + 13(+1) + 13(+1) = 45 */
				/* 64bit分のレジスタがあればwhile()は1個で充分 */

			bit_buf <<= cbits;
			bit_buf |= code;
			bit_idx += cbits;
			while (8 <= bit_idx) {
				putheader_bits();
				bit_idx -= 8;
				*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);
			}

			bit_buf <<= xbits1;
			bit_buf |= ext1;
			bit_idx += xbits1;
			while (8 <= bit_idx) {
				putheader_bits();
				bit_idx -= 8;
				*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);
			}

			bit_buf <<= xbits2;
			bit_buf |= ext2;
			bit_idx += xbits2;
			while (8 <= bit_idx) {
				putheader_bits();
				bit_idx -= 8;
				*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);
			}
		}
	}else{
		int	xlen = h->xlen;

		assert(tableindex > 0);
		for (i = start; i < end; i += 2) {
			int	x, x1 = ix[i], x2 = ix[i + 1];
			int	code, cbits;

			assert((x1 | x2) < 16u);
			x = x1 * xlen + x2;
			code = h->table[x]; /* t1HB..t15HB, 1 <= h->table[x1] <= 125 */
			cbits = h->hlen[x];	/* 1 <= h->hlen[] <= 21 */
			/* bits += cbits */;

			if (x1 != 0) {
				code <<= 1;
				code |= (xr_sign[i    ])? 1: 0;
			}
			if (x2 != 0) {
				code <<= 1;
				code |= (xr_sign[i + 1])? 1: 0;
			}

			bit_buf <<= cbits;
			bit_buf |= code;
			bit_idx += cbits;
			assert(bit_idx < MAX_LENGTH);
			while (8 <= bit_idx) {
				putheader_bits();
				bit_idx -= 8;
				*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);
			}
		}
	}
	assert(bit_idx < 8);
	*RW.bs.buf_byte_ptr = bit_buf;
	RW.bs.bit_idx = bit_idx;
	return /* bits */;
}

/*
  Note the discussion of huffmancodebits() on pages 28
  and 29 of the IS, as well as the definitions of the side
  information on pages 26 and 27.
  */
static void ShortHuffmancodebits(int *ix, gr_info * gi, uint32 * xr_sign)
{
	/* int bits; */
	int region1Start;

	region1Start = 3 * RO.scalefac_band.s[3];
	if (region1Start > gi->big_values)
		region1Start = gi->big_values;

	/* short blocks do not have a region2 */
	/* bits = */ Huffmancodebits(gi->table_select[0], 0, region1Start, ix, xr_sign);
	/* bits += */ Huffmancodebits(gi->table_select[1], region1Start, gi->big_values, ix, xr_sign);
	return /* bits */;
}

static void LongHuffmancodebits(int *ix, gr_info * gi, uint32 * xr_sign)
{
	int i, bigvalues /* , bits */;
	int region1Start, region2Start;

	bigvalues = gi->big_values;
	assert(0 <= bigvalues && bigvalues <= 576);

	i = gi->region0_count + 1;
	assert(i < 23);
	region1Start = RO.scalefac_band.l[i];
	i += gi->region1_count + 1;
	assert(i < 23);
	region2Start = RO.scalefac_band.l[i];

	if (region1Start > bigvalues)
		region1Start = bigvalues;

	if (region2Start > bigvalues)
		region2Start = bigvalues;

	/* bits  = */ Huffmancodebits(gi->table_select[0], 0, region1Start, ix, xr_sign);
	/* bits += */ Huffmancodebits(gi->table_select[1], region1Start, region2Start, ix, xr_sign);
	/* bits += */ Huffmancodebits(gi->table_select[2], region2Start, bigvalues, ix, xr_sign);
	return /* bits */;
}

static void writeMainData(gogo_thread_data * tl)
{
	int	gr, ch, sfb /*, data_bits, scale_bits, tot_bits = 0 */;
	int	bit_buf, bit_idx;

	if (RO.version == 1) {
		/* MPEG 1 */
		for (gr = 0; gr < 2; gr++) {
			for (ch = 0; ch < RO.channels_out; ch++) {
				gr_info *gi = &tl->l3_side.tt[gr][ch];

				int slen1 = slen1_tab[gi->scalefac_compress];	/* 0..4 */
				int slen2 = slen2_tab[gi->scalefac_compress];	/* 0..3 */
				/* data_bits = 0 */;
				/* scale_bits = 0 */;

				if (gi->block_type == SHORT_TYPE) {
					bit_buf = *RW.bs.buf_byte_ptr;
					bit_idx = RW.bs.bit_idx;
					for (sfb = 0; sfb < SBPSY_s; sfb++) {
						int slen = sfb < 6 ? slen1 : slen2;	/* 0..4 */

						assert(tl->scalefac[gr][ch].s[sfb][0] >= 0);
						assert(tl->scalefac[gr][ch].s[sfb][1] >= 0);
						assert(tl->scalefac[gr][ch].s[sfb][2] >= 0);

						bit_buf <<= slen;
						bit_buf |= tl->scalefac[gr][ch].s[sfb][0];
						bit_buf <<= slen;
						bit_buf |= tl->scalefac[gr][ch].s[sfb][1];
						bit_buf <<= slen;
						bit_buf |= tl->scalefac[gr][ch].s[sfb][2];
						bit_idx    += 3*slen;
						/* scale_bits += 3 * slen */;
						assert(bit_idx <= MAX_LENGTH);
						while(8 <= bit_idx){
							putheader_bits();
							bit_idx -= 8;
							*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);
						}
					}
					assert(bit_idx < 8);
					*RW.bs.buf_byte_ptr = bit_buf;
					RW.bs.bit_idx = bit_idx;
					/* data_bits += */ ShortHuffmancodebits(tl->l3_enc[gr][ch], gi, tl->xr_sign[gr][ch]);
				} else {
					int i;

					bit_buf = *RW.bs.buf_byte_ptr;
					bit_idx = RW.bs.bit_idx;
					for (i = 0; i < sizeof(scfsi_band) / sizeof(int) - 1; i++) {
						if (gr != 0 && tl->l3_side.scfsi[ch][i])
							continue;

						for (sfb = scfsi_band[i]; sfb < scfsi_band[i + 1]; sfb++) {
							int slen = sfb < 11 ? slen1 : slen2;	/* 0..4 */

							assert(tl->scalefac[gr][ch].l[sfb] >= 0);
							bit_buf <<= slen;
							bit_buf |= tl->scalefac[gr][ch].l[sfb];
							bit_idx += slen;
							/* scale_bits += slen */;
						}
						assert(bit_idx <= MAX_LENGTH);
						while(8 <= bit_idx){
							putheader_bits();
							bit_idx -= 8;
							*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);
						}
					}
					assert(bit_idx < 8);
					*RW.bs.buf_byte_ptr = bit_buf;
					RW.bs.bit_idx = bit_idx;
					/* data_bits += */ LongHuffmancodebits(tl->l3_enc[gr][ch], gi, tl->xr_sign[gr][ch]);
				}
				/* data_bits += */ huffman_coder_count1(tl->l3_enc[gr][ch], gi, tl->xr_sign[gr][ch]);
				/* does bitcount in quantize.c agree with actual bit count? */
				/* assert(data_bits == gi->part2_3_length - gi->part2_length) */;
				/* assert(scale_bits == gi->part2_length) */;
				/* tot_bits += scale_bits + data_bits */;
				if (tl->additional_part2_3_length[gr][ch]) {
					if (576 < tl->additional_part2_3_length[gr][ch]) {
						drain_unused_part2_3(576);
						drain_into_ancillary(tl->additional_part2_3_length[gr][ch] - 576);
					}
					else {
						drain_unused_part2_3(tl->additional_part2_3_length[gr][ch]);
					}
				}
			}					/* for ch */
		}						/* for gr */
	} else {
		/* MPEG 2 */
		gr = 0;
		for (ch = 0; ch < RO.channels_out; ch++) {
			gr_info *gi = &tl->l3_side.tt[gr][ch];

			int i, sfb_partition;
			assert(gi->sfb_partition_table);
			/* data_bits = 0 */;
			/* scale_bits = 0 */;

			sfb = 0;
			sfb_partition = 0;
			if (gi->block_type == SHORT_TYPE) {
				bit_buf = *RW.bs.buf_byte_ptr;
				bit_idx = RW.bs.bit_idx;
				for (; sfb_partition < 4; sfb_partition++) {
					int sfbs = gi->sfb_partition_table[sfb_partition];
					int slen = gi->slen[sfb_partition];	/* 0..4 */
					for (i = 0; i < sfbs; i += 3, sfb++) {
						bit_buf <<= slen;
						bit_buf |= Max(tl->scalefac[gr][ch].s[sfb][0], 0U);
						bit_buf <<= slen;
						bit_buf |= Max(tl->scalefac[gr][ch].s[sfb][1], 0U);
						bit_buf <<= slen;
						bit_buf |= Max(tl->scalefac[gr][ch].s[sfb][2], 0U);
						bit_idx    += 3 * slen;
						/* scale_bits += 3 * slen */;
						assert(bit_idx <= MAX_LENGTH);
						while(8 <= bit_idx){
							putheader_bits();
							bit_idx -= 8;
							*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);
						}
					}
				}
				assert(bit_idx < 8);
				*RW.bs.buf_byte_ptr = bit_buf;
				RW.bs.bit_idx = bit_idx;
				/* data_bits += */ ShortHuffmancodebits(tl->l3_enc[gr][ch], gi, tl->xr_sign[gr][ch]);
			} else {
				bit_buf = *RW.bs.buf_byte_ptr;
				bit_idx = RW.bs.bit_idx;
				for (; sfb_partition < 4; sfb_partition++) {
					int sfbs = gi->sfb_partition_table[sfb_partition];
					int slen = gi->slen[sfb_partition];	/* 0..4 */
					for (i = 0; i < sfbs; i++, sfb++) {
						bit_buf <<= slen;
						bit_buf |= Max(tl->scalefac[gr][ch].l[sfb], 0U);
						bit_idx += slen;
						/* scale_bits += slen */;
						if(8 <= bit_idx){
							putheader_bits();
							bit_idx -= 8;
							*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);
						}
					}
				}
				assert(bit_idx < 8);
				*RW.bs.buf_byte_ptr = bit_buf;
				RW.bs.bit_idx = bit_idx;
				/* data_bits += */ LongHuffmancodebits(tl->l3_enc[gr][ch], gi, tl->xr_sign[gr][ch]);
			}
			/* data_bits += */ huffman_coder_count1(tl->l3_enc[gr][ch], gi, tl->xr_sign[gr][ch]);

			/* does bitcount in quantize.c agree with actual bit count? */
			/* assert(data_bits == gi->part2_3_length - gi->part2_length) */;
			/* assert(scale_bits == gi->part2_length) */;
			/* tot_bits += scale_bits + data_bits */;
			if (tl->additional_part2_3_length[gr][ch]) {
				if (576 < tl->additional_part2_3_length[gr][ch]) {
					drain_unused_part2_3(576);
					drain_into_ancillary(tl->additional_part2_3_length[gr][ch] - 576);
				}
				else {
					drain_unused_part2_3(tl->additional_part2_3_length[gr][ch]);
				}
			}
		} /* for ch */
	} /* for gf */
	return /* tot_bits */;
}

void flush_bitstream(void)
{
	int flushbytes;
	Header_buf_t	*last_ptr;

	/* add this many bits to bitstream so we can flush all headers */
	last_ptr = RW.w_ptr;
	flushbytes = last_ptr->write_period;
	while(last_ptr != RW.h_ptr){
		last_ptr = last_ptr->next;
		flushbytes += last_ptr->write_period;
	}

	/* finally, add some bits so that the last frame is complete
	 * these bits are not necessary to decode the last frame, but
	 * some decoders will ignore last frame if these bits are missing 
	 */
	assert(flushbytes >= 0);
	drain_into_ancillary(flushbytes*8);
}

/* write 1 byte into the bit stream, ignoring frame headers */
void add_dummy_byte(unsigned char val)
{
	int	bit_buf = *RW.bs.buf_byte_ptr, bit_idx = RW.bs.bit_idx;

	bit_buf <<= 8; bit_buf |= val;
	*RW.bs.buf_byte_ptr++ = (bit_buf >> bit_idx);
	*RW.bs.buf_byte_ptr = bit_buf;
}

/*
  format_bitstream()

  This is called after a frame of audio has been quantized and coded.
  It will write the encoded audio to the bitstream. Note that
  from a layer3 encoder's perspective the bit stream is primarily
  a series of main_data() blocks, with header and side information
  inserted at the proper locations to maintain framing. (See Figure A.7
  in the IS).
  */
/* 42Kclk */
int format_bitstream(gogo_thread_data * tl, int bitsPerFrame)
{
	int		bytes;
	III_side_info_t	*l3_side;
	unsigned char	*bp;
	Header_buf_t	*hp;
	int gr, ch;
	int added_part2_3_length = 0;

	l3_side = &tl->l3_side;
	drain_into_ancillary(l3_side->resvDrain_pre);	/* 8*n bits */

	if (0 < l3_side->resvDrain_post) {
		int tartget_addtional_part2_3_length = l3_side->resvDrain_post;
		for (gr = 0; gr < RO.mode_gr; gr++) {
			for (ch = 0; ch < RO.channels_out; ch++) {
				tl->additional_part2_3_length[gr][ch] = ((4095-l3_side->tt[gr][ch].part2_3_length) < tartget_addtional_part2_3_length)? (4095-l3_side->tt[gr][ch].part2_3_length): tartget_addtional_part2_3_length;
				tartget_addtional_part2_3_length -= tl->additional_part2_3_length[gr][ch];
				added_part2_3_length += tl->additional_part2_3_length[gr][ch];
			}
		}
	}
	else {
		for (gr = 0; gr < RO.mode_gr; gr++) {
			for (ch = 0; ch < RO.channels_out; ch++) {
				tl->additional_part2_3_length[gr][ch] = 0;
			}
		}
	}
	encodeSideInfo2(tl, bitsPerFrame);

	assert(RW.bs.bit_idx == 0);
	bp = RW.bs.buf_byte_ptr;
	hp = RW.w_ptr;

	writeMainData(tl);
	drain_into_ancillary(l3_side->resvDrain_post - added_part2_3_length);

	bytes = RO.sideinfo_len + (RW.bs.buf_byte_ptr - bp);
	while(hp != RW.w_ptr){
		bytes -= RO.sideinfo_len;
		hp = hp->next;
	}

	RW.main_data_begin += bitsPerFrame/8 - bytes;
	assert(RW.main_data_begin * 8 == tl->ResvSize);

	return 0;
}

int put_mp3(void)
{
	int minimum;

	assert(RW.bs.bit_idx == 0);
	minimum = (RW.bs.buf_byte_ptr - RW.bs.buf);
	if (minimum == 0)
		return 0;
	assert(minimum <= LAME_MAXMP3BUFFER);
	RW.bs.buf_byte_ptr = RW.bs.buf;
	RW.bs.bit_idx = 0;
	if (writeData(RW.bs.buf, minimum) != minimum)
		return -1;
	return minimum;
}

void init_bit_stream_w(void)
{
	int	i;

	for(i = 0; i < MAX_HEADER_BUF-1; i++){
		RW.header[i].next = &RW.header[i+1];
	}
	RW.header[MAX_HEADER_BUF-1].next = &RW.header[0];
	RW.h_ptr = RW.w_ptr = &RW.header[0];
	RW.header[0].write_period = 0;

	RW.bs.buf_byte_ptr = RW.bs.buf;
	RW.bs.bit_idx = 0;
}

/* end of bitstream.c */
