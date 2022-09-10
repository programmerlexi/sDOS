bits 16
extern main
; following gets passed to stage 2:
;   dx - drive number
;   bx - sectors per track
;   cx - heads
entry:
    call fill_disk_struct
    mov si, stage_msg
    call print
    call main
.halt:
    hlt
    jmp .halt

fill_disk_struct:
    push ebp
    mov ebp, esp
    
    push di

    mov di, 0x5000
    mov [di], dl

    add di, 3

    mov [di], bx

    add di, 2
    
    mov [di], cx

    pop di

    mov esp, ebp
    pop ebp
    ret

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
    xor ax, ax
    in al, dx
    ret

[global get_drive_params]
get_drive_params:
    ; make new call frame
    push bp             ; save old call frame
    mov bp, sp          ; initialize new call frame

    ; save regs
    push es
    push bx
    push si
    push di

    ; call int13h
    mov dl, [bp + 4]    ; dl - disk drive
    mov ah, 08h
    mov di, 0           ; es:di - 0000:0000
    mov es, di
    stc
    int 13h

    ; return
    mov ax, 1
    sbb ax, 0

    and cl, 0x3F
    xor ch, ch
    mov si, [bp + 10]
    mov [si], cx

    inc dh
    mov si, [bp + 12]
    mov [si], dh

    ; out params
    ;mov si, [bp + 6]    ; drive type from bl
    ;mov [si], bl

    ;mov bl, ch          ; cylinders - lower bits in ch
    ;mov bh, cl          ; cylinders - upper bits in cl (6-7)
    ;shr bh, 6
    ;mov si, [bp + 8]
    ;mov [si], bx

    ;xor ch, ch          ; sectors - lower 5 bits in cl
    ;and cl, 3Fh
    ;mov si, [bp + 10]
    ;mov [si], cx

    ;mov cl, dh          ; heads - dh
    ;mov si, [bp + 12]
    ;mov [si], cx

    ; restore regs
    pop di
    pop si
    pop bx
    pop es

    ; restore old call frame
    mov sp, bp
    pop bp
    ret

[global x86_hs]
x86_hs:
    ; make new call frame
    push bp             ; save old call frame
    mov bp, sp          ; initialize new call frame

    ; call int13h
    push es
    mov dl, [bp + 4]    ; dl - disk drive
    mov ah, 08h
    int 13h
    pop es

    ; return
    mov ax, 1
    sbb ax, 0

    pusha
    and cl, 0x3F
    xor ch, ch
    mov si, [bp + 6]
    mov [si], cx

    inc dh
    mov si, [bp + 8]
    mov [si], dh
    popa

    mov sp, bp
    pop bp
    ret

[global x86_reset_disk]
x86_reset_disk:
    push bp
    mov bp, sp
    mov ah, 0
    mov dl, [bp + 4]
    stc
    int 13h
    mov ax, 1
    sbb ax, 0  
    mov sp, bp
    pop bp
    ret

[global x86_read_disk]
x86_read_disk:
    push bp
    mov bp, sp
    push bx
    push es
    mov dl, [bp + 4]
    mov ch, [bp + 6]
    mov cl, [bp + 7]
    shl cl, 6
    mov al, [bp + 8]
    and al, 3Fh
    or cl, al
    mov dh, [bp + 10]
    mov al, [bp + 12]
    mov bx, [bp + 16]
    mov es, bx
    mov bx, [bp + 14]
    mov ah, 02h
    stc
    int 13h
    mov ax, 1
    sbb ax, 0
    pop es
    pop bx
    mov sp, bp
    pop bp
    ret

stage_msg: db "Stage 2 loaded!", 0xD, 0xA, 0
g_boot_drive: db 0