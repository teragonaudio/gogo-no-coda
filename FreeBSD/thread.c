/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#ifdef	USE_PTHREAD
#include "../pthread/thread.c"
#elif  USE_RFORK
#include "../rfork/thread.c"
#endif

#if defined(USE_PTHREAD) || defined(USE_RFORK)

#include <sys/sysctl.h> // for sysctl

int
gogo_get_cpu_count(int* pCPUs, int *pTHREADs)
{
	int		nCPU = 0;
	int	sysname[] = {CTL_HW, HW_NCPU}, len, cpu;

	if(*pCPUs == 0){
		len = sizeof(int);
		if(sysctl(sysname, 2, &cpu, &len, NULL, 0) == 0) {
			if(len == sizeof(int)) nCPU = cpu;
		}
		*pCPUs = nCPU <= 1 ? 1 : nCPU;
	}
	*pTHREADs = *pCPUs;

	return 0;
}
#endif

