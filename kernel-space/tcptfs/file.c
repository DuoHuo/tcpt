#include "tcptfs.h"

extern uint8_t keyring[20];

static ssize_t tcptfs_read(struct file *file, char __user *buf,
			   size_t count, loff_t *ppos)
{
	int err;
	struct file *lower_file;
	struct dentry *dentry = file->f_path.dentry;
	size_t keylen;

	lower_file = tcptfs_lower_file(file);
	err = vfs_read(lower_file, buf, count, ppos);
	keylen = sizeof(keyring);
	decrypt(buf, count, keyring, keylen);
	/* update our inode atime upon a successful lower read */
	if (err >= 0)
		fsstack_copy_attr_atime(dentry->d_inode,
					lower_file->f_path.dentry->d_inode);

	return err;
}

static ssize_t tcptfs_write(struct file *file, const char __user *buf,
			    size_t count, loff_t *ppos)
{
	int err = 0;
	struct file *lower_file;
	struct dentry *dentry = file->f_path.dentry;
	size_t keylen;

	lower_file = tcptfs_lower_file(file);
	keylen = sizeof(keyring);
	encrypt((char *)buf, count, keyring, keylen);
	err = vfs_write(lower_file, (char *)buf, count, ppos);
	/* update our inode times+sizes upon a successful lower write */
	if (err >= 0) {
		fsstack_copy_inode_size(dentry->d_inode,
					lower_file->f_path.dentry->d_inode);
		fsstack_copy_attr_times(dentry->d_inode,
					lower_file->f_path.dentry->d_inode);
	}

	return err;
}

static int tcptfs_readdir(struct file *file, void *dirent, filldir_t filldir)
{
	int err = 0;
	struct file *lower_file = NULL;
	struct dentry *dentry = file->f_path.dentry;

	lower_file = tcptfs_lower_file(file);
	err = vfs_readdir(lower_file, filldir, dirent);
	file->f_pos = lower_file->f_pos;
	if (err >= 0)		/* copy the atime */
		fsstack_copy_attr_atime(dentry->d_inode,
					lower_file->f_path.dentry->d_inode);
	return err;
}

static long tcptfs_unlocked_ioctl(struct file *file, unsigned int cmd,
				  unsigned long arg)
{
	long err = -ENOTTY;
	struct file *lower_file;

	lower_file = tcptfs_lower_file(file);

	/* XXX: use vfs_ioctl if/when VFS exports it */
	if (!lower_file || !lower_file->f_op)
		goto out;
	if (lower_file->f_op->unlocked_ioctl)
		err = lower_file->f_op->unlocked_ioctl(lower_file, cmd, arg);

	/* some ioctls can change inode attributes (EXT2_IOC_SETFLAGS) */
	if (!err)
		fsstack_copy_attr_all(file->f_path.dentry->d_inode,
				      lower_file->f_path.dentry->d_inode);
out:
	return err;
}

#ifdef CONFIG_COMPAT
static long tcptfs_compat_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	long err = -ENOTTY;
	struct file *lower_file;

	lower_file = tcptfs_lower_file(file);

	/* XXX: use vfs_ioctl if/when VFS exports it */
	if (!lower_file || !lower_file->f_op)
		goto out;
	if (lower_file->f_op->compat_ioctl)
		err = lower_file->f_op->compat_ioctl(lower_file, cmd, arg);

out:
	return err;
}
#endif

static int tcptfs_mmap(struct file *file, struct vm_area_struct *vma)
{
	int err = 0;
	bool willwrite;
	struct file *lower_file;
	const struct vm_operations_struct *saved_vm_ops = NULL;

	/* this might be deferred to mmap's writepage */
	willwrite = ((vma->vm_flags | VM_SHARED | VM_WRITE) == vma->vm_flags);

	/*
	 * File systems which do not implement ->writepage may use
	 * generic_file_readonly_mmap as their ->mmap op.  If you call
	 * generic_file_readonly_mmap with VM_WRITE, you'd get an -EINVAL.
	 * But we cannot call the lower ->mmap op, so we can't tell that
	 * writeable mappings won't work.  Therefore, our only choice is to
	 * check if the lower file system supports the ->writepage, and if
	 * not, return EINVAL (the same error that
	 * generic_file_readonly_mmap returns in that case).
	 */
	lower_file = tcptfs_lower_file(file);
	if (willwrite && !lower_file->f_mapping->a_ops->writepage) {
		err = -EINVAL;
		printk(KERN_ERR "tcptfs: lower file system does not "
		       "support writeable mmap\n");
		goto out;
	}

	/*
	 * find and save lower vm_ops.
	 *
	 * XXX: the VFS should have a cleaner way of finding the lower vm_ops
	 */
	if (!TCPTFS_F(file)->lower_vm_ops) {
		err = lower_file->f_op->mmap(lower_file, vma);
		if (err) {
			printk(KERN_ERR "tcptfs: lower mmap failed %d\n", err);
			goto out;
		}
		saved_vm_ops = vma->vm_ops; /* save: came from lower ->mmap */
	}

	/*
	 * Next 3 lines are all I need from generic_file_mmap.  I definitely
	 * don't want its test for ->readpage which returns -ENOEXEC.
	 */
	file_accessed(file);
	vma->vm_ops = &tcptfs_vm_ops;
	vma->vm_flags |= VM_CAN_NONLINEAR;

	file->f_mapping->a_ops = &tcptfs_aops; /* set our aops */
	if (!TCPTFS_F(file)->lower_vm_ops) /* save for our ->fault */
		TCPTFS_F(file)->lower_vm_ops = saved_vm_ops;

out:
	return err;
}

static int tcptfs_open(struct inode *inode, struct file *file)
{
	int err = 0;
	struct file *lower_file = NULL;
	struct path lower_path;

	/* don't open unhashed/deleted files */
	if (d_unhashed(file->f_path.dentry)) {
		err = -ENOENT;
		goto out_err;
	}

	file->private_data =
		kzalloc(sizeof(struct tcptfs_file_info), GFP_KERNEL);
	if (!TCPTFS_F(file)) {
		err = -ENOMEM;
		goto out_err;
	}

	/* open lower object and link tcptfs's file struct to lower's */
	tcptfs_get_lower_path(file->f_path.dentry, &lower_path);
	lower_file = dentry_open(lower_path.dentry, lower_path.mnt,
				 file->f_flags, current_cred());
	if (IS_ERR(lower_file)) {
		err = PTR_ERR(lower_file);
		lower_file = tcptfs_lower_file(file);
		if (lower_file) {
			tcptfs_set_lower_file(file, NULL);
			fput(lower_file); /* fput calls dput for lower_dentry */
		}
	} else {
		tcptfs_set_lower_file(file, lower_file);
	}

	if (err)
		kfree(TCPTFS_F(file));
	else
		fsstack_copy_attr_all(inode, tcptfs_lower_inode(inode));
out_err:
	return err;
}

static int tcptfs_flush(struct file *file, fl_owner_t id)
{
	int err = 0;
	struct file *lower_file = NULL;

	lower_file = tcptfs_lower_file(file);
	if (lower_file && lower_file->f_op && lower_file->f_op->flush)
		err = lower_file->f_op->flush(lower_file, id);

	return err;
}

/* release all lower object references & free the file info structure */
static int tcptfs_file_release(struct inode *inode, struct file *file)
{
	struct file *lower_file;

	lower_file = tcptfs_lower_file(file);
	if (lower_file) {
		tcptfs_set_lower_file(file, NULL);
		fput(lower_file);
	}

	kfree(TCPTFS_F(file));
	return 0;
}

static int tcptfs_fsync(struct file *file, int datasync)
{
	int err;
	struct file *lower_file;
	struct path lower_path;
	struct dentry *dentry = file->f_path.dentry;

	lower_file = tcptfs_lower_file(file);
	tcptfs_get_lower_path(dentry, &lower_path);
	err = vfs_fsync(lower_file, datasync);
	tcptfs_put_lower_path(dentry, &lower_path);

	return err;
}

static int tcptfs_fasync(int fd, struct file *file, int flag)
{
	int err = 0;
	struct file *lower_file = NULL;

	lower_file = tcptfs_lower_file(file);
	if (lower_file->f_op && lower_file->f_op->fasync)
		err = lower_file->f_op->fasync(fd, lower_file, flag);

	return err;
}

const struct file_operations tcptfs_main_fops = {
	.llseek		= generic_file_llseek,
	.read		= tcptfs_read,
	.write		= tcptfs_write,
	.unlocked_ioctl	= tcptfs_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= tcptfs_compat_ioctl,
#endif
	.mmap		= tcptfs_mmap,
	.open		= tcptfs_open,
	.flush		= tcptfs_flush,
	.release	= tcptfs_file_release,
	.fsync		= tcptfs_fsync,
	.fasync		= tcptfs_fasync,
};

/* trimmed directory options */
const struct file_operations tcptfs_dir_fops = {
	.llseek		= generic_file_llseek,
	.read		= generic_read_dir,
	.readdir	= tcptfs_readdir,
	.unlocked_ioctl	= tcptfs_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= tcptfs_compat_ioctl,
#endif
	.open		= tcptfs_open,
	.release	= tcptfs_file_release,
	.flush		= tcptfs_flush,
	.fsync		= tcptfs_fsync,
	.fasync		= tcptfs_fasync,
};
