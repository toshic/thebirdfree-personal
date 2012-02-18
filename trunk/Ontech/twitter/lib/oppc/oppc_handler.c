/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    oppc_handler.c
    
DESCRIPTION
	Message handler source for the OPP Client library

*/

#include <bdaddr.h>
#include <message.h>
#include <stream.h>
#include <source.h>
#include <panic.h>
#include <print.h>
#include <string.h>

#include "oppc.h"
#include "oppc_private.h"

/* Error value conversion */
static oppc_lib_status convert_error(goep_lib_status status);

/* Internal Message Handlers */
static void handleIntConnect(oppcState *state, const OPPC_INT_CONNECT_T *msg);
static void handleIntAuthResp(oppcState *state, OPPC_INT_AUTH_RESP_T *msg);
static void handleIntDisconnect(oppcState *state);

static void handleIntAbort(oppcState *state, OPPC_INT_ABORT_T *msg);
static void handleIntPushObject(oppcState *state, OPPC_INT_PUSH_OBJECT_T *msg);
static void handleIntPushNextPacket(oppcState *state, OPPC_INT_PUSH_NEXT_PACKET_T *msg);
static void handleIntPushObjectSrc(oppcState *state, OPPC_INT_PUSH_OBJECT_SRC_T *msg);
static void handleIntPushNextPacketSrc(oppcState *state, OPPC_INT_PUSH_NEXT_PACKET_SRC_T *msg);
static void handleIntPullVCard(oppcState *state);
static void handleIntPullNextVCard(oppcState *state);

/* GOEP Library Message Handlers */
static void handleGoepInitCfm(oppcState *state, const GOEP_INIT_CFM_T *msg);
static void handleGoepChannelInd(oppcState *state, const GOEP_CHANNEL_IND_T *msg);
static void handleGoepConnectCfm(oppcState *state, GOEP_CONNECT_CFM_T *msg);
static void handleGoepAuthRequestInd(oppcState *state, GOEP_AUTH_REQUEST_IND_T *msg);
static void handleGoepDisconnectInd(oppcState *state, GOEP_DISCONNECT_IND_T *msg);
static void handleGoepPutCompleteInd(oppcState *state, GOEP_LOCAL_PUT_COMPLETE_IND_T *msg);
static void handleGoepPutMoreDateReq(oppcState *state);
static void handleGoepGetStartInd(oppcState *state, GOEP_LOCAL_GET_START_IND_T *msg);
static void handleGoepGetDataInd(oppcState *state, GOEP_LOCAL_GET_DATA_IND_T *msg);
static void handleGoepGetCompleteInd(oppcState *state, GOEP_LOCAL_GET_COMPLETE_IND_T *msg);

void oppcHandler(Task task, MessageId id, Message message)
{
	/* Get task control block */
	oppcState *state = (oppcState*)task;

    switch(id)
    {
        /* Internal FTP Client messages */
    case OPPC_INT_CONNECT:
        handleIntConnect(state, (OPPC_INT_CONNECT_T*)message);
        break;
	case OPPC_INT_AUTH_RESP:
		handleIntAuthResp(state, (OPPC_INT_AUTH_RESP_T *)message);
		break;
    case OPPC_INT_DISCONNECT:
        handleIntDisconnect(state);
        break;
		
    case OPPC_INT_ABORT:
        handleIntAbort(state, (OPPC_INT_ABORT_T*)message);
        break;
    case OPPC_INT_PUSH_OBJECT:
        handleIntPushObject(state, (OPPC_INT_PUSH_OBJECT_T*)message);
        break;
    case OPPC_INT_PUSH_NEXT_PACKET:
        handleIntPushNextPacket(state, (OPPC_INT_PUSH_NEXT_PACKET_T*)message);
        break;
    case OPPC_INT_PUSH_OBJECT_SRC:
        handleIntPushObjectSrc(state, (OPPC_INT_PUSH_OBJECT_SRC_T*)message);
        break;
    case OPPC_INT_PUSH_NEXT_PACKET_SRC:
        handleIntPushNextPacketSrc(state, (OPPC_INT_PUSH_NEXT_PACKET_SRC_T*)message);
        break;
	case OPPC_INT_PULL_VCARD:
        handleIntPullVCard(state);
		break;
	case OPPC_INT_PULL_NEXT_VCARD:
        handleIntPullNextVCard(state);
		break;
        
        /* Messages from the GOEP Library */
	case GOEP_INIT_CFM:
        handleGoepInitCfm(state, (GOEP_INIT_CFM_T*)message);
        break;
	case GOEP_CHANNEL_IND:
		handleGoepChannelInd(state, (GOEP_CHANNEL_IND_T*)message);
		break;
    case GOEP_CONNECT_CFM:
        handleGoepConnectCfm(state, (GOEP_CONNECT_CFM_T*)message);
        break;
	case GOEP_AUTH_REQUEST_IND:
		handleGoepAuthRequestInd(state, (GOEP_AUTH_REQUEST_IND_T *)message);		
		break;
    case GOEP_DISCONNECT_IND:
        handleGoepDisconnectInd(state, (GOEP_DISCONNECT_IND_T*)message);
        break;
    case GOEP_LOCAL_PUT_COMPLETE_IND:
        handleGoepPutCompleteInd(state, (GOEP_LOCAL_PUT_COMPLETE_IND_T*)message);
        break;
    case GOEP_LOCAL_PUT_DATA_REQUEST_IND:
        handleGoepPutMoreDateReq(state);
        break;
    case GOEP_LOCAL_GET_START_IND:
        handleGoepGetStartInd(state, (GOEP_LOCAL_GET_START_IND_T*)message);
        break;
    case GOEP_LOCAL_GET_DATA_IND:
        handleGoepGetDataInd(state, (GOEP_LOCAL_GET_DATA_IND_T*)message);
        break;
    case GOEP_LOCAL_GET_COMPLETE_IND:
        handleGoepGetCompleteInd(state, (GOEP_LOCAL_GET_COMPLETE_IND_T*)message);
        break;
	default:
		PRINT(("OPPC Unhandled Message : %d , 0x%0X\n",id,id) ); 
		break;
	}
}

/* Internal Message Handlers */

static void handleIntConnect(oppcState *state, const OPPC_INT_CONNECT_T *msg)
{
    PRINT(("handleIntConnect\n"));
	
	/* Store requested session parameters */
    state->packetSize = msg->maxPacketSize;
   	state->bdAddr = msg->bdAddr;
	
    state->currCom= oppc_com_Connect;
        
	if (state->handle)
	{ /* Goep library already initialised for this client */
		if (state->rfcChan == 0)
			GoepGetChannel(state->handle);
		else /* Already have a RFCOMM channel, send connect request */
			GoepConnect(state->handle, &state->bdAddr, state->rfcChan, state->packetSize , 0, NULL);
	}
	else
	{
	    /* Initialise GOEP */
    	GoepInit(&state->task, goep_Client, goep_OBEX);
	}
}

static void handleIntAuthResp(oppcState *state, OPPC_INT_AUTH_RESP_T *msg)
{
    PRINT(("handleIntAuthResp\n"));
	
	if (state->currCom!= oppc_com_Connect)
    { /* Not idle, Send Error Message */
		oppcMsgSendConnectCfm(state, oppc_wrong_state, 0);
    }
    else
    {
		GoepConnectAuthResponse(state->handle, msg->digest, msg->size_userid, msg->userid, msg->nonce);
	}
}

static void handleIntDisconnect(oppcState *state)
{
    PRINT(("handleIntDisconnect\n"));
    
    if (state->currCom!= oppc_com_None)
    { /* Not idle, Send Error Message */
		oppcMsgSendDisConnectCfm(state, oppc_not_idle);
    }
    else
    {        
        /* Send disconnect */
        GoepDisconnect(state->handle);
        /* Set command to close session */
        state->currCom= oppc_com_Disconnect;
    }
}

static void handleIntAbort(oppcState *state, OPPC_INT_ABORT_T *msg)
{
    PRINT(("handleIntAbort\n"));
    
	GoepAbort(state->handle);
}

static void handleIntPushObject(oppcState *state, OPPC_INT_PUSH_OBJECT_T *msg)
{
    PRINT(("handleIntPushObject\n"));
    
    if (state->currCom!= oppc_com_None)
    { /* Not idle, Send Error Message */
		oppcMsgPushCompleteInd(state, oppc_not_idle);
    }
    else
    {        
        GoepLocalPutFirstType(state->handle, msg->nameLen, msg->name, msg->typeLen, msg->type, 
                            msg->length, msg->packet, msg->totalLen, msg->onlyPacket);
        state->currCom= oppc_com_PushObject;
    }
}

static void handleIntPushNextPacket(oppcState *state, OPPC_INT_PUSH_NEXT_PACKET_T *msg)
{
    PRINT(("handleIntPushNextPacket\n"));
    
    if (state->currCom!= oppc_com_PushObject)
    { /* Not Pushing, Send Error Message */
		oppcMsgPushCompleteInd(state, oppc_wrong_command);
    }
    else
    {        
        GoepLocalPutNextPacket(state->handle, msg->length, msg->packet, msg->lastPacket);
        state->currCom= oppc_com_PushObject;
    }
}

static void handleIntPushObjectSrc(oppcState *state, OPPC_INT_PUSH_OBJECT_SRC_T *msg)
{
    PRINT(("handleIntPushObjectSrc\n"));
    
    if (state->currCom!= oppc_com_None)
    { /* Not idle, Send Error Message */
		oppcMsgPushCompleteInd(state, oppc_not_idle);
    }
    else
    {        
        GoepLocalPutFirstTypeSource(state->handle, msg->nameLen, msg->name, msg->typeLen, msg->type, 
                            msg->length, msg->src, msg->totalLen, msg->onlyPacket);
        state->currCom= oppc_com_PushObject;
    }
}

static void handleIntPushNextPacketSrc(oppcState *state, OPPC_INT_PUSH_NEXT_PACKET_SRC_T *msg)
{
    PRINT(("handleIntPushNextPacketSrc\n"));
    
    if (state->currCom!= oppc_com_PushObject)
    { /* Not Pushing, Send Error Message */
		oppcMsgPushCompleteInd(state, oppc_wrong_command);
    }
    else
    {        
        GoepLocalPutNextPacketSource(state->handle, msg->length, msg->src, msg->lastPacket);
        state->currCom= oppc_com_PushObject;
    }
}

static void handleIntPullVCard(oppcState *state)
{
    PRINT(("handleIntPullVCard\n"));
    
    if (state->currCom!= oppc_com_None)
    { /* Not idle, Send Error Message */
		oppcMsgPullBCCompleteInd(state, oppc_not_idle);
    }
    else
    {        
		GoepLocalGetFirstPacket(state->handle, 0, NULL, sizeof(vcardType), &vcardType[0]);

        state->currCom= oppc_com_PullVCard;
    }
}

static void handleIntPullNextVCard(oppcState *state)
{
    PRINT(("handleIntPullNextVCard\n"));
    
    if (state->currCom!= oppc_com_PullVCard)
    { /* Not idle, Send Error Message */
		oppcMsgPullBCCompleteInd(state, oppc_wrong_command);
    }
    else
    {        
		GoepLocalGetAck(state->handle);
    }
}




/* GOEP Library Message Handlers */
static void handleGoepInitCfm(oppcState *state, const GOEP_INIT_CFM_T *msg)
{
    PRINT(("handleGoepInitCfm\n"));
    
    if (msg->status != goep_success)
    { /* Couldn't start registration, clean up */
		PRINT(("     GOEP Status %d\n", msg->status));
		oppcMsgSendConnectCfm(state, oppc_failure, 0);
    }
    else
    { /* Registered with GOEP, start connection */
        state->handle = msg->goep;
        /* Get channel */
		if (state->rfcChan == 0)
			GoepGetChannel(state->handle);
		else /* Already have a RFCOMM channel, send connect request */
			GoepConnect(state->handle, &state->bdAddr, state->rfcChan, state->packetSize , 0, NULL);
    }
}

static void handleGoepChannelInd(oppcState *state, const GOEP_CHANNEL_IND_T *msg)
{
    PRINT(("handleGoepChannelInd\n"));
    
    if (msg->status != goep_success)
    { /* Couldn't start registration, clean up */
		PRINT(("     GOEP Status %d\n", msg->status));
		oppcMsgSendConnectCfm(state, oppc_failure, 0);
    }
    else
    { /* Registered with GOEP, start connection */
		state->rfcChan = msg->channel;
        /* Send Connect Request */
        GoepConnect(state->handle, &state->bdAddr, state->rfcChan, state->packetSize ,0, NULL);
    }
}

static void handleGoepConnectCfm(oppcState *state, GOEP_CONNECT_CFM_T *msg)
{
	oppc_lib_status status = oppc_success;
	uint16 packetSize = 0;
	
    PRINT(("handleGoepConnectCfm\n"));
	
	/* Null bdAddr pointer since we no longer have a need for it */
    BdaddrSetZero(&state->bdAddr);

    if (msg->status != goep_success)
    { /* Couldn't connect */
		PRINT(("     GOEP Status %d\n", msg->status));
        status = convert_error(msg->status);
    }
    else
    { /* GOEP has connected with the server, inform App. */
        /* Store connection data */
        state->packetSize = msg->maxPacketLen;
        
        packetSize = state->packetSize;
    }
	
	state->currCom= oppc_com_None;
	
	oppcMsgSendConnectCfm(state, status, packetSize);
}

static void handleGoepAuthRequestInd(oppcState *state, GOEP_AUTH_REQUEST_IND_T *msg)
{
	OPPC_AUTH_REQUEST_IND_T *message = (OPPC_AUTH_REQUEST_IND_T *)
										PanicUnlessMalloc(sizeof(OPPC_AUTH_REQUEST_IND_T) + msg->size_realm);
    PRINT(("handleGoepAuthRequestInd\n"));
	
	memcpy(message, msg, sizeof(OPPC_AUTH_REQUEST_IND_T) + msg->size_realm);
	message->oppc = state;
	
    MessageSend(state->theAppTask, OPPC_AUTH_REQUEST_IND, message);
}

static void handleGoepDisconnectInd(oppcState *state, GOEP_DISCONNECT_IND_T *msg)
{
	oppc_lib_status status = oppc_success;
    PRINT(("handleGoepDisconnectInd\n"));
    
    if (msg->status != goep_success)
    { /* Send error message */
        
        status= convert_error(msg->status);
    }
	state->currCom= oppc_com_None;
	
	oppcMsgSendDisConnectCfm(state, status);
}

static void handleGoepPutCompleteInd(oppcState *state, GOEP_LOCAL_PUT_COMPLETE_IND_T *msg)
{
    if (state->currCom == oppc_com_PushObject)
    {
		oppc_lib_status status = oppc_success;
    
        PRINT(("handleGoepPutCompleteInd\n"));
		
        if (msg->status != goep_success)
        {/* Pass on Error Message */
            status= oppc_failure;
        }
		
		oppcMsgPushCompleteInd(state, status);
            
        state->currCom= oppc_com_None; /* return idle */
    }
}

static void handleGoepPutMoreDateReq(oppcState *state)
{
    if (state->currCom == oppc_com_PushObject)
    {
        MAKE_OPPC_MESSAGE(OPPC_PUSH_MORE_DATA_IND);
    
        PRINT(("handleGoepPutCompleteInd\n"));
		
		message->oppc = state;
    
        MessageSend(state->theAppTask, OPPC_PUSH_MORE_DATA_IND, message);
    }
}

static void handleGoepGetStartInd(oppcState *state, GOEP_LOCAL_GET_START_IND_T *msg)
{
    if (state->currCom == oppc_com_PullVCard)
    {
        MAKE_OPPC_MESSAGE(OPPC_PULL_BC_START_IND);
    
        PRINT(("handleGoepGetStartInd\n"));
 
		message->oppc = state;
        message->src = msg->src;
	    /* Total object size if known */
        message->objectSize = msg->totalLength;
    	/* Type of object if known */
    	message->nameLen = msg->nameLength;
    	message->nameOffset = msg->nameOffset;
        message->packetLen = msg->dataLength;
        message->packetOffset = msg->dataOffset;
        message->moreData = msg->moreData;

        MessageSend(state->theAppTask, OPPC_PULL_BC_START_IND, message);
    }
}

static void handleGoepGetDataInd(oppcState *state, GOEP_LOCAL_GET_DATA_IND_T *msg)
{
    PRINT(("handleGoepGetDataInd : "));
    
    if (state->currCom == oppc_com_PullVCard)
    {
		MAKE_OPPC_MESSAGE(OPPC_PULL_BC_DATA_IND);
		
		message->oppc = state;
        message->src = msg->src;
        message->packetLen = msg->dataLength;
        message->packetOffset = msg->dataOffset;
        message->moreData = msg->moreData;
            
        MessageSend(state->theAppTask, OPPC_PULL_BC_DATA_IND, message);
    }
}

static void handleGoepGetCompleteInd(oppcState *state, GOEP_LOCAL_GET_COMPLETE_IND_T *msg)
{
    PRINT(("handleGoepGetCompleteInd : "));
    
    if (state->currCom == oppc_com_PullVCard)
	{
		oppc_lib_status status = oppc_success;
		if (msg->status != goep_success)
		{/* Pass on Error Message */
			switch (msg->status)
			{
			case goep_get_badrequest:
        		status = oppc_badrequest;
				break;
			case goep_get_forbidden:
        		status = oppc_forbidden;
				break;
			case goep_get_notfound:
        		status = oppc_notfound;
				break;
				default:
        		status = oppc_failure;
				break;
			};
		}
            
		oppcMsgPullBCCompleteInd(state, status);

        state->currCom= oppc_com_None; /* return idle */
    }
}

/* Error value conversion */
static oppc_lib_status convert_error(goep_lib_status status)
{
	oppc_lib_status ret = oppc_success;
	
	switch (status)
	{
	case goep_success:
		ret = oppc_success;
		break;
	case goep_host_abort:
	case goep_local_abort: /* Deliberate Fallthrough */
		ret = oppc_aborted;
		break;
	default:
		ret = oppc_failure;
		break;
	}
	
	return ret;
}
