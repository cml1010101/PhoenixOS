CPP_SRCS=$(wildcard src/*.cpp)
C_SRCS=$(wildcard src/*.c)
ASM_SRCS=$(wildcard src/*.asm)
CPP_OBJS=${CPP_SRCS:.cpp=.cpp.o}
C_OBJS=${C_SRCS:.c=.c.o}
ASM_OBJS=${ASM_SRCS:.asm=.asm.o}
all: graphics.elf
graphics.elf: ${CPP_OBJS} ${C_OBJS} ${ASM_OBJS}
	x86_64-elf-ld -r ${CRTI_OBJ} ${CRTBEGIN_OBJ} ${CRT0_OBJ} ${CPP_OBJS} ${C_OBJS} ${ASM_OBJS} ${CRTEND_OBJ} ${CRTN_OBJ} -o $@
%.c.o: %.c
	x86_64-elf-gcc -c $< -o $@
%.cpp.o: %.cpp
	x86_64-elf-g++ -c $< -o $@ -Iinclude -I../../kernel/include
%.asm.o: %.asm
	nasm $< -o $@ -f elf64
%.s.o: %.s
	x86_64-elf-as -c $< -o $@
clean:
	rm -rf ${CPP_OBJS} ${C_OBJS} ${ASM_OBJS} graphics.elf