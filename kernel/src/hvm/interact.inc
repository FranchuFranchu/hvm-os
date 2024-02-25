

; IN = RCX: Pointer to worker, RDX: Provenance, R8: First agent, R9: Second agent
hvm_interact:
  	print_msg_volatile "Doing interaction", .msg0
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

	cmp rax, hvm_tag_CTR or (hvm_tag_CTR shl 2)
	je .ctr_ctr
	; Undefined interaction
  	print_msg_volatile "Invalid interaction", .ii_msg
  	jmp $

.ctr_ctr:
  	print_msg_volatile "Ctr-Ctr", .msg3
	; Compare labels
	mov r10, r8
	shr r10, 48 ; Get label
	mov r11, r9
	shr r11, 48 ; Get label
	cmp r10, r11
	jne .commute
.annihilate:
  	print_msg_volatile "Annihilate", .msg4
	hvm_port_get_target r8
	hvm_port_get_target r9
	push r8
	push r9
	push r8
	push r9


	hvm_get_port_1 r8
	hvm_get_port_1 r9
	call hvm_link_wire_wire

	pop r9
	pop r8

	hvm_get_port_2 r8
	hvm_get_port_2 r9
	call hvm_link_wire_wire

	pop r9
	pop r8

	ret
.commute:
	sub rsp, 64
	; [rsp + 32] = a : Port
	; [rsp + 40] = b : Port
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

	; [rsp+0] = a1 : Port
	call hvm_alloc
	or rax, r9
	mov [rsp+0], rax
	; [rsp+8] = a2 : Port
	call hvm_alloc
	or rax, r9
	mov [rsp+8], rax
	; [rsp+16] = a1 : Port
	call hvm_alloc
	or rax, r8
	mov [rsp+16], rax
	; [rsp+24] = a1 : Port
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
	mov r9, [rsp+0]
	mov r8, [rsp+40]
	hvm_get_port_1 r8
	; r8 = Wire(b.1)
	; r9 = Port(a1)
	call hvm_link_wire_port

	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r9, [rsp+8]
	mov r8, [rsp+40]
	hvm_get_port_2 r8
	; r8 = Wire(b.2)
	; r9 = Port(a2)
	call hvm_link_wire_port

	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r9, [rsp+16]
	mov r8, [rsp+32]
	hvm_get_port_1 r8
	; r8 = Wire(a.1)
	; r9 = Port(b1)
	call hvm_link_wire_port

	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r9, [rsp+24]
	mov r8, [rsp+32]
	hvm_get_port_2 r8
	; r8 = Wire(a.2)
	; r9 = Port(b2)
	call hvm_link_wire_port

	; Link 4 aux ports

	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r8, [rsp+0]
	hvm_get_port_1 r8
	or r8, hvm_tag_AUX or 8
	mov r9, [rsp+16]
	hvm_get_port_1 r9
	or r9, hvm_tag_AUX or 8
	call hvm_link_port_port

	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r8, [rsp+0]
	hvm_get_port_1 r8
	or r8, hvm_tag_AUX
	mov r9, [rsp+24]
	hvm_get_port_1 r9
	or r9, hvm_tag_AUX or 8
	call hvm_link_port_port

	
	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r8, [rsp+8]
	hvm_get_port_1 r8
	or r8, hvm_tag_AUX or 8
	mov r9, [rsp+16]
	hvm_get_port_1 r9
	or r9, hvm_tag_AUX
	call hvm_link_port_port

	
	mov rcx, [rsp+56]
	mov rdx, [rsp+48]
	mov r8, [rsp+8]
	hvm_get_port_1 r8
	or r8, hvm_tag_AUX
	mov r9, [rsp+24]
	hvm_get_port_1 r9
	or r9, hvm_tag_AUX
	call hvm_link_port_port
	add rsp, 64

	ret
