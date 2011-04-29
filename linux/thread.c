/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#ifdef	USE_PTHREAD
/* POSIX thread を使います */
#include "../pthread/thread.c"

#include <stdio.h>  // for fopen, popen, fgets, sscanf 
#include <string.h>
#if defined(__FreeBSD__)
#	include <sys/sysctl.h> // for sysctl
#endif

int
gogo_get_cpu_count(int* pCPUs, int *pTHREADs)
{
	int		nCPU = 0;
	FILE	*cpuinfo;
	char	buffer[256];

	if(*pCPUs == 0){
		/* auto */
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
		*pCPUs = nCPU <= 1 ? 1 : nCPU;
		*pTHREADs = *pCPUs;
#if 0
// めもめも FreeBSDの場合
		int		nCPU = 0;
		int	sysname[] = {CTL_HW, HW_NCPU}, len, cpu;

		len = sizeof(int);
		if(sysctl(sysname, 2, &cpu, &len, NULL, 0) == 0) {
			if(len == sizeof(int)) nCPU = cpu;
		}
#endif
	}else{
		/* manual */
		*pTHREADs = *pCPUs;
	}

	return 0;
}
#endif

