/*
 * helper_jkret.c v1.0 <date>
 *
 * Kernel module that will be passed the address of a kernel module's function.
 * The job of this "helper" module is to setup the jprobe and kretprobe given 
 * the address.
 *
 * This way we capture the given function just prior to invocation (via the
 * jprobe) as well as just after it returns (via the kretprobe)!
 * A classic use-case is to capture the __kmalloc ; hence we can track 
 * allocations, and see if they fail..
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kprobes.h>

#define MODULE_VER 		"0.1"
#define MODULE_NAME    		"helper_jkret"

static char * funcname;

/*----------------------------- Module stuff */
/* module_param (var, type, sysfs_entry_permissions); 
 *  0 in last => no sysfs entry 
 */
module_param(funcname, charp, 0);
MODULE_PARM_DESC(funcname, 
	"Function name of the target LKM's function to attach probes to.");

/*
 * This probe runs just prior to "funcptr()" .
 * The signature MUST match that of the function being pre-probed.
 * (in this case it's __kmalloc).
 *
 * Careful: you'll very quickly using (large amounts of) space in the kernel 
 * logfile..unload this module soon.
 * Unless...We use the printk_ratelimit() to manage this, so no problem!
 */
static void * handler_pre(size_t sz, gfp_t flags)  /***************************
	NOTE AGAIN! This signature MUST be updated for
	any other function. Current: __kmalloc() . 
	******************************************************************/
{
	char intr='.';

	if (printk_ratelimit()) {
		//printk (KERN_INFO "pre_handler: process %s:%d kmalloc'ing %d bytes. [IntrCtx? %c]\n",
		if (in_interrupt()) {
			if (in_irq())
				intr='h';
			else if (in_softirq())
				intr='s';
		}
		pr_info("%s(%d):%ld:%c ", current->comm, current->tgid, sz, intr);
	}
	jprobe_return(); // must
	return 0;  // not reached.
}

static struct jprobe jpb = {
	.entry =  (kprobe_opcode_t *)handler_pre,
};

/* Return probe handler */
static int ret_handler(struct kretprobe_instance *kri, struct pt_regs *regs)
{
	u32 retval = regs_return_value(regs);
	pr_info("                                         0x%x\n", retval);
	//printk_ratelimited(KERN_INFO "                                         0x%x\n", retval);
	//printk (KERN_INFO "%s returns 0x%lx\n", funcname, retval);
	if (!retval)
		pr_warn(" *** FAILURE indicated!!\n");
	return 0;
}

static struct kretprobe kret = {
	.handler = ret_handler,
	.maxactive = 20  // max concurrent instances to handle
};

/*
 * function hello_oops_init
 */
static int __init helper_kp_init_module(void)
{
	int ret=0;

/*	if (!funcptr) {
		printk(KERN_INFO "helper_kp: Must pass funcptr \
address as a module parameter\n");
		return -EINVAL;
	}
*/
	if (!funcname) {
		printk(KERN_INFO "helper_kp: Must pass funcname \
as a module parameter\n");
		return -EINVAL;
	}
	/********* Possible SECURITY concern:
 	 * We just assume the pointer passed is valid and okay.
 	 */
	//printk (KERN_INFO "For funcname : %s\n", funcname);
	/* Register the handler */
	jpb.kp.symbol_name = kret.kp.symbol_name = funcname;
	//jpb.kp.addr = kret.kp.addr = (kprobe_opcode_t *)funcptr;

	if ((ret = register_jprobe(&jpb)) < 0) {
		printk ("register_jprobe failed, returned %d\n", ret);
		return ret;
	}
	if ((ret = register_kretprobe (&kret)) < 0) { 
		printk ("register_kretprobe failed, returned %d\n", ret);
		unregister_jprobe (&jpb);
		return ret;
	}
	printk (KERN_INFO "helper_kp: registered jprobe (and kretprobe) for function %s\n"
	"Format:\n"
	"Name:TGID:Size alloc:Interrupt status\n"
	" Retval\n",
		funcname);

	return 0;	/* success */
}

/*
 * function hello_oops_cleanup
 */
static void helper_kp_cleanup_module(void)
{
	unregister_kretprobe(&kret);
	unregister_jprobe(&jpb);
	printk(KERN_INFO "Unregistered. %s %s removed\n", MODULE_NAME, MODULE_VER);
}

module_init(helper_kp_init_module);
module_exit(helper_kp_cleanup_module);

MODULE_AUTHOR("Kaiwan NB <kaiwan@kaiwantech.com>");
MODULE_DESCRIPTION("Helper Kprobe module; registers a jprobe and kretprobe at the passed function.");
MODULE_LICENSE("GPL");
