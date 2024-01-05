format pe64 efi
entry main



section '.reloc' fixups data readable discardable
if $=$$
    dd 0,8          ; if there are no fixups, generate dummy entry
end if


include 'inc/uefi.inc'
include 'hvm.inc'
include 'string.inc'

section '.text' executable readable

main:

  InitializeLib
  
  uefi_call_wrapper ConOut, OutputString, ConOut, string

  call hvm_init
  mov rbx, rax
  mov rcx, rbx

  mov rdx, 1
  mov r9, 2
  mov r8, 3
  call hvm_redex_push

  mov rcx, rbx
  call hvm_redex_pop
  mov rax, rdx
  call print_rax
  mov rax, r8
  call print_rax
  mov rax, r9
  call print_rax

  mov rax, 0x0123456789ABCDEF
  call print_rax
  jmp $

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
  mov word [rsp+rcx], 0xa
  mov word [rsp+rcx+2], 0

  uefi_call_wrapper ConOut, OutputString, ConOut, rsp
  add rsp, 64
  pop_volatile
  ret

section '.data' readable writable

string du 'Hello, World!', 0xD, 0xA, 0