all: workspace build run clean

workspace:
	mkdir tmp
	mkdir imgs

stage2:
	nasm bootloader/stage2.asm -f elf -o tmp/stage2_entry.o
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c bootloader/stage2.cpp -o tmp/stage2.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c bootloader/disk.cpp -o tmp/disk.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c bootloader/fat.cpp -o tmp/fat.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c bootloader/memory.cpp -o tmp/memory.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-g++ -ffreestanding -m16 -g -c bootloader/string.cpp -o tmp/string.o -mno-red-zone -O1 -fpermissive -fno-pic -fno-builtin
	/usr/local/i386elfgcc/bin/i386-elf-ld -m elf_i386 -o tmp/stage2.bin -Ttext 0x00000000 tmp/stage2_entry.o tmp/stage2.o tmp/disk.o tmp/fat.o tmp/memory.o tmp/string.o --oformat binary

build: workspace
# Use nanobytes bootloader as my own does not work
	nasm bootloader/alt_boot.asm -f bin -o tmp/boot.bin
	make stage2
	dd if=/dev/zero of=imgs/OS.img bs=512 count=2880
	sudo mkfs.fat -F 12 -n "sDOS Boot" imgs/OS.img
	dd if=tmp/boot.bin of=imgs/OS.img conv=notrunc
	mcopy -i imgs/OS.img tmp/stage2.bin "::stage2.bin"

run: build
	qemu-system-x86_64 imgs/OS.img -m 128M

clean:
	rm -rf tmp
	rm -rf imgs

mount: build
	sudo mount imgs/OS.img mount

unmount:
	sudo umount mount