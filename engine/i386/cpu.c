/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#include <setjmp.h>
#include <signal.h>

#if defined(USE_PTHREAD)
	#include <pthread.h>
	#include <semaphore.h>
	#if defined(__FreeBSD__)
		#include <sys/sysctl.h>
	#endif
#endif
#ifdef	WIN32
	#include <windows.h>
	#include <winbase.h>
	#include <process.h>
#endif
#ifdef USE_OS2THREAD
	#define INCL_DOS
	#include <os2.h>
#endif
#ifdef USE_BTHREAD
	#include <OS.h>
#endif

#if defined(__FreeBSD__)
	#include <floatingpoint.h>
#endif
#ifdef ABORTFP
	#include <fpu_control.h>
#endif

#include "common.h"
#include "cpu.h"
#include "gogo.h"
#include "global.h"

/*********************************************************************/

static jmp_buf jmpPtr;

static void
SSE_not_support(int sig)
{
	signal(SIGILL, SIG_DFL);
	longjmp(jmpPtr, 1);
} /* SSE_not_support */

extern int haveUNITa(void); /* defined in cpua.nas */
extern void setPIII_round(void);
#if	defined(__BORLANDC__) || defined(__os2__)
#define		NEED_ALIGNCHECK
#endif

#if	defined(NEED_ALIGNCHECK)
extern char	__checkalign_choosetable__[], __checkalign_fht__[];
extern char	__checkalign_quantizea__[], __checkalign_psymodel__[];
extern char __checkalign_fftsse__[];
#endif

int
haveUNIT(void)
{
	static int unit = -1;

	if(unit != -1) return unit;
	unit = haveUNITa();

#if 0
	if(getNumOfCPU() > 1) unit |= MU_tMULTI;	// gogo.cへ移動
#endif
#if	defined(NEED_ALIGNCHECK)
	#define		ISALIGN16( VAR )		((((int)&VAR) & 15) == 0)
	/* check variable-align */
	if(	   !ISALIGN16( RO )
		|| !ISALIGN16( RW )
		|| !ISALIGN16( __checkalign_choosetable__ )
		|| !ISALIGN16( __checkalign_fht__ )
		|| !ISALIGN16( __checkalign_quantizea__ )
		|| !ISALIGN16( __checkalign_psymodel__ )
		|| !ISALIGN16( __checkalign_fftsse__ )
	){
		fprintf(stderr, "** WARNING ** This compiler can't use SSE/SSE2!\n");
		unit &= ~(MU_tSSE|MU_tSSE2);
	}
	#undef		ISALIGN16
#endif
// NT+SP4以前 で SSE 対応の可否検出の際,
// signal と setjmp ではうまく検出できないことがあるので
// Win で __try/__except があるなら signal と setjmp より好ましい
#if defined(WIN32) && defined(HAVE___TRY) && defined(HAVE_EXCEPTION_EXECUTE_HANDLER) 
	if(unit & MU_tSSE){
		__try{
			setPIII_round();
		}
		__except(EXCEPTION_EXECUTE_HANDLER){
			unit &= ~(MU_tSSE|MU_tSSE2);
		}
	}
#elif defined(__unix__) || defined(WIN32) || defined(__os2__) || defined(__dos__)
	if(unit & MU_tSSE){
		MERET ret;
		ret = setjmp(jmpPtr);
		if(!ret){
			signal(SIGILL, SSE_not_support);
			setPIII_round();
			signal(SIGILL, SIG_DFL);
		}else{
			unit &= ~(MU_tSSE|MU_tSSE2);
		}
	}
#else
	if(unit & MU_tSSE) setPIII_round();
#endif

	return unit;
} /* haveUNIT */

#if 0
int
getNumOfCPU(void)
{

#if defined(__linux__)

	FILE *cpuinfo;
	char buffer[256];
	int nCPU = 0;
	if((cpuinfo = fopen("/proc/cpuinfo", "r")) != NULL){
		while(fgets(buffer, sizeof buffer, cpuinfo) != NULL){
			if(memcmp(buffer, "processor", (sizeof "processor") - 1) == 0){
				nCPU++;
			}
		}
		fclose(cpuinfo);
	}
	/* CPU number detection for Linux emulator on FreeBSD */
	else if((cpuinfo = popen("/sbin/sysctl hw.ncpu", "r")) != NULL){
		if(fgets(buffer, sizeof buffer, cpuinfo) != NULL){
			int	cpu;

			if(sscanf(buffer, "hw.ncpu: %d", &cpu) == 1){
				nCPU = cpu;
			}
		}
		fclose(cpuinfo);
	}
	return nCPU;

#elif defined(__FreeBSD__)

	int	sysname[] = {CTL_HW, HW_NCPU}, len, cpu;
	int nCPU = 1;
	len = sizeof(int);
	if(sysctl(sysname, 2, &cpu, &len, NULL, 0) == 0){
		if(len == sizeof(int)) nCPU = cpu;
	}
	return nCPU;

#elif defined(BeOS)

	system_info udtsystem_info;
	get_system_info(&udtsystem_info);
	return udtsystem_info.cpu_count;

#elif defined(__os2__)

	const int QSV_NUMPROCESSORS = 26;
	APIRET apiretRc = 0;
	int nCPU = 1;
	apiretRc = DosQuerySysInfo(QSV_NUMPROCESSORS, QSV_NUMPROCESSORS, &nCPU, sizeof(nCPU));
	if(apiretRc != 0) nCPU = 1;
	return nCPU;

#elif defined(WIN32)

	SYSTEM_INFO cpuinfo;
	GetSystemInfo(&cpuinfo);
	return cpuinfo.dwNumberOfProcessors;

#endif	/* USE_WINTHREAD */

	return 1;
} /* getNumOfCPU */
#endif

void
initCPU(void)
{
#ifdef GOGO_DLL_EXPORTS
	getFpuState(&RO.originalFpuState);
#else
	exchangeFpuState(&RO.originalFpuState, GOGO_FPU_STATE);
#endif //GOGO_DLL_EXPORTS
} /* initCPU */

void
termCPU(void)
{
#ifndef GOGO_DLL_EXPORTS
	setFpuState(RO.originalFpuState);
#endif //GOGO_DLL_EXPORTS
} /* termCPU */

/*********************************************************************/
