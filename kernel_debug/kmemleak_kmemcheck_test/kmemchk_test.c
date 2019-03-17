/*
 kmemchk_test.c
 Kernel module.
 We've built a custom kernel (v3.18.22) with kmemcheck (CONFIG_KMEMCHECK) 
 enabled. 
 This is a simple module to test kmemcheck; we delibrately leave some memory
 uninitialized and access it.. this should cause kmemcheck to jump up and
 trigger noisy printk's.
 
 Tip: Ensure kmemcheck is currently enabled before testing..
 # echo 1 > /proc/sys/kernel/kmemcheck

 (c) kaiwanTECH
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
//#include "convenient.h"

static int gfp_test(void)
{
    unsigned long kbuf, *kp;
    kbuf = __get_free_page(GFP_KERNEL);
    kp = (unsigned long *)kbuf;
    if (!kp) {
        pr_warn("out of memory!");
        return -ENOMEM;
    }
    pr_info("$$$ gfp_test: kp=%p pa=%p\n", 
        (unsigned int)kp, 
        (unsigned int)virt_to_phys(kp));
    print_hex_dump_bytes("$$$ ", DUMP_PREFIX_ADDRESS, kp, 32);
    free_page(kbuf);
	return 0;
}

static int slab_test(void)
{
    void *kbuf;
    kbuf = kmalloc(512, GFP_KERNEL);
    if (!kbuf) {
        pr_warn("kmalloc: out of memory!");
        return -ENOMEM;
    }
    pr_info("### slab_test: kbuf=%p pa=%p\n", kbuf,
        virt_to_phys(kbuf));
    print_hex_dump_bytes("### ", DUMP_PREFIX_ADDRESS, kbuf, 32);
    kfree(kbuf);
    return 0;
}

static int vmalloc_test(unsigned long len)
{
    void *vbuf;
    vbuf = vmalloc(len);
    if (!vbuf) {
        pr_warn("vmalloc: out of memory!");
        return -ENOMEM;
    }
    pr_info("@@@ vmalloc_test: vmalloc(%u): vbuf=%p pa=%p\n", len, vbuf,
        virt_to_phys(vbuf));
    print_hex_dump_bytes("@@@ ", DUMP_PREFIX_ADDRESS, vbuf, len);
#ifndef KMEMLEAK_TEST
    vfree(vbuf);
#endif
    return 0;
}


static int __init kmemchk_test_init(void)
{
    int local_init=5, local_uninit;

	pr_info("Hi, accessing local vars now: local_init=%d local_uninit=%d\n", 
        local_init, local_uninit);
        /* Obvious bug: even gcc triggers a warning:
        "warning: ‘local_uninit’ is used uninitialized in this function [-Wuninitialized]"
        Even so, kmemcheck CANNOT catch this bug as it's compile-time 
        allocated memory!
        */

//	gfp_test();
//	slab_test();
	vmalloc_test(5*PAGE_SIZE);

    return 0; // success
}

static void __exit kmemchk_test_exit(void)
{
	pr_info("Goodbye\n");
}

module_init(kmemchk_test_init);
module_exit(kmemchk_test_exit);

MODULE_LICENSE("Dual MIT/GPL");
