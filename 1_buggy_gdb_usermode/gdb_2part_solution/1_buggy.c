/*
 * 1_buggy_using-gdb-userspace.c
 * 
 * Intended as an exercise for participants.
 *
 * This program has bugs.
 * a) Find and resolve them using the GNU debugger gdb.
 * b) Then create a patch.
 *
 * The purpose of this simple program is:
 *  Given a pathname as a parameter, it queries and prints some
 *  information about it (retrieved from it's inode using the 
 *  stat(2) syscall).
 * 
 * (c) Kaiwan.
 */

#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

main(int argc, char **argv)
{
	struct stat statbuf;

	if (argc != 2) {
		fprintf (stderr, "Usage: %s filename\n");
		exit (1);
	}

	/* Retrieve the information using stat(2) */
	stat (argv[1], &statbuf);
	
	/* 
	 * Lets print the:
	 *  -inode #
	 *  -size in bytes
	 *  -modification time
	 */
	printf ("%s:\ninode number: %d : size (bytes): %d : mtime: %s\n",
		argv[1], statbuf.st_ino, 
		statbuf.st_size, ctime(statbuf.st_mtime));
	
	exit (0);
}
