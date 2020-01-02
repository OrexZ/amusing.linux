#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/mount.h>
#include <linux/namei.h>

#define EZFS_MAGIC 0x64668735

static struct vfsmount *ezfs_mount;
static int ezfs_mount_count;

static struct inode * ezfs_get_inode(struct super_block *sb, int mode, dev_t dev)
{
    struct inode *inode = new_inode(sb);

    if (inode) {
        inode->i_mode = mode;
        inode->i_uid = current->fsuid;
        inode->i_gid = current->fsgid;

    }
}

static int ezfs_mknod(struct inode *dir, struct dentry *dentry,
                    int mode, dev_t dev)
{
    struct inode *inode;
    int error = -EPERM;

    if (dentry->d_inode)
        return -EEXIST;

    inode = ezfs_get_inode(dir->i_sb, mode, dev);
    if (!inode){
        d_instantiate(dentry, inode);
        dget(dentry);
        error = 0;
    }
    return error;
}

static int ezfs_mkdir(struct inode *dir, struct dentry *dentry, int mode)
{
    int res;

    res = ezfs_mknod(dir, dentry, mode | S_IFDIR, 0);
    if (!res)
        dir->i_nlink++;
    return res;
}

static int ezfs_create(struct inode *dir, struct dentry *dentry, int mode)
{
    return ezfs_mknod(dir, dentry, mode | S_IFREG, 0);
}


static int ezfs_fill_super(struct super_block *sb, void *data, int silent)
{
    static struct tree_descr debug_files[] = {{""}};
    return simple_fill_super(sb, EZFS_MAGIC, debug_files);
}

static struct super_block *ezfs_get_sb(struct file_system_type *fs_type,
                        int flags, const char *dev_name, void *data)
{
    return get_sb_single(fs_type, flags, data, ezfs_fill_super);
}

static int ezfs_create_by_name(const char *name, mode_t mode,
                    struct dentry *parent,
                    struct dentry **dentry)
{
    int error = 0;

    if (!parent) {
        if (ezfs_mount & ezfs_mount->mnt_sb) {
            parent = ezfs_mount->mnt_sb->s_root;
        }
    }

    if (!parent) {
        printk("Ah, can not find a parent!\n");
    }

    *dentry = NULL;
    mutex_lock(&parent->d_inode->i_mutex);
    *dentry = lookup_one_len(name, parent, strlen(name));
    if (!IS_ERR(dentry)) {
        if ((mode & S_IFMT) == S_IFDIR)
            error = ezfs_mkdir(parent->d_inode, *dentry, mode);
        else
            error = ezfs_create(parent->d_inode, *dentry, mode);
    }
    else
        error = PTR_ERR(dentry);

    mutex_unlock(&parent->d_iode->i_mutex);

    return error;
}

struct dentry * ezfs_create_file(const char *name, mode_t mode,
                            struct dentry *parent, void *data,
                            struct file_operations *fops)
{
    struct dentry *dentry = NULL;
    int error;

    printk("ezfs: creating file '%s'\n", name);

    error = ezfs_create_by_name(name, mode, parent, &dentry);
    if (error) {
        dentry = NULL;
        goto exit;
    }

    if (dentry->d_inode){
        if (data)
            dentry->d_inode->u.generic_ip = data;
        if (fops)
            dentry->d_inode->i_fop = fops;
    }

exit:
    return dentry;
}

struct dentry * ezfs_create_dir(const char *name, struct dentry *parent)
{
    return ezfs_create_file(name,
            S_IFDIR | S_IRWXU | S_IRUGO | S_IXUGO,
            parent, NULL, NULL);
}

static struct file_system_type ez_fs_type = {
    .owner = THIS_MODULE,
    .name  = "ezfs",
    .get_sb = ezfs_get_sb,
    .kill_sb = kill_litter_super,
};

int __init ez_fs_init(){
    int retval;
    struct dentry *pslot;

    retval = register_filesystem(&ez_fs_type);

    if (!retval){
        ezfs_mount = kern_mount(&ez_fs_type);
        if (IS_ERR(ezfs_mount)){
                printk(KERN_ERR "ezfs: could not mount!\n");
                unregister_filesystem(&ez_fs_type);
                return retval;
        }
    }

    pslot = ezfs_create_dir("king star", NULL);
    ezfs_create_file("aaa", S_IFREG | S_IRUGO, pslot, NULL, NULL);
    ezfs_create_file("bbb", S_IFREG | S_IRUGO, pslot, NULL, NULL);
    ezfs_create_file("ccc", S_IFREG | S_IRUGO, pslot, NULL, NULL);

    pslot = ezfs_create_dir("queen star", NULL);
    ezfs_create_file("nnn", S_IFREG | S_IRUGO, pslot, NULL, NULL);
    ezfs_create_file("mmm", S_IFREG | S_IRUGO, pslot, NULL, NULL);
    ezfs_create_file("lll", S_IFREG | S_IRUGO, pslot, NULL, NULL);

    return retval;
}

void __exit ez_fs_exit(){}

module_init(ez_fs_init);
module_exit(ez_fs_exit);
