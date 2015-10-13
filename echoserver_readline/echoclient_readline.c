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


//ssize_t read(int fd, void *buf, size_t count);
ssize_t readn(int fd, void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nread;
	char *bufp = (char *)buf;

	while (nleft > 0) {
		nread = read(fd, buf, nleft);
		if (nread < 0) {
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

	struct sockaddr_in localaddr;
	socklen_t addrlen = sizeof(localaddr);
	//int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	ret = getsockname(socketfd, (struct sockaddr *)&localaddr, &addrlen);
	if (ret < 0)
		exit(EXIT_FAILURE);
	printf("local ip address: %s, local port:%d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));

	char sendbuf[1024];
	char recvbuf[1024];
	memset(sendbuf, 0, sizeof(sendbuf));
	memset(recvbuf, 0, sizeof(recvbuf));

	struct sockaddr_in peeraddr;
	socklen_t peeraddrlen = sizeof(peeraddr);
	//int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	ret = getpeername(socketfd, (struct sockaddr *)&peeraddr, &peeraddrlen);
	if (ret < 0)
		exit(EXIT_FAILURE);
	printf("peer ip address: %s, peer port:%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

	int len;
	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
		//write(socketfd, sendbuf, strlen(sendbuf));

		len = strlen(sendbuf);
		writen(socketfd, sendbuf, len);
		//read(socketfd, recvbuf, sizeof(recvbuf));
		readline(socketfd, recvbuf, 1024);
		fputs(recvbuf, stdout);	
		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
	}

	close(socketfd);

	return 0;
}
