#include <binder/Parcel.h>
#include <cutils/jstring.h>
#include <cutils/sockets.h>
#include <telephony/ril.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

using namespace android;

int s_send_at_fd;

static char *
strdupReadString(Parcel &p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p.readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}

static void writeStringToParcel(Parcel &p, const char *s) {
    char16_t *s16;
    size_t s16_len;
    s16 = strdup8to16(s, &s16_len);
    p.writeString16(s16, s16_len);
    free(s16);
}

int sendDataToRild(Parcel &p)
{
        unsigned char* sendData = NULL;
        uint32_t sendDataLen = p.dataSize();
        unsigned char dataLength[4];
        int32_t res = -1;
        int error = 0;
        int fd = s_send_at_fd;
        
        // parcel length in big endian 转化数据长度为byte数组
        dataLength[0] = dataLength[1] = 0;
        dataLength[2] = (unsigned char)((sendDataLen >> 8) & 0xff);
        dataLength[3] = (unsigned char)((sendDataLen) & 0xff);

        //获取待发送数据
        sendData = (unsigned char*)malloc(p.dataSize());
        memcpy(sendData, p.data(), sendDataLen);

        //发送数据长度
        res = send(fd,(const void *)dataLength,4,0);

        //发送数据内容
        res = send(fd,(const void *)sendData,sendDataLen,0);
        printf("sendDataLen:%d\n",sendDataLen);
        free(sendData);
        return error;
}


int main(int argc, char* argv[])
{
	int fd;
	int index;
	int args_len;
	char args[80];
	char *stringp="setport?";
	fd = socket_local_client( "rild",ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
	if(fd < 0) {
		printf("create socket failed, %s\n", strerror(errno));
		return -1;
	}
	s_send_at_fd = fd;

	Parcel p;
	size_t parcel_position;
	parcel_position = p.dataPosition();
	p.writeInt32(104);//request
	p.writeInt32(2);//token
	p.writeInt32(1);//string count
	writeStringToParcel(p,(const char*)stringp);
	sendDataToRild(p);
	// p.setDataPosition(parcel_position);

        	Parcel recvP;
	char recvBuffer[500] = {0};
	int32_t response_type,token,r_err,countStrings;
	status_t status;
	size_t datalen;
	char **pStrings;
	 while(1){
	            // sleep(2);
	            int recvLen = recv(fd,recvBuffer,4,0);
	            printf("recvLen:%d\n", recvLen);
	            if(recvLen ==0)
	                continue;

	            //读取长度
	            printf("recvBuffer %02x %02x %02x %02x\n",recvBuffer[0],recvBuffer[1],recvBuffer[2],recvBuffer[3]);
	            int messageLength = ((recvBuffer[0] & 0xff) << 24)
	                | ((recvBuffer[1] & 0xff) << 16)
	                | ((recvBuffer[2] & 0xff) << 8)
	                | (recvBuffer[3] & 0xff);
	            printf("recv messageLength = %d\n",messageLength);
	            memset(recvBuffer, 0, 4);
	            //读取内容
	            recvLen = recv(fd,recvBuffer,messageLength,0);
	            recvP.setData((uint8_t*)recvBuffer,   recvLen);

	             status = recvP.readInt32 (&response_type);
	             status = recvP.readInt32 (&token);
	             status = recvP.readInt32 (&r_err);

		if(response_type == 1){
			printf(">>>>>>> It's from RIL_onUnsolicitedResponse\n", r_err);
			printf("response_type:%d\n", response_type);
			printf("token:%d\n", token);	
			printf("r_err:%d\n", r_err);
		}else if(response_type == 0){
			printf(">>>>>>>It's from RIL_onRequestComplete \n", r_err);
			printf("response_type:%d\n", response_type);
			printf("token:%d\n", token);	
			printf("r_err:%d\n", r_err);
			status = recvP.readInt32 (&countStrings);
			printf("countStrings:%d\n", countStrings);
			if (status != NO_ERROR) {
				printf("failed to read response\n");
			}
			if (countStrings == 0) {
			    // just some non-null pointer
			    pStrings = (char **)alloca(sizeof(char *));
			    datalen = 0;
			} else if (((int)countStrings) == -1) {
			    pStrings = NULL;
			    datalen = 0;
			} else {
			    datalen = sizeof(char *) * countStrings;

			    pStrings = (char **)alloca(datalen);

			    for (int i = 0 ; i < countStrings ; i++) {
			        pStrings[i] = strdupReadString(recvP);
			        printf("%s,",pStrings[i]);
			    }
			}
			printf("\n>>>>>> end\n");
			sleep(10);
			p.setDataPosition(0);
			p.writeInt32(104);//request
			p.writeInt32(2);//token
			p.writeInt32(1);//string count
			writeStringToParcel(p,(const char*)stringp);
			sendDataToRild(p);
		}else{
			continue;
		}
		printf("\n\n\n\n\n\n\n");
	        }

	return 0;
}
