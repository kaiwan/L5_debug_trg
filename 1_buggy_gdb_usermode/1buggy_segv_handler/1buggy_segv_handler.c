/*
 * 1_buggy_using-gdb-userspace.c
 * 
 * Intended as an exercise for participants.
 *
 * This program has bugs.
 * a) Find and resolve them using the GNU debugger gdb.
 * b) Then create a patch.
 *
 * The purpose of this simple program is:
 *  Given a pathname as a parameter, it queries and prints some
 *  information about it (retrieved from it's inode using the 
 *  stat(2) syscall).
 * 
 * (c) Kaiwan NB, kaiwanTECH
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

static void myfault(int signum, siginfo_t * siginfo, void *rest)
{
	static int c = 0;

	printf("*** %s: [%d] received signal %d. errno=%d\n"
	       " Cause/Origin: (si_code=%d): ",
	       __func__, ++c, signum, siginfo->si_errno, siginfo->si_code);

	switch (siginfo->si_code) {
	case SI_USER:
		printf("user\n");
		break;
	case SI_KERNEL:
		printf("kernel\n");
		break;
	case SI_QUEUE:
		printf("queue\n");
		break;
	case SI_TIMER:
		printf("timer\n");
		break;
	case SI_MESGQ:
		printf("mesgq\n");
		break;
	case SI_ASYNCIO:
		printf("async io\n");
		break;
	case SI_SIGIO:
		printf("sigio\n");
		break;
	case SI_TKILL:
		printf("t[g]kill\n");
		break;
		// other poss values si_code can have for SIGSEGV
	case SEGV_MAPERR:
		printf("SEGV_MAPERR: address not mapped to object\n");
		break;
	case SEGV_ACCERR:
		printf("SEGV_ACCERR: invalid permissions for mapped object\n");
		break;
		/* SEGV_BNDERR and SEGV_PKUERR result in compile failure ??
		 * Qs asked on SO here:
		 * https://stackoverflow.com/questions/45229308/attempting-to-make-use-of-segv-bnderr-and-segv-pkuerr-in-a-sigsegv-signal-handle
		 */
#if 0
	case SEGV_BNDERR:	/* 3.19 onward */
		printf("SEGV_BNDERR: failed address bound checks\n");
	case SEGV_PKUERR:	/* 4.6 onward */
		printf
		    ("SEGV_PKUERR: access denied by memory-protection keys\n");
#endif
	default:
		printf("-none-\n");
	}
	printf(" Faulting addr=%p\n", siginfo->si_addr);

	fprintf(stderr,
		"------------------------------------------------------------\n");
	psiginfo(siginfo, "psiginfo helper");
	fprintf(stderr,
		"------------------------------------------------------------\n");

	pause();

	/* 
	 * Placeholders for real-world apps:
	 *  crashed_write_to_log();
	 *  crashed_perform_cleanup();
	 *  crashed_inform_enduser();
	 *
	 * Now have the kernel generate the core dump by:
	 *  Reset the SIGSEGV to glibc default, and,
	 *  Re-raise it!
	 */
	if (signal(SIGSEGV, SIG_DFL) == SIG_ERR) {
		fprintf(stderr, "signal -reverting SIGSEGV to default- failed\n");
		exit(EXIT_FAILURE);
	}
	if (raise(SIGSEGV)) {
		fprintf(stderr, "raise SIGSEGV failed\n");
		exit(EXIT_FAILURE);
	}
}

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
	if (!ss.ss_sp){
		printf("malloc(%zu) for alt sig stack failed\n", stack_sz);
		return -ENOMEM;
	}

	ss.ss_size = stack_sz;
	ss.ss_flags = 0;
	if (sigaltstack(&ss, NULL) == -1){
		printf("sigaltstack for size %zu failed!\n", stack_sz);
		return -errno;
	}
	//printf("Alt signal stack uva (user virt addr) = %p\n", ss.ss_sp);

	return 0;
}

main(int argc, char **argv)
{
	struct sigaction act;
	struct stat statbuf;
	char cmd[128];

	if (argc != 2) {
		fprintf (stderr, "Usage: %s filename\n");
		exit (1);
	}
	printf("%s:%d\n", argv[0], getpid());

	/* Use a separate stack for signal handling via the SA_ONSTACK;
	 * This is critical, especially for handling the SIGSEGV; think on it, what
	 * if this process crashes due to stack overflow; then it will receive the
	 * SIGSEGV from the kernel (when it attempts to eat into unmapped memory
	 * following the end of the stack)! The SIGSEGV signal handler must now run
	 * But where will *its* stack frame reside? It cannot reside on the old stack
	 * - it's now corrupt! Hence, the need for an alternate signal stack.
	 */
	if (setup_altsigstack(10*1024*1024) < 0) {
		fprintf(stderr, "%s: setting up alt sig stack failed\n", argv[0]);
		exit(1);
	}

	memset(&act, 0, sizeof(act));
	act.sa_sigaction = myfault;
	act.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGSEGV, &act, 0) == -1) {
		perror("sigaction");
		exit(1);
	}

	/* Retrieve the information using stat(2) */
	stat(argv[1], &statbuf);
		/* Notice we have not checked the failure case for stat(2)!
		 * It can be a bad bug; just try passing an invalid file as
		 * parameter to this program and see what happens :)
		 */
	/* 
	 * Lets print the foll reg the file object:
	 *  inode #, size in bytes, modification time
	 *  uid, gid
	 *  optimal block size, # of blocks
	 */
	printf("%s:\n"
		" inode number: %d : size (bytes): %d : mtime: %s\n"
		" uid: %d gid: %d\n"
		" blksize: %d blk count: %d\n",
		argv[1], statbuf.st_ino, 
		statbuf.st_size, ctime(statbuf.st_mtime),
		statbuf.st_uid, statbuf.st_gid, 
		statbuf.st_blksize, statbuf.st_blocks);
	
	exit(0);
}
