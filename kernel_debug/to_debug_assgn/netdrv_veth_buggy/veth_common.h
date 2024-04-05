/*
 * veth_common.h
 * Common header for the 'Virtual Ethernet' "veth" network driver project
 */
#ifndef __VETH_COMMON_H__
#define __VETH_COMMON_H__

#ifdef __KERNEL__
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

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
#include "convenient.h"

#define DEBUG // want 'debug' mode On

#define PORTNUM     54295 // the port users will be connecting to
#define INTF_NAME  "veth"

#define DRVNAME		"vnet"
#define DUMP_PKTS	1
#define	Print_hex_dump_bytes(prefix_str, prefix, dataptr, len)    do { \
	if (DUMP_PKTS)  \
		print_hex_dump_bytes(prefix_str, prefix, dataptr, len); \
} while (0)

#define MAXPKTSZ	1500

/*
 * SKB_PEEK : glean info about the passed socket buffer, esp it's memory (n/w packet).
 * Ref: http://vger.kernel.org/~davem/skb_data.html
 */
#define SKB_PEEK(skb) do { \
	PRINT_CTX(); \
	pr_debug("////////////////////////// SKB_PEEK\n"   \
	"skb ptr: %px\n" \
	" len=%u truesize=%u users=%d\n" \
	" Offsets: mac_header:%u network_header:%u transport_header:%u\n" \
	" SKB packet pointers & offsets:\n" \
	"  headroom : head:%px - data:%px [%4ld bytes]\n"    \
	"  pkt data :                         data - tail:%3u       [%4u bytes]\n" \
	"  tailroom :                                tail - end:%3u [%4u bytes]\n",    \
	skb, \
	skb->len, skb->truesize, refcount_read(&skb->users), \
	skb->mac_header, skb->network_header, skb->transport_header, \
	skb->head, skb->data, \
	(skb->data-skb->head), /* headroom len */ \
	skb->tail, \
	skb->len,              /* pkt data len */ \
	skb->end, \
	(skb->end-skb->tail)   /* tailroom len */ \
	);    \
	if (skb) { \
		pr_info("FREEING !!!\n"); \
	  dev_kfree_skb(skb); \
	} \
	pr_debug("_______============_____\n");    \
	pr_debug("skb ptr: %px\n" \
	" len=%u truesize=%u users=%d\n", \
		skb, skb->len, skb->truesize, refcount_read(&skb->users));\
	print_hex_dump_bytes(" ", DUMP_PREFIX_OFFSET, skb->head, skb->end); \
} while (0)
//	pr_debug("////////////////////////\n");
	/* print_hex_dump_bytes() dumps at KERN_DEBUG if DEBUG is defined  */
	/* Buggy
	  print_hex_dump_bytes(" ", DUMP_PREFIX_OFFSET, skb->head, skb->tail - skb->end); 
            abv: UAF !
	fine:
	print_hex_dump_bytes(" ", DUMP_PREFIX_OFFSET, skb->head, skb->end); 
	*/

/*static struct {
	struct net_device *netdev;
	int txpktnum, rxpktnum;
	int tx_bytes, rx_bytes;
	unsigned int data_xform;
	spinlock_t lock;
} stVnetIntfCtx;*/
#endif /* kernel */
#endif
