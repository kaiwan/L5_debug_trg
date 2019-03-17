/*
** talker.c -- a datagram "client" demo
* (c) Beej's N/w Prg Guide
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT 54295 //56100	// the port users will be connecting to

static int send_pkt(int sd, char *dest_ip, char *msg)
{
	struct sockaddr_in dest_addr;
	int numbytes;

	dest_addr.sin_family = AF_INET; // host byte order
    dest_addr.sin_port = htons(SERVERPORT); // short, network byte order
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip);
    memset(&(dest_addr.sin_zero), '\0', 8); 

	printf ("Sending message over datagram socket to %s:%d now...\n", 
		dest_ip, SERVERPORT);
	if ((numbytes = sendto(sd, msg, strlen(msg), 0,
			 (struct sockaddr *)&dest_addr, sizeof(dest_addr))) == -1) {
		perror("talker: sendto");
		return -1;
	}

	printf("talker: sent %d bytes.\n", numbytes);
	return numbytes;
}

#if 0
#define MAXBUFLEN 1000
static int recv_pkt(int sd)
{
	struct sockaddr_in src_addr;	// connector's (peers) address information
	int numbytes;
	socklen_t addr_len;
	char buf[MAXBUFLEN];

	addr_len = sizeof(struct sockaddr);
	if ((numbytes = recvfrom(sd, buf, MAXBUFLEN-1, 0,
					 (struct sockaddr *)&src_addr,
					 &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	printf("got packet from %s\n", inet_ntoa(src_addr.sin_addr));
	printf("packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("packet contains \"%s\"\n", buf);

	return numbytes;
}
#endif


int main(int argc, char *argv[])
{
	int sockfd;

	if (geteuid()) {
		fprintf(stderr,"%s: need to run as root.\n", argv[0]);
		exit(1);
	}
	if (argc != 3) {
		fprintf(stderr,"usage: %s DEST-IP-address message\n", argv[0]);
		//fprintf(stderr,"usage: %s interface-to-bind-to DEST-IP-address message\n", argv[0]);
		exit(1);
	}

  if( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ) { 
        perror("socket");
        exit(1);
    }   

#if 0
	// Should be running as root!
    if (setsockopt (sockfd, SOL_SOCKET, SO_BINDTODEVICE, argv[1], strlen(argv[1])+1) < 0) {
        printf("\n%s:setsockopt failed...\n",argv[0]);
        close(sockfd);
        exit(1);
    }
    printf ("%s: successfully bound to interface '%s'\n", argv[0], argv[1]);
#endif

	if (send_pkt(sockfd, argv[1], argv[2]) == -1) {
		fprintf(stderr,"%s: send_pkt failed, aborting.\n", argv[0]);
   		close(sockfd);
		exit(1);
	}

	//recv_pkt(sockfd);
   	close(sockfd);

	return 0;
}
