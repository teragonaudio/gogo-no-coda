;
;   part of this code is origined from
;
;       GOGO-no-coda
;       Copyright (c) 1999-2003 gogo-developer
;

;%include "nasm.cfg"
%include "global.cfg"

	externdef		pretab		;char*

%define			pow43		(RO.pow43)	;float*
%define			pow20		(RO.pow20)	;float*

	segment_data
	align	8
D_i0x007FFFFF	dd     ((1<<23)-1), ((1<<23)-1)
D_f1		dd     (127<<23)  , (127<<23)
D_0			dd		0, 0
D_ABS		dd		0x7FFFFFFF, 0x7FFFFFFF
D_f0p4054	dd		0.4054, 0.4054
D_i127		dd	127, 127
D_fSQRT2	dd	1.4142135623, 1.4142135623
D_fLOG_E_2	dd	0.6931471805, 0.6931471805
D_fA2		dd	2.0000006209, 2.0000006209
D_fB2		dd	0.6664778517, 0.6664778517
D_fC2		dd	0.4139745860, 0.4139745860
D_f0p5		dd	0.5, 0.5
D_fm0p25	dd	-0.25, -0.25
D_f1p25		dd	1.25, 1.25
D_fSQRT2_DIV_2	dd	0.7071067811, 0.7071067811

	segment_text

%define		SBLIMIT		(32)

;	2002/01/22	Init. version by kei

; プリフェッチ長の最適値がわからん．
; AMD の Athlon マニュアルから求めた理論値
%define		prefetch_len	(384) 
%define		subband_buf	(RW.subband_buf+(2*18*SBLIMIT-prefetch_len/4)*4)
proc	shiftoutpfb_E3DN; (void* dest, void* src);
%assign _P (4*0)
		mov			eax, [esp+_P+4]			;dest
		mov			edx, [esp+_P+8]			;src
		mov			ecx, 2*18*SBLIMIT-prefetch_len/4
		lea			eax, [eax+ecx*4]
		lea			edx, [edx+ecx*4]
		neg			ecx
		loopalignK7	16
.lp:
		prefetchw	[subband_buf+ecx*4+prefetch_len] ; こいつは他の CPU のキャッシュにいる
		prefetchw	[eax+ecx*4+prefetch_len]         ; こいつは他の CPU のキャッシュにはいないけど，自分のキャッシュにいるかどうか微妙
;		prefetch	[edx+ecx*4+prefetch_len]		 ; こいつも自分のキャッシュにいる
.lp.extra:
		movq		mm0, [subband_buf+ecx*4+ 0]
		movq		mm1, [subband_buf+ecx*4+ 8]
		movq		mm2, [subband_buf+ecx*4+16]
		movq		mm3, [subband_buf+ecx*4+24]
		movq		mm4, [edx+ecx*4+ 0]
		movq		mm5, [edx+ecx*4+ 8]
		movq		mm6, [edx+ecx*4+16]
		movq		mm7, [edx+ecx*4+24]
		movq		[eax+ecx*4+ 0], mm0
		movq		[eax+ecx*4+ 8], mm1
		movq		[eax+ecx*4+16], mm2
		movq		[eax+ecx*4+24], mm3
		movq		[subband_buf+ecx*4+ 0], mm4
		movq		[subband_buf+ecx*4+ 8], mm5
		movq		[subband_buf+ecx*4+16], mm6
		movq		[subband_buf+ecx*4+24], mm7
		movq		mm0, [subband_buf+ecx*4+32]
		movq		mm1, [subband_buf+ecx*4+40]
		movq		mm2, [subband_buf+ecx*4+48]
		movq		mm3, [subband_buf+ecx*4+56]
		movq		mm4, [edx+ecx*4+32]
		movq		mm5, [edx+ecx*4+40]
		movq		mm6, [edx+ecx*4+48]
		movq		mm7, [edx+ecx*4+56]
		movq		[eax+ecx*4+32], mm0
		movq		[eax+ecx*4+40], mm1
		movq		[eax+ecx*4+48], mm2
		movq		[eax+ecx*4+56], mm3
		movq		[subband_buf+ecx*4+32], mm4
		movq		[subband_buf+ecx*4+40], mm5
		movq		[subband_buf+ecx*4+48], mm6
		movq		[subband_buf+ecx*4+56], mm7
		add			ecx, byte 16
		js			near .lp
.lp.end:
		cmp			ecx, byte prefetch_len/4
		jl			near .lp.extra
.exit:
		femms
		ret


;	2002/01/20	Init. version 1020clk@K7-500 by kei
;	2002/02/05	960clk@K7-500 by kei
;	2002/02/05	950clk@K7-500 by kei
proc	ms_convert_3DN ;(FLOAT8* srcl, FLOAT8* srcr, int n);
%assign _P (4*0)
		mov			ecx, [esp+_P+12]		;n
		mov			eax, [esp+_P+4]			;srcl
		mov			edx, [esp+_P+8]			;srcr
		lea			eax, [eax+ecx*4]
		lea			edx, [edx+ecx*4]
		neg			ecx
		loopalignK7	16
.lp:
		movq		mm0, [eax+ecx*4+ 0]
		movq		mm1, [eax+ecx*4+ 8]
		movq		mm2, [eax+ecx*4+16]
		movq		mm3, [eax+ecx*4+24]
		movq		mm4, mm0
		movq		mm5, mm1
		movq		mm6, mm2
		movq		mm7, mm3
		pfadd		mm0, [edx+ecx*4+ 0]
		pfsub		mm4, [edx+ecx*4+ 0]
		pfadd		mm1, [edx+ecx*4+ 8]
		pfsub		mm5, [edx+ecx*4+ 8]
		pfmul		mm0, [D_fSQRT2_DIV_2]
		movq		[eax+ecx*4+ 0], mm0 
		movq		mm0, [D_fSQRT2_DIV_2]
		pfadd		mm2, [edx+ecx*4+16]
		pfsub		mm6, [edx+ecx*4+16]
		pfmul		mm4, mm0
		pfadd		mm3, [edx+ecx*4+24]
		pfsub		mm7, [edx+ecx*4+24]
		add			ecx, byte 8
		pfmul		mm1, mm0
		pfmul		mm5, mm0
		pfmul		mm2, mm0
		pfmul		mm6, mm0
		pfmul		mm3, mm0
		pfmul		mm7, mm0
		movq		[edx+ecx*4+ 0-32], mm4 
		movq		[eax+ecx*4+ 8-32], mm1 
		movq		[edx+ecx*4+ 8-32], mm5 
		movq		[eax+ecx*4+16-32], mm2 
		movq		[edx+ecx*4+16-32], mm6 
		movq		[eax+ecx*4+24-32], mm3 
		movq		[edx+ecx*4+24-32], mm7 
		jl			near .lp
.lp.end:
.exit:
		femms
		ret


;	kei 2001/07/18 Init. version 740clk@K7-500
;	2001/09/11	loopalign 追加 by kei
;	2002/01/13  RO.ixend 対応  by herumi 590clk@K7-500

proc	quantize_xrpow_ISO_3DN
%assign _P (4*0)
		; ixend は 8の倍数
		mov			edx, [esp+_P+12]		;istepPtr
		mov			eax, [esp+_P+4]			;xr
		mov			ecx, [RO.ixend]
		movd		mm7, [edx]				;istep
		movq		mm6, [D_f0p4054]
		mov			edx, [esp+_P+8]			;ix
		lea			eax, [eax + ecx*4]
		punpckldq	mm7, mm7
		lea			edx, [edx + ecx*4]
		neg			ecx
		loopalignK7	16
.lp:
		movq		mm0, [eax+ecx*4+ 0]
		movq		mm1, [eax+ecx*4+ 8]
		movq		mm2, [eax+ecx*4+16]
		movq		mm3, [eax+ecx*4+24]
		pfmul		mm0, mm7
		pfmul		mm1, mm7
		pfmul		mm2, mm7
		pfmul		mm3, mm7
		add			ecx, byte 8
		pfadd		mm0, mm6
		pfadd		mm1, mm6
		pfadd		mm2, mm6
		pfadd		mm3, mm6
		pf2id		mm0, mm0
		pf2id		mm1, mm1
		pf2id		mm2, mm2
		pf2id		mm3, mm3
		movq		[edx+ecx*4-8*4+ 0], mm0
		movq		[edx+ecx*4-8*4+ 8], mm1
		movq		[edx+ecx*4-8*4+16], mm2
		movq		[edx+ecx*4-8*4+24], mm3
		jnz			near .lp
.exit:
		femms
		ret

;	2002/01/21  Init. version うるりさん方式 5.25kclk@K7-500 by kei
;	2002/01/24  力技版 4.5kclk@K7-500 by kei
;	2002/01/25  4.4kclk@K7-500 by kei
;	2002/02/04  へるみさん方式 4.0kclk@K7-500 by kei

%define	xr				(BASE+_P+ 4)
%define xrpow			(BASE+_P+ 8)
%define psum			(BASE+_P+12)
%define pmax			(BASE+_P+16)
%define xr_sign			(BASE+_P+20)

%define LOCAL_SIZE (16+8)
%define BASE (esp+LOCAL_SIZE)

%define srpow_sum@      (ebp+ 0) ; size 8
%define xrpow_max@		(ebp+ 8) ; size 8

proc	pow075_E3DN_HI
		push		ebp
		push		ebx
		sub			esp, byte LOCAL_SIZE
%assign _P 4*2
		mov			eax, [xr]
		mov			edx, [xr_sign]
		lea			ebp, [esp+7]
		mov			ebx, [xrpow]
		mov			ecx, [RO.ixend]
		pxor		mm4, mm4				
		and			ebp, ~7
		lea			eax, [eax + ecx*4]
		lea			edx, [edx + ecx*4]
		lea			ebx, [ebx + ecx*4]
		neg			ecx
		movq		[srpow_sum@], mm4

		loopalignK7	16
.lp:
		movq		[xrpow_max@], mm4
		movq		mm0, [eax+ecx*4+ 0]
		movq		mm1, [eax+ecx*4+ 8]
		movq		mm2, [eax+ecx*4+16]
		movq		mm3, [eax+ecx*4+24]
		movq		mm4, mm0
		movq		mm5, mm1
		movq		mm6, mm2
		movq		mm7, mm3
		psrad		mm0, 31
		psrad		mm1, 31
		psrad		mm2, 31
		psrad		mm3, 31
		pand		mm4, [D_ABS]
		pand		mm5, [D_ABS]
		pand		mm6, [D_ABS]
		pand		mm7, [D_ABS]
		movq		[edx+ecx*4+ 0], mm0
		movq		mm0, [srpow_sum@]
		movq		[edx+ecx*4+ 8], mm1
		movq		[edx+ecx*4+16], mm2
		pfadd		mm0, mm4
		movq		[edx+ecx*4+24], mm3
		movq		[eax+ecx*4+ 0], mm4
		pfadd		mm0, mm5
		movq		[eax+ecx*4+ 8], mm5
		movq		[eax+ecx*4+16], mm6
		pfadd		mm0, mm6
		movq		[eax+ecx*4+24], mm7
		pswapd		mm1, mm4
		pfadd		mm0, mm7
		pswapd		mm2, mm5
		pswapd		mm3, mm6
		movq		[srpow_sum@], mm0
		pswapd		mm0, mm7
		pfrsqrt     mm4, mm4
        pfrsqrt     mm1, mm1
		pfrsqrt     mm5, mm5
        pfrsqrt     mm2, mm2
		pfrsqrt     mm6, mm6
        pfrsqrt     mm3, mm3
		pfrsqrt     mm7, mm7
        pfrsqrt     mm0, mm0
		pfrsqrt     mm4, mm4
        pfrsqrt     mm1, mm1
		pfrsqrt     mm5, mm5
        pfrsqrt     mm2, mm2
		pfrsqrt     mm6, mm6
        pfrsqrt     mm3, mm3
		pfrsqrt     mm7, mm7
        pfrsqrt     mm0, mm0
		pfrcp       mm4, mm4
        pfrcp       mm1, mm1
		pfrcp       mm5, mm5
        pfrcp       mm2, mm2
		pfrcp       mm6, mm6
        pfrcp       mm3, mm3
		pfrcp       mm7, mm7
        pfrcp       mm0, mm0
		punpckldq	mm4, mm1				; y = approx. x^-0.25
		punpckldq	mm5, mm2				; y = approx. x^-0.25
		punpckldq	mm6, mm3				; y = approx. x^-0.25
		punpckldq	mm7, mm0				; y = approx. x^-0.25
		movq		mm1, mm4
		movq		mm2, mm5
		movq		mm3, mm6
		movq		mm0, mm7
		pfmul		mm4, mm4
		pfmul		mm5, mm5
		pfmul		mm6, mm6
		pfmul		mm7, mm7
		pfmul		mm4, mm4
		pfmul		mm5, mm5
		pfmul		mm6, mm6
		pfmul		mm7, mm7
		pfmul		mm4, [eax+ecx*4+ 0]
		pfmul		mm1, [eax+ecx*4+ 0]		; yx
		pfmul		mm5, [eax+ecx*4+ 8]
		pfmul		mm2, [eax+ecx*4+ 8]		; yx
		pfmul		mm6, [eax+ecx*4+16]
		pfmul		mm3, [eax+ecx*4+16]		; yx
		pfmul		mm7, [eax+ecx*4+24]
		pfmul		mm0, [eax+ecx*4+24]		; yx
		pfmul		mm4, [D_fm0p25]			; - 1/4 * x (y^4)
		pfmul		mm5, [D_fm0p25]			; - 1/4 * x (y^4)
		pfmul		mm6, [D_fm0p25]			; - 1/4 * x (y^4)
		pfmul		mm7, [D_fm0p25]			; - 1/4 * x (y^4)
		pfadd		mm4, [D_f1p25]			; 5/4 - 1/4 * x (y^4)
		pfadd		mm5, [D_f1p25]			; 5/4 - 1/4 * x (y^4)
		pfadd		mm6, [D_f1p25]			; 5/4 - 1/4 * x (y^4)
		pfadd		mm7, [D_f1p25]			; 5/4 - 1/4 * x (y^4)
		pfmul		mm4, mm1
		movq		mm1, [D_0]
		pfmul		mm5, mm2
		pfmul		mm6, mm3
		pfmul		mm7, mm0
		pfmax		mm4, mm1
		pfmax		mm5, mm1
		pfmax		mm6, mm1
		pfmax		mm7, mm1
		movq		[ebx+ecx*4+ 0], mm4		; xrpow
		pfmax		mm4, [xrpow_max@]
		movq		[ebx+ecx*4+ 8], mm5		; xrpow
		pfmax		mm5, mm6
		movq		[ebx+ecx*4+16], mm6		; xrpow
		pfmax		mm4, mm7
		movq		[ebx+ecx*4+24], mm7		; xrpow
		add			ecx, byte 8
		pfmax		mm4, mm5
		jnz			near .lp
.lp.end:
		movq		mm2, [srpow_sum@] 
		pswapd		mm1, mm4				; 
		mov			eax, [psum]				; psum
		mov			edx, [pmax]				; pmax
		add			esp, byte LOCAL_SIZE
		pfacc		mm2, mm2
		pfmax		mm4, mm1
		pop			ebx
		pop			ebp
		movd		[eax], mm2
		movd		[edx], mm4
		femms
		ret

;	2001/08/20
;	Init. version by kei
;	pow075_3DN 12.1Kclk@K7-500
;	2001/09/11	loopalign 追加
;	2001/09/11	ちょっと整理して HI, FAST に分離 by kei
;				11.8Kclk@K7-500
;	2002/01/18  RO.ixend 対応  by kei
;				8.4Kclk@K7-500
;	2002/01/20  うるりさん方式に変更 by kei
;				6.6Kclk@K7-500
;	2002/02/09  へるみさん方式 4.2kclk@K7-500 by kei
;               8.2kclk@K6-2-400
;	2002/04/13  K6-2/K6-3 に特化 7.8kclk@K6-2-400 by kei

%define LOCAL_SIZE (0)

proc	pow075_3DN_HI
		push		ebx
%assign _P 4*1
		mov			eax, [xr]
		mov			edx, [xr_sign]
		mov			ebx, [xrpow]
		mov			ecx, [RO.ixend]
		pxor		mm6, mm6	; srpow_sum
		pxor		mm7, mm7	; xrpow_max	
		lea			eax, [eax + ecx*4]
		lea			edx, [edx + ecx*4]
		lea			ebx, [ebx + ecx*4]
		neg			ecx

		loopalign	16
.lp:
		movq		mm3, [D_ABS]
		movq		mm0, [eax+ecx*4+ 0]
		movq		mm1, [eax+ecx*4+ 8]
		movq		mm4, mm0
		movq		mm5, mm1
		psrad		mm0, 31
		psrad		mm1, 31
		pand		mm4, mm3
		pand		mm5, mm3
		movq		[edx+ecx*4+ 0], mm0
		movq		[edx+ecx*4+ 8], mm1
		movq		[eax+ecx*4+ 0], mm4
		movq		[eax+ecx*4+ 8], mm5
		pfadd		mm6, mm4
		movq		mm0, mm4
		movq		mm1, mm5
		pfadd		mm6, mm5
		punpckhdq	mm0, mm0
		punpckhdq	mm1, mm1

		pfrsqrt     mm4, mm4
        pfrsqrt     mm0, mm0
		pfrsqrt     mm5, mm5
        pfrsqrt     mm1, mm1
		pfrsqrt     mm4, mm4
        pfrsqrt     mm0, mm0
		pfrsqrt     mm5, mm5
        pfrsqrt     mm1, mm1
		pfrcp       mm4, mm4
        pfrcp       mm0, mm0
		pfrcp       mm5, mm5
        pfrcp       mm1, mm1
		punpckldq	mm4, mm0				; y = approx. x^-0.25
		punpckldq	mm5, mm1				; y = approx. x^-0.25
		movq		mm0, mm4
		movq		mm1, mm5
		movq		mm2, [D_fm0p25]
		movq		mm3, [D_f1p25]
		pfmul		mm4, mm4
		pfmul		mm5, mm5
		pfmul		mm4, mm4
		pfmul		mm5, mm5
		pfmul		mm4, [eax+ecx*4+ 0]
		pfmul		mm0, [eax+ecx*4+ 0]		; yx
		pfmul		mm5, [eax+ecx*4+ 8]
		pfmul		mm1, [eax+ecx*4+ 8]		; yx
		pfmul		mm4, mm2 				; - 1/4 * x (y^4)
		pfmul		mm5, mm2				; - 1/4 * x (y^4)
		pfadd		mm4, mm3				; 5/4 - 1/4 * x (y^4)
		pfadd		mm5, mm3				; 5/4 - 1/4 * x (y^4)
		pxor		mm2, mm2
		pfmul		mm4, mm0
		pfmul		mm5, mm1
		pfmax		mm4, mm2
		pfmax		mm5, mm2
		movq		[ebx+ecx*4+ 0], mm4		; xrpow
		pfmax		mm7, mm4
		movq		[ebx+ecx*4+ 8], mm5		; xrpow
		add			ecx, byte 4
		pfmax		mm7, mm5
		jnz			near .lp
.lp.end:
		movq		mm1, mm7				; 
		mov			eax, [psum]				; psum
		mov			edx, [pmax]				; pmax
		punpckhdq	mm1, mm1				; 
		pfacc		mm6, mm6
		pfmax		mm7, mm1
		pop			ebx
		movd		[eax], mm6
		movd		[edx], mm7
		femms
		ret

;	2002/02/09	pow075_3DN_FAST から分離 2.75kclk@K7-500 by kei
;	2002/02/10	2.60kclk@K7-500 by kei

%define LOCAL_SIZE (16+8)

proc	pow075_E3DN_FAST
		push		ebp
		push		ebx
		sub			esp, byte LOCAL_SIZE
%assign _P 4*2
		mov			eax, [xr]
		mov			edx, [xr_sign]
		lea			ebp, [esp+7]
		mov			ebx, [xrpow]
		mov			ecx, [RO.ixend]
		pxor		mm4, mm4				
		and			ebp, ~7
		lea			eax, [eax + ecx*4]
		lea			edx, [edx + ecx*4]
		lea			ebx, [ebx + ecx*4]
		neg			ecx
		movq		[srpow_sum@], mm4

		loopalignK7	16
.lp:
		movq		[xrpow_max@], mm4
		movq		mm0, [eax+ecx*4+ 0]
		movq		mm1, [eax+ecx*4+ 8]
		movq		mm2, [eax+ecx*4+16]
		movq		mm3, [eax+ecx*4+24]
		movq		mm4, mm0
		movq		mm5, mm1
		movq		mm6, mm2
		movq		mm7, mm3
		psrad		mm0, 31
		psrad		mm1, 31
		psrad		mm2, 31
		psrad		mm3, 31
		pand		mm4, [D_ABS]
		pand		mm5, [D_ABS]
		pand		mm6, [D_ABS]
		pand		mm7, [D_ABS]
		movq		[edx+ecx*4+ 0], mm0
		movq		mm0, [srpow_sum@]
		movq		[edx+ecx*4+ 8], mm1
		movq		[edx+ecx*4+16], mm2
		pfadd		mm0, mm4
		movq		[edx+ecx*4+24], mm3
		movq		[eax+ecx*4+ 0], mm4
		pfadd		mm0, mm5
		movq		[eax+ecx*4+ 8], mm5
		movq		[eax+ecx*4+16], mm6
		pfadd		mm0, mm6
		movq		[eax+ecx*4+24], mm7
		pswapd		mm1, mm4
		pfadd		mm0, mm7
		pswapd		mm2, mm5
		pswapd		mm3, mm6
		movq		[srpow_sum@], mm0
		pswapd		mm0, mm7
		pfrsqrt     mm4, mm4
        pfrsqrt     mm1, mm1
		pfrsqrt     mm5, mm5
        pfrsqrt     mm2, mm2
		pfrsqrt     mm6, mm6
        pfrsqrt     mm3, mm3
		pfrsqrt     mm7, mm7
        pfrsqrt     mm0, mm0
		pfrsqrt     mm4, mm4
        pfrsqrt     mm1, mm1
		pfrsqrt     mm5, mm5
        pfrsqrt     mm2, mm2
		pfrsqrt     mm6, mm6
        pfrsqrt     mm3, mm3
		pfrsqrt     mm7, mm7
        pfrsqrt     mm0, mm0
		pfrcp       mm4, mm4
        pfrcp       mm1, mm1
		pfrcp       mm5, mm5
        pfrcp       mm2, mm2
		pfrcp       mm6, mm6
        pfrcp       mm3, mm3
		pfrcp       mm7, mm7
        pfrcp       mm0, mm0
		punpckldq	mm4, mm1				; y = approx. x^-0.25
		punpckldq	mm5, mm2				; y = approx. x^-0.25
		punpckldq	mm6, mm3				; y = approx. x^-0.25
		punpckldq	mm7, mm0				; y = approx. x^-0.25
		pfmul		mm4, [eax+ecx*4+ 0]
		pfmul		mm5, [eax+ecx*4+ 8]
		pfmul		mm6, [eax+ecx*4+16]
		pfmul		mm7, [eax+ecx*4+24]
		movq		[ebx+ecx*4+ 0], mm4		; xrpow
		pfmax		mm4, [xrpow_max@]
		movq		[ebx+ecx*4+ 8], mm5		; xrpow
		pfmax		mm5, mm6
		movq		[ebx+ecx*4+16], mm6		; xrpow
		pfmax		mm4, mm7
		movq		[ebx+ecx*4+24], mm7		; xrpow
		add			ecx, byte 8
		pfmax		mm4, mm5
		jnz			near .lp
.lp.end:
		movq		mm2, [srpow_sum@] 
		pswapd		mm1, mm4				; 
		mov			eax, [psum]				; psum
		mov			edx, [pmax]				; pmax
		add			esp, byte LOCAL_SIZE
		pfacc		mm2, mm2
		pfmax		mm4, mm1
		pop			ebx
		pop			ebp
		movd		[eax], mm2
		movd		[edx], mm4
		femms
		ret


;	2001/08/20
;	Init. version by kei
;	pow075_3DN 12.1Kclk@K7-500
;	2001/09/11	loopalign 追加
;	2001/09/11	ちょっと整理して HI, FAST に分離 by kei
;				4.8Kclk@K7-500
;	2002/01/18  RO.ixend 対応  by kei
;				3.8Kclk@K7-500
;	2002/01/18  ちょっと改良 by kei
;				[RO.ixend] の後ろを読み書きすることもあるけど，
;				[576] 以降を読み書きすることはない
;				3.4Kclk@K7-500
;	2002/02/09	2.9kclk@K7-500 by kei
;				[RO.ixend] の後ろを読み書きすることはなくなった
;	2002/02/10	2.67kclk@K7-500 by kei

proc	pow075_3DN_FAST
		push		ebp
		push		ebx
		sub			esp, byte LOCAL_SIZE
%assign _P 4*2
		mov			eax, [xr]
		mov			edx, [xr_sign]
		lea			ebp, [esp+7]
		mov			ebx, [xrpow]
		mov			ecx, [RO.ixend]
		pxor		mm4, mm4				
		and			ebp, ~7
		lea			eax, [eax + ecx*4]
		lea			edx, [edx + ecx*4]
		lea			ebx, [ebx + ecx*4]
		neg			ecx
		movq		[srpow_sum@], mm4

		loopalignK7	16
.lp:
		movq		[xrpow_max@], mm4
		movq		mm0, [eax+ecx*4+ 0]
		movq		mm1, [eax+ecx*4+ 8]
		movq		mm2, [eax+ecx*4+16]
		movq		mm3, [eax+ecx*4+24]
		movq		mm4, mm0
		movq		mm5, mm1
		movq		mm6, mm2
		movq		mm7, mm3
		psrad		mm0, 31
		psrad		mm1, 31
		psrad		mm2, 31
		psrad		mm3, 31
		pand		mm4, [D_ABS]
		pand		mm5, [D_ABS]
		pand		mm6, [D_ABS]
		pand		mm7, [D_ABS]
		movq		[edx+ecx*4+ 0], mm0
		movq		mm0, [srpow_sum@]
		movq		[edx+ecx*4+ 8], mm1
		movq		[edx+ecx*4+16], mm2
		pfadd		mm0, mm4
		movq		[edx+ecx*4+24], mm3
		movq		[eax+ecx*4+ 0], mm4
		pfadd		mm0, mm5
		movq		[eax+ecx*4+ 8], mm5
		movq		[eax+ecx*4+16], mm6
		pfadd		mm0, mm6
		movq		[eax+ecx*4+24], mm7
		movq		mm1, mm4
		movq		mm2, mm5
		pfadd		mm0, mm7
		movq		mm3, mm6
		punpckhdq	mm1, mm1
		movq		[srpow_sum@], mm0
		movq		mm0, mm7
		punpckhdq	mm2, mm2
		punpckhdq	mm3, mm3
		punpckhdq	mm0, mm0
		pfrsqrt     mm4, mm4
        pfrsqrt     mm1, mm1
		pfrsqrt     mm5, mm5
        pfrsqrt     mm2, mm2
		pfrsqrt     mm6, mm6
        pfrsqrt     mm3, mm3
		pfrsqrt     mm7, mm7
        pfrsqrt     mm0, mm0
		pfrsqrt     mm4, mm4
        pfrsqrt     mm1, mm1
		pfrsqrt     mm5, mm5
        pfrsqrt     mm2, mm2
		pfrsqrt     mm6, mm6
        pfrsqrt     mm3, mm3
		pfrsqrt     mm7, mm7
        pfrsqrt     mm0, mm0
		pfrcp       mm4, mm4
        pfrcp       mm1, mm1
		pfrcp       mm5, mm5
        pfrcp       mm2, mm2
		pfrcp       mm6, mm6
        pfrcp       mm3, mm3
		pfrcp       mm7, mm7
        pfrcp       mm0, mm0
		punpckldq	mm4, mm1				; y = approx. x^-0.25
		punpckldq	mm5, mm2				; y = approx. x^-0.25
		punpckldq	mm6, mm3				; y = approx. x^-0.25
		punpckldq	mm7, mm0				; y = approx. x^-0.25
		pfmul		mm4, [eax+ecx*4+ 0]
		pfmul		mm5, [eax+ecx*4+ 8]
		pfmul		mm6, [eax+ecx*4+16]
		pfmul		mm7, [eax+ecx*4+24]
		movq		[ebx+ecx*4+ 0], mm4		; xrpow
		pfmax		mm4, [xrpow_max@]
		movq		[ebx+ecx*4+ 8], mm5		; xrpow
		pfmax		mm5, mm6
		movq		[ebx+ecx*4+16], mm6		; xrpow
		pfmax		mm4, mm7
		movq		[ebx+ecx*4+24], mm7		; xrpow
		add			ecx, byte 8
		pfmax		mm4, mm5
		jnz			near .lp
.lp.end:
		movq		mm1, mm4				; 
		movq		mm2, [srpow_sum@] 
		mov			eax, [psum]				; psum
		mov			edx, [pmax]				; pmax
		punpckhdq	mm1, mm1				; 
		add			esp, byte LOCAL_SIZE
		pfacc		mm2, mm2
		pfmax		mm4, mm1
		pop			ebx
		pop			ebp
		movd		[eax], mm2
		movd		[edx], mm4
		femms
		ret

;	init version by kei
;	2001/09/13	2550clk@K7-500
;	2001/09/20  2410clk@K7-500

%if III_psy_xmin_s.l 
	%error estimated : offset II_psy_xmin of l is 0
%endif 

%if III_scalefac.l
	%error estimated : offset III_scalefac_t of l is 0
%endif

%if III_psy_xmin_s.l
	%error estimated : offset III_psy_xmin of l is 0
%endif

%if calc_noise_result_t.tot_noise+4  <> calc_noise_result_t.over_noise
	%error estimated : calc_noise_result_t.tot_noise+4  == calc_noise_result_t.over_noise
%endif

%define max_index		(BASE+_P+ 4)
%define xr				(BASE+_P+ 8)
%define ix				(BASE+_P+12)
%define code_info		(BASE+_P+16)
%define l3_xmin_l		(BASE+_P+20)
%define	scalefac_l		(BASE+_P+24)
%define xfsf_l			(BASE+_P+28)
%define res				(BASE+_P+32)

%define LOCAL_SIZE (84)
%define BASE (esp+LOCAL_SIZE)

%define qtot.exponent@  (esp+ 0) ; size 8
%define qtot.mantissa@  (esp+ 8) ; size 8
%define qover.exponent@ (esp+16) ; size 8
%define qover.mantissa@ (esp+24) ; size 8
%define over_count@		(esp+32) ; size 8
%define preflag_mask@	(esp+40) ; size 8
%define scalefac_scale@	(esp+48) ; size 8
%define global_gain@	(esp+56) ; size 8
%define	max_index.sub.1@ (esp+64) ; size 4
%define	n0@				(esp+68) ; size 4
%define	n1@				(esp+72) ; size 4
%define	n0.div.4@		(esp+76) ; size 4
%define	n1.div.4@		(esp+80) ; size 4

proc	calc_noise_long_3DN
		push		ebx
		push		esi
		push		edi
		push		ebp
		sub			esp, byte LOCAL_SIZE
%assign _P 4*4
		mov			eax, [code_info]
		xor			edx, edx
		pxor		mm4, mm4
		movq		mm3, [D_f1]

		mov			ecx, [eax+gr_info_s.scalefac_scale]
		mov			edi, [eax+gr_info_s.global_gain]
		cmp			edx, [eax+gr_info_s.preflag]
		mov			esi, [max_index]
		movq		[qtot.exponent@], mm4
		mov			ebx, 1
		sbb			ebp, ebp
		add			ecx, byte 3
		movq		[qover.exponent@], mm4
		lea			edi, [pow20+edi*4]
		movd		mm7, ebp
		shl			ebx, cl
		movq		[over_count@], mm4
		movd		mm5, edi
		movd		mm6, ebx  
		dec			esi
		punpckldq	mm7, mm7
		punpckldq	mm5, mm5
		punpcklwd	mm6, mm6 
		mov			ecx, [ix]
		mov			eax, [xr]
		mov			[max_index.sub.1@], esi
		movq		[qtot.mantissa@], mm3
		movq		[qover.mantissa@], mm3
		movq		[preflag_mask@], mm7
		movq		[global_gain@], mm5
		movq		[scalefac_scale@], mm6

;		eax			xr
;		ecx			ix
;		edx			sbf

		loopalignK7	12
.lp.sfb:
		mov			esi, [scalefac_l]
		punpcklbw	mm2, [pretab+edx]	; pretab [3]:*:[2]:*:[1]:*:[0]:*
		packssdw	mm1, [esi+edx*4]	; scalefac_l [1]:[0]:*:*
		psrlw		mm2, 8				; pretab [3]:[2]:[1]:[0]
		punpckhdq	mm1, mm1			; scalefac_l *:*:[1]:[0]
		pand		mm2, [preflag_mask@]; 
		movq		mm0, [global_gain@]
		pxor		mm4, mm4			; sum
		paddw		mm1, mm2			; scalefac_l+pretab *:*:[1]:[0]
		movq		mm3, [RO.scalefac_band+scalefac_struct_s.l+edx*4+4]		
		pmullw		mm1, [scalefac_scale@]
		pxor		mm5, mm5			; sum
		psubd		mm3, [RO.scalefac_band+scalefac_struct_s.l+edx*4+0]		
		punpcklwd	mm1, [D_0]
		movq		[n0@], mm3
		psubd		mm0, mm1
		psrld		mm3, 2
		movd		ebp, mm0
		punpckhdq	mm0, mm0
		movd		mm6, [ebp]
		movd		ebx, mm0
		movq		[n0.div.4@], mm3
		punpckldq	mm6, mm6			; step [0]:[0]
		movd		mm7, [ebx]

		loopalignK7	16
.lp.n0.4:
		mov			ebp, [ecx+ 0]
		mov			edi, [ecx+ 8]
		mov			esi, [ecx+ 4]
		mov			ebx, [ecx+12]
		movd		mm0, [pow43+ebp*4]
		movd		mm1, [pow43+edi*4]
		punpckldq	mm0, [pow43+esi*4]
		punpckldq	mm1, [pow43+ebx*4]
		pfmul		mm0, mm6
		pfmul		mm1, mm6
		add			ecx, byte 16
		pfsubr		mm0, [eax+ 0] 
		pfsubr		mm1, [eax+ 8] 
		add			eax, byte 16
		dec			dword [n0.div.4@]
		pfmul		mm0, mm0
		pfmul		mm1, mm1
		pfadd		mm4, mm0
		pfadd		mm5, mm1
		jnz			.lp.n0.4
.lp.n0.4.end:
		test		[n0@], byte 3
		pfadd		mm4, mm5
		jz			.lp.n0.2.end
.lp.n0.2:
		mov			ebp, [ecx+ 0]
		mov			esi, [ecx+ 4]
		movd		mm0, [pow43+ebp*4]
		punpckldq	mm0, [pow43+esi*4]
		add			ecx, byte 8
		pfmul		mm0, mm6
		pfsubr		mm0, [eax+ 0] 
		add			eax, byte 8
		pfmul		mm0, mm0
		pfadd		mm4, mm0
.lp.n0.2.end:
		cmp			edx, [max_index.sub.1@]	
		punpckldq	mm7, mm7			; step [1]:[1]
		pxor		mm2, mm2			; sum for sfb+1
		pxor		mm3, mm3			; sum for sfb+1
		jz			near .lp.n1.2.end		; jmp if sfb == 20 && max_index == 21

		loopalignK7	12
.lp.n1.4:
		mov			ebp, [ecx+ 0]
		mov			edi, [ecx+ 8]
		mov			esi, [ecx+ 4]
		mov			ebx, [ecx+12]
		movd		mm0, [pow43+ebp*4]
		movd		mm1, [pow43+edi*4]
		punpckldq	mm0, [pow43+esi*4]
		punpckldq	mm1, [pow43+ebx*4]
		pfmul		mm0, mm7
		pfmul		mm1, mm7
		add			ecx, byte 16
		pfsubr		mm0, [eax+ 0] 
		pfsubr		mm1, [eax+ 8] 
		add			eax, byte 16
		dec			dword [n1.div.4@]
		pfmul		mm0, mm0
		pfmul		mm1, mm1
		pfadd		mm2, mm0
		pfadd		mm3, mm1
		jnz			.lp.n1.4
.lp.n1.4.end:
		test		[n1@], byte 3
		pfadd		mm2, mm3
		jz			.lp.n1.2.end
.lp.n1.2:
		mov			ebp, [ecx+ 0]
		mov			esi, [ecx+ 4]
		movd		mm0, [pow43+ebp*4]
		punpckldq	mm0, [pow43+esi*4]
		add			ecx, byte 8
		pfmul		mm0, mm7
		pfsubr		mm0, [eax+ 0] 
		add			eax, byte 8
		pfmul		mm0, mm0
		pfadd		mm2, mm0
.lp.n1.2.end:
		mov			ebp, [l3_xmin_l]
		mov			edi, [xfsf_l]
		movq		mm0, [ebp+edx*4]
		pfrcp		mm6, mm0
		pfrcp		mm7, [ebp+edx*4+4]
		pfacc		mm4, mm2			; sum [1]:[0]
		punpckldq	mm6, mm7
		add			edx, byte 2
		pfrcpit1	mm0, mm6
		movq		mm5, [D_f1]
		pfrcpit2	mm0, mm6			; 1.0/l3_xmin_l [1]:[0]
		cmp			edx, [max_index]
		pfmul		mm0, mm4			; noise [1]:[0]
		movq		[edi+edx*4- 8], mm0

		movq		mm7, mm0			; ftmp
		movq		mm6, mm0			; itmp
		pfcmpgt		mm0, mm5			; noise > 1.0 ? -1 : 0
		pand		mm7, [D_i0x007FFFFF]
		psrld		mm6, 23				; qnoise.exponent
		por			mm7, [D_f1]	; qnoise.mantissa

		movq		mm3, mm7
		pfmul		mm7, [qtot.mantissa@]
		movq		mm4, mm0
		movq		mm2, mm6
		paddd		mm6, [qtot.exponent@]
		pand		mm3, mm0
		pandn		mm4, mm5
		pand		mm2, mm0
		paddd		mm0, [over_count@]
		por			mm3, mm4
		paddd		mm2, [qover.exponent@]
		pfmul		mm3, [qover.mantissa@]
		movq		[qtot.mantissa@], mm7
		movq		[qtot.exponent@], mm6
		movq		[over_count@], mm0
		movq		[qover.exponent@], mm2
		movq		[qover.mantissa@], mm3

		jl			near .lp.sfb

		movq		mm5, mm7
		movq		mm4, mm6
		pxor		mm1, mm1
		punpckldq	mm7, mm3
		punpckldq	mm6, mm2
		punpckhdq	mm5, mm3
		mov			eax, [res]
		punpckhdq	mm4, mm2
		add			esp, byte LOCAL_SIZE
		pfmul		mm7, mm5	; qover.mantissa : qtot.mantissa
		paddd		mm6, mm4	; qover.exponent : qtot.exponent
		psubd		mm1, mm0
		movq		mm2, mm7
		pop			ebp
		psrld		mm7, 23
		movq		mm0, mm1
		pand		mm2, [D_i0x007FFFFF]
		paddd		mm6, mm7
		pop			edi
		punpckhdq	mm1, mm1
		psrld		mm2, 4
		pslld		mm6, 19
		pop			esi
		paddd		mm0, mm1
		por			mm6, mm2
		pop			ebx
		movd		[eax+calc_noise_result_t.over_count], mm0
		movq		[eax+calc_noise_result_t.tot_noise], mm6
		femms	
		ret

;	2002/03/06 initial ver. 13.2kclk@K7-500 by kei

%define	buf_L	(eax)
%define	buf_R	(eax+4*576*4)

proc	dist_mfbuf_mpeg1stereo_E3DN
%assign _P 0
		mov			eax, [esp+_P+4]		; eax = buf_L
		mov			edx, [RW.fr0]
		add			eax, 1152*4
		add			edx, 1152*4
		mov			ecx, -1152
		loopalignK7	16
.lp.1152:
		movq		mm0, [edx+ecx*4+ 0]		; R:L:R:L
		movq		mm1, [edx+ecx*4+ 8]		; R:L:R:L
		movq		mm2, [edx+ecx*4+16]		; R:L:R:L
		movq		mm3, [edx+ecx*4+24]		; R:L:R:L
		pi2fw		mm4, mm0			; L:L
		pi2fw		mm5, mm1			; L:L
		pi2fw		mm6, mm2			; L:L
		pi2fw		mm7, mm3			; L:L
		add			ecx, byte 8
		psrlq		mm0, 16				; *:R:*:R
		psrlq		mm1, 16				; *:R:*:R
		psrlq		mm2, 16				; *:R:*:R
		psrlq		mm3, 16				; *:R:*:R
		pi2fw		mm0, mm0			; R:R
		pi2fw		mm1, mm1			; R:R
		pi2fw		mm2, mm2			; R:R
		pi2fw		mm3, mm3			; R:R
		movq		[buf_L+ecx*4+ 0-32], mm4
		movq		[buf_L+ecx*4+ 8-32], mm5
		movq		[buf_L+ecx*4+16-32], mm6
		movq		[buf_L+ecx*4+24-32], mm7
		movq		[buf_R+ecx*4+ 0-32], mm0
		movq		[buf_R+ecx*4+ 8-32], mm1
		movq		[buf_R+ecx*4+16-32], mm2
		movq		[buf_R+ecx*4+24-32], mm3
		js			near .lp.1152
.lp.1152.end:
		mov			edx, [RW.fr1]
		add			eax, 752*4
		add			edx, 752*4
		mov			ecx, -752
		loopalignK7	16
.lp.752:
		movq		mm0, [edx+ecx*4+ 0]		; R:L:R:L
		movq		mm1, [edx+ecx*4+ 8]		; R:L:R:L
		movq		mm2, [edx+ecx*4+16]		; R:L:R:L
		movq		mm3, [edx+ecx*4+24]		; R:L:R:L
		pi2fw		mm4, mm0			; L:L
		pi2fw		mm5, mm1			; L:L
		pi2fw		mm6, mm2			; L:L
		pi2fw		mm7, mm3			; L:L
		add			ecx, byte 8
		psrlq		mm0, 16				; *:R:*:R
		psrlq		mm1, 16				; *:R:*:R
		psrlq		mm2, 16				; *:R:*:R
		psrlq		mm3, 16				; *:R:*:R
		pi2fw		mm0, mm0			; R:R
		pi2fw		mm1, mm1			; R:R
		pi2fw		mm2, mm2			; R:R
		pi2fw		mm3, mm3			; R:R
		movq		[buf_L+ecx*4+ 0-32], mm4
		movq		[buf_L+ecx*4+ 8-32], mm5
		movq		[buf_L+ecx*4+16-32], mm6
		movq		[buf_L+ecx*4+24-32], mm7
		movq		[buf_R+ecx*4+ 0-32], mm0
		movq		[buf_R+ecx*4+ 8-32], mm1
		movq		[buf_R+ecx*4+16-32], mm2
		movq		[buf_R+ecx*4+24-32], mm3
		js			near .lp.752
.lp.752.end:
.exit:
		femms
		ret

		end
