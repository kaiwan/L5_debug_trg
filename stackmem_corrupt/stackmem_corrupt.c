/*
 * stackmem_corrupt.c
 *
 * Here, we run a testcase to cause more or less complete overwrite of stack
 * memory - stack corruption.
 * We show how using the SIGSEGV handler correctly can reveal the root cause.
 * ...
 * Within the SIGSEGV handler, to get the core dump, we must reset and re-raise
 * SIGSEGV. The problem though with doing it naively, is that it's raised on the
 * same alternate signal stack we're running upon. This will have it unwind the
 * stack there which is pretty meaningless (as the offending code would have run
 * on the stack of the offending thread, here, main (T0).
 * So, we need to reset so as to NOT use the alt signal stack and then
 * re-raise the signal!
 *
 * (c) Kaiwan N Billimoria, kaiwanTECH
 * Feb 2022.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>

static void myfault(int signum, siginfo_t *si, void *ucontext)
{
	fprintf(stderr,
	"\n------------------- FATAL signal ---------------------------\n");
	fprintf(stderr, " %s: received signal %d. errno=%d\n"
	       " Cause/Origin: (si_code=%d): ",
	       __func__, signum, si->si_errno, si->si_code);

	switch (si->si_code) {
	/* Possible values si_code can have for SIGSEGV */
	case SEGV_MAPERR:
		fprintf(stderr, "SEGV_MAPERR: address not mapped to object\n");
		break;
	case SEGV_ACCERR:
		fprintf(stderr, "SEGV_ACCERR: invalid permissions for mapped object\n");
		break;
	/* SEGV_BNDERR and SEGV_PKUERR result in compile failure? */
#if 0
	case SEGV_BNDERR: /* 3.19 onward */
		fprintf(stderr, "SEGV_BNDERR: failed address bound checks\n");
	case SEGV_PKUERR: /* 4.6 onward */
		fprintf(stderr, "SEGV_PKUERR: access denied by memory-protection keys\n");
#endif
	/* Other possibilities for si_code; here just to show them... */
	case SI_USER:
		fprintf(stderr, "user\n");
		break;
	case SI_KERNEL:
		fprintf(stderr, "kernel\n");
		break;
	case SI_QUEUE:
		fprintf(stderr, "queue\n");
		break;
	case SI_TIMER:
		fprintf(stderr, "timer\n");
		break;
	case SI_MESGQ:
		fprintf(stderr, "mesgq\n");
		break;
	case SI_ASYNCIO:
		fprintf(stderr, "async io\n");
		break;
	case SI_SIGIO:
		fprintf(stderr, "sigio\n");
		break;
	case SI_TKILL:
		fprintf(stderr, "t[g]kill\n");
		break;
	default:
		fprintf(stderr, "-none-\n");
	}

	fprintf(stderr, " Faulting instr or address = %p\n", si->si_addr);
	//fprintf(stderr, " --- Register Dump [x86_64] ---\n");
	//dump_regs(ucontext);
	fprintf(stderr,
		"------------------------------------------------------------\n");
	psiginfo(si, "psiginfo helper");
	fprintf(stderr,
		"------------------------------------------------------------\n");

	/*
	 * Placeholders for real-world apps:
	 *  crashed_write_to_log();
	 *  crashed_perform_cleanup();
	 *  crashed_inform_enduser();
	 *
	 * Now, to get the core dump, we must reset and re-raise SIGSEGV. The problem
	 * though with doing it naively, is that it's raised on the same alternate
	 * signal stack we're running upon. This will have it unwind the stack there
	 * which is pretty meaningless (as the offending code would have run on the
	 * stack of the offending thread, here, main (T0).
	 * So, we need to reset so as to NOT use the alt signal stack and then
	 * re-raise the signal!
	 */
	struct sigaction act;

	memset(&act, 0, sizeof(act));
	// use kernel default
	act.sa_handler = SIG_DFL;
	// DON'T use the alt sigstack
	act.sa_flags = SA_RESTART | SA_SIGINFO; // | SA_ONSTACK;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGSEGV, &act, 0) == -1) {
		perror("sigaction");
		exit(1);
	}
	// now re-raise it
	if (raise(SIGSEGV)) {
		fprintf(stderr, "raise SIGSEGV failed"); exit(1); }
}

#define USING_ALTSIGSTACK

/**
 * setup_altsigstack - Helper function to set alternate stack for sig-handler
 * @stack_sz:	required stack size
 *
 * Return: 0 on success, -ve errno on failure
 */
int setup_altsigstack(size_t stack_sz)
{
	stack_t ss;

	printf("Alt signal stack size = %zu bytes\n", stack_sz);
	ss.ss_sp = malloc(stack_sz);
	if (!ss.ss_sp) {
		printf("malloc(%zu) for alt sig stack failed\n", stack_sz);
		return -ENOMEM;
	}

	ss.ss_size = stack_sz;
	ss.ss_flags = 0;
	if (sigaltstack(&ss, NULL) == -1) {
		printf("sigaltstack for size %zu failed!\n", stack_sz);
		return -errno;
	}
	//printf("Alt signal stack uva (user virt addr) = %p\n", ss.ss_sp);

	return 0;
}

static void write_underflow_corruptstack(void)
{
	unsigned long stack, stackptr;
	struct rlimit reslimit;
	char cmd[128], stacksz_hexstr[18], sz[9];
	int stacksz = 0;
	FILE *fp;
	unsigned long stacksz_hex = 0;

	printf("%s(): approx stack top (~ curr sp) is %p\n",
		__func__, (void *)&stack);
	if (prlimit(0, RLIMIT_STACK, 0, &reslimit) < 0) {
		perror("prlimit failed");
		return;
	}
	printf("%s(): stack resource limits: soft=%ld, hard=%ld\n",
		__func__, reslimit.rlim_cur, reslimit.rlim_max);

	// Get actual size of stack mapping for this process via /proc/PID/maps
	memset(cmd, 0, 128);
	snprintf(cmd, 128, "./getmapsize %d stack", getpid());
	fp = popen(cmd, "r");
	if (!fp) {
		perror("popen failed");
		return;
	}
	stacksz = atoi(fgets(sz, 9, fp));
	if (stacksz <= 0) {
		pclose(fp);
		fprintf(stderr, "%s():getting actual stack size failed\n", __func__);
		return;
	}
	pclose(fp);
	printf("%s():actual process (T0 thread) stack size = %d (0x%x) bytes\n",
		__func__, stacksz, stacksz);
	if (stacksz % getpagesize())
		printf("%s():WARNING:actual stack size isn't a rounded page size\n", __func__);

	snprintf(stacksz_hexstr, 18, "0x%x", stacksz);
	stacksz_hex = strtoul(stacksz_hexstr, NULL, 16);
	stackptr = (unsigned long)&stack;
	printf("%s():actual start of stack is 0x%lx\n", __func__, stackptr - stacksz_hex);

#if 1
	// OVERWRITE - and thus corrupt - stack memory !
	memset((void *)(stackptr - stacksz_hex), 'x', stacksz);
#endif
	/*
	 * This of courses causes a segfault.
	 *
	 * WITHOUT the alt signal stack in place, this is what gdb sees as the stack
	 * from the core dump:
	   (gdb) bt
#0  __memset_avx2_unaligned_erms () at ../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:222
#1  0x7878787878787878 in ?? ()
#2  0x7878787878787878 in ?? ()
#3  0x7878787878787878 in ?? ()
#4  0x0000000000000000 in ?? ()
(gdb)
	*
	* 0x78 is 'x' ! the value we memset into the stack mem region...
	*/
	/*
	 * Now using the mechanisms described above in place - initially using the alt
	 * sig stack, then, when the SIGSEGV is raised switching back to the regular
	 * stack and re-raising the signal as default handler, getting the correct
	 * core dump.
	 * This time - with the caveat that we use the debug ver with symbolic info -
	 * we can clearly see the root cause of the crash - the stack memory corruption
	 * caused by the memset() ! (see frame #0).
	 *
	# gdb -q -c 'corefile:host=dbg-LKD:gPID=21264:gTID=21264:ruid=0:sig=11:exe=![...]!stackmem_corrupt_dbg' ./stackmem_corrupt_dbg
Reading symbols from ./stackmem_corrupt_dbg...

warning: core file may not match specified executable file.
[New LWP 21264]
Core was generated by `./stackmem_corrupt_dbg'.
Program terminated with signal SIGSEGV, Segmentation fault.
#0  __memset_avx2_unaligned_erms () at ../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:200
200	../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S: No such file or directory.
(gdb) bt
#0  __memset_avx2_unaligned_erms () at ../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:200
#1  0x000055a706d0ab4f in write_underflow_corruptstack () at stackmem_corrupt.c:181
#2  0x000055a706d0acc9 in main (argc=1, argv=0x7ffc052e7308) at stackmem_corrupt.c:239
(gdb)
	*/
}

int main(int argc, char **argv)
{
	struct sigaction act;
#if 1
	// Set up the stack to eat a lot of memory (7 MB)
#define NUM ((7*1024*1024)/sizeof(long long))
	long long longarr[NUM];

	memset(&longarr[0], 0xef, NUM);
#endif

	/* Use a separate stack for signal handling via the SA_ONSTACK;
	 * This is critical, especially for handling the SIGSEGV; think on it, what
	 * if this process crashes due to stack overflow; then it will receive the
	 * SIGSEGV from the kernel (when it attempts to eat into unmapped memory
	 * following the limit, the end of the stack)! The SIGSEGV signal handler
	 * must now run. But where? It cannot on the old stack - it's now corrupt!
	 * Hence, the need for an alternate signal stack !
	 */
	printf("%s: setting up an alt sigstack and trapping SIGSEGV now...\n", argv[0]);
#ifdef USING_ALTSIGSTACK
	if (setup_altsigstack(10*1024*1024) < 0) {
		fprintf(stderr, "%s: setting up alt sig stack failed\n", argv[0]);
		exit(1);
	}
#endif
	memset(&act, 0, sizeof(act));
	act.sa_sigaction = myfault;
	act.sa_flags = SA_RESTART | SA_SIGINFO;
#ifdef USING_ALTSIGSTACK
	act.sa_flags |= SA_ONSTACK;
#endif
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGSEGV, &act, 0) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("Running %s testcase now...\n", argv[0]);
	write_underflow_corruptstack();
	pause();
	exit(0);
}
