;
;   part of this code is origined from
;
;       GOGO-no-coda
;	Copyright (c) 2003 sakai
;       Copyright (c) 2003 gogo-developer
;

%include "../../engine/i386/nasm.cfg"

		segment_text

; int atomic_lock(int *mutex)
proc atomic_lock
	mov	edx,[esp+4]
	mov	eax,1
	xchg	[edx],eax	; lock prefix is not required for xchg
        ret

; void atomic_post(int *semaphore)
proc atomic_post
	mov	edx,[esp+4]
	lock inc	dword [edx]
	ret

; int atomic_tryget(int *semaphore)
proc atomic_tryget
	mov	edx,[esp+4]
	mov	eax,1
	lock dec	dword [edx]
	jns	.f1
	xor	eax,eax	; failed to get semaphore
	lock inc	dword [edx]
.f1:	ret

        end

