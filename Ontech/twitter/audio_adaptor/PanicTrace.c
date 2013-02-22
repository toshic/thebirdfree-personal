#include <panic.h>
#include <stdio.h>
#include <string.h>
#include "uart.h"

#ifdef PANIC_TRACE
void PANIC(const char *file,size_t line)
{
	char str_line[6];
	UartWrite("PANIC ",6);
	UartWrite(file,strlen(file));
	sprintf(str_line,"%d\n",line);
	UartWrite(str_line,strlen(str_line));
	for(;;);
}
void *PANICNULL(void *pv,const char *file,size_t line)
{
	char str_line[6];
	if(pv)
		return pv;
	
	UartWrite("PANICNULL ",10);
	UartWrite(file,strlen(file));
	sprintf(str_line,"%d\n",line);
	UartWrite(str_line,strlen(str_line));
	for(;;);
}
void PANICNOTNULL(const void *pv,const char *file,size_t line)
{
	char str_line[6];
	if(pv == NULL)
		return;
	
	UartWrite("PANICNOTNULL ",13);
	UartWrite(file,strlen(file));
	sprintf(str_line,"%d\n",line);
	UartWrite(str_line,strlen(str_line));
	for(;;);
}
void *PANICUNLESSMALLOC(size_t sz,const char *file,size_t line)
{
	char str_line[6];
	void *pv;
	pv = malloc(sz);
	if(pv)
		return pv;

	UartWrite("PANICUNLESSMALLOC ",18);
	UartWrite(file,strlen(file));
	sprintf(str_line,"%d\n",line);
	UartWrite(str_line,strlen(str_line));
	for(;;);
}
#endif
