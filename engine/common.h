/*
 *   part of this code is origined from
 *   GOGO-no-coda
 *
 *   Copyright(C) 2001,2002,2003 gogo-developer
 */


#ifndef COMMON_H_
#define COMMON_H_

#if defined(WIN32) || defined(__HIGHC__)
	#define DOS
#endif

#include <stdio.h>
#include <stdlib.h>
#ifdef __unix__
	#include <unistd.h>
#endif
#ifdef WIN32
	#include <windows.h>
#endif
#ifdef DOS
	#include <io.h>
#endif

//////////// 4並列fhtルーティンを使用する //////////////////
#define USE_VECTOR4_FHT
////////////////////////////////////////////////////////////

//////////// 午後2互換のfhtルーティンを使用する ////////////
#define USE_GOGO2_FHT
////////////////////////////////////////////////////////////

/* 全然一般的でない */
#if defined(WIN32) || defined(__linux__) || defined(DOS) || defined(_M_IX86) || defined(__os2__) || defined(_X86_) || defined(__FreeBSD__)
	#define CPU_I386
	#define MIE_LITTLE_ENDIAN
#elif defined(__MWERKS__) || defined(__ppc__)
	#define CPU_PPC		/* but can't run... */
	#define MIE_BIG_ENDIAN
#	undef	USE_VECTOR4_FHT
#	undef	USE_GOGO2_FHT
#else
	#error "Define CPU_TYPE"
#endif

#define SWAP16BIT(x)	( (((x)<<8)&0xFF00) | (((x)>>8)&0x00FF) )
#define SWAP32BIT(x)	( ((x)<<24) | (((x)<<8)&0x00FF0000) | (((x)>>8)&0x0000FF00) | (((x)>>24)&0xFF) )

#if !defined(MIE_LITTLE_ENDIAN) && !defined(MIE_BIG_ENDIAN)
	#error "define endian type"
#endif

#ifdef MIE_LITTLE_ENDIAN
	#define LE2uint16(x)	(x)
	#define LE2uint32(x)	(x)
	#define BE2uint16(x)	SWAP16BIT(x)
	#define BE2uint32(x)	SWAP32BIT(x)
#else
	#define BE2uint16(x)	(x)
	#define BE2uint32(x)	(x)
	#define LE2uint16(x)	SWAP16BIT(x)
	#define LE2uint32(x)	SWAP32BIT(x)
#endif

/*********   global definition   *********/

#if defined(__GNUC__) || defined(WIN32)
	#define INLINE __inline
#else
	#define INLINE
#endif

#ifdef WIN32
	#define MAX_FILE_LEN 260
#elif BeOS
	#define MAX_FILE_LEN 1024
#else
	#define MAX_FILE_LEN 256
#endif

#ifndef NUL
	#define NUL '\0'
#endif

#ifndef NOERR
	#define NOERR 0
	#define ERR 1
#endif

#ifndef TRUE
	#define TRUE 1
	#define FALSE 0
#endif

#if !defined(__cplusplus) && !defined(Min)
	#define Min(A, B) ((A) < (B) ? (A) : (B))
	#define Max(A, B) ((A) > (B) ? (A) : (B))
#endif

#ifndef PI
	#define PI 3.14159265358979323846
#endif

#ifndef SQRT2
	#define SQRT2 1.4142135623730950488
#endif

#define LOG_E_2  0.693147180559945309	/* log(2) */

#ifdef	_MSC_VER
	#pragma warning( disable : 4305 )	/* convert const double->float */
	#pragma warning( disable : 4244 )	/* convert double->float */
//	#pragma warning( disable : 4761 )	/* conflict prototype */
	#pragma warning( disable : 4018 )	/* compare signed with unsigned */
#endif

#if defined(__cplusplus)
	#define	BEGIN_C_DECLS	extern "C" {
	#define	END_C_DECLS		}
#else
	#define	BEGIN_C_DECLS	
	#define	END_C_DECLS		
#endif

typedef long double ieee854_float80_t;
typedef double      ieee754_float64_t;
typedef float       ieee754_float32_t;

#endif /* COMMON_H_ */
