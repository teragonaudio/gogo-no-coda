/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#ifndef THREAD_H_
#define THREAD_H_

/* プラットホーム非依存 */

typedef void* (*gogo_thread_func)(void *); 

/*
 *	マルチスレッドに対応する場合は、
 *	そのプラットホームに合わせて以下の型と関数またはマクロを用意すること
 *
 *	プラットホーム依存の型 
 *
 *	gogo_thread	スレッド情報を格納する型
 *	gogo_mutex	ミューテックス情報を格納する型
 *  gogo_semaphore セマフォ情報を格納する型
 *
 *	プラットホーム依存の関数またはマクロ 
 *	int を返す関数は正常終了で 0 を返し、異常終了で非 0 を返すこと。
 *
 *	スレッド関係の初期化。他のスレッド関係関数を使う前に一度だけ呼ばれる。
 *	int gogo_initialize_thread_unit(void)
 *
 *	スレッド関係の終了処理。一度だけ呼ばれる。これを呼んだ後、他のスレッド関数群を呼んではいけない。
 *	int gogo_finalize_thread_unit(void)
 *
 *	スレッドを生成する。
 *	int gogo_create_thread(gogo_thread* pthread, gogo_thread_func func, void *data)
 *
 *	スレッドの終了を待つ。
 *	int gogo_join_thread(gogo_thread* pthread)
 *
 *	スレッドを破棄する。破棄する前に終了を待つこと。
 *	int gogo_destroy_thread(gogo_thread* pthread)
 *
 *	ミューテックスを生成する。
 *	再帰ミューテックスかどうかはプラットホーム依存なので、再帰かどうかに依存しないように使うこと。
 *	int gogo_create_mutex(gogo_mutex* pmutex)
 *
 *	ミューテックスを破棄する。
 *	int gogo_destroy_mutex(gogo_mutex* pmutex)
 *
 *	ミューテックスを獲得する。獲得するできるまで永遠に待つ。
 *	int gogo_lock_mutex(gogo_mutex* pmutex)
 *
 *	ミューテックスを解放する。
 *	int gogo_unlock_mutex(gogo_mutex* pmutex)
 *
 *	セマフォを生成する。
 *	初期値0の2値セマフォ。
 *  int gogo_create_semaphore(gogo_semaphore* pSemaphore)
 *
 *	セマフォを破棄する。
 *  int gogo_destroy_semaphore(gogo_semaphore* pSemaphore)
 *
 *	セマフォを獲得する。獲得するできるまで永遠に待つ。
 *  int gogo_lock_semaphore(gogo_semaphore* pSemaphore)
 *
 *	セマフォを解放する。
 *  int gogo_unlock_semaphore(gogo_semaphore* pSemaphore)
 *
 *	セマフォを獲得する。獲得できるまで timeout ミリ秒待つ。
 *  獲得できたら 0 を返す。エラーまたは timeout したら、非0を返す。
 *  int gogo_trylocktimeout_semaphore(gogo_semaphore* pSemaphore, int timeout)
 *
 *	他のスレッドに実行権を与える。
 *  void gogo_yield_thread()						
 *
 *	CPU の数を *pCPUs に、生成するスレッド数を *pTHREADs に返す。
 *	わからない場合は両方必ず1にすること。
 *	int gogo_get_cpu_count(int *pCPUs, int *pTHREADs)
 *
 *	スレッドローカルデータ構造体に置きたい変数があれば、
 *	記号定数 GOGO_THREAD_VARIABLES に定義してください。
 *
 */

#define	MT_ENCODER

#if		defined(USE_WINTHREAD)
#  include	"../win/thread_win.h"
#elif	defined(USE_PTHREAD)
#  include	"../pthread/thread_pthread.h"
#elif defined(USE_BTHREAD)
#  include	"../be/thread_be.h"
#elif defined(USE_OS2THREAD)
#  include	"../os2/thread_os2.h"
#elif defined(USE_RFORK)
#  include	"../rfork/thread_rfork.h"
#else
#  undef	MT_ENCODER
#  define	ST_ENCODER
#  define	gogo_get_cpu_count(pCPU,pTHREADs) ((*(pCPU)=1),(*(pTHREADs)=1),0)
#endif

typedef unsigned int uint32;

/**
 * プラットホーム非依存ローカル変数
 */
typedef struct gogo_thread_data_s {
/* プラットホーム非依存ローカル変数 */
	float	mfbuf[2][4][576];	/* ->FFT, ->PFB */

  /* variables for newmdct.c */
        float	(*sb[4])[18][SBLIMIT];
        float	sb_sample[3][2][18][SBLIMIT];  /* PFB -> MDCT */

        float	xr[2][2][576];         /* MDCT -> Non-Linear Quantization  */
	uint32	xr_sign[2][2][576];

        float	xrpow[2][2][576];      /* Non-Linear Quantization -> Iteration Loop */
	float	work_xrpow[576]; //, save_xrpow[576], best_xrpow[576];	/* quantize.c */
        float	xrpow_sum[2][2];
        float	xrpow_max[2][2];

	int	l3_enc[2][2][576];
	III_scalefac_t	scalefac[2][2];

	int	ath_over[2][2];		/* calc_xmin() -> calc_noise() */
	III_psy_xmin	l3_xmin[2][2];	/* calc_xmin() -> calc_noise() */

	/* fft and energy calculation    */
        float	wsamp_L[2 /*granule*/][2 /*channel*/][BLKSIZE];     /* fft() -> L3psycho_anal() */
        float	wsamp_S[2 /*granule*/][2 /*channel*/][3][BLKSIZE_s];/* fft() -> L3psycho_anal() */
/* BLKSIZE=1024, BLKSIZE_s = 256 => max(BLKSIZE,BLKSIZE_s*3) = 1024 */
//		float	psywork[BLKSIZE*4];		/* wsampl_{L,S} 4つ分(energyと共用したほうがよいか?) */
		float	psywork[576*2*4*4];		/* FFTでもwork(576*2*4*4*4byte)として使う */
		                                /* 寿命が重ならない quantize.c の distort でも共用して使う */
        float	energy[HBLKSIZE*4];              /* 領域を4倍にすればFFT後にまとめて処理可能 */
        float	energy_s[3][HBLKSIZE_s*4];       /* 領域を4倍にすればFFT後にまとめて処理可能 */

	/* psycho acoustics */
	float	tot_ener[4];	/* private copy of RW.tot_ener */
	III_side_info_t	l3_side;

/* プラットホーム依存スレッドローカル変数 */
#ifdef GOGO_THREAD_VARIABLES
	GOGO_THREAD_VARIABLES;
#endif

/* アライメント関係ないものはここから下へ */
	struct gogo_thread_data_s	*next;
	void	*unaligned;
	int	tid;

	int	padding;		  /* padding for the current frame? */
	int	mode_ext;

	int	bitrate_index;
	int	ResvSize;

	// WMP6.4対応のため ancillary を削って part2_3_length を追加しているのだけど
	// その追加するビット数
	int additional_part2_3_length[2 /*granule*/][2 /*channel*/]; 

#ifdef	MT_ENCODER
	int	exit_status;
	gogo_thread	thread;
	gogo_mutex	*critical_region;
#endif
} gogo_thread_data;	// <- gogo2のencodeframe_arg_t相当

#endif // THREAD_H_

