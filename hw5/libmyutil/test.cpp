#include <iostream>
extern "C"
{
	#include "util.h"
}

using namespace std;

int main()
{

	char ipstr[1024];
	const char* hostname = "www.ynet.co.il";
	
	if(dnslookup(hostname, ipstr, sizeof(ipstr)) == UTIL_FAILURE)
	{
		cout<<"Error"<<endl;
	}
	else
	{
		cout<<ipstr<<endl;
	}

	return 0;
}
