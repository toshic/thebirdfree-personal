/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*****************************************************************************/


#include "avrcp_shim.h"

#include <panic.h>
#include <stream.h>
#include <stdlib.h>
#include <stdio.h>
#include <print.h>
#include <memory.h>


/* Static for now - need StreamRegionSource which actually owns the data */
static uint8 op_data[] = {
    0x31, 
    0x32, 
    0x33, 
    0x34, 
    0x45
};

static uint8 *gdata;

static TaskData cleanUpTask;

static void avrcpDataCleanUp(Task task, MessageId id, Message message)
{
    switch (id)
    {
    case MESSAGE_SOURCE_EMPTY:
        {
            if (gdata)
            {
                free(gdata);
                        
            }
        }
        break;

    default:
        break;
    }
}

static Source avrcpSourceFromData(AVRCP *avrcp, uint8 *data, uint16 length)
{
    uint16 i;
    Source src;

    if(!length || !data)
    {
        return 0;
    }
  
    gdata = (uint8*) malloc(length);
    for (i=0;i<length;i++)
        gdata[i] = data[i];
    
    src = StreamRegionSource(gdata, length);
    
    cleanUpTask.handler = avrcpDataCleanUp;

    MessageSinkTask(StreamSinkFromSource(src), &cleanUpTask);

    return src;
}

void AvrcpHandleComplexMessage(Task task, MessageId id, Message message)
{
    switch (id)
    {
        default:
            Panic();
            break;
    }
}

void AvrcpInitTestExtra(Task theAppTask, uint16 dev_type)
{
    avrcp_init_params config;
    config.device_type = dev_type;
    config.profile_extensions = AVRCP_EXTENSION_METADATA;

    if(dev_type & avrcp_target){

        config.supported_target_features = (0x0F) |
                    AVRCP_PLAYER_APPLICATION_SETTINGS|AVRCP_GROUP_NAVIGATION;
    }
    if(dev_type & avrcp_controller){
        config.supported_controller_features = 0x0f;
    }

    AvrcpInit(theAppTask, &config);
}


/*****************************************************************************/
void AvrcpInitLazyTestExtra(Task clientTask, Task connectionTask, uint16 dev_type)
{
    avrcp_init_params config;
    config.device_type = dev_type;

    config.profile_extensions = AVRCP_EXTENSION_METADATA;
    if(dev_type & avrcp_target){
        config.supported_target_features = AVRCP_PLAYER_APPLICATION_SETTINGS|AVRCP_GROUP_NAVIGATION;
    }
    if(dev_type & avrcp_controller){
        config.supported_controller_features = 0x0f;
    }

    AvrcpInitLazy(clientTask, connectionTask, &config);
}


/*****************************************************************************/
void AvrcpPassthroughTestExtra(AVRCP *avrcp, avc_subunit_type subunit_type, avc_subunit_id subunit_id, bool state, avc_operation_id opid)
{
    Source data_source = StreamRegionSource(op_data, 5);
    AvrcpPassthrough(avrcp, subunit_type, subunit_id, state, opid, 5, data_source);
}


/*****************************************************************************/
void AvrcpPassthroughVendorTestExtra(AVRCP *avrcp, avc_subunit_type subunit_type, avc_subunit_id subunit_id, bool state )
{
    Source data_source;
    avc_operation_id opid= opid_vendor_unique;
    uint8 *op_data = PanicUnlessMalloc(100);
    uint8 i, length=100;

    /* set some value instead of keeping junk */
    for(i=0; i< length; i++)
        op_data[i]=i;
    
    data_source = StreamRegionSource(op_data, length);
    AvrcpPassthrough(avrcp, subunit_type, subunit_id, state, opid, 100, data_source);
}


/*****************************************************************************/
void AvrcpVendorDependentTestExtra(AVRCP *avrcp, avc_subunit_type subunit_type, avc_subunit_id subunit_id, uint8 ctype, uint32 company_id)
{
    Source data_source = StreamRegionSource(op_data, 5);
    AvrcpVendorDependent(avrcp, subunit_type, subunit_id, ctype, company_id, 5, data_source);
}


/*****************************************************************************/
void AvrcpConnectTestNull(AVRCP *avrcp)
{
    AvrcpConnect(avrcp, 0);
}


/*****************************************************************************/
void AvrcpConnectLazyTestExtra(Task clientTask, const bdaddr *bd_addr, uint16 dev_type, uint16 priority)
{
    avrcp_init_params config;
    config.device_type = dev_type;

    AvrcpConnectLazy(clientTask, bd_addr, &config);
}


/*****************************************************************************/
void AvrcpConnectResponseLazyTestExtra(AVRCP *avrcp, uint16 connection_id, bool accept, uint16 dev_type)
{
    avrcp_init_params config;
    config.device_type = dev_type;

    AvrcpConnectResponseLazy(avrcp, connection_id, accept, &config);
}


/*****************************************************************************/
void AvrcpPassthroughTestNull(AVRCP *avrcp, avc_subunit_type subunit_type, avc_subunit_id subunit_id, bool state, avc_operation_id opid)
{
    AvrcpPassthrough(avrcp, subunit_type, subunit_id, state, opid, 0, 0);
}


/*****************************************************************************/
void AvrcpGetCapabilitiesResponseTestExtra(AVRCP *avrcp, avrcp_response_type response, avrcp_capability_id caps, uint16 size_caps_list, uint8* caps_list)
{
    Source pdu_src;

    pdu_src = avrcpSourceFromData(avrcp, caps_list, size_caps_list);
    AvrcpGetCapabilitiesResponse(avrcp, response, caps, size_caps_list, pdu_src);
}


/*****************************************************************************/
void AvrcpListAppSettingAttributeResponseTestExtra(AVRCP *avrcp, avrcp_response_type response, uint16 size_attributes, uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(avrcp, attributes, size_attributes);
    AvrcpListAppSettingAttributeResponse(avrcp, response, size_attributes, pdu_src);
}


/*****************************************************************************/
void AvrcpListAppSettingValueResponseTestExtra(AVRCP *avrcp, avrcp_response_type response, uint16 size_values, uint8 *values)
{
    Source pdu_src = avrcpSourceFromData(avrcp, values, size_values);
    AvrcpListAppSettingValueResponse(avrcp, response, size_values, pdu_src);
}


/*****************************************************************************/
void AvrcpGetCurrentAppSettingValueTestExtra(AVRCP *avrcp, uint16 size_attributes, uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(avrcp, attributes, size_attributes);
    AvrcpGetCurrentAppSettingValue(avrcp, size_attributes, pdu_src);
}


/*****************************************************************************/
void AvrcpGetCurrentAppSettingValueResponseTestExtra(AVRCP *avrcp, avrcp_response_type response, uint16 size_values, uint8 *values)
{
    Source pdu_src = avrcpSourceFromData(avrcp, values, size_values);
    AvrcpGetCurrentAppSettingValueResponse(avrcp, response, size_values, pdu_src);
}


/*****************************************************************************/
void AvrcpSetAppSettingValueTestExtra(AVRCP *avrcp, uint16 size_attributes, uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(avrcp, attributes, size_attributes);
    AvrcpSetAppSettingValue(avrcp, size_attributes, pdu_src);
}

/*****************************************************************************/
void AvrcpGetElementAttributesTestExtra(AVRCP *avrcp, uint32 identifier_high, uint32 identifier_low, uint16 size_attributes, uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(avrcp, attributes, size_attributes);
    AvrcpGetElementAttributes(avrcp, identifier_high, identifier_low, size_attributes, pdu_src);
}


/*****************************************************************************/
void AvrcpGetElementAttributesResponseTestExtra(AVRCP *avrcp, avrcp_response_type response, uint16 number_of_attributes, uint16 size_attributes, uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(avrcp, attributes, size_attributes);
    AvrcpGetElementAttributesResponse(avrcp, response, number_of_attributes, size_attributes, pdu_src);
}

/*****************************************************************************/
void AvrcpGetElementAttributesFragmentedResponseTestExtra(
                                      AVRCP               *avrcp,
                                      avrcp_response_type response,
                                      uint16              number_of_attributes) 
{
    uint8* attr_data;
    Source pdu_src;
    uint16 attr_len;
    uint16 length= 112;
 
    /* Allocate Memory greater than 502 to test Meta data fragmentation */
    attr_data = (uint8*)PanicUnlessMalloc(length);

    /* calculate the size of attribute values */
    attr_len = (length - 8);

    /* Fill the attributes */
    attr_data[0] =0x0;
    attr_data[1] =0x0;
    attr_data[2] =0x0;
    attr_data[3] =0x1;
    attr_data[4] = 0x00;
    attr_data[5] = 0x6A; /* Character Set - UTF-8 */
    attr_data[6] = attr_len >> 8;
    attr_data[7] = attr_len & 0xFF;
  
    gdata = attr_data;

    pdu_src = StreamRegionSource(gdata, length);
    
    cleanUpTask.handler = avrcpDataCleanUp;

    MessageSinkTask(StreamSinkFromSource(pdu_src), &cleanUpTask);

    AvrcpGetElementAttributesResponse(avrcp, response, 
                                      1, length , 
                                      pdu_src);
}

/*****************************************************************************/
void AvrcpEventPlayerAppSettingChangedResponseTestExtra(AVRCP *avrcp, avrcp_response_type response, uint16 size_attributes, uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(avrcp, attributes, size_attributes);
    AvrcpEventPlayerAppSettingChangedResponse(avrcp, response, size_attributes, pdu_src);
}

void AvrcpInformDisplayableCharacterSetTestExtra(AVRCP *avrcp, uint16 size_attributes, uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(avrcp, attributes, size_attributes);
    AvrcpInformDisplayableCharacterSet(avrcp, size_attributes, pdu_src);
}

void AvrcpGetAppSettingAttributeTextTestExtra(AVRCP *avrcp, uint16 size_attributes, uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(avrcp, attributes, size_attributes);
    AvrcpGetAppSettingAttributeText(avrcp, size_attributes, pdu_src);
}

void AvrcpGetAppSettingAttributeTextResponseTestExtra(AVRCP *avrcp, avrcp_response_type response, uint16 number_of_attributes, uint16 size_attributes, uint8 *attributes)
{
    Source pdu_src = avrcpSourceFromData(avrcp, attributes, size_attributes);
    AvrcpGetAppSettingAttributeTextResponse(avrcp, response, number_of_attributes, size_attributes, pdu_src);
}

void AvrcpGetAppSettingValueTextTestExtra(AVRCP *avrcp, uint16 attribute_id, uint16 size_values, uint8 *values)
{
    Source pdu_src = avrcpSourceFromData(avrcp, values, size_values);
    AvrcpGetAppSettingValueText(avrcp, attribute_id, size_values, pdu_src);
}

void AvrcpGetAppSettingValueTextResponseTestExtra(AVRCP *avrcp, avrcp_response_type response, uint16 number_of_values, uint16 size_values, uint8 *values)
{
    Source pdu_src = avrcpSourceFromData(avrcp, values, size_values);
    AvrcpGetAppSettingValueTextResponse(avrcp, response, number_of_values, size_values, pdu_src);
}

