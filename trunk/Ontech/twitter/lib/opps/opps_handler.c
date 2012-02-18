/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    opps_handler.c
    
DESCRIPTION
	Message handler source for the OPP Server library

*/

#include <message.h>
#include <stream.h>
#include <source.h>
#include <panic.h>
#include <print.h>
#include <string.h>

#include <app/bluestack/types.h>
#include <app/bluestack/bluetooth.h>
#include <app/bluestack/sdc_prim.h>
#include <service.h>
#include <connection.h>

#include <ctype.h>

#include <sdp_parse.h>
#include "opps.h"
#include "opps_private.h"

/* Static Strings */
static const uint8 vcardType[] = {'t','e','x','t','/','x','-','v','c','a','r','d'};

/* Default SDP Record */
static const uint8 serviceRecordOPP[] =
    {			
        /* Service class ID list */
        0x09,0x00,0x01,		/* AttrID , ServiceClassIDList */
        0x35,0x03,			/* 3 bytes in total DataElSeq */
        0x19,0x11,0x05,		/* 2 byte UUID, Service class = OBEXObjectPush */

        /* protocol descriptor list */
        0x09,0x00,0x04,		/* AttrId ProtocolDescriptorList */
        0x35,0x11,			/* 17 bytes in total DataElSeq */
        0x35,0x03,			/* 3 bytes in DataElSeq */
        0x19,0x01,0x00,		/* 2 byte UUID, Protocol = L2CAP */

        0x35,0x05,			/* 5 bytes in DataElSeq */
        0x19,0x00,0x03,		/* 2 byte UUID Protocol = RFCOMM */
        0x08,0x00,			/* 1 byte UINT - server channel template value 0 - to be filled in by app */

		0x35,0x03,			/* 3 bytes in DataElSeq */
		0x19, 0x00, 0x08,	/* 2 byte UUID, Protocol = OBEX */

		/* profile descriptor list */
        0x09,0x00,0x09,		/* AttrId, ProfileDescriptorList */
		0x35,0x08,			/* DataElSeq wrapper */
        0x35,0x06,			/* 6 bytes in total DataElSeq */
        0x19,0x11,0x05,		/* 2 byte UUID, Service class = OBEXObjectPush */
        0x09,0x01,0x00,		/* 2 byte uint, version = 100 */

		/* service name */
        0x09,0x01,0x00,		/* AttrId - Service Name */
        0x25,0x10,			/* 16 byte string - OBEX Object Push */
        'O','B','E','X',' ','O','b','j','e','c','t',' ','P','u','s','h',
        
		/* Supported Formats List */
		0x09,0x03,0x03,		/* AttrId - Supported Formats List */
		0x35,0x02,			/* 2 bytes in dataElSeq */
		0x08,0xff,			/* Accept Any Object */
    };

/* Error value conversion */
static opps_lib_status convert_error(goep_lib_status status);

/* Internal Message Handlers */
static void handleIntAcceptConnection(oppsState *state, OPPS_INT_ACCEPT_CONNECTION_T *msg);
static void handleIntAuthChallenge(oppsState *state, OPPS_INT_AUTH_CHALLENGE_T *msg);
static void handleIntAbort(oppsState *state);
static void handleIntGetNextPacket(oppsState *state, OPPS_INT_GET_NEXT_PACKET_T *msg);
static void handleIntPushVCard(oppsState *state, OPPS_INT_PUSH_VCARD_T *msg);
static void handleIntPushNextVCard(oppsState *state, OPPS_INT_PUSH_NEXT_VCARD_T *msg);
static void handleIntPushVCardSrc(oppsState *state, OPPS_INT_PUSH_VCARD_SRC_T *msg);
static void handleIntPushNextVCardSrc(oppsState *state, OPPS_INT_PUSH_NEXT_VCARD_SRC_T *msg);

/* GOEP Library Message Handlers */
static void handleGoepInitCfm(oppsState *state, GOEP_INIT_CFM_T *msg);
static void handleGoepChannelInd(oppsState *state, GOEP_CHANNEL_IND_T *msg);
static void handleGoepConnectInd(oppsState *state, GOEP_CONNECT_IND_T *msg);
static void handleGoepConnectCfm(oppsState *state, GOEP_CONNECT_CFM_T *msg);
static void handleGoepAuthResultInd(oppsState *state, GOEP_AUTH_RESULT_IND_T *msg);
static void handleGoepRemPutStartInd(oppsState *state, GOEP_REMOTE_PUT_START_IND_T *msg);
static void handleGoepRemPutDataInd(oppsState *state, GOEP_REMOTE_PUT_DATA_IND_T *msg);
static void handleGoepRemPutCompleteInd(oppsState *state, GOEP_REMOTE_PUT_COMPLETE_IND_T *msg);
static void handleGoepDisconnectInd(oppsState *state, GOEP_DISCONNECT_IND_T *msg);
static void handleGoepRemGetStart(oppsState *state, GOEP_REMOTE_GET_START_IND_T *msg);
static void handleGoepRemGetMoreData(oppsState *state, GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND_T *msg);
static void handleGoepRemGetCompleteInd(oppsState *state, GOEP_REMOTE_GET_COMPLETE_IND_T *msg);

/* Connection Library message handlers */
static void handleSDPRegisterCfm(oppsState *state, CL_SDP_REGISTER_CFM_T *msg);

/* Is the type header indicating a vCard? */
static bool isVCard(const uint8 *src, uint8 srcLen);

void oppsHandler(Task task, MessageId id, Message message)
{
	/* Get task control block */
	oppsState *state = (oppsState*)task;

    switch(id)
    {
        /* Internal OPP Server messages */
	case OPPS_INT_ACCEPT_CONNECTION:
		handleIntAcceptConnection(state, (OPPS_INT_ACCEPT_CONNECTION_T*)message);
		break;
	case OPPS_INT_AUTH_CHALLENGE:
		handleIntAuthChallenge(state, (OPPS_INT_AUTH_CHALLENGE_T*)message);
		break;
	case OPPS_INT_ABORT:
		handleIntAbort(state);
		break;
	case OPPS_INT_GET_NEXT_PACKET:
		handleIntGetNextPacket(state, (OPPS_INT_GET_NEXT_PACKET_T*)message);
		break;
	case OPPS_INT_PUSH_VCARD:
		handleIntPushVCard(state, (OPPS_INT_PUSH_VCARD_T*)message);
		break;
	case OPPS_INT_PUSH_NEXT_VCARD:
		handleIntPushNextVCard(state, (OPPS_INT_PUSH_NEXT_VCARD_T*)message);
		break;
	case OPPS_INT_PUSH_VCARD_SRC:
		handleIntPushVCardSrc(state, (OPPS_INT_PUSH_VCARD_SRC_T*)message);
		break;
	case OPPS_INT_PUSH_NEXT_VCARD_SRC:
		handleIntPushNextVCardSrc(state, (OPPS_INT_PUSH_NEXT_VCARD_SRC_T*)message);
		break;
		
        /* Messages from the GOEP Library */
	case GOEP_INIT_CFM:
		handleGoepInitCfm(state, (GOEP_INIT_CFM_T*)message);
		break;
	case GOEP_CHANNEL_IND:
		handleGoepChannelInd(state, (GOEP_CHANNEL_IND_T*)message);
		break;
    case GOEP_CONNECT_IND:
        handleGoepConnectInd(state, (GOEP_CONNECT_IND_T*) message);
        break;
    case GOEP_CONNECT_CFM:
        handleGoepConnectCfm(state, (GOEP_CONNECT_CFM_T*)message);
        break;
	case GOEP_AUTH_RESULT_IND:
		handleGoepAuthResultInd(state, (GOEP_AUTH_RESULT_IND_T*)message);
		break;
	case GOEP_REMOTE_PUT_START_IND:
		handleGoepRemPutStartInd(state, (GOEP_REMOTE_PUT_START_IND_T*)message);
		break;
	case GOEP_REMOTE_PUT_DATA_IND:
		handleGoepRemPutDataInd(state, (GOEP_REMOTE_PUT_DATA_IND_T*)message);
		break;
	case GOEP_REMOTE_PUT_COMPLETE_IND:
		handleGoepRemPutCompleteInd(state, (GOEP_REMOTE_PUT_COMPLETE_IND_T*)message);
		break;
    case GOEP_DISCONNECT_IND:
        handleGoepDisconnectInd(state, (GOEP_DISCONNECT_IND_T*)message);
        break;
	case GOEP_REMOTE_GET_START_IND:
		handleGoepRemGetStart(state, (GOEP_REMOTE_GET_START_IND_T*)message);
		break;
	case GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND:
		handleGoepRemGetMoreData(state, (GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND_T*)message);
		break;
	case GOEP_REMOTE_GET_COMPLETE_IND:
		handleGoepRemGetCompleteInd(state, (GOEP_REMOTE_GET_COMPLETE_IND_T*)message);
		break;
		
		/* Messages from the connection library */
	case CL_SDP_REGISTER_CFM:
		handleSDPRegisterCfm(state, (CL_SDP_REGISTER_CFM_T*)message);
		break;
        
	default:
		PRINT(("OPPS Unhandled Message : %d , 0x%0X\n",id,id) ); 
		break;
	}
}

/* Internal Message Handlers */
static void handleIntAcceptConnection(oppsState *state, OPPS_INT_ACCEPT_CONNECTION_T *msg)
{
    PRINT(("handleIntAcceptConnection\n"));
    
    if (state->currCom!= opps_com_Connecting)
    { /* Not idle, Send Error Message */
		oppsMsgSendConnectCfm(state, opps_wrong_command, 0);
		state->currCom= opps_com_None;
    }
    else
    {
		if (msg->accept)
		{ /* Send accept response code */
			GoepConnectResponse(state->handle, goep_svr_resp_OK, msg->pktSize);
		}
		else
		{ /* Send reject response code */
			GoepConnectResponse(state->handle, goep_svr_resp_BadRequest, msg->pktSize);
	        state->currCom= opps_com_None;
		}
    }
}

static void handleIntAuthChallenge(oppsState *state, OPPS_INT_AUTH_CHALLENGE_T *msg)
{
    PRINT(("handleIntAuthChallenge\n"));
    
    if (state->currCom!= opps_com_Connecting)
    { /* Not idle, Send Error Message */
		oppsMsgSendConnectCfm(state, opps_wrong_state, 0);
    }
    else
    {
		GoepConnectAuthChallenge(state->handle, msg->nonce, msg->options, msg->size_realm, msg->realm);
    }
}

static void handleIntAbort(oppsState *state)
{
    PRINT(("handleIntAbort\n"));
    
	GoepAbort(state->handle);
}

static void handleIntGetNextPacket(oppsState *state, OPPS_INT_GET_NEXT_PACKET_T *msg)
{
    PRINT(("handleIntGetNextPacket\n"));
    
	if ((state->currCom != opps_com_PushVCard) && (state->currCom != opps_com_PushObject))
    { /* Not receiving data, Send Error Message */
		if (state->currCom == opps_com_PushVCard)
			oppsMsgSendPushBcCompleteInd(state, opps_wrong_command);
		else
			oppsMsgSendPushObjectCompleteInd(state, opps_wrong_command);
		
		state->currCom= opps_com_None;
    }
    else
    {        
		if (msg->moreData)
			GoepRemotePutResponse(state->handle, goep_svr_resp_Continue);
		else
		{
        	GoepRemotePutResponse(state->handle, goep_svr_resp_OK);
		}
    }
}

static void handleIntPushVCard(oppsState *state, OPPS_INT_PUSH_VCARD_T *msg)
{
    PRINT(("handleIntPushVCard\n"));
    
	if (state->currCom != opps_com_PullVCard)
    { /* Not sending data, Send Error Message */
		oppsMsgSendPullBcCompleteInd(state, opps_wrong_command);
		state->currCom= opps_com_None;
    }
    else
    {    
		GoepRemoteGetResponse(state->handle, goep_svr_resp_OK, msg->totalLen,
				  msg->nameLen, msg->name, sizeof(vcardType), &vcardType[0], 
				  msg->length, msg->packet, msg->onlyPacket);
    }
}

static void handleIntPushNextVCard(oppsState *state, OPPS_INT_PUSH_NEXT_VCARD_T *msg)
{
    PRINT(("handleIntPushNextVCard\n"));
    
	if (state->currCom != opps_com_PullVCard)
    { /* Not sending data, Send Error Message */
		oppsMsgSendPullBcCompleteInd(state, opps_wrong_command);
		state->currCom= opps_com_None;
    }
    else
    {    
		GoepRemoteGetResponse(state->handle, goep_svr_resp_OK, 0,
				  0, NULL, 0, NULL, msg->length, msg->packet, msg->lastPacket);
    }
}

static void handleIntPushVCardSrc(oppsState *state, OPPS_INT_PUSH_VCARD_SRC_T *msg)
{
    PRINT(("handleIntPushVCardSrc\n"));
    
	if (state->currCom != opps_com_PullVCard)
    { /* Not sending data, Send Error Message */
		oppsMsgSendPullBcCompleteInd(state, opps_wrong_command);
		state->currCom= opps_com_None;
    }
    else
    {    
		GoepRemoteGetResponseSource(state->handle, goep_svr_resp_OK, msg->totalLen,
				  msg->nameLen, msg->name, sizeof(vcardType), &vcardType[0], 
				  msg->length, msg->src, msg->onlyPacket);
    }
}

static void handleIntPushNextVCardSrc(oppsState *state, OPPS_INT_PUSH_NEXT_VCARD_SRC_T *msg)
{
    PRINT(("handleIntPushNextVCardSrc\n"));
    
	if (state->currCom != opps_com_PullVCard)
    { /* Not sending data, Send Error Message */
		oppsMsgSendPullBcCompleteInd(state, opps_wrong_command);
		state->currCom= opps_com_None;
    }
    else
    {    
		GoepRemoteGetResponseSource(state->handle, goep_svr_resp_OK, 0,
				  0, NULL, 0, NULL, msg->length, msg->src, msg->lastPacket);
    }
}

/* GOEP Library Message Handlers */
static void handleGoepInitCfm(oppsState *state, GOEP_INIT_CFM_T *msg)
{
	PRINT(("handleGoepConnectInd\n"));
	
	if (msg->status == goep_success)
	{
        state->handle = msg->goep;
        state->currCom= opps_com_None;
		
		GoepGetChannel(state->handle);
	}
	else
	{
		oppsMsgSendInitCfm(NULL, opps_failure);
		free(state);
	}
}

static void handleGoepChannelInd(oppsState *state, GOEP_CHANNEL_IND_T *msg)
{
	PRINT(("handleGoepChannelInd\n"));
	
	if (msg->status == goep_success)
	{
		uint8* sdp;
				
		state->rfcChan = msg->channel;
		
		sdp = (uint8 *)PanicUnlessMalloc(sizeof(serviceRecordOPP));
		memcpy(sdp, serviceRecordOPP, sizeof(serviceRecordOPP));

		if (!SdpParseInsertRfcommServerChannel(sizeof(serviceRecordOPP), sdp, state->rfcChan))
		{
			oppsMsgSendInitCfm(NULL, opps_failure);
			free(sdp);
			free(state);
		}
		else
		{
			/* Send the service record to the connection lib to be registered with BlueStack */
			ConnectionRegisterServiceRecord(&state->task, sizeof(serviceRecordOPP),sdp);
		}
	}
	else
	{
		oppsMsgSendInitCfm(NULL, opps_failure);
		free(state);
	}
}

static void handleGoepConnectInd(oppsState *state, GOEP_CONNECT_IND_T *msg)
{
   	MAKE_OPPS_MESSAGE(OPPS_CONNECT_IND);
	
    PRINT(("handleGoepConnectInd\n"));
	
	message->opps = (OPPS*)state;
	message->bd_addr = msg->bd_addr;
	message->maxPacketLen = msg->maxPacketLen;
        
   	MessageSend(state->theAppTask, OPPS_CONNECT_IND, message);
		
	state->currCom = opps_com_Connecting;
}

static void handleGoepConnectCfm(oppsState *state, GOEP_CONNECT_CFM_T *msg)
{
    PRINT(("handleGoepConnectCfm\n"));
    
    if (msg->status != goep_success)
    { /* Couldn't connect */
		oppsMsgSendConnectCfm(state, opps_failure, 0);
		state->currCom= opps_com_None;
    }
    else
    { /* GOEP has connected with the server, inform App. */
        /* Return to Idle state */
        state->currCom= opps_com_None;
		
		oppsMsgSendConnectCfm(state, opps_success, msg->maxPacketLen);
    }
}

static void handleGoepAuthResultInd(oppsState *state, GOEP_AUTH_RESULT_IND_T *msg)
{
	OPPS_AUTH_RESULT_IND_T *message = (OPPS_AUTH_RESULT_IND_T *)
										PanicUnlessMalloc(sizeof(OPPS_AUTH_RESULT_IND_T) + msg->size_userid);
    PRINT(("handleGoepAuthResultInd\n"));
	
	memcpy(message, msg, sizeof(OPPS_AUTH_RESULT_IND_T) + msg->size_userid);
	message->opps = state;
	
    MessageSend(state->theAppTask, OPPS_AUTH_RESULT_IND, message);
}

static void handleGoepRemPutStartInd(oppsState *state, GOEP_REMOTE_PUT_START_IND_T *msg)
{
    PRINT(("handleGoepRemPutStartInd\n"));
	
	if (state->currCom == opps_com_None)
	{
		bool pushBC = FALSE;
		
		/* Check type to find the actual message id */
		if (msg->typeLength > 0)
		{
			const uint8 *s = SourceMap(msg->src);
			if (isVCard(&s[msg->typeOffset], msg->typeLength))
			{ /* Message is a Business Card push */
				pushBC = TRUE;
			}
		}
		
		if (pushBC)
		{
			oppsMsgSendPushBcStartInd(state, msg->src, msg->nameOffset, msg->nameLength,
										msg->totalLength, msg->dataLength, msg->dataOffset,
										msg->moreData);
			state->currCom = opps_com_PushVCard;
		}
		else
		{
			oppsMsgSendPushObjStartInd(state, msg->src, msg->nameOffset, msg->nameLength,
									msg->typeOffset, msg->typeLength, msg->totalLength, msg->dataLength,
									msg->dataOffset, msg->moreData);
			state->currCom = opps_com_PushObject;

		}
	}
	else
	{
		OppsPacketComplete(state);
		GoepRemotePutResponse(state->handle, goep_svr_resp_BadRequest);
	}
}

static void handleGoepRemPutDataInd(oppsState *state, GOEP_REMOTE_PUT_DATA_IND_T *msg)
{
    PRINT(("handleGoepRemPutDataInd\n"));
	
	if ((state->currCom == opps_com_PushVCard) || (state->currCom == opps_com_PushObject))
	{
		MessageId id=OPPS_PUSH_OBJ_DATA_IND;
		/* Create generic message */
		MAKE_OPPS_MESSAGE(OPPS_PUSH_OBJ_DATA_IND);
		/* Copy fields */
		message->opps = (OPPS*)state;
		message->src = msg->src;
		message->packetLen = msg->dataLength;
		message->packetOffset = msg->dataOffset;
		message->moreData = msg->moreData;
		
		/* Check which type of push we are doing */
		if (state->currCom == opps_com_PushVCard)
		{
			id = OPPS_PUSH_BC_DATA_IND;
		}
		
		/* send message */
   		MessageSend(state->theAppTask, id, message);
	}
	else
	{
		/* Flush the source */
		OppsPacketComplete(state);
		/* Send reject packet */
		GoepRemotePutResponse(state->handle, goep_svr_resp_BadRequest);
	}
}

static void handleGoepRemPutCompleteInd(oppsState *state, GOEP_REMOTE_PUT_COMPLETE_IND_T *msg)
{
    PRINT(("handleGoepRemPutCompleteInd\n"));
	
	if ((state->currCom == opps_com_PushVCard) || (state->currCom == opps_com_PushObject))
	{
		opps_lib_status status  = convert_error(msg->status);
		
		if (state->currCom == opps_com_PushVCard)
			oppsMsgSendPushBcCompleteInd(state, status);
		else
			oppsMsgSendPushObjectCompleteInd(state, status);
		
		state->currCom = opps_com_None;
	}
}

static void handleGoepDisconnectInd(oppsState *state, GOEP_DISCONNECT_IND_T *msg)
{
	MAKE_OPPS_MESSAGE(OPPS_DISCONNECT_IND);
    PRINT(("handleGoepDisconnectInd\n"));
	message->opps = (OPPS*)state;
    
	MessageSend(state->theAppTask, OPPS_DISCONNECT_IND, message);
}

static void handleGoepRemGetStart(oppsState *state, GOEP_REMOTE_GET_START_IND_T *msg)
{
    PRINT(("handleGoepRemGetStart\n"));
	
	if (state->currCom == opps_com_None)
	{
		/* Check name and contents of Type Header */
		const uint8 *s = SourceMap(msg->src);
		if ((msg->nameLength!=0) || (!isVCard(&s[msg->typeOffset], msg->typeLength)))
		{ /* Either name or type invalid, reject request */
			GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0,
				  0, NULL, 0, NULL, 0, NULL, TRUE);
		}
		else
		{ /* Send request upto App. */
			MAKE_OPPS_MESSAGE(OPPS_PULL_BC_START_IND);
			message->opps = (OPPS*)state;
	   		MessageSend(state->theAppTask, OPPS_PULL_BC_START_IND, message);
			state->currCom = opps_com_PullVCard;
		}
	}
	else
	{
		/* Send reject packet */
		GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0, 0, NULL, 0, NULL, 0, NULL,TRUE);
	}
	/* Flush the source */
	OppsPacketComplete(state);
}

static void handleGoepRemGetMoreData(oppsState *state, GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND_T *msg)
{
    PRINT(("handleGoepRemGetMoreData\n"));
    
	if (state->currCom == opps_com_PullVCard)
	{
		MAKE_OPPS_MESSAGE(OPPS_PULL_BC_MOREDATA_IND);
		message->opps = (OPPS*)state;
		MessageSend(state->theAppTask, OPPS_PULL_BC_MOREDATA_IND, message);
	}
	else
	{
		/* Send reject packet */
		GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0, 0, NULL, 0, NULL, 0, NULL,TRUE);
	}
}

static void handleGoepRemGetCompleteInd(oppsState *state, GOEP_REMOTE_GET_COMPLETE_IND_T *msg)
{
    PRINT(("handleGoepRemGetCompleteInd\n"));

	if (state->currCom != opps_com_PullVCard)
	{ /* Busy */
		PRINT(("       Busy\n"));
		GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0, 0, NULL, 0, NULL, 0, NULL, TRUE);
	}
	else
	{
		opps_lib_status status = convert_error(msg->status);
		
		oppsMsgSendPullBcCompleteInd(state, status);
		
		state->currCom = opps_com_None;
	}
}

/* Connection Library message handlers */
static void handleSDPRegisterCfm(oppsState *state, CL_SDP_REGISTER_CFM_T *msg)
{
	PRINT(("handleSDPRegisterCfm\n"));
	
	if (msg->status == success)
	{
		state->sdpHandle = msg->service_handle;
		oppsMsgSendInitCfm((OPPS*)state, opps_success);
	}
	else
	{
		oppsMsgSendInitCfm(NULL, opps_failure);
		free(state);
	}
}

/* Error value conversion */
static opps_lib_status convert_error(goep_lib_status status)
{
	opps_lib_status ret = opps_success;
	
	switch (status)
	{
	case goep_success:
		ret = opps_success;
		break;
	case goep_host_abort:
	case goep_local_abort: /* Deliberate Fallthrough */
		ret = opps_aborted;
		break;
	default:
		ret = opps_failure;
		break;
	}
	
	return ret;
}

#include <stdio.h>

/* Is the type header indicating a vCard? */
static bool isVCard(const uint8 *src, uint8 srcLen)
{
	const uint8 *s = &src[0];
	const uint8 *v = &vcardType[0];
	uint8 sc, vc;
	uint16 n = srcLen;
	
	/* if lengths different, strings must be different, unless one is NULL terminated */
	if ((srcLen != sizeof(vcardType)) && (s[srcLen-1] != 0)) 
		return FALSE;
	
	if (s[srcLen-1] == 0)
		n--; /* mask the NULL terminator */

	while (n-- > 0)
    {
		sc = (uint8) *s++;
		vc = (uint8) *v++;
		if (sc != vc)
		{
			if ((sc>='A') && (sc<='Z'))
			{
				if ((uint8)tolower(sc) != vc)
				{ /* Different real characters, so strings different */
					return FALSE;
				}
			}
			else
			{ /* Character different, and not a capital, so string different */
				return FALSE;
			}
		}
    }
	
	return TRUE;
}
