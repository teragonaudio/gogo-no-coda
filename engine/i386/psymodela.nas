;
;   part of this code is origined from
;
;       GOGO-no-coda
;       Copyright (c) 2001,2002,2003 gogo-developer
;

;%include "nasm.cfg"
%include "global.cfg"

	externdef		tonalityTbl		;float * x 2
%define tonalityTblNum 256	;defined in psymode.c
	globaldef	__checkalign_psymodel__

	segment_data
	align	32
__checkalign_psymodel__:
LIMIT_L		dd	0.048755842987912974	; exp((1-CONV1)/CONV2)
LIMIT_U		dd	0.498900382609471840	; exp((0-CONV1)/CONV2))
Q_coef3		dd	568.70622092899725		; tonalityTblNum / (LIMIT_U - LIMIT_L)
Q_8			dd	8.0						; rpelev = 2, rpelev2 = 16


	segment_text

;void convolute_energy_C(
;	const int typeFlag,
;	float *eb,
;	float *cb,
;	float *thr,
;	float *nb_12 );

%define typeFlag	BASE+4
%define eb			BASE+8
%define cb			BASE+12
%define thr			BASE+16
%define nb_12		BASE+20

%define BASE		esp+_P

;	2001/08/17
;	Init. version by shigeo
;	10.5Kclk@PIII
;	うー、SISDばっかり...

proc	convolute_energy_SSE
		push		ebx
		push		esi
		push		edi
		push		ebp
%assign _P 4*4
		xor			ebx, ebx				; ebx = b = 0;
		mov			ebp, RO.s3_l			; ebp = &RO.s3_l[b]
		mov			esi, [eb]				; esi = eb
		mov			edi, [cb]				; edi = cb
		mov			edx, [nb_12]			; edx = nb_12
		movss		xm5, [LIMIT_L]			; xm5 = LIMIT_L
.lp.b:
		mov			eax, [RO.s3ind+ebx*8]	; eax = k
		mov			ecx, [RO.s3ind+ebx*8+4]	; ecx = s3ind[b][1]
		xorps		xm0, xm0				; xm0 = ecb
		xorps		xm4, xm4				; xm4 = ctb

.lp.k:
		movss		xm3, [esi+eax*4]		; xm2 = eb
		movss		xm1, [edi+eax*4]		; xm3 = cb
		mulss		xm3, [ebp+eax*4]
		mulss		xm1, [ebp+eax*4]
		inc			eax
		cmp			eax, ecx
		addss		xm0, xm3
		addss		xm4, xm1
		jbe			.lp.k

		add			ebp, 256				; 256 = CBANDS * sizeof(float)

		xorps		xm3, xm3
		movss		xm7, xm0				; xm7 = ecb
		cmpneqss	xm3, xm0				; xm3 = (ecb) ? -1 : 0;

		; calc ctb / ecb
		movss		xm1, xm0				; xm1 = ecb
		rcpss		xm2, xm0
		andps		xm2, xm3				; xm2 = (ecb) ? 1/ecb : 0;
		mulss		xm1, xm2
		mulss		xm1, xm2
		addss		xm2, xm2
		subss		xm2, xm1				; real 1/ecb
		mulss		xm2, xm4				; tbb = ctb / ecb

		maxss		xm2, xm5
		minss		xm2, [LIMIT_U]			; clipping in (LIMIT_L, LIMIT_U)
		subss		xm2, xm5
		mulss		xm2, [Q_coef3]			; tbb = (tbb - LIMIT_L) * (tonalityTblNum / (LIMIT_U - LIMIT_L))

		cvttss2si	eax, xm2				; eax = (int)tbb
		cvtsi2ss	xm1, eax
		subss		xm2, xm1				; tbb - idx

		mulss		xm2, [tonalityTbl+eax*8+4]
		addss		xm2, [tonalityTbl+eax*8+0]	; xm2 = tbb
		mov			eax, [thr]
		minss		xm2, [RO.minval+ebx*4]
		cmp			dword [typeFlag], 0
		mulss		xm2, xm7				; ecb *= tbb
		movss		xm1, [edx+ebx*8]		; xm1 = nb_12[b*2]
		movss		[edx+ebx*8], xm2		; ecb

		jne			.typeFlag.is.nonzero
		movss		xm3, [edx+ebx*8+4]		; xm3 = nb_12[b*2+1]
		mulss		xm3, [Q_8]
		minss		xm3, xm1				; min(nb_12[b*2], 8*nb_12[b*2+1])
		addss		xm3, xm3				; min(2*nb_12[b*2], 16*nb_12[b*2+1])
		minss		xm2, xm3
.typeFlag.is.nonzero:
		movss		[eax+ebx*4], xm2		; thr[b]
		movss		[edx+ebx*8+4], xm1		; nb_12[b*2]
		inc			ebx
		cmp			ebx, [RO.npart_l]
		jne			near .lp.b
.exit:
		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		ret

		end
