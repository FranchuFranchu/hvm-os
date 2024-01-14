
struc hvm_ring {
  .item dptr
  .next dptr
}
struct hvm_ring

; IN = RCX: Pointer to worker,
; OUT = RCX: Pointer to worker, RDX: Provenance, R8: First port (Absolute), R9: Second port (Absolute), CF: Set if ring empty
hvm_redex_pop:
  push rbx
  push rbx
  push rcx
  cmp qword [rcx+hvm_worker.redex], 0
  je .no_redex
  mov rbx, [rcx+hvm_worker.redex]
  hvm_get_port_2 rbx ,rdx

  ; Agent 1
  hvm_get_port_1 rdx, rbx
  push rbx
  hvm_get_port_2 rdx, rbx

  call hvm_free

  ; Agent 2
  mov rdx, rbx

  hvm_get_port_1 rdx, rbx
  push rbx
  hvm_get_port_2 rdx, rbx

  call hvm_free

  ; Provenance
  mov rdx, rbx

  mov rbx, [rdx]
  push rbx

  hvm_get_port_2 rdx, rbx

  push rdx
  call hvm_free
  pop rdx

  cmp rdx, [rcx+hvm_worker.redex]
  je .empty_ring
  mov rdx, [rcx+hvm_worker.redex]
  hvm_set_port_2 rdx, rbx
  ; Re-link the ring.

  pop rdx
  pop r9
  pop r8
  pop rcx
  pop rbx
  pop rbx
  clc
  ret
.empty_ring:
  mov [rcx+hvm_worker.redex], 0
  pop rdx
  pop r9
  pop r8
  pop rcx
  pop rbx
  pop rbx
  clc
  ret
.no_redex:
  pop rcx
  pop rbx
  pop rbx
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