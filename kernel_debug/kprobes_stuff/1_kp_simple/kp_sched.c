/*
 * kp_sys_open.c
 *
 * Example kprobes module.
 * We insert a kprobe invoked pre- schedule() . We could easily
 * add useful instrumentation here..
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
	.symbol_name	= "schedule",
};

/*
 * This probe runs just prior to schedule() . In effect, current will be
 * running the scheduler at this time..
 *
 * Careful: you'll start quickly using space in the kernel logfile..unload
 * this module soon.
 * We use the printk_ratelimit() to manage this, so no problem!
 */
static int handler_pre_schedule(struct kprobe *p, struct pt_regs *regs)
{
	if (printk_ratelimit())
		printk(KERN_INFO "kprobe schedule pre_handler: intr ctx = %d :process %s:%d\n",
			in_interrupt() ? 1 : 0, current->comm, current->tgid);
	return 0;
}

#if 0
static void handler_post_schedule(struct kprobe *p, struct pt_regs
*regs, unsigned long flags){
  printk("post_handler_schedule: p->addr=0x%p\n", p->addr);
}
#endif

int kp_sched_init_module(void)
{
   kpb.pre_handler = handler_pre_schedule;
//   kpb.post_handler = handler_post_schedule;

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

