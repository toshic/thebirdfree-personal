/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_common.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_COMMON_H_
#define AVRCP_COMMON_H_


#include "avrcp_private.h"
/****************************************************************************
NAME	
	avrcpSetState

DESCRIPTION
	Update the avrcp state.
*/
void avrcpSetState(AVRCP *avrcp, avrcpState state);


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
void avrcpSendCommonCfmMessageToApp(uint16 message_id, avrcp_status_code status, Sink sink, AVRCP *avrcp);


/****************************************************************************
NAME	
	convertResponseToStatus

DESCRIPTION
	Convert response type to a status that the library uses.
*/
avrcp_status_code convertResponseToStatus(avrcp_response_type resp);


/****************************************************************************
NAME	
	avrcpHandleDeleteTask

DESCRIPTION
	Detele a dynamically allocated AVRCP task instance. Before deleting make 
    sure all messages for that task are flushed from the message queue.
*/
void avrcpHandleDeleteTask(AVRCP *avrcp);


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
	avrcpFindNextTransactionLabel

DESCRIPTION
	Returns what the next transaction label will be but doesn't actually increment 
	the transaction label. Note that a transaction label of zero has special meaning 
	as will therefore not be returned from this function.
*/
uint8 avrcpFindNextTransactionLabel(AVRCP *avrcp);


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
void avrcpSetAvctpHeader(AVRCP *avrcp, uint8 *ptr, uint8 packet_type, uint8 total_packets);


/****************************************************************************
NAME
    avrcpSourceProcessed

DESCRIPTION
    Called when the retrieved data source has been proccessed. So more data that has arrived
	over the air will be handled by the library until this is called.
*/
void avrcpSourceProcessed(AVRCP *avrcp);


/****************************************************************************
NAME
    avrcpMetadataEnabled

DESCRIPTION
    Make sure Metadata is enabled at either end, or both ends
*/
bool avrcpMetadataEnabled(AVRCP *avrcp);


/****************************************************************************
NAME
    avrcpMetadataAppSettingsEnabled

DESCRIPTION
    Make sure Metadata app settings is enabled at either end, or both ends
*/
bool avrcpMetadataAppSettingsEnabled(AVRCP *avrcp);


/****************************************************************************
NAME
    getRejectCode

DESCRIPTION
    Set reject code to include a reason if one specified.
*/
avrcp_status_code getRejectCode(AVRCP *avrcp, uint16 data_length, avrcp_status_code status, Source source);


/****************************************************************************
NAME
    insertRejectCode

DESCRIPTION
    Amend reject code to include a reason if one specified.
*/
uint8 *insertRejectCode(avrcp_response_type *response, uint16 *size_mandatory_data, uint16 *param_length);


/****************************************************************************
NAME
    standardiseCtypeStatus

DESCRIPTION
    Convert command type of STATUS into something reasonable if the app supplied an invalid response.
*/
void standardiseCtypeStatus(avrcp_response_type *response);


/****************************************************************************
NAME
    standardiseCtypeControl

DESCRIPTION
    Convert command type of CONTROL into something reasonable if the app supplied an invalid response.
*/
void standardiseCtypeControl(avrcp_response_type *response);


/****************************************************************************
NAME
    standardiseCtypeNotify

DESCRIPTION
    Convert command type of NOTIFY into something reasonable if the app supplied an invalid response.
*/
void standardiseCtypeNotify(avrcp_response_type *response);


/****************************************************************************
NAME
    combineToUint32

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
    getMessageDataFromPacket

DESCRIPTION
    Get the required data from a potentially fragmented packet that can be sent up to the app in the form of a message.
*/
bool getMessageDataFromPacket(uint16 ctp_packet_type, uint16 meta_packet_type, uint16 size_data, const uint8 *ptr, uint16 *total_packets, uint16 *data_offset, uint8 *data);


/****************************************************************************
NAME
    getResponseStatus

DESCRIPTION
    Get the Status from AVRCP Response Packet
*/
bool getResponseStatus(uint16 ctp_packet_type, const uint8 *ptr, avrcp_response_type *response);


/****************************************************************************
NAME
    avrcpSendCommonMetadataCfm

DESCRIPTION
    
*/
void avrcpSendCommonMetadataCfm(AVRCP *avrcp, avrcp_status_code status, uint16 transaction, uint16 id, Source source, uint16 packet_size);


/****************************************************************************
NAME
    avrcpSendCommonFragmentedMetadataCfm

DESCRIPTION
    
*/
void avrcpSendCommonFragmentedMetadataCfm(AVRCP *avrcp, avrcp_status_code status, bool response, uint16 id, uint16 transaction, uint16 total_packets, uint16 ctp_packet_type, uint16 metadata_packet_type, uint16 number_of_attributes, uint16 data_length, uint16 data_offset, Source source);


/****************************************************************************
NAME
    avrcpSendCommonMetadataInd

DESCRIPTION
    
*/
void avrcpSendCommonMetadataInd(AVRCP *avrcp, uint16 id, uint16 transaction);


/****************************************************************************
NAME
    avrcpSendCommonFragmentedMetadataInd

DESCRIPTION
    
*/
void avrcpSendCommonFragmentedMetadataInd(AVRCP *avrcp, uint16 id, uint16 transaction, uint16 total_packets, uint16 ctp_packet_type, uint16 metadata_packet_type, uint16 number_of_data_items, uint16 data_length, uint16 data_offset, Source source);


#endif /* AVRCP_COMMON_H_ */
