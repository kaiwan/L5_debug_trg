/**
 * showthrds_dbgfs.c
 * Simple demo usage of showing all live threads + debugfs usage.
 * Author: Kaiwan N Billimoria <kaiwan@kaiwantech.com>
 */

/* TODO- 
 * Setup one more write-only entry /proc/driver/showthreads: 
 * write the TGID, and this proc fn displays all threads
 * belonging to this process..
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <linux/debugfs.h>
#include <linux/vmalloc.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0) 
#include <linux/sched/signal.h>
#endif

#include "convenient.h"

#define	DRVNAME		"showthrds_dbgfs"

#define DBGFS_CREATE_ERR(pDentry, str) do {  \
		pr_info("%s: failed.\n", str);     \
		if (PTR_ERR(pDentry) == -ENODEV)     \
			pr_info(" debugfs support not available?\n");     \
		debugfs_remove_recursive(pDentry);	\
		return PTR_ERR(pDentry);     \
} while (0)

static struct dentry *parent;

static ssize_t dbgfs_showallthrds(struct file *filp, char __user *ubuf, size_t count, loff_t *fpos)
{
	struct task_struct *g, *t; // 'g' : process ptr; 't': thread ptr !
	char *data;
#define MEM_REQD_PER_THREAD 100
	char tmpbuf[128];
	ssize_t numthrds=0, n=0;

	do_each_thread(g, t) {
		numthrds ++;
	} while_each_thread(g, t);
	MSG("# threads alive: %zd\n", numthrds);

	data = vzalloc(numthrds*MEM_REQD_PER_THREAD);
	if (!data) {
	  pr_alert("%s: vzalloc failure!\n", DRVNAME);
	  return -ENOMEM;
	}

	snprintf (data, 300, 
"----------------------------------------------------------------------------\n"
"    TGID   PID            Name            # Threads\n"
"----------------------------------------------------------------------------\n"
	);

	do_each_thread(g, t) {
		task_lock(t);

		sprintf(tmpbuf, "%6d %6d ", g->tgid, t->pid);
		strncat (data, tmpbuf, 14);

		if (!g->mm) { // kernel thread
			 sprintf(tmpbuf, "[%20s]", g->comm);
		} else {
			 sprintf(tmpbuf, " %20s ", g->comm);
		}
		strncat (data, tmpbuf, 41);

		// "main" thread of multiple
		if (g->mm && (g->tgid == t->pid) && (atomic_read(&g->mm->mm_users) > 1)) {
			sprintf(tmpbuf, "    %4d", atomic_read(&g->mm->mm_users));
			strncat (data, tmpbuf, 8);
		}

		sprintf(tmpbuf, "\n");
		strncat (data, tmpbuf, 2);

		task_unlock(t);

		numthrds--;
		if (!numthrds) {
			pr_info("%s:Ouch! exceeded memory! aborting output now..", DRVNAME);
			goto out;
		}
	} while_each_thread(g, t);


//	int data =  (int)filp->f_dentry->d_inode->i_private; // retrieve the "data" from the inode
//	MSG("data: %d\n", data);

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
out:
	n = simple_read_from_buffer(ubuf, strlen(data), fpos, data, strlen(data));
	vfree (data);
	return n;
}

static struct file_operations dbg_fops1 = {
	.read = dbgfs_showallthrds,
};

static int __init showthrds_init_module(void)
{
	parent = debugfs_create_dir(DRVNAME, NULL);
	if (!parent) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_dir");
	}

	if (!debugfs_create_file("show_all_threads", 0644, parent, 
		(void *)0, &dbg_fops1)) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_file 1");
	}
	MSG("Debugfs entry setup successfully (under <debugfs_mount>/%s) .\n", 
			DRVNAME);
	return 0;
}

static void __exit showthrds_cleanup_module(void)
{
	debugfs_remove_recursive(parent);
	MSG("Removed.\n");
}

module_init(showthrds_init_module);
module_exit(showthrds_cleanup_module);

MODULE_AUTHOR("Kaiwan NB@kaiwanTECH");
MODULE_DESCRIPTION("Displays all threads.");
MODULE_LICENSE("Dual BSD/GPL");
