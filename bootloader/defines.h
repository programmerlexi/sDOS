#pragma once
#define SECTOR_SIZE 512
#define MAX_PATH_SIZE 256
#define MAX_FILE_HANDLES 10
#define ROOT_DIRECTORY_HANDLE -1
#define MEMORY_MIN          0x00000500
#define MEMORY_MAX          0x00080000
#define MEMORY_FAT_ADDR     ((void*)0x20000)
#define MEMORY_FAT_SIZE     0x00010000
#define MEMORY_LOAD_KERNEL  ((void*)0x30000)
#define MEMORY_LOAD_SIZE    0x00010000
#define MEMORY_KERNEL_ADDR  ((void*)0x100000)
#define NULL ((void*)0)
#define min(a,b)    ((a) < (b) ? (a) : (b))
#define max(a,b)    ((a) > (b) ? (a) : (b))