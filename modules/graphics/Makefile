CPP_SRCS=$(wildcard src/*.cpp)
C_SRCS=$(wildcard src/*.c)
ASM_SRCS=$(wildcard src/*.asm)
CPP_OBJS=${CPP_SRCS:.cpp=.cpp.o}
C_OBJS=${C_SRCS:.c=.c.o}
ASM_OBJS=${ASM_SRCS:.asm=.asm.o}
all: graphics.elf
graphics.elf: ${CPP_OBJS} ${C_OBJS} ${ASM_OBJS}
	x86_64-elf-gcc -r -shared ${CRTI_OBJ} ${CRTBEGIN_OBJ} ${CRT0_OBJ} ${CPP_OBJS} ${C_OBJS} ${ASM_OBJS} ${CRTEND_OBJ} ${CRTN_OBJ} -o $@ -nostdlib -ffreestanding -znocombreloc -Bsymbolic -L${HOME}/opt/cross/lib/gcc/x86_64-elf/13.1.0 -lgcc
%.c.o: %.c
	x86_64-elf-gcc -c $< -o $@
%.cpp.o: %.cpp
	x86_64-elf-g++ -c $< -o $@ -fno-pic -ffreestanding -mno-red-zone -mno-mmx -I/usr/include/efi -I/usr/include/efi/x86_64 \
		-fno-stack-protector -D_GLIBCXX_USE_CXX11_ABI=0 -fshort-wchar -fno-exceptions \
		-Wall -Wextra -g -fno-rtti -D__DEBUG__ -Iinclude -I../../kernel/include
%.asm.o: %.asm
	nasm $< -o $@ -f elf64
%.s.o: %.s
	x86_64-elf-as -c $< -o $@
clean:
	rm -rf ${CPP_OBJS} ${C_OBJS} ${ASM_OBJS} graphics.elf