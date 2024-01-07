
section '.text' executable readable
hvm_port_LABEL_MASK equ 0xFFFF shl 48
hvm_port_TAG_MASK equ 3
hvm_port_PRINCIPAL_MASK equ 2
hvm_port_TARGET_MASK equ 0xFFFFFFFFFFFC
hvm_tag_RED equ 0
hvm_tag_AUX equ 1
hvm_tag_CTR equ 2
hvm_tag_DAT equ 3
hvm_special_value_FREE equ 0
hvm_special_value_LOCK equ 1
hvm_special_value_GONE equ 2
hvm_special_value_ERAS equ 3
 
macro hvm_port_get_target reg {
  shl reg, 16 ; Shift out higher bits
  shr reg, 18 ; Shift out lower bits
  shl reg, 2 ; Restore original placement
}
macro hvm_port_get_port_1 reg {
  shl reg, 16 ; Shift out higher bits
  shr reg, 20 ; Shift out lower bits
  shl reg, 4 ; Restore original placement
}

macro push_volatile 
{
	push rax
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
	push r11
}
macro pop_volatile 
{
	pop r11
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdx
	pop rcx
	pop rax
}
macro hvm_get_port port_addr,out_val {
  mov out_val, [port_addr]
  add out_val, port_addr
}
macro hvm_set_port port_addr,in_val {
  sub in_val, port_addr
  mov [port_addr], in_val
  add in_val, port_addr
}
macro hvm_set_port_1 port_addr,in_val {
  hvm_set_port port_addr,in_val
}
macro hvm_get_port_1 port_addr,in_val {
  hvm_get_port port_addr,in_val
}
macro hvm_get_port_2 port_addr,out_val {
  mov out_val, [port_addr+8]
  add out_val, port_addr
  add out_val, 8
}
macro hvm_set_port_2 port_addr,in_val {
  sub in_val, port_addr
  sub in_val, 8
  mov [port_addr+8], in_val
  add in_val, port_addr
  add in_val, 8
}

include 'hvm/worker.inc'
include 'hvm/alloc.inc'
include 'hvm/interact.inc'
include 'hvm/redex_ring.inc'
include 'hvm/log.inc'


k.shutdown: 
  uefi_call_wrapper RuntimeServices, ResetSystem, 2, EFI_SUCCESS, 0, 0

section '.data' readable writable

macro dport dest, tag, lab {
  label @@ qword at $
  dq (hvm_tag_#tag) + (dest-@b) + (lab shl 48)
}
sample_data:
.0.1: 
  dport .1.2, AUX, 0
.0.2: 
  dq 0
; (λx x λy y) 
.1.1: 
  dport .2.1, CTR, 0
.1.2: 
  dport .0.1, AUX, 0
; λy y
.2.1:
  dport .2.2, AUX, 0
.2.2:
  dport .2.1, AUX, 0
; λx x
.3.1:
  dport .3.2, AUX, 0
.3.2:
  dport .3.1, AUX, 0
; Redex list
.4.1:
  dport .1.1, CTR, 0
.4.2:
  dq .5.1-$
.5.1:
  dport .3.1, CTR, 1
.5.2:
  dq .6.1-$
.6.1:
  dq 0
.6.2:
  dq .4.1-$
sample_data_end: