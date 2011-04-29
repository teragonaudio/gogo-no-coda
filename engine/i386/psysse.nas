;
;   part of this code is origined from
;
;       GOGO-no-coda
;       Copyright (c) 2001,2002,2003 gogo-developer
;

%include "global.cfg"

	externdef		tonalityTbl		;float * x 2

%define tonalityTblNum 256	;defined in psymode.c

	segment_data
	align	32
Q_f0p5		dd	0.5, 0.5, 0.5, 0.5
LIMIT_L		dd	0.0487558430, 0.0487558430, 0.0487558430, 0.0487558430	; exp((1-CONV1)/CONV2)
LIMIT_U		dd	0.4989003826, 0.4989003826, 0.4989003826, 0.4989003826	; exp((0-CONV1)/CONV2))
Q_coef3		dd	568.70622093, 568.70622093, 568.70622093, 568.70622093	; tonalityTblNum / (LIMIT_U - LIMIT_L)
Q_f8		dd	8.0, 8.0, 8.0, 8.0		; rpelev = 2, rpelev2 = 16
Q_fMAX		dd	1.0e35, 1.0e35, 1.0e35, 1.0e35
Q_f2:
Q_fSHORT_TYPE	dd	2.0, 2.0, 2.0, 2.0	; = SHROT_TYPE (floatなのに注意)
Q_f1		dd	1.0, 1.0, 1.0, 1.0
Q_f3		dd	3.0, 3.0, 3.0, 3.0
Q_i127		dd	127, 127, 127, 127
Q_i0x007FFFFF	dd	0x007FFFFF, 0x007FFFFF, 0x007FFFFF, 0x007FFFFF
Q_fSQRT2	dd	1.4142135623, 1.4142135623, 1.4142135623, 1.4142135623
Q_fLOG_E_2	dd	0.6931471805, 0.6931471805, 0.6931471805, 0.6931471805
Q_fA2		dd	2.0000006209, 2.0000006209, 2.0000006209, 2.0000006209
Q_fB2		dd	0.6664778517, 0.6664778517, 0.6664778517, 0.6664778517
Q_fC2		dd	0.4139745860, 0.4139745860, 0.4139745860, 0.4139745860
Q_f1em6		dd	1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6
Q_ABS		dd	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF
Q_f0p4		dd	0.4, 0.4, 0.4, 0.4
Q_0			dd	0, 0, 0, 0
Q_fm0p5		dd	-0.5, -0.5, -0.5, -0.5
Q_f1p5		dd	1.5, 1.5, 1.5, 1.5
Q_f16		dd	16.0, 16.0, 16.0, 16.0

%define BLKSIZE		1024
%define BLKSIZE_s	256
%define HBLKSIZE_s	(BLKSIZE_s/2 + 1)
%define NBPSY_s		13
%define SBMAX_l		22
%define SBMAX_s		13
%define CBANDS		64
%define sizeof_III_psy_xmin	sizeof_III_psy_xmin_s
%define cw_lower_index	6

	segment_text
;	2001/08/23 shigeo
;	6300clk@PIII
;	やっぱり速いね.

proc	inner_psy_sub1_SSE
		push		ebx
%assign _P 4*1
		mov			eax, [esp+_P+4]		; eax = tl
		lea			ebx, [eax+gogo_thread_data_s.psywork]	; ebx = wsamp_l
		lea			ecx, [eax+gogo_thread_data_s.energy]	; ecx = energy
		movaps		xm0, [ebx]			; wsamp
		mov			eax, 1*4			; j
		mulps		xm0, xm0			; tot
		mov			edx, (BLKSIZE-1)*4	; BLKSIZE - j
		movaps		[ecx], xm0			; energy
		movaps		xm7, [Q_f0p5]		; 0.5
.lp.j.1..10:
		movaps		xm1, [ebx+eax*4]	; re
		add			eax, byte 4
		movaps		xm2, [ebx+edx*4]	; im
		sub			edx, byte 4
		mulps		xm1, xm1
		cmp			eax, byte 11*4
		mulps		xm2, xm2
		addps		xm1, xm2
		mulps		xm1, xm7
		movaps		[ecx+eax*4-16], xm1
		jne			.lp.j.1..10

; 偶数回ループ (BLKSIZE/2-11+1)=BLKSIZE/2-10

		xorps		xm1, xm1			; tot'
.lp.j.over.10:
		movaps		xm2, [ebx+eax*4]	; re
		movaps		xm4, [ebx+eax*4+16]	; re'
		add			eax, byte 8
		movaps		xm3, [ebx+edx*4]	; im
		movaps		xm5, [ebx+edx*4-16]	; im'
		sub			edx, byte 8
		mulps		xm2, xm2
		mulps		xm4, xm4
		cmp			eax, (BLKSIZE/2+1)*4
		mulps		xm3, xm3
		mulps		xm5, xm5
		addps		xm2, xm3
		addps		xm4, xm5
		mulps		xm2, xm7
		mulps		xm4, xm7
		movaps		[ecx+eax*4-32], xm2
		movaps		[ecx+eax*4-16], xm4
		addps		xm0, xm2			; tot
		addps		xm1, xm4			; tot'
		jne			.lp.j.over.10
		addps		xm0, xm1
		movaps		[RW.tot_ener], xm0
.exit:
		pop			ebx
		ret

;	2001/08/23 shigeo
;	5300clk@PIII

proc	inner_psy_sub2_SSE
		push		ebx
		push		esi
%assign _P 4*2
		mov			esi, 3
		movaps		xm7, [Q_f0p5]		; 0.5
		mov			eax, [esp+_P+4]		; eax = tl
		lea			ebx, [eax+gogo_thread_data_s.psywork]	; ebx = wsamp_s
		lea			ecx, [eax+gogo_thread_data_s.energy_s]	; ecx = energy_s
.lp.b:
		movaps		xm0, [ebx]
		mulps		xm0, xm0
		movaps		[ecx], xm0
		mov			eax, 1*4			; j
		mov			edx, (BLKSIZE_s-1)*4	; BLKSIZE_s - j

	; BLKSIZE_s/2回ループ
.lp.j:
		movaps		xm0, [ebx+eax*4]	; re
		movaps		xm1, [ebx+edx*4]	; im
		movaps		xm2, [ebx+eax*4+16]	; re'
		movaps		xm3, [ebx+edx*4-16]	; im'
		add			eax, byte 8
		sub			edx, byte 8
		mulps		xm0, xm0
		mulps		xm1, xm1
		mulps		xm2, xm2
		mulps		xm3, xm3
		addps		xm0, xm1
		addps		xm2, xm3
		cmp			eax, (BLKSIZE_s/2+1)*4
		mulps		xm0, xm7
		mulps		xm2, xm7
		movaps		[ecx+eax*4-32], xm0
		movaps		[ecx+eax*4-16], xm2
		jne			.lp.j

		add			ebx, BLKSIZE_s*4*4
		add			ecx, HBLKSIZE_s*4*4
		dec			esi
		jnz			.lp.b
.exit:
		pop			esi
		pop			ebx
		ret

;	2001/08/24 shigeo
;	19Kclk@PIII
;	convolute_energy_SSEが10.5Kclk * 4だから2倍以上の速度アップ
;	2001/08/25
;	25Kclk@PIII (後半の log ルーチン追加)

;void inner_psy_sub3( float *eb, float *cb, float *thr );

%define eb			BASE+4
%define cb			BASE+8
%define thr			BASE+12

%define BASE		esp+_P
%assign LOCAL_SIZE	16*2+16
%assign _P 4*4+LOCAL_SIZE
%define tonal0@		edx
%define tonal1@		edx+16
%define tmp@		edx			;tonal0@をつかった後なので同じ領域

proc	inner_psy_sub3_SSE
		push		ebx
		push		esi
		push		edi
		push		ebp
		sub			esp, byte LOCAL_SIZE
		lea			edx, [esp+15]
		cvtpi2ps	xm6, [RW.blocktype_old]	; mm0 = [old1:old0]
		movlps		xm0, [Q_fSHORT_TYPE]
		and			edx, ~15
		cmpeqps		xm6, xm0				; xm6 = (type == SHORT) ? -1 : 0
		xor			ebx, ebx				; ebx = b = 0;
		andps		xm6, [Q_fMAX]

		mov			ebp, RO.s3_l			; ebp = &RO.s3_l[b]
		movlhps		xm6, xm6				; xm6 = (type == SHORT) ? fMAX : 0
		mov			esi, [eb]				; esi = eb
		mov			edi, [cb]				; edi = cb
		movaps		xm5, [LIMIT_L]			; xm5 = LIMIT_L
.lp.b:
		mov			eax, [RO.s3ind+ebx*8]	; eax = k
		mov			ecx, [RO.s3ind+ebx*8+4]	; ecx = s3ind[b][1]
		xorps		xm0, xm0				; xm0 = ecb
		xorps		xm1, xm1				; xm1 = tbb
		add			eax, eax
		add			ecx, ecx
.lp.k:
		movss		xm4, [ebp+eax*2]		; xm4 = s3	;2倍されてるので *2
		movaps		xm2, [esi+eax*8]		; xm2 = eb	;2倍されてるので *8
		movaps		xm3, [edi+eax*8]		; xm3 = cb
		shufps		xm4, xm4, 0				; xm4 = [s3:s3:s3:s3]
		add			eax, byte 2
		mulps		xm2, xm4
		mulps		xm3, xm4
		cmp			eax, ecx
		addps		xm0, xm2				; ecb
		addps		xm1, xm3				; tbb
		jbe			.lp.k

		add			ebp, 256				; 256 = CBANDS * sizeof(float)

		xorps		xm3, xm3
		movaps		xm7, xm0				; xm7 = ecb		; keep!!!
		cmpneqps	xm3, xm0				; xm3 = (ecb) ? -1 : 0;
		rcpps		xm2, xm0

		; calc tbb / ecb
		movaps		xm4, xm7				; xm4 = ecb
		andps		xm2, xm3				; xm2 = (ecb) ? 1/ecb : 0;
		mulps		xm4, xm2
		mulps		xm4, xm2
		addps		xm2, xm2
		subps		xm2, xm4				; real 1/ecb
		mulps		xm2, xm1				; tbb = tbb / ecb

		maxps		xm2, xm5
		minps		xm2, [LIMIT_U]			; clipping in (LIMIT_L, LIMIT_U)
		subps		xm2, xm5
		mulps		xm2, [Q_coef3]			; tbb = (tbb - LIMIT_L) * (tonalityTblNum / (LIMIT_U - LIMIT_L))

											; xm2 = [tbb3:tbb2:tbb1:tbb0]
		cvttps2pi	mm0, xm2				; mm0 = [(int)tbb1:(int)tbb0]
		movhlps		xm1, xm2				; xm1 = [*:*:tbb3:tbb2]
		movd		eax, mm0				; idx0
		cvttps2pi	mm1, xm1				; mm1 = [(int)tbb3:(int)tbb2]
		mov			ecx, [tonalityTbl+eax*8+0]
		mov			eax, [tonalityTbl+eax*8+4]
		cvtpi2ps	xm1, mm0
		psrlq		mm0, 32
		mov			[tonal0@+ 0], ecx
		movd		ecx, mm0				; idx1
		mov			[tonal1@+ 0], eax
		mov			eax, [tonalityTbl+ecx*8+0]
		cvtpi2ps	xm3, mm1
		mov			ecx, [tonalityTbl+ecx*8+4]
		movlhps		xm1, xm3				; xm1 = (int)[tbb3:tbb2:tbb1:tbb0]
		mov			[tonal0@+ 4], eax
		movd		eax, mm1				; idx2
		mov			[tonal1@+ 4], ecx
		mov			ecx, [tonalityTbl+eax*8+0]
		subps		xm2, xm1				; tbb - idx
		mov			eax, [tonalityTbl+eax*8+4]
		psrlq		mm1, 32
		mov			[tonal0@+ 8], ecx
		movd		ecx, mm1				; idx3
		mov			[tonal1@+ 8], eax
		mov			eax, [tonalityTbl+ecx*8+0]
		mov			ecx, [tonalityTbl+ecx*8+4]
		mov			[tonal0@+12], eax
		mov			[tonal1@+12], ecx
		; メモリのペナルティ重そう...SSE2使いたい。
		movss		xm1, [RO.minval+ebx*4]
		mulps		xm2, [tonal1@]
		shufps		xm1, xm1, 0
		addps		xm2, [tonal0@]			; xm2 = tbb

		mov			eax, [thr]
		lea			ecx, [ebx*4]			; ecx = b*4
		minps		xm2, xm1				; tbb = Min(RO.minval, tbb)
		movaps		xm1, [RW.nb_12+ecx*8]	; xm1 = nb_12[(b*2)*4]
		mulps		xm7, xm2				; ecb *= tbb

		movaps		xm0, [RW.nb_12+ecx*8+16]; xm0 = nb_12[(b*2+1)*4]
		mulps		xm0, [Q_f8]
		minps		xm0, xm1				; min(nb_12[(b*2)*4], 8*nb_12[(b*2+1)*4])
		addps		xm0, xm0				; min(2*nb_12[(b*2)*4], 16*nb_12[(b*2+1)*4])
		maxps		xm0, xm6				; (type == SHORT) ? fMAX : org
		minps		xm0, xm7				; (type == SHORT) ? ecb : min(ecb, org)
		movaps		[eax+ecx*4], xm0		; thr
		movaps		[RW.nb_12+ecx*8+16], xm1; nb_12[(b*2+1)*4] <= nb_12[(b*2)*4]
		movaps		[RW.nb_12+ecx*8], xm7	; ecb
		inc			ebx
		cmp			ebx, [RO.npart_l]
		jne			near .lp.b

		; log 部分後半
		xor			ebx, ebx				; ebx = b*4
		mov			eax, RW.ATH+ath_t.cb	; eax = ath
		mov			ecx, [RO.npart_l]
		mov			edi, RO.numlines_l		; edi = num
		shl			ecx, 2					; ecx = RO.npart_l*4
		mov			esi, [thr]				; esi = thr
		mov			ebp, [eb]				; ebp = eb
		xorps		xm6, xm6				; xm6 = RW.pe
		; fix data
		movq		mm4, [Q_i0x007FFFFF]
		movq		mm5, [Q_i127]
		movq		mm6, [Q_f1]				; 0x3F800000
		movaps		xm5, [Q_fSQRT2]
		movaps		xm7, [Q_f1]
.lp.b2:
		movss		xm0, [eax+ebx]
		shufps		xm0, xm0, 0
		movaps		xm1, [ebp+ebx*4]		; xm1 = eb
		maxps		xm0, [esi+ebx*4]		; xm0 = th
		rcpps		xm3, xm1				; eb = 0でも大丈夫か?(大丈夫だった)
		mulps		xm1, xm3
		mulps		xm1, xm3
		addps		xm3, xm3
		subps		xm3, xm1				; xm3 = 1/eb
		mulps		xm0, xm3				; xm0 = th = th/eb
		minps		xm0, xm7				; xm0 = min(th, 1)	;無限大はここで消える

		;ここから log ルーチン
		; input  xm0
		; output xm0
		; use    xm0, ..., xm4, mm0, .., mm3
		; mm4 = 0x007FFFFF
		; mm5 = 127
		; mm6 = 1.0
		; xm5 = Q_fSQRT2
		movaps		[tmp@], xm0
		movq		mm0, [tmp@+0]
		movq		mm1, [tmp@+8]
		movq		mm2, mm0
		movq		mm3, mm1
		psrld		mm0, 23
		psrld		mm1, 23
		pand		mm2, mm4				; [Q_i0x007FFFFF]
		pand		mm3, mm4
		psubd		mm0, mm5				; [b1:b0]
		psubd		mm1, mm5				; [b3:b2]
		por			mm2, mm6				; 0x3F800000 = 1.0
		por			mm3, mm6
		movq		[tmp@+0], mm2			; [a3:a2:a1:a0]
		movq		[tmp@+8], mm3
		movaps		xm0, [tmp@]
		cvtpi2ps	xm3, mm0				; xm3 = [b1:b0]
		movaps		xm1, xm0
		subps		xm0, xm5				; a - SQRT2
		cvtpi2ps	xm4, mm1				; xm4 = [b3:b2]
		addps		xm1, xm5				; a + SQRT2
		movlhps		xm3, xm4				; xm3 = [b3:b2:b1:b0]
		rcpps		xm2, xm1
		addps		xm3, [Q_f0p5]			; xm3 = b+0.5
		mulps		xm1, xm2
		mulps		xm3, [Q_fLOG_E_2]
		mulps		xm1, xm2
		addps		xm2, xm2
		subps		xm2, xm1
		mulps		xm0, xm2				; xm0 = z
		movaps		xm1, xm0
		mulps		xm1, xm0				; xm1 = zz
		movaps		xm2, xm1				; xm2 = zz
		mulps		xm1, [Q_fC2]
		; mumはint型
	cvtsi2ss		xm4, [edi+ebx]			; xm4 = (float)num
		addps		xm1, [Q_fB2]
		mulps		xm1, xm2
		addps		xm1, [Q_fA2]
	shufps		xm4, xm4, 0
		mulps		xm0, xm1
		addps		xm0, xm3				; log(z)
	; log 演算終わり
		add			ebx, byte 4
		mulps		xm0, xm4
		subps		xm6, xm0				; RW.pe -= num * log(th)
		cmp			ebx, ecx
		jne			near .lp.b2
		movaps		[RW.pe], xm6
.exit:
		add			esp, byte LOCAL_SIZE
		emms
		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		ret


;	2001/08/26 shigeo
;	40Kclk@P4 from SSE なぜにこんなに遅い???
;	27Kclk

proc	inner_psy_sub3_SSE2
		push		ebx
		push		esi
		push		edi
		push		ebp
		sub			esp, byte LOCAL_SIZE
		lea			edx, [esp+15]
		cvtpi2ps	xm6, [RW.blocktype_old]	; xm6 = [old1:old0]
		movlps		xm0, [Q_fSHORT_TYPE]
		and			edx, ~15
		cmpeqps		xm6, xm0				; xm6 = (type == SHORT) ? -1 : 0
		xor			ebx, ebx				; ebx = b = 0;
		andps		xm6, [Q_fMAX]

		mov			ebp, RO.s3_l			; ebp = &RO.s3_l[b]
		movlhps		xm6, xm6				; xm6 = (type == SHORT) ? fMAX : 0
		mov			esi, [eb]				; esi = eb
		mov			edi, [cb]				; edi = cb
		movaps		xm5, [LIMIT_L]			; xm5 = LIMIT_L
.lp.b:
		mov			eax, [RO.s3ind+ebx*8]	; eax = k
		mov			ecx, [RO.s3ind+ebx*8+4]	; ecx = s3ind[b][1]
		xorps		xm0, xm0				; xm0 = ecb
		xorps		xm1, xm1				; xm1 = tbb
		add			eax, eax
		add			ecx, ecx
.lp.k:
		movss		xm4, [ebp+eax*2]		; xm4 = s3	;2倍されてるので *2
		movaps		xm2, [esi+eax*8]		; xm2 = eb	;2倍されてるので *8
		movaps		xm3, [edi+eax*8]		; xm3 = cb
		shufps		xm4, xm4, 0				; xm4 = [s3:s3:s3:s3]
		add			eax, byte 2
		mulps		xm2, xm4
		mulps		xm3, xm4
		cmp			eax, ecx
		addps		xm0, xm2				; ecb
		addps		xm1, xm3				; tbb
		jbe			.lp.k

		add			ebp, 256				; 256 = CBANDS * sizeof(float)

		xorps		xm3, xm3
		movaps		xm7, xm0				; xm7 = ecb		; keep!!!
		cmpneqps	xm3, xm0				; xm3 = (ecb) ? -1 : 0;
		rcpps		xm2, xm0

		; calc tbb / ecb
		movaps		xm4, xm7				; xm4 = ecb
		andps		xm2, xm3				; xm2 = (ecb) ? 1/ecb : 0;
		mulps		xm4, xm2
		mulps		xm4, xm2
		addps		xm2, xm2
		subps		xm2, xm4				; real 1/ecb
		mulps		xm2, xm1				; tbb = tbb / ecb

		maxps		xm2, xm5
		minps		xm2, [LIMIT_U]			; clipping in (LIMIT_L, LIMIT_U)
		subps		xm2, xm5
		mulps		xm2, [Q_coef3]			; tbb = (tbb - LIMIT_L) * (tonalityTblNum / (LIMIT_U - LIMIT_L))

											; xm2 = [tbb3:tbb2:tbb1:tbb0]
		mov			ecx, tonalityTbl
		cvttps2dq	xm0, xm2				; xm0 = [idx3:idx2:idx1:idx0]
		cvtdq2ps	xm1, xm0
		subps		xm2, xm1				; xm2 = tbb - idx

		; pinsrw, pextrw は 16bit移動しかできないので使えない(最低)

		pshufd		xm0, xm0, PACK(0,1,2,3)	; xm0 = [idx0:idx1:idx2:idx3]
		movd		eax, xm0
		psrldq		xm0, 4					; xm0 = [   0:idx0:idx1:idx2]
		movss		xm1, [ecx+eax*8+0]		; 上位bit0 clear
		movss		xm3, [ecx+eax*8+4]
		pslldq		xm1, 4
		pslldq		xm3, 4

		movd		eax, xm0
		psrldq		xm0, 4					; xm0 = [   0:   0:idx0:idx1]
		movss		xm4, [ecx+eax*8+0]

		por			xm1, xm4				; movssより速い
		movss		xm4, [ecx+eax*8+4]
		por			xm3, xm4
		pslldq		xm1, 4
		pslldq		xm3, 4

		movd		eax, xm0
		psrldq		xm0, 4					; xm0 = [   0:   0:   0:idx0]
		movss		xm4, [ecx+eax*8+0]
		por			xm1, xm4
		movss		xm4, [ecx+eax*8+4]
		por			xm3, xm4
		pslldq		xm1, 4
		pslldq		xm3, 4

		movd		eax, xm0
		movss		xm4, [ecx+eax*8+0]
		por			xm1, xm4
		movss		xm4, [ecx+eax*8+4]
		por			xm3, xm4

		mulps		xm2, xm3
		addps		xm2, xm1
		movss		xm1, [RO.minval+ebx*4]
		shufps		xm1, xm1, 0

		mov			eax, [thr]
		lea			ecx, [ebx*4]			; ecx = b*4
		add			ebx, byte 1
		cmp			ebx, [RO.npart_l]
		minps		xm2, xm1				; tbb = Min(RO.minval, tbb)
		movaps		xm1, [RW.nb_12+ecx*8]	; xm1 = nb_12[(b*2)*4]
		mulps		xm7, xm2				; ecb *= tbb

		movaps		xm0, [RW.nb_12+ecx*8+16]; xm0 = nb_12[(b*2+1)*4]
		mulps		xm0, [Q_f8]
		minps		xm0, xm1				; min(nb_12[(b*2)*4], 8*nb_12[(b*2+1)*4])
		addps		xm0, xm0				; min(2*nb_12[(b*2)*4], 16*nb_12[(b*2+1)*4])
		maxps		xm0, xm6				; (type == SHORT) ? fMAX : org
		minps		xm0, xm7				; (type == SHORT) ? ecb : min(ecb, org)
		movaps		[eax+ecx*4], xm0		; thr
		movaps		[RW.nb_12+ecx*8+16], xm1; nb_12[(b*2+1)*4] <= nb_12[(b*2)*4]
		movaps		[RW.nb_12+ecx*8], xm7	; ecb
		jne			near .lp.b

		; log 部分後半
		xor			ebx, ebx				; ebx = b*4
		mov			eax, RW.ATH+ath_t.cb	; eax = ath
		mov			ecx, [RO.npart_l]
		mov			edi, RO.numlines_l		; edi = num
		shl			ecx, 2					; ecx = RO.npart_l*4
		mov			esi, [thr]				; esi = thr
		mov			ebp, [eb]				; ebp = eb
		xorps		xm6, xm6				; xm6 = RW.pe
		; fix data
		movaps		xm4, [Q_fC2]
		movaps		xm5, [Q_fSQRT2]
		movaps		xm7, [Q_f1]
.lp.b2:
		movss		xm0, [eax+ebx]
		shufps		xm0, xm0, 0
		movaps		xm1, [ebp+ebx*4]		; xm1 = eb
		maxps		xm0, [esi+ebx*4]		; xm0 = th
		rcpps		xm3, xm1				; eb = 0でも大丈夫か?(大丈夫だった)
		mulps		xm1, xm3
		mulps		xm1, xm3
		addps		xm3, xm3
		subps		xm3, xm1				; xm3 = 1/eb
		mulps		xm0, xm3				; xm0 = th = th/eb
		minps		xm0, xm7				; xm0 = min(th, 1)	;無限大はここで消える

		; ここから log ルーチン
		; こっちはSSE2のつぼにはまった
		; input  xm0
		; output xm0
		; use    xm0, ..., xm4, mm0, .., mm3
		; xm4 = Q_fC2
		; xm5 = Q_fSQRT2

		movaps		xm3, xm0
		pand		xm0, [Q_i0x007FFFFF]
		psrld		xm3, 23
		por			xm0, [Q_f1]				; a
		psubd		xm3, [Q_i127]
		movaps		xm1, xm0
		subps		xm0, xm5				; a - SQRT2
		cvtdq2ps	xm3, xm3				; (float)b
		addps		xm1, xm5				; a + SQRT2

		rcpps		xm2, xm1
		addps		xm3, [Q_f0p5]			; xm3 = b+0.5
		mulps		xm1, xm2
		mulps		xm3, [Q_fLOG_E_2]
		mulps		xm1, xm2
		addps		xm2, xm2
		subps		xm2, xm1
		mulps		xm0, xm2				; xm0 = z
		movaps		xm1, xm0
		mulps		xm1, xm0				; xm1 = zz
		movaps		xm2, xm1				; xm2 = zz
		mulps		xm1, xm4				; [Q_fC2]
		; mumはint型
		addps		xm1, [Q_fB2]
		mulps		xm1, xm2
	cvtsi2ss		xm2, [edi+ebx]			; xm2 = (float)num
		addps		xm1, [Q_fA2]
	shufps		xm2, xm2, 0
		mulps		xm0, xm1
		addps		xm0, xm3				; log(z)
	; log 演算終わり
		add			ebx, byte 4
		mulps		xm0, xm2
		subps		xm6, xm0				; RW.pe -= num * log(th)
		cmp			ebx, ecx
		jne			near .lp.b2
		movaps		[RW.pe], xm6
.exit:
		add			esp, byte LOCAL_SIZE
		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		ret

proc	inner_psy_sub3_SSE2_P4
		push		ebx
		push		esi
		push		edi
		push		ebp
		sub			esp, byte LOCAL_SIZE
		mov			edx, esp
		add			edx, 15
		and			edx, ~15				;edxはスタック内のアライメントを取った領域

		xor			ebx, ebx				; ebx = b = 0;

		mov			ebp, RO.s3_l			; ebp = &RO.s3_l[b]
		mov			esi, [eb]				; esi = &eb[0]
		mov			edi, [cb]				; edi = &cb[0]

		movaps		xm5, [LIMIT_L]			; xm5 = LIMIT_L

		cvtpi2ps	xm6, [RW.blocktype_old]	; xm6 = [old1:old0]
		cmpeqps		xm6, [Q_fSHORT_TYPE]	; xm6 = (type == SHORT) ? -1 : 0
		andps		xm6, [Q_fMAX]
		movlhps		xm6, xm6				; xm6 = (type == SHORT) ? fMAX : 0
.lp.b.init:
		mov			eax, [RO.s3ind+ebx*8]	; eax = s3ind[b][0] = k
		mov			ecx, [RO.s3ind+ebx*8+4]	; ecx = s3ind[b][1]
		xorps		xm0, xm0				; xm0 = ecb
		xorps		xm1, xm1				; xm1 = tbb
		sub			ecx, eax				; ecx = loop_count-1
		add			eax, eax				; eax*2
		jmp			.lp.k
		align 16
.lp.b.loop:
.lp.k:
		movss		xm4, [ebp+eax*2]		; xm4 = s3_l[b][k]
		shufps		xm4, xm4, 0				; xm4 = [s3:s3:s3:s3]
		movaps		xm2, [esi+eax*8]		; xm2 = eb[k*4+0]
		mulps		xm2, xm4
		movaps		xm3, [edi+eax*8]		; xm3 = cb[k*4+0]
		mulps		xm3, xm4
		add			eax, 2					; k++
		addps		xm0, xm2				; ecb
		addps		xm1, xm3				; tbb
		sub			ecx, 1
		jge			.lp.k

		add			ebp, 256				; 256 = CBANDS * sizeof(float) ;s3_l[b+1][k]

		movaps		xm7, xm0				; xm7 = ecb		; keep!!!

		; calc tbb / ecb
		rcpps		xm2, xm0
		mulps		xm0, xm2
		mulps		xm2, xm1				; tbb = (ecb) ? tbb/approx.(ecb) : inf
		movaps		xm4, [Q_f2]
		subps		xm4, xm0
		mulps		xm2, xm4				; tbb = (ecb) ? tbb/ecb : QnaN

		maxps		xm2, xm5
		minps		xm2, [LIMIT_U]			; clipping in (LIMIT_L, LIMIT_U)
		subps		xm2, xm5
		mulps		xm2, [Q_coef3]			; tbb = (tbb - LIMIT_L) * (tonalityTblNum / (LIMIT_U - LIMIT_L))

		cvttps2dq	xm0, xm2				; xm0 = [idx3:idx2:idx1:idx0]
		cvtdq2ps	xm1, xm0
		subps		xm2, xm1				; xm2 = tbb - idx

		pextrw		eax, xm0, 0
		pextrw		ecx, xm0, 2
		movd		xm3, [tonalityTbl+eax*8+4]
		movd		xm4, [tonalityTbl+ecx*8+4]
		punpckldq	xm3, xm4
		movd		xm1, [tonalityTbl+eax*8+0]
		movd		xm4, [tonalityTbl+ecx*8+0]
		punpckldq	xm1, xm4
		;
		pextrw		eax, xm0, 4
		pextrw		ecx, xm0, 6
		movd		xm0, [tonalityTbl+eax*8+4]
		movd		xm4, [tonalityTbl+ecx*8+4]
		punpckldq	xm0, xm4
		movlhps		xm3, xm0
		movd		xm0, [tonalityTbl+eax*8+0]
		movd		xm4, [tonalityTbl+ecx*8+0]
		punpckldq	xm0, xm4
		movlhps		xm1, xm0

		mulps		xm2, xm3				;tonalityTbl[idx*2+1] * (tbb2-idx)
		addps		xm2, xm1				;tonalityTbl[idx*2] + tonalityTbl[idx*2+1] * (tbb2-idx)
		movss		xm1, [RO.minval+ebx*4]
		shufps		xm1, xm1, 0
		minps		xm2, xm1				; tbb = Min(RO.minval, tbb)
		mulps		xm7, xm2				; ecb *= tbb

		lea			ecx, [ebx*4]			; ecx = b*4

		movaps		xm0, [RW.nb_12+ecx*8+16]; xm0 = nb_12[(b*2+1)*4]
		movaps		xm1, [RW.nb_12+ecx*8]	; xm1 = nb_12[(b*2)*4]
		mulps		xm0, [Q_f16]
		movaps		[RW.nb_12+ecx*8], xm7	; ecb
		movaps		[RW.nb_12+ecx*8+16], xm1; copy
		addps		xm1, xm1
		minps		xm0, xm1				; min(2*nb_12[(b*2)*4], 16*nb_12[(b*2+1)*4])
		maxps		xm0, xm6				; (type == SHORT) ? fMAX : org
		minps		xm0, xm7				; (type == SHORT) ? ecb : min(ecb, org)
		mov			eax, [thr]
		movaps		[eax+ecx*4], xm0		; thr

		add			ebx, 1
		cmp			ebx, [RO.npart_l]
		jz			.lp.b.exit

;.lp.b.next:
		mov			eax, [RO.s3ind+ebx*8]	; eax = s3ind[b][0] = k
		mov			ecx, [RO.s3ind+ebx*8+4]	; ecx = s3ind[b][1]
		xorps		xm0, xm0				; xm0 = ecb
		xorps		xm1, xm1				; xm1 = tbb
		sub			ecx, eax				; ecx = loop_count-1
		add			eax, eax				; eax*2
		jmp			near .lp.b.loop
		align 16
.lp.b.exit:

		; log 部分後半
		xor			ebx, ebx				; ebx = b*4
		mov			eax, RW.ATH+ath_t.cb	; eax = ath
		mov			ecx, [RO.npart_l]
		mov			edi, RO.numlines_l		; edi = num
		mov			esi, [thr]				; esi = thr
		mov			ebp, [eb]				; ebp = eb
		xorps		xm6, xm6				; xm6 = RW.pe
		; fix data
		movaps		xm4, [Q_fC2]
		movaps		xm5, [Q_fSQRT2]
		movaps		xm7, [Q_f1]
		jmp			.lp.b2
		align 16
.lp.b2:
		movss		xm3, [eax+ebx]			; aht[b]
		shufps		xm3, xm3, 0
		maxps		xm3, [esi+ebx*4]		; xm3 = th = Max(thr[b*4+0], ath)
		movaps		xm1, [ebp+ebx*4]		; xm1 = eb
		rcpps		xm0, xm1
		mulps		xm1, xm0
		mulps		xm0, xm3				; xm0 = th = th/approx.eb
		movaps		xm2, [Q_f2]
		subps		xm2, xm1
		mulps		xm0, xm2				; xm0 = th = (eb) ? th/eb : QnaN
		minps		xm0, xm7				; xm0 = min(th, 1)

		; ここから log ルーチン
		; こっちはSSE2のつぼにはまった
		; input  xm0
		; output xm0
		; use    xm0, ..., xm4, mm0, .., mm3
		; xm4 = Q_fC2
		; xm5 = Q_fSQRT2
		; xm7 = Q_f1

		movaps		xm3, xm0
		pand		xm0, [Q_i0x007FFFFF]
		por			xm0, xm7				; a
		movaps		xm1, xm0
		subps		xm0, xm5				; a - SQRT2
		addps		xm1, xm5				; a + SQRT2

		psrld		xm3, 23
		psubd		xm3, [Q_i127]			; b
		cvtdq2ps	xm3, xm3				; (float)b
		addps		xm3, [Q_f0p5]			; xm3 = b+0.5
		mulps		xm3, [Q_fLOG_E_2]

		rcpps		xm2, xm1
		mulps		xm1, xm2
		mulps		xm0, xm2				; xm0 = approx.z
		movaps		xm2, [Q_f2]
		subps		xm2, xm1
		mulps		xm0, xm2				; xm0 = z

		movaps		xm2, xm0
		mulps		xm2, xm0				; xm2 = zz
		movaps		xm1, xm4				; [Q_fC2]
		mulps		xm1, xm2
		addps		xm1, [Q_fB2]
		mulps		xm1, xm2
		addps		xm1, [Q_fA2]
		mulps		xm0, xm1
		addps		xm0, xm3				; log(z)
		; log 演算終わり

		cvtsi2ss	xm3, [edi+ebx]			; xm2 = (float)num
		shufps		xm3, xm3, 0
		mulps		xm0, xm3
		subps		xm6, xm0				; RW.pe -= num * log(th)
		add			ebx, 4
		sub			ecx, 1
		jnz			near .lp.b2

		movaps		[RW.pe], xm6
.exit:
		add			esp, byte LOCAL_SIZE
		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		ret

;	2001/09/02 shigeo, 16Kclk@PIII first version
;	2001/09/04 bug fix
;	2002/04/08 sakai, 10Kclk@PIII 書き換えたけどいらんような...
;	2002/04/09 sakai, 9Kclk@PIII 自分で書いてたのを忘れてる(2002年のいつぞや)。わからん。

%assign _P 4*3
proc	inner_psy_sub4_SSE
		push		ebx
		push		esi
		push		edi

		mov			esi, [esp+_P+4]				; esi = tl
		mov			ebx, 2*4*4					; ebx = k*4
		lea			eax, [esi+gogo_thread_data_s.energy_s]	; eax = energy_s
		mov			edx, (BLKSIZE_s-2)*4*4		; edx = (BLKSIZE_s-k)*4
		lea			esi, [esi+gogo_thread_data_s.psywork]	; esi = wsamp_s
		mov			ecx, [RO.cw_upper_index]
		sub			ecx, cw_lower_index			; j = cw_lower_index
		shr			ecx,2
		mov			edi, RW.cw+(cw_lower_index*4)*4	; edi = &RW.cw[j*4]
		jmp			short .lp.j

		align 16
.lp.j:
		movaps		xmm2,[eax+ebx]			; xm2 = E1
		movaps		xmm0,[eax+ebx+(HBLKSIZE_s*4)*2*4]	; xm0 = E2
%if 0
		xorps		xm4, xm4
		sqrtps		xmm3,xmm2					; xm3 = r = sqrt(E1)
		sqrtps		xmm5,xmm0					; xm5 = tmp = sqrt(E2)
%else
		movaps		xmm4,[Q_f3]
		movaps		xmm1,[Q_fm0p5]
		rsqrtps		xmm3,xmm2
		rsqrtps		xmm5,xmm0
		movaps		xmm7,xmm3
		movaps		xmm6,xmm5
		mulps		xmm7,xmm2
		mulps		xmm6,xmm0
		mulps		xmm3,xmm7
		mulps		xmm5,xmm6
		subps		xmm3,xmm4
		subps		xmm5,xmm4
		xorps		xm4, xm4
		mulps		xmm3,xmm7
		mulps		xmm5,xmm6
		mulps		xmm3,xmm1
		mulps		xmm5,xmm1
%endif
		cmpneqps	xm4, xm2					; xm4 = (E1)?-1:0
		movaps		xm7, [esi+edx]			; xm7 = b
		movaps		xm1, [esi+ebx]			; xm1 = a
		andps		xm2, xm4
		andps		xm7, xm4
		andps		xm1, xm4
		andnps		xmm4,[Q_f1]
		orps		xm2, xm4					; xm2 = (E1)?E1:1
		orps		xm7, xm4					; xm7 = (E1)?b:1;
		orps		xm1, xm4					; xm1 = (E1)?a:1;

		movaps		xm4, xm1
		mulps		xm1, xm7					; xm1 = R
		mulps		xm4, xm4					; a*a
		mulps		xm7, xm7					; b*b
		subps		xm4, xm7
		mulps		xm4, [Q_f0p5]				; xm4 = I

		movaps		[edi],xmm2
; xmm3:r, xmm4:I, xmm1:R, xmm2:E1, xmm5:sqrt(E2), xmm0:E2
		addps		xmm3,xmm3					; xm3 = r*2
		xorps		xmm7,xmm7
		cmpneqps	xmm7,xmm0							; xm7 = (E2)?-1:0
		subps		xmm3,xmm5					; xm3 = r = r*2-tmp
		movaps		xmm2,[esi+ebx+(BLKSIZE_s*4)*2*4]	; xm2 = a
		movaps		xmm6,[esi+edx+(BLKSIZE_s*4)*2*4]	; xm6 = b

		andps		xmm0,xmm7
		andps		xmm5,xmm7
		andps		xmm2,xmm7
		andps		xmm6,xmm7
		andnps		xmm7,[Q_f1]
		orps		xmm0,xmm7			; = E2 = (E2)? E2: 1.0
		orps		xmm5,xmm7			; = tmp = (E2)? tmp: 1.0
		orps		xmm2,xmm7			; = a = (E2)? a: 1.0
		orps		xmm6,xmm7			; = b = (E2)? b: 1.0

		movaps		xmm7,xmm4			; = I
		mulps		xmm4,xmm2			; xmm4 = a*I
		mulps		xmm2,xmm1			; xmm2 = a*R
		mulps		xmm1,xmm6			; xmm1 = b*R
		mulps		xmm6,xmm7			; xmm6 = b*I
		addps		xmm4,xmm1			; xmm4 = I = a*I + b*R
		subps		xmm2,xmm6			; xmm2 = R = a*R - b*I
; xmm1, xmm7, xmm6 空き

		movaps		xmm6,[edi]
		mulps		xmm5,xmm6			; = den

%if 0
		sqrtps		xmm7,[eax+ebx+(HBLKSIZE_s*4)*1*4]	; xm7 = sqrt(E3)
%else
		movaps		xmm1,[eax+ebx+(HBLKSIZE_s*4)*1*4]	; xm7 = sqrt(E3)
		rsqrtps		xmm7,xmm1
		mulps		xmm1,xmm7
		mulps		xmm7,xmm1
		subps		xmm7,[Q_f3]
		mulps		xmm7,xmm1
		mulps		xmm7,[Q_fm0p5]
%endif
%if 0
		movaps		xmm1,xmm3					; xm1 = r
		divps		xmm1,xmm5					; xm1 = r = r/den
%else
		rcpps		xmm1,xmm5
		mulps		xmm5,xmm1
		mulps		xmm5,xmm1
		addps		xmm1,xmm1
		subps		xmm1,xmm5
		mulps		xmm1,xmm3
%endif
		andps		xmm3,[Q_ABS]
		addps		xmm3,xmm7					; xm3 = tmp

		mulps		xm4,[esi+ebx+(BLKSIZE_s*4)*1*4]	; xm4 = a*I
		mulps		xm2,[esi+edx+(BLKSIZE_s*4)*1*4]	; xm2 = b*R
		addps		xm2, xm4					; xm2 = a*I + b*R
		mulps		xm2, xm1					; xm2 = r*(a*I + b*R)
		mulps		xmm1,xmm6			; r*E1
		mulps		xm1, xm1			; (r*E1)*(r*E1)
		mulps		xmm1,xmm0					; (r*E1)*(r*E1)*E2
		subps		xm1, xm2			; (r*E1) * (r*E1) * E2 - r * (a*I + b*R);
		addps		xm1, [eax+ebx+(HBLKSIZE_s*4)*1*4]	; E3
; xmm0, xmm2, xmm4-7 空き

		xorps		xm7, xm7
		movaps		xm6, xm3			; xm6 = tmp
		cmpneqps	xm6, xm7			; xm6 = (tmp)?-1:0
		cmpltps		xm7, xm1			; xm7 = (0<xm1)?-1:0
		andps		xmm6,xmm7			; xmm6 = (求解可能)? -1: 0;

		andps		xmm1,xmm6			; xmm1 = (求解可能>0)?E3:0
		andps		xmm3,xmm6
		andnps		xmm6,[Q_f1]
		orps		xmm3,xmm6			; xmm3 = (求解可能)?tmp:1;
%if 0
		sqrtps		xm1, xm1					; xm1 = sqrt(E3)
%else
		rsqrtps		xmm2,xmm1
		mulps		xmm1,xmm2
		mulps		xmm2,xmm1
		subps		xmm2,[Q_f3]
		mulps		xmm1,xmm2
		mulps		xmm1,[Q_fm0p5]
%endif
		rcpps		xm0, xm3
		mulps		xm3, xm0
		mulps		xm3, xm0
		addps		xm0, xm0
		subps		xm0, xm3

		mulps		xm0, xm1					; tmp = sqrt(E3) / tmp
		movaps		[edi+16*0], xm0
		movaps		[edi+16*1], xm0
		movaps		[edi+16*2], xm0
		movaps		[edi+16*3], xm0

		add			ebx, byte 16					; (k*4)++
		sub			edx, byte 16					; ((BLKSIZE_s-k)*4)--
		add			edi, byte 16*4
		dec			ecx
		jnz			near .lp.j
.exit:
		pop			edi
		pop			esi
		pop			ebx
		ret

%assign _P 4*4

proc	inner_psy_sub4_SSE_P4
		push		ebx
		push		esi
		push		edi
		push		ebp

		mov			esi, [esp+_P+4]							; esi = tl
		lea			eax, [esi+gogo_thread_data_s.energy_s]	; eax = energy_s
		lea			esi, [esi+gogo_thread_data_s.psywork]	; esi = wsamp_s

		;j == (cw_lower_index<=j<RO.cw_upper_index) == (6<=j<RO.cw_upper_index); j+=4
		;k == (j+2)/4 == (2<=k<RO.cw_upper_index); k+=1
		mov			ebx, 2*4								; ebx = k*4
		mov			edx, (BLKSIZE_s-2)*4					; edx = (BLKSIZE_s-k)*4
		mov			edi, RW.cw+(cw_lower_index*4)*4			; edi = &RW.cw[j*4]
		mov			ecx, [RO.cw_upper_index]
		sub			ecx, cw_lower_index						; ecx = loopcount for j
		loopalign
.lp.j:
		movaps		xm0, [eax+ebx*4]					; E1
		movaps		xm1, [eax+ebx*4+(HBLKSIZE_s*4)*2*4]	; E2

		;sqrt & 1/sqrt
		movaps		xm6, xm0
		movaps		xm7, xm1
		rsqrtps		xm2, xm0
		rsqrtps		xm3, xm1
		movaps		xm4, [Q_fm0p5]
		mulps		xm0, xm4
		mulps		xm1, xm4
		movaps		xm4, xm2					; approx.1/sqrt
		movaps		xm5, xm3
		mulps		xm2, xm2
		mulps		xm3, xm3
		mulps		xm0, xm2
		mulps		xm1, xm3
		movaps		xm2, [Q_f1p5]
		addps		xm0, xm2
		addps		xm1, xm2
		mulps		xm0, xm4					; (x) ? 1/sqrt : QnaN
		mulps		xm1, xm5
		mulps		xm6, xm0					; (x) ? sqrt : QnaN
		mulps		xm7, xm1

		;clip
		movaps		xm2, [Q_f1]
		cmpordps	xm4, xm0					; (x) ? -1 : 0
		cmpordps	xm5, xm1
		xorps		xm0, xm2
		xorps		xm1, xm2
		andps		xm0, xm4
		andps		xm1, xm5
		xorps		xm0, xm2					; (x) ? 1/sqrt(x) : 1
		xorps		xm1, xm2
		movaps		xm3, [Q_0]
		maxps		xm6, xm3					; (x) ? sqrt(x) : 0
		maxps		xm7, xm3

		mulps		xm0, xm0
		mulps		xm0, xm1					; rsq
		addps		xm6, xm6
		subps		xm6, xm7					; tmp
		mulps		xm0, xm6					; r

		; xm0 = r
		; xm3 = 0.0
		; xm6 = tmp
		movaps		xm2, [eax+ebx*4+(HBLKSIZE_s*4)*1*4]	; E3
		movaps		xm1, [Q_ABS]
		andps		xm1, xm6					; fabs(tmp)
		mulps		xm6, xm6					; tmp^2
		addps		xm6, xm2					; rr

		; sqrt(E3)
		; <in>
		; xm2 = E3
		; xm3 = 0.0
		; <keep>
		; xm0 = r
		; xm1 = fabs(tmp)
		; xm3 = 0.0
		; xm6 = rr
		rsqrtps		xm5, xm2
		movaps		xm4, xm2
		mulps		xm2, [Q_fm0p5]
		mulps		xm4, xm5					; approx.sqrt
		mulps		xm5, xm5
		mulps		xm2, xm5
		addps		xm2, [Q_f1p5]
		mulps		xm2, xm4					; sqrt : QnaN
		maxps		xm2, xm3					; sqrt

		addps		xm2, xm1					; rrr

		; 1 / rrr
		; <in>
		; xm2 = rrr
		; xm3 = 0.0
		; <keep>
		; xm0 = r
		; xm3 = 0.0
		; xm6 = rr
		rcpps		xm1, xm2
		mulps		xm2, xm1
		movaps		xm4, [Q_f2]
		subps		xm4, xm2
		mulps		xm1, xm4					; 1/rrr : QnaN
		maxps		xm1, xm3					; 1/rrr

		; xm0 = r
		; xm1 = 1/rrr
		; xm3 = 0.0
		; xm6 = rr
		movaps		xm4, [esi+ebx*4]			; a
		movaps		xm5, [esi+edx*4]			; b

		movaps		xm2, [eax+ebx*4]			; E1
		cmpneqps	xm2, xm3					; (E1) ? -1 : 0
		andps		xm4, xm2
		andps		xm5, xm2
		andnps		xm2, [Q_f1]
		orps		xm4, xm2					; (E1) ? a : 1
		orps		xm5, xm2					; (E1) ? b : 1

		movaps		xm7, xm5
		mulps		xm5, xm4					; R = a*b
		mulps		xm4, xm4
		mulps		xm7, xm7
		subps		xm4, xm7
		mulps		xm4, [Q_f0p5]				; I = (aa-bb)/2

		; xm0 = r
		; xm1 = 1/rrr
		; xm3 = 0.0
		; xm4 = I
		; xm5 = R
		; xm6 = rr
		movaps		xm2, [esi+ebx*4+(BLKSIZE_s*4)*2*4]	; a
		movaps		xm7, [esi+edx*4+(BLKSIZE_s*4)*2*4]	; b

		cmpneqps	xm3, [eax+ebx*4+(HBLKSIZE_s*4)*2*4]	; (E2) ? -1 : 0
		andps		xm2, xm3
		andps		xm7, xm3
		andnps		xm3, [Q_f1]
		orps		xm2, xm3					; (E2) ? a : 1
		orps		xm7, xm3					; (E2) ? b : 1

		movaps		xm3, xm4					; I
		mulps		xm4, xm2					; a*I
		mulps		xm2, xm5					; a*R
		mulps		xm5, xm7					; b*R
		mulps		xm7, xm3					; b*I
		addps		xm4, xm5					; I = a*I+b*R
		subps		xm2, xm7					; R = a*R-b*I

		; xm0 = r
		; xm1 = 1/rrr
		; xm4 = I
		; xm2 = R
		; xm6 = rr
		mulps		xm4, [esi+ebx*4+(BLKSIZE_s*4)*1*4]	; a*I
		mulps		xm2, [esi+edx*4+(BLKSIZE_s*4)*1*4]	; b*R
		addps		xm2, xm4
		mulps		xm2, xm0
		subps		xm6, xm2					; rr

		; sqrt(rr) * 1/rrr
		; <in>
		; xm1 = 1/rrr
		; xm6 = rr
		rsqrtps		xm3, xm6
		mulps		xm1, xm6					; rr * 1/rrr
		mulps		xm6, [Q_fm0p5]
		mulps		xm1, xm3					; approx.sqrt * 1/rrr
		mulps		xm3, xm3
		mulps		xm6, xm3
		addps		xm6, [Q_f1p5]
		mulps		xm6, xm1					; sqrt * 1/rrr : QnaN
		maxps		xm6, [Q_0]					; sqrt * 1/rrr

		movaps		[edi+16*0], xm6
		movaps		[edi+16*1], xm6
		movaps		[edi+16*2], xm6
		movaps		[edi+16*3], xm6

		add			ebx, byte 4					; (k*4)++
		sub			edx, byte 4					; ((BLKSIZE_s-k)*4)--
		add			edi, byte 16*4
		sub			ecx, byte 4
		ja			near .lp.j
.exit:
		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		ret

;	2001/08/31 shigeo, 17Kclk@PIII とりあえず作っただけ。最適化してません。
;	2002/04/08 sakai, 11kclk@PIII なぜか edx が余ってます(^^;;

;void inner_psy_sub5( gogo_thread_data *tl, float *eb, float *thr );

%if 1
;		float		(*eb)[CBANDS*4] = tl->psywork;
;		float		(*thr)[CBANDS*4] = tl->psywork+CBANDS*4*3;

proc	inner_psy_sub5_SSE
		push		ebx
		push		esi
		push		edi
		push		ebp
%assign _P 4*4
		mov			esi, [esp+_P+4]			; esi = tl
		lea			edi, [esi+gogo_thread_data_s.psywork+0]				; edi = eb
;;
		imul		eax,[RO.npart_s_orig],4		; eax = npart_sorig*4
		add			esi,gogo_thread_data_s.energy_s	; esi = energy_s
		xor			ebx,ebx			; b
		jmp			short .lp.b0
	
		align 16
.lp.b0:	movaps		xmm0,[esi]				; xmm0
		movaps		xmm1,[esi+4*HBLKSIZE_s*4*1]
		movaps		xmm2,[esi+4*HBLKSIZE_s*4*2]
		mov			ecx, [RO.numlines_s+ebx]	; ecx = RO.numlines_s[b]
		jmp			short .f0

		align 16
.lp.i0:	addps		xmm0,[esi]		; energy
		addps		xmm1,[esi+4*HBLKSIZE_s*4*1]
		addps		xmm2,[esi+4*HBLKSIZE_s*4*2]
.f0:	add			esi,byte 16				; (j*4)++
		loop		.lp.i0
		movaps		[edi+ebx*4],xmm0		; eb[0]
		movaps		[edi+ebx*4+4*CBANDS*4*1],xmm1		; eb[1]
		movaps		[edi+ebx*4+4*CBANDS*4*2],xmm2		; eb[2]
		lea			ebx,[ebx+ 4]				; (b*4)++
		cmp			ebx, eax
		jb			.lp.b0
;;
		imul		eax,[RO.npart_s],4	; eax = npart_sorig*4
		xor			ebx,ebx			; b
		mov			esi,RO.s3_s		; 
		movaps		xmm7,[Q_f1em6]
		jmp			short .lp.b1

		align 16
.lp.b1:
		imul		ebp,[RO.s3ind_s+ebx*2+0],4
		imul		ecx,[RO.s3ind_s+ebx*2+4],4
		movss		xmm6,[esi+ebp]	; tmp
		shufps		xmm6,xmm6,0
		movaps		xmm0,[edi+ebp*4]
		movaps		xmm1,[edi+ebp*4+4*CBANDS*4*1]
		movaps		xmm2,[edi+ebp*4+4*CBANDS*4*2]
		mulps		xmm0,xmm6
		mulps		xmm1,xmm6
		mulps		xmm2,xmm6
		jmp			short .f1

		align 16
.lp.i1:	movss		xmm6,[esi+ebp]	; tmp
		shufps		xmm6,xmm6,0
		movaps		xmm3,[edi+ebp*4]
		movaps		xmm4,[edi+ebp*4+4*CBANDS*4*1]
		movaps		xmm5,[edi+ebp*4+4*CBANDS*4*2]
		mulps		xmm3,xmm6
		mulps		xmm4,xmm6
		mulps		xmm5,xmm6
		addps		xmm0,xmm3
		addps		xmm1,xmm4
		addps		xmm2,xmm5
.f1:	lea			ebp,[ebp+4]			; i++
		cmp			ebp,ecx
		jbe			.lp.i1

		movss		xmm6,[RO.SNR_s+ebx]
		shufps		xmm6,xmm6,0
		mulps		xmm0,xmm6
		mulps		xmm1,xmm6
		mulps		xmm2,xmm6
		maxps		xmm0,xmm7
		maxps		xmm1,xmm7
		maxps		xmm2,xmm7
		movaps		[edi+ebx*4+4*CBANDS*4*3],xmm0		; thr[0]
		movaps		[edi+ebx*4+4*CBANDS*4*4],xmm1		; thr[1]
		movaps		[edi+ebx*4+4*CBANDS*4*5],xmm2		; thr[2]
		lea			ebx,[ebx+ 4]				; (b*4)++
		lea			esi,[esi+ 4*CBANDS]
		cmp			ebx, eax
		jb			near .lp.b1
;;
		mov			esi,RW.en+III_psy_xmin_s.s
		xor			ebx,ebx			; b
		movaps		xmm7,[Q_f0p5]
		jmp			short .lp.b2

		align 16
.lp.b2:
		imul		ebp,[RO.bu_s+ebx],16
		imul		ecx,[RO.bo_s+ebx],16
		movaps		xmm0,[edi+ebp]
		addps		xmm0,[edi+ecx]
		movaps		xmm1,[edi+ebp+4*CBANDS*4*1]
		addps		xmm1,[edi+ecx+4*CBANDS*4*1]
		movaps		xmm2,[edi+ebp+4*CBANDS*4*2]
		addps		xmm2,[edi+ecx+4*CBANDS*4*2]
		movaps		xmm3,[edi+ebp+4*CBANDS*4*3]
		addps		xmm3,[edi+ecx+4*CBANDS*4*3]
		movaps		xmm4,[edi+ebp+4*CBANDS*4*4]
		addps		xmm4,[edi+ecx+4*CBANDS*4*4]
		movaps		xmm5,[edi+ebp+4*CBANDS*4*5]
		addps		xmm5,[edi+ecx+4*CBANDS*4*5]
		mulps		xmm0,xmm7
		mulps		xmm1,xmm7
		mulps		xmm2,xmm7
		mulps		xmm3,xmm7
		mulps		xmm4,xmm7
		mulps		xmm5,xmm7
		jmp			short .f2

		align 16
.lp.i2:
		addps		xmm0,[edi+ebp]
		addps		xmm1,[edi+ebp+4*CBANDS*4*1]
		addps		xmm2,[edi+ebp+4*CBANDS*4*2]
		addps		xmm3,[edi+ebp+4*CBANDS*4*3]
		addps		xmm4,[edi+ebp+4*CBANDS*4*4]
		addps		xmm5,[edi+ebp+4*CBANDS*4*5]
.f2:	lea			ebp,[ebp+16]			; i++
		cmp			ebp,ecx
		jb			.lp.i2

		test		esi,4
		jz			near .store.even
.store.odd:
		movaps		xmm6,xmm1
		unpcklps	xmm1,xmm2
		unpckhps	xmm6,xmm2

		movss		[esi],xmm0
		shufps		xmm0,xmm0,00111001B
		movlps		[esi+4],xmm1

		movss		[esi+sizeof_III_psy_xmin*1],xmm0
		shufps		xmm0,xmm0,00111001B
		movhps		[esi+sizeof_III_psy_xmin*1+4],xmm1

		movss		[esi+sizeof_III_psy_xmin*2],xmm0
		shufps		xmm0,xmm0,00111001B
		movlps		[esi+sizeof_III_psy_xmin*2+4],xmm6

		movss		[esi+sizeof_III_psy_xmin*3],xmm0
		movhps		[esi+sizeof_III_psy_xmin*3+4],xmm6

		movaps		xmm1,xmm4
		unpcklps	xmm4,xmm5
		unpckhps	xmm1,xmm5

		movss		[esi-rw_t.en+rw_t.thm],xmm3
		shufps		xmm3,xmm3,00111001B
		movlps		[esi-rw_t.en+rw_t.thm+4],xmm4

		movss		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*1],xmm3
		shufps		xmm3,xmm3,00111001B
		movhps		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*1+4],xmm4

		movss		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*2],xmm3
		shufps		xmm3,xmm3,00111001B
		movlps		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*2+4],xmm1

		movss		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*3],xmm3
		movhps		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*3+4],xmm1

		lea			esi,[esi+4*3]
		lea			ebx,[ebx+ 4]				; (b*4)++
		cmp			ebx, NBPSY_s*4
		jb			near .lp.b2

.store.even:
		movaps		xmm6,xmm0
		unpcklps	xmm0,xmm1
		unpckhps	xmm6,xmm1

		movlps		[esi],xmm0
		movss		[esi+8],xmm2
		shufps		xmm2,xmm2,00111001B

		movhps		[esi+sizeof_III_psy_xmin*1],xmm0
		movss		[esi+sizeof_III_psy_xmin*1+8],xmm2
		shufps		xmm2,xmm2,00111001B

		movlps		[esi+sizeof_III_psy_xmin*2],xmm6
		movss		[esi+sizeof_III_psy_xmin*2+8],xmm2
		shufps		xmm2,xmm2,00111001B

		movhps		[esi+sizeof_III_psy_xmin*3],xmm6
		movss		[esi+sizeof_III_psy_xmin*3+8],xmm2

		movaps		xmm1,xmm3
		unpcklps	xmm3,xmm4
		unpckhps	xmm1,xmm4

		movlps		[esi-rw_t.en+rw_t.thm],xmm3
		movss		[esi-rw_t.en+rw_t.thm+8],xmm5
		shufps		xmm5,xmm5,00111001B

		movhps		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*1],xmm3
		movss		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*1+8],xmm5
		shufps		xmm5,xmm5,00111001B

		movlps		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*2],xmm1
		movss		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*2+8],xmm5
		shufps		xmm5,xmm5,00111001B

		movhps		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*3],xmm1
		movss		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*3+8],xmm5

		lea			esi,[esi+4*3]
		lea			ebx,[ebx+ 4]				; (b*4)++
		cmp			ebx, NBPSY_s*4
		jb			near .lp.b2
;;
		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		ret
%else
%assign LOCAL_SIZE	12
%define k@				esp
%define npart_s_origX4@	esp+4
%define npart_sX4@		esp+8


proc	inner_psy_sub5_SSE
		push		ebx
		push		esi
		push		edi
		push		ebp
		sub			esp, byte LOCAL_SIZE
%assign _P 4*4+LOCAL_SIZE
		mov			[k@], dword 0
		mov			eax, [RO.npart_s_orig]
		mov			ebx, [RO.npart_s]
		shl			eax, 2
		shl			ebx, 2
		mov			[npart_s_origX4@], eax
		mov			[npart_sX4@], ebx
		movaps		xm7, [Q_f1em6]
		movaps		xm6, [Q_f0p5]
		mov			esi, [esp+_P+4]			; esi = tl
		mov			edi, [esp+_P+8]			; edi = eb
		lea			esi, [esi+gogo_thread_data_s.energy_s]	;esi = &energy_s[k][0]
.lp.k:
		mov			ecx, [npart_s_origX4@]
		xor			edx, edx				; edx = j*4
		xor			ebx, ebx				; ebx = b*4
.lp.b0:
		xorps		xm0, xm0				; xm0 = ecb
		mov			ebp, [RO.numlines_s+ebx]	; ebp = i
.lp.i0:
		movaps		xm1, [esi+edx*4]		; energy_s
		add			edx, byte 4				; (j*4) <- (j*4+4)
		dec			ebp
		addps		xm0, xm1
		jnz			.lp.i0
		movaps		[edi+ebx*4], xm0
		add			ebx, byte 4
		cmp			ebx, ecx
		jne			.lp.b0
		add			esi, HBLKSIZE_s*4*4		; &energy_s[k][0]
		;;;

		mov			ecx, [esp+_P+12]		; ecx = thr
		xor			ebx, ebx				; ebx = b*4
		mov			eax, RO.s3_s			; eax = &RO.s3_s[0][0]
.lp.b1:
		mov			edx, [RO.s3ind_s+ebx*2]	; edx = RO.s3ind_s[b][0]
		mov			ebp, [RO.s3ind_s+ebx*2+4]	; ebx = RO.s3ind_s[b][1]
		shl			edx, 2					; edx = i*4
		shl			ebp, 2					; ebp = (end of i) * 4
		xorps		xm0, xm0				; ecb
.lp.i:
		movss		xm1, [eax+edx]			; tmp = RO.s3_s[b][i]
		shufps		xm1, xm1, 0
		mulps		xm1, [edi+edx*4]		; eb
		add			edx, byte 4				; (i*4) <- (i*4+4)
		cmp			edx, ebp
		addps		xm0, xm1
		jbe			.lp.i

		movss		xm1, [RO.SNR_s+ebx]		; tmp = RO.SNR_s[b]
		add			eax, CBANDS*4
		shufps		xm1, xm1, 0
		mulps		xm0, xm1
		maxps		xm0, xm7
		movaps		[ecx+ebx*4], xm0
		add			ebx, byte 4
		cmp			ebx, [npart_sX4@]
		jnz			.lp.b1

		;;;
		;	ecx = thr
		;	edi = eb
		;	xm6 = 0.5
		xor			ebx, ebx				; ebx = b*4
		mov			ebp, [k@]
		lea			ebp, [III_psy_xmin_s.s+ebp*4]	;&s[0][k]
.lp.b2:
		mov			eax, [RO.bu_s+ebx]		; eax = bu
		mov			edx, [RO.bo_s+ebx]
		shl			eax, 2
		shl			edx, 2
		movaps		xm0, [edi+eax*4]		; eb[bu]
		movaps		xm1, [ecx+eax*4]		; thr[bu]
		addps		xm0, [edi+edx*4]		; += eb[bo]
		addps		xm1, [ecx+edx*4]		; += thr[bo]
		mulps		xm0, xm6				; *= 0.5
		mulps		xm1, xm6
		add			eax, byte 4				; eax = (bu_s[b]+1)*4
		cmp			eax, edx
		jae			.skip					; bu_s = bo_sのとき対策のため修正
.lp.i2:
		addps		xm0, [edi+eax*4]		; enn
		addps		xm1, [ecx+eax*4]		; thm
		add			eax, byte 4
		cmp			eax, edx
		jnz			.lp.i2

.skip:
		; &RW.en [chn].s[b][k]
		;= RW.en + III_psy_xmin_s.s + chn * sizeof_III_psy_xmin + b * 3*4 + k * 4
		movss		[RW.en + sizeof_III_psy_xmin*0 + ebp], xm0
		movss		[RW.thm+ sizeof_III_psy_xmin*0 + ebp], xm1
		shufps		xm0, xm0, PACK(0, 3, 2, 1)
		shufps		xm1, xm1, PACK(0, 3, 2, 1)
		movss		[RW.en + sizeof_III_psy_xmin*1 + ebp], xm0
		movss		[RW.thm+ sizeof_III_psy_xmin*1 + ebp], xm1
		shufps		xm0, xm0, PACK(0, 3, 2, 1)
		shufps		xm1, xm1, PACK(0, 3, 2, 1)
		movss		[RW.en + sizeof_III_psy_xmin*2 + ebp], xm0
		movss		[RW.thm+ sizeof_III_psy_xmin*2 + ebp], xm1
		shufps		xm0, xm0, PACK(0, 3, 2, 1)
		shufps		xm1, xm1, PACK(0, 3, 2, 1)
		movss		[RW.en + sizeof_III_psy_xmin*3 + ebp], xm0
		movss		[RW.thm+ sizeof_III_psy_xmin*3 + ebp], xm1
		add			ebp, byte 12			; =3*sizeof(float)
		add			ebx, byte 4
		cmp			ebx, byte (NBPSY_s * 4)
		jnz			near .lp.b2
		inc			dword [k@]
		cmp			[k@], dword 3
		jnz			near .lp.k
.exit:
		add			esp, byte LOCAL_SIZE
		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		ret
%endif

;void inner_psy_sub5_SSE_P7( gogo_thread_data *tl, float *eb, float *thr );
;   2002/04/08 sakai, Pentium4向け 11Kclk

proc	inner_psy_sub5_SSE_P7
		push		ebx
		push		esi
		push		edi
		push		ebp
%assign _P 4*4
		mov			esi, [esp+_P+4]			; esi = tl
		lea			edi, [esi+gogo_thread_data_s.psywork+0]				; edi = eb
;;
		mov			eax, [RO.npart_s_orig]
		lea			esi, [esi+gogo_thread_data_s.energy_s]	; esi = energy_s
		xor			ebx,ebx			; b
		add			eax,eax
		add			eax,eax			; eax = npart_sorig*4
		jmp			short .lp.b0
	
		align 16
.lp.b0:	movaps		xmm0,[esi]				; xmm0
		movaps		xmm1,[esi+4*HBLKSIZE_s*4*1]
		movaps		xmm2,[esi+4*HBLKSIZE_s*4*2]
		mov			ecx, [RO.numlines_s+ebx]	; ecx = RO.numlines_s[b]
		jmp			short .f0

		align 16
.lp.i0:	addps		xmm0,[esi]		; energy
		addps		xmm1,[esi+4*HBLKSIZE_s*4*1]
		addps		xmm2,[esi+4*HBLKSIZE_s*4*2]
.f0:	lea			esi,[esi+16]				; (j*4)++
		sub			ecx,byte 1
		jnz			.lp.i0
		movaps		[edi+ebx*4],xmm0		; eb[0]
		movaps		[edi+ebx*4+4*CBANDS*4*1],xmm1		; eb[1]
		movaps		[edi+ebx*4+4*CBANDS*4*2],xmm2		; eb[2]
		lea			ebx,[ebx+ 4]				; (b*4)++
		cmp			ebx, eax
		jb			.lp.b0
;;
		mov			eax, [RO.npart_s]
		xor			ebx,ebx			; b
		mov			esi,RO.s3_s		; 
		add			eax,eax
		add			eax,eax			; eax = npart_sorig*4
		movaps		xmm7,[Q_f1em6]
		jmp			short .lp.b1

		align 16
.lp.b1:
		mov			ebp,[RO.s3ind_s+ebx*2+0]
		add			ebp,ebp
		add			ebp,ebp
		mov			ecx,[RO.s3ind_s+ebx*2+4]
		add			ecx,ecx
		add			ecx,ecx
		movss		xmm6,[esi+ebp]	; tmp
		shufps		xmm6,xmm6,0
		movaps		xmm0,[edi+ebp*4]
		movaps		xmm1,[edi+ebp*4+4*CBANDS*4*1]
		movaps		xmm2,[edi+ebp*4+4*CBANDS*4*2]
		mulps		xmm0,xmm6
		mulps		xmm1,xmm6
		mulps		xmm2,xmm6
		jmp			short .f1

		align 16
.lp.i1:	movss		xmm6,[esi+ebp]	; tmp
		shufps		xmm6,xmm6,0
		movaps		xmm3,[edi+ebp*4]
		movaps		xmm4,[edi+ebp*4+4*CBANDS*4*1]
		movaps		xmm5,[edi+ebp*4+4*CBANDS*4*2]
		mulps		xmm3,xmm6
		mulps		xmm4,xmm6
		mulps		xmm5,xmm6
		addps		xmm0,xmm3
		addps		xmm1,xmm4
		addps		xmm2,xmm5
.f1:	lea			ebp,[ebp+4]			; i++
		cmp			ebp,ecx
		jbe			.lp.i1

		movss		xmm6,[RO.SNR_s+ebx]
		shufps		xmm6,xmm6,0
		mulps		xmm0,xmm6
		mulps		xmm1,xmm6
		mulps		xmm2,xmm6
		maxps		xmm0,xmm7
		maxps		xmm1,xmm7
		maxps		xmm2,xmm7
		movaps		[edi+ebx*4+4*CBANDS*4*3],xmm0		; thr[0]
		movaps		[edi+ebx*4+4*CBANDS*4*4],xmm1		; thr[1]
		movaps		[edi+ebx*4+4*CBANDS*4*5],xmm2		; thr[2]
		lea			ebx,[ebx+ 4]				; (b*4)++
		lea			esi,[esi+ 4*CBANDS]
		cmp			ebx, eax
		jb			near .lp.b1
;;
		mov			esi,RW.en+III_psy_xmin_s.s
		xor			ebx,ebx			; b
		movaps		xmm7,[Q_f0p5]
		jmp			short .lp.b2

		align 16
.lp.b2:
		mov			ebp,[RO.bu_s+ebx]
		mov			ecx,[RO.bo_s+ebx]
		shl			ebp,4
		shl			ecx,4
		movaps		xmm0,[edi+ebp]
		addps		xmm0,[edi+ecx]
		movaps		xmm1,[edi+ebp+4*CBANDS*4*1]
		addps		xmm1,[edi+ecx+4*CBANDS*4*1]
		movaps		xmm2,[edi+ebp+4*CBANDS*4*2]
		addps		xmm2,[edi+ecx+4*CBANDS*4*2]
		movaps		xmm3,[edi+ebp+4*CBANDS*4*3]
		addps		xmm3,[edi+ecx+4*CBANDS*4*3]
		movaps		xmm4,[edi+ebp+4*CBANDS*4*4]
		addps		xmm4,[edi+ecx+4*CBANDS*4*4]
		movaps		xmm5,[edi+ebp+4*CBANDS*4*5]
		addps		xmm5,[edi+ecx+4*CBANDS*4*5]
		mulps		xmm0,xmm7
		mulps		xmm1,xmm7
		mulps		xmm2,xmm7
		mulps		xmm3,xmm7
		mulps		xmm4,xmm7
		mulps		xmm5,xmm7
		jmp			short .f2

		align 16
.lp.i2:
		addps		xmm0,[edi+ebp]
		addps		xmm1,[edi+ebp+4*CBANDS*4*1]
		addps		xmm2,[edi+ebp+4*CBANDS*4*2]
		addps		xmm3,[edi+ebp+4*CBANDS*4*3]
		addps		xmm4,[edi+ebp+4*CBANDS*4*4]
		addps		xmm5,[edi+ebp+4*CBANDS*4*5]
.f2:	lea			ebp,[ebp+16]			; i++
		cmp			ebp,ecx
		jb			.lp.i2

		movaps		xmm6,xmm0
		unpcklps	xmm0,xmm1
		unpckhps	xmm6,xmm1

		movlps		[esi],xmm0
		movss		[esi+8],xmm2
		shufps		xmm2,xmm2,00111001B

		movhps		[esi+sizeof_III_psy_xmin*1],xmm0
		movss		[esi+sizeof_III_psy_xmin*1+8],xmm2
		shufps		xmm2,xmm2,00111001B

		movlps		[esi+sizeof_III_psy_xmin*2],xmm6
		movss		[esi+sizeof_III_psy_xmin*2+8],xmm2
		shufps		xmm2,xmm2,00111001B

		movhps		[esi+sizeof_III_psy_xmin*3],xmm6
		movss		[esi+sizeof_III_psy_xmin*3+8],xmm2

		movaps		xmm1,xmm3
		unpcklps	xmm3,xmm4
		unpckhps	xmm1,xmm4

		movlps		[esi-rw_t.en+rw_t.thm],xmm3
		movss		[esi-rw_t.en+rw_t.thm+8],xmm5
		shufps		xmm5,xmm5,00111001B

		movhps		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*1],xmm3
		movss		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*1+8],xmm5
		shufps		xmm5,xmm5,00111001B

		movlps		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*2],xmm1
		movss		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*2+8],xmm5
		shufps		xmm5,xmm5,00111001B

		movhps		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*3],xmm1
		movss		[esi-rw_t.en+rw_t.thm+sizeof_III_psy_xmin*3+8],xmm5

		lea			esi,[esi+4*3]
		lea			ebx,[ebx+ 4]				; (b*4)++
		cmp			ebx, NBPSY_s*4
		jb			near .lp.b2
;;
		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		ret

;	2001/09/03 shigeo, 4.5Kclk@PIII, first version
;   2002/04/05 sakai, Pentium4よりAthlon-MPのほうが速い(^^;;;
;   2002/04/09 sakai, inner_psy_sub6_3DNと合わせただけ

; inner_psy_chn4()と合わせること。
%define eb	(gogo_thread_data_s.psywork)
%define cb	(gogo_thread_data_s.psywork+CBANDS*4*4)

;void inner_psy_sub6( gogo_thread_data *tl, float *eb, float *cb );
proc	inner_psy_sub6_SSE
		push		ebx
		push		esi
		push		edi
		push		ebp
%assign _P 4*4
		mov			edx, [esp+_P+4]			; esi = tl
;;
		mov			ebp, [RO.npart_l_pre_max]
		lea			edi, [edx+eb]			; edi = eb
		lea			esi, [edx+gogo_thread_data_s.energy]	; esi = energy
		shl			ebp, 2					; ebp = npart_l_pre_max*4
		mov			eax,RW.cw
		sub			eax,esi
		xor			ebx, ebx				; ebx = (b*4)
		jmp			short .lp.b0
	
		align 16
.lp.b0:	movaps		xmm0,[esi]				; xm0 = ebb
		movaps		xmm1,xmm0
		mulps		xmm1,[esi+eax]				; xm1 = cbb
		mov			ecx, [RO.numlines_l+ebx]	; ecx = RO.numlines_l[b]
		jmp			short .f0

		align 16
.lp.i0:	movaps		xm2, [esi]		; energy
		addps		xm0, xm2
		mulps		xm2, [esi+eax]
		addps		xm1, xm2
.f0:	lea			esi,[esi+16]				; (j*4)++
		sub			ecx,byte 1
		jnz			.lp.i0
		movaps		[edi+ebx*4], xm0		; eb
		movaps		[edi-eb+cb+ebx*4], xm1		; cb
		lea			ebx,[ebx+ 4]				; (b*4)++
		cmp			ebx, ebp
		jnz			.lp.b0
;;
		mov			ebp, [RO.npart_l_orig]
		shl			ebp, 2
		cmp			ebx, ebp
		je			.exit
		movaps		xm7, [Q_f0p4]			; xm7 = 0.4
.lp.b1:	movaps		xmm0,[esi]				; ebb
		mov			eax,[RO.numlines_l+ebx]
		shl			eax,4
		add			esi,eax
		neg			eax
		jmp			short .f1

		align 16
.lp.i1:	addps		xm0, [esi+eax]		; energy
.f1:	add			eax, byte 16
		jnz			.lp.i1

		movaps		[edi+ebx*4], xm0		; eb
		mulps		xm0, xm7
		movaps		[edi-eb+cb+ebx*4], xm0		; cb
		lea			ebx,[ebx+ 4]
		cmp			ebx, ebp
		jnz			.lp.b1
.exit:
		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		ret

%undef eb
%undef cb
			end
