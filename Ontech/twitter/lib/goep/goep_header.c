/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_header.c
    
DESCRIPTION
	Header manipulation source file for the Generic Object Exchange Profile (GOEP) library

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/

#include "goep.h"
#include "goep_private.h"
#include "goep_header.h"

#include <sink.h>
#include <string.h>


bool goepHdrSetUpHeader(goepState *state, Sink sink, const uint8 pcktType, uint16 *length)
{
	uint8* s=SinkMap(sink);
	uint16 o=SinkClaim(sink,3); /* 1 byte type and 2 byte length */
	
	if (o==0xffff)
		return FALSE; /* Error */
	
	state->firstOffset=o;
	s+=o;
	/* Packet type */
	s[0]=pcktType;
	/* Packet length */
	s[1]=0;
	s[2]=0;
	
	*length=*length+3;
	
	return TRUE;
}

void goepHdrSetPacketLen(goepState *state, Sink sink, const uint16 length)
{
	uint8* s=SinkMap(sink);
	
	/* The packet is already in the sink, and we know where it starts, 
		so just jump in there and set the length field */
	s+=state->firstOffset+1;
	s[0] = (length>>8)&0xff;
	s[1] = length&0xff;
}

bool goepHdrAddUint8Field(Sink sink, const uint8 val, uint16 *length)
{
	uint8* s=SinkMap(sink);
	uint16 o=SinkClaim(sink,1);
	
	if (o==0xffff)
		return FALSE; /* Error */
	
	s+=o;
	/* Packet type */
	s[0]=val;
	
	*length=*length+1;
	
	return TRUE;
}

/* Add Max Length Field */
bool goepHdrAddUint16Field(Sink sink, const uint16 val, uint16 *length)
{
	uint8* s=SinkMap(sink);
	uint16 o=SinkClaim(sink,2);
	
	if (o==0xffff)
		return FALSE; /* Error */
	
	s+=o;
	/* Packet type */
	s[0] = (val>>8)&0xff;
	s[1] = val&0xff;
	
	*length=*length+2;
	
	return TRUE;
}

/* Add uint32 header */
bool goepHdrAddUint32Header(Sink sink, const uint8 type, const uint32 val, uint16 *length)
{
	uint8* s=SinkMap(sink);
	uint16 o=SinkClaim(sink,5);
	
	if (o==0xffff)
		return FALSE; /* Error */
	
	s+=o;
	/* Packet type */
	s[0] = type;
	s[1] = (val>>24)&0xff;
	s[2] = (val>>16)&0xff;
	s[3] = (val>>8)&0xff;
	s[4] = (val>>0)&0xff;
	
	*length=*length+5;
	
	return TRUE;
}
        
bool goepHdrAddStringHeader(Sink sink, const uint8 type, const uint16 hdrLen, const uint8* data, uint16 *length)
{
	uint8* s=SinkMap(sink);
	uint16 len=hdrLen+3; /* Header ID + 2 byte len + data */
	uint16 o=SinkClaim(sink, len);
	
	if (o==0xffff)
		return FALSE; /* Error */
	
	s+=o;
	/* Packet type */
	s[0] = type;
	s[1] = (len>>8)&0xff;
	s[2] = len&0xff;
	
    if (hdrLen>0)
    	memmove(&s[3],data,hdrLen);
	
	*length=*length+len;
	
	return TRUE;
}

/* Find uint32 Header
    Returns an offset into the buffer [0 offset on error]
*/
uint16 goepHdrFindUint32Header(const uint8* buffer, const uint16 start, const uint16 stop, const uint8 type)
{
    uint16 c=start;
    uint8 msk;
    
    while (c<stop)
    {
        if (buffer[c]==type)
        { /* Found the header */
            return (c+1); /* Step over header byte */
        }
        /* Find length of this header and advance */
        msk=buffer[c] & 0xC0;
        if (msk==0xC0)
            c+=5; /* 4 Byte Quanity + Hdr */
        else
        {
            if (msk==0x80)
                c+=2;  /* 1 Byte Quantity + Hdr */
            else
                c+=(buffer[c+1]<<8)+buffer[c+2];  /* String with Length */
        }
    }
    
    return 0;
}

/* Find String Header
    Returns an offset into the buffer [0 offset and length on error]
*/
uint16 goepHdrFindStringHeader(const uint8* buffer, const uint16 start, const uint16 stop, const uint8 type, uint16 *length)
{
    uint16 c=start;
    uint8 msk;
    
    while (c<stop)
    {
        if (buffer[c]==type)
        { /* Found the header */
            c++; /* Header byte */
            *length=((buffer[c]<<8)+buffer[c+1])-3;  /* Size of the header */
            return (c+2); /* Size of the length */
        }
        /* Find length of this header and advance */
        msk=buffer[c] & 0xC0;
        if (msk==0xC0)
            c+=5; /* 4 Byte Quanity + Hdr */
        else
        {
            if (msk==0x80)
                c+=2;  /* 1 Byte Quantity + Hdr */
            else
                c+=(buffer[c+1]<<8)+buffer[c+2];  /* String with Length */
        }
    }
    
    *length=0;
    return 0;
}

/* Does the packet contain a body header
        Either a BODY or END_OF_BODY
*/
bool goepHdrContainBody(const uint8* buffer, const uint16 start, const uint16 stop)
{
    uint16 c=start;
    uint8 msk;
    
    while (c<stop)
    {
        if ((buffer[c]==goep_Hdr_Body) || (buffer[c]==goep_Hdr_EndOfBody))
        { /* Found the header */
            return TRUE;
        }
        /* Find length of this header and advance */
        msk=buffer[c] & 0xC0;
        if (msk==0xC0)
            c+=5; /* 4 Byte Quanity + Hdr */
        else
        {
            if (msk==0x80)
                c+=2;  /* 1 Byte Quantity + Hdr */
            else
                c+=(buffer[c+1]<<8)+buffer[c+2];  /* String with Length */
        }
    }
    
    return FALSE;
}

/* Add header info for application specific parameters
*/
void goepHdrAddAppSpecHeader(Sink sink, uint8* data, uint16 dataLen, uint16 *length)
{
	uint8* s = SinkMap(sink);
	uint16 len = 3 + dataLen; /* Header ID */
	uint16 o = SinkClaim(sink, len);
	
	if (o==0xffff)
		Panic(); /* Error */
	
	s+=o;
	/* Packet type */
	s[0] = goep_Hdr_AppSpecific;
	/* Packet Length - Will be updated later with the correct value */
	s[1] = (len>>8)&0xff;
	s[2] = len&0xff;

	if(dataLen)
		memcpy(s + 3, data, dataLen);
	
	*length=*length+len;
}

/* Add Header without any body (just the header section) */
bool goepHdrAddEmptyHeader(Sink sink, const uint8 type, const uint16 hdrLen, uint16 *length)
{
	uint8* s = SinkMap(sink);
	uint16 len = 3; /* Header ID */
	uint16 o = SinkClaim(sink, len);
	
	len += hdrLen;
	
	if (o==0xffff)
		return FALSE;
	
	s+=o;
	/* Packet type */
	s[0] = type;
	/* Packet Length - Will be updated later with the correct value */
	s[1] = (len>>8)&0xff;
	s[2] = len&0xff;
	
	*length=*length+len;
	
	return TRUE;
}

/* Update header length for application specific parameters
 	Offset is where the application started adding fields.  As returned by goepHdrAddAppSpecHeader.
	Returns the total length of the packet
*/
#include <stdio.h>
uint16 goepHdrUpdateAppSpecLength(goepState *state, uint16 length)
{
	uint8* s = SinkMap(state->sink);
	uint16 len = goepHdrFindUint16Field(s, 1); /* get the packet length */
	
	s += len-2;
	len += length;
	length += 3; /* Update for the length of the header */
	s[0] = (length>>8)&0xff;
	s[1] = length&0xff;
	
	goepHdrSetPacketLen(state, state->sink, len);
	
	return len;
}
