#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/api.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "FreeRTOS.h"

#define PROTO_NONE  0
#define PROTO_HTTP  80
#define PROTO_HTTPS 81
#define PROTO_FTP   82

void http_req(char *url)
{
    char *ptr;
    char pctemp[256];

    int protocol = PROTO_NONE;
    int port;
    char *hostname;
    char *location;

    int http_socket;
    int i,ret;

    struct hostent *http_host;
    struct sockaddr_in sock_addr;
    
    /* split host, port, url */

    /* search :// */
    
    ptr = strchr(url,':');
    if(ptr){
        if(ptr[1] == '/' && ptr[2] == '/'){
            printf("protocol given\n");
            if( (ptr - url) == 4 && !strncmp(url,"http",4)){ /* assume "http" */
                printf("http://\n"); 
                protocol = PROTO_HTTP;
            }else if( (ptr - url) == 5 && !strncmp(url,"https",5)){ /* assume "https" */
                printf("https://\n");
                protocol = PROTO_HTTPS;
            }else{
                strncpy(pctemp, url, ptr - url);
                pctemp[ptr - url] = 0;
                printf("unknown protocol %s\n",pctemp);
                return;
            }
            url = ptr + 3;
        }else{
            printf("protocol not given, assume http\n");
            protocol = PROTO_HTTP;
        }
    }else{
        printf("protocol not given, assume http\n");
        protocol = PROTO_HTTP;
    }
    
    /* check specific port number */
    ptr = strchr(url,':');
    if(ptr){
        printf("port assigned\n");
        hostname = (char *)malloc(ptr - url + 1);
        strncpy(hostname,url,ptr-url);
        hostname[ptr-url]=0;

        /* collect port number */
        url = ptr + 1;
        ptr = strchr(url,'/');
        if(ptr){
            printf("location exist\n");
            strncpy(pctemp,url,ptr - url);
            pctemp[ptr-url] = 0;
            port = atoi(pctemp);

            location = malloc(strlen(ptr) + 1);
            strcpy(location,ptr);
        }else{
            printf("default location\n");
            port = atoi(url);
            
            location = malloc(2);
            strcpy(location,"/");
        }                

    }else{
        printf("port is default 80\n");
        port  = 80;

        ptr = strchr(url,'/');
        if(ptr){
            printf("location exist\n");

            hostname = (char *)malloc(ptr - url + 1);
            strncpy(hostname,url,ptr-url);
            hostname[ptr-url]=0;

            location = malloc(strlen(ptr) + 1);
            strcpy(location,ptr);
        }else{
            printf("default location\n");
            hostname = malloc(strlen(url) + 1);
            strcpy(hostname,url);

            location = malloc(2);
            strcpy(location,"/");
        }
    }

    printf("url = %s\n",url);
    printf("\thost = %s\n",hostname);
    printf("\tport = %d\n",port);
    printf("\tloc = %s\n",location);
/* host, port, location collected */  

    http_socket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    
    if(http_socket < 0){
        printf("socket create fail\n");
        return;
    }

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_len = sizeof(struct sockaddr_in);
    sock_addr.sin_port = htons(port);
    
    http_host = gethostbyname(hostname);
    /* debug */
    if(http_host){
        printf("hostent struct\n");
        printf("=================\n");
        printf("h_name = %s\n",http_host->h_name);
        if(http_host->h_aliases){
            for(ptr = http_host->h_aliases[0], i=0;ptr;i++, ptr = http_host->h_aliases[i]){
                printf("h_aliases[%d] = %s\n",i,ptr);
            }
        }
        printf("h_addrtype = %d\n",http_host->h_addrtype);
        printf("h_length = %d\n",http_host->h_length);
        if(http_host->h_addr_list){
            for(ptr = http_host->h_addr_list[0], i=0;ptr;i++, ptr = http_host->h_addr_list[i]){
                printf("h_addr_list[%d] = \n",i);
    			printf("\taddr[0] = %d\n",ptr[0]);
    			printf("\taddr[1] = %d\n",ptr[1]);
    			printf("\taddr[2] = %d\n",ptr[2]);
    			printf("\taddr[3] = %d\n",ptr[3]);
            }
        }
    }    
    if (http_host != NULL && http_host->h_addrtype == AF_INET) {
        sock_addr.sin_addr.s_addr = *(u32_t *)http_host->h_addr;
    } else {
        if(!inet_aton(hostname, &(sock_addr.sin_addr))){
            printf("host addr %s can't resolved\n",hostname);
            return;
        }
    }

    printf("host name = %s, host ip = %s\n",hostname,ipaddr_ntoa((ip_addr_t*)&sock_addr.sin_addr.s_addr));

    ret = connect(http_socket, (struct sockaddr*)(&sock_addr), sizeof(sock_addr));

    if(ret != 0){
        printf("socket connect fail\n");
        return;
    }

    /* send url */
    ret = send(http_socket,"GET /\r\n", strlen("GET /\r\n"),0);
    if(ret < 0){
        printf("socket send fail\n");
        return;
    }
    
    
    /* receive data */
    while(ret>0){
        ret = recv(http_socket,pctemp,256,0);
        if(ret < 0){
            printf("socket recv fail %d\n",ret);
            break;;
        }
        for(i=0;i<ret;i++){
            printf("%c",pctemp[i]);
        }
    }
    printf("\n==================\n");
    
    close(http_socket);
 
    free(hostname);
    free(location);
}
