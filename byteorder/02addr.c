#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
	unsigned int addr = inet_addr("192.168.1.1");
	printf("addr=%u\n", ntohl(addr));

	struct in_addr ipaddr;
	ipaddr.s_addr = addr;

	printf("ipaddr: %s\n", inet_ntoa(ipaddr));

	return 0;
}
