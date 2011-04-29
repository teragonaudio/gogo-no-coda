/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

int
gogo_get_cpu_count(int *pCPUs, int *pTHREADs)
{
        system_info udtsystem_info;

	if(*pCPUs == 0){
		/* auto */
        	get_system_info(&udtsystem_info);
	        *pCPUs = udtsystem_info.cpu_count;
        	*pTHREADs = (*pCPUs > 1)? *pCPUs: 1;
	}else{
		/* manual */
        	*pTHREADs = (*pCPUs > 1)? *pCPUs: 1;
	}

        return 0;
}

