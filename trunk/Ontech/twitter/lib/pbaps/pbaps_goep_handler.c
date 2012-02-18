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
#include <pbap_common.h>
#include <goep.h>
#include <string.h>

#include <app/bluestack/types.h>
#include <app/bluestack/bluetooth.h>
#include <app/bluestack/sdc_prim.h>
#include <service.h>
#include <connection.h>

#include <sdp_parse.h>
#include "pbaps.h"
#include "pbaps_private.h"

static const uint8 serviceRecordPBAP[] =
    {			
        /* Service class ID list */
        0x09,0x00,0x01,		/* AttrID , ServiceClassIDList */
        0x35,0x03,			/* 3 bytes in total DataElSeq */
        0x19,((goep_PBAP_PSE>>8)&0xFF),(goep_PBAP_PSE&0xFF),
							/* 2 byte UUID, Service class = Phonebook Access Server */

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
        0x19,((goep_PBAP_PSE>>8)&0xFF),(goep_PBAP_PSE&0xFF),
							/* 2 byte UUID, Service class = OBEXPhonebookAccess */
        0x09,0x01,0x00,		/* 2 byte uint, version = 100 */

		/* service name */
        0x09,0x01,0x00,		/* AttrId - Service Name */
        0x25,0x10,			/* 16 byte string - OBEX PBAP Server */
        'O','B','E','X',' ','P','B','A','P',' ','S','e','r','v','e','r',
        
	    /* Supported Repositories */    
	    0x09, ((PBAP_REPOS>>8)&0xFF),(PBAP_REPOS&0xFF),
							/* AttrId - Supported Repositories */
    	0x08, 0x00    		/* 1 byte UINT - Passed in by app. */
    };

static void handleGoepInitCfm(pbapsState *state, GOEP_INIT_CFM_T *msg);
static void handleGoepChannelInd(pbapsState *state, GOEP_CHANNEL_IND_T *msg);

static void handleGoepConnectInd(pbapsState *state, GOEP_CONNECT_IND_T *msg);
static void handleGoepConnectCfm(pbapsState *state, GOEP_CONNECT_CFM_T *msg);
static void handleGoepAuthResultInd(pbapsState *state, GOEP_AUTH_RESULT_IND_T *msg);
static void handleGoepDisconnectInd(pbapsState *state, GOEP_DISCONNECT_IND_T *msg);

static void handleGoepSetPathInd(pbapsState *state, GOEP_SET_PATH_IND_T *msg);

static void handleGoepRemGetStartHdrsInd(pbapsState *state, GOEP_REMOTE_GET_START_HDRS_IND_T *msg);
static void handleGoepRemGetStartInd(pbapsState *state, GOEP_REMOTE_GET_START_IND_T *msg);
static void handleGoepRemGetDataReqInd(pbapsState *state, GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND_T *msg);
static void handleGoepRemGetCompleteInd(pbapsState *state, GOEP_REMOTE_GET_COMPLETE_IND_T *msg);

static void handleGoepGetAppHeadersInd(pbapsState *state, GOEP_GET_APP_HEADERS_IND_T *msg);

static void handleUnsupportedGOEPMessage(pbapsState *state);

/* Connection Library message handlers */
static void handleSDPRegisterCfm(pbapsState *state, CL_SDP_REGISTER_CFM_T *msg);


static pbaps_running_command findMimeType(pbapsState *state, Source src, uint16 offset, uint16 length);

/****************************************************************************
NAME	
    pbabsGoepHandler

DESCRIPTION
    Handler for messages received by the PBABS Task from GOEP.
*/
void pbapsGoepHandler(pbapsState *state, MessageId id, Message message)
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

static void handleUnsupportedGOEPMessage(pbapsState *state)
{
	PBAPS_DEBUG(("handleUnsupportedGOEPMessage\n"));
	GoepRemotePutResponse(state->handle, goep_svr_resp_BadRequest);
}


		
/* Connection Library message handlers */
static void handleSDPRegisterCfm(pbapsState *state, CL_SDP_REGISTER_CFM_T *msg)
{
	PRINT(("handleSDPRegisterCfm\n"));
	
	if (msg->status == success)
	{
		state->sdpHandle = msg->service_handle;
		pbapsMsgSendInitCfm((PBAPS*)state, pbaps_success);
	}
	else
	{
		pbapsMsgSendInitCfm(state, pbaps_sdp_failure);
		free(state);
	}
}

static void handleGoepInitCfm(pbapsState *state, GOEP_INIT_CFM_T *msg)
{
	PRINT(("handleGoepInitCfm\n"));
	
	if (msg->status == goep_success)
	{
        state->handle  = msg->goep;
        state->currCom = pbaps_com_None;
		
		GoepGetChannel(state->handle);
	}
	else
	{
		pbapsMsgSendInitCfm(state, pbaps_failure);
		free(state);
	}
}

static void handleGoepChannelInd(pbapsState *state, GOEP_CHANNEL_IND_T *msg)
{
	PRINT(("handleGoepChannelInd\n"));
	
	if (msg->status == goep_success)
	{
		uint8* sdp;
				
		state->rfcChan = msg->channel;
		
		sdp = (uint8 *)PanicUnlessMalloc(sizeof(serviceRecordPBAP));
		memmove(sdp, serviceRecordPBAP, sizeof(serviceRecordPBAP));

		if (!SdpParseInsertRfcommServerChannel(sizeof(serviceRecordPBAP), sdp, state->rfcChan))
		{
			pbapsMsgSendInitCfm(state, pbaps_sdp_failure);
			free(sdp);
			free(state);
		}
		else
		{
			if (!SdpParseInsertPbapRepos(sizeof(serviceRecordPBAP), sdp, (uint32)state->repos))
			{
				pbapsMsgSendInitCfm(state, pbaps_sdp_failure);
				free(sdp);
				free(state);
			}
			else
			{
				/* Send the service record to the connection lib to be registered with BlueStack */
				ConnectionRegisterServiceRecord(&state->task, sizeof(serviceRecordPBAP),sdp);
			}
		}
	}
	else
	{
		pbapsMsgSendInitCfm(state, pbaps_failure);
		free(state);
	}
}

static void handleGoepConnectInd(pbapsState *state, GOEP_CONNECT_IND_T *msg)
{
	uint16 len;
	const uint8 *targ = PbapcoGetTargetString(&len);
	
	PRINT(("handleGoepConnectInd\n"));

	if ((msg->size_target>0) && (memcmp(targ, &msg->target[0], len)==0))
	{
	   	MAKE_PBAPS_MESSAGE(PBAPS_CONNECT_IND);
		
		message->pbaps = (PBAPS *)state;
		message->bd_addr = msg->bd_addr;
		message->maxPacketLen = msg->maxPacketLen;
        
   		MessageSend(state->theAppTask, PBAPS_CONNECT_IND, message);
			
		state->currCom = pbaps_com_Connecting;
	}
	else
	{
		/* Reject the connection attempt due to an invalid target header */
		state->currCom = pbaps_com_None;
		GoepConnectResponse(state->handle, goep_svr_resp_BadRequest, 0);
	}
}

static void handleGoepConnectCfm(pbapsState *state, GOEP_CONNECT_CFM_T *msg)
{
    PRINT(("handleGoepConnectCfm\n"));
    
    if (msg->status != goep_success)
    { /* Couldn't connect */
		pbapsMsgSendConnectCfm(state, pbaps_failure, 0);
		state->currCom= pbaps_com_None;
    }
    else
    {
        /* Return to Idle state */
        state->currCom= pbaps_com_None;
		
		pbapsMsgSendConnectCfm(state, pbaps_success, msg->maxPacketLen);
    }
}

static void handleGoepAuthResultInd(pbapsState *state, GOEP_AUTH_RESULT_IND_T *msg)
{
	PBAPS_AUTH_RESULT_IND_T *message = (PBAPS_AUTH_RESULT_IND_T *)
										PanicUnlessMalloc(sizeof(PBAPS_AUTH_RESULT_IND_T) + msg->size_userid);
    PRINT(("handleGoepAuthResultInd\n"));
	
	memmove(message, msg, sizeof(PBAPS_AUTH_RESULT_IND_T) + msg->size_userid);
	message->pbaps = state;
	
    MessageSend(state->theAppTask, PBAPS_AUTH_RESULT_IND, message);
}

static void handleGoepDisconnectInd(pbapsState *state, GOEP_DISCONNECT_IND_T *msg)
{
    PRINT(("handleGoepDisconnectInd\n"));
    PBAPS_SEND_IND(state, PBAPS_DISCONNECT_IND);
	state->currCom= pbaps_com_None;
}

static void handleGoepSetPathInd(pbapsState *state, GOEP_SET_PATH_IND_T *msg)
{
    PRINT(("\nhandleGoepRemSetPathInd\n"));

	if (state->currCom != pbaps_com_None)
	{ /* Error */
		PRINT(("       Error\n"));
		GoepRemoteSetPathResponse(state->handle, goep_svr_resp_BadRequest);
	}
	else
	{
		state->currCom = pbaps_com_SetPhonebook;
		
		/* decide on type of set path */
		if (msg->nameLength > 0)
		{ /* goto sub folder */
			const uint8 *s = SourceMap(msg->src);
			pbap_phone_book book = pbap_b_unknown;
			pbap_phone_repository rep = pbap_r_unknown;
			
			book = PbapcoGetBookIDFromName(&s[msg->nameOffset], msg->nameLength);
			if (book == pbap_b_unknown)
				rep = PbapcoGetRepositoryIDFromName(&s[msg->nameOffset], msg->nameLength);
			
			if (book != pbap_b_unknown)
			{ /* Change to a phonebook folder */
			   	MAKE_PBAPS_MESSAGE(PBAPS_SET_PB_BOOK_IND);
		
				message->pbaps = (PBAPS *)state;
				message->book = book;
        
		   		MessageSend(state->theAppTask, PBAPS_SET_PB_BOOK_IND, message);
				PRINT(("    Goto Phonebook.  ID = %d\n", book));
			}
			else if (rep != pbap_r_unknown)
			{ /* Change to a repository folder */
				uint16 repMask = 1<<(rep - pbap_local);
				
				PRINT(("    Goto Repository. ID = %d\n", rep));
				if ((state->repos & repMask) == repMask)
				{
				   	MAKE_PBAPS_MESSAGE(PBAPS_SET_PB_REPOSITORY_IND);
		
					message->pbaps = (PBAPS *)state;
					message->repository = rep;
        
		   			MessageSend(state->theAppTask, PBAPS_SET_PB_REPOSITORY_IND, message);
				}
				else
				{
					state->currCom = pbaps_com_None;
					GoepRemoteSetPathResponse(state->handle, goep_svr_resp_NotFound);
					PRINT(("        No Repository\n"));
				}
#ifdef DEBUG_PRINT_ENABLED
				{
					uint16 c;
	
					PRINT(("         "));
					for (c=0; c<msg->nameLength; c++)
					{
						if (s[c+msg->nameOffset]>=' ')
						{
							PRINT(("%c", s[c+msg->nameOffset]));
						}
						else
						{
							PRINT(("."));
						}
					}
					PRINT(("\n"));
				}
#endif /* DEBUG_PRINT_ENABLED */
			}
			else
			{ /* Completely invalid */
				state->currCom = pbaps_com_None;
				GoepRemoteSetPathResponse(state->handle, goep_svr_resp_NotFound);
				PRINT(("    Goto Subfolder but not found\n"));
			}
		}
		else if (msg->flags == (GOEP_PATH_PARENT | GOEP_PATH_NOCREATE))
		{ /* goto parent folder */
			PBAPS_SEND_IND(state, PBAPS_SET_PB_PARENT_IND);
			PRINT(("    Goto Parent\n"));
		}
		else
		{ /* goto root folder */
			PBAPS_SEND_IND(state, PBAPS_SET_PB_ROOT_IND);
			PRINT(("    Goto Root\n"));
		}
	}
	GoepPacketComplete(state->handle);
}

static void handleGoepRemGetStartHdrsInd(pbapsState *state, GOEP_REMOTE_GET_START_HDRS_IND_T *msg)
{
    PRINT(("handleGoepRemGetStartHdrsInd\n"));

	if (state->currCom != pbaps_com_None)
	{ /* Error */
		PRINT(("       Error\n"));
		GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0, 0, NULL, 0, NULL, 0, NULL, TRUE);
	}
	else
	{
		/* Find Type */
		switch (findMimeType(state, msg->src, msg->typeOffset, msg->typeLength))
		{
		case pbaps_com_PullvCardList:
			pbapsExtractvCardListingParameters(state, msg);
			state->currCom = pbaps_com_PullvCardList;
			break;
		case pbaps_com_PullvCard:
			if (msg->nameLength > 0)
			{
				pbapsExtractvCardEntryParameters(state, msg);
				state->currCom = pbaps_com_PullvCard;
			}
			else
			{ /* There must be a name included */
				GoepRemoteGetResponse(state->handle, goep_svr_resp_PreConFail, 0, 0, NULL, 0, NULL, 0, NULL, TRUE);
			}
			break;
		case pbaps_com_PullPhonebook:
			PRINT(("    pbaps_com_PullPhonebook\n"));
			
			if (pbapsExtractPhonebookParameters(state, msg))
				state->currCom = pbaps_com_PullPhonebook;
			else
				GoepRemoteGetResponse(state->handle, goep_svr_resp_PreConFail, 0, 0, NULL, 0, NULL, 0, NULL, TRUE);
			break;
		default:
			PRINT(("     Unknown Minetype\n"));
			GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0, 0, NULL, 0, NULL, 0, NULL, TRUE);
			break;
		}
	}
	GoepPacketComplete(state->handle);
}

static void handleGoepRemGetStartInd(pbapsState *state, GOEP_REMOTE_GET_START_IND_T *msg)
{
    PRINT(("handleGoepRemGetStartInd\n"));

	if (state->currCom != pbaps_com_None)
	{ /* Error */
		PRINT(("       Error\n"));
		GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0, 0, NULL, 0, NULL, 0, NULL, TRUE);
	}
	else
	{
		/* Find Type */
		switch (findMimeType(state, msg->src, msg->typeOffset, msg->typeLength))
		{
		case pbaps_com_PullvCardList:
			{
				const uint8 *src = SourceMap(msg->src);
				const uint8 *s;
				pbaps_lib_status status = pbaps_success;
				pbap_phone_book pbook = pbap_b_unknown;
	
				PRINT(("    pbaps_com_PullvCardList\n"));
				
				if ((msg->nameLength != 0) && (msg->nameOffset!=0))
				{ /* Packet contains a folder name */
					s = &src[msg->nameOffset];
		
					pbook = PbapcoGetBookIDFromName(s, msg->nameLength);
					if (pbook == pbap_b_unknown)
					{ /* Supplied folder is invalid - send error to application */
						status = pbaps_pull_vcl_invalid_folder;
					}
				}
				
				pbapsMsgSendGetvCardListStartInd(state, status, pbap_param_order_def, NULL, 0,
										pbook, pbap_param_srch_attr_def, 
										pbap_param_max_list_def, pbap_param_strt_offset_def);
				state->currCom = pbaps_com_PullvCardList;
				break;
			}
		case pbaps_com_PullvCard:
			PRINT(("    pbaps_com_PullvCard\n"));
			
			if (msg->nameLength > 0)
			{
				const uint8 *src = SourceMap(msg->src);
				uint16 entry = pbapsExtractvCardEntry(&src[msg->nameOffset+1]);
				pbapsMsgSendGetvCardEntryStartInd(state, entry, pbap_param_format_def, 0, 0);
					
				state->currCom = pbaps_com_PullvCard;
			}
			else
			{ /* There must be a name included */
				GoepRemoteGetResponse(state->handle, goep_svr_resp_PreConFail, 0, 0, NULL, 0, NULL, 0, NULL, TRUE);
			}
			break;
		case pbaps_com_PullPhonebook:
			PRINT(("    pbaps_com_PullPhonebook\n"));
			
			/* Send error since there must be a MaxListCount Parameter */
			GoepRemoteGetResponse(state->handle, goep_svr_resp_PreConFail, 0, 0, NULL, 0, NULL, 0, NULL, TRUE);
			break;
		default:
			PRINT(("     Unknown Mimetype\n"));
			GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0, 0, NULL, 0, NULL, 0, NULL, TRUE);
			break;
		}
	}
	GoepPacketComplete(state->handle);
}

static void handleGoepRemGetDataReqInd(pbapsState *state, GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND_T *msg)
{
    PRINT(("handleGoepRemGetDataReqInd\n"));

	/* Find Type */
	switch (state->currCom)
	{
	case pbaps_com_PullvCardList:
		PBAPS_SEND_IND(state,PBAPS_GET_VCARD_LIST_NEXT_IND);
		break;
	case pbaps_com_PullvCard:
		PBAPS_SEND_IND(state,PBAPS_GET_VCARD_ENTRY_NEXT_IND);
		break;
	case pbaps_com_PullPhonebook:
		PBAPS_SEND_IND(state,PBAPS_GET_PHONEBOOK_NEXT_IND);
		break;
	default:
		GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0, 0, NULL, 0, NULL, 0, NULL, TRUE);
		break;
	}
}

static void handleGoepRemGetCompleteInd(pbapsState *state, GOEP_REMOTE_GET_COMPLETE_IND_T *msg)
{
	pbaps_lib_status result;
	
    PRINT(("handleGoepRemGetCompleteInd\n"));

	switch (msg->status)
	{
	case goep_success:
		result = pbaps_success;
		break;
	case goep_host_abort:
		result = pbaps_remote_abort;
		break;
	default:
		result = pbaps_failure;
		break;
	}
	/* Find Type */
	switch (state->currCom)
	{
	case pbaps_com_PullvCardList:
		pbapsMsgSendGetvCardListCompleteInd(state, result);
		break;
	case pbaps_com_PullvCard:
		pbapsMsgSendGetvCardEntryCompleteInd(state, result);
		break;
	case pbaps_com_PullPhonebook:
		pbapsMsgSendGetPhonebookCompleteInd(state, result);
		break;
	default:
		PRINT(("       Error\n"));
		break;
	}
		
	state->currCom = pbaps_com_None;
}

static void handleGoepGetAppHeadersInd(pbapsState *state, GOEP_GET_APP_HEADERS_IND_T *msg)
{
	uint16 lenUsed = 0;
	
	PRINT(("handleGoepGetAppHeadersInd\n"));
	
	switch (state->currCom)
	{
	case pbaps_com_PullvCardList:
	case pbaps_com_PullPhonebook:
		lenUsed = pbapsAddApplicationHeaders(state, msg->sink);
		break;
	default:
		PBAPS_DEBUG(("PBAPS - Get App. Headers - Using a command that does not support app headers\n"));
		break;
	};
	
	GoepSendAppSpecificPacket(state->handle, lenUsed);
}

