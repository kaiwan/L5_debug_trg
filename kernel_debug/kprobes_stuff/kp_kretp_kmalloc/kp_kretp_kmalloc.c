/*
 * kp_kretp_kmalloc.c
 * Setup a jprobe and a kretprobe on __kmalloc
 * Kaiwan NB, kaiwanTECH.
 */ 
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kprobes.h>
#include <linux/slab.h>
#include "../convenient.h"

MODULE_LICENSE("Dual MIT/GPL");

static void * jp_kmalloc(size_t sz, gfp_t flags)  /* This function signature MUST MATCH jprobed func */
{
	MSG_SHORT ("__kmalloc(%lu) : ", sz);
	PRINT_CTX();
	jprobe_return();
	return 0;
}

static struct jprobe jpb = {
	.entry			= jp_kmalloc,
	.kp = {
		.symbol_name	= "__kmalloc",
	},
};

/* Return probe handler */
static int ret_handler(struct kretprobe_instance *kri, struct pt_regs *regs)
{
	MSG_SHORT("__kmalloc return ptr: 0x%lx : ", regs->ax);
	PRINT_CTX();
	if (regs->ax <= 0)
		printk (KERN_WARNING "    FAILURE indicated!!\n");
	return 0;
}

static struct kretprobe kret = {
	.handler = ret_handler,
	.maxactive = 20  // max concurrent instances to handle
};

static int __init kp_init(void)
{
	int ret=0;

	/* Register the handler */
	if ((ret = register_jprobe(&jpb)) < 0) {
		printk ("register_jprobe failed, returned %d\n", ret);
		return ret;
	}
	kret.kp.symbol_name = "__kmalloc";
	if ((ret = register_kretprobe (&kret)) < 0) { 
		printk ("register_kretprobe failed, returned %d\n", ret);
		unregister_jprobe (&jpb);
		return ret;
	}
	return 0;
}

static void __exit kp_cleanup(void)
{
	unregister_kretprobe(&kret);
	unregister_jprobe(&jpb);
}

module_init(kp_init);
module_exit(kp_cleanup);
