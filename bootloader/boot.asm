[bits 16]
[org 0x7c00]

jmp short entry
nop

bpb_oem: db "MSWIN4.1"
bpb_bps: dw 0x200
bpb_spc: db 1
bpb_rs:  dw 1
bpb_fc:  db 2
bpb_rde: dw 0x0E0
bpb_ts:  dw 2880        ; 1.44 MB
bpb_mt:  db 0xF0        ; 3.5" floppy disk
bpb_spf: dw 9
bpb_spt: dw 18
bpb_h:   dw 2
bpb_hs:  dd 0
bpb_lsc: dd 0

ebr_dn:  db 0
         db 0
ebr_sig: db 0x29
ebr_vid: db 0x73, 0x44, 0x4f, 0x53
ebr_vn:  db "sDOS Boot "
ebr_sid: db "FAT12   "

[global entry]
entry:
    mov [ebr_dn], dl
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    clc
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, 0x80
    int 0x13
    mov ax, 0x1
    jc error

    mov si, msg_loading
    call print

    push es
    mov ah, 08h
    int 13h
    mov ax, 0x2 ; 0x2 = Disk error
    jc error ; Call error in case there actually was an error
    pop es

    and cl, 0x3F
    xor ch, ch
    mov [bpb_spt], cx

    inc dh
    mov [bpb_h], dh

    mov ax, [bpb_spf]
    mov bl, [bpb_fc]
    xor bh, bh
    mul bx
    add ax, [bpb_rs]
    push ax

    mov ax, [bpb_rde]
    shl ax, 5
    xor dx, dx
    div word [bpb_bps]

    test dx, dx
    jz .root_dir_after
    inc ax
.root_dir_after:
    mov [blkcnt], al
    pop ax
    mov [d_lba], ax
    mov word [db_add], buffer
    mov word [db_add+2], 0
    call read_disk

    xor bx, bx
    mov di, buffer
.search_kernel:
    mov si, stage2_file
    mov cx, 11
    push di
    repe cmpsb
    pop di
    je .found_kernel

    add di, 32
    inc bx
    cmp bx, [bpb_rde]
    jl .search_kernel

    mov ax, 0x3
    call error
.found_kernel:
    mov ax, [di+26]
    mov [stage2_cluster], ax
    mov ax, [bpb_rs]
    mov [d_lba], ax
    mov word [db_add], buffer
    mov ax, [bpb_spf]
    mov [blkcnt], ax
    call read_disk

    mov bx, 0
    mov es, bx
    mov bx, 0x1000
.load_kernel_loop:
    mov ax, [stage2_cluster]
    add ax, 31
    mov [d_lba], ax
    mov word [blkcnt], 1
    call read_disk
    add bx, [bpb_bps]
    mov ax, [stage2_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx
    mov si, buffer
    add si, ax
    mov ax, [ds:si]
    or dx, dx
    jz .even
.odd:
    shr ax, 4
    jmp .next_cluser_after
.even:
    and ax, 0x0FFF
.next_cluser_after:
    mov dl, [ebr_dn]
    mov ax, 0
    mov ds, ax
    mov es, ax
    jmp 0:0x1000

read_disk:
    pusha
    clc
    mov si, PACKET
    mov ah, 0x42
    mov dl, [ebr_dn]
    int 0x13
    mov ax, 0x2
    jc .on_error
    popa
    ret
.on_error:
    cmp ah, 0
    mov ax, 0x2
    jne error
    popa
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

error:
    mov si, error_msg
    call print
    cmp ax, 0x1
    je .lba_error
    cmp ax, 0x2
    je .disk_error
    cmp ax, 0x3
    je .nf_error
.end:
    jmp $
    ret
.lba_error:
    mov si, lba_msg
    call print
    jmp .end
.disk_error:
    mov si, disk_msg
    call print
    jmp .end
.nf_error:
    mov si, nf_msg
    call print
    jmp .end

; Variables
PACKET:
        db 0x10 ; Packet size
        db 0    ; always 0
blkcnt: dw 5    ; number of sectors
db_add: dw 0    ; memory buffer
        dw 0    ; memory page
d_lba:  dd 1    ; lba
        dd 0    ; if the lba is > 4 Bytes

; Strings
error_msg: db "ERROR", 0xD,0xA,0
lba_msg: db "LBA not supported", 0
disk_msg: db "Disk Error", 0
nf_msg: db "Stage 2 not found", 0
msg_loading: db "Loading...", 0xD, 0xA, 0
stage2_file: db "STAGE2  BIN", 0
stage2_cluster: dw 0
times 510-($-$$) db 0
dw 0xAA55
buffer: