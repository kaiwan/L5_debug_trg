/*
 * hello_gdb.c v1.0 <date>
 *
 * Simple kernel module, that, when inserted into the kernel
 * allocates space for and initializes a global data structure. 
 * This is used to demonstrate the usage of gdb to look up 
 * kernel / module stuff.
 *
 * [L]GPL
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
//#include <linux/proc_fs.h>
#include <linux/slab.h>

#define MODULE_VER 	"1.0"
#define MODULE_NAME    "hello_gdb"
#define procname	"gdb_test"

#ifdef DEBUG
	#define MSG(string, args...) \
		printk(KERN_DEBUG "%s:%s:%d: " string, \
			MODULE_NAME, __FUNCTION__, __LINE__, ##args)
#else
	#define MSG(string, args...)
#endif

/* Some globals -- just here as a demo for looking up using gdb 
   with /proc/kcore as:
  #  gdb <path/to/>/linux-2.6.17/vmlinux /proc/kcore
     ...
 */
typedef struct {
	short tx, rx;
	u32 j;
} MYS;
static MYS *mine;

#if 0
/* read callback */
static int procread(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	int n = sprintf(buf, "In proc read callback.\n\
PID %d running on processor # %d\n",
		current->pid, smp_processor_id());
	
	*eof=1;
	return n;
}
#endif

/*
 * function hello_gdb_init
 */
static int __init hello_gdb_init_module(void)
{
#if 0
	if (!create_proc_read_entry (procname, 0444, /* mode */
		NULL, 		/* parent dir is /proc */
		procread, 	/* read callback */
		NULL )) { 	/* client data or tag */
			MSG("Proc entry creation failure, aborting..\n");
			return -ENOMEM;
	}
#endif
	mine = (MYS *)kmalloc (sizeof(MYS), GFP_KERNEL);
	if (!mine) {
		printk(KERN_ALERT "hello_gdb: kmalloc failed.\n");
		return -ENOMEM;
	}
	mine->tx = mine->rx = 0;
	mine->j = jiffies;

	printk(KERN_INFO "%s %s initialized\n", MODULE_NAME, MODULE_VER);
	return 0;	/* success */
}

/*
 * function hello_gdb_cleanup
 */
static void hello_gdb_cleanup_module(void)
{
//	remove_proc_entry (procname, NULL);
	kfree (mine);
	printk(KERN_INFO "%s %s removed\n", MODULE_NAME, MODULE_VER);
}

module_init(hello_gdb_init_module);
module_exit(hello_gdb_cleanup_module);

MODULE_AUTHOR("Kaiwan");
MODULE_DESCRIPTION("Demo of using gdb with kernel module");
MODULE_LICENSE("GPL");

/* End hello_gdb.c */
