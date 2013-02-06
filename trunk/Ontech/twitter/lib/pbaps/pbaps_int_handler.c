/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbaps_int_handler.h
    
DESCRIPTION
	PhoneBook Access Profile Server Library - handler functions for internal messages.

*/

#include <vm.h>
#include <print.h>
#include <pbap_common.h>
#include <goep.h>

#include "pbaps.h"
#include "pbaps_private.h"

static void sendSetPhonebookResult(pbapsState *state, pbaps_set_phonebook_result result);

static void handleIntConnectResponse(pbapsState *state, PBAPS_INT_CONNECTION_RESP_T *msg);
static void handleIntAuthChallenge(pbapsState *state, PBAPS_INT_AUTH_CHALLENGE_T *msg);

static void handleIntSetPBRootResponse(pbapsState *state, PBAPS_INT_SET_PB_ROOT_RESP_T *msg);
static void handleIntSetPBRepositoryResponse(pbapsState *state, PBAPS_INT_SET_PB_REPOSITORY_RESP_T *msg);
static void handleIntSetPBBookResponse(pbapsState *state, PBAPS_INT_SET_PB_BOOK_RESP_T *msg);
static void handleIntSetPBParentResponse(pbapsState *state, PBAPS_INT_SET_PB_PARENT_RESP_T *msg);

static void handleIntGetvCardListFirst(pbapsState *state, PBAPS_GET_VCARD_LIST_FIRST_T *msg);
static void handleIntGetvCardEntryFirst(pbapsState *state, PBAPS_GET_VCARD_ENTRY_FIRST_T *msg);
static void handleIntPhonebookFirst(pbapsState *state, PBAPS_GET_PHONEBOOK_FIRST_T *msg);

static void handleIntGetSendFirstSrc(pbapsState *state, PBAPS_GET_SEND_FIRST_SRC_T *msg);
static void handleIntGetSendNext(pbapsState *state, PBAPS_GET_SEND_NEXT_T *msg);
static void handleIntGetSendNextSrc(pbapsState *state, PBAPS_GET_SEND_NEXT_SRC_T *msg);

/****************************************************************************
NAME	
    pbapsIntHandler

DESCRIPTION
    Handler for messages received by the PBABS Task.
*/
void pbapsIntHandler(Task task, MessageId id, Message message)
{
	/* Get task control block */
	pbapsState *state = (pbapsState*)task;
	
	if ((id >= PBAPS_INT_CONNECTION_RESP) && (id <= PBAPS_INT_ENDOFLIST))
	{
		switch (id)
		{
		case PBAPS_INT_CONNECTION_RESP:
			handleIntConnectResponse(state, (PBAPS_INT_CONNECTION_RESP_T*)message);
			break;
		case PBAPS_INT_AUTH_CHALLENGE:
			handleIntAuthChallenge(state, (PBAPS_INT_AUTH_CHALLENGE_T*)message);
			break;
			
		case PBAPS_INT_SET_PB_ROOT_RESP:
			handleIntSetPBRootResponse(state, (PBAPS_INT_SET_PB_ROOT_RESP_T*)message);
			break;
		case PBAPS_INT_SET_PB_REPOSITORY_RESP:
			handleIntSetPBRepositoryResponse(state, (PBAPS_INT_SET_PB_REPOSITORY_RESP_T*)message);
			break;
		case PBAPS_INT_SET_PB_BOOK_RESP:
			handleIntSetPBBookResponse(state, (PBAPS_INT_SET_PB_BOOK_RESP_T*)message);
			break;
		case PBAPS_INT_SET_PB_PARENT_RESP:
			handleIntSetPBParentResponse(state, (PBAPS_INT_SET_PB_PARENT_RESP_T*)message);
			break;
			
		case PBAPS_GET_VCARD_LIST_FIRST:
			handleIntGetvCardListFirst(state, (PBAPS_GET_VCARD_LIST_FIRST_T*)message);
			break;
		case PBAPS_GET_VCARD_ENTRY_FIRST:
			handleIntGetvCardEntryFirst(state, (PBAPS_GET_VCARD_ENTRY_FIRST_T*)message);
			break;
		case PBAPS_GET_PHONEBOOK_FIRST:
			handleIntPhonebookFirst(state, (PBAPS_GET_PHONEBOOK_FIRST_T*)message);
			break;
			
		case PBAPS_GET_SEND_FIRST_SRC:
			handleIntGetSendFirstSrc(state, (PBAPS_GET_SEND_FIRST_SRC_T*)message);
			break;
		case PBAPS_GET_SEND_NEXT:
			handleIntGetSendNext(state, (PBAPS_GET_SEND_NEXT_T*)message);
			break;
		case PBAPS_GET_SEND_NEXT_SRC:
			handleIntGetSendNextSrc(state, (PBAPS_GET_SEND_NEXT_SRC_T*)message);
			break;
			
			default:
			PRINT(("PAPS Unhandled message : 0x%X\n",id));
			break;
		};
	}
	else
	{
		pbapsGoepHandler(state, id, message);
	}
}

static void sendSetPhonebookResult(pbapsState *state, pbaps_set_phonebook_result result)
{
	goep_svr_resp_codes res = goep_svr_resp_OK;

	switch (result)
	{
	case pbaps_spb_ok:
		res = goep_svr_resp_OK;
		break;
	case pbaps_spb_no_repository:
	case pbaps_spb_no_phonebook:
		res = goep_svr_resp_NotFound;
		break;
	case pbaps_spb_unauthorised:
		res = goep_svr_resp_Unauthorized;
		break;
	default:
		break;
	}
	
	GoepRemoteSetPathResponse(state->handle, res);
}

static void handleIntConnectResponse(pbapsState *state, PBAPS_INT_CONNECTION_RESP_T *msg)
{
    PRINT(("handleIntConnectResponse\n"));
    
    if (state->currCom!= pbaps_com_Connecting)
    { /* Not idle, Send Error Message */
		pbapsMsgSendConnectCfm(state, pbaps_wrong_state, 0);
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
	        state->currCom= pbaps_com_None;
		}
    }
}

static void handleIntAuthChallenge(pbapsState *state, PBAPS_INT_AUTH_CHALLENGE_T *msg)
{
    PRINT(("handleIntAuthChallenge\n"));
    
    if (state->currCom!= pbaps_com_Connecting)
    { /* Not idle, Send Error Message */
		pbapsMsgSendConnectCfm(state, pbaps_wrong_state, 0);
    }
    else
    {
		GoepConnectAuthChallenge(state->handle, msg->nonce, msg->options, msg->size_realm, msg->realm);
    }
}

static void handleIntSetPBRootResponse(pbapsState *state, PBAPS_INT_SET_PB_ROOT_RESP_T *msg)
{
    PRINT(("handleIntSetPBRootResponse\n"));
    
    if (state->currCom!= pbaps_com_SetPhonebook)
    {
		PRINT(("       Error\n"));
    }
    else
    {
		sendSetPhonebookResult(state, msg->result);

		state->currCom = pbaps_com_None;
    }
}

static void handleIntSetPBRepositoryResponse(pbapsState *state, PBAPS_INT_SET_PB_REPOSITORY_RESP_T *msg)
{
    PRINT(("handleIntSetPBRepositoryResponse\n"));
    
    if (state->currCom!= pbaps_com_SetPhonebook)
    {
		PRINT(("       Error\n"));
    }
    else
    {
		sendSetPhonebookResult(state, msg->result);

		state->currCom = pbaps_com_None;
    }
}

static void handleIntSetPBBookResponse(pbapsState *state, PBAPS_INT_SET_PB_BOOK_RESP_T *msg)
{
    PRINT(("handleIntSetPBBookResponse\n"));
    
    if (state->currCom!= pbaps_com_SetPhonebook)
    {
		PRINT(("       Error\n"));
    }
    else
    {
		sendSetPhonebookResult(state, msg->result);

		state->currCom = pbaps_com_None;
    }
}

static void handleIntSetPBParentResponse(pbapsState *state, PBAPS_INT_SET_PB_PARENT_RESP_T *msg)
{
    PRINT(("handleIntSetPBParentResponse\n"));
    
    if (state->currCom!= pbaps_com_SetPhonebook)
    {
		PRINT(("       Error\n"));
    }
    else
    {
		sendSetPhonebookResult(state, msg->result);

		state->currCom = pbaps_com_None;
    }
}

static void handleIntGetvCardListFirst(pbapsState *state, PBAPS_GET_VCARD_LIST_FIRST_T *msg)
{
    PRINT(("handleIntGetvCardListFirst\n"));
    
    if (state->currCom!= pbaps_com_PullvCardList)
    {
		PRINT(("       Error\n"));
		pbapsMsgSendGetvCardListCompleteInd(state, pbaps_wrong_state);
	}
    else
    {
		if (msg->result == pbaps_get_ok)
		{
			uint16 typeLen;
			const uint8* type = PbapcoGetvCardListingMimeType(&typeLen);
		
			if ((msg->pbook_size != 0) || (msg->new_missed !=0))
			{ /* Send using App. Params */
				/* Store Parameters */
				state->pbook_size = msg->pbook_size;
				state->new_missed = msg->new_missed;
				
				GoepRemoteGetResponseHeaders(state->handle, goep_svr_resp_OK, msg->totalLen,
						0, NULL, typeLen, type);
			}
			else
			{ /* Just send a reponse */
				GoepRemoteGetResponse(state->handle, goep_svr_resp_OK, msg->totalLen,
						0, NULL, typeLen, type, msg->size_packet, msg->packet, msg->onlyPacket);
			}
		}
		else
		{
			GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0,
					0, NULL, 0, NULL, 0, NULL, TRUE);
		}
    }
}

static void handleIntGetvCardEntryFirst(pbapsState *state, PBAPS_GET_VCARD_ENTRY_FIRST_T *msg)
{
    PRINT(("handleIntGetvCardEntryFirst\n"));
    
    if (state->currCom!= pbaps_com_PullvCard)
    {
		PRINT(("       Error\n"));
		pbapsMsgSendGetvCardEntryCompleteInd(state, pbaps_wrong_state);
	}
    else
    {
		if (msg->result == pbaps_get_ok)
		{
			uint16 typeLen;
			const uint8* type = PbapcoGetvCardMimeType(&typeLen);
		
			GoepRemoteGetResponse(state->handle, goep_svr_resp_OK, msg->totalLen,
						0, NULL, typeLen, type, msg->size_packet, msg->packet, msg->onlyPacket);
		}
		else
		{
			GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0,
					0, NULL, 0, NULL, 0, NULL, TRUE);
		}
    }
}

static void handleIntPhonebookFirst(pbapsState *state, PBAPS_GET_PHONEBOOK_FIRST_T *msg)
{
    PRINT(("handleIntPhonebookFirst\n"));
    
    if (state->currCom!= pbaps_com_PullPhonebook)
    {
		PRINT(("       Error\n"));
		pbapsMsgSendGetPhonebookCompleteInd(state, pbaps_wrong_state);
	}
    else
    {
		if (msg->result == pbaps_get_ok)
		{
			uint16 typeLen;
			const uint8* type = PbapcoGetPhonebookMimeType(&typeLen);
		
			if ((msg->pbook_size != 0) || (msg->new_missed !=0))
			{ 
				uint8 appHeader[8];
				uint16 hdrLen = 0;
				/* Send using App. Params */
				/* Store Parameters */
				state->pbook_size = msg->pbook_size;
				state->new_missed = msg->new_missed;

				hdrLen = pbapsBuildApplicationHeaders(state,appHeader);
				
				GoepRemoteGetResponseAppHeaders(state->handle, goep_svr_resp_OK, msg->totalLen,
						hdrLen, appHeader, typeLen, type);
			}
			else
			{ /* Just send a reponse */
				GoepRemoteGetResponse(state->handle, goep_svr_resp_OK, msg->totalLen,
						0, NULL, typeLen, type, msg->size_packet, msg->packet, msg->onlyPacket);
			}
		}
		else
		{
			GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0,
					0, NULL, 0, NULL, 0, NULL, TRUE);
		}
    }
}

static void handleIntGetSendFirstSrc(pbapsState *state, PBAPS_GET_SEND_FIRST_SRC_T *msg)
{
    PRINT(("handleIntGetSendFirstSrc\n"));

    if (state->currCom != msg->command)
    {
		PRINT(("       Error\n"));
		pbapsMsgSendGetCompleteInd(state, msg->command, pbaps_wrong_state);
    }
    else
    {
		if (msg->result == pbaps_get_ok)
		{
			uint16 typeLen;
			const uint8* type = PbapcoGetvCardListingMimeType(&typeLen);
		
			GoepRemoteGetResponseSource(state->handle, goep_svr_resp_OK, msg->totalLen,
					  0, NULL, typeLen, type, msg->size_packet, msg->src, msg->onlyPacket);
		}
		else
		{
			GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0,
					0, NULL, 0, NULL, 0, NULL, TRUE);
		}
    }
}

static void handleIntGetSendNext(pbapsState *state, PBAPS_GET_SEND_NEXT_T *msg)
{
    PRINT(("handleIntGetSendNext\n"));

    if (state->currCom != msg->command)
    {
		PRINT(("       Error\n"));
		pbapsMsgSendGetCompleteInd(state, msg->command, pbaps_wrong_state);
    }
    else
    {
		GoepRemoteGetResponsePkt(state->handle, msg->size_packet, msg->packet, msg->lastPacket);
    }
}

static void handleIntGetSendNextSrc(pbapsState *state, PBAPS_GET_SEND_NEXT_SRC_T *msg)
{
    PRINT(("handleIntGetSendNextSrc\n"));

    if (state->currCom != msg->command)
    {
		PRINT(("       Error\n"));
		pbapsMsgSendGetCompleteInd(state, msg->command, pbaps_wrong_state);
    }
    else
    {
		GoepRemoteGetResponsePktSource(state->handle, msg->size_packet, msg->src, msg->lastPacket);
    }
}

