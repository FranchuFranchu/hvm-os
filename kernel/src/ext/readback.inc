; IN: RCX = Port to readback from.
ext_readback:
	mov rax, rcx
	and rax, hvm_port_INTERACT
	jz .non_interacting
.interacting:

.non_interacting:

	ret