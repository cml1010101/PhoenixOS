all: boot/BOOTX64.efi kernel/kernel.elf modules/graphics/graphics.elf
boot/BOOTX64.efi:
	cd boot && make all
kernel/kernel.elf:
	cd kernel && make all
modules/graphics/graphics.elf:
	cd modules/graphics && make all
clean:
	cd kernel && make clean
	cd boot && make clean
	cd modules/graphics && make clean
disk.img:
	qemu-img create -f raw disk.img 5G
	parted disk.img -s -a minimal mklabel gpt
	parted disk.img -s -a minimal mkpart EFI FAT16 2048s 93716s
	parted disk.img -s -a minimal toggle 1 boot
mount: disk.img
	dd if=/dev/zero of=/tmp/part.img bs=512 count=91669
	mformat -i /tmp/part.img -h 32 -t 32 -n 64 -c 1
	mmd -i /tmp/part.img ::/EFI
	mmd -i /tmp/part.img ::/EFI/BOOT
	mcopy -i /tmp/part.img boot/BOOTX64.efi ::/EFI/BOOT/BOOTX64.efi
	mcopy -i /tmp/part.img kernel/kernel.elf ::KERNEL.ELF
	mcopy -i /tmp/part.img modules/modules.cfg ::MODULES.CFG
	mcopy -i /tmp/part.img modules/graphics/graphics.elf ::GRAPHICS.ELF
	dd if=/tmp/part.img of=disk.img bs=512 count=91669 seek=2048 conv=notrunc
run: mount
	qemu-system-x86_64 -d interrupt -s -serial stdio -cpu qemu64 -m 256 -d cpu_reset -bios /usr/share/qemu/OVMF.fd -drive file=disk.img,if=ide
debug: mount
	qemu-system-x86_64 -d interrupt -s -S -serial stdio -cpu qemu64 -m 256 -d cpu_reset -bios /usr/share/qemu/OVMF.fd -drive file=disk.img,if=ide