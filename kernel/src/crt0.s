.section .text
.global _start
.extern _init
.extern kernel_main
.extern _fini
_start:
    jmp kernel_main
    jmp .