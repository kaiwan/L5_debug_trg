/*
 * .c
 * 
 * History:
 *
 * Author(s) : 
 * Kaiwan N Billimoria
 *  <kaiwan -at- kaiwantech -dot- com>
 *
 * License(s): MIT permissive
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/*---------------- Macros -------------------------------------------*/


/*---------------- Typedef's, constants, etc ------------------------*/


/*---------------- Functions ----------------------------------------*/


int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s \n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	exit (EXIT_SUCCESS);
}

/* vi: ts=8 */
