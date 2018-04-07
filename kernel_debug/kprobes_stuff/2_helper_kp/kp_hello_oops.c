/*
 * kp_hello_oops.c v1.0 <date>
 *
 * Simple kernel module that delibrately causes an Oops! dump.
 * A kprobe routine gets registered via the 'helper' kprobe
 * module kp_helper.
 *
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

/*
 * kp_hello_oops.c
 *
 * Simple kernel module, that, when inserted into the kernel
 * creates a proc-based file. When this file is read, the read callback is
 * delibrately made to cause an Oops! by dereferencing a NULL pointer.
 * The Oops dump can be looked up using dmesg.
 * 
 * Observed that on RHEL4 AS (kernel 2.6.9-34.ELsmp), this would often 
 * result in a complete system hang, whereas on SLED10 and RHEL4 with a 
 * vanilla kernel, the Oops would occur.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>

#define MODULE_VER 		"1.0"
#define MODULE_NAME    		"hello_oops"
#define procname		"oops-test"

#ifdef DEBUG
	#define MSG(string, args...) \
		printk(KERN_DEBUG "%s:%s:%d: " string, \
			MODULE_NAME, __FUNCTION__, __LINE__, ##args)
#else
	#define MSG(string, args...)
#endif


/* read callback */
static 
int hello_procread(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	int n, *p;

	n = sprintf(buf, "In proc read callback.\n\
PID %d running on processor # %d\n",
		current->pid, smp_processor_id());

#if 1
	/* lets generate an OOPS here...
 	 * Observed that on RHEL4 AS (kernel 2.6.9-34.ELsmp), this would often 
	 * result in a complete system hang, whereas on SLED10 and RHEL4 with a 
 	 * vanilla kernel, the Oops would occur.
 	 */
	p=0;
	printk (KERN_INFO "%s: %d\n", MODULE_NAME, 
		*p); /* dereferencing a NULL pointer! 
	OR:
	*p = n;
	 would also cause the Oops
	*/
#endif

	*eof=1;
	return n;
}


/*
 * function hello_oops_init
 */
static int __init hello_oops_init_module(void)
{
	if (!create_proc_read_entry (procname, 0444, /* mode */
		NULL, 		/* parent dir is /proc */
		hello_procread, 	/* read callback */
		NULL )) { 	/* client data or tag */
			MSG("Proc entry creation failure, aborting..\n");
			return -ENOMEM;
	}

	printk(KERN_INFO "%s %s initialized\n", MODULE_NAME, MODULE_VER);
	return 0;	/* success */
}

/*
 * function hello_oops_cleanup
 */
static void hello_oops_cleanup_module(void)
{
	remove_proc_entry (procname, NULL);
	printk(KERN_INFO "%s %s removed\n", MODULE_NAME, MODULE_VER);
}

module_init(hello_oops_init_module);
module_exit(hello_oops_cleanup_module);

MODULE_AUTHOR("Kaiwan NB");
MODULE_DESCRIPTION("A very simple Oops! dump demo");
MODULE_LICENSE("GPL");

/* End kp_hello_oops.c */
