/*
 * oops_lab.c v1.0 <date>
 *
 * Simple kernel module with bugs. Debug it.
 *---------------------------------------------------------------------
 * Instructions:
 *--------------
 * 1. Build, load and test (remember to look up the kernel log buffer
 *     with dmesg)
 * 2. Debug, repeat step 1, correct the first (fairly obvious) bug.
 * 3. Experiment:
 *    a) set "SZ" to 5 MB and try it out
 *    b) enable the "copy_to_user (ARBIT_USER...)" code and see the
 *       effect, both with the return error-check and without it.
 *    c) Try out the Oops/panic that gets generated (if you enable the 
 *       code at the bottom of the init function).
 *---------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library
 * General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Library
 * General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Headers */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>

#define MODULE_VER     "1.0"
#define MODULE_NAME    "oops_lab"

/* MSG macro : only [f]print[k|f] when debugging is on.
 * From LDD3, ch 4, slightly modified.
 */
#undef MSG /* undef it, just in case */
#ifdef DEBUG
# ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
# define MSG(string, args...) printk (KERN_DEBUG "%s:%s : " string, \
__FILE__, __FUNCTION__, ##args)
# else
/* This one for user space */
#define MSG(string, args...) \
fprintf(stderr, "%s:%s : " string, __FILE__, __FUNCTION__, ##args)
# endif
# else
#define MSG(fmt, args...) /* not debugging: nothing */
#endif

#define	SZ	PAGE_SIZE
#define	ARBIT_USER	1024*1024*32

/*
 * function oops_lab1_init
 */
static int __init oops_lab1_init(void)
{
	char * kbuf;
	int ret;
	
	printk(KERN_INFO "Hello, %s ver %s initialized\n",
		MODULE_NAME, MODULE_VER);

	MSG ("allocating %ld bytes memory now..\n", SZ);
	if (!(kbuf = kmalloc (SZ, GFP_KERNEL))) {
		printk (KERN_DEBUG "%s: out of memory.\n", MODULE_NAME);
		return -ENOMEM;
	}
	memset (kbuf, 0xae, SZ);
#if 0
	ret = copy_to_user (ARBIT_USER, kbuf, SZ);
	if (ret) {
		printk (KERN_DEBUG "%s: copy_to_user failure\n", MODULE_NAME);
		kfree (kbuf);
		return -EFAULT;
	}
#endif	
	vfree (kbuf);
	MSG ("%ld bytes freed up..\n", SZ);

/* Make "#if 1" to see a kernel panic (on RHEL4's default kernel, we seem to
 * get a panic here) / Oops here (assuming the memory is freed up).
 */	
#if 0
	kbuf=0;
	*kbuf = 0xae;
#endif	
#if 0
	*(int *)0 = 0;
#endif	
	return 0;	// success
}


/*
 * function oops_lab1_cleanup
 */
static void oops_lab1_cleanup(void)
{
	printk(KERN_INFO "Goodbye, %s ver %s removed\n",
		MODULE_NAME, MODULE_VER);
}

module_init(oops_lab1_init);
module_exit(oops_lab1_cleanup);

MODULE_AUTHOR("My Name");
MODULE_DESCRIPTION("A simple buggy kernel module demo");
MODULE_LICENSE("GPL");

/* End oops_lab.c */

