#include <telephony/ril.h>
#include <telephony/ril_cdma_sms.h>
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

// //TJD add  for v7r1
// /*AT^SETPORT?*/
// #define RIL_REQUEST_SUPPORT_PORTS   104
// /*AT^CGCATT?*/
// #define RIL_REQUEST_PSCS_BIND_STATE   105
// /*AT^NDISDUP=1,1,"CMNET"*/ 
// #define RIL_REQUEST_NDIS_DIAL   106
// /*AT+CGPADDR*/
// #define RIL_REQUEST_PDP_ADDR   107
// /*AT^DHCP?*/
// #define RIL_REQUEST_DHCP_INFO   108

// #define RIL_UNSOL_NDIS_DIAL   1031


using namespace android;

int s_send_at_fd;
static pthread_t s_tid_reader;

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
        // printf("sendDataLen:%d\n",sendDataLen);
        free(sendData);
        return error;
}

static void *readerLoop(void *arg)
{
       	Parcel recvP;
	char recvBuffer[500] = {0};
	int32_t response_type,token,r_err,countStrings;
	status_t status;
	size_t datalen;
	char **pStrings;
	int fd = s_send_at_fd;

	while(1){
	            // sleep(2);
	            int recvLen = recv(fd,recvBuffer,4,0);
	            // printf("recvLen:%d\n", recvLen);
	            if(recvLen ==0)
	                continue;

	            //读取长度
	            // printf("recvBuffer %02x %02x %02x %02x\n",recvBuffer[0],recvBuffer[1],recvBuffer[2],recvBuffer[3]);
	            int messageLength = ((recvBuffer[0] & 0xff) << 24)
	                | ((recvBuffer[1] & 0xff) << 16)
	                | ((recvBuffer[2] & 0xff) << 8)
	                | (recvBuffer[3] & 0xff);
	            // printf("recv messageLength = %d\n",messageLength);
	            memset(recvBuffer, 0, 4);
	            //读取内容
	            recvLen = recv(fd,recvBuffer,messageLength,0);
	            recvP.setData((uint8_t*)recvBuffer,   recvLen);

	             status = recvP.readInt32 (&response_type);
	             status = recvP.readInt32 (&token);
		if(response_type == 1){
			printf(">>>>>>> This  is from RIL_onUnsolicitedResponse\n");
			if(token == RIL_UNSOL_NDIS_DIAL){
				status = recvP.readInt32 (&countStrings);
				printf("countStrings:%d\n", countStrings);
				if (countStrings == 0) {
				    // just some non-null pointer
				    // pStrings = (char **)alloca(sizeof(char *));
				    // datalen = 0;
				    continue;
				} else if (((int)countStrings) == -1) {
				    // pStrings = NULL;
				    // datalen = 0;
				    continue;
				} else {
				    datalen = sizeof(char *) * countStrings;

				    pStrings = (char **)alloca(datalen);

				    printf("recv strings:\n");
				    for (int i = 0 ; i < countStrings ; i++) {
				        pStrings[i] = strdupReadString(recvP);
				        printf("%s\n",pStrings[i]);
				    }
				}
			}

			printf("\n>>>>>> end\n");
		}else if(response_type == 0){
		             status = recvP.readInt32 (&r_err);

			printf("response_type:%d\n", response_type);
			printf("token:%d\n", token);			
		             if(r_err){
				printf("recv error, r_err:%d\n\n\n\n\n\n", r_err);
				continue;
		             }
			printf(">>>>>>>This is from RIL_onRequestComplete \n", r_err);
			if(token == RIL_REQUEST_NDIS_DIAL){
				printf("\n\n\n\n\n\n\n");
				continue;
			}
			status = recvP.readInt32 (&countStrings);
			printf("countStrings:%d\n", countStrings);
			if (countStrings == 0) {
			    // just some non-null pointer
			    // pStrings = (char **)alloca(sizeof(char *));
			    // datalen = 0;
			    continue;
			} else if (((int)countStrings) == -1) {
			    // pStrings = NULL;
			    // datalen = 0;
			    continue;
			} else {
			    datalen = sizeof(char *) * countStrings;

			    pStrings = (char **)alloca(datalen);

			    printf("recv strings:\n");
			    for (int i = 0 ; i < countStrings ; i++) {
			        pStrings[i] = strdupReadString(recvP);
			        printf("%s\n",pStrings[i]);
			    }
			}
			printf("\n>>>>>> end\n");
			// sleep(10);
			// p.setDataPosition(0);
			// p.writeInt32(104);//request
			// p.writeInt32(2);//token
			// p.writeInt32(1);//string count
			// writeStringToParcel(p,(const char*)stringp);
			// sendDataToRild(p);
		}
		printf("\n\n\n\n\n\n\n");
	}
	return NULL;
}

int test_dhcp(void)
{
	Parcel p;
	p.setDataPosition(0);
	p.writeInt32(RIL_REQUEST_PSCS_BIND_STATE);//request
	p.writeInt32(1);//token
	p.writeInt32(1);//string count
	writeStringToParcel(p,"?");
	sendDataToRild(p);
sleep(3);
	p.setDataPosition(0);
	p.writeInt32(RIL_REQUEST_NDIS_DIAL);//request
	p.writeInt32(2);//token
	p.writeInt32(1);//string count
	writeStringToParcel(p,"=1,1,\"CMNET\"");
	sendDataToRild(p);
sleep(2);
	p.setDataPosition(0);
	p.writeInt32(RIL_REQUEST_PDP_ADDR);//request
	p.writeInt32(3);//token
	p.writeInt32(0);//string count
	// writeStringToParcel(p," ");
	sendDataToRild(p);
sleep(2);
	p.setDataPosition(0);
	p.writeInt32(RIL_REQUEST_DHCP_INFO);//request
	p.writeInt32(4);//token
	p.writeInt32(1);//string count
	writeStringToParcel(p,"?");
	sendDataToRild(p);
sleep(2);

	return 0;
}

int main(int argc, char* argv[])
{
	int fd;
	int index;
	int args_len;
	int ret;
	char cmd[20];
	pthread_t tid;
	pthread_attr_t attr;
	Parcel p;

	char args[80];
	char *stringp="setport?";
	fd = socket_local_client( "rild",ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
	if(fd < 0) {
		printf("create socket failed, %s\n", strerror(errno));
		return -1;
	}
	s_send_at_fd = fd;


	pthread_attr_init (&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&s_tid_reader, &attr, readerLoop, &attr);
	if (ret < 0) {
		perror ("pthread_create");
	}

	while(1){
		scanf("%s",&cmd);

		/*test for setport*/
// 		p.setDataPosition(0);
// 		p.writeInt32(RIL_REQUEST_SUPPORT_PORTS);//request
// 		p.writeInt32(2);//token
// 		p.writeInt32(1);//string count
// 		writeStringToParcel(p,"?");
// 		sendDataToRild(p);
// sleep(2);
// 		p.setDataPosition(0);
// 		p.writeInt32(RIL_REQUEST_SUPPORT_PORTS);//request
// 		p.writeInt32(2);//token
// 		p.writeInt32(1);//string count
// 		writeStringToParcel(p,"=?");
// 		sendDataToRild(p);
// sleep(2);
// 		test for cgcatt
// 		p.setDataPosition(0);
// 		p.writeInt32(RIL_REQUEST_PSCS_BIND_STATE);//request
// 		p.writeInt32(2);//token
// 		p.writeInt32(1);//string count
// 		writeStringToParcel(p,"?");
// 		sendDataToRild(p);
// sleep(2);
// 		p.setDataPosition(0);
// 		p.writeInt32(RIL_REQUEST_PSCS_BIND_STATE);//request
// 		p.writeInt32(2);//token
// 		p.writeInt32(1);//string count
// 		writeStringToParcel(p,"=?");
// 		sendDataToRild(p);
// sleep(2);
		/*test for get network*/
		test_dhcp();

		// sleep(3);
	}
	return 0;
}
