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

#define HTTP_BUFFER_SIZE    1024
#define LINEBUFFER_SIZE	256

typedef enum{
	http_v10,
	http_v11,
	http_content_length,
	http_content_type,
	http_transfer_encoding,
}http_string;

static const char * const http_string_table[] = 
{
	"HTTP/1.0 ",
	"HTTP/1.1 ",
	"Content-Length: ",
	"Content-Type: ",
	"Transfer-Encoding: chunked"
};



/* split host, port, url */
static int parseUrl(char *url, char **hostname, unsigned int *port_num, char **location)
{
    char *ptr;
    char pctemp[LINEBUFFER_SIZE];
    int protocol = PROTO_NONE;

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
				return -1;
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
		*hostname = (char *)malloc(ptr - url + 1);
		if(!*hostname){
			printf("Insufficient memory error\n");
			return -1;
		}
		strncpy(*hostname,url,ptr-url);
		*hostname[ptr-url]=0;

		/* collect port number */
		url = ptr + 1;
		ptr = strchr(url,'/');
		if(ptr){
			printf("location exist\n");
			strncpy(pctemp,url,ptr - url);
			pctemp[ptr-url] = 0;
			*port_num = atoi(pctemp);

			*location = malloc(strlen(ptr) + 1);
			if(!*location){
				printf("Insufficient memory error\n");
				free(*hostname);
				return -1;
			}
			strcpy(*location,ptr);
		}else{
			printf("default location\n");
			*port_num = atoi(url);
			
			*location = malloc(2);
			if(!*location){
				printf("Insufficient memory error\n");
				free(*hostname);
				return -1;
			}
			strcpy(*location,"/");
		}				 

	}else{
		printf("port is default 80\n");
		*port_num = 80;

		ptr = strchr(url,'/');
		if(ptr){
			printf("location exist\n");

			*hostname = (char *)malloc(ptr - url + 1);
			if(!*hostname){
				printf("Insufficient memory error\n");
				return -1;
			}
			strncpy(*hostname,url,ptr-url);
			*hostname[ptr-url]=0;

			*location = malloc(strlen(ptr) + 1);
			if(!*location){
				printf("Insufficient memory error\n");
				free(*hostname);
				return -1;
			}
			strcpy(*location,ptr);
		}else{
			printf("default location\n");
			*hostname = malloc(strlen(url) + 1);
			if(!*hostname){
				printf("Insufficient memory error\n");
				return -1;
			}
			strcpy(*hostname,url);

			*location = malloc(2);
			if(!*location){
				printf("Insufficient memory error\n");
				free(*hostname);
				return -1;
			}
			strcpy(*location,"/");
		}
	}

	printf("url = %s\n",url);
	printf("\thost = %s\n",*hostname);
	printf("\tport = %d\n",*port_num);
	printf("\tloc = %s\n",*location);
	return protocol;
}

static char *GetLine(int sock)
{
	int pack_len;
	static char linebuffer[LINEBUFFER_SIZE];
	static int index;
	char c;
	while((pack_len = recv(sock,&c,1,0)) == 1)
	{
		if(c=='\n' && index > 0 && linebuffer[index-1] == '\r')
		{
			linebuffer[index++] = c;
			linebuffer[index] = '\0';
			index = 0;
			printf(">>%s",linebuffer);
			return linebuffer;
		}			
		else
		{
			linebuffer[index] = c;
			if(index < LINEBUFFER_SIZE - 2 ) // for \r\n\0
			{
				index++;
			}else
			{
				// skip overflow data
			}
		}
	}
	return NULL;
}


int http_get(char *hostname, unsigned short port, char *location)
{
    char *ptr;
    char *recv_buffer;

    int http_socket;
	int ret_code = -1;
    int i,ret;
	
	int http_chunked = 0;
	int http_status = 0;
	long http_length = 0,total_length = 0, chunk_length = 0;
	int pack_len;

    struct hostent *http_host;
    struct sockaddr_in sock_addr;

	/* need lock for thread safe */

    http_socket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    
    if(http_socket < 0){
        printf("socket create fail\n");
        return http_status;
    }

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_len = sizeof(struct sockaddr_in);
    sock_addr.sin_port = htons(port);
    
    http_host = gethostbyname(hostname);

    if (http_host != NULL && http_host->h_addrtype == AF_INET) {
        sock_addr.sin_addr.s_addr = *(u32_t *)http_host->h_addr;
    } else {
        if(!inet_aton(hostname, &(sock_addr.sin_addr))){
            printf("host addr %s can't resolved\n",hostname);
            return http_status;
        }
    }

    printf("host name = %s, host ip = %s\n",hostname,ipaddr_ntoa((ip_addr_t*)&sock_addr.sin_addr.s_addr));

    ret = connect(http_socket, (struct sockaddr*)(&sock_addr), sizeof(sock_addr));

    if(ret != 0){
        printf("socket connect fail\n");
        return http_status;
    }

    /* send url - use MSG_MORE to save buffer memory */

	ret = send(http_socket,"GET ",MSG_MORE);
	if(ret > 0)
		ret = send(http_socket,location,MSG_MORE);
	if(ret > 0)
		ret = send(http_socket," HTTP/1.1\r\nAccept: *.*\r\n",MSG_MORE);
	if(ret > 0)
		ret = send(http_socket,"Host: ",MSG_MORE);
	if(ret > 0)
		ret = send(http_socket,hostname,MSG_MORE);
	if(ret > 0)
		ret = send(http_socket,"\r\nConnection: close\r\n\r\n",0);

	if(ret < 0){
		printf("socket send fail\n");
		close(http_socket);
		return http_status;
	}
	

    /* receive data */
	ptr = GetLine(http_socket);

	while ( ptr && strcmp(ptr,"\r\n") )
	{
		if(!strncmp(ptr,http_string_table[http_v10],sizeof(http_string_table[http_v10])))
		{
			http_status = atoi(ptr + sizeof(http_string_table[http_v10]));
		}
		else if(!strncmp(ptr,http_string_table[http_v11],sizeof(http_string_table[http_v11])))
		{
			http_status = atoi(ptr + sizeof(http_string_table[http_v11]));
		}
		else if(!strncmp(ptr,http_string_table[http_content_length],sizeof(http_string_table[http_content_length])))
		{
			http_length = atol(ptr + sizeof(http_string_table[http_content_length]));
		}
		else if(!strncmp(ptr,http_string_table[http_transfer_encoding],sizeof(http_string_table[http_transfer_encoding])))
		{
			http_chunked = 1;
		}
		else if(!strncmp(ptr,http_string_table[http_content_type],sizeof(http_string_table[http_content_type])))
		{
		}

		ptr = GetLine(http_socket);
	}

	if(http_status != 200){
		close(http_socket);
		return http_status;
	}

	recv_buffer = malloc(HTTP_BUFFER_SIZE);
	if(!recv_buffer){
		printf("memory allocation fail\n");
		close(http_socket);
		return http_status;
	}
	
	if(http_chunked)
	{
		printf("chunked format\n"); 
		
		// get first chunk length
		ptr = GetLine(http_socket);
		sscanf(ptr,"%x\r\n",&http_length);
		while(http_length)
		{
			chunk_length = 0;
			while(chunk_length < http_length)
			{
				pack_len = recv(http_socket,recv_buffer,http_length-chunk_length,0);
				if(pack_len > 0)
				{
					total_length += pack_len;
					chunk_length += pack_len;
					for(i=0;i<pack_len;i++)
						printf("%c",recv_buffer[i]);
				}
				else
				{
					close(http_socket);
					free(recv_buffer);
					return -7;
				}
			}
			// get next chunk length
			ptr = GetLine(http_socket);
			if(strcmp(line,"\r\n"))
				break;
			ptr = GetLine(http_socket);
			sscanf(ptr,"%x\r\n",&http_length);
		}	
	}else{
		while( (pack_len = recv(http_socket,recv_buffer,HTTP_BUFFER_SIZE),0) > 0 )
		{
			total_length += pack_len;
			for(i=0;i<pack_len;i++)
				printf("%c",recv_buffer[i]);
		}
		if(http_length != total_length){
			printf("length mismatch\n");
			close(http_socket);
			free(recv_buffer);
			return http_status;
		}
	}
	
    close(http_socket);
	free(recv_buffer);
	return http_status;
}

void http_req(char *url)
{
    char *hostname;
    char *location;
    int port;
	int protocol;

	protocol = parseUrl(url,&hostname,&port,&location);
	if( protocol == PROTO_HTTP ){
		http_get(hostname,port,location);
		free(hostname);
		free(location);
	}
}


