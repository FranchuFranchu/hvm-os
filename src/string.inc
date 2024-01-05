
section '.text' executable readable
; IN = AL: Number to write
; OUT = AL: As a character
os_string_convert_1hex:
    and al, 0xF ; Mask out higher nibble

    push ax
    sub al, 0x9
    jng .number
.notnumber: ; A-F
    pop ax
    ; ascii 41 = A
    sub al, 0xA

    add al, "A"
    jmp .done

.number:
    pop ax
    add al, "0"

.done:
    ret

; IN = AL: Number to write
; OUT = AX: As two characters
os_string_convert_2hex:
    push rdx

    push rax
    mov rdx, 0

    call os_string_convert_1hex
    or dl, al

    pop rax
    shr rax, 4
    call os_string_convert_1hex
    shl rdx, 8
    or dl, al
    mov rax, rdx

    pop rdx
    ret
