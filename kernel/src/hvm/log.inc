section '.text' executable readable

; IN = RCX: Worker, RDX: Amount of entries to log.
hvm_log_worker:
	push rsi
	push rdi
	push rbx
	push r12

	
	push rcx
	push rdx

	sub rsp, 32
	mov rdx, rsp
	uefi_call_wrapper BootServices, AllocatePages, 0, 2, 8, rdx
	mov rax, [rsp]
	add rsp, 32

	pop rdx
	pop rcx


	push rcx


	; first, copy the entries
	mov rsi, [rcx+hvm_worker.area_start]
	mov rdi, rax
	mov rcx, rdx
	shl rcx, 1

	rep movsq

	pop rcx

	mov rsi, [rcx+hvm_worker.area_start]
	xor rcx, rcx

	; now, iterate over them and print them.
.loopy:
	mov r8, rcx
	shl r8, 3
	add r8, rsi


	push rcx
	sub rsp, 32
	mov rax, rcx
	and rax, 0xFF
	; Print the current position.
	shr al, 1
	and cl, 1
	mov cl, [rcx+log_strings.port_texts]
	call os_string_convert_2hex
	mov byte [rsp+0], al
	mov byte [rsp+1], 0
	mov byte [rsp+2], ah
	mov byte [rsp+3], 0
	mov byte [rsp+4], '.'
	mov byte [rsp+5], 0
	mov byte [rsp+6], cl
	mov byte [rsp+7], 0
	mov byte [rsp+8], ':'
	mov byte [rsp+9], 0
	mov byte [rsp+10], ' '
	mov byte [rsp+11], 0
	mov byte [rsp+12], 0
	mov byte [rsp+13], 0
	mov byte [rsp+14], 0
	mov byte [rsp+15], 0
	push rdx
	mov rdx, rsp
	add rdx, 8

	push_volatile
	uefi_call_wrapper ConOut, OutputString, ConOut, rdx
	pop_volatile

	; Restore RCX and RDX
	pop rdx
	pop rcx
	push rcx
	push rdx

	hvm_get_port r8, r9

	; Get and print the tag
	mov rax, r9
	and rax, hvm_port_TAG_MASK
	mov rdx, [log_strings.tag_texts+rax*8]

	push_volatile
	uefi_call_wrapper ConOut, OutputString, ConOut, rdx
	pop_volatile

	; Get and print the label
	mov rax, r9
	shr rax, 48
	call os_string_convert_2hex

	sub rsp, 32
	mov byte [rsp+0], '-'
	mov byte [rsp+1], 0
	mov byte [rsp+2], al
	mov byte [rsp+3], 0
	mov byte [rsp+4], ah
	mov byte [rsp+5], 0
	mov byte [rsp+6], 0
	mov byte [rsp+7], 0

	mov rdx, rsp

	push_volatile
	uefi_call_wrapper ConOut, OutputString, ConOut, rdx
	pop_volatile

	add rsp, 32

	; Get and print the target
	mov rax, r9
	hvm_port_get_target rax
	sub rax, rsi
	shr rax, 3
	and rax, 0xFF
	mov rcx, rax
	shr al, 1
	and cl, 1
	mov cl, [rcx+log_strings.port_texts]
	call os_string_convert_2hex

	sub rsp, 32
	mov byte [rsp+0], 32
	mov byte [rsp+1], 0
	mov byte [rsp+2], al
	mov byte [rsp+3], 0
	mov byte [rsp+4], ah
	mov byte [rsp+5], 0
	mov byte [rsp+6], '.'
	mov byte [rsp+7], 0
	mov byte [rsp+8], cl
	mov byte [rsp+9], 0
	mov byte [rsp+10], 0xd
	mov byte [rsp+11], 0
	mov byte [rsp+12], 0xa
	mov byte [rsp+13], 0
	mov byte [rsp+14], 0

	mov rdx, rsp

	push_volatile
	uefi_call_wrapper ConOut, OutputString, ConOut, rdx
	pop_volatile

	add rsp, 32
	pop rdx
	add rsp, 32
	pop rcx

	inc rcx
	mov rax, rcx
	sub rax, rdx
	jl .loopy


	pop r12
	pop rbx
	pop rdi
	pop rsi
	ret

section '.data' readable writeable
log_strings:
.tag_texts:
dq .red_text
dq .aux_text
dq .ctr_text
dq .dat_text
.red_text: du "RED", 0
.aux_text: du "EXT", 0
.ctr_text: du "AUX", 0
.dat_text: du "CTR", 0
.special_value_texts:
dq .free_text
dq .lock_text
dq .gone_text
dq .eras_text
.free_text: du "FREE", 0
.eras_text: du "ERAS", 0
.gone_text: du "GONE", 0
.lock_text: du "LOCK", 0
.port_texts:
db '1'
db '2'
align 16
section '.text' executable readable