/* 
 * mysh.c
 * "my simple shell"
 * 
 * Memory checking with valgrind..run as:
 * $ valgrind --leak-check=full -v ./mysh_dbg
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define LEN		128
#define MAXARGS		5	//1024
/*#define MAXARGS 5*/
#define PROMPT	">>> "
#define SEP     " "

static int DBG=1;

#define SHOWARGS() {\
	int i;	\
	if (DBG) { \
		for (i=0;i<MAXARGS;i++)	\
			if (newargs[i]) { \
				printf("\
newargs[%d]     =0x%08x\n\
orig_newargs[%d]=0x%08x\n\n", \
					i, (unsigned)newargs[i],\
					i, (unsigned)orig_newargs[i]);\
			} \
	} \
}

inline void free_args(char **args)
{
	int i;
	for (i=0; i<MAXARGS; i++) {
		if (args[i])
			free (args[i]);
	}
}
	

int main( int argc, char ** argv )
{
	char cmd[LEN];
	/* 
	 * IMPORTANT NOTE:-
	 *
	 * Below, you'll see that we allocate memory (using malloc) to the set
	 * of pointers called 'newargs'. Logically, we should free this memory
	 * when we no longer require it, using free(), preventing any memory 
	 * leakage. We do that. Still, valgrind shows us that there is leakage!
	 *
	 * The problem is this: 
	 * Apparently, the strtok routines (below), break up and *change* 
	 * the 'newarg' pointers. So, we now save them at allocation time in 
	 * 'orig_newargs', and free *these* pointers later, preventing any
	 * leakage.
	 *
	 *------------------------------------------------------------------
	 * From strtok man page:
	 *--snip--
	 BUGS
	  Avoid using these functions.  If you do use them, note that:

              These functions modify their first argument.

              These functions cannot be used on constant strings.
	 *--snip--
	 *------------------------------------------------------------------
	 *
	 * So, attempting to free the pointers on termination results in "free
	 * errors" being caught (by glibc 2.4 and valgrind).
	 * So,as mentioned above, we save the pointers in 'orig_newargs' and free 
	 * *these*, not 'newargs'.
	 *
	 * To test and see this clearly, try reducing MAXARGS to a small number 
	 * (like 5), make mysh_dbg && ./mysh_dbg
	 */
	char *newargs[MAXARGS];
	char *orig_newargs[MAXARGS];
	int i;
	char *exit_str="q";

#ifdef DEBUG
	DBG=1;
#endif

	for(i=0;i<MAXARGS;i++) {   
		newargs[i]=malloc(LEN);
		if( newargs[i] == NULL ) {
			fprintf(stderr,"%s: no memory for newargs[%d]..",argv[0],i);
			exit( 1 );
		}
		orig_newargs[i] = newargs[i];
	}

	while( 1 ) {
		SHOWARGS();

		printf("%s", PROMPT );
		fflush( stdout );

		/* fsgets is safe; gets is not */
		fgets( cmd, LEN, stdin );
		cmd[strlen(cmd)-1]='\0';  /* remove trailing \n */

		if( strcmp(cmd,exit_str) == 0 ) {
				printf("q pressed.\n");
				free_args (orig_newargs);
				exit( 0 );
		}

		if( cmd[0]==0 ) /* try to take care of LF */
			continue;

		/* Tokenize..
		 * warning : strtok is not thread-safe (and, see comment above)..
		 */
		newargs[0]=strtok(cmd, SEP);
		i=1;
		while( (newargs[i++]=strtok(NULL, SEP)) ) ;

		if ((i-2) > MAXARGS) {
			/*printf("# args (i-1)=%d\n", i-1);*/
			fprintf(stderr, "%s: Too many arguments (max is %d), aborting command.\n", 
				argv[0], MAXARGS);
			continue;
		}

		if( DBG ) {
			i=0;
			while( newargs[i] ) {
				printf("newargs[%d]=%s\n",i,newargs[i]);
				i++;
			}
			printf("\n");
		}

		switch( fork() ) {
			case -1 : perror("fork failed");
				break;
			case 0 :  // Child
				free_args (orig_newargs);
				if( execvp( newargs[0], newargs ) == -1 ) {
					perror( "exec failure" );
					exit( 1 );
				}
				/* code never reaches here..*/
			default : // Parent
				if( wait( 0 ) == -1 )
					perror("wait"),exit(1);
		}
	} // while
	/* code never reaches here..*/
} // main()

/*
Running valgrind on this (mysh_dbg - debug ver) with verbose switch on also clearly
shows how the pointers are changing.

$ make && valgrind --leak-check=full -v ./mysh_dbg
==10573== Memcheck, a memory error detector.
==10573== Copyright (C) 2002-2006, and GNU GPL'd, by Julian Seward et al.
==10573== Using LibVEX rev 1658, a library for dynamic binary translation.
==10573== Copyright (C) 2004-2006, and GNU GPL'd, by OpenWorks LLP.
==10573== Using valgrind-3.2.1, a dynamic binary instrumentation framework.
==10573== Copyright (C) 2000-2006, and GNU GPL'd, by Julian Seward et al.
==10573==
--10573-- Command line
--10573--    ./mysh_dbg
--10573-- Startup, with flags:
--10573--    --leak-check=full
--10573--    -v
--10573-- Contents of /proc/version:
--10573--   Linux version 2.6.16.21-0.8-smp (geeko@buildhost) (gcc version 4.1.0 (SUSE Linux)) #1 SMP Mon Jul 3 18:25:39 UTC 2006
--10573-- Arch and hwcaps: X86, x86-sse1-sse2
--10573-- Valgrind library directory: /usr/local/lib/valgrind
--10573-- Reading syms from /lib/ld-2.4.so (0x4000000)
--10573-- Reading syms from /mnt/big1_40GB/trg/linux/DG-L4/dbg_tools/tests/valgrind_test/mysh_dbg (0x8048000)
--10573-- Reading syms from /usr/local/lib/valgrind/x86-linux/memcheck (0x38000000)
--10573--    object doesn't have a dynamic symbol table
--10573-- Reading suppressions file: /usr/local/lib/valgrind/default.supp
--10573-- REDIR: 0x4014CC0 (index) redirected to 0x38027D7B (vgPlain_x86_linux_REDIR_FOR_index)
--10573-- Reading syms from /usr/local/lib/valgrind/x86-linux/vgpreload_core.so (0x401D000)
--10573-- Reading syms from /usr/local/lib/valgrind/x86-linux/vgpreload_memcheck.so (0x401F000)
==10573== WARNING: new redirection conflicts with existing -- ignoring it
--10573--     new: 0x04014CC0 (index     ) R-> 0x04022090 index
--10573-- REDIR: 0x4014E60 (strlen) redirected to 0x4022250 (strlen)
--10573-- Reading syms from /lib/libc-2.4.so (0x404B000)
--10573-- REDIR: 0x40B5230 (rindex) redirected to 0x4021F70 (rindex)
--10573-- REDIR: 0x40B2180 (malloc) redirected to 0x402134B (malloc)
newargs[0]     =0x0416c028
orig_newargs[0]=0x0416c028

newargs[1]     =0x0416c0d8
orig_newargs[1]=0x0416c0d8

newargs[2]     =0x0416c188
orig_newargs[2]=0x0416c188

newargs[3]     =0x0416c238
orig_newargs[3]=0x0416c238

newargs[4]     =0x0416c2e8
orig_newargs[4]=0x0416c2e8

--10573-- REDIR: 0x40B4E80 (strlen) redirected to 0x4022230 (strlen)
>>> ps
#(our comment)---vvvvvvvvv------------------------vvvvvvvvv--------
--10573-- REDIR: 0x40B5BB0 (memchr) redirected to 0x4022420 (memchr)
#(our comment)---^^^^^^^^^------------------------^^^^^^^^^--------
#(our comment) can see how strtok has changed the pointer
--10573-- REDIR: 0x40B65B0 (memcpy) redirected to 0x4022C20 (memcpy)
--10573-- REDIR: 0x40B48F0 (strcmp) redirected to 0x4022300 (strcmp)

newargs[0]=ps

--10574-- REDIR: 0x40AFF00 (free) redirected to 0x4020F65 (free)
--10574-- REDIR: 0x40B4780 (index) redirected to 0x4022060 (index)
--10574-- REDIR: 0x40B5080 (strncmp) redirected to 0x4022290 (strncmp)
--10574-- REDIR: 0x40B6F30 (strchrnul) redirected to 0x40225E0 (strchrnul)
  PID TTY          TIME CMD
 4684 pts/0    00:00:01 bash
10573 pts/0    00:00:00 memcheck
10574 pts/0    00:00:00 ps
newargs[0]     =0xbe9d6940
orig_newargs[0]=0x0416c028

newargs[2]     =0x0416c188
orig_newargs[2]=0x0416c188

newargs[3]     =0x0416c238
orig_newargs[3]=0x0416c238

newargs[4]     =0x0416c2e8
orig_newargs[4]=0x0416c2e8

>>> ls -l -F

newargs[0]=ls

newargs[1]=-l

newargs[2]=-F

--10575-- REDIR: 0x40AFF00 (free) redirected to 0x4020F65 (free)
--10575-- REDIR: 0x40B4780 (index) redirected to 0x4022060 (index)
--10575-- REDIR: 0x40B5080 (strncmp) redirected to 0x4022290 (strncmp)
--10575-- REDIR: 0x40B6F30 (strchrnul) redirected to 0x40225E0 (strchrnul)
total 124
-rw------- 1 kaiwan users 294912 2006-12-31 07:51 core
-rw-r--r-- 1 kaiwan users    382 2006-12-31 07:38 Makefile
-rwxr-xr-x 1 kaiwan users   8844 2006-12-31 07:36 mysh*
-rw-r--r-- 1 kaiwan users   6307 2006-12-31 07:54 mysh.c
-rwxr-xr-x 1 kaiwan users  11474 2006-12-31 07:53 mysh_dbg*
drwxr-xr-x 2 kaiwan users   4096 2006-12-31 07:32 older/
newargs[0]     =0xbe9d6940
orig_newargs[0]=0x0416c028

newargs[1]     =0xbe9d6943
orig_newargs[1]=0x0416c0d8

newargs[2]     =0xbe9d6946
orig_newargs[2]=0x0416c188

newargs[4]     =0x0416c2e8
orig_newargs[4]=0x0416c2e8

>>>
*/

