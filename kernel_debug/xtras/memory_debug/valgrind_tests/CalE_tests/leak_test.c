/*
 * leaky.c
 * Demonstrates memory "leakage" by (incorrectly) allowing the function
 * leaky() to allocate a chunk of memory and not free it..over time, 
 * the amt of free memory drops and the amt of mem allocated to the 
 * process becomes larger & larger.
 *
 * Compile with:
 *  gcc  leaky.c -o leaky -Wall [-Os]
 *
 * Run as (for eg.) : $ ./leaky 1024 5000    (1MB alloc, 5000 times)
 * -run [g]top in another window & see the leakage occuring!
 *
 * kaiwan.
 *
 * History/Changelog
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void leaky(int sz)
{
   void * p;

   p = malloc (sz);
   if (!p) {
      fprintf (stderr,"malloc failed");
      exit (1);
   }
#if 0
  free(p);
#endif
}

static void leaktest(char **argv)
{
	int i=0;
	int n;

   n = atoi(argv[2]);
   while (i < n) {
      if (!(i%100)) {
         printf ("%d ", i);
         fflush (stdout);
      }
      leaky(atoi(argv[1])*1024);
      usleep (10000);   /* 10ms */
      i++;
   }
}

int main(int argc, char **argv)
{
   if (argc != 3) {
      fprintf (stderr, "Usage: %s buffer_size_to_allocate (in KB) loop_count\n", argv[0]);
      exit (1);
   }

   leaktest(argv);

   exit( 0 );
}
