#include <stdio.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char *argv[])
{
	int status;

	if(fork() == 0)
	{
		if (execlp("ls","ls","-l",NULL) == -1)
			printf("Error occured during execute ls.\n");
	}

	wait(&status);
	printf("The ls command successfully executed.\n");

	return 0;
}