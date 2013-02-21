/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_metadata_transfer.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_METADATA_TRANSFER_H_
#define AVRCP_METADATA_TRANSFER_H_


#include "avrcp_private.h"


/****************************************************************************
NAME	
	abortContinuation

DESCRIPTION
	Abort any stored continutation messages.
*/
void abortContinuation(AVRCP *avrcp);


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************
NAME	
	avrcpSendMetadataFailCfmToClient

DESCRIPTION
	Send a confirmation message to the application depending on the Metadata
    command sent.
*/
void avrcpSendMetadataFailCfmToClient(AVRCP *avrcp, avrcp_status_code status);
#endif

/****************************************************************************
NAME	
	avrcpHandleMetadataCommand

DESCRIPTION
	Handle a Metadata Transfer specific PDU command (encapsulated within a 
    vendordependent PDU).
*/
void avrcpHandleMetadataCommand(AVRCP *avrcp, Source source, uint16 packet_size);


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************
NAME	
	avrcpHandleMetadataResponse

DESCRIPTION
	Handle a Metadata Transfer specific PDU response (encapsulated within a 
    vendordependent PDU).
*/
void avrcpHandleMetadataResponse(AVRCP *avrcp, Source source, uint16 packet_size);
#endif

/****************************************************************************
NAME	
	prepareMetadataControlResponse

DESCRIPTION
	Prepare the Metadata response for a command type of CONTROL.
*/
void prepareMetadataControlResponse(AVRCP *avrcp, avrcp_response_type response, uint16 id);


/****************************************************************************
NAME	
	prepareMetadataStatusResponse

DESCRIPTION
	Prepare the Metadata response for a command type of STATUS.
*/
void prepareMetadataStatusResponse(AVRCP *avrcp, avrcp_response_type response, uint16 id, uint16 param_length, Source data_list, uint16 size_mandatory_data, uint8 *mandatory_data);


/****************************************************************************
NAME	
	sendMetadataResponse

DESCRIPTION
	Sends the Metadata response.
*/
void sendMetadataResponse(AVRCP *avrcp, avrcp_response_type response, uint8 pdu_id, Source caps_list, avrcp_packet_type metadata_packet_type, uint16 param_length, uint16 size_mandatory_data, uint8 *mandatory_data);


/****************************************************************************
NAME
    avrcpCreateMetadataTransferCmd

DESCRIPTION
    Allocates the PDU and sets up the Metadata transfer PDU header.
*/
uint8 *avrcpCreateMetadataTransferCmd(uint8 pdu_id, uint16 param_length, uint8 *params, uint16 extra_data_length, uint8 pkt_type, uint16 *length);


/****************************************************************************
NAME
    avrcpCreateMetadataTransferRsp

DESCRIPTION
    This function creates a metadata transfer response PDU. It claims the 
    space in the given sink and creates all necessary headers for the PDU.
    It returns a pointer to the correct place in the sink so the extra
    data can be copied in. The length of the extra data is passed in
    using param_length. This function also sets bytes_to_flush to the 
    total bytes that must be flushed from the sink once the PDU has
    been fully created.
*/
uint8 *avrcpCreateMetadataTransferRsp(AVRCP *avrcp, uint8 pdu_id, Sink sink, avrcp_response_type response, uint16 param_length, uint16 size_mandatory_data, avrcp_packet_type metadata_packet_type, uint16 *bytes_flush, avrcp_packet_type *packet_type, uint16 *continue_packets, uint8 transaction);


#endif /* AVRCP_METADATA_TRANSFER_H_ */
