/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#include <assert.h>
#include <limits.h>
#include "common.h"
#include "global.h"
#include "tool.h"
#include "gogo.h"
#include "lame.h"
#include "gogo_io.h"
#include "bitstream.h"
#include "vbrtag.h"
#include "filehead.h"
#include "cpu.h"

#if defined(WIN32) || defined(__os2__)
 	#define	STR_NOCASECOMP		stricmp
#elif defined(__unix__)
 	#define STR_NOCASECOMP		strcasecmp
#else
 	#define STR_NOCASECOMP		strcmp
#endif

static MERET open_input_device(OPEN_INPUT_DEVICE_FUNC open_func, PCM_FORMAT* pcm_format)
{
	MERET result;
	assert(open_func);
	result = open_func(&(RO.input_device_handle), BE.inFileName, pcm_format, &(RO.read_input_device), &(BE.close_input_device));
	if ( result != ME_NOERR )	return result;
	if (RO.read_input_device == NULL || BE.close_input_device == NULL) { return ME_INTERNALERROR; }
	return result;
}

static MERET open_output_device(OPEN_OUTPUT_DEVICE_FUNC open_func)
{
	MERET result;
	assert(open_func);
	result = open_func(&(RO.output_device_handle), BE.outFileName, &(RO.write_output_device), &(BE.seek_top_output_device), &(BE.close_output_device));	
	if ( result != ME_NOERR )	return result;
	if (RO.write_output_device == NULL || BE.close_output_device == NULL) { return ME_INTERNALERROR; }
	return result;
}

/* open input device and read wave header */
MERET
initRead(void)
{
	PCM_FORMAT pcm_format;
	MERET open_result;
	int filter_index;

	memset(&pcm_format, 0, sizeof(pcm_format));
	pcm_format.nSize = MC_INPDEV_MEMORY_NOSIZE;

	if( BE.open_input_device == NULL ){
		BE.open_input_device = open_input_file;
	}

#if		!defined(USE_LIBSNDIO)
	open_result = open_input_device(BE.open_input_device, &pcm_format);
	if (open_result != ME_NOERR) { return open_result; }

	if (BE.open_input_format) {
		open_result = open_input_device(BE.open_input_format, &pcm_format);
		if (open_result != ME_NOERR) { return open_result; }
	}
	// !USE_LIBSNDIO
#else
	// USE_LIBSNDIO
	do {
		open_result = open_input_device(BE.open_input_device, &pcm_format);
		if (open_result != ME_NOERR) 
			break;

		if (BE.open_input_format) {
			open_result = open_input_device(BE.open_input_format, &pcm_format);
			if (open_result != ME_NOERR)
				break;
		}
	} while(0);
	if ( open_result != ME_NOERR) {
		if( ME_WAVETYPE_ERR == open_result && BE.open_input_device == open_input_file ){
			// libsndに切り替えします
			finalizeRead();
			BE.open_input_device = open_input_libsnd;

			do {
				open_result = open_input_device(BE.open_input_device, &pcm_format);
				if (open_result != ME_NOERR)	
					break;

				if (BE.open_input_format) {
					open_result = open_input_device(BE.open_input_format, &pcm_format);
					if (open_result != ME_NOERR) 
						break;
				}
			} while(0);
		}
		if( open_result != ME_NOERR )	return	open_result;
	}
	// USE_LIBSNDIO
#endif

	if (pcm_format.nBit == 8) { 
		open_result = open_input_device(open_input_8to16_filter, &pcm_format);
		if (open_result != ME_NOERR) { return open_result; }
	}

	if (RO.mode == MONO && pcm_format.nChn == 2){ 
		open_result = open_input_device(open_input_stereo_to_mono_filter, &pcm_format);
		if (open_result != ME_NOERR) { return open_result; }
	}

	// 明示的な入力周波数設定があればそれを優先
	if (BE.inpFreqHz) {
		pcm_format.nFreq = BE.inpFreqHz;
	}

	if (RO.out_samplerate) {
		RO.out_samplerate = map2MP3Frequency(RO.out_samplerate);
	} else {
		RO.out_samplerate = map2MP3Frequency(pcm_format.nFreq);
	}
	if (RO.out_samplerate != pcm_format.nFreq) {
		open_result = open_input_device(open_input_resampling_filter, &pcm_format);
		if (open_result != ME_NOERR) { return open_result; }
	}

	for (filter_index = 0; filter_index < BE.input_filter_count; filter_index++) {
		open_result = open_input_device(BE.open_input_filters[filter_index], &pcm_format);
		if (open_result != ME_NOERR) { return open_result; }
	}

	if (pcm_format.nChn != 1 && pcm_format.nChn  != 2) { 
		return ME_WAVETYPE_ERR;
	}

	if (pcm_format.nBit != 16) { 
		return ME_WAVETYPE_ERR;
	}

	RO.nChannel = pcm_format.nChn;
	BE.inpFreqHz = pcm_format.nFreq;
	if (pcm_format.nSize == MC_INPDEV_MEMORY_NOSIZE) {
		RO.nSample = -1;
	} else {
 		RO.nSample = pcm_format.nSize / (pcm_format.nChn * pcm_format.nBit / 8);
 	}

	return ME_NOERR;
} /* initRead */

void
finalizeRead(void)
{
	if (RO.input_device_handle) {
		if (BE.close_input_device) {
			BE.close_input_device(RO.input_device_handle);
		}
		RO.input_device_handle = NULL;
	}
}

/* open output device */
MERET
initWrite(void)
{
	MERET open_result;

	if( BE.open_output_device == NULL ){
		if (BE.open_input_device == open_input_stdin) {
			BE.open_output_device = open_output_stdout;
		}
		else {
			BE.open_output_device = open_output_file;
		}
	}

	if(	BE.outFileName[0] == '\0'){
		char name[MAX_FILE_LEN];
		const char *pExt;
		if ( BE.open_output_format == open_output_mp3_format){
			pExt = ".mp3";
		} 
		else if (BE.open_output_format == open_output_wav_format){
			pExt = ".wav";
		}
		else if (BE.open_output_format == open_output_rmp_format){
			pExt = ".rmp";
		}
		else {
			BE.open_output_format = open_output_mp3_format;
			pExt = ".mp3";
		}
		if(changeSuffix(name, BE.inFileName, pExt, MAX_FILE_LEN) == ERR) return ME_INTERNALERROR;
		strcpy(BE.outFileName, name);
	}

	// ビットストリームの初期化
	init_bit_stream_w();

	open_result = open_output_device(BE.open_output_device);
	if (open_result != ME_NOERR) { return open_result; }

	if( BE.open_output_format == NULL ) {
		BE.open_output_format = open_output_mp3_format;
		if (BE.seek_top_output_device) {
			char	*pExt = strrchr( BE.outFileName, '.' );
			// 拡張子にかかわらず mp3 形式に出力するように
			// コマンドラインから設定する方法ってないよね？
			// -mp3 とかってオプションを新設すればいいのかな？
			if ( pExt ) {
				if( STR_NOCASECOMP( pExt, ".wav" ) == 0 || 
					STR_NOCASECOMP( pExt, ".wave" ) == 0 ){
					BE.open_output_format = open_output_wav_format;
				} else
				if( STR_NOCASECOMP( pExt, ".rmp" ) == 0 ) {
					BE.open_output_format = open_output_rmp_format;
				} else
				if( STR_NOCASECOMP( pExt, ".raw" ) == 0 ){
					BE.open_output_format = open_output_mp3_format;
				}
			}
		}
	}

	if (BE.open_output_format) {
		open_result = open_output_device(BE.open_output_format);
		if (open_result != ME_NOERR) { return open_result; }
	}

	return ME_NOERR;
} /* initIO */

void
finalizeWrite(void)
{
	if (RO.output_device_handle) {
		if (BE.close_output_device) {
			BE.close_output_device(RO.output_device_handle);
		}
		RO.output_device_handle = NULL;
	}
}

int
readData(void *data, size_t size)
{
#if !defined(NDEBUG)
	int result;
	assert(RO.input_device_handle);
	result = RO.read_input_device(RO.input_device_handle, data, size);
	assert(result <= size);
	return result;
#else
	return RO.read_input_device(RO.input_device_handle, data, size);
#endif
} /* readData */

int
writeData(void *data, size_t size)
{
#if  defined(BENCH_ONLY)
	int i;
	for (i = 0; i < size; i++) {
		((char*)data)[i] &= 0xAA;
	}
#endif //BENCH_ONLY
	assert(RO.output_device_handle);
	RW.OutputDoneSize += size;
	return RO.write_output_device(RO.output_device_handle, data, size);
} /* writeData */

int
writeZeroData(size_t size)
{
	char nullData = 0;
	int result = 0;
	while( size ){
		result += writeData(&nullData, sizeof( nullData ) );
		size--;
	}
	return result;
} /* writeZeroData */


//	close で何もしなくてもよいデバイスで共用
void close_no_device(void* handle)
{
//	nothing to do
}


//	ファイル入力関連 open, read, close
static
size_t	read_input_file(void* handle, void* buf, size_t nLength)
{
	return fread(buf, 1, nLength, (FILE*)handle);
}

static
void close_input_file(DEVICE_HANDLE handle)
{
	fclose((FILE*)handle);
}

MERET open_input_file(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close)
{
	FILE* file;
	assert(*handle == NULL);

	file = fopen(file_name, "rb");
	if (file == NULL) {
		return ME_INFILE_NOFOUND;
	}

	*read = read_input_file;
	*close = close_input_file;
	*handle = file;

	fseek(file, 0, SEEK_END);
	pcm_format->nSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	// BE.open_input_format がないと処理できないので
	// ファイル名に合わせて選択する
	if( BE.open_input_format == NULL) {
		char	*pExt = strrchr( BE.inFileName, '.' );
		BE.open_input_format = open_input_wav_format;	// DFLT
		if( pExt != NULL ){
			if( STR_NOCASECOMP( pExt, ".wav" ) == 0 || 
				STR_NOCASECOMP( pExt, ".wave" ) == 0 ){
				BE.open_input_format = open_input_wav_format;
			} else 
			if( STR_NOCASECOMP( pExt, ".raw" ) == 0 ){
				BE.open_input_format = open_input_raw_format;	
			}
		}
	}

	return ME_NOERR;
}

//	標準入力関連 open, close
MERET open_input_stdin(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close)
{
	FILE* file = stdin;
	assert(*handle == NULL);

	setBinaryMode(file);

	*read = read_input_file;
	*close = close_no_device;
	*handle = file;

	// BE.open_input_format がないと処理できないので
	// wav フォーマットに決め打ちしておく
	if( BE.open_input_format == NULL) {
		BE.open_input_format = open_input_wav_format;	// DFLT
	}

	return ME_NOERR;
}

//	ファイル出力関連 open, write, seek_top, close
static
size_t	write_output_file(DEVICE_HANDLE handle, void* buf, size_t nLength)
{
	return fwrite(buf, 1, nLength, (FILE*)handle);
}

static
void close_output_file(DEVICE_HANDLE handle)
{
	fclose((FILE*)handle);
}

static
MERET seek_top_output_file(DEVICE_HANDLE handle)
{
	if (fseek((FILE*)handle, 0, SEEK_SET) != 0) {
		return ME_CANNOT_SEEK;
	}

	return ME_NOERR;
}

MERET open_output_file(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close)
{
	FILE* file = fopen(file_name, "wb+");

	if (file == NULL) {
		return ME_OUTFILE_NOFOUND;
	}

	*write		= write_output_file;
	*seek_top	= seek_top_output_file;
	*close		= close_output_file;
	*handle		= file;

	return  ME_NOERR;
}

//	標準出力関連 open
MERET open_output_stdout(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close)
{
	FILE* file = stdout;

	setBinaryMode(file);

	*write		= write_output_file;
	*seek_top	= NULL;
	*close		= close_no_device;
	*handle		= file;

	return  ME_NOERR;
}

//	wav 形式入力
MERET open_input_wav_format(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close)
{
	assert(*handle != NULL);

	if (getWaveInfo(&(pcm_format->nSize), &(pcm_format->nBit), &(pcm_format->nFreq), &(pcm_format->nChn)) != NOERR) {
		return ME_WAVETYPE_ERR;
	}

	return ME_NOERR;
}

//	raw 形式入力
static int input_raw_format_nBit = 16;
static int input_raw_format_nChn = 2;
static int input_raw_format_nFreq = 44100;
static CLOSE_DEVICE_FUNC input_raw_format_close_prev;

MERET init_input_raw_format(int nBit, int nChn, int nFreq)
{
	input_raw_format_nBit = nBit;
	input_raw_format_nChn = nChn;
	input_raw_format_nFreq = nFreq;

	return ME_NOERR;
}

static
void close_input_raw_format(DEVICE_HANDLE handle)
{
	input_raw_format_nBit = 16;
	input_raw_format_nChn = 2;
	input_raw_format_nFreq = 44100;
	input_raw_format_close_prev(handle);
}

MERET open_input_raw_format(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close)
{
	assert(*handle != NULL);

	pcm_format->nBit  = input_raw_format_nBit;
	pcm_format->nChn  = input_raw_format_nChn;
	pcm_format->nFreq = input_raw_format_nFreq;

	input_raw_format_close_prev = *close;
	*close = close_input_raw_format;

	return ME_NOERR;
}

//	mp3 形式出力
static CLOSE_DEVICE_FUNC output_mp3_format_close_prev;

static
void close_output_mp3_format(DEVICE_HANDLE handle)
{
	if (BE.addtagInfo.pAddtagInfos) {
		RO.write_output_device(handle, BE.addtagInfo.pAddtagInfos, BE.addtagInfo.addtagLen);
	}

	if (BE.seek_top_output_device && RO.bWriteVbrTag ) {
		size_t	outputDoneSize = RW.OutputDoneSize;
		int nQuality=BE.VBR_q * 100 / 9;
		void	*pVBRTag;
		size_t	vbrTagSizeResult;
		pVBRTag =	NULL;

		if( CreateVbrTag( &pVBRTag, &vbrTagSizeResult, nQuality, outputDoneSize) == 0 &&
			BE.seek_top_output_device(RO.output_device_handle) == ME_NOERR) {
			RO.write_output_device(RO.output_device_handle, pVBRTag, vbrTagSizeResult);
		}
	}

	output_mp3_format_close_prev(handle);
}

MERET open_output_mp3_format(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close)
{
	output_mp3_format_close_prev = *close;
	*close = close_output_mp3_format;

	return  ME_NOERR;
}

//	rmp 形式出力
static CLOSE_DEVICE_FUNC output_rmp_format_close_prev;

static
void close_output_rmp_format(DEVICE_HANDLE handle)
{
	if (BE.seek_top_output_device) {
		struct CK_RMP header;
		int		fileSize;
		int		dataSize;
		size_t	outputDoneSize = RW.OutputDoneSize;
		int nQuality=BE.VBR_q * 100 / 9;
		void	*pVBRTag;
		size_t	vbrTagSizeResult;
		pVBRTag =	NULL;

		fileSize = outputDoneSize;
		dataSize = fileSize-SizeOfCkRmp;
		if( fileSize % 2 ) {
			writeZeroData(1);
			fileSize++;
		}

		if( BE.riffInfo.riffInfosLen ) {
			struct CK_LIST list;
			memmove(&(list.chunk), "LIST", 4);
			list.size = BE.riffInfo.riffInfosLen+4;
			memmove( &(list.form), "INFO", 4);

			writeVar(list.chunk);
			writeVar(list.size);
			writeVar(list.form);

			RO.write_output_device(RO.output_device_handle, BE.riffInfo.pRiffInfos, BE.riffInfo.riffInfosLen);
		}

		if (BE.seek_top_output_device(RO.output_device_handle) == ME_NOERR) {
			memmove( &(header.riff.chunk), "RIFF", 4);
			header.riff.size = fileSize-8;
			if( BE.riffInfo.riffInfosLen ) {
				header.riff.size += BE.riffInfo.riffInfosLen+12;
			}
			memmove( &(header.riff.form), "RMP3", 4);
			
			memmove( &(header.data.chunk), "data", 4);
			header.data.size = dataSize;
			
			writeVar(header.riff.chunk);
			writeVar(header.riff.size);
			writeVar(header.riff.form);
			
			writeVar(header.data.chunk);
			writeVar(header.data.size);

			if( RO.bWriteVbrTag && CreateVbrTag( &pVBRTag, &vbrTagSizeResult, nQuality, dataSize) == 0 ){
				RO.write_output_device(RO.output_device_handle, pVBRTag, vbrTagSizeResult);
			}
		}
	}

	output_rmp_format_close_prev(handle);
}

MERET open_output_rmp_format(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close)
{
	if (writeZeroData(SizeOfCkRmp) != SizeOfCkRmp) {
		return ME_WRITEERROR;
	}

	output_rmp_format_close_prev = *close;
	*close = close_output_rmp_format;

	return  ME_NOERR;
}

//	wav 形式出力
static CLOSE_DEVICE_FUNC output_wav_format_close_prev;

static
void close_output_wav_format(DEVICE_HANDLE handle)
{
	if (BE.seek_top_output_device) {
		struct CK_WAVE header;
		int		fileSize;
		int		dataSize;
		size_t	outputDoneSize = RW.OutputDoneSize;
		int nQuality=BE.VBR_q * 100 / 9;
		void	*pVBRTag;
		size_t	vbrTagSizeResult;
		pVBRTag =	NULL;
		
		fileSize = outputDoneSize;
		dataSize = fileSize - SizeOfCkWave;
		if( fileSize % 2 ) {
			writeZeroData(1);
			fileSize++;
		}

		if( BE.riffInfo.riffInfosLen ) {
			struct CK_LIST list;
			memmove(&(list.chunk), "LIST", 4);
			list.size = BE.riffInfo.riffInfosLen+4;
			memmove( &(list.form), "INFO", 4);

			writeVar(list.chunk);
			writeVar(list.size);
			writeVar(list.form);

			RO.write_output_device(RO.output_device_handle, BE.riffInfo.pRiffInfos, BE.riffInfo.riffInfosLen);
		}

		if (BE.seek_top_output_device(RO.output_device_handle) == ME_NOERR) {
			memmove( &(header.riff.chunk), "RIFF", 4);
			header.riff.size = fileSize-8;
			if( BE.riffInfo.riffInfosLen ) {
				header.riff.size += BE.riffInfo.riffInfosLen+12;
			}
			memmove( &(header.riff.form), "WAVE", 4);
			
			memmove( &(header.fmt.chunk), "fmt ", 4);
			header.fmt.size = SizeOfCkFmt-8;
			header.fmt.formatID = 0x55;
			header.fmt.num_of_channel = RO.nChannel;
			header.fmt.srate = RO.out_samplerate;
			header.fmt.avg_bytes_per_sec = RO.VBR ? 1.0*(fileSize-SizeOfCkWave)*header.fmt.srate/(1.0*RW.frameNum*RO.framesize) : BE.rateKbps*1000/8;
			header.fmt.block_size = 1;
			header.fmt.bits_per_sample = 0;
			header.fmt.cbSize = 12;
			header.fmt.wID = 1;
			header.fmt.fdwFlags = 2;
			header.fmt.nBlockSize = RO.VBR ? 144.0*320000/header.fmt.srate : 144.0*BE.rateKbps*1000/header.fmt.srate;
			header.fmt.nFramesPerBlock = 1;
			header.fmt.nCodecDelay = 0x0571;
			
			memmove( &(header.fact.chunk), "fact", 4);
			header.fact.size = SizeOfCkFact-8;
			header.fact.num_of_sample = RW.frameNum*RO.framesize;
			
			memmove( &(header.data.chunk), "data", 4);
			header.data.size = dataSize;

			writeVar(header.riff.chunk);
			writeVar(header.riff.size);
			writeVar(header.riff.form);
			
			writeVar(header.fmt.chunk);
			writeVar(header.fmt.size);
			writeVar(header.fmt.formatID);
			writeVar(header.fmt.num_of_channel);
			writeVar(header.fmt.srate);
			writeVar(header.fmt.avg_bytes_per_sec);
			writeVar(header.fmt.block_size);
			writeVar(header.fmt.bits_per_sample);
			writeVar(header.fmt.cbSize);
			writeVar(header.fmt.wID);
			writeVar(header.fmt.fdwFlags);
			writeVar(header.fmt.nBlockSize);
			writeVar(header.fmt.nFramesPerBlock);
			writeVar(header.fmt.nCodecDelay);

			writeVar(header.fact.chunk);
			writeVar(header.fact.size);
			writeVar(header.fact.num_of_sample);

			writeVar(header.data.chunk);
			writeVar(header.data.size);

			if( RO.bWriteVbrTag && CreateVbrTag( &pVBRTag, &vbrTagSizeResult, nQuality, dataSize) == 0 ){
				RO.write_output_device(RO.output_device_handle, pVBRTag, vbrTagSizeResult);
			}
		}
	}

	output_wav_format_close_prev(handle);
}

MERET open_output_wav_format(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close)
{
	if (writeZeroData(SizeOfCkWave) != SizeOfCkWave) {
		return ME_WRITEERROR;
	}

	output_wav_format_close_prev = *close;
	*close = close_output_wav_format;

	return  ME_NOERR;
}

// STEREO → MONO 変換フィルター
static IO_DEVICE_FUNC	 input_stereo_to_mono_filter_read_prev;

static
size_t	read_input_stereo_to_mono_filter(void* handle, void* buf, size_t nLength)
{
	short convBuf[576*2]; // これで大抵はループ1回で済むはず
	int lengthFromPrev;
	int doneLengthFromPrev;
	int doneSampleCountFromPrev;
	int result;
	int i;
	short* destBuf;

	assert(nLength % 2 == 0); // 手抜きですまん^^;;

	result = 0;
	destBuf = (short*)buf;
	while(1) {
		lengthFromPrev = nLength << 1;
		if (sizeof(convBuf) < lengthFromPrev) {
			lengthFromPrev = sizeof(convBuf);
		}
		nLength -= lengthFromPrev >> 1;

		doneLengthFromPrev = input_stereo_to_mono_filter_read_prev(handle, convBuf, lengthFromPrev);
		doneSampleCountFromPrev = doneLengthFromPrev >> 1;
		for( i = 0; i < doneSampleCountFromPrev; i += 2) {
			*destBuf++ = (((int)convBuf[i])+((int)convBuf[i+1]))/2;
		}
		result += doneLengthFromPrev >> 1;
		
		if (nLength == 0 || doneLengthFromPrev < lengthFromPrev) {
			break;
		}
	}
	return result;
}

MERET open_input_stereo_to_mono_filter(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close)
{
	assert(*handle != NULL);

	if (pcm_format->nChn != 2 || pcm_format->nBit != 16) {
		return ME_INTERNALERROR;
	}

	pcm_format->nChn  = 1;
	if (pcm_format->nSize != MC_INPDEV_MEMORY_NOSIZE) {
		pcm_format->nSize /= 2;
	}

	input_stereo_to_mono_filter_read_prev = *read;
	*read = read_input_stereo_to_mono_filter;

	return ME_NOERR;
}


//	8→16bit 変換入力フィルター
static IO_DEVICE_FUNC	 input_8to16_filter_read_prev;

static
size_t	read_input_8to16_filter(void* handle, void* buf, size_t nLength)
{
	unsigned char convBuf[576*2]; // これで大抵はループ1回で済むはず
	int lengthFromPrev;
	int doneLengthFromPrev;
	int result;
	int i;
	short* destBuf;

	assert(nLength % 2 == 0); // 手抜きですまん^^;;

	result = 0;
	destBuf = (short*)buf;
	while(1) {
		lengthFromPrev = nLength >> 1;
		if (sizeof(convBuf) < lengthFromPrev) {
			lengthFromPrev = sizeof(convBuf);
		}
		nLength -= lengthFromPrev << 1;

		doneLengthFromPrev = input_8to16_filter_read_prev(handle, convBuf, lengthFromPrev);

		for( i = 0; i < doneLengthFromPrev; i++) {
			destBuf[i] = (((int)convBuf[i])-128) << 8;
		}
		result += doneLengthFromPrev << 1;
		destBuf += doneLengthFromPrev;
		
		if (nLength == 0 || doneLengthFromPrev < lengthFromPrev) {
			break;
		}
	}
	return result;
}

MERET open_input_8to16_filter(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close)
{
	assert(*handle != NULL);

	if (pcm_format->nBit != 8) {
		return ME_INTERNALERROR;
	}

	pcm_format->nBit  = 16;
	if (pcm_format->nSize != MC_INPDEV_MEMORY_NOSIZE) {
		pcm_format->nSize *= 2;
	}

	input_8to16_filter_read_prev = *read;
	*read = read_input_8to16_filter;

	return ME_NOERR;
}

// 入力サンプリングレート変換フィルター
typedef  struct {
	IO_DEVICE_FUNC read_func_prev;
	DEVICE_HANDLE handle_prev;
	int input_samplerate;
	int pcm_position;
	int pcm_position_sub;
	int pcm_eof;
	int pcm_filled_len;
	short pcm_buffer[ 576 * 2 ];
	float rev_output_samplerate;
	float ratio_outin_samplerate;
	CLOSE_DEVICE_FUNC close_func_prev;
} INPUT_RESAMPLING_FILTER_STATUS;

static
void	read_input_resampling_filter_fill_pcm_buffer(INPUT_RESAMPLING_FILTER_STATUS* status)
{
	size_t toReadByteCount;
	size_t readByteCount;
	int moveLen = 0;
	if (status->pcm_position < status->pcm_filled_len) {
		moveLen = status->pcm_filled_len - status->pcm_position;
		memmove(status->pcm_buffer, status->pcm_buffer + status->pcm_position, sizeof(short) * moveLen);
	}
	toReadByteCount = sizeof(status->pcm_buffer) - sizeof(short) * moveLen;
	readByteCount = status->read_func_prev(status->handle_prev, status->pcm_buffer + moveLen, toReadByteCount);
	if (readByteCount < toReadByteCount) {
		int toFillZeroByteCount = toReadByteCount - readByteCount;
		memset(((char*)status->pcm_buffer) + sizeof(status->pcm_buffer) - toFillZeroByteCount, 0, toFillZeroByteCount);
	}
	status->pcm_filled_len = readByteCount / sizeof(short) + moveLen;
	status->pcm_position = 0;
	status->pcm_eof = readByteCount == 0;
}

static
size_t	read_input_upsampling_stereo_filter(void* handle, void* buf, size_t nLength)
{
	INPUT_RESAMPLING_FILTER_STATUS* status = handle;
	short* destBuf;
	size_t result;
	const int width = 4;
	int weight;

	assert(nLength % 4 == 0);

	destBuf = (short*)buf;
	result = 0;
	while(result < nLength && !status->pcm_eof) {
		if (status->pcm_filled_len < status->pcm_position + width) {
			read_input_resampling_filter_fill_pcm_buffer(handle);
			if (status->pcm_filled_len == 0) {
				break;
			}
		}
		weight = RO.out_samplerate - status->pcm_position_sub;
		destBuf[0] = 
			((int)status->pcm_buffer[status->pcm_position + 0] * weight + 
			 (int)status->pcm_buffer[status->pcm_position + 2] * status->pcm_position_sub) / RO.out_samplerate; 
		destBuf[1] =
			((int)status->pcm_buffer[status->pcm_position + 1] * weight + 
			 (int)status->pcm_buffer[status->pcm_position + 3] * status->pcm_position_sub) / RO.out_samplerate; 
		destBuf += 2;
		result += 2 * sizeof(short);
		status->pcm_position_sub += 
			status->input_samplerate;
		if (RO.out_samplerate <= status->pcm_position_sub) {
			status->pcm_position_sub -= RO.out_samplerate;
			status->pcm_position += 2;
		}
	}
	return result;
}

static
size_t	read_input_downsampling_stereo_filter(void* handle, void* buf, size_t nLength)
{
	INPUT_RESAMPLING_FILTER_STATUS* status = handle;
	short* destBuf;
	size_t result;
	float weight;
	int width;
	int width_sub;
	float lValue;
	float rValue;
	int i;

	assert(nLength % 4 == 0);

	destBuf = (short*)buf;
	result = 0;
	while(result < nLength && !status->pcm_eof) {
		width = ((status->pcm_position_sub + status->input_samplerate) / RO.out_samplerate) * 2;
		width_sub = (status->pcm_position_sub + status->input_samplerate) % RO.out_samplerate;
		if (status->pcm_filled_len < status->pcm_position + width + (width_sub ? 2 : 0)) {
			read_input_resampling_filter_fill_pcm_buffer(handle);
			if (status->pcm_filled_len == 0) {
				break;
			}
		}
		{
			weight = 1. * (RO.out_samplerate - status->pcm_position_sub) * status->rev_output_samplerate;
			lValue = status->pcm_buffer[status->pcm_position + 0] * weight;
			rValue = status->pcm_buffer[status->pcm_position + 1] * weight;
		}
		for (i = 1; i < width/2; i++ ) {
			lValue += 1.0 * status->pcm_buffer[status->pcm_position + i * 2 + 0];
			rValue += 1.0 * status->pcm_buffer[status->pcm_position + i * 2 + 1];
		}
		{
			weight = 1. * width_sub * status->rev_output_samplerate;
			lValue += status->pcm_buffer[status->pcm_position + i * 2 + 0] * weight;
			rValue += status->pcm_buffer[status->pcm_position + i * 2 + 1] * weight;
		}
		destBuf[0] = lValue * status->ratio_outin_samplerate;
		destBuf[1] = rValue * status->ratio_outin_samplerate;
		destBuf += 2;
		result += 2 * sizeof(short);
		status->pcm_position += width;
		status->pcm_position_sub = width_sub;
	}

	return result;
}

static
size_t	read_input_upsampling_mono_filter(void* handle, void* buf, size_t nLength)
{
	INPUT_RESAMPLING_FILTER_STATUS* status = handle;
	short* destBuf;
	size_t result;
	int weight;
	const int width = 2;

	assert(nLength % 2 == 0);

	destBuf = (short*)buf;
	result = 0;
	while(result < nLength && !status->pcm_eof) {
		if (status->pcm_filled_len < status->pcm_position + width) {
			read_input_resampling_filter_fill_pcm_buffer(handle);
			if (status->pcm_filled_len == 0) {
				break;
			}
		}
		weight = RO.out_samplerate - status->pcm_position_sub;
		destBuf[0] = 
			((int)status->pcm_buffer[status->pcm_position + 0] * weight + 
			 (int)status->pcm_buffer[status->pcm_position + 1] * status->pcm_position_sub) / RO.out_samplerate; 
		destBuf += 1;
		result += 1 * sizeof(short);
		status->pcm_position_sub += 
			status->input_samplerate;
		if (RO.out_samplerate <= status->pcm_position_sub) {
			status->pcm_position_sub -= RO.out_samplerate;
			status->pcm_position += 1;
		}
	}
	return result;
}

static
size_t	read_input_downsampling_mono_filter(void* handle, void* buf, size_t nLength)
{
	INPUT_RESAMPLING_FILTER_STATUS* status = handle;
	short* destBuf;
	size_t result;
	float weight;
	int width;
	int width_sub;
	float lValue;
	int i;

	assert(nLength % 2 == 0);

	destBuf = (short*)buf;
	result = 0;
	while(result < nLength && !status->pcm_eof) {
		width = ((status->pcm_position_sub + status->input_samplerate) / RO.out_samplerate) * 1;
		width_sub = (status->pcm_position_sub + status->input_samplerate) % RO.out_samplerate;
		if (status->pcm_filled_len < status->pcm_position + width + (width_sub ? 1 : 0)) {
			read_input_resampling_filter_fill_pcm_buffer(handle);
			if (status->pcm_filled_len == 0) {
				break;
			}
		}
		{
			weight = 1. * (RO.out_samplerate - status->pcm_position_sub) * status->rev_output_samplerate;
			lValue = status->pcm_buffer[status->pcm_position] * weight;
		}
		for (i = 1; i < width; i++ ) {
			lValue += 1.0 * status->pcm_buffer[status->pcm_position + i];
		}
		{
			weight = 1. * width_sub * status->rev_output_samplerate;
			lValue += status->pcm_buffer[status->pcm_position] * weight;
		}
		destBuf[0] = lValue * status->ratio_outin_samplerate;
		destBuf += 1;
		result += 1 * sizeof(short);
		status->pcm_position += width;
		status->pcm_position_sub = width_sub;
	}

	return result;
}

static
void close_input_resampling_filter(DEVICE_HANDLE handle)
{
	INPUT_RESAMPLING_FILTER_STATUS* status = handle;

	assert( handle != NULL);
	
	if (status->close_func_prev) {
		status->close_func_prev(status->handle_prev);
	}

	free(status);
}

MERET open_input_resampling_filter(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close)
{
	INPUT_RESAMPLING_FILTER_STATUS* status;

	assert(*handle != NULL);

	if (pcm_format->nBit != 16) {
		return ME_INTERNALERROR;
	}

	if (RO.out_samplerate * 255 < pcm_format->nFreq) {
		return ME_FREQERROR;
	}

	if (pcm_format->nFreq * 255 < RO.out_samplerate) {
		return ME_FREQERROR;
	}

	if (32767*255 < pcm_format->nFreq) {
		return ME_FREQERROR;
	}

	status = malloc(sizeof(INPUT_RESAMPLING_FILTER_STATUS));

	memset(status, 0, sizeof(*status));

	status->read_func_prev = *read;
	status->close_func_prev = *close;
	status->handle_prev = *handle;
	status->input_samplerate = pcm_format->nFreq;
	status->pcm_position = 0;
	status->pcm_position_sub = 0;
	status->rev_output_samplerate = 1. / RO.out_samplerate;
	status->ratio_outin_samplerate = 1. * RO.out_samplerate / pcm_format->nFreq;
	
	if (pcm_format->nSize != MC_INPDEV_MEMORY_NOSIZE) {
		pcm_format->nSize = ceil((float)pcm_format->nSize * RO.out_samplerate / pcm_format->nFreq / sizeof(short) / pcm_format->nChn) * sizeof(short) * pcm_format->nChn;
	}
	if (pcm_format->nChn == 1) {
		if (RO.out_samplerate > pcm_format->nFreq) {
			*read = read_input_upsampling_mono_filter;
		} else {
			*read = read_input_downsampling_mono_filter;
		}
	} else {
		if (RO.out_samplerate > pcm_format->nFreq) {
			*read = read_input_upsampling_stereo_filter;
		} else {
			*read = read_input_downsampling_stereo_filter;
		}
	}
	*close = close_input_resampling_filter;
	*handle = (DEVICE_HANDLE)status;

	return ME_NOERR;
}

//	ユーザー入力関連 
static struct MCP_INPDEV_USERFUNC input_user_func;

MERET init_input_user_func(struct MCP_INPDEV_USERFUNC* userIn)
{
	input_user_func = *userIn;
	return ME_NOERR;
}

static
size_t	read_input_user_func(void* handle, void* buf, size_t nLength)
{
	MERET read_result;
	assert(input_user_func.pUserFunc);
	setFpuState(RO.originalFpuState);
	read_result = input_user_func.pUserFunc(buf, nLength);
	setFpuState(GOGO_FPU_STATE);
	if (read_result == ME_NOERR) {
		return nLength;
	}
	else {
		return 0;
	}
}

MERET open_input_user_func(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close)
{
	assert(*handle == NULL);

	if (input_user_func.pUserFunc == NULL) {
		return ME_INTERNALERROR;
	}

	*read  = read_input_user_func;
	*close = close_no_device;

	pcm_format->nBit  = input_user_func.nBit;
	pcm_format->nChn  = input_user_func.nChn;
	pcm_format->nFreq = input_user_func.nFreq;
	pcm_format->nSize = input_user_func.nSize;

	*handle = &input_user_func;

	//	PCM フォーマットの設定は自力でやっているので
	//	BE.open_input_format での処理は要らない
	BE.open_input_format = NULL;

	return ME_NOERR;
}

//	ユーザー出力関連 
static MPGE_USERFUNC output_user_func;

MERET init_output_user_func(MPGE_USERFUNC userOut)
{
	output_user_func = userOut;
	return ME_NOERR;
}

static
size_t	write_output_user_func(DEVICE_HANDLE handle, void* buf, size_t nLength)
{
	MERET write_result;

	assert(output_user_func);
	setFpuState(RO.originalFpuState);
	write_result = output_user_func(buf, nLength);
	setFpuState(GOGO_FPU_STATE);
	if (write_result == ME_NOERR) {
		return nLength;
	}
	else {
		return 0;
	}
}

static
void close_output_user_func(DEVICE_HANDLE handle)
{
	if (output_user_func) {
		setFpuState(RO.originalFpuState);
		output_user_func(NULL, 0);
		setFpuState(GOGO_FPU_STATE);
	}
}

MERET open_output_user_func(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close)
{
	assert(*handle == NULL);

	if (output_user_func == NULL) {
		return ME_INTERNALERROR;
	}

	*write		= write_output_user_func;
	*seek_top	= NULL;
	*close		= close_output_user_func;

	*handle		= output_user_func;

	return  ME_NOERR;
}


static
MERET seek_top_output_user_func_withvbrtag(DEVICE_HANDLE handle)
{
	int user_func_result;

	assert(output_user_func);
	setFpuState(RO.originalFpuState);
	user_func_result = output_user_func(NULL, 0);
	setFpuState(GOGO_FPU_STATE);
	if (user_func_result <= 0) {
		return ME_CANNOT_SEEK;
	}
	return ME_NOERR;
}

MERET open_output_user_func_withvbrtag(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close)
{
	MERET open_result;

	open_result = open_output_user_func(handle, file_name, write, seek_top, close);
	if (open_result != ME_NOERR) {
		return open_result;
	}

	*seek_top	= seek_top_output_user_func_withvbrtag;

	return  ME_NOERR;
}

//	ベンチモード用入出力
#define		BENCH_SAMPLE_MAX_CHANNEL			(16)
static int bench_input_rest_length;
static unsigned int bench_input_seed;
static short (*bench_samples)[BENCH_SAMPLE_MAX_CHANNEL][2304];

static int bench_input_genrand()
{
	bench_input_seed = bench_input_seed * 0x32842851 + 12398321;
	return bench_input_seed & INT_MAX;
}

static
MERET bench_input_func(void *buf, unsigned long nLength )
{
	int copy_length;
	if  (bench_input_rest_length <= 0) {
		if (bench_samples) {
			free(bench_samples);
			bench_samples = NULL;
		}
		return ME_EMPTYSTREAM;
	}
	bench_input_rest_length -= nLength;

	while(0 < nLength) {
		copy_length = Min(nLength, 4608);
		memcpy( buf, (*bench_samples)[ bench_input_genrand() % BENCH_SAMPLE_MAX_CHANNEL ],  copy_length );
		nLength -= copy_length;
	}
	return ME_NOERR;
}


static
MERET bench_output_func(void *buf, unsigned long nLength )
{
	return ME_NOERR;
}

MERET init_bench_input(struct MCP_INPDEV_USERFUNC* user_in)
{
	int		i, j, nMax;

	if (!bench_samples) {
		bench_samples = malloc(BENCH_SAMPLE_MAX_CHANNEL*2304*sizeof(short));
		if (!bench_samples) {
			return ME_NOMEMORY;
		}
	}
	bench_input_seed = 0;
	for(i = 0;i < BENCH_SAMPLE_MAX_CHANNEL; i ++ ){
		nMax = 0x4000 + 0x6000 * i / BENCH_SAMPLE_MAX_CHANNEL;
		for(j = 0;j < 2304;j++){
			(*bench_samples)[i][j] = (bench_input_genrand() % nMax) - nMax/2 ;
		}
	}

	user_in->pUserFunc = bench_input_func;
	if (user_in->nSize == MC_INPDEV_MEMORY_NOSIZE) {
		user_in->nSize     = user_in->nFreq * user_in->nChn * (user_in->nBit/8) * 600;
	}

	bench_input_rest_length = user_in->nSize;

	return ME_NOERR;
}

MERET init_bench_output(MPGE_USERFUNC* user_out)
{
	*user_out = bench_output_func;
	return ME_NOERR;
}
