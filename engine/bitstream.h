/*
 *	MP3 bitstream Output interface for LAME
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

#ifndef LAME_BITSTREAM_H
#define LAME_BITSTREAM_H

int format_bitstream(gogo_thread_data *tl, int i);
void flush_bitstream(void);
void add_dummy_byte(unsigned char val);

int put_mp3(void);
void init_bit_stream_w(void);

#endif
