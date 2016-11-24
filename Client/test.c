#include <stdio.h>
#include <unistd.h>
#include <errno.h>
int main()
{
	if(close (0) == -1)
		printf("%s close error!",strerror(errno));
	if(close (1) == -1)
		printf("%s close error!",strerror(errno));
	if(close (2) == -1)
		printf("%s close error!",strerror(errno));
	if(close (3) == -1)
		printf("%s close error!",strerror(errno));


	if( execl("ls", "", NULL)  == -1)
		printf("%s execve error!",strerror(errno));
	//printf("hello world!");	
}
