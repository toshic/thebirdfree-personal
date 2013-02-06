/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_packet.h
    
DESCRIPTION
	Packet manipulation header file for the Generic Object Exchange Profile (GOEP) library

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/

#ifndef	GOEP_PACKET_H_
#define GOEP_PACKET_H_

#include <source.h>

enum
{
    goep_Rsp_Continue       =0x90,
    goep_Rsp_Success        =0xA0,
    
    goep_Rsp_Unauthorised   =0x41,
    
    goep_Rsp_BadRequest     =0xC0,
    goep_Rsp_UnauthComp     =0xC1, /* Unauthorised + Final Packet */
    goep_Rsp_Forbidden      =0xC3,
    goep_Rsp_NotFound       =0xC4,
    goep_Rsp_PreConFail     =0xCC,
    goep_Rsp_ServUnavail    =0xD3,
    
    goep_Rsp_EOL
}; /* Packet Response Codes */

enum
{
	goep_Pkt_Connect	=0x80,
	goep_Pkt_Disconnect	=0x81,
    
    goep_Pkt_Put        =0x02,
    goep_Pkt_PutLast    =0x82,
    
    goep_Pkt_Get        =0x03,
    goep_Pkt_GetNext    =0x83,
    
    goep_Pkt_SetPath    =0x85,
    goep_Pkt_Abort      =0xFF,
			
	goep_Pkt_EOL
}; /* Packet Types */


/* Send Connect Packet */
#define  goepSendConnect(s) goepSendFullConnect(s, NULL, 0, NULL, NULL)

/* Send Connect Packet */
goep_lib_status goepSendFullConnect(goepState *state, const uint8 *digest, uint16 size_userid, const uint8 *userid, const uint8 *nonce);

/* Send Disconnect Packet */
goep_lib_status goepSendDisconnect(goepState *state);

/* Send First Put Packet */
goep_lib_status goepSendPutFirst(goepState *state, 
				  		const uint16 nameLen, const uint8* name, 
						const uint16 length, Source packet, 
                        const uint32 totalLen, const bool onlyPacket);

/* Send First Put Packet including a type header*/
goep_lib_status goepSendPutFirstType(goepState *state, 
				  		const uint16 nameLen, const uint8* name, 
				  		const uint16 typeLen, const uint8* type, 
						const uint16 length, Source packet, 
                        const uint32 totalLen, const bool onlyPacket);

/* Send Next Put Packet */
goep_lib_status goepSendPutNext(goepState *state, 
						const uint16 length, Source packet, 
                        const bool lastPacket);

/* Send First Get Packet (the one that starts us getting something) */
goep_lib_status goepSendGetStart(goepState *state, 
				  		const uint16 nameLen, const uint8* name, 
						const uint16 typeLen, const uint8 *type);

/* Send Next Get Packet */
goep_lib_status goepSendGetNext(goepState *state);

/* Send Abort Packet */
goep_lib_status goepSendAbort(goepState *state);

/* Send Set Path packet */
goep_lib_status goepSendSetPath(goepState *state, const uint8 flags, 
								uint16 length, const uint8* folder);

/* Send Simple Responce Packet */
goep_lib_status goepSendResponse(goepState *state, uint16 resp);

/* Send Remote Connection Response 
	nonce MUST be a valid pointer containing 16 bytes of data, or NULL */
goep_lib_status goepSendConnectResponse(goepState *state, uint16 resp, 
										const uint8 *nonce, uint8 options, uint16 size_realm, const uint8 *realm);

/* Send Remote Get Response */
goep_lib_status goepSendGetResponse(goepState *state, uint16 respCode, uint32 totLen, 
									const uint8* name, uint16 nameLen, 
									const uint8* type, uint16 typeLen, 
									Source data,	   uint16 dataLen);

goep_lib_status goepSendGetResponseHdr(goepState *state, uint16 respCode, uint32 totLen, 
									const uint8* name, uint16 nameLen, 
									const uint8* type, uint16 typeLen, 
									Source data,	   uint16 dataLen,
									uint8* appHeader, uint16 hdrLen);

/* Send Packet containing app. specific parameters */
void goepSendAppSpecific(goepState *state, uint16 length);


#if defined(GOEP_LIBRARY_DECODE_PACKETS) && defined(GOEP_LIBRARY_DEBUG) 
    void goepDecodePacket(const uint8* s, uint16 start, uint16 len, uint16 command);
#else
    #define goepDecodePacket(a, b, c, d)
#endif /* defined(GOEP_LIBRARY_DECODE_PACKETS) && defined(GOEP_LIBRARY_DEBUG) */

#endif /* GOEP_PACKET_H_ */

