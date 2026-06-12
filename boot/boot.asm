[org 0x7c00]

KERNEL_OFFSET equ 0x10000      ; load kernel at 64KB, clear of this sector

[bits 16]

start:
    mov [BOOT_DRIVE], dl
    mov bp, 0x9f00
    mov sp, bp
    in  al, 0x92               ; fast A20 (buffers live above 1MB)
    or  al, 2
    out 0x92, al
    xor ax, ax                 ; VBE mode info block -> 0x0600
    mov es, ax
    mov di, 0x600
    mov ax, 0x4f01
    mov cx, 0x4115             ; 800x600x24 + linear framebuffer
    int 0x10
    cmp ax, 0x004f
    jne vbe_fail
    mov ax, 0x4f02
    mov bx, 0x4115
    int 0x10
    cmp ax, 0x004f
    jne vbe_fail
    call load_kernel
    call switch_to_pm
    jmp $

vbe_fail:
    mov bx, VBE_ERR_MSG
    call print_rm
    jmp $

print_rm:
    pusha
    mov ah, 0x0e
.loop:
    mov al, [bx]
    cmp al, 0
    je .done
    int 0x10
    add bx, 1
    jmp .loop
.done:
    popa
    ret

SECTORS equ 160                ; kernel sectors to load (LBA 1..SECTORS)

load_kernel:
    mov ax, KERNEL_OFFSET >> 4
    mov es, ax
    mov word [LBA], 1
.next:
    mov ax, [LBA]
    cmp ax, SECTORS + 1
    ja  .done
    ; CHS for 1.44MB floppy: 18 spt, 2 heads
    xor dx, dx
    mov cx, 18
    div cx                     ; ax = track#, dx = sector-1
    mov cl, dl
    inc cl                     ; CL = sector (1-based)
    mov dh, al
    and dh, 1                  ; head = track & 1
    mov ch, al
    shr ch, 1                  ; cyl  = track >> 1
    mov dl, [BOOT_DRIVE]
    xor bx, bx
    mov ah, 0x02
    mov al, 1
    int 0x13
    jc .disk_error
    mov ax, es
    add ax, 0x20               ; next 512-byte slot
    mov es, ax
    inc word [LBA]
    jmp .next
.done:
    ret
.disk_error:
    mov bx, DISK_ERR_MSG
    call print_rm
    jmp $

LBA dw 0

gdt_start:
    dd 0x00000000
    dd 0x00000000
gdt_code:
    dw 0xffff
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
gdt_data:
    dw 0xffff
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

switch_to_pm:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or  eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp
    jmp KERNEL_OFFSET

BOOT_DRIVE   db 0
DISK_ERR_MSG db "Disk err", 0
SECT_ERR_MSG db "Sect err", 0
VBE_ERR_MSG  db "VBE err", 0

times 510-($-$$) db 0
dw 0xaa55
