/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#include "config.h"
#include "global.h"

#ifdef	USE_WINTHREAD

#include "../engine/machine.h"
#include "../engine/encoder.h"
#include "../engine/thread.h"

#include <windows.h>
#include <process.h>  // for _beginthreadex

static 
unsigned WINAPI
inner_thread_func(LPVOID data)
{
	return (DWORD)(((gogo_thread*)data)->func)(((gogo_thread*)data)->data);
}

int
gogo_create_thread(gogo_thread* pThread, gogo_thread_func func, void *data)
{
	pThread->func = func;
	pThread->data = data;
	pThread->handle = (HANDLE)_beginthreadex( 
		NULL,			// security
		0,				// stksize 
		inner_thread_func,
		(void *)pThread,
		0, 
		(unsigned *)&(pThread->id)
	);
	return pThread->handle == 0;
}

int
gogo_get_cpu_count(int *pCPUs, int *pTHREADs)
{
	if(*pCPUs == 0){
		/* auto */
		SYSTEM_INFO		cpuinfo;
		GetSystemInfo( &cpuinfo );
		*pCPUs = cpuinfo.dwNumberOfProcessors;
		*pTHREADs = (*pCPUs > 1)? *pCPUs: 1;
	}else{
		/* manual */
		*pTHREADs = (*pCPUs > 1)? *pCPUs: 1;
	}
	return 0;
}
#endif
