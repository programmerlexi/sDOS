bits 16
extern main
entry:
    mov si, stage_msg
    call print
    call main
.halt:
    hlt
    jmp .halt

print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0e
    int 0x10
    jmp print
.done:
    ret

%macro LinearToSegOffset 4

    mov %3, %1      ; linear address to eax
    shr %3, 4
    mov %2, %4
    mov %3, %1      ; linear address to eax
    and %3, 0xf

%endmacro

[global outb]
outb:
    mov dx, [bp + 4]
    mov al, [bp + 8]
    out dx, al
    ret

[global inb]
inb:
    mov dx, [bp + 4]
    xor eax, eax
    in al, dx
    ret

[global get_drive_params]
get_drive_params:
    push bp
    mov bp, sp
    push es
    push bx
    push esi
    push di
    mov dl, [bp + 8]
    mov ah, 0x08
    mov di, 0
    stc
    int 0x13
    mov eax, 1
    sbb eax, 0
    LinearToSegOffset [bp + 12], es, esi, si
    mov [es:si], bl
    mov bl, ch
    mov bh, cl
    shr bh, 6
    inc bx
    LinearToSegOffset [bp + 16], es, esi, si
    mov [es:si], bx
    xor ch, ch
    and cl, 0x3F
    LinearToSegOffset [bp + 20], es, esi, si
    mov [es:si], cx
    mov cl, dh
    inc cx
    LinearToSegOffset [bp + 24], es, esi, si
    mov [es:si], cx
    pop di
    pop esi
    pop bx
    pop es
    mov sp, bp
    pop bp
    ret

[global x86_reset_disk]
x86_reset_disk:
    push bp
    mov bp, sp
    mov ah, 0
    mov dl, [bp + 8]
    stc
    int 0x13
    mov eax, 1
    sbb eax, 0
    mov sp, bp
    pop bp
    ret

[global x86_read_disk]
x86_read_disk:
    push bp
    mov bp, sp
    push ebx
    push es
    mov dl, [bp + 8]
    mov ch, [bp + 12]
    mov cl, [bp + 13]
    shl cl, 6
    mov al, [bp + 16]
    and al, 0x3F
    or cl, al
    mov dh, [bp + 20]
    mov al, [bp + 24]
    LinearToSegOffset [bp + 28], es, ebx, bx
    mov ah, 0x02
    stc
    int 0x13
    mov eax, 1
    sbb eax, 0
    pop es
    pop ebx
    mov sp, bp
    pop bp
    ret

stage_msg: db "Stage 2 loaded!", 0xD, 0xA, 0