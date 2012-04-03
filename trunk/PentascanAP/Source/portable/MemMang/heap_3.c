/*
    FreeRTOS V7.1.0 - Copyright (C) 2011 Real Time Engineers Ltd.
	

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/


/*
 * Implementation of pvPortMalloc() and vPortFree() that relies on the
 * compilers own malloc() and free() implementations.
 *
 * This file can only be used if the linker is configured to to generate
 * a heap memory area.
 *
 * See heap_2.c and heap_1.c for alternative implementations, and the memory
 * management pages of http://www.FreeRTOS.org for more information.
 */

#include <string.h>
#include <stdlib.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#if( configUSE_MEMALLOCTRACE != 1)
/*-----------------------------------------------------------*/

void *pvPortMalloc( size_t xWantedSize )
{
void *pvReturn;

	vTaskSuspendAll();
	{
		pvReturn = malloc( xWantedSize );
	}
	xTaskResumeAll();

	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if( pvReturn == NULL )
		{
			extern void vApplicationMallocFailedHook( void );
			vApplicationMallocFailedHook();
		}
	}
	#endif
	
	return pvReturn;
}

void *pvPortCalloc( size_t xWantedNum, size_t xWantedSize  )
{
void *pvReturn;

	vTaskSuspendAll();
	{
		pvReturn = calloc( xWantedNum, xWantedSize );
	}
	xTaskResumeAll();

	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if( pvReturn == NULL )
		{
			extern void vApplicationMallocFailedHook( void );
			vApplicationMallocFailedHook();
		}
	}
	#endif
	
	return pvReturn;
}

/*-----------------------------------------------------------*/

void vPortFree( void *pv )
{
	if( pv )
	{
		vTaskSuspendAll();
		{
			free( pv );
		}
		xTaskResumeAll();
	}
}

#else
#define MAX_LIST 128

#define SHOW_ALL_HOOKx

struct {
    char *file;
    unsigned long line;
}
white_list[] = 
{
    "..\\Common\\ethernet\\lwip-1.4.0\\src\\core\\tcp.c",1185
};

static struct {
    void *pv;
    size_t size;
    char *file;
    unsigned long line;
}alloctable[MAX_LIST];

static unsigned long max_alloc_size;
static unsigned long total_alloc_size;
static int max_entry;
static int total_entry;

static void *_pvPortMalloc( size_t xWantedSize )
{
void *pvReturn;

	vTaskSuspendAll();
	{
		pvReturn = malloc( xWantedSize );
	}
	xTaskResumeAll();

	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if( pvReturn == NULL )
		{
			extern void vApplicationMallocFailedHook( void );
			vApplicationMallocFailedHook();
		}
	}
	#endif
	
	return pvReturn;
}

static void *_pvPortCalloc( size_t xWantedNum, size_t xWantedSize  )
{
void *pvReturn;

	vTaskSuspendAll();
	{
		pvReturn = calloc( xWantedNum, xWantedSize );
	}
	xTaskResumeAll();

	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if( pvReturn == NULL )
		{
			extern void vApplicationMallocFailedHook( void );
			vApplicationMallocFailedHook();
		}
	}
	#endif
	
	return pvReturn;
}

/*-----------------------------------------------------------*/

static void _vPortFree( void *pv )
{
	if( pv )
	{
		vTaskSuspendAll();
		{
			free( pv );
		}
		xTaskResumeAll();
	}
}


void free_hook(void *pv)
{
    int i,j;
    char *str;
    _vPortFree(pv);
    if(pv){
        for(i=0;i<MAX_LIST;i++){
            if(pv == alloctable[i].pv){
                total_alloc_size -= alloctable[i].size;
                total_entry--;
                
                for(j=0;j<sizeof(white_list);j++){
                    if(!strcmp(alloctable[i].file,white_list[j].file) && alloctable[i].line == white_list[j].line){
                        str = strrchr(alloctable[i].file,'\\');
                        if(str == NULL)
                            str = alloctable[i].file;
                        printf("free size = %ld[%p](%s:%ld)\n",alloctable[i].size,pv,str,alloctable[i].line );
                    }
                }
                alloctable[i].pv = NULL;
                break;
            }
        }
    }else{
        printf("#### Freeing null pointer\n");
    }
}
void *malloc_hook(size_t size, const char *file, unsigned long line)
{
    void *ptr;
    char *str;
    int i,j;
    ptr = _pvPortMalloc(size);
    if(ptr){
        total_alloc_size += size;
        if(total_alloc_size > max_alloc_size)
            max_alloc_size = total_alloc_size;
        total_entry++;
        if(total_entry > max_entry)
            max_entry = total_entry;
            
        for(i=0;i<MAX_LIST;i++){
            if(alloctable[i].pv == NULL){
                alloctable[i].pv = ptr;
                alloctable[i].size = size;
                alloctable[i].file = (char *)file;
                alloctable[i].line = line;
                for(j=0;j<sizeof(white_list);j++){
                    if(!strcmp(file,white_list[j].file) && line == white_list[j].line){
                        str = strrchr(alloctable[i].file,'\\');
                        if(str == NULL)
                            str = alloctable[i].file;
                        printf("malloc size = %ld[%p](%s:%ld)\n",size,ptr,str,line);   
                    }
                }
                return ptr;
            }
        }
        printf("#### alloctable is full\n");
    }else{
        printf("#### Malloc Fail = %ld[%p](%s:%ld)\n",size,ptr,str,line);   
    }
    return ptr;
}
void *calloc_hook(size_t num, size_t size, const char *file, unsigned long line)
{
    void *ptr;
    char *str;
    int i,j;
    ptr = _pvPortCalloc(num, size);
    if(ptr){
        total_alloc_size += size;
        if(total_alloc_size > max_alloc_size)
            max_alloc_size = total_alloc_size;
        total_entry++;
        if(total_entry > max_entry)
            max_entry = total_entry;

        for(i=0;i<MAX_LIST;i++){
            if(alloctable[i].pv == NULL){
                alloctable[i].pv = ptr;
                alloctable[i].size = num * size;
                alloctable[i].file = (char *)file;
                alloctable[i].line = line;
                for(j=0;j<sizeof(white_list);j++){
                    if(!strcmp(file,white_list[j].file) && line == white_list[j].line){
                        str = strrchr(alloctable[i].file,'\\');
                        if(str == NULL)
                            str = alloctable[i].file;
                        printf("calloc num = %ld, size = %ld[%p](%s:%ld)\n",num, size,ptr,str,line);
                    }
                }
                return ptr;
            }
        }
        printf("#### alloctable is full\n");
    }else{
        printf("#### Calloc Fail = %ld, size = %ld[%p](%s:%ld)\n",num, size,ptr,str,line);
    }
    return ptr;
}

void show_alloctable(void)
{
    char *str;
    int i;
    int entry = 0;
    unsigned long total = 0;

//    vTaskSuspendAll();
    printf("### Alloc Table\n");
    for(i=0;i<MAX_LIST;i++){
        if(alloctable[i].pv){
            entry++;
            total += alloctable[i].size;
            str = strrchr(alloctable[i].file,'\\');
            if(str == NULL)
                str = alloctable[i].file;
            printf("%s:%ld [%p](%ld)\n",str,alloctable[i].line,alloctable[i].pv,alloctable[i].size);
        }
    }
    printf("Total %d(%d) entry, size = %ld(%ld)\n",entry,total_entry,total,total_alloc_size);
    printf("Maximum %d entry, size = %ld\n",max_entry,max_alloc_size);
//    xTaskResumeAll();
}

void show_free(char *buffer)
{
	unsigned long total, used, max;
	unsigned int total_entry, used_entry, max_entry;
	
	sprintf(buffer,"\ttotal\tused\max\n"\
		"mem\t%d\t%d\t%d\n"\
		"entry\t%d\t%d\t%d\n",total,used,max,total_entry,used_entry,max_entry);
}
#endif /* MEM_USE_TRACE */


