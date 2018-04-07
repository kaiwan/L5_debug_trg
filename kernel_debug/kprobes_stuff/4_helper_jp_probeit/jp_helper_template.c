/*
 * Jprobes helper framework.
 * This file is (a copy of) a simple "template" for writing your own JProbes.
 *
 * The basic idea:
 * *Copy* this C source file as is, calling it by a different name of course, 
 * and then modify it as instructed below to attach your jprobe(s).
 *
 * ___The good news___ : this work is automated by the helper script jprobeit.sh .
 * It copies the file appropriately (pathname format: 
 *   work_jps/jp_<func>_<date>/jp_<func>_<date>.c
 *
 * ___ The limitation___ : it (currently) cannot modify the jprobe'd function 
 * handler's signature to exactly match that of the function being jprobed.
 *
 * For more information on theory of operation of jprobes, see
 * Documentation/kprobes.txt
 *
 * Original Author: Kaiwan N Billimoria
 * <kaiwan -at- kaiwantech -dot- com>
 * License: GPL
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/interrupt.h>
#include "convenient.h"
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//#include <linux/whatever.h>

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#define MYNAME xxx

/*
 * Jumper probe template for xxx
 *
 * THE THING TO DO:
 * Replace the function signature below such that it exactly matches the 
 * function being jprobed.
 * 
 * Mirror principle enables access to arguments of the probed routine
 * from the probe handler.
 */
static <ret type> jp_xxx( /* ... params ... */ )  //%%%
{
#if 1
	PRINT_CTX();
#endif

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Your Jprobe handler code goes here...


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	/* Always end with a call to jprobe_return(). */
	jprobe_return(); 
	return <something>; //%%%
}


static struct jprobe jps_xxx = {
	.entry			= jp_xxx,
	.kp = {
		.symbol_name	= "xxx",
	},
};

static int __init jprobe_init(void)
{
	int ret;

	ret = register_jprobe(&jps_xxx);
	if (ret < 0) {
		printk(KERN_INFO "register_jprobe failed, returned %d\n"
		"Check: is the to-be-probed function invalid, static, "
		"inline or attribute-marked '__kprobes' ?\n", ret);
		return ret;
	}
	printk(KERN_INFO "%s: Jprobe(s) registered.\n", MYNAME);
	return 0;
}

static void __exit jprobe_exit(void)
{
	unregister_jprobe(&jps_xxx);
	printk(KERN_INFO "%s: Jprobe(s) unregistered\n", MYNAME);
}

module_init(jprobe_init);
module_exit(jprobe_exit);
MODULE_LICENSE("GPL");
