cmake_minimum_required(VERSION 3.24)
project(PhoenixOS VERSION 2.0.0 LANGUAGES C CXX ASM ASM_NASM)
option(IS_QEMU_BUILD "Whether to use debugging information for qemu" ON)
set(CMAKE_SHARED_LIBRARY_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "")
set(CMAKE_SHARED_LIBRARY_ASM_FLAGS "")
add_subdirectory(boot)
add_subdirectory(kernel)
add_subdirectory(modules)
add_custom_command(
    OUTPUT disk.img
    COMMAND qemu-img create -f raw disk.img 5G
    COMMAND parted disk.img -s -a minimal mklabel gpt
	COMMAND parted disk.img -s -a minimal mkpart EFI FAT16 2048s 93716s
	COMMAND parted disk.img -s -a minimal toggle 1 boot
    COMMENT "Creating 5GB disk image"
)
add_custom_target(disk ALL
    DEPENDS disk.img
)
add_custom_target(
    mount
    COMMAND dd if=disk.img of=/tmp/part.img bs=512 count=91669 skip=2048 conv=notrunc
    COMMAND mformat -i /tmp/part.img -h 32 -t 32 -n 64 -c 1
    COMMAND mmd -i /tmp/part.img ::/EFI
    COMMAND mmd -i /tmp/part.img ::/EFI/BOOT
    COMMAND mcopy -i /tmp/part.img ${CMAKE_BINARY_DIR}/boot/boot.efi ::/EFI/BOOT/BOOTX64.efi
    COMMAND mcopy -i /tmp/part.img ${CMAKE_BINARY_DIR}/kernel/kernel.elf ::KERNEL.ELF
    COMMAND mcopy -i /tmp/part.img ${CMAKE_BINARY_DIR}/modules/initramfs.tar ::initramfs.tar
    COMMAND dd if=/tmp/part.img of=disk.img bs=512 count=91669 seek=2048 conv=notrunc
    DEPENDS boot kernel.elf initramfs
)
add_custom_target(
    run
    COMMAND qemu-system-x86_64 -d interrupt -s -serial stdio -cpu qemu64 -m 256 -d cpu_reset -bios /usr/share/qemu/OVMF.fd -drive file=disk.img,if=ide
    DEPENDS mount
)
add_custom_target(
    debug
    COMMAND qemu-system-x86_64 -d interrupt -s -S -serial stdio -cpu qemu64 -m 256 -d cpu_reset -bios /usr/share/qemu/OVMF.fd -drive file=disk.img,if=ide
    DEPENDS mount
)