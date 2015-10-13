#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//int socket(int domain, int type, int protocol);
//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
//int listen(int sockfd, int backlog);
//int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

#define ERR_EXIT(m) \
	do \
{	\
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
	int listenfd;
	int ret;

	listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenfd < 0) {
		ERR_EXIT("socket");	
	}


//	struct sockaddr_in {
//		sa_family_t    sin_family; /* address family: AF_INET */
//		in_port_t      sin_port;   /* port in network byte order */
//		struct in_addr sin_addr;   /* internet address */
//	};
//	struct in_addr {
//		    in_addr_t s_addr;
//	};
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(5188);
	serveraddr.sin_addr.s_addr= inet_addr("127.0.0.1");

//   int setsockopt(int sockfd, int level, int optname,
//							const void *optval, socklen_t optlen);

	int on = 1;
	ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (ret < 0)
		ERR_EXIT("setsockopt");

	ret = bind(listenfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (ret < 0) {
		ERR_EXIT("bind");
	}

	ret = listen(listenfd, SOMAXCONN);
	if (ret < 0) {
		 ERR_EXIT("listen");
	}

	struct sockaddr_in peeraddr;
	socklen_t peeraddr_len = sizeof(peeraddr); 

	int connectfd;
	connectfd = accept(listenfd, (struct sockaddr *)&peeraddr, &peeraddr_len);
	if (connectfd < 0) {
		ERR_EXIT("accept");
	}

	printf("ip address: %s, port: %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
	
	pid_t pid;
	pid = fork();
	if (pid < 0) {
		ERR_EXIT("fork");		
	} else if (pid == 0) {
		char recbuf[1024];
		int len;
		while(1) {
			memset(recbuf, 0, sizeof(recbuf));
			len = read(connectfd, recbuf, sizeof(recbuf));
			if (len < 0) {
				ERR_EXIT("read");
			} else if (len == 0) {
				printf("peer client close\n");
				break;
			} else {
				printf("%s", recbuf);
			}
		}
		close(connectfd);
		close(listenfd);
		printf("server child close\n");
		kill(getppid(), SIGUSR1);
	} else {
		char sendbuf[1024] = {0};	
		signal(SIGUSR1, handler);
		while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
			ret = write(connectfd, sendbuf, strlen(sendbuf));
			if (ret < 0)
				ERR_EXIT("write");
			memset(sendbuf, 0, sizeof(sendbuf));
		}
		close(listenfd);
		close(connectfd);
	    //#include <signal.h>
	    //typedef void (*sighandler_t)(int);
	    //sighandler_t signal(int signum, sighandler_t handler);
		printf("server parent close\n");
	}

	return 0;
}
