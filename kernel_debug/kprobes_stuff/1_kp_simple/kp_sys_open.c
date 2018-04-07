/*
 * kp_sys_open.c
 *
 * Example kprobes module for the sys_open routine.
 * From: LDPT, SB pg 325
 * (c) Steve Best
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

static struct kprobe kpb;

static int handler_pre_sys_open(struct kprobe *p, struct pt_regs
*regs)  {
   printk("sys_open pre_handler: p->addr=0x%p\n", p->addr);
   return 0;
}

static void handler_post_sys_open(struct kprobe *p, struct pt_regs
*regs, unsigned long flags){
  printk("post_handler_sys_open: p->addr=0x%p\n", p->addr);
}

static int handler_fault_sys_open(struct kprobe *p, struct pt_regs
*regs, int trapnr) {
    printk("fault_handler_sys_open: p->addr=0x%p\n", p->addr);
    return 0;
}

int init_module(void)
{
   kpb.fault_handler = handler_fault_sys_open;
   kpb.pre_handler = handler_pre_sys_open;
   kpb.post_handler = handler_post_sys_open;

   kpb.symbol_name = "sys_open";
   register_kprobe(&kpb);
   printk(" register kprobe \n");
   return 0;
}

void cleanup_module(void)
{
   unregister_kprobe(&kpb);
   printk("unregister kprobe\n");
}
MODULE_LICENSE("GPL");

