#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Lwiplib.h"
#include "lwip/netdb.h"
#include "httpc.h"
#include "rtc.h"

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

#define HTTP_BUFFER_SIZE    2048
#define LINEBUFFER_SIZE	160

typedef enum{
	http_v10,
	http_v11,
	http_content_length,
	http_content_type,
	http_transfer_encoding,
    http_date,
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
    "Date: ",
	" HTTP/1.1\r\nAccept: *.*\r\n",
	"\r\nConnection: close\r\n\r\n"
};

/* split host, port, url */
static int parseUrl(char *url, char **hostname, unsigned int *port_num, char **location)
{
    char *ptr;
    char pctemp[LINEBUFFER_SIZE];
    int protocol = PROTO_NONE;

	printf("**hostname = %p\n",hostname);

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
		*hostname = (char *)mem_malloc(ptr - url + 1);
		if(!*hostname){
			DEBUG_HTTP(("Insufficient memory error\n"));
			return -1;
		}
		strncpy(*hostname,url,ptr-url);
		(*hostname)[ptr-url]=0;

		/* collect port number */
		url = ptr + 1;
		ptr = strchr(url,'/');
		if(ptr){
			DEBUG_HTTP(("location exist\n"));
			strncpy(pctemp,url,ptr - url);
			pctemp[ptr-url] = 0;
			*port_num = atoi(pctemp);

			*location = mem_malloc(strlen(ptr) + 1);
			if(!*location){
				DEBUG_HTTP(("Insufficient memory error\n"));
				mem_free(*hostname);
				return -1;
			}
			strcpy(*location,ptr);
		}else{
			DEBUG_HTTP(("default location\n"));
			*port_num = atoi(url);
			
			*location = mem_malloc(2);
			if(!*location){
				DEBUG_HTTP(("Insufficient memory error\n"));
				mem_free(*hostname);
				return -1;
			}
			strcpy(*location,"/");
		}				 

	}else{
		DEBUG_HTTP(("port is default 80\n"));
		*port_num = 80;

		ptr = strchr(url,'/');
		if(ptr){
			DEBUG_HTTP(("location exist\n"));

			*hostname = (char *)mem_malloc(ptr - url + 1);
			if(!*hostname){
				DEBUG_HTTP(("Insufficient memory error\n"));
				return -1;
			}
			strncpy(*hostname,url,ptr-url);
			(*hostname)[ptr-url]=0;
			
			*location = mem_malloc(strlen(ptr) + 1);
			if(!*location){
				DEBUG_HTTP(("Insufficient memory error\n"));
				mem_free(*hostname);
				return -1;
			}
			strcpy(*location,ptr);
		}else{
			DEBUG_HTTP(("default location\n"));
			*hostname = mem_malloc(strlen(url) + 1);
			if(!*hostname){
				DEBUG_HTTP(("Insufficient memory error\n"));
				return -1;
			}
			strcpy(*hostname,url);

			*location = mem_malloc(2);
			if(!*location){
				DEBUG_HTTP(("Insufficient memory error\n"));
				mem_free(*hostname);
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

static char *GetLine(int sock, char *buffer, int buffer_len)
{
	int pack_len;
	int index = 0;
	char last_c;
	char c;
	while((pack_len = recv(sock,&c,1,0)) == 1)
	{
        if(index < buffer_len - 1)
            buffer[index++] = c;	
            
		if(c=='\n' && index > 0 && last_c == '\r')
		{
			buffer[index] = '\0';
			index = 0;
			DEBUG_HTTP((">>%s",buffer));
			return buffer;
		}		
		last_c = c;
	}
	return NULL;
}


int http_get(char *hostname, unsigned short port, char *location, http_parse_cb callback, void *pv)
{
    char *ptr;
    char *recv_buffer;
    char *line_buffer;

    int http_socket;
	int ret_code = -1;
    int i,ret;
	
	int http_chunked = 0;
	int http_status = 0;
	long http_length = 0, total_length = 0, chunk_length = 0, remain_length;
	int pack_len;

    struct hostent *http_host;
    struct sockaddr_in sock_addr;

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
            close(http_socket);
            return http_status;
        }
    }

    DEBUG_HTTP(("host name = %s, host ip = %s\n",hostname,ipaddr_ntoa((ip_addr_t*)&sock_addr.sin_addr.s_addr)));

    ret = connect(http_socket, (struct sockaddr*)(&sock_addr), sizeof(sock_addr));

    if(ret != 0){
        DEBUG_HTTP(("socket connect fail\n"));
		close(http_socket);
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
    line_buffer = mem_malloc(256);
    if(line_buffer == NULL){
		DEBUG_HTTP(("line buffer malloc fail\n"));
		close(http_socket);
        return http_status;
    }
	ptr = GetLine(http_socket,line_buffer, 256);

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
		else if(!strncmp(ptr,http_string_table[http_date],strlen(http_string_table[http_date])))
		{
		    /* date set usign http date */
		    unsigned long http_time, system_time;
		    system_time = RtcGetTime();
		    http_time = RtcConvertDateString(ptr + strlen(http_string_table[http_date]));
		    if(http_time){
		        if(system_time > http_time){
		            if(system_time - http_time > 10 * 60) // 10 minute
		                RtcSetTime(http_time);
		        }else{
		            if(http_time - system_time > 10 * 60) // 10 minute
		                RtcSetTime(http_time);
		        }
		    }
		}
		else if(!strncmp(ptr,http_string_table[http_content_type],strlen(http_string_table[http_content_type])))
		{
		}

        ptr = GetLine(http_socket,line_buffer, 256);
	}

	DEBUG_HTTP(("http result code = %d\n",http_status));
	DEBUG_HTTP(("content length = %ld\n",http_length));

	recv_buffer = mem_malloc(HTTP_BUFFER_SIZE);
	if(!recv_buffer){
		DEBUG_HTTP(("memory allocation fail\n"));
		close(http_socket);
        mem_free(line_buffer);
		return -1;
	}
	
	if(http_chunked)
	{
		DEBUG_HTTP(("chunked format\n"));
		
		// get first chunk length
        ptr = GetLine(http_socket,line_buffer, 256);
		sscanf(ptr,"%x\r\n",&chunk_length);
		DEBUG_HTTP(("chunk length = %d\n",chunk_length));
		while(chunk_length)
		{
		    remain_length = chunk_length;
			while(remain_length)
			{
			    if(remain_length >= HTTP_BUFFER_SIZE)
    				pack_len = recv(http_socket,recv_buffer,HTTP_BUFFER_SIZE,0);
    			else
    				pack_len = recv(http_socket,recv_buffer,remain_length,0);

				if(pack_len > 0)
				{
					total_length += pack_len;
					remain_length -= pack_len;

					/* process data here */
					if(callback)
					    (callback)(pack_len, recv_buffer, pv);
#if 0
					for(i=0;i<pack_len;i++)
						printf("%c",recv_buffer[i]);
#endif						
				}
				else
				{
					close(http_socket);
					mem_free(recv_buffer);
                    mem_free(line_buffer);
					return -7;
				}
			}
            ptr = GetLine(http_socket,line_buffer, 256);
            if(strcmp(ptr,"\r\n")){
			    DEBUG_HTTP(("abnormal chunk, abort\n"));
			    /* consume remaining buffer, to prevent memory leak obsereved in lwip */
			    while(recv(http_socket,recv_buffer,remain_length,0) > 0);
			    break;
			}			
			// get next chunk length
            ptr = GetLine(http_socket,line_buffer, 256);
			sscanf(ptr,"%x\r\n",&chunk_length);
			if(chunk_length == 0){
                ptr = GetLine(http_socket,line_buffer, 256);
                if(strcmp(ptr,"\r\n")){
                    DEBUG_HTTP(("abnormal termination, abort\n"));
                }
			    break;
			}
            DEBUG_HTTP(("chunk length = %d\n",chunk_length));
		}	
		DEBUG_HTTP(("total length = %d\n",total_length));
	}else{
		while( (pack_len = recv(http_socket,recv_buffer,HTTP_BUFFER_SIZE,0)) > 0 )
		{
			total_length += pack_len;
            /* process data here */
            if(callback)
                (callback)(pack_len, recv_buffer, pv);
#if 0
			for(i=0;i<pack_len;i++)
				printf("%c",recv_buffer[i]);
#endif						
		}
		if(http_length == 0){
			DEBUG_HTTP(("length unknown\n"));
		}else if(http_length != total_length){
			DEBUG_HTTP(("length mismatch\n"));
			close(http_socket);
			mem_free(recv_buffer);
            mem_free(line_buffer);
			return -2;
		}
	}

	DEBUG_HTTP(("===========\n"));
	
    close(http_socket);
	mem_free(recv_buffer);
    mem_free(line_buffer);
	return http_status;
}

int http_req(char *url, http_parse_cb callback, void *pv)
{
    char *hostname;
    char *location;
    int port;
	int protocol;
	int code = 0;

	protocol = parseUrl(url,&hostname,&port,&location);
	printf("hostname = \"%s\"\n",hostname);
	printf("location = \"%s\"\n",location);

	if( protocol == PROTO_HTTP ){
		code = http_get(hostname,port,location, callback, pv);
		mem_free(hostname);
		mem_free(location);
	}
	return code;
}


