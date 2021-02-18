/*
 * hang_panic.c v1.0 <date>
 *
 * Simple kernel module that delays for a second during init,
 * and sets up a kernel timer that is setup to expire in 1 second. 
 * Once timeout occours, the function 'ding' is called.
 *
 * But, of course, it has a subtle bug (or bugs). Read, run, test, 
 * debug, correct, verify, repeat.
 *
 * (c) kaiwan.
 * MIT
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

#define MODULE_VER	"1.0"
#define MODULE_NAME    "hang_panic"

static void ding(struct timer_list *timer)
{
	pr_info("%s: In timeout function \"ding\" now...\n", MODULE_NAME);

/*
info: PID %d, interrupt-context: %d, processor # %d\n",
	MODULE_NAME, dope, current->pid, 
	in_interrupt() ? 1:0, smp_processor_id());
	dump_stack();
*/
}

static void try_timeout(void)
{
	struct timer_list timer;

    pr_info("In try_timeout()\n\
PID %d running on processor # %d\n",
		current->pid, smp_processor_id());

	/* Setup a timeout */
	pr_info("%s setting up timer (1s) now..\n", MODULE_NAME);
	timer.expires = jiffies+HZ;	/* 1 sec */
	timer.function = ding;
	timer.flags = 0;
	timer_setup(&timer, ding, 0);

	add_timer(&timer); // start !
}

/*
 * function hang_panic_init
 */
static int __init hang_panic_init_module(void)
{
	/* Delay for 1 second; just for the heck of it */
	pr_info("%s delaying for 1s now..\n", MODULE_NAME);
	set_current_state (TASK_INTERRUPTIBLE);
	if (schedule_timeout (HZ)) {
		printk (KERN_INFO 
		"%s: schedule_timeout returned non-zero (signal interruption?)",
			MODULE_NAME);
		return -ERESTARTSYS;
	}

	pr_info("%s %s initialized\n",
		MODULE_NAME, MODULE_VER);

 	try_timeout();
	return 0;	// success
}

/*
 * function hang_panic_cleanup
 */
static void hang_panic_cleanup_module(void)
{
	//del_timer_sync(&timer);
	pr_info("%s %s removed\n", MODULE_NAME, MODULE_VER);
}

module_init(hang_panic_init_module);
module_exit(hang_panic_cleanup_module);

MODULE_AUTHOR("Kaiwan NB");
MODULE_DESCRIPTION("A simple module demo  - a subtle bug where a timer is set up.");
MODULE_LICENSE("Dual MIT/GPL");
/* End hang_panic.c */
