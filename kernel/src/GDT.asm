[bits 64]
global load_gdt
load_gdt:
    mov rax, [rsp + 8]
    push rbp
    mov rbp, rsp
    mov rbx, rdi
    lgdt [rbx]
    mov ss, rax
    push rax
    push rbp
    pushfq
    push rsi
    push qword .l
    iretq
.l:
    mov gs, r9w
    mov es, r8w
    mov fs, cx
    mov ds, dx
    mov rsp, rbp
    pop rbp
    ret
global read_gdt
read_gdt:
    sgdt [rdi]
    ret