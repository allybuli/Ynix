#include "../include/ynix/fs.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/stat.h"
#include "../include/ynix/string.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

// 判断文件名是否相等?
// 判断entry_name是否是name的前缀
bool match_name(const char *name, const char *entry_name, char **next) {
    char *lhs = (char *)name;
    char *rhs = (char *)entry_name;
    while (*lhs == *rhs && *lhs != EOS && *rhs != EOS)
    {
        lhs++;
        rhs++;
    }
    if (*rhs)
        return false;
    if (*lhs && !IS_SEPARATOR(*lhs))
        return false;
    if (IS_SEPARATOR(*lhs))
        lhs++;
    *next = lhs;
    return true;
}

// 获取 dir 目录下的 name 目录 所在的 dentry_t 和 buffer_t
buffer_t *find_entry(inode_t **dir, const char *name, char **next, dentry_t **result) {
    assert(ISDIR((*dir)->desc->mode));

    u32 entries = (*dir)->desc->size / sizeof(dentry_t);

    idx_t i = 0;
    idx_t block = 0;
    idx_t nr = 0;
    dentry_t* entry = NULL;
    buffer_t* buf = NULL;
    
    // 性能有待优化，存在读取重复文件块到buf
    for(; i < entries; i++, entry++) {
        // !buf特判是因为一开始entry为NULL
        if(!buf || (u32)entry >= (u32)buf->data + BLOCK_SIZE) {
            brelse(buf);
            block = bmap((*dir), i / BLOCK_DENTRIES, false);
            assert(block);

            buf = bread((*dir)->dev, block);
            entry = (dentry_t*)buf->data;
        }
        if(match_name(name, entry->name, next)) {
            *result = entry;
            return buf;
        }
    }
    brelse(buf);
    return NULL;
}

// 在 dir 目录中添加 name 目录项
// 在dir目录中找到一个nr为0的dentry_t项，作为name目录项
buffer_t *add_entry(inode_t *dir, const char *name, dentry_t **result) {
    char *next = NULL;

    buffer_t *buf = find_entry(&dir, name, &next, result);
    if (buf)
    {
        return buf;
    }

    // name 中不能有分隔符
    for (size_t i = 0; i < NAME_LEN && name[i]; i++)
    {
        assert(!IS_SEPARATOR(name[i]));
    }

    // super_block_t *sb = get_super(dir->dev);
    // assert(sb);

    idx_t i = 0;
    idx_t block = 0;
    dentry_t *entry;

    for (; true; i++, entry++)
    {
        if (!buf || (u32)entry >= (u32)buf->data + BLOCK_SIZE)
        {
            brelse(buf);
            block = bmap(dir, i / BLOCK_DENTRIES, true);
            assert(block);

            buf = bread(dir->dev, block);
            entry = (dentry_t *)buf->data;
        }
        if (i * sizeof(dentry_t) >= dir->desc->size)
        {
            entry->nr = 0;
            dir->desc->size = (i + 1) * sizeof(dentry_t);
            dir->buf->dirty = true;
        }
        if (entry->nr)
            continue;

        strncpy(entry->name, name, NAME_LEN);
        buf->dirty = true;
        // dir->desc->mtime = time();
        dir->buf->dirty = true;
        *result = entry;
        return buf;
    };
}