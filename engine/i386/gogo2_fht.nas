;
;   part of this code is origined from
;
;       GOGO-no-coda
;       Copyright (c) 1999-2003 gogo-developer
;

%include "global.cfg"

	globaldef	__checkalign_fht__

	segment_data
	align	32
__checkalign_fht__:
F4_PR2_PR2_MR2_PR2	dd 0x3f3504f3, 0xbf3504f3, 0x3f3504f3, 0x3f3504f3
F4_MC1_PC3_MC3_PC1	dd 0x3f6c835e, 0xbec3ef15, 0x3ec3ef15, 0xbf6c835e
F4_PS1_PS3_PS3_PS1	dd 0x3ec3ef16, 0x3f6c835e, 0x3f6c835e, 0x3ec3ef16
F4_MR2_MR2_PR2_PR2	dd 0x3f3504f3, 0x3f3504f3, 0xbf3504f3, 0xbf3504f3
F4_PR2_PR2_PR2_PR2	dd 0x3f3504f3, 0x3f3504f3, 0x3f3504f3, 0x3f3504f3

D2_MSB1_0			dd 0x00000000, 0x80000000
F2_PR2_PR2			dd 0x3f3504f3, 0x3f3504f3

tbl_cos_fht:
	dd 0x00000000
	dd 0x3f3504f3,0x3f6c835e,0xbec3ef15,0x3f7b14be,0xbe47c5bc,0x3f0e39d9,0xbf54db31,0x3f7ec46d
	dd 0xbdc8bd35,0x3f226799,0xbf45e403,0x3f61c597,0xbef15ae7,0x3e94a030,0xbf74fa0b,0x3f7fb10f
	dd 0xbd48fb29,0x3f2beb49,0xbf3daef9,0x3f676bd8,0xbedae881,0x3eac7cd3,0xbf710908,0x3f7853f8
	dd 0xbe78cfc8,0x3f039c3c,0xbf5b941a,0x3f4d9f02,0xbf187fc0,0x3e164085,0xbf7d3aac,0x3f7fec43
	dd 0xbcc90a7e,0x3f3085ba,0xbf396842,0x3f6a09a6,0xbecf7bc9,0x3eb84427,0xbf6ed89e,0x3f79c79d
	dd 0xbe605c12,0x3f08f59b,0xbf584853,0x3f514d3d,0xbf13682a,0x3e2f10a0,0xbf7c3b28,0x3f7e1324
	dd 0xbdfab26c,0x3f1d7fd1,0xbf49d112,0x3f5ebe05,0xbefc5d27,0x3e888e93,0xbf76ba07,0x3f731447
	dd 0xbea09ae2,0x3ee63374,0xbf64aa59,0x3f41d870,0xbf273655,0x3d96a8fb,0xbf7f4e6d,0x3f7ffb11
	dd 0xbc490e86,0x3f32c8c9,0xbf373a22,0x3f6b4b0c,0xbec9b950,0x3ebe1d48,0xbf6db293,0x3f7a7302
	dd 0xbe5414fe,0x3f0b9a6b,0xbf5695e5,0x3f531848,0xbf10d3cd,0x3e3b6ecf,0xbf7baccd,0x3f7e70b0
	dd 0xbde1bc22,0x3f1ff6ca,0xbf47de65,0x3f604621,0xbef6e0c9,0x3e8e9a1f,0xbf75dec6,0x3f740bdd
	dd 0xbe9aa086,0x3eebcbbb,0xbf633c5a,0x3f43e200,0xbf24d225,0x3dafb67b,0xbf7f0e58,0x3f7f84ab
	dd 0xbd7b2b77,0x3f299414,0xbf3fc767,0x3f660f88,0xbee0924d,0x3ea68f10,0xbf721352,0x3f778bc5
	dd 0xbe827dc0,0x3f00e7e5,0xbf5d2d53,0x3f4bbbf8,0xbf1b02c5,0x3e09cf85,0xbf7dabcc,0x3f7cbfc9
	dd 0xbe22abb1,0x3f15f6d9,0xbf4f7a1f,0x3f59f26a,0xbf064b82,0x3e6c9a81,0xbf791298,0x3f6ff573
	dd 0xbeb263ef,0x3ed53641,0xbf68bf3b,0x3f3b8f3b,0xbf2e3bde,0x3d16c31c,0xbf7fd397,0x3f7ffec4
	dd 0xbbc90f4f,0x3f33e7bc,0xbf36206c,0x3f6be858,0xbec6d52a,0x3ec1071d,0xbf6d1c1d,0x3f7ac516
	dd 0xbe4dee5b,0x3f0cead0,0xbf55b992,0x3f53fac2,0xbf0f8784,0x3e419b39,0xbf7b61fc,0x3f7e9bc9
	dd 0xbdd53dbb,0x3f212ff8,0xbf46e229,0x3f6106f2,0xbef41f05,0x3e919ddb,0xbf756d97,0x3f748422
	dd 0xbe97a116,0x3eee9479,0xbf628210,0x3f44e3f5,0xbf239da8,0x3dbc3ac1,0xbf7eea9d,0x3f7f9c18
	dd 0xbd621467,0x3f2ac082,0xbf3ebc1b,0x3f66becc,0xbeddbe77,0x3ea986c3,0xbf718f57,0x3f77f110
	dd 0xbe7ee6de,0x3f0242b0,0xbf5c61c6,0x3f4cae79,0xbf19c200,0x3e1008b7,0xbf7d7474,0x3f7cfe73
	dd 0xbe1c76d8,0x3f173c07,0xbf4e8d90,0x3f5ac450,0xbf04f483,0x3e72b64c,0xbf78b47b,0x3f708066
	dd 0xbeaf713a,0x3ed8106b,0xbf6816a8,0x3f3ca003,0xbf2d1469,0x3d2fdffc,0xbf7fc38f,0x3f7fe129
	dd 0xbcfb4991,0x3f2f61a4,0xbf3a7ca4,0x3f696591,0xbed25a08,0x3eb554e9,0xbf6f6830,0x3f796e4e
	dd 0xbe667c66,0x3f07a136,0xbf591e6a,0x3f5064af,0xbf14b039,0x3e28def9,0xbf7c7eb0,0x3f7de0b1
	dd 0xbe039500,0x3f1c420c,0xbf4ac77f,0x3f5df6be,0xbeff17b3,0x3e8586ce,0xbf772417,0x3f7294f8
	dd 0xbea395c3,0x3ee363f9,0xbf655e0b,0x3f40d0da,0xbf286605,0x3d8a1ffe,0xbf7f6ac7,0x3f7f2f9d
	dd 0xbda33084,0x3f26050a,0xbf42de28,0x3f63f473,0xbee900b7,0x3e9d9e78,0xbf73913f,0x3f764d97
	dd 0xbe8b9504,0x3ef9a02c,0xbf5f8327,0x3f48d8b4,0xbf1ebc11,0x3dee386d,0xbf7e4323,0x3f7bf531
	dd 0xbe354097,0x3f121eaf,0xbf5233c6,0x3f577026,0xbf0a48ae,0x3e5a3995,0xbf7a1e84,0x3f6e46be
	dd 0xbebb319f,0x3ecc9b89,0xbf6aab7b,0x3f385215,0xbf31a81d,0x3c96c9ba,0xbf7ff4e6

tbl_sin_fht:
	dd 0x00000000
	dd 0x3f3504f3,0x3ec3ef16,0x3f6c835e,0x3e47c5c2,0x3f7b14bf,0x3f54db32,0x3f0e39da,0x3dc8bd36
	dd 0x3f7ec46d,0x3f45e403,0x3f22679a,0x3ef15aea,0x3f61c598,0x3f74fa0b,0x3e94a031,0x3d48fb30
	dd 0x3f7fb10f,0x3f3daefa,0x3f2beb4a,0x3edae880,0x3f676bd8,0x3f710908,0x3eac7cd4,0x3e78cfcd
	dd 0x3f7853f8,0x3f5b941b,0x3f039c3d,0x3f187fc0,0x3f4d9f02,0x3f7d3aac,0x3e164083,0x3cc90ab0
	dd 0x3f7fec43,0x3f396842,0x3f3085bb,0x3ecf7bcb,0x3f6a09a7,0x3f6ed89e,0x3eb8442a,0x3e605c13
	dd 0x3f79c79d,0x3f584853,0x3f08f59b,0x3f13682b,0x3f514d3d,0x3f7c3b28,0x3e2f10a3,0x3dfab273
	dd 0x3f7e1324,0x3f49d112,0x3f1d7fd2,0x3efc5d28,0x3f5ebe05,0x3f76ba07,0x3e888e94,0x3ea09ae5
	dd 0x3f731448,0x3f64aa59,0x3ee63375,0x3f273656,0x3f41d871,0x3f7f4e6d,0x3d96a905,0x3c490e90
	dd 0x3f7ffb11,0x3f373a23,0x3f32c8ca,0x3ec9b954,0x3f6b4b0c,0x3f6db293,0x3ebe1d4a,0x3e541502
	dd 0x3f7a7302,0x3f5695e5,0x3f0b9a6b,0x3f10d3cd,0x3f531849,0x3f7baccd,0x3e3b6ecf,0x3de1bc2e
	dd 0x3f7e70b0,0x3f47de66,0x3f1ff6cb,0x3ef6e0ca,0x3f604622,0x3f75dec7,0x3e8e9a22,0x3e9aa086
	dd 0x3f740bdd,0x3f633c5a,0x3eebcbbb,0x3f24d225,0x3f43e201,0x3f7f0e58,0x3dafb681,0x3d7b2b75
	dd 0x3f7f84ab,0x3f3fc767,0x3f299415,0x3ee0924f,0x3f660f88,0x3f721353,0x3ea68f13,0x3e827dc1
	dd 0x3f778bc5,0x3f5d2d53,0x3f00e7e4,0x3f1b02c5,0x3f4bbbf8,0x3f7dabcc,0x3e09cf87,0x3e22abb6
	dd 0x3f7cbfc9,0x3f4f7a1f,0x3f15f6da,0x3f064b83,0x3f59f26a,0x3f791298,0x3e6c9a80,0x3eb263ef
	dd 0x3f6ff573,0x3f68bf3c,0x3ed53642,0x3f2e3bde,0x3f3b8f3b,0x3f7fd398,0x3d16c32c,0x3bc90f88
	dd 0x3f7ffec4,0x3f36206c,0x3f33e7bc,0x3ec6d529,0x3f6be858,0x3f6d1c1d,0x3ec1071e,0x3e4dee60
	dd 0x3f7ac516,0x3f55b993,0x3f0cead1,0x3f0f8785,0x3f53fac3,0x3f7b61fc,0x3e419b37,0x3dd53dba
	dd 0x3f7e9bc9,0x3f46e22a,0x3f212ff9,0x3ef41f08,0x3f6106f3,0x3f756d98,0x3e919dde,0x3e97a117
	dd 0x3f748422,0x3f628210,0x3eee9479,0x3f239da9,0x3f44e3f5,0x3f7eea9d,0x3dbc3ac3,0x3d621469
	dd 0x3f7f9c18,0x3f3ebc1b,0x3f2ac082,0x3eddbe7a,0x3f66becd,0x3f718f57,0x3ea986c4,0x3e7ee6e1
	dd 0x3f77f111,0x3f5c61c7,0x3f0242b2,0x3f19c201,0x3f4cae7a,0x3f7d7474,0x3e1008b7,0x3e1c76de
	dd 0x3f7cfe73,0x3f4e8d90,0x3f173c07,0x3f04f484,0x3f5ac450,0x3f78b47b,0x3e72b651,0x3eaf713a
	dd 0x3f708066,0x3f6816a8,0x3ed8106c,0x3f2d146a,0x3f3ca003,0x3f7fc38f,0x3d2fe007,0x3cfb49ba
	dd 0x3f7fe129,0x3f3a7ca5,0x3f2f61a5,0x3ed25a0a,0x3f696591,0x3f6f6830,0x3eb554ec,0x3e667c66
	dd 0x3f796e4e,0x3f591e6a,0x3f07a136,0x3f14b03a,0x3f5064af,0x3f7c7eb0,0x3e28defd,0x3e039503
	dd 0x3f7de0b1,0x3f4ac77f,0x3f1c420c,0x3eff17b2,0x3f5df6be,0x3f772417,0x3e8586cf,0x3ea395c5
	dd 0x3f7294f9,0x3f655e0c,0x3ee363fb,0x3f286605,0x3f40d0da,0x3f7f6ac7,0x3d8a200a,0x3da3308c
	dd 0x3f7f2f9e,0x3f42de29,0x3f26050b,0x3ee900b7,0x3f63f473,0x3f73913f,0x3e9d9e79,0x3e8b9507
	dd 0x3f764d97,0x3f5f8327,0x3ef9a02d,0x3f1ebc12,0x3f48d8b4,0x3f7e4324,0x3dee3877,0x3e354098
	dd 0x3f7bf531,0x3f5233c7,0x3f121eb0,0x3f0a48ae,0x3f577025,0x3f7a1e84,0x3e5a3998,0x3ebb31a1
	dd 0x3f6e46bf,0x3f6aab7b,0x3ecc9b8b,0x3f31a81d,0x3f385216,0x3f7ff4e6,0x3c96c9b6

	segment_text
;------------------------------------------------------------------------
;void fht(float *fz, int n);

%define fz		BASE+_P+ 4
%define n		BASE+_P+ 8

%define LOCAL_SIZE 16
%define BASE esp+LOCAL_SIZE

%define kq@			esp+0
%define m_k@		esp+4

proc	gogo2_fht_SSE
		push		ebp
		push		edi
		push		esi
		push		ebx
		sub			esp, LOCAL_SIZE
%assign _P (4*4)

fht_SSE_part_1:
		mov			eax, [fz]
		
		;0-7
		movaps		xmm0, [eax]					;ar3_0
		movaps		xmm2, [eax+(4*4)]			;ar7_4
		movaps		xmm1, xmm0
		shufps		xmm0, xmm2, PACK(2,0,2,0)	;ar6 ar4 ar2 ar0
		shufps		xmm1, xmm2, PACK(3,1,3,1)	;ar7 ar5 ar3 ar1
		movaps		xmm2, xmm0
		subps		xmm0, xmm1					;xr7 xr5 xr3 xr1
		addps		xmm1, xmm2					;xr6 xr4 xr2 xr0
		movaps		xmm2, xmm1
		unpcklps	xmm1, xmm0					;ar3 ar2 ar1 ar0
		movaps		[eax], xmm1
		shufps		xmm0, xmm0, PACK(2,3,3,2)	;xr5 xr7 xr7 xr5
		mulps		xmm0, [F4_PR2_PR2_MR2_PR2]	;xr5' xr7' -xr7' xr5'
		movlhps		xmm3, xmm0					;-xr7' xr5' - -
		addps		xmm0, xmm3					;(xr5-xr7)' (xr5+xr7)' - -
		unpckhps	xmm2, xmm0					;ar7 ar6 ar5 ar4
		movaps		[eax+(4*4)], xmm2

		;8-15
		movaps		xmm0, [eax+(4*8)]			;ar3_0
		movaps		xmm2, [eax+(4*12)]			;ar7_4
		movaps		xmm1, xmm0
		shufps		xmm0, xmm2, PACK(2,0,2,0)	;ar6 ar4 ar2 ar0
		shufps		xmm1, xmm2, PACK(3,1,3,1)	;ar7 ar5 ar3 ar1
		movaps		xmm2, xmm0
		subps		xmm0, xmm1					;xr7 xr5 xr3 xr1
		addps		xmm1, xmm2					;xr6 xr4 xr2 xr0
		movaps		xmm2, xmm0
		shufps		xmm0, xmm0, PACK(0,1,2,3)	;xr1 xr3 xr5 xr7
		mulps		xmm2, [F4_MC1_PC3_MC3_PC1]	;-xr7 -xr5 xr3 xr1
		mulps		xmm0, [F4_PS1_PS3_PS3_PS1]	;xr1 xr3 xr5 xr7
		addps		xmm0, xmm2					;xr7' xr5' xr3' xr1'
		movaps		xmm2, xmm1
		unpcklps	xmm1, xmm0					;ar3 ar2 ar1 ar0
		movaps		[eax+(4*8)], xmm1
		unpckhps	xmm2, xmm0					;ar7 ar6 ar5 ar4
		movaps		[eax+(4*12)], xmm2

		;16-
		mov			ebx, (4*4)					;c
		mov			ecx, 32						;k*2
		lea			edx, [eax+(4*16)]			;fz+16
		lea			esi, [eax+(4*24)]			;fz+24
		xor			edi, edi					;i=0
		xor			ebp, ebp					;-i=0
		jmp			.lp_k
		align 16
	.lp_k:
	.lp_i:
		movaps		xmm0, [edx+edi]				;ar3_0
		movaps		xmm2, [edx+edi+(4*4)]		;ar7_4
		movaps		xmm1, xmm0
		shufps		xmm0, xmm2, PACK(2,0,2,0)	;ar6 ar4 ar2 ar0
		shufps		xmm1, xmm2, PACK(3,1,3,1)	;ar7 ar5 ar3 ar1
		movaps		xmm2, xmm0
		subps		xmm0, xmm1					;xr7 xr5 xr3 xr1
		addps		xmm1, xmm2					;xr6 xr4 xr2 xr0
		movaps		xmm2, [esi+ebp]				;ai3_0
		movaps		xmm4, [esi+ebp+(4*4)]		;ai7_4
		movaps		xmm3, xmm2
		shufps		xmm2, xmm4, PACK(2,0,2,0)	;ai6 ai4 ai2 ai0
		shufps		xmm3, xmm4, PACK(3,1,3,1)	;ai7 ai5 ai3 ai1
		movaps		xmm4, xmm2
		subps		xmm2, xmm3					;xi7 xi5 xi3 xi1
		addps		xmm3, xmm4					;xi6 xi4 xi2 xi0
		movaps		xmm4, [tbl_cos_fht+ebx]
		movaps		xmm5, [tbl_sin_fht+ebx]
		movaps		xmm6, xmm0
		mulps		xmm0, xmm4					;c*(xr7 xr5 xr3 xr1)
		mulps		xmm6, xmm5					;s*(xr7 xr5 xr3 xr1)
		shufps		xmm2, xmm2, PACK(0,1,2,3)
		mulps		xmm5, xmm2					;s*(xi1 xi3 xi5 xi7)
		mulps		xmm4, xmm2					;c*(xi1 xi3 xi5 xi7)
		addps		xmm0, xmm5					;r = c*r + s*i
		subps		xmm6, xmm4					;i = s*r - c*i
		movaps		xmm2, xmm1
		unpcklps	xmm1, xmm0					;ar3 ar2 ar1 ar0
		movaps		[edx+edi], xmm1
		unpckhps	xmm2, xmm0					;ar7 ar6 ar5 ar4
		movaps		[edx+edi+(4*4)], xmm2
		shufps		xmm6, xmm6, PACK(0,1,2,3)
		movaps		xmm2, xmm3
		unpcklps	xmm3, xmm6					;ai3 ai2 ai1 ai0
		movaps		[esi+ebp], xmm3
		unpckhps	xmm2, xmm6					;ai7 ai6 ai5 ai4
		movaps		[esi+ebp+(4*4)], xmm2
	.lp_i_next:
		add			edi, 4*8					;i+=4
		sub			ebp, 4*8					;-i+=4
		add			ebx, (4*4)					;c+=4
		cmp			edi, ecx					;if(i<kq)
		jl near		.lp_i
	.lp_k_next:
		lea			edx, [edx+ecx*2]			;kr+=k
		lea			esi, [esi+ecx*4]			;lr+=k*2
		add			ecx, ecx					;k*=2
		xor			edi, edi					;i=0
		xor			ebp, ebp					;-i=0
		cmp			ecx, [n]					;if(k*2<=n)
		jle near	.lp_k

fht_SSE_part_2:
		;0-7
		movlps		xmm0, [eax+(4*0)]			;ar1_0
		movlps		xmm2, [eax+(4*2)]			;ar3_2
		movhps		xmm0, [eax+(4*4)]			;ar5_4 ar1_0
		movhps		xmm2, [eax+(4*6)]			;ar7_6 ar3_2
		movaps		xmm1, xmm0
		addps		xmm0, xmm2					;xr5_4 xr1_0
		subps		xmm1, xmm2					;xr7_6 xr3_2
		movlps		[eax+(4*0)], xmm0
		movhps		[eax+(4*4)], xmm0
		movlps		[eax+(4*2)], xmm1
		movhps		[eax+(4*6)], xmm1

		;8-15
		movlps		xmm0, [eax+(4*8 )]			;ar1_0
		movlps		xmm2, [eax+(4*10)]			;ar3_2
		movhps		xmm0, [eax+(4*12)]			;ar5_4 ar1_0
		movhps		xmm2, [eax+(4*14)]			;ar7_6 ar3_2
		movaps		xmm1, xmm0
		subps		xmm0, xmm2					;xr7_6 xr3_2
		addps		xmm1, xmm2					;xr5_4 xr1_0
		movaps		xmm2, xmm0
		shufps		xmm0, xmm0, PACK(1,0,3,2)	;xr3_2 xr7_6
		movlps		[eax+(4*8 )], xmm1
		movhps		[eax+(4*12)], xmm1
		mulps		xmm2, [F4_MR2_MR2_PR2_PR2]	;-xr7_6' xr3_2'
		mulps		xmm0, [F4_PR2_PR2_PR2_PR2]	;xr3_2' xr7_6'
		addps		xmm0, xmm2					;ar7_6 ar3_2
		movlps		[eax+(4*10)], xmm0
		movhps		[eax+(4*14)], xmm0

		;16-
		mov			ebx, (4*2)					;c
		mov			ecx, 32						;k*4
		lea			edx, [eax+(4*16)]			;fz+16
		lea			esi, [eax+(4*24)]			;fz+24
		xor			edi, edi					;i=0
		xor			ebp, ebp					;-i=0
		jmp			.lp_k
		align 16
	.lp_k:
	.lp_i:
		movlps		xmm0, [edx+edi+(4*0)]		;ar1_0
		movlps		xmm2, [edx+edi+(4*2)]		;ar3_2
		movhps		xmm0, [edx+edi+(4*4)]		;ar5_4 ar1_0
		movhps		xmm2, [edx+edi+(4*6)]		;ar7_6 ar3_2
		movaps		xmm1, xmm0
		addps		xmm0, xmm2					;xr5_4 xr1_0
		subps		xmm1, xmm2					;xr7_6 xr3_2
		movlps		xmm6, [tbl_cos_fht+ebx]
		movlps		xmm7, [tbl_sin_fht+ebx]
		movlps		[edx+edi+(4*0)], xmm0
		movhps		[edx+edi+(4*4)], xmm0
		shufps		xmm6, xmm6, PACK(1,1,0,0)	;c1 c1 c0 c0
		shufps		xmm7, xmm7, PACK(1,1,0,0)	;s1 s1 s0 s0
		movlps		xmm2, [esi+ebp+(4*0)]		;ai1_0
		movlps		xmm4, [esi+ebp+(4*2)]		;ai3_2
		movhps		xmm2, [esi+ebp+(4*4)]		;ai5_4 ai1_0
		movhps		xmm4, [esi+ebp+(4*6)]		;ai7_6 ai3_2
		movaps		xmm3, xmm2
		addps		xmm2, xmm4					;xi5_4 xi1_0
		subps		xmm3, xmm4					;xi7_6 xi3_2
		movaps		xmm0, xmm1
		mulps		xmm1, xmm6					;c1*xr7_6 c0*xr3_2
		mulps		xmm0, xmm7					;s1*xr7_6 s0*xr3_2
		shufps		xmm3, xmm3, PACK(1,0,3,2)	;xi3_2 xi7_6
		mulps		xmm7, xmm3					;s1*xi3_2 s0*xi7_6
		mulps		xmm6, xmm3					;c1*xi3_2 c0*xi7_6
		movlps		[esi+ebp+(4*0)], xmm2
		movhps		[esi+ebp+(4*4)], xmm2
		addps		xmm1, xmm7					;r = c*r + s*i
		subps		xmm0, xmm6					;i = s*r - c*i
		movlps		[edx+edi+(4*2)], xmm1
		movhps		[edx+edi+(4*6)], xmm1
		movhps		[esi+ebp+(4*2)], xmm0
		movlps		[esi+ebp+(4*6)], xmm0
	.lp_i_next:
		add			edi, 2*16					;i+=2
		sub			ebp, 2*16					;-i+=2
		add			ebx, (4*2)					;c+=2
		cmp			edi, ecx					;if(i<kq)
		jl near		.lp_i
	.lp_k_next:
		lea			edx, [edx+ecx*2]			;kr+=k*2
		lea			esi, [esi+ecx*4]			;lr+=k*4
		add			ecx, ecx					;k*=2
		xor			edi, edi					;i=0
		xor			ebp, ebp					;-i=0
		cmp			ecx, [n]					;if(k*4<=n)
		jle near	.lp_k

fht_SSE_part_3:
		mov			ebp, 8						;m=8
		mov			ebx, eax					;&a[0]
		add			ebx, 16						;&a[mh]
		xor			ecx, ecx					;k*2=0
		mov			edx, ebp					;mh*2
		jmp			.lp_ebx
		align 16
	.lp_m
	.lp_ebx:
		movaps		xmm0, [eax+ecx*2]
		movaps		xmm2, [ebx+ecx*2]
		movaps		xmm1, xmm0
		addps		xmm0, xmm2
		subps		xmm1, xmm2
		movaps		[eax+ecx*2], xmm0
		movaps		[ebx+ecx*2], xmm1
		add			ecx, 8						;k+=4
		sub			edx, 8						;if(k<mh)
		jnz			.lp_ebx
		cmp			ebp, [n]					;if(m==n)
		je near		.lp_m_exit

		lea			eax, [eax+ebp*4]			;&a[m]
		lea			ebx, [ebx+ebp*4]			;&a[m+mh]
		xor			ecx, ecx					;k*2=0
		mov			edx, ebp					;mh*2
		jmp			.lp_ecx
		align 16
	.lp_ecx:
		movaps		xmm0, [eax+ecx*2]
		movaps		xmm2, [ebx+ecx*2]
		movaps		xmm1, xmm0
		addps		xmm0, xmm2
		subps		xmm1, xmm2
		movaps		[eax+ecx*2], xmm0
		movaps		[ebx+ecx*2], xmm1
		add			ecx, 8						;k+=4
		sub			edx, 8						;if(k<mh)
		jnz			.lp_ecx
		mov			ecx, ebp
		add			ecx, ebp
		cmp			ecx, [n]					;if(m*2==n)
		je near		.lp_m_next

		lea			eax, [eax+ebp*4]			;&a[m*2]
		lea			ebx, [ebx+ebp*4]			;&a[m*2+mh]
		xor			ecx, ecx					;k*2=0
		lea			edx, [eax+ebp*4]			;&a[m*3]
		lea			esi, [ebx+ebp*4]			;&a[m*3+mh]
		movaps		xmm7, [F4_PR2_PR2_PR2_PR2]
		mov			edi, ebp					;mh*2
		jmp			.lp_c
		align 16
	.lp_c:
		movaps		xmm0, [eax+ecx*2]
		movaps		xmm2, [ebx+ecx*2]
		movaps		xmm1, xmm0
		subps		xmm0, xmm2
		addps		xmm1, xmm2
		movaps		xmm3, [edx+ecx*2]
		movaps		xmm5, [esi+ecx*2]
		movaps		xmm4, xmm3
		subps		xmm3, xmm5
		addps		xmm4, xmm5
		movaps		[eax+ecx*2], xmm1
		movaps		xmm2, xmm0
		addps		xmm0, xmm3
		subps		xmm2, xmm3
		movaps		[edx+ecx*2], xmm4
		mulps		xmm0, xmm7
		mulps		xmm2, xmm7
		movaps		[ebx+ecx*2], xmm0
		movaps		[esi+ecx*2], xmm2
		add			ecx, 8						;k+=4
		sub			edi, 8						;if(k<mh)
		jnz			.lp_c
		lea			ecx, [ebp*4]
		cmp			ecx, [n]					;if(m*4==n)
		je near		.lp_m_next

		lea			edx, [ebp*8]				;m*k
		mov			ecx, [fz]
		mov			[m_k@], edx
		lea			eax, [ecx+edx*2]			;&a[m*kh]
		sub			edx, ebp
		lea			ebx, [ecx+edx*4]			;&a[m*k-m]
		movss		xmm6, [tbl_cos_fht+(4*2)]
		movss		xmm7, [tbl_sin_fht+(4*2)]
		mov			edi, (4*3)					;c=3
		shufps		xmm6, xmm6, PACK(0,0,0,0)
		shufps		xmm7, xmm7, PACK(0,0,0,0)
		mov			ecx, 2						;kq=2
		mov			[kq@], ecx
		xor			edx, edx					;j*2=0
		mov			esi, ebp					;(j+mh)*2
		jmp			.lp_j
		align 16
	.lp_k:
	.lp_i:
	.lp_j:
		movaps		xmm0, [eax+edx*2]
		movaps		xmm2, [eax+esi*2]
		movaps		xmm1, xmm0
		subps		xmm0, xmm2
		addps		xmm1, xmm2
		movaps		xmm3, [ebx+edx*2]
		movaps		xmm5, [ebx+esi*2]
		movaps		xmm4, xmm3
		subps		xmm3, xmm5
		addps		xmm4, xmm5
		movaps		[eax+edx*2], xmm1
		movaps		xmm2, xmm0
		mulps		xmm0, xmm6
		mulps		xmm2, xmm7
		movaps		[ebx+edx*2], xmm4
		movaps		xmm5, xmm3
		mulps		xmm3, xmm7
		mulps		xmm5, xmm6
		addps		xmm0, xmm3
		subps		xmm2, xmm5
		movaps		[eax+esi*2], xmm0
		movaps		[ebx+esi*2], xmm2
	.lp_j_next:
		add			edx, 8						;j*2+=4
		add			esi, 8						;(j+mh)*2+=4
		cmp			edx, ebp					;if(j<mh)
		jl			.lp_j
	.lp_i_next:
		lea			edx, [ebp*4]
		movss		xmm6, [tbl_cos_fht+edi]
		movss		xmm7, [tbl_sin_fht+edi]
		add			eax, edx					;kr+=m
		sub			ebx, edx					;lr-=m
		add			edi, 4
		shufps		xmm6, xmm6, PACK(0,0,0,0)
		shufps		xmm7, xmm7, PACK(0,0,0,0)
		xor			edx, edx					;j*2=0
		mov			esi, ebp					;(j+mh)*2
		dec			ecx							;if(i<kq)
		jnz near	.lp_i
	.lp_k_next:
		mov			edx, [m_k@]
		mov			ecx, [fz]
		add			edx, edx
		mov			[m_k@], edx					;(m*k)*=2
		lea			eax, [ecx+edx*2]			;&a[m*kh]
		mov			ebx, edx
		sub			ebx, ebp
		lea			ebx, [ecx+ebx*4]			;&a[m*k-m]
		mov			ecx, [kq@]
		add			ecx, ecx
		cmp			edx, [n]
		mov			[kq@], ecx					;k*=2
		mov			edx, 0
		jle near	.lp_k

	.lp_m_next:
		mov			eax, [fz]					;&a[0]
		lea			ebx, [eax+ebp*4]			;&a[mh]
		add			ebp, ebp
		xor			ecx, ecx					;k*2=0
		mov			edx, ebp					;mh*2
		cmp			ebp, [n]
		jle near	.lp_m

	.lp_m_exit:
		add			esp, LOCAL_SIZE
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret

proc	gogo2_fht_3DN
		push		ebp
		push		edi
		push		esi
		push		ebx
		sub			esp, LOCAL_SIZE
		femms
%assign _P (4*4)

fht_3DN_part_1:
		mov			eax, [fz]
		movq		mm7, [D2_MSB1_0]
		
		;0-3
		movq		mm0, [eax]					;ar1 ar0
		movq		mm2, mm7
		movq		mm1, [eax+(4*2)]			;ar3 ar2
		movq		mm3, mm7
		pxor		mm2, mm0					;-ar1 ar0
		pxor		mm3, mm1					;-ar3 ar2
		pfacc		mm0, mm2					;ar1 ar0
		pfacc		mm1, mm3					;ar3 ar2
		movq		[eax], mm0
		movq		[eax+(4*2)], mm1

		;4-7
		movq		mm0, [eax+(4*4)]			;ar1 ar0
		movq		mm2, [eax+(4*6)]			;ar3 ar2
		movq		mm1, mm0
		punpckldq	mm0, mm2					;ar2 ar0
		punpckhdq	mm1, mm2					;ar3 ar1
		movq		mm2, mm0
		pfsub		mm0, mm1					;xr3 xr1
		pfadd		mm2, mm1					;ar2 ar0
		movq		mm1, mm0
		pxor		mm0, mm7					;-xr3 xr1
		pfacc		mm1, mm0					;xr3' xr1'
		pfmul		mm1, [F2_PR2_PR2]			;ar3 ar1
		movq		mm0, mm2
		punpckldq	mm2, mm1					;ar0 ar1
		punpckhdq	mm0, mm1					;ar3 ar2
		movq		[eax+(4*4)], mm2
		movq		[eax+(4*6)], mm0

		;8-
		mov			ebx, (4*2)					;c=2
		mov			ecx, 16						;k*2
		lea			edx, [eax+(4*8)]			;fz+8
		lea			esi, [eax+(4*12)]			;fz+12
		xor			edi, edi					;i*8=0
		xor			ebp, ebp					;-i*8=0
		jmp			.lp_k
		align 16
	.lp_k:
	.lp_i:
		movq		mm0, [edx+edi]				;ar1 ar0
		movq		mm2, [edx+edi+(4*2)]		;ar3 ar2
		movq		mm1, mm0
		punpckldq	mm0, mm2					;ar2 ar0
		punpckhdq	mm1, mm2					;ar3 ar1
		movq		mm2, mm0
		pfsub		mm0, mm1					;xr3 xr1
		pfadd		mm1, mm2					;ar2 ar0
		movq		mm2, [esi+ebp]				;ai1 ai0
		movq		mm4, [esi+ebp+(4*2)]		;ai3 ai2
		movq		mm3, mm2
		punpckldq	mm2, mm4					;ai2 ai0
		punpckhdq	mm3, mm4					;ai3 ai1
		movq		mm4, mm2
		pfsub		mm2, mm3					;xi3 xi1
		pfadd		mm3, mm4					;ai2 ai0
		movq		mm4, [tbl_cos_fht+ebx]
		movq		mm5, [tbl_sin_fht+ebx]
		movq		mm6, mm0
		punpckldq	mm7, mm2
		pfmul		mm0, mm4					;c*(xr3 xr1)
		punpckhdq	mm2, mm7
		pfmul		mm6, mm5					;s*(xr3 xr1)
		pfmul		mm5, mm2					;s*(xi1 xi3)
		pfmul		mm4, mm2					;c*(xi1 xi3)
		pfadd		mm0, mm5					;r = c*r + s*i
		pfsub		mm6, mm4					;i = s*r - c*i
		movq		mm2, mm1
		punpckldq	mm1, mm0					;ar1 ar0
		punpckhdq	mm2, mm0					;ar3 ar2
		punpckldq	mm7, mm3					;ai0 - 
		movq		[edx+edi], mm1
		punpckhdq	mm3, mm3					;ai2 -
		movq		[edx+edi+(4*2)], mm2
		punpckhdq	mm7, mm6					;ai1 ai0
		punpckldq	mm3, mm6					;ai3 ai2
		movq		[esi+ebp], mm7
		movq		[esi+ebp+(4*2)], mm3
	.lp_i_next:
		add			edi, 2*8					;i*8+=2
		sub			ebp, 2*8					;-i*8+=2
		add			ebx, (4*2)					;c+=2
		cmp			edi, ecx					;if(i<kq)
		jl near		.lp_i
	.lp_k_next:
		lea			edx, [edx+ecx*2]			;kr+=k
		lea			esi, [esi+ecx*4]			;lr+=k*2
		add			ecx, ecx					;k*=2
		xor			edi, edi					;i*8=0
		xor			ebp, ebp					;-i*8=0
		cmp			ecx, [n]					;if(k*2<=n)
		jle near	.lp_k

fht_3DN_part_2:
		mov			ebp, 4						;m=4
		mov			ebx, eax					;&a[0]
		add			ebx, 8						;&a[mh]
		xor			ecx, ecx					;k*2=0
		mov			edx, ebp					;mh*2
		jmp			.lp_ebx
		align 16
	.lp_m
	.lp_ebx:
		movq		mm0, [eax+ecx*2]
		movq		mm2, [ebx+ecx*2]
		movq		mm1, mm0
		pfadd		mm0, mm2
		pfsub		mm1, mm2
		movq		[eax+ecx*2], mm0
		movq		[ebx+ecx*2], mm1
		add			ecx, 4						;k*2+=2
		sub			edx, 4						;if(k<mh)
		jnz			.lp_ebx
		cmp			ebp, [n]					;if(m==n)
		je near		.lp_m_exit

		lea			eax, [eax+ebp*4]			;&a[m]
		lea			ebx, [ebx+ebp*4]			;&a[m+mh]
		xor			ecx, ecx					;k*2=0
		mov			edx, ebp					;mh*2
		jmp			.lp_ecx
		align 16
	.lp_ecx:
		movq		mm0, [eax+ecx*2]
		movq		mm2, [ebx+ecx*2]
		movq		mm1, mm0
		pfadd		mm0, mm2
		pfsub		mm1, mm2
		movq		[eax+ecx*2], mm0
		movq		[ebx+ecx*2], mm1
		add			ecx, 4						;k*2+=2
		sub			edx, 4						;if(k<mh)
		jnz			.lp_ecx
		mov			ecx, ebp
		add			ecx, ebp
		cmp			ecx, [n]					;if(m*2==n)
		je near		.lp_m_next

		lea			eax, [eax+ebp*4]			;&a[m*2]
		lea			ebx, [ebx+ebp*4]			;&a[m*2+mh]
		xor			ecx, ecx					;k*2=0
		lea			edx, [eax+ebp*4]			;&a[m*3]
		lea			esi, [ebx+ebp*4]			;&a[m*3+mh]
		movq		mm7, [F2_PR2_PR2]
		mov			edi, ebp					;mh*2
		jmp			.lp_c
		align 16
	.lp_c:
		movq		mm0, [eax+ecx*2]
		movq		mm2, [ebx+ecx*2]
		movq		mm1, mm0
		pfsub		mm0, mm2
		pfadd		mm1, mm2
		movq		mm3, [edx+ecx*2]
		movq		mm5, [esi+ecx*2]
		movq		mm4, mm3
		pfsub		mm3, mm5
		pfadd		mm4, mm5
		movq		mm2, mm0
		pfadd		mm0, mm3
		pfsub		mm2, mm3
		movq		[eax+ecx*2], mm1
		pfmul		mm0, mm7
		movq		[edx+ecx*2], mm4
		pfmul		mm2, mm7
		movq		[ebx+ecx*2], mm0
		movq		[esi+ecx*2], mm2
		add			ecx, 4						;k*2+=4
		sub			edi, 4						;if(k<mh)
		jnz			.lp_c
		lea			ecx, [ebp*4]
		cmp			ecx, [n]					;if(m*4==n)
		je near		.lp_m_next

		lea			edx, [ebp*8]				;m*k
		mov			ecx, [fz]
		mov			[m_k@], edx
		lea			eax, [ecx+edx*2]			;&a[m*kh]
		sub			edx, ebp
		lea			ebx, [ecx+edx*4]			;&a[m*k-m]
		movd		mm6, [tbl_cos_fht+(4*2)]
		movd		mm7, [tbl_sin_fht+(4*2)]
		mov			edi, (4*3)					;c=3
		punpckldq	mm6, mm6
		punpckldq	mm7, mm7
		mov			ecx, 2						;kq=2
		mov			[kq@], ecx
		xor			edx, edx					;j*2=0
		mov			esi, ebp					;(j+mh)*2
		jmp			.lp_j
		align 16
	.lp_k:
	.lp_i:
	.lp_j:
		movq		mm0, [eax+edx*2]
		movq		mm2, [eax+esi*2]
		movq		mm1, mm0
		pfsub		mm0, mm2
		pfadd		mm1, mm2
		movq		mm3, [ebx+edx*2]
		movq		mm5, [ebx+esi*2]
		movq		mm4, mm3
		pfsub		mm3, mm5
		pfadd		mm4, mm5
		movq		mm2, mm0
		pfmul		mm0, mm6
		pfmul		mm2, mm7
		movq		mm5, mm3
		pfmul		mm3, mm7
		pfmul		mm5, mm6
		movq		[eax+edx*2], mm1
		pfadd		mm0, mm3
		movq		[ebx+edx*2], mm4
		pfsub		mm2, mm5
		movq		[eax+esi*2], mm0
		movq		[ebx+esi*2], mm2
	.lp_j_next:
		add			edx, 4						;j*2+=2
		add			esi, 4						;(j+mh)*2+=2
		cmp			edx, ebp					;if(j<mh)
		jl			.lp_j
	.lp_i_next:
		lea			edx, [ebp*4]
		movd		mm6, [tbl_cos_fht+edi]
		movd		mm7, [tbl_sin_fht+edi]
		add			eax, edx					;kr+=m
		sub			ebx, edx					;lr-=m
		add			edi, 4
		punpckldq	mm6, mm6
		punpckldq	mm7, mm7
		xor			edx, edx					;j*2=0
		mov			esi, ebp					;(j+mh)*2
		dec			ecx							;if(i<kq)
		jnz near	.lp_i
	.lp_k_next:
		mov			edx, [m_k@]
		mov			ecx, [fz]
		add			edx, edx
		mov			[m_k@], edx					;(m*k)*=2
		lea			eax, [ecx+edx*2]			;&a[m*kh]
		mov			ebx, edx
		sub			ebx, ebp
		lea			ebx, [ecx+ebx*4]			;&a[m*k-m]
		mov			ecx, [kq@]
		add			ecx, ecx
		cmp			edx, [n]
		mov			[kq@], ecx					;k*=2
		mov			edx, 0
		jle near	.lp_k

	.lp_m_next:
		mov			eax, [fz]					;&a[0]
		lea			ebx, [eax+ebp*4]			;&a[mh]
		add			ebp, ebp
		xor			ecx, ecx					;k*2=0
		mov			edx, ebp					;mh*2
		cmp			ebp, [n]
		jle near	.lp_m

	.lp_m_exit:
		femms
		add			esp, LOCAL_SIZE
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret

		end
