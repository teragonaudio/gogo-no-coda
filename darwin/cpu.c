/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2002,2003 gogo-developer
 */

#include <stdio.h>
#include <string.h>
#include <sys/sysctl.h>

#include "../engine/cpu.h"
#include "../engine/gogo.h"
#include "../engine/global.h"

int
haveUNIT(void)
{
	static int unit = -1;
	int	vectorunit[] = {CTL_HW, HW_VECTORUNIT};
	int	len, available;

	if(unit != -1) return unit;

	unit |= MU_tPPC;
	len = sizeof(int);
	if(sysctl(vectorunit, 2, &available, &len, NULL, 0) == 0) {
		if(len == sizeof(int) && available) unit |= MU_tALTIVEC;
	}
	return unit;
} /* haveUNIT */

void
initCPU(void)
{
} /* initCPU */

void
termCPU(void){
} /* termCPU */

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

