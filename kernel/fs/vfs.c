#include "vfs.h"
#include "../core/string.h"


#define MAX_FS_DRIVERS 8
static struct fs_driver* fs_drivers[MAX_FS_DRIVERS];
static int fs_driver_count = 0;

static vfs_node_t vfs_root = {
    .name = "/",
    .type = VFS_TYPE_DIR,
    .parent = 0,
    .children = 0,
    .next = 0,
    .size = 0,
    .fs_data = 0,
    .fs = 0,
    .blockdev = 0
};

void vfs_register_fs(struct fs_driver* fs) {
    if (fs_driver_count < MAX_FS_DRIVERS) {
        fs_drivers[fs_driver_count++] = fs;
    }
}


// Find FS driver by name
static struct fs_driver* find_fs_driver(const char* name) {
    for (int i = 0; i < fs_driver_count; ++i) {
        if (strcmp(fs_drivers[i]->name, name) == 0) return fs_drivers[i];
    }
    return 0;
}

// Find node by path (very simple, only supports root and direct children)
static vfs_node_t* vfs_find_node(const char* path) {
    if (!path || strcmp(path, "/") == 0) return &vfs_root;
    if (path[0] == '/') path++;
    vfs_node_t* child = vfs_root.children;
    while (child) {
        if (strcmp(child->name, path) == 0) return child;
        child = child->next;
    }
    return 0;
}

int vfs_mount(const char* devname, const char* fstype, const char* mountpoint) {
    blockdev_t* dev = blockdev_find(devname);
    if (!dev) return -1;
    struct fs_driver* fs = find_fs_driver(fstype);
    if (!fs) return -2;
    if (fs->probe && !fs->probe(dev)) return -3;

    // Create mountpoint node (only supports mounting at root for now)
    vfs_node_t* mp = (vfs_node_t*)malloc(sizeof(vfs_node_t));
    memset(mp, 0, sizeof(vfs_node_t));
    strncpy(mp->name, mountpoint[0] == '/' ? mountpoint + 1 : mountpoint, 63);
    mp->type = VFS_TYPE_DIR;
    mp->parent = &vfs_root;
    mp->fs = fs;
    mp->blockdev = dev;
    mp->next = vfs_root.children;
    vfs_root.children = mp;

    if (fs->mount) fs->mount(dev, mp);
    return 0;
}


vfs_node_t* vfs_open(const char* path) {
    return vfs_find_node(path);
}


int vfs_read(vfs_node_t* node, void* buf, size_t size, size_t offset) {
(void)node; (void)buf; (void)size; (void)offset;
    // For demo, just return 0; real FS would implement this
    return 0;
}
