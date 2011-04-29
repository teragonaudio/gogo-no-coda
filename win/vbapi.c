/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */

#ifdef	_WIN32
#include <stdio.h>
#include "gogo.h"

/* FUNCTION EXPORT FOR VISUAL BASIC */
typedef	MERET (__stdcall *MPGE_CALLBACK_VB)( void *, unsigned long );

static  MPGE_CALLBACK_VB	g_src_cbfunc;
static  MPGE_CALLBACK_VB	g_dst_cbfunc;

static  MERET __cdecl mpge_src_cbfunc( void *buf, unsigned long nLength )
{
	if( g_src_cbfunc )
		return g_src_cbfunc( buf, nLength );
	return ME_INTERNALERROR;
}

static  MERET __cdecl mpge_dst_cbfunc( void *buf, unsigned long nLength )
{
	if( g_dst_cbfunc )
		return g_dst_cbfunc( buf, nLength );
	return ME_INTERNALERROR;
}


MERET	EXPORT_VB	MPGE_initializeWorkVB()
{
	g_src_cbfunc = NULL;
	g_dst_cbfunc = NULL;
	return MPGE_initializeWork();
}

MERET	EXPORT_VB	MPGE_setConfigureVB(UPARAM mode, UPARAM dwPara1, UPARAM dwPara2 )
{
	if( mode == MC_INPUTFILE ){
		// コールバック登録の場合
		if( dwPara1 == MC_INPDEV_USERFUNC ){
			struct MCP_INPDEV_USERFUNC func =  *((struct MCP_INPDEV_USERFUNC *)dwPara2);
			g_src_cbfunc = (MPGE_CALLBACK_VB)func.pUserFunc;
			func.pUserFunc = mpge_src_cbfunc;
			return MPGE_setConfigure( mode, dwPara1, (UPARAM)&func );
		}
	} else 
	if( mode == MC_OUTPUTFILE){
		// コールバック登録の場合
		if( dwPara1 == MC_OUTDEV_USERFUNC ||
			dwPara1 == MC_OUTDEV_USERFUNC_WITHVBRTAG ){
			g_dst_cbfunc = (MPGE_CALLBACK_VB)dwPara2;
			dwPara2 = (UPARAM)&mpge_dst_cbfunc;
			return	MPGE_setConfigure( mode, dwPara1, dwPara2 );
		}
	}
	return	MPGE_setConfigure( mode, dwPara1, dwPara2 );
}

MERET	EXPORT_VB	MPGE_getConfigureVB(UPARAM mode, void *para1 )
{
	return	MPGE_getConfigure( mode, para1 );
}

MERET	EXPORT_VB	MPGE_detectConfigureVB()
{
	return	MPGE_detectConfigure();
}

MERET	EXPORT_VB	MPGE_processFrameVB()
{
	return	MPGE_processFrame();
}

MERET	EXPORT_VB	MPGE_closeCoderVB()
{
	return	MPGE_closeCoder();
}

MERET	EXPORT_VB	MPGE_endCoderVB()
{
	return	MPGE_endCoder();
}

MERET	EXPORT_VB	MPGE_getUnitStatesVB( unsigned long *unit )
{
	return	MPGE_getUnitStates(  unit );
}

//MERET	EXPORT_VB	MPGE_processTrackVB(int *frameNum)
//{
//	return	MPGE_processTrack( frameNum );
//}

MERET	EXPORT_VB	MPGE_getVersionVB( unsigned long *vercode,  char *verstring )
{
	return	MPGE_getVersion( vercode,  verstring );
}


#endif // _WIN32
