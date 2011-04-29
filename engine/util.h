/*
 *	lame utility library include file
 *
 *	Copyright (c) 1999 Albert L Faber
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

#ifndef LAME_UTIL_H
#define LAME_UTIL_H

#define MAX_BITS 4095

/***********************************************************************
*
*  Global Function Prototype Declarations
*
***********************************************************************/
extern int            BitrateIndex(int, int);
extern int            FindNearestBitrate(int,int);
extern int            map2MP3Frequency(int freq);
int SmpFrqIndex ( int sample_freq, unsigned char* const version );
void adjust_ATH(FLOAT8 tot_ener[2][4]);
extern FLOAT8         ATHformula(FLOAT8 f);
extern FLOAT8         freq2bark(FLOAT8 freq);

void CBR_getframebits(gogo_thread_data *tl, int *bitsPerFrame, int *mean_bits);
void VBRABR_getframebits(gogo_thread_data *tl, int *bitsPerFrame, int *mean_bits);

int select_kth_int(int b[], int N, int k);

#endif /* LAME_UTIL_H */
