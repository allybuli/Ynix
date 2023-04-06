#include "../include/ynix/fs.h"
#include "../include/ynix/assert.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define INODE_NR 64

static inode_t inode_table[INODE_NR];

// 获取根inode
inode_t* get_root_inode() {
    return inode_table;
}

static inode_t* get_free_inode() {
    for(size_t i = 0; i < INODE_NR; i++) {
        inode_t* inode = &inode_table[i];
        if(EOF == inode->dev) {
            return inode;
        }
    }
    panic("no more inode!!!");
}

static void put_free_inode(inode_t* inode) {
    assert(inode != inode_table); // 不能是根inode
    assert(inode->count == 0);
    inode->dev = EOF;
}

// 从已有 inode 中查找编号为 nr 的 inode, 超级块的inode链表中
static inode_t* find_inode(dev_t dev, idx_t nr) {
    super_block_t* sb = get_super(dev);
    assert(sb);
    list_t* list = &sb->inode_list;
    for(list_node_t* node = list->head.next; node != &list->tail; node = node->next) {
        inode_t* inode = element_entry(inode_t, node, node);
        if(inode->nr == nr) {
            return inode;
        }
    }
    return NULL;
}

// 计算 inode nr 对应的块号
// 块号指的是什么？
static inline idx_t inode_block(super_block_t *sb, idx_t nr) {
    // inode 编号 从 1 开始
    return 2 + sb->desc->imap_blocks + sb->desc->zmap_blocks + (nr - 1) / BLOCK_INODES;
}

// 获得设备 dev 的 nr inode
inode_t *iget(dev_t dev, idx_t nr) {
    inode_t* inode = find_inode(dev, nr);
    if(inode) {
        inode->count ++;
        return inode;
    }

    super_block_t* sb = get_super(dev);
    assert(sb);
    assert(nr <= sb->desc->inodes);

    inode = get_free_inode();
    inode->dev = dev;
    inode->nr = nr;
    inode->count = 1;

    // 加入超级块inode链表，统一管理
    list_push(&sb->inode_list, &inode->node);

    idx_t block = inode_block(sb, inode->nr);
    buffer_t* buf = bread(inode->dev, block);
    inode->buf = buf;

    // 将缓冲视为一个 inode 描述符数组，获取对应的指针；
    inode->desc = &((inode_desc_t *)buf->data)[(inode->nr - 1) % BLOCK_INODES];

    return inode;
}

// 释放 inode
void iput(inode_t *inode) {
    if(!inode) {
        return;
    }
    inode->count --;
    if(inode->count) {
        return;
    }
    // 引用计数已为0
    // 释放缓冲块，（缓冲块被放回free链表中）
    brelse(inode->buf);

    // 从超级块链表中移除
    list_remove(&inode->node);

    // 释放 inode 内存(将该inode设为未使用)
    put_free_inode(inode);
}

void inode_init() {
    for(size_t i = 0; i < INODE_NR; i++) {
        inode_t* inode = &inode_table[i];
        inode->dev = EOF;
    }
}