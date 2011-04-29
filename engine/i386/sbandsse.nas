;
;   part of this code is origined from
;
;       GOGO-no-coda
;       Copyright (c) 2002 k.sakai
;       Copyright (c) 2002,2003 gogo-developer
;

%include "global.cfg"

        segment_data
		externdef	enwindow

        segment_text
;		2002/5/17	1st version by k.sakai, 1350clk@河童, 1210clk@じーおん, 1290clk@明日論
proc 	window_subband_sub1_SSE
		push	ebx

%assign	_P 1*4
		mov		edx,[esp+_P+4]	; x1
		mov		ecx,[esp+_P+8]	; a
		add		ecx,64
		mov		ebx,12*4		; i
		lea		eax,[edx-62*4]	; x2 = &x1[-62], aligned to 16byte
		sub		edx,15*4
;		jmp		short .lp0
%define	x1 edx
%define x2 ebx+eax
%define wp ebx+enwindow

		align 16
.lp0:
%if 0
static void window_subband_sub1_C(const float *x1, float *a)
{
	int i;
	const float *x2 = &x1[238 - 14 - 286];/* x2 = &x1[-62] */
	const float *wp = enwindow;
	float w1, w2, s, t, u, v;

	for (i = 0; i < 15; i++) {
%endif
	;xmm1 = t =  x1[ 224] * wp[ 0*16]
	;xmm3 = v =  x1[ 256] * wp[ 0*16]
	;xmm0 = s =  x2[-224] * wp[ 0*16]
	;xmm2 = u =  x1[ 224] * wp[15*16]
		movaps	xmm1,[x1 + 224*4+4]
		movhps	xmm2,[x1 + 224*4-4]
		movss	xmm2,xmm1
		shufps	xmm1,xmm2,11000110B		; t
		movaps	xmm3,[x1 + 256*4+4]
		movhps	xmm5,[x1 + 256*4-4]
		movss	xmm5,xmm3
		shufps	xmm3,xmm5,11000110B		; v
		movaps	xmm0,[x2 - 224*4]		; s
		movaps	xmm2,xmm1				; u

		movaps	xmm7,[wp +  0*16*4]
		movaps	xmm6,[wp + 15*16*4]
		mulps	xmm1,xmm7				; t
		mulps	xmm3,xmm7				; v
		mulps	xmm0,xmm7				; s
		mulps	xmm2,xmm6				; u
	;u += x2[-192] * wp[ 0*16]
	;t -= x2[-192] * wp[15*16]
		movaps	xmm4,[x2 - 192*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[-160] * wp[ 1*16]
	;v -= x2[-160] * wp[15*16]
		movaps	xmm7,[wp +  1*16*4]
		movaps	xmm5,[x2 - 160*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[ 192] * wp[15*16]
	;v += x1[ 192] * wp[ 1*16]
		movaps	xmm4,[x1 + 192*4+4]
		movhps	xmm5,[x1 + 192*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[ 160] * wp[14*16]
	;t += x1[ 160] * wp[ 1*16]
		movaps	xmm6,[wp + 14*16*4]
		movaps	xmm4,[x1 + 160*4+4]
		movhps	xmm5,[x1 + 160*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		movaps	xmm5,xmm4
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[-128] * wp[ 1*16]
	;t -= x2[-128] * wp[14*16]
		movaps	xmm4,[x2 - 128*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[ -96] * wp[ 2*16]
	;v -= x2[ -96] * wp[14*16]
		movaps	xmm7,[wp +  2*16*4]
		movaps	xmm5,[x2 -  96*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[ 128] * wp[14*16]
	;v += x1[ 128] * wp[ 2*16]
		movaps	xmm4,[x1 + 128*4+4]
		movhps	xmm5,[x1 + 128*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[  96] * wp[13*16]
	;t += x1[  96] * wp[ 2*16]
		movaps	xmm6,[wp + 13*16*4]
		movaps	xmm4,[x1 +  96*4+4]
		movhps	xmm5,[x1 +  96*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		movaps	xmm5,xmm4
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[ -64] * wp[ 2*16]
	;t -= x2[ -64] * wp[13*16]
		movaps	xmm4,[x2 -  64*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[ -32] * wp[ 3*16]
	;v -= x2[ -32] * wp[13*16]
		movaps	xmm7,[wp +  3*16*4]
		movaps	xmm5,[x2 -  32*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[  64] * wp[13*16]
	;v += x1[  64] * wp[ 3*16]
		movaps	xmm4,[x1 +  64*4+4]
		movhps	xmm5,[x1 +  64*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[  32] * wp[12*16]
	;t += x1[  32] * wp[ 3*16]
		movaps	xmm6,[wp + 12*16*4]
		movaps	xmm4,[x1 +  32*4+4]
		movhps	xmm5,[x1 +  32*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		movaps	xmm5,xmm4
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[   0] * wp[ 3*16]
	;t -= x2[   0] * wp[12*16]
		movaps	xmm4,[x2 +   0*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[  32] * wp[ 4*16]
	;v -= x2[  32] * wp[12*16]
		movaps	xmm7,[wp +  4*16*4]
		movaps	xmm5,[x2 +  32*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[   0] * wp[12*16]
	;v += x1[   0] * wp[ 4*16]
		movaps	xmm4,[x1 +   0*4+4]
		movhps	xmm5,[x1 +   0*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[ -32] * wp[11*16]
	;t += x1[ -32] * wp[ 4*16]
		movaps	xmm6,[wp + 11*16*4]
		movaps	xmm4,[x1 -  32*4+4]
		movhps	xmm5,[x1 -  32*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		movaps	xmm5,xmm4
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[  64] * wp[ 4*16]
	;t -= x2[  64] * wp[11*16]
		movaps	xmm4,[x2 +  64*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[  96] * wp[ 5*16]
	;v -= x2[  96] * wp[11*16]
		movaps	xmm7,[wp +  5*16*4]
		movaps	xmm5,[x2 +  96*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[ -64] * wp[11*16]
	;v += x1[ -64] * wp[ 5*16]
		movaps	xmm4,[x1 -  64*4+4]
		movhps	xmm5,[x1 -  64*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[ -96] * wp[10*16]
	;t += x1[ -96] * wp[ 5*16]
		movaps	xmm6,[wp + 10*16*4]
		movaps	xmm4,[x1 -  96*4+4]
		movhps	xmm5,[x1 -  96*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		movaps	xmm5,xmm4
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[ 128] * wp[ 5*16]
	;t -= x2[ 128] * wp[10*16]
		movaps	xmm4,[x2 + 128*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[ 160] * wp[ 6*16]
	;v -= x2[ 160] * wp[10*16]
		movaps	xmm7,[wp +  6*16*4]
		movaps	xmm5,[x2 + 160*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[-128] * wp[10*16]
	;v += x1[-128] * wp[ 6*16]
		movaps	xmm4,[x1 - 128*4+4]
		movhps	xmm5,[x1 - 128*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[-160] * wp[ 9*16]
	;t += x1[-160] * wp[ 6*16]
		movaps	xmm6,[wp + 9*16*4]
		movaps	xmm4,[x1 - 160*4+4]
		movhps	xmm5,[x1 - 160*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		movaps	xmm5,xmm4
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[ 192] * wp[ 6*16]
	;t -= x2[ 192] * wp[ 9*16]
		movaps	xmm4,[x2 + 192*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[ 224] * wp[ 7*16]
	;v -= x2[ 224] * wp[ 9*16]
		movaps	xmm7,[wp +  7*16*4]
		movaps	xmm5,[x2 + 224*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[-192] * wp[ 9*16]
	;v += x1[-192] * wp[ 7*16]
		movaps	xmm4,[x1 - 192*4+4]
		movhps	xmm5,[x1 - 192*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[-224] * wp[ 8*16]
	;t += x1[-224] * wp[ 7*16]
		movaps	xmm6,[wp + 8*16*4]
		movaps	xmm4,[x1 - 224*4+4]
		movhps	xmm5,[x1 - 224*4-4]
		movss	xmm5,xmm4
		shufps	xmm4,xmm5,11000110B
		movaps	xmm5,xmm4
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[ 256] * wp[ 7*16]
	;t -= x2[ 256] * wp[ 8*16]
		movaps	xmm4,[x2 + 256*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x1[-256] * wp[ 8*16]
	;v -= x2[ 288] * wp[ 8*16]
%if 0
		movups	xmm5,[x1 - 256*4]
		shufps	xmm5,xmm5,00011011B
%else
		movaps	xmm5,[x1 - 256*4+4]
		movhps	xmm7,[x1 - 256*4-4]
		movss	xmm7,xmm5
		shufps	xmm5,xmm7,11000110B
%endif
		movaps	xmm4,[x2 + 288*4]
		mulps	xmm5,xmm6
		mulps	xmm4,xmm6
		addps	xmm0,xmm5
		subps	xmm3,xmm4

	;s *= wp[16*16]
	;u *= wp[16*16]
		movaps	xmm7,[wp + 16*16*4]
		mulps	xmm0,xmm7
		mulps	xmm2,xmm7

	;w1 = t - s
	;w2 = v - u
		movaps	xmm6,[wp + 17*16*4]
		movaps	xmm4,xmm1
		movaps	xmm5,xmm3
		subps	xmm4,xmm0		; w1
		subps	xmm5,xmm2		; w2
	;a[i * 2 + 1] = w1 * wp[17*16]
	;a[i * 2 + 33] = w2 * wp[17*16]
	;a[i * 2 + 0] = t + s
	;a[i * 2 + 32] = v + u
		mulps	xmm4,xmm6
		mulps	xmm5,xmm6
		addps	xmm1,xmm0
		addps	xmm3,xmm2
		movaps	xmm0,xmm1
		movaps	xmm2,xmm3
		unpcklps	xmm0,xmm4
		unpckhps	xmm1,xmm4
		unpcklps	xmm2,xmm5
		unpckhps	xmm3,xmm5

		movaps	[ecx+ebx*2-64 +  0*4],xmm0
		movaps	[ecx+ebx*2-64 +  4*4],xmm1
		movaps	[ecx+ebx*2-64 + 32*4],xmm2
		movaps	[ecx+ebx*2-64 + 36*4],xmm3
%if 0
		wp++;
		x1--;
		x2++;
	}
}
%endif

		add		edx,byte 4*4
		sub		ebx,byte 4*4
		jnc		near .lp0

%undef x1
%undef x2
%undef wp

		pop		ebx
		ret

;		2002/5/18	1st version by k.sakai, 1300clk@じーおん
proc 	window_subband_sub1_SSE2
		push	ebx

%assign	_P 1*4
		mov		edx,[esp+_P+4]	; x1
		mov		ecx,[esp+_P+8]	; a
		add		ecx,64
		mov		ebx,12*4		; i
		lea		eax,[edx-62*4]	; x2 = &x1[-62], aligned to 16byte
		sub		edx,15*4
;		jmp		short .lp0
%define	x1 edx
%define x2 ebx+eax
%define wp ebx+enwindow

		align 16
.lp0:
	;xmm1 = t =  x1[ 224] * wp[ 0*16]
	;xmm3 = v =  x1[ 256] * wp[ 0*16]
	;xmm0 = s =  x2[-224] * wp[ 0*16]
	;xmm2 = u =  x1[ 224] * wp[15*16]
		pshufd	xmm1,[x1 + 224*4+4],00000110B
		pinsrw	xmm1,[x1 + 224*4],6
		pinsrw	xmm1,[x1 + 224*4+2],7
		pshufd	xmm3,[x1 + 256*4+4],00000110B
		pinsrw	xmm3,[x1 + 256*4],6
		pinsrw	xmm3,[x1 + 256*4+2],7
		movaps	xmm0,[x2 - 224*4]
		movaps	xmm2,xmm1
		movaps	xmm7,[wp +  0*16*4]
		movaps	xmm6,[wp + 15*16*4]
		mulps	xmm1,xmm7				; t
		mulps	xmm3,xmm7				; v
		mulps	xmm0,xmm7				; s
		mulps	xmm2,xmm6				; u
	;u += x2[-192] * wp[ 0*16]
	;t -= x2[-192] * wp[15*16]
		movaps	xmm4,[x2 - 192*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[-160] * wp[ 1*16]
	;v -= x2[-160] * wp[15*16]
		movaps	xmm7,[wp +  1*16*4]
		movaps	xmm5,[x2 - 160*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[ 192] * wp[15*16]
	;v += x1[ 192] * wp[ 1*16]
		pshufd	xmm4,[x1 + 192*4+4],00000110B
		pinsrw	xmm4,[x1 + 192*4],6
		pinsrw	xmm4,[x1 + 192*4+2],7
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[ 160] * wp[14*16]
	;t += x1[ 160] * wp[ 1*16]
		movaps	xmm6,[wp + 14*16*4]
		pshufd	xmm5,[x1 + 160*4+4],00000110B
		pinsrw	xmm5,[x1 + 160*4],6
		pinsrw	xmm5,[x1 + 160*4+2],7
		movaps	xmm4,xmm5
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[-128] * wp[ 1*16]
	;t -= x2[-128] * wp[14*16]
		movaps	xmm4,[x2 - 128*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[ -96] * wp[ 2*16]
	;v -= x2[ -96] * wp[14*16]
		movaps	xmm7,[wp +  2*16*4]
		movaps	xmm5,[x2 -  96*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[ 128] * wp[14*16]
	;v += x1[ 128] * wp[ 2*16]
		pshufd	xmm4,[x1 + 128*4+4],00000110B
		pinsrw	xmm4,[x1 + 128*4],6
		pinsrw	xmm4,[x1 + 128*4+2],7
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[  96] * wp[13*16]
	;t += x1[  96] * wp[ 2*16]
		movaps	xmm6,[wp + 13*16*4]
		pshufd	xmm5,[x1 +  96*4+4],00000110B
		pinsrw	xmm5,[x1 +  96*4],6
		pinsrw	xmm5,[x1 +  96*4+2],7
		movaps	xmm4,xmm5
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[ -64] * wp[ 2*16]
	;t -= x2[ -64] * wp[13*16]
		movaps	xmm4,[x2 -  64*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[ -32] * wp[ 3*16]
	;v -= x2[ -32] * wp[13*16]
		movaps	xmm7,[wp +  3*16*4]
		movaps	xmm5,[x2 -  32*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[  64] * wp[13*16]
	;v += x1[  64] * wp[ 3*16]
		pshufd	xmm4,[x1 +  64*4+4],00000110B
		pinsrw	xmm4,[x1 +  64*4],6
		pinsrw	xmm4,[x1 +  64*4+2],7
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[  32] * wp[12*16]
	;t += x1[  32] * wp[ 3*16]
		movaps	xmm6,[wp + 12*16*4]
		pshufd	xmm5,[x1 +  32*4+4],00000110B
		pinsrw	xmm5,[x1 +  32*4],6
		pinsrw	xmm5,[x1 +  32*4+2],7
		movaps	xmm4,xmm5
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[   0] * wp[ 3*16]
	;t -= x2[   0] * wp[12*16]
		movaps	xmm4,[x2 +   0*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[  32] * wp[ 4*16]
	;v -= x2[  32] * wp[12*16]
		movaps	xmm7,[wp +  4*16*4]
		movaps	xmm5,[x2 +  32*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[   0] * wp[12*16]
	;v += x1[   0] * wp[ 4*16]
		pshufd	xmm4,[x1 +   0*4+4],00000110B
		pinsrw	xmm4,[x1 +   0*4],6
		pinsrw	xmm4,[x1 +   0*4+2],7
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[ -32] * wp[11*16]
	;t += x1[ -32] * wp[ 4*16]
		movaps	xmm6,[wp + 11*16*4]
		pshufd	xmm5,[x1 -  32*4+4],00000110B
		pinsrw	xmm5,[x1 -  32*4],6
		pinsrw	xmm5,[x1 -  32*4+2],7
		movaps	xmm4,xmm5
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[  64] * wp[ 4*16]
	;t -= x2[  64] * wp[11*16]
		movaps	xmm4,[x2 +  64*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[  96] * wp[ 5*16]
	;v -= x2[  96] * wp[11*16]
		movaps	xmm7,[wp +  5*16*4]
		movaps	xmm5,[x2 +  96*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[ -64] * wp[11*16]
	;v += x1[ -64] * wp[ 5*16]
		pshufd	xmm4,[x1 -  64*4+4],00000110B
		pinsrw	xmm4,[x1 -  64*4],6
		pinsrw	xmm4,[x1 -  64*4+2],7
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[ -96] * wp[10*16]
	;t += x1[ -96] * wp[ 5*16]
		movaps	xmm6,[wp + 10*16*4]
		pshufd	xmm5,[x1 -  96*4+4],00000110B
		pinsrw	xmm5,[x1 -  96*4],6
		pinsrw	xmm5,[x1 -  96*4+2],7
		movaps	xmm4,xmm5
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[ 128] * wp[ 5*16]
	;t -= x2[ 128] * wp[10*16]
		movaps	xmm4,[x2 + 128*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[ 160] * wp[ 6*16]
	;v -= x2[ 160] * wp[10*16]
		movaps	xmm7,[wp +  6*16*4]
		movaps	xmm5,[x2 + 160*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[-128] * wp[10*16]
	;v += x1[-128] * wp[ 6*16]
		pshufd	xmm4,[x1 - 128*4+4],00000110B
		pinsrw	xmm4,[x1 - 128*4],6
		pinsrw	xmm4,[x1 - 128*4+2],7
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[-160] * wp[ 9*16]
	;t += x1[-160] * wp[ 6*16]
		movaps	xmm6,[wp + 9*16*4]
		pshufd	xmm5,[x1 - 160*4+4],00000110B
		pinsrw	xmm5,[x1 - 160*4],6
		pinsrw	xmm5,[x1 - 160*4+2],7
		movaps	xmm4,xmm5
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[ 192] * wp[ 6*16]
	;t -= x2[ 192] * wp[ 9*16]
		movaps	xmm4,[x2 + 192*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x2[ 224] * wp[ 7*16]
	;v -= x2[ 224] * wp[ 9*16]
		movaps	xmm7,[wp +  7*16*4]
		movaps	xmm5,[x2 + 224*4]
		movaps	xmm4,xmm5
		mulps	xmm4,xmm7
		mulps	xmm5,xmm6
		addps	xmm0,xmm4
		subps	xmm3,xmm5
	;s += x1[-192] * wp[ 9*16]
	;v += x1[-192] * wp[ 7*16]
		pshufd	xmm4,[x1 - 192*4+4],00000110B
		pinsrw	xmm4,[x1 - 192*4],6
		pinsrw	xmm4,[x1 - 192*4+2],7
		mulps	xmm6,xmm4
		mulps	xmm4,xmm7
		addps	xmm0,xmm6
		addps	xmm3,xmm4

	;u += x1[-224] * wp[ 8*16]
	;t += x1[-224] * wp[ 7*16]
		movaps	xmm6,[wp + 8*16*4]
		pshufd	xmm5,[x1 - 224*4+4],00000110B
		pinsrw	xmm5,[x1 - 224*4],6
		pinsrw	xmm5,[x1 - 224*4+2],7
		movaps	xmm4,xmm5
		mulps	xmm4,xmm6
		mulps	xmm5,xmm7
		addps	xmm2,xmm4
		addps	xmm1,xmm5
	;u += x2[ 256] * wp[ 7*16]
	;t -= x2[ 256] * wp[ 8*16]
		movaps	xmm4,[x2 + 256*4]
		mulps	xmm7,xmm4
		mulps	xmm4,xmm6
		addps	xmm2,xmm7
		subps	xmm1,xmm4

	;s += x1[-256] * wp[ 8*16]
	;v -= x2[ 288] * wp[ 8*16]
		pshufd	xmm5,[x1 - 256*4+4],00000110B
		pinsrw	xmm5,[x1 - 256*4],6
		pinsrw	xmm5,[x1 - 256*4+2],7
		movaps	xmm4,[x2 + 288*4]
		mulps	xmm5,xmm6
		mulps	xmm4,xmm6
		addps	xmm0,xmm5
		subps	xmm3,xmm4

	;s *= wp[16*16]
	;u *= wp[16*16]
		movaps	xmm7,[wp + 16*16*4]
		mulps	xmm0,xmm7
		mulps	xmm2,xmm7

	;w1 = t - s
	;w2 = v - u
		movaps	xmm6,[wp + 17*16*4]
		movaps	xmm4,xmm1
		movaps	xmm5,xmm3
		subps	xmm4,xmm0		; w1
		subps	xmm5,xmm2		; w2
	;a[i * 2 + 1] = w1 * wp[17*16]
	;a[i * 2 + 33] = w2 * wp[17*16]
	;a[i * 2 + 0] = t + s
	;a[i * 2 + 32] = v + u
		mulps	xmm4,xmm6
		mulps	xmm5,xmm6
		addps	xmm1,xmm0
		addps	xmm3,xmm2
		movaps	xmm0,xmm1
		movaps	xmm2,xmm3
		unpcklps	xmm0,xmm4
		unpckhps	xmm1,xmm4
		unpcklps	xmm2,xmm5
		unpckhps	xmm3,xmm5

		movaps	[ecx+ebx*2-64 +  0*4],xmm0
		movaps	[ecx+ebx*2-64 +  4*4],xmm1
		movaps	[ecx+ebx*2-64 + 32*4],xmm2
		movaps	[ecx+ebx*2-64 + 36*4],xmm3

		add		edx,byte 4*4
		sub		ebx,byte 4*4
		jnc		near .lp0

%undef x1
%undef x2
%undef wp

		pop		ebx
		ret


%if 0
static void
window_subband_sub2(const sample_t * x1, FLOAT8 a[SBLIMIT])
{
	FLOAT8 s, t, u, v, w, x, y, z, xr;
	int	i;

	{
		t =   x1[  -31]            * enwindow[ 0*16+15];
		w =   x1[    1]            * enwindow[ 0*16+15];
		w += (x1[ -31] - x1[  33]) * enwindow[ 1*16+15];
		t += (x1[ -63] - x1[   1]) * enwindow[ 1*16+15];
		t += (x1[ -95] + x1[  33]) * enwindow[ 2*16+15];
		w += (x1[ -63] + x1[  65]) * enwindow[ 2*16+15];
		w += (x1[ -95] - x1[  97]) * enwindow[ 3*16+15];
		t += (x1[-127] - x1[  65]) * enwindow[ 3*16+15];
		t += (x1[-159] + x1[  97]) * enwindow[ 4*16+15];
		w += (x1[-127] + x1[ 129]) * enwindow[ 4*16+15];
		w += (x1[-159] - x1[ 161]) * enwindow[ 5*16+15];
		t += (x1[-191] - x1[ 129]) * enwindow[ 5*16+15];
		t += (x1[-223] + x1[ 161]) * enwindow[ 6*16+15];
		w += (x1[-191] + x1[ 193]) * enwindow[ 6*16+15];
		w += (x1[-223] - x1[ 225]) * enwindow[ 7*16+15];
		t += (x1[-255] - x1[ 193]) * enwindow[ 7*16+15];

		s =   x1[-239]             * enwindow[11*16+15];
		x =   x1[-207]             * enwindow[11*16+15];
		s +=  x1[-175]             * enwindow[10*16+15];
		x +=  x1[-143]             * enwindow[10*16+15];
		s +=  x1[-111]             * enwindow[ 9*16+15];
		x +=  x1[ -79]             * enwindow[ 9*16+15];
		s +=  x1[ -47]             * enwindow[ 8*16+15];
		x +=  x1[ -15]             * enwindow[ 8*16+15];
		s -=  x1[  17]             * enwindow[12*16+15];
		x -=  x1[  49]             * enwindow[12*16+15];
		s -=  x1[  81]             * enwindow[13*16+15];
		x -=  x1[ 113]             * enwindow[13*16+15];
		s -=  x1[ 145]             * enwindow[14*16+15];
		x -=  x1[ 177]             * enwindow[14*16+15];
		s -=  x1[ 209];
		x -=  x1[ 241];

		u = s - t;
		y = x - w;
		v = s + t;
		z = x + w;

		t = a[14];
		w = a[46];
		s = a[15] - t;
		x = a[47] - w;

		a[14] = v - t;	// A3
		a[15] = u - s;	// A2
		a[30] = u + s;	// A1
		a[31] = v + t;	// A0

		a[46] = z - w;	// A3
		a[47] = y - x;	// A2
		a[62] = y + x;	// A1
		a[63] = z + w;	// A0

	}
	for(i=0; i<2; i++){
		xr = a[28] - a[0];
		a[0] += a[28];
		a[28] = xr * enwindow[13 + 17*16];
		xr = a[29] - a[1];
		a[1] += a[29];
		a[29] = xr * enwindow[13 + 17*16];

		xr = a[26] - a[2];
		a[2] += a[26];
		a[26] = xr * enwindow[11 + 17*16];
		xr = a[27] - a[3];
		a[3] += a[27];
		a[27] = xr * enwindow[11 + 17*16];

		xr = a[24] - a[4];
		a[4] += a[24];
		a[24] = xr * enwindow[9 + 17*16];
		xr = a[25] - a[5];
		a[5] += a[25];
		a[25] = xr * enwindow[9 + 17*16];

		xr = a[22] - a[6];
		a[6] += a[22];
		a[22] = xr * SQRT2;
		xr = a[23] - a[7];
		a[7] += a[23];
		a[23] = xr * SQRT2 - a[7];
		a[7] -= a[6];
		a[22] -= a[7];
		a[23] -= a[22];

		xr = a[6];
		a[6] = a[31] - xr;
		a[31] = a[31] + xr;
		xr = a[7];
		a[7] = a[30] - xr;
		a[30] = a[30] + xr;
		xr = a[22];
		a[22] = a[15] - xr;
		a[15] = a[15] + xr;
		xr = a[23];
		a[23] = a[14] - xr;
		a[14] = a[14] + xr;

		xr = a[20] - a[8];
		a[8] += a[20];
		a[20] = xr * enwindow[5 + 17*16];
		xr = a[21] - a[9];
		a[9] += a[21];
		a[21] = xr * enwindow[5 + 17*16];

		xr = a[18] - a[10];
		a[10] += a[18];
		a[18] = xr * enwindow[3 + 17*16];
		xr = a[19] - a[11];
		a[11] += a[19];
		a[19] = xr * enwindow[3 + 17*16];

		xr = a[16] - a[12];
		a[12] += a[16];
		a[16] = xr * enwindow[1 + 17*16];
		xr = a[17] - a[13];
		a[13] += a[17];
		a[17] = xr * enwindow[1 + 17*16];

		xr = -a[20] + a[24];
		a[20] += a[24];
		a[24] = xr * enwindow[3 + 17*16];
		xr = -a[21] + a[25];
		a[21] += a[25];
		a[25] = xr * enwindow[3 + 17*16];

		xr = a[4] - a[8];
		a[4] += a[8];
		a[8] = xr * enwindow[3 + 17*16];
		xr = a[5] - a[9];
		a[5] += a[9];
		a[9] = xr * enwindow[3 + 17*16];

		xr = a[0] - a[12];
		a[0] += a[12];
		a[12] = xr * enwindow[11 + 17*16];
		xr = a[1] - a[13];
		a[1] += a[13];
		a[13] = xr * enwindow[11 + 17*16];
		xr = a[16] - a[28];
		a[16] += a[28];
		a[28] = xr * enwindow[11 + 17*16];
		xr = -a[17] + a[29];
		a[17] += a[29];
		a[29] = xr * enwindow[11 + 17*16];

		xr = SQRT2 * (a[2] - a[10]);
		a[2] += a[10];
		a[10] = xr;
		xr = SQRT2 * (a[3] - a[11]);
		a[3] += a[11];
		a[11] = xr;
		xr = SQRT2 * (-a[18] + a[26]);
		a[18] += a[26];
		a[26] = xr - a[18];
		xr = SQRT2 * (-a[19] + a[27]);
		a[19] += a[27];
		a[27] = xr - a[19];

		xr = a[2];
		a[19] -= a[3];
		a[3] -= xr;
		a[2] = a[31] - xr;
		a[31] += xr;
		xr = a[3];
		a[11] -= a[19];
		a[18] -= xr;
		a[3] = a[30] - xr;
		a[30] += xr;
		xr = a[18];
		a[27] -= a[11];
		a[19] -= xr;
		a[18] = a[15] - xr;
		a[15] += xr;

		xr = a[19];
		a[10] -= xr;
		a[19] = a[14] - xr;
		a[14] += xr;
		xr = a[10];
		a[11] -= xr;
		a[10] = a[23] - xr;
		a[23] += xr;
		xr = a[11];
		a[26] -= xr;
		a[11] = a[22] - xr;
		a[22] += xr;
		xr = a[26];
		a[27] -= xr;
		a[26] = a[7] - xr;
		a[7] += xr;

		xr = a[27];
		a[27] = a[6] - xr;
		a[6] += xr;

		xr = SQRT2 * (a[0] - a[4]);
		a[0] += a[4];
		a[4] = xr;
		xr = SQRT2 * (a[1] - a[5]);
		a[1] += a[5];
		a[5] = xr;
		xr = SQRT2 * (a[16] - a[20]);
		a[16] += a[20];
		a[20] = xr;
		xr = SQRT2 * (a[17] - a[21]);
		a[17] += a[21];
		a[21] = xr;

		xr = -SQRT2 * (a[8] - a[12]);
		a[8] += a[12];
		a[12] = xr - a[8];
		xr = -SQRT2 * (a[9] - a[13]);
		a[9] += a[13];
		a[13] = xr - a[9];
		xr = -SQRT2 * (a[25] - a[29]);
		a[25] += a[29];
		a[29] = xr - a[25];
		xr = -SQRT2 * (a[24] + a[28]);
		a[24] -= a[28];
		a[28] = xr - a[24];

		xr = a[24] - a[16];
		a[24] = xr;
		xr = a[20] - xr;
		a[20] = xr;
		xr = a[28] - xr;
		a[28] = xr;

		xr = a[25] - a[17];
		a[25] = xr;
		xr = a[21] - xr;
		a[21] = xr;
		xr = a[29] - xr;
		a[29] = xr;

		xr = a[17] - a[1];
		a[17] = xr;
		xr = a[9] - xr;
		a[9] = xr;
		xr = a[25] - xr;
		a[25] = xr;
		xr = a[5] - xr;
		a[5] = xr;
		xr = a[21] - xr;
		a[21] = xr;
		xr = a[13] - xr;
		a[13] = xr;
		xr = a[29] - xr;
		a[29] = xr;

		xr = a[1] - a[0];
		a[1] = xr;
		xr = a[16] - xr;
		a[16] = xr;
		xr = a[17] - xr;
		a[17] = xr;
		xr = a[8] - xr;
		a[8] = xr;
		xr = a[9] - xr;
		a[9] = xr;
		xr = a[24] - xr;
		a[24] = xr;
		xr = a[25] - xr;
		a[25] = xr;
		xr = a[4] - xr;
		a[4] = xr;
		xr = a[5] - xr;
		a[5] = xr;
		xr = a[20] - xr;
		a[20] = xr;
		xr = a[21] - xr;
		a[21] = xr;
		xr = a[12] - xr;
		a[12] = xr;
		xr = a[13] - xr;
		a[13] = xr;
		xr = a[28] - xr;
		a[28] = xr;
		xr = a[29] - xr;
		a[29] = xr;

		s = a[ 0]; t = a[31];
		a[ 0] = s + t;
		a[31] = t - s;
		s = a[ 1]; t = a[30];
		a[ 1] = s + t;
		a[30] = t - s;
		s = a[29]; t = a[ 2];
		a[29] = s + t;
		a[ 2] = t - s;
		s = a[28]; t = a[ 3];
		a[28] = s + t;
		a[ 3] = t - s;
		s = a[ 4]; t = a[27];
		a[ 4] = s + t;
		a[27] = t - s;
		s = a[ 5]; t = a[26];
		a[ 5] = s + t;
		a[26] = t - s;
		s = a[25]; t = a[ 6];
		a[25] = s + t;
		a[ 6] = t - s;
		s = a[24]; t = a[ 7];
		a[24] = s + t;
		a[ 7] = t - s;
		s = a[ 8]; t = a[23];
		a[ 8] = s + t;
		a[23] = t - s;
		s = a[ 9]; t = a[22];
		a[ 9] = s + t;
		a[22] = t - s;
		s = a[21]; t = a[10];
		a[21] = s + t;
		a[10] = t - s;
		s = a[20]; t = a[11];
		a[20] = s + t;
		a[11] = t - s;
		s = a[12]; t = a[19];
		a[12] = s + t;
		a[19] = t - s;
		s = a[13]; t = a[18];
		a[13] = s + t;
		a[18] = t - s;
		s = a[17]; t = a[14];
		a[17] = s + t;
		a[14] = t - s;
		s = a[16]; t = a[15];
		a[16] = s + t;
		a[15] = t - s;

		a += 32;
	}
	/*
	 * Compensate for inversion in the analysis filter
	 */
	for (i = 1; i < 32; i += 2) {
		a[i - 32] = -a[i - 32];
	}
}
%endif

		end

