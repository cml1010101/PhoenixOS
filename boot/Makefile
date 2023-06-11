SRCS=$(wildcard src/*.c)
OBJS=${SRCS:.c=.c.o}
all: BOOTX64.efi
BOOTX64.efi: BOOTX64.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc \
    	--target efi-app-x86_64 --subsystem=10 BOOTX64.so BOOTX64.efi
BOOTX64.so: ${OBJS}
	ld ${OBJS} /usr/lib/crt0-efi-x86_64.o -o $@ -shared -Bsymbolic -T/usr/lib/elf_x86_64_efi.lds -L/usr/lib -lefi \
		-lgnuefi -nostdlib -znocombreloc
%.c.o: %.c
	gcc -I/usr/include/efi -I/usr/include/efi/x86_64 -fpic -fno-stack-protector -fshort-wchar \
		-mno-red-zone -DEFI_FUNCTION_WRAPPER -c $< -o $@
clean:
	rm -rf ${OBJS} BOOTX64.so BOOTX64.efi