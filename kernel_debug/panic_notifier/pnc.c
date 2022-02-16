/* 
 * pnc = panic notifier chain
 * Sample LKM for panic handler notification via the panic notifier chain.
 * Kaiwan NB, kaiwanTECH
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <asm-generic/bug.h>

MODULE_LICENSE("Dual BSD/GPL");

//------------------------------
static int my_panic_notifier_call(struct notifier_block *nb,
				  unsigned long code, void *_param)
{
	pr_emerg("Panic !ALARM! @ %s_%s_%d\n", __FILE__, __FUNCTION__, __LINE__);
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
}

module_init(pnc_init);
module_exit(pnc_exit);
