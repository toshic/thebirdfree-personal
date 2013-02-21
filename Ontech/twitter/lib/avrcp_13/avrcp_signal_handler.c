/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_signal_handler.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_caps_handler.h"
#include "avrcp_common.h"
#include "avrcp_group_navigation_handler.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_player_app_settings_handler.h"
#include "avrcp_send_response.h"
#include "avrcp_signal_handler.h"
#include "avrcp_signal_passthrough.h"
#include "avrcp_signal_unit_info.h"
#include "avrcp_signal_vendor.h"
#include "avrcp_notification_handler.h"
#include "avrcp_private.h"

#include <panic.h>
#include <source.h>
#include <stdlib.h>
#include <string.h>


/*lint -e641 -e572 */

/*****************************************************************************/
void avrcpHandleReceivedData(AVRCP *avrcp)
{
	Source source = StreamSourceFromSink(avrcp->sink);
	uint16 packet_size;

	while (((packet_size = SourceBoundary(source)) != 0) && !avrcp->block_received_data && !avrcp->srcUsed)
	{
		const uint8 *ptr = SourceMap(source);
	
		if (packet_size > 6)
		{
			avrcp->srcUsed = packet_size;

			if ((ptr[0] & AVCTP0_CR_MASK) == AVCTP0_CR_COMMAND)
                avrcpHandleCommand(avrcp, source, packet_size);
            else
#ifdef AVRCP_CT_SUPPORT
                avrcpHandleResponse(avrcp, source, packet_size);
#else
				SourceDrop(source, packet_size);
#endif
        }
		else
			SourceDrop(source, packet_size);
    }
}
    

/*****************************************************************************/
void avrcpHandleCommand(AVRCP *avrcp, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
    uint16 ctype_offset = 3, opcode_offset = 5; /* default for single packet */
    uint16 packet_type = ptr[0] & AVCTP0_PACKET_TYPE_MASK;


    if (packet_type == AVCTP0_PACKET_TYPE_SINGLE) 
    {
        if (ptr[1] != AVCTP1_PROFILE_AVRCP_HIGH)
		{
            avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_bad_profile);
            return;
        }

		if(ptr[2] != AVCTP2_PROFILE_AVRCP_REMOTECONTROL){
			if(((avrcp->device_type == avrcp_target) && (ptr[2] != AVCTP2_PROFILE_AVRCP_CONTROLTARGET)) ||
				((avrcp->device_type == avrcp_controller) && (ptr[2] != AVCTP2_PROFILE_AVRCP_REMOTECONTROLLER)))
			{
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_bad_profile);
				return;
			}

		}
    } 
	else if (packet_type == AVCTP0_PACKET_TYPE_START)
	{
		ctype_offset = 4;
		opcode_offset = 6;
        if (ptr[2] != AVCTP1_PROFILE_AVRCP_HIGH)
		{
            avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_bad_profile);
            return;
        }

		if(ptr[3] != AVCTP2_PROFILE_AVRCP_REMOTECONTROL){
			if(((avrcp->device_type == avrcp_target) && (ptr[3] != AVCTP2_PROFILE_AVRCP_CONTROLTARGET)) ||
				((avrcp->device_type == avrcp_controller) && (ptr[3] != AVCTP2_PROFILE_AVRCP_REMOTECONTROLLER)))
			{
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_bad_profile);
				return;
			}

		}
	}
	else if ((packet_type == AVCTP0_PACKET_TYPE_CONTINUE) || (packet_type == AVCTP0_PACKET_TYPE_END))
	{
		opcode_offset = 3;
		ctype_offset = 1;
	}
    else
    {
        avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_not_implemented);
        return;
    }

	if ((ptr[ctype_offset] == AVRCP0_CTYPE_CONTROL) && (ptr[opcode_offset] == AVRCP2_PASSTHROUGH))
	    avrcpHandlePassthroughCommand(avrcp, ptr, packet_size);
    else if ((ptr[ctype_offset] == AVRCP0_CTYPE_STATUS) && (ptr[opcode_offset] == AVRCP2_UNITINFO) && (packet_type == AVCTP0_PACKET_TYPE_SINGLE))
        avrcpHandleUnitInfoCommand(avrcp, ptr, packet_size);
    else if ((ptr[ctype_offset] == AVRCP0_CTYPE_STATUS) && (ptr[opcode_offset] == AVRCP2_SUBUNITINFO) && (packet_type == AVCTP0_PACKET_TYPE_SINGLE))
        avrcpHandleSubUnitInfoCommand(avrcp, ptr, packet_size);
    else if (ptr[opcode_offset] == AVRCP2_VENDORDEPENDENT)
    {
        uint32 cid = avrcpGetCompanyId(ptr, opcode_offset+1);
        if (cid == AVRCP_BT_COMPANY_ID)
            avrcpHandleMetadataCommand(avrcp, source, packet_size);
        else
            avrcpHandleVendorCommand(avrcp, ptr, packet_size);
    }
    else
        avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_not_implemented);
}


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void avrcpHandleResponse(AVRCP *avrcp, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
    uint16 opcode_offset = 5; /* default for single packet */
    uint16 packet_type = ptr[0] & AVCTP0_PACKET_TYPE_MASK;
    
    if (packet_type == AVCTP0_PACKET_TYPE_START)
		opcode_offset = 6;
	else if ((packet_type == AVCTP0_PACKET_TYPE_CONTINUE) || (packet_type == AVCTP0_PACKET_TYPE_END))
		opcode_offset = 3;

	if ((avrcp->pending == avrcp_passthrough) && (ptr[opcode_offset] == AVRCP2_PASSTHROUGH))
	    avrcpHandlePassthroughResponse(avrcp, ptr, packet_size);
	else if (((avrcp->pending == avrcp_next_group)||(avrcp->pending == avrcp_previous_group)) && (ptr[opcode_offset] == AVRCP2_PASSTHROUGH))
	    avrcpHandleGroupResponse(avrcp, source, packet_size);
    else if ((avrcp->pending == avrcp_unit_info) && (ptr[opcode_offset] == AVRCP2_UNITINFO) && (packet_type == AVCTP0_PACKET_TYPE_SINGLE))
        avrcpHandleUnitInfoResponse(avrcp, ptr, packet_size);
    else if ((avrcp->pending == avrcp_subunit_info) && (ptr[opcode_offset] == AVRCP2_SUBUNITINFO) && (packet_type == AVCTP0_PACKET_TYPE_SINGLE))
        avrcpHandleSubUnitInfoResponse(avrcp, ptr, packet_size);
    else if ((avrcp->pending == avrcp_vendor) && (ptr[opcode_offset] == AVRCP2_VENDORDEPENDENT))
        avrcpHandleVendorResponse(avrcp, ptr, packet_size);
    else if (ptr[opcode_offset] == AVRCP2_VENDORDEPENDENT)
        avrcpHandleMetadataResponse(avrcp, source, packet_size);
	else
		/* The source has been processed so drop it here. */
		avrcpSourceProcessed(avrcp);
}
#endif

/*****************************************************************************/
void avrcpSendResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size, avrcp_response_type response)
{
	Sink sink = avrcp->sink;
 
    uint8 *res;
		
	/* PID will be there only in Start/Single Packet. Caller function will take care of that.*/
	if (response == avctp_response_bad_profile)
	{

		packet_size = AVCTP_HEADER_SIZE_SINGLE_PKT;
	}

    /*
        Currently doesn't handle a packet size > max_mtu as the received 
	    packet should always be <= max_mtu, and split into a start, continue 
	    and end packets if neccessary.
    */
	res = avrcpGrabSink(sink, packet_size);

	/* Unable to Grab Sink cause Panic on Debug Mode*/
	if(res){

		/* Copy the original message */
		memcpy(res, ptr, packet_size);

		/* flip CR bit */
		res[0] ^= AVCTP0_CR_MASK;

		if (response == avctp_response_bad_profile)
		{
			/*Translate our bad profile error with the invalid PID bit set. */
			res[0] |= AVCTP0_IPID;
		}
		else 
		{
			/* Overwrite command type with response */
			if ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START)
				res[4] = response;
			else if ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_SINGLE) 
				res[3] = response;
			else
				res[1] = response;
			
		}

		/* Send response */
		(void) SinkFlush(sink,packet_size);
	}
    
	/* Drop the source here. */
	avrcpSourceProcessed(avrcp);

	/* Allow data arriving to be proccessed. */
    avrcpUnblockReceivedData(avrcp);
}


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void avrcpHandleInternalWatchdogTimeout(AVRCP *avrcp)
{
	if (avrcp->sink)
	{
        switch (avrcp->pending)
        {
            case avrcp_passthrough:
			    avrcpSendPassthroughCfmToClient(avrcp, avrcp_timeout);
                break;
            case avrcp_unit_info:
			    avrcpSendUnitInfoCfmToClient(avrcp, avrcp_timeout, 0, 0, (uint32) 0);
                break;
            case avrcp_subunit_info:
			    avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_timeout, 0, 0);
                break;
            case avrcp_vendor:
			    avrcpSendVendordependentCfmToClient(avrcp, avrcp_timeout, 0);
                break;
            default:
                if (avrcp->pending >= avrcp_get_caps)
                    avrcpSendMetadataFailCfmToClient(avrcp, avrcp_timeout);
                break;
        }		
	}
	avrcp->pending = avrcp_none;
}
#endif

/*****************************************************************************/
void avrcpBlockReceivedData(AVRCP *avrcp, avrcpPending pending_command, uint16 data)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT);
	avrcp->block_received_data = pending_command;
	message->pending_command = pending_command;
	message->data = data;
	MessageSendLater(&avrcp->task, AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT, message, AVCTP_SEND_RESPONSE_TIMEOUT);
}


/*****************************************************************************/
void avrcpUnblockReceivedData(AVRCP *avrcp)
{
	avrcp->block_received_data = 0;
	MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT);
}


/*****************************************************************************/
void avrcpHandleInternalSendResponseTimeout(AVRCP *avrcp, const AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT_T *res)
{
	/* Send a reject response if the application fails to send a response to the pending command
		within the alloted time. */
    switch (res->pending_command)
    {
	case avrcp_none:
		break;
	case avrcp_passthrough:
		sendPassThroughResponse(avrcp, avctp_response_rejected);
		break;
	case avrcp_unit_info:
		sendUnitInfoResponse(avrcp, 0, 0, 0, 0);
		break;
	case avrcp_subunit_info:
		sendSubunitInfoResponse(avrcp, 0, 0);
		break;
	case avrcp_vendor:
		sendVendorDependentResponse(avrcp, avctp_response_rejected);
		break;
	case avrcp_get_caps:
		avrcpSendGetCapsResponse(avrcp, avctp_response_rejected, res->data, 0, 0);
		break;
	case avrcp_list_app_attributes:
		sendListAttrResponse(avrcp, avctp_response_rejected, 0, 0);
		break;
	case avrcp_list_app_values:
		sendListValuesResponse(avrcp, avctp_response_rejected, 0, 0);
		break;
	case avrcp_get_app_values:
		sendGetValuesResponse(avrcp, avctp_response_rejected, 0, 0);
		break;
	case avrcp_set_app_values:
		sendSetValuesResponse(avrcp, avctp_response_rejected);
		break;
	case avrcp_get_app_attribute_text:
		sendGetAttributeTextResponse(avrcp, avctp_response_rejected, 0, 0, 0);
		break;
	case avrcp_get_app_value_text:
		sendGetValueTextResponse(avrcp, avctp_response_rejected, 0, 0, 0);
		break;
	case avrcp_get_element_attributes:
		sendGetElementsResponse(avrcp, avctp_response_rejected, 0, 0, 0);
		break;
	case avrcp_get_play_status:
		sendPlayStatusResponse(avrcp, avctp_response_rejected, 0, 0, 0);
		break;
	case avrcp_playback_status:
	case avrcp_track_changed:
	case avrcp_track_reached_end:
	case avrcp_track_reached_start:
	case avrcp_playback_pos_changed:
	case avrcp_batt_status_changed:
	case avrcp_system_status_changed:
	case avrcp_device_setting_changed:
		sendRegisterNotificationResponse(avrcp,avctp_response_rejected,0,0,0,0);
        clearRegisterNotification(avrcp, res->pending_command - avrcp_get_play_status); 
		break;
	case avrcp_request_continuation:
		break;
	case avrcp_abort_continuation:
		break;
	case avrcp_next_group:
		sendNextGroupResponse(avrcp, avctp_response_rejected);
		break;
	case avrcp_previous_group:
		sendPreviousGroupResponse(avrcp, avctp_response_rejected);
		break;
	case avrcp_battery_information:
		avrcpSendInformBatteryResponse(avrcp, avctp_response_rejected);
		break;
	case avrcp_character_set:
		avrcpSendInformCharSetResponse(avrcp, avctp_response_rejected);
		break;
	default:
		break;
    }		
}


/*lint +e641 +e572 */
