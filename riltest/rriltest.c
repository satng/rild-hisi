#include <cutils/sockets.h>
#include <telephony/ril.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>


int main(int argc, char* argv[])
{
	int fd;
	int index;
	while(1)
	{
		printf("RIL Layer Test:\n");
		printf("Test Case Index\tRIL Request To Be Tested\n");
		printf("Please input index for the RIL request you want to test:\n");
		scanf("%d",&index);
        	fd = socket_local_client( "rild-debug",
            	ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
		if(fd < 0) {
			printf("create socket failed\n");
			return -1;
		}
		if(index > 0 && index < 80)
			send(fd,&index,sizeof(int),0);
	}
	return 0;
}

