/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2


FILE NAME
    avrcp_vendor.c

DESCRIPTION
    This file defines the APIs for Vendor Specific commands which includes
    common interface function for sending Vendor specific AVC commands.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "avrcp_signal_vendor.h"
#include "avrcp_metadata_transfer.h"

/****************************************************************************
NAME    
    AvrcpVendorDependent    

DESCRIPTION
    This function is called to send vendor specific data to the peer entity.
    If the data_length is greater than the l2cap mtu then the message
    become fragmented.
    
    subunit_type
    subunit_id                - AV/C protocol - Used to form the targets address
    company_id                - 24-bit unique ID obtained from IEEE RAC.
    data                    - Required for the Vendor Unique operation.   
    data_length               
    
                     -----------------------------------------------
                    | MSB |     |    |     |    |    |    |    LSB  |
                    |-----------------------------------------------|
    opcode          |              VENDOR-DEPENDENT (0x00)          |
                     -----------------------------------------------
    operand(0)      |MSB                                            |
    operand(1)      |                     company_id                |
    operand(2)      |                                        LSB    |
                    |-----------------------------------------------| 
    operand(3)      |                                               |
        :           |                vendor_dependent_data          |
    operand(n)      |                                              | 
                     -----------------------------------------------

MESSAGE RETURNED
    AVRCP_VENDORDEPENDENT_CFM is returned indicating the status 
    of the connection request
   
*/
/*lint -e818 -e830 */
void AvrcpVendorDependent(AVRCP *avrcp, 
                          avc_subunit_type subunit_type, 
                          avc_subunit_id subunit_id, 
                          uint8 ctype, 
                          uint32 company_id, 
                          uint16 data_length, 
                          Source data)
{
    
#ifdef AVRCP_DEBUG_LIB    
    if (subunit_type > 0x1F)
    {
        AVRCP_DEBUG(("Out of range subunit type  0x%x\n", subunit_type));
    }
    if (subunit_id > 0x07)
    {
        AVRCP_DEBUG(("Out of range subunit id  0x%x\n", subunit_id));
    }
    if (company_id > 0xFFFFFF)
    {
        AVRCP_DEBUG(("Out of range company id  0x%lx\n", company_id));
    }
#endif
    
    if (avrcp->block_received_data || (avrcp->pending && 
        (avrcp->pending < avrcp_get_caps)))
    {
        avrcpSendVendordependentCfmToClient(avrcp, avrcp_busy, 0);
    }
    else if (!avrcp->sink)
    {
        /* Immediately reject the request if we have not been passed a
          valid sink */
        if (avrcp->pending >= avrcp_get_caps)
        {
            avrcpSendMetadataFailCfmToClient(avrcp, avrcp_invalid_sink);
        }
        else
        {
            avrcpSendVendordependentCfmToClient(avrcp, avrcp_invalid_sink, 0);
        }
    }
    else
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_VENDORDEPENDENT_REQ);
        message->company_id = company_id;
        message->subunit_type = subunit_type;
        message->subunit_id = subunit_id;
        message->ctype = ctype;
        message->data = data;
        message->data_length = data_length;
        MessageSend(&avrcp->task, AVRCP_INTERNAL_VENDORDEPENDENT_REQ, message);
    }
}
/*lint +e818 +e830 */


/*****************************************************************************/
void AvrcpVendorDependentResponse(AVRCP *avrcp, avrcp_response_type response)
{
    sendVendorDependentResponse(avrcp, response);
}

