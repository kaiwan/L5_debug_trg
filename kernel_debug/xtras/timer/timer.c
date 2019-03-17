/*
 * timer.c
 *
 * Simple kernel module that delays for a second during init,
 * is setup (for 1 second). Once timeout occours, the function
 * 'ding' is called.
 *
 * (c) kaiwan.
 * GPL / LGPL
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include "../../convenient.h"

#define MODULE_VER	 "1.0"
#define MODULE_NAME  "mytimer"

static int tmout_sec=1;
module_param (tmout_sec, int, 0);
MODULE_PARM_DESC (tmout_sec, "Number of seconds to timeout in [default=1].");

static struct timer_list mytimer;

static void ding(unsigned long dope)
{
#if 1
	PRINT_CTX();

/*
   printk(KERN_INFO "Context: in_interrupt:%3s in_irq:%3s in_softirq:%3s in_serving_softirq:%3s preempt_count=0x%x\n",
          (in_interrupt()?"yes":"no"), (in_irq()?"yes":"no"), (in_softirq()?"yes":"no"),
          (in_serving_softirq()?"yes":"no"), preempt_count()); */
#else
QP;
#endif

#if 0
	printk(KERN_INFO 
		"%s: In timeout function \"%s\" now...dope=%lu\n",
		MODULE_NAME, __func__, dope);
#endif

	// make it cyclic
	init_timer (&mytimer);
	mytimer.function = ding;
	mytimer.data = 2; //(int)mytimer.data + 1;
	mytimer.expires = jiffies + (tmout_sec*HZ);
	//mytimer.expires = jiffies + msecs_to_jiffies(tmout_sec*1000);
	add_timer(&mytimer);
}


/*
 * function timer_init
 */
static int __init timer_init_module(void)
{
#if 0
	/* Delay for 1 second; just for the heck of it */
	printk(KERN_INFO "%s delaying for 1s now..\n", MODULE_NAME);
	set_current_state (TASK_INTERRUPTIBLE);
	if (schedule_timeout (HZ)) {
		printk (KERN_INFO 
		"%s: schedule_timeout returned non-zero (signal interruption?)",
			MODULE_NAME);
		return -ERESTARTSYS;
	}
#endif

	pr_info("%s %s initialized. tmout_sec=%d\n"
		MODULE_NAME, MODULE_VER, tmout_sec);

	/* Setup a timeout */
	printk(KERN_INFO "%s: setting up timer (%ds) now..\n", 
		MODULE_NAME, tmout_sec);

	init_timer (&mytimer);
	mytimer.expires = jiffies + (tmout_sec*HZ);
	//mytimer.expires = jiffies + msecs_to_jiffies(tmout_sec*1000);
	mytimer.function = ding;
	mytimer.data = 1; //jiffies;
	add_timer (&mytimer);
	return 0;	// success
}

/*
 * function timer_cleanup
 */
static void timer_cleanup_module(void)
{
	del_timer_sync (&mytimer);
	pr_info("%s %s removed\n", MODULE_NAME, MODULE_VER);
}

module_init(timer_init_module);
module_exit(timer_cleanup_module);

MODULE_AUTHOR("Kaiwan NB");
MODULE_DESCRIPTION("A simple module demo where a timer is set up.");
MODULE_LICENSE("GPL");
/* End timer.c */
