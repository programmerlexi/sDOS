#include "functions.h"
void print_char(char c) {
    asm(
        "mov %0, %%al;"
		"mov $0x0E, %%ah;"
		"int $0x10;"
		:
		: "r" (c)
    );
}

void print_string(char* string) {
    int i = 0;
    while (string[i]) {
        print_char(string[i]);
        i++;
    }
}

void print_string(const char *string) {
    print_string((char*)string);
}

extern "C" void main(uint16_t boot_drive) {
    print_string("Hello, World from C++!\n\r");
    print_string("Initializing disk driver... ");
    disk_t disk;
    if (!disk_init(&disk,boot_drive)) {
        print_string("Disk init error!\n\r");
        goto end;
    }
    print_string("OK\n\r");
    print_string("Initializing FAT driver... ");
    if (!fat_init(&disk)) {
        print_string("FAT init error!\n\r");
        goto end;
    }
    print_string("OK\n\r");
    print_string("Loading kernel.bin... ");
end:
    for (;;);
}