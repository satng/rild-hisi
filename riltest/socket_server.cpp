#include <cutils/sockets.h>
#include <cutils/record_stream.h>
#include <pthread.h>

#include <sys/types.h>
#include <pwd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <alloca.h>
#include <sys/un.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

static void freeDebugCallbackArgs(int number, char **args) {
    for (int i = 0; i < number; i++) {
        if (args[i] != NULL) {
            free(args[i]);
        }
    }
    free(args);
}


int main(int argc, char* argv[])
{
	int s_fdListen, ret;
	int clen;
	struct sockaddr_un caddr;
	int cfd;
	char buf[50];
    int acceptFD, option;
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);
    int data;
    unsigned int qxdm_data[6];
    const char *deactData[1] = {"1"};
    char *actData[1];
    int hangupData[1] = {1};
    int number;
    char args[5][80];

	ret = socket_local_server ("rild-debug", ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
	if (ret < 0) {
		printf("s_name_ril Unable to bind socket errno:%s\n", strerror(errno));
		exit (-1);
	}

	s_fdListen = ret;
	// fcntl(s_fdListen, F_SETFD, FD_CLOEXEC);  
 	// printf("fcntl %d !\n", s_fdListen);


 	while(1){
 		acceptFD = accept(s_fdListen,  (sockaddr *) &peeraddr, &socklen);
 		if (acceptFD < 0) {
 			printf ("error accepting on debug port: %d\n\n", strerror(errno));
 			continue;
 		}
 		// fcntl(acceptFD, F_SETFD, FD_CLOEXEC);  
 		if (recv(acceptFD, &number, sizeof(int), 0) != sizeof(int)) {
 			printf ("error reading on socket: number of Args: \n\n");
 			return -1;
 		}
 		printf("read number:%d !\n", number);

 		for (int i = 0; i < number; i++) {
 			int len;
 			if (recv(acceptFD, &len, sizeof(int), 0) != sizeof(int)) {
 				printf("error reading on socket: Len of Args: \n\n");
 				// freeDebugCallbackArgs(i, args);
 				return -1;
 			}
 			printf("svc read len:%d !\n", len);
       			 // +1 for null-term
 			// args[i] = (char *) malloc((sizeof(char) * len) + 1);
 			printf("server args: %s\n\n",args[i]);
 			if (recv(acceptFD, args[i], sizeof(char) * len, 0)!= (int)sizeof(char) * len) {
 				printf("error reading on socket: Args[%d] \n\n", i);
	 			// freeDebugCallbackArgs(i, args);
	 			return -1;
 			}
 			printf("server read args:%s !\n", args[i]);
	 		// char * buf = args[i];
	 		// buf[len] = 0;
	 		// freeDebugCallbackArgs(i, args);
 		}

 	}

	return 0;
}