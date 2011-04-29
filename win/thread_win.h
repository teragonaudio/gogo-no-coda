/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

/*
 *	これは engine\thread.h から #include されるファイルです。
 *	他のファイルから直接 #include されないことを想定しています。
 */

#include <windows.h>
#include "common.h"

typedef	struct {
	HANDLE           handle;
	DWORD			 id;
	gogo_thread_func func;
#if 0
	gogo_thread_data data;
#else
	void *data;
#endif
} gogo_thread;
typedef CRITICAL_SECTION gogo_mutex;
typedef HANDLE gogo_semaphore;

BEGIN_C_DECLS
#define	gogo_initialize_thread_unit()			(0)
#define	gogo_finalize_thread()					(0)
int gogo_create_thread(gogo_thread* pThread, gogo_thread_func func, void *data);
#define	gogo_join_thread(pThread)				(WaitForSingleObject((pThread)->handle, INFINITE) != WAIT_OBJECT_0)
#define	gogo_destroy_thread(pThread)			(!CloseHandle((pThread)->handle))
#define gogo_wait_terminatethread(pThread)		(WaitForSingleObject((pThread)->handle, INFINITE) != WAIT_ABANDONED)
#define	gogo_create_mutex(pMutex)				(InitializeCriticalSection(pMutex), 0)
#define	gogo_destroy_mutex(pMutex)				(DeleteCriticalSection(pMutex), 0)
#define	gogo_lock_mutex(pMutex)					(EnterCriticalSection(pMutex), 0)
#define	gogo_unlock_mutex(pMutex)				(LeaveCriticalSection(pMutex), 0)
#define	gogo_create_semaphore(pSemaphore)		((*(pSemaphore)=CreateSemaphore(NULL,1,1,NULL)) == NULL)
#define	gogo_destroy_semaphore(pSemaphore)		(CloseHandle(*(pSemaphore)) == 0)
#define	gogo_lock_semaphore(pSemaphore)			(WaitForSingleObject(*(pSemaphore), INFINITE) != WAIT_OBJECT_0)
#define	gogo_unlock_semaphore(pSemaphore)		(ReleaseSemaphore(*(pSemaphore),1,NULL) == 0)
#define	gogo_trylocktimeout_semaphore(pSemaphore, timeout) \
												(WaitForSingleObject(*(pSemaphore), timeout) != WAIT_OBJECT_0)
#define	gogo_yield_thread()						(Sleep(0))
int gogo_get_cpu_count(int *pCPUs, int *pTHREADs);
#define gogo_finalize_thread_unit()				(0)
END_C_DECLS

#if 0 
// プラットホーム依存でスレッドローカルデータ構造体に置きたい変数があれば、
// ここに書いて下さい
#define	GOGO_THREAD_VARIABLES                                           \
        short                   *buf;                                   \
        unsigned                threadaddr;                             \
        unsigned long           hThread;        /* Thread Handle */     \
        char                    firstthreadin;                          \
        char                    finthread;                              \
        volatile char           readytoencode;  /* Ready To Encode */   \
        CRITICAL_SECTION        section;                                \
        HANDLE                  hEnterFrame;                            \
        HANDLE                  hLeaveFrame;                            \
        HANDLE                  hWaitEncoding;                          \
        HANDLE                  hExistThread
#endif
