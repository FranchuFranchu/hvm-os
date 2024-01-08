
section '.data' readable writable

macro dport dest, tag, lab {
  label @@ qword at $
  dq (hvm_tag_#tag) + (dest-@b) + (lab shl 48)
}
hvm_data:
.0.1: 
  dport .1.2, AUX, 0
.0.2: 
  dq 0
; (λx x λy y) 
.1.1: 
  dport .2.1, CTR, 0
.1.2: 
  dport .0.1, AUX, 0
; λy y
.2.1:
  dport .2.2, AUX, 0
.2.2:
  dport .2.1, AUX, 0
; λx x
.3.1:
  dport .3.2, AUX, 0
.3.2:
  dport .3.1, AUX, 0
; Redex list
.4.1:
  dport .1.1, CTR, 0
.4.2:
  dq .5.1-$
.5.1:
  dport .3.1, CTR, 1
.5.2:
  dq .6.1-$
.6.1:
  dq 0
.6.2:
  dq .4.1-$
hvm_data_end: