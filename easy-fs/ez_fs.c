#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/mount.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/namei.h>
// #include <asm/current.h>

#define EZFS_MAGIC 0x64668735

static struct vfsmount *ezfs_mount;
static int ezfs_mount_count;

static struct inode * ezfs_get_inode(struct super_block *sb, int mode, dev_t dev)
{
    struct inode *inode = new_inode(sb);
    // const struct cred * cred = current_cred();

    if (inode) {
        inode->i_mode = mode;
        //inode->i_uid = cred->fsuid;
        inode->i_uid = current_fsuid();
        //inode->i_gid = cred->fsgid;
        inode->i_gid = current_fsgid();

        // inode->i_blksize = PAGE_CACHE_SIZE;
        inode->i_blocks = 0;
        inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;

        switch (mode & S_IFMT){
            default:
                init_special_inode(inode, mode, dev);
                break;
            case S_IFREG:
                printk("create a file \n");
                break;
            case S_IFDIR:
                inode->i_op = &simple_dir_inode_operations;
                inode->i_fop = &simple_dir_operations;
                printk("create a dir file \n");
                inode->i_nlink++;
                break;
        }
    }
    return inode;
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

static int ezfs_get_sb(struct file_system_type *fs_type,
                        int flags, const char *dev_name, void *data, struct vfsmount *mnt)
{
    return get_sb_single(fs_type, flags, data, ezfs_fill_super, mnt);
}

static int ezfs_create_by_name(const char *name, mode_t mode,
                    struct dentry *parent,
                    struct dentry **dentry)
{
    int error = 0;

    if (!parent) {
        if (ezfs_mount && ezfs_mount->mnt_sb) {
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

    mutex_unlock(&parent->d_inode->i_mutex);

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
            //dentry->d_inode->u.generic_ip = data;
            dentry->d_inode->i_generation = (__u32) data;
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

int __init ez_fs_init(void){
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

void __exit ez_fs_exit(void){
    simple_release_fs(&ezfs_mount, &ezfs_mount_count);
    unregister_filesystem(&ez_fs_type);
}

module_init(ez_fs_init);
module_exit(ez_fs_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("EZ FS");
MODULE_VERSION("Ver 0.1");

