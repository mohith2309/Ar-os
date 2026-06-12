[bits 32]
[global _start]
[extern kernel_main]
[extern __bss_start]
[extern _end]

_start:
    ; the kernel BSS (64KB framebuffer) overlaps 0x7c00 where the boot
    ; sector's GDT lives — load a GDT owned by the kernel image instead
    lgdt [gdt_desc]
    jmp 0x08:.reload
.reload:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; zero BSS — bare metal gives no guarantee this memory is clean
    mov edi, __bss_start
    mov ecx, _end
    sub ecx, edi
    xor eax, eax
    rep stosb
    call kernel_main
.hang:
    hlt
    jmp .hang

align 8
gdt:
    dd 0, 0                                  ; null
    dw 0xffff, 0x0000                        ; code: base 0, limit 4GB
    db 0x00, 10011010b, 11001111b, 0x00
    dw 0xffff, 0x0000                        ; data: base 0, limit 4GB
    db 0x00, 10010010b, 11001111b, 0x00
gdt_desc:
    dw gdt_desc - gdt - 1
    dd gdt
