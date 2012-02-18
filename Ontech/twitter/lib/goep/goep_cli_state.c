/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_cli_state.c
    
DESCRIPTION
	Client state machine source for the Generic Object Exchange Profile (GOEP) library

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/

#include <stream.h>
#include <source.h>
#include <connection.h>
#include <memory.h>

#include "goep.h"
#include "goep_private.h"
#include "goep_cli_state.h"
#include "goep_svr_state.h"
#include "goep_packet.h"
#include "goep_header.h"

static void handleConnect(goepState *sess, Source src);
static void handleDisconnect(goepState *sess, Source src);
static void handlePushLast(goepState *sess, Source src);
static void handlePushNext(goepState *sess, Source src);
static void handleDelete(goepState *sess, Source src);
static void handlePulling(goepState *sess, Source src);
static void handleAbort(goepState *sess, Source src);
static void handleSetPath(goepState *sess, Source src);

static void cleanupConInfo(goepState *sess);

void handleClientStates(goepState *state, Source src)
{
    const uint8* s=SourceMap(src);
    uint16 len=SourceBoundary(src);

    /*
     * Check if source exists, or check if heeader data is present. If both of
     * them present find out the length of the packet.
     */
    if ((!s) || (len<3) || (goepHdrFindUint16Field(s, 1)>len))
    { /* Either empty packet, or packet incomplete */
		GOEP_DEBUG(("          Ignoring empty packet\n"));
        return ;
    }

	/* We have a complete packet.  Remember the amount of source used so we can clean up later. */
	state->srcUsed = len;
	
	
    if (!handleServerCommand(state, src))
    {
        switch (state->state)
        {
    	case goep_connecting:
		case goep_connect_auth:
            handleConnect(state, src);
            break;
    	case goep_disconnecting:
    	case goep_connect_abort:
    	case goep_connect_cancel:
            handleDisconnect(state, src);
            break;
    	case goep_pushing:
            handlePushNext(state, src);
            break;
        case goep_pushing_last:
            handlePushLast(state, src);
            break;
		case goep_deleting:
			handleDelete(state, src);
			break;
    	case goep_pulling_first:  /* Deliberate Fallthrough */
    	case goep_pulling:
            handlePulling(state, src);
            break;
    	case goep_aborting:
            handleAbort(state, src);
            break;
    	case goep_setpath:
            handleSetPath(state, src);
            break;
    	case goep_connected:
        
    	case goep_unknown:       /* Should never get here in this state */
    	case goep_initialising:  /* Should never get here in this state */
        case goep_initialised:    /* Should never get here in this state */
        default:
            {
                uint16 c;
                const uint8* s=SourceMap(src);
                uint16 len=SourceBoundary(src);
                
                GOEP_DEBUG(("Unexpected Packet\n"));
                GOEP_DEBUG(("Length %d, 0x%X\n",len,len));
                for (c=0;c<len;c++)
                {
                    if ((s[c]>=' ') && (s[c]<126))
                    {
                        GOEP_DEBUG(("%c",s[c]));
                    }
                    else
                    {
                        GOEP_DEBUG(("."));
                    }
                }
                GOEP_DEBUG(("\n"));
                for (c=0;c<len;c++)
                {
                    GOEP_DEBUG(("%X, ",s[c]));
                }
                GOEP_DEBUG(("\n"));
				GoepPacketComplete(state);
            }
            break;
        }
    }
}

static void handleConnect(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
	uint8 response = s[0];
    uint16 len=SourceBoundary(src);
    
    GOEP_DEBUG(("handleConnect\n"));
    
    goepDecodePacket(s, 0, len, goep_Pkt_Connect);    
         
    switch (response) /* Check Response Code */
    {
    case goep_Rsp_Success:      /* Connection was successful */
        {
            uint16 maxLen;
            uint16 conIdPos;
			uint16 whoPos;
			uint16 whoLen=0;
     
			/* Check the WHO header if required */
			if ((sess->conLen>0) && (sess->conInfo))
			{ /* Target was specified with the connect request */
				whoPos = goepHdrFindStringHeader(s, 7, len, goep_Hdr_Who, &whoLen);
				if ((whoPos==0) || (memcmp(sess->conInfo, &s[whoPos], whoLen)!=0))
				{ /* Check to ensure this is present and the same as the connection Target */
					/* Send Error to App. */
					goepMsgSendConnectConfirm(sess, goep_server_invalid_serv, 0);
					/* Start disconnect */
					sess->state = goep_connect_abort;
					GoepDisconnect(sess);
						
					/* Clean up connection data */
					cleanupConInfo(sess);
					
					break; /* Abort switch statement */
				}
			}
            /* Get connection ID */
            conIdPos = goepHdrFindUint32Header(s, 7, len, goep_Hdr_ConnID);
            /* Get Max Packet Length */
            maxLen = goepHdrFindUint16Field(s, 5);
            if (maxLen<sess->pktLen)
                sess->pktLen=maxLen;
            
            cleanupConInfo(sess);
			if (conIdPos>0)
			{
            	sess->conID = (uint32)(s[conIdPos+0])<<24;
	            sess->conID|= (uint32)(s[conIdPos+1])<<16;
    	        sess->conID|= (uint32)(s[conIdPos+2])<<8;
        	    sess->conID|= (uint32)(s[conIdPos+3]);
        	    sess->useConID=TRUE;
			}
            
            sess->state=goep_connected;
            
			goepMsgSendConnectConfirm(sess, goep_success, sess->pktLen);
            break;
        }
    case goep_Rsp_Unauthorised: /* Connection failed because we need to authenticate */
    case goep_Rsp_UnauthComp:
		{
			uint16 authPos;
			uint16 authLen=0;
			/* Find auth header */
			authPos = goepHdrFindStringHeader(s, 7, len, goep_Hdr_AuthChallenge, &authLen);
			
			GOEP_DEBUG(("    Authentication Needed\n"));
			
			if (authLen == 0)
			{
			    GOEP_DEBUG(("    Header Missing\n"));
				goepMsgSendConnectConfirm(sess, goep_connect_unauthorised, 0);
				/* Start disconnect */
				sess->state = goep_connect_abort;
				GoepDisconnect(sess);
				/* Clean up connection data */
				cleanupConInfo(sess);
			}
			else
			{
				uint8 options = 0;
				uint16 size_realm = 0;
				const uint8 *realm = NULL, *nonce = NULL;
				GOEP_AUTH_REQUEST_IND_T *message = NULL;
				
				uint16 curr = authPos;
				uint16 end = authPos + authLen;
				
			    GOEP_DEBUG(("    Header Present\n"));
				
				if (sess->authChallenge)
					free(sess->authChallenge);
				sess->authChallenge = (uint8 *)PanicUnlessMalloc(sizeof(uint8)*(authLen+1));
				/* If the string is longer than 255 bytes, the allocation above will probably have caused a Panic */
				sess->authChallenge[0] = (uint8)authLen;
				memmove(&sess->authChallenge[1], &s[authPos], authLen);
				
				/* Find values */
				while (curr < end)
				{
					switch (s[curr])
					{
					case GOEP_AUTH_REQ_NONCE:
						nonce = &s[curr+2];
						break;
					case GOEP_AUTH_REQ_OPTIONS:
						options = s[curr+2];
						break;
					case GOEP_AUTH_REQ_REALM:
						realm = &s[curr+2];
						size_realm = s[curr+1];
						break;
					}
					curr += s[curr+1];
					curr += 2;
				}
				
				/* Create message */
				if (size_realm > GOEP_SIZE_MD5_STRING)
					size_realm = GOEP_SIZE_MD5_STRING;
				
				message = (GOEP_AUTH_REQUEST_IND_T *)PanicUnlessMalloc(sizeof(GOEP_AUTH_REQUEST_IND_T)+size_realm);
				/* Copy values into message */
				message->goep = sess;
				memmove(&message->nonce[0], nonce, GOEP_SIZE_DIGEST);
				message->options = options;
				message->size_realm = size_realm;
				if (size_realm > 0)
				{
					memmove(&message->realm[0], realm, size_realm);
				}
				else
					message->realm[0] = 0;
				
	    		MessageSend(sess->theApp, GOEP_AUTH_REQUEST_IND, message);
			}
			
			
			sess->state = goep_connect_auth;
    	    break;
		}
    case goep_Rsp_ServUnavail:  /* Service is unavailable */
			goepMsgSendConnectConfirm(sess, goep_server_unsupported, 0);
            sess->state = goep_connect_refused;
			cleanupConInfo(sess);
			break;
        default: /* Error */
			goepMsgSendConnectConfirm(sess, goep_connection_refused, 0);
            sess->state = goep_connect_refused;
			cleanupConInfo(sess);
            break;
    }
    
    GoepPacketComplete(sess);
	
	if ((response != goep_Rsp_Success) && (response != goep_Rsp_Unauthorised))
	{
        /* This check is provided for remote server requesting authentication */
        if((response != goep_Rsp_UnauthComp) && (sess->state != goep_connect_auth))
        {
            /* Connection failed for some reason, close RFCOMM connection */
            ConnectionRfcommDisconnectRequest(&sess->task, sess->sink);
	    }
    }
}

static void handleDisconnect(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
    
    switch (s[0]) /* Check Response Code */
    {
    case goep_Rsp_Success:      /* Disconnection was successful */
        {            
			cleanupConInfo(sess);
            
            GOEP_DEBUG( ("ConnectionRfcommDisconnectRequest sent\n") );
            ConnectionRfcommDisconnectRequest(&sess->task, sess->sink);
            
            break;
        }
        default: /* Error - This should never happen */
			goepMsgSendDisconnectConfirm(sess, goep_failure);
            sess->state=goep_connected;
            break;
        
    }
    
    GoepPacketComplete(sess);
}

static void handlePushLast(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
    
    switch (s[0]) /* Check Response Code */
    {
    case goep_Rsp_Success:      /* Last packet sent ok */
        {            
            sess->state=goep_connected;
            
            {
				goepMsgSendLocalPutCompleteInd(sess, goep_success);
            }
            
            break;
        }
        default: /* Error */
			goepMsgSendLocalPutCompleteInd(sess, goep_failure);
            sess->state=goep_connected;
            break;
        
    }
    
    GoepPacketComplete(sess);
}

static void handlePushNext(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
    
    switch (s[0]) /* Check Response Code */
    {
    case goep_Rsp_Continue:      /* Next packet please */
        {            
   			MAKE_GOEP_MESSAGE(GOEP_LOCAL_PUT_DATA_REQUEST_IND);
            message->goep = sess;
    		MessageSend(sess->theApp, GOEP_LOCAL_PUT_DATA_REQUEST_IND, message);
           
            break;
        }
        default: /* Error */
			goepMsgSendLocalPutCompleteInd(sess, goep_failure);
            sess->state=goep_connected;
            break;
        
    }
    
    GoepPacketComplete(sess);
}

static void handleDelete(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
	goep_lib_status result;
    
    switch (s[0]) /* Check Response Code */
    {
    case goep_Rsp_Success:
		{
			result = goep_success;
            break;
        }
        default: /* Error */
			result = goep_failure;
            break;
        
    }
	goepMsgSendDeleteConfirm(sess, result);
    
    sess->state=goep_connected;
    GoepPacketComplete(sess);
}

static void handlePulling(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
    uint16 len=SourceBoundary(src);
	uint16 size=SourceSize(src);
    uint16 strt;
    
    GOEP_DEBUG( ("handlePulling\n") );
	
	if (len != size)
	{ /* Must be another packet */
		strt = len;
	}
	else
	{
		strt = 0;
	}
    
    goepDecodePacket(s, strt, len, goep_Pkt_Get);

    switch (s[0]) /* Check Response Code */
    {
    case goep_Rsp_Success:
    case goep_Rsp_Continue:
        {
			uint16 dataOffset;
			uint16 dataLength=0;
			uint32 totalLength=0;
			bool moreData;
			
			if (s[0] == goep_Rsp_Success)
			{
				dataOffset = goepHdrFindStringHeader(s, strt+3, len, goep_Hdr_EndOfBody, &dataLength);
				moreData = FALSE;
			}
			else
			{
				dataOffset = goepHdrFindStringHeader(s, strt+3, len, goep_Hdr_Body, &dataLength);
				moreData = TRUE;
			}
			
			
            if (s[0]== goep_Rsp_Success)
				moreData = FALSE;
			else
				moreData = TRUE;
			
            if (sess->state == goep_pulling_first)
            {
				uint16 nameOffset;
				uint16 nameLength=0;
				uint16 typeOffset;
				uint16 typeLength=0;
				uint16 hdrOffset;
				uint16 hdrLength=0;
                uint16 pos;
				
                /* Find Name */
                nameOffset = goepHdrFindStringHeader(s, strt+3, len, goep_Hdr_Name, &nameLength);
                
                /* Find type */
                typeOffset = goepHdrFindStringHeader(s, strt+3, len, goep_Hdr_Type, &typeLength);
                
                /* Find type */
                hdrOffset = goepHdrFindStringHeader(s, strt+3, len, goep_Hdr_AppSpecific, &hdrLength);
                
                /* Find total size */
                pos = goepHdrFindUint32Header(s, strt+3, len, goep_Hdr_Length);
                if (pos>0)
                {
                    totalLength = (uint32)(s[pos+0])<<24;
                    totalLength|= (uint32)(s[pos+1])<<16;
                    totalLength|= (uint32)(s[pos+2])<<8;
                    totalLength|= (uint32)(s[pos+3]);
                }
				/* Send Start Ind */
				if ((hdrOffset>0) && (hdrLength>0))
					goepMsgSendLocalGetStartHdrInd(sess, src, nameOffset, nameLength, typeOffset, typeLength,
													dataOffset, dataLength, hdrOffset, hdrLength, 
													totalLength, moreData);
				else
					goepMsgSendLocalGetStartInd(sess, src, nameOffset, nameLength, typeOffset, typeLength,
													dataOffset, dataLength, totalLength, moreData);

			}
			else
			{
				/* Send Data Ind */
				goepMsgSendLocalGetDataInd(sess, src, dataOffset, dataLength, moreData);
			}
			
            if (s[0]== goep_Rsp_Success)
            {
				/* Send get complete ind */
				goepMsgSendLocalGetCompleteInd(sess, goep_success);
                sess->state= goep_connected;
            }
            break;
        }
        default:
        {
            goep_lib_status err;
            switch (s[0])
            {
            case goep_Rsp_BadRequest:
                err= goep_get_badrequest;
                break;
            case goep_Rsp_Forbidden:
                err= goep_get_forbidden;
                break;
            case goep_Rsp_NotFound:
                err= goep_get_notfound;
                break;
                default: /* Error */            
                err= goep_failure;
                break;
            }
			goepMsgSendLocalGetCompleteInd(sess, err);
            sess->state=goep_connected;
            GoepPacketComplete(sess);
            break;
        }
        
    }
}

static void handleAbort(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
    uint16 len=SourceBoundary(src);
    uint16 start=0;
    
    GOEP_DEBUG( ("handleAbort\n") );
    
    if (s[0]==goep_Rsp_Continue)
    { /* Probably the result of a PUT */
        start=goepHdrFindUint16Field(s,1);
        if (start>=len)
        { /* Only the PUT continue packet.  Drop it and wait for the Abort complete */
            GoepPacketComplete(sess);
            return;
        }
    }
    
    if (!goepHdrContainBody(s, start+3, len))
    { /* Packet doesn't contain a body header, so must be the abort response */
		goepMsgSendAbortConfirm(sess, sess->abort_state, goep_local_abort);
		sess->state=goep_connected;
    }
    
    GoepPacketComplete(sess);
}

static void handleSetPath(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
    
    switch (s[0]) /* Check Response Code */
    {
    case goep_Rsp_Success:
        {            
            sess->state=goep_connected;
            
            {
				goepMsgSendSetPathConfirm(sess, goep_success);
            }
            
            break;
        }
    case goep_Rsp_UnauthComp:
			goepMsgSendSetPathConfirm(sess, goep_setpath_unauthorised);
            sess->state=goep_connected;
        break;
    case goep_Rsp_NotFound:
			goepMsgSendSetPathConfirm(sess, goep_setpath_notfound);
            sess->state=goep_connected;
        break;
        default: /* Error */
			goepMsgSendSetPathConfirm(sess, goep_failure);
            sess->state=goep_connected;
            break;
        
    }
    
    GoepPacketComplete(sess);
}

static void cleanupConInfo(goepState *sess)
{
    GOEP_DEBUG( ("cleanupConInfo\n") );
	
	if (sess->conInfo)
	{
		free(sess->conInfo);
	}
	sess->conInfo = NULL;
	sess->conLen = 0;
}

