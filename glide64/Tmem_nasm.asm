
%if BITS=64
DEFAULT REL
%else
%define rax eax
%define rbx ebx
%define rcx ecx
%define rsi esi
%define rdi edi
%define rdx edx
%endif

global CopyBlock
global _SwapBlock32
global _SwapBlock64

[section .text]

;****************************************************************
; CopyBlock - copies a block from base_addr+offset to dest_addr, while unswapping the
;  data within.
;
; edi = dest_addr -> end of dest
; ecx = num_words
; esi = base_addr (preserved)
; edx = offset (preserved)
;****************************************************************

ALIGN 4

CopyBlock:
	push rax
	push rbx
	push rsi
	push rdx

	or ecx,ecx
	jz near copyblock_end

	push rcx

	; first, set the source address and check if not on a dword boundary
	push rsi
	push rdx
	mov ebx,edx
	and edx,0FFFFFFFCh
	add esi,edx

	and ebx,3				; ebx = # we DON'T need to copy
	jz copyblock_copy

	mov edx,4				; ecx = # we DO need to copy
	sub edx,ebx

	; load the first word, accounting for swapping

	mov eax, [rsi]
	add rsi,4
copyblock_precopy_skip:
	rol eax,8
	dec ebx
	jnz copyblock_precopy_skip

copyblock_precopy_copy:
	rol eax,8
	mov [rdi],al
	inc rdi
	dec edx
	jnz copyblock_precopy_copy

	mov eax,[rsi]
	add rsi,4
	bswap eax
	mov [rdi],eax
	add rdi,4

	dec ecx		; 1 less word to copy
	jz copyblock_postcopy

copyblock_copy:
	mov eax,[rsi]
	bswap eax
	mov [rdi],eax

	mov eax,[rsi+4]
	bswap eax
	mov [rdi+4],eax

	add rsi,8
	add rdi,8

	dec ecx
	jnz copyblock_copy

copyblock_postcopy:
	pop rdx
	pop rsi
	pop rcx

	; check again if on dword boundary
	mov ebx,edx						; ebx = # we DO need to copy

	and ebx,3
	jz copyblock_end

	shl ecx,3						; ecx = num_words * 8
	add edx,ecx
	and edx,0FFFFFFFCh
	add rsi,rdx

	mov eax,[rsi]

copyblock_postcopy_copy:
	rol eax,8
	mov [rdi],al
	inc rdi
	dec ebx
	jnz copyblock_postcopy_copy

copyblock_end:
	pop rdx
	pop rsi
	pop rbx
	pop rax
	ret


;****************************************************************
; SwapBlock - swaps every other 32-bit word at addr
;
; ecx = num_words -> 0
; edi = addr -> end of dest
;****************************************************************

ALIGN 4

_SwapBlock32:
	push rax
	push rbx
	or ecx,ecx
	jz swapblock_end
swapblock_loop:
	mov eax,[rdi]
	mov ebx,[rdi+4]
	mov [rdi+4],eax
	mov [rdi],ebx
	add rdi,8
	dec ecx
	jnz swapblock_loop
swapblock_end:
	pop rbx
	pop rax
	ret


ALIGN 4

_SwapBlock64:
	push rax
	push rbx
	push rdx
	shr ecx,1
	or ecx,ecx
	jz swapblock_end2
swapblock_loop2:
	mov eax,[rdi]
	mov edx,[rdi+8]
	mov ebx,[rdi+4]
	mov [rdi+8],eax
	mov eax,[rdi+12]
	mov [rdi],edx
	mov [rdi+12],ebx
	mov [rdi+4],eax
	add rdi,16
	dec ecx
	jnz swapblock_loop2
swapblock_end2:
	pop rdx
	pop rbx
	pop rax
	ret


END
