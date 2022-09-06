#include "functions.h"
bool disk_init(disk_t *disk, uint8_t drive_number) {
    uint8_t drive_type;
    uint16_t cylinders, sectors, heads = 0;
    if (!get_drive_params(disk->id, &drive_type, &cylinders, &sectors, &heads))
        return false;
    
    disk->id = drive_number;
    disk->cylinders = cylinders;
    disk->heads = heads;
    disk->sectors = sectors;
    print_string("Disk initialized!\n\r");
    return true;
}

void disk_lba2chs(disk_t *disk, uint32_t lba, uint16_t *cylinder, uint16_t *sector, uint16_t *head) {
    print_string("LBA2CHS\n\r");
    print_string("Sector\n\r");
    *sector = (uint16_t)(lba % disk->sectors + 1);
    print_string("Cylinder\n\r");
    *cylinder = (uint16_t)(lba / disk->sectors) / disk->heads;
    print_string("Head\n\r");
    *head = (lba / disk->sectors) % disk->heads;
}

bool disk_read(disk_t *disk, uint32_t lba, uint8_t sectors, void *data) {
    print_string("Read disk!\n\r");
    uint16_t cylinder, sector, head;
    disk_lba2chs(disk, lba, &cylinder, &sector, &head);
    print_string("Attempting to read...\n\r");
    for (int i = 0; i < 3; i++) {
        print_string("x86 read!\n\r");
        if (x86_read_disk(disk->id, cylinder, sector, head, sectors, data))
            return true;
        print_string("Disk reset!\n\r");
        x86_reset_disk(disk->id);
    }
    return false;
}