.section .text
.global _start
.extern _init
.extern kernel_main
.extern _fini
_start:
    call _call_global_constructors
    jmp kernel_main
    jmp .   
/* Helper function to call the global constructors */
.global _call_global_constructors
_call_global_constructors:
    /* Get the addresses of the constructor functions from the symbol table */
    mov $__init_array_start, %rax
    mov $__init_array_end, %rdx
    
    /* Iterate over the symbol table and call each constructor function */
    1: cmp %rax, %rdx
       jge 2f
       mov (%rax), %rcx
       test %rcx, %rcx
       je 2f
       call *%rcx
       add $8, %rax
       jmp 1b
    2:
    
    ret