

; IN = RCX: Pointer to worker, RDX: Provenance, R8: First agent, R9: Second agent
hvm_interact:
	sub rsp, 8
	mov rax, rdx
	call print_rax
	mov rax, r8
	call print_rax
	mov rax, r9
	call print_rax
	add rsp, 8

	mov r10, r8
	and r10, hvm_port_TAG_MASK
	xor rax, rax
	or rax, r10
	
	shl rax, 2
	mov r10, r9
	and r10, hvm_port_TAG_MASK
	or rax, r10

	cmp rax, hvm_tag_CTR or hvm_tag_CTR shl 2
	je .ctr_ctr
	; Undefined interaction
	print_msg 'Invalid interaction',.ii_msg
	jmp $

.ctr_ctr:
	; Compare labels
	mov r10, r8
	shr r10, 48 ; Get label
	mov r11, r9
	shr r11, 48 ; Get label
	cmp r10, r11
	jne .commute
.annihilate:
	hvm_port_get_target r8
	hvm_port_get_target r9

	push r8
	push r9
	push r8
	push r9

	hvm_get_port_1 r8, rax
	mov r8, rax
	hvm_get_port_1 r9, rax
	mov r9, rax
	call hvm_link_port_port

	pop r9
	pop r8

	hvm_get_port_2 r8, rax
	mov r8, rax
	hvm_get_port_2 r9, rax
	mov r9, rax
	call hvm_link_port_port

	pop r9
	pop r8

	mov rdx, r8
	call hvm_free
	mov rdx, r9
	call hvm_free

	ret
.commute:
	sub rsp, 64
	mov [rsp+56], rcx
	mov [rsp+48], rdx
	mov [rsp+32], r8
	mov [rsp+40], r9

	; Replace r8 and r9 by their label and tag as bitfields.
	ror r8, 2
	shr r8, 46
	ror r8, 16
	ror r9, 2
	shr r9, 46
	ror r9, 16

	; a1
	call hvm_alloc
	or rax, r9
	mov [rsp+0], rax
	; a2
	call hvm_alloc
	or rax, r9
	mov [rsp+8], rax
	; b1
	call hvm_alloc
	or rax, r8
	mov [rsp+16], rax
	; b2
	call hvm_alloc
	or rax, r8
	mov [rsp+24], rax

	mov r9, [rsp+32]
	mov r8, [rsp+40]
	hvm_port_get_target r8
	hvm_port_get_target r9
	mov [rsp+32], r8
	mov [rsp+40], r9

	; Link 4 main ports
	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r8, [rsp+0]
	mov r10, [rsp+40]
	hvm_port_get_target r10
	hvm_get_port_1 r10, r9
	call hvm_link_port_port

	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r8, [rsp+8]
	mov r10, [rsp+40]
	hvm_port_get_target r10
	hvm_get_port_2 r10, r9
	call hvm_link_port_port

	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r9, [rsp+16]
	mov r10, [rsp+32]
	hvm_port_get_target r10
	hvm_get_port_1 r10, r8
	call hvm_link_port_port

	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r9, [rsp+24]
	mov r10, [rsp+32]
	hvm_port_get_target r10
	hvm_get_port_2 r10, r8
	call hvm_link_port_port

	; Link 4 aux ports

	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r8, [rsp+0]
	hvm_port_get_port_1 r8
	or r8, 1 or 8
	mov r9, [rsp+16]
	hvm_port_get_port_1 r9
	or r9, 1 or 8
	call hvm_link_port_port

	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r8, [rsp+0]
	hvm_port_get_port_1 r8
	or r8, 1
	mov r9, [rsp+24]
	hvm_port_get_port_1 r9
	or r9, 1 or 8
	call hvm_link_port_port

	
	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r8, [rsp+8]
	hvm_port_get_port_1 r8
	or r8, 1 or 8
	mov r9, [rsp+16]
	hvm_port_get_port_1 r9
	or r9, 1
	call hvm_link_port_port

	
	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r8, [rsp+8]
	hvm_port_get_port_1 r8
	or r8, 1
	mov r9, [rsp+24]
	hvm_port_get_port_1 r9
	or r9, 1
	call hvm_link_port_port
	add rsp, 64

	ret

; IN = RCX: Pointer to worker, RDX: Provenance, R8: First port, R9: Second port.
hvm_link_port_port:
	; now, check if they're both principal ports
	mov r10, hvm_port_PRINCIPAL_MASK
	and r10, r8
	and r10, r9
	jz .aux
.principal:
	push rcx
	push rdx
	push r8
	push r9
	call hvm_redex_push
	pop r9
	pop r8
	pop rdx
	pop rcx
	ret
.aux:
	mov r10, r8
	and r10, 2
	jnz @f
	mov r10, r8
	hvm_port_get_target r10
	hvm_set_port r10, r9
@@:
	mov r10, r9
	and r10, 2
	jnz @f

	mov rax, r9
	call print_rax
	mov r10, r9
	hvm_port_get_target r10
	hvm_set_port r10, r8
@@:
	ret

