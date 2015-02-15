#include "tcptfs.h"

/**
 * tcptfs_read_lower
 * @data: The data read is stored in this location
 * @offset: Byte offset in the lower file
 * @size: Number of bytes to read
 * @tcptfs_inode: the tcptfs inode
 * @lower_file: the lower file
 *
 * Read @size bytes of data at byte offset @offset from the lower
 * inode into memory location @data.
 *
 * Returns bytes read on success; 0 on EOF; less than zero on error
 */
static int tcptfs_read_lower(char *data, loff_t offset, size_t size,
			struct inode *tcptfs_inode, struct file *lower_file)
{
	mm_segment_t fs_save;
	ssize_t rc;

	fs_save = get_fs();
	set_fs(get_ds());
	rc = vfs_read(lower_file, data, size, &offset);
	set_fs(fs_save);
	printk(KERN_ERR "RRRRRRReturn from tcptfs_read_lower with rc=%d\n", rc);
	return rc;
}

/**
 * tcptfs_readpage_from_lower
 * @tcptfs_file: the tcptfs file
 * @page: the page we will deal with
 *
 * tcptfs_readpage_from_lower is a wrapper of tcptfs_read_lower
 * It prepare parameters, convert upper file to lower file, for
 * tcptfs_read_lower.
 *
 * Returns 0 if success else negative.
 */
static int tcptfs_readpage_from_lower(struct file *tcptfs_file, struct page *page)
{
	loff_t offset;	/* offset in file */
	struct file *lower_file;
	char *virt;
	int rc;

	offset = ((loff_t)page->index) << PAGE_CACHE_SHIFT;
	virt = kmap(page);
	lower_file = tcptfs_lower_file(tcptfs_file);
	if (!lower_file)
		return -EIO;
	rc = tcptfs_read_lower(virt, offset, PAGE_CACHE_SIZE, page->mapping->host, lower_file);
	if (rc >0)
		/* tcptfs_read_lower return bytes read, but we return status */
		rc = 0;
	kunmap(page);
	flush_dcache_page(page);
	printk(KERN_ERR "RRRRRRReturn from tcptfs_readpage_from_lower with rc=%d\n",rc);
	return rc;
}

static int tcptfs_readpage(struct file *file, struct page *page)
{
	int rc;

	rc = tcptfs_readpage_from_lower(file, page);
	rc = tcptfs_decrypt_page(page);
	if (rc)
		ClearPageUptodate(page);
	else
		SetPageUptodate(page);
	unlock_page(page);
	printk(KERN_ERR "RRRRRRRReturn from tcptfs_readpage with rc=%d\n", rc);
	return rc;
}

static int tcptfs_writepage(struct page *page, struct writeback_control *wbc)
{
	printk(KERN_ERR "invoking tcptfs_writepage\n");
	return 0;
}

static int tcptfs_write_begin(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned flags,
			struct page **page, void **fsdata)
{
	printk(KERN_ERR "invoking tcptfs_write_begin\n");
	return 0;
}

static int tcptfs_write_end(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned copied,
			struct page *page, void *fsdata)
{
	printk(KERN_ERR "invoking tcptfs_write_end\n");
	return 0;
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
	.writepage = tcptfs_writepage,
	.readpage = tcptfs_readpage,
	.write_begin = tcptfs_write_begin,
	.write_end = tcptfs_write_end,
};

const struct vm_operations_struct tcptfs_vm_ops = {
	.fault		= tcptfs_fault,
};
