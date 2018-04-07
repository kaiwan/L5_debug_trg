/*
 * kp_intr.c
 *
 * Example kprobes module.
 * We insert a kprobe invoked pre- timer_interrupt() . We could easily
 * add useful instrumentation here..
 * Shows that we can set up a probe even in interrupt context.
 *
 * kaiwan.
 *
 * TODO
 * Make this generic; a shell script will take the name of the kernel routine to 
 * add a probe (pre-handler); the script will determine it's kernel (virt) address
 * and pass it as a parameter to this module, which will register the kprobe.
 *
 *  Done! See the 'helper_kp' kernel module.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/kprobes.h>

static struct kprobe kpb = {
	.symbol_name	= "timer_interrupt",
};

/*
 * This probe runs just prior to the timer interrupt.
 *
 * Careful: you'll start quickly using space in the kernel logfile..unload
 * this module soon.
 * We use the printk_ratelimit() to manage this, so no problem!
 */
static int handler_pre_timerintr(struct kprobe *p, struct pt_regs *regs)
{
	if (printk_ratelimit())
		printk(KERN_INFO "kprobe timer interrupt pre_handler: intr ctx = %d\
 :process %s:%d\n",
			in_interrupt() ? 1 : 0, current->comm, current->tgid);
	return 0;
}

int kp_sched_init_module(void)
{
   kpb.pre_handler = handler_pre_timerintr;
   register_kprobe(&kpb);
   printk("registered kprobe \n");
   return 0;
}

void kp_sched_cleanup_module(void)
{
   unregister_kprobe(&kpb);
   printk("unregister kprobe, and unloaded.\n");
}
module_init(kp_sched_init_module);
module_exit(kp_sched_cleanup_module);

MODULE_AUTHOR("Kaiwan NB");
MODULE_DESCRIPTION("Simple kprobe demo");
MODULE_LICENSE("GPL");

