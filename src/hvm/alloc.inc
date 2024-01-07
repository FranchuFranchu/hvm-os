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
  hvm_get_port rax, rdx
  mov [rcx+hvm_worker.alloc_head], rdx
  mov rdx,hvm_special_value_FREE
  hvm_set_port_1 rax, rdx
  mov rdx,hvm_special_value_FREE
  hvm_set_port_2 rax, rdx
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
