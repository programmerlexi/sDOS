org 0x0
bits 16
entry:
    mov si, stage_msg
    call print
    jmp $

print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0e
    int 0x10
    jmp print
.done:
    ret

stage_msg: db "Stage 2 loaded!", 0xD, 0xA, 0