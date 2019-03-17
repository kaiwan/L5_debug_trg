/*
 * kt1.c
 * Simple demo of a kernel thread.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include "../../convenient.h"

#define MODULE_NAME	"kt"
struct task_struct *gKthrd;

/* Our simple kernel thread. */
static int mykt(void *arg)
{
	MSG("name: %s PID: %d TGID: %d\n",
		current->comm, current->pid, current->tgid);
	if (!current->mm)
		MSG("mm field NULL.\n");

	allow_signal(SIGINT);
	allow_signal(SIGQUIT);

	while (1) {
		MSG("K Thread %d going to sleep now...\n", current->pid);
		set_current_state (TASK_INTERRUPTIBLE);
		schedule();	// and yield the processor...
		// we're back on! has to be due to a signal
		if (signal_pending (current))
			break;
	}
	// We've been interrupted by a signal...
	set_current_state (TASK_RUNNING);
	MSG("K thread %d exiting now...\n", current->pid);
	do_exit(0);
	BUG(); // do_exit() should never return
	return 0;
}

static int kthred_init(void)
{
	int ret=0, i=0;

	MSG("Create a kernel thread...\n");
	gKthrd = kthread_run(mykt, NULL, "%s.%d", MODULE_NAME, i);
		/* 2nd arg is (void * arg) to pass, ret val is task ptr on success */
	if (ret < 0) {
		pr_err("kt1: kthread_create failed (%d)\n", ret);
		return ret;
	}
	get_task_struct(gKthrd); // inc refcnt, "take" the task struct

	MSG("Module %s initialized, thread task ptr is %p.\n\
See the new k thread 'mykt' with ps (and kill it with SIGINT or SIGQUIT)\n", 
		MODULE_NAME, gKthrd);
	return 0;
}

static void kthred_exit(void)
{
	if(gKthrd) 
		kthread_stop(gKthrd);
	put_task_struct(gKthrd); // dec refcnt, "release" the task struct
	MSG("Module %s unloaded.\n", MODULE_NAME);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple demo of a kernel thread.");

module_init(kthred_init);
module_exit(kthred_exit);
