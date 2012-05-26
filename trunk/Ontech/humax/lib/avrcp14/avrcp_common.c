/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_common.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "avrcp_private.h"
#include "avrcp_common.h"

/****************************************************************************
* NAME    
*   avrcpGetNextTransactionLabel    
*
* DESCRIPTION
*  Get the next transaction label 
*
* RETURNS
*    void
*******************************************************************************/
uint8 avrcpGetNextTransactionLabel(AVRCP *avrcp)
{
    /*
        Returns the next transaction label. This is used to route responses 
        from the remote device to the correct local initiator. Note that 
        zero will never be issued and hence can be used as a special case to 
        indicate no command is pending.
    */
   avrcp->cmd_transaction_label=AVRCP_NEXT_TRANSACTION(
                                    avrcp->cmd_transaction_label);

   return avrcp->cmd_transaction_label;
}

/****************************************************************************
* NAME    
*   avrcpFindNextTransactionLabel    
*
* DESCRIPTION
*  Get the next transaction label but it doesn't actually increment 
*    the transaction label. Note that a transaction label of zero has 
*    special meaning as will therefore not be returned
*
* RETURNS
*    void
*******************************************************************************/

uint8 avrcpFindNextTransactionLabel(AVRCP* avrcp)
{
    return AVRCP_NEXT_TRANSACTION(avrcp->cmd_transaction_label);
}

/****************************************************************************
NAME    
    avrcpSendCommonCfmMessageToApp

DESCRIPTION
    Create a common CFM message (many messages sent to the app
    have the form of the message below and a common function can be used to
    allocate them). Send the message not forgetting to set the correct 
    message id.

RETURNS
    void
******************************************************************************/
void avrcpSendCommonCfmMessageToApp(uint16              message_id,
                                    avrcp_status_code   status, 
                                    Sink                sink, 
                                    AVRCP               *avrcp)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_CFM_MESSAGE);
    message->status = status;
    message->sink = sink;
    message->avrcp = avrcp;
    MessageSend(avrcp->clientTask, message_id, message);

    if((message_id == AVRCP_CONNECT_CFM) && 
       (status != avrcp_success) && avrcp->lazy)
        MessageSend(&avrcp->task, AVRCP_INTERNAL_TASK_DELETE_REQ, 0);
}

/****************************************************************************
* NAME    
*   convertResponseToStatus    
*
* DESCRIPTION
*  Map the received response code to a corresponding Status value.
*
* RETURNS
*    void
*******************************************************************************/
avrcp_status_code convertResponseToStatus(avrcp_response_type resp)
{
    /* Turn a response into a simplified state that the AVRCP library
     can return to the application. */
    switch (resp)
    {
    case avctp_response_accepted:
    case avctp_response_in_transition:
    case avctp_response_stable:
    case avctp_response_changed:
        return avrcp_success;
    case avctp_response_not_implemented:
    case avctp_response_bad_profile:
        return avrcp_unsupported;
    case avctp_response_interim:
        return avrcp_interim_success;
    case avctp_response_rejected:
        return avrcp_rejected;
    default:
        return avrcp_fail;
    }    
}


/*****************************************************************************/
uint32 avrcpGetCompanyId(const uint8 *ptr, uint16 offset)
{
    uint32 cid = 0;
    uint32 cid_tmp = 0;

    cid_tmp = (uint32) ptr[offset];
    cid = (cid_tmp << 16);
    cid_tmp = (uint32) ptr[offset+1];
    cid |= (cid_tmp << 8);
    cid_tmp = (uint32) ptr[offset+2];
    cid |= cid_tmp;

    return cid;
}


/*****************************************************************************/
uint8 *avrcpGrabSink(Sink sink, uint16 size)
{
    uint8 *dest = SinkMap(sink);
    uint16 claim_result = SinkClaim(sink, size);
    if (claim_result == 0xffff)
    {
        AVRCP_DEBUG(("SinkClaim return Invalid Offset"));
        return NULL;
    }

    return (dest + claim_result);
}

/****************************************************************************
* NAME    
*   avrcpSourceProcessed    
*
* DESCRIPTION
*  Free all memory resources. 
*
*  PARAMETERS
*  intern - TRUE - Call initiated by internal function. Free all resources.
*           FALSE- Call Initiated by app. Free only the external resource if
*                  other shared resources are not yet processed by the library.
*
* RETURNS
*    void
*******************************************************************************/
void avrcpSourceProcessed(AVRCP *avrcp, bool intern)
{
    if(intern || !avrcp->block_received_data)
    {
        Source source = StreamSourceFromSink(avrcp->sink);

        if(avrcp->av_msg_len)
        {
            /* lib allocates memory only for start packets */
            if((avrcp->av_msg[AVCTP_HEADER_START_OFFSET] & 
                AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START)
            {
                free(avrcp->av_msg);
            }
            avrcp->av_msg = NULL;
            avrcp->av_msg_len = 0;
        }
    
        if (avrcp->srcUsed)
        {
            SourceDrop(source, avrcp->srcUsed);
            avrcp->srcUsed = 0;
        }
        if(SourceBoundary(source))
        {
            /* There is data in the L2CAP sink to be processed.
             Message Internally to trigger*/
            MessageSend(&avrcp->task, AVRCP_INTERNAL_MESSAGE_MORE_DATA, 0);
        }
    }

    /* Free the data source provided to the application */
    if(avrcp->data_app_ind)
    {
        SourceEmpty(avrcp->data_app_ind);
        avrcp->data_app_ind = 0;
    }

}


/****************************************************************************
* NAME    
*    avrcpGetErrorStatusCode
*
* DESCRIPTION
*   Responses to AVRCP Specific Browsing Commands and AVRCP Specific 
*   AV/C Commands. Contain 1 byte Error Status Code. This function extracts
*   the Error Status code from the application response. Append the error
*   code to the pdu if required.
*
* PARAMETERS
*    *response       - Application Response , standardised by this function
*   command_type    - Should be one of the values AVRCP0_CTYPE_CONTROL, 
*                        AVRCP0_CTYPE_STATUS or AVRCP0_CTYPE_NOTIFY
*
* RETURNS
*    void
********************************************************************************/
uint8 avrcpGetErrorStatusCode(avrcp_response_type *response, uint8 command_type)
{
    uint8 error_code = AVRCP_STATUS_SUCCESS;

    /* if response value is any specific rejected error response values,
       convert the response to avrcp_response_rejected after extracting the 
       error status  code from  the response value, otherwise standardize the
       response.*/

    if(*response < avrcp_response_rejected_invalid_pdu)
    {
        switch(*response)
        {
        case avctp_response_accepted:
        case avctp_response_changed: /* fall through */
        case avctp_response_stable: /* fall through */
            
            if(command_type == AVRCP0_CTYPE_STATUS)
            {
                *response = avctp_response_stable;
            }
            else if(command_type == AVRCP0_CTYPE_CONTROL)
            {
                *response = avctp_response_accepted;
            }
            else
            {
                *response = avctp_response_changed;
            }
            break;
        
        case avctp_response_interim:
            /* accept */
            break;
        case avctp_response_rejected:
            error_code = AVRCP_STATUS_INTERNAL_ERROR;
            break;
        default:
            /* ignore */
            break;            
        }
    }
    else
    {
        /* return correspondent status code */
        error_code = *response & AVRCP_RESPONSE_ERROR_STATUS_MASK;
        *response = avctp_response_rejected;
    }

    return error_code;

}

/*****************************************************************************/
uint32 convertUint8ValuesToUint32(const uint8 *ptr)
{
    return ((((uint32)ptr[0] << 24) & 0xFF000000) | 
            (((uint32)ptr[1] << 16) & 0x00FF0000) | 
            (((uint32)ptr[2] << 8) & 0x0000FF00)  | 
            ((uint32)ptr[3] & 0x000000FF));
}


/*****************************************************************************/
void convertUint32ToUint8Values(uint8 *ptr, uint32 value)
{
    ptr[0] = (value >> 24) & 0xFF;
    ptr[1] = (value >> 16) & 0xFF;
    ptr[2] = (value >> 8) & 0xFF;
    ptr[3] = value & 0xFF;
}

/****************************************************************************
* NAME    
* avrcpSendCommonMetadataCfm    
*
* DESCRIPTION
*
* PARAMETERS
*    *response       - Application Response , standardised by this function
*   command_type    - Should be one of the values AVRCP0_CTYPE_CONTROL, 
*                        AVRCP0_CTYPE_STATUS or AVRCP0_CTYPE_NOTIFY
*
* RETURNS
*    void
*******************************************************************************/
void avrcpSendCommonMetadataCfm(AVRCP *avrcp, 
                                avrcp_status_code status, 
                                uint16 id)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_METADATA_CFM_MESSAGE);

    message->avrcp = avrcp;
    
    message->status = status;

#ifdef AVRCP_ENABLE_DEPRECATED
    if(avrcp->av_msg)
    {
        message->transaction = (avrcp->av_msg[AVCTP_HEADER_START_OFFSET] >> 
                                AVCTP0_TRANSACTION_SHIFT);
    }
#endif
    
    MessageSend(avrcp->clientTask, id, message);
}
/****************************************************************************
* NAME    
* avrcpSendCommonFragmentedMetadataCfm    
*
* DESCRIPTION
*   Send MetaData confirm to the CT application with the extracted data from 
*   the response
*
* RETURNS
*    void
*******************************************************************************/
bool  avrcpSendCommonFragmentedMetadataCfm(AVRCP          *avrcp, 
                                        avrcp_status_code status,
                                        uint16            id, 
                                        uint16            metadata_packet_type,
                                        uint16            data_length, 
                                      const uint8*        data)
{
    bool source_used = FALSE;
    uint16 offset=0;

    {
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_FRAGMENTED_METADATA_CFM);
    
    message->avrcp = avrcp;
    message->status = status;

#ifdef AVRCP_ENABLE_DEPRECATED
    if(avrcp->av_msg)
    {
        message->transaction = (avrcp->av_msg[AVCTP_HEADER_START_OFFSET] >> 
                                AVCTP0_TRANSACTION_SHIFT);
    }
    else
    {
        message->transaction =0;
    }

    message->no_packets = 1;
    message->ctp_packet_type = AVCTP0_PACKET_TYPE_SINGLE;
    message->data_offset = 0;
#endif

    message->metadata_packet_type = metadata_packet_type;
    message->number_of_data_items = 0;

    if(data_length)
    {
        if((status == avrcp_rejected))
        {
            /* The first byte is error code */
            message->status = data[0] + AVRCP_ERROR_STATUS_BASE;
            offset=1;
        }
        else if ((metadata_packet_type == avrcp_packet_type_single) || 
                 (metadata_packet_type == avrcp_packet_type_start))
        {
            /* Extract the first 2 bytes which is capability ID and capability length */
            offset=1;
            message->number_of_data_items = data[0];
        }
    }

            
    if(data_length > offset)
    {        
        message->size_data = data_length-offset;
        message->data = avrcpSourceFromConstData(avrcp, data+offset, message->size_data);
         source_used = TRUE;
    }
    else
    {
        message->data=0;
        message->size_data=0;
    }

    MessageSend(avrcp->clientTask, id, message);
    }

    return source_used;
}
/****************************************************************************
* NAME    
* avrcpSendCommonMetadataInd    
*
* DESCRIPTION
*   Send MetaData Indication to the TG application if there was no data 
*   received part of Metadata request message.
*
* RETURNS
*    void
*******************************************************************************/
void avrcpSendCommonMetadataInd(AVRCP *avrcp, uint16 id)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_METADATA_IND_MESSAGE);

    message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED
    message->transaction = avrcp->rsp_transaction_label;
#endif

    MessageSend(avrcp->clientTask, id, message);
}

/****************************************************************************
* NAME    
* avrcpSendCommonFragmentedMetadataInd    
*
* DESCRIPTION
*   Send MetaData Indication to the TG application if there was data 
*   received part of Metadata request message.
*
* RETURNS
*    void
*******************************************************************************/
void avrcpSendCommonFragmentedMetadataInd(    AVRCP     *avrcp, 
                                            uint16     id, 
                                            uint16     metadata_packet_type, 
                                            uint16     number_of_data_items, 
                                            uint16     data_length, 
                                            Source     source)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_FRAGMENTED_METADATA_IND);

    message->avrcp = avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED
    message->transaction = avrcp->rsp_transaction_label;
    message->no_packets = 1; /* Deprecated */
    message->ctp_packet_type = AVCTP0_PACKET_TYPE_SINGLE;
    message->data_offset = 0; /* Deprecated */
#endif

    message->metadata_packet_type = metadata_packet_type;
    message->number_of_data_items = number_of_data_items;
    message->size_data = data_length;
    message->data = source;

    MessageSend(avrcp->clientTask, id, message);
}
/****************************************************************************
* NAME    
* avrcpSourceFromConstData    
*
* DESCRIPTION
*  Create a Source by allocating memory and copying data from constant memory.   
*
* RETURNS
*    void
*******************************************************************************/
Source avrcpSourceFromConstData(AVRCP *avrcp, const uint8 *data, uint16 length)
{
    uint8* ptr;
    Source src;
    /* allocate memory */

    ptr = (uint8*)malloc(length);

    if(!ptr)
        return 0;

    memmove(ptr, data, length);

    /* Create a source from the data */
    src = StreamRegionSource(ptr, length);

    /* Register a task for freeing the data and store a ptr to it */
    avrcp->dataFreeTask.sent_data = ptr;
    avrcp->data_app_ind = src;
    MessageSinkTask(StreamSinkFromSource(src),&avrcp->dataFreeTask.cleanUpTask);

    return src;
}
/****************************************************************************
* NAME    
* avrcpSourceFromData    
*
* DESCRIPTION
*  Create a Source from the passed memory location.
*
* RETURNS
*    void
*******************************************************************************/
Source avrcpSourceFromData(AVRCP *avrcp, uint8 *data, uint16 length)
{
    /* Create a source from the data */
    Source src = StreamRegionSource(data, length);

    /* Register a task for freeing the data and store a ptr to it */
    avrcp->dataFreeTask.sent_data = data;
    MessageSinkTask(StreamSinkFromSource(src), &avrcp->dataFreeTask.cleanUpTask);

    return src;
}


