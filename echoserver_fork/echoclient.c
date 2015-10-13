#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define ERR_EXIT(m) \
	    do \
{   \
	    perror(m); \
	    exit(EXIT_FAILURE); \
}while(0)

int main()
{
	int socketfd;
	int ret;
	
	socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socketfd < 0) {
		ERR_EXIT("socket");
	}

	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(5188);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(socketfd, (const struct sockaddr *)&serveraddr,
				                      (socklen_t)sizeof(serveraddr));
	if (ret < 0) {
		ERR_EXIT("connect");
	}

	char sendbuf[1024] = {0};
	char recvbuf[1024] = {0};
	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
		write(socketfd, sendbuf, strlen(sendbuf));
		read(socketfd, recvbuf, sizeof(recvbuf));
		fputs(recvbuf, stdout);	
		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
	}

	close(socketfd);

	return 0;
}
