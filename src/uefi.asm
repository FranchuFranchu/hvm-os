format pe64 efi
entry main



section '.reloc' fixups data readable discardable
if $=$$
    dd 0,8          ; if there are no fixups, generate dummy entry
end if


macro print_msg msg,UL {
  section '.text' executable readable
  uefi_call_wrapper ConOut, OutputString, ConOut, UL#.dat
  jmp UL#.link
  section '.data' readable writable
  UL#.dat: du msg, 0xd, 0, 0xa, 0, 0, 0, 61, 0
  section '.text' executable readable
  UL#.link:
}

include 'inc/uefi.inc'
include 'hvm.inc'
include 'string.inc'

section '.text' executable readable


main:
  
  InitializeLib
  
  mov rax, [sample_data.4.1]
  add rax, sample_data.4.1
  call print_rax
  
  sub rsp, 8
  uefi_call_wrapper ConOut, OutputString, ConOut, string

  mov rax, rsp
  call print_rax
  
  call hvm_init
  mov rbx, rax
  mov rcx, rbx

  mov [rsp], rcx
.loopy:
  mov rcx, [rsp]
  mov rdx, [rcx+hvm_worker.alloc_next]
  sub rdx, [rcx+hvm_worker.area_start]
  shr rdx, 3
  call hvm_log_worker

  ; Wait for key and maybe break
  sub rsp, 16
  uefi_call_wrapper ConIn, Reset, ConIn, 0
.poll:
  uefi_call_wrapper ConIn, ReadKeyStroke, ConIn, rsp
  mov rdx, EFI_NOT_READY
  sub rdx, rax
  jz .poll
  call print_rax
  add rsp, 16


  mov rcx, [rsp]
  call hvm_redex_pop
  jc .all_reduced

  mov rcx, [rsp]
  call hvm_interact

  jmp .loopy
.all_reduced:
  uefi_call_wrapper ConOut, OutputString, ConOut, some_msg
  sub rsp, 16
  uefi_call_wrapper ConIn, Reset, ConIn, 0
@@:
  uefi_call_wrapper ConIn, ReadKeyStroke, ConIn, rsp
  mov rdx, EFI_NOT_READY
  sub rdx, rax
  jz @b
  call print_rax
  add rsp, 16


.finish:
  add rsp, 8
  ret

print_rax:
  push_volatile
  mov r8, rax
  sub rsp, 64
  xor rcx, rcx

repeat 8
  rol r8, 8
  mov rax, r8
  call os_string_convert_2hex
  mov byte [rsp+rcx+1], 0
  mov byte [rsp+rcx], al
  add rcx, 2
  mov byte [rsp+rcx+1], 0
  mov byte [rsp+rcx], ah
  add rcx, 2
end repeat
  mov word [rsp+rcx], 0xd
  mov word [rsp+rcx+2], 0xa
  mov word [rsp+rcx+4], 0

  uefi_call_wrapper ConOut, OutputString, ConOut, rsp
  add rsp, 64
  pop_volatile
  ret

section '.data' readable writable

string du 'Hello, World!', 0xD, 0xA, 0
some_msg du 'Net reduced. Press any key to restart.', 0xD, 0xA, 0