/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#include "config.h"
#include "global.h"

#ifdef	USE_PTHREAD

static pthread_attr_t		attr_child;

int
gogo_initialize_thread_unit(void)
{
	pthread_attr_init(&attr_child);

	if (pthread_attr_setscope(&attr_child, PTHREAD_SCOPE_SYSTEM)) {
		return 1;
	}
	return pthread_attr_setdetachstate(&attr_child, PTHREAD_CREATE_JOINABLE);
}

int
gogo_create_thread(gogo_thread* pThread, gogo_thread_func func, void *data)
{
	return pthread_create(pThread, &attr_child, func, data);
}

#endif

