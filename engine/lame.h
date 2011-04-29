/*
 *	Interface to GOGO / MP3 LAME encoding engine
 *
 *	Copyright (c) 1999 Mark Taylor
 *	Copyright (c) 2001,2002,2003 gogo-developer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef LAME_LAME_H
#define LAME_LAME_H

#include <stdio.h>
#include <stdarg.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(WIN32)
#undef CDECL
#define CDECL _cdecl
#else
#define CDECL
#endif

/* MPEG modes -- このヘッダのenum値は 変更してはならない*/
typedef enum MPEG_mode_e {
  STEREO=0,
  JOINT_STEREO,
  DUAL_CHANNEL,   /* LAME doesn't supports this! */
  MONO,
  NOT_SET,
  MAX_INDICATOR   /* Don't use this! It's used for sanity checks. */ 
} MPEG_mode;


/***********************************************************************
 *
 *  The LAME API
 *  These functions should be called, in this order, for each
 *  MP3 file to be encoded 
 *
 ***********************************************************************/


/*
 * REQUIRED:
 * initialize the encoder.  sets default for all encoder paramters,
 */
void CDECL lame_init(void);

/*
 * OPTIONAL:
 * set as needed to override defaults
 */


/********************************************************************
 *  general control parameters
 ***********************************************************************/

/*
 * REQUIRED:
 * sets more internal configuration based on data provided above.
 * returns !ME_NOERR if something failed.
 */
MERET lame_init_params(void);

/***********************************************************************
*
*  list of valid bitrates [kbps] & sample frequencies [Hz].
*  first index: 0: MPEG-2   values  (sample frequencies 16...24 kHz) 
*               1: MPEG-1   values  (sample frequencies 32...48 kHz)
*               2: MPEG-2.5 values  (sample frequencies  8...12 kHz)
***********************************************************************/
extern const int      bitrate_table    [2] [16];
extern const int      samplerate_table [2] [ 4];

#if defined(__cplusplus)
}
#endif
#endif /* LAME_LAME_H */
