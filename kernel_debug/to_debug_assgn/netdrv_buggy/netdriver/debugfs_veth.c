/*
 * debugfs_veth.c
 *
 * Debugfs interface code for the 'Virtual Ethernet' "veth" network driver 
 * project [veth_dyn_ops].
 * Refer: Documentation/filesystems/debugfs.txt
 *
 * Kaiwan NB
 */
#include "veth_common.h"

#define DBGFS_CREATE_ERR(pDentry, str) do {  \
		printk("%s: failed.\n", str);     \
		if (PTR_ERR(pDentry) == -ENODEV)     \
			printk(" debugfs support not available?\n");     \
		debugfs_remove_recursive(pDentry);	\
		return (pDentry);     \
} while (0)

u32 xform_data_fn=0;
struct dentry *parent=NULL;

struct dentry * setup_debugfs_entries(void)
{
	QP;
	parent = debugfs_create_dir(DRVNAME, NULL);
	if (!parent) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_dir");
	}

	if (!debugfs_create_u32("xform_data_func", 0644, parent, &xform_data_fn)) {
		DBGFS_CREATE_ERR(parent, "debugfs_create_u32 ");
	}

	return parent;
}
