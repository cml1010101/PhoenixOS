ENTRY(_start)
SECTIONS
{
    _cpu_init_start = .;
    . = 0x2000;
    .cpu_init BLOCK(4K) : ALIGN(4K)
    {
        *(.cpu_init)
    }
    _cpu_init_end = .;
    . = 0x10000;
    _kernel_start = .;
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
    .dynsym BLOCK(4K) : ALIGN(4K)
    {
        *(.dynsym)
    }
    .dynamic BLOCK(4K) : ALIGN(4K)
    {
        *(.dynamic)
    }
    .rel BLOCK(4K) : ALIGN(4K)
    {
        *(.rel)
    }
    .rela BLOCK(4K) : ALIGN(4K)
    {
        *(.rela)
    }
    .reloc BLOCK(4K) : ALIGN(4K)
    {
        *(.reloc)
    }
    .sdata BLOCK(4K) : ALIGN(4K)
    {
        *(.sdata)
    }
    .init BLOCK(4K) : ALIGN(4K)
    {
        *(.init)
    }
    .fini BLOCK(4K) : ALIGN(4K)
    {
        *(.fini)
    }
    .init_array BLOCK(4K) : ALIGN(4K)
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
    }
    .init_array.start BLOCK(4K) : ALIGN(4K)
    {
        *(.init_array.start)
    }
    .init_array.end BLOCK(4K) : ALIGN(4K)
    {
        *(.init_array.end)
    }
    _kernel_end = .;
}