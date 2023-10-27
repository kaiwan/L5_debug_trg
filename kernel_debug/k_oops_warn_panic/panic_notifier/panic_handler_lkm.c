/**
 * panic_handler_lkm.c
 * A sample LKM for panic handler notification via the panic notifier chain.
 * Kaiwan NB, kaiwanTECH
 */
#include <linux/init.h>
#include <linux/module.h>
#include <asm-generic/bug.h>

// see kernel commit f39650de687e35766572ac89dbcd16a5911e2f0a
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
#include <linux/panic_notifier.h>
#else
#include <linux/notifier.h>
#endif

MODULE_LICENSE("Dual BSD/GPL");

static u32 armsp;
static void arm_getreg(void)
{
#if defined CONFIG_ARM
	__asm__ __volatile__ (
		"mov r3, sp"
		: "=r" (armsp) /* output operands */
		:              /* input operands */
		: "r3");       /* clobbers */
#endif
}

#if 0
#if defined CONFIG_ARM
#define ARM_GETREG(REG) \
do {      \
	__asm__ __volatile__ (                     \
		"mov r3, #REG"                           \
		: "=r" (armsp) /* output operands */                           \
		:              /* input operands */                           \
		: "r3");       /* clobbers */                           \
} while(0)
#endif
#endif


static int my_panic_notifier_call(struct notifier_block *nb,
				  unsigned long code, void *_param)
{
	int loc=5;

	pr_emerg("Panic !ALARM! @ %s_%s_%d\n", __FILE__, __FUNCTION__, __LINE__);
	dump_stack();
#if defined CONFIG_ARM
	arm_getreg();
	pr_emerg("loc=0x%p, ARM stack reg val = 0x%x\n", &loc, armsp);
#endif
	return NOTIFY_OK;
}

static struct notifier_block k_panic_notifier_block = {
	.notifier_call = my_panic_notifier_call,
	.next = NULL,
	.priority = INT_MAX
};

void pnc_register_panic_notifier(void)
{
    atomic_notifier_chain_register(&panic_notifier_list, &k_panic_notifier_block);
	/*
	 * err = notifier_chain_register(&panic_notifier_list, &k_panic_notifier_block);
	 * Fails with:
        error: implicit declaration of function ‘notifier_chain_register’ [-Werror=implicit-function-declaration]
     * atomic_notifier_chain_register() is a simple wrapper over it but is 
	 *  EXPORT_SYMBOL_GPL ! thus giving us access via an LKM if we're GPL.
	 */ 
}

void pnc_unregister_panic_notifier(void)
{
    atomic_notifier_chain_unregister(&panic_notifier_list, &k_panic_notifier_block);
}

static int __init pnc_init(void)
{
	pnc_register_panic_notifier();
	pr_debug("Regd panic notifier.\n");
	return 0;
}
static void __exit pnc_exit(void)
{
	pnc_unregister_panic_notifier();
	pr_debug("Unregd panic notifier.\n");
}

module_init(pnc_init);
module_exit(pnc_exit);
