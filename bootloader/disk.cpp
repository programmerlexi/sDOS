#include "functions.h"
bool disk_init(disk_t *disk, uint8_t drive_number) {
    uint8_t drive_type;
    uint16_t cylinders, sectors, heads = 0;
    if (!get_drive_params(drive_number, &drive_type, &cylinders, &sectors, &heads))
        return false;
    
    disk->id = drive_number;
    disk->cylinders = cylinders;
    disk->heads = heads;
    disk->sectors = sectors;
    return true;
}

void disk_lba2chs(disk_t *disk, uint32_t lba, uint16_t *cylinder, uint16_t *sector, uint16_t *head) {
    *sector = lba % disk->sectors + 1;
    *cylinder = (lba / disk->sectors) / disk->heads;
    *head = (lba / disk->sectors) % disk->heads;
}

bool disk_read(disk_t *disk, uint32_t lba, uint8_t sectors, void *data) {
    uint16_t cylinder, sector, head = 0;
    disk_lba2chs(disk, lba, &cylinder, &sector, &head);
    for (int i = 0; i < 3; i++) {
        if (x86_read_disk(disk->id, cylinder, sector, head, sectors, data))
            return true;
        x86_reset_disk(disk->id);
    }
    return false;
}