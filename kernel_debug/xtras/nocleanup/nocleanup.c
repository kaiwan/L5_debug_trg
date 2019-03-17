#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int __init hello_init(void)
{
	printk(KERN_ALERT "Hello, world\n");
#if 1
	return 0;
#endif
}

static void __exit hello_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
#if 0
module_exit(hello_exit);
#endif

/*
Indeed, it's not possible to rmmod / cleanup now!
# lsmod|grep nocleanup
nocleanup               8239  0 [permanent]
# 
# rmmod nocleanup 
ERROR: Removing 'nocleanup': Device or resource busy
# 
*/
