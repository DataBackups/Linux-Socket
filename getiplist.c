#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
 #include <netdb.h>


#define ERR_EXIT(m) \
	do \
{   \
	perror(m); \
	exit(EXIT_FAILURE); \
}while(0)


	//	The hostent structure is defined in <netdb.h> as follows:
	//
	//		struct hostent {
	//			char  *h_name;            /* official name of host */
	//			char **h_aliases;         /* alias list */
	//			int    h_addrtype;        /* host address type */
	//			int    h_length;          /* length of address */
	//			char **h_addr_list;       /* list of addresses */
	//		}
	//	#define h_addr h_addr_list[0] /* for backward compatibility */

	//	       struct hostent *gethostbyname(const char *name);

int main()
{
	char host[100] = {0};
	int  ret;

	ret = gethostname(host, sizeof(host));
	if (ret < 0)
		ERR_EXIT("gethostname");

	printf("hostname: %s\n", host);

	struct hostent *hp;
	if ((hp = gethostbyname(host)) == NULL)
		ERR_EXIT("gethostbyname error");

	int i = 0;
	while (hp->h_addr_list[i] != NULL)
	{

		printf("%s\n", inet_ntoa(*(struct in_addr *)hp->h_addr_list[i]));
		i++;
	}

	return 0;
}
