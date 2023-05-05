
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//#define 32BIT
#undef BITS_32

#ifdef BITS_32
	#define FMTSPC "%08x"
	#define TYPECST unsigned int
#else /*64-bit*/
	#define FMTSPC "%08lx"
	#define TYPECST unsigned long
#endif
typedef unsigned int u32;

extern char *environ;
int gi=5, gu;

int main(int argc, char **argv)
{
	char local=0xff, *heapmem;
	int i=1, j=2, k=3;

	heapmem = malloc(1024);
	printf ("[pid %d]:\nva: &main = 0x" FMTSPC " &gi = 0x" FMTSPC " &gu = 0x" FMTSPC " &heapmem = 0x" \
FMTSPC " &local = 0x" FMTSPC " &environ = 0x" FMTSPC "\n",
		getpid(), (TYPECST) &main, (TYPECST) &gi, (TYPECST) &gu, 
		(TYPECST) heapmem, (TYPECST) &local, (TYPECST)environ);

	pause();
	free (heapmem);
	exit (0);
}
