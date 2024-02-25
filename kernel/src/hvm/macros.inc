
hvm_port_LABEL_MASK equ (0xFFFF shl 48)
hvm_port_TAG_MASK equ 3
hvm_port_PRINCIPAL_MASK equ 1
hvm_port_TARGET_MASK equ 0xFFFFFFFFFFFC
hvm_port_INTERACT equ 1
hvm_port_VANILLA equ 2
hvm_port_LAZY equ (1 shl 2)
hvm_port_PORT_NUMBER equ (1 shl 3)
hvm_tag_RED equ 0
hvm_tag_EXT equ hvm_port_INTERACT
hvm_tag_AUX equ hvm_port_VANILLA
hvm_tag_CTR equ (hvm_port_VANILLA or  hvm_port_INTERACT)
hvm_special_value_FREE equ hvm_tag_RED
hvm_special_value_ERAS equ hvm_tag_EXT
hvm_special_value_LOCK equ hvm_tag_AUX
hvm_special_value_GONE equ hvm_tag_CTR
 
macro hvm_port_get_target reg {
  shl reg, 16 ; Shift out higher bits
  shr reg, 19 ; Shift out lower bits
  shl reg, 3 ; Restore original placement
}
macro hvm_get_port_1 reg {
  shl reg, 16 ; Shift out higher bits
  shr reg, 20 ; Shift out lower bits
  shl reg, 4 ; Restore original placement
}
macro hvm_get_port_2 reg {
  shl reg, 16 ; Shift out higher bits
  shr reg, 20 ; Shift out lower bits
  shl reg, 4 ; Restore original placement
  add reg, 8
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

macro hvm_wire_offset wire,out_val {
  sub out_val, wire
}

macro hvm_wire_deoffset wire,out_val {
  add out_val, wire
}

macro hvm_wire_load_port port_addr,out_val {
  mov out_val, [port_addr]
  add out_val, port_addr
}
macro hvm_wire_store_port port_addr,in_val {
  sub in_val, port_addr
  mov [port_addr], in_val
  add in_val, port_addr
}
macro hvm_wire_swap_port port_addr,swap_val {
  sub swap_val, port_addr
  xchg [port_addr], swap_val
  add swap_val, port_addr
}
macro hvm_wire_swap_port_lock port_addr,swap_val {
  sub swap_val, port_addr
  lock xchg [port_addr], swap_val
  add swap_val, port_addr
}
macro hvm_wire_store_port_1 port_addr,in_val {
  hvm_wire_store_port port_addr,in_val
}
macro hvm_wire_load_port_1 port_addr,in_val {
  hvm_wire_load_port port_addr,in_val
}
macro hvm_wire_load_port_2 port_addr,out_val {
  mov out_val, [port_addr+8]
  add out_val, port_addr
  add out_val, 8
}
macro hvm_wire_store_port_2 port_addr,in_val {
  sub in_val, port_addr
  sub in_val, 8
  mov [port_addr+8], in_val
  add in_val, port_addr
  add in_val, 8
}
macro hvm_wire_lock_target port_addr,out_val {
  @@:
  mov out_val, hvm_special_value_LOCK
  hvm_wire_swap_port_lock port_addr,out_val
  cmp out_val, hvm_special_value_LOCK
  je @b
}
macro hvm_rax_log_port {
  push_volatile
  mov rdx, rax
  call hvm_log_port
  pop_volatile
}
macro hvm_rax_log_wire {
  push_volatile
  mov rdx, rax
  call hvm_log_wire
  pop_volatile
}