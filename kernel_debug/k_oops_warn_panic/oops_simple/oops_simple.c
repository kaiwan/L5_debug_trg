/*
 * Silly demo - make it Oops!
 * By dereferencing an invalid (null) structure pointer..
 *
 * Kaiwan NB, kaiwanTECH
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "../../convenient.h"

MODULE_LICENSE("Dual MIT/GPL");

struct faker {
	long longs[7];
	char dumb[7];
	long bad_cache_align;
};
struct faker *f1; // pointers have no memory !

static int __init oops2_init(void)
{
#if 0
	f1 = kmalloc(sizeof(struct faker), GFP_KERNEL);
	if (!f1)
		return -ENOMEM;
	pr_info("sizeof(long) = %ld, sizeof(struct faker) = %lu, actual space alloced = %lu\n",
	    sizeof(long), sizeof(struct faker), ksize(f1));
#else
	pr_info("Hello, about to Oops!\n");
#endif
	f1->bad_cache_align = 0xabcd;
	return 0;
}

static void __exit oops2_exit(void)
{
#if 0
	kfree(f1);
#endif
	pr_info("Goodbye\n");
}

module_init(oops2_init);
module_exit(oops2_exit);
