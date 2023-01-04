#pragma once
#include "defines.h"
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

typedef struct {
    uint8_t name[11];
    uint8_t attributes;
    uint8_t _reserved;
    uint8_t created_time_tenths;
    uint16_t created_time;
    uint16_t created_date;
    uint16_t accessed_date;
    uint16_t first_cluster_high;
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t first_cluster_low;
    uint32_t size;
} __attribute__ ((packed)) fat_directory_entry_t;

typedef struct {
    int handle;
    bool is_directory;
    uint32_t position;
    uint32_t size;
} fat_file_t;

enum fat_attributes {
    FAT_ATTRIBUTE_READ_ONLY         = 0x01,
    FAT_ATTRIBUTE_HIDDEN            = 0x02,
    FAT_ATTRIBUTE_SYSTEM            = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID         = 0x08,
    FAT_ATTRIBUTE_DIRECTORY         = 0x10,
    FAT_ATTRIBUTE_ARCHIVE           = 0x20,
    FAT_ATTRIBUTE_LFN               = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

typedef struct {
    // Boot Record
    uint8_t boot_jump_instruction[3];
    uint8_t oem_id[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t root_dir_entries;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t large_sector;
    // Extended boot record
    uint8_t drive_number;
    uint8_t _reserved;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t sys_id;
} __attribute__ ((packed)) fat_boot_sector_t;

typedef struct {
    uint8_t buffer[SECTOR_SIZE];
    fat_file_t pub;
    bool opened;
    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t current_sector_in_cluster;
} fat_file_data_t;

typedef struct {
    union {
        fat_boot_sector_t boot_sector;
        uint8_t boot_sector_bytes[SECTOR_SIZE];
    } bs;
    fat_file_data_t root_directory;
    fat_file_data_t open_files[MAX_FILE_HANDLES];
} fat_data_t;

typedef struct {
    uint8_t id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
} disk_t;

typedef struct device {
    uint16_t base;
    uint16_t dev_ctl;
    uint8_t master_slave_bit;
    uint64_t type;
    char* dev_name;
    device(uint16_t b, uint16_t ctl, uint8_t ms, char* name);
} device_t;

#define DEV_OFF_DATA 0
#define DEV_OFF_ERR 1
#define DEV_OFF_FEAT 1
#define DEV_OFF_SC 2
#define DEV_OFF_SN 3
#define DEV_OFF_CL 4
#define DEV_OFF_CH 5
#define DEV_OFF_DSEL 6
#define DEV_OFF_STAT 7
#define DEV_OFF_CMD 7

enum dev_type {
    DEV_PATA,
    DEV_SATA,
    DEV_PATAPI,
    DEV_SATAPI,
    DEV_UNKNOWN,
    DEV_DISCONNECTED,
    DEV_UNINTIALIZED
};