/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */


#ifndef CPU_H_
#define CPU_H_

#include "common.h"
#include "gogo.h"

/* return avairable units */
/* Be care of compatibility of cpua.nas */
int haveUNIT(void);

/* initialize and finalize state  of FPU */
void initCPU(void);
void termCPU(void);

/* select routines according to RO.unit */
void setupUNIT(int unit);

#if 0
/* get the number of CPUs */
int getNumOfCPU(void);
#endif

void putCPUinfo(void);

void clkbegin(void);
void clkend(void);
void clkput(void);

#ifdef CPU_I386
void setFpuState(int);
void getFpuState(int*);
void exchangeFpuState(int* oldState, int newState);
#else
#define	setFpuState(a)	(0)
#define	getFpuState(a)	(0)
#define	exchangeFpuState(a, b)	(0)
#endif

#ifdef GOGO_DLL_EXPORTS
#define	saveFpuState(pOriginalFpuState)		exchangeFpuState((pOriginalFpuState), GOGO_FPU_STATE)
#define	restoreFpuState(originalFpuState)	setFpuState((originalFpuState))
#define defFpuStateBackupVar(originalFpuState)	int (originalFpuState)
#else
#define	saveFpuState(pOriginalFpuState)
#define	restoreFpuState(originalFpuState)
#define defFpuStateBackupVar(originalFpuState)
#endif //GOGO_DLL_EXPORTS

#define GOGO_FPU_STATE	(0x272)

#endif /* CPU_H_ */
