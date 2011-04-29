/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#if	defined(__linux__) || defined(__ppc__) || defined(__FreeBSD__)
#define	USE_ITIMER
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#endif
#if defined(__linux__) || defined(__os2__) || defined(__ppc__) || defined(__CYGWIN32__) || defined(__FreeBSD__)
#  define	min(x,y)	(((x)<(y))?(x):(y))
#  define	max(x,y)	(((x)>(y))?(x):(y))
#endif
#if defined(WIN32)
#include <windows.h>
#include <float.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gogo.h"

#ifndef NOERR
	#define NOERR 0
	#define ERR 1
#endif
#ifndef	TRUE
#  define	TRUE 1
#endif
#ifndef	FALSE
#  define	FALSE 0
#endif

static void usage(void);
static void opening(void);
static void putConfig(void);

static int getOpt(int argc, char *argv[]);
static char *getErrMsg(MERET code);

static int	silent, debug, libsnd, bench, bench_sec;

// UI側で面倒見なきゃならんから移動。
#define isKANJI1(x) ((unsigned char)((x^0x20)-0xA1)<=0x3B)

#if defined(WIN32)
	#define ISFILESEP( VAL )		((VAL)=='\\' || (VAL) == '/')
#else
#if defined(__HIGHC__)
	#define ISFILESEP( VAL )		((VAL)=='\\')
#else
	#define ISFILESEP( VAL )		((VAL) == '/')
#endif // HIGHC
#endif // WIN32


#ifdef WIN32
	#define MAX_FILE_LEN 260
#elif BeOS
	#define MAX_FILE_LEN 1024
#else
	#define MAX_FILE_LEN 256
#endif

static int convertToLongFileName(char *fname, const unsigned int maxLen)
{
#if !defined(WIN32)
        return NOERR;
#else
        LPHANDLE hdl;
        WIN32_FIND_DATA info;
        int i, j;
        hdl = FindFirstFile(fname, &info);
        if(hdl == INVALID_HANDLE_VALUE) return ERR;
        if(!*info.cFileName){
                FindClose(hdl);
                return ERR;
        }

        i = (fname[1] == ':') ? 2 : 0;  /* for drive */
        j = i - 1;
        while(i < (int)strlen(fname)){
#if defined(JAPANESE)
                if(isKANJI1(fname[i])){
                        i++;
                }else
#endif
                {
                        if( ISFILESEP( fname[i] ) ) j = i;
                }
                i++;
        }
        fname[j+1] = '\0';       /* path名を取り出す */
        if(strlen(fname) + strlen(info.cFileName) >= maxLen) return ERR;
        strcat(fname, info.cFileName);
//      if( debug ) fprintf( stderr, "long file name : %s\n", fname );
        FindClose(hdl);
        return NOERR;
#endif /* WIN32 */
} /* convertToLongFileName */
#undef isKANJI1

/* change suffix of src to mp3  */
/* suppose sizeof(dest)>=sizeof(src) */
static int
changeSuffix(char *dest, const char *src, const size_t maxLen)
{
        char *p;
        size_t i, len;
        len = strlen(src);
        if(len >= maxLen) return ERR;
        strcpy(dest, src);
        p = &dest[len - 1];
        len = min(len, 4); /* if the len of suffix < 4 then replace it with "mp3" */
        for(i = 0; i< len; i++, p--){
                if(*p == '\\' || *p == '/')break; /* no suffix */
                if(*p == '.'){
                        *p = '\0';
                        break;
                }
        }
        if(strlen(dest) + 4 >= maxLen) return ERR;
        strcat(dest, ".mp3");
        return NOERR;
} /* changeSuffix */

/*********************************************************************/

void
putCPUinfo(void)
{
	unsigned long unit;

	if(silent) return;

	MPGE_getUnitStates(&unit);
#if defined(WIN32) || defined(__linux__) || defined(DOS) || defined(_M_IX86) || defined(__os2__) || defined(_X86_) || defined(__FreeBSD__)
    if(unit & MU_tFAMILY4) fprintf( stderr, "family 4:");
	else
    if(unit & MU_tFAMILY5) fprintf( stderr, "family 5:");
	else
    if(unit & MU_tFAMILY6) fprintf( stderr, "family 6:");
	else
    if(unit & MU_tFAMILY7) fprintf( stderr, "family 7(or over):");

    fprintf( stderr, "vendor ");
    if(unit & MU_tINTEL) fprintf( stderr, "Intel\n");
    else
    if(unit & MU_tAMD  ) fprintf( stderr, "AMD\n");
    else
    if(unit & MU_tCYRIX) fprintf( stderr, "Cyrix\n");
    else
    if(unit & MU_tIDT  ) fprintf( stderr, "IDT\n");
    else fprintf( stderr, "unknown\n");

    fprintf( stderr, "extended instruction - ");
    if(unit & MU_tMMX ) fprintf( stderr, "MMX ");
    if(unit & MU_tEMMX ) fprintf( stderr, "EMMX ");
    if(unit & MU_tCMOV) fprintf( stderr, "CMOV ");
    if(unit & MU_t3DN ) fprintf( stderr, "3D Now! ");
    if(unit & MU_tE3DN) fprintf( stderr, "Ext. 3D Now! ");
    if(unit & MU_tSSE ) fprintf( stderr, "SSE ");
    if(unit & MU_tSSE2) fprintf( stderr, "SSE2 ");
    if(unit & MU_tCLFLUSH) fprintf( stderr, "CLFLUSH ");
#elif defined(__MWERKS__) || defined(__ppc__)
	fprintf( stderr, "PowerPC\n");
	fprintf( stderr, "extended instruction - ");
	if(unit & MU_tALTIVEC) fprintf( stderr, "AltiVec ");
#endif
    fprintf( stderr, "\n");

} /* putCPUinfo */

/******************   for report   ******************/
static unsigned int totalframes, frameNum, freqHz, samplesPerFrame;
#ifdef	USE_ITIMER
static struct timeval   ElapsedTime;
static volatile struct itimerval itimer;
static struct itimerval old_itimer;
static struct sigaction new_sig, old_sig;
#elif	defined( USE_TTIMER )
#include "global.h"
#include "thread.h"
static void *			reportThread(gogo_thread_data *tl);
static volatile	clock_t startTime, curTime, beforeTime;
static gogo_thread		gThreadShow;
static gogo_semaphore   gTimerEnabledSemaphore;
#else
static clock_t startTime, curTime, beforeTime;
#endif



// ASM版を使う？
static void
write_time_C(int fd, long elapsed_sec, long elapsed_usec, unsigned frame_num,
        unsigned tot_frame, unsigned sps, unsigned spf)
{
	char	buf[1024];
	double	dclock, frame_per, total_sec;
	int	eh,em,es,ems, h,m,s,ms, t;

	dclock = max(elapsed_sec + elapsed_usec*0.000001, 0.001);
	total_sec = (frame_num > 0)? dclock * tot_frame / frame_num: dclock;
	frame_per = (tot_frame > 0)? 100.0*(double)frame_num/(double)tot_frame: 0;

	t = (int)(total_sec * 100.0);
	ms = t % 100; t /= 100;
	s  = t % 60 ; t /= 60;
	m  = t % 60 ; t /= 60;
	h  = t;

	t = (int)((total_sec-dclock) * 100.0);
	ems = t % 100; t /= 100;
	es  = t % 60 ; t /= 60;
	em  = t % 60 ; t /= 60;
	eh  = t;

	sprintf(buf, "{%7d/%7d} %4.1f%% (%6.2fx)  "
        "re:[%02d:%02d:%02d.%02d] to:[%02d:%02d:%02d.%02d]\r",
	frame_num, tot_frame, frame_per,
	(dclock > 0)? (frame_num * (double)spf) / ((double)sps * dclock): 0.0,
	eh,em,es,ems, h,m,s,ms);
	write(fd, buf, strlen(buf));
}

// if(n == 0) の場合32フレームごと表示。
// シグナルハンドラとして呼ばれた場合は(n != 0)
static void
reportDsp(int n)
{
#ifdef	USE_ITIMER
	ElapsedTime.tv_sec += itimer.it_interval.tv_sec;
	ElapsedTime.tv_usec += itimer.it_interval.tv_usec;
	ElapsedTime.tv_sec += ElapsedTime.tv_usec/1000000;
	ElapsedTime.tv_usec = ElapsedTime.tv_usec%1000000;
	MPGE_getConfigure(MG_FRAME, &frameNum);
	if(totalframes < frameNum) MPGE_getConfigure(MG_COUNT_FRAME, &totalframes);
	write_time_C(fileno(stderr), ElapsedTime.tv_sec, ElapsedTime.tv_usec,
		frameNum, totalframes, freqHz, samplesPerFrame);
#else
	if(silent) return;

	MPGE_getConfigure(MG_FRAME, &frameNum);
	if((n == 0) && (frameNum & 31) != 0) return;

	curTime = clock();
	if((n == 0) && (curTime - beforeTime < CLOCKS_PER_SEC / 5 )) return;
	if(totalframes < frameNum) MPGE_getConfigure(MG_COUNT_FRAME, &totalframes);
	beforeTime = curTime;
	curTime -= startTime;
	write_time_C(fileno(stdout), curTime/CLOCKS_PER_SEC,
		((long)curTime%(long)CLOCKS_PER_SEC)*(1000000L/CLOCKS_PER_SEC),
		frameNum, totalframes, freqHz, samplesPerFrame);
#endif
}

static void
reportInit(void)
{
#ifdef USE_ITIMER
       	ElapsedTime.tv_sec = ElapsedTime.tv_usec = 0;
	if(silent){
		gettimeofday(&itimer.it_value, NULL);
	}else{
        	itimer.it_interval.tv_sec = 0;
        	itimer.it_interval.tv_usec = 200000;    /* 更新周期 = 200ミリ秒 */
        	itimer.it_value = itimer.it_interval;
        	memset(&new_sig, 0, sizeof(struct sigaction));
        	new_sig.sa_handler = reportDsp;
        	new_sig.sa_flags = SA_RESTART;
        	sigaction(SIGALRM, &new_sig, &old_sig);
        	setitimer(ITIMER_REAL, &itimer, &old_itimer);
	}
#else
#if		defined(USE_TTIMER)
	gogo_create_semaphore(&gTimerEnabledSemaphore);
	gogo_lock_semaphore(&gTimerEnabledSemaphore);
	gogo_create_thread( &gThreadShow, (gogo_thread_func)reportThread, NULL );
#endif
	startTime = clock();
	beforeTime = startTime;
#endif
	MPGE_getConfigure(MG_OUTFREQ, &freqHz);
	MPGE_getConfigure(MG_NUM_OF_SAMPLES, &samplesPerFrame);
	totalframes = 0;
	frameNum = 0;
}

static void
reportTerm(void)
{
#ifdef USE_ITIMER
	if(silent){
		gettimeofday(&itimer.it_interval, NULL);
	}else{
        	setitimer(ITIMER_REAL, &old_itimer, &itimer);
        	sigaction(SIGALRM, &old_sig, NULL);
	}
       	itimer.it_interval.tv_sec  -= itimer.it_value.tv_sec;
       	itimer.it_interval.tv_usec -= itimer.it_value.tv_usec;
/*	if(itimer.it_interval.tv_usec < 0){
		itimer.it_interval.tv_sec--;
		itimer.it_interval.tv_usec += 1000000L;
	}	*/
#endif
#if		defined(USE_TTIMER)
	gogo_unlock_semaphore(&gTimerEnabledSemaphore);
	gogo_wait_terminatethread( &gThreadShow );
	gogo_destroy_semaphore(&gTimerEnabledSemaphore);
#endif
	reportDsp(1);
	fputc('\n', stderr);
	fflush(stderr);
}

#if		defined( USE_TTIMER )
static void *
reportThread(gogo_thread_data *tl)
{
	while(gogo_trylocktimeout_semaphore(&gTimerEnabledSemaphore, 200)) {
		reportDsp( 1 );
	}
	return	NULL;
}
#endif

extern void clkput(void);
int
main(int argc, char *argv[])
{
	MERET ret;
	int getOptSuccessed;

#if defined(_MSC_VER) && !defined(NDEBUG)
	/* 厳密な演算チェックのため */
	_controlfp(~_MCW_EM | _EM_INEXACT, _MCW_EM);
#endif
#if	defined(__FreeBSD__)
	{	/* デフォルト動作が他のOSと違う。*/
		struct sigaction new_sig;
        	memset(&new_sig, 0, sizeof(struct sigaction));
        	new_sig.sa_handler = SIG_IGN;
        	sigaction(SIGFPE, &new_sig, NULL);
	}
#endif
	if(MPGE_initializeWork() == ME_NOFPU) return ERR;

	getOptSuccessed = 1 < argc && getOpt(argc, argv) == NOERR;
	opening();
	if(!getOptSuccessed){
		usage();
		MPGE_endCoder();
		return ERR;
	}

	ret = MPGE_detectConfigure();
	if(ret != ME_NOERR){
		if(!silent) fprintf(stderr, "%s\n", getErrMsg(ret));
		return ERR;
	}

	putConfig();

	reportInit();	/* report progress */

#if defined(USE_ITIMER) || (defined(_CONSOLE) && defined(WIN32) && defined( USE_TTIMER ))
	ret = MPGE_processTrack();
	if(ret != ME_NOERR){
		if(!silent) fprintf(stderr, "%s\n", getErrMsg(ret));
	}
#else
	for(;;){
		ret = MPGE_processFrame();
		if(ret == ME_EMPTYSTREAM) break;
		if(ret != ME_NOERR){
			if(!silent) fprintf(stderr, "%s\n", getErrMsg(ret));
			break;
		}
		reportDsp(0);
	} /* main loop */
#endif
	ret = MPGE_closeCoder();
	if(ret != ME_NOERR) if(!silent) fprintf(stderr, "%s\n", getErrMsg(ret));

	reportTerm();

	MPGE_endCoder();
	clkput();
	return NOERR;
} /* main */

/*************************** option analysis **********************************/
/* no option argument */
static int
tNONE(UPARAM *para, const char *arg)
{
	*para = TRUE;
	return 0;	/* stop, no error */
}

/* integer */
static int
tINT(UPARAM *para, const char *arg)
{
	char *endptr;

	*para = (UPARAM)strtol(arg, &endptr, 10);
	if(*endptr) return -1;	/* stop, error */
	return 1;		/* continue */
}

/* (int)(float_para * 1000 + 0.5) */
static int
tX1000(UPARAM *para, const char *arg)
{
	char *endptr;

	*para = (UPARAM)(strtod(arg, &endptr) * 1000.0 + 0.5);
	if(*endptr) return -1;	/* stop, error */
	return 1;		/* continue */
}

/* on/off */
static int
tBOOL(UPARAM *para, const char *arg)
{
	if(!strcmp(arg, "on")){
		*para = TRUE;
		return 1;	/* continue */
	}else if(!strcmp(arg, "off")){
		*para = FALSE;
		return 1;	/* continue */
	}
	return -1;		/* stop, error */
}

/* pointer of string */
static int
tPTR(UPARAM *para, const char *arg)
{
	*para = (UPARAM)arg;
	return 1;		/* continue */
}

/* igonore(unsupported?) */
static int
tIGNORE(UPARAM *para, const char *arg)
{
	return -1;		/* stop, error */
}

typedef struct{
	char cmd[8];
	UPARAM para;
} cmd_tbl_t;

/* selection */
static int
tM(UPARAM *para, const char *arg)
{
	int	i;
	const static cmd_tbl_t cmd_tbl[] = {
		{"m", MC_MODE_MONO},
		{"s", MC_MODE_STEREO},
		{"j", MC_MODE_JOINT},
		{"f", MC_MODE_MSSTEREO},
		{"d", MC_MODE_DUALCHANNEL}
	};

	for(i = 0; i < (sizeof cmd_tbl)/sizeof(cmd_tbl_t); i++){
		if(strcmp(cmd_tbl[i].cmd, arg) == 0){
			*para = (UPARAM)cmd_tbl[i].para;
			return 1;	/* continue */
		}
	}
	return -1;			/* stop, error */
}

/* selection */
static int
tEMP(UPARAM *para, const char *arg)
{
	int	i;
	const static cmd_tbl_t cmd_tbl[] = {
		{"n", MC_EMP_NONE},
		{"5", MC_EMP_5015MS},
		{"c", MC_EMP_CCITT}
	};

	for(i = 0; i < (sizeof cmd_tbl)/sizeof(cmd_tbl_t); i++){
		if(strcmp(cmd_tbl[i].cmd, arg) == 0){
			*para = (UPARAM)cmd_tbl[i].para;
			return 1;	/* continue */
		}
	}
	return -1;			/* stop, error */
}


/* selection */
static int
tRIF(UPARAM *para, const char *arg)
{
	int	i;
	const static cmd_tbl_t cmd_tbl[] = {
		{"wave", MC_OUTPUT_RIFF_WAVE},
		{"rmp",  MC_OUTPUT_RIFF_RMP}
	};

	for(i = 0; i < (sizeof cmd_tbl)/sizeof(cmd_tbl_t); i++){
		if(strcmp(cmd_tbl[i].cmd, arg) == 0){
			*para = (UPARAM)cmd_tbl[i].para;
			return 1;	/* continue */
		}
	}
	return -1;			/* stop, error */
}


#if defined(WIN32)
/* selection */
static int
tPRI(UPARAM *para, const char *arg)
{
	int	i;
	const static cmd_tbl_t cmd_tbl[] = {
		{"highest", THREAD_PRIORITY_TIME_CRITICAL},
		{"high",    THREAD_PRIORITY_HIGHEST},
		{"normal",  THREAD_PRIORITY_NORMAL},
		{"low",     THREAD_PRIORITY_BELOW_NORMAL},
		{"lowest",  THREAD_PRIORITY_IDLE}
	};

	for(i = 0; i < (sizeof cmd_tbl)/sizeof(cmd_tbl_t); i++){
		if(strcmp(cmd_tbl[i].cmd, arg) == 0){
			*para = (UPARAM)cmd_tbl[i].para;
			return 1;	/* continue */
		}
	}
	return -1;			/* stop, error */
}
#endif

enum{
	CON_DEBUG = 1,
	CON_SILENT,
	CON_TEST,
	CON_DELETE,
	CON_LANG,
	CON_LIBSND,
};

typedef struct{
	char name[12];
	int (*type[2])(UPARAM *, const char *);
	UPARAM mode; /* != 0 */
} opt_t;

/* table of options for only console */
static opt_t optTblCON[] = {
	{"debug",	{tNONE,  tNONE},  CON_DEBUG},
	{"silent",	{tNONE,  tNONE},  CON_SILENT},
	{"test",	{tINT,   tNONE},  CON_TEST},
	{"delete",	{tNONE,  tNONE},  CON_DELETE},
	{"lang",	{tPTR,   tNONE},  CON_LANG},
	/* ここから下はぷち午後で新設 */
	{"libsnd",	{tNONE,   tNONE},  CON_LIBSND},
	{"",		{tIGNORE,tIGNORE},	0}
};

/* table of options for GOGO-API */
static opt_t optTblDLL[] = {
	{"m",		{tM,     tNONE},  MC_ENCODEMODE},
//	{"bswap",	{tNONE,  tNONE},  MC_BYTE_SWAP},
	{"b",		{tINT,   tNONE},  MC_BITRATE},
//	{"br",		{tIGNORE,tNONE},  0},
	{"s",		{tX1000, tNONE},  MC_INPFREQ},
	{"d",		{tX1000, tNONE},  MC_OUTFREQ},
	{"emp",		{tEMP,   tNONE},  MC_EMPHASIS},
#if defined(WIN32) || defined(__linux__) || defined(DOS) || defined(_M_IX86) || defined(__os2__) || defined(_X86_) || defined(__FreeBSD__)
	{"mmx",		{tBOOL,  tNONE},  MC_USEMMX},
	{"3dn",		{tBOOL,  tNONE},  MC_USE3DNOW},
	{"sse",		{tBOOL,  tNONE},  MC_USESSE},
	{"kni",		{tBOOL,  tNONE},  MC_USESSE},
	{"e3dn",	{tBOOL,  tNONE},  MC_USEE3DNOW},
	{"cmov",	{tBOOL,  tNONE},  MC_USECMOV},
	{"emmx",	{tBOOL,  tNONE},  MC_USEEMMX},
	{"sse2",	{tBOOL,  tNONE},  MC_USESSE2},
#elif defined(__MWERKS__) || defined(__ppc__)
	{"altivec",	{tBOOL,  tNONE},  MC_USEALTIVEC},
#endif
	{"lpf",		{tBOOL,  tNONE},  MC_USELPF16},
	{"th",		{tINT,   tINT},   MC_MSTHRESHOLD},
	{"i",		{tNONE,  tNONE},  MC_VERIFY},
	{"cpu",		{tINT,   tNONE},  MC_CPU},
#if defined(BeOS)
	{"priority",	{tINT,   tNONE},  MC_THREAD_PRIORITY},
	{"readthread",	{tINT,   tNONE},  MC_READTHREAD_PRIORITY},
#elif defined(WIN32)
	{"priority",	{tPRI,   tNONE}, MC_THREAD_PRIORITY},
#endif
	{"spc1",	{tNONE,  tNONE},  MC_USESPC1},
	{"spc2",	{tNONE,  tNONE},  MC_USESPC2},
	{"v",		{tINT,   tNONE},  MC_VBR},
	{"vb",		{tINT,   tINT},   MC_VBRBITRATE},
	{"offset",	{tINT,   tNONE},  MC_STARTOFFSET},
	{"8bit",	{tNONE,  tNONE},  MC_8BIT_PCM},
	{"mono",	{tNONE,  tNONE},  MC_MONO_PCM},
	{"tos",		{tNONE,  tNONE},  MC_TOWNS_SND},
	{"o",		{tPTR,   tNONE},  MC_OUTPUTDIR},
	{"nopsy",	{tNONE,  tNONE},  MC_USEPSY},
	{"riff",	{tRIF,   tNONE}, MC_OUTPUT_FORMAT},
	/* ここから下はぷち午後で新設 */
	{"q",		{tINT,   tNONE},  MC_ENCODE_QUALITY},
	{"a",		{tNONE,  tNONE},  MC_ABR},
	{"lametag",	{tBOOL,  tNONE},  MC_WRITELAMETAG},
	{"vbrtag",	{tBOOL,  tNONE},  MC_WRITEVBRTAG},
	{"",		{tIGNORE,tIGNORE},0}
};

static int
setConsoleOpt(UPARAM mode, UPARAM *para)
{
	switch(mode){
	case CON_DEBUG:
		debug = 1;
		break;
	case CON_SILENT:
		silent = 1;
		break;
	case CON_LIBSND:
		libsnd = 1;
		break;
	case CON_TEST:
		bench = 1;
		bench_sec = *para;
		break;
	default:
	case CON_LANG:
	case CON_DELETE:
		if(!silent) fprintf(stderr, "not supported\n");
		return ERR;
	}
	return NOERR;
} /* setConsoleOpt */

/* return: the number of used args */
static int
getOpt1(opt_t *tbl, int argc, char *argv[], UPARAM *para, UPARAM *mode)
{
	int i, num, sel, used;

	/* search option in table */
	for(sel = 0; tbl[sel].name[0]; sel++){
		if(!strcmp(*argv + 1, tbl[sel].name)){
			argc--, argv++;
			*mode = tbl[sel].mode;

			para[0] = para[1] = TRUE;
			num = min(2, argc);

			for(i = 0; i < num; i++){
				used = (*tbl[sel].type[i])(para++, *argv++);
				if(used == 0){
					break;
				}else if(used < 0){
					return used;
				}
			} /* for(i = 0;...) */
			return i + 1;
		}
	}
	return -1;
} /* getOpt1 */

static int
getOpt(int argc, char *argv[])
{
	char inName[MAX_FILE_LEN], outName[MAX_FILE_LEN];
	char *inPtr, *outPtr;

	inPtr = outPtr = NULL;
	argc--, argv++;

	while(argc > 0){
		if(**argv == '-'){
			int usedArgNum;
			UPARAM para[2], mode;

			/* for MPGE_setConfigure */
			usedArgNum = getOpt1(optTblDLL, argc, argv, para, &mode);

			if(usedArgNum > 0){
				/* converse(special case) */
				if(mode == MC_USEPSY) para[0] = !para[0];

				if(MPGE_setConfigure(mode, para[0], para[1]) != ME_NOERR) goto OPT_ERR;

				argc -= usedArgNum;
				argv += usedArgNum;
				continue;
			}

			/* for console option */
			usedArgNum = getOpt1(optTblCON, argc, argv, para, &mode);

			if(usedArgNum > 0){
				if(setConsoleOpt(mode, para) == ERR) goto OPT_ERR;
				argc -= usedArgNum;
				argv += usedArgNum;
				continue;
			}
			goto OPT_ERR;
		}else if(!inPtr){
			inPtr = *argv;
		}else if(!outPtr){
			outPtr = *argv;
		}else{
			goto OPT_ERR;
		}
		argc--, argv++;
	} /* while(argc) */

	if (bench) {
		struct MCP_INPDEV_USERFUNC user_in;
		MPGE_USERFUNC user_out;

		memset( &user_in, 0, sizeof(user_in) );
		user_in.nSize = 44100 * 2 * 2 * bench_sec;
		user_in.nChn  = 2;				// stereo
		user_in.nFreq = 44100;			// 44100Hz
		user_in.nBit  = 16;			// 16bit PCM

		//	エラーの時はどうしよう？？？
		init_bench_input(&user_in);
		init_bench_output(&user_out);

		MPGE_setConfigure( MC_INPUTFILE, MC_INPDEV_USERFUNC, (UPARAM)&user_in );
		MPGE_setConfigure( MC_OUTPUTFILE, MC_OUTDEV_USERFUNC, (UPARAM)user_out );
		if( !silent ){
			printf( "test time %dsec\n", bench_sec );
		}
	}
	else {
		if(!inPtr || strlen(inPtr) >= MAX_FILE_LEN) return ERR;
		strcpy(inName, inPtr);
		if (strcmp(inName, "stdin")) {
			if(convertToLongFileName(inName, MAX_FILE_LEN) == ERR) return ERR;
			if (libsnd) {
				MPGE_setConfigure(MC_INPUTFILE, MC_INPDEV_LIBSND, (UPARAM)inName);
			}
			else {
				MPGE_setConfigure(MC_INPUTFILE, MC_INPDEV_FILE, (UPARAM)inName);
			}
		} else {
			if (libsnd) {
				MPGE_setConfigure(MC_INPUTFILE, MC_INPDEV_LIBSND, (UPARAM)"-");
			}
			else {
				MPGE_setConfigure(MC_INPUTFILE, MC_INPDEV_STDIO, 0);
			}
		}

		if(!outPtr){
			if(changeSuffix(outName, inName, MAX_FILE_LEN) == ERR) return ERR;
		}else{
			if(strlen(outPtr) >= MAX_FILE_LEN) return ERR;
			strcpy(outName, outPtr);
		}
		if (strcmp(outName, "stdout")) {
			MPGE_setConfigure(MC_OUTPUTFILE, MC_OUTDEV_FILE, (UPARAM)outName);
		} else {
			MPGE_setConfigure(MC_OUTPUTFILE, MC_OUTDEV_STDOUT, 0);
		}
		if( !silent ){
			printf("encode %s to %s\n", inName, outName );
		}
	}

	return NOERR;
OPT_ERR:;
	if(!silent) fprintf(stderr, "opt err[%s]\n", *argv);
	return ERR;
} /* getOpt */

static void
usage(void)
{
	if(silent) return;
	fprintf(stderr, "USAGE\n");
	fprintf(stderr, "gogo [options] input.wav [output.mp3]\n");
	fprintf(stderr, "if input.wav is `stdin' then GOGO reads from stdin.\n");
	fprintf(stderr, "-b [kbps]\tbitrate\n");
	fprintf(stderr, "-m {s/m/j}\tencode mode(Stereo/Mono/Joint-stereo)\n");
	fprintf(stderr, "-v [0-9]\tVBR quality\n");
	fprintf(stderr, "-vb [min] [max]\tVBR bitrate range\n");
	fprintf(stderr, "-nopsy\t\thigh speed encoding\n"
					"\t\tWITH psycho-acoustics different from `gogo2 -nopsy'!!\n"
					"\t\tThe quality is near to `gogo2 WITHOUT -nopsy option'\n");
	fprintf(stderr, "-q [0-9]\tquality(0:high 9:fast)\n");
	fprintf(stderr, "-a\t\tABR\n");
	fprintf(stderr, "-silent\t\tdisable progress message\n");
	fprintf(stderr, "-s [KHz]\tinput sampling rate\n");
	fprintf(stderr, "-d [KHz]\toutput sampling rate\n");
	fprintf(stderr, "-test [sec]\tgogo bench:see http://homepage1.nifty.com/herumi/bench.html\n");
}

static void
opening(void)
{
	if(!silent) {
		char			verbuf[ MGV_BUFLENGTH ];
		unsigned long	vercode;
		if( MPGE_getVersion( &vercode, verbuf ) != ME_NOERR )
			verbuf[0] = '\0';
		fprintf(stderr,
			"GOGO-no-coda %s is a mp3 encoder based on lame 3.88,\n"
			"which is distributed under LGPL on http://www.mp3dev.org/mp3/ .\n"
			"See http://member.nifty.ne.jp/~pen/ ,\n"
			"    http://homepage1.nifty.com/herumi/gogo_e.html .\n", verbuf);
	}
}

static char *
getErrMsg(MERET code)
{
	switch(code){
	case ME_NOERR:
	case ME_EMPTYSTREAM:
		return "no error";
	case ME_PARAMERROR:
		return "parameter error";
	case ME_NOFPU:
		return "FPU is not found";
	case ME_INFILE_NOFOUND:
		return "can't open input file";
	case ME_OUTFILE_NOFOUND:
		return "can't open output file";
	case ME_FREQERROR:
		return "illegal input sampling frequency";
	case ME_BITRATEERROR:
	case ME_BITRATE_ERR:
		return "illegal output bitrate";
	case ME_WAVETYPE_ERR:
		return "illegal wave type";
	case ME_CANNOT_SEEK:
		return "can't seek";
	case ME_HALTED:
		return "halt";
	default:
		return "undefined error";
	}
} /* getErrMsg */

static void
putConfig(void)
{
	putCPUinfo();

	if(!silent) {
		int nParam;
		int para;
		char *strBuf;
		char szInName[ MAX_FILE_LEN ];
		char szOutName[ MAX_FILE_LEN ];

		fprintf( stderr, "enabled extended instruction - ");
#if defined(WIN32) || defined(__linux__) || defined(DOS) || defined(_M_IX86) || defined(__os2__) || defined(_X86_) || defined(__FreeBSD__)
		if( !MPGE_getConfigure( MG_USEMMX	, &nParam ) && nParam ) fprintf( stderr, "MMX ");
		if( !MPGE_getConfigure( MG_USEEMMX	, &nParam ) && nParam ) fprintf( stderr, "EMMX ");
		if( !MPGE_getConfigure( MG_USECMOV	, &nParam ) && nParam ) fprintf( stderr, "CMOV ");
		if( !MPGE_getConfigure( MG_USE3DNOW	, &nParam ) && nParam ) fprintf( stderr, "3D Now! ");
		if( !MPGE_getConfigure( MG_USEE3DNOW, &nParam ) && nParam ) fprintf( stderr, "Ext. 3D Now! ");
		if( !MPGE_getConfigure( MG_USESSE	, &nParam ) && nParam ) fprintf( stderr, "SSE ");
		if( !MPGE_getConfigure( MG_USESSE2	, &nParam ) && nParam ) fprintf( stderr, "SSE2 ");
		if( !MPGE_getConfigure( MG_CLFLUSH  , &nParam ) && nParam ) fprintf( stderr, "CLFLUSH ");
#elif defined(__MWERKS__) || defined(__ppc__)
		if( !MPGE_getConfigure( MG_USEALTIVEC	, &nParam ) && nParam ) fprintf( stderr, "ALTIVEC ");
#endif
		fprintf( stderr, "\n");

		if( !MPGE_getConfigure( MG_OUTFREQ, &nParam ) ){
			if (nParam < 32000) {
				fprintf(stderr, "MPEG 2 layer III ");
			} else {
				fprintf(stderr, "MPEG 1 layer III ");
			}
		}

		if( !MPGE_getConfigure( MG_ENCODEMODE, &nParam ) ){
			switch( nParam ){
				case MC_MODE_MONO:
					strBuf = "mono";
					break;
				case MC_MODE_STEREO:
					strBuf = "stereo";
					break;
				case MC_MODE_JOINT:
					strBuf = "j-stereo";
					break;
				case MC_MODE_MSSTEREO:
					strBuf = "m/s stereo";
					break;
				case MC_MODE_DUALCHANNEL:
					strBuf = "dual channel";
					break;
				default:
					strBuf = "undefined";
			}
			fprintf(stderr,"%s\n", strBuf );
		}

		if( !MPGE_getConfigure( MG_INPFREQ, &nParam ) ){
			fprintf(stderr, "%s=%.1fkHz ", "inp sampling-freq", nParam / 1000.0 );
		}
		if( !MPGE_getConfigure( MG_OUTFREQ, &nParam ) ){
			fprintf(stderr, "%s=%.1fkHz ", "out sampling-freq", nParam / 1000.0 );
		}
		if( !MPGE_getConfigure( MG_BITRATE, &nParam ) ){
			fprintf(stderr,"%s=%dkbps\n", "bitrate", nParam );
		}

		if( !MPGE_getConfigure( MG_INPUTFILE, szInName )  &&
			!MPGE_getConfigure( MG_OUTPUTFILE, szOutName ) ){
			fprintf(stderr, "%s `%s'\n%s `%s'\n", "input  file", szInName,
				"output file", szOutName);
		}
	}
} /* putConfig */

