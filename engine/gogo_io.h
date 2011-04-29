/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#ifndef GOGO_IO_H_
#define GOGO_IO_H_

#include "lame.h"

MERET	initRead(void);
void	finalizeRead(void);
MERET	initWrite(void);
void	finalizeWrite(void);
int		readData(void *data, size_t size);
int		writeData(void *data, size_t size);
int		writeZeroData(size_t size);
#define	writeVar(var)	(writeData(&(var), sizeof(var)))
void	close_no_device(void* handle);

MERET	open_input_file(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close);
MERET	open_input_stdin(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close);
MERET	open_output_file(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close);
MERET	open_output_stdout(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close);

MERET	open_input_wav_format(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close);
MERET	init_input_raw_format(int nBit, int nChn, int nFreq);
MERET	open_input_raw_format(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close);

MERET	open_input_8to16_filter(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close);
MERET	open_input_stereo_to_mono_filter(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close);
MERET	open_input_resampling_filter(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close);

MERET	open_output_mp3_format(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close);
MERET	open_output_wav_format(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close);
MERET	open_output_rmp_format(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close);

MERET	init_input_user_func(struct MCP_INPDEV_USERFUNC* userIn);
MERET	open_input_user_func(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close);
MERET   init_output_user_func(MPGE_USERFUNC userOut);
MERET	open_output_user_func(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close);
MERET	open_output_user_func_withvbrtag(DEVICE_HANDLE* handle, const char* file_name, IO_DEVICE_FUNC* write, SEEK_TOP_OUTPUT_DEVICE_FUNC* seek_top, CLOSE_DEVICE_FUNC* close);

MERET	open_input_libsnd(DEVICE_HANDLE* handle, const char* file_name, PCM_FORMAT* pcm_format, IO_DEVICE_FUNC* read, CLOSE_DEVICE_FUNC* close);

MERET	init_bench_input(struct MCP_INPDEV_USERFUNC* user_in);
MERET	init_bench_output(MPGE_USERFUNC* user_out);

#ifdef WIN32
	#define	STR_NOCASECOMP		stricmp
#elif __unix__
	#define STR_NOCASECOMP		strcasecmp
#else
	#define STR_NOCASECOMP		strcmp
#endif

#endif /* GOGO_IO_H_ */
