/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_continuation.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "avrcp_common.h"


/****************************************************************************
*NAME    
*    AvrcpConnect    
*
*DESCRIPTION
*  API function to request Continuing Response.
*    
*PARAMETERS
*   avrcp            - Task
*   pdu_id           - PDU_ID of the received fragmented message which requires
*                      a continuation response. 
*
*RETURN
*  AVRCP_REQUEST_CONTINUING_RESPONSE_CFM 
*******************************************************************************/
void AvrcpRequestContinuing(AVRCP *avrcp, uint16 pdu_id)
{
    uint8 params[] = {0};
    avrcp_status_code status;

    params[0] = pdu_id & 0xFF;

    status = avrcpMetadataStatusCommand(avrcp, 
                                    AVRCP_REQUEST_CONTINUING_RESPONSE_PDU_ID,
                                    avrcp_request_continuation, 1, 
                                    params, 0,0);

    if (status != avrcp_success)
    {
        MAKE_AVRCP_MESSAGE(AVRCP_REQUEST_CONTINUING_RESPONSE_CFM);

        message->avrcp = avrcp;
        message->status = status;
        message->pdu_id = params[0];

#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
        message->transaction = 0;
#endif
        MessageSend(avrcp->clientTask, 
                    AVRCP_REQUEST_CONTINUING_RESPONSE_CFM, message);
    }
}


/****************************************************************************
*NAME    
*    AvrcpAbortContinuing    
*
*DESCRIPTION
*  API function to Abort Continuing Response.
*    
*PARAMETERS
*   avrcp            - Task
*   pdu_id           - PDU_ID of the received fragmented message to abort.
*
*RETURN
*  AVRCP_ABORT_CONTINUING_RESPONSE_CFM 
*******************************************************************************/
void AvrcpAbortContinuing(AVRCP *avrcp, uint16 pdu_id)
{
    uint8 params[] = {0};
    avrcp_status_code status;

    params[0] = pdu_id & 0xFF;

    status = avrcpMetadataStatusCommand(avrcp,
                                  AVRCP_ABORT_CONTINUING_RESPONSE_PDU_ID, 
                                  avrcp_abort_continuation, 1, params, 0, 0);

    if (status != avrcp_success)
    {
        avrcpSendCommonMetadataCfm(avrcp, status, 
                                   AVRCP_ABORT_CONTINUING_RESPONSE_CFM);
    }
}
