/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2


FILE NAME
    avrcp_unit_subunit.c      

DESCRIPTION
    This file defines the APIs for AVRCP UNIT_INFO and SUBUNIT_INFO commands.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "avrcp_signal_unit_info.h"


/****************************************************************************
NAME    
    AvrcpUnitInfo

DESCRIPTION
    This function is called to request that a UnitInfo control command
    is sent to the target on the connection identified by the sink.
    
    The UnitInfo command is used to obtain information that pertains to the
    AV/C unit as a whole
    
    
                     -----------------------------------------------
                    | MSB |    |    |    |    |     |      |    LSB |
                    |-----------------------------------------------|
    opcode          |               UNITINFO (0x30)                 |
                     -----------------------------------------------
    operand[0]      |                      0xFF                     |
                     -----------------------------------------------
    operand[1]      |                      0xFF                     |
                     -----------------------------------------------
    operand[2]      |                      0xFF                     |
                     -----------------------------------------------
    operand[3]      |                      0xFF                     |
                     -----------------------------------------------
    operand[4]      |                      0xFF                     |
                     -----------------------------------------------

MESSAGE RETURNED
    AVRCP_UNITINFO_CFM - This message contains the unit_type and a
    unique 24-bit Company ID
*/

/*****************************************************************************/
void AvrcpUnitInfo(AVRCP *avrcp)
{
    if (avrcp->dataFreeTask.sent_data || avrcp->block_received_data || 
        avrcp->pending)
        avrcpSendUnitInfoCfmToClient(avrcp, avrcp_busy, 0, 0, (uint32) 0);
    else if (!avrcp->sink)
        /* Immediately reject the request if we have not been
           passed a valid sink */
        avrcpSendUnitInfoCfmToClient(avrcp, avrcp_invalid_sink,
                                     0, 0, (uint32) 0);
    else
    {
        MessageSend(&avrcp->task, AVRCP_INTERNAL_UNITINFO_REQ, 0);
    }
}

/*****************************************************************************/
void AvrcpUnitInfoResponse(AVRCP *avrcp,
                           bool accept, 
                           avc_subunit_type unit_type, 
                           uint8 unit,
                           uint32 company_id)
{
    sendUnitInfoResponse(avrcp, accept, unit_type, unit, company_id);
}



/*****************************************************************************/
/*lint -e818 -e830 */
void AvrcpSubUnitInfo(AVRCP *avrcp, uint8 page)
{
    
#ifdef AVRCP_DEBUG_LIB    
    if (page > 0x07)
    {
        AVRCP_DEBUG(("Out of range page  0x%x\n", page));
    }
#endif
    
    if (avrcp->dataFreeTask.sent_data || 
        avrcp->block_received_data || avrcp->pending)
        avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_busy, 0, 0);
    else if (!avrcp->sink)
        /* Immediately reject the request if we have not
         been passed a valid sink */
        avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_invalid_sink, 0, 0);
    else
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SUBUNITINFO_REQ);
        message->page = page;
        MessageSend(&avrcp->task, AVRCP_INTERNAL_SUBUNITINFO_REQ, message);
    }    
}
/*lint +e818 +e830 */


/****************************************************************************
NAME    
    AvrcpSubUnitInfoResponse

DESCRIPTION    
    The SubUnitInfo command is used to obtain information about the subunit(s)
    of a device.
    
    accept                    - Flag accepting or rejecting request for
                                 SubUnitInfo
    page_data                - Four entries from the subunit table for
                               the requested   page on the target device
                              
            
MESSAGE RETURNED

*/
/*****************************************************************************/
void AvrcpSubUnitInfoResponse(AVRCP *avrcp, bool accept, const uint8 *page_data)
{
    sendSubunitInfoResponse(avrcp, accept, page_data);
}



