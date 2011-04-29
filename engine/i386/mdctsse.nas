;
;   part of this code is origined from
;
;       GOGO-no-coda
;       Copyright (c) 2001           shigeo
;       Copyright (c) 2001,2002,2003 gogo-developer
;

%include "global.cfg"


	segment_data
	align	32
%define cx0	 0.98480775301220802032
%define cx1  0.64278760968653936292
%define cx2  0.34202014332566882393
%define cx3  0.93969262078590842791
%define cx4 -0.17364817766693030343
%define cx5 -0.76604444311897790243
%define cx6  0.86602540378443870761
%define cx7  0.50000000000000000000

%define mcx4 0.17364817766693030343
%define mcx5 0.76604444311897790243

;cof		dd	cx0, cx1, cx2, cx3, cx4, cx5, cx6, cx7
cof		dd	cx7, cx6, 0, 0x80000000

;			tc1, tc2, tc3, tc4
pc1		dd	cx0, cx6, cx1, cx2
pc2		dd	cx1,-cx6,-cx2, cx0
pc3		dd	cx2,-cx6, cx0,-cx1

;			ts5, ts5, ts6, ts8
ps1		dd	1.0,mcx4,mcx5, cx3
ps2		dd	1.0,mcx5,-cx3, cx4
ps3		dd -1.0, cx3, cx4,mcx5

;			ts1, ts2, ts3, ts4
ps12	dd	cx2, cx6, cx0, cx1
ps22	dd	cx1, cx6,-cx2,-cx0
ps32	dd	cx0,-cx6, cx1,-cx2

;		   -tc6, tc5, tc7, tc8
pc12	dd	1.0, cx3, cx4, cx5
pc22	dd -1.0,mcx5,-cx3,mcx4
pc32	dd -1.0,mcx4,mcx5,-cx3

	segment_text
;void mdct_long(FLOAT8 * out, FLOAT8 * in);

%define LOCAL_SIZE	4*4
;	2001/1/6 shigeo
;	522clk=>308clk

proc	mdct_long_SSE2
		mov			edx, [esp+4]		; edx = out
		mov			eax, [esp+8]		; eax = in
%assign LOCAL_SIZE	(16*4+16)
		sub			esp, byte LOCAL_SIZE
		lea			ecx, [esp+15]
		and			ecx, -16

		movups		xm0, [eax+0*4]		; xm0 = [in3:in2:in1:in0]
		movss		xm4, [eax+5*4]
		movss		xm2, [eax+6*4]
		movss		xm3, [eax+7*4]
		movss		xm1, [eax+8*4]
		punpckldq	xm2, xm4			; xm2 = [*:*:in5:in6]
		punpckldq	xm1, xm3			; xm1 = [*:*:in7:in8]
		punpcklqdq	xm1, xm2			; xm1 = [in5:in6:in7:in8]

		movups		xm2, [eax+9*4]		; xm2 = [in12:in11:in10:in9]

		movups		xm3, [eax+14*4]		; xm3 = [in17:in16:in15:in14]
		shufps		xm3, xm3, PACK(0,1,2,3)	; xm3 = [in14:in15:in16:in17]

		; keep
		movaps		[ecx], xm0			; [in3:in2:in1:in0]
		movaps		[ecx+16], xm1		; [in5:in6:in7:in8]
		movaps		[ecx+32], xm2		; [in12:in11:in10:in9]
		movaps		[ecx+48], xm3		; [in14:in15:in16:in17]

		addps		xm0, xm1			; xm0 = [ts8:ts7:ts6:ts5]
		subps		xm3, xm2			; xm3 = [tc4:tc3:tc2:tc1]

		movss		xm4, xm0			; xm4 = ts5
		movhlps		xm5, xm0			; xm5 = ts7
		addss		xm4, xm5			; xm4 = ts5+ts7
		pshufd		xm6, xm0, PACK(0,0,0,3)	; xm6 = ts8
		pshufd		xm7, xm0, PACK(0,0,0,1)	; xm7 = ts6
		subss		xm4, xm6			; xm4 = ts5+ts7-ts8
		subss		xm7, [eax+4*4]		; xm7 = ts6 - in4
		shufps		xm0, xm0, PACK(3,2,0,1)	; xm0 = [ts8:ts7:ts5:ts6]
		movss		xm5, xm3			; xm5 = tc1
		movhlps		xm6, xm3			; xm6 = tc3
		movss		xm1, xm4			; xm1 = ts5+ts7-ts8
		mulss		xm0, [cof]			; xm0 = [ts8:ts7:ts5:ts6*cx7]
		subss		xm4, xm7			; xm4 = out17
		subss		xm5, xm6			; xm5 = tc1-tc3
		mulss		xm1, [cof]			; xm1 = (ts5+ts7-ts8) * cx7
		pshufd		xm6, xm3, PACK(0,0,0,3)	; xm6 = tc4
		addss		xm1, xm7			; xm1 = st
		subss		xm5, xm6			; xm5 = tc1-tc3-tc4
		movss		[edx+17*4], xm4
		mulss		xm5, [cof+1*4]		; xm5 = ct
		movss		xm2, xm5			; xm2 = ct
		addss		xm5, xm1			; out5
		subss		xm2, xm1			; out6
		addss		xm0, [eax+4*4]		; xm0 = [ts8:ts7:ts5:ts6'=ts6*cx7+in4]
		movss		[edx+5*4], xm5
		movss		[edx+6*4], xm2
										; xm0 = [ts8:ts7:ts5:ts6]
		movaps		xm1, xm3			; xm1 = [tc4:tc3:tc2:tc1]
		movaps		xm2, xm0			; xm2 = [ts8:ts7:ts5:ts6]
										; xm3 = [tc4:tc3:tc2:tc1]
		movaps		xm4, xm0
		movaps		xm5, xm1
		mulps		xm0, [ps1]
		mulps		xm1, [pc1]
		mulps		xm2, [ps2]
		mulps		xm3, [pc2]
		mulps		xm4, [ps3]
		mulps		xm5, [pc3]

		; st = sum of xm0, ct = sum of xm1
		movaps		xm6, xm0
		movaps		xm7, xm1
		punpcklqdq	xm0, xm1			; [c1:c0:s1:s0]
		punpckhqdq	xm6, xm7			; [c3:c2:s3:s2]
		addps		xm0, xm6			; [c1+c3:c0+c2:s1+s3:s0+s2]
		pshufd		xm1, xm0, PACK(0,0,3,1)	; [*:*:c1+c3:s1+s3]
		pshufd		xm0, xm0, PACK(0,0,2,0)	; [*:*:c0+c2:s0+s2]
		addps		xm0, xm1			; [*:*:ct:st]
		pshufd		xm1, xm0, PACK(0,0,0,1)	; xm1 = st
		movss		xm6, xm1
		addss		xm1, xm0			; ct + st
		subss		xm6, xm0			; ct - st
		movss		[edx+1*4], xm1
		movss		[edx+2*4], xm6

		; out[1] <= ct = sum of xm2
		; out[1] <= st = sum of xm3
		; out[9] <= ct = sum of xm4
		; out[9] <= ct = sum of xm5

		movaps		xm0, xm2
		movaps		xm1, xm4
		punpcklqdq	xm0, xm3	; [s1:s0:c1:c0]
		punpcklqdq	xm1, xm5	; [s1:s0:c1:c0]'
		punpckhqdq	xm2, xm3	; [s3:s2:c3:c2]
		punpckhqdq	xm4, xm5	; [s3:s2:c3:c2]'
		addps		xm0, xm2	; [s1+s3:s0+s2:c1+c3:c2+c0]
		addps		xm4, xm1	; [s1+s3:s0+s2:c1+c3:c2+c0]'

		pshufd		xm2, xm0, PACK(2,3,0,1)	; [s0+s2:s1+s3:c2+c0:c1+c3]
		pshufd		xm1, xm4, PACK(2,3,0,1)
		xorps		xm7, xm7
		addps		xm0, xm2	; [st:st:ct:ct]
		addps		xm1, xm4	; [st:st:ct:ct]'
		movlps		xm7, [cof+2*4]	; [*:*:0x80000000:0]
		movhlps		xm2, xm0	; [*:*:st:st]
		movhlps		xm3, xm1	; [*:*:st:st]'
		xorps		xm0, xm7
		xorps		xm1, xm7
		addps		xm0, xm2	; [*:*:ct-st:ct+st]
		addps		xm1, xm3	; [*:*:ct-st:ct+st]'

%if 1	; ばらしたほうがよい
		movss		[edx+9*4], xm0
		psrlq		xm0, 32
		movss		[edx+13*4], xm1
		psrlq		xm1, 32
		movss		[edx+10*4], xm0
		movss		[edx+14*4], xm1
%else
		movlps		[edx+9*4], xm0
		movlps		[edx+13*4], xm1
%endif

		movaps		xm1, [ecx+16]		; [in5:in6:in7:in8]
		subps		xm1, [ecx]			; [in3:in2:in1:in0]
		movaps		xm3, [ecx+32]		; [in12:in11:in10:in9]
		addps		xm3, [ecx+48]		; [in14:in15:in16:in17]

		movss		xm4, xm3			; xm4 = tc5
		movhlps		xm5, xm3			; xm5 = tc7
		addss		xm4, xm5			; xm4 = tc5+tc7
		pshufd		xm6, xm3, PACK(0,0,0,3)	; xm6 = tc8
		pshufd		xm7, xm3, PACK(0,0,0,1)	; xm7 = tc6
		addss		xm4, xm6			; xm4 = tc5+tc7+tc8
		addss		xm7, [eax+13*4]		; xm7 = tc6 + in13
		shufps		xm3, xm3, PACK(3,2,0,1)	; xm3 = [tc8:tc7:tc5:tc6]
		movss		xm5, xm1			; xm5 = ts1
		movhlps		xm6, xm1			; xm6 = ts3
		movss		xm0, xm4			; xm0 = tc5+tc7+tc8
		mulss		xm3, [cof]			; xm3 = [tc8:tc7:tc5:tc6*cx7]
		addss		xm4, xm7			; xm4 = out0
		subss		xm5, xm6			; xm5 = ts1-ts3
		mulss		xm0, [cof]			; xm0 = (tc5+tc7+tc8) * cx7
		pshufd		xm6, xm1, PACK(0,0,0,3)	; xm6 = ts4
		subss		xm0, xm7			; xm0 = ct
		addss		xm5, xm6			; xm5 = ts1-ts3+ts4
		movss		[edx+0*4], xm4
		mulss		xm5, [cof+1*4]		; xm5 = st
		movss		xm2, xm5
		addss		xm5, xm0			; out5
		subss		xm0, xm2			; out6
		subss		xm3, [eax+13*4]		; xm3 = [tc8:tc7:tc5:-tc6'=tc6*cx7-in13]
		movss		[edx+11*4], xm5
		movss		[edx+12*4], xm0

		; xm3 = [tc8:tc7:tc5:-tc6]
		; xm1 = [ts4:ts3:ts2:ts1]

		movaps		xm0, xm1
		movaps		xm2, xm1
		movaps		xm4, xm3
		movaps		xm5, xm3

		mulps		xm0, [ps12]
		mulps		xm1, [ps22]
		mulps		xm2, [ps32]
		mulps		xm3, [pc12]
		mulps		xm4, [pc22]
		mulps		xm5, [pc32]

		; st = sum of xm0, ct = sum of xm3
		movaps		xm6, xm0
		movaps		xm7, xm3
		punpcklqdq	xm0, xm3			; [c1:c0:s1:s0]
		punpckhqdq	xm6, xm7			; [c3:c2:s3:s2]
		addps		xm0, xm6			; [c1+c3:c0+c2:s1+s3:s0+s2]
		pshufd		xm3, xm0, PACK(0,0,3,1)	; [*:*:c1+c3:s1+s3]
		pshufd		xm0, xm0, PACK(0,0,2,0)	; [*:*:c0+c2:s0+s2]
		addps		xm0, xm3			; [*:*:ct:st]
		pshufd		xm3, xm0, PACK(0,0,0,1)	; xm1 = st
		movss		xm6, xm3
		addss		xm3, xm0			; ct + st
		subss		xm6, xm0			; ct - st
		movss		[edx+3*4], xm3
		movss		[edx+4*4], xm6

		; out[7] <= ct = sum of xm4
		; out[7] <= st = sum of xm1
		; out[15] <= ct = sum of xm5
		; out[15] <= st = sum of xm2

		movaps		xm0, xm4
		movaps		xm3, xm5
		punpcklqdq	xm0, xm1	; [s1:s0:c1:c0]
		punpcklqdq	xm3, xm2
		punpckhqdq	xm4, xm1	; [s3:s2:c3:c2]
		punpckhqdq	xm5, xm2
		addps		xm0, xm4	; [s1+s3:s0+s2:c1+c3:c2+c0]
		addps		xm3, xm5	; [s1+s3:s0+s2:c1+c3:c2+c0]'

		pshufd		xm2, xm0, PACK(2,3,0,1)	; [s0+s2:s1+s3:c2+c0:c1+c3]
		pshufd		xm1, xm3, PACK(2,3,0,1)
		xorps		xm7, xm7
		addps		xm0, xm2	; [st:st:ct:ct]
		addps		xm1, xm3	; [st:st:ct:ct]'
		movlps		xm7, [cof+2*4]	; [*:*:0x80000000:0]
		movhlps		xm2, xm0	; [*:*:st:st]
		movhlps		xm3, xm1	; [*:*:st:st]'
		xorps		xm2, xm7
		xorps		xm3, xm7
		addps		xm0, xm2	; [*:*:ct-st:ct+st]
		addps		xm1, xm3	; [*:*:ct-st:ct+st]'

%if 1	; ばらしたほうがよい
		movss		[edx+7*4], xm0
		psrlq		xm0, 32
		movss		[edx+15*4], xm1
		psrlq		xm1, 32
		movss		[edx+8*4], xm0
		movss		[edx+16*4], xm1
%else
		movlps		[edx+7*4], xm0
		movlps		[edx+15*4], xm1
%endif
		add			esp, byte LOCAL_SIZE
		ret

; 遅かった(;;

%if 0
proc	mdct_long_SSE
		mov			edx, [esp+4]		; edx = out
		mov			eax, [esp+8]		; eax = in
;		mov			ecx, cof			; ecx = cx
		movups		xm0, [eax+0*4]		; xm0 = [in3:in2:in1:in0]
		movups		xm1, [eax+5*4]		; xm1 = [in8:in7:in6:in5]
		movups		xm2, [eax+9*4]		; xm2 = [in12:in11:in10:in9]
		movups		xm3, [eax+14*4]		; xm3 = [in17:in16:in15:in14]
		shufps		xm1, xm1, PACK(0,1,2,3)	; xm1 = [in5:in6:in7:in8]
		shufps		xm3, xm3, PACK(0,1,2,3)	; xm3 = [in14:in15:in16:in17]
		addps		xm0, xm1			; xm0 = [ts8:ts7:ts6:ts5]
		subps		xm3, xm2			; xm3 = [tc4:tc3:tc2:tc1]
		movss		xm4, xm0			; xm4 = ts5
		movhlps		xm5, xm0			; xm5 = ts7
		movaps		xm6, xm0
		addss		xm4, xm5			; xm4 = ts5+ts7
		shufps		xm6, xm6, PACK(0,0,0,3)	; xm6 = ts8
		movaps		xm7, xm0
		shufps		xm7, xm7, PACK(0,0,0,1)	; xm7 = ts6
		subss		xm4, xm6			; xm4 = ts5+ts7-ts8
		subss		xm7, [eax+4*4]		; xm7 = ts6 - in4
		shufps		xm0, xm0, PACK(3,2,0,1)	; xm0 = [ts8:ts7:ts5:ts6]
		movss		xm5, xm3			; xm5 = tc1
		movhlps		xm6, xm3			; xm6 = tc3
		movss		xm1, xm4			; xm1 = ts5+ts7-ts8
;		mulss		xm0, [ecx+7*4]		; xm0 = [ts8:ts7:ts5:ts6*cx7]
		mulss		xm0, [cof]			; xm0 = [ts8:ts7:ts5:ts6*cx7]
		subss		xm4, xm7			; xm4 = out17
		subss		xm5, xm6			; xm5 = tc1-tc3
		movaps		xm6, xm3
;		mulss		xm1, [ecx+7*4]		; xm1 = (ts5+ts7-ts8) * cx7
		mulss		xm1, [cof]			; xm1 = (ts5+ts7-ts8) * cx7
		shufps		xm6, xm6, PACK(0,0,0,3)	; xm6 = tc4
		addss		xm1, xm7			; xm1 = st
		subss		xm5, xm6			; xm5 = tc1-tc3-tc4
		movss		[edx+17*4], xm4
;		mulss		xm5, [ecx+6*4]		; xm5 = ct
		mulss		xm5, [cof+1*4]		; xm5 = ct
		movss		xm2, xm5			; xm2 = ct
		addss		xm5, xm1			; out5
		subss		xm2, xm1			; out6
		addss		xm0, [eax+4*4]		; xm0 = [ts8:ts7:ts5:ts6'=ts6*cx7+in4]
		movss		[edx+5*4], xm5
		movss		[edx+6*4], xm2
										; xm0 = [ts8:ts7:ts5:ts6]
		movaps		xm1, xm3			; xm1 = [tc4:tc3:tc2:tc1]
		movaps		xm2, xm0			; xm2 = [ts8:ts7:ts5:ts6]
										; xm3 = [tc4:tc3:tc2:tc1]
		movaps		xm4, xm0
		movaps		xm5, xm1
		mulps		xm0, [ps1]
		mulps		xm1, [pc1]
		movhlps		xm6, xm0			; [*:*:3:2]
		movhlps		xm7, xm1
		addps		xm0, xm6			; [*:*:3+1:2+0]
		addps		xm1, xm7
		mulps		xm2, [ps2]
		mulps		xm3, [pc2]
		mulps		xm4, [ps3]
		mulps		xm5, [pc3]
		movaps		xm6, xm0
		movaps		xm7, xm1
		shufps		xm6, xm6, PACK(0,0,0,1)	; [*:*:*:3+1]
		shufps		xm7, xm7, PACK(0,0,0,1)
		addss		xm0, xm6			; st
		addss		xm1, xm7			; ct
		movss		xm6, xm1
		addss		xm1, xm0			; ct + st
		subss		xm6, xm0			; ct - st
		movss		[edx+1*4], xm1
		movss		[edx+2*4], xm6

		movhlps		xm6, xm2
		movhlps		xm7, xm3
		addps		xm2, xm6
		addps		xm3, xm7
		movaps		xm6, xm2
		movaps		xm7, xm3
		shufps		xm6, xm6, PACK(0,0,0,1)
		shufps		xm7, xm7, PACK(0,0,0,1)
		addss		xm2, xm6
		addss		xm3, xm7

		movss		xm6, xm3
		addss		xm3, xm2			; ct + st
		subss		xm6, xm2			; ct - st
		movss		[edx+9*4], xm3
		movss		[edx+10*4], xm6

		movhlps		xm6, xm4
		movhlps		xm7, xm5
		addps		xm4, xm6
		addps		xm5, xm7
		movaps		xm6, xm4
		movaps		xm7, xm5
		shufps		xm6, xm6, PACK(0,0,0,1)
		shufps		xm7, xm7, PACK(0,0,0,1)
		addss		xm4, xm6
		addss		xm5, xm7

		movss		xm6, xm5
		addss		xm5, xm4			; ct + st
		subss		xm6, xm4			; ct - st
		movss		[edx+13*4], xm5
		movss		[edx+14*4], xm6

		ret
%endif

%if 0
proc	mdct_long_FPU
		mov			edx, [esp+4]		; edx = out
		mov			eax, [esp+8]		; eax = in
		sub			esp, byte LOCAL_SIZE
		mov			ecx, cof			; ecx = cx
%if 0
		fldz
		fld			dword [eax+0*4]
		fld			dword [eax+1*4]
		fld			dword [eax+2*4]
		fld			dword [eax+3*4]		; i3,i2,i1,i0
		fadd		dword [eax+5*4]		; s8,i2,i1,i0
		fxch		st1					; i2,s8,i1,i0
		fadd		dword [eax+6*4]		; s7,s8,i1,i0
		fxch		st2					; i1,s8,s7,i0
		fadd		dword [eax+7*4]		; s6,s8,s7,i0
		fxch		st3					; i0,s8,s7,s6
		fadd		dword [eax+8*4]		; s5,s8,s7,s6,*
		fxch		st4
		fstp		st0
%else
		fld			dword [eax+0*4]		; i0
		fadd		dword [eax+8*4]		; s5
		fld			dword [eax+1*4]		; i1
		fadd		dword [eax+7*4]		; s6,s5
		fld			dword [eax+2*4]		; i2
		fadd		dword [eax+6*4]		; s7,s6,s5
		fld			dword [eax+3*4]		; i3
		fadd		dword [eax+5*4]		; s8,s7,s6,s5
%endif
		fld			dword [eax+4*4]		; i4,s8,s7,s6,s5
		fld			st4					; s5,i4,s8,s7,s6,s5
		fadd		st3					; s5+s7,i4,s8,s7,s6,s5
		fld			st4					; s6,s5+s7,i4,s8,s7,s6,s5
		fsub		st2					; s6-i4,s5+s7,i4,s8,s7,s6,s5
		fxch		st1					; s5+s7,s6-i4,i4,s8,s7,s6,s5
		fsub		st3					; s5+s7-s8,s6-i4,i4,s8,s7,s6,s5
		fld			dword [ecx+7*4]		; cx7,s5+s7-s8,s6-i4,i4,s8,s7,s6,s5
		fmul		st1					; (s5+s7-s8)*cx7,s5+s7-s8,s6-i4,i4,s8,s7,s6,s5
		fxch		st1					; s5+s7-s8,(s5+s7-s8)*cx7,s6-i4,i4,s8,s7,s6,s5
		fsub		st2					; out17,(s5+s7-s8)*cx7,s6-i4,i4,s8,s7,s6,s5
		fxch		st1					; (s5+s7-s8)*cx7,out17,s6-i4,i4,s8,s7,s6,s5
		faddp		st2					; out17,st,i4,s8,s7,s6,s5
		fld			dword [eax+17*4]	; i17
		fsub		dword [eax+9*4]		; tc1,out17,st,i4,s8,s7,s6,s5
		fxch		st7					; s5,out17,st,i4,s8,s7,s6,tc1
		fxch		st3					; i4,out17,st,s5,s8,s7,s6,tc1
;無駄
		fstp		st0					; out17,st,s5,i4,s7,s6,tc1

		fld			dword [eax+15*4]
		fsub		dword [eax+11*4]	; tc3,out17,st,s5,s8,s7,s6,tc1
		fxch		st1					; out17,tc3,st,s5,s8,s7,s6,tc1
		fstp		dword [edx+17*4]	; tc3,st,s5,s8,s7,s6,tc1
		fst			dword [esp+4]

		; [esp+4] = tc3

		fld			dword [eax+14*4]
		fsub		dword [eax+12*4]	; tc4,tc3,st,s5,s8,s7,s6,tc1
		fxch		st7					; tc1,tc3,st,s5,s8,s7,s6,tc4
		fst			dword [esp]

		; [esp] = tc1

		fsubrp		st1, st0			; tc1-tc3,st,s5,s8,s7,s6,tc4
		fld			dword [eax+16*4]
		fsub		dword [eax+10*4]	; i16-i10,tc1-tc3,st,s5,s8,s7,s6,tc4
		fxch		st1					; tc1-tc3,i16-i10,st,s5,s8,s7,s6,tc4
		fsubrp		st7, st0			; i16-i10,st,s5,s8,s7,s6,tc1-tc3-tc4
		fld			dword [ecx+6*4]		; cx6,i16-i10,st,s5,s8,s7,s6,tc1-tc3-tc4
		fmul		st7, st0			; cx6,i16-i10,st,s5,s8,s7,s6,ct
		fmulp		st1					; tc2,st,s5,s8,s7,s6,ct
		fld			st6					; ct,tc2,st,s5,s8,s7,s6,ct
		fsub		st2					; out6,tc2,st,s5,s8,s7,s6,ct
		fxch		st2					; st,tc2,out6,s5,s8,s7,s6,ct
		faddp		st7, st0			; tc2,out6,s5,s8,s7,s6,out5
		fxch		st5					; s6,out6,s5,s8,s7,tc2,out5
		fmul		dword [ecx+7*4]		; s6*cx7,out6,s5,s8,s7,tc2,out5
		fxch		st6					; out5,out6,s5,s8,s7,tc2,s6*cx7
		fstp		dword [edx+5*4]		; out6,s5,s8,s7,tc2,s6*cx7
		fstp		dword [edx+6*4]		; s5,s8,s7,tc2,s6*cx7

		fxch		st4					; s6*cx7,s8,s7,tc2,s5
		fadd		dword [eax+4*4]		; ts6,s8,s7,tc2,s5
		fld			dword [esp]			; tc1,ts6,s8,s7,tc2,s5
		fmul		dword [ecx+0*4]		; tc1*cx0,ts6,s8,s7,tc2,s5
		fld			dword [esp+4]		; tc3,tc1*cx0,ts6,s8,s7,tc2,s5
		fmul		dword [ecx+1*4]		; tc3*cx1,tc1*cx0,ts6,s8,s7,tc2,s5
		fxch		st1					; tc1*cx0,tc3*cx1,ts6,s8,s7,tc2,s5
		fadd		st5					; tc1*cx0+tc2,tc3*cx1,ts6,s8,s7,tc2,s5
		fld			dword [eax+14*4]
		fsub		dword [eax+12*4]	; tc4,tc1*cx0+tc2,tc3*cx1,ts6,s8,s7,tc2,s5
		fxch		st1					; tc1*cx0+tc2,tc4,tc3*cx1,ts6,s8,s7,tc2,s5
		faddp		st2					; tc4,tc1*cx0+tc2+tc3*cx1,ts6,s8,s7,tc2,s5
		fld			st0					; tc4,tc4,tc1*cx0+tc2+tc3*cx1,ts6,s8,s7,tc2,s5
		fmul		dword [ecx+2*4]		; tc4*cx2,tc4,tc1*cx0+tc2+tc3*cx1,ts6,s8,s7,tc2,s5
		faddp		st2					; tc4,ct,ts6,s8,s7,tc2,s5
		fstp		dword [esp+8]		; ct,ts6,s8,s7,tc2,s5

		; [esp+8] = tc4

		fld			st5					; s5,ct,s6,s8,s7,tc2,s5
		fmul		dword [ecx+4*4]		; s5*cx4,ct,s6,s8,s7,tc2,s5
		fld			st4					; s7,s5*cx4,ct,s6,s8,s7,tc2,s5
		fmul		dword [ecx+5*4]		; s7*cx5,s5*cx4,ct,s6,s8,s7,tc2,s5
		fxch		st1					; s5*cx4,s7*cx5,ct,s6,s8,s7,tc2,s5
		fsub		st3					; s5*cx4-s6,s7*cx5,ct,s6,s8,s7,tc2,s5
		faddp		st1					; s5*cx4-s6+s7*cx5,ct,s6,s8,s7,tc2,s5
		fld			st3					; s8,s5*cx4-s6+s7*cx5,ct,s6,s8,s7,tc2,s5
		fmul		dword [ecx+3*4]		; s8*cx3,s5*cx4-s6+s7*cx5,ct,s6,s8,s7,tc2,s5
		fsubrp		st1, st0			; st,ct,s6,s8,s7,tc2,s5
		fld			st1					; ct,st,ct,s6,s8,s7,tc2,s5
		fadd		st1					; out1,st,ct,s6,s8,s7,tc2,s5
		fxch		st1					; st,out1,ct,s6,s8,s7,tc2,s5
		fsubp		st2, st0			; out1,out2,s6,s8,s7,tc2,s5
		fstp		dword [edx+1*4]		; out2,s6,s8,s7,tc2,s5
		fstp		dword [edx+2*4]		; s6,s8,s7,tc2,s5

		fld			dword [esp]			; tc1,s6,s8,s7,tc2,s5
		fld			dword [esp+4]		; tc3,tc1,s6,s8,s7,tc2,s5
		fmul		dword [ecx+2*4]		; tc3*cx2,tc1,s6,s8,s7,tc2,s5
		fld			dword [esp+8]		; tc4,tc3*cx2,tc1,s6,s8,s7,tc2,s5
		fmul		dword [ecx+0*4]		; tc4*cx0,tc3*cx2,tc1,s6,s8,s7,tc2,s5
		fxch		st2					; tc1,tc3*cx2,tc4*cx0,s6,s8,s7,tc2,s5
		fmul		dword [ecx+1*4]		; tc1*cx1,tc3*cx2,tc4*cx0,s6,s8,s7,tc2,s5
		fxch		st1					; tc3*cx2,tc1*cx1,tc4*cx0,s6,s8,s7,tc2,s5
		fadd		st6					; tc2+tc3*cx2,tc1*cx1,tc4*cx0,s6,s8,s7,tc2,s5
		fxch		st1					; tc1*cx1,tc2+tc3*cx2,tc4*cx0,s6,s8,s7,tc2,s5
		faddp		st2					; tc2+tc3*cx2,tc1*cx1+tc4*cx0,s6,s8,s7,tc2,s5
		fsubp		st1, st0			; ct,s6,s8,s7,tc2,s5
		fld			dword [ecx+5*4]		; cx5,ct,s6,s8,s7,tc2,s5
		fmul		st6					; s5*cx5,ct,s6,s8,s7,tc2,s5
		fld			st4					; s7,s5*cx5,ct,s6,s8,s7,tc2,s5
		fmul		dword [ecx+3*4]		; s7*cx3,s5*cx5,ct,s6,s8,s7,tc2,s5
		faddp		st1, st0			; s5*cx5+s7*cx3,ct,s6,s8,s7,tc2,s5
		fld			st3					; s8,s5*cx5+s7*cx3,ct,s6,s8,s7,tc2,s5
		fmul		dword [ecx+4*4]		; s8*cx4,s5*cx5+s7*cx3,ct,s6,s8,s7,tc2,s5
		fxch		st1					; s5*cx5+s7*cx3,s8*cx4,ct,s6,s8,s7,tc2,s5
		fsub		st3					; s5*cx5+s7*cx3-s6,s8*cx4,ct,s6,s8,s7,tc2,s5
		fsubp		st1					; st,ct,s6,s8,s7,tc2,s5
		fld			st1					; ct,st,ct,s6,s8,s7,tc2,s5
		fadd		st1					; out9,st,ct,s6,s8,s7,tc2,s5
		fxch		st1					; st,out9,ct,s6,s8,s7,tc2,s5
		fsubp		st2, st0			; out9,out10,s6,s8,s7,tc2,s5
		fstp		dword [edx+9*4]		; out10,s6,s8,s7,tc2,s5
		fstp		dword [edx+10*4]	; s6,s8,s7,tc2,s5

		fld			dword [esp]			; tc1,s6,s8,s7,tc2,s5
		fmul		dword [ecx+2*4]		; tc1*cx2,s6,s8,s7,tc2,s5
		fld			dword [esp+4]		; tc3,tc1*cx2,s6,s8,s7,tc2,s5
		fmul		dword [ecx+0*4]		; tc3*cx0,tc1*cx2,s6,s8,s7,tc2,s5
		fld			dword [esp+8]		; tc4,tc3*cx0,tc1*cx2,s6,s8,s7,tc2,s5
		fmul		dword [ecx+1*4]		; tc4*cx1,tc3*cx0,tc1*cx2,s6,s8,s7,tc2,s5
		fxch		st2					; tc1*cx2,tc3*cx0,tc4*cx1,s6,s8,s7,tc2,s5
		fsubrp		st6					; tc3*cx0,tc4*cx1,s6,s8,s7,tc1*cx2-tc2,s5
		fxch		st6					; s5,tc4*cx1,s6,s8,s7,tc1*cx2-tc2,tc3*cx0
		fmul		dword [ecx+3*4]		; s5*cx3,tc4*cx1,s6,s8,s7,tc1*cx2-tc2,tc3*cx0
		fxch		st1					; tc4*cx1,s5*cx3,s6,s8,s7,tc1*cx2-tc2,tc3*cx0
		fsubp		st5, st0			; s5*cx3,s6,s8,s7,tc1*cx2-tc2-tc4*cx1,tc3*cx0
		fsubrp		st1					; s5*cx3-s6,s8,s7,tc1*cx2-tc2-tc4*cx1,tc3*cx0
		fxch		st3					; tc1*cx2-tc2-tc4*cx1,s8,s7,s5*cx3-s6,tc3*cx0
		faddp		st4					; s8,s7,s5*cx3-s6,ct
		fmul		dword [ecx+5*4]		; s8*cx5,s7,s5*cx3-s6,ct
		fxch		st1					; s7,s8*cx5,s5*cx3-s6,ct
		fmul		dword [ecx+4*4]		; s7*cx4,s8*cx5,s5*cx3-s6,ct
		fxch		st2					; s5*cx3-s6,s8*cx5,s7*cx4,ct
		faddp		st2					; s8*cx5,s5*cx3-s6+s7*cx4,ct
		fsubp		st1, st0			; st,ct
		fld			st1					; ct,st,ct
		fadd		st1					; out13,st,ct
		fxch		st1					; st,out13,ct
		fsubp		st2, st0			; out13,out14
		fstp		dword [edx+13*4]	; out14
		fstp		dword [edx+14*4]
.exit:
		add			esp, byte LOCAL_SIZE

		ret
%endif

		end
