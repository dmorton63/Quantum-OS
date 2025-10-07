#include "../../core/blockdev.h"
#include "../core/string.h"
#include "../core/stdtools.h"


#define RAMDISK_SIZE (128 * 1024) // 128 KiB
#define RAMDISK_BLOCK_SIZE 512
static uint8_t ramdisk_data[RAMDISK_SIZE];

static int ramdisk_read(blockdev_t* dev, uint64_t lba, void* buf, size_t count) {
    if (!dev || !buf) return -1;
    if ((lba + count) * RAMDISK_BLOCK_SIZE > RAMDISK_SIZE) return -1;
    memcpy(buf, &ramdisk_data[lba * RAMDISK_BLOCK_SIZE], count * RAMDISK_BLOCK_SIZE);
    return 0;
}

static int ramdisk_write(blockdev_t* dev, uint64_t lba, const void* buf, size_t count) {
    if (!dev || !buf) return -1;
    if ((lba + count) * RAMDISK_BLOCK_SIZE > RAMDISK_SIZE) return -1;
    memcpy(&ramdisk_data[lba * RAMDISK_BLOCK_SIZE], buf, count * RAMDISK_BLOCK_SIZE);
    return 0;
}

static blockdev_t ramdisk_dev = {
    .type = BLOCKDEV_TYPE_RAMDISK,
    .name = "ram0",
    .num_blocks = RAMDISK_SIZE / RAMDISK_BLOCK_SIZE,
    .block_size = RAMDISK_BLOCK_SIZE,
    .driver_data = 0,
    .read = ramdisk_read,
    .write = ramdisk_write,
    .next = 0
};

void ramdisk_init(void) {
    memset(ramdisk_data, 0, sizeof(ramdisk_data));
    blockdev_register(&ramdisk_dev);
}
