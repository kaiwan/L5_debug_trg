/* 
 * mem2.c
 * 
 * Two types of memory errors here:
 * 1. using uninitialized data
 * 2. buffer underrun and overrun (fence errors or “off-by-one” errors)
 *
 * Memory checking with valgrind..run as:
 * $ valgrind --leak-check=yes -v ./mem2
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>	/* M_PI */

#define SZ	512

void uninit (int n, char *buf)
{
	int i;
	
	switch (n) {
		case 0: puts("Case 0"); 
				/* call something() ... */
				break;
		case 1: puts ("Case 1"); 
				/* call something() ... */
				break;
		case 2:  puts ("Case 2");
				/* call something() ... */
				break;
		default: puts ("Case default.");		
	}
	
	/* ERROR :: buffer underrun and overrun. */
	for (i=-1; i<=SZ; i++)
		buf[i] = i;
}

int main(int argc, char **argv)
{
	int u;
	char *ptr;
	double nono;

	ptr = malloc (SZ);
	if (!ptr) {
		fprintf (stderr, "%s: out of memory\n", argv[0]);
		exit (1);
	}
	
	/*
	 * ERROR :: uninitilaized variable usage.
	 * Interesting..
	 * Only if we *make use* of the value "nono" in any way (like printf),
	 * valgrind detects the error and dumps the
	 * "Conditional jump or move depends on uninitialised value(s)" message.
	 * Buf if we comment out the printf, this is *not* caught.
	 */
	nono = (double)u/M_PI;
#if 0
	printf ("nono = %6.2f\n", nono);
#endif

	/* ERROR :: uninitilaized variable usage.  */
	uninit (u, ptr);
#if 0		/* ERROR :: make this '#if 0' to invoke a memory leak. */
	free (ptr);
#endif
	printf("%s: Done, stayin' alive, stayin' alive ...\n", argv[0]);
	pause();
	uninit (u, ptr);
	exit (0);
}

