#include "functions.h"
#include <stdarg.h>
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

void print_string(char* string, int length) {
    for (int i = 0; i < length; i++) {
        print_char(string[i]);
    }
}

void printk(uint8_t *format, ...)
{
    va_list ap;
    va_start(ap, format);

    uint8_t *ptr;

    for (ptr = format; *ptr != '\0'; ptr++) {
        if (*ptr == '%') {
            ptr++;
            switch (*ptr) {
                case 's':
                    print_string(va_arg(ap, char *));
                    break;
                case '%':
                    print_char('%');
                    break;
            }
        } else {
            print_char(*ptr);
        }
    }
    va_end(ap);
}

uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;
uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;

typedef void (*KernelStart)();

extern "C" void main(uint16_t boot_drive) {
    print_string("Hello, World from C++!\n\r");
    print_string("Initializing disk driver... \n\r");
    disk_t* disk = (disk_t*)0x5000;
    device_t dev0 = device_t(0x1f0,0x3F6,0xA0,"Disk 1");
    device_t dev1 = device_t(0x1f0,0x3F6,0xB0,"Disk 2");
    device_t dev2 = device_t(0x170,0x376,0xA0,"Disk 3");
    device_t dev3 = device_t(0x170,0x376,0xB0,"Disk 4");
    device_t* dev = &dev0;
    fat_boot_sector_t* boot_sector = (fat_boot_sector_t*)MEMORY_FAT_ADDR;
    if (!disk_init(dev,boot_drive)) {
        device_t* dev = &dev1;
        if (!disk_init(dev,boot_drive)) {
            device_t* dev = &dev2;
            if (!disk_init(dev,boot_drive)) {
                device_t* dev = &dev3;
                if (!disk_init(dev,boot_drive)) {
                    print_string("Disk init error!\n\r");
                    for (;;);
                }
            }
        }
    }
    print_string("OK\n\r");
    print_string("Initializing FAT driver... ");
    if (!fat_init(dev)) {
        print_string("FAT init error!\n\r");
        for (;;);
    }
    print_string("OK\n\r");
    print_string("VolumeId: ");
    print_string((char*)boot_sector->volume_label,11);
    print_string("\n\r");
    print_string("Loading kernel.bin... ");
    fat_file_t* fd = fat_open(dev,"/kernel.bin");
    if (fd != NULL) {
        uint32_t read;
        uint8_t* kernelBuffer = Kernel;
        while ((read = fat_read(dev, fd, MEMORY_LOAD_SIZE, Kernel)))
        {
            memcpy(kernelBuffer, KernelLoadBuffer, read);
            kernelBuffer += read;
            print_string("Copied bytes\n\r");
        }
        fat_close(fd);
        print_string("Done\n\r");
        // execute kernel
        print_string("Jumping to kernel...\n\r");
        return;
    } else {
        print_string("Something went wrong!\n\r");
    }
    for (;;);
}