/*server_socket */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <string.h>

#define HOSTLEN 256
#define BACKLOG 1

int server_socket(int portnum, int backlog)
{
	struct sockaddr_in saddr;
	struct hostent *hp;
	char hostname[HOSTLEN];
	int sock_id;
printf("portnum:[%d]\n", portnum);
	sock_id = socket(PF_INET, SOCK_STREAM, 0);
	if (sock_id == -1)
		return -1;

	bzero((void *)&saddr, sizeof(saddr));
	/* 
	gethostname(hostname, HOSTLEN);
printf("hostname:[%s]\n", hostname);
	hp = gethostbyname(hostname);

	bcopy( (void *)hp->h_addr, (void *)&saddr.sin_addr, hp->h_length);
	*/
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(portnum);
	saddr.sin_family = AF_INET;
	if (bind(sock_id, (struct sockaddr *) &saddr, 
		sizeof(saddr)) != 0)
		return -1;

	if (listen(sock_id, backlog) != 0)
		return -1;

	return sock_id;

}

