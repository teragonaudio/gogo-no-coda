/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2003 gogo-developer
 */

#include "config.h"
#include "global.h"

#ifdef	USE_RFORK

#ifdef USE_PIPE

int
gogo_create_mutex(gogo_mutex *pMutex)
{
	int	status;
	char	c;

	status = pipe(pMutex->mutex);
	if(status == 0){
		status = write(pMutex->mutex[1], &c, 1);
		if(status == 1) return 0;	/* unlock state */
		if(status == 0) return 1;
	}
	return status;
}

int
gogo_destroy_mutex(gogo_mutex *pMutex)
{
	close(pMutex->mutex[0]);
	close(pMutex->mutex[1]);
	return 0;
}

int
gogo_lock_mutex(gogo_mutex *pMutex)
{
	int	status;
	char	c;

	while((status = read(pMutex->mutex[0], &c, 1)) == EINTR);
	if(status == 1) return 0;	/* lock state */
	if(status == 0) return 1;
	return status;
}

int
gogo_unlock_mutex(gogo_mutex *pMutex)
{
	int	status;
	char	c;

	status = write(pMutex->mutex[1], &c, 1);
	if(status == 1) return 0;	/* unlock state */
	if(status == 0) return 1;
	return status;
}

int
gogo_create_semaphore(gogo_semaphore *pSemaphore)
{
	int	status;
	char	c;

	status = pipe(pSemaphore->semaphore);
	if(status == 0){
		status = write(pSemaphore->semaphore[1], &c, 1);
		if(status == 1) return 0;	/* unlock state */
		if(status == 0) return 1;
	}
	return status;
}

int
gogo_destroy_semaphore(gogo_semaphore *pSemaphore)
{
	close(pSemaphore->semaphore[0]);
	close(pSemaphore->semaphore[1]);
	return 0;
}

int
gogo_lock_semaphore(gogo_semaphore *pSemaphore)
{
	int	status;
	char	c;

	while((status = read(pSemaphore->semaphore[0], &c, 1)) == EINTR);
	if(status == 1) return 0;	/* lock state */
	if(status == 0) return 1;
	return status;
}

int
gogo_unlock_semaphore(gogo_semaphore *pSemaphore)
{
	int	status;
	char	c;

	status = write(pSemaphore->semaphore[1], &c, 1);
	if(status == 1) return 0;	/* unlock state */
	if(status == 0) return 1;
	return status;
}

int
gogo_trylocktimeout_semaphore(gogo_semaphore *pSemaphore, unsigned long timeout)
{
//	if(atomic_tryget(&pSemaphore->semaphore)){
		return 0;
//	}
//	usleep(timeout*1000);
//	if(atomic_tryget(&pSemaphore->semaphore)){
//		return 0;
//	}
//	return EAGAIN;
}
#else
int atomic_lock(int *mutex);
void atomic_post(int *semaphore);
int atomic_tryget(int *semaphore);

int
gogo_create_mutex(gogo_mutex *pMutex)
{
	pMutex->mutex = 0;	/* unlock state */
	return 0;
}

int
gogo_destroy_mutex(gogo_mutex *pMutex)
{
	return 0;
}

int
gogo_lock_mutex(gogo_mutex *pMutex)
{
	while(atomic_lock(&pMutex->mutex)){
		sched_yield();
	}
	return 0;
}

int
gogo_unlock_mutex(gogo_mutex *pMutex)
{
	pMutex->mutex = 0;	/* unlock state */
	return 0;
}

int
gogo_create_semaphore(gogo_semaphore *pSemaphore)
{
	pSemaphore->semaphore = 1;
	return 0;
}

int
gogo_destroy_semaphore(gogo_semaphore *pSemaphore)
{
	return 0;
}

int
gogo_lock_semaphore(gogo_semaphore *pSemaphore)
{
	while(atomic_tryget(&pSemaphore->semaphore) == 0){
		sched_yield();
	}
	return 0;
}

int
gogo_unlock_semaphore(gogo_semaphore *pSemaphore)
{
	atomic_post(&pSemaphore->semaphore);
	return 0;
}

int
gogo_trylocktimeout_semaphore(gogo_semaphore *pSemaphore, unsigned long timeout)
{
	if(atomic_tryget(&pSemaphore->semaphore)){
		return 0;
	}
	usleep(timeout*1000);
	if(atomic_tryget(&pSemaphore->semaphore)){
		return 0;
	}
	return EAGAIN;
}
#endif

int
gogo_create_thread(gogo_thread* pThread, gogo_thread_func func, void *data)
{
	pThread->sp = malloc(512*1024);	/* 512K */
	if(pThread->sp == NULL){
		return -1;
	}

	pThread->pid = rfork_thread(RFPROC|RFMEM, pThread->sp, func, data);
	if(pThread->pid == -1){
		free(pThread->sp);
		return -1;
	}
	return 0;
}

int
gogo_join_thread(gogo_thread* pThread)
{
	int	ret_val;
	int	status;

	ret_val = waitpid(pThread->pid, &status, 0);
	free(pThread->sp);
	if(ret_val == pThread->pid) return 0;
	return -1;
}

#endif

