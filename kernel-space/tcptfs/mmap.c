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
	/* f_mode need to be set for read */
	lower_file->f_mode |= FMODE_READ;
	rc = vfs_read(lower_file, data, size, &offset);
	set_fs(fs_save);
	return rc;
}

/**
 * tcptfs_readpage_from_lower
 * @tcptfs_file: the tcptfs file
 * @page: the page we will deal with
 * @tcptfs_inode: tcptfs inode object
 * @page index: page index
 *
 * tcptfs_readpage_from_lower is a wrapper of tcptfs_read_lower
 * It prepare parameters, convert upper file to lower file, for
 * tcptfs_read_lower.
 *
 * Returns 0 if success else negative.
 */
static int tcptfs_readpage_from_lower(struct file *tcptfs_file, struct page *page, struct inode *tcptfs_inode, pgoff_t page_index)
{
	loff_t offset;	/* offset in file */
	struct file *lower_file;
	char *virt;
	int rc;

	offset = ((loff_t)page_index) << PAGE_CACHE_SHIFT;
	virt = kmap(page);
	lower_file = tcptfs_lower_file(tcptfs_file);
	if (!lower_file) {
		printk(KERN_ERR "Error atempting to get lower file\n");
		return -EIO;
	}
	rc = tcptfs_read_lower(virt, offset, PAGE_CACHE_SIZE, tcptfs_inode, lower_file);
	if (rc >0)
		/* tcptfs_read_lower return bytes read, but we return status */
		rc = 0;
	kunmap(page);
	flush_dcache_page(page);
	return rc;
}

static int tcptfs_readpage(struct file *file, struct page *page)
{
	int rc;

	rc = tcptfs_readpage_from_lower(file, page, page->mapping->host, page->index);
	rc = tcptfs_decrypt_page(page);
	if (rc)
		ClearPageUptodate(page);
	else
		SetPageUptodate(page);
	unlock_page(page);
	return rc;
}

/**
  * tcptfs_writepage
  * @page: page that is locked before this call is made
  * 
  * Return zero when success; no-zero otherwise
  *
  * This is where we encrypt data and pass the encrypted date to lower fs.
  */
static int tcptfs_writepage(struct page *page, struct writeback_control *wbc)
{
	int rc;

	/*
	 * Refuse to write the page out if we are called from reclaim context
	 * since our writepage() path may potentially allocate memory when
	 * calling into the lower fs vfs_write() which may in turn invoke
	 * us again.
	 */
	if (current->flags & PF_MEMALLOC) {
		redirty_page_for_writepage(wbc, page);
		rc = 0;
		goto out;
	}

	rc = tcptfs_encrypt_page(page);
	SetPageUptodate(page);
out:
	unlock_page(page);
	return rc;
}
/**
 * tcptfs_write
 * @file: the tcptfs file object
 * @mapping: the tcptfs object
 * @pos: the file offset at which we start writing
 * @len: length of data we need to write
 * @flags: various flags
 * @page: pointer to return the page
 * @fsdata: pointer to return fs data(unused)
 *
 * This function must zero any hole we create
 *
 * return zero on success; non-zero otherwise
 */
static int tcptfs_write_begin(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned flags,
			struct page **pagep, void **fsdata)
{
	pgoff_t index = pos >> PAGE_CACHE_SHIFT;
	struct page *page;
	loff_t prev_page_end_size;
	int rc = 0;

	page = grab_cache_page_write_begin(mapping, index, flags);
	if (!page)
		return -ENOMEM;
	*pagep = page;

	prev_page_end_size = ((loff_t)index << PAGE_CACHE_SHIFT);
	if (!PageUptodate(page)) {
		rc = tcptfs_readpage_from_lower(file, page, mapping->host, index);
		if (rc) {
			printk(KERN_ERR "Error reading lower page\n");
			ClearPageUptodate(page);
			goto out;
		}
		SetPageUptodate(page);
	}
	/* if creating a page or more of holes, zero them out via stuncate
	 * note that this will increase i_size */
	if (index != 0) {
		if (prev_page_end_size > i_size_read(page->mapping->host)) {
			/* TODO: rc = we_dont_have_truncate(); */

		}
	}
	if ((i_size_read(mapping->host) == prev_page_end_size) && (pos != 0))
		zero_user(page, 0, PAGE_CACHE_SIZE);

out:
	if (unlikely(rc)) {
		unlock_page(page);
		page_cache_release(page);
		*pagep = NULL;
	}
	return rc;
}

static int tcptfs_write_lower(struct inode *tcptfs_inode, char *data, loff_t offset, size_t size, struct file *tcptfs_file)
{
	struct file *lower_file;
	mm_segment_t fs_save;
	ssize_t rc;

	lower_file = tcptfs_lower_file(tcptfs_file);
	if (!lower_file)
		return -EIO;
	fs_save = get_fs();
	set_fs(get_ds());
	rc = vfs_write(lower_file, data, size, &offset);
	set_fs(fs_save);
	mark_inode_dirty_sync(tcptfs_inode);
	return rc;
}

static int tcptfs_writepage_to_lower(struct inode *tcptfs_inode, struct page *page_for_lower, size_t offset_in_page, size_t size, struct file *tcptfs_file)
{
	char *virt;
	loff_t offset;
	int rc;

	offset =((((loff_t)page_for_lower->index) << PAGE_CACHE_SHIFT) + offset_in_page);
	virt = kmap(page_for_lower);
	rc = tcptfs_write_lower(tcptfs_inode, virt, offset, size, tcptfs_file);
	if (rc > 0)
		rc = 0;
	kunmap(page_for_lower);
	return rc;
}

/**
 * tcptfs_write_end
 * @file: the tcptfs file object
 * @mapping: the tcptfs object
 * @pos: the file position
 * @len: (unused)
 * @copied: the amount of data copied
 * @page: the tcptfs page
 * @fdata: (unused)
 *///TODO
static int tcptfs_write_end(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned copied,
			struct page *page, void *fsdata)
{
	unsigned from = pos & (PAGE_CACHE_SIZE - 1);
	unsigned to = from + copied;
	struct inode *tcptfs_inode = mapping->host;
	int rc;
	//int need_unlock_page = 1;

	rc = tcptfs_writepage_to_lower(tcptfs_inode, page, 0, to, file);
	if (!rc) {
		rc = copied;
		fsstack_copy_inode_size(tcptfs_inode, tcptfs_lower_inode(tcptfs_inode));
	}
	unlock_page(page);
	page_cache_release(page);
	return rc;
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
