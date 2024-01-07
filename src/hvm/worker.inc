

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
  add rcx, sample_data.6.1-sample_data
  mov [rdx+hvm_worker.redex], rcx
  add [rdx+hvm_worker.alloc_next], sample_data_end-sample_data
  mov [rdx+hvm_worker.alloc_head], 0
  mov [rdx+hvm_worker.worker_id], 0
  mov [rdx+hvm_worker.workers], 1
  mov rax, rdx

  ret
