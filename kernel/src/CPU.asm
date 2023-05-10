global cpu_start_16
global cpu_start_stack
global cpu_start_cr3
global cpu_start_id
extern core_initialization_routine
section .cpu_init
[bits 16]
cpu_start_16:
    cli
    cld
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    mov eax, [cpu_start_cr3]
    mov cr3, eax
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    or eax, 1
    wrmsr
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax
    lgdt [cpu_start_gdt_ptr]
    jmp 0x8:cpu_start_64
[bits 64]
cpu_start_64:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rax, cr0
    and ax, 0xFFFB
    or ax, 0x2
    mov cr0, rax
    mov rax, cr4
    or ax, 3 << 9
    mov cr4, rax
    mov eax, [cpu_start_cr3]
    mov cr3, eax
    mov rsp, [cpu_start_stack]
    xor rbx, rbx
    mov rdi, [cpu_start_id]
    call core_initialization_routine
    jmp $
cpu_start_status:
    dw 0
cpu_start_gdt:
    dq 0
    dq 0xFFFF0000009AFA00
    dq 0xFFFF00000092FC00
cpu_start_gdt_ptr:
    dw (3 * 8 - 1)
    dq (cpu_start_gdt)
cpu_start_stack:
    dq 0
cpu_start_cr3:
    dd 0
cpu_start_id:
    dq 0