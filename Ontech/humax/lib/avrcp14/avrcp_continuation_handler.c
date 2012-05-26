/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_continuation_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "avrcp_continuation_handler.h"
#include "avrcp_metadata_transfer.h"



/*****************************************************************************/
void avrcpHandleRequestContinuingCommand(AVRCP *avrcp, uint16 pdu)
{
    /* Make sure the continuing PDU requested is the same as the one we 
       are storing data for. */
    if (avrcp->continuation_pdu == pdu)
    {
        /* A message will be sent on this variable being reset, 
           if there is more data to be sent. */
        avrcp->continuation_pdu = 0;
    }
    else
    {
        avrcpSourceProcessed(avrcp,TRUE);
    }
    
}


/*****************************************************************************/
void avrcpHandleAbortContinuingCommand(AVRCP *avrcp, uint16 pdu)
{
    /* Send response back stating if the abort was successful. */
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_ABORT_CONTINUING_RES);

    if (avrcp->continuation_pdu == pdu)
    {
        /* Forget about the data stored. */
        abortContinuation(avrcp);
        message->response = avctp_response_accepted;
    }
    else
        /* The abort continuing PDU requested is not the same as the
           one we are storing data for, so reject. */
        message->response = avctp_response_rejected;

    MessageSend(&avrcp->task, AVRCP_INTERNAL_ABORT_CONTINUING_RES, message);
}


/*****************************************************************************/
void avrcpHandleInternalAbortContinuingResponse(AVRCP       *avrcp, 
                const AVRCP_INTERNAL_ABORT_CONTINUING_RES_T *res)
{
    /* Send a response to the abort continuing request. */
    avrcpSendMetadataResponse(avrcp,  res->response, 
                              AVRCP_ABORT_CONTINUING_RESPONSE_PDU_ID, 0, 
                              avrcp_packet_type_single, 0, 0, 0);

}


/*****************************************************************************/
void avrcpStoreNextContinuationPacket(  AVRCP   *avrcp, 
                                        Source  data, 
                                        uint16  param_length, 
                                        uint16  pdu_id, 
                                        uint16  response, 
                                        uint8 transaction)
{
    /* Store futher fragments until the CT request them. */
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET);
    message->data = data;
    message->param_length = param_length;
    message->pdu_id = pdu_id;
    message->response = response;
    message->transaction = transaction;
    
    avrcp->continuation_pdu = pdu_id;
    avrcp->continuation_data = data;

    MessageSendConditionally(&avrcp->task,
                             AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET,
                             message, &avrcp->continuation_pdu);
}


/*****************************************************************************/
void avrcpHandleNextContinuationPacket(AVRCP                    *avrcp, 
                const AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET_T *ind)
{
    /* The next fragment has been requested, so send it. */
    uint16 data_length = AVRCP_AVC_MAX_DATA_SIZE;
    avrcp_packet_type packet_type = avrcp_packet_type_continue;

    if (ind->param_length < AVRCP_AVC_MAX_DATA_SIZE)
    {
        /* This is the last fragment to be sent. */
        data_length = ind->param_length;
        packet_type = avrcp_packet_type_end;
        avrcp->continuation_pdu = ind->pdu_id;
    }
    else
    {
        /* There are more fragments to be sent, 
            store the data for the following fragments. */
        avrcpStoreNextContinuationPacket(avrcp, 
                               ind->data, 
                               ind->param_length - AVRCP_AVC_MAX_DATA_SIZE,
                               ind->pdu_id,
                               ind->response, ind->transaction);
    }

    if (ind->response)
    {
        /* This is a fragmented response. */
        avrcpSendMetadataResponse(avrcp,  ind->response, 
                                  ind->pdu_id, ind->data, 
                                  packet_type, data_length, 0, 0);

    }
    else
    {
        /* This is a fragmented command. */
        avrcp->identifier = avrcpCreateMetadataTransferCmd(ind->pdu_id, 
                                                     data_length, NULL, 0,
                                                     packet_type);

        AvrcpVendorDependent(avrcp, subunit_panel, 0x00, 
                             AVRCP0_CTYPE_STATUS, AVRCP_BT_COMPANY_ID, 
                             data_length, ind->data);
    }
}
