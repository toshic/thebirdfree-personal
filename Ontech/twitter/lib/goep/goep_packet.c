/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_packet.c
    
DESCRIPTION
	Packet manipulation source file for the Generic Object Exchange Profile (GOEP) library

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/

#include <sink.h>
#include <string.h>
#include <source.h>
#include <stream.h>

#include "goep.h"
#include "goep_private.h"
#include "goep_packet.h"
#include "goep_header.h"

/* Send Connect Packet */
goep_lib_status goepSendFullConnect(goepState *state, const uint8 *digest, uint16 size_userid, const uint8 *userid, const uint8 *nonce)
{
	uint16 len=0;
	
	/* Setup Header fields */
	if (!goepHdrSetUpHeader(state, state->sink, goep_Pkt_Connect, &len))
		return goep_failure;
	
	/* Add version field */
	if (!goepHdrAddUint8Field(state->sink, GOEP_OBEX_VER, &len))
		return goep_failure;
	
	/* Add flags */
	if (!goepHdrAddUint8Field(state->sink, 0x00, &len))
		return goep_failure;
	
	/* Add max length */
	if (!goepHdrAddUint16Field(state->sink, state->pktLen, &len))
		return goep_failure;
	
	/* Add target field if supplied */
	if ((state->conLen>0) && (state->conInfo))
	{
		if (!goepHdrAddStringHeader(state->sink, goep_Hdr_Target, state->conLen, state->conInfo, &len))
			return goep_failure;
	}
	
	if (digest)
	{
		uint8 *base = SinkMap(state->sink);
		uint8 *s;
		uint8 *h; /* start of the header */
		uint16 o;
		uint16 cha_len = state->authChallenge[0];
		uint16 hlen = 3;
		
		/* Add header header */
		o = SinkClaim(state->sink, 3+cha_len);
		h = base + o;
		h[0] = goep_Hdr_AuthChallenge;
		h[1] = 0;
		h[2] = 3+cha_len;
		memmove(&h[3], &state->authChallenge[1], cha_len);
		free(state->authChallenge);
		state->authChallenge = NULL;
		len += 3+cha_len;
		
		/* Add header header */
		o = SinkClaim(state->sink, 3); /* Nonce length = 16 + tag byte + length byte */
		h = base + o;
		h[0] = goep_Hdr_AuthResponse;
		
		/* add digest */
		o = SinkClaim(state->sink, GOEP_SIZE_DIGEST+2); /* Nonce length = 16 + tag byte + length byte */
		s = base + o;
		s[0] = GOEP_AUTH_RES_DIGEST;
		s[1] = GOEP_SIZE_DIGEST;
		memmove(&s[2], digest, GOEP_SIZE_DIGEST);
		hlen += GOEP_SIZE_DIGEST+2;
		
		if (size_userid > 0)
		{ /* add user ID */
			o = SinkClaim(state->sink, size_userid+2);
			s = base + o;
			s[0] = GOEP_AUTH_RES_USERID;
			s[1] = size_userid;
			memmove(&s[2], userid, size_userid);
			hlen += size_userid+2;
		}
		
		if (nonce)
		{ /* add nonce */
			o = SinkClaim(state->sink, GOEP_SIZE_DIGEST+2); /* Nonce length = 16 + tag byte + length byte */
			s = base + o;
			s[0] = GOEP_AUTH_RES_NONCE;
			s[1] = GOEP_SIZE_DIGEST;
			memmove(&s[2], nonce, GOEP_SIZE_DIGEST);
			hlen += GOEP_SIZE_DIGEST+2;
		}
		
		/* update header fields */
		h[1] = (hlen >> 8) & 0xff;
		h[2] = hlen & 0xff;
		len += hlen;
	}
	
	/* update length field */
	goepHdrSetPacketLen(state, state->sink, len);
	
	/* Send packet */
	SinkFlush(state->sink, len);
	
	return goep_success;
}

/* Send Disconnect Packet */
goep_lib_status goepSendDisconnect(goepState *state)
{
	uint16 len=0;
    
    /* Setup Header fields */
	if (!goepHdrSetUpHeader(state, state->sink, goep_Pkt_Disconnect, &len))
		return goep_failure;
    
    /* Add connection ID header */
	if (state->useConID)
	{
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_ConnID, state->conID, &len))
            return goep_failure;
    }
    
	/* update length field */
	goepHdrSetPacketLen(state, state->sink, len);
	
	/* Send packet */
	SinkFlush(state->sink, len);
	
	return goep_success;
}

/* Send First Put Packet */
goep_lib_status goepSendPutFirst(goepState *state, 
				  		const uint16 nameLen, const uint8* name, 
						const uint16 length, Source packet, 
                        const uint32 totalLen, const bool onlyPacket)
{
	uint16 len=0;
    uint8 type=onlyPacket?goep_Pkt_PutLast:goep_Pkt_Put;
    uint8 body=onlyPacket?goep_Hdr_EndOfBody:goep_Hdr_Body;
    
    /* Setup Header fields */
	if (!goepHdrSetUpHeader(state, state->sink, type, &len))
		return goep_failure;
    
    /* Add connection ID header */
    if (state->useConID)
    {
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_ConnID, state->conID, &len))
            return goep_failure;
    }
    
    /* Send Name header */
    if (!goepHdrAddStringHeader(state->sink, goep_Hdr_Name, nameLen, name, &len))
        return goep_failure;
    
    /* Send Total Length */
    if (totalLen>0)
    {
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_Length, totalLen, &len))
            return goep_failure;
    }
    
    /* Send Body Header */
    if (length>0)
    { /* Only send a body header if there is one to send. */
        if (!goepHdrAddEmptyHeader(state->sink, body, length, &len))
            return goep_failure;
		StreamMove(state->sink, packet, length);
    }
    
	/* update length field */
	goepHdrSetPacketLen(state, state->sink, len);
	
	/* Send packet */
	SinkFlush(state->sink, len);
    
    state->state=onlyPacket?	goep_pushing_last:goep_pushing;
	
	return goep_success;
}

/* Send First Put Packet with a Type header */
goep_lib_status goepSendPutFirstType(goepState *state, 
				  		const uint16 nameLen, const uint8* name, 
				  		const uint16 typeLen, const uint8* type, 
						const uint16 length, Source packet, 
                        const uint32 totalLen, const bool onlyPacket)
{
	uint16 len=0;
    uint8 pcktType=onlyPacket?goep_Pkt_PutLast:goep_Pkt_Put;
    uint8 body=onlyPacket?goep_Hdr_EndOfBody:goep_Hdr_Body;

    /* Setup Header fields */
	if (!goepHdrSetUpHeader(state, state->sink, pcktType, &len))
		return goep_failure;
    
    /* Add connection ID header */
    if (state->useConID)
    {
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_ConnID, state->conID, &len))
            return goep_failure;
    }
    
    /* Send Name header */
    if (!goepHdrAddStringHeader(state->sink, goep_Hdr_Name, nameLen, name, &len))
        return goep_failure;
	
    /* Send Type header */
	if ((typeLen>0) && (type))
	{
	    if (!goepHdrAddStringHeader(state->sink, goep_Hdr_Type, typeLen, type, &len))
    	    return goep_failure;
	}
    
    /* Send Total Length */
    if (totalLen>0)
    {
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_Length, totalLen, &len))
            return goep_failure;
    }
    
    /* Send Body Header */
    if (length>0)
    { /* Only send a body header if there is one to send. */
        if (!goepHdrAddEmptyHeader(state->sink, body, length, &len))
            return goep_failure;
		StreamMove(state->sink, packet, length);
    }
    
	/* update length field */
	goepHdrSetPacketLen(state, state->sink, len);
	
	/* Send packet */
	SinkFlush(state->sink, len);
    
    state->state=onlyPacket?	goep_pushing_last:goep_pushing;
	
	return goep_success;
}

/* Send Next Put Packet */
goep_lib_status goepSendPutNext(goepState *state, 
						const uint16 length, Source packet, 
                        const bool lastPacket)
{
	uint16 len=0;
    uint8 type= lastPacket?goep_Pkt_PutLast:goep_Pkt_Put;
    uint8 body= lastPacket?goep_Hdr_EndOfBody:goep_Hdr_Body;
    
    /* Setup Header fields */
	if (!goepHdrSetUpHeader(state, state->sink, type, &len))
		return goep_failure;
    
    /* Add connection ID header */
    if (state->useConID)
    {
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_ConnID, state->conID, &len))
            return goep_failure;
    }
    
    /* Send Body Header */
	if (!goepHdrAddEmptyHeader(state->sink, body, length, &len))
    	return goep_failure;
	StreamMove(state->sink, packet, length);
    
	/* update length field */
	goepHdrSetPacketLen(state, state->sink, len);
	
	/* Send packet */
	SinkFlush(state->sink, len);
    
    state->state= lastPacket?	goep_pushing_last:goep_pushing;
	
	return goep_success;
}

/* Send First Get Packet (the one that starts us getting something) */
goep_lib_status goepSendGetStart(goepState *state, 
				  		const uint16 nameLen, const uint8* name, 
						const uint16 typeLen, const uint8 *type)
{
	uint16 len=0;
    
    /* Setup Header fields */
	if (!goepHdrSetUpHeader(state, state->sink, goep_Pkt_GetNext, &len))
		return goep_failure;
    
    /* Add connection ID header */
    if (state->useConID)
    {
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_ConnID, state->conID, &len))
            return goep_failure;
    }
    
    /* Send Type Header */
    if ((typeLen>0) && (type))
    {
        if (!goepHdrAddStringHeader(state->sink, goep_Hdr_Type, typeLen, type, &len))
            return goep_failure;
    }
    
    /* Send Name Header */
    if ((nameLen>0) && (name))
    {
        if (!goepHdrAddStringHeader(state->sink, goep_Hdr_Name, nameLen, name, &len))
            return goep_failure;
    }
    else if (nameLen>0)
    {
        if (!goepHdrAddEmptyHeader(state->sink, goep_Hdr_Name, 0, &len))
            return goep_failure;
    }
	
	if (state->useHeaders)
	{
		/* Add App Headers 'header' */
		goepHdrAddAppSpecHeader(state->sink, NULL, 0, &len);
		
		/* update length field */
		goepHdrSetPacketLen(state, state->sink, len);

		/* send GET APP HEADERS request */
		goepMsgSendGetAppHeadersInd(state, state->sink, state->pktLen - len);
	}
	else
	{
		/* update length field */
		goepHdrSetPacketLen(state, state->sink, len);

		/* Send packet */
		SinkFlush(state->sink, len);
	}
    
    state->state= goep_pulling_first;
	
	return goep_success;
}

/* Send Next Get Packet */
goep_lib_status goepSendGetNext(goepState *state)
{
	uint16 len=0;
    
    /* Setup Header fields */
	if (!goepHdrSetUpHeader(state, state->sink, goep_Pkt_GetNext, &len))
		return goep_failure;
    
    /* Add connection ID header */
    if (state->useConID)
    {
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_ConnID, state->conID, &len))
            return goep_failure;
    }
    
	/* update length field */
	goepHdrSetPacketLen(state, state->sink, len);
	
	/* Send packet */
	SinkFlush(state->sink, len);
    
    state->state= goep_pulling;
	
	return goep_success;
}

/* Send Abort Packet */
goep_lib_status goepSendAbort(goepState *state)
{
	uint16 len=0;
    
    /* Setup Header fields */
	if (!goepHdrSetUpHeader(state, state->sink, goep_Pkt_Abort, &len))
		return goep_failure;
    
    /* Add connection ID header */
    if (state->useConID)
    {
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_ConnID, state->conID, &len))
            return goep_failure;
    }   
    
	/* update length field */
	goepHdrSetPacketLen(state, state->sink, len);
	
	/* Send packet */
	SinkFlush(state->sink, len);
	
	state->abort_state = state->state;
    state->state = goep_aborting;
	
	return goep_success;
}

/* Send Set Path packet */
goep_lib_status goepSendSetPath(goepState *state, const uint8 flags, uint16 length, const uint8* folder)
{
	uint16 len=0;
    
    /* Setup Header fields */
	if (!goepHdrSetUpHeader(state, state->sink, goep_Pkt_SetPath, &len))
		return goep_failure;
    
    /* Add flags field */
	if (!goepHdrAddUint8Field(state->sink, flags, &len))
		return goep_failure;
    
    /* Add reserve NULL constants field */
	if (!goepHdrAddUint8Field(state->sink, 0, &len))
		return goep_failure;
    
    /* Add connection ID header */
    if (state->useConID)
    {
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_ConnID, state->conID, &len))
            return goep_failure;
    }
    
    /* Add name header */
    if (!(flags&GOEP_PATH_PARENT))
    { /* Only add the name if we aren't browsing to the parent */
        if (!goepHdrAddStringHeader(state->sink, goep_Hdr_Name, length, folder, &len))
            return goep_failure;
    }
    
	/* update length field */
	goepHdrSetPacketLen(state, state->sink, len);
	
	/* Send packet */
	SinkFlush(state->sink, len);
    
    state->state= goep_setpath;
	
	return goep_success;
}

/* Send Simple Responce Packet */
goep_lib_status goepSendResponse(goepState *state, uint16 resp)
{
	uint16 len=0;

    /* Setup Header fields */
	if (!goepHdrSetUpHeader(state, state->sink, resp, &len))
		return goep_failure;
    
    /* Add connection ID header */
    if (state->useConID)
    {
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_ConnID, state->conID, &len))
            return goep_failure;
    }
    
	/* update length field */
	goepHdrSetPacketLen(state, state->sink, len);
	
	/* Send packet */
	SinkFlush(state->sink, len);
	
	return goep_success;
}

/* Send Remote Connection Response */
goep_lib_status goepSendConnectResponse(goepState *state, uint16 resp, 
										const uint8 *nonce, uint8 options, uint16 size_realm, const uint8 *realm)
{
	uint16 len=0;

    /* Setup Header fields */
	if (!goepHdrSetUpHeader(state, state->sink, resp, &len))
		return goep_failure;
    
	/* Add version field */
	if (!goepHdrAddUint8Field(state->sink, GOEP_OBEX_VER, &len))
		return goep_failure;
	
	/* Add flags */
	if (!goepHdrAddUint8Field(state->sink, 0x00, &len))
		return goep_failure;
	
	/* Add max length */
	if (!goepHdrAddUint16Field(state->sink, state->pktLen, &len))
		return goep_failure;
	
    /* Add connection ID header */
    if (state->useConID)
    {
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_ConnID, state->conID, &len))
            return goep_failure;
    }
	
	if (state->conInfo)
	{
        if (!goepHdrAddStringHeader(state->sink, goep_Hdr_Who, state->conLen, state->conInfo, &len))
            return goep_failure;
	}
	
	if (nonce)
	{
		uint8 *base = SinkMap(state->sink);
		uint8 *s;
		uint8 *h; /* start of the header */
		uint16 o;
		uint16 hlen = 3;
		
		/* Add header header */
		o = SinkClaim(state->sink, 3); /* Nonce length = 16 + tag byte + length byte */
		h = base + o;
		h[0] = goep_Hdr_AuthChallenge;
		
		/* add nonce */
		o = SinkClaim(state->sink, GOEP_SIZE_DIGEST+2); /* Nonce length = 16 + tag byte + length byte */
		s = base + o;
		s[0] = GOEP_AUTH_REQ_NONCE;
		s[1] = GOEP_SIZE_DIGEST;
		memmove(&s[2], nonce, GOEP_SIZE_DIGEST);
		hlen += GOEP_SIZE_DIGEST+2;
		
		if (options > 0)
		{ /* add options */
			o = SinkClaim(state->sink, 3);
			s = base + o;
			s[0] = GOEP_AUTH_REQ_OPTIONS;
			s[1] = 1;
			s[2] = options;
			hlen += 3;
		}
		
		if (size_realm > 0)
		{ /* add realm */
			o = SinkClaim(state->sink, size_realm+2);
			s = base + o;
			s[0] = GOEP_AUTH_REQ_OPTIONS;
			s[1] = size_realm;
			memmove(&s[2], realm, size_realm);
			hlen += size_realm+2;
		}
		/* update header fields */
		h[1] = (hlen >> 8) & 0xff;
		h[2] = hlen & 0xff;
		
		len += hlen;
	}
	
	if (state->authChallenge)
	{
		uint8 *base = SinkMap(state->sink);
		uint8 *h; /* start of the header */
		uint16 o;
		uint16 cha_len = state->authChallenge[0];
		
		o = SinkClaim(state->sink, 3+cha_len);
		h = base + o;
		h[0] = goep_Hdr_AuthResponse;
		h[1] = 0;
		h[2] = 3+cha_len;
		memmove(&h[3], &state->authChallenge[1], cha_len);
		free(state->authChallenge);
		state->authChallenge = NULL;
		
		len += 3+cha_len;
	}
	
	/* update length field */
	goepHdrSetPacketLen(state, state->sink, len);
	
	/* Send packet */
	SinkFlush(state->sink, len);
	
	return goep_success;
}

/* Send Remote Get Response */


goep_lib_status goepSendGetResponse(goepState *state, uint16 respCode, uint32 totLen, 
									const uint8* name, uint16 nameLen, 
									const uint8* type, uint16 typeLen, 
									Source data, 	   uint16 dataLen)
{
	return goepSendGetResponseHdr(state,respCode,totLen,name,nameLen,type,typeLen,data,dataLen,NULL,0);
}

goep_lib_status goepSendGetResponseHdr(goepState *state, uint16 respCode, uint32 totLen, 
									const uint8* name, uint16 nameLen, 
									const uint8* type, uint16 typeLen, 
									Source data,	   uint16 dataLen,
									uint8* appHeader, uint16 hdrLen)
{
	uint16 len=0;

    /* Setup Header fields */
	if (!goepHdrSetUpHeader(state, state->sink, respCode, &len))
		return goep_failure;
    
	/* Add total length */
	if (totLen!=0)
	{
		if (!goepHdrAddUint32Header(state->sink, goep_Hdr_Length, totLen, &len))
			return goep_failure;
	}
	
    /* Add connection ID header */
    if (state->useConID)
    {
        if (!goepHdrAddUint32Header(state->sink, goep_Hdr_ConnID, state->conID, &len))
            return goep_failure;
    }
	
	if (nameLen>0)
	{
        if (!goepHdrAddStringHeader(state->sink, goep_Hdr_Name, nameLen, name, &len))
            return goep_failure;
	}
	
	if (typeLen>0)
	{
        if (!goepHdrAddStringHeader(state->sink, goep_Hdr_Type, typeLen, type, &len))
            return goep_failure;
	}
	
	if (dataLen>0)
	{
		if (respCode==goep_Rsp_Continue)
		{
	        if (!goepHdrAddEmptyHeader(state->sink, goep_Hdr_Body, dataLen, &len))
    	        return goep_failure;
		}
		else
		{
	        if (!goepHdrAddEmptyHeader(state->sink, goep_Hdr_EndOfBody, dataLen, &len))
    	        return goep_failure;
		}
		StreamMove(state->sink, data, dataLen);
	}

	if(hdrLen)
	{
		/* Add App Headers 'header' */
		goepHdrAddAppSpecHeader(state->sink, appHeader, hdrLen, &len);

		/* update length field */
		goepHdrSetPacketLen(state, state->sink, len);
	
		/* Send packet */
		SinkFlush(state->sink, len);
	}
	else if (state->useHeaders)
	{
		/* Add App Headers 'header' */
		goepHdrAddAppSpecHeader(state->sink, NULL, 0, &len);

		/* update length field */
		goepHdrSetPacketLen(state, state->sink, len);

		/* send GET APP HEADERS request */
		goepMsgSendGetAppHeadersInd(state, state->sink, state->pktLen - len);
	}
	else
	{
		/* update length field */
		goepHdrSetPacketLen(state, state->sink, len);

		/* Send packet */
		SinkFlush(state->sink, len);
	}
	
	return goep_success;
}

/* Send Packet containing app. specific parameters */
void goepSendAppSpecific(goepState *state, uint16 length)
{
	uint16 len = goepHdrUpdateAppSpecLength(state, length);
	
	/* Send packet */
	SinkFlush(state->sink, len);
	
	state->useHeaders = FALSE;
}

#if defined(GOEP_LIBRARY_DECODE_PACKETS) && defined(GOEP_LIBRARY_DEBUG) 
void goepDecodePacket(const uint8* s, uint16 start, uint16 len, uint16 command)
{
    uint16 c=3,c2=0;
    uint16 l;
    uint8 v;
    
    GOEP_DEBUG(("\n Packet Decode \n\n"));
    
    GOEP_DEBUG(("Reported  Length: %d, 0x%X\n",len,len));
    
    /* Get Response */
    GOEP_DEBUG(("Response Code : 0x%X\n",s[0]));
    /* Get Packet Length */
    l=goepHdrFindUint16Field(s, 1);
    GOEP_DEBUG(("Packet Length : %d 0x%X\n",l,l));
    
    if (command==goep_Pkt_Connect)
    {
        v=goepHdrFindUint8Field(s, 3);
        GOEP_DEBUG(("OBEX Version : %d.%d\n",(v>>4)&0xf, v&0xf));
        v=goepHdrFindUint8Field(s, 4);
        GOEP_DEBUG(("Flags : %d\n",v));
        l=goepHdrFindUint16Field(s, 5);
        GOEP_DEBUG(("Max Length : %d, 0x%x\n",l,l));
        c=7;
    }
    
    while (c<len)
    {
        switch (s[c])
        {
        case goep_Hdr_Name:
            {
                GOEP_DEBUG(("Name : "));
                l=goepHdrFindUint16Field(s, c+1);
                if (l>3)
                {
	                for (c2=0;c2<(l-3);c2++)
	                {
	                    if (s[c+3+c2]>' ')
	                    {
	                        GOEP_DEBUG(("%c",s[c+3+c2]));
	                    }
	                    else
	                    {
	                        GOEP_DEBUG(("?"));
	                    }
	                }
                }
                else
                	GOEP_DEBUG(("Empty"));
                c+=l;
                GOEP_DEBUG(("\n"));
                break;
            }
        case goep_Hdr_Type:
            {
                GOEP_DEBUG(("Type : "));
                l=goepHdrFindUint16Field(s, c+1);
                for (c2=0;c2<(l-3);c2++)
                {
                    if (s[c+3+c2]>' ')
                    {
                        GOEP_DEBUG(("%c",s[c+3+c2]));
                    }
                    else
                    {
                        GOEP_DEBUG(("?"));
                    }
                }
                c+=l;
                GOEP_DEBUG(("\n"));
                break;
            }
        case goep_Hdr_Length:
            {
                GOEP_DEBUG(("Length : "));
                for (c2=0;c2<4;c2++)
                    GOEP_DEBUG(("0x%0X, ",s[c+1+c2]));
                c+=5;
                GOEP_DEBUG(("\n"));
                break;
            }
        case goep_Hdr_Who:
        case goep_Hdr_Target:
            {
                GOEP_DEBUG(("Target/Who : "));
                l=goepHdrFindUint16Field(s, c+1);
                for (c2=0;c2<(l-3);c2++)
                {
                    GOEP_DEBUG(("0x%0X, ",s[c+3+c2]));
                }
                c+=l;
                GOEP_DEBUG(("\n"));
                break;
            }
        case goep_Hdr_Body:
        case goep_Hdr_EndOfBody:
            {
                GOEP_DEBUG(("Body : %X\n     ",s[c]));
                l=goepHdrFindUint16Field(s, c+1);
                for (c2=0;c2<(l-3);c2++)
                {
                    if (s[c+3+c2]>=' ')
                    {
                        GOEP_DEBUG(("%c",s[c+3+c2]));
                    }
                    else
                    {
                        GOEP_DEBUG(("?"));
                    }
                }
                GOEP_DEBUG(("\n"));
                c+=l;
                break;
            }
        case goep_Hdr_ConnID:
            {
                GOEP_DEBUG(("Conn ID : "));
                for (c2=0;c2<4;c2++)
                    GOEP_DEBUG(("0x%0X, ",s[c+1+c2]));
                GOEP_DEBUG(("\n"));
                c+=5;
                break;
            }
        case goep_Hdr_AuthChallenge:
            {
                GOEP_DEBUG(("Auth Challenge : "));
                l=goepHdrFindUint16Field(s, c+1);
                for (c2=0;c2<(l-3);c2++)
                {
                    GOEP_DEBUG(("0x%0X, ",s[c+1+c2]));
                }
                c+=l;
                GOEP_DEBUG(("\n"));
                break;
            }
        case goep_Hdr_AuthResponse:
            {
                GOEP_DEBUG(("Auth Response : "));
                l=goepHdrFindUint16Field(s, c+1);
                for (c2=0;c2<(l-3);c2++)
                {
                    GOEP_DEBUG(("0x%0X, ",s[c+1+c2]));
                }
                c+=l;
                GOEP_DEBUG(("\n"));
                break;
            }
		case goep_Hdr_AppSpecific:
			{
				uint16 c3, len;
                GOEP_DEBUG(("App Specific : \n"));
                l=goepHdrFindUint16Field(s, c+1);
				c2 = 0;
				while (c2 < (l-3))
				{
					len = s[c+c2+3+1];
					GOEP_DEBUG(("     ID : %d\n",s[c+c2+3]));
					GOEP_DEBUG(("    Len : %d\n",len));
					GOEP_DEBUG(("    Val : "));
					for (c3 = 0;c3 < len; c3++)
					{
	                    GOEP_DEBUG(("0x%0X, ",s[c+5+c2+c3]));
					}
					GOEP_DEBUG(("\n\n"));
					c2 += (len + 2);
				}
                c+=l;
				break;
			}
            default:
            {
                GOEP_DEBUG(("Unknown : %X\n",s[c]));
                c++;
                break;
            }
        }
    }
    GOEP_DEBUG(("\n\n"));
}

#endif /* defined(GOEP_LIBRARY_DECODE_PACKETS) && defined(GOEP_LIBRARY_DEBUG) */
