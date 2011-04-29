/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#ifndef TOOL_H_
#define TOOL_H_

#include <stdio.h>

int getWaveInfo(unsigned int *size, int *bit, int *freq, int *channel);
void setBinaryMode(FILE *fp);
//int convertToLongFileName(char *fname, const int maxLen);
int changeSuffix(char *dest, const char *src, const char *ext, const size_t maxLen);

/* for error */
void errPrintf(const char *msg, ...);
void bzeroAligned8(void *dest, int size);

#endif /* TOOL_H_ */
