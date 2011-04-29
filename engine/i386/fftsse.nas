;
;   part of this code is origined from
;
;       GOGO-no-coda
;	Copyright (c) 2001,2002,2003 shigeo
;       Copyright (c) 2001,2002,2003 gogo-developer
;
;	2001/10/21 Init. version gogo2とほぼ同じ
;	2001/10/28 ちょっとgogo2-FFTより速くなったかも
;	2001/12/01 brush up

%include "global.cfg"

%define MDCTDELAY	48
%define FFTOFFSET	(224+MDCTDELAY)
%define BLKSIZE		1024
%define BLKSIZE_s	256
	globaldef	__checkalign_fftsse__

	segment_data
	align	32
__checkalign_fftsse__:
Q_SQRT2	dd	1.41421356237, 1.41421356237, 1.41421356237, 1.41421356237

Q_1		dd	1.0, 1.0, 1.0, 1.0
costab	dd	9.238795325112867e-01F, 9.238795325112867e-01F, 9.238795325112867e-01F, 9.238795325112867e-01F
		dd	3.826834323650898e-01F, 3.826834323650898e-01F, 3.826834323650898e-01F, 3.826834323650898e-01F
		dd	9.951847266721969e-01F, 9.951847266721969e-01F, 9.951847266721969e-01F, 9.951847266721969e-01F
		dd	9.801714032956060e-02F, 9.801714032956060e-02F, 9.801714032956060e-02F, 9.801714032956060e-02F
		dd	9.996988186962042e-01F, 9.996988186962042e-01F, 9.996988186962042e-01F, 9.996988186962042e-01F
		dd	2.454122852291229e-02F, 2.454122852291229e-02F, 2.454122852291229e-02F, 2.454122852291229e-02F
		dd	9.999811752826011e-01F, 9.999811752826011e-01F, 9.999811752826011e-01F, 9.999811752826011e-01F
		dd	6.135884649154475e-03F, 6.135884649154475e-03F, 6.135884649154475e-03F, 6.135884649154475e-03F

	align	32
revLongInit	db 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
revLongNext	db 0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80

revShortInit db 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00
revShortNext db 0x00, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00

	align	16
Q_M_P_M_P	dd	0x00000000, 0x80000000, 0x00000000, 0x80000000
Q_rsq2		dd	0.707106781, -0.707106781, 0.707106781, -0.707106781
	segment_text

;	long

;	k4=64 x 64
;	kx=8 x 1
;	k4=256 x 16
;	kx=32 x 7
;	k4=1024 x 4
;	kx=128 x 31
;	k4=4096 x 1
;	kx=512 x 127

;	short x 3

;	k4=64 x 16
;	kx=8 x 1
;	k4=256 x 4
;	kx=32 x 7
;	k4=1024 x 1
;	kx=128 x 31
;------------------------------------------------------------------------
;void fht(float *fz, int n);

%define fz		BASE+ 4
%define n		BASE+ 8

%define LOCAL_SIZE (16*5+4*2+16)
%define BASE esp+_P

%define Q_c2@	ebp+0
%define Q_s2@	ebp+16
%define Q_c1@	ebp+32
%define Q_s1@	ebp+48
%define work@	ebp+64
%define n@		ebp+80+4*0
%define fn@		ebp+80+4*1

proc	fht_vector4_SSE
		push		ebx
		push		esi
		push		edi
		push		ebp
%assign _P 4*4+LOCAL_SIZE
		sub			esp, LOCAL_SIZE
		lea			ebp, [esp+15]
		and			ebp, -16

		mov			esi, [fz]
		mov			ebx, [n]
		add			ebx, ebx
		lea			ecx, [esi+ebx*8]
		mov			[n@], ebx			; n *= 2
		mov			[fn@], ecx			; fn = &fz[n]

.for_0:
		movaps		xm0, [esi+0*16]
		movaps		xm1, [esi+1*16]
		movaps		xm3, [esi+2*16]
		movaps		xm4, [esi+3*16]
		movaps		xm2, xm0
		movaps		xm5, xm3
		subps		xm0, xm1			; f1 = fi[0] - fi[1]
		addps		xm2, xm1			; f0 = fi[0] + fi[1]
		subps		xm3, xm4			; f3 = fi[2] - fi[3]
		addps		xm5, xm4			; f2 = fi[2] + fi[3]

		movaps		xm1, xm2			; f0
		movaps		xm4, xm0			; f1
		subps		xm2, xm5			; f0 - f2
		subps		xm0, xm3			; f1 - f3
		addps		xm1, xm5			; f0 + f2
		addps		xm4, xm3			; f1 + f3
		movaps		[esi+0*16], xm1
		movaps		[esi+1*16], xm4
		movaps		[esi+2*16], xm2
		movaps		[esi+3*16], xm0
		add			esi, 4*16
		cmp			esi, ecx
		jb			.for_0

		mov			edx, costab
		mov			ebx, 2*4			; ebx = kx*4

		; kx = ebx
		; k1 = ebx*2
		; k2 = ebx*4
		; k3 = ecx
		; k4 = ebx*8

.do_1:	; {
		shl			ebx, 2				; kx <<= 2
		mov			esi, [fz]			; esi = fi
		lea			ecx, [ebx+ebx*2]
		add			ecx, ecx			; ecx = ebx*6 = k3
		lea			edi, [esi+ebx]		; edi = gi = &fi[kx]

		movaps		xm7, [Q_SQRT2]
.do_2:
		movaps		xm0, [esi+  0]		; fi[0]
		movaps		xm1, [esi+ebx*2]	; fi[k1]
		movaps		xm2, [esi+ebx*4]	; fi[k2]
		movaps		xm3, [esi+ecx]		; fi[k3]
		movaps		xm4, xm0
		subps		xm0, xm1			; f1
		movaps		xm5, xm2
		addps		xm1, xm4			; f0
		subps		xm2, xm3			; f3
		addps		xm3, xm5			; f2

		movaps		xm4, xm1
		movaps		xm5, xm0
		subps		xm1, xm3			; f0 - f2
		addps		xm3, xm4			; f0 + f2
		subps		xm0, xm2			; f1 - f3
		addps		xm2, xm5			; f1 + f3

		movaps		[esi+ebx*4], xm1	; fi[k2]
		movaps		[esi+  0], xm3		; fi[0]
		movaps		[esi+ecx], xm0		; fi[k3] 
		movaps		[esi+ebx*2], xm2	; fi[k1]

		movaps		xm1, [edi]
		movaps		xm0, [edi]
		subps		xm1, [edi+ebx*2]	; f1
		movaps		xm3, [edi+ecx]
		addps		xm0, [edi+ebx*2]	; f0
		movaps		xm2, [edi+ebx*4]
		mulps		xm3, xm7			; f3
		mulps		xm2, xm7			; f2
		movaps		xm4, xm0			; f0
		movaps		xm5, xm1			; f1
		subps		xm0, xm2			; f0 - f2
		subps		xm1, xm3			; f1 - f3

		lea			esi, [esi+ebx*8]
		addps		xm2, xm4			; f0 + f2
		addps		xm3, xm5			; f1 + f3
		cmp			esi, [fn@]
		movaps		[edi+ebx*4], xm0	; gi[k2]
		movaps		[edi+  0], xm2		; gi[0]
		movaps		[edi+ecx], xm1		; gi[k3]
		movaps		[edi+ebx*2], xm3	; gi[k1]
		lea			edi, [edi+ebx*8]
		jb			near .do_2

		movaps		xm0, [edx]
		movaps		xm1, [edx+16]
		movaps		[Q_c1@], xm0
		movaps		[Q_s1@], xm1

		mov			eax, 4*4			; eax = i*4
.for_1:	;{
		mov			edi, [fz]
		lea			esi, [edi+eax]		; esi = fi = &fz[i]
		sub			edi, eax
		lea			edi, [edi+ebx*2]	; edi = gi = &fz[k1-i]

		movaps		xm3, [Q_1]
		movaps		xm0, [Q_s1@]
		movaps		xm2, [Q_c1@]
		movaps		xm1, xm0
		addps		xm0, xm0
		mulps		xm2, xm0			; s2 = (s1+s1) * c1
		mulps		xm0, xm1
		subps		xm3, xm0			; c2 = 1 - (s1+s1) * s1
		movaps		[Q_c2@], xm3
		movaps		[Q_s2@], xm2

.do_3:	; {
		movaps		xm6, [Q_c2@]			; c2
		movaps		xm7, [Q_s2@]			; s2
		movaps		xm0, [esi+ebx*2]		; fi[k1]
		movaps		xm1, xm0				; fi[k1]
		mulps		xm0, xm7				; s2 * fi
		mulps		xm1, xm6				; c2 * fi
		movaps		xm2, xm6				; c2
		movaps		xm3, xm7				; s2
		mulps		xm2, [edi+ebx*2]		; c2 * gi[k1]
		mulps		xm3, [edi+ebx*2]		; s2 * gi[k1]
		subps		xm0, xm2				; b
		addps		xm1, xm3				; a
		movaps		xm2, [esi]				; fi[0]
		movaps		xm3, [edi]				; gi[0]
		subps		xm2, xm1				; f1 = fi - a
		subps		xm3, xm0				; g1 = gi - b
		addps		xm1, [esi]				; f0 = fi + a
		addps		xm0, [edi]				; g0 = gi + b

		movaps		xm4, [esi+ecx]			; fi[k3]
		movaps		xm5, xm4				; fi[k3]
		mulps		xm4, xm7				; s2 * fi
		mulps		xm5, xm6				; c2 * fi
		mulps		xm6, [edi+ecx]			; c2 * gi
		mulps		xm7, [edi+ecx]			; s2 * gi
		subps		xm4, xm6				; b
		addps		xm5, xm7				; a
		movaps		xm6, [esi+ebx*4]		; fi[k2]
		movaps		xm7, [edi+ebx*4]		; gi[k2]
		subps		xm6, xm5				; f3 = fi - a
		subps		xm7, xm4				; g3 = gi - b
		addps		xm5, [esi+ebx*4]		; f2 = fi + a
		addps		xm4, [edi+ebx*4]		; g2 = gi + b
		movaps		[work@], xm0			; keep g0
		movaps		[esi+ebx*4], xm1		; keep f0
		movaps		xm1, [Q_s1@]			; s1
		movaps		xm0, [Q_c1@]			; c1
		mulps		xm1, xm5				; s1 * f2
		mulps		xm0, xm7				; c1 * g3
		subps		xm1, xm0				; b
		movaps		xm0, xm1				; b
		addps		xm0, xm3				; g1 + b
		subps		xm3, xm1				; g1 - b
		movaps		[edi+ecx], xm3			; gi[k3]
		movaps		[edi+ebx*2], xm0		; gi[k1]

		mulps		xm5, [Q_c1@]			; c1 * f2
		movaps		xm0, [esi+ebx*4]		; f0
		mulps		xm7, [Q_s1@]			; s1 * g3
		movaps		xm1, xm0				; f0
		addps		xm5, xm7				; a
		subps		xm0, xm5				; f0 - a
		addps		xm1, xm5				; f0 + a
		movaps		[esi+ebx*4], xm0		; fi[k2]
		movaps		[esi+  0], xm1			; fi[0]

		movaps		xm0, xm4				; g2
		movaps		xm1, xm6				; f3
		mulps		xm4, [Q_s1@]			; s1 * g2
		mulps		xm6, [Q_s1@]			; s1 * f3
		mulps		xm0, [Q_c1@]			; c1 * g2
		mulps		xm1, [Q_c1@]			; c1 * f3
		subps		xm0, xm6				; b
		addps		xm1, xm4				; a
		movaps		xm6, [work@]			; g0
		subps		xm6, xm1				; g0 - a
		addps		xm1, [work@]			; g0 + a

		movaps		xm3, xm2				; f1
		movaps		[edi+ebx*4], xm6		; gi[k2]
		subps		xm2, xm0				; f1 - b
		movaps		[edi+  0], xm1			; gi[0]
		addps		xm0, xm3				; f1 + b
		movaps		[esi+ecx], xm2			; fi[k3]
		movaps		[esi+ebx*2], xm0		; fi[k1]
		lea			esi, [esi+ebx*8]		; fi += k4
		lea			edi, [edi+ebx*8]		; gi += k4
		cmp			esi, [fn@]
		jb			near .do_3		; }

		movaps		xm0, [Q_c1@]
		movaps		[Q_c2@], xm0
		movaps		xm1, xm0
		movaps		xm2, [Q_s1@]
		mulps		xm0, [edx]		; c2 * tri[0]
		movaps		xm3, xm2
		mulps		xm1, [edx+16]	; c2 * tri[1]
		mulps		xm2, [edx+16]	; s1 * tri[1]
		mulps		xm3, [edx]		; s1 * tri[0]
		subps		xm0, xm2
		addps		xm1, xm3
		movaps		[Q_c1@], xm0	; c1
		movaps		[Q_s1@], xm1	; s1

		add			eax, byte 4*4	; i += 4
		cmp			eax, ebx
		jb			near .for_1		; }

		add			edx, dword 2*4*4	; tri += 2
		cmp			ebx, [n@]
		jb			near .do_1		; }
.exit:
		add			esp, LOCAL_SIZE
		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		ret

;	2001/10/25	shigeo
;	genuine first version
;	一発完動後いじってない(80Kclk)
;	2001/10/27 rb_tblを使わないようにする(速度は変わらず)
;	RO.windwのデータ読み込みには7Kclkほど必要。うまく先読みできるかな。


; void fft_long_vector4(float *wsamp, float *mfbuf );
proc	fft_long_vector4_SSE
		push		esi
		push		edi
		push		ebp
%assign _P 4*3
		mov			esi, [esp+_P+4]		; wsamp
		mov			edi, [esp+_P+8]		; mfbuf
		add			esi, (BLKSIZE/2)*4*4		; x
		add			edi, (576-FFTOFFSET)*4*4	; buffer
		mov			ecx, BLKSIZE/8		; j
		movq		mm0, [revLongInit]
		movq		mm1, [revLongNext]
		mov			ebp, RO.window
		movaps		xm0, [ebp+ 0]
		jmp			.lp_j
		align		16
.lp_j:
		pmovmskb	eax, mm0			; trick for bit reverse
		paddb		mm0, mm1
		shl			eax, 2

		movaps		xm1, xm0
		shufps		xm0, xm0, PACK(0,0,0,0)
		movaps		xm2, xm1
		movaps		xm4, [ebp+16]
		shufps		xm1, xm1, PACK(1,1,1,1)
		movaps		xm3, xm2
		shufps		xm2, xm2, PACK(2,2,2,2)
		shufps		xm3, xm3, PACK(3,3,3,3)

		movaps		xm5, xm4
		shufps		xm4, xm4, PACK(0,0,0,0)
		movaps		xm6, xm5
		shufps		xm5, xm5, PACK(1,1,1,1)
		movaps		xm7, xm6
		shufps		xm6, xm6, PACK(2,2,2,2)
		shufps		xm7, xm7, PACK(3,3,3,3)

		mulps		xm0, [edi+eax*4+0x000*16]
		mulps		xm1, [edi+eax*4+0x001*16]
		mulps		xm2, [edi+eax*4+0x200*16]
		mulps		xm3, [edi+eax*4+0x201*16]

		movaps		[esi+(0-4)*16], xm0
		movaps		[esi+(1-4)*16], xm2
		movaps		[esi+(BLKSIZE/2+0-4)*16], xm1
		movaps		[esi+(BLKSIZE/2+1-4)*16], xm3
		mulps		xm4, [edi+eax*4+0x100*16]
		mulps		xm5, [edi+eax*4+0x101*16]
		mulps		xm6, [edi+eax*4+0x300*16]
		mulps		xm7, [edi+eax*4+0x301*16]

		movaps		xm0, [ebp+32+ 0]

		movaps		[esi+(2-4)*16], xm4
		movaps		[esi+(BLKSIZE/2+2-4)*16], xm5
		movaps		[esi+(3-4)*16], xm6
		movaps		[esi+(BLKSIZE/2+3-4)*16], xm7
		dec			ecx
		lea			esi, [esi-4*4*4]				; x -= 4*4
		lea			ebp, [ebp+8*4]					; RO.window
		jnz			near .lp_j
		push		dword BLKSIZE
		push		esi
		call		fht_vector4_SSE
		add			esp, byte 8
.exit:
		emms
		pop			ebp
		pop			edi
		pop			esi
		ret

;	2001/10/28	shigeo
;	first version

; void fft_short_vector4(float *wsamp, float *mfbuf );
proc	fft_short_vector4_SSE
		push		ebx
		push		esi
		push		edi
		push		ebp
%assign _P 4*4
		mov			edi, [esp+_P+4]		; wsamp
		mov			esi, [esp+_P+8]		; mfbuf
		add			edi, (BLKSIZE_s/2)*4*4		; x
		add			esi, (576-FFTOFFSET + 576/3)*4*4	; buffer
		mov			ebx, 3				; b = ebx
.lp.b:
		mov			ecx, BLKSIZE_s/8	; j = ecx
		mov			ebp, RO.window_s	; ebp = win1
		lea			edx, [ebp+(128-4)*4]; edx = win2
		movq		mm0, [revShortInit]
		movq		mm1, [revShortNext]
		jmp			.lp.j
		align	16
.lp.j:
		pmovmskb	eax, mm0			; trick for bit reverse
		paddb		mm0, mm1
		shl			eax, 2				; i*4
		movaps		xm0, [ebp]			; win1
		movaps		xm4, [edx]			; win2
		movaps		xm1, xm0
		movaps		xm5, xm4
		shufps		xm0, xm0, PACK(0,0,0,0)	; win1[0]
		shufps		xm4, xm4, PACK(3,3,3,3)	; win2[3]
		movaps		xm2, xm1
		movaps		xm6, xm5
		shufps		xm1, xm1, PACK(1,1,1,1)	; win1[1]
		shufps		xm5, xm5, PACK(2,2,2,2)	; win2[2]
		movaps		xm3, xm2
		movaps		xm7, xm6
		shufps		xm2, xm2, PACK(2,2,2,2)	; win1[2]
		shufps		xm6, xm6, PACK(1,1,1,1)	; win2[1]
		shufps		xm3, xm3, PACK(3,3,3,3) ; win1[3]
		shufps		xm7, xm7, PACK(0,0,0,0)	; win2[0]

		mulps		xm0, [esi+0x00*16+eax*4]
		mulps		xm4, [esi+0x01*16+eax*4]

		mulps		xm1, [esi+0x80*16+eax*4]
		mulps		xm5, [esi+0x81*16+eax*4]

		mulps		xm2, [esi+0x40*16+eax*4]
		mulps		xm6, [esi+0x41*16+eax*4]

		mulps		xm3, [esi+0xC0*16+eax*4]
		mulps		xm7, [esi+0xC1*16+eax*4]

		movaps		[edi-4*16], xm0
		movaps		[edi-3*16], xm1
		movaps		[edi-2*16], xm2
		movaps		[edi-1*16], xm3
		movaps		[edi+(BLKSIZE_s/2-4)*16], xm4
		movaps		[edi+(BLKSIZE_s/2-3)*16], xm5
		movaps		[edi+(BLKSIZE_s/2-2)*16], xm6
		movaps		[edi+(BLKSIZE_s/2-1)*16], xm7
		sub			edi, byte 4*4*4			; x -= 4*4
		add			ebp, byte 4*4
		sub			edx, byte 4*4
		dec			ecx
		jnz			near .lp.j

		push		dword BLKSIZE_s
		push		edi
		call		fht_vector4_SSE
		add			esp, byte 8
		add			esi, 576/3*4*4			; buffer += 576/3*4
		add			edi, (BLKSIZE_s * 4+4*4*BLKSIZE_s/8) * 4	; x
		dec			ebx
		jnz			near .lp.b
.exit:
		emms
		pop			ebp
		pop			edi
		pop			esi
		pop			ebx
		ret

;	2001/12/01	shigeo
;	first version
;void fft_prepare_SSE(float *dest, const float *src, int count);
proc	fft_prepare_SSE
%assign _P 4*0
		mov			edx, [esp+_P+4]		; edx = dest
		mov			eax, [esp+_P+8]		; eax = src
		mov			ecx, [esp+_P+12]	; ecx = count
		jmp			.lp
		align	16
.lp:
		movaps		xm7, [eax]
		movaps		xm6, [eax+4*576*4]
		movaps		xm5, [eax+576*4]
		movaps		xm4, [eax+5*576*4]
		add			eax, byte 16
		movaps		xm0, xm7
		unpcklps	xm7, xm6
		movlps		[edx], xm7
		movhps		[edx+16], xm7
		unpckhps	xm0, xm6
		movlps		[edx+32], xm0
		movhps		[edx+48], xm0
		movaps		xm0, xm5
		unpcklps	xm5, xm4
		unpckhps	xm0, xm4
		movlps		[edx+8], xm5
		movhps		[edx+24], xm5
		movlps		[edx+40], xm0
		movhps		[edx+56], xm0
		add			edx, 64
		sub			ecx, byte 4
		jnz			.lp
		ret

;void psy_prepare(float *dest, const float *src, int count);
proc	psy_prepare_SSE
		mov			edx, [esp+4]
		mov			eax, [esp+8]
		mov			ecx, [esp+12]
		movaps		xm7, [Q_M_P_M_P]
		movaps		xm6, [Q_rsq2]	; [-rsq2:rsq2:-rsq2:rsq2]
		shr			ecx, 2
		jmp			.lp
		align	16
.lp:
		movlps		xm0, [eax]
		movhps		xm0, [eax+16]	; [R':L':R:L]
		movlps		xm2, [eax+32]
		movhps		xm2, [eax+48]
		add			eax, byte 64
		movaps		xm1, xm0
		movaps		xm3, xm2
		movlps		[edx], xm0
		movhps		[edx+16], xm0
		movlps		[edx+32], xm2
		movhps		[edx+48], xm2
		shufps		xm1, xm1, PACK(2,3,0,1)	; [L':R':L:R]
		shufps		xm3, xm3, PACK(2,3,0,1)
		xorps		xm1, xm7		; [-L':R':-L:R]
		xorps		xm3, xm7
		addps		xm0, xm1		; [R'-L':R'+L':R-L:R+L]
		addps		xm2, xm3
		mulps		xm0, xm6		; [L'-R':R'+L':L-R:R+L] * rsq2
		mulps		xm2, xm6
		movlps		[edx+8], xm0
		movhps		[edx+24], xm0
		movlps		[edx+32+8], xm2
		movhps		[edx+32+24], xm2
		add			edx, byte 64
		dec			ecx
		jnz			.lp
		ret

		end
