;
;   part of this code is origined from
;
;       GOGO-no-coda
;       Copyright (c) 2001,2002,2003 gogo-developer
;

%include "global.cfg"

	externdef		tonalityTbl		;float * x 2

	segment_data
	
	align	4
inner_psy_sub3_3DN_FAST.block_jmp_tbl
	dd	_inner_psy_sub3_3DN_FAST.long_long
	dd	_inner_psy_sub3_3DN_FAST.long_short
	dd	_inner_psy_sub3_3DN_FAST.short_long
	dd	_inner_psy_sub3_3DN_FAST.short_short

inner_psy_sub3_3DN_HI.block_jmp_tbl
	dd	_inner_psy_sub3_3DN_HI.long_long
	dd	_inner_psy_sub3_3DN_HI.long_short
	dd	_inner_psy_sub3_3DN_HI.short_long
	dd	_inner_psy_sub3_3DN_HI.short_short

	align	8
D_f0p5		dd	0.5, 0.5
D_i2		dd	2, 2
D_0			dd	0, 0
D_LIMIT_L	dd	0.0487558430, 0.0487558430
D_LIMIT_U	dd	0.4989003826, 0.4989003826
D_f2		dd	2.0, 2.0
D_f8		dd	8.0, 8.0
D_fMAX		dd	1.0e35, 1.0e35
D_coef3		dd	568.70622093, 568.70622093	; tonalityTblNum / (LIMIT_U - LIMIT_L)
D_f1		dd	1.0, 1.0
D_i127		dd	127, 127
D_i0x007FFFFF	dd	0x007FFFFF, 0x007FFFFF
D_fSQRT2	dd	1.4142135623, 1.4142135623
D_fLOG_E_2	dd	0.6931471805, 0.6931471805
D_fA2		dd	2.0000006209, 2.0000006209
D_fB2		dd	0.6664778517, 0.6664778517
D_fC2		dd	0.4139745860, 0.4139745860
D_f0p4		dd	0.4, 0.4
D_f1em6		dd	1.0e-6, 1.0e-6
D_im1		dd	-1, -1
D_ABS		dd  0x7FFFFFFF, 0x7FFFFFFF

%define	SHORT_TYPE		(2)		;defined in encoder.h
%define tonalityTblNum	(256)	;defined in psymode.c
%define BLKSIZE		(1024)
%define BLKSIZE_s	(256)
%define HBLKSIZE_s    (BLKSIZE_s/2 + 1)
%define NBPSY_s		(13)
%define cw_lower_index	(6)
%define CBANDS        (64)

	segment_text
;	2001/08/26 initial ver.  6.00Kclk@K7-500 by kei
;	2002/04/10	レジスタ整理 5.85Kclk@K7-500 by kei

%define energy	(edx-gogo_thread_data_s.psywork+gogo_thread_data_s.energy)

proc	inner_psy_sub1_3DN
%assign _P 4*0
		mov			eax, [esp+_P+4]		; eax = tl
		mov			ecx, 5
		lea			edx, [eax+gogo_thread_data_s.psywork+16]; &(wsamp_l[4])
		movq		mm6, [edx+0-16]
		movq		mm7, [edx+8-16]
		lea			eax, [eax+gogo_thread_data_s.psywork+(BLKSIZE-1)*16]; &(wsamp_l[(BLKSIZE-1)*4])
		pfmul		mm6, mm6
		pfmul		mm7, mm7
		movq		[energy+0-16], mm6
		movq		[energy+8-16], mm7

		loopalignK7	16
.lp.j.1..10:
		movq		mm0, [edx   ]
		movq		mm1, [edx+ 8]
		movq		mm2, [eax   ]
		movq		mm3, [eax+ 8]
		movq		mm4, [edx+16]
		movq		mm5, [edx+24]
		movq		mm6, [eax-16]
		movq		mm7, [eax- 8]
		pfmul		mm0, mm0
		pfmul		mm1, mm1
		pfmul		mm2, mm2
		pfmul		mm3, mm3
		add			edx, byte 32
		sub			eax, byte 32
		pfmul		mm4, mm4
		pfmul		mm5, mm5
		pfmul		mm6, mm6
		pfmul		mm7, mm7
		pfadd		mm0, mm2
		movq		mm2, [D_f0p5]
		pfadd		mm1, mm3
		pfadd		mm4, mm6
		pfadd		mm5, mm7
		pfmul		mm0, mm2
		pfmul		mm1, mm2
		dec			ecx
		pfmul		mm4, mm2
		pfmul		mm5, mm2
		movq		[energy+ 0-32], mm0
		movq		[energy+ 8-32], mm1
		movq		[energy+16-32], mm4
		movq		[energy+24-32], mm5
		jnz			near .lp.j.1..10

		mov			ecx, ((BLKSIZE/2)-10)/2
		movq		mm6, [energy-(11*16)  ]
		movq		mm7, [energy-(11*16)+8]
		loopalignK7	4
.lp.j.over.10:
		movq		mm0, [edx   ]
		movq		mm1, [edx+ 8]
		movq		mm2, [eax   ]
		movq		mm3, [eax+ 8]
		movq		mm4, [edx+16]
		movq		mm5, [edx+24]
		pfmul		mm0, mm0
		pfmul		mm1, mm1
		add			edx, byte 32
		pfmul		mm2, mm2
		pfmul		mm3, mm3
		pfadd		mm0, mm2
		pfadd		mm1, mm3
		movq		mm2, [eax-16]
		movq		mm3, [eax- 8]
		pfmul		mm4, mm4
		pfmul		mm5, mm5
		sub			eax, byte 32
		pfmul		mm2, mm2
		pfmul		mm3, mm3
		pfmul		mm0, [D_f0p5]
		pfmul		mm1, [D_f0p5]
		pfadd		mm4, mm2
		pfadd		mm5, mm3
		pfadd		mm6, mm0
		pfadd		mm7, mm1
		pfmul		mm4, [D_f0p5]
		pfmul		mm5, [D_f0p5]
		dec			ecx
		pfadd		mm6, mm4
		pfadd		mm7, mm5
		movq		[energy+ 0-32], mm0
		movq		[energy+ 8-32], mm1
		movq		[energy+16-32], mm4
		movq		[energy+24-32], mm5
		jnz			near .lp.j.over.10

		movq		[RW.tot_ener+0], mm6
		movq		[RW.tot_ener+8], mm7

.exit:
		femms
		ret


;	2001/08/26	initial ver. 4.00Kclk@K7-500 by kei
;	2002/04/10	レジスタ整理 3.99Kclk@K7-500 by kei

%define energy	(edx-gogo_thread_data_s.psywork+gogo_thread_data_s.energy_s)

proc	inner_psy_sub2_3DN
%assign _P 4*0
		mov			eax, [esp+_P+4]		; eax = tl
		mov			ecx, BLKSIZE_s/2
		lea			edx, [eax+gogo_thread_data_s.psywork+16]
		lea			eax, [eax+gogo_thread_data_s.psywork+(BLKSIZE_s-1)*16]
		movq		mm0, [edx+BLKSIZE_s*16*0+ 0-16]
		movq		mm1, [edx+BLKSIZE_s*16*0+ 8-16]
		movq		mm2, [edx+BLKSIZE_s*16*1+ 0-16]
		movq		mm3, [edx+BLKSIZE_s*16*1+ 8-16]
		movq		mm4, [edx+BLKSIZE_s*16*2+ 0-16]
		movq		mm5, [edx+BLKSIZE_s*16*2+ 8-16]
		pfmul		mm0, mm0
		pfmul		mm1, mm1
		pfmul		mm2, mm2
		pfmul		mm3, mm3
		pfmul		mm4, mm4
		pfmul		mm5, mm5
		movq		[energy+HBLKSIZE_s*16*0+ 0-16], mm0
		movq		[energy+HBLKSIZE_s*16*0+ 8-16], mm1
		movq		[energy+HBLKSIZE_s*16*1+ 0-16], mm2
		movq		[energy+HBLKSIZE_s*16*1+ 8-16], mm3
		movq		[energy+HBLKSIZE_s*16*2+ 0-16], mm4
		movq		[energy+HBLKSIZE_s*16*2+ 8-16], mm5

		loopalignK7		4
.lp.j:
		movq		mm0, [edx+BLKSIZE_s*16*0+ 0]		; re
		movq		mm2, [eax+BLKSIZE_s*16*0+ 0]		; im
		movq		mm1, [edx+BLKSIZE_s*16*0+ 8]		; re
		movq		mm3, [eax+BLKSIZE_s*16*0+ 8]		; im
		movq		mm4, [edx+BLKSIZE_s*16*1+ 0]		; re
		movq		mm6, [eax+BLKSIZE_s*16*1+ 0]		; im
		movq		mm5, [edx+BLKSIZE_s*16*1+ 8]		; re
		movq		mm7, [eax+BLKSIZE_s*16*1+ 8]		; im
		pfmul		mm0, mm0
		pfmul		mm2, mm2
		pfmul		mm1, mm1
		pfmul		mm3, mm3
		pfmul		mm4, mm4
		pfmul		mm6, mm6
		pfmul		mm5, mm5
		pfmul		mm7, mm7
		pfadd		mm0, mm2
		pfadd		mm1, mm3
		movq		mm2, [edx+BLKSIZE_s*16*2+ 0]		; re
		movq		mm3, [edx+BLKSIZE_s*16*2+ 8]		; re
		pfadd		mm4, mm6
		movq		mm6, [eax+BLKSIZE_s*16*2+ 0]		; im
		pfadd		mm5, mm7
		movq		mm7, [eax+BLKSIZE_s*16*2+ 8]		; im
		pfmul		mm2, mm2
		pfmul		mm6, mm6
		pfmul		mm3, mm3
		pfmul		mm7, mm7
		add			edx, byte 16
		sub			eax, byte 16
		dec			ecx
		pfadd		mm2, mm6
		movq		mm6, [D_f0p5]
		pfadd		mm3, mm7
		pfmul		mm0, mm6
		pfmul		mm1, mm6
		pfmul		mm4, mm6
		pfmul		mm5, mm6
		pfmul		mm2, mm6
		pfmul		mm3, mm6
		movq		[energy+HBLKSIZE_s*16*0+ 0-16], mm0
		movq		[energy+HBLKSIZE_s*16*0+ 8-16], mm1
		movq		[energy+HBLKSIZE_s*16*1+ 0-16], mm4
		movq		[energy+HBLKSIZE_s*16*1+ 8-16], mm5
		movq		[energy+HBLKSIZE_s*16*2+ 0-16], mm2
		movq		[energy+HBLKSIZE_s*16*2+ 8-16], mm3
		jnz			near .lp.j
.exit:
		femms
		ret


;	2001/08/31 kei
;	HI
;	21.6Kclk@K7-500
;	FAST
;	20.6Kclk@K7-500

%define eb				(BASE+4)
%define cb				(BASE+8)
%define thr				(BASE+12)

%define BASE			(esp+_P+LOCAL_SIZE)
%assign LOCAL_SIZE		16
%define	npart_l4@		(esp+ 0)		;size 4
%define	block_jmp@		(esp+ 4)		;size 4
%define block_mask@		(esp+ 8)		;size 8

;static void inner_psy_sub3_C( float *eb, float *cb, float *thr )

%imacro	inner_psy_sub3_3DN 1
proc inner_psy_sub3_3DN_%1
		push		ebx
		push		ebp
		push		edi
		push		esi
		sub			esp, LOCAL_SIZE
%assign _P 4*4
		movq		mm0, [RW.blocktype_old]
		movq		mm1, [D_fMAX]
		mov			esi, [RO.npart_l]
		pcmpeqd		mm0, [D_i2]
		mov			eax, [eb]
		movq		mm2, mm0
		xor			ecx, ecx		; b*4
		pand		mm1, mm0
		punpckhdq	mm2, mm2
		pslld		mm0, 1
		mov			edx, [cb]
		movq		[block_mask@], mm1
		paddd		mm0, mm2
		lea			esi, [esi*4]
		movd		ebp, mm0
		xor			edi, edi		; b*32
		neg			ebp
		mov			ebx, [thr]
		mov			ebp, [inner_psy_sub3_3DN_%1.block_jmp_tbl+ebp*4]
		mov			[npart_l4@], esi
		mov			[block_jmp@], ebp			
		loopalignK7	4
.lp.b0:
		mov			ebp, [RO.s3ind+ecx*2]	; k
		mov			esi, [RO.s3ind+ecx*2+4]	
		pxor		mm0, mm0	; ecb0, ecb1
		pxor		mm1, mm1	; ecb2, ecb3
		pxor		mm2, mm2	; tbb0, tbb1
		pxor		mm3, mm3	; tbb2, tbb3
		lea			ebp, [ebp*4]			; k*4
		lea			esi, [esi*4]
		loopalignK7	8
.lp.k:
		movd		mm7, [RO.s3_l+edi*8+ebp]
		movq		mm4, [eax+ebp*4+ 0]
		punpckldq	mm7, mm7
		movq		mm5, [eax+ebp*4+ 8]
		movq		mm6, [edx+ebp*4+ 0]
		add			ebp, byte 4
		pfmul		mm4, mm7
		pfmul		mm5, mm7
		pfmul		mm6, mm7
		pfmul		mm7, [edx+ebp*4+ 8-16]
		cmp			ebp, esi
		pfadd		mm0, mm4
		pfadd		mm1, mm5
		pfadd		mm2, mm6
		pfadd		mm3, mm7
		jle			.lp.k
.lp.k.end:
		movq		mm4, mm0
		movq		mm5, mm1
		pfcmpeq		mm4, [D_0]
		pfcmpeq		mm5, [D_0]
		movq		mm6, mm0
		movq		mm7, mm1
%ifidni %1, HI
		pandn		mm4, mm2	; (ecb) ? tbb : 0 [1]:[0]
		pfrcp		mm2, mm0
		pandn		mm5, mm3	; (ecb) ? tbb : 0 [3]:[2]
		pfrcp		mm3, mm1
		punpckhdq	mm6, mm6
		punpckhdq	mm7, mm7
		pfrcp		mm6, mm6
		pfrcp		mm7, mm7
		punpckldq	mm2, mm6
		punpckldq	mm3, mm7
		movq		mm6, mm0
		movq		mm7, mm1
		pfrcpit1	mm6, mm2
		pfrcpit1	mm7, mm3
		pfrcpit2	mm6, mm2
		pfrcpit2	mm7, mm3
		movq		mm2, [D_LIMIT_L]
		pfmul		mm4, mm6
		pfmul		mm5, mm7
		pfmax		mm4, mm2
		pfmax		mm5, mm2
		pfmin		mm4, [D_LIMIT_U]
		pfmin		mm5, [D_LIMIT_U]
		pfsub		mm4, mm2
		pfsub		mm5, mm2
%else	; %1, HI
		pandn		mm4, mm2	; (ecb) ? tbb : 0 [1]:[0]
		pandn		mm5, mm3	; (ecb) ? tbb : 0 [3]:[2]
		pfrcp		mm2, mm0
		pfrcp		mm3, mm1
		punpckhdq	mm6, mm6
		punpckhdq	mm7, mm7
		pfrcp		mm6, mm6
		pfrcp		mm7, mm7
		punpckldq	mm2, mm6
		punpckldq	mm3, mm7
		movq		mm6, [D_LIMIT_L]
		pfmul		mm4, mm2
		pfmul		mm5, mm3
		pfmax		mm4, mm6
		pfmax		mm5, mm6
		pfmin		mm4, [D_LIMIT_U]
		pfmin		mm5, [D_LIMIT_U]
		pfsub		mm4, mm6
		pfsub		mm5, mm6
%endif  ; %1, HI
		pfmul		mm4, [D_coef3]
		pfmul		mm5, [D_coef3]
		pf2id		mm2, mm4
		movd		ebp, mm2
		pf2id		mm3, mm5
		pi2fd		mm6, mm2
		punpckhdq	mm2, mm2
		pi2fd		mm7, mm3
		movd		esi, mm2
		movd		mm2, [tonalityTbl+ebp*8]
		pfsub		mm4, mm6
		punpckldq	mm2, [tonalityTbl+esi*8]
		movd		mm6, [tonalityTbl+ebp*8+4]
		movd		ebp, mm3
		punpckhdq	mm3, mm3
		pfsub		mm5, mm7
		punpckldq	mm6, [tonalityTbl+esi*8+4]
		movd		esi, mm3
		movd		mm3, [tonalityTbl+ebp*8]
		pfmul		mm4, mm6
		movd		mm6, [RO.minval+ecx]
		punpckldq	mm3, [tonalityTbl+esi*8]
		movd		mm7, [tonalityTbl+ebp*8+4]
		punpckldq	mm6, mm6
		punpckldq	mm7, [tonalityTbl+esi*8+4]
		add			edi, byte 32
		add			ecx, byte 4
		pfmul		mm5, mm7
		pfadd		mm4, mm2
		pfadd		mm5, mm3
		pfmin		mm4, mm6
		pfmin		mm5, mm6
		pfmul		mm0, mm4
		pfmul		mm1, mm5
		movq		mm2, [RW.nb_12+edi+ 0-32]		; nb_12 [(b*2)*4+1]:[(b*2)*4+0]
		movq		mm3, [RW.nb_12+edi+ 8-32]		; nb_12 [(b*2)*4+3]:[(b*2)*4+2]
		movq		[RW.nb_12+edi+ 0-32], mm0
		movq		[RW.nb_12+edi+ 8-32], mm1
		jmp			[block_jmp@]
.long_long:
		movq		mm6, [RW.nb_12+edi+16-32]
		movq		mm7, [RW.nb_12+edi+24-32]
		pfmul		mm6, [D_f8]
		pfmul		mm7, [D_f8]
		pfmin		mm6, mm2
		pfmin		mm7, mm3
		pfmul		mm6, [D_f2]
		pfmul		mm7, [D_f2]
		pfmin		mm0, mm6
		pfmin		mm1, mm7
.short_short:
.block_jmp.end:
		cmp			ecx, [npart_l4@]
		movq		[RW.nb_12+edi+16-32], mm2
		movq		[RW.nb_12+edi+24-32], mm3
		movq		[ebx+ecx*4+ 0-16], mm0
		movq		[ebx+ecx*4+ 8-16], mm1
		jl			near .lp.b0
.lp.b0.end:

		pxor		mm0, mm0	
		mov			edx, RW.pe
		mov			edi, RW.ATH+ath_t.cb
		mov			ebp, RO.numlines_l
		mov			esi, [npart_l4@]
		xor			ecx, ecx	; b*4
		movq		[edx+0], mm0
		movq		[edx+8], mm0
		loopalignK7	4
.lp.b1:
		movq		mm2, [eax+ecx*4+ 0]
		movq		mm3, [eax+ecx*4+ 8]
		movq		mm4, mm2
		movq		mm5, mm3
		pfcmpeq		mm4, mm0
		pfcmpeq		mm5, mm0
		movq		mm6, mm4
		movq		mm7, mm5
		pandn		mm4, mm2		
		pandn		mm5, mm3
		pand		mm6, [D_f1]
		pand		mm7, [D_f1]
		por			mm6, mm4		; eb ? eb : 1.0;
		por			mm7, mm5		; eb ? eb : 1.0;
%ifidni %1, HI
		movq		mm2, mm6
		movq		mm3, mm7
		pfrcp		mm4, mm6
		pfrcp		mm5, mm7
		punpckhdq	mm2, mm2
		punpckhdq	mm3, mm3
		movd		mm0, [edi+ecx]
		pfrcp		mm2, mm2
		pfrcp		mm3, mm3
		punpckldq	mm0, mm0
		movq		mm1, mm0
		punpckldq	mm4, mm2
		punpckldq	mm5, mm3
		pfrcpit1	mm6, mm4
		pfrcpit1	mm7, mm5
		pfmax		mm0, [ebx+ecx*4+ 0]
		pfmax		mm1, [ebx+ecx*4+ 8]
		pfrcpit2	mm6, mm4		; eb ? 1.0/eb : 1.0;
		pfrcpit2	mm7, mm5		; eb ? 1.0/eb : 1.0;
		pfmul		mm0, mm6
		pfmul		mm1, mm7
%else
		pfrcp		mm4, mm6
		pfrcp		mm5, mm7
		punpckhdq	mm6, mm6
		punpckhdq	mm7, mm7
		movd		mm0, [edi+ecx]
		pfrcp		mm6, mm6
		pfrcp		mm7, mm7
		punpckldq	mm0, mm0
		movq		mm1, mm0
		pfmax		mm0, [ebx+ecx*4+ 0]
		pfmax		mm1, [ebx+ecx*4+ 8]
		punpckldq	mm4, mm6		; eb ? 1.0/eb : 1.0;
		punpckldq	mm5, mm7		; eb ? 1.0/eb : 1.0;
		pfmul		mm0, mm4
		pfmul		mm1, mm5
%endif
		movq		mm2, [D_f1]
		pfmin		mm0, mm2
		pfmin		mm1, mm2
		movq		mm4, mm0
		movq		mm5, mm1
		pand		mm0, [D_i0x007FFFFF]
		pand		mm1, [D_i0x007FFFFF]
		por			mm0, mm2
		por			mm1, mm2
		movq		mm2, mm0
		movq		mm3, mm1
		pfadd		mm0, [D_fSQRT2]
		pfadd		mm1, [D_fSQRT2]
		pfrcp		mm6, mm0
		movq		mm7, mm0
		punpckhdq	mm0, mm0
		pfrcp		mm0, mm0
		psrld		mm4, 23
		psrld		mm5, 23
		punpckldq	mm6, mm0
		psubd		mm4, [D_i127]
		psubd		mm5, [D_i127]
		pfrcpit1	mm7, mm6
		pfsub		mm2, [D_fSQRT2]
		pfsub		mm3, [D_fSQRT2]
		pfrcpit2	mm7, mm6
		pfrcp		mm6, mm1
		pfmul		mm2, mm7		
		movq		mm7, mm1
		punpckhdq	mm1, mm1
		pfrcp		mm1, mm1
		pi2fd		mm4, mm4
		pi2fd		mm5, mm5
		movq		mm0, mm2		; z
		punpckldq	mm6, mm1
		pfrcpit1	mm7, mm6
		pfadd		mm4, [D_f0p5]
		pfadd		mm5, [D_f0p5]
		pfmul		mm2, mm2		; zz
		pfrcpit2	mm7, mm6
		movq		mm6, [D_fC2]
		pfmul		mm4, [D_fLOG_E_2]
		pfmul		mm5, [D_fLOG_E_2]
		pfmul		mm3, mm7		
		movq		mm7, mm6
		movq		mm1, mm3		; z
		pfmul		mm3, mm3		; zz
		pfmul		mm6, mm2
		pfmul		mm7, mm3
		pfadd		mm6, [D_fB2]
		pfadd		mm7, [D_fB2]
		pfmul		mm6, mm2
		movd		mm2, [ebp+ecx]
		pfmul		mm7, mm3
		pfadd		mm6, [D_fA2]
		pfadd		mm7, [D_fA2]
		punpckldq	mm2, mm2		
		pfmul		mm6, mm0
		pfmul		mm7, mm1
		pi2fd		mm2, mm2		; num
		pfadd		mm6, mm4		; log(th)
		pfadd		mm7, mm5		; log(th)
		add			ecx, byte 4
		pfmul		mm6, mm2
		pfmul		mm7, mm2
		pfsubr		mm6, [edx+ 0]
		pfsubr		mm7, [edx+ 8]
		cmp			ecx, esi
		pxor		mm0, mm0	
		movq		[edx+ 0], mm6
		movq		[edx+ 8], mm7
		jl			near .lp.b1
.lp.b1.end:
.exit:
		add			esp, LOCAL_SIZE
		femms
		pop			esi
		pop			edi
		pop			ebp
		pop			ebx
		ret
.short_long:
.long_short:
		movq		mm6, [RW.nb_12+edi+16-32]
		movq		mm7, [RW.nb_12+edi+24-32]
		movq		mm4, [block_mask@]
		pfmul		mm6, [D_f8]
		pfmul		mm7, [D_f8]
		pfmin		mm6, mm2
		pfmin		mm7, mm3
		pfmul		mm6, [D_f2]
		pfmul		mm7, [D_f2]
		pfadd		mm6, mm4
		pfadd		mm7, mm4
		pfadd		mm6, mm4
		pfadd		mm7, mm4
		pfmin		mm0, mm6
		pfmin		mm1, mm7
		jmp			near .block_jmp.end
%endmacro

inner_psy_sub3_3DN HI
inner_psy_sub3_3DN FAST

;	2001/09/19	initial ver. 14.5Kclk@K7-500 by kei
;	2002/04/10  レジスタ整理とかいろいろ 14.1Kclk@K7-500 by kei
;				eb, cb は見ないようになった
;

%define tl				(BASE+4)
%define energy	(ecx)
%define eb		(edx+gogo_thread_data_s.psywork)
%define thr		(edx+gogo_thread_data_s.psywork+CBANDS*8*4)

%define BASE			(esp+_P)

proc	inner_psy_sub5_3DN
		push		esi
		push		ebp
		push		ebx
		push		edi
%assign _P 4*4
		mov			edx, [tl]			; tl
		mov			esi, [RO.npart_s_orig]
		xor			ebp, ebp		; k
		xor			eax, eax ; j*4
		xor			edi, edi ; b*4
		lea			ecx, [edx+gogo_thread_data_s.energy_s] ; tl->energy_s[k]
		shl			esi, 2
		loopalign	12
.lp.k:
.lp.b0:
		mov			ebx, [RO.numlines_s+edi]
		pxor		mm0, mm0
		pxor		mm1, mm1
		dec			ebx
		pxor		mm2, mm2
		pxor		mm3, mm3
		jz			.lp.i0.1
		loopalign	16
.lp.i0.2:
		add			eax, byte 8
		sub			ebx , byte 2
		pfadd		mm0, [energy+eax*4+ 0-32]
		pfadd		mm1, [energy+eax*4+ 8-32]
		pfadd		mm2, [energy+eax*4+16-32]
		pfadd		mm3, [energy+eax*4+24-32]
		jg			.lp.i0.2
.lp.i0.2.end:
		pfadd		mm0, mm2
		pfadd		mm1, mm3
		jnz			.lp.i0.1.end
.lp.i0.1:
		pfadd		mm0, [energy+eax*4+ 0]
		pfadd		mm1, [energy+eax*4+ 8]
		add			eax, byte 4
.lp.i0.1.end:
		add			edi, 4
		movq		[eb+edi*4+ 0-16], mm0
		cmp			edi, esi
		movq		[eb+edi*4+ 8-16], mm1
		jl			.lp.b0
.lp.b0.end:

		mov			esi, [RO.npart_s]
		xor			edi, edi ; b*4
		shl			esi, 2
		loopalign	12
.lp.b1:
		mov			ebx, [RO.s3ind_s+edi*2+4]
		mov			eax, [RO.s3ind_s+edi*2] 
		pxor		mm0, mm0
		pxor		mm2, mm2
		shl			eax, 2 ; i*4
		sub			ebx, [RO.s3ind_s+edi*2+0]
		lea			edi, [edi*8] ; b*32
		pxor		mm1, mm1
		pxor		mm3, mm3
		jz			.lp.i1.1
		loopalign	16
.lp.i1.2:
		movq		mm5, [RO.s3_s+edi*8+eax]
		movq		mm7, mm5
		movq		mm4, [eb+eax*4+ 0]
		movq		mm6, [eb+eax*4+16]
		punpckldq	mm5, mm5
		punpckhdq	mm7, mm7
		pfmul		mm4, mm5
		pfmul		mm6, mm7
		pfmul		mm5, [eb+eax*4+ 8]
		pfmul		mm7, [eb+eax*4+24]
		add			eax, byte 8
		sub			ebx , byte 2
		pfadd		mm0, mm4
		pfadd		mm2, mm6
		pfadd		mm1, mm5
		pfadd		mm3, mm7
		jg			.lp.i1.2
.lp.i1.2.end:
		pfadd		mm0, mm2
		pfadd		mm1, mm3
		jnz			.lp.i1.1.end
.lp.i1.1:
		movd		mm5, [RO.s3_s+edi*8+eax]
		movq		mm4, [eb+eax*4+ 0]
		punpckldq	mm5, mm5
		pfmul		mm4, mm5
		pfmul		mm5, [eb+eax*4+ 8]
		pfadd		mm0, mm4
		pfadd		mm1, mm5
.lp.i1.1.end:
		shr			edi, 3 ; b*4
		movq		mm5, [D_f1em6]
		movd		mm4, [RO.SNR_s+edi]
		add			edi, byte 4
		punpckldq	mm4, mm4
		pfmul		mm0, mm4
		pfmul		mm1, mm4
		cmp			edi, esi
		pfmax		mm0, mm5
		pfmax		mm1, mm5
		movq		[thr+edi*4+ 0-16], mm0
		movq		[thr+edi*4+ 8-16], mm1
		jnz			near .lp.b1
.lp.b1.end:

		xor			edi, edi ; b*4
		movq		mm7, [D_f0p5]
		loopalign	12
.lp.b2:
		mov			esi, [RO.bo_s+edi]
		mov			eax, [RO.bu_s+edi]
		shl			esi, 2 ; bo*4
		lea			eax, [eax*4] ; bu*4
		movq		mm0, [eb+esi*4+ 0] ; enn0, enn1,
		movq		mm1, [eb+esi*4+ 8] ; enn2, enn3
		movq		mm2, [thr+esi*4+ 0] ; thm0, thm1 
		movq		mm3, [thr+esi*4+ 8] ; thm2, thm3
		sub			esi, eax
		pfadd		mm0, [eb+eax*4+ 0]
		pfadd		mm1, [eb+eax*4+ 8]
		pfadd		mm2, [thr+eax*4+ 0]
		shr			esi, 2 ; bo-bu
		pfadd		mm3, [thr+eax*4+ 8]
		pfmul		mm0, mm7
		add			eax, byte 4 ; i*4
		sub			esi, byte 2 
		pfmul		mm1, mm7
		pfmul		mm2, mm7
		pfmul		mm3, mm7
		js			.lp.i2.1.end
		jz			.lp.i2.1
		loopalign	16
.lp.i2.2:
		add			eax, byte 8
		sub			esi, byte 2
		pfadd		mm0, [eb+eax*4+ 0-32]
		pfadd		mm1, [eb+eax*4+ 8-32]
		pfadd		mm2, [thr+eax*4+ 0-32]
		pfadd		mm3, [thr+eax*4+ 8-32]
		pfadd		mm0, [eb+eax*4+16-32]
		pfadd		mm1, [eb+eax*4+24-32]
		pfadd		mm2, [thr+eax*4+16-32]
		pfadd		mm3, [thr+eax*4+24-32]
		jg			.lp.i2.2
.lp.i2.2.end:
		jnz			.lp.i2.1.end
.lp.i2.1:
		pfadd		mm0, [eb+eax*4+ 0]
		pfadd		mm1, [eb+eax*4+ 8]
		pfadd		mm2, [thr+eax*4+ 0]
		pfadd		mm3, [thr+eax*4+ 8]
.lp.i2.1.end:
		lea			eax, [edi*4]
		sub			eax, edi
		add			edi, byte 4
		lea			eax, [RW.en+eax+ebp*4+III_psy_xmin_s.s] ; RW.en[0].s[b][k]
		movd		[eax+sizeof_III_psy_xmin_s*0], mm0
		movd		[eax+sizeof_III_psy_xmin_s*2], mm1
		movd		[eax+sizeof_III_psy_xmin_s*0+RW.thm-RW.en], mm2
		movd		[eax+sizeof_III_psy_xmin_s*2+RW.thm-RW.en], mm3
		cmp			edi, byte NBPSY_s*4
		punpckhdq	mm0, mm0
		punpckhdq	mm1, mm1
		punpckhdq	mm2, mm2
		punpckhdq	mm3, mm3
		movd		[eax+sizeof_III_psy_xmin_s*1], mm0
		movd		[eax+sizeof_III_psy_xmin_s*3], mm1
		movd		[eax+sizeof_III_psy_xmin_s*1+RW.thm-RW.en], mm2
		movd		[eax+sizeof_III_psy_xmin_s*3+RW.thm-RW.en], mm3
		jl			near .lp.b2
.lp.b2.end:

		inc			ebp
		mov			esi, [RO.npart_s_orig]
		xor			edi, edi ; b*4
		xor			eax, eax ; j*4
		cmp			ebp, byte 3
		lea			ecx, [ecx+HBLKSIZE_s*4*4]
		lea			esi, [esi*4]
		jl			near .lp.k
.lp.k.end:

.exit:
		femms
		pop			edi
		pop			ebx
		pop			ebp
		pop			esi
		ret

;	2001/09/03 kei
;	3.8Kclk@K7-500
;	2002/02/05	fix bug on encoding 16Khz by kei
;	2002/04/09	レジスタ整理 引数の eb, cb は見ていない 3.4Kclk@K7-500 by kei

%define energy	(ebx)
%define eb		(ebx-gogo_thread_data_s.energy+gogo_thread_data_s.psywork)
%define cb		(ebx-gogo_thread_data_s.energy+gogo_thread_data_s.psywork+CBANDS*4*4)

;void inner_psy_sub6( gogo_thread_data *tl);
proc	inner_psy_sub6_3DN
		push		ebx
		push		ebp
		push		edi
%assign _P 4*3
		mov			ebx, [esp+_P+4]			; tl
		mov			ebp, RO.numlines_l		
		add			ebx, gogo_thread_data_s.energy	; energy
		xor			eax, eax				; j
		xor			edx, edx				; b
		mov			ecx, [RO.npart_l_pre_max]
		align		4
.lp.b0:
		mov			edi, [ebp]
		pxor		mm0, mm0	; ebb
		pxor		mm1, mm1	; ebb
		add			ebp, byte 4
		dec			edi
		movq		mm2, mm0	; cbb
		movq		mm3, mm0	; cbb
		jz			.lp.i0.odd
		align		4
.lp.i0.even:
		movq		mm4, [energy+eax+ 0]
		movq		mm5, [energy+eax+ 8]
		movq		mm6, [energy+eax+16]
		movq		mm7, [energy+eax+24]
		add			eax, byte 32
		pfadd		mm0, mm4
		pfadd		mm1, mm5
		pfmul		mm4, [RW.cw+eax+ 0-32]
		pfmul		mm5, [RW.cw+eax+ 8-32]
		pfadd		mm0, mm6
		pfadd		mm1, mm7
		sub			edi, byte 2
		pfadd		mm2, mm4
		pfadd		mm3, mm5
		pfmul		mm6, [RW.cw+eax+16-32]
		pfmul		mm7, [RW.cw+eax+24-32]
		pfadd		mm2, mm6
		pfadd		mm3, mm7
		jg			.lp.i0.even
.lp.i0.even.end:
		jnz			.lp.i0.odd.end
.lp.i0.odd:
		movq		mm4, [energy+eax+ 0]
		movq		mm5, [energy+eax+ 8]
		add			eax, byte 16
		pfadd		mm0, mm4
		pfadd		mm1, mm5
		pfmul		mm4, [RW.cw+eax+0-16]
		pfmul		mm5, [RW.cw+eax+8-16]
		pfadd		mm2, mm4
		pfadd		mm3, mm5
.lp.i0.odd.end:
		add			edx, byte 16
		dec			ecx
		movq		[eb+edx+ 0-16], mm0
		movq		[eb+edx+ 8-16], mm1
		movq		[cb+edx+ 0-16], mm2
		movq		[cb+edx+ 8-16], mm3
		jnz			near .lp.b0
.lp.b0.end:

		mov			ecx, [RO.npart_l_orig]
		pxor		mm6, mm6
		sub			ecx, [RO.npart_l_pre_max]
		movq		mm7, [D_f0p4]
		jle			near .exit
		align		4
.lp.b1:
		mov			edi, [ebp]
		pxor		mm0, mm0	; ebb
		movq		mm1, mm6	; ebb
		add			ebp, byte 4
		dec			edi
		pxor		mm2, mm2	; ebb
		movq		mm3, mm6	; ebb
		jz			.lp.i1.odd
		align		4
.lp.i1.even:
		add			eax, byte 32
		sub			edi, byte 2
		pfadd		mm0, [energy+eax+ 0-32]
		pfadd		mm1, [energy+eax+ 8-32]
		pfadd		mm2, [energy+eax+16-32]
		pfadd		mm3, [energy+eax+24-32]
		jg			.lp.i1.even
.lp.i1.even.end:
		pfadd		mm0, mm2
		pfadd		mm1, mm3
		jnz			.lp.i1.odd.end
.lp.i1.odd:
		pfadd		mm0, [energy+eax+ 0]
		pfadd		mm1, [energy+eax+ 8]
		add			eax, byte 16
.lp.i1.odd.end:
		movq		[eb+edx+ 0], mm0
		movq		[eb+edx+ 8], mm1
		add			edx, byte 16
		dec			ecx
		pfmul		mm0, mm7
		pfmul		mm1, mm7
		movq		[cb+edx+ 0-16], mm0
		movq		[cb+edx+ 8-16], mm1
		jnz			near .lp.b1
.lp.b1.end:

.exit:
		femms
		pop			edi
		pop			ebp
		pop			ebx
		ret

%define BASE		(esp+_P)
%define E1@			(ebp+ 0)
%define R@			(ebp+16)
%define I@			(ebp+32)
%define E2@			(ebp+48)

%define k16			(ebx+8)  ;  32  48   k*16
%define mk16		(ecx)    ; -32 -48   k*16
%define j16			(ebx*4)  ;  96 160   j*16

%define _energy_s_k(i)	(edx+(i)*HBLKSIZE_s*16+k16)
%define _wsamp_s_k(i)	(eax+(i)*BLKSIZE_s*16+k16)
%define _wsamp_s_mk(i)	(eax+(i)*BLKSIZE_s*16+mk16)
%define _RW_cw(j)		(RW.cw+j16+(j)*16)

%imacro inner_psy_sub4_3DN_prologue 0
%assign LOCAL_SIZE	16*5+8
%assign _P 4*2+LOCAL_SIZE
		mov			ecx, [RO.cw_upper_index]
		mov			eax, [esp+4]
		push		ebx
		sub			ecx, byte 3
		push		ebp
		and			ecx, ~3
		sub			esp, byte LOCAL_SIZE
		lea			ecx, [ecx*4+8]
		lea			edx, [eax+gogo_thread_data_s.energy_s]  ; energy_s
		mov			ebx, ecx                                ; j*4
		neg			ecx
		lea			ebp, [esp+8]
		add			eax, gogo_thread_data_s.psywork			; wsamp_s
		lea			ecx, [ecx+BLKSIZE_s*16-8]				; (BLKSIZE_s-k)*16
		and			ebp, ~7
%endmacro

%imacro inner_psy_sub4_3DN_epilogue 0
		femms
		add			esp, byte LOCAL_SIZE
		pop			ebp
		pop			ebx
%endmacro

;	自分で作っておいてなんだが、何をやっているのか非常にわかりづらい(^^;; by kei
;	2001/12/03 kei	10.1Kclk@K7-500
;void inner_psy_sub4_C( gogo_thread_data *tl )
proc inner_psy_sub4_E3DN_HI
		inner_psy_sub4_3DN_prologue

		loopalignK7	4
.lp.j:
		movq		mm0, [_energy_s_k(0)+0]
		movq		mm1, [_energy_s_k(0)+8] 
		pfcmpeq		mm0, [D_0]
		pfcmpeq		mm1, [D_0]
		movq		mm6, mm0
		movq		mm7, mm1
		pxor		mm0, [D_im1]
		pxor		mm1, [D_im1]
		pand		mm6, [D_f1]
		pand		mm7, [D_f1]
		movq		mm2, mm0
		movq		mm3, mm1
		movq		mm4, mm0
		movq		mm5, mm1
		pand		mm0, [_energy_s_k(0)+0]
		pand		mm1, [_energy_s_k(0)+8]
		pand		mm2, [_wsamp_s_k(0)+0] ; wsamp_s[0][k*4+chn]
		pand		mm3, [_wsamp_s_k(0)+8] 
		pand		mm4, [_wsamp_s_mk(0)+0] ; wsamp_s[0][(BLKSIZE_s-k)*4+chn]
		pand		mm5, [_wsamp_s_mk(0)+8] 
		por			mm0, mm6
		por			mm1, mm7
		por			mm2, mm6 ; a
		por			mm3, mm7
		por			mm4, mm6 ; b
		por			mm5, mm7
		movq		mm6, [_energy_s_k(2)+0] ; tl->energy_s[2][k*4+chn]
		movq		mm7, [_energy_s_k(2)+8] 
		movq		[E1@+0], mm0
		movq		[E1@+8], mm1
		movq		mm0, mm2
		movq		mm1, mm3
		pfmul		mm2, mm2
		pfmul		mm3, mm3
		pfmul		mm0, mm4
		pfmul		mm1, mm5
		pfcmpeq		mm6, [D_0]
		pfcmpeq		mm7, [D_0]
		pfmul		mm4, mm4
		pfmul		mm5, mm5
		movq		[R@+0], mm0
		movq		[R@+8], mm1
		movq		mm0, mm6
		movq		mm1, mm7
		pfsub		mm2, mm4
		pfsub		mm3, mm5
		movq		mm4, mm6
		movq		mm5, mm7
		pfmul		mm2, [D_f0p5]
		pfmul		mm3, [D_f0p5]
		pand		mm6, [D_f1]
		pand		mm7, [D_f1]
		pandn		mm0, [_wsamp_s_k(2)+0] ; wsamp_s[2][k*4+chn]
		pandn		mm1, [_wsamp_s_k(2)+8]
		pandn		mm4, [_wsamp_s_mk(2)+0] ; wsamp_s[2][(BLKSIZE_s-k)*4+chn]
		pandn		mm5, [_wsamp_s_mk(2)+8]
		por			mm0, mm6 ; a
		por			mm1, mm7
		por			mm4, mm6 ; b
		por			mm5, mm7
		movq		[I@+0], mm2
		movq		[I@+8], mm3
		movq		mm2, mm0
		movq		mm3, mm1
		movq		mm6, mm4
		movq		mm7, mm5
		pfmul		mm0, [I@+0]
		pfmul		mm1, [I@+8]
		pfmul		mm4, [R@+0]
		pfmul		mm5, [R@+8]
		pfmul		mm2, [R@+0]
		pfmul		mm3, [R@+8]
		pfmul		mm6, [I@+0]
		pfmul		mm7, [I@+8]
		pfadd		mm0, mm4
		pfadd		mm1, mm5
		pfsub		mm2, mm6
		pfsub		mm3, mm7
		pfrsqrt		mm4, [_energy_s_k(0)+ 0] ; tl->energy_s[0][k*4+chn]
		pfrsqrt		mm5, [_energy_s_k(0)+ 4] 
		pfrsqrt		mm6, [_energy_s_k(0)+ 8]
		pfrsqrt		mm7, [_energy_s_k(0)+12]
		movq		[I@+0], mm0
		movq		[I@+8], mm1
		movq		[R@+0], mm2
		movq		[R@+8], mm3
		pfrsqrt		mm0, [_energy_s_k(2)+ 0] ; tl->energy_s[2][k*4+chn]
		pfrsqrt		mm1, [_energy_s_k(2)+ 4] 
		pfrsqrt		mm2, [_energy_s_k(2)+ 8]
		pfrsqrt		mm3, [_energy_s_k(2)+12]
		punpckldq	mm4, mm5
		punpckldq	mm6, mm7
		punpckldq	mm0, mm1
		punpckldq	mm2, mm3
        movq        mm5, mm4
        movq        mm7, mm6
        movq        mm1, mm0
        movq        mm3, mm2
        pfmul       mm4, mm4
        pfmul       mm6, mm6
        pfmul       mm0, mm0
        pfmul       mm2, mm2
        pfrsqit1    mm4, [_energy_s_k(0)+ 0]
        pfrsqit1    mm6, [_energy_s_k(0)+ 8]
        pfrsqit1    mm0, [_energy_s_k(2)+ 0]
        pfrsqit1    mm2, [_energy_s_k(2)+ 8]
        pfrcpit2    mm4, mm5 
        pfrcpit2    mm6, mm7
		movq		mm5, [_energy_s_k(2)+ 0] ; E2
		movq		mm7, [_energy_s_k(2)+ 8]
        pfrcpit2    mm0, mm1 
        pfrcpit2    mm2, mm3
		movq		mm1, [D_f1]
		movq		mm3, [D_f1]
		pfcmpeq		mm5, [D_0]
		pfcmpeq		mm7, [D_0]
		pfmul		mm4, [_energy_s_k(0)+ 0] ; r
		pfmul		mm6, [_energy_s_k(0)+ 8]
		pfmul		mm0, [_energy_s_k(2)+ 0] ; tmp
		pfmul		mm2, [_energy_s_k(2)+ 8]
		pand		mm1, mm5
		pand		mm3, mm7
		pfadd		mm4, mm4 ; r
		pfadd		mm6, mm6 
		pxor		mm5, [D_im1]
		pxor		mm7, [D_im1]
		pfsub		mm4, mm0 ; r
		pfsub		mm6, mm2 
		pand		mm0, mm5
		pand		mm2, mm7
		pand		mm5, [_energy_s_k(2)+ 0] 
		pand		mm7, [_energy_s_k(2)+ 8] 
		por			mm0, mm1 ; E2 ? tmp : 1.0
		por			mm2, mm3
		pfmul		mm0, [E1@+ 0] ; den
		pfmul		mm2, [E1@+ 8]
		por			mm5, mm1 ; E2 = E2 ? E2 : 1.0
		por			mm7, mm3
		pswapd		mm1, mm0
		pswapd		mm3, mm2
		movq		[E2@+0], mm5
		movq		[E2@+8], mm7
		pfrcp		mm5, mm0
		pfrcp		mm7, mm2
		pfrcp		mm1, mm1
		pfrcp		mm3, mm3
		punpckldq	mm5, mm1
		punpckldq	mm7, mm3
		pfrsqrt		mm1, [_energy_s_k(1)+ 0] ; tl->energy_s[1][k*4+chn]
		pfrsqrt		mm3, [_energy_s_k(1)+ 4] 
		pfrcpit1	mm0, mm5
		pfrcpit1	mm2, mm7
		pfrcpit2	mm0, mm5 ; 1.0/den
		pfrcpit2	mm2, mm7 
		pfrsqrt		mm5, [_energy_s_k(1)+ 8]
		pfrsqrt		mm7, [_energy_s_k(1)+12]

		punpckldq	mm1, mm3
		punpckldq	mm5, mm7
        movq        mm3, mm1
        movq        mm7, mm5
        pfmul       mm1, mm1
        pfmul       mm5, mm5
		pfmul		mm0, mm4 ; r/den
		pfmul		mm2, mm6
        pfrsqit1    mm1, [_energy_s_k(1)+ 0]
        pfrsqit1    mm5, [_energy_s_k(1)+ 8]
		pand		mm4, [D_ABS]
		pand		mm6, [D_ABS]
        pfrcpit2    mm1, mm3 
        pfrcpit2    mm5, mm7
		movq		mm3, [I@+0]
		movq		mm7, [I@+8]
        pfmul		mm1, [_energy_s_k(1)+ 0] ; sqrt(E3)
        pfmul		mm5, [_energy_s_k(1)+ 8] 
		pfmul		mm3, [_wsamp_s_k(1)+0]
		pfmul		mm7, [_wsamp_s_k(1)+8]
		pfadd		mm1, mm4 ; tmp
		pfadd		mm5, mm6
		movq		mm4, [R@+0]
		movq		mm6, [R@+8]
		pfmul		mm4, [_wsamp_s_mk(1)+0]
		pfmul		mm6, [_wsamp_s_mk(1)+8]
		pfadd		mm3, mm4
		pfadd		mm7, mm6
		pfmul		mm3, mm0
		pfmul		mm7, mm2
		pfmul		mm0, [E1@+0]
		pfmul		mm2, [E1@+8]
		pfsubr		mm3, [_energy_s_k(1)+ 0] ; tl->energy_s[1][k*4+chn]
		pfsubr		mm7, [_energy_s_k(1)+ 8]
		pfmul		mm0, mm0
		pfmul		mm2, mm2
		pfmul		mm0, [E2@+0]
		pfmul		mm2, [E2@+8]
		pfadd		mm0, mm3 ; E3
		pfadd		mm2, mm7
		pswapd		mm4, mm0 ; E3
		pswapd		mm6, mm2
		movq        mm3, mm0
		movq        mm7, mm2
		pfcmpgt		mm0, [D_0]
		pfcmpgt		mm2, [D_0]
		pfrsqrt     mm4, mm4
		pfrsqrt     mm6, mm6
		pand		mm1, mm0 ; tmp = E3 > 0 ? tmp : 0
		pfrsqrt     mm0, mm3
		pand		mm5, mm2
		pfrsqrt     mm2, mm7
		punpckldq   mm0, mm4
		punpckldq   mm2, mm6
		movq        mm4, mm0
		movq        mm6, mm2
		pfmul       mm0, mm0
		pfmul       mm2, mm2
		pfrsqit1    mm0, mm3
		pfrsqit1    mm2, mm7
		sub			ebx, byte 16
		pfrcpit2    mm0, mm4 
		pfrcpit2    mm2, mm6
		pfmul		mm0, mm3 ; sqrt(E3)
		pfmul		mm2, mm7
		pswapd		mm3, mm1 ; tmp
		pswapd		mm7, mm5
		pfrcp		mm4, mm1
		pfrcp		mm6, mm5
		pfrcp		mm3, mm3
		pfrcp		mm7, mm7
		punpckldq	mm4, mm3
		punpckldq	mm6, mm7
		movq		mm3, mm1
		movq		mm7, mm5
		pfrcpit1	mm1, mm4
		pfrcpit1	mm5, mm6
		pfcmpeq		mm3, [D_0]
		pfcmpeq		mm7, [D_0]
		pfrcpit2	mm1, mm4 ; 1.0/tmp
		pfrcpit2	mm5, mm6 
		pfmul		mm0, mm1
		pfmul		mm2, mm5
		add			ecx, byte 16
		pandn		mm3, mm0 ; tmp = tmp ? sqrt(E3)/tmp : 0
		pandn		mm7, mm2
		cmp			ebx, (cw_lower_index*4)
		movq		[_RW_cw(0)+0+64], mm3
		movq		[_RW_cw(0)+8+64], mm7
		movq		[_RW_cw(1)+0+64], mm3
		movq		[_RW_cw(1)+8+64], mm7
		movq		[_RW_cw(2)+0+64], mm3
		movq		[_RW_cw(2)+8+64], mm7
		movq		[_RW_cw(3)+0+64], mm3
		movq		[_RW_cw(3)+8+64], mm7
		jge			near .lp.j
.lp.j.end:

.exit:
		inner_psy_sub4_3DN_epilogue
		ret

;	2001/12/01 kei	10.5Kclk@K7-500
;	2001/12/03 kei	10.3Kclk@K7-500
proc inner_psy_sub4_3DN
%assign LOCAL_SIZE	16*5+8
%assign _P 4*2+LOCAL_SIZE
		inner_psy_sub4_3DN_prologue

		loopalignK6	4
.lp.j:
		movq		mm0, [_energy_s_k(0)+0]
		movq		mm1, [_energy_s_k(0)+8] 
		pfcmpeq		mm0, [D_0]
		pfcmpeq		mm1, [D_0]
		movq		mm6, mm0
		movq		mm7, mm1
		pxor		mm0, [D_im1]
		pxor		mm1, [D_im1]
		pand		mm6, [D_f1]
		pand		mm7, [D_f1]
		movq		mm2, mm0
		movq		mm3, mm1
		movq		mm4, mm0
		movq		mm5, mm1
		pand		mm0, [_energy_s_k(0)+0]
		pand		mm1, [_energy_s_k(0)+8]
		pand		mm2, [_wsamp_s_k(0)+0] ; wsamp_s[0][k*4+chn]
		pand		mm3, [_wsamp_s_k(0)+8] 
		pand		mm4, [_wsamp_s_mk(0)+0] ; wsamp_s[0][(BLKSIZE_s-k)*4+chn]
		pand		mm5, [_wsamp_s_mk(0)+8] 
		por			mm0, mm6
		por			mm1, mm7
		por			mm2, mm6 ; a
		por			mm3, mm7
		por			mm4, mm6 ; b
		por			mm5, mm7
		movq		mm6, [_energy_s_k(2)+0] ; tl->energy_s[2][k*4+chn]
		movq		mm7, [_energy_s_k(2)+8] 
		movq		[E1@+0], mm0
		movq		[E1@+8], mm1
		movq		mm0, mm2
		movq		mm1, mm3
		pfmul		mm2, mm2
		pfmul		mm3, mm3
		pfmul		mm0, mm4
		pfmul		mm1, mm5
		pfmul		mm4, mm4
		pfmul		mm5, mm5
		movq		[R@+0], mm0
		movq		[R@+8], mm1
		pfcmpeq		mm6, [D_0]
		pfcmpeq		mm7, [D_0]
		movq		mm0, mm6
		movq		mm1, mm7
		pfsub		mm2, mm4
		pfsub		mm3, mm5
		movq		mm4, mm6
		movq		mm5, mm7
		pand		mm6, [D_f1]
		pand		mm7, [D_f1]
		pfmul		mm2, [D_f0p5]
		pfmul		mm3, [D_f0p5]
		pandn		mm0, [_wsamp_s_k(2)+0] ; wsamp_s[2][k*4+chn]
		pandn		mm1, [_wsamp_s_k(2)+8]
		pandn		mm4, [_wsamp_s_mk(2)+0] ; wsamp_s[2][(BLKSIZE_s-k)*4+chn]
		pandn		mm5, [_wsamp_s_mk(2)+8]
		por			mm0, mm6 ; a
		por			mm1, mm7
		por			mm4, mm6 ; b
		por			mm5, mm7
		movq		[I@+0], mm2
		movq		[I@+8], mm3
		movq		mm2, mm0
		movq		mm3, mm1
		movq		mm6, mm4
		movq		mm7, mm5
		pfmul		mm0, [I@+0]
		pfmul		mm1, [I@+8]
		pfmul		mm4, [R@+0]
		pfmul		mm5, [R@+8]
		pfmul		mm2, [R@+0]
		pfmul		mm3, [R@+8]
		pfmul		mm6, [I@+0]
		pfmul		mm7, [I@+8]
		pfadd		mm0, mm4
		pfadd		mm1, mm5
		pfsub		mm2, mm6
		pfsub		mm3, mm7
		pfrsqrt		mm4, [_energy_s_k(0)+ 0] ; tl->energy_s[0][k*4+chn]
		pfrsqrt		mm5, [_energy_s_k(0)+ 4] 
		pfrsqrt		mm6, [_energy_s_k(0)+ 8]
		pfrsqrt		mm7, [_energy_s_k(0)+12]
		movq		[I@+0], mm0
		movq		[I@+8], mm1
		movq		[R@+0], mm2
		movq		[R@+8], mm3
		pfrsqrt		mm0, [_energy_s_k(2)+ 0] ; tl->energy_s[2][k*4+chn]
		pfrsqrt		mm1, [_energy_s_k(2)+ 4] 
		pfrsqrt		mm2, [_energy_s_k(2)+ 8]
		pfrsqrt		mm3, [_energy_s_k(2)+12]
		punpckldq	mm4, mm5
		punpckldq	mm6, mm7
		punpckldq	mm0, mm1
		punpckldq	mm2, mm3
        movq        mm5, mm4
        movq        mm7, mm6
        movq        mm1, mm0
        movq        mm3, mm2
        pfmul       mm4, mm4
        pfmul       mm6, mm6
        pfmul       mm0, mm0
        pfmul       mm2, mm2
        pfrsqit1    mm4, [_energy_s_k(0)+ 0]
        pfrsqit1    mm6, [_energy_s_k(0)+ 8]
        pfrsqit1    mm0, [_energy_s_k(2)+ 0]
        pfrsqit1    mm2, [_energy_s_k(2)+ 8]
        pfrcpit2    mm4, mm5 
        pfrcpit2    mm6, mm7
		movq		mm5, [_energy_s_k(2)+ 0] ; E2
		movq		mm7, [_energy_s_k(2)+ 8]
        pfrcpit2    mm0, mm1 
        pfrcpit2    mm2, mm3
		movq		mm1, [D_f1]
		movq		mm3, [D_f1]
		pfcmpeq		mm5, [D_0]
		pfcmpeq		mm7, [D_0]
		pfmul		mm4, [_energy_s_k(0)+ 0] ; r
		pfmul		mm6, [_energy_s_k(0)+ 8]
		pfmul		mm0, [_energy_s_k(2)+ 0] ; tmp
		pfmul		mm2, [_energy_s_k(2)+ 8]
		pand		mm1, mm5
		pand		mm3, mm7
		pfadd		mm4, mm4 ; r
		pfadd		mm6, mm6 
		pxor		mm5, [D_im1]
		pxor		mm7, [D_im1]
		pfsub		mm4, mm0 ; r
		pfsub		mm6, mm2 
		pand		mm0, mm5
		pand		mm2, mm7
		pand		mm5, [_energy_s_k(2)+ 0] 
		pand		mm7, [_energy_s_k(2)+ 8] 
		por			mm0, mm1 ; E2 ? tmp : 1.0
		por			mm2, mm3
		por			mm5, mm1 ; E2 = E2 ? E2 : 1.0
		por			mm7, mm3
		pfmul		mm0, [E1@+ 0] ; den
		pfmul		mm2, [E1@+ 8]
		movq		[E2@+0], mm5
		movq		[E2@+8], mm7
		movq		mm5, mm0
		movq		mm7, mm2
		pfrcp		mm1, mm0
		pfrcp		mm3, mm2
		punpckhdq	mm5, mm5
		punpckhdq	mm7, mm7
		pfrcp		mm5, mm5
		pfrcp		mm7, mm7
		punpckldq	mm1, mm5
		punpckldq	mm3, mm7
		pfrsqrt		mm5, [_energy_s_k(1)+ 0] ; tl->energy_s[1][k*4+chn]
		pfrsqrt		mm7, [_energy_s_k(1)+ 4] 
		pfrcpit1	mm0, mm1
		pfrcpit1	mm2, mm3
		pfrcpit2	mm0, mm1 ; 1.0/den
		pfrcpit2	mm2, mm3 
		pfrsqrt		mm1, [_energy_s_k(1)+ 8]
		pfrsqrt		mm3, [_energy_s_k(1)+12]
		pfmul		mm0, mm4 ; r/den
		pfmul		mm2, mm6
		punpckldq	mm5, mm7
		punpckldq	mm1, mm3
        movq        mm7, mm5
        movq        mm3, mm1
        pfmul       mm5, mm5
        pfmul       mm1, mm1
        pfrsqit1    mm5, [_energy_s_k(1)+ 0]
        pfrsqit1    mm1, [_energy_s_k(1)+ 8]
        pfrcpit2    mm5, mm7 
        pfrcpit2    mm1, mm3
		pand		mm4, [D_ABS]
		pand		mm6, [D_ABS]
        pfmul		mm5, [_energy_s_k(1)+ 0] ; sqrt(E3)
        pfmul		mm1, [_energy_s_k(1)+ 8] 
		movq		mm7, [I@+0]
		movq		mm3, [I@+8]
		pfadd		mm5, mm4 ; tmp
		pfadd		mm1, mm6
		movq		mm4, [R@+0]
		movq		mm6, [R@+8]
		pfmul		mm7, [_wsamp_s_k(1)+0]
		pfmul		mm3, [_wsamp_s_k(1)+8]
		pfmul		mm4, [_wsamp_s_mk(1)+0]
		pfmul		mm6, [_wsamp_s_mk(1)+8]
		pfadd		mm7, mm4
		pfadd		mm3, mm6
		pfmul		mm7, mm0
		pfmul		mm3, mm2
		pfmul		mm0, [E1@+0]
		pfmul		mm2, [E1@+8]
		pfsubr		mm7, [_energy_s_k(1)+ 0] ; tl->energy_s[1][k*4+chn]
		pfsubr		mm3, [_energy_s_k(1)+ 8]
		pfmul		mm0, mm0
		pfmul		mm2, mm2
		pfmul		mm0, [E2@+0]
		pfmul		mm2, [E2@+8]
		pfadd		mm0, mm7 ; E3
		pfadd		mm2, mm3
		movq		mm4, mm0 ; E3
		movq		mm6, mm2
		movq        mm7, mm0
		movq        mm3, mm2
		pfcmpgt		mm0, [D_0]
		pfcmpgt		mm2, [D_0]
		punpckhdq   mm4, mm4
		punpckhdq   mm6, mm6
		pand		mm5, mm0 ; tmp = E3 > 0 ? tmp : 0
		pfrsqrt     mm0, mm7
		pand		mm1, mm2
		pfrsqrt     mm4, mm4
		pfrsqrt     mm2, mm3
		pfrsqrt     mm6, mm6
		punpckldq   mm0, mm4
		punpckldq   mm2, mm6
		movq        mm4, mm0
		movq        mm6, mm2
		pfmul       mm0, mm0
		pfmul       mm2, mm2
		pfrsqit1    mm0, mm7
		pfrsqit1    mm2, mm3
		pfrcpit2    mm0, mm4 
		pfrcpit2    mm2, mm6
		pfmul		mm0, mm7 ; sqrt(E3)
		pfmul		mm2, mm3
		movq		mm7, mm5 ; tmp
		movq		mm3, mm1
		pfrcp		mm4, mm5
		pfrcp		mm6, mm1
		punpckhdq	mm5, mm5
		punpckhdq	mm1, mm1
		pfrcp		mm5, mm5
		pfrcp		mm1, mm1
		sub			ebx, byte 16
		punpckldq	mm4, mm5
		punpckldq	mm6, mm1
		movq		mm5, mm7
		movq		mm1, mm3
		pfrcpit1	mm7, mm4
		pfrcpit1	mm3, mm6
		add			ecx, byte 16
		pfrcpit2	mm7, mm4 ; 1.0/tmp
		pfrcpit2	mm3, mm6 
		pfmul		mm0, mm7
		pfmul		mm2, mm3
		pfcmpeq		mm5, [D_0]
		pfcmpeq		mm1, [D_0]
		cmp			ebx, (cw_lower_index*4)
		pandn		mm5, mm0 ; tmp = tmp ? sqrt(E3)/tmp : 0
		pandn		mm1, mm2
		movq		[_RW_cw(0)+0+64], mm5
		movq		[_RW_cw(0)+8+64], mm1
		movq		[_RW_cw(1)+0+64], mm5
		movq		[_RW_cw(1)+8+64], mm1
		movq		[_RW_cw(2)+0+64], mm5
		movq		[_RW_cw(2)+8+64], mm1
		movq		[_RW_cw(3)+0+64], mm5
		movq		[_RW_cw(3)+8+64], mm1

		jge			near .lp.j
.lp.j.end:

.exit:
		inner_psy_sub4_3DN_epilogue
		ret

;	2001/12/03 kei	7.7Kclk@K7-500
proc inner_psy_sub4_E3DN_FAST
%assign LOCAL_SIZE	16*5+8
%assign _P 4*2+LOCAL_SIZE
		inner_psy_sub4_3DN_prologue

		loopalignK7	4
.lp.j:
		movq		mm0, [_energy_s_k(0)+0]
		movq		mm1, [_energy_s_k(0)+8] 
		pfcmpeq		mm0, [D_0]
		pfcmpeq		mm1, [D_0]
		movq		mm6, mm0
		movq		mm7, mm1
		pxor		mm0, [D_im1]
		pxor		mm1, [D_im1]
		pand		mm6, [D_f1]
		pand		mm7, [D_f1]
		movq		mm2, mm0
		movq		mm3, mm1
		movq		mm4, mm0
		movq		mm5, mm1
		pand		mm0, [_energy_s_k(0)+0]
		pand		mm1, [_energy_s_k(0)+8]
		pand		mm2, [_wsamp_s_k(0)+0] ; wsamp_s[0][k*4+chn]
		pand		mm3, [_wsamp_s_k(0)+8] 
		pand		mm4, [_wsamp_s_mk(0)+0] ; wsamp_s[0][(BLKSIZE_s-k)*4+chn]
		pand		mm5, [_wsamp_s_mk(0)+8] 
		por			mm0, mm6
		por			mm1, mm7
		por			mm2, mm6 ; a
		por			mm3, mm7
		por			mm4, mm6 ; b
		por			mm5, mm7
		movq		mm6, [_energy_s_k(2)+0] ; tl->energy_s[2][k*4+chn]
		movq		mm7, [_energy_s_k(2)+8] 
		movq		[E1@+0], mm0
		movq		[E1@+8], mm1
		movq		mm0, mm2
		movq		mm1, mm3
		pfmul		mm2, mm2
		pfmul		mm3, mm3
		pfcmpeq		mm6, [D_0]
		pfcmpeq		mm7, [D_0]
		pfmul		mm0, mm4
		pfmul		mm1, mm5
		pfmul		mm4, mm4
		pfmul		mm5, mm5
		movq		[R@+0], mm0
		movq		[R@+8], mm1
		movq		mm0, mm6
		movq		mm1, mm7
		pfsub		mm2, mm4
		pfsub		mm3, mm5
		movq		mm4, mm6
		movq		mm5, mm7
		pfmul		mm2, [D_f0p5]
		pfmul		mm3, [D_f0p5]
		pand		mm6, [D_f1]
		pand		mm7, [D_f1]
		pandn		mm0, [_wsamp_s_k(2)+0] ; wsamp_s[2][k*4+chn]
		pandn		mm1, [_wsamp_s_k(2)+8]
		pandn		mm4, [_wsamp_s_mk(2)+0] ; wsamp_s[2][(BLKSIZE_s-k)*4+chn]
		pandn		mm5, [_wsamp_s_mk(2)+8]
		por			mm0, mm6 ; a
		por			mm1, mm7
		por			mm4, mm6 ; b
		por			mm5, mm7
		movq		[I@+0], mm2
		movq		[I@+8], mm3
		movq		mm2, mm0
		movq		mm3, mm1
		movq		mm6, mm4
		movq		mm7, mm5
		pfmul		mm0, [I@+0]
		pfmul		mm1, [I@+8]
		pfmul		mm4, [R@+0]
		pfmul		mm5, [R@+8]
		pfmul		mm2, [R@+0]
		pfmul		mm3, [R@+8]
		pfmul		mm6, [I@+0]
		pfmul		mm7, [I@+8]
		pfadd		mm0, mm4
		pfadd		mm1, mm5
		pfsub		mm2, mm6
		pfsub		mm3, mm7
		pfrsqrt		mm4, [_energy_s_k(0)+ 0] ; tl->energy_s[0][k*4+chn]
		pfrsqrt		mm5, [_energy_s_k(0)+ 4] 
		pfrsqrt		mm6, [_energy_s_k(0)+ 8]
		pfrsqrt		mm7, [_energy_s_k(0)+12]
		movq		[I@+0], mm0
		movq		[I@+8], mm1
		movq		[R@+0], mm2
		movq		[R@+8], mm3
		pfrsqrt		mm0, [_energy_s_k(2)+ 0] ; tl->energy_s[2][k*4+chn]
		pfrsqrt		mm1, [_energy_s_k(2)+ 4] 
		pfrsqrt		mm2, [_energy_s_k(2)+ 8]
		pfrsqrt		mm3, [_energy_s_k(2)+12]
		punpckldq	mm4, mm5
		punpckldq	mm6, mm7
		movq		mm5, [_energy_s_k(2)+ 0] ; E2
		movq		mm7, [_energy_s_k(2)+ 8]
		punpckldq	mm0, mm1
		punpckldq	mm2, mm3
		movq		mm1, [D_f1]
		movq		mm3, [D_f1]
		pfmul		mm4, [_energy_s_k(0)+ 0] ; r
		pfmul		mm6, [_energy_s_k(0)+ 8]
		pfcmpeq		mm5, [D_0]
		pfcmpeq		mm7, [D_0]
		pfmul		mm0, [_energy_s_k(2)+ 0] ; tmp
		pfmul		mm2, [_energy_s_k(2)+ 8]
		pand		mm1, mm5
		pand		mm3, mm7
		pfadd		mm4, mm4 ; r
		pfadd		mm6, mm6 
		pxor		mm5, [D_im1]
		pxor		mm7, [D_im1]
		pfsub		mm4, mm0 ; r
		pfsub		mm6, mm2 
		pand		mm0, mm5
		pand		mm2, mm7
		pand		mm5, [_energy_s_k(2)+ 0] 
		pand		mm7, [_energy_s_k(2)+ 8] 
		por			mm0, mm1 ; E2 ? tmp : 1.0
		por			mm2, mm3
		por			mm5, mm1 ; E2 = E2 ? E2 : 1.0
		por			mm7, mm3
		pfrsqrt		mm1, [_energy_s_k(1)+ 0] ; tl->energy_s[1][k*4+chn]
		pfrsqrt		mm3, [_energy_s_k(1)+ 4] 
		pfmul		mm0, [E1@+ 0] ; den
		pfmul		mm2, [E1@+ 8]
		movq		[E2@+0], mm5
		movq		[E2@+8], mm7
		pswapd		mm5, mm0
		pswapd		mm7, mm2
		pfrcp		mm0, mm0
		pfrcp		mm2, mm2
		pfrcp		mm5, mm5
		pfrcp		mm7, mm7
		punpckldq	mm0, mm5
		punpckldq	mm2, mm7
		pfrsqrt		mm5, [_energy_s_k(1)+ 8]
		pfrsqrt		mm7, [_energy_s_k(1)+12]
		pfmul		mm0, mm4 ; r/den
		pfmul		mm2, mm6
		pand		mm4, [D_ABS]
		pand		mm6, [D_ABS]
		punpckldq	mm1, mm3
		punpckldq	mm5, mm7
        pfmul		mm1, [_energy_s_k(1)+ 0] ; sqrt(E3)
        pfmul		mm5, [_energy_s_k(1)+ 8] 
		movq		mm3, [I@+0]
		movq		mm7, [I@+8]
		pfadd		mm1, mm4 ; tmp
		pfadd		mm5, mm6
		movq		mm4, [R@+0]
		movq		mm6, [R@+8]
		pfmul		mm3, [_wsamp_s_k(1)+0]
		pfmul		mm7, [_wsamp_s_k(1)+8]
		pfmul		mm4, [_wsamp_s_mk(1)+0]
		pfmul		mm6, [_wsamp_s_mk(1)+8]
		pfadd		mm3, mm4
		pfadd		mm7, mm6
		pfmul		mm3, mm0
		pfmul		mm7, mm2
		pfmul		mm0, [E1@+0]
		pfmul		mm2, [E1@+8]
		pfsubr		mm3, [_energy_s_k(1)+ 0] ; tl->energy_s[1][k*4+chn]
		pfsubr		mm7, [_energy_s_k(1)+ 8]
		pfmul		mm0, mm0
		pfmul		mm2, mm2
		pfmul		mm0, [E2@+0]
		pfmul		mm2, [E2@+8]
		pfadd		mm0, mm3 ; E3
		pfadd		mm2, mm7
		pswapd		mm4, mm0 ; E3
		pswapd		mm6, mm2
		movq        mm3, mm0
		movq        mm7, mm2
		pfcmpgt		mm0, [D_0]
		pfcmpgt		mm2, [D_0]
		pand		mm1, mm0 ; tmp = E3 > 0 ? tmp : 0
		pfrsqrt     mm0, mm3
		pand		mm5, mm2
		pfrsqrt     mm4, mm4
		pfrsqrt     mm2, mm7
		pfrsqrt     mm6, mm6
		punpckldq   mm0, mm4
		punpckldq   mm2, mm6
		pfmul		mm0, mm3 ; sqrt(E3)
		pfmul		mm2, mm7
		pswapd		mm3, mm1 ; tmp
		pswapd		mm7, mm5
		pfrcp		mm4, mm1
		pfrcp		mm6, mm5
		pfrcp		mm3, mm3
		pfrcp		mm7, mm7
		sub			ebx, byte 16
		punpckldq	mm4, mm3
		punpckldq	mm6, mm7
		pfmul		mm0, mm4
		pfmul		mm2, mm6
		pfcmpeq		mm1, [D_0]
		pfcmpeq		mm5, [D_0]
		add			ecx, byte 16
		pandn		mm1, mm0 ; tmp = tmp ? sqrt(E3)/tmp : 0
		pandn		mm5, mm2
		cmp			ebx, (cw_lower_index*4)
		movq		[_RW_cw(0)+0+64], mm1
		movq		[_RW_cw(0)+8+64], mm5
		movq		[_RW_cw(1)+0+64], mm1
		movq		[_RW_cw(1)+8+64], mm5
		movq		[_RW_cw(2)+0+64], mm1
		movq		[_RW_cw(2)+8+64], mm5
		movq		[_RW_cw(3)+0+64], mm1
		movq		[_RW_cw(3)+8+64], mm5
		jge			near .lp.j
.lp.j.end:

.exit:
		inner_psy_sub4_3DN_epilogue
		ret


		end



