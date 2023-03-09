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
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

#define MODULE_VER	"1.0"
#define MODULE_NAME    "hang_panic"

//static void ding(unsigned long dope)
static void ding(struct timer_list *timer)
{
	pr_info("In timeout function \"ding\" now...\n");
/*
	pr_info("PID %d, interrupt-context: %d, processor # %d\n",
	current->pid, 
	in_interrupt() ? 1:0, raw_smp_processor_id());
	dump_stack();
*/
}

static void try_timeout(void)
{
	struct timer_list mytimer;

    pr_info("In try_timeout()\n\
PID %d running on processor # %d\n",
		current->pid, smp_processor_id());

	/* Setup a timeout */
	pr_info("%s setting up timer (1s) now..\n", MODULE_NAME);

	/* >= 4.15 : init_timer(), etc replaced:
	timer_setup(timer, callback, flags) */
	timer_setup(&mytimer, ding, 0);
	/* timer expires when the jiffies var reaches this value */
	mytimer.expires = msecs_to_jiffies(1000); /* 1000ms = 1s */
	pr_info("Arming the timer now!\n");
	add_timer(&mytimer);
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

	pr_info("initialized\n");
 	try_timeout();

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

MODULE_AUTHOR("Kaiwan NB");
MODULE_DESCRIPTION("A simple module demo  - a subtle bug where a timer is set up.");
MODULE_LICENSE("Dual MIT/GPL");
