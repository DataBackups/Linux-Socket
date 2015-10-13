#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
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

void handler(int sig)
{
	printf("receive a signal %d\n", sig);
	exit(EXIT_SUCCESS);
}

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

	pid_t pid;
	pid = fork();
	if (pid < 0) {
		ERR_EXIT("fork");
	} else if (pid == 0) {
		char recvbuf[1024] = {0};
		while(1) {
			memset(recvbuf, 0, sizeof(recvbuf));
			ret = read(socketfd, recvbuf, sizeof(recvbuf));		
			if (ret < 0) {
				ERR_EXIT("read");
			} else if (ret == 0) {
				printf("peer close\n");
				break;
			} else {
				fputs(recvbuf, stdout);	
			}
		}
		close(socketfd);	
		printf("client child close\n");
		kill(getppid(), SIGUSR1);
		exit(EXIT_SUCCESS);
	} else {
		char sendbuf[1024] = {0};
		signal(SIGUSR1, handler);
		while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
			write(socketfd, sendbuf, strlen(sendbuf));	
		}
		close(socketfd);
		printf("client parent close\n");
	}

	return 0;
}
