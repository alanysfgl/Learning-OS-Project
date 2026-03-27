// fat.c
#include "fat.h"
#include "storage.h"
#include "logger.h"
#include "libc.h"
#include "vfs.h"

struct fat_bpb {
    u8int  jmp[3];
    u8int  oem[8];
    u16int bytes_per_sector;
    u8int  sectors_per_cluster;
    u16int reserved_sectors;
    u8int  num_fats;
    u16int root_entries;
    u16int total_sectors16;
    u8int  media;
    u16int fat_size16;
    u16int sectors_per_track;
    u16int num_heads;
    u32int hidden_sectors;
    u32int total_sectors32;
    u32int fat_size32;
    u16int ext_flags;
    u16int fs_version;
    u32int root_cluster;
    u16int fs_info;
    u16int backup_boot;
    u8int  reserved[12];
    u8int  drive_number;
    u8int  reserved1;
    u8int  boot_sig;
    u32int volume_id;
    u8int  volume_label[11];
    u8int  fs_type[8];
} __attribute__((packed));

struct fat_fs {
    struct storage_device *dev;
    u32int bytes_per_sector;
    u32int sectors_per_cluster;
    u32int reserved_sectors;
    u32int num_fats;
    u32int fat_size;
    u32int root_entries;
    u32int root_dir_sectors;
    u32int first_fat_sector;
    u32int first_data_sector;
    u32int root_dir_sector;
    u32int root_cluster;
    u32int fat_type; // 16 or 32
    u32int total_clusters;
};

static struct fat_fs fat;
static struct fat_file_info fat_files[32];
static u32int fat_file_count = 0;

static int read_sector(struct storage_device *dev, u32int lba, void *buf) {
    if (!dev || !dev->read) return 0;
    return dev->read(dev, lba, 1, buf) == 1;
}

static int write_sector(struct storage_device *dev, u32int lba, const void *buf) {
    if (!dev || !dev->write) return 0;
    return dev->write(dev, lba, 1, buf) == 1;
}

static u32int fat_cluster_to_lba(u32int cluster) {
    return fat.first_data_sector + (cluster - 2) * fat.sectors_per_cluster;
}

static u32int fat_read_fat_entry(u32int cluster) {
    u8int sector[512];
    if (fat.fat_type == 16) {
        u32int offset = cluster * 2;
        u32int sector_num = fat.first_fat_sector + (offset / fat.bytes_per_sector);
        u32int entry_offset = offset % fat.bytes_per_sector;
        if (!read_sector(fat.dev, sector_num, sector)) return 0;
        return *(u16int *)(sector + entry_offset);
    } else {
        u32int offset = cluster * 4;
        u32int sector_num = fat.first_fat_sector + (offset / fat.bytes_per_sector);
        u32int entry_offset = offset % fat.bytes_per_sector;
        if (!read_sector(fat.dev, sector_num, sector)) return 0;
        return (*(u32int *)(sector + entry_offset)) & 0x0FFFFFFF;
    }
}

static int fat_write_fat_entry(u32int cluster, u32int value) {
    if (fat.fat_type != 16) return 0;
    u8int sector[512];
    u32int offset = cluster * 2;
    u32int sector_num = fat.first_fat_sector + (offset / fat.bytes_per_sector);
    u32int entry_offset = offset % fat.bytes_per_sector;
    if (!read_sector(fat.dev, sector_num, sector)) return 0;
    *(u16int *)(sector + entry_offset) = (u16int)value;
    return write_sector(fat.dev, sector_num, sector);
}

static int fat_read_cluster_sector(u32int cluster, u32int sector_index, u8int *buffer) {
    if (sector_index >= fat.sectors_per_cluster) return 0;
    u32int lba = fat_cluster_to_lba(cluster) + sector_index;
    return read_sector(fat.dev, lba, buffer);
}

static void fat_add_file(const char *name, u32int size, u32int cluster) {
    if (fat_file_count >= (sizeof(fat_files) / sizeof(fat_files[0]))) return;
    struct fat_file_info *fi = &fat_files[fat_file_count++];
    u32int len = strlen(name);
    if (len > 12) len = 12;
    memcpy(fi->name, name, len);
    fi->name[len] = 0;
    fi->size = size;
    fi->first_cluster = cluster;
}

static void fat_parse_root_dir(u8int *sector, u32int entries_per_sector) {
    for (u32int i = 0; i < entries_per_sector; i++) {
        u8int *ent = sector + (i * 32);
        if (ent[0] == 0x00) return;
        if (ent[0] == 0xE5) continue;
        if (ent[11] == 0x0F) continue;
        if (ent[11] & 0x08) continue;
        char name[13];
        int pos = 0;
        for (int j = 0; j < 8; j++) {
            if (ent[j] == ' ') break;
            name[pos++] = ent[j];
        }
        if (ent[8] != ' ') {
            name[pos++] = '.';
            for (int j = 8; j < 11; j++) {
                if (ent[j] == ' ') break;
                name[pos++] = ent[j];
            }
        }
        name[pos] = 0;
        u16int cl_lo = *(u16int *)(ent + 26);
        u16int cl_hi = *(u16int *)(ent + 20);
        u32int cluster = ((u32int)cl_hi << 16) | cl_lo;
        u32int size = *(u32int *)(ent + 28);
        fat_add_file(name, size, cluster);
    }
}

void fat_init(void) {
    struct storage_device *dev = storage_get_default();
    if (!dev || !dev->read) {
        log_warn("FAT: no storage device with read support.\n");
        return;
    }

    u8int sector[512];
    if (!read_sector(dev, 0, sector)) {
        log_warn("FAT: boot sector read failed.\n");
        return;
    }

    struct fat_bpb *bpb = (struct fat_bpb *)sector;
    memset(&fat, 0, sizeof(fat));
    fat.dev = dev;
    fat.bytes_per_sector = bpb->bytes_per_sector;
    fat.sectors_per_cluster = bpb->sectors_per_cluster;
    fat.reserved_sectors = bpb->reserved_sectors;
    fat.num_fats = bpb->num_fats;
    fat.root_entries = bpb->root_entries;
    fat.fat_size = bpb->fat_size16 ? bpb->fat_size16 : bpb->fat_size32;
    fat.first_fat_sector = fat.reserved_sectors;
    fat.root_dir_sectors = ((fat.root_entries * 32) + (fat.bytes_per_sector - 1)) / fat.bytes_per_sector;
    fat.first_data_sector = fat.reserved_sectors + (fat.num_fats * fat.fat_size) + fat.root_dir_sectors;
    fat.root_dir_sector = fat.reserved_sectors + (fat.num_fats * fat.fat_size);
    fat.root_cluster = bpb->root_cluster;

    u32int total_sectors = bpb->total_sectors16 ? bpb->total_sectors16 : bpb->total_sectors32;
    u32int data_sectors = total_sectors - (fat.reserved_sectors + (fat.num_fats * fat.fat_size) + fat.root_dir_sectors);
    u32int total_clusters = data_sectors / fat.sectors_per_cluster;
    fat.total_clusters = total_clusters;

    if (total_clusters < 4085) {
        log_info("FAT12 detected (unsupported).\n");
        return;
    } else if (total_clusters < 65525) {
        fat.fat_type = 16;
        log_info("FAT16 detected (read-only).\n");
    } else {
        fat.fat_type = 32;
        log_info("FAT32 detected (read-only).\n");
    }

    fat_file_count = 0;
    u8int dir_sector[512];
    if (fat.fat_type == 16) {
        for (u32int i = 0; i < fat.root_dir_sectors; i++) {
            if (!read_sector(dev, fat.root_dir_sector + i, dir_sector)) {
                break;
            }
            fat_parse_root_dir(dir_sector, fat.bytes_per_sector / 32);
        }
    } else if (fat.fat_type == 32) {
        u32int cluster = fat.root_cluster;
        while (cluster >= 2 && cluster < 0x0FFFFFF8) {
            for (u32int s = 0; s < fat.sectors_per_cluster; s++) {
                if (!fat_read_cluster_sector(cluster, s, dir_sector)) {
                    break;
                }
                fat_parse_root_dir(dir_sector, fat.bytes_per_sector / 32);
            }
            cluster = fat_read_fat_entry(cluster);
        }
    }
    log_info("FAT root entries: %u\n", fat_file_count);
}

static void fat_refresh_root_cache(void) {
    fat_file_count = 0;
    u8int dir_sector[512];
    if (fat.fat_type == 16) {
        for (u32int i = 0; i < fat.root_dir_sectors; i++) {
            if (!read_sector(fat.dev, fat.root_dir_sector + i, dir_sector)) {
                break;
            }
            fat_parse_root_dir(dir_sector, fat.bytes_per_sector / 32);
        }
    } else if (fat.fat_type == 32) {
        u32int cluster = fat.root_cluster;
        while (cluster >= 2 && cluster < 0x0FFFFFF8) {
            for (u32int s = 0; s < fat.sectors_per_cluster; s++) {
                if (!fat_read_cluster_sector(cluster, s, dir_sector)) {
                    break;
                }
                fat_parse_root_dir(dir_sector, fat.bytes_per_sector / 32);
            }
            cluster = fat_read_fat_entry(cluster);
        }
    }
}

int fat_list_root(struct fat_file_info *out, u32int max) {
    if (!out || max == 0) return 0;
    u32int count = (fat_file_count < max) ? fat_file_count : max;
    for (u32int i = 0; i < count; i++) {
        out[i] = fat_files[i];
    }
    return (int)count;
}

int fat_read_file(const char *name, u8int *buffer, u32int max) {
    if (!name || !buffer || max == 0) return -1;
    struct fat_file_info *fi = 0;
    for (u32int i = 0; i < fat_file_count; i++) {
        if (strcmp(fat_files[i].name, name) == 0) {
            fi = &fat_files[i];
            break;
        }
    }
    if (!fi) return -1;

    u32int to_read = fi->size;
    if (to_read > max) to_read = max;
    u32int read = 0;
    u32int cluster = fi->first_cluster;
    u8int sector_buf[512];

    while (cluster >= 2 && cluster < 0x0FFFFFF8 && read < to_read) {
        for (u32int s = 0; s < fat.sectors_per_cluster && read < to_read; s++) {
            if (!fat_read_cluster_sector(cluster, s, sector_buf)) {
                break;
            }
            u32int chunk = fat.bytes_per_sector;
            if (read + chunk > to_read) chunk = to_read - read;
            memcpy(buffer + read, sector_buf, chunk);
            read += chunk;
        }
        cluster = fat_read_fat_entry(cluster);
        if (fat.fat_type == 16 && cluster >= 0xFFF8) break;
    }
    return (int)read;
}

static int fat_format_name(const char *name, u8int out[11]) {
    if (!name || !*name) return 0;
    for (int i = 0; i < 11; i++) out[i] = ' ';

    int i = 0;
    int j = 0;
    while (name[i] && name[i] != '.' && j < 8) {
        char c = name[i++];
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
        out[j++] = (u8int)c;
    }
    if (name[i] == '.') i++;
    j = 8;
    int k = 0;
    while (name[i] && k < 3) {
        char c = name[i++];
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
        out[j++] = (u8int)c;
        k++;
    }
    return 1;
}

static int fat_find_root_entry(const u8int name[11], u32int *out_sector, u32int *out_offset) {
    u8int sector[512];
    u32int free_sector = 0xFFFFFFFF;
    u32int free_offset = 0xFFFFFFFF;

    for (u32int i = 0; i < fat.root_dir_sectors; i++) {
        u32int lba = fat.root_dir_sector + i;
        if (!read_sector(fat.dev, lba, sector)) {
            break;
        }
        for (u32int e = 0; e < fat.bytes_per_sector / 32; e++) {
            u8int *ent = sector + (e * 32);
            if (ent[0] == 0x00 || ent[0] == 0xE5) {
                if (free_sector == 0xFFFFFFFF) {
                    free_sector = lba;
                    free_offset = e * 32;
                }
                if (ent[0] == 0x00) break;
                continue;
            }
            if (ent[11] == 0x0F) continue;
            if (memcmp(ent, name, 11) == 0) {
                if (out_sector) *out_sector = lba;
                if (out_offset) *out_offset = e * 32;
                return 1;
            }
        }
    }

    if (free_sector != 0xFFFFFFFF) {
        if (out_sector) *out_sector = free_sector;
        if (out_offset) *out_offset = free_offset;
        return 0;
    }
    return -1;
}

static void fat_free_chain(u32int start_cluster) {
    u32int cluster = start_cluster;
    while (cluster >= 2 && cluster < 0xFFF8) {
        u32int next = fat_read_fat_entry(cluster);
        fat_write_fat_entry(cluster, 0x0000);
        cluster = next;
    }
}

static u32int fat_find_free_run(u32int count) {
    u32int run = 0;
    u32int start = 2;
    for (u32int c = 2; c < fat.total_clusters; c++) {
        if (fat_read_fat_entry(c) == 0) {
            if (run == 0) start = c;
            run++;
            if (run >= count) return start;
        } else {
            run = 0;
        }
    }
    return 0;
}

int fat_write_file(const char *name, const u8int *buffer, u32int size) {
    if (!name || !buffer) return -1;
    if (!fat.dev || !fat.dev->write) return -1;
    if (fat.fat_type != 16) return -1;

    u8int fat_name[11];
    if (!fat_format_name(name, fat_name)) return -1;

    u32int sector = 0, offset = 0;
    int found = fat_find_root_entry(fat_name, &sector, &offset);
    if (found < 0) return -1;

    u8int dir_sector[512];
    if (!read_sector(fat.dev, sector, dir_sector)) return -1;
    u8int *ent = dir_sector + offset;

    u32int old_cluster = *(u16int *)(ent + 26);
    if (found == 1 && old_cluster >= 2) {
        fat_free_chain(old_cluster);
    }

    u32int cluster_size = fat.bytes_per_sector * fat.sectors_per_cluster;
    u32int clusters_needed = (size + cluster_size - 1) / cluster_size;
    u32int first_cluster = 0;

    if (size > 0) {
        first_cluster = fat_find_free_run(clusters_needed);
        if (first_cluster == 0) return -1;
        for (u32int i = 0; i < clusters_needed; i++) {
            u32int cur = first_cluster + i;
            u32int val = (i + 1 == clusters_needed) ? 0xFFFF : (cur + 1);
            if (!fat_write_fat_entry(cur, val)) return -1;
        }

        u32int remaining = size;
        u32int cluster = first_cluster;
        u8int sector_buf[512];
        while (remaining > 0 && cluster >= 2 && cluster < 0xFFF8) {
            for (u32int s = 0; s < fat.sectors_per_cluster; s++) {
                u32int lba = fat_cluster_to_lba(cluster) + s;
                u32int chunk = fat.bytes_per_sector;
                if (remaining < chunk) {
                    memset(sector_buf, 0, sizeof(sector_buf));
                    memcpy(sector_buf, buffer, remaining);
                    if (!write_sector(fat.dev, lba, sector_buf)) return -1;
                    remaining = 0;
                } else {
                    if (!write_sector(fat.dev, lba, buffer)) return -1;
                    buffer += chunk;
                    remaining -= chunk;
                }
                if (remaining == 0) break;
            }
            cluster = fat_read_fat_entry(cluster);
        }
    }

    memcpy(ent, fat_name, 11);
    ent[11] = 0x20; // archive
    *(u16int *)(ent + 20) = 0;
    *(u16int *)(ent + 26) = (u16int)first_cluster;
    *(u32int *)(ent + 28) = size;
    if (!write_sector(fat.dev, sector, dir_sector)) return -1;

    fat_refresh_root_cache();
    return 0;
}
