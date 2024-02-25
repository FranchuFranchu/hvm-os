section '.text' executable readable

; IN = RCX: Worker, RDX: Amount of entries to log.
hvm_log_worker:
	push rsi
	push rdi
	push rbx
	push r12

	
	mov rdx, [rcx+hvm_worker.area_start]
	mov rdi, [rcx+hvm_worker.alloc_next]

.loopy:

	push_volatile
	call hvm_log_wire
	pop_volatile

	add rdx, 8

	cmp rdi, rdx
	jg .loopy

	pop r12
	pop rbx
	pop rdi
	pop rsi
	ret

; IN = RCX: Worker, RDX: Wire to log
hvm_log_wire:
	push rcx
	push rdx
	mov rax, rdx
	sub rax, [rcx+hvm_worker.area_start]
	shr rdx, 3
	; Print the current position.
	shr ax, 4
	and rdx, 1
	mov dl, [rdx+log_strings.port_texts]
	call os_string_convert_2hex
	sub rsp, 32
	mov byte [rsp+0], al
	mov byte [rsp+1], 0
	mov byte [rsp+2], ah
	mov byte [rsp+3], 0
	mov byte [rsp+4], '.'
	mov byte [rsp+5], 0
	mov byte [rsp+6], dl
	mov byte [rsp+7], 0
	mov byte [rsp+8], ':'
	mov byte [rsp+9], 0
	mov byte [rsp+10], ' '
	mov byte [rsp+11], 0
	mov byte [rsp+12], 0
	mov byte [rsp+13], 0
	mov byte [rsp+14], 0
	mov byte [rsp+15], 0
	mov rdx, rsp

	uefi_call_wrapper ConOut, OutputString, ConOut, rdx
	add rsp, 32

	pop rdx
	hvm_wire_load_port rdx, rax
	mov rdx, rax
	pop rcx
	jmp hvm_log_port

; IN = RCX: Worker, RDX: Port to log
hvm_log_port:
	mov r9, rdx
	mov r10, hvm_port_TAG_MASK
	not r10
	mov rax, r9
	and rax, r10
	cmp rax, 0
	je .special_value
.regular_value:
	; Get and print the tag
	mov rax, r9
	and rax, hvm_port_TAG_MASK
	mov r10, rax
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
	sub rax, [rcx+hvm_worker.area_start]
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
	jmp .end
.special_value:
	mov rax, r9
	and rax, hvm_port_TAG_MASK
	mov rdx, [log_strings.special_value_texts+rax*8]
	push_volatile
	uefi_call_wrapper ConOut, OutputString, ConOut, rdx
	pop_volatile

.end:
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
dq .eras_text
dq .lock_text
dq .gone_text
.free_text: du "FREE", 0xd, 0xa, 0
.eras_text: du "ERAS", 0xd, 0xa, 0
.lock_text: du "LOCK", 0xd, 0xa, 0
.gone_text: du "GONE", 0xd, 0xa, 0
.port_texts:
db '1'
db '2'
align 16
section '.text' executable readable
