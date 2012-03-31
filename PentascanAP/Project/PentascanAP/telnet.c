#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Lwiplib.h"
#include "lwip/netdb.h"

static void telnet_proc( void *pvParameters )
{
    int client_socket = (int)pvParameters;
    char *buffer = mem_malloc(1024);
    int nbytes;

    if(buffer == NULL){
        printf("malloc fail\n");
        close(client_socket);
        return;
    }
    do{
        nbytes = recv(client_socket, buffer, sizeof(buffer),0);
        if (nbytes>0) 
            lwip_send(client_socket, buffer, nbytes, 0);
    }while(nbytes>0);
    
    mem_free(buffer);
    close(client_socket);
    
    vTaskDelete( NULL );
}

static void telnetd(void *pvParameters)
{
    int listen_socket;
    struct sockaddr_in local_addr;
    unsigned int port = (unsigned int) pvParameters;
    int i = 0;
    char child_name[6];

    listen_socket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    
    if(listen_socket < 0){
        printf("socket create fail\n");
        return;
    }

    memset((char *)&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_len = sizeof(local_addr);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(port);

    if (bind(listen_socket, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        printf("socket bind fail\n");
        close(listen_socket);
        return;
    }

    if ( listen(listen_socket, 2) != 0 ){
        printf("socket listen fail\n");
        close(listen_socket);
        return;
    }

    printf("telnet ready at port %d\n",port);

    while (1) {
        int clientfd;
        struct sockaddr_in client_addr;
        int addrlen=sizeof(client_addr);

        clientfd = accept(listen_socket, (struct sockaddr*)&client_addr, (socklen_t *)&addrlen);
        if (clientfd>0){
            sprintf(child_name,"tty%02d",i++);
            xTaskCreate( telnet_proc, ( signed portCHAR * ) child_name, 128, (void*)clientfd, tskIDLE_PRIORITY + 1, NULL );
        }
    } 
    lwip_close(listen_socket);
}

void telnet_start(unsigned int port)
{
    static int start;
    if(!start){
        /* simple way to prevent double excution */
        start = 1;
        xTaskCreate( telnetd, ( signed portCHAR * ) "telnetd", 128, (void*)port, tskIDLE_PRIORITY + 1, NULL );
    }
}

