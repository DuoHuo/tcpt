#include "tcptfs.h"

static int tcptfs_readpage(struct file *file, struct page *page)
{
	printk(KERN_ERR "invoking tcptfs_readpage\n");
	return -1;
}


static int tcptfs_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	int err;
	struct file *file, *lower_file;
	const struct vm_operations_struct *lower_vm_ops;
	struct vm_area_struct lower_vma;

	memcpy(&lower_vma, vma, sizeof(struct vm_area_struct));
	file = lower_vma.vm_file;
	lower_vm_ops = TCPTFS_F(file)->lower_vm_ops;
	BUG_ON(!lower_vm_ops);

	lower_file = tcptfs_lower_file(file);
	/*
	 * XXX: vm_ops->fault may be called in parallel.  Because we have to
	 * resort to temporarily changing the vma->vm_file to point to the
	 * lower file, a concurrent invocation of tcptfs_fault could see a
	 * different value.  In this workaround, we keep a different copy of
	 * the vma structure in our stack, so we never expose a different
	 * value of the vma->vm_file called to us, even temporarily.  A
	 * better fix would be to change the calling semantics of ->fault to
	 * take an explicit file pointer.
	 */
	lower_vma.vm_file = lower_file;
	err = lower_vm_ops->fault(&lower_vma, vmf);
	return err;
}

/*
 * XXX: the default address_space_ops for tcptfs is empty.  We cannot set
 * our inode->i_mapping->a_ops to NULL because too many code paths expect
 * the a_ops vector to be non-NULL.
 */
const struct address_space_operations tcptfs_aops = {
	/* empty on purpose */
	.readpage = tcptfs_readpage,
};

const struct vm_operations_struct tcptfs_vm_ops = {
	.fault		= tcptfs_fault,
};
