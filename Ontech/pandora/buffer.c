/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
	Implementation for handling buffer functionality

	
FILE
	buffer.c
*/

#include <print.h>
#include <stdlib.h>

#include "buffer.h"
#include "audioAdaptor_private.h"

/*
	Allocate a buffer of at least minSize words
*/
bool allocBuffer(uint16 minSize)
{
	bool lRet = FALSE;
	
	DEBUG(("allocBuffer : MinSize %d\n",minSize));
	
	if (!the_app->buffer.buffer)
	{
		uint16 size = PBAPS_BUFFER_START_SIZE;
		uint8 *buffer = malloc(size);
		
		while ((!buffer) && (size>minSize))
		{
			size -= 5;
			buffer = malloc(size);
		}
		if (buffer)
		{
			if (size >= minSize)
			{
				DEBUG(("    Buffer allocated. Size %d\n",size));
				the_app->buffer.buffer = buffer;
				the_app->buffer.sizeBuffer = size;
				the_app->buffer.freeSpace = size;
				the_app->buffer.nextPos = 0;
				the_app->buffer.used = 0;
				lRet = TRUE;
			}
			else
			{
				DEBUG(("    Unable to allocate a big enough buffer.  Size %d\n",size));
				free(buffer);
			}
		}
		else
		{
			DEBUG(("    Unable to allocate buffer.  Size %d\n",size));
		}
	}
	else
	{
		DEBUG(("    Buffer already in use\n"));
	}
	
	return lRet;
}

/*
	freeBuffer created using allocBuffer
*/
void freeBuffer(void)
{
	if (the_app->buffer.buffer)
	{
		free(the_app->buffer.buffer);
		the_app->buffer.buffer = NULL;
		the_app->buffer.sizeBuffer = 0;
		the_app->buffer.freeSpace = 0;
		the_app->buffer.nextPos = 0;
		the_app->buffer.used = 0;
	}
}

/*
	reset buffer
*/
void resetBuffer(void)
{
	the_app->buffer.freeSpace = the_app->buffer.sizeBuffer;
	the_app->buffer.nextPos = 0;
	the_app->buffer.used = 0;
}
