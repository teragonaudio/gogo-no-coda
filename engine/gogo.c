/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#define _GOGO_C_
#include <assert.h>

#include "config.h"
#include "global.h"
#include "version.h"

#include "lame.h"
#include "get_audio.h"
#include "cpu.h"
#include "tool.h"
#include "gogo_io.h"
#include "util.h"
#include "newmdct.h"
#include "psymodel.h"
#include "quantize.h"
#include "quantize_pvt.h"
#include "bitstream.h"
#include "vbrtag.h"
#include "reservoir.h"
#include "vfta.h"

/*
 *	初期化コードの流れ
 *
 *	1) MPGE_initializeWork()内
 *		1. RO の zero clear
 *		2. lame_init()
 *		3. ROの初期化
 *
 *	2) MPGE_setConfigure()内
 *		個別のRO
 *
 *	3) MPGE_detectConfigure()内
 *		1. initRead();
 *				> nChannel, inpFreqHz, nSample
 *		2. initWrite();
 *				> outFileName
 *		3. lame_init_params()
 */
MERET EXPORT
MPGE_initializeWork(void)
{

	/* setup Read Only global variables */
	memset(&RO, 0, sizeof(RO));
	memset(&BE, 0, sizeof(BE));
	lame_init();

	RO.nChannel = 2;
	//BE.inpFreqHz = 44100;
	RO.out_samplerate = 0;	/* RO.encFreqHz = 0; 0=auto configure */
	RO.nSample = -1;
	BE.rateKbps = 128;
	BE.quality = 5;
	RO.mode = NOT_SET;

	RO.debug = 0;
	RO.silent = 0;
	RO.printf = errPrintf;

	BE.riffInfo.pRiffInfos		=	NULL;
	BE.riffInfo.riffInfosLen	=	0;

	BE.addtagInfo.pAddtagInfos = NULL;
	BE.addtagInfo.addtagLen    = 0;

	/* setup CPU */
	initCPU();
	BE.unit = haveUNIT();
	RO.nCPU = RO.nThread = 0;	/* auto configure */
	if(gogo_get_cpu_count(&RO.nCPU, &RO.nThread)) return ME_INTERNALERROR;
	if(RO.nThread > 1) BE.unit |= MU_tMULTI;
#if defined(MT_ENCODER)
	if(gogo_initialize_thread_unit()) return ME_INTERNALERROR;
#endif
#if defined(CPU_I386)
	if((BE.unit & MU_tFPU) == 0) return ME_NOFPU;
#elif defined(CPU_PPC)
	if((BE.unit & MU_tPPC) == 0) return ME_NOFPU;
#else
//	#error "unknown CPU"
#endif

	return ME_NOERR;
} /* MPGE_initializeWork */

MERET EXPORT
MPGE_getUnitStates(unsigned long *pUnit)
{
	*pUnit = haveUNIT();

	return ME_NOERR;
} /* MPGE_getUnitStates */

MERET EXPORT
MPGE_endCoder(void)
{
	if( BE.riffInfo.pRiffInfos ){
		free( BE.riffInfo.pRiffInfos );
		BE.riffInfo.pRiffInfos		= NULL;
		BE.riffInfo.riffInfosLen	=	0;
	}

	if( BE.addtagInfo.pAddtagInfos){
		free( BE.addtagInfo.pAddtagInfos );
		BE.addtagInfo.pAddtagInfos = NULL;
		BE.addtagInfo.addtagLen    = 0;
	}

#if defined(MT_ENCODER)
	if(gogo_finalize_thread_unit()) return ME_INTERNALERROR;
#endif

	termCPU();

	return ME_NOERR;
} /* MPGE_endCoder */

MERET EXPORT
MPGE_setConfigure(UPARAM mode, UPARAM dwPara1, UPARAM dwPara2)
{

	switch(mode){
 	case MC_INPUTFILE: /* OK */
 		if(dwPara1 == MC_INPDEV_FILE){
			BE.open_input_device = open_input_file;
 			if(strlen((char *)dwPara2) >= MAX_FILE_LEN) return ME_PARAMERROR;
 			strcpy(BE.inFileName, (char *)dwPara2);
 			return ME_NOERR;
 		}else
 		if(dwPara1 == MC_INPDEV_STDIO){
			BE.open_input_device = open_input_stdin;
 			strcpy(BE.inFileName, "stdin");
 			return ME_NOERR;
 		}else
 		if(dwPara1 == MC_INPDEV_USERFUNC){
			BE.open_input_device = open_input_user_func;
			init_input_user_func(((struct MCP_INPDEV_USERFUNC *)dwPara2));
 			BE.inFileName[0] = '\0';
 			return ME_NOERR;
#ifdef	USE_LIBSNDIO
 		}else
			if(dwPara1 == MC_INPDEV_LIBSND){
			BE.open_input_device = open_input_libsnd;
 			if(strlen((char *)dwPara2) >= MAX_FILE_LEN) return ME_PARAMERROR;
 			strcpy(BE.inFileName, (char *)dwPara2);
 			return ME_NOERR;
#endif
		}
		return ME_PARAMERROR;

	case MC_OUTPUTFILE: /* OK */
 		if(dwPara1 == MC_OUTDEV_FILE){
			BE.open_output_device = open_output_file;
 			if(strlen((char *)dwPara2) >= MAX_FILE_LEN) return ME_PARAMERROR;
 			strcpy(BE.outFileName, (char *)dwPara2);
 			return ME_NOERR;
 		}else
		if(dwPara1 == MC_OUTDEV_STDOUT){
			BE.open_output_device = open_output_stdout;
			strcpy(BE.outFileName, "stdout");
			return ME_NOERR;
		}else
 		if(dwPara1 == MC_OUTDEV_USERFUNC){
			BE.open_output_device = open_output_user_func;
			init_output_user_func((MPGE_USERFUNC)dwPara2);
 			return ME_NOERR;
 		}else
 		if(dwPara1 == MC_OUTDEV_USERFUNC_WITHVBRTAG){
			BE.open_output_device = open_output_user_func_withvbrtag;
			init_output_user_func((MPGE_USERFUNC)dwPara2);
 			return ME_NOERR;
 		}
 		return ME_PARAMERROR;

	case MC_ENCODEMODE: /* OK */
		switch(dwPara1){
		case MC_MODE_MONO:
			RO.mode = MONO;
			break;
		case MC_MODE_STEREO:
			RO.mode = STEREO;
			break;
		case MC_MODE_JOINT:
		case MC_MODE_MSSTEREO:
			RO.mode = JOINT_STEREO;
			break;
		case MC_MODE_DUALCHANNEL:
			// どうしよう？
			// RO.mode = JOINT_STEREO;
			// break;
			return ME_PARAMERROR;
		default:
			return ME_PARAMERROR;
		}
		return ME_NOERR;

	case MC_BITRATE:
		BE.rateKbps = dwPara1;
		return ME_NOERR;

	case MC_INPFREQ: /* OK */
		BE.inpFreqHz = dwPara1;
		return ME_NOERR;

	case MC_OUTFREQ: /* OK */
		RO.out_samplerate = dwPara1;	/* RO.encFreqHz = dwPara1; */
		return ME_NOERR;

	case MC_EMPHASIS: /* OK */
		RO.emphasis = dwPara1;
		return ME_NOERR;

	case MC_USEPSY: /* OK */
//			fast_mode = dwPara1 ? FALSE: TRUE ;
		BE.quality = dwPara1 ? 5 : 8;


		return ME_NOERR;

	case MC_ENCODE_QUALITY: /* OK */
		if(dwPara1 < 0 || dwPara1 > 9) return ME_PARAMERROR;
		BE.quality = dwPara1;
		return ME_NOERR;

	case MC_OUTPUT_FORMAT:
		if( dwPara1 != MC_OUTPUT_NORMAL && 
			dwPara1 != MC_OUTPUT_RIFF_WAVE && 
			dwPara1 != MC_OUTPUT_RIFF_RMP ) return ME_PARAMERROR;
		{
 			if( dwPara1 == MC_OUTPUT_NORMAL )
				BE.open_output_format = open_output_mp3_format;
 			else
 			if( dwPara1 == MC_OUTPUT_RIFF_WAVE )
				BE.open_output_format = open_output_wav_format;
 			else
 			if( dwPara1 == MC_OUTPUT_RIFF_RMP )
				BE.open_output_format = open_output_rmp_format;
 		}
 		return ME_NOERR;

	case MC_ADDTAG:
		if( 0 < dwPara1 && dwPara2 ){
			// dwPara1  length of TAG;タグ長  
			// dwPara2  pointer to TAG;タグデータのポインタ
			char	*pTmp, *pSrc;
			
			pSrc= (char *)dwPara2;
			pTmp = realloc( BE.addtagInfo.pAddtagInfos, BE.addtagInfo.addtagLen+dwPara1 );
			if( pTmp == NULL )
				return ME_NOMEMORY;
			BE.addtagInfo.pAddtagInfos = pTmp;
			pTmp += BE.addtagInfo.addtagLen;
			memmove( pTmp , (void *)pSrc, dwPara1 );
			BE.addtagInfo.addtagLen += dwPara1;
		} else {
			return ME_PARAMERROR;
 		}
 		return ME_NOERR;

	case MC_RIFF_INFO:
		if( 4 <= dwPara1 && dwPara2 ){
			int		addInfoSize = (dwPara1 & 1) ? dwPara1 + 5 : dwPara1 + 4;
			char	*pTmp, *pSrc;
			
			pSrc= (char *)dwPara2;
			pTmp = realloc( BE.riffInfo.pRiffInfos, BE.riffInfo.riffInfosLen + addInfoSize );
			if( pTmp == NULL )
				return ME_NOMEMORY;
			BE.riffInfo.pRiffInfos = pTmp;
			pTmp += BE.riffInfo.riffInfosLen;
			memmove( pTmp , (void *)pSrc, 4 );
			*(unsigned long *)(pTmp + 4) = dwPara1 - 4;
			memmove( pTmp + 8, pSrc + 4, dwPara1 - 4 );
			BE.riffInfo.riffInfosLen += addInfoSize;
		} else {
			return ME_PARAMERROR;
		}
		return ME_NOERR;
		
	case MC_USEMMX:
		if( dwPara1 && (haveUNIT() & MU_tMMX) ){
			BE.unit |= MU_tMMX;
		}else{
			BE.unit &= ~(MU_tMMX);
			BE.unit &= ~(MU_tEMMX);	/* MMXを使わない時はEMMXも使わない */
		}
		return ME_NOERR;

	case MC_USEEMMX:
		if( dwPara1 && (haveUNIT() & MU_tEMMX) ){
			BE.unit |= MU_tMMX;		/* EMMXを使う時はMMXも使う */
			BE.unit |= MU_tEMMX;
		}else{
			BE.unit &= ~(MU_tEMMX);
		}
		return ME_NOERR;

	case MC_USECMOV:
		if( dwPara1 && (haveUNIT() & MU_tCMOV) ){
			BE.unit |= MU_tCMOV;
		}else{
			BE.unit &= ~(MU_tCMOV);
		}
		return ME_NOERR;

	case MC_USE3DNOW:
		if( dwPara1 && (haveUNIT() & MU_t3DN) ){
			BE.unit |= MU_t3DN;
		}else{
			BE.unit &= ~(MU_t3DN);
			BE.unit &= ~(MU_tE3DN);	/* 3DNを使わない時はE3DNも使わない */
		}
		return ME_NOERR;

	case MC_USEE3DNOW:
		if( dwPara1 && (haveUNIT() & MU_tE3DN) ){
			BE.unit |= MU_t3DN;		/* E3DNを使う時は3DNも使う */
			BE.unit |= MU_tE3DN;
		}else{
			BE.unit &= ~(MU_tE3DN);
		}
		return ME_NOERR;

	case MC_USESSE:
		if( dwPara1 && (haveUNIT() & MU_tSSE) ){
			BE.unit |= MU_tSSE;
		}else{
			BE.unit &= ~(MU_tSSE);
			BE.unit &= ~(MU_tSSE2);	/* SSEを使わない時はSSE2も使わない */
		}
		return ME_NOERR;

	case MC_USESSE2:
		if( dwPara1 && (haveUNIT() & MU_tSSE2) ){
			BE.unit |= MU_tSSE;		/* SSE2を使う時はSSEも使う */
			BE.unit |= MU_tSSE2;
		}else{
			BE.unit &= ~(MU_tSSE2);
		}
		return ME_NOERR;

	case MC_USESPC1:
		if( dwPara1 ){
			BE.unit |= MU_tSPC1;
		}else{
			BE.unit &= ~(MU_tSPC1);
		}
		return ME_NOERR;

	case MC_USESPC2:
		if( dwPara1 ){
			BE.unit |= MU_tSPC2;
		}else{
			BE.unit &= ~(MU_tSPC2);
		}
		return ME_NOERR;

/* テスト中 */
		case MC_ABR:
			RO.VBR = vbr_abr;
			return ME_NOERR;

		case MC_WRITELAMETAG:
			RO.bWriteLameTag = dwPara1 ? TRUE : FALSE;
			return	ME_NOERR;

		case MC_WRITEVBRTAG:
			RO.bWriteVbrTag = dwPara1 ? TRUE : FALSE;
			return	ME_NOERR;

		case MC_CPU:
			RO.nCPU = dwPara1; RO.nThread = 0;
			if(RO.nCPU < 0) return ME_PARAMERROR;
			BE.unit &= ~MU_tMULTI;
			gogo_get_cpu_count(&RO.nCPU, &RO.nThread);
			if(RO.nThread > 1) BE.unit |= MU_tMULTI;
			return ME_NOERR;
/* ただいま移植中 */
		case MC_VBR:
			RO.VBR = vbr_default;
			BE.VBR_q = dwPara1;
			if (BE.VBR_q <0) BE.VBR_q=0;
			if (BE.VBR_q >9) BE.VBR_q=9;
			return ME_NOERR;
		case MC_VBRBITRATE:
//			VBR_min_rate_idx = get_rate_idx(dwPara1, 1); 
//			VBR_max_rate_idx = get_rate_idx(dwPara2, 1);
//			if(dwPara1 > dwPara2) return ME_PARAMERROR;
//			if( VBR_min_rate_idx == -1 || VBR_max_rate_idx == -1 ) return ME_PARAMERROR;
			RO.brate = dwPara1;
			BE.VBR_min_bitrate_kbps=RO.brate;
			if((int)dwPara2 != -1) BE.VBR_max_bitrate_kbps=dwPara2;
			return ME_NOERR;
		case MC_USELPF16:
			if (dwPara1) {
				BE.lowpassfreq = 0;
			}
			else {
				BE.lowpassfreq = -1;
			}
			return ME_NOERR;
		default:
			return ME_PARAMERROR;
	}/* switch(mode) */
	return ME_PARAMERROR;
} /* MPGE_setConfigure */

static int
initializeThread(void)
{
	int	i;
	gogo_thread_data	*aligned;
	void	*unaligned;

	assert(RO.nThread >= 1);
	RO.tl = NULL;

	assert(RO.nThread >= 1);
	for(i = 0; i < RO.nThread; i++){
		unaligned = malloc(sizeof(gogo_thread_data) + 16);
		if(unaligned == NULL) return ME_NOMEMORY;

		/* align to 16 byte boundary */
		aligned = (void *)(((int)unaligned + 15) & -16);

		/* initialize */
		memset(aligned, 0, sizeof(gogo_thread_data));
		aligned->tid = i;
		aligned->unaligned = unaligned;
		aligned->sb[0] = aligned->sb_sample[0];
		aligned->sb[1] = aligned->sb_sample[1];
		aligned->sb[2] = aligned->sb_sample[2];

		aligned->bitrate_index = BE.CBR_bitrate;

		aligned->next = RO.tl;
		RO.tl = aligned;
	}

	return ME_NOERR;
}

#if defined(MT_ENCODER)
static void *encodethread(gogo_thread_data *tl);
#endif

MERET EXPORT
MPGE_detectConfigure(void)
{
	MERET ret;

	ret = initRead();
 	if(ret != ME_NOERR) return ret;
	ret = initWrite();
	if(ret != ME_NOERR) {
		finalizeRead();
		return ret;
	}

	if( RO.VBR != vbr_abr )
		BE.rateKbps	=	FindNearestBitrate( BE.rateKbps, (RO.out_samplerate > 28000) ? 1 : 0 );
	RO.brate = BE.rateKbps;

	ret = lame_init_params();
	if(ret != ME_NOERR) return ret;

	ret = initializeThread();
	if(ret != ME_NOERR) return ret;

	RO.lame_encode_init=0;

#if defined(MT_ENCODER)
	/* create threads */
	if(RO.nThread > 1){
		int i;
		gogo_thread_data *tl;

		for(i= 0; i < MAX_CRITICAL_REGION; i++){
			ret = gogo_create_mutex(&RW.mutex[i]);
			if(ret) return ME_INTERNALERROR;
		}

		for(i= 0; i < MAX_SEMAPHORE; i++){
			ret = gogo_create_semaphore(&RW.semaphore[i]);
			if(ret) return ME_INTERNALERROR;
		}

		/* init flag */
		RW.termencode_flag = FALSE;
		RW.getpcm_status = ME_NOERR;
/*		RW.end_tl = NULL;	*/

		/* init getpcm() sync */
		ret  = gogo_lock_semaphore(&RW.semaphore[SEMAPHORE_GETPCM_IN]);
		ret |= gogo_lock_semaphore(&RW.semaphore[SEMAPHORE_GETPCM_OUT]);
		if(ret) return ME_INTERNALERROR;

		/* keep all threads sleeping */
		ret = gogo_lock_semaphore(&RW.semaphore[SEMAPHORE_STARTENCODE]);
		if(ret) return ME_INTERNALERROR;

		for(tl = RO.tl; tl != NULL; tl = tl->next){
			ret = gogo_create_thread(&tl->thread, (gogo_thread_func)encodethread, tl);
			if(ret) return ME_INTERNALERROR;
		}
	}
#endif

	if(RO.VBR == vbr_off){ 
		RO.iteration_loop = CBR_iteration_loop;
	}else if(RO.VBR == vbr_abr){
		RO.iteration_loop = ABR_iteration_loop;
		RO.iteration_finish = iteration_finish;
	}else{
		RO.iteration_loop = VBR_iteration_loop;
		RO.iteration_finish = iteration_finish;
	}

	{
		int whole_SpF, brate;
		if(RO.VBR == vbr_off){
			brate = RO.brate;
			RO.getframebits = CBR_getframebits;
		}else {
			brate = bitrate_table[RO.version][1];
			RO.getframebits = VBRABR_getframebits;
		}
		whole_SpF = (RO.version+1)*72000* brate / RO.out_samplerate;
		RO.bitsPerFrame = 8 * whole_SpF;
	}

	setupUNIT( BE.unit );	/* オプション解析の結果を使うようになるかもしれないので */
	return ME_NOERR;
} /* MPGE_detectConfigure */

/* 定義順の関係で encoder.h に置けんかった(^^; */
void encodeframe_init(gogo_thread_data *tl);
int lame_encode_mp3_frame(gogo_thread_data *tl);
#if defined(MT_ENCODER)
void encodethread_init(gogo_thread_data *tl);
int lame_encode_mp3_frame_multi(gogo_thread_data *tl);
#endif

static int
encodeframe(void)
{
	int iread, imp3;
	gogo_thread_data	*tl = RO.tl;

	if (RO.lame_encode_init==0 )  {
		RO.lame_encode_init=1;

		get_audio_init();
		dist_mfbuf(tl->mfbuf);
		encodeframe_init(tl);
	}
	iread = get_pcm();
	if(iread < 0) return  -1 /* iread */;
	if(iread == 0){
                /* mp3 related stuff.  bit buffer might still contain some mp3 data */
                flush_bitstream();
	}else{
		RW.frameNum++;
		if (RO.nSample <= 0 && RW.totalframes < RW.frameNum)
				RW.totalframes = RW.frameNum;
		dist_mfbuf(tl->mfbuf);
		imp3 = lame_encode_mp3_frame(tl);
		if(imp3 < 0){
			RO.printf("mp3 internal error:  error code=%i\n",imp3);
			return -1;
		}
	}

	if( put_mp3() < 0 ) return -1;
	return iread;
} /* encodeframe */

#if defined(MT_ENCODER)
static void *
encodethread(gogo_thread_data *tl)
{
	int iread, imp3;

	setFpuState(GOGO_FPU_STATE);
	/* sleep until startencode */
	tl->exit_status  = gogo_lock_semaphore(&RW.semaphore[SEMAPHORE_STARTENCODE]);
	/* wakes another threads up */
	tl->exit_status |= gogo_unlock_semaphore(&RW.semaphore[SEMAPHORE_STARTENCODE]);
	if(tl->exit_status){
		tl->exit_status = ME_INTERNALERROR;
		return NULL;
	}

	while(1){
		tl->critical_region = RW.mutex;
		tl->exit_status = gogo_lock_mutex(tl->critical_region);
		if(tl->exit_status) break;

		/* before get_pcm() sync */
		if( !RW.trackencode_flag )
			gogo_lock_semaphore(&RW.semaphore[SEMAPHORE_GETPCM_IN]);

		/* check term encode flag */
		if( RW.termencode_flag ){
			RW.getpcm_status = tl->exit_status = ME_EMPTYSTREAM;
			break;
		}

		iread = get_pcm();
		if(iread <= 0){
			if( iread == 0 )
				tl->exit_status = ME_EMPTYSTREAM;
			else
				tl->exit_status = ME_INTERNALERROR;
		}

		/* save get_pcm() result */
		RW.getpcm_status = tl->exit_status;

		/* after get_pcm() sync */
		if( !RW.trackencode_flag )
			gogo_unlock_semaphore(&RW.semaphore[SEMAPHORE_GETPCM_OUT]);

		if( tl->exit_status != ME_NOERR )
			break;

/*		RW.end_tl = tl;		*/

		RW.frameNum++;
		if (RO.nSample <= 0 && RW.totalframes < RW.frameNum)
			 	RW.totalframes = RW.frameNum;

		dist_mfbuf(tl->mfbuf);

		tl->exit_status = gogo_lock_mutex(tl->critical_region+1);
		if(tl->exit_status) break;
		tl->exit_status = gogo_unlock_mutex(tl->critical_region++);
		if(tl->exit_status) break;

		imp3 = lame_encode_mp3_frame_multi(tl);
		if(imp3 < 0){
			RO.printf("thread %d: mp3 internal error:  error code=%i\n",imp3, tl->tid);
			tl->exit_status = ME_INTERNALERROR;
			break;
		}

		if( put_mp3() < 0 ){
			tl->exit_status = ME_WRITEERROR;
			break;
		}

		tl->exit_status = gogo_unlock_mutex(tl->critical_region);
		if(tl->exit_status) break;
	}
	if( tl->exit_status < 0 )
		tl->exit_status = ME_INTERNALERROR;
	gogo_unlock_mutex(tl->critical_region);
	if( !RW.trackencode_flag )
		gogo_unlock_semaphore(&RW.semaphore[SEMAPHORE_GETPCM_IN]);
	return NULL;
}
#endif

MERET EXPORT
MPGE_processFrame(void)
{
	int ret;
	defFpuStateBackupVar(originalFpuState);
	saveFpuState(&originalFpuState);

#if defined(MT_ENCODER)
	if(RO.nThread > 1){
		/* multithread encoding */
		if (RO.lame_encode_init==0){
			RO.lame_encode_init=1;

			/* initialize */
			get_audio_init();
			dist_mfbuf(RO.tl->mfbuf);
			encodethread_init(RO.tl);

			/* frame encode setting */
			RW.trackencode_flag = FALSE;

			/* wake threads up */
			ret = gogo_unlock_semaphore(&RW.semaphore[SEMAPHORE_STARTENCODE]);
			if(ret) {
				restoreFpuState(originalFpuState);
				return ME_INTERNALERROR;
			}
		}

		/* before get_pcm() sync */
		ret = gogo_unlock_semaphore(&RW.semaphore[SEMAPHORE_GETPCM_IN]);
		if(ret) {
			restoreFpuState(originalFpuState);
			return ME_INTERNALERROR;
		}

		/* after get_pcm() sync */
		ret = gogo_lock_semaphore(&RW.semaphore[SEMAPHORE_GETPCM_OUT]);
		restoreFpuState(originalFpuState);
		if(ret) return ME_INTERNALERROR;

		return RW.getpcm_status;
	}
#endif

	ret = encodeframe();
	restoreFpuState(originalFpuState);
	if(ret == -1){
		return ME_INTERNALERROR;
	}else
	if(ret == 0){
		return ME_EMPTYSTREAM;
	}
	return ME_NOERR;
} /* MPGE_processFrame */

MERET EXPORT
MPGE_processTrack(void)
{
	int ret;
	defFpuStateBackupVar(originalFpuState);
	saveFpuState(&originalFpuState);

#if defined(MT_ENCODER)
	if(RO.nThread > 1){
		/* multithread encoding */
		/* initialize */
		RO.lame_encode_init=1;

		get_audio_init();
		dist_mfbuf(RO.tl->mfbuf);
		encodethread_init(RO.tl);

		/* track encode setting */
		RW.trackencode_flag = TRUE;

		/* wake threads up */
		ret = gogo_unlock_semaphore(&RW.semaphore[SEMAPHORE_STARTENCODE]);
		if(ret) {
			restoreFpuState(originalFpuState);
			return ME_INTERNALERROR;
		}

		return ME_NOERR;
	}
#endif

	while((ret = encodeframe()) > 0);
	restoreFpuState(originalFpuState);
	if(ret == -1){
		return ME_INTERNALERROR;
	}
	return ME_NOERR;
} /* MPGE_processTrack */

static int
finalizeThread(void)
{
	gogo_thread_data	*aligned;
	void	*unaligned;

	while(RO.tl != NULL) {
		aligned = RO.tl;
		RO.tl = aligned->next;
		unaligned = aligned->unaligned;
		free(unaligned);
	}
	return ME_NOERR;
}

MERET EXPORT
MPGE_closeCoder(void)
{
	int ret = 0;

#if defined(MT_ENCODER)
	if(RO.nThread > 1){
		/* finish encoding */
		int i;
		gogo_thread_data *tl;

		if (RO.lame_encode_init==0){
			/* if threads haven't waked up, wake threads up with frame_encode setting */
			RW.trackencode_flag = FALSE;
			RW.getpcm_status = ME_NOERR;
			gogo_unlock_semaphore(&RW.semaphore[SEMAPHORE_STARTENCODE]);
		}

		/* term encoding only when encoding by a frame */
		if( !RW.trackencode_flag && RW.getpcm_status == ME_NOERR ){
			RW.termencode_flag = TRUE;
			ret = gogo_unlock_semaphore(&RW.semaphore[SEMAPHORE_GETPCM_IN]);
			if(ret) return ME_INTERNALERROR;
		}

		for(tl = RO.tl; tl != NULL; tl = tl->next){
			if( gogo_join_thread(&tl->thread) )
				return ME_INTERNALERROR;
			if( gogo_destroy_thread(&tl->thread) )
				return ME_INTERNALERROR;
			if(tl->exit_status != ME_NOERR && tl->exit_status != ME_EMPTYSTREAM) 
				ret = tl->exit_status;
		}

		/* mp3 related stuff.  bit buffer might still contain some mp3 data */
/*		if( RW.end_tl ) flush_bitstream(RW.end_tl);	*/
		flush_bitstream();
/* めもめも: tl不要にした。最後にエンコードしたスレッドローカルがあれば、
	エンコードしたということで、フラッシュしてたんだよね？
	書き換え後は、エンコードしてなければ flush_bitstream() は何もしないので、
	これでいいと思う。けど確かめてください。＞ ＰＥＮさん */

		if( put_mp3() < 0 ){
			return ME_INTERNALERROR;
		}

		for(i= 0; i < MAX_SEMAPHORE; i++){
			if( gogo_destroy_semaphore(&RW.semaphore[i]) )
				return ME_INTERNALERROR;
		}

		for(i= 0; i < MAX_CRITICAL_REGION; i++){
			if( gogo_destroy_mutex(&RW.mutex[i]) )
				return ME_INTERNALERROR;
		}

		if(ret != ME_NOERR) return ret;
	}
#endif

	/* close input device */
	finalizeRead();
 
	/* cloase output device  */
	finalizeWrite();

	ret = finalizeThread();
	if(ret != ME_NOERR) return ret;

	return ME_NOERR;
} /* MPGE_closeCoder */

MERET EXPORT
MPGE_getConfigure(UPARAM mode, void *param )
{
	int		*iparam = (int *)param;

	// エンコードが開始するまで設定取得を呼び出せない
//	if( !musicin )
//		return ME_PARAMERROR;

	switch( mode ){
		case MG_INPUTFILE:
			strcpy( param, BE.inFileName );
			return ME_NOERR;
		case MG_OUTPUTFILE:
			strcpy( param, BE.outFileName );
			return ME_NOERR;
		case MG_ENCODEMODE:
			switch( RO.mode ){
				case MONO:
					*iparam = MC_MODE_MONO;
					return ME_NOERR;
				case STEREO:
					*iparam = MC_MODE_STEREO;
					return ME_NOERR;
				case JOINT_STEREO:
					*iparam = MC_MODE_JOINT;
					return ME_NOERR;
			}
			return ME_PARAMERROR;
		case MG_BITRATE:
			*iparam = BE.rateKbps;
			return ME_NOERR;
		case MG_INPFREQ:
			*iparam = BE.inpFreqHz;
			return ME_NOERR;
		case MG_OUTFREQ:
			*iparam = RO.out_samplerate;	/* RO.encFreqHz; */
			return ME_NOERR;
		case MG_STARTOFFSET:
			*iparam = -1;
			return ME_NOERR;
		case MG_USEPSY:
			*iparam = BE.quality <= 5 ? TRUE : FALSE;
			return ME_NOERR;
		case MG_USEMMX:
			*iparam = ( BE.unit & MU_tMMX ) ? TRUE : FALSE;
			return ME_NOERR;
		case MG_USE3DNOW:
			*iparam = ( BE.unit & MU_t3DN ) ? TRUE : FALSE;
			return ME_NOERR;
		case MG_USESSE:
			*iparam = ( BE.unit & MU_tSSE ) ? TRUE : FALSE;
			return ME_NOERR;
		case MG_USEE3DNOW:
			*iparam = ( BE.unit & MU_tE3DN ) ? TRUE : FALSE;
			return ME_NOERR;
		case MG_USECMOV:
			*iparam = ( BE.unit & MU_tCMOV ) ? TRUE : FALSE;
			return ME_NOERR;
		case MG_USEEMMX:
			*iparam = ( BE.unit & MU_tEMMX ) ? TRUE : FALSE;
			return ME_NOERR;
		case MG_USESSE2:
			*iparam = ( BE.unit & MU_tSSE2 ) ? TRUE : FALSE;
			return ME_NOERR;
		case MG_CLFLUSH:
			*iparam = ( BE.unit & MU_tCLFLUSH ) ? TRUE : FALSE;
			return ME_NOERR;
		case MG_USEALTIVEC:
			*iparam = ( BE.unit & MU_tALTIVEC ) ? TRUE : FALSE;
			return ME_NOERR;
		case MG_USESPC1:
			*iparam = ( BE.unit & MU_tSPC1 ) ? TRUE : FALSE;
			return ME_NOERR;
		case MG_USESPC2:
			*iparam = ( BE.unit & MU_tSPC2 ) ? TRUE : FALSE;
			return ME_NOERR;

		case MG_COUNT_FRAME:
			*iparam = RW.totalframes;		//totalframes;
			return ME_NOERR;
		case MG_NUM_OF_SAMPLES:
			*iparam = RO.framesize;		//gl.frameSize;
			return ME_NOERR;
		case MG_MPEG_VERSION:
			*iparam = 0;		//gl.version;
			return ME_NOERR;
#if	defined(USE_BTHREAD)
		case MG_READTHREAD_PRIORITY:
			*iparam = intReadThreadPriority;
			return ME_NOERR;
#endif
		case MG_FRAME:
			*iparam = RW.frameNum;
			return ME_NOERR;
	}
	return ME_PARAMERROR;
}

MERET EXPORT
MPGE_getVersion( unsigned long *vercode,  char *verstring )
{
	*vercode = VERSION_NUM;
	strcpy( verstring, VERSION );
#if defined(BENCH_ONLY)
	strcat( verstring, " for only benchmark" );
#endif
	return ME_NOERR;
}

#define BITLEN 10
#define SIZE (1<<BITLEN)
static float s_M0p25_idxTbl[256], s_M0p25_powTbl[SIZE];

static void init_powM0p25(void)
{
	int i;
	const float A = -0.25F;
	for (i = 0; i < 256; i++) {
		s_M0p25_idxTbl[i] = pow(2, (i - 127) * A);
	}
	for (i = 0; i < SIZE; i++) {
		s_M0p25_powTbl[i] = pow(1 + i / (double)SIZE, A);
	}
}

/* see http://homepage1.nifty.com/herumi/adv/adv45.html */
static INLINE float pow075sub(float *px)
{
	int s, t;
	float x, y;
	x = *px;
	s = t = *(int *)px;
	s >>= 23;                               /* 指数部 */
	t = (t & ((1<<23)-1)) >> (23 - BITLEN); /* 仮数部 */
	y = s_M0p25_idxTbl[s] * s_M0p25_powTbl[t];
	return (1.25 - 0.25 * x * (y * y) * (y * y)) * y * x;
}
#undef BITLEN
#undef SIZE

/* 53Kclk@PIII => 49Kclk */
/* 31Kclk@K7-500 */
/* 2002/2/3 => 19Kclk@PIII */
/* 2002/2/9 17.8Kclk@PIII */
void pow075_C(float xr[576], float xrpow[576], float *psum, float *pmax, uint32 *xr_sign)
{
	float srpow_sum = 0.0, xrpow_max = 0.0, tmp;
	int i;
	const int n = RO.ixend;
#if 0	///////////////////////////////////
	float __a[576*3+16];
	float *_xr = (float *)(((int)__a + 15) & ~15);
	float *_xrpow = &_xr[576];
	uint32 *_xr_sign = (uint32 *)&_xrpow[576];
	float _psum, _pmax;
void pow075_SSE(float xr[576], float xrpow[576], float *psum, float *pmax, uint32 *xr_sign);
	memcpy(_xr, xr, 576 * 4);
	pow075_SSE(_xr, _xrpow, &_psum, &_pmax, _xr_sign);
#endif//////////////////////////////////////
	for (i = 0; i < n; ++i) {
//		xr_sign[i] = (xr[i] < 0) ? 0xFFFFFFFF : 0;
		xr_sign[i] = (*(int *)&xr[i]) >> 31;
		tmp = xr[i] = fabs (xr[i]);	// 符号はxr_signに入ってる
		srpow_sum += tmp;
#if 0
		/* 値が小さいときに 0 になりつづけると駄目かもしれない */
		/* 駄目なときがわりとあるようなのでこの最適化をやめる */
		xrpow[i] = pow075sub(&xr[i]);
#else
		xrpow[i] = sqrt (tmp * sqrt(tmp));
#endif
		if(xrpow_max < xrpow[i]) xrpow_max = xrpow[i];
	}
	*psum = srpow_sum;
	*pmax = xrpow_max;
#if 0/////////////////////////////////
	puts("(");
	for (i = 0; i < n; i++) {
		if (fabs(xrpow[i]-_xrpow[i])>1e-5)
		{
			printf("%3d pow=(%e,%e)\n", i, xrpow[i], _xrpow[i]);
		}
	}
	if (_psum != *psum || _pmax != *pmax) {
		printf("sum=(%f, %f) max=(%f,%f)\n", *psum, _psum, *pmax, _pmax);
	}
	puts(")");
	{
		static ccc = 0;
//		ccc++;if(ccc==78)exit(1);
	}
//	exit(1);
#endif/////////////////////////////////
}

#ifdef CPU_I386
void pow075_SSE(float xr[576], float xrpow[576], float *psum, float *pmax, uint32 *xr_sign);
void pow075_SSE2(float xr[576], float xrpow[576], float *psum, float *pmax, uint32 *xr_sign);
void pow075_E3DN_HI(float xr[576], float xrpow[576], float *psum, float *pmax, uint32 *xr_sign);
void pow075_3DN_HI(float xr[576], float xrpow[576], float *psum, float *pmax, uint32 *xr_sign);
void pow075_E3DN_FAST(float xr[576], float xrpow[576], float *psum, float *pmax, uint32 *xr_sign);
void pow075_3DN_FAST(float xr[576], float xrpow[576], float *psum, float *pmax, uint32 *xr_sign);
void pow075_MMX(float xr[576], float xrpow[576], float *psum, float *pmax, uint32 *xr_sign);
#endif

#ifdef CPU_I386
void setup_pow075( int unit )
{
//	if( unit & MU_tSSE2 ){
//		pow075 = pow075_SSE2;	/* 速くない... */
//	}else
	if( (unit & MU_tSSE) && (unit & MU_tMMX) && (unit & MU_t3DN) && (unit & MU_tE3DN)){
		// for Athlon-XP
		if (USE_LOW_PRECISIOIN) { // _SSE 3.90kclk _E3DN_HI 4.00kclk _E3DN_FAST 2.53Kclk
			pow075 = pow075_E3DN_FAST;
		} else 
		{
			pow075 = pow075_SSE;
		}
	}else
	if( unit & MU_tSSE ){
		pow075 = pow075_SSE;
	}else
	if( (unit & MU_tMMX) && (unit & MU_t3DN) ){
		if (USE_LOW_PRECISIOIN) {
			if( unit & MU_tE3DN ){
				pow075 = pow075_E3DN_FAST;
			}else
			{
				pow075 = pow075_3DN_FAST;
			}
		} else 
		{
			if( (unit & MU_tE3DN) && (unit & MU_tEMMX) ){
				// pow075_E3DN_HI は EMMX を使っていないが
				// K6-2/K6-3 は pow075_3DN_HI を使いたいので
				pow075 = pow075_E3DN_HI; 
			}else
			{
				pow075 = pow075_3DN_HI;
			}
		}
	}else
	{
		init_powM0p25();
		pow075 = pow075_C;
	}
}
#else
void setup_pow075( int unit )
{
	init_powM0p25();
}
#endif

// psymodel.c の ms_convert_short, ms_convert_long もろともASMしてちょ
/* convert from L/R <-> Mid/Side */
/* NEED asm */
//	統合前     psymodel.c の ms_convert_short, ms_convert_long とここの平均
//              2.60kclk@K7-500
//	2002-01-20 psymodel.c の ms_convert_short, ms_convert_long と統合
//             2.55kclk@K7-500 by kei
//             早くなりそうに見えないんだけど，統合したことによるキャッシュ効果？

// n は 8 の倍数
void ms_convert_C(FLOAT8* srcl, FLOAT8* srcr, int n)
{
    FLOAT8 l0;
    FLOAT8 r0;
    int i;

    for (i = 0; i < n; i ++) {
        l0 = *srcl;
        r0 = *srcr;
        *srcl++ = (l0+r0) * (FLOAT8)(SQRT2*0.5);
        *srcr++ = (l0-r0) * (FLOAT8)(SQRT2*0.5);
    }
}

#ifdef CPU_I386
void ms_convert_3DN(FLOAT8* srcl, FLOAT8* srcr, int n);
void ms_convert_SSE(FLOAT8* srcl, FLOAT8* srcr, int n);

void setup_ms_convert( int unit )
{
	if( (unit & MU_tSSE) ) {
		ms_convert = ms_convert_SSE;
	}else
	if( (unit & MU_tMMX) && (unit & MU_t3DN) ){
		ms_convert = ms_convert_3DN;
	}else
	{
		ms_convert = ms_convert_C;
	}
}
#endif

void shiftoutpfb_C(void* dest, void* src)
{
	memcpy(dest, RW.subband_buf, 2*18*SBLIMIT*sizeof(float));
	memcpy(RW.subband_buf, src, 2*18*SBLIMIT*sizeof(float));
}

#ifdef CPU_I386
void shiftoutpfb_E3DN(void* dest, void* src);
void shiftoutpfb_SSE(void* dest, void* src);

void setup_shiftoutpfb( int unit )
{
	if( (unit & MU_tSSE) ) {
		shiftoutpfb = shiftoutpfb_SSE;
	}else
	if( (unit & MU_tMMX) && (unit & MU_t3DN) && (unit & MU_tE3DN) ){
		shiftoutpfb = shiftoutpfb_E3DN;
	}else
	{
		shiftoutpfb = shiftoutpfb_C;
	}
}
#endif

#include "encoder.c"
#if defined(MT_ENCODER)
#  undef MT_ENCODER
#  define ST_ENCODER
#include "encoder.c"
#endif
