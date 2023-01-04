all: workspace build run clean

workspace:
	mkdir tmp
	mkdir tmp/kernel
	mkdir imgs

build_stage2:
	nasm bootloader/stage2.asm -f elf -o tmp/stage2_entry.o
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c bootloader/stage2.cpp -o tmp/stage2.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c bootloader/disk.cpp -o tmp/disk.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c bootloader/fat.cpp -o tmp/fat.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c bootloader/memory.cpp -o tmp/memory.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c bootloader/string.cpp -o tmp/string.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c bootloader/port.cpp -o tmp/port.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-ld -m elf_i386 -o tmp/stage2.bin -Ttext 0x00000500 tmp/stage2_entry.o tmp/stage2.o tmp/disk.o tmp/fat.o tmp/memory.o tmp/string.o tmp/port.o --oformat binary

build_kernel:
	nasm kernel/kernel_entry.asm -f elf -o tmp/kernel/kernel_entry.o
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c kernel/kernel.cpp -o tmp/kernel/kernel.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c kernel/vga.cpp -o tmp/kernel/vga.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c kernel/keyboard.cpp -o tmp/kernel/keyboard.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c kernel/port.cpp -o tmp/kernel/port.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-ld -m elf_i386 -o tmp/kernel/kernel.bin -Ttext 0x500 tmp/kernel/kernel_entry.o tmp/kernel/kernel.o tmp/kernel/vga.o tmp/kernel/keyboard.o tmp/kernel/port.o --oformat binary

build_boot:
	nasm bootloader/alt_boot.asm -f bin -o tmp/boot.bin
	dd if=/dev/zero of=imgs/OS.img bs=512 count=2880
	sudo mkfs.fat -F 12 -n "SDOS SYSTEM" imgs/OS.img
	dd if=tmp/boot.bin of=imgs/OS.img conv=notrunc

build: workspace
	make build_boot
	make build_stage2
	make build_kernel
	mcopy -i imgs/OS.img tmp/stage2.bin "::stage2.bin"
	mcopy -i imgs/OS.img tmp/kernel/kernel.bin "::kernel.bin"

run: build
# Boot from HDA, 128 MB of memory
	qemu-system-x86_64 imgs/OS.img -m 128M

clean:
	rm -rf tmp
	rm -rf imgs

mount: build
	sudo mount imgs/OS.img mount

unmount:
	sudo umount mount