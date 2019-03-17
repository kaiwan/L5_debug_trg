/*
 * Virtual Ethernet - veth - NIC driver.
 *
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * This version has a few deliberate bugs.
 * Run, test, catch and fix 'em!
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * This network driver creates a "virtual interface" 'veth2' on the system.
 *
 * Tx:
 * A user-land datagram socket app sends network packet(s) to the interface.
 * the "packet sniffing" in the transmit routine confirms this.
 *
 * Rx:
 * As a demo, and since this is not real hardware and therefore cannot
 * actually generate Rx interrupts, we "emulate" network Rx by having the 
 * transmit method artificially invoke the receive method/"interrupt". The
 * Rx code then delivers a proper network packet to the kernel protocol stack.
 *
 * The IP network packet is emulated; in this version, we dynamically generate it!
 * The to-be-Rx packet looks like this:

Len:
 Abs:    0                16                         36           44           56
 Rel:      2     14                    20                   8           x 
         +-----------------------------------------------------------------------+
         |  |  Eth II Hdr  |         IPv4 Hdr         |  UDP Hdr   |  Data       |
         +-----------------------------------------------------------------------+
         ^  ^                                                             ^      ^
         |  |                                                             |      |
skb-> head data                                                         tail   end

 *
 * This artificially constructed IP packet is then sent up the protocol stack..
 * The POC lies in the fact that a user-land (datagram socket) app doing a 
 * recvfrom(2) on the appropriate port, actually receives this packet!
 * 
 *
 * Interesting aside:
 * Why not explore (the Linux network stack) further by using in-built hooks in the 
 * IP layer, namely, NETFILTER Hooks? 
 * (Try out the kernel module '1nf.ko' (or 2nf) found in ../netfilter folder above).
 *
 * Kaiwan NB <kaiwan@designergraphix.com>
 * [L]GPL / BSD.
 */
#include "veth_common.h"

static pstVnetIntfCtx gpstCtx=NULL;
static void vnet_rx(unsigned long, struct sk_buff *);

/* 
   The Tx entry point.
   Runs in interrupt - softirq - context; to be precise, the 
   NET_TX_SOFTIRQ context (fyi, resolves to the core routine 
    net/core/dev.c:net_tx_action).

 To get to the Tx path, try:
 - running our "custom" simple datagram app "talker_dgram"
    ./talker_dgram 10.10.1.x "<some msg>"
 - 'ping -c1 10.10.1.x' , with 'x' != IP addr (5, in our current setup;
   also but realize that ping won't actually work on a local interface).
   [TODO- make ping work by, in the Rx method, sending an artificial ICMP_REPLY 
   packet back up the stack].

 Use a network analyzer (eg Wireshark) to see packets flowing across the interface.. 
*/
static int tx_pkt_count=0, rx_pkt_count=0;
static int vnet_start_xmit (struct sk_buff *skb, struct net_device *dev)
{
	struct iphdr *ip;
	struct udphdr *udph;
	pstVnetIntfCtx pstCtx = netdev_priv (dev);
	//struct timespec ts1, ts2, tdiff;
	struct timeval ts1, ts2, tdiff;

	if (!skb) { // paranoia!
		printk (KERN_ALERT "%s:%s: skb NULL!\n", DRVNAME, __func__);
		return -EAGAIN;
	}
//	SKB_PEEK(skb);

/*
   The to-be-Tx packet:
Len:      2       14                    20                   8           x 
         +----------------------------------------------------------------------+
         |  |  Eth II Hdr  |         IPv4 Hdr         |  UDP Hdr   |  Data      |
         +----------------------------------------------------------------------+
         ^  ^                                                            ^      ^
         |  |                                                            |      |
skb-> head data                                                        tail   end

*/
#if 0  // just for testing the time the spinlock is held...
	spin_lock(&pstCtx->lock);
	do_gettimeofday(&ts1);
#endif

	/*---------Packet Filtering :) --------------*/
	/* If the outgoing packet is not of the UDP protocol, discard it */ 
	ip = ip_hdr(skb);
	MSG("proto: 0x%x: ", ip->protocol);
	if (ip->protocol != 0x11) { // not UDP proto?
		MSG("not UDP,disregarding pkt..\n");
		//spin_unlock(&pstCtx->lock);
		goto out_tx;
	}
//	SKB_PEEK(skb);

	/* If the outgoing packet (of protocol UDP) does not have destination port=54295,
	 * discard it.
	 */
	udph = udp_hdr(skb);
	if (ntohs(udph->dest) != 0xd417) {
		MSG("other port, disregarding pkt..\n");
		//spin_unlock(&pstCtx->lock);
		goto out_tx;
	}
	//------------------------------

	printk("ah, a UDP packet with dest port 54295!\n");

#if 0
	MSG("data payload:\n"); // it's after the Eth + IP + UDP headers
	print_hex_dump_bytes (" ", DUMP_PREFIX_OFFSET, skb->head+16+20+8, skb->tail - skb->data);
#endif

	spin_lock(&pstCtx->lock);
	do_gettimeofday(&ts1);
	//getnstimeofday(&ts1);

	pstCtx->txpktnum = ++tx_pkt_count;
	pstCtx->tx_bytes += skb->len;

	//getnstimeofday(&ts2);
	do_gettimeofday(&ts2);
	spin_unlock(&pstCtx->lock);

	if (1 == timeval_subtract (&tdiff, &ts2, &ts1))
		MSG("Warning: timeval_subtract returned -ve!\n");
	else
		MSG("spin lock hold time: %lds %ldus\n", tdiff.tv_sec, tdiff.tv_usec);

	MSG("Emulating Rx by artificially invoking vnet_rx() now...\n");
	vnet_rx((unsigned long)dev, skb);
out_tx:
	dev_kfree_skb (skb);
	return 0;
}

#define TMOUT_SEC 5 //60

/*
 * Calculate & return the IP header checksum.
 * Src: http://www.unix.com/programming/117551-calculate-ip-header-checksum-manually.html
 *
 * Essentially, the IP checksum is the 1's complement of the 16-bit sum of all the 16-bit 
 * octet pairs in the IP header.
 * (Nice explanation of how to arrive at IP csum given at the url above).
 * 
 * ptr: pointer to the buffer of u16's to cal the checksum of (basically, the IP hdr ptr)
 * length: # of bytes in 'ptr'
 *
 * Remember, we're holding the spinlock.. 
 */
u16 ip_checksum(u16 *ptr, int length)
{
	int sum = 0;
	u16 answer = 0;
	u16 *w = ptr;
	int nleft = length;

	while(nleft > 1){
		sum += *w++;
		nleft -= 2;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	answer = ~sum;
	return(answer);
}

static u8 veth_MAC_addr[6] = {0x48, 0x0F, 0x0E, 0x0D, 0x0A, 0x02};
		/* http://curreedy.com/stu/nic/ shows that '480f0e' is not taken!
                 *  fyi, oui = Organizationally Unique Identifier (or 'Company_ID') 
 		 *             = first 3 bytes of MAC addr
		 * Details: http://stackoverflow.com/questions/5580394/mac-address-vs-ip-address
		 */
enum {
	DATA_XFORM_ECHO = 0,
	DATA_XFORM_REVERSE,
	DATA_XFORM_ENCODE,
	DATA_XFORM_METAPKTINFO,
} e_data_xform;

/* Remember, we're holding the spin lock... */
static inline void xform_data(void *genptr, int data_xform, 
						unsigned char *tx_data, int tx_datalen, struct sk_buff *tx_skb)
{
	int	i, dataoff=ETH_HLEN+sizeof(struct iphdr)+sizeof(struct udphdr); /* 14+20+8=42 */

	switch (data_xform) {
	case DATA_XFORM_ECHO:
		memcpy (genptr+dataoff, tx_data, tx_datalen);
		break;
	case DATA_XFORM_REVERSE:
		for (i=0; i<tx_datalen; i++)
			*(u8 *)(genptr+dataoff+i) = tx_data[tx_datalen-i-1];
		break;
	case DATA_XFORM_ENCODE:
		for (i=0; i<tx_datalen; i++)
			*(u8 *)(genptr+dataoff+i) = tx_data[i]+1; // silly ;-p
		break;

	// send the packet metadata; not really a "transform" like the rest...
	case DATA_XFORM_METAPKTINFO:
		MSG ("s->data:%x len %u \n", (u32)tx_skb->data, tx_datalen);
		print_hex_dump_bytes (" ", DUMP_PREFIX_OFFSET, tx_skb->data, tx_datalen);
//		memcpy (genptr+dataoff, tx_skb->data, tx_datalen);
		for (i=0; i<tx_datalen; i++)
			*(u8 *)(genptr+dataoff+i) = tx_skb->data[i];
		break;

	default: MSG("Whoops! default case? Fallback to 'ECHO' xform..\n");
		memcpy (genptr+dataoff, tx_data, tx_datalen);
		break;
	}

	return;
}

/* Remember, we're holding the spin lock... */
static inline void populate_data(void *genptr, struct sk_buff *tx_skb, int tx_datalen)
{
	unsigned char *tx_data=0;
	struct net_device *dev = tx_skb->dev;
	pstVnetIntfCtx pstCtx =  netdev_priv (dev);

	//SKB_PEEK(tx_skb);
	tx_data = skb_transport_header(tx_skb)+sizeof(struct udphdr);
	MSG("data_xform=%d tx_data=0x%08x datalen=%d\n", 
		xform_data_fn, (u32)tx_data, tx_datalen);

	pstCtx->data_xform=xform_data_fn; //  xform_data_fn is the debugfs variable
	xform_data(genptr, pstCtx->data_xform, tx_data, tx_datalen, tx_skb);
}

/* 
 * build_packet
 *
 * We artificially "construct" an incoming IP packet and send it up the protcol 
 * stack, from where it will ultimately reach the userland app blocking on it..
 * Build the IP network packet dynamically, as shown below:
 *
 * See the header diagrams and/or wireshark o/p; the fields 
 * are commented below..
 *
   The to-be-Rx (by userland app) packet:
Len:
 Abs:    0   2             16                         36           44           44+x
 Rel:        0             14                         34           42           42+x
 Len:      2       14                    20                   8           x 
         +----------------------------------------------------------------------+
         |  |  Eth II Hdr  |         IPv4 Hdr         |  UDP Hdr   |  Data      |
         +----------------------------------------------------------------------+
         ^  ^                                                            ^      ^
         |  |                                                            |      |
skb-> head data                                                        tail   end

    The skb_put() (called in the vnet_rx()) updates the skb 'tail' moving it to 
    the reqd location..
 *
 * Return val: length of the packet
 *
 * Remember, we're holding the spinlock.. 
 */
static int build_packet(u8 *rx_pkt, struct sk_buff *tx_skb)
{
	int i, j;
	u8 src_hwaddr[6] = {0x00, 0x82, 0x56, 0xc0, 0x00, 0x39}; /* arbitrary */
	int ipoff=ETH_HLEN, /* 14 (2-byte padding will be added later into the skb) */ 
		udpoff=ETH_HLEN+sizeof(struct iphdr), /* 14+20=34 */
		dataoff=ETH_HLEN+sizeof(struct iphdr)+sizeof(struct udphdr); /* 14+20+8=42 */
	void *genptr = rx_pkt;
	u16 csum=0;

	unsigned char *tx_data = skb_transport_header(tx_skb)+sizeof(struct udphdr);
	int tx_datalen; // = tx_skb->tail-tx_data;

	if (xform_data_fn != DATA_XFORM_METAPKTINFO)
		tx_datalen = tx_skb->tail-tx_data;
	else
		tx_datalen = tx_skb->tail-tx_skb->data;

  //--------------- Ethernet Header (14 octets   :offset 0)
	// Dest HW Address
	for (i=0; i<ETH_ALEN; i++) // ETH_ALEN is 6
		rx_pkt[i] = veth_MAC_addr[i];
	// Src HW Address
	for (i=ETH_ALEN, j=0; j<ETH_ALEN; i++, j++)
		rx_pkt[i] = src_hwaddr[j];
	// type (IP packet = 0x0800)
	*(u16 *)(genptr+ETH_HLEN-2) = cpu_to_be16(0x0800); // protocol headers are big-endian

  //--------------- IP v4 Header (20 octets)   :offset 14
	*(u16 *)(genptr+ipoff+0) = cpu_to_be16(0x4500); /* ver, hdr len, TOS */

	// IP/UDP total len = IPhdr+UDPhdr+Data = 20+8+?? = ??
	*(u16 *)(genptr+ipoff+2) = cpu_to_be16(sizeof(struct iphdr)+sizeof(struct udphdr) + tx_datalen);

	*(u32 *)(genptr+ipoff+4) = 0x4000; // Fragment flags, offset 
					   // (1st 3 bits relevant here; = 0x0100 => (Don't Fragment))
	*(u8 *) (genptr+ipoff+8) = 0x40; // TTL
	*(u8 *) (genptr+ipoff+9) = 0x11; // protocol; 0x11 = 17 = UDP
	*(u16 *)(genptr+ipoff+10) = 0x0; // IP hdr checksum; set to 0 for now..
	*(u32 *)(genptr+ipoff+12) = cpu_to_be32(0x0a0a0135); // Src IP addr = 10.10.1.53 (arbit)
	*(u32 *)(genptr+ipoff+16) = cpu_to_be32(0x0a0a0105); // Dest IP addr = 10.10.1.5
	
	// Calculate & update IP hdr checksum
	csum = ip_checksum(genptr+ipoff, sizeof(struct iphdr));
	//MSG("csum=0x%x\n", ntohs(csum));
	*(u16 *)(genptr+ipoff+10) = csum; // IP hdr checksum

  //--------------- UDP Header (8 octets)   :offset 14+20=34
	*(u16 *)(genptr+udpoff+0) = cpu_to_be16(0xd417); //  Src port = 0xd417 = 54295
	*(u16 *)(genptr+udpoff+2) = cpu_to_be16(0xd417); // Dest port = 0xd417 = 54295
	*(u16 *)(genptr+udpoff+4) = cpu_to_be16(sizeof(struct udphdr)+tx_datalen); 
							// len: UDP hdr len + data len =  8+tx_datalen bytes
	*(u16 *)(genptr+udpoff+6) = cpu_to_be16(0x2421); // UDP Checksum: validation disabled

  //--------------- Data (payload): <tx_datalen> octets   :offset 14+20+8=42
	populate_data(genptr, tx_skb, tx_datalen);
	wmb();
#if 0
	print_hex_dump_bytes (" ", DUMP_PREFIX_OFFSET, rx_pkt, 
							ETH_HLEN+sizeof(struct iphdr)+sizeof(struct udphdr)+tx_datalen);
#endif
	return (dataoff+tx_datalen);
}

/* The receive - Rx - routine.
   Here, it's "artificially" invoked by the Tx method.

   Therefore it runs in interrupt - softirq - context; to be precise, the 
   NET_TX_SOFTIRQ context. Normally (in the case of a typical nic driver), 
   this Rx routine would be the top half (hard IRQ) of the interrupt - the 
   interrupt handler.
 */
static void vnet_rx(unsigned long data, struct sk_buff *tx_skb)
{
	struct net_device *dev = (struct net_device *)data;
	pstVnetIntfCtx pstCtx =  netdev_priv (dev);
	u8 *rx_pkt=NULL; // the to-be-built Rx IP packet
	struct sk_buff *skb=NULL;
	int len=MAXPKTSZ;

QP;
	spin_lock(&pstCtx->lock);
		/* FIXME- Lock's granularity is quite coarse..improve it? */

	// Build the n/w packet
	rx_pkt = kzalloc(MAXPKTSZ, GFP_KERNEL);
	if (!rx_pkt) {
		printk(KERN_ALERT "%s: kmalloc failure!\n", DRVNAME);
		spin_unlock(&pstCtx->lock);
		return;
	}

	len = build_packet(rx_pkt, tx_skb);
	MSG("len=%u\n",len);

	//------------------------Setup a Rx SKB to point to the incoming packet
	skb = netdev_alloc_skb(dev, len);
	if (!skb) {
		printk (KERN_ALERT "%s:%s: out of memory\n", DRVNAME, __func__);
		kfree(rx_pkt);
		spin_unlock(&pstCtx->lock);
		return;
	}

	memcpy(skb->data, rx_pkt, len);
	kfree(rx_pkt);

	skb_push(skb, len); /* add data to the skb */

	skb_set_network_header(skb, ETH_HLEN);
	skb_set_transport_header(skb, ETH_HLEN+sizeof(struct iphdr));
	skb->dev = dev;
	skb->protocol = eth_type_trans (skb, dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY;

//	SKB_PEEK(skb);

	// Send the packet up the protocol stack, (and finally to the app)
	netif_rx (skb);

	pstCtx->rxpktnum = ++rx_pkt_count;
	pstCtx->rx_bytes += skb->len;

	spin_unlock(&pstCtx->lock);
	return;
}

static int vnet_open (struct net_device *dev)
{
	QP;

	netif_carrier_on (dev);
	netif_start_queue (dev);
	return 0;
}
static int vnet_stop (struct net_device *dev)
{
	//pstVnetIntfCtx pstCtx = netdev_priv (dev);
	QP;

	netif_stop_queue (dev);
	netif_carrier_off (dev);
	return 0;
}

// Do an 'ifconfig veth1' to see the effect of this 'getstats' routine..
static struct net_device_stats ndstats;
static struct net_device_stats* vnet_get_stats(struct net_device *dev)
{
	pstVnetIntfCtx pstCtx = netdev_priv (dev);

	spin_lock(&pstCtx->lock);
	ndstats.rx_packets = pstCtx->rxpktnum;
	ndstats.rx_bytes = pstCtx->rx_bytes;
	ndstats.tx_packets = pstCtx->txpktnum;
	ndstats.tx_bytes = pstCtx->tx_bytes;
	spin_unlock(&pstCtx->lock);

	return (&ndstats);
}

static void vnet_tx_timeout(struct net_device *dev)
{
	MSG("!! Tx timed out !!\n");
}

static const struct net_device_ops vnet_netdev_ops = {
        .ndo_open               = vnet_open,
        .ndo_stop               = vnet_stop,
        .ndo_get_stats          = vnet_get_stats,
//        .ndo_do_ioctl           = vnet_ioctl,
        .ndo_start_xmit         = vnet_start_xmit,
        .ndo_tx_timeout         = vnet_tx_timeout,
};

/*
FIXME: [  579.273860] Device 'veth2.0' does not have a release() function, it is broken and must be fixed.
*/

static int vnet_probe(struct platform_device *pdev)
{
	struct net_device *dev=NULL;
	int res=0;

QP;
	dev = alloc_etherdev (sizeof (stVnetIntfCtx));
	if (!dev) {
		printk (KERN_ALERT "%s: alloc_etherdev failed!\n", DRVNAME);
		return -ENOMEM;
	}
#if 0
	SET_NETDEV_DEV(dev, &pdev->dev); /* we can't do this here, as the 'pdev->dev' 
    is a pointer to the 'device' structure, not the net_device ... 
	So, to make this work, (being able to lookup the net_device struct at will), 
    we *need* one global pointer to our "driver context"; this is 'gpstCtx' & is 
    initialized below.. */
#endif

	ether_setup (dev);
	strcpy (dev->name, INTF_NAME);
	memcpy (dev->dev_addr, veth_MAC_addr, sizeof (veth_MAC_addr));

#if 0
	MAC addr..
ndo methods:
	open
	xmit
	stop
	get_stats
	do_ioctl ?

	iomem addr
	irq
#endif

    if (!is_valid_ether_addr (dev->dev_addr)) {
          MSG("%s: Invalid ethernet MAC address. Please set using ifconfig\n",
          dev->name);
    }

	/* keep the default flags, just add NOARP */
    dev->flags           |= IFF_NOARP;
    //dev->features        |= NETIF_F_NO_CSUM;

	dev->watchdog_timeo = 8*HZ;
	spin_lock_init(&gpstCtx->lock);

	/* Initializing the netdev ops struct is essential; else, we get an Oops.. */
	dev->netdev_ops = &vnet_netdev_ops;

	if ((res = register_netdev (dev))) {
		printk (KERN_ALERT "%s: failed to register net device!\n", DRVNAME);
		goto out_regnetdev_fail;
	}
	gpstCtx->netdev = dev;
	
	netif_stop_queue (dev);
	netif_carrier_off (dev);
	return 0;

out_regnetdev_fail:
	free_netdev (dev);
	return res;
}
static int vnet_remove(struct platform_device *pdev)
{
	struct net_device *dev=gpstCtx->netdev;

QP;
	netif_carrier_off (dev);
	netif_stop_queue (dev);
	unregister_netdev (dev);
	free_netdev (dev);
	return 0;
}

/* 
 * Setup a bare-bones platform device & associated driver.
 * Platform devices get bound to their driver simply on the basis of the 'name' field;
 * if they match, the driver core "binds" them, invoking the 'probe' routine. Conversely, the 
 * 'remove' method is invoked at unload/shutdown.
 * Done here mainly so that we have a 'probe' method that will get invoked on it's registration. 
 */
static struct platform_device veth0 = {
        .name          = DRVNAME,
        .id            = 0,
};
static struct platform_device * veth_platform_devices[] __initdata = {
        &veth0,
};
static struct platform_driver virtnet = {
	.probe   = vnet_probe,
	.remove  = vnet_remove,
	.driver  = {
		.name = DRVNAME,
		.owner = THIS_MODULE,
	},
};


struct dentry * setup_debugfs_entries(void);
static struct dentry *dbgfs_parent;

static int __init vnet_init(void)
{
	int res=0;
	MSG ("%s: Initializing network driver...\n", DRVNAME);

	gpstCtx = kzalloc (sizeof (stVnetIntfCtx), GFP_KERNEL);
	if (!gpstCtx) {
		printk (KERN_ALERT "%s:%s kzalloc failed!\n", DRVNAME, __func__);
		return -ENOMEM;
	}

 	if ((res = platform_add_devices(veth_platform_devices, ARRAY_SIZE(veth_platform_devices)))) {
		printk(KERN_ALERT "%s: platform_add_devices failed!\n", DRVNAME);
		goto out_fail_pad;
	}

	res = platform_driver_register (&virtnet);
	if (res) {
		printk(KERN_ALERT "%s: platform_driver_register failed!\n", DRVNAME);
		goto out_fail_pdr;
	}
	// successful platform_driver_register() will cause the registered 'probe' 
	// method to be invoked now..

	dbgfs_parent = setup_debugfs_entries();
	if (!dbgfs_parent) {
		printk(KERN_ALERT "%s: debugfs setup failed!\n", DRVNAME);
		res = PTR_ERR(dbgfs_parent);
		goto out_fail_dbgfs;
	}

	printk(KERN_INFO "%s: loaded.\n", DRVNAME);
	return res;

out_fail_dbgfs:
	platform_driver_unregister (&virtnet);
out_fail_pdr:
	platform_device_unregister (&veth0);
out_fail_pad:
	kfree (gpstCtx);
	return res;
}
static void __exit vnet_exit(void)
{
	debugfs_remove_recursive(dbgfs_parent);
	platform_driver_unregister (&virtnet);
	platform_device_unregister (&veth0);
	kfree (gpstCtx);
	printk(KERN_INFO "%s: unloaded.\n", DRVNAME);
}

module_init(vnet_init);
module_exit(vnet_exit);

MODULE_DESCRIPTION("Simple demo virtual ethernet (NIC) driver; dynamically generates an Rx packet \
based on a 'transform' debugfs variable's value.");
MODULE_AUTHOR("Kaiwan NB <kaiwan@designergraphix.com>");
MODULE_LICENSE("GPL");
