#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual MIT/GPL");

#define TOOBIG  (8*PAGE_SIZE)
static void	oops_overflow(void)
{
	char localbuf[TOOBIG];
	int i;
	
	pr_info("Populating stack/local buf, size %ld bytes now...", TOOBIG);
	for (i=0; i<TOOBIG; i++)
		localbuf[i] = 'x';
	//memcpy(localbuf, (int)'x', TOOBIG);
}

static int __init stkovf_init(void)
{
	pr_info("Hello, stack overflow\n");
	oops_overflow();

	return 0; // success
}

static void __exit stkovf_exit(void)
{
	pr_info("Goodbye\n");
}

module_init(stkovf_init);
module_exit(stkovf_exit);
