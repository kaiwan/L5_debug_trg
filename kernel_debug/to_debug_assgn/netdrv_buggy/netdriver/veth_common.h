/*
 * veth_common.h
 *
 * Common header for the 'Virtual Ethernet' "veth" network driver 
 * project [veth_dyn_ops].
 * 
 */
#ifndef __VETH_COMMON_H__
#define __VETH_COMMON_H__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/timer.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/debugfs.h>

#include "../convenient.h"

#define DRVNAME		"vnet"
#define INTF_NAME	"veth3" // 'veth1' is the intf emulated by the 'static' 
							// packet generation ver of this driver

#define DUMP_PKTS   0
#define	Print_hex_dump_bytes(prefix_str, prefix, dataptr, len)    do { \
	if (DUMP_PKTS)  \
		print_hex_dump_bytes (prefix_str, prefix, dataptr, len); \
} while (0)

#define MAXPKTSZ	1500

/* 
 * SKB_PEEK : glean info about the passed socket buffer, esp it's memory (n/w packet).
 * Ref: http://vger.kernel.org/~davem/skb_data.html
 * Minor note: The printk o/p of print_hex_dump_bytes may not appear straight away 
 * (on minicom) due to the KERN_ loglevel it's written at; use dmesg to see everything...  
 */
#define SKB_PEEK(skb) do { \
	PRINT_CTX(); \
	MSG_SHORT("////////////////////////// SKB_PEEK (Ctx: <-- %s. Intr? %c. Prcs: %s(%d))\n"   \
 	"skb ptr: 0x%08x\n" \
	" len=%u truesize=%u users=%d\n" \
	" mac_header: 0x%08x network_header: 0x%08x transport_header: 0x%08x\n" \
	" head: 0x%08x -  end:0x%08x :entire packet buffer (%d bytes)\n"    \
	" data: 0x%08x - tail:0x%08x :packet payload (%d bytes)\n",    \
	__FUNCTION__, (in_interrupt()?'y':'n'), current->comm, current->pid, \
	(unsigned int)skb, \
	skb->len, skb->truesize, atomic_read(&skb->users), \
	(unsigned int)skb->mac_header, (unsigned int)skb->network_header, (unsigned int)skb->transport_header, \
	(unsigned int)skb->head, (unsigned int)skb->end, skb->end - skb->head,    \
	(unsigned int)skb->data, (unsigned int)skb->tail, skb->tail - skb->data    \
	);    \
	print_hex_dump_bytes (" ", DUMP_PREFIX_ADDRESS, skb->head, (skb->end - skb->head));    \
	MSG_SHORT("////////////////////////\n");    \
} while(0)


typedef struct VirtNetIntfCtx {
	struct net_device *netdev;
	int txpktnum, rxpktnum;
	int tx_bytes, rx_bytes;
	unsigned int data_xform;
	spinlock_t lock;
} stVnetIntfCtx, *pstVnetIntfCtx;

extern u32 xform_data_fn;
#endif
