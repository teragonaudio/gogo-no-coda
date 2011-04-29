/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2003 gogo-developer
 */

/*
 *	This file can be included by only engine/thread.h.
 */

#include <unistd.h>	/* rfork_thread() */
#include <semaphore.h>	/* semaphore */
#include <sys/types.h>	/* wait4() */
#include <sys/resource.h>	/* wait4() */
#include <sys/wait.h>	/* wait4() */

#include "common.h"

typedef	struct gogo_thread_s {
	void	*sp;
	int	pid;
} gogo_thread;

#ifdef USE_PIPE
typedef struct gogo_mutex_s {
	int	mutex[2];
} gogo_mutex;

typedef struct gogo_semaphore_s {
	int	semaphore[2];
} gogo_semaphore;
#else
typedef struct gogo_mutex_s {
	int	mutex;
} gogo_mutex;

typedef struct gogo_semaphore_s {
	int	semaphore;
} gogo_semaphore;
#endif

BEGIN_C_DECLS
#define gogo_initialize_thread_unit() (0)
int gogo_create_thread(gogo_thread* pThread, gogo_thread_func func, void *data);
int gogo_join_thread(gogo_thread* pThread);
#define	gogo_destroy_thread(pThread)		(0)			
#define gogo_finalize_thread_unit()		(0)
#define	gogo_yield_thread()				(sched_yield())

int gogo_create_mutex(gogo_mutex *pMutex);
int gogo_destroy_mutex(gogo_mutex *pMutex);
int gogo_lock_mutex(gogo_mutex *pMutex);
int gogo_unlock_mutex(gogo_mutex *pMutex);

int gogo_create_semaphore(gogo_semaphore *pSemaphore);
int gogo_destroy_semaphore(gogo_semaphore *pSemaphore);
int gogo_lock_semaphore(gogo_semaphore *pSemaphore);
int gogo_unlock_semaphore(gogo_semaphore *pSemaphore);

int gogo_trylocktimeout_semaphore(gogo_semaphore *pSemaphore, unsigned long timeout);

int gogo_get_cpu_count(int *pCPUs, int *nTHERADs);
END_C_DECLS

