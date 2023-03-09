/* 
 * pnc = panic notifier chain
 * Sample LKM for panic handler notification via the panic notifier chain.
 * Kaiwan NB, kaiwanTECH
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/panic_notifier.h> // updated!

MODULE_LICENSE("Dual MIT/GPL");

//------------------------------
static int my_panic_notifier_call(struct notifier_block *nb,
				  unsigned long code, void *_param)
{
	pr_emerg("Panic !ALARM! @ %s_%s_%d\n", __FILE__, __func__, __LINE__);
	return NOTIFY_OK;
}

static struct notifier_block k_panic_notifier_block = {
	.notifier_call = my_panic_notifier_call,
	.next = NULL,
	.priority = INT_MAX
};

static int __init pnc_init(void)
{
	// Register our panic handler with the kenrel's panic notifier list
	atomic_notifier_chain_register(&panic_notifier_list, &k_panic_notifier_block);
	pr_debug("Regd panic notifier.\n");
	return 0;
}
static void __exit pnc_exit(void)
{
	// Unregister our panic handler with the kenrel's panic notifier list
	atomic_notifier_chain_unregister(&panic_notifier_list, &k_panic_notifier_block);
}

module_init(pnc_init);
module_exit(pnc_exit);
