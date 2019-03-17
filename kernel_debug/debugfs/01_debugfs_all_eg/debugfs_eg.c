/*
 * debugfs_eg.c
 * Simple kernel module to demo using debugfs...
 * Refer: Documentation/filesystems/debugfs.txt
 *
 * Author: Kaiwan N Billimoria, kaiwanTECH
 * Dual GPL/MIT.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
//#include <asm/uaccess.h> // [OLD]
#include "../../convenient.h"

#define	DRVNAME		"debugfs_simple"

#define DBGFS_CREATE_ERR(pDentry, str) do {                   \
	printk("%s: failed.\n", str);                         \
	if (PTR_ERR(pDentry) == -ENODEV)                      \
		printk(" debugfs support not available?\n");  \
	debugfs_remove_recursive(pDentry);	              \
	return PTR_ERR(pDentry);                              \
} while (0)


static struct dentry *parent;


// TODO : make concurrent-safe

//- 1. -------------------------------------------Generic R/W debugfs hooks
static ssize_t dbgfs_genread(struct file *filp, char __user *ubuf, size_t count, loff_t *fpos)
{
	char *data =  (char *)filp->f_inode->i_private; // retrieve the "data" from the inode
	//char *data =  (char *)filp->f_dentry->d_inode->i_private; // [OLD] retrieve the "data" from the inode
	MSG("data: %s len=%ld\n", data, strlen(data));

	/* simple_read_from_buffer - copy data from the buffer to user space:
     * @to: the user space buffer to read to
     * @count: the maximum number of bytes to read
     * @ppos: the current position in the buffer
     * @from: the buffer to read from
     * @available: the size of the buffer
     *
     * The simple_read_from_buffer() function reads up to @count bytes from the
     * buffer @from at offset @ppos into the user space address starting at @to.
     *
     * On success, the number of bytes read is returned and the offset @ppos is
     * advanced by this number, or negative value is returned on error.

         ssize_t simple_read_from_buffer(void __user *to, size_t count, loff_t *ppos,
                 const void *from, size_t available)
	*/
	return simple_read_from_buffer(ubuf, strlen(data), fpos, data, strlen(data));
}

#define MAXUPASS 100 // careful- k stack is small!
static ssize_t dbgfs_genwrite(struct file *filp, const char __user *ubuf, size_t count, loff_t *fpos)
{
	char udata[MAXUPASS];
	QP;
	if (count > MAXUPASS) {
		printk("%s: too much data attempted to be passed from userspace to here: %s:%d\n", 
			DRVNAME, __FUNCTION__, __LINE__);
		return -ENOSPC;
	}
	if (copy_from_user(udata, ubuf, count)) {
		printk("%s:%s:%d: copy_from_user failed!\n", 
			DRVNAME, __FUNCTION__, __LINE__);
		return -EIO;
	}
	print_hex_dump_bytes(" ", DUMP_PREFIX_NONE, udata, count);
	return count;
}

static struct file_operations dbg_fops1 = {
	.read = dbgfs_genread,
	.write = dbgfs_genwrite,
};

//- 2. -----------------------------------------------------Passing a structure pointer
typedef struct {
	short tx, rx;
	u32 j;
	char sec[20];
} MYS;
static MYS *mine;

static ssize_t dbgfs_genread2(struct file *filp, char __user *ubuf, size_t count, loff_t *fpos)
{
	MYS *data =  (MYS *)filp->f_inode->i_private; // retrieve the "data" from the inode
	//MYS *data =  (MYS *)filp->f_dentry->d_inode->i_private; // retrieve the "data" from the inode
	char loc[MAXUPASS];

	data->j = jiffies;
	MSG("data: tx=%d rx=%d j=%u sec=%s\n", data->tx, data->rx, data->j, data->sec);
	snprintf(loc, count-1, "data: tx=%d rx=%d j=%u sec=%s\n", data->tx, data->rx, data->j, data->sec);
	return simple_read_from_buffer(ubuf, MAXUPASS, fpos, loc, strlen(loc));
}

static struct file_operations dbg_fops2 = {
	.read = dbgfs_genread2,
};

//- 3. -------------------------------------------- Various debugfs helpers
static u32 myu32=100;
static u64 myu64=64;
static u32 myx32=0x100;
static bool mybool=1;

// 5. -------------------------------------------------------- Blob wrapper
static struct debugfs_blob_wrapper myblob;


//-----------------------------------------------------------------------------
static int setup_debugfs_entries(void)
{
	QP;
	parent = debugfs_create_dir(DRVNAME, NULL);
	if (!parent) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_dir");
	}

	/* 1. Generic debugfs file + passing a pointer to string as a demo..
       4th param is a generic void * ptr; it's contents will be stored into the i_private field
	   of the file's inode.
	*/
	if (!debugfs_create_file("generic_1", 0644, parent, (void *)"somejunk data", &dbg_fops1)) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_file 1");
	}

	/* 2. Generic debugfs file + passing a pointer to a data structure as a demo..
       4th param is a generic void * ptr; it's contents will be stored into the i_private field
	   of the file's inode.
	*/
	mine = (MYS *)kmalloc (sizeof(MYS), GFP_KERNEL);
	if (!mine) {
		printk(KERN_ALERT "%s: kmalloc failed!\n", DRVNAME);
		return -ENOMEM;
	}
	mine->tx = mine->rx = 0;
	mine->j = jiffies;
	strncpy (mine->sec, "Security Proto 1B", 20);
	if (!debugfs_create_file("generic_2", 0440, parent, (void *)mine, &dbg_fops2)) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_file 2");
	}

	/* 3. In a number of cases, the creation of a set of file operations is not
       actually necessary; the debugfs code provides a number of helper functions
       for simple situations.  ...
      struct dentry *debugfs_create_u32(const char *name, mode_t mode,
				      struct dentry *parent, u32 *value); 
	  ...
    */
	if (!debugfs_create_u32("helper_u32", 0644, parent, &myu32)) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_u32 ");
	}
	if (!debugfs_create_u64("helper_u64", 0644, parent, &myu64)) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_u64 ");
	}
	// For hex, use the debugfs_create_x[8|16|32|64] helpers...
	if (!debugfs_create_x32("helper_x32", 0644, parent, &myx32)) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_x32 ");
	}

	/* 4. Boolean values can be placed in debugfs with:

       struct dentry *debugfs_create_bool(const char *name, mode_t mode,
				       struct dentry *parent, u32 *value);

       A read on the resulting file will yield either Y (for non-zero values) or
       N, followed by a newline.  If written to, it will accept either upper- or
       lower-case values, or 1 or 0.  Any other input will be silently ignored.
     */
	if (!debugfs_create_bool("helper_bool", 0644, parent, &mybool)) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_bool ");
	}

	/* 5. A block of arbitrary binary data can be exported with:

           struct debugfs_blob_wrapper {
           	   void *data;
       	       unsigned long size;
           };

           struct dentry *debugfs_create_blob(const char *name, mode_t mode,
				       struct dentry *parent,
				       struct debugfs_blob_wrapper *blob);

       A read of this file will return the data pointed to by the
       debugfs_blob_wrapper structure.  Some drivers use "blobs" as a simple way
       to return several lines of (static) formatted text output.  This function
       can be used to export binary information, but there does not appear to be
       any code which does so in the mainline.  Note that all files created with
       debugfs_create_blob() are read-only.
	*/
	myblob.data = mine;
	myblob.size = sizeof(MYS);
	if (!debugfs_create_blob("myblob", 0444, parent, &myblob)) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_blob ");
	}

	// 6. Soft link
	if  (!debugfs_create_symlink("thedata", parent, "generic_2")) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_symlink ");
	}
	return 0;
}

static int debugfs_init(void)
{
	setup_debugfs_entries();
	pr_info("%s: sample debugfs entries setup at: <debugfs_mount>/%s\n",
			DRVNAME, DRVNAME);
	return 0;
}

static void debugfs_exit(void)
{
	kfree (mine);
	debugfs_remove_recursive(parent);
	MSG("Done.\n");
}

MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("Simple demo of using the Linux debugfs filesystem");
MODULE_AUTHOR("Kaiwan NB");

module_init(debugfs_init);
module_exit(debugfs_exit);
