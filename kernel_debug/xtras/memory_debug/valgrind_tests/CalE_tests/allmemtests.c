/*
 * THIS SOFTWARE IS NOT COPYRIGHTED
 *
 * MontaVista Software, Inc. offers the following for use in the public
 * domain.  MontaVista makes no warranty with regard to the software or
 * its performance and the user accepts the software "AS IS" with all faults.
 *
 * MontaVista disclaims any warranties, express or implied, with regard to
 * to this software including but not limited to the warranties
 * of merchantability and fitness for a particular purpose.
 *
 * Please contact the author (cal_erickson@mvista.com) for updates or * 
 * to submit modifications and/or enhancements.
 *
 * Thank you, Cal Erickson May 02, 2002.
 */

/* This program illustrates several different ways to cause
   memory leaks. It is written as a test for various malloc
   and other type leak detectors. Each test is encapsulated
   in its own test function. this allow for modularity and
   ease of debugging. This method was chosen rather than 
   separate test programs.

*/
/*
 * Minor addition of conditional 'double_free' by adding a parameter to main().
 * - kaiwan.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Turn this on if using mtrace */
#ifdef MTRACE
#include <mcheck.h> /* get the mtrace definitions */
#endif

/* This gets turned on for using mem_watch tool */
#ifdef MEMWATCH
#include "../memwatch/memwatch.h"
//#include "../memwatch_2.71/memwatch.h"
#endif

/* This gets turned on if using dmalloc tool */
#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define ONE_K (1024)


/* Example of Read after the end of the allocated string */

int post_read(void) 
{
	char *str;

	/* Set up a string of 10 charaters */
	str = (char *) malloc(10);

	/* Fill the string with some characters */
	//memset(str,0x20,sizeof(str));
	memset(str,0x20,sizeof str );

	str[0] = 'A';
	str[1] = 'B';
	str[2] = 'C';

	/* The string is not null terminated */
	printf("%s\n",str);
	
	/* Free up the malloc'ed string */
	free(str);
	
	return(0);
}


/* Example of Write after the end of the allocated string */

int post_write(void) 
{
	char *str;
	
	/* Set up string of 10 characters */
	str = (char *) malloc(10);

	/* The strcpy write beyond the allocated area */
	strcpy(str,"AbCdEfGhIjK");

	/* print the filled string */
	printf("%s\n",str);

	/* Free up the malloc'ed string */
	free(str);

	return(0);
}

/* Example of Read before the allocated string */

int read_before(void) 
{
	char *str;

	/* Allocate a 10 character string */
	str = (char *) malloc(10);

	/* Put something in the string */
	strcpy(str,"FiLl It");

	/* Point to before allocated string starts */ 
	str--;

	/* iPrint before the allocated string starts */
	printf("%s\n",str);

	/* Adjust pointer so the free will work */
	str++;

	/* Free up the allocated area */
	free(str);

	return(0);
}

/* Example of write before allocated area */

int write_before(void) 
{
	char *str;

	/* Allocate a 10 character string */
	str = (char *) malloc(10);

	/* Force pointer to before the allocated string */
	str--;

	/* strcpy before the allocated area */
	strcpy(str,"FiLl It  ");

	/* Print the allocated string with bad pointer */
	printf("%s (0x%x)\n",str, (unsigned int)str);

	/* Adjust pointer so the free will work */
	str++;

	/* Free up the allocated area */
	free(str); 

	return(0);
}


/* Example of missed memory free of allocated string */

int miss_free(void)
{

	char *str;
	
	/* Allocate a chunk of space for some characters */
	str = (char *) calloc(5,sizeof(char) * 10);
	
	/* Forget to release the memory */

	return(0);
}


/* Example of using uninitialized memory */

int uninit_mem(void)

{
	/* give the address of a pointer to a char */
	char *str = NULL;

	/* Set the value to 10 */
	*str = 10;
	
	return(0);
}

int main(int argc, char **argv)
{
    char *some_memory[10]; /* an arrray of memory pointers */
    int exit_code = EXIT_FAILURE; /* set up exit code for failure */
    int i, j;
    int double_free = 1;

    if (argc == 1) {
	fprintf (stderr, "Usage: %s double_free [0|1]\n\
 0 : don't do the double-free (this lets the process continue with the remaining tests)\n\
 1 : perform the double-free (typically glibc catches this, sending SIGABRT aborting the process)\n",
		argv[0]);
	exit (1);
    }
    if (atoi(argv[1]) == 0)
	double_free = 0;

/* Call to mtrace routines in glibc library */
#ifdef MTRACE
    mtrace();  /* Turn on mtrace function */
#endif

    printf("Start of general malloc testing\n\n");

    printf("Start allocating 10 1k pieces of memory\n");
    for (i=0; i<10; i++)
    {
	some_memory[i] = (char *)malloc(ONE_K);
	printf ("Allocated a piece at 0x%x\n", (int)some_memory[i]);
    }
    printf("We have allocated some memory\n");
    printf("Now lets free part some memory\n");

    for (j=0; j<5; j++)
    {
    if (some_memory[j*2] != NULL) 
        {
        free(some_memory[j*2]);

	if (double_free)
	        free(some_memory[j*2]);

        printf ("Freed a piece at 0x%x\n", (int)some_memory[j*2]);
        exit_code = EXIT_SUCCESS ;
        }
    }

    printf("\n\nNow let us check other functions\n\n");

#ifndef DMALLOC

    printf("Read after the end of the allocated string\n");

    post_read(); 

#endif

    printf("\nWrite after the end of the allocated string\n");

    post_write(); 

    printf ("\nRead before the allocated string\n");

    read_before(); 

#ifndef MTRACE

    printf("\nWrite before allocated area\n");

    write_before(); 

#endif

    printf("\nMissed memory free of allocated string\n");

    miss_free();

#if !defined(MTRACE) && !defined(MEMWATCH) && !defined(DMALLOC)
    printf("\nUsing uninitialized memory\n");

    uninit_mem();

#endif

#ifdef MTRACE
    muntrace();  /* Turn off mtrace function */
#endif

    exit(exit_code);
}

