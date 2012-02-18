/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbaps_goep_handler.h
    
DESCRIPTION
	PhoneBook Access Profile Server Library - handler functions for GOEP messages.

*/

#include <vm.h>
#include <print.h>
#include <goep.h>
#include <string.h>

#include <app/bluestack/types.h>
#include <app/bluestack/bluetooth.h>
#include <app/bluestack/sdc_prim.h>
#include <service.h>
#include <connection.h>

#include <sdp_parse.h>
#include "syncs.h"
#include "syncs_private.h"

static const uint8 serviceRecordSYNC[] =
    {			
        /* Service class ID list */
        0x09,0x00,0x01,		/* AttrID , ServiceClassIDList */
        0x35,0x03,			/* 3 bytes in total DataElSeq */
        0x19,((goep_SYNC_PSE>>8)&0xFF),(goep_SYNC_PSE&0xFF),
							/* 2 byte UUID, Service class = IrMCSync */

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
        0x19,((goep_SYNC_PSE>>8)&0xFF),(goep_SYNC_PSE&0xFF),
							/* 2 byte UUID, Service class = IrMCSync */
        0x09,0x01,0x00,		/* 2 byte uint, version = 100 */
#if 1
		/* service name */
        0x09,0x01,0x00,		/* AttrId - Service Name */
        0x25,21,			/* 16 byte string - OBEX PBAP Server */
        'O','B','E','X',' ','I','r','M','C',' ','S','y','n','c',' ','S','e','r','v','e','r',
#endif        
	    /* Supported Repositories */    
	    0x09, 0x03, 0x01,
							/* AttrId - Supported Repositories */
    	0x35, 0x04,    		/* 1 byte UINT - Passed in by app. */
    	0x08, 0x06,
		0x08, 0xff
    };

static void handleGoepInitCfm(syncsState *state, GOEP_INIT_CFM_T *msg);
static void handleGoepChannelInd(syncsState *state, GOEP_CHANNEL_IND_T *msg);

static void handleGoepConnectInd(syncsState *state, GOEP_CONNECT_IND_T *msg);
static void handleGoepConnectCfm(syncsState *state, GOEP_CONNECT_CFM_T *msg);
static void handleGoepAuthResultInd(syncsState *state, GOEP_AUTH_RESULT_IND_T *msg);
static void handleGoepDisconnectInd(syncsState *state, GOEP_DISCONNECT_IND_T *msg);

static void handleGoepSetPathInd(syncsState *state, GOEP_SET_PATH_IND_T *msg);

static void handleGoepRemGetStartHdrsInd(syncsState *state, GOEP_REMOTE_GET_START_HDRS_IND_T *msg);
static void handleGoepRemGetStartInd(syncsState *state, GOEP_REMOTE_GET_START_IND_T *msg);
static void handleGoepRemGetDataReqInd(syncsState *state, GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND_T *msg);
static void handleGoepRemGetCompleteInd(syncsState *state, GOEP_REMOTE_GET_COMPLETE_IND_T *msg);

static void handleGoepGetAppHeadersInd(syncsState *state, GOEP_GET_APP_HEADERS_IND_T *msg);

static void handleUnsupportedGOEPMessage(syncsState *state);

/* Connection Library message handlers */
static void handleSDPRegisterCfm(syncsState *state, CL_SDP_REGISTER_CFM_T *msg);


/****************************************************************************
NAME	
    pbabsGoepHandler

DESCRIPTION
    Handler for messages received by the PBABS Task from GOEP.
*/
void syncsGoepHandler(syncsState *state, MessageId id, Message message)
{
	switch (id)
	{
	case GOEP_INIT_CFM:
		handleGoepInitCfm(state, (GOEP_INIT_CFM_T*)message);
		break;
	case GOEP_CHANNEL_IND:
		handleGoepChannelInd(state, (GOEP_CHANNEL_IND_T*)message);
		break;
		
	case GOEP_CONNECT_IND:
        handleGoepConnectInd(state, (GOEP_CONNECT_IND_T*)message);
        break;
    case GOEP_CONNECT_CFM:
        handleGoepConnectCfm(state, (GOEP_CONNECT_CFM_T*)message);
        break;
	case GOEP_AUTH_RESULT_IND:
		handleGoepAuthResultInd(state, (GOEP_AUTH_RESULT_IND_T*)message);
		break;
    case GOEP_DISCONNECT_IND:
        handleGoepDisconnectInd(state, (GOEP_DISCONNECT_IND_T*)message);
        break;
	case GOEP_SET_PATH_IND:
		handleGoepSetPathInd(state, (GOEP_SET_PATH_IND_T*)message);
		break;
	case GOEP_REMOTE_GET_START_HDRS_IND:
		handleGoepRemGetStartHdrsInd(state, (GOEP_REMOTE_GET_START_HDRS_IND_T*)message);
		break;
	case GOEP_REMOTE_GET_START_IND:
		handleGoepRemGetStartInd(state, (GOEP_REMOTE_GET_START_IND_T *)message);
		break;
	case GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND:
		handleGoepRemGetDataReqInd(state, (GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND_T*)message);
		break;
	case GOEP_REMOTE_GET_COMPLETE_IND:
		handleGoepRemGetCompleteInd(state, (GOEP_REMOTE_GET_COMPLETE_IND_T *)message);
		break;
		
	case GOEP_GET_APP_HEADERS_IND:
		handleGoepGetAppHeadersInd(state, (GOEP_GET_APP_HEADERS_IND_T *)message);
		break;		
		
		/* Unsupported Messages from the GOEP library */
	case GOEP_DELETE_CFM:
	case GOEP_DELETE_IND:
	case GOEP_REMOTE_PUT_START_IND:
	case GOEP_REMOTE_PUT_DATA_IND:
	case GOEP_REMOTE_PUT_COMPLETE_IND:
		handleUnsupportedGOEPMessage(state);
		break;
	
		/* Messages from the connection library */
	case CL_SDP_REGISTER_CFM:
		handleSDPRegisterCfm(state, (CL_SDP_REGISTER_CFM_T*)message);
		break;
	default:
		PRINT(("PAPS - GOEP Unhandled message : 0x%X\n",id));
		break;
	};
}

#if 0
static pbaps_running_command findMimeType(pbapsState *state, Source src, uint16 offset, uint16 length)
{
	uint16 len;
	const uint8 *type = PbapcoGetvCardListingMimeType(&len);
	const uint8 *s = SourceMap(src) + offset;
	
	if ((len == length) && (memcmp(type, s, length)==0))
	{
		state->currCom = pbaps_com_PullvCardList;
	}
	else
	{
		type = PbapcoGetvCardMimeType(&len);
		if ((len == length) && (memcmp(type, s, length)==0))
		{
			state->currCom = pbaps_com_PullvCard;
		}
		else
		{
			type = PbapcoGetPhonebookMimeType(&len);
			if ((len == length) && (memcmp(type, s, length)==0))
			{
				state->currCom = pbaps_com_PullPhonebook;
			}
		}
	}
	
	return state->currCom;
}
#endif

static void handleUnsupportedGOEPMessage(syncsState *state)
{
	SYNCS_DEBUG(("handleUnsupportedGOEPMessage\n"));
	GoepRemotePutResponse(state->handle, goep_svr_resp_BadRequest);
}


		
/* Connection Library message handlers */
static void handleSDPRegisterCfm(syncsState *state, CL_SDP_REGISTER_CFM_T *msg)
{
	PRINT(("handleSDPRegisterCfm\n"));
	
	if (msg->status == success)
	{
		state->sdpHandle = msg->service_handle;
		syncsMsgSendInitCfm((SYNCS*)state, syncs_success);
	}
	else
	{
		syncsMsgSendInitCfm(state, syncs_sdp_failure);
		free(state);
	}
}

static void handleGoepInitCfm(syncsState *state, GOEP_INIT_CFM_T *msg)
{
	PRINT(("handleGoepInitCfm\n"));
	
	if (msg->status == goep_success)
	{
        state->handle  = msg->goep;
        state->currCom = syncs_com_None;
		
		GoepGetChannel(state->handle);
	}
	else
	{
		syncsMsgSendInitCfm(state, syncs_failure);
		free(state);
	}
}

static void handleGoepChannelInd(syncsState *state, GOEP_CHANNEL_IND_T *msg)
{
	PRINT(("handleGoepChannelInd\n"));
	
	if (msg->status == goep_success)
	{
		uint8* sdp;
				
		state->rfcChan = msg->channel;
		
		sdp = (uint8 *)PanicUnlessMalloc(sizeof(serviceRecordSYNC));
		memmove(sdp, serviceRecordSYNC, sizeof(serviceRecordSYNC));

		if (!SdpParseInsertRfcommServerChannel(sizeof(serviceRecordSYNC), sdp, state->rfcChan))
		{
			syncsMsgSendInitCfm(state, syncs_sdp_failure);
			free(sdp);
			free(state);
		}
		else
		{
/*			if (!SdpParseInsertSyncStore(sizeof(serviceRecordSYNC), sdp, state->stores))
			{
				syncsMsgSendInitCfm(state, syncs_sdp_failure);
				free(sdp);
				free(state);
			}
			else*/
			{
#ifdef SYNCS_LIBRARY_DEBUG
				PRINT(("serviceRecordSYNC %d\n",sizeof(serviceRecordSYNC)));
				{
					uint16 i;
					for(i=0;i<sizeof(serviceRecordSYNC);i++)
					{
						PRINT(("%x ",sdp[i]));
					}
					PRINT(("\n"));
				}
#endif
				/* Send the service record to the connection lib to be registered with BlueStack */
				ConnectionRegisterServiceRecord(&state->task, sizeof(serviceRecordSYNC),sdp);
			}
		}
	}
	else
	{
		syncsMsgSendInitCfm(state, syncs_failure);
		free(state);
	}
}

static void handleGoepConnectInd(syncsState *state, GOEP_CONNECT_IND_T *msg)
{
	const uint8 targ[] = {'I','R','M','C','-','S','Y','N','C'};
	
	PRINT(("handleGoepConnectInd\n"));

	if ((msg->size_target>0) && (memcmp(targ, &msg->target[0], 9)==0))
	{
	   	MAKE_SYNCS_MESSAGE(SYNCS_CONNECT_IND);
		
		message->syncs = (SYNCS *)state;
		message->bd_addr = msg->bd_addr;
		message->maxPacketLen = msg->maxPacketLen;
        
   		MessageSend(state->theAppTask, SYNCS_CONNECT_IND, message);
			
		state->currCom = syncs_com_Connecting;
	}
	else
	{
		/* Reject the connection attempt due to an invalid target header */
		state->currCom = syncs_com_None;
		GoepConnectResponse(state->handle, goep_svr_resp_BadRequest, 0);
	}
}

static void handleGoepConnectCfm(syncsState *state, GOEP_CONNECT_CFM_T *msg)
{
    PRINT(("handleGoepConnectCfm\n"));
    
    if (msg->status != goep_success)
    { /* Couldn't connect */
		syncsMsgSendConnectCfm(state, syncs_failure, 0);
		state->currCom= syncs_com_None;
    }
    else
    {
        /* Return to Idle state */
        state->currCom= syncs_com_None;
		
		syncsMsgSendConnectCfm(state, syncs_success, msg->maxPacketLen);
    }
}

static void handleGoepAuthResultInd(syncsState *state, GOEP_AUTH_RESULT_IND_T *msg)
{
	SYNCS_AUTH_RESULT_IND_T *message = (SYNCS_AUTH_RESULT_IND_T *)
										PanicUnlessMalloc(sizeof(SYNCS_AUTH_RESULT_IND_T) + msg->size_userid);
    PRINT(("handleGoepAuthResultInd\n"));
	
	memmove(message, msg, sizeof(SYNCS_AUTH_RESULT_IND_T) + msg->size_userid);
	message->syncs = state;
	
    MessageSend(state->theAppTask, SYNCS_AUTH_RESULT_IND, message);
}

static void handleGoepDisconnectInd(syncsState *state, GOEP_DISCONNECT_IND_T *msg)
{
    PRINT(("handleGoepDisconnectInd\n"));
    SYNCS_SEND_IND(state, SYNCS_DISCONNECT_IND);
	state->currCom= syncs_com_None;
}

static void handleGoepSetPathInd(syncsState *state, GOEP_SET_PATH_IND_T *msg)
{
    PRINT(("\nhandleGoepRemSetPathInd\n"));

	if (state->currCom != syncs_com_None)
	{ /* Error */
		PRINT(("       Error\n"));
		GoepRemoteSetPathResponse(state->handle, goep_svr_resp_BadRequest);
	}
	else
	{
		PRINT(("       SetPathInd\n"));

	}
	GoepPacketComplete(state->handle);
}

static void handleGoepRemGetStartHdrsInd(syncsState *state, GOEP_REMOTE_GET_START_HDRS_IND_T *msg)
{
    PRINT(("handleGoepRemGetStartHdrsInd\n"));

	if (state->currCom != syncs_com_None)
	{ /* Error */
		PRINT(("       Error\n"));
		GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0, 0, NULL, 0, NULL, 0, NULL, TRUE);
	}
	else
	{
		/* Find Type */
	}
	GoepPacketComplete(state->handle);
}

static void handleGoepRemGetStartInd(syncsState *state, GOEP_REMOTE_GET_START_IND_T *msg)
{
    PRINT(("handleGoepRemGetStartInd\n"));

	if (state->currCom != syncs_com_None)
	{ /* Error */
		PRINT(("       Error\n"));
		GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0, 0, NULL, 0, NULL, 0, NULL, TRUE);
	}
	else
	{
		/* Find Type */
	}
	GoepPacketComplete(state->handle);
}

static void handleGoepRemGetDataReqInd(syncsState *state, GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND_T *msg)
{
    PRINT(("handleGoepRemGetDataReqInd\n"));

	/* Find Type */
}

static void handleGoepRemGetCompleteInd(syncsState *state, GOEP_REMOTE_GET_COMPLETE_IND_T *msg)
{
	syncs_lib_status result;
	
    PRINT(("handleGoepRemGetCompleteInd\n"));

	switch (msg->status)
	{
	case goep_success:
		result = syncs_success;
		break;
	case goep_host_abort:
		result = syncs_remote_abort;
		break;
	default:
		result = syncs_failure;
		break;
	}
	/* Find Type */
		
	state->currCom = syncs_com_None;
}

static void handleGoepGetAppHeadersInd(syncsState *state, GOEP_GET_APP_HEADERS_IND_T *msg)
{
	uint16 lenUsed = 0;
	
	PRINT(("handleGoepGetAppHeadersInd\n"));
	
	GoepSendAppSpecificPacket(state->handle, lenUsed);
}

