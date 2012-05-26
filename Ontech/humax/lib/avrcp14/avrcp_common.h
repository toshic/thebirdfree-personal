/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_common.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_COMMON_H_
#define AVRCP_COMMON_H_


#include "avrcp_private.h"
#include "avrcp_signal_handler.h"
#include "avrcp_send_response.h"

/* Macros */
/* Use the START PKT Header to set Continuation packet header */
#define AVCTP_SET_CONT_PKT_HEADER(new,old) \
            (new =(old & (~AVCTP0_PACKET_TYPE_MASK))|AVCTP0_PACKET_TYPE_CONTINUE)

/* Use Start packet header to set End packet header */
#define AVCTP_SET_END_PKT_HEADER(new, old)  \
            (new =(old & (~AVCTP0_PACKET_TYPE_MASK))| AVCTP0_PACKET_TYPE_END)

#define avrcpMapResponseCode(res,error) if(res==avctp_response_rejected) \
             res =avrcp_response_rejected_invalid_pdu + error;

#define avrcpMetadataEnabled(avrcp) \
            (avrcp->local_extensions & AVRCP_EXTENSION_METADATA)

#define avrcpMetadataAppSettingsEnabled(avrcp) \
            (avrcp->local_target_features & AVRCP_PLAYER_APPLICATION_SETTINGS)

/*  Update the avrcp state. */
#define avrcpSetState(avrcp,cur_state)  (avrcp->state = cur_state)

/* Generate the next transaction ID at CT Side */
#define AVRCP_NEXT_TRANSACTION(transaction) \
            (((transaction)==0x0F)? 1 :(transaction)+1);


/* Delete a dynamically allocated AVRCP task instance. Before deleting make 
   sure all messages for that task are flushed from the message queue.
*/
#define avrcpHandleDeleteTask(avrcp) {(void) MessageFlushTask(&avrcp->task); \
            free(avrcp); }

#define avrcpMetadataControlCommand avrcpSendMetadataCommand
#define avrcpMetadataStatusCommand  avrcpSendMetadataCommand


/****************************************************************************
NAME    
    avrcpFindNextTransactionLabel

DESCRIPTION
    Get the next transaction label but it doesn't actually increment 
    the transaction label. Note that a transaction label of zero has 
    special meaning as will therefore not be returned

RETURNS
    void
*/

uint8 avrcpFindNextTransactionLabel(AVRCP* avrcp);

/****************************************************************************
NAME    
    avrcpSendCommonCfmMessageToApp

DESCRIPTION
    Create a common cfm message (many messages sent to the app
    have the form of the message below and a common function can be used to
    allocate them). Send the message not forgetting to set the correct 
    message id.

RETURNS
    void
*/
void avrcpSendCommonCfmMessageToApp(uint16              message_id, 
                                    avrcp_status_code   status, 
                                    Sink                sink, 
                                    AVRCP               *avrcp);


/****************************************************************************
NAME    
    convertResponseToStatus

DESCRIPTION
    Convert response type to a status that the library uses.
*/
avrcp_status_code convertResponseToStatus(avrcp_response_type resp);


/****************************************************************************
NAME    
    avrcpGetNextTransactionLabel

DESCRIPTION
    Increments the transaction label and returns the new value. Note that
    a transaction label of zero has special meaning as will therefore not be
    returned from this function.
*/
uint8 avrcpGetNextTransactionLabel(AVRCP *avrcp);


/****************************************************************************
NAME
    avrcpGetCompanyId

DESCRIPTION
    Extract the company id from a data buffer.
*/
uint32 avrcpGetCompanyId(const uint8 *ptr, uint16 offset);


/****************************************************************************
NAME
    avrcpGrabSink

DESCRIPTION
    Get access to the Sink.
*/
uint8 *avrcpGrabSink(Sink sink, uint16 size);


/****************************************************************************
NAME
    avrcpSetAvctpHeader

DESCRIPTION
    Add the AVCTP packet header.
*/
void avrcpAvctpSetCmdHeader(AVRCP   *avrcp, 
                            uint8   *ptr, 
                            uint8   packet_type, 
                            uint8   total_packets);


/****************************************************************************
*NAME    
*    avrcpAvctpSendMessage    
*
*DESCRIPTION
*    This function is called to Frame the AVCTP messages (Command or Response)
*   and send it to the peer. Before calling this function, 
*   fill the AVRCP Message  Header in the sink.
*    
*RETURN
*   avrcp_status_code
*****************************************************************************/

avrcp_status_code avrcpAvctpSendMessage( AVRCP     *avrcp,
                                         uint8      cr_type,
                                         uint8     *ptr,
                                         uint16     hdr_size,
                                         uint16     data_len,
                                         Source     data);   


/****************************************************************************
*NAME    
*    avrcpAvctpReceiveMessage    
*
*DESCRIPTION
*    This function is called to process the AVCTP message received. 
*    
*RETURN
*  bool - Return TRUE if the message is ready to process, otherwise return FALSE
******************************************************************************/
bool avrcpAvctpReceiveMessage(  AVRCP          *avrcp,
                                const uint8*    ptr,
                                uint16          packet_size);



/****************************************************************************
NAME
    avrcpSourceProcessed

DESCRIPTION
    Called when the retrieved data source has been proccessed. So more data
    that has arrived over the air will be handled by the library until this
    is called.
*/
void avrcpSourceProcessed(AVRCP *avrcp, bool intern);


/****************************************************************************
* NAME    
*    avrcpGetErrorStatusCode
*
* DESCRIPTION
*   Responses to AVRCP Specific Browsing Commands and AVRCP Specific AV/C
*   Commands. contain 1 byte Error Status Code. This function extracts the
*   Error Status code from the application response
*   
******************************************************************************/
uint8 avrcpGetErrorStatusCode(avrcp_response_type *response, uint8 command_type);


/****************************************************************************
NAME
   convertUint8ValuesToUint32 

DESCRIPTION
    Combine 4 consecutive uint8 values into a uint32.
*/
uint32 convertUint8ValuesToUint32(const uint8 *ptr);


/****************************************************************************
NAME
    convertUint32ToUint8Values

DESCRIPTION
    Convert uint32 into 4 consecutive uint8 values.
*/
void convertUint32ToUint8Values(uint8 *ptr, uint32 value);



/****************************************************************************
NAME
    avrcpSendCommonMetadataCfm

DESCRIPTION
    
*/
void avrcpSendCommonMetadataCfm(AVRCP *avrcp, 
                                avrcp_status_code status, 
                                uint16 id);


/****************************************************************************
NAME
    avrcpSendCommonFragmentedMetadataCfm

DESCRIPTION
    
*/
bool  avrcpSendCommonFragmentedMetadataCfm(AVRCP        *avrcp, 
                                       avrcp_status_code status,
                                       uint16            id, 
                                       uint16            metadata_packet_type,
                                       uint16            data_length, 
                                       const uint8*      data);


/****************************************************************************
NAME
    avrcpSendCommonMetadataInd

DESCRIPTION
    
*/
void avrcpSendCommonMetadataInd(AVRCP *avrcp, uint16 id);


/****************************************************************************
NAME
    avrcpSendCommonFragmentedMetadataInd

DESCRIPTION
    
*/
void avrcpSendCommonFragmentedMetadataInd(  AVRCP     *avrcp, 
                                            uint16     id, 
                                            uint16     metadata_packet_type, 
                                            uint16     number_of_data_items, 
                                            uint16     data_length, 
                                            Source     source);

/****************************************************************************
NAME
   avrcpSourceFromConstData 

DESCRIPTION
    This function allocates a data block, copies the const data  and 
    returns a source. It also stored the ptr to the data block so it 
    can be freed when the source empties.
*/
Source avrcpSourceFromConstData(AVRCP *avrcp, const uint8 *data, uint16 length);


/****************************************************************************
NAME
    avrcpSourceFromData

DESCRIPTION
    This function takes an allocated data block and returns a source. It 
    also stored the ptr to the data block so it can freed when the
    source empties.
*/
Source avrcpSourceFromData(AVRCP *avrcp, uint8 *data, uint16 length);


/****************************************************************************
NAME
    avrcpSendMetadataCommand

DESCRIPTION
    Prepares and send Metadata command for to set the values at remote device.
*/
avrcp_status_code avrcpSendMetadataCommand(AVRCP        *avrcp, 
                                           uint16         id, 
                                           avrcpPending   pending, 
                                           uint16         extra_param_len,
                                           uint8*         extra_param,
                                           uint16         data_size, 
                                           Source         data);


#endif /* AVRCP_COMMON_H_ */
