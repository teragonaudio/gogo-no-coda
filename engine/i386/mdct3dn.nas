;
;   part of this code is origined from
;
;       GOGO-no-coda
;       Copyright (c) 2002,2003 gogo-developer
;

%include "global.cfg"

;	externdef		wins

	segment_data
	align	8
%define cx0	  0.98480775301220802032
%define cx1   0.64278760968653936292
%define cx2   0.34202014332566882393
%define cx3   0.93969262078590842791
%define cx4  -0.17364817766693030343
%define cx5  -0.76604444311897790243
%define cx6   0.86602540378443870761
%define cx7   0.50000000000000000000
%define mcx0 -0.98480775301220802032
%define mcx1 -0.64278760968653936292
%define mcx2 -0.34202014332566882393
%define mcx3 -0.93969262078590842791
%define mcx4  0.17364817766693030343
%define mcx5  0.76604444311897790243
%define mcx6 -0.86602540378443870761
%define mcx7 -0.50000000000000000000

D_cx5_cx1	dd cx1, cx5
D_mcx7_cx6	dd cx6, mcx7
D_m1_0		dd 0, -1
D_mcx4_cx0	dd cx0, mcx4
D_MSB_0		dd 0, 0x80000000
D_mcx3_cx2	dd cx2, mcx3
D_cx4_cx0	dd cx0, cx4
D_mcx5_cx1	dd cx1, mcx5
D_mcx0_cx4	dd cx4, mcx0
D_mcx6_cx7	dd cx7, mcx6
D_0_m1		dd -1, 0
D_cx1_cx5	dd cx5, cx1
D_mcx2_cx3	dd cx3, mcx2
D_cx0_cx4	dd cx4, cx0
D_mcx1_cx5	dd cx5, mcx1

	segment_text

;	2002/04/23 initial ver.  160clk@K7-500 by kei
;	2002/04/23 147clk@K7-500 by kei
;void mdct_long(FLOAT8 * out, FLOAT8 * in);

%define in(n)	(eax+(n)*4)
%define out(n)	(edx+(n)*4)
;%define	cx(n)	(wins+(n)*4+12*4)
%define	local(n)	(ecx+(n)*8)

proc	mdct_long_E3DN
		mov			eax, [esp+8]		; eax = in
		mov			edx, [esp+4]		; edx = out
%assign LOCAL_SIZE	(8*3+8)
		sub			esp, byte LOCAL_SIZE
		pswapd		mm4, [in(16)]
		pswapd		mm6, [in(7)]
		pswapd		mm5, [in(14)]
		pswapd		mm7, [in(5)]
		lea			ecx, [esp+7]
		movd		mm2, [in(4)]
		pfsub		mm4, [in(9)] ; tc2:tc1
		pfadd		mm6, [in(0)] ; ts6:ts5
		pfsub		mm5, [in(11)] ; tc4:tc3
		pfadd		mm7, [in(2)] ; ts8:ts7
		movq		mm1, [D_cx5_cx1]
		movq		mm3, mm4		
		and			ecx, -8
		psllq		mm2, 32
		punpckldq	mm4, mm6 ; ts5:tc1
		punpckhdq	mm3, mm6
		pfsubr		mm6, mm2
		movq		mm0, mm5 ; 
		pfmul		mm3, [D_mcx7_cx6]
		punpckldq	mm5, mm7 ; ts7:tc3
		pand		mm6, [D_m1_0] ; -(ts6-in[4]):0
		punpckhdq	mm0, mm7 ; ts8:tc4 
		movq		mm7, [D_mcx4_cx0]
		pfsub		mm3, mm2 ; -ts6:tc2
		movq		mm2, [D_MSB_0]	; ts7:tc3
		pfmul		mm1, mm5
		pfmul		mm7, mm5
		pxor		mm2, mm5		; -ts7:tc3
		movq		[local(0)], mm1
		movq		[local(1)], mm7
		movq		mm7, [D_mcx3_cx2]
		movq		mm1, [D_cx4_cx0]
		pfmul		mm5, mm7
		pfsub		mm2, mm4		; -ts7-ts5:tc3-tc1
		pfmul		mm1, mm4
		pfmul		mm7, mm4
		pfmul		mm4, [D_cx5_cx1]
		pfadd		mm1, [local(0)]
		pfadd		mm7, [local(1)]
		pfsubr		mm5, mm4
		pswapd		mm4, [in(16)]
		movq		[local(0)], mm1
		movq		[local(1)], mm7
		movq		mm1, [D_mcx3_cx2]
		movq		mm7, [D_mcx5_cx1]
		pfadd		mm2, mm0		; -ts7-ts5+ts8:tc3-tc1+tc4 
		pfmul		mm1, mm0
		pfmul		mm7, mm0
		pfmul		mm0, [D_mcx4_cx0]
		movq		[local(2)], mm2
		pfadd		mm4, [in(9)] ; tc6:tc5
		pfadd		mm1, [local(0)]
		pfsubr		mm7, [local(1)]
		pfadd		mm5, mm0
		pfmul		mm2, [D_mcx7_cx6]
		pswapd		mm0, [in(7)]
		pfadd		mm1, mm3
		pfsub		mm7, mm3
		pfsubr		mm2, mm6 ; st:ct
		pxor		mm3, [D_MSB_0]  ; ts6:tc2 
		pfpnacc		mm1, mm1 
		pfsub		mm6, [local(2)]
		pfsub		mm5, mm3
		pswapd		mm3, [in(14)]
		pfsub		mm0, [in(0)] ; ts2:ts1
		pfpnacc		mm7, mm7 
		pfpnacc		mm2, mm2 ; cs+st:ct-st
		pfpnacc		mm5, mm5 
		punpckhdq	mm6, mm6
		pfadd		mm3, [in(11)] ; tc8:tc7
		movq		[out(1)], mm1  
		pswapd		mm1, [in(5)]
		movq		[out(13)], mm7 
		movq		[out(5)], mm2
		movq		mm2, mm4
		punpckldq	mm4, mm0 ; ts1:tc5
		movq		[out(9)], mm5
		movd		[out(17)], mm6 
		movq		mm6, [D_mcx0_cx4]
		movd		mm7, [in(13)]
		pfsub		mm1, [in(2)] ; ts4:ts3
		punpckhdq	mm2, mm0 ; ts2:tc6
		movq		mm5, mm3
		movq		mm0, mm2
		punpckldq	mm3, mm1 ; ts3:tc7
		pfadd		mm2, mm7
		pfmul		mm0, [D_mcx6_cx7]
		punpckhdq	mm5, mm1 ; ts4:tc8
		pand		mm2, [D_0_m1] ; 0:(tc6+in[13])
		pfsubr		mm0, mm7 ;  ts2:tc6
		movq		mm7, [D_cx1_cx5]
		movq		mm1, [D_MSB_0]	; 
		pfmul		mm6, mm3
		pfmul		mm7, mm3
		pxor		mm1, mm3		; -ts3:tc7
		movq		[local(0)], mm6
		movq		[local(1)], mm7
		movq		mm6, [D_mcx2_cx3]
		movq		mm7, [D_cx0_cx4]
		pfmul		mm3, mm6
		pfadd		mm1, mm4		; ts1-ts3:tc5+tc7
		pfmul		mm6, mm4
		pfmul		mm7, mm4
		pfmul		mm4, [D_cx1_cx5]
		pfadd		mm6, [local(0)]
		pfadd		mm7, [local(1)]
		pfadd		mm3, mm4
		movq		[local(0)], mm6
		movq		[local(1)], mm7
		movq		mm6, [D_mcx1_cx5]
		movq		mm7, [D_mcx2_cx3]
		pfadd		mm1, mm5		; ts1-ts3+ts4:tc5+tc7+tc8
		pfmul		mm6, mm5
		pfmul		mm7, mm5
		pfmul		mm5, [D_mcx0_cx4]
		movq		[local(2)], mm1
		pfadd		mm6, [local(0)]
		pfadd		mm7, [local(1)]
		pfadd		mm3, mm5
		pfmul		mm1, [D_mcx6_cx7]
		pfsub		mm6, mm0
		pfsubr		mm7, mm0
		pxor		mm0, [D_MSB_0]  ; -ts2:tc6
		pfsub		mm1, mm2 ; st:ct
		pfadd		mm2, [local(2)]
		pfpnacc		mm6, mm6 
		pfsub		mm0, mm3
		pfpnacc		mm1, mm1 ; ct+st:ct-st
		pfpnacc		mm7, mm7 
		pfpnacc		mm0, mm0 
		add			esp, byte LOCAL_SIZE
		movq		[out(3)], mm6  
		movd		[out(0)], mm2
		movq		[out(11)], mm1
		movq		[out(15)], mm7  
		movq		[out(7)], mm0  
.exit:
		femms
		ret

		end
