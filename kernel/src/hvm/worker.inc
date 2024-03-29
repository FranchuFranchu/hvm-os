

; Size must be a multiple of 16!
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
  .workers2 dptr
  .size = $ - .area_start
}
struct hvm_worker

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
  mov rsi, hvm_binary_code
  mov rdi, rax
  mov rcx, hvm_binary_code.end-hvm_binary_code
  rep movsq
  add rsp, 32
  ; Initialize the Worker struct
  ; It is in rdx
  mov [rdx+hvm_worker.alloc_next], rax
  mov [rdx+hvm_worker.area_start], rax
  mov rcx, 8*4096
  sub rcx, hvm_worker.size
  mov [rdx+hvm_worker.area_size], rcx
  mov rcx, [rax+8]
  cmp rcx, 0
  je .no_redexes
  add rcx, rax
  add rcx, 8
  mov [rdx+hvm_worker.redex], rcx
  jmp @f
.no_redexes:
  mov [rdx+hvm_worker.redex], 0
@@:
  add [rdx+hvm_worker.alloc_next], hvm_binary_code.end-hvm_binary_code
  mov [rdx+hvm_worker.alloc_head], 0
  mov [rdx+hvm_worker.worker_id], 0
  mov [rdx+hvm_worker.workers], 1
  mov rax, rdx

  ret
