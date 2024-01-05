
section '.text' executable readable
hvm_port_LABEL_MASK equ 0xFFFF shl 48
hvm_port_TAG_MASK equ 3
hvm_port_TARGET_MASK equ (not 0) and not hvm_port_LABEL_MASK and not hvm_TAG_MASK
hvm_tag_RED equ 0
hvm_tag_AUX equ 1
hvm_tag_CTR equ 2
hvm_tag_DAT equ 3

struc hvm_worker {
  ; The beginning of the memory area assigned
  ; to this worker
  .area_start dptr
  .area_size dptr
  .redex dptr
  .alloc_next dptr
  .alloc_head dptr
  .worker_id dptr
  .workers dptr
  .size = $ - .area_start
}
struct hvm_worker

struc hvm_ring {
  .item dptr
  .next dptr
}
struct hvm_ring

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

; Create a worker
; Out = RAX: Worker
hvm_init:
  sub rsp, 32
  uefi_call_wrapper BootServices, AllocatePages, 0, 2, 8, rsp
  call print_rax
  mov rax, [rsp]
  mov rdx, rax
  add rax, hvm_worker.size
  call print_rax
  mov rsi, sample_data
  mov rdi, rax
  mov rcx, sample_data_end-sample_data
  rep movsq
  add rsp, 32
  ; Initialize the Worker struct
  ; It is in rdx
  mov [rdx+hvm_worker.alloc_next], rax
  mov [rdx+hvm_worker.area_start], rax
  mov rcx, 8*4096
  sub rcx, hvm_worker.size
  mov [rdx+hvm_worker.area_size], rcx
  mov rcx, rax
  add rcx, sample_data.7.1-sample_data
  mov [rdx+hvm_worker.redex], rcx
  add [rdx+hvm_worker.alloc_next], sample_data_end-sample_data
  mov [rdx+hvm_worker.alloc_head], 0
  mov [rdx+hvm_worker.worker_id], 0
  mov [rdx+hvm_worker.workers], 1
  mov rax, rdx

  ret

; IN = RCX: Pointer to worker,
; OUT = RCX: Pointer to worker, RDX: Provenance, R8: First port (Absolute), R9: Second port (Absolute), CF: Set if ring empty
hvm_redex_pop:
  push rbx
  push rcx
  cmp qword [rcx+hvm_worker.redex], 0
  je .no_redex
  mov rbx, [rcx+hvm_worker.redex]
  mov rdx, [rbx+hvm_ring.next]
  add rdx, rbx
  add rdx, hvm_ring.next

  ; Agent 1
  mov rbx, [rdx+hvm_ring.item]
  add rbx, rdx
  add rbx, hvm_ring.item
  push rbx

  mov rbx, [rdx+hvm_ring.next]
  add rbx, rdx
  add rbx, hvm_ring.next

  call hvm_free

  ; Agent 2
  mov rdx, rbx

  mov rbx, [rdx+hvm_ring.item]
  add rbx, rdx
  add rbx, hvm_ring.item
  push rbx

  mov rbx, [rdx+hvm_ring.next]
  add rbx, rdx
  add rbx, hvm_ring.next

  call hvm_free

  ; Provenance
  mov rdx, rbx

  mov rbx, [rdx+hvm_ring.item]
  push rbx

  mov rbx, [rdx+hvm_ring.next]
  add rbx, rdx
  add rbx, hvm_ring.next

  call hvm_free
  pop rdx
  pop r9
  pop r8
  pop rcx
  pop rbx
  ret
.no_redex:
  stc
  ret

; Push a redex to the redex ring
; IN = RCX: Pointer to worker, RDX: Provenance, R8: First port (Absolute), R9: Second port (Absolute).
hvm_redex_push:
  ; Push a set of redexes.
  push rbx
  push r12
  push r13
  push r14 
  push r15
  push r15
  push r15
  push r15



  mov r10, rdx
  mov r11, rcx


  ; Store the provenance
  mov rcx, r11
  call hvm_alloc
  mov [rax+hvm_ring.item], r10
  mov r13, rax
  mov r10, rax

  ; Store second port
  mov rcx, r11
  call hvm_alloc
  sub r9, rax
  sub r9, hvm_ring.item
  mov [rax+hvm_ring.item], r9
  sub r13, rax
  sub r13, hvm_ring.next
  mov [rax+hvm_ring.next], r13
  mov r13, rax

  ; Store first port
  mov rcx, r11
  call hvm_alloc
  sub r8, rax
  sub r8, hvm_ring.item
  mov [rax+hvm_ring.item], r8
  sub r13, rax
  sub r13, hvm_ring.next
  mov [rax+hvm_ring.next], r13

  mov rcx, r11
  ; Pointer to current.
  mov r12, [rcx+hvm_worker.redex]
  cmp r12, 0
  jnz .has_redex
.no_redex:
  mov [rcx+hvm_worker.redex], r10

  sub rax, r10
  sub rax, hvm_ring.next
  mov [r10+hvm_ring.next], rax

  jmp .e

.has_redex:
  mov [rcx+hvm_worker.redex], r10

  sub rax, r12
  sub rax, hvm_ring.next
  mov [r12+hvm_ring.next], rax

  mov rax, [r12+hvm_ring.next]
  add rax, r12
  sub rax, r10
  mov [r10+hvm_ring.next], rax

  jmp .e

.e:
  pop r15
  pop r15
  pop r15
  pop r15
  pop r14
  pop r13
  pop r12
  pop rbx
  ret

; Allocate 16 bytes
; IN = RCX: Pointer to worker
; OUT = RAX: Allocated pointers
; Only changes RAX, RCX, RDX
hvm_alloc:
  mov rax, [rcx+hvm_worker.alloc_head]
  cmp rax, 0
  je .no_head
.has_head:
  ; Load head
  mov rdx, [rax]
  mov [rcx+hvm_worker.alloc_head], rdx
  ret
.no_head:
  ; Increment next
  mov rax, [rcx+hvm_worker.alloc_next]
  mov rdx, [rcx+hvm_worker.area_start]
  add rdx, [rcx+hvm_worker.area_size]
  cmp rdx, rax
  jl .oom
  add [rcx+hvm_worker.alloc_next], 16
  ret
.oom:
  stc
  jmp $
  xor rax, rax
  ret
; Free 16 bytes
; IN = RCX: Pointer to worker, RDX: Allocation to free
hvm_free:
  mov rax, [rcx+hvm_worker.alloc_head]
  sub rax, rdx
  mov [rdx], rax
  mov [rcx+hvm_worker.alloc_head], rdx
  ret


k.shutdown: 
  uefi_call_wrapper RuntimeServices, ResetSystem, 2, EFI_SUCCESS, 0, 0

section '.data' readable writable

macro dport dest, tag, lab {
  dq hvm_tag_#tag or (dest-$) or lab shl 48
}
sample_data:
.1.1: 
  dport .2.2, AUX, 0
.1.2: 
  dq 0
; (λx x λy y) 
.2.1: 
  dport .3.1, CTR, 0
.2.2: 
  dport .1.1, AUX, 0
; λy y
.3.1:
  dport .3.2, AUX, 0
.3.2:
  dport .3.1, AUX, 0
; λx x
.4.1:
  dport .4.2, AUX, 0
.4.2:
  dport .4.1, AUX, 0
; Redex list
.5.1:
  dport .2.1, CTR, 0
.5.2:
  dq .6.1-$
.6.1:
  dport .3.1, CTR, 0
.6.2:
  dq .7.1-$
.7.1:
  dq 0
.7.2:
  dq .5.1-$
sample_data_end: