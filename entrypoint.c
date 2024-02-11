#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "entrypoint.h"
#include "http_client.h"

MODULE_AUTHOR("kaladin");
MODULE_DESCRIPTION("Networkfs kernel module");
MODULE_LICENSE("GPL");

struct file_system_type networkfs_fs_type = {
    .name = "networkfs",
    .mount = networkfs_mount,
    .kill_sb = networkfs_kill_sb};

struct file_operations networkfs_dir_ops =
    {
        .iterate = networkfs_iterate,
};

struct inode_operations networkfs_inode_ops =
    {
        .lookup = networkfs_lookup,
        .create = networkfs_create,
        .unlink = networkfs_unlink,
        .mkdir = networkfs_mkdir,
        .rmdir = networkfs_rmdir};

void networkfs_kill_sb(struct super_block *sb)
{
    tcp_client_disconnect();

    printk(KERN_INFO "networkfs super block is destroyed. Unmount successfully.\n");
}

int networkfs_create(struct user_namespace *ns, struct inode *parent_inode,
                     struct dentry *child_dentry, umode_t mode, bool b)
{
    ino_t new_inode;
    ino_t root;
    struct inode *inode;
    const char *name;

    name = child_dentry->d_name.name;
    root = parent_inode->i_ino;

    inode = networkfs_get_inode(parent_inode->i_sb, NULL, S_IFREG, create(name));
    d_add(child_dentry, inode);
    return 0;
}

struct dentry *networkfs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flag)
{
    ino_t root;
    struct inode *inode;

    const char *name = child_dentry->d_name.name;
    root = parent_inode->i_ino;

    inode = networkfs_get_inode(parent_inode->i_sb, NULL, S_IFREG, lookup(name));
    d_add(child_dentry, inode);
    return child_dentry;
}

int networkfs_iterate(struct file *filp, struct dir_context *ctx)
{
    char current_filename[256];
    struct dentry *dentry;
    struct inode *inode;
    unsigned long offset;
    unsigned char current_ftype;
    int stored;
    ino_t ino;
    ino_t current_dino;

    dentry = filp->f_path.dentry;
    inode = dentry->d_inode;
    offset = filp->f_pos;
    stored = 0;
    ino = inode->i_ino;

    printk("request for %ld\n", ino);
    char entries[4096];
    memset(&entries, 0, 100);

    list(entries);
    printk("got %s\n", entries);
    int i = 0;

    while (true)
    {
        if (offset == 0)
        {
            strcpy(current_filename, ".");
            current_ftype = DT_DIR;
            current_dino = ino;
        }
        else if (offset == 1)
        {
            strcpy(current_filename, "..");
            current_ftype = DT_DIR;
            current_dino = dentry->d_parent->d_inode->i_ino;
        }
        else if (i + 1 < strlen(entries))
        {
            current_filename[0] = entries[i];
            current_filename[1] = '\0';
            current_ftype = DT_REG;
            printk("at %s\n", current_filename);
            printk("num %d\n", i);
            current_dino = 1002 + i;
            i++;
        }
        else
        {
            printk("ended\n");
            return stored;
        }
        dir_emit(ctx, current_filename, strlen(current_filename), current_dino, current_ftype);

        stored++;
        offset++;
        ctx->pos = offset;
    }
    return stored;
}

struct inode *networkfs_get_inode(struct super_block *sb,
                                  const struct inode *dir, umode_t mode,
                                  int i_ino)
{
    struct inode *inode;

    inode = new_inode(sb);
    if (inode != NULL)
    {
        inode->i_ino = i_ino;
        inode->i_op = &networkfs_inode_ops;
        if (mode & S_IFDIR)
        {
            inode->i_fop = &networkfs_dir_ops;
        }
        inode_init_owner(&init_user_ns, inode, dir, mode | S_IRWXUGO);
    }
    return inode;
}

int networkfs_fill_super(struct super_block *sb, void *data,
                         int silent)
{
    struct inode *inode;
    inode = networkfs_get_inode(sb, NULL, S_IFDIR, 1000);
    sb->s_root = d_make_root(inode);
    if (sb->s_root == NULL)
    {
        return -ENOMEM;
    }
    printk(KERN_INFO "return 0\n");
    return 0;
}

struct dentry *networkfs_mount(struct file_system_type
                                   *fs_type,
                               int flags, const char *token, void *data)
{
    tcp_client_connect();
    // send_holla();

    struct dentry *ret;
    ret = mount_nodev(fs_type, flags, data, networkfs_fill_super);
    if (ret == NULL)
    {
        printk(KERN_ERR "Can't mount file system\n");
    }
    else
    {
        printk(KERN_INFO "Mounted successfuly\n");
    }
    return ret;
}

static int __init custom_init(void)
{
    register_filesystem(&networkfs_fs_type);
    print_hell_msg();

    return 0;
}
static void __exit custom_exit(void)
{
    unregister_filesystem(&networkfs_fs_type);
    printk(KERN_INFO "Goodbye my friend, I shall miss you dearly...\n");
}

module_exit(custom_exit);
module_init(custom_init);
