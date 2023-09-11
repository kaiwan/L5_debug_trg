/*
 * oops.c
 * Silly demo - make it Oops!
 * Kaiwan NB, kaiwanTECH
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("Dual BSD/GPL");
char *ptr = NULL; /* checkpatch says: ERROR: do not initialise globals to NULL */

static int __init oops2_init(void)
{
	pr_info("Hello, about to Oops!\n");
	*(ptr + 0x20) = 'a';
	return 0;
}

static void __exit oops2_exit(void)
{
	pr_info("Goodbye\n");
}

module_init(oops2_init);
module_exit(oops2_exit);
