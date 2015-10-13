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

//ssize_t write(int fd, const void *buf, size_t count);
ssize_t writen(int fd, const void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nwrite;
	char *bufp = (char *)buf;

	while (nleft > 0) {
		nwrite = write(fd, buf, nleft);
		if (nwrite < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		} else if (nwrite == 0) {
			return count - nleft;
		} else {
			nleft -= nwrite;
			bufp -= nwrite;
		}
	}

	return count;
}

//ssize_t read(int fd, void *buf, size_t count);
ssize_t readn(int fd, void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nread;
	char *bufp = (char *)buf;

	while (nleft > 0) {
		nread = read(fd, buf, nleft);
		if (nread < 0 ) {
			if (errno == EINTR)
				continue;
			return -1;
		} else if (nread == 0) {
			return count - nleft;
		} else {
			nleft -= nread;
			bufp -= nread;	
		}
	}

	return count;
}


// ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
	int ret;

	while (1) {
		ret = recv(sockfd, buf, len, MSG_PEEK); 
		if (ret == -1 && errno == EINTR)
			continue;
		return ret;
	}   
}

ssize_t readline(int sockfd, void *buf, size_t maxline)
{
	int ret;
	int nleft = maxline;
	int nread;
	char *bufp = buf;
	int i;

	while (1) {
		ret = recv_peek(sockfd, bufp, nleft);
		if (ret < 0)
			return ret; 
		if (ret == 0)
			return 0;

		nread = ret;
		for (i=0; i < nread; i++) {
			if (bufp[i] == '\n') {
				ret = readn(sockfd, bufp, i+1);
				if (ret != i+1)
					exit(EXIT_FAILURE);
				return ret;
			}   
		}   

		if (nread > nleft)
			exit(EXIT_FAILURE);
		ret = readn(sockfd, bufp, nread);
		if (ret != nread)
			exit(EXIT_FAILURE);
		bufp += nread;
		nleft -= nread;

	}
}



void do_service(int connectfd)
{
	char recvbuf[1024];
	int len;
	int ret;

	while (1) {
		memset(recvbuf, 0, sizeof(recvbuf));
		//len = read(connectfd, recbuf, sizeof(recbuf));

		ret = readline(connectfd, recvbuf, 1024);	
		if (ret == -1) {
			ERR_EXIT("read");
		} else if (ret == 0) {
			printf("client close\n");
			break;
		}
		printf("len:%-4d\tmsg: %s", (int)strlen(recvbuf), recvbuf);

		//write(connectfd, recbuf, len);
		writen(connectfd, recvbuf, ret);
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
	while (1) {

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
