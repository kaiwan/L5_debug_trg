/*
 * .c
 * 
 * History:
 *
 * Author(s) : 
 * Kaiwan N Billimoria
 *  <kaiwan -at- kaiwantech -dot- com>
 *
 * License(s): [L]GPL
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/*---------------- Macros -------------------------------------------*/


/*---------------- Typedef's, constants, etc ------------------------*/


/*---------------- Functions ----------------------------------------*/

void test_free_bug()
{
  void *p=0;
  p = malloc(1024);

#if 1
  p --;
#endif

  usleep(555);
  free (p);
}

int main (int argc, char **argv)
{
	test_free_bug();	
	exit (0);
}

/* vi: ts=4 */
