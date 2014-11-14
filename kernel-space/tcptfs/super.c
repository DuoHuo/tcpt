#include "tcptfs.h"

/*
 * The inode cache is used with alloc_inode for both our inode info and the
 * vfs inode.
 */
static struct kmem_cache *tcptfs_inode_cachep;

/* final actions when unmounting a file system */
static void tcptfs_put_super(struct super_block *sb)
{
	struct tcptfs_sb_info *spd;
	struct super_block *s;

	spd = TCPTFS_SB(sb);
	if (!spd)
		return;

	/* decrement lower super references */
	s = tcptfs_lower_super(sb);
	tcptfs_set_lower_super(sb, NULL);
	atomic_dec(&s->s_active);

	kfree(spd);
	sb->s_fs_info = NULL;
}

static int tcptfs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	int err;
	struct path lower_path;

	tcptfs_get_lower_path(dentry, &lower_path);
	err = vfs_statfs(&lower_path, buf);
	tcptfs_put_lower_path(dentry, &lower_path);

	/* set return buf to our f/s to avoid confusing user-level utils */
//	buf->f_type = TCPTFS_SUPER_MAGIC;
	buf->f_type = 0x7ee8;

	return err;
}

/*
 * @flags: numeric mount options
 * @options: mount options string
 */
static int tcptfs_remount_fs(struct super_block *sb, int *flags, char *options)
{
	int err = 0;

	/*
	 * The VFS will take care of "ro" and "rw" flags among others.  We
	 * can safely accept a few flags (RDONLY, MANDLOCK), and honor
	 * SILENT, but anything else left over is an error.
	 */
	if ((*flags & ~(MS_RDONLY | MS_MANDLOCK | MS_SILENT)) != 0) {
		printk(KERN_ERR
		       "tcptfs: remount flags 0x%x unsupported\n", *flags);
		err = -EINVAL;
	}

	return err;
}

/*
 * Called by iput() when the inode reference count reached zero
 * and the inode is not hashed anywhere.  Used to clear anything
 * that needs to be, before the inode is completely destroyed and put
 * on the inode free list.
 */
static void tcptfs_evict_inode(struct inode *inode)
{
	struct inode *lower_inode;

	truncate_inode_pages(&inode->i_data, 0);
	end_writeback(inode);
	/*
	 * Decrement a reference to a lower_inode, which was incremented
	 * by our read_inode when it was created initially.
	 */
	lower_inode = tcptfs_lower_inode(inode);
	tcptfs_set_lower_inode(inode, NULL);
	iput(lower_inode);
}

static struct inode *tcptfs_alloc_inode(struct super_block *sb)
{
	struct tcptfs_inode_info *i;

	i = kmem_cache_alloc(tcptfs_inode_cachep, GFP_KERNEL);
	if (!i)
		return NULL;

	/* memset everything up to the inode to 0 */
	memset(i, 0, offsetof(struct tcptfs_inode_info, vfs_inode));

	i->vfs_inode.i_version = 1;
	return &i->vfs_inode;
}

static void tcptfs_destroy_inode(struct inode *inode)
{
	kmem_cache_free(tcptfs_inode_cachep, TCPTFS_I(inode));
}

/* tcptfs inode cache constructor */
static void init_once(void *obj)
{
	struct tcptfs_inode_info *i = obj;

	inode_init_once(&i->vfs_inode);
}

int tcptfs_init_inode_cache(void)
{
	int err = 0;

	tcptfs_inode_cachep =
		kmem_cache_create("tcptfs_inode_cache",
				  sizeof(struct tcptfs_inode_info), 0,
				  SLAB_RECLAIM_ACCOUNT, init_once);
	if (!tcptfs_inode_cachep)
		err = -ENOMEM;
	return err;
}

/* tcptfs inode cache destructor */
void tcptfs_destroy_inode_cache(void)
{
	if (tcptfs_inode_cachep)
		kmem_cache_destroy(tcptfs_inode_cachep);
}

/*
 * Used only in nfs, to kill any pending RPC tasks, so that subsequent
 * code can actually succeed and won't leave tasks that need handling.
 */
static void tcptfs_umount_begin(struct super_block *sb)
{
	struct super_block *lower_sb;

	lower_sb = tcptfs_lower_super(sb);
	if (lower_sb && lower_sb->s_op && lower_sb->s_op->umount_begin)
		lower_sb->s_op->umount_begin(lower_sb);
}

const struct super_operations tcptfs_sops = {
	.put_super	= tcptfs_put_super,
	.statfs		= tcptfs_statfs,
	.remount_fs	= tcptfs_remount_fs,
	.evict_inode	= tcptfs_evict_inode,
	.umount_begin	= tcptfs_umount_begin,
	.show_options	= generic_show_options,
	.alloc_inode	= tcptfs_alloc_inode,
	.destroy_inode	= tcptfs_destroy_inode,
	.drop_inode	= generic_delete_inode,
};
