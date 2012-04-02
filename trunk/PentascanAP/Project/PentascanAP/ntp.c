#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "Lwiplib.h"
#include "lwip/netdb.h"
#include "rtc.h"

#define SNTP_PORT                   123
/** SNTP receive timeout - in milliseconds */
#define SNTP_RECV_TIMEOUT           3000
/** SNTP update delay - in milliseconds */
#define SNTP_UPDATE_DELAY           60000

/* SNTP protocol defines */
#define SNTP_MAX_DATA_LEN           48
#define SNTP_RCV_TIME_OFS           32
#define SNTP_LI_NO_WARNING          0x00
#define SNTP_VERSION               (4/* NTP Version 4*/<<3) 
#define SNTP_MODE_CLIENT            0x03
#define SNTP_MODE_SERVER            0x04
#define SNTP_MODE_BROADCAST         0x05
#define SNTP_MODE_MASK              0x07

/* number of seconds between 1900 and 1970 */
#define DIFF_SEC_1900_1970         (2208988800LL)

void sntp_request(char *hostname) 
{
	int sock;
    struct hostent *ntp_server;
	struct sockaddr_in local;
	struct sockaddr_in to;
	int tolen;
	int size;
	int timeout;
	u8_t *sntp_buffer;
	u32_t timestamp;
	time_t t;

    printf("sntp_request %s\n",hostname);
    
	/* create new socket */
	sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock < 0){
        printf("socket create fail\n");
        return;
    }

	/* prepare local address */
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons(INADDR_ANY);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	/* bind to local address */
	if (bind( sock, (struct sockaddr *)&local, sizeof(local)) != 0) {
	    printf("bind fail\n");
        closesocket(sock);
	    return;
	}
	/* set recv timeout */
	timeout = SNTP_RECV_TIMEOUT;
	setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

	sntp_buffer = mem_malloc(SNTP_MAX_DATA_LEN);
	if(sntp_request == NULL){
        printf("malloc fail\n");
        return;
    }	

	/* prepare SNTP request */
	memset(sntp_buffer, 0, SNTP_MAX_DATA_LEN);
	sntp_buffer[0] = SNTP_LI_NO_WARNING | SNTP_VERSION | SNTP_MODE_CLIENT;

	/* prepare SNTP server address */
	memset(&to, 0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_port = htons(SNTP_PORT);

    ntp_server = gethostbyname(hostname);

    if (ntp_server != NULL && ntp_server->h_addrtype == AF_INET) {
        to.sin_addr.s_addr = *(u32_t *)ntp_server->h_addr;
    } else {
	    printf("dns fail\n");
        closesocket(sock);
        mem_free(sntp_buffer);
	    return;
    }

    printf("host name = %s, host ip = %s\n",hostname,ipaddr_ntoa((ip_addr_t*)&to.sin_addr.s_addr));

	/* send SNTP request to server */
	if (sendto( sock, sntp_buffer, SNTP_MAX_DATA_LEN, 0, (struct sockaddr *)&to, sizeof(to)) < 0){
	    printf("send fail\n");
        closesocket(sock);
        mem_free(sntp_buffer);
        return;
	}
	/* receive SNTP server response */
	tolen = sizeof(to);
	size = recvfrom( sock, sntp_buffer, SNTP_MAX_DATA_LEN, 0, (struct sockaddr *)&to, (socklen_t *)&tolen);

	/* if the response size is good */
	if (size == SNTP_MAX_DATA_LEN) {
		/* if this is a SNTP response... */
		if (((sntp_buffer[0] & SNTP_MODE_MASK) == SNTP_MODE_SERVER) || 
		    ((sntp_buffer[0] & SNTP_MODE_MASK) == SNTP_MODE_BROADCAST)) {
			/* extract GMT time from response */
			SMEMCPY( &timestamp, (sntp_buffer+SNTP_RCV_TIME_OFS), sizeof(timestamp));
			t = (ntohl(timestamp) - DIFF_SEC_1900_1970);
			t += 9 * 3600;
            RtcSetTime(t);
            printf("time set\n");
		} else {
			printf("sntp_request: not response frame code\n");
		}
	} else {
		printf("sntp_request: not recvfrom==\n");
	}
	closesocket(sock);
    mem_free(sntp_buffer);
}


