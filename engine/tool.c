/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#include <assert.h>
#include <stdarg.h>
#if defined(WIN32)
	#include <fcntl.h> /* for _setmode */
#endif

#include "config.h"
#include "global.h"

#include "gogo_io.h"

/******************   for file   ******************/

/* RIFF_DATA_CHUNK */
struct CK_RIFF {
	long	chunk;				/* "RIFF" */
	long	size;				/* sizeof "RIFF" */
} ;

struct CK_FMT {
	long	chunk;				/* "WAVE" */
	long	form;				/* "fmt " */
	long	size;				/* 16 */
	short	formatID;			/*  1 = RAW WAVE */
	short	num_of_channel;		/* 1 or 2 */
	long	freqHz;				/* 44100 etc. */
	long	avg_bytes_par_sec;	/* freq * elementsize */
	short	block_size;			/* byte per element */
	short	bits_par_sample;	/* 16 */
} ;

struct CK_DATA {
	long	chunk;				/* "DATA" */
	long	size;				/* sizeof "DATA" */
} ;

struct CK_WAVE {
	struct	CK_RIFF		riff;
	struct	CK_FMT		fmt;
} ;

int getWaveInfo(unsigned int *size, int *bit, int *freq, int *channel){
	struct CK_WAVE wav;
	char prevbuf[8];

	*size = *bit = *freq = *channel = 0;

	if(readData(&wav, sizeof(wav)) != sizeof(wav)) return ERR;

	if(memcmp(&wav.riff.chunk, "RIFF", 4)
		|| memcmp(&wav.fmt.chunk, "WAVEfmt ", 8)
		|| LE2uint32(wav.fmt.formatID) != 1) return ERR;

	memset(prevbuf, 0, sizeof(prevbuf));

	do{
		memcpy(prevbuf, prevbuf+1, sizeof(prevbuf) - 1);
		if(readData(&prevbuf[sizeof(prevbuf)-1], 1) != 1) return ERR;
	}while(memcmp(prevbuf, "data", 4));

	/* ↑ここでprevbuf='data...'(...はDWORDでdataチャンクのサイズ)となる */

	*size    = *(unsigned int *)(prevbuf + 4);
	*freq    = wav.fmt.freqHz;
	*bit     = wav.fmt.bits_par_sample;
	*channel = wav.fmt.num_of_channel; 

	*size    = LE2uint32(*size); /* 動くのか? */
	*freq    = LE2uint32(*freq);
	*bit     = LE2uint16(*bit);
	*channel = LE2uint16(*channel);

	return NOERR;
} /* getWaveInfo */

void setBinaryMode(FILE *fp){
#if !defined(__unix__)
	#if defined(__EMX__)			/* OS/2 emx + gcc */
		_fsetmode(fp, "b");
	#elif defined(__WATCOMC__) || defined(__BORLANDC__)
		setmode(_fileno(fp), _O_BINARY);
	#elif defined(GO32)				/* MS-DOS DJGPP gcc */
		setmode(fileno(fp), O_BINARY);
	#elif defined(__HIGHC__)
		_setmode(fp, _BINARY);
	#elif defined(__CYGWIN__)
		setmode(fileno(fp), _O_BINARY);
	#elif defined(WIN32)
		_setmode(_fileno(fp), _O_BINARY);
	#else
		#error "write setmode for your compiler"
	#endif
#endif
} /* setBinaryMode */


/* change suffix of src to ".ext"  */
/* suppose sizeof(dest)>=sizeof(src) */
// UIと2重に持ってますが、切り離しのため消さないこと。
int changeSuffix(char *dest, const char *src, const char *ext, const size_t maxLen){
	char *p;
	size_t i, len, extlen;
	extlen = strlen( ext );
	len = strlen(src);
	if(len >= maxLen) return ERR;
	strcpy(dest, src);
	p = &dest[len - 1];
	len = Min(len, extlen); /* if the len of suffix < 4 then replace it with "mp3" */
	for(i = 0; i< len; i++, p--){
		if(*p == '\\' || *p == '/')break; /* no suffix */
		if(*p == '.'){
			*p = NUL;
			break;
		}
	}
	if(strlen(dest) + extlen >= maxLen) return ERR;
	strcat(dest, ext );
	return NOERR;
} /* changeSuffix */


/******************   for error   ******************/
void errPrintf(const char *msg, ...){
	char buf[1024];

	va_list vlist;
	va_start(vlist, msg);
	vsprintf(buf, msg, vlist);
#if	defined( _USRDLL ) && defined( WIN32 )
	MessageBox( NULL, buf, "GOGO.DLL", MB_OK );
#else
	fputs(buf, stderr);
#endif
	va_end(vlist);
} /* errPrintf */

void bzeroAligned8(void *dest, int size)
{
	int *p = (int *)dest;
	assert((size & 7) == 0 && size > 0);
	do {
		p[0] = p[1] = 0;
		p += 2;
		size -= 8;
	} while(size);
}
