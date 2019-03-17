/*
 * convenient.h
 *
 * A few convenience routines..
 *
 * Author: Kaiwan N Billimoria <kaiwan@kaiwantech.com>
 * Released under the terms of the MIT License.
 *  https://en.wikipedia.org/wiki/MIT_License
 */
#ifndef __CONVENIENT_H__
#define __CONVENIENT_H__

#include <asm/param.h>		/* HZ */
#include <linux/sched.h>

/*------------------------ MSG, QP ------------------------------------*/

#ifdef __KERNEL__
#include <linux/interrupt.h>
#define PRINT_CTX() {        \
  if (printk_ratelimit()) { \
	  printk("PRINT_CTX:: in function %s on cpu #%2d\n", __func__, smp_processor_id()); \
      if (!in_interrupt()) \
	  	printk(" in process context: %s:%d\n", current->comm, current->pid); \
	  else \
        printk(" in interrupt context: in_interrupt:%3s. in_irq:%3s. in_softirq:%3s. in_serving_softirq:%3s. preempt_count=0x%x\n",  \
          (in_interrupt()?"yes":"no"), (in_irq()?"yes":"no"), (in_softirq()?"yes":"no"),        \
          (in_serving_softirq()?"yes":"no"), preempt_count());        \
  } \
}
#endif

#ifdef DEBUG
#ifdef __KERNEL__
#define MSG(string, args...) \
		 printk(KERN_INFO "%s:%d : " string, __FUNCTION__, __LINE__, ##args)
#else
#define MSG(string, args...) \
		fprintf(stderr, "%s:%d : " string, __FUNCTION__, __LINE__, ##args)
#endif

#ifdef __KERNEL__
#define MSG_SHORT(string, args...) \
			printk(KERN_INFO string, ##args)
#else
#define MSG_SHORT(string, args...) \
			fprintf(stderr, string, ##args)
#endif

#define QP MSG("\n")

#ifdef __KERNEL__
#define QPDS do { \
		 MSG("\n"); \
		 dump_stack(); \
	  } while(0)
#define PRCS_CTX do { \
		 if (!in_interrupt()) { \
			MSG("prcs ctx: %s(%d)\n", current->comm, current->pid); \
		 } \
		 else { \
			MSG("irq ctx\n"); \
			PRINT_IRQCTX();   \
		} \
	  } while(0)
#endif

#ifdef __KERNEL__
#define HexDump(from_addr, len) \
 	    print_hex_dump_bytes (" ", DUMP_PREFIX_ADDRESS, from_addr, len);
#endif
#else
#define MSG(string, args...)
#define MSG_SHORT(string, args...)
#define QP
#define QPDS
#endif

/*------------------------ assert ---------------------------------------*/
#ifdef __KERNEL__
#define assert(expr) \
if (!(expr)) { \
 printk("********** Assertion [%s] failed! : %s:%s:%d **********\n", \
  #expr, __FILE__, __func__, __LINE__); \
}
#endif

/*------------------------ DELAY_LOOP --------------------------------*/
static inline void beep(int what)
{
#ifdef __KERNEL__
	(void)printk(KERN_INFO "%c", (char)what);
#else
	(void)printf("%c", (char)what);
#endif
}

/* 
 * DELAY_LOOP macro
 * @val : ASCII value to print
 * @loop_count : times to loop around
 */
#define DELAY_LOOP(val,loop_count) \
{ \
	int c=0, m;\
	unsigned int for_index,inner_index; \
	\
	for(for_index=0;for_index<loop_count;for_index++) { \
		beep((val)); \
		c++;\
			for(inner_index=0;inner_index<HZ*1000*8;inner_index++) \
				for(m=0;m<50;m++); \
		} \
	/*printf("c=%d\n",c);*/\
}
/*------------------------------------------------------------------------*/

#ifdef __KERNEL__
/*------------ DELAY_SEC -------------------------*
 * Delays execution for n seconds.
 * MUST be called from process context.
 *------------------------------------------------*/
#define DELAY_SEC(val) \
{ \
	if (!in_interrupt()) {	\
		set_current_state (TASK_INTERRUPTIBLE); \
		schedule_timeout (val * HZ); \
	}	\
}
#endif

#ifndef __KERNEL__
int timeval_subtract(struct timeval *, struct timeval *, struct timeval *);
void dec2bin(long, char *);
int r_sleep(time_t, long);
int err_exit(char *, char *, int);
void hex_dump(unsigned char *data, int size, char *caption, int verbose);

/* Verbose printf ... */
#define VP(verbose, str, args...) \
do { \
	if (verbose) \
		printf(str, ##args); \
} while (0)
#endif

#endif
