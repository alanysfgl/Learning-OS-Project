// storage.c
#include "storage.h"
#include "logger.h"

static struct storage_device *default_dev = 0;

int storage_register(struct storage_device *dev) {
    if (!dev || !dev->read) {
        return -1;
    }
    if (!default_dev) {
        default_dev = dev;
        log_info("Storage device registered: %s\n", dev->name ? dev->name : "noname");
    }
    return 0;
}

struct storage_device *storage_get_default(void) {
    return default_dev;
}
