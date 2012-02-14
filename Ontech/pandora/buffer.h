/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
	Interface definition for handling buffer functionality

	
FILE
	buffer.h
*/


#ifndef BUFFER_H
#define BUFFER_H

/*
	Allocate a buffer of at least minSize words.
	Returns TRUE on success else FALSE.
*/
bool allocBuffer(uint16 minSize);

/*
	freeBuffer created using allocBuffer
*/
void freeBuffer(void);

/*
	reset buffer
*/
void resetBuffer(void);

#endif /* BUFFER_H */
