#include "tcptfs.h"
#include <linux/module.h>

/*
 * our custom d_alloc_root work-alike
 *
 * we can't use d_alloc_root if we want to use our own interpose function
 * unchanged, so we simply call our own "fake" d_alloc_root
 */
static struct dentry *tcptfs_d_alloc_root(struct super_block *sb)
{
	struct dentry *ret = NULL;

	if (sb) {
		static const struct qstr name = {
			.name = "/",
			.len = 1
		};

		ret = d_alloc(NULL, &name);
		if (ret) {
			d_set_d_op(ret, &tcptfs_dops);
			ret->d_sb = sb;
			ret->d_parent = ret;
		}
	}
	return ret;
}

/*
 * There is no need to lock the tcptfs_super_info's rwsem as there is no
 * way anyone can have a reference to the superblock at this point in time.
 */
static int tcptfs_read_super(struct super_block *sb, void *raw_data, int silent)
{
	int err = 0;
	struct super_block *lower_sb;
	struct path lower_path;
	char *dev_name = (char *) raw_data;

	if (!dev_name) {
		printk(KERN_ERR
		       "tcptfs: read_super: missing dev_name argument\n");
		err = -EINVAL;
		goto out;
	}

	/* parse lower path */
	err = kern_path(dev_name, LOOKUP_FOLLOW | LOOKUP_DIRECTORY,
			&lower_path);
	if (err) {
		printk(KERN_ERR	"tcptfs: error accessing "
		       "lower directory '%s'\n", dev_name);
		goto out;
	}

	/* allocate superblock private data */
	sb->s_fs_info = kzalloc(sizeof(struct tcptfs_sb_info), GFP_KERNEL);
	if (!TCPTFS_SB(sb)) {
		printk(KERN_CRIT "tcptfs: read_super: out of memory\n");
		err = -ENOMEM;
		goto out_free;
	}

	/* set the lower superblock field of upper superblock */
	lower_sb = lower_path.dentry->d_sb;
	atomic_inc(&lower_sb->s_active);
	tcptfs_set_lower_super(sb, lower_sb);

	/* inherit maxbytes from lower file system */
	sb->s_maxbytes = lower_sb->s_maxbytes;

	/*
	 * Our c/m/atime granularity is 1 ns because we may stack on file
	 * systems whose granularity is as good.
	 */
	sb->s_time_gran = 1;

	sb->s_op = &tcptfs_sops;

	/* see comment next to the definition of tcptfs_d_alloc_root */
	sb->s_root = tcptfs_d_alloc_root(sb);
	if (!sb->s_root) {
		err = -ENOMEM;
		goto out_sput;
	}

	/* link the upper and lower dentries */
	sb->s_root->d_fsdata = NULL;
	err = new_dentry_private_data(sb->s_root);
	if (err)
		goto out_freeroot;

	/* set the lower dentries for s_root */
	tcptfs_set_lower_path(sb->s_root, &lower_path);

	/* call interpose to create the upper level inode */
	err = tcptfs_interpose(sb->s_root, sb, &lower_path);
	if (!err) {
		if (!silent)
			printk(KERN_INFO
			       "tcptfs: mounted on top of %s type %s\n",
			       dev_name, lower_sb->s_type->name);
		goto out;
	}
	/* else error: fall through */

	free_dentry_private_data(sb->s_root);
out_freeroot:
	dput(sb->s_root);
out_sput:
	/* drop refs we took earlier */
	atomic_dec(&lower_sb->s_active);
	kfree(TCPTFS_SB(sb));
	sb->s_fs_info = NULL;
out_free:
	path_put(&lower_path);

out:
	return err;
}

struct dentry *tcptfs_mount(struct file_system_type *fs_type, int flags,
			    const char *dev_name, void *raw_data)
{
	void *lower_path_name = (void *) dev_name;

	return mount_nodev(fs_type, flags, lower_path_name,
			   tcptfs_read_super);
}

static struct file_system_type tcptfs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= TCPTFS_NAME,
	.mount		= tcptfs_mount,
	.kill_sb	= generic_shutdown_super,
	.fs_flags	= FS_REVAL_DOT,
};

static int __init init_tcptfs_fs(void)
{
	int err;

	pr_info("Registering tcptfs " TCPTFS_VERSION "\n");

	err = tcptfs_init_inode_cache();
	if (err)
		goto out;
	err = tcptfs_init_dentry_cache();
	if (err)
		goto out;
	err = register_filesystem(&tcptfs_fs_type);
out:
	if (err) {
		tcptfs_destroy_inode_cache();
		tcptfs_destroy_dentry_cache();
	}
	return err;
}

static void __exit exit_tcptfs_fs(void)
{
	tcptfs_destroy_inode_cache();
	tcptfs_destroy_dentry_cache();
	unregister_filesystem(&tcptfs_fs_type);
	pr_info("Completed tcptfs module unload\n");
}

MODULE_AUTHOR("Freeman Zhang@NUIST");
MODULE_DESCRIPTION("tcptfs " TCPTFS_VERSION);
MODULE_LICENSE("GPL");

module_init(init_tcptfs_fs);
module_exit(exit_tcptfs_fs);
