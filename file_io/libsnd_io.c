/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#include <assert.h>
#include "common.h"
#include "global.h"
#include "gogo_io.h"
#include "sndfile.h"
#if defined(WIN32)
#include "libsnd\common.h"
#else
#include "libsnd/common.h"
#endif

//	libsnd 入力関連 
static
size_t	read_input_libsnd8(void* handle, void* buf, size_t nLength)
{
	size_t read_sample_count;
	int i;

	read_sample_count = sf_read_short((SNDFILE*)handle, buf, nLength/sizeof(short));
	for ( i = 0; i < read_sample_count; i++ ) {
		((short*)buf)[i] = ((short*)buf)[i] << 8;
	}
	return read_sample_count*sizeof(short);
}

static
size_t	read_input_libsnd16(void* handle, void* buf, size_t nLength)
{
	return sf_read_short((SNDFILE*)handle, buf, nLength/sizeof(short))*sizeof(short);
}

static void close_input_libsnd(DEVICE_HANDLE handle)
{
	sf_close((SNDFILE*)handle);
}

MERET open_input_libsnd(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close)
{
	SNDFILE *sndfile;
	SF_INFO sfinfo;

	assert(*handle == NULL);

	memset( &sfinfo, 0, sizeof( sfinfo ) );
	sndfile = sf_open_read(file_name, &sfinfo);
	if (sndfile == NULL) {
		int	err	=	sf_getrerr(NULL);
		switch( err ){
		case SFE_MALLOC_FAILED:
			return	ME_NOMEMORY;
		case SFE_OPEN_FAILED:
			return ME_INFILE_NOFOUND;
		case SFE_UNKNOWN_FORMAT:
			return ME_WAVETYPE_ERR;
		default:
			if( err >= SFE_WAV_NO_RIFF )
				return	ME_WAVETYPE_ERR;
			else
				return	ME_INTERNALERROR;
		}
	}

	pcm_format->nBit  = 16;
	pcm_format->nChn  = sfinfo.channels;
	pcm_format->nFreq = sfinfo.samplerate;
	pcm_format->nSize = sfinfo.samples*sfinfo.channels*sizeof(short);

	if (sfinfo.pcmbitwidth == 8) {
		*read  = read_input_libsnd8;
	} else if (sfinfo.pcmbitwidth == 16) {
		*read  = read_input_libsnd16;
	} else {
		sf_close( sndfile );
		return ME_WAVETYPE_ERR;
	}
	*close = close_input_libsnd;
	*handle = sndfile;

	//	PCM フォーマットの設定は自力でやっているので
	//	RO.open_input_format での処理は要らない
	BE.open_input_format = NULL;

	return ME_NOERR;
}

