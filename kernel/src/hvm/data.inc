
section '.data' readable writable

macro dport dest, tag, lab {
  label @@ qword at $
  dq (hvm_tag_#tag) + (dest-@b) + (lab shl 48)
}
hvm_binary_code:
FILE "../kernel.bin"
hvm_binary_code.end: