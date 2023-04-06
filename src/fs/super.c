#include "../include/ynix/fs.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/device.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/buffer.h"
#include "../include/ynix/string.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define SUPER_NR 16
// 允许系统挂载最多16个文件系统
static super_block_t super_table[SUPER_NR];
static super_block_t* root; //根文件系统超级块

static super_block_t* get_free_super() {
    for(size_t i = 0; i < SUPER_NR; i++) {
        if(EOF == super_table[i].dev) {
            return &super_table[i];
        }
    }
    panic("no more super block!!!");
}

super_block_t* get_super(dev_t dev) {
    for(size_t i = 0; i < SUPER_NR; i++) {
        if(dev == super_table[i].dev) {
            return &super_table[i];
        }
    }
    return NULL;
}

super_block_t* read_super(dev_t dev) {
    super_block_t* sb = get_super(dev);
    if(sb) {
        return sb;
    }
    
    LOGK("Reading super block of device %d\n", dev);

    sb = get_free_super();
    buffer_t* buf = bread(dev, 1);
    sb->buf;
    sb->desc = (super_desc_t*)buf->data;
    sb->dev = dev;
    assert(MINIX1_MAGIC == sb->desc->magic);

    memset(sb->imaps, 0, sizeof(sb->imaps));
    memset(sb->zmaps, 0, sizeof(sb->zmaps));
    // 读取inode位图
    int idx = 2;
    for(size_t i = 0; i < sb->desc->imap_blocks; i++) {
        assert(i < IMAP_NR);
        if((sb->imaps[i] = bread(dev, idx))) {
            idx ++;
        } else {
            break;
        }
    }
    // 读取块位图
    for(size_t i = 0; i < sb->desc->zmap_blocks; i++) {
        assert(i < ZMAP_NR);
        if((sb->zmaps[i] = bread(dev, idx))) {
            idx  ++;
        } else {
            break;
        }
    }
    return sb;
}

// 挂载根文件系统
static void mount_root() {
    LOGK("Mount root file system...\n");
    // 假设主硬盘第一个分区是根文件系统
    device_t* device = device_find(DEV_IDE_PART, 0);
    assert(device);

    root = read_super(device->dev);
    
    // 初始化根目录 inode
    root->iroot = iget(device->dev, 1);  // 获得根目录 inode
    root->imount = iget(device->dev, 1); // 根目录挂载 inode

    idx_t idx = 0;
    inode_t *inode = iget(device->dev, 1);

    // 直接块
    idx = bmap(inode, 3, true);

    // 一级间接块
    idx = bmap(inode, 7 + 7, true);

    // 二级间接块
    idx = bmap(inode, 7 + 512 * 3 + 510, true);

    iput(inode);
}   

void super_init() {
    for(size_t i = 0; i < SUPER_NR; i++) {
        super_block_t* sb = &super_table[i];
        sb->buf = NULL;
        sb->desc = NULL;
        sb->dev = EOF;
        sb->imount = NULL;
        sb->iroot = NULL;
        list_init(&sb->inode_list);
    }
    mount_root();
}