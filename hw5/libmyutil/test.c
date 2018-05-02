#include "util.h"

int main()
{

	char ipstr[1024];
	char* hostname = "www.ynet.co.il";
	
	if(dnslookup(hostname, ipstr, sizeof(ipstr)) == UTIL_FAILURE)
	{
		printf("Error\n");
	}
	else
	{
		printf("%s\n", ipstr);
	}

	return 0;
}
