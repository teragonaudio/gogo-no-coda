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

	globaldef	__checkalign_quantizea__

	segment_data
	align	32
__checkalign_quantizea__:
Q_f1		dd		1.0, 1.0, 1.0, 1.0
Q_1shl23m1	dd		(1<<23)-1, (1<<23)-1, (1<<23)-1, (1<<23)-1
Q_127shl23	dd		127<<23, 127<<23, 127<<23, 127<<23
Q_ABS		dd		0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF
Q_i31		dd	31, 31, 31, 31
Q_f1p25		dd	1.25, 1.25, 1.25, 1.25
Q_fm0p25	dd	-0.25, -0.25, -0.25, -0.25
Q_fSQRT2_DIV_2	dd	0.7071067811, 0.7071067811, 0.7071067811, 0.7071067811

f0p4054		dd		0.4054

	segment_text

%define		SBLIMIT		(32)

; プリフェッチ長の最適値がわからん．
; 2002/02/27 init ver. 8.5Kclk@Pen4-1.7G-dual(C版 15Kclk) by kei
; 2003/07/03 メモリアクセス順序の変更。by sakai
%define		prefetch_len	(384) 
%define		subband_buf	(RW.subband_buf+(2*18*SBLIMIT-prefetch_len/4)*4)
proc	shiftoutpfb_SSE;
%assign _P (4*0)
		mov			eax, [esp+_P+4]			;dest
		mov			edx, [esp+_P+8]			;src
		mov			ecx, 2*18*SBLIMIT-prefetch_len/4
		lea			eax, [eax+ecx*4]
		lea			edx, [edx+ecx*4]
		neg			ecx
		prefetchnta	[RW.subband_buf+prefetch_len]
		prefetchnta	[RW.subband_buf+prefetch_len+64]
		movaps		xmm0, [RW.subband_buf+ 0]
		movaps		xmm1, [RW.subband_buf+16]
		movaps		xmm2, [RW.subband_buf+32]
		movaps		xmm3, [RW.subband_buf+48]
		jmp			short .f0

		align	16
.lp:
		prefetchnta	[subband_buf+ecx*4+prefetch_len] ; こいつは他の CPU のキャッシュにいる
		prefetchnta	[subband_buf+ecx*4+prefetch_len+64] ; 入れすぎてもアカンし。
.lp.extra:
		movaps		xmm0, [subband_buf+ecx*4+ 0]
		movaps		xmm1, [subband_buf+ecx*4+16]
		movaps		xmm2, [subband_buf+ecx*4+32]
		movaps		xmm3, [subband_buf+ecx*4+48]
		movaps		[subband_buf+ecx*4+ 0+64-128], xmm4
		movaps		[subband_buf+ecx*4+16+64-128], xmm5
		movaps		[subband_buf+ecx*4+32+64-128], xmm6
		movaps		[subband_buf+ecx*4+48+64-128], xmm7
.f0:
		movaps		xmm4, [edx+ecx*4+ 0]
		movaps		xmm5, [edx+ecx*4+16]
		movaps		xmm6, [edx+ecx*4+32]
		movaps		xmm7, [edx+ecx*4+48]
		movaps		[eax+ecx*4+ 0], xmm0
		movaps		[eax+ecx*4+16], xmm1
		movaps		[eax+ecx*4+32], xmm2
		movaps		[eax+ecx*4+48], xmm3
		movaps		xmm0, [subband_buf+ecx*4+ 0+64]
		movaps		xmm1, [subband_buf+ecx*4+16+64]
		movaps		xmm2, [subband_buf+ecx*4+32+64]
		movaps		xmm3, [subband_buf+ecx*4+48+64]
		movaps		[subband_buf+ecx*4+ 0], xmm4
		movaps		[subband_buf+ecx*4+16], xmm5
		movaps		[subband_buf+ecx*4+32], xmm6
		movaps		[subband_buf+ecx*4+48], xmm7
		movaps		xmm4, [edx+ecx*4+ 0+64]
		movaps		xmm5, [edx+ecx*4+16+64]
		movaps		xmm6, [edx+ecx*4+32+64]
		movaps		xmm7, [edx+ecx*4+48+64]
		movaps		[eax+ecx*4+ 0+64], xmm0
		movaps		[eax+ecx*4+16+64], xmm1
		movaps		[eax+ecx*4+32+64], xmm2
		movaps		[eax+ecx*4+48+64], xmm3
		add			ecx, byte 32
		js			near .lp
.lp.end:
		cmp			ecx, byte prefetch_len/4
		jl			near .lp.extra
.exit:
		movaps		[subband_buf+ecx*4+ 0+64-128], xmm4
		movaps		[subband_buf+ecx*4+16+64-128], xmm5
		movaps		[subband_buf+ecx*4+32+64-128], xmm6
		movaps		[subband_buf+ecx*4+48+64-128], xmm7
		ret

;void quantize_xrpow_ISO( float *xr, int *ix, float *istepPtr );

;	shigeo
;	2001/07/20 Init. version
;	1360clk@PIII-700 何故にAthlonの倍?(^^;

; RO.ixendは8の倍数であること
proc	quantize_xrpow_ISO_SSE
%assign _P (4*0)
		mov			eax, [esp+_P+12]
		movss		xm0, [eax]				;istep
		mov			eax, [esp+_P+4]			;xr
		mov			edx, [esp+_P+8]			;ix
		movss		xm1, [f0p4054]
		mov			ecx, [RO.ixend]
		shufps		xm0, xm0, 0				;xm0 = [istep]
		shufps		xm1, xm1, 0				;xm1 = [0.4054]

		test		ecx, 15
		jz			short .lp
		movaps		xm2, [eax+4* 0]
		movaps		xm3, [eax+4* 4]
		add			eax, byte 32
		mulps		xm2, xm0
		mulps		xm3, xm0
		addps		xm2, xm1
		addps		xm3, xm1
		cvttps2pi	mm0, xm2
		movhlps		xm2, xm2
		cvttps2pi	mm2, xm3
		movhlps		xm3, xm3
		cvttps2pi	mm1, xm2
		cvttps2pi	mm3, xm3
		sub			ecx, byte 8
		movq		[edx+4* 0], mm0
		movq		[edx+4* 2], mm1
		movq		[edx+4* 4], mm2
		movq		[edx+4* 6], mm3
		lea			edx, [edx + 32]
.lp:
		movaps		xm2, [eax+4* 0]
		movaps		xm3, [eax+4* 4]
		movaps		xm4, [eax+4* 8]
		movaps		xm5, [eax+4*12]
		add			eax, byte 64
		mulps		xm2, xm0
		mulps		xm3, xm0
		mulps		xm4, xm0
		mulps		xm5, xm0
		addps		xm2, xm1
		addps		xm3, xm1
		addps		xm4, xm1
		addps		xm5, xm1
		cvttps2pi	mm0, xm2
		movhlps		xm2, xm2
		cvttps2pi	mm2, xm3
		movhlps		xm3, xm3
		cvttps2pi	mm4, xm4
		movhlps		xm4, xm4
		cvttps2pi	mm6, xm5
		movhlps		xm5, xm5
		cvttps2pi	mm1, xm2
		cvttps2pi	mm3, xm3
		cvttps2pi	mm5, xm4
		cvttps2pi	mm7, xm5
		sub			ecx, byte 16
		movq		[edx+4* 0], mm0
		movq		[edx+4* 2], mm1
		movq		[edx+4* 4], mm2
		movq		[edx+4* 6], mm3
		movq		[edx+4* 8], mm4
		movq		[edx+4*10], mm5
		movq		[edx+4*12], mm6
		movq		[edx+4*14], mm7
		lea			edx, [edx + 64]
		jnz			.lp
.exit:
		emms
		ret

proc	quantize_xrpow_ISO_SSE2
%assign _P (4*0)
		mov			eax, [esp+_P+12]
		movss		xm0, [eax]				;istep
		mov			eax, [esp+_P+4]			;xr
		mov			edx, [esp+_P+8]			;ix
		movss		xm1, [f0p4054]
		mov			ecx, [RO.ixend]
		shufps		xm0, xm0, 0				;xm0 = [istep]
		shufps		xm1, xm1, 0				;xm1 = [0.4054]

		test		ecx, 15
		jz			short .lp
		movaps		xm2, [eax+4* 0]
		movaps		xm3, [eax+4* 4]
		add			eax, byte 32
		mulps		xm2, xm0
		mulps		xm3, xm0
		addps		xm2, xm1
		addps		xm3, xm1
		cvttps2dq	xm2, xm2
		cvttps2dq	xm3, xm3
		movaps		[edx+4* 0], xm2
		movaps		[edx+4* 4], xm3
		add			edx, byte 32
		sub			ecx, byte 8
		jmp			.lp
		align 16
.lp:
		movaps		xm2, [eax+4* 0]
		movaps		xm3, [eax+4* 4]
		movaps		xm4, [eax+4* 8]
		movaps		xm5, [eax+4*12]
		add			eax, byte 64
		mulps		xm2, xm0
		mulps		xm3, xm0
		mulps		xm4, xm0
		mulps		xm5, xm0
		addps		xm2, xm1
		addps		xm3, xm1
		addps		xm4, xm1
		addps		xm5, xm1
		cvttps2dq	xm2, xm2
		cvttps2dq	xm3, xm3
		cvttps2dq	xm4, xm4
		cvttps2dq	xm5, xm5
		movaps		[edx+4* 0], xm2
		movaps		[edx+4* 4], xm3
		movaps		[edx+4* 8], xm4
		movaps		[edx+4*12], xm5
		add			edx, byte 64
		sub			ecx, byte 16
		jnz			.lp
.exit:
		ret

;	cf.
;	quantize_xrpow_ISO with TAKEHIRO_IEEE754_HACK(3900clk@PIII)

;	by shigeo
;	Init. version 2400clk@PIII

proc	quantize_xrpow_ISO_FPU
%assign _P (4*0)
		mov			eax, [esp+_P+12]		; istepPtr
		fld			dword [eax]				; istep
		fld			dword [f0p4054]			; 0.4054, istep
		mov			ecx, [RO.ixend]
		mov			eax, [esp+_P+4]			; xr
		mov			edx, [esp+_P+8]			; ix
		lea			eax, [eax+ecx*4]
		lea			edx, [edx+ecx*4]
		neg			ecx

		;切り捨てモード
%if 0
		push		eax
		fnstcw		word [esp]
		or			word [esp], 0x0C00
		fldcw		word [esp]
%else
		push		dword 0xE72				; きめうちだけどいいでしょう
		fldcw		word [esp]
%endif
%if 1
.lp:	;2400clk@PIII
		fld			dword [eax+ecx*4]		; xr, 0.4054, istep
		fld			dword [eax+ecx*4+4]
		fld			dword [eax+ecx*4+8]
		fld			dword [eax+ecx*4+12]	; 3, 2, 1, 0, 0.4054, istep
		fmul		st0, st5
		fxch		st1						; 2, 3x, 1, 0
		fmul		st0, st5
		fxch		st2						; 1, 3x, 2x, 0
		fmul		st0, st5
		fxch		st3						; 0, 3x, 2x, 1x
		fmul		st0, st5
		fxch		st1						; 3x, 0x, 2x, 1x
		fadd		st0, st4
		fxch		st2						; 2x, 0x, 3x+, 1x
		fadd		st0, st4
		fxch		st3						; 1x, 0x, 3x+, 2x+
		fadd		st0, st4
		fxch		st1						; 0x, 1x+, 3x+, 2x+
		fadd		st0, st4
		fxch		st2						; 3x+, 1x+, 0x+, 2x+
		fistp		dword [edx+ecx*4+12]
		fistp		dword [edx+ecx*4+4]
		fistp		dword [edx+ecx*4]
		fistp		dword [edx+ecx*4+8]
		add			ecx, 4
		jnz			.lp
%else
.lp:	;3000clk@PIII
		fld			dword [eax+ecx*4]		; xr, 0.4054, istep
		fmul		st0, st2
		fadd		st0, st1
		fistp		dword [edx+ecx*4]
		inc			ecx
		jnz			.lp
%endif
.exit:
		fcompp
		;元に戻す
%if 0
		and			word [esp], ~0x0C00
		fldcw		word [esp]
%else
		mov			dword [esp], GOGO_FPU_STATE
		fldcw		word [esp]
%endif
		pop			eax

		ret


;void pow075(
;	float xr[576],
;	float xrpow[576],
;	float *psum,
;	float *pmax,
;	uint32 *xr_sign);

;	2001/08/11
;	Init. version by shigeo
;	14.6Kclk@PIII
;	TODO: 近似計算によるルートの高速化
;	2002/1/19 12.6Kclk => 4250clk(うるりさんの近似計算 for SSE2を借用)
;	2002/2/3 近似計算の簡略化 3700clk(さてどうしよう?)

proc	pow075_SSE
		push		ebx
%assign _P 4*1
		mov			eax, [esp+_P+4]			; eax = xr
		mov			ebx, [esp+_P+8]			; ebx = xrpow
		mov			edx, [esp+_P+20]		; edx = xr_sign
		mov			ecx, [RO.ixend]
		shl			ecx, 2
		xorps		xm2, xm2				; xm2 = srpow_sum
		xorps		xm3, xm3				; xm3 = xrpow_max
		neg			ecx
		sub			eax, ecx
		sub			ebx, ecx
		sub			edx, ecx
		movaps		xm7, [Q_fm0p25]
		jmp			.lp
		align 16
.lp:
		movaps		xm0, [eax+ecx+ 0]
		movaps		xm1, [eax+ecx+16]
		xorps		xm4, xm4				; xm4 = 0
		movaps		xm5, xm0
		andps		xm0, [Q_ABS]			; xm0 = |xr|
		movaps		xm6, xm1
		andps		xm1, [Q_ABS]
		cmpltps		xm5, xm4				; xm5 = (xr<0) ? -1 : 0
		cmpltps		xm6, xm4
		movaps		[eax+ecx+ 0], xm0
		movaps		[eax+ecx+16], xm1
		addps		xm2, xm0
		movaps		[edx+ecx+ 0], xm5
		movaps		[edx+ecx+16], xm6
		addps		xm2, xm1

		;------x^(3/4)------
		rsqrtps		xm4, xm0
		movaps		xm5, xm0
		rsqrtps		xm4, xm4
		mulps		xm5, xm7				; -x/4
		rcpps		xm4, xm4				; y = approx. x^-0.25
		movaps		xm6, xm4
		mulps		xm4, xm4				; y^2
		mulps		xm0, xm6				; yx
		mulps		xm5, xm4
		mulps		xm5, xm4
		addps		xm5, [Q_f1p25]			; 5/4 - 1/4 * x (y^4)
		mulps		xm0, xm5

		rsqrtps		xm4, xm1
		movaps		xm5, xm1
		rsqrtps		xm4, xm4
		mulps		xm5, xm7				; -x/4
		rcpps		xm4, xm4				; y = approx. x^-0.25
		movaps		xm6, xm4
		mulps		xm4, xm4				; y^2
		mulps		xm1, xm6				; yx
		mulps		xm5, xm4
		mulps		xm5, xm4
		xorps		xm4, xm4
		addps		xm5, [Q_f1p25]			; 5/4 - 1/4 * x (y^4)
		mulps		xm1, xm5

		maxps		xm0, xm4				; if(x == 0) then x^(3/4) = 0
		maxps		xm1, xm4
		;------------------

		maxps		xm3, xm0
		maxps		xm3, xm1
		movaps		[ebx+ecx+ 0], xm0		; xrpow
		movaps		[ebx+ecx+16], xm1
		add			ecx, 8*4
		jnz near	.lp

		movhlps		xm4, xm2				;* * 3 2
		movhlps		xm5, xm3				;* * 3 2
		addps		xm4, xm2				;* * 1 0
		maxps		xm5, xm3				;* * 1 0
		mov			eax, [esp+_P+12]		; psum
		mov			edx, [esp+_P+16]		; pmax
		movaps		xm2, xm4
		shufps		xm4, xm4, PACK(1,1,1,1)	;* * * 1
		movaps		xm3, xm5
		shufps		xm5, xm5, PACK(1,1,1,1)	;* * * 1
		addps		xm4, xm2				;* * * 0
		maxps		xm5, xm3				;* * * 0
		movss		[eax], xm4
		movss		[edx], xm5
.exit:
		pop			ebx
		ret

proc	pow075_SSE2
		push		ebx
%assign _P 4*1
		mov			eax, [esp+_P+4]			; eax = xr
		mov			ebx, [esp+_P+8]			; ebx = xrpow
		mov			edx, [esp+_P+20]		; edx = xr_sign
		mov			ecx, [RO.ixend]
		shl			ecx, 2
		xorps		xm2, xm2				; xm2 = srpow_sum
		xorps		xm3, xm3				; xm3 = xrpow_max
		neg			ecx
		sub			eax, ecx
		sub			ebx, ecx
		sub			edx, ecx
		jmp			.lp
		align 16
.lp:
		movaps		xm0, [eax+ecx+ 0]
		movaps		xm1, [eax+ecx+16]
		movaps		xm3, [Q_ABS]
		movaps		xm4, [Q_i31]
		movaps		xm6, xm0
		andps		xm0, xm3				; xm0 = |xr|
		movaps		xm7, xm1
		andps		xm1, xm3
		psrad		xm6, xm4
		psrad		xm7, xm4
		movaps		[eax+ecx+ 0], xm0
		movaps		[eax+ecx+16], xm1
		addps		xm2, xm0
		addps		xm2, xm1
		movaps		[edx+ecx+ 0], xm6
		movaps		[edx+ecx+16], xm7

		;------x^(3/4)------
		movaps		xm7, [Q_fm0p25]
		rsqrtps		xm4, xm0
		movaps		xm5, xm0
		rsqrtps		xm4, xm4
		mulps		xm5, xm7				; -x/4
		rcpps		xm4, xm4				; y = approx. x^-0.25
		movaps		xm6, xm4
		mulps		xm4, xm4				; y^2
		mulps		xm0, xm6				; yx
		mulps		xm5, xm4
		mulps		xm5, xm4
		addps		xm5, [Q_f1p25]			; 5/4 - 1/4 * x (y^4)
		mulps		xm0, xm5

		rsqrtps		xm4, xm1
		movaps		xm5, xm1
		rsqrtps		xm4, xm4
		mulps		xm5, xm7				; -x/4
		rcpps		xm4, xm4				; y = approx. x^-0.25
		movaps		xm6, xm4
		mulps		xm4, xm4				; y^2
		mulps		xm1, xm6				; yx
		mulps		xm5, xm4
		mulps		xm5, xm4
		xorps		xm4, xm4
		addps		xm5, [Q_f1p25]			; 5/4 - 1/4 * x (y^4)
		mulps		xm1, xm5

		maxps		xm0, xm4				; if (x == 0) then x^(3/4) = 0
		maxps		xm1, xm4
		;------------------

		maxps		xm3, xm0
		maxps		xm3, xm1
		movaps		[ebx+ecx+ 0], xm0		; xrpow
		movaps		[ebx+ecx+16], xm1
		add			ecx, 8*4
		jnz near	.lp

		movhlps		xm4, xm2				;* * 3 2
		movhlps		xm5, xm3				;* * 3 2
		addps		xm4, xm2				;* * 1 0
		maxps		xm5, xm3				;* * 1 0
		mov			eax, [esp+_P+12]		; psum
		mov			edx, [esp+_P+16]		; pmax
		movaps		xm2, xm4
		shufps		xm4, xm4, PACK(1,1,1,1)	;* * * 1
		movaps		xm3, xm5
		shufps		xm5, xm5, PACK(1,1,1,1)	;* * * 1
		addps		xm4, xm2				;* * * 0
		maxps		xm5, xm3				;* * * 0
		movss		[eax], xm4
		movss		[edx], xm5
.exit:
		pop			ebx
		ret


%define max_index		BASE+_P+ 4
%define xr				BASE+_P+ 8
%define ix				BASE+_P+12
%define cod_info		BASE+_P+16
%define l3_xmin			BASE+_P+20
%define scalefac		BASE+_P+24
%define xfsf			BASE+_P+28
%define res				BASE+_P+32

%define LOCAL_SIZE 4*24+576*4+64				; キャッシュラインに整合
%define BASE esp+LOCAL_SIZE

%define sum@			ebp+0
%define through@		ebp+4*24		; mm <---> mem <---> xmm 相互移動用

;	2001/08/15
;	test version by shigeo
;	4500clk@PIII あんまり速くない
;	2001/09/05
;	3800clk	うー全部書き直したのに...
;	2001/09/06
;	3600clk 気持ち速くなったかも
;	log部分追加 3800clk 遅い。悔しい。
;	2001/09/07
;	こっちではもっと遅いの...	4710->3610 速くなった？ by sakai
;	2001/09/08
;	3610->3270 by sakai
;	2001/09/20
;	私んとこで2800clk by shigeo

proc	calc_noise_long_SSE
		push		ebx
		push		esi
		push		edi
		mov			edx, RO.pow20
		push		ebp
		sub			esp, LOCAL_SIZE
		lea			ebp, [esp+63]
		and			ebp, ~63
%assign _P 4*4
		mov			edi, [cod_info]
		mov			esi, [scalefac]
		lea			esi, [esi+III_scalefac.l]
		mov			ecx, [edi+gr_info_s.scalefac_scale]
		inc			ecx
		movd		mm4,[edi+gr_info_s.global_gain]
		movd		mm6,[edi+gr_info_s.preflag]
		pxor		mm5,mm5
		pshufw		mm4,mm4,0	; mm4 = [gain:gain:gain:gain]
		punpckldq	mm6,mm6		; mm6 = [preflag:preflag]
		movd		mm7, ecx				; mm7 = scalefac_scale+1
		pcmpgtd		mm6,mm5		; mm6 = (preflag)?-1:0

		; 後半のsumとsを共用するけど型が違うよん
	; sfb = 0..7
		movq		mm2, [pretab+0]
		pand		mm2, mm6
		movq		mm3,mm2
		punpcklbw	mm2,mm5
		punpckhbw	mm3,mm5
		movq		mm0, [esi+ 0*4]
		packssdw	mm0, [esi+ 2*4]			; mm0 = [ 3: 2: 1: 0]
		movq		mm1, [esi+ 4*4]
		packssdw	mm1, [esi+ 6*4]			; mm1 = [ 7: 6: 5: 4]
		paddw		mm0, mm2				; if(preflag) s+=pretab[sfb]
		psllw		mm0, mm7				; s <<= (cod_info->scalefac_scale + 1)
		movq		mm2, mm4
		psubw		mm2, mm0				; s = global_gain - s
		pextrw		eax,mm2,0
		pextrw		ecx,mm2,1
		movd		mm0,[edx+eax*4]			; step = POW20[s]
		punpckldq	mm0,[edx+ecx*4]			; step = POW20[s]
		pextrw		eax,mm2,2
		pextrw		ecx,mm2,3
		movd		mm2,[edx+eax*4]			; step = POW20[s]
		punpckldq	mm2,[edx+ecx*4]			; step = POW20[s]
		paddw		mm1, mm3
		psllw		mm1, mm7				; s <<= (cod_info->scalefac_scale + 1)
		movq		mm3, mm4
		psubw		mm3, mm1				; s = global_gain - s
		pextrw		eax,mm3,0
		pextrw		ecx,mm3,1
		movd		mm1,[edx+eax*4]			; step = POW20[s]
		punpckldq	mm1,[edx+ecx*4]			; step = POW20[s]
		pextrw		eax,mm3,2
		pextrw		ecx,mm3,3
		movd		mm3,[edx+eax*4]			; step = POW20[s]
		punpckldq	mm3,[edx+ecx*4]			; step = POW20[s]
		movq		[sum@+ 0*4],mm0
		movq		[sum@+ 2*4],mm2
		movq		[sum@+ 4*4],mm1
		movq		[sum@+ 6*4],mm3

	; sfb = 8..15
		movq		mm2, [pretab+8]
		pand		mm2, mm6
		movq		mm3,mm2
		punpcklbw	mm2,mm5
		punpckhbw	mm3,mm5
		movq		mm0, [esi+ 8*4]
		packssdw	mm0, [esi+10*4]			; mm0 = [11:10: 9: 8]
		movq		mm1, [esi+12*4]
		packssdw	mm1, [esi+14*4]			; mm1 = [15:14:13:12]
		paddw		mm0, mm2				; if(preflag) s+=pretab[sfb]
		psllw		mm0, mm7				; s <<= (cod_info->scalefac_scale + 1)
		movq		mm2, mm4
		psubw		mm2, mm0				; s = global_gain - s
		pextrw		eax,mm2,0
		pextrw		ecx,mm2,1
		movd		mm0,[edx+eax*4]			; step = POW20[s]
		punpckldq	mm0,[edx+ecx*4]			; step = POW20[s]
		pextrw		eax,mm2,2
		pextrw		ecx,mm2,3
		movd		mm2,[edx+eax*4]			; step = POW20[s]
		punpckldq	mm2,[edx+ecx*4]			; step = POW20[s]
		paddw		mm1, mm3
		psllw		mm1, mm7				; s <<= (cod_info->scalefac_scale + 1)
		movq		mm3, mm4
		psubw		mm3, mm1				; s = global_gain - s
		pextrw		eax,mm3,0
		pextrw		ecx,mm3,1
		movd		mm1,[edx+eax*4]			; step = POW20[s]
		punpckldq	mm1,[edx+ecx*4]			; step = POW20[s]
		pextrw		eax,mm3,2
		pextrw		ecx,mm3,3
		movd		mm3,[edx+eax*4]			; step = POW20[s]
		punpckldq	mm3,[edx+ecx*4]			; step = POW20[s]
		movq		[sum@+ 8*4],mm0
		movq		[sum@+10*4],mm2
		movq		[sum@+12*4],mm1
		movq		[sum@+14*4],mm3

	; sfb = 16..23(だけど20,21までで22,23は使わない)
		pand		mm6, [pretab+16]
		movq		mm3,mm6
		punpcklbw	mm6,mm5
		punpckhbw	mm3,mm5
		movq		mm0, [esi+16*4]
		packssdw	mm0, [esi+18*4]			; mm0 = [19:18:17:16]
		movq		mm1, [esi+20*4]
		packssdw	mm1, mm1				; mm1 = [21:20:21:20]
		paddw		mm0, mm6				; if(preflag) s+=pretab[sfb]
		psllw		mm0, mm7				; s <<= (cod_info->scalefac_scale + 1)
		movq		mm2, mm4
		psubw		mm2, mm0				; s = global_gain - s
		pextrw		eax,mm2,0
		pextrw		ecx,mm2,1
		movd		mm0,[edx+eax*4]			; step = POW20[s]
		punpckldq	mm0,[edx+ecx*4]			; step = POW20[s]
		pextrw		eax,mm2,2
		pextrw		ecx,mm2,3
		movd		mm2,[edx+eax*4]			; step = POW20[s]
		punpckldq	mm2,[edx+ecx*4]			; step = POW20[s]
		paddw		mm1, mm3
		psllw		mm1, mm7				; s <<= (cod_info->scalefac_scale + 1)
		psubw		mm4, mm1				; s = global_gain - s
		pextrw		eax,mm4,0
		pextrw		ecx,mm4,1
		movd		mm1,[edx+eax*4]			; step = POW20[s]
		punpckldq	mm1,[edx+ecx*4]			; step = POW20[s]
		movq		[sum@+16*4],mm0
		movq		[sum@+18*4],mm2
		movq		[sum@+20*4],mm1
		; sum[22], sum[23]は使わない

;;ここまではほとんど時間を消費しない。

; edi = [cod_info], mm5 = 0
; 時間を食うところを切りだした。
		mov			eax,[ix]
		mov			ecx,[max_index]
		mov			ebx,[RO.scalefac_band+scalefac_struct_s.l+ecx*4]
		mov			ecx,[edi+gr_info_s.big_values]
		sub			ecx,ebx
		jae			.fill_big_value
		mov			edx,[edi+gr_info_s.count1]
		sub			edx,ebx
		jae			.fill_count1

.fill_zero
;; zero area
		lea			edi,[through@+ebx*4]
		add			ebx,edx
		sub			ecx,edx

		loopalign
.fill_zero.lp:
		movq		[edi+edx*4],mm5
		add			edx,byte 2
		jnz			.fill_zero.lp
		test		ecx,ecx
		jz			.filled_count1
;		jecxz		.filled_count1

.fill_count1
;; count1 area(VBR時は多少こっちにくるらしい)
		lea			edi,[through@+ebx*4]
		lea			esi,[eax+ebx*4]
		add			ebx,ecx
		movd		mm6,[RO.pow43+4]
		punpckldq	mm6,mm6

		loopalign
.fill_count1.lp:
		movq		mm1,[esi+ecx*4]
		pcmpgtd		mm1,mm5
		pand		mm1,mm6
		movq		[edi+ecx*4],mm1
		add			ecx,2
		jnz			.fill_count1.lp

.filled_count1:
		test		ebx,ebx
		jz			.filled_big_value

.fill_big_value
;; big_value area
		lea			ebx,[ebx*4+31]
		and			 bl,11100000B
		lea			edi,[through@+ebx]
		lea			esi,[eax+ebx]
		neg			ebx
		lea			ecx,[RO.pow43]

		loopalign
.fill_big_value.lp:
		mov			edx, [esi+ebx]
		mov			eax, [esi+ebx+4]
		movd		mm0,[ecx+edx*4]
		punpckldq	mm0,[ecx+eax*4]
		movq		[edi+ebx],mm0

		mov			edx, [esi+ebx+8]
		mov			eax, [esi+ebx+12]
		movd		mm1,[ecx+edx*4]
		punpckldq	mm1,[ecx+eax*4]
		movq		[edi+ebx+8],mm1

		mov			edx, [esi+ebx+16]
		mov			eax, [esi+ebx+20]
		movd		mm2,[ecx+edx*4]
		punpckldq	mm2,[ecx+eax*4]
		movq		[edi+ebx+16],mm2

		mov			edx, [esi+ebx+24]
		mov			eax, [esi+ebx+28]
		movd		mm3,[ecx+edx*4]
		punpckldq	mm3,[ecx+eax*4]
		movq		[edi+ebx+24],mm3

		add			ebx,byte 32
		jnz			.fill_big_value.lp
.filled_big_value:

;;
		lea			esi, [through@]
		mov			edi, [xr]
		xor			edx, edx				; edx = sfb = 0
		mov			ebx, [RO.scalefac_band+scalefac_struct_s.l+4]

		loopalign
.lp.sfb0.dual:
		shl			ebx,2
		movlps		xmm7,[sum@+edx*4]	; xm7 = step
		unpcklps	xmm7,xmm7
		movlps		xmm0,[esi]
		movhps		xmm0,[esi+ebx]
		movlps		xmm4, [edi]
		movhps		xmm4, [edi+ebx]
		mulps		xmm0, xmm7
		subps		xmm0, xmm4
		mulps		xmm0, xmm0
		neg			ebx
		lea			eax,[ebx+8]
		shl			ebx,1
		sub			esi,ebx
		sub			edi,ebx
		add			ebx,2*4

		loopalign
;; サイドバンド２つずつ×２要素
.lp.n.dual:
		movlps		xmm1,[esi+ebx]
		movhps		xmm1,[esi+eax]
		mulps		xmm1, xmm7
		movlps		xmm4, [edi+ebx]
		movhps		xmm4, [edi+eax]
		subps		xmm1, xmm4
		mulps		xmm1, xmm1
		addps		xmm0, xmm1
		add			ebx,2*4
		add			eax,2*4
		jnz			short .lp.n.dual

		shufps		xmm0,xmm0,11011000B
		movhlps		xmm1,xmm0
		addps		xmm0,xmm1
		movlps		[sum@+edx*4],xmm0
		add			edx, 2

		mov			eax, [RO.scalefac_band+scalefac_struct_s.l+edx*4+8]
		mov			ebx, [RO.scalefac_band+scalefac_struct_s.l+edx*4+4]
		sub			eax, ebx
		sub			ebx, [RO.scalefac_band+scalefac_struct_s.l+edx*4]		; n
		sub			eax,ebx
		jz			near .lp.sfb0.dual

		cmp			eax,2
		jne			.lp.sfb0.single

;; サイドバンド１つ×２要素
		movlps		xmm7,[sum@+edx*4]	; xm7 = step
		unpcklps	xmm7,xmm7

		xorps		xmm0,xmm0
		movhps		xmm0,[esi+ebx*4]
		xorps		xmm4,xmm4
		movhps		xmm4,[edi+ebx*4]
		mulps		xmm0,xmm7
		subps		xmm0,xmm4
		mulps		xmm0,xmm0

		lea			esi,[esi+ebx*8+8]
		lea			edi,[edi+ebx*8+8]
		neg			ebx
		lea			eax,[ebx*4]
		lea			ebx,[ebx*8-8]
		jmp			.lp.n.dual

		align	16
.lp.sfb0:
		mov			ebx, [RO.scalefac_band+scalefac_struct_s.l+edx*4+4]
		sub			ebx, [RO.scalefac_band+scalefac_struct_s.l+edx*4]		; n
.lp.sfb0.single:
		xorps		xmm0,xmm0
		movss		xm7,[sum@+edx*4]	; xm7 = step
		shufps		xmm7,xmm7,0
		shl			ebx,2
		add			esi,ebx
		add			edi,ebx
		neg			ebx
		test		ebx,2*4
		jz			.lp.2n
;; サイドバンド１つ×２要素
.lp.n:
		xorps		xmm4,xmm4
		movlps		xmm0,[esi+ebx]
		movlps		xmm4,[edi+ebx]
		mulps		xmm0,xmm7
		subps		xmm0,xmm4
		mulps		xmm0,xmm0
		add			ebx,2*4

		loopalign
;; サイドバンド１つずつ×４要素
.lp.2n:
		movlps		xmm2,[esi+ebx]
		movhps		xmm2,[esi+ebx+8]
		movlps		xmm1,[edi+ebx]
		movhps		xmm1,[edi+ebx+8]
		add			ebx,4*4
		mulps		xm2, xm7
		subps		xm2, xm1
		mulps		xm2, xm2
		addps		xm0, xm2
		jnz			.lp.2n

		movhlps		xm1,xm0
		addps		xm0, xm1
		movaps		xm2,xm0
		shufps		xm2,xm2,01010101B
		addss		xm0,xm2
		movss		[sum@+edx*4], xm0
		inc			edx
		cmp			edx, [max_index]
		jnz			near .lp.sfb0

;;
		mov			esi, [l3_xmin]			; esi = l3_xmin
		mov			edi, [xfsf]				; edi = xfsf
		lea			esi, [esi+III_psy_xmin_s.l]
		lea			edi, [edi+III_psy_xmin_s.l]

		; noise[0], ..., noise[11]
		movaps		xm0, [esi+ 0*4]			; xm0 = l3_xmin_l
		movaps		xm2, [esi+ 4*4]
		movaps		xm4, [esi+ 8*4]
		rcpps		xm1, xm0
		rcpps		xm3, xm2
		rcpps		xm5, xm4
		mulps		xm0, xm1
		mulps		xm2, xm3
		mulps		xm4, xm5
		mulps		xm0, xm1
		mulps		xm2, xm3
		mulps		xm4, xm5
		addps		xm1, xm1
		addps		xm3, xm3
		addps		xm5, xm5
		subps		xm1, xm0
		subps		xm3, xm2
		subps		xm5, xm4
		; noise[12], ..., noise[23]
		movaps		xm0, [esi+12*4]
		movaps		xm2, [esi+16*4]
		movaps		xm4, [esi+20*4]
		mulps		xm1, [sum@+ 0*4]		; xm1 = sum / l3_xmin_l[sfb]
		mulps		xm3, [sum@+ 4*4]
		mulps		xm5, [sum@+ 8*4]
		movaps		[edi+ 0*4], xm1			; noise = xfsf_l
		movaps		[edi+ 4*4], xm3
		movaps		[edi+ 8*4], xm5

		rcpps		xm1, xm0
		rcpps		xm3, xm2
		rcpps		xm5, xm4
		mulps		xm0, xm1
		mulps		xm2, xm3
		mulps		xm4, xm5
		mulps		xm0, xm1
		mulps		xm2, xm3
		mulps		xm4, xm5
		addps		xm1, xm1
		addps		xm3, xm3
		addps		xm5, xm5
		subps		xm1, xm0
		subps		xm3, xm2
		subps		xm5, xm4
		mulps		xm1, [sum@+12*4]
		mulps		xm3, [sum@+16*4]
		mulps		xm5, [sum@+20*4]
		movaps		[edi+12*4], xm1
		movaps		[edi+16*4], xm3
		; noise[21:20]
		movlps		[edi+20*4], xm5

		movq		mm3, [Q_f1]
		mov			eax, [max_index]
		mov			ecx, [edi+21*4]
		movd		edx, mm3
		cmp			eax, byte 21
		cmove		ecx, edx				; ecx = (max_index===21)?1.0:sum[21]
		movq		[edi+22*4], mm3			; sum[22] = sum[23] = 1.0;
		pxor		mm0, mm0				; mm0 = qtot_noise.exponent
		mov			[edi+21*4], ecx

		mov			ecx, 20					; 20 = (max_index+3) & ~3
		movq		mm1, mm0				; mm1 = qover_noise.exponent
		movq		mm2, mm0				; mm2 = over
		movaps		xm6, [Q_f1]				; xm6 = qtot_noise.mantissa
		movaps		xm7, xm6				; xm7 = qover_noise.mantissa
		movaps		xm5, xm6				; xm5 = 1.0

		movaps		xm0, [Q_1shl23m1]
		movaps		xm3, [Q_127shl23]

		loopalign
.lp.sfb1:
		movq		mm4, [edi+ecx*4]
		movq		mm5, [edi+ecx*4+8]
		movaps		xm1, [edi+ecx*4]		; noise
		movaps		xm4, xm1
		andps		xm1, xm0
		movq		mm6, mm4
		movq		mm7, mm5
		orps		xm1, xm3				; ftmp
		psrld		mm4, 23
		psrld		mm5, 23
		pcmpgtd		mm6, mm3				; mm6 = (noise>1)?-1:0
		pcmpgtd		mm7, mm3
		cmpnleps	xm4, xm5				; xm4 = (noise>1)?-1:0
		psubd		mm2, mm6				; if(noise>1)over++;
		mulps		xm6, xm1				; qtot_noise.mantissa
		psubd		mm2, mm7
		andps		xm1, xm4				; xm1 = (noise>1)?mantissa:0
		pand		mm6, mm4				; mm6 = (noise>1)?exponent:0
		andnps		xm4, xm5				; xm4 = (noise>1)?0:1.0
		sub			ecx, 4
		pand		mm7, mm5
		paddd		mm0, mm4				; qtot_noise->exponent
		paddd		mm1, mm6
		orps		xm1, xm4				; xm4 = (noise>1)?mantissa:1.0
		paddd		mm0, mm5
		paddd		mm1, mm7
		mulps		xm7, xm1				; qover_noise->mantissa
		jnc			.lp.sfb1

		; mm0 = qtot_noise
		; mm1 = qover_noise
		; mm2 = over
		; xm6 = qtot_noise
		; xm7 = qover_noise

		movq		mm3, mm0				; qtot_noise [1:0]
		movq		mm4, mm1				; qover_noise[1:0]
		movhlps		xm0, xm6				; [*:*:3:2]
		punpckldq	mm0, mm1				; [qover0:qtot0]
		punpckhdq	mm3, mm4				; [qover1:qtot1]
		movhlps		xm1, xm7
		pshufw		mm5, mm2, PACK(0,0,3,2)	; over[1:0]
		movlhps		xm6, xm7				; [1':0':1:0]
		movlhps		xm0, xm1				; [3':2':3:2]
		paddd		mm0, mm3				; [qover:qtot].exponent
		mulps		xm0, xm6				; [3'*1':2'*0':3*1:2*0]
		mov			eax, [res]
		paddd		mm2, mm5				; over
		movaps		xm1, xm0
		movd		[eax+calc_noise_result_t.over_count], mm2
		shufps		xm0, xm0, PACK(0,0,2,0)	; [*:*:2'*0':2*0]
		shufps		xm1, xm1, PACK(0,0,3,1)	; [*:*:3'*1':3*1]
		mulps		xm0, xm1				; [*:*:qover:qtot]
		movlps		[sum@], xm0				; [qover:qtot].mantissa
		movq		mm1, [sum@]

		;	mm0 = [qover.exponent:qtot.exponent]
		;	mm1 = [qover.mantissa:qtot.mantissa]
		movq		mm2, mm1
		psrld		mm2, 23
		pand		mm1, [Q_1shl23m1]
		paddd		mm0, mm2
		psrld		mm1, 4
		pslld		mm0, 19
		por			mm0, mm1
.exit:
		add			esp, LOCAL_SIZE
		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		movq		[eax+calc_noise_result_t.tot_noise], mm0
		emms
		ret

;	2001/12/06 shigeo
;	first version
;	32000clk@PIII => 14000clk
;	1152loop/752loop
;	float	mfbuf[2][4][576];	/* ->FFT, ->PFB */
;	1152 = 144 * 8
;	752  =  94 * 8
proc	dist_mfbuf_mpeg1stereo_SSE
		push		ebx
%assign _P 4*1
		mov			eax, [esp+_P+4]		; eax = buf_L
		lea			ebx, [eax+4*576*4]	; ebx = buf_R

		mov			edx, [RW.fr0]		; edx = fr0
		mov			ecx, 1152/8
		jmp			.lp0
		align		16
.lp0:
		movq		mm0, [edx]			; mm0 = [3:2:1:0]
		movq		mm1, [edx+8]		; mm1 = [7:6:5:4]
		movq		mm4, [edx+16]
		movq		mm5, [edx+24]
		add			edx, byte 32
		pshufw		mm2, mm0, PACK(2,2,0,0)	; mm2 = [2:*:0:*]
		pshufw		mm6, mm4, PACK(2,2,0,0)
		psrad		mm0, 16				; mm0 = [  3:  1]
		psrad		mm4, 16
		pshufw		mm3, mm1, PACK(2,2,0,0)	; mm3 = [6:*:4:*]
		pshufw		mm7, mm5, PACK(2,2,0,0)
		psrad		mm1, 16				; mm1 = [  7:  5]
		psrad		mm5, 16
		psrad		mm2, 16				; mm2 = [  2:  0]
		psrad		mm6, 16
		psrad		mm3, 16				; mm3 = [  6:  4]
		psrad		mm7, 16
		cvtpi2ps	xm0, mm2			; xm0 = [*:*:2:0]
		cvtpi2ps	xm4, mm6
		cvtpi2ps	xm1, mm0			; xm1 = [*:*:3:1]
		cvtpi2ps	xm5, mm4
		cvtpi2ps	xm2, mm3			; xm2 = [*:*:6:4]
		cvtpi2ps	xm6, mm7
		cvtpi2ps	xm3, mm1			; xm3 = [*:*:7:5]
		cvtpi2ps	xm7, mm5
		movlhps		xm0, xm2			; xm0 = [6:4:2:0]
		movlhps		xm4, xm6
		movlhps		xm1, xm3			; xm1 = [7:5:3:1]
		movlhps		xm5, xm7
		movaps		[eax], xm0
		movaps		[eax+16], xm4
		add			eax, byte 32
		movaps		[ebx], xm1
		movaps		[ebx+16], xm5
		add			ebx, byte 32
		dec			ecx
		jnz			.lp0

		mov			edx, [RW.fr1]		; edx = fr1
		mov			ecx, 752/8
		jmp			.lp1
		; 上のlp0-loopと全く同じ
		align		16
.lp1:
		movq		mm0, [edx]			; mm0 = [3:2:1:0]
		movq		mm1, [edx+8]		; mm1 = [7:6:5:4]
		movq		mm4, [edx+16]
		movq		mm5, [edx+24]
		add			edx, byte 32
		pshufw		mm2, mm0, PACK(2,2,0,0)	; mm2 = [2:*:0:*]
		pshufw		mm6, mm4, PACK(2,2,0,0)
		psrad		mm0, 16				; mm0 = [  3:  1]
		psrad		mm4, 16
		pshufw		mm3, mm1, PACK(2,2,0,0)	; mm3 = [6:*:4:*]
		pshufw		mm7, mm5, PACK(2,2,0,0)
		psrad		mm1, 16				; mm1 = [  7:  5]
		psrad		mm5, 16
		psrad		mm2, 16				; mm2 = [  2:  0]
		psrad		mm6, 16
		psrad		mm3, 16				; mm3 = [  6:  4]
		psrad		mm7, 16
		cvtpi2ps	xm0, mm2			; xm0 = [*:*:2:0]
		cvtpi2ps	xm4, mm6
		cvtpi2ps	xm1, mm0			; xm1 = [*:*:3:1]
		cvtpi2ps	xm5, mm4
		cvtpi2ps	xm2, mm3			; xm2 = [*:*:6:4]
		cvtpi2ps	xm6, mm7
		cvtpi2ps	xm3, mm1			; xm3 = [*:*:7:5]
		cvtpi2ps	xm7, mm5
		movlhps		xm0, xm2			; xm0 = [6:4:2:0]
		movlhps		xm4, xm6
		movlhps		xm1, xm3			; xm1 = [7:5:3:1]
		movlhps		xm5, xm7
		movaps		[eax], xm0
		movaps		[eax+16], xm4
		add			eax, byte 32
		movaps		[ebx], xm1
		movaps		[ebx+16], xm5
		add			ebx, byte 32
		dec			ecx
		jnz			.lp1
.exit:
		emms
		pop			ebx
		ret

;	2004/01/05	Init. version 1510clk@P4-2.4G by kei
;               (C版は4730clk@P4-2.4G)
;	2004/01/06	1480clk@P4-2.4G by kei
proc	ms_convert_SSE ;(FLOAT8* srcl, FLOAT8* srcr, int n);
%assign _P (4*0)
		mov			ecx, [esp+_P+12]		;n
		mov			eax, [esp+_P+4]			;srcl
		mov			edx, [esp+_P+8]			;srcr
		lea			eax, [eax+ecx*4]
		lea			edx, [edx+ecx*4]
		neg			ecx
		movaps		xm7, [Q_fSQRT2_DIV_2]
		loopalign	16
.lp:
		movaps		xm2, [edx+ecx*4+ 0] ; srcr
		movaps		xm0, [eax+ecx*4+ 0] ; srcl
		movaps		xm3, [edx+ecx*4+16] ; srcr
		movaps		xm1, [eax+ecx*4+16] ; srcl
		addps		xm2, xm0 ; l + r
		addps		xm3, xm1 ; l + r
		subps		xm0, [edx+ecx*4+ 0] ; l - r
		subps		xm1, [edx+ecx*4+16] ; l - r
		add			ecx, byte 8
		mulps		xm2, xm7
		mulps		xm3, xm7
		mulps		xm0, xm7
		mulps		xm1, xm7
		movaps		[eax+ecx*4+ 0-32], xm2 
		movaps		[eax+ecx*4+16-32], xm3 
		movaps		[edx+ecx*4+ 0-32], xm0 
		movaps		[edx+ecx*4+16-32], xm1 
		jl			near .lp
.lp.end:
.exit:
		ret

		end
