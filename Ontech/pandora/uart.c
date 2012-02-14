#include "uart.h"
#include <stdio.h>
#include <string.h>
#include <stream.h>
#include <source.h>
#include <sink.h>
#include <stdlib.h>

#define MAX_DEBUG_STRING	100

#define STR_NO_MEM    "UartSend Malloc Fail\n"

void UartWrite(const char *s, unsigned int n)
{
	Sink uart = StreamUartSink();
	if(uart)
	{
		if(SinkClaim(uart, n) != 0xFFFF)
		{
			memcpy(SinkMap(uart), s, n);
			SinkFlush(uart, n);
			  
		}
		else
		{
            
			if(SinkClaim(uart, 2) != 0xFFFF)
			{
				memcpy(SinkMap(uart), "!\n", 2 );
                SinkFlush(uart, 2);
			}
		}
	}
}

void UartPrintf(const char *s, ...)
{
	char *guDebugString;
	va_list argptr;

	guDebugString = malloc(MAX_DEBUG_STRING+1);
	if(guDebugString == NULL)
	{
		UartWrite(STR_NO_MEM,strlen(STR_NO_MEM));
		return;
	}

	va_start(argptr,s);

	vsprintf(guDebugString,s,argptr);
	va_end(argptr);

	UartWrite(guDebugString, strlen(guDebugString));

	free(guDebugString);
}	
