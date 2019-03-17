/* mysh.c
my shell.


o -v : show child termination status info
+o -D : DEBUG is ON, else OFF

*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define LEN		128
#define MAXARGS 1024
#define PROMPT	"$ "
#define SEP     " "

#define ON	1
#define OFF	0

int DEBUG=OFF;

int main( int argc, char ** argv )
{
	char cmd[LEN];
	char *newargs[MAXARGS];
	int i;
	char *exit_str="q";

	if( (argc > 1) && (strcmp(argv[1],"-D")==0) )
		DEBUG=ON;

	for(i=0;i<LEN;i++) {   
		newargs[i]=malloc(LEN);
		if( newargs[i] == NULL ) {
			fprintf(stderr,"%s: no memory for newargs[%d]..",argv[0],i);
			exit( 1 );
		}
	}
	newargs[i]=0;

	while( 1 ) {
		for(i=0;i<MAXARGS;i++)
			newargs[i]=0;

		printf("%s", PROMPT );
		fflush( stdout );

		/* fsgets is safe; gets is not */
		fgets( cmd, LEN, stdin );
		cmd[strlen(cmd)-1]='\0';  //remove trailing \n

		if( strcmp(cmd,exit_str) == 0 )
				exit( 0 );
		if( cmd[0]==0 ) // try to take care of LF 
			continue;

		/* tokenize..
		 * warning/FIXME : strtok is not reentrant-safe
		 */
		newargs[0]=strtok(cmd, SEP);
		i=1;
		while( (newargs[i++]=strtok(NULL, SEP)) ) ;

		if( DEBUG ) {
			i=0;
			while( newargs[i] ) {
				printf("\nnewargs[%d]=%s",i,newargs[i]);
				fflush( stdout );
				i++;
			}
			printf("\n");
		}

		switch( fork() ) {
			case -1 : perror("fork failed");
				break;
			case 0 :  // Child
				if( execvp( newargs[0], newargs ) == -1 ) {
					perror( "exec failure" );
					exit( 1 );
				}
				// code never reaches here..
			default : // Parent
				if( wait( 0 ) == -1 )
					perror("wait"),exit(1);
		}
	} // while

	fflush(stdout);
	return 0;
} // main()

