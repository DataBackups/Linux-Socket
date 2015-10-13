#include <stdio.h>
#include <arpa/inet.h>

int main()
{
	unsigned int x = 0x12345678;
	unsigned char *p = (unsigned char *)&x;
	printf("0x%x 0x%x 0x%x 0x%x\n", p[0], p[1], p[2], p[3]);

	if (p[0] == 0x78)
		printf("little endian\n");
	else
		printf("big endian\n");

	unsigned int y = htonl(x);
	p = (unsigned char *)&y;
	printf("network byte order: 0x%x 0x%x 0x%x 0x%x\n", p[0], p[1], p[2], p[3]);

	return 0;
}
