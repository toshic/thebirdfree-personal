/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_caps.c        

DESCRIPTION
This file defines the APIs for GetCapabilities feature    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "avrcp_caps_handler.h"


/****************************************************************************
*NAME    
*    AvrcpGetCapabilities    
*
*DESCRIPTION
*  API function to send AvrcpGetCapabilities Request from CT.
*    
*PARAMETERS
*   avrcp            - Task
*   caps             - Requested capability. capability value MUST be either
*                      avrcp_capability_company_id or 
*                      avrcp_capability_event_supported.
*
*RETURN
*******************************************************************************/
void AvrcpGetCapabilities(AVRCP *avrcp, avrcp_capability_id caps)
{
    uint8 caps_params[] = {0};
    avrcp_status_code status;

#ifdef AVRCP_DEBUG_LIB    
    if ((caps != avrcp_capability_company_id) && 
        (caps != avrcp_capability_event_supported))
        AVRCP_DEBUG(("Invalid capability type requested 0x%x\n", caps));
#endif

    caps_params[0] = caps;

    status = avrcpMetadataStatusCommand(avrcp, AVRCP_GET_CAPS_PDU_ID, 
                                        avrcp_get_caps, 
                                        sizeof(caps_params),
                                        caps_params, 0, 0);

    if (status != avrcp_success)
    {
        avrcpSendGetCapsCfm(avrcp, status, 0, 0, 0, 0, 0);
    }
}


/****************************************************************************
*NAME    
*   AvrcpGetCapabilitiesResponse 
*
*DESCRIPTION
*  API function to respond to AvrcpGetCapabilitiesResponse request.
*  TG application shall
*  call function on receiving AVRCP_GET_CAPABILITIES_IND.
*    
*PARAMETERS
*   avrcp                   - Task
*   avrcp_response_code     - response.  Expected responses are 
*                             avctp_response_stable or avctp_response_rejected
*                             or avrcp_response_rejected_invalid_param.
*   caps                    - Capability ID requested.
*   caps_list               - List for capability values.
*
*RETURN
*******************************************************************************/
void AvrcpGetCapabilitiesResponse(AVRCP                 *avrcp, 
                                  avrcp_response_type   response, 
                                  avrcp_capability_id   caps, 
                                  uint16                size_caps_list, 
                                  Source                caps_list)
{
#ifdef AVRCP_DEBUG_LIB    
    if ((caps != avrcp_capability_company_id) && 
        (caps != avrcp_capability_event_supported))
        AVRCP_DEBUG(("Invalid capability type requested 0x%x\n", caps));
#endif

    /* Only allow a response to be sent if the corresponding command arrived. */
    if (avrcp->block_received_data == avrcp_get_caps)
    {
        avrcpSendGetCapsResponse(   avrcp, 
                                    response, 
                                    caps, 
                                    size_caps_list, 
                                    caps_list);
    }
    else
        AVRCP_INFO(("AvrcpGetCapabilitiesResponse: CT not waiting for response\n"));
}

