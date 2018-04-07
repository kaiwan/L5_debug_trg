/*
 * Here's a sample kernel module showing the use of jprobes to dump
 * the arguments of do_fork() and do_execve().
 *
 * For more information on theory of operation of jprobes, see
 * Documentation/kprobes.txt
 *
 * Build and insert the kernel module as done in the kprobe example.
 * You will see the trace data in /var/log/messages and on the
 * console whenever do_fork() is invoked to create a new process.
 * (Some messages may be suppressed if syslogd is configured to
 * eliminate duplicate messages.)
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/interrupt.h>
#include "convenient.h"

#define MYNAME "jprobe_eg"
/*
 * Jumper probes for do_fork & do_sys_open.
 * Mirror principle enables access to arguments of the probed routine
 * from the probe handler.
 */

/* Proxy routine having the same arguments as actual do_sys_open() routine */
static long jp_do_sys_open(int dfd, const char __user *filename, int flags, int mode)
{
	printk (KERN_INFO "%s:%s:filename: %s", MYNAME, __func__, filename);
	if (!in_interrupt())
		printk (KERN_INFO " [%s]\n", current->comm);
	else
		printk (KERN_INFO "\n");

	//DELAY_SEC(5);
	/* Always end with a call to jprobe_return(). */
	jprobe_return();
	return 0;
}

/* Proxy routine having the same arguments as actual do_fork() routine */
static long jp_do_fork(unsigned long clone_flags, unsigned long stack_start,
	      struct pt_regs *regs, unsigned long stack_size,
	      int __user *parent_tidptr, int __user *child_tidptr)
{
	printk(KERN_INFO "%s:%s:clone_flags = 0x%lx, stack_size = 0x%lx [%s]\n",
	       MYNAME, __func__, clone_flags, stack_size, current->comm);

	/* Always end with a call to jprobe_return(). */
	jprobe_return();
	return 0;
}

static int jp_do_execve(char * filename,
     char __user *__user *argv,
     char __user *__user *envp,
     struct pt_regs * regs)
{
	printk(KERN_INFO "%s:%s:predecessor -> successor: %s -> %s\n", MYNAME, __func__, current->comm, filename);
/*
	if (!in_interrupt())
		printk (KERN_INFO " [%s]\n", current->comm);
	else
		printk (KERN_INFO "\n");
*/

	/* Always end with a call to jprobe_return(). */
	jprobe_return();
	return 0;
}


static struct jprobe my_do_fork_jprobe = {
	.entry			= jp_do_fork,
	.kp = {
		.symbol_name	= "do_fork",
	},
};
static struct jprobe my_vfs_open_jprobe = {
	.entry			= jp_do_sys_open,
	.kp = {
		.symbol_name	= "do_sys_open",
	},
};
static struct jprobe my_do_execve_jprobe = {
	.entry			= jp_do_execve,
	.kp = {
		.symbol_name	= "do_execve",
	},
};

static int __init jprobe_init(void)
{
	int ret;

	ret = register_jprobe(&my_do_fork_jprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_jprobe failed, returned %d\n", ret);
		return -1;
	}

	// Enabling the do_sys_open causes a huge bunch of prints, stressing the ring buffer...
#if 0
	ret = register_jprobe(&my_vfs_open_jprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_jprobe failed, returned %d\n", ret);
		return -1;
	}
#endif
	ret = register_jprobe(&my_do_execve_jprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_jprobe failed, returned %d\n", ret);
		return -1;
	}
	printk(KERN_INFO "%s: Jprobes registered.\n", MYNAME);
	return 0;
}

static void __exit jprobe_exit(void)
{
	//unregister_jprobe(&my_vfs_open_jprobe);
	unregister_jprobe(&my_do_fork_jprobe);
	unregister_jprobe(&my_do_execve_jprobe);
	printk(KERN_INFO "%s: jprobes unregistered\n", MYNAME);
}

module_init(jprobe_init)
module_exit(jprobe_exit)
MODULE_LICENSE("GPL");

