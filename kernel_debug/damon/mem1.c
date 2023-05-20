#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static char *p, *q;

int main()
{
	int k, i;
	size_t sz = getpagesize()*10; //*1024;

	p = malloc(sz);

	for (k=0; k<50000000; k++) {
	   for (i=0, q = p; q < p + sz; i++) {
		q[0x100] = i; //'a';
		printf("i=%4d q=%p *q+0x100=0x%x\n", i, q, *(q+0x100));
		q += getpagesize();
	   }
	}
	exit(0);
}
