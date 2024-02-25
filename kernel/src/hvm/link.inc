; IN: RCX = Pointer to worker, RDX: Provenance, R8: a_wire, R9: b_wire
hvm_link_wire_wire:
	hvm_wire_lock_target r8, r11
	hvm_wire_lock_target r9, r10

	mov rax, r10
	and rax, r11
	and rax, hvm_port_PRINCIPAL_MASK
	cmp rax, 0
	je .not_principal

	push rcx
	push rdx
	push r10
	push r11
	push r8
	push r9
	mov rdx, r9
	call hvm_half_free
	pop rdx
	pop rdx
	call hvm_half_free
	pop r9
	pop r8
	pop rdx
	pop rcx
	jmp hvm_redex_push
.not_principal:
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
	xchg r8, r9
	mov r9, r8
	mov r8, r10
	mov r10, r11
	call hvm_link_half_wire_port
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdx
	pop rcx
	xchg r8, r9
	mov r8, r11
	jmp hvm_link_half_wire_port

; IN: RCX = Pointer to worker, RDX: Provenance, R8: a_wire, R9: b_port
hvm_link_wire_port:
	; r10 = a_port
	hvm_wire_lock_target r8, r10
	mov rax, r8
	and rax, r10
	and rax, hvm_port_PRINCIPAL_MASK
	cmp rax, 0
	je .not_principal

	push rcx
	push rdx
	push r9
	push r10
	mov rdx, r8
	call hvm_half_free
	pop r8
	pop r9
	pop rdx
	pop rcx
	jmp hvm_redex_push

.not_principal:
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
	call hvm_link_half_wire_port

	pop r11
	pop r10
	pop r9
	pop r8
	pop rdx
	pop rcx
	mov r8, r10
	call hvm_link_half_port_port

; IN: RCX = Pointer to worker, RDX: Provenance, R8: a_port, R9: b_port
hvm_link_port_port:
	mov rax, r8
	and rax, r9
	and rax, hvm_port_PRINCIPAL_MASK
	cmp rax, 0
	je .not_principal
	jmp hvm_redex_push
.not_principal:
	push rcx
	push rdx
	push r8
	push r9
	call hvm_link_half_port_port
	pop r8 ; Implicitly swaps r8 and r9
	pop r9
	pop rdx
	pop rcx
	jmp hvm_link_half_port_port

; IN: RCX = Pointer to worker, RDX: Provenance, R8: a_port, R9: a_wire, R10: b_port
hvm_link_half_wire_port:
	mov rax, r8
	and rax, hvm_port_TAG_MASK
	cmp rax, hvm_tag_AUX
	jne .principal
.auxiliary:
	mov rax, r9
	and rax, hvm_tag_AUX
	sub r8, hvm_tag_AUX
	sub rax, r8
	sub r10, r8
	lock cmpxchg [r8], r10
	jnz .conflict
.success:
	mov rdx, r9
	add rdx, r8
	call hvm_half_free
	ret
.conflict:
	add r10, r8
	add r8, hvm_tag_AUX
	mov rax, r10
	and rax, hvm_port_TAG_MASK
	cmp rax, hvm_tag_AUX
	jne .conflict_principal
.conflict_aux:
	sub r10, hvm_tag_AUX
	add r10, hvm_tag_RED
	hvm_wire_store_port r9, r10
	ret
.conflict_principal:
	hvm_wire_store_port r9, r10
	jmp hvm_link_resolve_redirect
.principal: 
	mov rdx, r9
	jmp hvm_half_free

; IN: RCX = Pointer to worker, RDX: Provenance, R8: a_port, R9: b_port
hvm_link_half_port_port:
	mov rax, r8
	and rax, hvm_port_TAG_MASK
	cmp rax, hvm_tag_AUX
	jne @f
	hvm_port_get_target r8
	hvm_wire_store_port r8, r9
@@:
	
	ret



; IN: RCX = Pointer to worker, RDX: Provenance, R8: a_port, R9: a_wire, R10: b_port
hvm_link_resolve_redirect:
	print_msg_volatile "resolve redirect", .aa
	; r8 = t_wire
	hvm_port_get_target r8
	; r11 = t_port
	hvm_wire_load_port r8, r11
	cmp r11, hvm_special_value_LOCK
	jne @f
	jmp hvm_link_resolve_redirect
@@:
	mov rax, r11
	and rax, hvm_port_TAG_MASK
	cmp rax, hvm_tag_RED
	jne @f
	; clear the redirection

	push rdx
	push r8
	push r9
	push r10
	mov rdx, r8
	call hvm_half_free
	pop r10
	pop r9
	pop r8
	pop rdx
	jmp hvm_link_resolve_redirect
@@:
	mov rax, r11
	and rax, hvm_port_TAG_MASK
	cmp rax, hvm_tag_AUX
	jne .principal

; It's a var. Try replacing it.
	mov rax, r11
	hvm_wire_offset r8, rax
	hvm_wire_offset r8, r10
	lock cmpxchg [r8], r10
	jz @f

	; Failure
	hvm_wire_deoffset r8, r10
	jmp hvm_link_resolve_redirect
@@:
	hvm_wire_deoffset r8, r10

	xchg r8, r11
	hvm_port_get_target r8
	hvm_wire_load_port r8, r11
	
	mov rdx, r9
	push_volatile
	call hvm_half_free
	pop_volatile
	; Collect backward path
.loopy:

	mov rax, r11
	and rax, hvm_port_TAG_MASK
	cmp rax, hvm_tag_RED
	jne @f
	mov rax, r11
	cmp rax, hvm_special_value_LOCK
	je @f


	push rdx
	push r8
	push r9
	push r10
	mov rdx, r8
	call hvm_half_free
	pop r10
	pop r9
	pop r8
	pop rdx

	xchg r8, r11
	hvm_port_get_target r8
	hvm_wire_load_port r8, r11
	jmp .loopy
@@:
	ret
.principal:
	; Sort references to avoid deadlocks
	cmp r8, r9
	jg @f
	xchg r8, r9
@@:
	mov rax, hvm_special_value_GONE
	hvm_wire_swap_port_lock r8, rax
	cmp rax, hvm_special_value_GONE
	je .second_thread
.first_thread:
	; Create redex
	mov r8, hvm_special_value_GONE
	hvm_wire_swap_port_lock r9, r8
	mov r9, rax
	jmp hvm_redex_push
	
.second_thread:
	; Clear up memory
	jmp $ ; TODO
	

