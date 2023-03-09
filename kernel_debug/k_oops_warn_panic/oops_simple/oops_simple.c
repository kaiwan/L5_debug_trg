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

/*
 * On another tangent:
 * The cpu cacheline size is 64 bytes here:
$ getconf -a|grep CACHE_LINESIZE
LEVEL1_ICACHE_LINESIZE             64
LEVEL1_DCACHE_LINESIZE             64
LEVEL2_CACHE_LINESIZE              64
LEVEL3_CACHE_LINESIZE              64
LEVEL4_CACHE_LINESIZE              0
$
 */
struct faker {
	long longs[7];		// (on x86_64 a 'long' takes 8 bytes, so) use 8*7=56 bytes
	char dumb[7];		// use 56+7=63 bytes; now 1 byte left in this cacheline!
	long bad_cache_align;	// use 63+8=71 bytes, thus spilling over the cacheline! Very naughty.
};
struct faker *f1;

static int __init oops2_init(void)
{
#if 0
	f1 = kmalloc(sizeof(struct faker), GFP_KERNEL);
	if (!f1)
		return -ENOMEM;
	MSG("sizeof(long) = %ld, sizeof(struct faker) = %lu, actual space alloced = %lu\n",
	    sizeof(long), sizeof(struct faker), ksize(f1));
#endif
	MSG("Hello, about to Oops!\n");
	f1->bad_cache_align = 0xabcd;

	return 0;
}

static void __exit oops2_exit(void)
{
#if 0
	kfree(f1);
#endif
	MSG("Goodbye\n");
}

module_init(oops2_init);
module_exit(oops2_exit);
