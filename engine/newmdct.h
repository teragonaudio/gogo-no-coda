/*
 *	New Modified DCT include file
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

#ifndef LAME_NEWMDCT_H
#define LAME_NEWMDCT_H

void pfb(float (*subband)[18][SBLIMIT], float (*mfbuf)[4][576]);
void mdct(float (*xr)[576], float (*subband0)[576], float (*subband1)[576], int block_type[2]);

#endif /* LAME_NEWMDCT_H */
