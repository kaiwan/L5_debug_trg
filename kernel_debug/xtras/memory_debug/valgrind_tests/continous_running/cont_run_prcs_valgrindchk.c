/*
 * cont_run_prcs_valgrindchk.c
 * 
 * How does one run the Valgrind mem-check tool on a continously running 
 * process, typically a daemon process which never dies?
 * A simple test case to check this out; pl read article below:
 * Res: http://tromey.com/blog/?p=731
 *
 * Author(s) : 
 * Kaiwan N Billimoria
 *  <kaiwan -at- kaiwantech -dot- com>
 *
 * License(s): [L]GPL
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

/*---------------- Macros -------------------------------------------*/
/*------------------------ DELAY_LOOP --------------------------------*/
static inline void beep(int what)
{
	char buf[2];
	buf[0]='.'; buf[1]='\0';
	//buf[0]=what; buf[1]='\0';
	(void)write(1, buf, 1);
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
			for(inner_index=0;inner_index<100;inner_index++) \
				for(m=0;m<5;m++); \
		} \
}

/*---------------- Typedef's, constants, etc ------------------------*/


/*---------------- Functions ----------------------------------------*/
static void do_work(int n)
{
	char *bad = malloc(20);

	memset(bad, 0, 20);
	if (n>122)
		n-=65;
	bad[21]=n; // Bug!
	DELAY_LOOP(n, 1);
	usleep(10000); // 10ms
	free(bad);
}

int main (int argc, char **argv)
{
   int i=0, step=20;

   while (1) {
      if (!(i%step)) {
         printf ("### %d:\n", i);
      }
      do_work(i++);
   }
	exit (0);
}

/* vi: ts=4 */
