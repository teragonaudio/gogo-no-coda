/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#include "../cpu.h"
#include "../gogo.h"
#include "../global.h"

int haveUNIT(void){
	static int unit = -1;
	int OSErr err;
	long atr;
	if(unit != -1) return unit;

	unit = 0;
	err = Gestalt(gestaltSysArchitecture, &atr);
	if(err != noErr) goto ERR_EXIT;
	switch(atr){
	default:
	case gestalt68k:
		goto ERR_EXIT;
	case gestaltPowerPC:
		unit |= MU_tPPC;
		break;
	}
	err = Gestalt(gestaltPowerPCProcessorFeatures, &atr);
	if(err != noErr) goto ERR_EXIT;

	if(atr & (1<<gestaltPowerPCHasGraphicsInstructions)){
		unit |= MU_tGRAP;
	}
	if(atr & (1<<gestaltPowerPCHasSquareRootInstructions)){
		unit |= MU_tFSQRT;
	}
	if(atr & (1<<gestaltPowerPCHasVectorInstructions)){
		unit |= MU_tALTIVEC;
	}
	return unit;
ERR_EXIT:
	RO.printf("can't get information of CPU\n");
	return 0;
} /* haveUNIT */

void initCPU(void){
} /* initCPU */

void
termCPU(void){
} /* termCPU */

void putCPUinfo(void){
	unsigned long unit;

	MPGE_getUnitStates(&unit);

	RO.printf("PowerPC\n");
	RO.printf("extended instruction - ");
	if(unit & MU_tGRAP) RO.printf("Graphics ");
	if(unit & MU_tFSQRT) RO.printf("SquareRoot ");
	if(unit & MU_tALTIVEC) RO.printf("AltiVec ");
	RO.printf("\n");
} /* putCPUinfo */


static unsigned int CLKsave;
static unsigned int CLKcount = 0;
static double CLKclock = 0;

static unsigned int getMFTB(void){
#ifdef	__MWERKS__
	asm{
		mftb		r3
	}
#else
	return 0;
#endif
}

void clkbegin(void){
	CLKsave = getMFTB();
} /* clkbegin */

void clkend(void){
	unsigned int clk;
	clk = getMFTB() - CLKsave;
	CLKclock += (double)clk;
	CLKcount++;
} /* clkend */

void clkput(void){
#ifdef	__MWERKS__
	const double scale = 1 / 24.8 * 450;/* assume PPC 450MHz */
#else
	const double scale = 1;
#endif
	if(!CLKcount) return;
	RO.printf("call %dtimes:ave %fclk\n", CLKcount, CLKclock * scale / CLKcount);
} /* clkput */

void setupUNIT(int unit){
	RO.printf("setup function\n");
} /* setupUNIT */
