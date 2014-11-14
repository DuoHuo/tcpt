#ifndef _TCPTFS_H_
#define _TCPTFS_H_

#include <linux/dcache.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/seq_file.h>
#include <linux/statfs.h>
#include <linux/fs_stack.h>
#include <linux/magic.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/scatterlist.h>
#include <linux/highmem.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/crypto.h>

/* the file system name */
#define TCPTFS_NAME "tcptfs"

/* tcptfs root inode number */
#define TCPTFS_ROOT_INO     1

/* useful for tracking code reachability */
#define UDBG printk(KERN_DEFAULT "DBG:%s:%s:%d\n", __FILE__, __func__, __LINE__)

/* operations vectors defined in specific files */
extern const struct file_operations tcptfs_main_fops;
extern const struct file_operations tcptfs_dir_fops;
extern const struct inode_operations tcptfs_main_iops;
extern const struct inode_operations tcptfs_dir_iops;
extern const struct inode_operations tcptfs_symlink_iops;
extern const struct super_operations tcptfs_sops;
extern const struct dentry_operations tcptfs_dops;
extern const struct address_space_operations tcptfs_aops, tcptfs_dummy_aops;
extern const struct vm_operations_struct tcptfs_vm_ops;

extern int tcptfs_init_inode_cache(void);
extern void tcptfs_destroy_inode_cache(void);
extern int tcptfs_init_dentry_cache(void);
extern void tcptfs_destroy_dentry_cache(void);
extern int new_dentry_private_data(struct dentry *dentry);
extern void free_dentry_private_data(struct dentry *dentry);
extern struct dentry *tcptfs_lookup(struct inode *dir, struct dentry *dentry,
				    struct nameidata *nd);
extern int tcptfs_interpose(struct dentry *dentry, struct super_block *sb,
			    struct path *lower_path);

/* file private data */
struct tcptfs_file_info {
	struct file *lower_file;
	const struct vm_operations_struct *lower_vm_ops;
};

/* tcptfs inode data in memory */
struct tcptfs_inode_info {
	struct inode *lower_inode;
	struct inode vfs_inode;
};

/* tcptfs dentry data in memory */
struct tcptfs_dentry_info {
	spinlock_t lock;	/* protects lower_path */
	struct path lower_path;
};

/* tcptfs super-block data in memory */
struct tcptfs_sb_info {
	struct super_block *lower_sb;
};

/*
 * inode to private data
 *
 * Since we use containers and the struct inode is _inside_ the
 * tcptfs_inode_info structure, TCPTFS_I will always (given a non-NULL
 * inode pointer), return a valid non-NULL pointer.
 */
static inline struct tcptfs_inode_info *TCPTFS_I(const struct inode *inode)
{
	return container_of(inode, struct tcptfs_inode_info, vfs_inode);
}

/* dentry to private data */
#define TCPTFS_D(dent) ((struct tcptfs_dentry_info *)(dent)->d_fsdata)

/* superblock to private data */
#define TCPTFS_SB(super) ((struct tcptfs_sb_info *)(super)->s_fs_info)

/* file to private Data */
#define TCPTFS_F(file) ((struct tcptfs_file_info *)((file)->private_data))

/* file to lower file */
static inline struct file *tcptfs_lower_file(const struct file *f)
{
	return TCPTFS_F(f)->lower_file;
}

static inline void tcptfs_set_lower_file(struct file *f, struct file *val)
{
	TCPTFS_F(f)->lower_file = val;
}

/* inode to lower inode. */
static inline struct inode *tcptfs_lower_inode(const struct inode *i)
{
	return TCPTFS_I(i)->lower_inode;
}

static inline void tcptfs_set_lower_inode(struct inode *i, struct inode *val)
{
	TCPTFS_I(i)->lower_inode = val;
}

/* superblock to lower superblock */
static inline struct super_block *tcptfs_lower_super(
	const struct super_block *sb)
{
	return TCPTFS_SB(sb)->lower_sb;
}

static inline void tcptfs_set_lower_super(struct super_block *sb,
					  struct super_block *val)
{
	TCPTFS_SB(sb)->lower_sb = val;
}

/* path based (dentry/mnt) macros */
static inline void pathcpy(struct path *dst, const struct path *src)
{
	dst->dentry = src->dentry;
	dst->mnt = src->mnt;
}
/* Returns struct path.  Caller must path_put it. */
static inline void tcptfs_get_lower_path(const struct dentry *dent,
					 struct path *lower_path)
{
	spin_lock(&TCPTFS_D(dent)->lock);
	pathcpy(lower_path, &TCPTFS_D(dent)->lower_path);
	path_get(lower_path);
	spin_unlock(&TCPTFS_D(dent)->lock);
	return;
}
static inline void tcptfs_put_lower_path(const struct dentry *dent,
					 struct path *lower_path)
{
	path_put(lower_path);
	return;
}
static inline void tcptfs_set_lower_path(const struct dentry *dent,
					 struct path *lower_path)
{
	spin_lock(&TCPTFS_D(dent)->lock);
	pathcpy(&TCPTFS_D(dent)->lower_path, lower_path);
	spin_unlock(&TCPTFS_D(dent)->lock);
	return;
}
static inline void tcptfs_reset_lower_path(const struct dentry *dent)
{
	spin_lock(&TCPTFS_D(dent)->lock);
	TCPTFS_D(dent)->lower_path.dentry = NULL;
	TCPTFS_D(dent)->lower_path.mnt = NULL;
	spin_unlock(&TCPTFS_D(dent)->lock);
	return;
}
static inline void tcptfs_put_reset_lower_path(const struct dentry *dent)
{
	struct path lower_path;
	spin_lock(&TCPTFS_D(dent)->lock);
	pathcpy(&lower_path, &TCPTFS_D(dent)->lower_path);
	TCPTFS_D(dent)->lower_path.dentry = NULL;
	TCPTFS_D(dent)->lower_path.mnt = NULL;
	spin_unlock(&TCPTFS_D(dent)->lock);
	path_put(&lower_path);
	return;
}

/* locking helpers */
static inline struct dentry *lock_parent(struct dentry *dentry)
{
	struct dentry *dir = dget_parent(dentry);
	mutex_lock_nested(&dir->d_inode->i_mutex, I_MUTEX_PARENT);
	return dir;
}

static inline void unlock_dir(struct dentry *dir)
{
	mutex_unlock(&dir->d_inode->i_mutex);
	dput(dir);
}

/* Crypto */
static inline  void hexdump(unsigned char *buf,unsigned int len) {
    while(len--)
        printk("%02x",*buf++);
    printk("\n");
}

int encrypt(char *buf, size_t buflen, char *key, size_t keylen);
int decrypt(char *buf, size_t buflen, char *key, size_t keylen);

#endif	/* not _TCPTFS_H_ */
