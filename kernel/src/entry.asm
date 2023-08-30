MAGIC equ 0xE85250D6
ARCH equ 0
LENGTH equ (multiboot_end - multiboot_start)
CHECKSUM equ -(MAGIC + ARCH + LENGTH)
section .boot.text
multiboot_start:
    align 8
    dd MAGIC
    dd ARCH
    dd LENGTH
    dd CHECKSUM
framebuffer_tag:
    align 8
    dw 5
    dw 0
    dd 20
    dd 0
    dd 0
    dd 32
module_tag:
    align 8
    dw 6
    dw 0
    dd 8
end_tag:
    align 8
    dw 0
    dw 0
    dd 8
multiboot_end:
entry:
    mov dword [multiboot_addr], ebx
    mov eax, [ebx + 88]
    mov dword [framebuffer_address], eax
    mov eax, [ebx + 96]
    mov dword [framebuffer_pitch], eax
    mov eax, [ebx + 104]
    mov dword [fframebuffer_height]
framebuffer_address:
    dd 0
framebuffer_pitch:
    dd 0
fframebuffer_height:
    dd 0
multiboot_addr:
    dd 0
    dd 0