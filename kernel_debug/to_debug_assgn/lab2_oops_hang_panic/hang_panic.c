/*
 * hang_panic.c v1.0 <date>
 *
 *
 * Simple kernel module that delays for a second during init,
 * creates a proc file; when the proc file is read, a timeout
 * is setup (for 1 second). Once timeout occours, the function
 * 'ding' is called.
 *
 * But, of course, it has a bug (or bugs). Read, run, test, 
 * debug, correct, verify, repeat.
 *
 * (c) kaiwan.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

#define MODULE_VER	"1.0"
#define MODULE_NAME    "hang_panic"
#define procname	"lab2_test"

static int ding(unsigned long dope)
{
	printk(KERN_INFO 
		"%s: In timeout function \"ding\" now...dope=%ld\n",
		MODULE_NAME, dope);
/*
info: PID %d, interrupt-context: %d, processor # %d\n",
	MODULE_NAME, dope, current->pid, 
	in_interrupt() ? 1:0, smp_processor_id());
*/
	return 0;
}


/* read callback */
static int my_procread(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	int n;
	struct timer_list timer;

	n = sprintf(buf, "In proc read callback.\n\
PID %d running on processor # %d\n",
		current->pid, smp_processor_id());

	/* Setup a timeout */
	printk(KERN_INFO "%s setting up timer (1s) now..\n", 
		MODULE_NAME);
	init_timer (&timer);
	timer.expires = jiffies+HZ;	/* 1 sec */
	timer.function = ding;
	timer.data = jiffies;
	add_timer (&timer);
	
//msleep(100);
//add_timer (&timer);

	*eof=1;
	return n;
}

/*
 * function hang_panic_init
 */
static int __init hang_panic_init_module(void)
{

	if (!create_proc_read_entry (procname, 0444, /* mode */
		NULL, 		/* parent dir is /proc */
		my_procread, 	/* read callback */
		NULL )) { 	/* client data or tag */
			printk(KERN_ALERT "Proc entry creation failure, aborting..\n");
			return -ENOMEM;
	}

	/* Delay for 1 second; just for the heck of it */
	printk(KERN_INFO "%s delaying for 1s now..\n", MODULE_NAME);
	set_current_state (TASK_INTERRUPTIBLE);
	if (schedule_timeout (HZ)) {
		printk (KERN_INFO 
		"%s: schedule_timeout returned non-zero (signal interruption?)",
			MODULE_NAME);
		return -ERESTARTSYS;
	}

	printk(KERN_INFO "%s %s initialized\n",
		MODULE_NAME, MODULE_VER);
	return 0;	// success
}

/*
 * function hang_panic_cleanup
 */
static void hang_panic_cleanup_module(void)
{
	printk(KERN_INFO "%s %s removed\n",
		MODULE_NAME, MODULE_VER);
}

module_init(hang_panic_init_module);
module_exit(hang_panic_cleanup_module);

MODULE_AUTHOR("My Name");
MODULE_DESCRIPTION("A simple module demo where a timer is set up.");
MODULE_LICENSE("GPL");

/* End hang_panic.c */
