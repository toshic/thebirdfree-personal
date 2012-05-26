/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_element_attributes.c

DESCRIPTION
    This file defines the APIs for GetElementAttributes feature    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "avrcp_common.h"

/****************************************************************************
*NAME    
*    AvrcpGetElementAttributes    
*
*DESCRIPTION
*  API function to request GetElementAttributes. 
*    
*PARAMETERS
*   avrcp            - Task
*   identifier_high -  Element identifier is 8 octets in big-endian format, 
*                      this parameter should be the most significant 4 
*                      octets. Currently only allowed identifier value is 
*                      PLAYING (0x0), so it must be set to 0.
*   identifier_low  - least significant 4 octets of the element identifier.
*                     currently allowed value is only PLAYING(0x0), so it 
*                     must be set to 0.
*   size_attributes - Size of attributes in Bytes
*   attributes      - list of attributes. 
*
*RETURN
*  AVRCP_GET_ELEMENT_ATTRIBUTES_CFM 
*******************************************************************************/
void AvrcpGetElementAttributes(AVRCP    *avrcp,
                               uint32   identifier_high, 
                               uint32   identifier_low, 
                               uint16   size_attributes, 
                               Source   attributes)
{
    uint8 extra_params[AVRCP_GET_ELEMENTS_HDR_SIZE];
    avrcp_status_code status;

    /* Fill in the extra Header for Get Elements which is Identifier and
       number of elements*/
    convertUint32ToUint8Values(&extra_params[0], identifier_high);
    convertUint32ToUint8Values(&extra_params[4], identifier_low);
    extra_params[8] = size_attributes/4;

    status = avrcpMetadataStatusCommand(avrcp, 
                                        AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID,
                                        avrcp_get_element_attributes, 
                                        AVRCP_GET_ELEMENTS_HDR_SIZE, 
                                        extra_params,size_attributes,
                                        attributes);

    if (status != avrcp_success)
    {
        avrcpSendCommonFragmentedMetadataCfm(avrcp, status, 
                                            AVRCP_GET_ELEMENT_ATTRIBUTES_CFM, 
                                            0, 0, 0);
    }
}
/****************************************************************************
*NAME    
*    AvrcpGetElementAttributesResponse    
*
*DESCRIPTION
*  API function at TG to respond to GetElementAttributes. 
*    
*PARAMETERS
*   avrcp            - Task
*   response         - Expected values are avrcp_response_stable,
*                       avrcp_response_rejected or 
                        avrcp_response_rejected_invalid_param
*   identifier_high - Must be 0 (Reserved for future)
*   identifier_low  - Must be 0  (Reserved for future)
*   size_attributes - Size of attributes in Bytes
*   attributes      - list of attributes. 
*
*RETURN
*******************************************************************************/

void AvrcpGetElementAttributesResponse(AVRCP               *avrcp, 
                                       avrcp_response_type response, 
                                       uint16              number_of_attributes,
                                       uint16              size_attributes, 
                                       Source              attributes)
{
    /* Only allow a response to be sent if the corresponding command arrived. */
    if (avrcp->block_received_data == avrcp_get_element_attributes)
    {
        sendGetElementsResponse(avrcp, response, 
                                number_of_attributes, 
                                size_attributes, attributes);    
    }
    else
        AVRCP_INFO(("AvrcpGetElementAttributesResponse:"
                "CT not waiting for response\n"));
}
