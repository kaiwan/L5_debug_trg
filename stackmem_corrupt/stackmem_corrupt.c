/*
 * stackmem_corrupt.c
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

static void write_underflow_corruptstack(void)
{
	unsigned long stack, stackptr;
	struct rlimit reslimit;
	char cmd[128], stacksz_hexstr[18], sz[9];
	int stacksz = 0;
	FILE *fp;
	unsigned long stacksz_hex = 0;

	printf("%s(): approx stack top (~ curr sp) is %p\n", __func__, &stack);
	if (prlimit(0, RLIMIT_STACK, 0, &reslimit) < 0) {
		perror("prlimit failed");
		return;
	}
	printf("%s(): stack resource limits: soft=%ld, hard=%ld\n",
		__func__, reslimit.rlim_cur,reslimit.rlim_max);
	
	// Get actual size of stack mapping for this process via /proc/PID/maps
	memset(cmd, 0, 128);
	snprintf(cmd, 128, "./getmapsize %d stack", getpid());
	fp = popen(cmd, "r");
	if (!fp) {
		perror("popen failed");
		return;
	}
	stacksz = atoi(fgets(sz, 127, fp));
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

#if 0
	// OVERWRITE - corrupt - stack memory !
	memset((void *)(stackptr - stacksz_hex), 'x', stacksz);
#endif
	/*
	 * THis of courses causes a segfault.
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
	* Ha! 0x78 is 'x' ! the value we memset into the stack mem region...
	*/

}

int main(int argc, char **argv)
{
	printf("Running %s testcase now...\n", argv[0]);
	write_underflow_corruptstack();
	pause();
	exit(0);
}
