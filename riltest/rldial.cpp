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

using namespace android;

int s_send_at_fd;
static pthread_t s_tid_reader;

static pthread_mutex_t t_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t t_psbind_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t t_apn_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t t_pdp_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t t_dhcp_cond = PTHREAD_COND_INITIALIZER;

struct timeval now;
struct timespec outtime;

static int g_dial_success = 0;


#define BYTE0(d)	((d) & 0xFF)
#define BYTE1(d)	(((d) >> 8) & 0xFF)
#define BYTE2(d)	(((d)  >> 16) & 0xFF)
#define BYTE3(d)	(((d) >> 24) & 0xFF)

#define  DIAL_RESULT_FILE	"/data/dial_result"

uint32_t IpRawStrdupToHex(char *data)
{
	uint32_t  hex;
	char *out_str = (char *)alloca(16);
	sscanf(data, "%x", &hex);
	return hex;
}

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
	int32_t response_type,request,token,r_err,countStrings;
	status_t status;
	size_t datalen;
	char **pStrings = NULL;
	uint32_t ip, netmask, gateway, dhcpsvr, pdns, sdns;
	FILE * pFile;

	int fd = s_send_at_fd;
	while(1){
	            int recvLen = recv(fd,recvBuffer,4,0);
	            if(recvLen ==0){
			continue;
	            }

	            //read len
	            int messageLength = ((recvBuffer[0] & 0xff) << 24)
	                | ((recvBuffer[1] & 0xff) << 16)
	                | ((recvBuffer[2] & 0xff) << 8)
	                | (recvBuffer[3] & 0xff);
	            memset(recvBuffer, 0, 4);
	            //read data
	            recvLen = recv(fd,recvBuffer,messageLength,0);
	            recvP.setData((uint8_t*)recvBuffer,   recvLen);

	             status = recvP.readInt32(&response_type);
	             status = recvP.readInt32(&request);
		printf("response_type:%d\n", response_type);
		printf("request:%d\n", request);	
		if(response_type == 1){
			switch(request) {
				/*like ^DCONN:1,"IPV4"*/
				case  RIL_UNSOL_NDIS_DIAL:
					status = recvP.readInt32 (&countStrings);
					if (countStrings > 0) {
					    datalen = sizeof(char *) * countStrings;
					    pStrings = (char **)alloca(datalen);

					    for (int i = 0 ; i < countStrings ; i++) {
					        pStrings[i] = strdupReadString(recvP);
					    }
					}
					printf("DCONN(%s)\n", pStrings[0]);
					pthread_cond_signal(&t_apn_cond);
					break;
				default:
					break;
			}
		} else if(response_type == 0){
			status = recvP.readInt32(&token);
		             status = recvP.readInt32 (&r_err);
			printf("token:%d\n", token);
		             if(r_err){
		             	printf("recv error, r_err:%d\n\n\n\n\n\n", r_err);
		             	switch(request) {
				case RIL_REQUEST_PSCS_BIND_STATE:
				// printf("RIL_REQUEST_PSCS_BIND_STATE\n");
				pthread_cond_signal(&t_psbind_cond);
				break;
				case RIL_REQUEST_NDIS_DIAL:
				// printf("RIL_REQUEST_NDIS_DIAL\n");
				pthread_cond_signal(&t_apn_cond);
				break;
				case RIL_REQUEST_PDP_ADDR:
				// printf("RIL_REQUEST_PDP_ADDR\n");
				pthread_cond_signal(&t_pdp_cond);
				break;
				case RIL_REQUEST_DHCP_INFO:
				// printf("RIL_REQUEST_DHCP_INFO\n");
				pthread_cond_signal(&t_dhcp_cond);
				g_dial_success = 0;
				break;
				default:
					break;
				}
				continue;
		             }
			status = recvP.readInt32 (&countStrings);
			if (countStrings > 0) {
				datalen = sizeof(char *) * countStrings;
				pStrings = (char **)alloca(datalen);
				for (int i = 0 ; i < countStrings ; i++) {
					pStrings[i] = strdupReadString(recvP);
					printf("%s ", pStrings[i]);
				}
				printf("\n");
			}

			switch(request) {
				case  RIL_REQUEST_PSCS_BIND_STATE:
					if(token == 1)
						printf("EDIT PSCS OK. (%s)\n", pStrings[0]);
					else
						printf("PS:%s CS:%s\n", pStrings[0], pStrings[1]);
					pthread_cond_signal(&t_psbind_cond);
					break;
				case RIL_REQUEST_NDIS_DIAL:
					// printf("RIL_REQUEST_NDIS_DIAL finish\n");
					break;
				case  RIL_REQUEST_PDP_ADDR:
					printf("cid:%s ip:%s\n", pStrings[0], pStrings[1]);
					pthread_cond_signal(&t_pdp_cond);
					break;
				case RIL_REQUEST_DHCP_INFO:
					char cmdString[100];
					pFile = fopen (DIAL_RESULT_FILE,"w+");
					ip=IpRawStrdupToHex(pStrings[0]);//ip
					printf("address\t%d.%d.%d.%d\n", BYTE0(ip), BYTE1(ip), BYTE2(ip), BYTE3(ip));
					fprintf(pFile, "address\t%d.%d.%d.%d\n", BYTE0(ip), BYTE1(ip), BYTE2(ip), BYTE3(ip));
					netmask=IpRawStrdupToHex(pStrings[1]);//netmask
					printf("netmask\t%d.%d.%d.%d\n", BYTE0(netmask), BYTE1(netmask), BYTE2(netmask), BYTE3(netmask));
					fprintf(pFile, "netmask\t%d.%d.%d.%d\n", BYTE0(netmask), BYTE1(netmask), BYTE2(netmask), BYTE3(netmask));
					gateway=IpRawStrdupToHex(pStrings[2]);//gate
					printf("gateway\t%d.%d.%d.%d\n", BYTE0(gateway), BYTE1(gateway), BYTE2(gateway), BYTE3(gateway));
					fprintf(pFile, "gateway\t%d.%d.%d.%d\n", BYTE0(gateway), BYTE1(gateway), BYTE2(gateway), BYTE3(gateway));
					dhcpsvr=IpRawStrdupToHex(pStrings[3]);//dhcp
					printf("dhcpsvr\t%d.%d.%d.%d\n", BYTE0(dhcpsvr), BYTE1(dhcpsvr), BYTE2(dhcpsvr), BYTE3(dhcpsvr));
					fprintf(pFile, "dhcpsvr\t%d.%d.%d.%d\n", BYTE0(dhcpsvr), BYTE1(dhcpsvr), BYTE2(dhcpsvr), BYTE3(dhcpsvr));
					pdns=IpRawStrdupToHex(pStrings[4]);//pdns
					printf("pdns\t%d.%d.%d.%d\n", BYTE0(pdns), BYTE1(pdns), BYTE2(pdns), BYTE3(pdns));
					fprintf(pFile, "pdns\t%d.%d.%d.%d\n", BYTE0(pdns), BYTE1(pdns), BYTE2(pdns), BYTE3(pdns));
					sdns=IpRawStrdupToHex(pStrings[5]);//sdns
					printf("sdns\t%d.%d.%d.%d\n", BYTE0(sdns), BYTE1(sdns), BYTE2(sdns), BYTE3(sdns));
					fprintf(pFile, "sdns\t%d.%d.%d.%d\n", BYTE0(sdns), BYTE1(sdns), BYTE2(sdns), BYTE3(sdns));
					fclose(pFile);
					// sprintf(cmdString[0], "ifconfig");
					// sprintf(cmdString[1], "ethx");
					// sprintf(cmdString[2], "%d.%d.%d.%d", BYTE0(ip), BYTE1(ip), BYTE2(ip), BYTE3(ip));
					// sprintf(cmdString[3], "255.255.255.0");
					// sprintf(cmdString[4], "up");
					// cmdString[5] = NULL;
					sprintf(cmdString, "busybox ifconfig eth_x %d.%d.%d.%d netmask 255.255.255.0 up", BYTE0(ip), BYTE1(ip), BYTE2(ip), BYTE3(ip));
					printf("%s\n", cmdString);
					// execvp("busybox",cmdString);
					// system((const char*)cmdString);
					// memset(cmdString, 0, sizeof(cmdString));
					sprintf(cmdString, "busybox route add default gw %d.%d.%d.%d", BYTE0(gateway), BYTE1(gateway), BYTE2(gateway), BYTE3(gateway));
					printf("%s\n", cmdString);
					// system((const char*)cmdString);
					g_dial_success = 1;
					pthread_cond_signal(&t_dhcp_cond);
					break;
				default:
					break;
			}
		}

		if (pStrings != NULL) {
			for (int i = 0 ; i < countStrings ; i++) {
				free(pStrings[i]);
			}
		}
	}

	return NULL;
}

int ps_bind(void)
{
	Parcel p;
	int err = 0;
	printf("++++++++++ps_bind in\n");
	p.writeInt32(RIL_REQUEST_PSCS_BIND_STATE);//request
	if(1){
		p.writeInt32(0);//token
		p.writeInt32(1);//string count
		writeStringToParcel(p,"?");	
	}else{
		p.writeInt32(1);//token
		p.writeInt32(2);//string count
		writeStringToParcel(p,"1");
		writeStringToParcel(p,"3");
	}
	sendDataToRild(p);

	gettimeofday(&now, NULL);
	outtime.tv_sec = now.tv_sec + 5;
	outtime.tv_nsec = now.tv_usec * 1000;
	err = pthread_cond_timedwait(&t_psbind_cond, &t_mutex, &outtime);
	if (err == ETIMEDOUT) {
		printf("ps_bind timeout");
		return ETIMEDOUT;
	}
	printf("++++++++++ps_bind out\n");
	return 0;
}

// apn_connect(1, 1, "CMNET")
int apn_connect(int pdpid, int connect, char* apnname)
{
	Parcel p;
	int err = 0;
	char args[100];
	printf("++++++++++apn_connect in\n");
	p.writeInt32(RIL_REQUEST_NDIS_DIAL);//request
	p.writeInt32(2);//token
	p.writeInt32(1);//string count
	sprintf(args, "=%d,%d,\"%s\"", pdpid, connect, apnname);
	printf("apn_connect: %s\n", args);
	writeStringToParcel(p, (const char *)args);
	sendDataToRild(p);

	gettimeofday(&now, NULL);
	outtime.tv_sec = now.tv_sec + 5;
	outtime.tv_nsec = now.tv_usec * 1000;
	err = pthread_cond_timedwait(&t_apn_cond, &t_mutex, &outtime);
	if (err == ETIMEDOUT) {
		printf("apn_connect timeout");
		return ETIMEDOUT;
	}
	printf("++++++++++apn_connect out\n");
	return 0;
}

int get_pdp_addr(void)
{
	Parcel p;
	int err = 0;
	printf("++++++++++get_pdp_addr in\n");
	p.writeInt32(RIL_REQUEST_PDP_ADDR);//request
	p.writeInt32(3);//token
	p.writeInt32(0);//string count
	sendDataToRild(p);

	gettimeofday(&now, NULL);
	outtime.tv_sec = now.tv_sec + 5;
	outtime.tv_nsec = now.tv_usec * 1000;
	err = pthread_cond_timedwait(&t_pdp_cond, &t_mutex, &outtime);
	if (err == ETIMEDOUT) {
		printf("get_pdp_addr timeout");
		return ETIMEDOUT;
	}
	printf("++++++++++get_pdp_addr out\n");
	return 0;
}

int get_dhcp_info(void)
{
	Parcel p;
	int err = 0;
	printf("++++++++++get_dhcp_info in\n");
	p.setDataPosition(0);
	p.writeInt32(RIL_REQUEST_DHCP_INFO);//request
	p.writeInt32(4);//token
	p.writeInt32(1);//string count
	writeStringToParcel(p,"?");
	sendDataToRild(p);

	gettimeofday(&now, NULL);
	outtime.tv_sec = now.tv_sec + 5;
	outtime.tv_nsec = now.tv_usec * 1000;
	err = pthread_cond_timedwait(&t_dhcp_cond, &t_mutex, &outtime);
	if (err == ETIMEDOUT) {
		printf("get_dhcp_info timeout");
		return ETIMEDOUT;
	}
	printf("++++++++++get_dhcp_info out\n");
	return 0;
}

int dial_up(void)
{
	ps_bind();
	apn_connect(1, 1, "CMNET");
	get_pdp_addr();
	get_dhcp_info();
	if(!g_dial_success){
		printf("dial up Fail\n");
		return -1;
	}
	printf("dial up OK\n");
	return 0;
}

int main(int argc, char* argv[])
{
	int fd;
	int index;
	int args_len;
	int ret;
	void* retval;
	char cmd[20];
	pthread_t tid;
	pthread_attr_t attr;

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
		perror ("pthread_create\n");
	}
	dial_up();
	// pthread_join(s_tid_reader, &retval);
	return 0;
}
