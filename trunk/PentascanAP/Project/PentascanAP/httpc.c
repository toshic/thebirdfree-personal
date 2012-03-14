#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/api.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "FreeRTOS.h"

#define HTTP_DEBUGx

#ifdef HTTP_DEBUG
    #define DEBUG_HTTP(x)   do { printf x; } while(0)
#else
    #define DEBUG_HTTP(x)
#endif

#define PROTO_NONE  0
#define PROTO_HTTP  80
#define PROTO_HTTPS 81
#define PROTO_FTP   82

#define HTTP_BUFFER_SIZE    1024
#define LINEBUFFER_SIZE	80

typedef enum{
	http_v10,
	http_v11,
	http_content_length,
	http_content_type,
	http_transfer_encoding,

	http_resp_version,
	http_connection_close
}http_string;

static const char * const http_string_table[] = 
{
	"HTTP/1.0 ",
	"HTTP/1.1 ",
	"Content-Length: ",
	"Content-Type: ",
	"Transfer-Encoding: chunked",

	" HTTP/1.1\r\nAccept: *.*\r\n",
	"\r\nConnection: close\r\n\r\n"
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
			DEBUG_HTTP(("protocol given\n"));
			if( (ptr - url) == 4 && !strncmp(url,"http",4)){ /* assume "http" */
				DEBUG_HTTP(("http://\n")); 
				protocol = PROTO_HTTP;
			}else if( (ptr - url) == 5 && !strncmp(url,"https",5)){ /* assume "https" */
				DEBUG_HTTP(("https://\n"));
				protocol = PROTO_HTTPS;
			}else{
				strncpy(pctemp, url, ptr - url);
				pctemp[ptr - url] = 0;
				DEBUG_HTTP(("unknown protocol %s\n",pctemp));
				return -1;
			}
			url = ptr + 3;
		}else{
			DEBUG_HTTP(("protocol not given, assume http\n"));
			protocol = PROTO_HTTP;
		}
	}else{
		DEBUG_HTTP(("protocol not given, assume http\n"));
		protocol = PROTO_HTTP;
	}
	
	/* check specific port number */
	ptr = strchr(url,':');
	if(ptr){
		DEBUG_HTTP(("port assigned\n"));
		*hostname = (char *)pvPortMalloc(ptr - url + 1);
		if(!*hostname){
			DEBUG_HTTP(("Insufficient memory error\n"));
			return -1;
		}
		strncpy(*hostname,url,ptr-url);
		*hostname[ptr-url]=0;

		/* collect port number */
		url = ptr + 1;
		ptr = strchr(url,'/');
		if(ptr){
			DEBUG_HTTP(("location exist\n"));
			strncpy(pctemp,url,ptr - url);
			pctemp[ptr-url] = 0;
			*port_num = atoi(pctemp);

			*location = pvPortMalloc(strlen(ptr) + 1);
			if(!*location){
				DEBUG_HTTP(("Insufficient memory error\n"));
				vPortFree(*hostname);
				return -1;
			}
			strcpy(*location,ptr);
		}else{
			DEBUG_HTTP(("default location\n"));
			*port_num = atoi(url);
			
			*location = pvPortMalloc(2);
			if(!*location){
				DEBUG_HTTP(("Insufficient memory error\n"));
				vPortFree(*hostname);
				return -1;
			}
			strcpy(*location,"/");
		}				 

	}else{
		DEBUG_HTTP(("port is default 80\n"));
		*port_num = 80;

		ptr = strchr(url,'/');
		if(ptr){
		    int i;
			DEBUG_HTTP(("location exist\n"));

			*hostname = (char *)pvPortMalloc(ptr - url + 1);
			if(!*hostname){
				DEBUG_HTTP(("Insufficient memory error\n"));
				return -1;
			}
			strncpy(*hostname,url,ptr-url);
			*hostname[ptr-url]=0;
			
            for(i=0;i<100;i++){
                char ttt[2];
                if(ptr[i] == NULL)
                    break;
                ttt[0] = ptr[i];
                ttt[1] = 0;
                RIT128x96x4StringDraw(ttt,0,0,15);
            }
			*location = pvPortMalloc(strlen(ptr) + 1);
			if(!*location){
				DEBUG_HTTP(("Insufficient memory error\n"));
				vPortFree(*hostname);
				return -1;
			}
			strcpy(*location,ptr);
		}else{
			DEBUG_HTTP(("default location\n"));
			*hostname = pvPortMalloc(strlen(url) + 1);
			if(!*hostname){
				DEBUG_HTTP(("Insufficient memory error\n"));
				return -1;
			}
			strcpy(*hostname,url);

			*location = pvPortMalloc(2);
			if(!*location){
				DEBUG_HTTP(("Insufficient memory error\n"));
				vPortFree(*hostname);
				return -1;
			}
			strcpy(*location,"/");
		}
	}

	DEBUG_HTTP(("url = %s\n",url));
	DEBUG_HTTP(("\thost = %s\n",*hostname));
	DEBUG_HTTP(("\tport = %d\n",*port_num));
	DEBUG_HTTP(("\tloc = %s\n",*location));
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
			DEBUG_HTTP((">>%s",linebuffer));
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
        DEBUG_HTTP(("socket create fail\n"));
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
            DEBUG_HTTP(("host addr %s can't resolved\n",hostname));
            return http_status;
        }
    }

    DEBUG_HTTP(("host name = %s, host ip = %s\n",hostname,ipaddr_ntoa((ip_addr_t*)&sock_addr.sin_addr.s_addr)));

    ret = connect(http_socket, (struct sockaddr*)(&sock_addr), sizeof(sock_addr));

    if(ret != 0){
        DEBUG_HTTP(("socket connect fail\n"));
        return http_status;
    }

    /* send url - use MSG_MORE to save buffer memory */

	ret = send(http_socket,"GET ",4,MSG_MORE);
	if(ret > 0)
		ret = send(http_socket,location,strlen(location),MSG_MORE);
	if(ret > 0)
		ret = send(http_socket,http_string_table[http_resp_version],strlen(http_string_table[http_resp_version]),MSG_MORE);
	if(ret > 0)
		ret = send(http_socket,"Host: ",6,MSG_MORE);
	if(ret > 0)
		ret = send(http_socket,hostname,strlen(hostname),MSG_MORE);
	if(ret > 0)
		ret = send(http_socket,http_string_table[http_connection_close],strlen(http_string_table[http_connection_close]),0);

	if(ret < 0){
		DEBUG_HTTP(("socket send fail\n"));
		close(http_socket);
		return http_status;
	}
	

    /* receive data */
	ptr = GetLine(http_socket);

	while ( ptr && strcmp(ptr,"\r\n") )
	{
		if(!strncmp(ptr,http_string_table[http_v10],strlen(http_string_table[http_v10])))
		{
			http_status = atoi(ptr + strlen(http_string_table[http_v10]));
		}
		else if(!strncmp(ptr,http_string_table[http_v11],strlen(http_string_table[http_v11])))
		{
			http_status = atoi(ptr + strlen(http_string_table[http_v11]));
		}
		else if(!strncmp(ptr,http_string_table[http_content_length],strlen(http_string_table[http_content_length])))
		{
			http_length = atol(ptr + strlen(http_string_table[http_content_length]));
		}
		else if(!strncmp(ptr,http_string_table[http_transfer_encoding],strlen(http_string_table[http_transfer_encoding])))
		{
			http_chunked = 1;
		}
		else if(!strncmp(ptr,http_string_table[http_content_type],strlen(http_string_table[http_content_type])))
		{
		}

		ptr = GetLine(http_socket);
	}

	DEBUG_HTTP(("http result code = %d\n",http_status));
	DEBUG_HTTP(("content length = %ld\n",http_length));

	if(http_status != 200){
		close(http_socket);
		return http_status;
	}

	recv_buffer = pvPortMalloc(HTTP_BUFFER_SIZE);
	if(!recv_buffer){
		DEBUG_HTTP(("memory allocation fail\n"));
		close(http_socket);
		return -1;
	}
	
	if(http_chunked)
	{
		DEBUG_HTTP(("chunked format\n"));
		
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
#if 0					
					for(i=0;i<pack_len;i++)
						printf("%c",recv_buffer[i]);
#endif						
				}
				else
				{
					close(http_socket);
					vPortFree(recv_buffer);
					return -7;
				}
			}
			// get next chunk length
			ptr = GetLine(http_socket);
			if(strcmp(ptr,"\r\n"))
				break;
			ptr = GetLine(http_socket);
			sscanf(ptr,"%x\r\n",&http_length);
		}	
	}else{
		while( (pack_len = recv(http_socket,recv_buffer,HTTP_BUFFER_SIZE,0)) > 0 )
		{
			total_length += pack_len;
#if 0					
			for(i=0;i<pack_len;i++)
				printf("%c",recv_buffer[i]);
#endif						
		}
		if(http_length != total_length){
			DEBUG_HTTP(("length mismatch\n"));
			close(http_socket);
			vPortFree(recv_buffer);
			return -2;
		}
	}

	DEBUG_HTTP(("===========\n"));
	
    close(http_socket);
	vPortFree(recv_buffer);
	return http_status;
}

int http_req(char *url)
{
    char *hostname;
    char *location;
    int port;
	int protocol;
	int code = 0;

	protocol = parseUrl(url,&hostname,&port,&location);
	if( protocol == PROTO_HTTP ){
		code = http_get(hostname,port,location);
		vPortFree(hostname);
		vPortFree(location);
	}
	return code;
}


