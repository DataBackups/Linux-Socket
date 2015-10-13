#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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


void do_service(int connectfd)
{
	char recbuf[1024];
	int len;

	while(1) {
		memset(recbuf, 0, sizeof(recbuf));
		len = read(connectfd, recbuf, sizeof(recbuf));
		if (len == 0) {
			printf("client close\n");
			break;
		}
		if (len < 0) // = -1
		{
			ERR_EXIT("read fail");
		}
		printf("%s", recbuf);
		write(connectfd, recbuf, len);
	}
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

	pid_t pid;
	while(1) {

		connectfd = accept(listenfd, (struct sockaddr *)&peeraddr, &peeraddr_len);
		if (connectfd < 0) {
			ERR_EXIT("accept");
		}
		printf("ip address: %s, port: %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
	

		pid = fork();
		if (pid == -1) {
			ERR_EXIT("fork fail");
		} else if (pid == 0) { //child
			close(listenfd);
			do_service(connectfd);
			exit(EXIT_SUCCESS);
		} else { //father
			close(connectfd);
		}
	}

	return 0;
}
