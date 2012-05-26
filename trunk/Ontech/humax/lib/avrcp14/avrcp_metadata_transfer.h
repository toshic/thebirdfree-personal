/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_metadata_transfer.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_METADATA_TRANSFER_H_
#define AVRCP_METADATA_TRANSFER_H_

#include "avrcp_common.h"

/****************************************************************************
NAME    
    abortContinuation

DESCRIPTION
    Abort any stored continutation messages.
*/
void abortContinuation(AVRCP *avrcp);


/****************************************************************************
NAME    
    avrcpSendMetadataFailCfmToClient

DESCRIPTION
    Send a confirmation message to the application depending on the Metadata
    command sent.
*/
void avrcpSendMetadataFailCfmToClient(AVRCP *avrcp, avrcp_status_code status);


/****************************************************************************
NAME    
    avrcpHandleMetadataCommand

DESCRIPTION
    Handle a Metadata Transfer specific PDU command (encapsulated within a 
    vendordependent PDU).
*/
void avrcpHandleMetadataCommand(AVRCP       *avrcp, 
                                const uint8 *ptr, 
                                uint16      packet_size);

/****************************************************************************
NAME    
    avrcpHandleMetadataResponse

DESCRIPTION
    Handle a Metadata Transfer specific PDU response (encapsulated within a 
    vendordependent PDU).
*/

void avrcpHandleMetadataResponse(   AVRCP       *avrcp, 
                                    const uint8 *ptr, 
                                    uint16      packet_size);

/****************************************************************************
NAME    
    prepareMetadataControlResponse

DESCRIPTION
    Prepare the Metadata response for a command type of CONTROL.
*/
void prepareMetadataControlResponse(AVRCP               *avrcp,
                                    avrcp_response_type response,
                                    uint16              id);


/****************************************************************************
NAME    
    prepareMetadataStatusResponse

DESCRIPTION
    Prepare the Metadata response for a command type of STATUS.
*/
void prepareMetadataStatusResponse(AVRCP                *avrcp,
                                 avrcp_response_type    response, 
                                 uint16                 id, 
                                 uint16                 param_length, 
                                 Source                 data_list, 
                                 uint16                 size_mandatory_data, 
                                 uint8                  *mandatory_data);


/****************************************************************************
NAME    
    avrcpSendMetadataResponse

DESCRIPTION
    Prepare and send the Metadata response.
*/
void avrcpSendMetadataResponse(AVRCP                *avrcp,
                               avrcp_response_type   response,
                               uint8                 pdu_id, 
                               Source                caps_list, 
                               avrcp_packet_type     metadata_packet_type, 
                               uint16                param_length, 
                               uint16                size_mandatory_data, 
                               uint8                *mandatory_data);


/****************************************************************************
NAME
    avrcpCreateMetadataTransferCmd

DESCRIPTION
    Allocates the PDU and sets up the Metadata transfer PDU header.
*/
uint8 *avrcpCreateMetadataTransferCmd(uint8     pdu_id, 
                                      uint16    param_length, 
                                      uint8     *params, 
                                      uint16    extra_data_length, 
                                      uint8     pkt_type);

/****************************************************************************
NAME
   avrcpSendRejectMetadataResponse 

DESCRIPTION
    Post a message to send reject response for the received metadata command.
*/
void avrcpSendRejectMetadataResponse(AVRCP              *avrcp, 
                                    avrcp_response_type response, 
                                    uint16              id);

/****************************************************************************
NAME
   avrcpHandleInternalRejectMetadataResponse 

DESCRIPTION
   Internal function to send metadata reject response.
*/
void avrcpHandleInternalRejectMetadataResponse(AVRCP              *avrcp, 
                                              avrcp_response_type response, 
                                              uint16              id);


#endif /* AVRCP_METADATA_TRANSFER_H_ */
