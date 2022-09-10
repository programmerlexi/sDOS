bits 16
extern main
entry:
    mov si, kernel_loading
    call print
    call main
    call wait_key_and_reboot

print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0e
    int 0x10
    jmp print
.done:
    ret

wait_key_and_reboot:
    mov ah, 0
    int 16h                     ; wait for keypress
    jmp 0FFFFh:0                ; jump to beginning of BIOS, should reboot

kernel_loading: db "Starting sDos...", 0xD, 0xA, 0