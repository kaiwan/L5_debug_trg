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
 * (c) Kaiwan NB, kaiwanTECH
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
	struct stat statbuf;

	if (argc != 2) {
		fprintf (stderr, "Usage: %s filename\n", argv[0]);
		exit (1);
	}

	/* Retrieve the information using stat(2) */
	stat(argv[1], &statbuf);
		/* Notice we have not checked the failure case for stat(2)!
		 * It can be a bad bug; just try passing an invalid file as
		 * parameter to this program and see what happens :)
		 */
	/* 
	 * Lets print the:
	 *  -inode #
	 *  -size in bytes
	 *  -modification time
	 */
	printf("%s:\n"
		" inode number: %ld : size (bytes): %ld : mtime: %s "
		" uid: %d gid: %d\n"
		" blksize: %ld blk count: %ld\n",
		argv[1], statbuf.st_ino, 
		statbuf.st_size, ctime(&statbuf.st_mtime),
		statbuf.st_uid, statbuf.st_gid, 
		statbuf.st_blksize, statbuf.st_blocks);
	
	exit(0);
}
