;
;   part of this code is origined from
;
;       GOGO-no-coda
;       Copyright (c) 1999-2003 shigeo
;       Copyright (c) 2001,2002,2003 gogo-developer

%include "nasm.cfg"

		globaldef	CLKcount
		globaldef	CLKclock
%if 0
		externdef	RO		; RO.printf ¤¬É¬Í×
%else
		externdef	errPrintf
%endif

;		segment_bss
		segment_data
		align	16
CLKclock	dd	0,0
CLKcount	dd	0
fmt_stat	db	"call %dtimes:ave %fclk",13,10,0

		segment_text

; void clkbegin(void)
proc clkbegin
        push    eax
        push    edx
%ifdef __tos__
		cli
%endif
        rdtsc
        sub     [CLKclock],eax
        sbb     [CLKclock+4],edx
        pop     edx
        pop     eax
        ret

; void clkend(void)
proc clkend
        push    eax
        push    edx
        rdtsc
%ifdef __tos__
		cli
%endif
        add     [CLKclock],eax
        adc     [CLKclock+4],edx
        inc     dword [CLKcount]
        pop     edx
        pop     eax
        ret

; void clkput(void)
; {
;   RO.printf("call %dtimes:ave %fclk\n",CLKcount,CLKclock/CLKcount);
; }
proc clkput
		cmp		dword [CLKcount], 0
		jnz		.L0
		ret
.L0:
        fild    qword [CLKclock]
        mov     eax,[CLKcount]
        fild    dword [CLKcount]
        fdivp   st1
        sub     esp,16
        mov     dword [esp],fmt_stat
        mov     [esp+4],eax
        fstp    qword [esp+8]
%if 0
	mov	eax,[RO.printf]
        call    eax
%else
	call	errPrintf
%endif
        add     esp,16
        xor     eax,eax
        mov     [CLKclock],eax
        mov     [CLKclock+4],eax
        mov     [CLKcount],eax
        ret

        end
