/*
 * oops2.c
 * Slightly les Silly demo - make it Oops!
 * This time, by dereferencing an invalid (null) structure pointer..
 *
 * Kaiwan NB, kaiwanTECH
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "convenient.h"

MODULE_LICENSE("Dual BSD/GPL");

/*
 * On another tangent:
 * The cacheline size seems to be 64 bytes here:
$ getconf -a|grep CACHE_LINESIZE
LEVEL1_ICACHE_LINESIZE             64
LEVEL1_DCACHE_LINESIZE             64
LEVEL2_CACHE_LINESIZE              64
LEVEL3_CACHE_LINESIZE              64
LEVEL4_CACHE_LINESIZE              0
$ 
*/ 
struct faker {
  long longs[7];  // (on x86_64 a 'long' takes 8 bytes, so) use 8*7=56 bytes
  char dumb[7];   // use 56+7=63 bytes; now 1 byte left in this cacheline!
  long bad_cache_align; // use 63+8=71 bytes, thus spilling over the cacheline! Very naughty.
};
struct faker *f1;

static int __init oops2_init(void)
{
	f1 = kmalloc(sizeof(struct faker), GFP_KERNEL);
	if (!f1) {
	  pr_warn("kmalloc f1 failed");
          return -ENOMEM;
        }
	MSG("sizeof(long) = %d, sizeof(struct faker) = %lu, actual space alloced = %lu\n", 
	     sizeof(long), sizeof(struct faker), ksize(f1));

	MSG("Hello, about to Oops!\n");
	f1->dumb[0] = 'a';
	f1->bad_cache_align = 0xabcd;

	return 0;
}

static void __exit oops2_exit(void)
{
	kfree(f1);
	MSG("Goodbye\n");
}

module_init(oops2_init);
module_exit(oops2_exit);
