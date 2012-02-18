/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_header.h
    
DESCRIPTION
	Header manipulation header file for the Generic Object Exchange Profile (GOEP) library

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/

#ifndef	GOEP_HEADER_H_
#define GOEP_HEADER_H_

#include <sink.h>

enum
{
	goep_Hdr_Name			=0x01,
	goep_Hdr_Type			=0x42,
	goep_Hdr_Who			=0x4a,
	goep_Hdr_Length			=0xc3,
	goep_Hdr_Target			=0x46,
	goep_Hdr_Body			=0x48,
	goep_Hdr_EndOfBody		=0x49,
	goep_Hdr_ConnID			=0xcb,
	goep_Hdr_AuthChallenge	=0x4d,
	goep_Hdr_AuthResponse	=0x4e,
	goep_Hdr_AppSpecific	=0x4c,
	
	goep_Hdr_EOL
}; /* Header Types */

/* Setup Header Fields (Packet type and packet length) */
bool goepHdrSetUpHeader(goepState *state, Sink sink, const uint8 pcktType, uint16 *length);

/* Update Length Field */
void goepHdrSetPacketLen(goepState *state, Sink sink, const uint16 length);
		
/* Add uint8 field */
bool goepHdrAddUint8Field(Sink sink, const uint8 val, uint16 *length);

/* add uint16 field */
bool goepHdrAddUint16Field(Sink sink, const uint16 val, uint16 *length);

/* Add uint32 header */
bool goepHdrAddUint32Header(Sink sink, const uint8 type, const uint32 val, uint16 *length);

/* add Header */
bool goepHdrAddStringHeader(Sink sink, const uint8 type, const uint16 hdrLen, const uint8* data, uint16 *length);

/* Add header info for application specific parameters
*/
void goepHdrAddAppSpecHeader(Sink sink, uint16 *length);

/* Add Header without any body (just the header section) */
bool goepHdrAddEmptyHeader(Sink sink, const uint8 type, const uint16 hdrLen, uint16 *length);

/* Update header length for application specific parameters
 	Offset is where the application started adding fields.  As returned by goepHdrAddAppSpecHeader.
	Returns the total length of the packet
*/
uint16 goepHdrUpdateAppSpecLength(goepState *state, uint16 length);


/* Find uint8 Field */
#define goepHdrFindUint8Field(b, p) ((b)[(p)])

/* Find uint16 Field */
#define goepHdrFindUint16Field(b, p) ( ((b)[(p)]<<8) + ((b)[(p)+1]) )

/* Find uint32 Header
    Returns an offset into the buffer [0 offset and length on error]
*/
uint16 goepHdrFindUint32Header(const uint8* buffer, const uint16 start, const uint16 stop, const uint8 type);

/* Find String Header
    Returns an offset into the buffer [0 offset and length on error]
*/
uint16 goepHdrFindStringHeader(const uint8* buffer, const uint16 start, const uint16 stop, const uint8 type, uint16 *length);

/* Does the packet contain a body header
        Either a BODY or END_OF_BODY
*/
bool goepHdrContainBody(const uint8* buffer, const uint16 start, const uint16 stop);


#endif /* GOEP_HEADER_H_ */
