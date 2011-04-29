/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 T.Narita
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */


#ifndef __FILEHEAD_H__
#define __FILEHEAD_H__


//	RIFF の先頭のチャンクヘッダ
struct CK_RIFF {
	long	chunk;						/* "RIFF" */
	long	size;						/* sizeof "RIFF" */
	long	form;						/* "WAVE" or etc */
} ;

//	RIFF/WAVE の fmt チャンク
struct CK_FMT {
	long	chunk;						/* "fmt " */
	long	size;						/* 30 */
	short	formatID;					/* 0x55 = WAVE_FORMAT_MPEGLAYER3 */
	short	num_of_channel;				/* 1 or 2 */
	long	srate;						/* 44100 etc. */
	long	avg_bytes_per_sec;			/* 平均ビットレート(バイト/秒) */
	short	block_size;					/* 1 */
	short	bits_per_sample;			/* 0 */
	short	cbSize;						/* 12 */
	short 	wID;						/* 1 = MPEGLAYER3_ID_MPEG */
	long	fdwFlags;					/* 2 */
  	short	nBlockSize;					/* フレームのバイト数 */
  	short	nFramesPerBlock;			/* 1 */
  	short	nCodecDelay;				/* ?? */
} ;

//	RIFF/WAVE の fact チャンク
struct CK_FACT {
	long	chunk;						/* "fact" */
	long	size;						/* 4 */
	long	num_of_sample;				/* サンプル数 */
} ;

//	RIFF の data チャンクのヘッダ
struct CK_DATA {
	long	chunk;						/* "data" */
	long	size;						/* sizeof "DATA" */
} ;

//	RIFF/WAVE の mp3 本体より前の部分
struct CK_WAVE {
	struct	CK_RIFF		riff;
	struct	CK_FMT		fmt;
	struct  CK_FACT		fact;
	struct	CK_DATA		data;
} ;

//	RIFF/RMP の mp3 本体より前の部分
struct CK_RMP {
	struct	CK_RIFF		riff;
	struct	CK_DATA		data;
};

//	RIFF の LIST チャンクのヘッダ
struct CK_LIST {
	long	chunk;						/* "LIST" */
	long	size;						/* sizeof "LIST" */
	long	form;						/* "INFO" etc */
} ;

//	各チャンク構造体の隙間無しのバイト数
#define	SizeOfCkRiff	(12)
#define	SizeOfCkFmt	(38)
#define	SizeOfCkFact	(12)
#define	SizeOfCkData	(8)
#define	SizeOfCkWave	(SizeOfCkRiff+SizeOfCkFmt+SizeOfCkFact+SizeOfCkData)
#define	SizeOfCkRmp	(SizeOfCkRiff+SizeOfCkData)
#define	SizeOfCkList	(12)

#endif /* __FILEHEAD_H__ */
