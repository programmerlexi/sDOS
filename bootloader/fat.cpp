#include "functions.h"
static fat_data_t* g_data;
static uint8_t* g_fat = (uint8_t*)NULL;
static uint32_t g_data_section_lba;

bool fat_read_boot_sector(device_t *disk) {
    print_string("Reading boot sector!\n\r");
    return disk_read(disk, 0, 1, g_data->bs.boot_sector_bytes);
}

bool fat_read_fat(device_t *disk) {
    print_string("Reading fat!\n\r");
    return disk_read(disk, g_data->bs.boot_sector.reserved_sectors, g_data->bs.boot_sector.sectors_per_fat, &g_fat);
}

bool fat_init(device_t *disk) {
    g_data = (fat_data_t*)MEMORY_FAT_ADDR;
    if (!fat_read_boot_sector(disk)) {
        print_string("FAT: failed to read boot sector\n\r");
        return false;
    }
    print_string("Got boot sector!\n\r");
    g_fat = (uint8_t*)g_data + sizeof(fat_data_t);
    uint32_t fat_size = g_data->bs.boot_sector.bytes_per_sector * g_data->bs.boot_sector.sectors_per_fat;
    if (sizeof(fat_data_t)+fat_size >= MEMORY_FAT_SIZE) {
        print_string("FAT: not enough memory to read fat\n\r");
        return false;
    }
    if (!fat_read_fat(disk)) {
        print_string("FAT: failed to read fat\n\r");
        return false;
    }
    print_string("Got fat!\n\r");
    uint32_t root_dir_lba = g_data->bs.boot_sector.reserved_sectors + g_data->bs.boot_sector.sectors_per_fat * g_data->bs.boot_sector.fat_count;
    uint32_t root_dir_size = sizeof(fat_directory_entry_t) * g_data->bs.boot_sector.root_dir_entries;
    g_data->root_directory.pub.handle = ROOT_DIRECTORY_HANDLE;
    g_data->root_directory.pub.is_directory = true;
    g_data->root_directory.pub.position = 0;
    g_data->root_directory.pub.size = sizeof(fat_directory_entry_t) * g_data->bs.boot_sector.root_dir_entries;
    g_data->root_directory.opened = true;
    g_data->root_directory.first_cluster = root_dir_lba;
    g_data->root_directory.current_cluster = root_dir_lba;
    g_data->root_directory.current_sector_in_cluster = 0;
    if (!disk_read(disk, root_dir_lba, 1, g_data->root_directory.buffer)) {
        print_string("FAT: failed to read root directory");
        return false;
    }
    print_string("Got root directory!\n\r");
    uint32_t root_dir_sectors = (root_dir_size + 512 - 1) / 512;
    g_data_section_lba = root_dir_lba + root_dir_sectors;
    for (int i = 0; i < MAX_FILE_HANDLES; i++)
        g_data->open_files[i].opened = false;
    print_string("Cleared handles!\n\r");
    return true;
}

uint32_t fat_cluster2lba(uint32_t cluster) {
    return g_data_section_lba + (cluster - 2) * g_data->bs.boot_sector.sectors_per_cluster;
}

fat_file_t *fat_open_entry(device_t *disk, fat_directory_entry_t *entry) {
    int handle = -1;
    for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++) {
        if (!g_data->open_files[i].opened)
        handle = i;
    }
    if (handle < 0) {
        print_string("FAT: out of file handles\n\r");
        return (fat_file_t*)NULL;
    }
    fat_file_data_t *fd = &g_data->open_files[handle];
    fd->pub.handle = handle;
    fd->pub.is_directory = (entry->attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fd->pub.position = 0;
    fd->pub.size = entry->size;
    fd->first_cluster = entry->first_cluster_low + ((uint32_t)entry->first_cluster_high << 16);
    fd->current_cluster = fd->first_cluster;
    fd->current_sector_in_cluster = 0;
    if (!disk_read(disk, fat_cluster2lba(fd->current_cluster), 1, fd->buffer)) {
        print_string("FAT: failed to open entry: ");
        for (int i = 0; i < 11; i++)
            print_char(entry->name[i]);
        print_string("\n\r");
        return (fat_file_t*)NULL;
    }
    fd->opened = true;
    return &fd->pub;
}

uint32_t fat_next_cluster(uint32_t current_cluster) {
    uint32_t fat_index = current_cluster * 3 / 2;
    if (current_cluster % 2 == 0) {
        return (*(uint16_t*)(g_fat + fat_index)) & 0x0fff;
    } else {
        return (*(uint16_t*)(g_fat + fat_index)) >> 4;
    }
}

uint32_t fat_read(device_t *disk, fat_file_t *file, uint32_t count, void* data) {
    fat_file_data_t *fd = (file->handle == ROOT_DIRECTORY_HANDLE)
        ? &g_data->root_directory
        : &g_data->open_files[file->handle];
    uint8_t *u_data = (uint8_t*)data;
    if (!fd->pub.is_directory || (fd->pub.is_directory && fd->pub.size != 0))
        count = min(count, fd->pub.size - fd->pub.position);
    while (count > 0) {
        uint32_t left_in_buffer = SECTOR_SIZE - (fd->pub.position % SECTOR_SIZE);
        uint32_t take = min(count, left_in_buffer);
        memcpy(u_data, fd->buffer + fd->pub.position % SECTOR_SIZE, take);
        u_data += take;
        fd->pub.position += take;
        count -= take;
        if (left_in_buffer == take) {
            if (fd->pub.handle == ROOT_DIRECTORY_HANDLE) {
                ++fd->current_cluster;
                if (!disk_read(disk, fd->current_cluster, 1, fd->buffer)) {
                    print_string("FAT: read error!\n\r");
                    break;
                }
            } else {
                if (++fd->current_sector_in_cluster >= g_data->bs.boot_sector.sectors_per_cluster) {
                    fd->current_sector_in_cluster = 0;
                    fd->current_cluster = fat_next_cluster(fd->current_cluster);
                }
                if (fd->current_cluster >= 0xFF8) {
                    fd->pub.size = fd->pub.position;
                    break;
                }
                if (!disk_read(disk, fat_cluster2lba(fd->current_cluster) + fd->current_sector_in_cluster, 1, fd->buffer)) {
                    print_string("FAT: read error\n\r");
                    break;
                }
            }
        }
    }
    return u_data - (uint8_t*)data;
}

bool fat_read_entry(device_t *disk, fat_file_t *file, fat_directory_entry_t *directory_entry) {
    return fat_read(disk, file, sizeof(fat_directory_entry_t), directory_entry) == sizeof(fat_directory_entry_t);
}

void fat_close(fat_file_t *file) {
    if (file->handle == ROOT_DIRECTORY_HANDLE) {
        file->position = 0;
        g_data->root_directory.current_cluster = g_data->root_directory.first_cluster;
    } else {
        g_data->open_files[file->handle].opened = false;
    }
}

bool fat_find_file(device_t *disk, fat_file_t* file, const char* name, fat_directory_entry_t *out_entry) {
    char fat_name[11];
    fat_directory_entry_t entry;
    memset(fat_name, ' ', sizeof(fat_name));
    fat_name[11] = 0;
    const char* ext = strchr(name, '.');
    if (!ext) {
        ext = name + 11;
    }
    for (int i = 0; i < 8 && name[i] && name+i < ext; i++) {
        fat_name[i] = toupper(name[i]);
    }
    if (ext != name + 11) {
        for (int i = 0; i < 3 && ext[i+1]; i++) {
            fat_name[i+8] = toupper(ext[i+1]);
        }
    }
    while (fat_read_entry(disk, file, &entry)) {
        if (memcmp(fat_name, entry.name, 11) == 0) {
            *out_entry = entry;
            return true;
        }
    }
    return false;
}

fat_file_t *fat_open(device_t *disk, const char *path) {
    char name[MAX_PATH_SIZE];
    if (path[0]=='/')
        path++;
    fat_file_t *current = &g_data->root_directory.pub;
    while (*path) {
        bool is_last = false;
        const char* delim = strchr(path, '/');
        if (delim != NULL) {
            memcpy(name, (void*)path, delim - path);
            name[delim - path + 1] = 0;
            path = delim + 1;
        } else {
            unsigned len = strlen(path);
            memcpy(name, (void*)path, len);
            name[len+1] = 0;
            path += len;
            is_last = true;
        }
        fat_directory_entry_t entry;
        if (fat_find_file(disk, current, name, &entry)) {
            print_string("Found file in path\n\r");
            fat_close(current);
            print_string("Closed directory\n\r");
            if (!is_last && entry.attributes & FAT_ATTRIBUTE_DIRECTORY == 0) {
                print_string("FAT: not a directory\n\r");
                return (fat_file_t*)NULL;
            }
            print_string("Opening next file in path\n\r");
            current = fat_open_entry(disk, &entry);
        } else {
            fat_close(current);
            print_string("FAT: file not found\n\r");
            return (fat_file_t*)NULL;
        }
    }
    print_string("Got file\n\r");
    return current;
}