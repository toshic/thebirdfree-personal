/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_svr_state.c
    
DESCRIPTION
	Server driven state machine source for the Generic Object Exchange Profile (GOEP) library

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/

#include <stream.h>
#include <source.h>
#include <connection.h>
#include <memory.h>

#include "goep.h"
#include "goep_private.h"
#include "goep_svr_state.h"
#include "goep_packet.h"
#include "goep_header.h"

static bool checkConID(goepState *sess, const uint8* buffer, uint16 start, uint16 end);

static void handleRemoteAbort(goepState *sess, Source src);
static void handleRemoteDisconnect(goepState *sess, Source src);
static void handleRemoteConnect(goepState *sess, Source src);
static void handleRemotePut(goepState *sess, Source src);
static void handleRemoteGet(goepState *sess, Source src);
static void handleRemoteSetPath(goepState *sess, Source src);

bool handleServerCommand(goepState *sess, Source src)
{
    bool ret=FALSE;
    const uint8* s=SourceMap(src);
	
    switch (s[0])
    {
    case goep_Pkt_Abort:
        handleRemoteAbort(sess, src);
		GoepPacketComplete(sess);
        ret=TRUE;
        break;
    case goep_Pkt_Disconnect:
        handleRemoteDisconnect(sess, src);
        ret=TRUE;
		GoepPacketComplete(sess);
        break;
    case goep_Pkt_Connect:
        handleRemoteConnect(sess, src);
        ret=TRUE;
		GoepPacketComplete(sess);
        break;
    case goep_Pkt_Put:
    case goep_Pkt_PutLast: /* Deliberate Fallthrough */
		handleRemotePut(sess, src);
        ret=TRUE;
        break;
    case goep_Pkt_Get:
    case goep_Pkt_GetNext:
		handleRemoteGet(sess, src);
        ret=TRUE;
        break;
    case goep_Pkt_SetPath:
		handleRemoteSetPath(sess, src);
        ret=TRUE;
        break;
    default:
        ret=FALSE;
        break;
    }
	
    return ret;
}

static bool checkConID(goepState *sess, const uint8* buffer, uint16 start, uint16 end)
{
	bool ret=TRUE;
	uint16 pos=goepHdrFindUint32Header(buffer, start, end, goep_Hdr_ConnID);
	uint32 conID=0;
	if (pos>0)
	{
		conID = (uint32)(buffer[pos+0])<<24;
        conID|= (uint32)(buffer[pos+1])<<16;
        conID|= (uint32)(buffer[pos+2])<<8;
        conID|= (uint32)(buffer[pos+3]);
	}
	if (conID != sess->conID)
	{
		GOEP_DEBUG( ("     Rejecting due to Invalid Connection ID\n") );
		goepSendResponse(sess, goep_Rsp_ServUnavail);
		ret = FALSE;
	}
	
	return ret;
}

static void handleRemoteAbort(goepState *sess, Source src)
{
    uint8 respCode= goep_Rsp_Success;

    switch (sess->state)
    {
    case goep_pushing:  /* Deliberate Fallthrough */
    case goep_pushing_last:
    case goep_remote_put:
    case goep_pulling_first:  /* Deliberate Fallthrough */
    case goep_pulling:
    case goep_remote_get:
		goepMsgSendAbortConfirm(sess, sess->state, goep_host_abort);
		break;
    case goep_connecting:
    case goep_disconnecting:
    case goep_aborting:
    case goep_setpath:
    case goep_connected:
    case goep_unknown:
    case goep_initialising:
    case goep_initialised:
    default:
        respCode= goep_Rsp_BadRequest;
            break;
    }
    
    
    /* send response packet - respCode */
    goepSendResponse(sess, respCode);
    
    /* set state to connected */
    sess->state=goep_connected;
    
    /* Empty the Source as it's in an unknown state */
    SourceEmpty(src);
}

static void handleRemoteDisconnect(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
    uint16 len=SourceBoundary(src);
    
    GOEP_DEBUG( ("handleRemoteDisconnect\n") );
	
	/* Check to ensure this client is a server */
	if (sess->role == goep_Server)
	{
		if (sess->useConID)
		{ /* Check connection ID */
			if (!checkConID(sess, s, 3, len))
				return; /* Invalid connection ID */
		}
		if (sess->conInfo)
		{
            free(sess->conInfo);
			sess->conLen = 0;
            sess->conInfo=NULL;
		}
		
		if (sess->state == goep_aborting)
		{
			GOEP_DEBUG( ("     Disconnecting while aborting, abort is complete\n") );
			goepMsgSendAbortConfirm(sess, sess->state, goep_host_abort);
		}
		/* It is illegal to refuse a disconnect (unless the ConnID is wrong. */
		/* Reply to the remote client with an OK. */
		goepSendResponse(sess, goep_Rsp_Success);
		if ((sess->state == goep_connect_auth) || (sess->state == goep_connecting))
			sess->state = goep_connect_cancel;
		else
			sess->state = goep_rem_disconnecting;
	}
	else
	{ /* This client is not a server and so does not support remote connect */
		GOEP_DEBUG( ("     Rejecting due to client type\n") );
		goepSendResponse(sess, goep_Rsp_BadRequest);
	}
}

static void handleRemoteConnect(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
    uint16 len=SourceBoundary(src);
	
    GOEP_DEBUG( ("handleRemoteConnect\n") );
    goepDecodePacket(s, 0, len, goep_Pkt_Connect);    
	
	/* Check to ensure this client is a server */
	if ((sess->role == goep_Server) || (len < 7)) /* Minimum size of an OBEX connect is 7 bytes */
	{
		uint8 ver;
		uint16 maxPkt;
		uint16 tgtPos, tgtLen;
		
		/* Get and check OBEX version */
		ver=goepHdrFindUint8Field(s, 3);
		if (ver == GOEP_OBEX_VER)
		{
			/* Get max packet size */
			maxPkt = goepHdrFindUint16Field(s, 5);
			sess->pktLen = maxPkt;
			
			/* Get target */
			if (len>7)
				tgtPos = goepHdrFindStringHeader(s, 7, len, goep_Hdr_Target, &tgtLen);
			else
			{
				tgtPos=0;
				tgtLen=0;
			}
			
			if (tgtLen > MAX_TARGET_LEN)
				tgtLen = MAX_TARGET_LEN;
			
			if (sess->state == goep_connect_auth)
			{
				uint16 auth_pos, auth_len = 0;
				
				auth_pos = goepHdrFindStringHeader(s, 7, len, goep_Hdr_AuthResponse, &auth_len);
				if (auth_pos > 0)
				{
					uint16 size_user = 0;
					const uint8 *request = NULL, *nonce = NULL, *user = NULL;
					GOEP_AUTH_RESULT_IND_T *message = NULL;
				
					uint16 curr = auth_pos;
					uint16 end = auth_pos + auth_len;
					
				    GOEP_DEBUG(("    Header Present\n"));
					
					if (sess->authChallenge)
						free(sess->authChallenge);
					sess->authChallenge = (uint8 *)PanicUnlessMalloc(sizeof(uint8)*(auth_len+1));
					/* If the string is longer than 255 bytes, the allocation above will probably have caused a Panic */
					sess->authChallenge[0] = (uint8)auth_len;
					memmove(&sess->authChallenge[1], &s[auth_pos], auth_len);
				
					/* Find values */
					while (curr < end)
					{
						switch (s[curr])
						{
						case GOEP_AUTH_RES_DIGEST:
							request = &s[curr+2];
							break;
						case GOEP_AUTH_RES_USERID:
							size_user = s[curr+1];
							user = &s[curr+2];
							break;
						case GOEP_AUTH_RES_NONCE:
							nonce = &s[curr+2];
							break;
						}
						curr += s[curr+1];
						curr += 2;
					}
				
					/* Create message */
					if (size_user > GOEP_SIZE_MD5_STRING)
						size_user = GOEP_SIZE_MD5_STRING;
				
					message = (GOEP_AUTH_RESULT_IND_T *)PanicUnlessMalloc(sizeof(GOEP_AUTH_RESULT_IND_T));
					/* Copy values into message */
					message->goep = sess;
					memmove(&message->request[0], request, GOEP_SIZE_DIGEST);
					if (nonce)
						memmove(&message->nonce[0], nonce, GOEP_SIZE_DIGEST);
					else
						message->nonce[0] = 0;
					message->size_userid = size_user;
					if (size_user > 0)
					{
						message->userid = (uint8*)PanicUnlessMalloc(size_user * sizeof(uint8));
						memmove(message->userid, user, size_user);
					}
					else
						message->userid = NULL;
		    		MessageSend(sess->theApp, GOEP_AUTH_RESULT_IND, message);
				}
				else
				{ /* no response header! */
					GOEP_DEBUG( ("     Rejecting connection due to there not being a response header\n") );
					goepSendResponse(sess, goep_Rsp_BadRequest);
				}
			}
			else
	        { /* Send to application */
    			MAKE_GOEP_MESSAGE_WITH_LEN(GOEP_CONNECT_IND, tgtLen);
				message->goep = sess;
				message->bd_addr = sess->bdAddr;
				message->maxPacketLen = maxPkt;
				message->size_target = tgtLen;
				if (tgtLen > 0)
				{
					memmove(&message->target[0], &s[tgtPos], tgtLen);
					/* Store for future use */
					sess->conInfo = (uint8*)PanicUnlessMalloc(sizeof(uint8)*tgtLen);
					sess->conLen = tgtLen;
					memmove(sess->conInfo, &s[tgtPos], sizeof(uint8)*tgtLen);
				}
				else
					message->target[0] = 0;
				
    			MessageSend(sess->theApp, GOEP_CONNECT_IND, message);
	       }
		}
		else
		{ /* We do not support this version of OBEX, reject the connection */
			GOEP_DEBUG( ("     Rejecting connection due to wrong OBEX Version\n") );
			goepSendResponse(sess, goep_Rsp_BadRequest);
		}
	}
	else
	{ /* This client is not a server and so does not support remote connect */
		GOEP_DEBUG( ("     Rejecting connection due to client type or invalid packet\n") );
		goepSendResponse(sess, goep_Rsp_BadRequest);
	}
}

static void handleRemotePut(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
    uint16 len=SourceBoundary(src);
	uint16 size=SourceSize(src);
    uint16 strt;
    
    GOEP_DEBUG( ("handleRemotePut\n") );
	
	if (len != size)
	{ /* Must be another packet */
		strt = len;
	}
	else
	{
		strt = 0;
	}
	
	/* Check to ensure this client is a server */
	if (sess->role == goep_Server)
	{
		if (sess->useConID)
		{ /* Check connection ID */
			if (!checkConID(sess, s, strt+3, len))
			{				
				GoepPacketComplete(sess);
				return; /* Invalid connection ID */
			}
		}
		
		/* Check state to ensure we can accept the command */
		if ((sess->state == goep_connected) || (sess->state == goep_remote_put))
		{
			uint16 nameOffset=0;
			uint16 nameLength;
			uint16 typeOffset=0;
			uint16 typeLength;
			uint16 dataOffset;
			uint16 dataLength=0;
			uint32 totalLength=0;
			bool moreData, del_used=FALSE;
		
			
			if (s[0] == goep_Pkt_PutLast)
			{
				dataOffset = goepHdrFindStringHeader(s, strt+3, len, goep_Hdr_EndOfBody, &dataLength);
				moreData = FALSE;
			}
			else
			{
				dataOffset = goepHdrFindStringHeader(s, strt+3, len, goep_Hdr_Body, &dataLength);
				moreData = TRUE;
			}
			
			if (sess->state == goep_connected)
			{ /* Must be first packet */
				nameOffset = goepHdrFindStringHeader(s, strt+3, len, goep_Hdr_Name, &nameLength);
				
				if ((dataOffset == 0) && (dataLength == 0) && (s[0] == goep_Pkt_PutLast))
				{ /* PUT is a delete request */
					MAKE_GOEP_MESSAGE(GOEP_DELETE_IND);
					message->goep = sess;
					
					message->src = src;
					message->nameOffset = nameOffset;
					message->nameLength = nameLength;

					MessageSend(sess->theApp, GOEP_DELETE_IND, message);

					GOEP_DEBUG(("     Sending Delete Ind\n"));
					del_used = TRUE;
				}
				else
				{
					uint16 lenpos;
					/* Extract object details and send Remote Put Start Message */	
					typeOffset = goepHdrFindStringHeader(s, strt+3, len, goep_Hdr_Type, &typeLength);
					lenpos = goepHdrFindUint32Header(s, strt+3, len, goep_Hdr_Length);
					if (lenpos>0)
					{
						totalLength = (uint32)(s[lenpos+0])<<24;
						totalLength|= (uint32)(s[lenpos+1])<<16;
						totalLength|= (uint32)(s[lenpos+2])<<8;
						totalLength|= (uint32)(s[lenpos+3]);
					}
				}
			}
			
			if (!del_used)
			{ /* Request is a normal PUT and not delete */
				if (sess->state == goep_connected)
				{ /* Send Start Ind */
					goepMsgSendRemotePutStartInd(sess, src, nameOffset, nameLength, typeOffset, typeLength,
													dataOffset, dataLength, totalLength, moreData);
				}
				else
				{ /* Send Data Ind */
					goepMsgSendRemotePutDataInd(sess, src, dataOffset, dataLength, moreData);
				}
			}
			sess->state = goep_remote_put;
		}
		else
		{
			if (sess->state == goep_aborting)
			{
				GOEP_DEBUG( ("     Aborting, so dump the packet\n") );
			}
			else
			{
				GOEP_DEBUG( ("     Rejecting due to client being busy\n") );
				goepSendResponse(sess, goep_Rsp_BadRequest);
			}
			GoepPacketComplete(sess);
		}
	}
	else
	{ /* This client is not a server and so does not support remote put */
		GOEP_DEBUG( ("     Rejecting due to client type\n") );
		goepSendResponse(sess, goep_Rsp_BadRequest);
		GoepPacketComplete(sess);
	}
}

static void handleRemoteGet(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
    uint16 len=SourceBoundary(src);
    
    GOEP_DEBUG( ("handleRemoteGet\n") );
	
	/* Check to ensure this client is a server */
	if (sess->role == goep_Server)
	{
		if (sess->useConID)
		{ /* Check connection ID */
			if (!checkConID(sess, s, 3, len))
			{				
				GoepPacketComplete(sess);
				return; /* Invalid connection ID */
			}
		}
		
		/* Check state to ensure we can accept the command */
		if ((sess->state == goep_connected) || (sess->state == goep_remote_get))
		{
			goepDecodePacket(s, 0, len, goep_Pkt_Get);
			if (sess->state == goep_connected)
			{ /* First Packet, extract data and sent to client */
				uint16 nameOffset;
				uint16 nameLength=0;
				uint16 typeOffset;
				uint16 typeLength=0;
				uint16 hdrOffset;
				uint16 hdrLength=0;
				
				/* Extract Name Header */
				nameOffset = goepHdrFindStringHeader(s, 3, len, goep_Hdr_Name, &nameLength);
				/* Extract Type Header */
				typeOffset = goepHdrFindStringHeader(s, 3, len, goep_Hdr_Type, &typeLength);
				/* Extract App. Specific Headers Header */
				hdrOffset = goepHdrFindStringHeader(s, 3, len, goep_Hdr_AppSpecific, &hdrLength);
				
				if (nameLength == 0)
					nameOffset = 0;
				
				/* Send to App */
				if ((hdrOffset>0) && (hdrLength>0))
					goepMsgSendRemoteGetStartHdrsInd(sess, src, nameOffset, nameLength, typeOffset, typeLength,
													hdrOffset, hdrLength);
				else
					goepMsgSendRemoteGetStartInd(sess, src, nameOffset, nameLength, typeOffset, typeLength);
			}
			else
			{
	   			MAKE_GOEP_MESSAGE(GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND);
    	        message->goep = sess;
    			MessageSend(sess->theApp, GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND, message);
				GoepPacketComplete(sess);
			}
			sess->state = goep_remote_get;
		}
	}
	else
	{ /* This client is not a server and so does not support remote put */
		GOEP_DEBUG( ("     Rejecting due to client type\n") );
		goepSendResponse(sess, goep_Rsp_BadRequest);
		GoepPacketComplete(sess);
	}
}

static void handleRemoteSetPath(goepState *sess, Source src)
{
    const uint8* s=SourceMap(src);
    uint16 len=SourceBoundary(src);
    
    GOEP_DEBUG( ("handleRemoteSetPath\n") );
	
	/* Check to ensure this client is a server */
	if ((sess->role == goep_Server) && (sess->state == goep_connected))
	{
		if (sess->useConID)
		{ /* Check connection ID */
			if (!checkConID(sess, s, 5, len))
			{				
				GoepPacketComplete(sess);
				return; /* Invalid connection ID */
			}
		}
		{
			MAKE_GOEP_MESSAGE(GOEP_SET_PATH_IND);
			message->goep = sess;
			message->src=src;
			message->flags = goepHdrFindUint8Field(s, 3);
			message->nameOffset = goepHdrFindStringHeader(s, 5, len, goep_Hdr_Name, &message->nameLength);
			
			MessageSend(sess->theApp, GOEP_SET_PATH_IND, message);
			sess->state = goep_setpath;		
		}
	}
	else
	{ /* This client is not a server and so does not support remote put */
		GOEP_DEBUG( ("     Rejecting due to client type\n") );
		goepSendResponse(sess, goep_Rsp_BadRequest);
		GoepPacketComplete(sess);
	}
}

