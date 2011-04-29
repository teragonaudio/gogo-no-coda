/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

/*
 *	This file can be included by only engine/thread.h.
 */

#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#include <os2.h>

#ifndef QSV_NUMPROCESSORS
#define QSV_NUMPROCESSORS         26
#endif

#include <sys/fmutex.h>
#include <sys/builtin.h>

typedef int       gogo_thread;
typedef HMTX      gogo_mutex;
typedef _fmutex   gogo_semaphore;

BEGIN_C_DECLS
#define gogo_initialize_thread_unit() (0)
#define gogo_create_thread(pThread,pFunc,pData) (-1==(*(pThread)=_beginthread(pFunc,NULL,65536,pData)))
//#define gogo_create_thread(pThread,pFunc,pData) (DosCreateThread((PID*)(pThread),pFunc,(ULONG)(pData),0,65536))

#define gogo_join_thread(pThread)               (DosWaitThread((PID*)(pThread),DCWW_WAIT))
#define gogo_destroy_thread(pThread)            (DosKillThread(*(pThread)),0)
#define gogo_wait_terminatethread(pThread)	(DosWaitThread((PID*)(pThread),DCWW_WAIT))

#define gogo_create_mutex(pMutex)               (DosCreateMutexSem(NULL, (pMutex), 0, 0))
#define gogo_destroy_mutex(pMutex)              (DosCloseMutexSem(*(pMutex)))
#define gogo_lock_mutex(pMutex)                 (DosRequestMutexSem(*(pMutex), -1))
#define gogo_unlock_mutex(pMutex)               (DosReleaseMutexSem(*(pMutex)))

#define gogo_create_semaphore(pSemaphore)       (_fmutex_create(pSemaphore,0))
#define gogo_destroy_semaphore(pSemaphore)      (_fmutex_close(pSemaphore))
#define gogo_lock_semaphore(pSemaphore)         (_fmutex_request(pSemaphore,0))
#define gogo_unlock_semaphore(pSemaphore)       (_fmutex_release(pSemaphore))
#define	gogo_trylocktimeout_semaphore(pSemaphore, timeout) \
    ( _fmutex_request(pSemaphore,_FMR_NOWAIT) ? (DosSleep(timeout),_fmutex_request(pSemaphore,0)) : 0 )

#define gogo_yield_thread()                     (DosSleep(0))
#define gogo_get_cpu_count(pCPUs,pTHREADs) \
    (*(pTHREADs)=(*(pCPUs)>0 ? *(pCPUs) : (DosQuerySysInfo(QSV_NUMPROCESSORS, QSV_NUMPROCESSORS, pCPUs, sizeof(int)) ? 1 : *(pCPUs))),0)

#define gogo_finalize_thread_unit()             (0)
END_C_DECLS
