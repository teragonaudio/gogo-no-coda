;
;   part of this code is origined from
;
;       GOGO-no-coda
;       Copyright (c) 2003 Kei Ishida
;       Copyright (c) 2003 gogo-developer
;

%include "global.cfg"

        segment_data
		externdef	enwindow

        segment_text
;		2003/01/03	kei, 1200clk@K7-500
;					測定した範囲において、a に関して window_subband_sub1_C 
;					との差異が最大で0.5%程でます。
;		2003/01/04	計算順序を window_subband_sub1_C に近くして差異が最大を0.1%程に。
; static void window_subband_sub1_C(const float *x1, float *a)
proc 	window_subband_sub1_E3DN
		push	edi
%assign	_P 1*4
		mov		eax, [esp+_P+4]	; x1
		mov		edx, [esp+_P+8]	; a
		mov		ecx, 14*4		; i
		lea		edi, [eax-62*4]	; x2 = &x1[-62]
		sub		eax, byte 15*4
%define	x1(n) (eax+(n)*4)
%define x2(n) (edi+ecx+(n)*4)
%define wp(n) (enwindow+ecx+(n)*16*4)
%define a(n)  (edx+ecx*2+(n)*4)

		loopalignK7	16
%if 1; わかりづらいバージョン
	.lp:
		pswapd	mm2, [x1(224)]  ; u
		movq	mm7, [wp(15)]
		movq	mm3, mm2		; t
		pfmul	mm2, mm7
		movq	mm4, [wp(0)]
		movq	mm6, [x2(-192)]
		movq	mm0, [x2(-224)] ; s
		pswapd	mm1, [x1(256)]	; v
		pfmul	mm7, mm6
		pfmul	mm6, mm4
		pfmul	mm3, mm4
		pfmul	mm0, mm4
		pfmul	mm1, mm4
		pfadd	mm2, mm6
		pfsub	mm3, mm7
		
		movq	mm4, [x2(-160)]
		pswapd	mm6, [x1(160)]
		movq	mm5, mm4
		movq	mm7, mm6
		pfmul	mm4, [wp(1)]
		pfmul	mm6, [wp(14)]
		pfmul	mm5, [wp(15)]
		pfmul	mm7, [wp(1)]
		pfadd	mm0, mm4
		pfadd	mm2, mm6
		pfsub	mm1, mm5
		pfadd	mm3, mm7
		pswapd	mm4, [x1(192)]
		movq	mm6, [x2(-128)]
		movq	mm5, mm4
		movq	mm7, mm6
		pfmul	mm4, [wp(15)]
		pfmul	mm6, [wp(1)]
		pfmul	mm5, [wp(1)]
		pfmul	mm7, [wp(14)]
		pfadd	mm0, mm4
		pfadd	mm2, mm6
		pfadd	mm1, mm5
		pfsub	mm3, mm7

		movq	mm4, [x2(-96)]
		pswapd	mm6, [x1(96)]
		movq	mm5, mm4
		movq	mm7, mm6
		pfmul	mm4, [wp(2)]
		pfmul	mm6, [wp(13)]
		pfmul	mm5, [wp(14)]
		pfmul	mm7, [wp(2)]
		pfadd	mm0, mm4
		pfadd	mm2, mm6
		pfsub	mm1, mm5
		pfadd	mm3, mm7
		pswapd	mm4, [x1(128)]
		movq	mm6, [x2(-64)]
		movq	mm5, mm4
		movq	mm7, mm6
		pfmul	mm4, [wp(14)]
		pfmul	mm6, [wp(2)]
		pfmul	mm5, [wp(2)]
		pfmul	mm7, [wp(13)]
		pfadd	mm0, mm4
		pfadd	mm2, mm6
		pfadd	mm1, mm5
		pfsub	mm3, mm7

		movq	mm4, [x2(-32)]
		pswapd	mm6, [x1(32)]
		movq	mm5, mm4
		movq	mm7, mm6
		pfmul	mm4, [wp(3)]
		pfmul	mm6, [wp(12)]
		pfmul	mm5, [wp(13)]
		pfmul	mm7, [wp(3)]
		pfadd	mm0, mm4
		pfadd	mm2, mm6
		pfsub	mm1, mm5
		pfadd	mm3, mm7
		pswapd	mm4, [x1(64)]
		movq	mm6, [x2(0)]
		movq	mm5, mm4
		movq	mm7, mm6
		pfmul	mm4, [wp(13)]
		pfmul	mm6, [wp(3)]
		pfmul	mm5, [wp(3)]
		pfmul	mm7, [wp(12)]
		pfadd	mm0, mm4
		pfadd	mm2, mm6
		pfadd	mm1, mm5
		pfsub	mm3, mm7

		movq	mm4, [x2(32)]
		pswapd	mm6, [x1(-32)]
		movq	mm5, mm4
		movq	mm7, mm6
		pfmul	mm4, [wp(4)]
		pfmul	mm6, [wp(11)]
		pfmul	mm5, [wp(12)]
		pfmul	mm7, [wp(4)]
		pfadd	mm0, mm4
		pfadd	mm2, mm6
		pfsub	mm1, mm5
		pfadd	mm3, mm7
		pswapd	mm4, [x1(0)]
		movq	mm6, [x2(64)]
		movq	mm5, mm4
		movq	mm7, mm6
		pfmul	mm4, [wp(12)]
		pfmul	mm6, [wp(4)]
		pfmul	mm5, [wp(4)]
		pfmul	mm7, [wp(11)]
		pfadd	mm0, mm4
		pfadd	mm2, mm6
		pfadd	mm1, mm5
		pfsub	mm3, mm7

		movq	mm4, [x2(96)]
		pswapd	mm6, [x1(-96)]
		movq	mm5, mm4
		movq	mm7, mm6
		pfmul	mm4, [wp(5)]
		pfmul	mm6, [wp(10)]
		pfmul	mm5, [wp(11)]
		pfmul	mm7, [wp(5)]
		pfadd	mm0, mm4
		pfadd	mm2, mm6
		pfsub	mm1, mm5
		pfadd	mm3, mm7
		pswapd	mm4, [x1(-64)]
		movq	mm6, [x2(128)]
		movq	mm5, mm4
		movq	mm7, mm6
		pfmul	mm4, [wp(11)]
		pfmul	mm6, [wp(5)]
		pfmul	mm5, [wp(5)]
		pfmul	mm7, [wp(10)]
		pfadd	mm0, mm4
		pfadd	mm2, mm6
		pfadd	mm1, mm5
		pfsub	mm3, mm7

		movq	mm4, [x2(160)]
		pswapd	mm6, [x1(-160)]
		movq	mm5, mm4
		movq	mm7, mm6
		pfmul	mm4, [wp(6)]
		pfmul	mm6, [wp(9)]
		pfmul	mm5, [wp(10)]
		pfmul	mm7, [wp(6)]
		pfadd	mm0, mm4
		pfadd	mm2, mm6
		pfsub	mm1, mm5
		pfadd	mm3, mm7
		pswapd	mm4, [x1(-128)]
		movq	mm6, [x2(192)]
		movq	mm5, mm4
		movq	mm7, mm6
		pfmul	mm4, [wp(10)]
		pfmul	mm6, [wp(6)]
		pfmul	mm5, [wp(6)]
		pfmul	mm7, [wp(9)]
		pfadd	mm0, mm4
		pfadd	mm1, mm5
		movq	mm4, [x2(224)]
		movq	mm5, mm4
		pfadd	mm2, mm6
		pfsub	mm3, mm7
		pfmul	mm4, [wp(7)]
		pfmul	mm5, [wp(9)]
		pswapd	mm6, [x1(-224)]
		movq	mm7, mm6
		pfadd	mm0, mm4
		pfsub	mm1, mm5
		pswapd	mm4, [x1(-192)]
		movq	mm5, mm4
		pfmul	mm6, [wp(8)]
		pfmul	mm4, [wp(9)]
		pfmul	mm7, [wp(7)]
		pfmul	mm5, [wp(7)]
		pfadd	mm2, mm6
		pfadd	mm0, mm4
		pfadd	mm3, mm7
		pfadd	mm1, mm5
		movq	mm5, [wp(8)]
		movq	mm6, [x2(256)]
		pswapd	mm4, [x1(-256)]
		movq	mm7, mm6
		pfmul	mm4, mm5
		pfmul	mm6, [wp(7)]
		pfmul	mm5, [x2(288)]
		pfmul	mm7, [wp(8)]
		pfadd	mm0, mm4
		pfadd	mm2, mm6
		pfsub	mm1, mm5
		pfsub	mm3, mm7

		pfmul	mm0, [wp(16)]	; s *= wp[16*16]
		pfmul	mm2, [wp(16)]	; u *= wp[16*16]
		movq	mm6, mm3 ; t
		movq	mm4, mm1 ; v
		pfsub	mm3, mm0 ; w1 = t - s
		pfsub	mm1, mm2 ; w2 = v - u
		pfadd	mm0, mm6 ; t + s
		pfadd	mm2, mm4 ; v + u
		pfmul	mm3, [wp(17)] ; w1 * wp[17*16]
		pfmul	mm1, [wp(17)] ; w2 * wp[17*16]
		add		eax, byte 2*4
		sub		ecx, byte 2*4
		movq	mm5, mm0
		movq	mm7, mm2
		punpckldq	mm0, mm3
		punpckldq	mm2, mm1
		punpckhdq	mm5, mm3
		punpckhdq	mm7, mm1
		movq	[a(0)+16], mm0
		movq	[a(32)+16], mm2
		movq	[a(2)+16], mm5
		movq	[a(34)+16], mm7
		jnc		near .lp
%else ; ちょっとはわかりやすいバージョン
	.lp:
		movq	mm0, [x2(-224)] ; s
		pswapd	mm1, [x1(256)]	; v
		pswapd	mm2, [x1(224)]  ; u
		movq	mm3, mm2		; t

		pfmul	mm0, [wp(0)]
		pfmul	mm1, [wp(0)]

		movq	mm7, [wp(15)]
		pfmul	mm2, mm7
		pfmul	mm3, [wp(0)]
		movq	mm6, [x2(-192)]
		pfmul	mm7, mm6
		pfmul	mm6, [wp(0)]
		pfadd	mm2, mm6
		pfsub	mm3, mm7
		
		movq	mm4, [x2(-160)]
		movq	mm5, mm4
		pfmul	mm4, [wp(1)]
		pfmul	mm5, [wp(15)]
		pfadd	mm0, mm4
		pfsub	mm1, mm5
		pswapd	mm4, [x1(192)]
		movq	mm5, mm4
		pfmul	mm4, [wp(15)]
		pfmul	mm5, [wp(1)]
		pfadd	mm0, mm4
		pfadd	mm1, mm5

		pswapd	mm6, [x1(160)]
		movq	mm7, mm6
		pfmul	mm6, [wp(14)]
		pfmul	mm7, [wp(1)]
		pfadd	mm2, mm6
		pfadd	mm3, mm7
		movq	mm6, [x2(-128)]
		movq	mm7, mm6
		pfmul	mm6, [wp(1)]
		pfmul	mm7, [wp(14)]
		pfadd	mm2, mm6
		pfsub	mm3, mm7

		movq	mm4, [x2(-96)]
		movq	mm5, mm4
		pfmul	mm4, [wp(2)]
		pfmul	mm5, [wp(14)]
		pfadd	mm0, mm4
		pfsub	mm1, mm5
		pswapd	mm4, [x1(128)]
		movq	mm5, mm4
		pfmul	mm4, [wp(14)]
		pfmul	mm5, [wp(2)]
		pfadd	mm0, mm4
		pfadd	mm1, mm5

		pswapd	mm6, [x1(96)]
		movq	mm7, mm6
		pfmul	mm6, [wp(13)]
		pfmul	mm7, [wp(2)]
		pfadd	mm2, mm6
		pfadd	mm3, mm7
		movq	mm6, [x2(-64)]
		movq	mm7, mm6
		pfmul	mm6, [wp(2)]
		pfmul	mm7, [wp(13)]
		pfadd	mm2, mm6
		pfsub	mm3, mm7

		movq	mm4, [x2(-32)]
		movq	mm5, mm4
		pfmul	mm4, [wp(3)]
		pfmul	mm5, [wp(13)]
		pfadd	mm0, mm4
		pfsub	mm1, mm5
		pswapd	mm4, [x1(64)]
		movq	mm5, mm4
		pfmul	mm4, [wp(13)]
		pfmul	mm5, [wp(3)]
		pfadd	mm0, mm4
		pfadd	mm1, mm5

		pswapd	mm6, [x1(32)]
		movq	mm7, mm6
		pfmul	mm6, [wp(12)]
		pfmul	mm7, [wp(3)]
		pfadd	mm2, mm6
		pfadd	mm3, mm7
		movq	mm6, [x2(0)]
		movq	mm7, mm6
		pfmul	mm6, [wp(3)]
		pfmul	mm7, [wp(12)]
		pfadd	mm2, mm6
		pfsub	mm3, mm7

		movq	mm4, [x2(32)]
		movq	mm5, mm4
		pfmul	mm4, [wp(4)]
		pfmul	mm5, [wp(12)]
		pfadd	mm0, mm4
		pfsub	mm1, mm5
		pswapd	mm4, [x1(0)]
		movq	mm5, mm4
		pfmul	mm4, [wp(12)]
		pfmul	mm5, [wp(4)]
		pfadd	mm0, mm4
		pfadd	mm1, mm5

		pswapd	mm6, [x1(-32)]
		movq	mm7, mm6
		pfmul	mm6, [wp(11)]
		pfmul	mm7, [wp(4)]
		pfadd	mm2, mm6
		pfadd	mm3, mm7
		movq	mm6, [x2(64)]
		movq	mm7, mm6
		pfmul	mm6, [wp(4)]
		pfmul	mm7, [wp(11)]
		pfadd	mm2, mm6
		pfsub	mm3, mm7

		movq	mm4, [x2(96)]
		movq	mm5, mm4
		pfmul	mm4, [wp(5)]
		pfmul	mm5, [wp(11)]
		pfadd	mm0, mm4
		pfsub	mm1, mm5
		pswapd	mm4, [x1(-64)]
		movq	mm5, mm4
		pfmul	mm4, [wp(11)]
		pfmul	mm5, [wp(5)]
		pfadd	mm0, mm4
		pfadd	mm1, mm5

		pswapd	mm6, [x1(-96)]
		movq	mm7, mm6
		pfmul	mm6, [wp(10)]
		pfmul	mm7, [wp(5)]
		pfadd	mm2, mm6
		pfadd	mm3, mm7
		movq	mm6, [x2(128)]
		movq	mm7, mm6
		pfmul	mm6, [wp(5)]
		pfmul	mm7, [wp(10)]
		pfadd	mm2, mm6
		pfsub	mm3, mm7

		movq	mm4, [x2(160)]
		movq	mm5, mm4
		pfmul	mm4, [wp(6)]
		pfmul	mm5, [wp(10)]
		pfadd	mm0, mm4
		pfsub	mm1, mm5
		pswapd	mm4, [x1(-128)]
		movq	mm5, mm4
		pfmul	mm4, [wp(10)]
		pfmul	mm5, [wp(6)]
		pfadd	mm0, mm4
		pfadd	mm1, mm5

		pswapd	mm6, [x1(-160)]
		movq	mm7, mm6
		pfmul	mm6, [wp(9)]
		pfmul	mm7, [wp(6)]
		pfadd	mm2, mm6
		pfadd	mm3, mm7
		movq	mm6, [x2(192)]
		movq	mm7, mm6
		pfmul	mm6, [wp(6)]
		pfmul	mm7, [wp(9)]
		pfadd	mm2, mm6
		pfsub	mm3, mm7

		movq	mm4, [x2(224)]
		movq	mm5, mm4
		pfmul	mm4, [wp(7)]
		pfmul	mm5, [wp(9)]
		pfadd	mm0, mm4
		pfsub	mm1, mm5
		pswapd	mm4, [x1(-192)]
		movq	mm5, mm4
		pfmul	mm4, [wp(9)]
		pfmul	mm5, [wp(7)]
		pfadd	mm0, mm4
		pfadd	mm1, mm5

		pswapd	mm6, [x1(-224)]
		movq	mm7, mm6
		pfmul	mm6, [wp(8)]
		pfmul	mm7, [wp(7)]
		pfadd	mm2, mm6
		pfadd	mm3, mm7
		movq	mm6, [x2(256)]
		movq	mm7, mm6
		pfmul	mm6, [wp(7)]
		pfmul	mm7, [wp(8)]
		pfadd	mm2, mm6
		pfsub	mm3, mm7

		movq	mm5, [wp(8)]
		pswapd	mm4, [x1(-256)]
		pfmul	mm4, mm5
		pfmul	mm5, [x2(288)]
		pfadd	mm0, mm4
		pfsub	mm1, mm5

		pfmul	mm0, [wp(16)]	; s *= wp[16*16]
		pfmul	mm2, [wp(16)]	; u *= wp[16*16]
		movq	mm6, mm3 ; t
		movq	mm4, mm1 ; v
		pfsub	mm3, mm0 ; w1 = t - s
		pfsub	mm1, mm2 ; w2 = v - u
		pfadd	mm0, mm6 ; t + s
		pfadd	mm2, mm4 ; v + u
		pfmul	mm3, [wp(17)] ; w1 * wp[17*16]
		pfmul	mm1, [wp(17)] ; w2 * wp[17*16]
		movq	mm5, mm0
		movq	mm7, mm2
		punpckldq	mm0, mm3
		punpckldq	mm2, mm1
		punpckhdq	mm5, mm3
		punpckhdq	mm7, mm1
		movq	[a(0)], mm0
		movq	[a(32)], mm2
		movq	[a(2)], mm5
		movq	[a(34)], mm7
		
		add		eax, byte 2*4
		sub		ecx, byte 2*4
		jnc		near .lp
%endif
	.exit:
		femms
		pop		edi
		ret

		end

