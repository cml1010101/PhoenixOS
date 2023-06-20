CPP_SRCS=$(wildcard src/*.cpp)
C_SRCS=$(wildcard src/*.c)
ASM_SRCS=$(wildcard src/*.asm)
CPP_OBJS=${CPP_SRCS:.cpp=.cpp.o}
C_OBJS=${C_SRCS:.c=.c.o}
ASM_OBJS=${ASM_SRCS:.asm=.asm.o}
CRTI_OBJ=src/crti.s.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ=src/crtn.s.o
CRT0_OBJ=src/crt0.s.o
all: kernel.elf
kernel.elf: ${CPP_OBJS} ${C_OBJS} ${ASM_OBJS} ${CRT0_OBJ} ${CRTBEGIN_OBJ} ${CRTEND_OBJ} ${CRTN_OBJ} ${CRTI_OBJ}
	x86_64-elf-gcc ${CRTI_OBJ} ${CRTBEGIN_OBJ} ${CRT0_OBJ} ${CPP_OBJS} ${C_OBJS} ${ASM_OBJS} ${CRTEND_OBJ} ${CRTN_OBJ} -o $@ -Tlinker.ld -nostdlib -ffreestanding -znocombreloc -Bsymbolic -L${HOME}/opt/cross/lib/gcc/x86_64-elf/13.1.0 -lgcc
%.c.o: %.c
	x86_64-elf-gcc -c $< -o $@
%.cpp.o: %.cpp
	x86_64-elf-g++ -c $< -o $@ -fno-pic -ffreestanding -mno-red-zone -mno-mmx -I/usr/include/efi -I/usr/include/efi/x86_64 \
		-fno-stack-protector -D_GLIBCXX_USE_CXX11_ABI=0 -fshort-wchar -fno-exceptions \
		-Wall -Wextra -g -fno-rtti -D__DEBUG__ -Iinclude
%.asm.o: %.asm
	nasm $< -o $@ -f elf64
%.s.o: %.s
	x86_64-elf-as -c $< -o $@
clean:
	rm -rf kernel.elf kernel.a ${CPP_OBJS} ${C_OBJS} ${ASM_OBJS}