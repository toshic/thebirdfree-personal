/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_metadata_transfer.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_battery_handler.h"
#include "avrcp_character_handler.h"
#include "avrcp_common.h"
#include "avrcp_caps_handler.h"
#include "avrcp_continuation_handler.h"
#include "avrcp_element_attributes_handler.h"
#include "avrcp_group_navigation_handler.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_notification_handler.h"
#include "avrcp_play_status_handler.h"
#include "avrcp_player_app_settings_handler.h"
#include "avrcp_private.h"
#include "avrcp_signal_handler.h"

#include <panic.h>
#include <print.h>
#include <source.h>
#include <stdlib.h>
#include <string.h>


static void avrcpSendRejectMetadataResponse(AVRCP *avrcp, uint16 pdu_id, avrcp_reject_code error_code)
{
	uint16 size_mandatory_data = 1;
	uint8 *mandatory_data = 0;
	uint16 param_length = size_mandatory_data;

	/* Insert the mandatory data which will be the error code */
	mandatory_data = (uint8 *) malloc(size_mandatory_data);
	mandatory_data[0] = error_code;

    sendMetadataResponse(avrcp, avctp_response_rejected, pdu_id, 0, avrcp_packet_type_single, param_length, size_mandatory_data, mandatory_data);
}


/* 
	Return the corresponding enum value from a PDU ID, only if arriving data should be blocked
	until the app sends a response. 
*/
static avrcpPending convertPduToEnum(uint16 pdu_id)
{
	switch (pdu_id)
	{
	case AVRCP_GET_CAPS_PDU_ID:
		return avrcp_get_caps;
		break;
	case AVRCP_LIST_APP_ATTRIBUTES_PDU_ID:		
		return avrcp_list_app_attributes;
		break;
	case AVRCP_LIST_APP_VALUE_PDU_ID:					
		return avrcp_list_app_values;
		break;
	case AVRCP_GET_APP_VALUE_PDU_ID:					
		return avrcp_get_app_values;
		break;
	case AVRCP_SET_APP_VALUE_PDU_ID:					
		return avrcp_set_app_values;
		break;
	case AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID:
		return avrcp_get_app_attribute_text;
		break;
	case AVRCP_GET_APP_VALUE_TEXT_PDU_ID:
		return avrcp_get_app_value_text;
		break;
	case AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID:			
		return avrcp_get_element_attributes;
		break;
	case AVRCP_GET_PLAY_STATUS_PDU_ID:				
		return avrcp_get_play_status;
		break;
	case AVRCP_INFORM_BATTERY_STATUS_PDU_ID:
		return avrcp_battery_information;
		break;
	case AVRCP_INFORM_CHARACTER_SET_PDU_ID:
		return avrcp_character_set;
		break;
	default:
		return avrcp_none;
		break;
	}
}


/****************************************************************************/
void abortContinuation(AVRCP *avrcp)
{
	/* Abort sending continuing packets back to CT. */
	if (avrcp->continuation_data)
		SourceEmpty(avrcp->continuation_data);

	MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET);
	avrcp->continuation_pdu = 0;
}


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************/
void avrcpSendMetadataFailCfmToClient(AVRCP *avrcp, avrcp_status_code status)
{
    switch (avrcp->pending)
    {
        case avrcp_get_caps:
			avrcpSendGetCapsCfm(avrcp, status, TRUE, 0, 1, 0, 0, 0, 0, 0, 0, 0);
            break;
        case avrcp_list_app_attributes:
            avrcpSendCommonFragmentedMetadataCfm(avrcp, status, TRUE, AVRCP_LIST_APP_ATTRIBUTE_CFM, 0, 1, 0, 0, 0, 0, 0, 0);
            break;
		case avrcp_list_app_values:
            avrcpSendCommonFragmentedMetadataCfm(avrcp, status, TRUE, AVRCP_LIST_APP_VALUE_CFM, 0, 1, 0, 0, 0, 0, 0, 0);
            break;
		case avrcp_get_app_values:
			avrcpSendCommonFragmentedMetadataCfm(avrcp, status, TRUE, AVRCP_GET_APP_VALUE_CFM, 0, 1, 0, 0, 0, 0, 0, 0);
			break;
		case avrcp_set_app_values:
			avrcpSendCommonMetadataCfm(avrcp, status, 0, AVRCP_SET_APP_VALUE_CFM, 0, 0);
			break;
		case avrcp_get_app_attribute_text:
			avrcpSendCommonFragmentedMetadataCfm(avrcp, status, TRUE, AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM, 0, 1, 0, 0, 0, 0, 0, 0);
			break;
		case avrcp_get_app_value_text:
			avrcpSendCommonFragmentedMetadataCfm(avrcp, status, TRUE, AVRCP_GET_APP_VALUE_TEXT_CFM, 0, 1, 0, 0, 0, 0, 0, 0);
			break;
		case avrcp_get_element_attributes:
			avrcpSendCommonFragmentedMetadataCfm(avrcp, status, TRUE, AVRCP_GET_ELEMENT_ATTRIBUTES_CFM, 0, 1, 0, 0, 0, 0, 0, 0);
			break;
		case avrcp_get_play_status:
			avrcpSendGetPlayStatusCfm(avrcp, status, 0, 0, 0, 0, 0, 0);
			break;
		case avrcp_playback_status:
		case avrcp_track_changed:
		case avrcp_track_reached_end:
		case avrcp_track_reached_start:
		case avrcp_playback_pos_changed:
		case avrcp_batt_status_changed:
		case avrcp_system_status_changed:
		case avrcp_device_setting_changed:
			avrcpSendRegisterNotificationFailCfm(avrcp, status, avrcp->pending - avrcp_playback_status + 1);
			break;
		case avrcp_next_group:
			avrcpSendCommonMetadataCfm(avrcp, status, 0, AVRCP_NEXT_GROUP_CFM, 0, 0);
			break;
		case avrcp_previous_group:
			avrcpSendCommonMetadataCfm(avrcp, status, 0, AVRCP_PREVIOUS_GROUP_CFM, 0, 0);
			break;
		case avrcp_abort_continuation:
			avrcpSendCommonMetadataCfm(avrcp, status, 0, AVRCP_ABORT_CONTINUING_RESPONSE_CFM, 0, 0);
			break;
		case avrcp_battery_information:
			avrcpSendCommonMetadataCfm(avrcp, status, 0, AVRCP_INFORM_BATTERY_STATUS_CFM, 0, 0);
			break;
		case avrcp_character_set:
			avrcpSendCommonMetadataCfm(avrcp, status, 0, AVRCP_INFORM_CHARACTER_SET_CFM, 0, 0);
			break;
        default:
            break;
    }

	avrcp->pending = avrcp_none;
}
#endif

/****************************************************************************/
void avrcpHandleMetadataCommand(AVRCP *avrcp, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	uint16 pdu_id = ptr[9];
    uint16 ctp_packet_type = ptr[0] & AVCTP0_PACKET_TYPE_MASK;
	uint16 meta_packet_type = ptr[10];
	uint16 transaction = (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F;
	bool source_processed = TRUE;
    uint16 pdu_size;
    
	if (ctp_packet_type == AVCTP0_PACKET_TYPE_START)
	{
		pdu_id = ptr[10];
		meta_packet_type = ptr[11];
		avrcp->last_metadata_pdu = meta_packet_type;
		avrcp->last_metadata_fragment = meta_packet_type;
        pdu_size = 14 + ((ptr[12] << 8) | ptr[13]); 
	}
	else if ((ctp_packet_type == AVCTP0_PACKET_TYPE_CONTINUE) || (ctp_packet_type == AVCTP0_PACKET_TYPE_END))
	{
		pdu_id = avrcp->last_metadata_pdu;
		meta_packet_type = avrcp->last_metadata_fragment;
        pdu_size = 11 + ((ptr[9] << 8) | ptr[10]);
	}
	else
    {
        pdu_size = 13 + ((ptr[11] >> 8) | ptr[12]);
		avrcp->last_metadata_pdu = 0;
    }

    if(pdu_size < avrcp->srcUsed)
    {
        avrcp->srcUsed = pdu_size;
    }

	/* Metadata must be enabled at this end to handle these commands. */
	if (!avrcpMetadataEnabled(avrcp))
	{
		sendMetadataResponse(avrcp, avctp_response_not_implemented, pdu_id, 0, avrcp_packet_type_single, 0, 0, 0);
		/* Drop the source here. */
		avrcpSourceProcessed(avrcp);
		return;
	}

	/* Discard any continuation packets if the CT sent a new command */
	if (avrcp->continuation_pdu && (pdu_id != AVRCP_REQUEST_CONTINUING_RESPONSE_PDU_ID) &&
		(pdu_id != AVRCP_ABORT_CONTINUING_RESPONSE_PDU_ID))
	{
		abortContinuation(avrcp);
	}

	/* If this is the last packet of a command then must wait for the app response before processing any more commands. 
	   The app should only send a metadata response once all parts of the command has arrived. */
	if (((meta_packet_type == AVCTP0_PACKET_TYPE_SINGLE)&&(ctp_packet_type == AVCTP0_PACKET_TYPE_SINGLE)) ||
			((meta_packet_type == AVCTP0_PACKET_TYPE_SINGLE)&&(ctp_packet_type == AVCTP0_PACKET_TYPE_END)) ||
			((meta_packet_type == AVCTP0_PACKET_TYPE_END)&&(ctp_packet_type == AVCTP0_PACKET_TYPE_SINGLE)) ||
			((meta_packet_type == AVCTP0_PACKET_TYPE_END)&&(ctp_packet_type == AVCTP0_PACKET_TYPE_END))
	)
	{
		if (convertPduToEnum(pdu_id) != avrcp_none)
		{
			uint8 data = 0;
			if (pdu_id == AVRCP_GET_CAPS_PDU_ID)
				data = ptr[13];
			/* Stop processing anymore data at this end until a response is sent. */
			avrcpBlockReceivedData(avrcp, convertPduToEnum(pdu_id), data);
			avrcp->rsp_transaction_label = transaction;
		}
		else if (pdu_id == AVRCP_REGISTER_NOTIFICATION_PDU_ID)
		{
			/* Stop processing anymore data at this end until a response is sent. */
			avrcpBlockReceivedData(avrcp, (avrcp_get_play_status + ptr[13]), 0);
		}
	}

    switch (pdu_id)
    {
    case AVRCP_GET_CAPS_PDU_ID:
        {
			avrcpHandleGetCapsCommand(avrcp, transaction, ptr[13]);
        }
        break;

    case AVRCP_LIST_APP_ATTRIBUTES_PDU_ID:
        {
			avrcpHandleListAppAttributesCommand(avrcp, transaction);
        }
        break;

	case AVRCP_LIST_APP_VALUE_PDU_ID:
        {
			avrcpHandleListAppValuesCommand(avrcp, transaction, ptr[13]);
        }
        break;
	case AVRCP_GET_APP_VALUE_PDU_ID:
        {
			if (!avrcpHandleGetAppValuesCommand(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				avrcpSendRejectMetadataResponse(avrcp, pdu_id, avrcp_reject_internal_error);
			else
				source_processed = FALSE;
        }
        break;
	case AVRCP_SET_APP_VALUE_PDU_ID:
        {
			if (!avrcpHandleSetAppValuesCommand(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				avrcpSendRejectMetadataResponse(avrcp, pdu_id, avrcp_reject_internal_error);
			else
				source_processed = FALSE;
        }
        break;
	case AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID:
		{
			if (!avrcpHandleGetAppAttributeTextCommand(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				avrcpSendRejectMetadataResponse(avrcp, pdu_id, avrcp_reject_internal_error);
			else
				source_processed = FALSE;
		}
		break;
	case AVRCP_GET_APP_VALUE_TEXT_PDU_ID:
		{
			if (!avrcpHandleGetAppValueTextCommand(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				avrcpSendRejectMetadataResponse(avrcp, pdu_id, avrcp_reject_internal_error);
			else
				source_processed = FALSE;
		}
		break;

	case AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID:
        {
			if (!avrcpHandleGetElementAttributesCommand(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				avrcpSendRejectMetadataResponse(avrcp, pdu_id, avrcp_reject_internal_error);
			else
				source_processed = FALSE;
        }
        break;
	case AVRCP_GET_PLAY_STATUS_PDU_ID:
        {
			avrcpSendCommonMetadataInd(avrcp, AVRCP_GET_PLAY_STATUS_IND, transaction);
        }
        break;
	case AVRCP_REGISTER_NOTIFICATION_PDU_ID:
		{
			avrcpHandleRegisterNotificationCommand(avrcp, transaction, ptr);
        }
        break;
	case AVRCP_REQUEST_CONTINUING_RESPONSE_PDU_ID:
		{
			avrcpHandleRequestContinuingCommand(avrcp, transaction, ptr[13]);
		}
		break;
	case AVRCP_ABORT_CONTINUING_RESPONSE_PDU_ID:
		{
			avrcpHandleAbortContinuingCommand(avrcp, transaction, ptr[13]);
		}
		break;
	case AVRCP_INFORM_BATTERY_STATUS_PDU_ID:
		{
			avrcpHandleInformBatteryStatusCommand(avrcp, transaction, ptr[13]);
		}
		break;
	case AVRCP_INFORM_CHARACTER_SET_PDU_ID:
		{
			if (!avrcpHandleInformCharSetCommand(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				avrcpSendRejectMetadataResponse(avrcp, pdu_id, avrcp_reject_internal_error);
			else
				source_processed = FALSE;
		}
		break;

    default:
        avrcpSendRejectMetadataResponse(avrcp, pdu_id, avrcp_reject_unknown_pdu);
    }

	/* Drop the source here if everyone is finished with it. */
	if (source_processed)
		avrcpSourceProcessed(avrcp);
}


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************/
void avrcpHandleMetadataResponse(AVRCP *avrcp, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
    uint16 pdu_id = ptr[9];
    uint16 ctp_packet_type = ptr[0] & AVCTP0_PACKET_TYPE_MASK;
	uint16 meta_packet_type = ptr[10];
	uint8 transaction = (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F;
	bool source_processed = TRUE;
	bool response_error = FALSE;
    uint16 pdu_size;
    
	if (ctp_packet_type == AVCTP0_PACKET_TYPE_START)
	{
		pdu_id = ptr[10];
		meta_packet_type = ptr[11];
		avrcp->last_metadata_pdu = meta_packet_type;
		avrcp->last_metadata_fragment = meta_packet_type;
        pdu_size = 14 + ((ptr[12] << 8) | ptr[13]); 
	}
	else if ((ctp_packet_type == AVCTP0_PACKET_TYPE_CONTINUE) || (ctp_packet_type == AVCTP0_PACKET_TYPE_END))
	{
		pdu_id = avrcp->last_metadata_pdu;
		meta_packet_type = avrcp->last_metadata_fragment;
        pdu_size = 11 + ((ptr[9] << 8) | ptr[10]); 

	}
	else
    {
		avrcp->last_metadata_pdu = 0;
        pdu_size = 13 + ((ptr[11] << 8) | ptr[12]); 

    }


    if(pdu_size < avrcp->srcUsed)
    {
        avrcp->srcUsed = pdu_size;
    }

	switch (pdu_id)
    {
    case AVRCP_GET_CAPS_PDU_ID:
		{			
			if (!avrcpHandleGetCapsResponse(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				response_error = TRUE;	
			else
				source_processed = FALSE;
		}
		break;

    case AVRCP_LIST_APP_ATTRIBUTES_PDU_ID:
		{
			if (!avrcpHandleListAppAttributesResponse(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				response_error = TRUE;	
			else
				source_processed = FALSE;
		}
		break;

	case AVRCP_LIST_APP_VALUE_PDU_ID: 
		{
			if (!avrcpHandleListAppValuesResponse(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				response_error = TRUE;	
			else
				source_processed = FALSE;
		}
		break;

	case AVRCP_GET_APP_VALUE_PDU_ID: 
		{
			if (!avrcpHandleGetAppValuesResponse(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				response_error = TRUE;	
			else
				source_processed = FALSE;
		}
		break;

	case AVRCP_SET_APP_VALUE_PDU_ID: 
		{
			avrcpHandleSetAppValuesResponse(avrcp, convertResponseToStatus(ptr[3]), transaction, source, packet_size);
		}
		break;

	case AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID:
		{
			if (!avrcpHandleGetAppAttributeTextResponse(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				response_error = TRUE;	
			else
				source_processed = FALSE;
		}
		break;

	case AVRCP_GET_APP_VALUE_TEXT_PDU_ID:
		{
			if (!avrcpHandleGetAppValueTextResponse(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				response_error = TRUE;	
			else
				source_processed = FALSE;
		}
		break;

	case AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID:
		{
			if (!avrcpHandleGetElementAttributesResponse(avrcp, ctp_packet_type, meta_packet_type, source, packet_size))
				response_error = TRUE;	
			else
				source_processed = FALSE;
		}
		break;

	case AVRCP_GET_PLAY_STATUS_PDU_ID: 
		{
			if (!avrcpHandleGetPlayStatusResponse(avrcp, transaction, ptr, source, packet_size))
				response_error = TRUE;	
		}
		break;

	case AVRCP_REGISTER_NOTIFICATION_PDU_ID:
		{
			avrcpSendNotification(avrcp, source, packet_size);   
			source_processed = FALSE;
        }
        break;

	case AVRCP_ABORT_CONTINUING_RESPONSE_PDU_ID:
		{
			avrcpSendCommonMetadataCfm(avrcp, convertResponseToStatus(ptr[3]), transaction, AVRCP_ABORT_CONTINUING_RESPONSE_CFM, source, packet_size);
		}
		break;
	case AVRCP_INFORM_BATTERY_STATUS_PDU_ID:
		{
			avrcpSendCommonMetadataCfm(avrcp, convertResponseToStatus(ptr[3]), transaction, AVRCP_INFORM_BATTERY_STATUS_CFM, source, packet_size);
		}
		break;
	case AVRCP_INFORM_CHARACTER_SET_PDU_ID:
		{
			avrcpSendCommonMetadataCfm(avrcp, convertResponseToStatus(ptr[3]), transaction, AVRCP_INFORM_CHARACTER_SET_CFM, source, packet_size);
		}
		break;

	default:
		/* Ignore any PDUs that aren't recognised. */
		response_error = TRUE;
		break;
	}

	/* Drop the source here if everyone is finished with it. */
	if (source_processed)
		avrcpSourceProcessed(avrcp);

	/* Quit now if the response wasn't correctly processed as a Metadata response. */
	if (response_error)
		return;
    
    (void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);
    
    if ((ctp_packet_type == AVCTP0_PACKET_TYPE_SINGLE) || (ctp_packet_type == AVCTP0_PACKET_TYPE_END))
		/* No longer waiting */
		avrcp->pending = avrcp_none;
	else if (avrcp->pending)
		/* Set timeout for next fragment to arrive. */
		MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_TIMEOUT);

	if ((meta_packet_type != AVCTP0_PACKET_TYPE_SINGLE) && (meta_packet_type != AVCTP0_PACKET_TYPE_END))
		/* Store the PDU ID of the fragmented data. */
		avrcp->continuation_pdu = pdu_id;
	else
		avrcp->continuation_pdu = 0;
}
#endif

/*****************************************************************************/
void prepareMetadataControlResponse(AVRCP *avrcp, avrcp_response_type response, uint16 id)
{
	uint16 size_mandatory_data = 0;
	uint8 *mandatory_data = 0;
	uint16 param_length = 0;
	avrcp_response_type res = response;

	standardiseCtypeControl(&res);

	if (res != avctp_response_accepted)
	{
		mandatory_data = insertRejectCode(&res, &size_mandatory_data, &param_length);
	}

	/* Send a response to this PDU now. */
    sendMetadataResponse(avrcp,  res, id, 0, avrcp_packet_type_single, param_length, size_mandatory_data, mandatory_data);

	/* Process any new data that may have arrived. */
    avrcpHandleReceivedData(avrcp);
}


/*****************************************************************************/
void prepareMetadataStatusResponse(AVRCP *avrcp, avrcp_response_type response, uint16 id, uint16 param_length, Source data_list, uint16 size_mandatory_data, uint8 *mandatory_data)
{
	/* Check if Metadata packet has to be fragmented to fit into 512 AV/C frame size restriction. */
    if ((param_length + 13) > 515)
    {
		/* Metadata fragmentation will occur. */
        uint16 mtu_packet_data_length = 515 - 13;  
		uint8 response_transaction = avrcp->rsp_transaction_label;
 
		/* Send the Start packet immediately. */
        sendMetadataResponse(avrcp, response, id, data_list, avrcp_packet_type_start, mtu_packet_data_length, size_mandatory_data, mandatory_data);

		/* Store the continuation packets until the CT specifically requests them. */
		avrcpStoreNextContinuationPacket(avrcp, data_list, param_length, id, 2, response, response_transaction);
    }
    else
	{
		/* No metadata fragmentation */		
        sendMetadataResponse(avrcp,  response, id, data_list, avrcp_packet_type_single, param_length, size_mandatory_data, mandatory_data);
		/* The data has been sent so empty the source that was supplied from the app. */
		SourceEmpty(data_list);
	}

	/* Process any new data that may have arrived. */
    avrcpHandleReceivedData(avrcp);
}


/*****************************************************************************/
void sendMetadataResponse(AVRCP *avrcp, avrcp_response_type response, uint8 pdu_id, Source caps_list, avrcp_packet_type metadata_packet_type, uint16 param_length, uint16 size_mandatory_data, uint8 *mandatory_data)
{
    Sink sink = avrcp->sink;    
    uint16 bytes_to_flush = 0;
    uint16 flushed_data = param_length - size_mandatory_data;
    avrcp_packet_type ctp_packet_type = avrcp_packet_type_single;
    uint16 continue_packets = 0;
    uint16 stream_move = 0;
	uint8 transaction = 0;
    
    /* Retrieve correct max packet size (max mtu) */
    uint16 max_packet_size = avrcp->l2cap_mtu;

	avrcpUnblockReceivedData(avrcp);
	avrcpSourceProcessed(avrcp);

    if (sink)
    {
        uint8 *ptr = 0;

		/* Retrieve correct transaction for registered events */
		if (pdu_id == AVRCP_REGISTER_NOTIFICATION_PDU_ID)
			transaction = avrcp->notify_transaction_label[mandatory_data[0]-1];

        /* Create the response PDU - AVRCP and Metadata transfer headers */
        ptr = avrcpCreateMetadataTransferRsp(avrcp, pdu_id, sink, response, param_length, size_mandatory_data, metadata_packet_type, &bytes_to_flush, &ctp_packet_type, &continue_packets, transaction);

        if (!ptr)
        {
            /* No space in the sink */
            return;
        }
 
		if ((metadata_packet_type == avrcp_packet_type_single) || (metadata_packet_type == avrcp_packet_type_start))
        {
			uint16 i;
			for (i = 0; i < size_mandatory_data; i++)
				ptr[i] = mandatory_data[i];
		}

        if (ctp_packet_type == avrcp_packet_type_start)
        {
			flushed_data = max_packet_size - 14 - size_mandatory_data;
			param_length = param_length - flushed_data - size_mandatory_data;;
        }
		if (size_mandatory_data)
			free(mandatory_data);
        
        /* If optional params sent copy them now */
        if (flushed_data)              
            stream_move = StreamMove(sink, caps_list, flushed_data);

        /* Send the data */
        (void) SinkFlush(sink, bytes_to_flush);
        
        if (ctp_packet_type == avrcp_packet_type_start)
        {
            uint16 i;
            ctp_packet_type = avrcp_packet_type_continue;
            for (i = 0; i < continue_packets; i++)
            {
                /* Create the response PDU - AVRCP and Metadata transfer headers */
                ptr = avrcpCreateMetadataTransferRsp(avrcp, pdu_id, sink, response, param_length, size_mandatory_data, metadata_packet_type, &bytes_to_flush, &ctp_packet_type, &continue_packets, transaction);
                /* Add the data */
                flushed_data = max_packet_size - 11;
                param_length -= flushed_data;
                stream_move = StreamMove(sink, caps_list, flushed_data);
                /* Send the data */
                (void) SinkFlush(sink, max_packet_size);
            }
            ctp_packet_type = avrcp_packet_type_end;
            /* Create the response PDU - AVRCP and Metadata transfer headers */
            ptr = avrcpCreateMetadataTransferRsp(avrcp, pdu_id, sink, response, param_length, size_mandatory_data, metadata_packet_type, &bytes_to_flush, &ctp_packet_type, &continue_packets, transaction);
            /* Add the data */
            stream_move = StreamMove(sink, caps_list, param_length);   
            /* Send the data */
            (void) SinkFlush(sink, param_length);
        }
    }
}


/*****************************************************************************/
uint8 *avrcpCreateMetadataTransferCmd(uint8 pdu_id, uint16 param_length, uint8 *params, uint16 extra_data_length, uint8 pkt_type, uint16 *length)
{
    uint8 *pdu;

	if (!extra_data_length)
		extra_data_length = param_length;

	*length  = METADATA_HEADER_SIZE + extra_data_length;

    /* Allocate the PDU */
    pdu = (uint8 *) malloc(*length);

    if (pdu)
    {
        /* Set the PDU ID - one octet */ 
        pdu[0] = pdu_id & 0xff;

        /* Pkt type - single, start, continue, end */
        pdu[1] = pkt_type;

        /* Parameter length */
        pdu[2] = (param_length >> 8) & 0xff;
        pdu[3] = param_length & 0xff;

        /* Copy the passed in params into the end of the pdu */
        if (params)
            memcpy(pdu+4, params, param_length);
    }

    return pdu;
}


/*****************************************************************************/
uint8 *avrcpCreateMetadataTransferRsp(AVRCP *avrcp, uint8 pdu_id, Sink sink, avrcp_response_type response, uint16 param_length, uint16 size_mandatory_data, avrcp_packet_type metadata_packet_type, uint16 *bytes_flush, avrcp_packet_type *ctp_packet_type, uint16 *continue_packets, uint8 transaction)
{
    uint8 *ptr = 0;
  
    /* Retrieve correct max packet size (max mtu) */
    uint16 max_packet_size = avrcp->l2cap_mtu;
    
    uint16 start_offset = 0;
    uint16 packet_offset = 3;

	uint8 transaction_id = 0;
    
    /* Need to see if Metadata has to be fragmented across CTP packets. */
    switch (*ctp_packet_type)
    {
        case avrcp_packet_type_start:
            /* Work out if data can fit in single packet */
            if ((13 + param_length) > max_packet_size)
            {
                /* Can't fit in single packet */
                *bytes_flush = max_packet_size;
                start_offset = 1;
            }
            else
            {
                /* Able to fit in single packet */
                *bytes_flush = (13 + param_length);
                *ctp_packet_type = avrcp_packet_type_single;
            }
            break;
        case avrcp_packet_type_continue:
        case avrcp_packet_type_end:
            packet_offset = 1;
            *bytes_flush = (11 + param_length);
            if (*bytes_flush > max_packet_size)
                *bytes_flush = max_packet_size;
            break;            
        case avrcp_packet_type_single:
        default:
            *bytes_flush = (13 + param_length);
            if (*bytes_flush > max_packet_size)
            {
                /* If data can't fit in single packet then make it a start packet */
                *bytes_flush = max_packet_size;
                *ctp_packet_type = avrcp_packet_type_start;
                start_offset = 1;
            }
            break;
    } 
    
    if (*ctp_packet_type == avrcp_packet_type_start)
    {
        uint16 start_data_length = max_packet_size - 14;
        uint16 continue_data_length = max_packet_size - 11;
        *continue_packets = (param_length - start_data_length) / continue_data_length;
        if (!((param_length - start_data_length) % continue_data_length))	
		    *continue_packets--;
    }

    /* Create packet header */
	ptr = avrcpGrabSink(sink, 10 + packet_offset + start_offset + size_mandatory_data);
	if (!ptr)
		return 0;

    /* AVCTP header */
	if (avrcp->rsp_transaction_label)
	{
		transaction_id = avrcp->rsp_transaction_label;
		avrcp->rsp_transaction_label = 0;
	}
	else if (transaction)
		transaction_id = transaction;
	else
		PRINT(("Invalid transaction ID in response\n"));

    ptr[0] = (transaction_id << AVCTP0_TRANSACTION_SHIFT) | ((*ctp_packet_type << 2) & AVCTP0_PACKET_TYPE_MASK) | AVCTP0_CR_RESPONSE;
    
    if (start_offset)
        ptr[1] = *continue_packets + 2;
    if (packet_offset == 3)
    {
	   ptr[1 + start_offset] = AVCTP1_PROFILE_AVRCP_HIGH;
	   ptr[2 + start_offset] = AVCTP2_PROFILE_AVRCP_REMOTECONTROL;
    }

	/* AVRCP header */
	ptr[packet_offset + start_offset] = response;
	ptr[1 + packet_offset + start_offset] = ((subunit_panel << AVRCP1_SUBUNIT_TYPE_SHIFT) & AVRCP1_SUBUNIT_TYPE_MASK) |
		 (0x00 & AVRCP1_SUBUNIT_ID_MASK);
	ptr[2 + packet_offset + start_offset] = AVRCP2_VENDORDEPENDENT;
			
	ptr[3 + packet_offset + start_offset] = (AVRCP_BT_COMPANY_ID & 0xFF0000) >> 16;
	ptr[4 + packet_offset + start_offset] = (AVRCP_BT_COMPANY_ID & 0x00FF00) >> 8;
	ptr[5 + packet_offset + start_offset] = (AVRCP_BT_COMPANY_ID & 0x0000FF);

    /* Create metadata PDU header */
    ptr[6 + packet_offset + start_offset] = pdu_id & 0xff;        
    ptr[7 + packet_offset + start_offset] = metadata_packet_type << 2;
    ptr[8 + packet_offset + start_offset] = (param_length >> 8) & 0xff;
    ptr[9 + packet_offset + start_offset] = param_length & 0xff;

    return (ptr + 10 + packet_offset + start_offset);
}


