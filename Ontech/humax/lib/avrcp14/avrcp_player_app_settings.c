/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_player_app_settings.c

DESCRIPTION
This file defines the API Functions for following Player Application Settings Features.     

    - ListPlayerApplicationSettingsAttributes
    - ListPlayerApplicationSettingsValue
    - GetCurrentPlayerApplicationSettingValue
    - SetPlayerApplicationsSettingValue
    - GetPlayerApplicationsSettingsAttributeText
    - GetPlayerApplicationSettingValueText

    - InformDisplayableCharacterSet
    - InformBatteryStatusOfCT 

NOTES

*/


/****************************************************************************
    Header files
*/
#include "avrcp_common.h"


/*****************************************************************************/
void AvrcpListAppSettingAttribute(AVRCP *avrcp)
{
    avrcp_status_code status = avrcpMetadataStatusCommand(avrcp,
                                        AVRCP_LIST_APP_ATTRIBUTES_PDU_ID, 
                                        avrcp_list_app_attributes,
                                        0, 0, 0, 0);

    if (status != avrcp_success)
    {
        avrcpSendCommonFragmentedMetadataCfm(avrcp, status, 
                                            AVRCP_LIST_APP_ATTRIBUTE_CFM, 
                                            0, 0, 0);
    }
}


/*****************************************************************************/
void AvrcpListAppSettingAttributeResponse(AVRCP *avrcp, 
                                          avrcp_response_type response, 
                                          uint16 size_attributes, 
                                          Source attributes)
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->block_received_data == avrcp_list_app_attributes)
    {
        sendListAttrResponse(avrcp, response, size_attributes, attributes);
    }
    else
        AVRCP_INFO(("AvrcpListAppSettingAttributeResponse:"
              "CT not waiting for response\n"));
}



/*****************************************************************************/
void AvrcpListAppSettingValue(AVRCP *avrcp, uint16 attribute_id)
{
    uint8 status_params[] = {0};
    avrcp_status_code status;

    status_params[0] = attribute_id;

    status = avrcpMetadataStatusCommand(avrcp, AVRCP_LIST_APP_VALUE_PDU_ID,
                                        avrcp_list_app_values, 
                                        sizeof(status_params), 
                                        status_params, 0, 0);

    if (status != avrcp_success)
    {
        avrcpSendCommonFragmentedMetadataCfm(avrcp, status, 
                                             AVRCP_LIST_APP_VALUE_CFM, 
                                             0, 0, 0);
    }
}


/*****************************************************************************/
void AvrcpListAppSettingValueResponse(AVRCP              *avrcp, 
                                      avrcp_response_type response, 
                                      uint16              size_values, 
                                      Source              values)
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->block_received_data == avrcp_list_app_values)
    {
        sendListValuesResponse(avrcp, response, size_values, values);
    }
    else
        AVRCP_INFO(("AvrcpListAppSettingValueResponse:"
              "CT not waiting for response\n"));
}

/****************************************************************************
*NAME    
*    AvrcpGetCurrentAppSettingValue    
*
*DESCRIPTION
*  API function to send GetCurrentAppSettingValue Request from CT.
*    
*PARAMETERS
*   avrcp                   - Task
*   size_attributes         - Number of Bytes in attributes (1-255)
*   attributes              - Player application setting attribute ID for
*                             which the corresponding current set value is
*                             requested
*RETURN
************************************************************************************/

void AvrcpGetCurrentAppSettingValue(AVRCP   *avrcp, 
                                    uint16  size_attributes, 
                                    Source  attributes)
{
    uint8 extra_params[AVRCP_APP_NUM_ATTR_HDR_SIZE];
    avrcp_status_code status;

    extra_params[0] = size_attributes;

    status = avrcpMetadataStatusCommand(avrcp, AVRCP_GET_APP_VALUE_PDU_ID, 
                                        avrcp_get_app_values, 
                                        AVRCP_APP_NUM_ATTR_HDR_SIZE, 
                                        extra_params,
                                        size_attributes,
                                        attributes);

    if (status != avrcp_success)
    {
        avrcpSendCommonFragmentedMetadataCfm(avrcp, status,
                                             AVRCP_GET_APP_VALUE_CFM, 0, 0, 0);
    }
}

/*****************************************************************************/
void AvrcpGetCurrentAppSettingValueResponse(AVRCP *avrcp, 
                                            avrcp_response_type response, 
                                            uint16 size_values, 
                                            Source values)
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->block_received_data == avrcp_get_app_values)
    {
        sendGetValuesResponse(avrcp, response, size_values, values);
    }
    else
        AVRCP_INFO(("AvrcpGetCurrentAppSettingValueResponse:"
               "CT not waiting for response\n"));
}

/*****************************************************************************/
void AvrcpSetAppSettingValue(AVRCP *avrcp, 
                             uint16 size_attributes, 
                             Source attributes)
{
    avrcp_status_code status;
    uint8 num_attr[1];


    /* Size of Attributes should be multiple of 2 contains the ID & 
       Value combination */
    if(size_attributes % 2)
    {
        avrcpSendCommonMetadataCfm(avrcp, 
                                   avrcp_rejected_invalid_param, 
                                   AVRCP_SET_APP_VALUE_CFM);
        return;
    }

    num_attr[0] = size_attributes/2;

    status = avrcpMetadataControlCommand(avrcp, AVRCP_SET_APP_VALUE_PDU_ID,
                                         avrcp_set_app_values, 
                                         1, num_attr,
                                         size_attributes,attributes);

    if (status != avrcp_success)
    {
        avrcpSendCommonMetadataCfm(avrcp, status, AVRCP_SET_APP_VALUE_CFM);
    }
}

/*****************************************************************************/
void AvrcpSetAppSettingValueResponse(AVRCP *avrcp, avrcp_response_type response)
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->block_received_data == avrcp_set_app_values)
    {
        sendSetValuesResponse(avrcp, response);
    }
    else
        AVRCP_INFO(("AvrcpSetAppSettingValueResponse: "
                "CT not waiting for response\n"));
}


/****************************************************************************
*NAME    
*    AvrcpGetAppSettingAttributeText    
*
*DESCRIPTION
*  API function to send GetAppSettingAttributeText Request from CT.
*    
*PARAMETERS
*   avrcp                   - Task
*   size_attributes         - Number of Bytes in attributes (1-255)
*   attributes              - Player application setting attribute ID for
*                             which the corresponding current set value 
*                             Text is requested
*RETURN
************************************************************************************/
void AvrcpGetAppSettingAttributeText(AVRCP *avrcp, 
                                     uint16 size_attributes, 
                                     Source attributes)
{
    uint8 extra_params[AVRCP_APP_NUM_ATTR_HDR_SIZE];
    avrcp_status_code status;

    extra_params[0] = size_attributes;

    status = avrcpMetadataStatusCommand(avrcp, 
                                    AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID, 
                                    avrcp_get_app_attribute_text, 
                                    AVRCP_APP_NUM_ATTR_HDR_SIZE, extra_params, 
                                    size_attributes,
                                    attributes);

    if (status != avrcp_success)
    {
        avrcpSendCommonFragmentedMetadataCfm(avrcp, status, 
                                            AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM,
                                            0, 0,  0);
    }
}

/*****************************************************************************/
void AvrcpGetAppSettingAttributeTextResponse(AVRCP *avrcp, 
                                        avrcp_response_type response, 
                                        uint16 number_of_attributes, 
                                        uint16 size_attributes, 
                                        Source attributes)
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->block_received_data == avrcp_get_app_attribute_text)
    {
        sendGetAttributeTextResponse(avrcp, response, 
                                    number_of_attributes, size_attributes,
                                    attributes);
    }
    else
        AVRCP_INFO(("AvrcpGetAppSettingAttributeTextResponse:"
                " CT not waiting for response\n"));
}


/****************************************************************************
*NAME    
*    AvrcpGetAppSettingValueText    
*
*DESCRIPTION
*  API function to send GetAppSettingValueText Request from CT.
*    
*PARAMETERS
*   avrcp                   - Task
*   attribute_id            - Player Attribute ID. 
*                             See Appendix F of AVRCP 1.4 Spec
*   size_values             - Number of Bytes in Values
*   Values                  - Player application setting value IDs for which the
*                             corresponding value string is needed
*
*RETURN
************************************************************************************/
void AvrcpGetAppSettingValueText(   AVRCP *avrcp, 
                                    uint16 attribute_id, 
                                    uint16 size_values, 
                                    Source values)
{
    uint8 extra_params[AVRCP_APP_VAL_TXT_HDR_SIZE];
    avrcp_status_code status;

    extra_params[0] = attribute_id;
    extra_params[1] = size_values;

    status = avrcpMetadataStatusCommand(avrcp, 
                                        AVRCP_GET_APP_VALUE_TEXT_PDU_ID, 
                                        avrcp_get_app_value_text,  
                                         AVRCP_APP_VAL_TXT_HDR_SIZE,
                                        extra_params,size_values,values);

    if (status != avrcp_success)
    {
        avrcpSendCommonFragmentedMetadataCfm(avrcp, status, 
                                              AVRCP_GET_APP_VALUE_TEXT_CFM, 
                                             0, 0, 0);
    }
}

/*****************************************************************************/
void AvrcpGetAppSettingValueTextResponse(AVRCP *avrcp, 
                                        avrcp_response_type response, 
                                        uint16 number_of_values,
                                        uint16 size_values, 
                                        Source values)
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->block_received_data == avrcp_get_app_value_text)
    {
        sendGetValueTextResponse(avrcp, response, number_of_values,
                                 size_values, values);
    }
    else
        AVRCP_INFO(("AvrcpGetAppSettingValueResponse:"
                " CT not waiting for response\n"));
}

/*****************************************************************************/
void AvrcpInformBatteryStatusOfCt(AVRCP *avrcp, 
                                  avrcp_battery_status battery_status)
{
    uint8 status_params[] = {0};
    avrcp_status_code status;

    status_params[0] = battery_status;

    status = avrcpMetadataControlCommand(avrcp,
                                         AVRCP_INFORM_BATTERY_STATUS_PDU_ID,
                                         avrcp_battery_information,
                                         sizeof(status_params),
                                         status_params,0,0);
    if (status != avrcp_success)
    {
        avrcpSendCommonMetadataCfm(avrcp, status, 
                                   AVRCP_INFORM_BATTERY_STATUS_CFM);
    }
}


/*****************************************************************************/
void AvrcpInformBatteryStatusOfCtResponse(AVRCP *avrcp,
                                         avrcp_response_type response)
{
    /* Only allow a response to be sent if the corresponding command arrived. */
    if (avrcp->block_received_data == avrcp_battery_information)
    {
        avrcpSendInformBatteryResponse(avrcp, response);       
    }
    else
        AVRCP_INFO(("AvrcpInformBatteryStatusOfCtResponse:"
                "CT not waiting for response\n"));
}

/*****************************************************************************/
void AvrcpInformDisplayableCharacterSet(AVRCP   *avrcp, 
                                        uint16  size_attributes, 
                                        Source  attributes)
{
    avrcp_status_code status;
    uint8 num_char_set[1];

    /* Validate the number of attributes are in pair */
    if(size_attributes % 2)
    {
        avrcpSendCommonMetadataCfm(avrcp, 
                                   avrcp_rejected_invalid_param, 
                                   AVRCP_INFORM_CHARACTER_SET_CFM);
        return;
    }

    num_char_set[0] = size_attributes/2;

    status = avrcpMetadataControlCommand(avrcp, 
                                         AVRCP_INFORM_CHARACTER_SET_PDU_ID,
                                         avrcp_character_set, 1, num_char_set,
                                         size_attributes, attributes);

    if (status != avrcp_success)
    {
        avrcpSendCommonMetadataCfm(avrcp, status, 
                                   AVRCP_INFORM_CHARACTER_SET_CFM);
    }
}

void AvrcpInformDisplayableCharacterSetResponse(AVRCP *avrcp, 
                                   avrcp_response_type response)
{
    /* Only allow a response to be sent if the corresponding command arrived. */
    if (avrcp->block_received_data == avrcp_character_set)
    {
        avrcpSendInformCharSetResponse(avrcp, response);     
    }
    else
        AVRCP_INFO(("AvrcpInformDisplayableCharacterSetResponse:"
               "CT not waiting for response\n"));
}
