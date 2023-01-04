#include "functions.h"

device_t::device(uint16_t b, uint16_t ctl, uint8_t ms, char* name) {
    base = b;
    dev_ctl = ctl;
    master_slave_bit = ms;
    type = DEV_UNINTIALIZED;
    dev_name = name;
}

void reset_device(device_t* dev) {
    uint8_t dctl = inb(dev->dev_ctl);
    outb(dev->dev_ctl, dctl | 4);
    dctl &= ~4;
    outb(dev->dev_ctl, dctl);
}

void select(device_t* dev) {
    outb(dev->base + DEV_OFF_DSEL, dev->master_slave_bit);
    inb(dev->dev_ctl);
    inb(dev->dev_ctl);
    inb(dev->dev_ctl);
    inb(dev->dev_ctl);
}

void select(device_t* dev, uint8_t flags) {
    outb(dev->base + DEV_OFF_DSEL, dev->master_slave_bit | flags);
    inb(dev->dev_ctl);
    inb(dev->dev_ctl);
    inb(dev->dev_ctl);
    inb(dev->dev_ctl);
}

dev_type determine_dev_type(device_t* dev,dev_type def) {
    reset_device(dev);
    select(dev);
    reset_device(dev);
    select(dev);
    uint8_t cl = inb(dev->base + DEV_OFF_CL);
    uint8_t ch = inb(dev->base + DEV_OFF_CH);
    if (cl==0x14 && ch==0xEB) return DEV_PATAPI;
    if (cl==0 && ch==0) return DEV_PATA;
    if (cl==0x69 && ch==0x96) return DEV_SATAPI;
    if (cl==0x3c && ch==0xc3) return DEV_SATA;
    return def;
}

bool init_disk(device_t* dev) {
    if (inb(dev->base + DEV_OFF_DATA) == 0xFF) {
        dev->type = determine_dev_type(dev,DEV_DISCONNECTED);
        return dev->type != DEV_DISCONNECTED;
    }
    dev->type = determine_dev_type(dev,DEV_UNKNOWN);
    return true;
}

void wait_disk_ready(device_t* dev) {
    uint8_t stat = inb(dev->dev_ctl);
    while (stat & (1 << 7)) stat = inb(dev->dev_ctl);
    while (!(stat & (1 << 3))) stat = inb(dev->dev_ctl);
}

void read_disk(device_t* dev, uint8_t* buffer, uint32_t lba, uint8_t sectors) {
    select(dev, 0xE0 | ((lba >> 24)&0x0F));
    outb(dev->base + DEV_OFF_SC, sectors);
    outb(dev->base + DEV_OFF_SN, lba);
    outb(dev->base + DEV_OFF_CL, lba >> 8);
    outb(dev->base + DEV_OFF_CH, lba >> 16);
    outb(dev->base + DEV_OFF_CMD, 0x20);
    print_string("Wait disk\n\r");
    wait_disk_ready(dev);
    uint16_t* buffer16 = (uint16_t*)buffer;
    for (uint16_t i = 0; i < sectors; i++) {
        wait_disk_ready(dev);
        for (uint16_t j = 0; j < 256; j++) {
            *buffer16 = inw(dev->base + DEV_OFF_DATA);
            buffer16++;
            print_string("Read byte\n\r");
        }
    }
}

bool disk_init(device_t *disk, uint8_t drive_number) {
   return init_disk(disk);
}

bool disk_read(device_t *disk, uint32_t lba, uint8_t sectors, void *data) {
    print_string("Reading disk\n\r");
    read_disk(disk,(uint8_t*)data,lba,sectors);
    print_string("Got sector!\n\r");
    return true;
}