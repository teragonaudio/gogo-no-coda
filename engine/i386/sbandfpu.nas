;
;   part of this code is origined from
;
;       GOGO-no-coda
;       Copyright (C) 2002 k.sakai
;       Copyright (c) 2002,2003 gogo-developer
;

%include "global.cfg"

        segment_data
		externdef	enwindow

        segment_text
;		2002/5/17	1st version by k.sakai, 3000clk@河童, 2500clk@じーおん, 1800clk@明日論
proc 	window_subband_sub1_FPU
		push	ebx
		push	ebp
		push	edi

%assign	_P 3*4
		mov		ebp,[esp+_P+4]	; x1
		mov		eax,[esp+_P+8]	; a
		mov		ebx,14*4		; i
		lea		edi,[ebp-62*4]	; x2 = &x1[-62]
		sub		ebp,ebx
		jmp		short .lp0
%define	x1 ebp
%define x2 ebx+edi
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
; t, v, s, u
	;t =  x1[ 224] * wp[ 0*16]
	;v =  x1[ 256] * wp[ 0*16]
	;s =  x2[-224] * wp[ 0*16]
	;u =  x1[ 224] * wp[15*16]
		fld		dword [x1 + 224*4]
		fld		dword [x1 + 256*4]
		fld		dword [x2 - 224*4]
		fld		st2						; x1[ 224], x1[ 256], x2[-224], x1[ 224]
		fld		dword [wp + 15*16*4]	; x1[ 224], x1[ 256], x2[-224], x1[ 224], wp[15]
		fmul	st1,st0					; x1[ 224], x1[ 256], x2[-224], u, wp[15]
		fld		dword [wp +  0*16*4]	; x1[ 224], x1[ 256], x2[-224], u, wp[15], wp[ 0]
		fmul	st5,st0					; t, x1[ 256], x2[-224], u, wp[15], wp[ 0]
		fmul	st4,st0					; t, v, x2[-224], u, wp[15], wp[ 0]
		fmul	st3,st0					; t, v, s, u, wp[15], wp[ 0]
	;u += x2[-192] * wp[ 0*16]
	;t -= x2[-192] * wp[15*16]
		fld		dword [x2 - 192*4]		; t, v, s, u, wp[15], wp[ 0], x2[-192]
		fmul	st1,st0
		fmul	st0,st2					; t, v, s, u, wp[15], u, t
		fsubp	st6,st0					; t, v, s, u, wp[15], u
		faddp	st2,st0					; t, v, s, u, wp[15]

	;s += x2[-160] * wp[ 1*16]
	;v -= x2[-160] * wp[15*16]
		fld		dword [wp +  1*16*4]
		fxch							; t, v, s, u, wp[1], wp[15]
		fld		dword [x2 - 160*4]
		fld		st0						; t, v, s, u, wp[1], wp[15], x2[-160], x2[-160]
		fmul	st0,st3					; t, v, s, u, wp[1], wp[15], x2[-160], s
		faddp	st5,st0					; t, v, s, u, wp[1], wp[15], x2[-160]
		fmul	st0,st1					; t, v, s, u, wp[1], wp[15], v
		fsubp	st5,st0				 	; t, v, s, u, wp[1], wp[15]
	;s += x1[ 192] * wp[15*16]
	;v += x1[ 192] * wp[ 1*16]
		fld		dword [x1 + 192*4]	 	; t, v, s, u, wp[1], wp[15], x1[ 192]
		fmul	st1,st0				 	; t, v, s, u, wp[1], s, x1[ 192]
		fmul	st0,st2			 		; t, v, s, u, wp[1], s, v
		faddp	st5,st0
		faddp	st3,st0			 		; t, v, s, u, wp[1]

	;u += x1[ 160] * wp[14*16]
	;t += x1[ 160] * wp[ 1*16]
		fld		dword [wp + 14*16*4]
		fxch							; t, v, s, u, wp[14], wp[ 1]
		fld		dword [x1 + 160*4]
		fld		st0						; t, v, s, u, wp[14], wp[ 1], x1[160], x1[160]
		fmul	st0,st3					; t, v, s, u, wp[14], wp[ 1], x1[160], u
		faddp	st4,st0					; t, v, s, u, wp[14], wp[ 1], x1[160]
		fmul	st0,st1					; t, v, s, u, wp[14], wp[ 1], t
		faddp	st6,st0				 	; t, v, s, u, wp[14], wp[ 1]
	;u += x2[-128] * wp[ 1*16]
	;t -= x2[-128] * wp[14*16]
		fld		dword [x2 - 128*4]	 	; t, v, s, u, wp[14], wp[ 1], x2[-128]
		fmul	st1,st0				 	; t, v, s, u, wp[14], u, x2[-128]
		fmul	st0,st2			 		; t, v, s, u, wp[14], u, t
		fsubp	st6,st0
		faddp	st2,st0			 		; t, v, s, u, wp[14]

	;s += x2[ -96] * wp[ 2*16]
	;v -= x2[ -96] * wp[14*16]
		fld		dword [wp +  2*16*4]
		fxch							; t, v, s, u, wp[2], wp[14]
		fld		dword [x2 -  96*4]
		fld		st0						; t, v, s, u, wp[2], wp[14], x2, x2
		fmul	st0,st3					; t, v, s, u, wp[2], wp[14], x2, s
		faddp	st5,st0					; t, v, s, u, wp[2], wp[14], x2
		fmul	st0,st1					; t, v, s, u, wp[2], wp[14], v
		fsubp	st5,st0				 	; t, v, s, u, wp[2], wp[14]
	;s += x1[ 128] * wp[14*16]
	;v += x1[ 128] * wp[ 2*16]
		fld		dword [x1 + 128*4]	 	; t, v, s, u, wp[2], wp[14], x1
		fmul	st1,st0				 	; t, v, s, u, wp[2], s, x1
		fmul	st0,st2			 		; t, v, s, u, wp[2], s, v
		faddp	st5,st0
		faddp	st3,st0			 		; t, v, s, u, wp[2]

	;u += x1[  96] * wp[13*16]
	;t += x1[  96] * wp[ 2*16]
		fld		dword [wp + 13*16*4]
		fxch							; t, v, s, u, wp[13], wp[ 2]
		fld		dword [x1 +  96*4]
		fld		st0						; t, v, s, u, wp[13], wp[ 2], x1, x1
		fmul	st0,st3					; t, v, s, u, wp[13], wp[ 2], x1, u
		faddp	st4,st0					; t, v, s, u, wp[13], wp[ 2], x1
		fmul	st0,st1					; t, v, s, u, wp[13], wp[ 2], t
		faddp	st6,st0				 	; t, v, s, u, wp[13], wp[ 2]
	;u += x2[ -64] * wp[ 2*16]
	;t -= x2[ -64] * wp[13*16]
		fld		dword [x2 -  64*4]	 	; t, v, s, u, wp[13], wp[ 2], x2
		fmul	st1,st0				 	; t, v, s, u, wp[13], u, x2
		fmul	st0,st2			 		; t, v, s, u, wp[13], u, t
		fsubp	st6,st0
		faddp	st2,st0			 		; t, v, s, u, wp[13]

	;s += x2[ -32] * wp[ 3*16]
	;v -= x2[ -32] * wp[13*16]
		fld		dword [wp +  3*16*4]
		fxch							; t, v, s, u, wp[3], wp[13]
		fld		dword [x2 -  32*4]
		fld		st0						; t, v, s, u, wp[3], wp[13], x2, x2
		fmul	st0,st3					; t, v, s, u, wp[3], wp[13], x2, s
		faddp	st5,st0					; t, v, s, u, wp[3], wp[13], x2
		fmul	st0,st1					; t, v, s, u, wp[3], wp[13], v
		fsubp	st5,st0				 	; t, v, s, u, wp[3], wp[13]
	;s += x1[  64] * wp[13*16]
	;v += x1[  64] * wp[ 3*16]
		fld		dword [x1 +  64*4]	 	; t, v, s, u, wp[3], wp[13], x1
		fmul	st1,st0				 	; t, v, s, u, wp[3], s, x1
		fmul	st0,st2			 		; t, v, s, u, wp[3], s, v
		faddp	st5,st0
		faddp	st3,st0			 		; t, v, s, u, wp[3]

	;u += x1[  32] * wp[12*16]
	;t += x1[  32] * wp[ 3*16]
		fld		dword [wp + 12*16*4]
		fxch							; t, v, s, u, wp[12], wp[ 3]
		fld		dword [x1 +  32*4]
		fld		st0						; t, v, s, u, wp[12], wp[ 3], x1, x1
		fmul	st0,st3					; t, v, s, u, wp[12], wp[ 3], x1, u
		faddp	st4,st0					; t, v, s, u, wp[12], wp[ 3], x1
		fmul	st0,st1					; t, v, s, u, wp[12], wp[ 3], t
		faddp	st6,st0				 	; t, v, s, u, wp[12], wp[ 3]
	;u += x2[   0] * wp[ 3*16]
	;t -= x2[   0] * wp[12*16]
		fld		dword [x2 +   0*4]	 	; t, v, s, u, wp[12], wp[ 3], x2
		fmul	st1,st0				 	; t, v, s, u, wp[12], u, x2
		fmul	st0,st2			 		; t, v, s, u, wp[12], u, t
		fsubp	st6,st0
		faddp	st2,st0			 		; t, v, s, u, wp[12]

	;s += x2[  32] * wp[ 4*16]
	;v -= x2[  32] * wp[12*16]
		fld		dword [wp +  4*16*4]
		fxch							; t, v, s, u, wp[4], wp[12]
		fld		dword [x2 +  32*4]
		fld		st0						; t, v, s, u, wp[4], wp[12], x2, x2
		fmul	st0,st3					; t, v, s, u, wp[4], wp[12], x2, s
		faddp	st5,st0					; t, v, s, u, wp[4], wp[12], x2
		fmul	st0,st1					; t, v, s, u, wp[4], wp[12], v
		fsubp	st5,st0				 	; t, v, s, u, wp[4], wp[12]
	;s += x1[   0] * wp[12*16]
	;v += x1[   0] * wp[ 4*16]
		fld		dword [x1 +   0*4]	 	; t, v, s, u, wp[4], wp[12], x1
		fmul	st1,st0				 	; t, v, s, u, wp[4], s, x1
		fmul	st0,st2			 		; t, v, s, u, wp[4], s, v
		faddp	st5,st0
		faddp	st3,st0			 		; t, v, s, u, wp[4]

	;u += x1[ -32] * wp[11*16]
	;t += x1[ -32] * wp[ 4*16]
		fld		dword [wp + 11*16*4]
		fxch							; t, v, s, u, wp[11], wp[ 4]
		fld		dword [x1 -  32*4]
		fld		st0						; t, v, s, u, wp[11], wp[ 4], x1, x1
		fmul	st0,st3					; t, v, s, u, wp[11], wp[ 4], x1, u
		faddp	st4,st0					; t, v, s, u, wp[11], wp[ 4], x1
		fmul	st0,st1					; t, v, s, u, wp[11], wp[ 4], t
		faddp	st6,st0				 	; t, v, s, u, wp[11], wp[ 4]
	;u += x2[  64] * wp[ 4*16]
	;t -= x2[  64] * wp[11*16]
		fld		dword [x2 +  64*4]	 	; t, v, s, u, wp[11], wp[ 4], x2
		fmul	st1,st0				 	; t, v, s, u, wp[11], u, x2
		fmul	st0,st2			 		; t, v, s, u, wp[11], u, t
		fsubp	st6,st0
		faddp	st2,st0			 		; t, v, s, u, wp[11]

	;s += x2[  96] * wp[ 5*16]
	;v -= x2[  96] * wp[11*16]
		fld		dword [wp +  5*16*4]
		fxch							; t, v, s, u, wp[5], wp[11]
		fld		dword [x2 +  96*4]
		fld		st0						; t, v, s, u, wp[5], wp[11], x2, x2
		fmul	st0,st3					; t, v, s, u, wp[5], wp[11], x2, s
		faddp	st5,st0					; t, v, s, u, wp[5], wp[11], x2
		fmul	st0,st1					; t, v, s, u, wp[5], wp[11], v
		fsubp	st5,st0				 	; t, v, s, u, wp[5], wp[11]
	;s += x1[ -64] * wp[11*16]
	;v += x1[ -64] * wp[ 5*16]
		fld		dword [x1 -  64*4]	 	; t, v, s, u, wp[5], wp[11], x1
		fmul	st1,st0				 	; t, v, s, u, wp[5], s, x1
		fmul	st0,st2			 		; t, v, s, u, wp[5], s, v
		faddp	st5,st0
		faddp	st3,st0			 		; t, v, s, u, wp[5]

	;u += x1[ -96] * wp[10*16]
	;t += x1[ -96] * wp[ 5*16]
		fld		dword [wp + 10*16*4]
		fxch							; t, v, s, u, wp[10], wp[ 5]
		fld		dword [x1 -  96*4]
		fld		st0						; t, v, s, u, wp[10], wp[ 5], x1, x1
		fmul	st0,st3					; t, v, s, u, wp[10], wp[ 5], x1, u
		faddp	st4,st0					; t, v, s, u, wp[10], wp[ 5], x1
		fmul	st0,st1					; t, v, s, u, wp[10], wp[ 5], t
		faddp	st6,st0				 	; t, v, s, u, wp[10], wp[ 5]
	;u += x2[ 128] * wp[ 5*16]
	;t -= x2[ 128] * wp[10*16]
		fld		dword [x2 + 128*4]	 	; t, v, s, u, wp[10], wp[ 5], x2
		fmul	st1,st0				 	; t, v, s, u, wp[10], u, x2
		fmul	st0,st2			 		; t, v, s, u, wp[10], u, t
		fsubp	st6,st0
		faddp	st2,st0			 		; t, v, s, u, wp[10]

	;s += x2[ 160] * wp[ 6*16]
	;v -= x2[ 160] * wp[10*16]
		fld		dword [wp +  6*16*4]
		fxch							; t, v, s, u, wp[6], wp[10]
		fld		dword [x2 + 160*4]
		fld		st0						; t, v, s, u, wp[6], wp[10], x2, x2
		fmul	st0,st3					; t, v, s, u, wp[6], wp[10], x2, s
		faddp	st5,st0					; t, v, s, u, wp[6], wp[10], x2
		fmul	st0,st1					; t, v, s, u, wp[6], wp[10], v
		fsubp	st5,st0				 	; t, v, s, u, wp[6], wp[10]
	;s += x1[-128] * wp[10*16]
	;v += x1[-128] * wp[ 6*16]
		fld		dword [x1 - 128*4]	 	; t, v, s, u, wp[6], wp[10], x1
		fmul	st1,st0				 	; t, v, s, u, wp[6], s, x1
		fmul	st0,st2			 		; t, v, s, u, wp[6], s, v
		faddp	st5,st0
		faddp	st3,st0			 		; t, v, s, u, wp[6]

	;u += x1[-160] * wp[ 9*16]
	;t += x1[-160] * wp[ 6*16]
		fld		dword [wp + 9*16*4]
		fxch							; t, v, s, u, wp[9], wp[ 6]
		fld		dword [x1 - 160*4]
		fld		st0						; t, v, s, u, wp[9], wp[ 6], x1, x1
		fmul	st0,st3					; t, v, s, u, wp[9], wp[ 6], x1, u
		faddp	st4,st0					; t, v, s, u, wp[9], wp[ 6], x1
		fmul	st0,st1					; t, v, s, u, wp[9], wp[ 6], t
		faddp	st6,st0				 	; t, v, s, u, wp[9], wp[ 6]
	;u += x2[ 192] * wp[ 6*16]
	;t -= x2[ 192] * wp[ 9*16]
		fld		dword [x2 + 192*4]	 	; t, v, s, u, wp[9], wp[ 6], x2
		fmul	st1,st0				 	; t, v, s, u, wp[9], u, x2
		fmul	st0,st2			 		; t, v, s, u, wp[9], u, t
		fsubp	st6,st0
		faddp	st2,st0			 		; t, v, s, u, wp[9]

	;s += x2[ 224] * wp[ 7*16]
	;v -= x2[ 224] * wp[ 9*16]
		fld		dword [wp +  7*16*4]
		fxch							; t, v, s, u, wp[7], wp[9]
		fld		dword [x2 + 224*4]
		fld		st0						; t, v, s, u, wp[7], wp[9], x2, x2
		fmul	st0,st3					; t, v, s, u, wp[7], wp[9], x2, s
		faddp	st5,st0					; t, v, s, u, wp[7], wp[9], x2
		fmul	st0,st1					; t, v, s, u, wp[7], wp[9], v
		fsubp	st5,st0				 	; t, v, s, u, wp[7], wp[9]
	;s += x1[-192] * wp[ 9*16]
	;v += x1[-192] * wp[ 7*16]
		fld		dword [x1 - 192*4]	 	; t, v, s, u, wp[7], wp[9], x1
		fmul	st1,st0				 	; t, v, s, u, wp[7], s, x1
		fmul	st0,st2			 		; t, v, s, u, wp[7], s, v
		faddp	st5,st0
		faddp	st3,st0			 		; t, v, s, u, wp[7]

	;u += x1[-224] * wp[ 8*16]
	;t += x1[-224] * wp[ 7*16]
		fld		dword [wp + 8*16*4]
		fxch							; t, v, s, u, wp[8], wp[ 7]
		fld		dword [x1 - 224*4]
		fld		st0						; t, v, s, u, wp[8], wp[ 7], x1, x1
		fmul	st0,st3					; t, v, s, u, wp[8], wp[ 7], x1, u
		faddp	st4,st0					; t, v, s, u, wp[8], wp[ 7], x1
		fmul	st0,st1					; t, v, s, u, wp[8], wp[ 7], t
		faddp	st6,st0				 	; t, v, s, u, wp[8], wp[ 7]
	;u += x2[ 256] * wp[ 7*16]
	;t -= x2[ 256] * wp[ 8*16]
		fld		dword [x2 + 256*4]	 	; t, v, s, u, wp[8], wp[ 7], x2
		fmul	st1,st0				 	; t, v, s, u, wp[8], u, x2
		fmul	st0,st2			 		; t, v, s, u, wp[8], u, t
		fsubp	st6,st0
		faddp	st2,st0			 		; t, v, s, u, wp[8]

	;s += x1[-256] * wp[ 8*16]
	;v -= x2[ 288] * wp[ 8*16]
		fld		dword [x1 - 256*4]
		fld		dword [x2 + 288*4]		; t, v, s, u, wp[8], x1, x2
		fmul	st0,st2					; t, v, s, u, wp[8], x1, v
		fsubp	st5,st0					; t, v, s, u, wp[8], x1
		fmulp	st1,st0					; t, v, s, u, s
		faddp	st2,st0				 	; t, v, s, u

	;s *= wp[16*16]
	;u *= wp[16*16]
		fld		dword [wp + 16*16*4] 	; t, v, s, u, wp[16]
		fmul	st2,st0
		fmulp	st1,st0			 		; t, v, s, u

	;w1 = t - s
	;w2 = v - u
		fld		st3
		fsub	st0,st2			 		; t, v, s, u, w1
		fld		st3
		fsub	st0,st2		 			; t, v, s, u, w1, w2
	;a[i * 2 + 1] = w1 * wp[17*16]
	;a[i * 2 + 33] = w2 * wp[17*16]
		fld		dword [wp + 17*16*4] 	; t, v, s, u, w1, w2, wp[17]
		fmul	st2,st0
		fmulp	st1,st0			 		; t, v, s, u, w1, w2
		fxch
		fstp	dword [eax+ebx*2 +  1*4]
		fstp	dword [eax+ebx*2 + 33*4]
	;a[i * 2 + 32] = v + u
	;a[i * 2 + 0] = t + s
		faddp	st2,st0
		faddp	st2,st0					; t, v
		fstp	dword [eax+ebx*2 + 32*4]
		fstp	dword [eax+ebx*2 +  0*4]
%if 0
		wp++;
		x1--;
		x2++;
	}
}
%endif

		add		ebp,byte 4
		sub		ebx,byte 4
		jnc		near .lp0

%undef x1
%undef x2
%undef wp

		pop		edi
		pop		ebp
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

