/*
 * asan_try.c
 * 
 * History:
 *
 * Author(s) : 
 * Kaiwan N Billimoria
 *  <kaiwan -at- kaiwantech -dot- com>
 *
 * License(s): MIT
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

/*---------------- Macros -------------------------------------------*/


/*---------------- Typedef's, constants, etc ------------------------*/


/*---------------- Functions ----------------------------------------*/

#if 0
const char *__asan_default_options() {
  return "debug=true:verbosity=1:malloc_context_size=20";
}
#endif

static void fun1(void)
{
	char buf[12];
	memset(buf, 'x', 14); // stack overflow!
}


#define QP	fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__);

static int gArr[10];
int main(int argc, char **argv)
{
	/*if (argc < 2) {
		fprintf(stderr, "Usage: %s \n", argv[0]);
		exit(EXIT_FAILURE);
	}*/

	QP;
	//printf("%ld\n", __STDC_VERSION__); // '201112[L]' =>this gcc is C11 compliant

	/* asan-globals : Enable buffer overflow detection for global objects. */
	//gArr[argc+10]=7;

	fun1();

	exit (EXIT_SUCCESS);
}

/* vi: ts=4 */
