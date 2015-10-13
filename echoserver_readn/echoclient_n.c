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


struct packet {
	int len;
	char buf[1024];
};

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

	struct packet sendbuf;
	struct packet recvbuf;
	memset(&sendbuf, 0, sizeof(sendbuf));
	memset(&recvbuf, 0, sizeof(recvbuf));

	int len;
	while(fgets(sendbuf.buf, sizeof(sendbuf.buf), stdin) != NULL) {
		//write(socketfd, sendbuf, strlen(sendbuf));

		len = strlen(sendbuf.buf);
		sendbuf.len = htonl(len);//notice the network byte order
		writen(socketfd, &sendbuf, 4 + len);
		//read(socketfd, recvbuf, sizeof(recvbuf));
		readn(socketfd, &recvbuf, 4 + len);
		fputs(recvbuf.buf, stdout);	
		memset(&sendbuf, 0, sizeof(sendbuf));
		memset(&recvbuf, 0, sizeof(recvbuf));
	}

	close(socketfd);

	return 0;
}
