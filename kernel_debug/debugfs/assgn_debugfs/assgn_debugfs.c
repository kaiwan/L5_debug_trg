/*
 * assgn_debugfs.c
 * Simple kernel module to demo using debugfs...
 *
 * Assignment:
 * Write a kernel module that sets up a debugfs file; when any decimal or hex value
 * is written to the file, it should display it's corresponding binary value.
 *
 * Eg:
 * $ echo 100 > /sys/kernel/debug/assgn_debugfs/dec2bin
 * $ cat /sys/kernel/debug/assgn_debugfs/dec2bin
 * 1100100$
 *
 * Refer: Documentation/filesystems/debugfs.txt
 * Kaiwan NB
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
//#include <asm/uaccess.h> // OLD!
#include <linux/uaccess.h>
#include "../../convenient.h"

#define	DRVNAME		"assgn_debugfs"

#define DBGFS_CREATE_ERR(pDentry, str) do {  \
		printk("%s: failed.\n", str);     \
		if (PTR_ERR(pDentry) == -ENODEV)     \
			printk(" debugfs support not available?\n");     \
		debugfs_remove_recursive(pDentry);	\
		return PTR_ERR(pDentry);     \
} while (0)

static struct dentry *parent;
static char binstr[80];
// TODO : make concurrent-safe

static ssize_t dbgfs_genread(struct file *filp, char __user *ubuf, size_t count, loff_t *fpos)
{
	//char *data =  (char *)filp->f_dentry->d_inode->i_private; // retrieve the "data" from the inode
	//MSG("data: %s len=%d\n", data, strlen(data));

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
	//MSG("binstr: %s\n", binstr);
	return simple_read_from_buffer(ubuf, strlen(binstr), fpos, binstr, strlen(binstr));
}


/*
 * Converts decimal to binary. 
 * Credits: vegaseat. URL: http://www.daniweb.com/software-development/c/code/216349
 * accepts a decimal integer and returns a binary coded string
 *
 * @decimal : decimal value to convert to binary (IN)
 * @binary  : the binary result as a string (OUT)
 *
 */
void dec2bin(long decimal, char *binary)
{
        int k = 0, n = 0;
        int neg_flag = 0;
        int remain;
        /*
           gcc 4.6.3 : we get the warning:
           "warning: variable ‘old_decimal’ set but not used [-Wunused-but-set-variable]"
           To get rid of this warning, have #ifdef'd the test... -kaiwan.
          Keep one of the following below (wrt TESTMODE); comment out the other.
           UN-defining by default.
         */
//#define TESTMODE
#undef TESTMODE

#ifdef TESTMODE
        int old_decimal;        // for test
#endif
        char temp[80];

        // take care of negative input
        if (decimal < 0) {
                decimal = -decimal;
                neg_flag = 1;
        }
        do {
#ifdef TESTMODE
                old_decimal = decimal;  // for test
#endif
                remain = decimal % 2;
               // whittle down the decimal number
                decimal = decimal / 2;
                // this is a test to show the action
#ifdef TESTMODE
                printf("%d/2 = %d  remainder = %d\n", old_decimal, decimal,
                       remain);
#endif
                // converts digit 0 or 1 to character '0' or '1'
                temp[k++] = remain + '0';
        } while (decimal > 0);

        if (neg_flag)
                temp[k++] = '-';        // add - sign
        else
                temp[k++] = ' ';        // space

        // reverse the spelling
        while (k >= 0)
                binary[n++] = temp[--k];

        binary[n - 1] = 0;      // end with NULL
}

#define MAXUPASS 80 // careful- k stack is small!
static ssize_t dbgfs_genwrite(struct file *filp, const char __user *ubuf, size_t count, loff_t *fpos)
{
	char udata[MAXUPASS];
	long num=0;

	if (count > MAXUPASS) {
		printk("%s: too much data attempted to be passed from userspace to here: %s:%d\n", 
			DRVNAME, __FUNCTION__, __LINE__);
		return -ENOSPC;
	}
	if (copy_from_user(udata, ubuf, count))
		return -EIO;
	//print_hex_dump_bytes(" ", DUMP_PREFIX_NONE, udata, count);
	udata[count-1]='\0';
	MSG("count=%zu user passed: %s\n", count, udata);

	if (udata[0] == '0' && udata[1] == 'x') // it's a hex #
		num = simple_strtol(udata, NULL, 16);
	else
		num = simple_strtol(udata, NULL, 10);

	dec2bin(num, binstr);
	return count;
}

static struct file_operations dbg_fops = {
	.read = dbgfs_genread,
	.write = dbgfs_genwrite,
};

static int setup_debugfs_entries(void)
{
	parent = debugfs_create_dir(DRVNAME, NULL);
	if (!parent) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_dir");
	}

	if (!debugfs_create_file("dec2bin", 0666, parent, NULL, &dbg_fops)) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_file 1");
	}
	return 0;
}

static int debugfs_init(void)
{
	setup_debugfs_entries();
	return 0;
}

static void debugfs_exit(void)
{
	debugfs_remove_recursive(parent);
	MSG("Done.\n");
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Simple assignment: convert decimal to binary using the Linux debugfs filesystem");
MODULE_AUTHOR("Kaiwan NB");

module_init(debugfs_init);
module_exit(debugfs_exit);
