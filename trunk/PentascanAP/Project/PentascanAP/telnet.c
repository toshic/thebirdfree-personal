#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Lwiplib.h"

#include "console.h"

#define MAX_TELNET_SESSION  2
#define TELNET_RCV_BUFFER_LENGTH    1024

struct __FILE { int handle; /* Add whatever you need here */ };

static FILE g_telnet_handle[MAX_TELNET_SESSION];

void telnet_putchar_all(char ch)
{
    int i;
    for(i=0;i<MAX_TELNET_SESSION;i++){
        if(g_telnet_handle[i].handle)
            send(g_telnet_handle[i].handle, &ch, 1, 0);
    }    
}

int telnet_putchar(FILE *file, char ch)
{
    int i;
    if(file->handle)
        send(file->handle, &ch, 1, 0);

    return ch;
}

FILE *telnet_add_tty(int socket)
{
    int i;
    for(i=0;i<MAX_TELNET_SESSION;i++){
        if(g_telnet_handle[i].handle == 0){
            g_telnet_handle[i].handle = socket;
            return &g_telnet_handle[i];
        }
    }
    return NULL;
}

static void telnet_del_tty(int socket)
{
    int i;
    for(i=0;i<MAX_TELNET_SESSION;i++){
        if(g_telnet_handle[i].handle == socket)
            g_telnet_handle[i].handle = 0;
    }
}

static void telnet_proc( void *pvParameters )
{
    int client_socket = (int)pvParameters;
    char *buffer = mem_malloc(TELNET_RCV_BUFFER_LENGTH);
    line_buffer *cmd_buffer;
    int i, nbytes;

    if(buffer == NULL){
        printf("buffer malloc fail\n");
        close(client_socket);
        vTaskDelete( NULL );
        return;
    }

    cmd_buffer = console_buffer_get(100);
    if(cmd_buffer == NULL){
        printf("cmd buffer malloc fail\n");
        mem_free(buffer);
        close(client_socket);
        vTaskDelete( NULL );
        return;
    }

    cmd_buffer->file = telnet_add_tty(client_socket);
    do{
        nbytes = recv(client_socket, buffer, TELNET_RCV_BUFFER_LENGTH,0);
        for(i=0;i<nbytes;i++){
            console_parse(cmd_buffer,buffer[i]);
        }
    }while(nbytes>0);
    
    mem_free(buffer);
    mem_free(cmd_buffer);
    telnet_del_tty(client_socket);
    close(client_socket);
    vTaskDelete( NULL );
    return;
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

    if ( listen(listen_socket, MAX_TELNET_SESSION) != 0 ){
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
            xTaskCreate( telnet_proc, ( signed portCHAR * ) child_name, 256, (void*)clientfd, tskIDLE_PRIORITY + 1, NULL );
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

